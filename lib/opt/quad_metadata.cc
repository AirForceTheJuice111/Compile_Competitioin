#include "quad_metadata.hh"

#include <algorithm>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

#include "temp.hh"

namespace quad {
namespace {

void addTermUse(QuadTerm *term, std::set<Temp *> &uses) {
    if (term != nullptr && term->kind == QuadTermKind::TEMP &&
        term->get_temp() != nullptr && term->get_temp()->temp != nullptr) {
        uses.insert(term->get_temp()->temp);
    }
}

void addCallUses(QuadCall *call, std::set<Temp *> &uses) {
    if (call == nullptr) return;
    addTermUse(call->obj_term, uses);
    if (call->args != nullptr) {
        for (auto *arg : *call->args) addTermUse(arg, uses);
    }
}

void addExtCallUses(QuadExtCall *call, std::set<Temp *> &uses) {
    if (call == nullptr || call->args == nullptr) return;
    for (auto *arg : *call->args) addTermUse(arg, uses);
}

void rebuildStatementMetadata(QuadStm *stm) {
    if (stm == nullptr) return;
    auto *defs = new std::set<Temp *>();
    auto *uses = new std::set<Temp *>();
    switch (stm->kind) {
    case QuadKind::MOVE: {
        auto *s = static_cast<QuadMove *>(stm);
        if (s->dst != nullptr && s->dst->temp != nullptr) defs->insert(s->dst->temp);
        addTermUse(s->src, *uses);
        break;
    }
    case QuadKind::LOAD: {
        auto *s = static_cast<QuadLoad *>(stm);
        if (s->dst != nullptr && s->dst->temp != nullptr) defs->insert(s->dst->temp);
        addTermUse(s->src, *uses);
        break;
    }
    case QuadKind::STORE: {
        auto *s = static_cast<QuadStore *>(stm);
        addTermUse(s->src, *uses);
        addTermUse(s->dst, *uses);
        break;
    }
    case QuadKind::MOVE_BINOP: {
        auto *s = static_cast<QuadMoveBinop *>(stm);
        if (s->dst != nullptr && s->dst->temp != nullptr) defs->insert(s->dst->temp);
        addTermUse(s->left, *uses);
        addTermUse(s->right, *uses);
        break;
    }
    case QuadKind::CALL: {
        auto *s = static_cast<QuadCall *>(stm);
        addCallUses(s, *uses);
        break;
    }
    case QuadKind::MOVE_CALL: {
        auto *s = static_cast<QuadMoveCall *>(stm);
        if (s->dst != nullptr && s->dst->temp != nullptr) defs->insert(s->dst->temp);
        addCallUses(s->call, *uses);
        if (s->call != nullptr) {
            s->call->def = new std::set<Temp *>();
            s->call->use = new std::set<Temp *>(*uses);
        }
        break;
    }
    case QuadKind::EXTCALL: {
        auto *s = static_cast<QuadExtCall *>(stm);
        addExtCallUses(s, *uses);
        break;
    }
    case QuadKind::MOVE_EXTCALL: {
        auto *s = static_cast<QuadMoveExtCall *>(stm);
        if (s->dst != nullptr && s->dst->temp != nullptr) defs->insert(s->dst->temp);
        addExtCallUses(s->extcall, *uses);
        if (s->extcall != nullptr) {
            s->extcall->def = new std::set<Temp *>();
            s->extcall->use = new std::set<Temp *>(*uses);
        }
        break;
    }
    case QuadKind::CJUMP: {
        auto *s = static_cast<QuadCJump *>(stm);
        addTermUse(s->left, *uses);
        addTermUse(s->right, *uses);
        break;
    }
    case QuadKind::PHI: {
        auto *s = static_cast<QuadPhi *>(stm);
        if (s->temp_exp != nullptr && s->temp_exp->temp != nullptr) defs->insert(s->temp_exp->temp);
        if (s->args != nullptr) {
            for (const auto &arg : *s->args) if (arg.first != nullptr) uses->insert(arg.first);
        }
        break;
    }
    case QuadKind::RETURN:
        addTermUse(static_cast<QuadReturn *>(stm)->exp, *uses);
        break;
    case QuadKind::PTR_CALC: {
        auto *s = static_cast<QuadPtrCalc *>(stm);
        if (s->dst != nullptr && s->dst->kind == QuadTermKind::TEMP &&
            s->dst->get_temp() != nullptr && s->dst->get_temp()->temp != nullptr) {
            defs->insert(s->dst->get_temp()->temp);
        }
        addTermUse(s->ptr, *uses);
        addTermUse(s->offset, *uses);
        break;
    }
    default:
        break;
    }
    stm->def = defs;
    stm->use = uses;
}

QuadTemp *definedTemp(QuadStm *stm) {
    if (stm == nullptr) return nullptr;
    switch (stm->kind) {
    case QuadKind::MOVE: return static_cast<QuadMove *>(stm)->dst;
    case QuadKind::LOAD: return static_cast<QuadLoad *>(stm)->dst;
    case QuadKind::MOVE_BINOP: return static_cast<QuadMoveBinop *>(stm)->dst;
    case QuadKind::MOVE_CALL: return static_cast<QuadMoveCall *>(stm)->dst;
    case QuadKind::MOVE_EXTCALL: return static_cast<QuadMoveExtCall *>(stm)->dst;
    case QuadKind::PHI: return static_cast<QuadPhi *>(stm)->temp_exp;
    case QuadKind::PTR_CALC: {
        auto *dst = static_cast<QuadPtrCalc *>(stm)->dst;
        return dst != nullptr && dst->kind == QuadTermKind::TEMP ? dst->get_temp() : nullptr;
    }
    default: return nullptr;
    }
}

bool isPureDefinition(QuadStm *stm) {
    return stm != nullptr && (stm->kind == QuadKind::MOVE ||
        stm->kind == QuadKind::MOVE_BINOP || stm->kind == QuadKind::PHI ||
        stm->kind == QuadKind::PTR_CALC);
}

void rewriteTerm(QuadTerm *&term, const std::map<int, int> &replacements) {
    if (term == nullptr || term->kind != QuadTermKind::TEMP || term->get_temp() == nullptr ||
        term->get_temp()->temp == nullptr) return;
    int current = term->get_temp()->temp->num;
    std::set<int> visited;
    while (visited.insert(current).second) {
        auto found = replacements.find(current);
        if (found == replacements.end() || found->second == current) break;
        current = found->second;
    }
    if (current != term->get_temp()->temp->num) {
        term = new QuadTerm(new QuadTemp(new Temp(current), term->get_temp()->type));
    }
}

} // namespace

void rebuildQuadFunctionMetadata(QuadFuncDecl *func) {
    if (func == nullptr || func->quadblocklist == nullptr) return;
    int maxTemp = func->last_temp_num;
    int maxLabel = func->last_label_num;
    if (func->params != nullptr) {
        for (auto *param : *func->params) if (param != nullptr) maxTemp = std::max(maxTemp, param->num);
    }
    for (auto *block : *func->quadblocklist) {
        if (block == nullptr) continue;
        if (block->entry_label != nullptr) maxLabel = std::max(maxLabel, block->entry_label->num);
        if (block->quadlist == nullptr) continue;
        for (auto *stm : *block->quadlist) {
            rebuildStatementMetadata(stm);
            if (stm == nullptr) continue;
            if (stm->def != nullptr) for (auto *temp : *stm->def) if (temp != nullptr) maxTemp = std::max(maxTemp, temp->num);
            if (stm->use != nullptr) for (auto *temp : *stm->use) if (temp != nullptr) maxTemp = std::max(maxTemp, temp->num);
            if (stm->kind == QuadKind::LABEL) {
                auto *label = static_cast<QuadLabel *>(stm)->label;
                if (label != nullptr) maxLabel = std::max(maxLabel, label->num);
            }
        }
        auto *exits = new std::vector<Label *>();
        QuadStm *last = block->quadlist->empty() ? nullptr : block->quadlist->back();
        if (last != nullptr && last->kind == QuadKind::JUMP) {
            auto *label = static_cast<QuadJump *>(last)->label;
            if (label != nullptr) exits->push_back(label);
        } else if (last != nullptr && last->kind == QuadKind::CJUMP) {
            auto *jump = static_cast<QuadCJump *>(last);
            if (jump->t != nullptr) exits->push_back(jump->t);
            if (jump->f != nullptr && (jump->t == nullptr || jump->f->num != jump->t->num)) exits->push_back(jump->f);
        } else if (last == nullptr || (last->kind != QuadKind::RETURN &&
                   !(last->kind == QuadKind::EXTCALL &&
                     static_cast<QuadExtCall *>(last)->extfun == "exit"))) {
            // Optimizers in this file do not alter control flow. Retain an
            // explicit fallthrough edge if the block has no terminal.
            if (block->exit_labels != nullptr) *exits = *block->exit_labels;
        }
        block->exit_labels = exits;
        for (auto *label : *exits) if (label != nullptr) maxLabel = std::max(maxLabel, label->num);
    }
    func->last_temp_num = maxTemp;
    func->last_label_num = maxLabel;
}

void rebuildQuadProgramMetadata(QuadProgram *program) {
    if (program == nullptr || program->quadFuncDeclList == nullptr) return;
    int maxTemp = program->last_temp_num;
    int maxLabel = program->last_label_num;
    for (auto *func : *program->quadFuncDeclList) {
        rebuildQuadFunctionMetadata(func);
        if (func != nullptr) {
            maxTemp = std::max(maxTemp, func->last_temp_num);
            maxLabel = std::max(maxLabel, func->last_label_num);
        }
    }
    program->last_temp_num = maxTemp;
    program->last_label_num = maxLabel;
}

bool verifySsaFunction(QuadFuncDecl *func, ControlFlowInfo *cfi, std::string *reason) {
    auto fail = [&](const std::string &message) {
        if (reason != nullptr) *reason = message;
        return false;
    };
    if (func == nullptr || cfi == nullptr || func->quadblocklist == nullptr ||
        cfi->entryBlock < 0 || cfi->allBlocks.count(cfi->entryBlock) == 0) {
        return fail("missing function or CFG entry");
    }
    struct DefSite { int block = -1; int index = -1; QuadType type = QuadType::INT; };
    std::map<int, DefSite> definitions;
    std::map<int, QuadType> types;
    auto noteType = [&](int temp, QuadType type) {
        auto found = types.find(temp);
        if (found != types.end() && found->second != type) return false;
        types[temp] = type;
        return true;
    };
    std::set<int> params;
    if (func->params != nullptr) for (auto *param : *func->params) {
        if (param == nullptr || !params.insert(param->num).second) return fail("duplicate parameter temp");
        definitions[param->num] = DefSite{cfi->entryBlock, -1, QuadType::INT};
    }
    std::map<QuadBlock *, int> blockLabels;
    for (auto *block : *func->quadblocklist) {
        if (block == nullptr || block->entry_label == nullptr || block->quadlist == nullptr) return fail("malformed block");
        int label = block->entry_label->num;
        if (blockLabels.count(block) != 0) return fail("duplicate block");
        blockLabels[block] = label;
        for (std::size_t i = 0; i < block->quadlist->size(); ++i) {
            auto *def = definedTemp(block->quadlist->at(i));
            if (def == nullptr || def->temp == nullptr) continue;
            int number = def->temp->num;
            if (params.count(number) != 0 || definitions.count(number) != 0) return fail("temp has multiple definitions");
            if (!noteType(number, def->type)) return fail("definition type mismatch");
            definitions[number] = DefSite{label, static_cast<int>(i), def->type};
        }
    }
    auto checkUse = [&](int temp, QuadType type, int block, int index) {
        if (!noteType(temp, type)) return false;
        auto def = definitions.find(temp);
        if (def == definitions.end()) return false;
        auto doms = cfi->dominators.find(block);
        if (doms == cfi->dominators.end() || doms->second.count(def->second.block) == 0) return false;
        return def->second.block != block || def->second.index < index;
    };
    auto checkTerm = [&](QuadTerm *term, QuadType expected, int block, int index) {
        if (term == nullptr || term->kind == QuadTermKind::CONST) return true;
        if (term->kind == QuadTermKind::NAME) return expected == QuadType::PTR;
        auto *temp = term->get_temp();
        return temp != nullptr && temp->temp != nullptr && temp->type == expected &&
               checkUse(temp->temp->num, temp->type, block, index);
    };
    for (auto *block : *func->quadblocklist) {
        int label = block->entry_label->num;
        for (std::size_t i = 0; i < block->quadlist->size(); ++i) {
            auto *stm = block->quadlist->at(i);
            if (stm == nullptr) return fail("null statement");
            int index = static_cast<int>(i);
            bool ok = true;
            switch (stm->kind) {
            case QuadKind::MOVE: {
                auto *s = static_cast<QuadMove *>(stm); ok = s->dst != nullptr && checkTerm(s->src, s->dst->type, label, index); break;
            }
            case QuadKind::LOAD: ok = checkTerm(static_cast<QuadLoad *>(stm)->src, QuadType::PTR, label, index); break;
            case QuadKind::STORE: {
                auto *s = static_cast<QuadStore *>(stm); ok = checkTerm(s->dst, QuadType::PTR, label, index);
                if (ok && s->src != nullptr && s->src->kind == QuadTermKind::TEMP) {
                    auto *t = s->src->get_temp(); ok = t != nullptr && checkUse(t->temp->num, t->type, label, index);
                }
                break;
            }
            case QuadKind::MOVE_BINOP: {
                auto *s = static_cast<QuadMoveBinop *>(stm); ok = checkTerm(s->left, QuadType::INT, label, index) && checkTerm(s->right, QuadType::INT, label, index); break;
            }
            case QuadKind::CALL: {
                auto *s = static_cast<QuadCall *>(stm); ok = checkTerm(s->obj_term, QuadType::PTR, label, index);
                if (ok && s->args != nullptr) for (auto *arg : *s->args) if (arg != nullptr && arg->kind == QuadTermKind::TEMP) {
                    auto *t = arg->get_temp(); if (t == nullptr || !checkUse(t->temp->num, t->type, label, index)) { ok = false; break; }
                }
                break;
            }
            case QuadKind::MOVE_CALL: {
                auto *s = static_cast<QuadMoveCall *>(stm); auto *call = s->call; ok = call != nullptr && checkTerm(call->obj_term, QuadType::PTR, label, index);
                if (ok && call->args != nullptr) for (auto *arg : *call->args) if (arg != nullptr && arg->kind == QuadTermKind::TEMP) {
                    auto *t = arg->get_temp(); if (t == nullptr || !checkUse(t->temp->num, t->type, label, index)) { ok = false; break; }
                }
                break;
            }
            case QuadKind::EXTCALL:
            case QuadKind::MOVE_EXTCALL: {
                QuadExtCall *call = stm->kind == QuadKind::EXTCALL ? static_cast<QuadExtCall *>(stm) : static_cast<QuadMoveExtCall *>(stm)->extcall;
                ok = call != nullptr; if (ok && call->args != nullptr) for (auto *arg : *call->args) if (arg != nullptr && arg->kind == QuadTermKind::TEMP) {
                    auto *t = arg->get_temp(); if (t == nullptr || !checkUse(t->temp->num, t->type, label, index)) { ok = false; break; }
                }
                break;
            }
            case QuadKind::CJUMP: {
                auto *s = static_cast<QuadCJump *>(stm); QuadType lt = s->left != nullptr && s->left->kind == QuadTermKind::TEMP ? s->left->get_temp()->type : QuadType::INT;
                QuadType rt = s->right != nullptr && s->right->kind == QuadTermKind::TEMP ? s->right->get_temp()->type : lt;
                ok = checkTerm(s->left, lt, label, index) && checkTerm(s->right, rt, label, index); break;
            }
            case QuadKind::RETURN: {
                auto *s = static_cast<QuadReturn *>(stm); ok = checkTerm(s->exp, func->return_type, label, index); break;
            }
            case QuadKind::PTR_CALC: {
                auto *s = static_cast<QuadPtrCalc *>(stm); ok = checkTerm(s->ptr, QuadType::PTR, label, index) && checkTerm(s->offset, QuadType::INT, label, index); break;
            }
            case QuadKind::PHI: {
                auto *s = static_cast<QuadPhi *>(stm); ok = s->temp_exp != nullptr && s->args != nullptr;
                std::set<int> seenPreds;
                if (ok) for (const auto &arg : *s->args) {
                    int pred = arg.second == nullptr ? -1 : arg.second->num;
                    auto def = arg.first == nullptr ? definitions.end() : definitions.find(arg.first->num);
                    if (pred < 0 || !seenPreds.insert(pred).second || cfi->predecessors[label].count(pred) == 0 || def == definitions.end() ||
                        !noteType(arg.first->num, s->temp_exp->type) || cfi->dominators[pred].count(def->second.block) == 0) { ok = false; break; }
                }
                if (ok && seenPreds != cfi->predecessors[label]) ok = false;
                break;
            }
            default: break;
            }
            if (!ok) return fail("use is undefined, non-dominating, or type-inconsistent");
        }
    }
    return true;
}

void rewriteQuadUses(QuadFuncDecl *func, const std::map<int, int> &replacements) {
    if (func == nullptr || func->quadblocklist == nullptr || replacements.empty()) return;
    for (auto *block : *func->quadblocklist) if (block != nullptr && block->quadlist != nullptr) {
        for (auto *stm : *block->quadlist) {
            rewriteQuadStatementUses(stm, replacements);
        }
    }
}

void rewriteQuadStatementUses(QuadStm *stm,
                              const std::map<int, int> &replacements) {
            if (stm == nullptr || replacements.empty()) return;
            switch (stm->kind) {
            case QuadKind::MOVE: rewriteTerm(static_cast<QuadMove *>(stm)->src, replacements); break;
            case QuadKind::LOAD: rewriteTerm(static_cast<QuadLoad *>(stm)->src, replacements); break;
            case QuadKind::STORE: { auto *s = static_cast<QuadStore *>(stm); rewriteTerm(s->src, replacements); rewriteTerm(s->dst, replacements); break; }
            case QuadKind::MOVE_BINOP: { auto *s = static_cast<QuadMoveBinop *>(stm); rewriteTerm(s->left, replacements); rewriteTerm(s->right, replacements); break; }
            case QuadKind::CALL: { auto *s = static_cast<QuadCall *>(stm); rewriteTerm(s->obj_term, replacements); if (s->args) for (auto *&arg : *s->args) rewriteTerm(arg, replacements); break; }
            case QuadKind::MOVE_CALL: { auto *s = static_cast<QuadMoveCall *>(stm); if (s->call) { rewriteTerm(s->call->obj_term, replacements); if (s->call->args) for (auto *&arg : *s->call->args) rewriteTerm(arg, replacements); } break; }
            case QuadKind::EXTCALL: { auto *s = static_cast<QuadExtCall *>(stm); if (s->args) for (auto *&arg : *s->args) rewriteTerm(arg, replacements); break; }
            case QuadKind::MOVE_EXTCALL: { auto *s = static_cast<QuadMoveExtCall *>(stm); if (s->extcall && s->extcall->args) for (auto *&arg : *s->extcall->args) rewriteTerm(arg, replacements); break; }
            case QuadKind::CJUMP: { auto *s = static_cast<QuadCJump *>(stm); rewriteTerm(s->left, replacements); rewriteTerm(s->right, replacements); break; }
            case QuadKind::RETURN: rewriteTerm(static_cast<QuadReturn *>(stm)->exp, replacements); break;
            case QuadKind::PTR_CALC: { auto *s = static_cast<QuadPtrCalc *>(stm); rewriteTerm(s->ptr, replacements); rewriteTerm(s->offset, replacements); break; }
            case QuadKind::PHI: { auto *s = static_cast<QuadPhi *>(stm); if (s->args) for (auto &arg : *s->args) {
                int current = arg.first == nullptr ? -1 : arg.first->num; std::set<int> visited;
                while (current >= 0 && visited.insert(current).second) { auto found = replacements.find(current); if (found == replacements.end() || found->second == current) break; current = found->second; }
                if (arg.first != nullptr && current != arg.first->num) arg.first = new Temp(current);
            } break; }
            default: break;
            }
}

int eliminateDeadPureQuadDefs(QuadFuncDecl *func) {
    if (func == nullptr || func->quadblocklist == nullptr) return 0;
    int eliminated = 0;
    bool changed = true;
    while (changed) {
        changed = false;
        rebuildQuadFunctionMetadata(func);
        std::map<int, int> uses;
        for (auto *block : *func->quadblocklist) if (block && block->quadlist) for (auto *stm : *block->quadlist) if (stm && stm->use) {
            for (auto *temp : *stm->use) if (temp != nullptr) ++uses[temp->num];
        }
        for (auto *block : *func->quadblocklist) if (block && block->quadlist) {
            auto *kept = new std::vector<QuadStm *>();
            for (auto *stm : *block->quadlist) {
                auto *def = definedTemp(stm);
                if (isPureDefinition(stm) && def != nullptr && def->temp != nullptr && uses[def->temp->num] == 0) {
                    ++eliminated; changed = true;
                } else {
                    kept->push_back(stm);
                }
            }
            block->quadlist = kept;
        }
    }
    rebuildQuadFunctionMetadata(func);
    return eliminated;
}

} // namespace quad
