#define DEBUG
#undef DEBUG

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "treep.hh"
#include "quad.hh"
#include "tree2quad.hh"

using namespace std;
using namespace tree;
using namespace quad;

/*
 * Instruction selection (pattern matching) to convert the IR tree to Quad.
 * For statements: visit adds QuadStm* to visit_result.
 * For expressions: visit sets output_term (a QuadTerm*) and may add intermediate QuadStm*.
 */

// Helper: convert tree::Type to quad::QuadType
static QuadType toQuadType(tree::Type t) {
    return (t == tree::Type::INT) ? QuadType::INT : QuadType::PTR;
}

// Helper: if term is a TEMP, add its Temp* to the use set
static void addTermUse(set<Temp*> *s, QuadTerm *term) {
    if (term && term->kind == QuadTermKind::TEMP) s->insert(term->get_temp()->temp);
}

// Helper: split a flat list of QuadStm into basic blocks
static vector<QuadBlock*>* splitBlocks(vector<QuadStm*> *stms, Temp_map *tmap) {
    auto blocks = new vector<QuadBlock*>();
    if (!stms || stms->empty()) return blocks;

    // Ensure the first statement is a label
    if (stms->front()->kind != QuadKind::LABEL) {
        auto lbl = tmap->newlabel();
        stms->insert(stms->begin(),
            new QuadLabel(lbl, new set<Temp*>(), new set<Temp*>()));
    }

    vector<QuadStm*> *cur = nullptr;
    Label *entry = nullptr;

    for (size_t i = 0; i < stms->size(); i++) {
        auto stm = stms->at(i);

        if (stm->kind == QuadKind::LABEL) {
            // Close the previous block (fall-through – exit to this label)
            if (cur && !cur->empty()) {
                auto exits = new vector<Label*>();
                exits->push_back(static_cast<QuadLabel*>(stm)->label);
                blocks->push_back(new QuadBlock(cur, entry, exits));
            }
            cur = new vector<QuadStm*>();
            entry = static_cast<QuadLabel*>(stm)->label;
            cur->push_back(stm);
        } else { // QuadStm that is not a label, must belong to the current block
            if (!cur) { cur = new vector<QuadStm*>(); entry = nullptr; }
            cur->push_back(stm);

            if (stm->kind == QuadKind::JUMP) {
                auto exits = new vector<Label*>();
                exits->push_back(static_cast<QuadJump*>(stm)->label);
                blocks->push_back(new QuadBlock(cur, entry, exits));
                cur = nullptr; entry = nullptr;
            } else if (stm->kind == QuadKind::CJUMP) {
                auto cj = static_cast<QuadCJump*>(stm);
                auto exits = new vector<Label*>();
                exits->push_back(cj->t);
                exits->push_back(cj->f);
                blocks->push_back(new QuadBlock(cur, entry, exits));
                cur = nullptr; entry = nullptr;
            } else if (stm->kind == QuadKind::RETURN) {
                blocks->push_back(new QuadBlock(cur, entry, new vector<Label*>()));
                cur = nullptr; entry = nullptr;
            }
        }
    }
    // Close any remaining un-terminated block
    if (cur && !cur->empty())
        blocks->push_back(new QuadBlock(cur, entry, new vector<Label*>()));

    return blocks;
}

QuadProgram* tree2quad(Program* prog) {
    if (!prog) return nullptr;
    Tree2Quad v;
    prog->accept(v);
    return v.quadprog;
}

void Tree2Quad::visit(tree::Program *prog) {
    auto funcs = new vector<QuadFuncDecl*>();
    int max_label = 0, max_temp = 0;

    if (prog && prog->funcdecllist) {
        for (auto func : *prog->funcdecllist) {
            if (!func) continue;

            // Reset temp_map for each function
            temp_map = new Temp_map();
            temp_map->next_temp = func->last_temp_num + 1;
            temp_map->next_label = func->last_label_num + 1;

            // Collect quad statements into visit_result
            visit_result = new vector<QuadStm*>();
            if (func->stm) func->stm->accept(*this);

            // Organize flat statement list into basic blocks
            auto blocks = splitBlocks(visit_result, temp_map);

            int fn_last_label = temp_map->next_label - 1;
            int fn_last_temp = temp_map->next_temp - 1;
            funcs->push_back(new QuadFuncDecl(
                func->name, func->args, blocks, fn_last_label, fn_last_temp
            ));
            if (fn_last_label > max_label) max_label = fn_last_label;
            if (fn_last_temp > max_temp) max_temp = fn_last_temp;
        }
    }
    quadprog = new QuadProgram(funcs, max_label, max_temp);
}
void Tree2Quad::visit(tree::FuncDecl *func) {
    // Handled inline by visit(Program)
}
void Tree2Quad::visit(tree::Jump *jump) {
    if (!jump) return;
    visit_result->push_back(
        new QuadJump(jump->label, new set<Temp*>(), new set<Temp*>())
    );
}
void Tree2Quad::visit(tree::Cjump *cjump) {
    if (!cjump) return;
    cjump->left->accept(*this);
    auto left_term = output_term;
    cjump->right->accept(*this);
    auto right_term = output_term;

    auto use = new set<Temp*>();
    addTermUse(use, left_term);
    addTermUse(use, right_term);

    visit_result->push_back(
        new QuadCJump(cjump->relop, left_term, right_term,
                      cjump->t, cjump->f, new set<Temp*>(), use)
    );
}
void Tree2Quad::visit(tree::Move *move) {
    if (!move) return;

    if (move->dst->getTreeKind() == Kind::MEM) {
        // ===== STORE: mem(addr) <- src =====
        auto mem_node = static_cast<tree::Mem*>(move->dst);
        mem_node->mem->accept(*this);
        auto addr_term = output_term;

        move->src->accept(*this);
        auto src_term = output_term;

        auto use = new set<Temp*>();
        addTermUse(use, addr_term);
        addTermUse(use, src_term);

        visit_result->push_back(
            new QuadStore(src_term, addr_term, new set<Temp*>(), use)
        );

    } else if (move->dst->getTreeKind() == Kind::TEMPEXP) {
        // ===== Move to a temp: pattern-match on src =====
        auto dst_texp = static_cast<tree::TempExp*>(move->dst);
        auto dst_type = toQuadType(dst_texp->type);
        auto dst = new QuadTemp(dst_texp->temp, dst_type);
        auto src = move->src;
        auto src_kind = src->getTreeKind();

        if (src_kind == Kind::CALL) {
            // MOVE_CALL: temp <- call(obj, args...)
            auto call = static_cast<tree::Call*>(src);
            call->obj->accept(*this);
            auto obj_term = output_term;

            auto args = new vector<QuadTerm*>();
            auto call_use = new set<Temp*>();
            addTermUse(call_use, obj_term);
            for (auto a : *call->args) {
                a->accept(*this);
                args->push_back(output_term);
                addTermUse(call_use, output_term);
            }

            auto inner = new QuadCall( // inner call for MOVE_CALL
                call->id, obj_term, args,
                new set<Temp*>(), new set<Temp*>(*call_use)
            );
            auto def = new set<Temp*>();
            def->insert(dst->temp);
            visit_result->push_back(
                new QuadMoveCall(dst, inner, def, call_use)
            );

        } else if (src_kind == Kind::EXTCALL) {
            // MOVE_EXTCALL: temp <- extcall(args...)
            auto extcall = static_cast<tree::ExtCall*>(src);
            auto args = new vector<QuadTerm*>();
            auto ext_use = new set<Temp*>();
            for (auto a : *extcall->args) {
                a->accept(*this);
                args->push_back(output_term);
                addTermUse(ext_use, output_term);
            }

            auto inner = new QuadExtCall(
                extcall->extfun, args,
                new set<Temp*>(), new set<Temp*>(*ext_use)
            );
            auto def = new set<Temp*>();
            def->insert(dst->temp);
            visit_result->push_back(
                new QuadMoveExtCall(dst, inner, def, ext_use)
            );

        } else if (src_kind == Kind::MEM) {
            // LOAD: temp <- mem(addr)
            auto mem_node = static_cast<tree::Mem*>(src);
            mem_node->mem->accept(*this);
            auto addr_term = output_term;

            auto def = new set<Temp*>();
            def->insert(dst->temp);
            auto use = new set<Temp*>();
            addTermUse(use, addr_term);
            visit_result->push_back(
                new QuadLoad(dst, addr_term, def, use)
            );

        } else if (src_kind == Kind::BINOP) {
            // MOVE_BINOP or PTR_CALC: temp <- left op right
            auto binop = static_cast<tree::Binop*>(src);
            binop->left->accept(*this);
            auto left_term = output_term;
            binop->right->accept(*this);
            auto right_term = output_term;

            auto def = new set<Temp*>();
            def->insert(dst->temp);
            auto use = new set<Temp*>();
            addTermUse(use, left_term);
            addTermUse(use, right_term);

            if (binop->type == tree::Type::PTR && binop->op == "+") {
                // PTR_CALC: temp <- ptr + offset
                auto dst_term = new QuadTerm(new QuadTemp(dst->temp, dst_type));
                visit_result->push_back(
                    new QuadPtrCalc(dst_term, left_term, right_term, def, use)
                );
            } else {
                // MOVE_BINOP: temp <- left op right
                visit_result->push_back(
                    new QuadMoveBinop(dst, left_term, binop->op, right_term, def, use)
                );
            }

        } else {
            // Simple MOVE: temp <- term (Const, Name, TempExp, Eseq)
            src->accept(*this);
            auto src_term = output_term;

            auto def = new set<Temp*>();
            def->insert(dst->temp);
            auto use = new set<Temp*>();
            addTermUse(use, src_term);

            visit_result->push_back(
                new QuadMove(dst, src_term, def, use)
            );
        }
    }
}
void Tree2Quad::visit(tree::Seq *seq) {
    if (!seq || !seq->sl) return;
    for (auto stm : *seq->sl) {
        if (stm) stm->accept(*this);
    }
}
void Tree2Quad::visit(tree::LabelStm *labelstm) {
    if (!labelstm) return;
    visit_result->push_back(
        new QuadLabel(labelstm->label, new set<Temp*>(), new set<Temp*>())
    );
}
void Tree2Quad::visit(tree::Return *ret) {
    if (!ret) return;
    ret->exp->accept(*this);
    auto term = output_term;

    auto use = new set<Temp*>();
    addTermUse(use, term);

    visit_result->push_back(
        new QuadReturn(term, new set<Temp*>(), use)
    );
}
void Tree2Quad::visit(tree::ExpStm *expstm) { // ExpStm: visit for side effects, ignore result
    if (!expstm || !expstm->exp) return;
    auto exp = expstm->exp;

    if (exp->getTreeKind() == Kind::CALL) {
        // ExpStm(Call) → QuadCall (result ignored)
        auto call = static_cast<tree::Call*>(exp);
        call->obj->accept(*this);
        auto obj_term = output_term;

        auto args = new vector<QuadTerm*>();
        auto use = new set<Temp*>();
        addTermUse(use, obj_term);
        for (auto a : *call->args) {
            a->accept(*this);
            args->push_back(output_term);
            addTermUse(use, output_term);
        }
        visit_result->push_back(
            new QuadCall(call->id, obj_term, args, new set<Temp*>(), use)
        );
    } else if (exp->getTreeKind() == Kind::EXTCALL) {
        // ExpStm(ExtCall) → QuadExtCall (result ignored)
        auto extcall = static_cast<tree::ExtCall*>(exp);
        auto args = new vector<QuadTerm*>();
        auto use = new set<Temp*>();
        for (auto a : *extcall->args) {
            a->accept(*this);
            args->push_back(output_term);
            addTermUse(use, output_term);
        }
        visit_result->push_back(
            new QuadExtCall(extcall->extfun, args, new set<Temp*>(), use)
        );
    } else {
        // Other expressions – visit for side effects, result ignored
        exp->accept(*this);
    }
}
void Tree2Quad::visit(tree::Binop *binop) {
    if (!binop) return;
    binop->left->accept(*this);
    auto left_term = output_term;
    binop->right->accept(*this);
    auto right_term = output_term;

    auto new_temp = temp_map->newtemp(); // Quad also uses tree::Temp for temps. we need to allocate a new temp for the result of this binop
    auto result_type = toQuadType(binop->type);
    auto dst = new QuadTemp(new_temp, result_type);

    auto def = new set<Temp*>();
    def->insert(new_temp);
    auto use = new set<Temp*>();
    addTermUse(use, left_term);
    addTermUse(use, right_term);

    if (binop->type == tree::Type::PTR && binop->op == "+") {
        // Pointer arithmetic → PTR_CALC
        auto dst_term = new QuadTerm(dst);
        visit_result->push_back(
            new QuadPtrCalc(dst_term, left_term, right_term, def, use)
        );
    } else {
        // Regular arithmetic → MOVE_BINOP
        visit_result->push_back(
            new QuadMoveBinop(dst, left_term, binop->op, right_term, def, use)
        );
    }

    output_term = new QuadTerm(new QuadTemp(new_temp, result_type)); // newing a QuadTemp again instead of using dst_term is ok because QuadTerm equality is based on the temp's number, not the object!
}
void Tree2Quad::visit(tree::Mem *mem) { // Mem[addr]
    if (!mem) return;
    // Visit the address expression
    mem->mem->accept(*this);
    auto addr_term = output_term;

    // Load from memory into a new temp
    auto new_temp = temp_map->newtemp();
    auto result_type = toQuadType(mem->type);
    auto dst = new QuadTemp(new_temp, result_type);

    auto def = new set<Temp*>();
    def->insert(new_temp);
    auto use = new set<Temp*>();
    addTermUse(use, addr_term);

    visit_result->push_back(
        new QuadLoad(dst, addr_term, def, use)
    );

    output_term = new QuadTerm(new QuadTemp(new_temp, result_type));
}
void Tree2Quad::visit(tree::TempExp *tempexp) {
    if (!tempexp) return;
    auto qt = new QuadTemp(tempexp->temp, toQuadType(tempexp->type));
    output_term = new QuadTerm(qt);
}
void Tree2Quad::visit(tree::Eseq *eseq) {
    if (!eseq) return;
    // Statement part adds quads to visit_result
    if (eseq->stm) eseq->stm->accept(*this);
    // Expression part sets output_term
    if (eseq->exp) eseq->exp->accept(*this);
}
void Tree2Quad::visit(tree::Name *name) {
    if (!name) return;
    if (name->sname)
        output_term = new QuadTerm(name->sname->str());
    else if (name->name)
        output_term = new QuadTerm(name->name->str());
}
void Tree2Quad::visit(tree::Const *node) {
    output_term = new QuadTerm(node->constVal);
}
void Tree2Quad::visit(tree::Call *call) {
    if (!call) return;
    // Call as expression: result goes to a new temp
    call->obj->accept(*this);
    auto obj_term = output_term;

    auto args = new vector<QuadTerm*>();
    auto use = new set<Temp*>();
    addTermUse(use, obj_term);
    for (auto a : *call->args) {
        a->accept(*this);
        args->push_back(output_term);
        addTermUse(use, output_term);
    }

    auto new_temp = temp_map->newtemp();
    auto result_type = toQuadType(call->type);
    auto dst = new QuadTemp(new_temp, result_type);

    auto inner = new QuadCall(
        call->id, obj_term, args,
        new set<Temp*>(), new set<Temp*>(*use)
    );
    auto def = new set<Temp*>();
    def->insert(new_temp);

    visit_result->push_back(
        new QuadMoveCall(dst, inner, def, use)
    );
    output_term = new QuadTerm(new QuadTemp(new_temp, result_type));
}
void Tree2Quad::visit(tree::ExtCall *extcall) {
    if (!extcall) return;
    // ExtCall as expression: result goes to a new temp
    auto args = new vector<QuadTerm*>();
    auto use = new set<Temp*>();
    for (auto a : *extcall->args) {
        a->accept(*this);
        args->push_back(output_term);
        addTermUse(use, output_term);
    }

    auto new_temp = temp_map->newtemp();
    auto result_type = toQuadType(extcall->type);
    auto dst = new QuadTemp(new_temp, result_type);

    auto inner = new QuadExtCall(
        extcall->extfun, args,
        new set<Temp*>(), new set<Temp*>(*use)
    );
    auto def = new set<Temp*>();
    def->insert(new_temp);

    visit_result->push_back(
        new QuadMoveExtCall(dst, inner, def, use)
    );
    output_term = new QuadTerm(new QuadTemp(new_temp, result_type));
}
