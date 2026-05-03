#define DEBUG
#undef DEBUG

#include <string>
#include <stack>
#include <variant>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include "quad.hh"
#include "opt.hh"

using namespace std;
using namespace tree;
using namespace quad;

static bool isSameRtValue(RtValue lhs, RtValue rhs) {
    if (lhs.getType() != rhs.getType()) return false;
    if (lhs.getType() != ValueType::ONE_VALUE) return true; // both NO_VALUE or both MANY_VALUES
    return lhs.getIntValue() == rhs.getIntValue();
}

static RtValue joinRtValue(RtValue current, RtValue incoming) {
    if (current.getType() == ValueType::MANY_VALUES) return current;
    if (incoming.getType() == ValueType::NO_VALUE) return current;
    if (current.getType() == ValueType::NO_VALUE) return incoming;
    if (incoming.getType() == ValueType::MANY_VALUES) return incoming;
    if (current.getIntValue() == incoming.getIntValue()) return current;
    return RtValue(ValueType::MANY_VALUES);
}

static bool updateRtValue(map<int, RtValue> &temp_value, int temp_num, RtValue incoming) { // update temp_value[temp_num] by joining with incoming; return true if changed
    RtValue current = temp_value.count(temp_num) ? temp_value[temp_num] : RtValue();
    RtValue joined = joinRtValue(current, incoming);
    if (isSameRtValue(current, joined)) return false;
    temp_value[temp_num] = joined;
    return true;
}

static bool updateExecutable(map<int, bool> &block_executable, int label_num) { // update block_executable[label_num] to true; return true if changed
    if (block_executable[label_num]) return false;
    block_executable[label_num] = true;
    return true;
}

static bool isExecutable(const map<int, bool> &block_executable, int label_num) {
    auto it = block_executable.find(label_num);
    return it != block_executable.end() && it->second;
}

static RtValue evalBinop(const string &binop, int left, int right) {
    if (binop == "+") return RtValue(left + right);
    if (binop == "-") return RtValue(left - right);
    if (binop == "*") return RtValue(left * right);
    if (binop == "/") {
        if (right == 0) return RtValue(ValueType::MANY_VALUES);
        return RtValue(left / right);
    }
    if (binop == "%") {
        if (right == 0) return RtValue(ValueType::MANY_VALUES);
        return RtValue(left % right);
    }
    if (binop == "&&") return RtValue((left != 0 && right != 0) ? 1 : 0);
    if (binop == "||") return RtValue((left != 0 || right != 0) ? 1 : 0);
    if (binop == "&") return RtValue(left & right);
    if (binop == "|") return RtValue(left | right);
    if (binop == "^") return RtValue(left ^ right);
    if (binop == "<<") return RtValue(left << right);
    if (binop == ">>") return RtValue(left >> right);
    if (binop == "==") return RtValue(left == right ? 1 : 0);
    if (binop == "!=") return RtValue(left != right ? 1 : 0);
    if (binop == "<") return RtValue(left < right ? 1 : 0);
    if (binop == "<=") return RtValue(left <= right ? 1 : 0);
    if (binop == ">") return RtValue(left > right ? 1 : 0);
    if (binop == ">=") return RtValue(left >= right ? 1 : 0);
    return RtValue(ValueType::MANY_VALUES);
}

static bool evalRelop(const string &relop, int left, int right) { // relop is short for "relational operator", but can also be equality or logical operators used in CJUMP conditions
    if (relop == "==") return left == right;
    if (relop == "!=") return left != right;
    if (relop == "<") return left < right;
    if (relop == "<=") return left <= right;
    if (relop == ">") return left > right;
    if (relop == ">=") return left >= right;
    return true;
}

static QuadTerm *rewriteTerm(Opt *opt, QuadTerm *term) { // rewrite term by replacing TEMP with known constant value when possible; return new term (may be same as input)
    if (term == nullptr) return nullptr;
    if (term->kind != QuadTermKind::TEMP) return term->clone();
    int temp_num = term->get_temp()->temp->num;
    RtValue value = opt->getRtValue(temp_num);
    if (value.getType() == ValueType::ONE_VALUE) return new QuadTerm(value.getIntValue());
    return term->clone();
}

static void rewriteCallLike(Opt *opt, QuadCall *call) {
    if (call == nullptr) return;
    call->obj_term = rewriteTerm(opt, call->obj_term);
    if (call->args == nullptr) return;
    for (auto &arg : *call->args)
        arg = rewriteTerm(opt, arg);
}

static void rewriteExtCallLike(Opt *opt, QuadExtCall *call) {
    if (call == nullptr || call->args == nullptr) return;
    for (auto &arg : *call->args)
        arg = rewriteTerm(opt, arg);
}

static RtValue evalTerm(Opt *opt, QuadTerm *term) {
    if (term == nullptr) return RtValue(ValueType::MANY_VALUES);
    if (term->kind == QuadTermKind::CONST) return RtValue(term->get_const());
    if (term->kind == QuadTermKind::TEMP) return opt->getRtValue(term->get_temp()->temp->num);
    return RtValue(ValueType::MANY_VALUES);
}

// Like evalTerm, but called for uses in REACHABLE blocks outside of phi nodes.
// If the temp still has NO_VALUE at use time, the program contains undefined behavior:
// report the error, promote to MANY_VALUES, and continue (per README).
static RtValue evalTermChecked(Opt *opt, QuadTerm *term, bool &changed) {
    if (term == nullptr || term->kind != QuadTermKind::TEMP) return evalTerm(opt, term);
    int num = term->get_temp()->temp->num;
    RtValue val = opt->getRtValue(num);
    if (val.getType() == ValueType::NO_VALUE) {
        cerr << "Warning: t" << num << " used in reachable block with no determined value (undefined use); promoting to MANY_VALUES" << endl;
        changed |= updateRtValue(opt->temp_value, num, RtValue(ValueType::MANY_VALUES));
        return RtValue(ValueType::MANY_VALUES);
    }
    return val;
}

static RtValue evalPhi(Opt *opt, QuadPhi *phi) { // if phi has no executable inputs, return NO_VALUE; else if all executable inputs have the same ONE_VALUE, return that value; else if any executable input is MANY_VALUES, return MANY_VALUES; else return NO_VALUE (executable inputs with no value)
    RtValue result;
    bool seen_executable_input = false;
    if (phi->args == nullptr) return result;
    for (auto &arg : *phi->args) {
        int pred_label = arg.second->num;
        if (!isExecutable(opt->block_executable, pred_label)) continue;
        seen_executable_input = true;
        RtValue incoming = opt->getRtValue(arg.first->num);
        if (incoming.getType() == ValueType::NO_VALUE) continue;
        if (incoming.getType() == ValueType::MANY_VALUES) return incoming; // all MANY_VALUES RtValues are the same, just return it
        result = joinRtValue(result, incoming);
        if (result.getType() == ValueType::MANY_VALUES) return result;
    }
    if (!seen_executable_input) return RtValue();
    return result;
}

void Opt::calculateBT() { // Backward dataflow to determine executable blocks and constant values. BT = "block tracing"
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto *block : *func->quadblocklist) {
            int label_num = block->entry_label->num;
            if (!isExecutable(block_executable, label_num)) continue;
            for (auto *stm : *block->quadlist) {
                if (stm == nullptr) continue;
                switch (stm->kind) {
                    case QuadKind::MOVE: {
                        auto *move = static_cast<QuadMove*>(stm);
                        changed |= updateRtValue(temp_value, move->dst->temp->num, evalTermChecked(this, move->src, changed));
                        break;
                    }
                    case QuadKind::LOAD: {
                        auto *load = static_cast<QuadLoad*>(stm);
                        changed |= updateRtValue(temp_value, load->dst->temp->num, RtValue(ValueType::MANY_VALUES));
                        break;
                    }
                    case QuadKind::STORE:
                    case QuadKind::CALL:
                    case QuadKind::EXTCALL:
                    case QuadKind::LABEL:
                    case QuadKind::RETURN:
                        break;
                    case QuadKind::MOVE_BINOP: {
                        auto *binop = static_cast<QuadMoveBinop*>(stm);
                        RtValue left = evalTermChecked(this, binop->left, changed);
                        RtValue right = evalTermChecked(this, binop->right, changed);
                        RtValue result;
                        if (left.getType() == ValueType::MANY_VALUES || right.getType() == ValueType::MANY_VALUES)
                            result = RtValue(ValueType::MANY_VALUES);
                        else if (left.getType() == ValueType::ONE_VALUE && right.getType() == ValueType::ONE_VALUE)
                            result = evalBinop(binop->binop, left.getIntValue(), right.getIntValue());
                        changed |= updateRtValue(temp_value, binop->dst->temp->num, result);
                        break;
                    }
                    case QuadKind::MOVE_CALL: {
                        auto *move_call = static_cast<QuadMoveCall*>(stm);
                        changed |= updateRtValue(temp_value, move_call->dst->temp->num, RtValue(ValueType::MANY_VALUES));
                        break;
                    }
                    case QuadKind::MOVE_EXTCALL: {
                        auto *move_extcall = static_cast<QuadMoveExtCall*>(stm);
                        changed |= updateRtValue(temp_value, move_extcall->dst->temp->num, RtValue(ValueType::MANY_VALUES));
                        break;
                    }
                    case QuadKind::JUMP: {
                        auto *jump = static_cast<QuadJump*>(stm);
                        changed |= updateExecutable(block_executable, jump->label->num);
                        break;
                    }
                    case QuadKind::CJUMP: {
                        auto *cjump = static_cast<QuadCJump*>(stm);
                        RtValue left = evalTermChecked(this, cjump->left, changed);
                        RtValue right = evalTermChecked(this, cjump->right, changed);
                        if (left.getType() == ValueType::ONE_VALUE && right.getType() == ValueType::ONE_VALUE) {
                            if (evalRelop(cjump->relop, left.getIntValue(), right.getIntValue()))
                                changed |= updateExecutable(block_executable, cjump->t->num);
                            else changed |= updateExecutable(block_executable, cjump->f->num);
                        } else {
                            changed |= updateExecutable(block_executable, cjump->t->num);
                            changed |= updateExecutable(block_executable, cjump->f->num);
                        }
                        break;
                    }
                    case QuadKind::PHI: {
                        auto *phi = static_cast<QuadPhi*>(stm);
                        changed |= updateRtValue(temp_value, phi->temp_exp->temp->num, evalPhi(this, phi));
                        break;
                    }
                    case QuadKind::PTR_CALC: {
                        auto *ptr_calc = static_cast<QuadPtrCalc*>(stm);
                        int dst_num = ptr_calc->dst != nullptr && ptr_calc->dst->kind == QuadTermKind::TEMP
                            ? ptr_calc->dst->get_temp()->temp->num : -1;
                        if (dst_num >= 0)
                            changed |= updateRtValue(temp_value, dst_num, RtValue(ValueType::MANY_VALUES)); // address is not int (can be 64 bits), so can't fit into ONE_VALUE
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }
}

void Opt::modifyFunc() {
    // Lambda: check if edge (pred -> curr) is executable.
    // Sets *is_cjump_fold to true when pred IS executable but CJUMP folds away from curr. (fold means curr is eliminated from executable blocks because the condition is known at compile time, so the edge is not executable even though pred is executable)
    auto isEdgeExec = [&](int pred, int curr, bool *is_cjump_fold = nullptr) -> bool {
        if (is_cjump_fold) *is_cjump_fold = false;
        if (!isExecutable(block_executable, pred)) return false;
        auto it = label2block.find(pred);
        if (it == label2block.end()) return true;
        for (auto *s : *it->second->quadlist) {
            if (s->kind == QuadKind::JUMP) return static_cast<QuadJump*>(s)->label->num == curr;
            if (s->kind == QuadKind::CJUMP) {
                auto *cj = static_cast<QuadCJump*>(s);
                RtValue lv = evalTerm(this, cj->left), rv = evalTerm(this, cj->right);
                if (lv.getType() == ValueType::ONE_VALUE && rv.getType() == ValueType::ONE_VALUE) {
                    bool taken = evalRelop(cj->relop, lv.getIntValue(), rv.getIntValue());
                    int tgt = taken ? cj->t->num : cj->f->num;
                    bool exec = (tgt == curr);
                    if (!exec && is_cjump_fold) *is_cjump_fold = true;
                    return exec;
                }
                return true; // both branches executable when condition unknown
            }
        }
        return true;
    };

    // Phase 1: Collect temps used in reachable blocks (liveness).
    set<int> live_temps;
    for (auto *block : *func->quadblocklist) {
        if (!isExecutable(block_executable, block->entry_label->num)) continue;
        for (auto *stm : *block->quadlist)
            if (stm->use) for (auto *t : *stm->use) live_temps.insert(t->num);
    }

    // Phase 2: Pre-scan phi nodes to identify constant inputs needing fresh temps.
    // For a phi with MANY_VALUES result, if a reachable-edge input has a constant value,
    // we must create a fresh MOVE in the predecessor block to replace the removed assignment.
    map<int, vector<tuple<int, int, QuadType>>> fresh_moves; // pred_label -> [(fresh_num, const_val, type)]
    map<pair<int,int>, int> phi_subst; // (old_temp_num, pred_label) -> fresh_temp_num

    for (auto *block : *func->quadblocklist) {
        int curr = block->entry_label->num;
        if (!isExecutable(block_executable, curr)) continue;
        for (auto *stm : *block->quadlist) {
            if (stm->kind != QuadKind::PHI) continue;
            auto *phi = static_cast<QuadPhi*>(stm);
            int phi_dst = phi->temp_exp->temp->num;
            if (getRtValue(phi_dst).getType() != ValueType::MANY_VALUES) continue;
            if (!phi->args) continue;
            for (auto &arg : *phi->args) {
                int pred = arg.second->num;
                if (!isEdgeExec(pred, curr)) continue;
                int src_num = arg.first->num;
                if (getRtValue(src_num).getType() != ValueType::ONE_VALUE) continue;
                // This constant-valued phi input will have its definition removed;
                // allocate a fresh temp and insert a MOVE in the predecessor block.
                int fresh = ++func->last_temp_num;
                int const_val = getRtValue(src_num).getIntValue();
                fresh_moves[pred].emplace_back(fresh, const_val, phi->temp_exp->type);
                phi_subst[{src_num, pred}] = fresh;
            }
        }
    }

    // Phase 3: Build new blocks (skip unreachable, remove constant assignments, fix phis).
    auto *new_blocks = new vector<QuadBlock*>();
    for (auto *block : *func->quadblocklist) {
        int curr = block->entry_label->num;
        if (!isExecutable(block_executable, curr)) continue;

        auto *new_quadlist = new vector<QuadStm*>();
        auto *new_exits = new vector<Label*>();
        auto &fmoves = fresh_moves[curr]; // fresh MOVEs to insert before exit

        for (auto *stm : *block->quadlist) {
            if (stm == nullptr) continue;
            switch (stm->kind) {
                case QuadKind::MOVE: {
                    auto *move = static_cast<QuadMove*>(stm);
                    // Remove assignment if result is a known constant.
                    if (getRtValue(move->dst->temp->num).getType() == ValueType::ONE_VALUE) break;
                    auto *nm = static_cast<QuadMove*>(stm->clone());
                    nm->src = rewriteTerm(this, nm->src);
                    new_quadlist->push_back(nm);
                    break;
                }
                case QuadKind::LOAD: {
                    auto *load = static_cast<QuadLoad*>(stm->clone());
                    load->src = rewriteTerm(this, load->src);
                    new_quadlist->push_back(load);
                    break;
                }
                case QuadKind::STORE: {
                    auto *store = static_cast<QuadStore*>(stm->clone());
                    store->src = rewriteTerm(this, store->src);
                    store->dst = rewriteTerm(this, store->dst);
                    new_quadlist->push_back(store);
                    break;
                }
                case QuadKind::MOVE_BINOP: {
                    auto *binop = static_cast<QuadMoveBinop*>(stm);
                    // Remove if result is a known constant.
                    if (getRtValue(binop->dst->temp->num).getType() == ValueType::ONE_VALUE) break;
                    auto *nb = static_cast<QuadMoveBinop*>(stm->clone());
                    nb->left = rewriteTerm(this, nb->left);
                    nb->right = rewriteTerm(this, nb->right);
                    new_quadlist->push_back(nb);
                    break;
                }
                case QuadKind::CALL: {
                    auto *call = static_cast<QuadCall*>(stm->clone());
                    rewriteCallLike(this, call);
                    new_quadlist->push_back(call);
                    break;
                }
                case QuadKind::MOVE_CALL: {
                    auto *mc = static_cast<QuadMoveCall*>(stm->clone());
                    rewriteCallLike(this, mc->call);
                    new_quadlist->push_back(mc);
                    break;
                }
                case QuadKind::EXTCALL: {
                    auto *ec = static_cast<QuadExtCall*>(stm->clone());
                    rewriteExtCallLike(this, ec);
                    new_quadlist->push_back(ec);
                    break;
                }
                case QuadKind::MOVE_EXTCALL: {
                    auto *mec = static_cast<QuadMoveExtCall*>(stm->clone());
                    rewriteExtCallLike(this, mec->extcall);
                    new_quadlist->push_back(mec);
                    break;
                }
                case QuadKind::LABEL:
                    new_quadlist->push_back(static_cast<QuadStm*>(stm->clone()));
                    break;
                case QuadKind::JUMP: {
                    auto *jump = static_cast<QuadJump*>(stm->clone());
                    // Insert fresh constant MOVEs for phi inputs in successor block, then jump.
                    for (auto &[fn, fv, ft] : fmoves)
                        new_quadlist->push_back(new QuadMove(
                            new QuadTemp(new Temp(fn), ft), new QuadTerm(fv), nullptr, nullptr));
                    new_exits->push_back(new Label(jump->label->num));
                    new_quadlist->push_back(jump);
                    break;
                }
                case QuadKind::CJUMP: {
                    auto *cjump = static_cast<QuadCJump*>(stm->clone());
                    cjump->left = rewriteTerm(this, cjump->left);
                    cjump->right = rewriteTerm(this, cjump->right);
                    bool t_exec = isEdgeExec(curr, cjump->t->num);
                    bool f_exec = isEdgeExec(curr, cjump->f->num);
                    // Insert fresh MOVEs before exit.
                    for (auto &[fn, fv, ft] : fmoves)
                        new_quadlist->push_back(new QuadMove(
                            new QuadTemp(new Temp(fn), ft), new QuadTerm(fv), nullptr, nullptr));
                    if (t_exec && !f_exec) {
                        new_exits->push_back(new Label(cjump->t->num));
                        new_quadlist->push_back(new QuadJump(new Label(cjump->t->num), nullptr, nullptr));
                    } else if (!t_exec && f_exec) {
                        new_exits->push_back(new Label(cjump->f->num));
                        new_quadlist->push_back(new QuadJump(new Label(cjump->f->num), nullptr, nullptr));
                    } else { // both branches executable or both not executable (shouldn't be both not executable since current block is executable, but handle conservatively just in case), keep CJUMP but update exits
                        if (t_exec) new_exits->push_back(new Label(cjump->t->num));
                        if (f_exec) new_exits->push_back(new Label(cjump->f->num));
                        new_quadlist->push_back(cjump);
                    }
                    break;
                }
                case QuadKind::PHI: {
                    auto *phi = static_cast<QuadPhi*>(stm);
                    int phi_dst = phi->temp_exp->temp->num;
                    RtValue phi_val = getRtValue(phi_dst);
                    // Remove phi if result is a known constant (substituted at use sites).
                    if (phi_val.getType() == ValueType::ONE_VALUE) break;

                    // Filter inputs by edge executability; track if any removal was a CJUMP fold.
                    auto *new_args = new vector<pair<Temp*, Label*>>();
                    bool had_cjump_fold = false;
                    if (phi->args) {
                        for (auto &arg : *phi->args) {
                            int pred = arg.second->num;
                            bool cjump_fold = false;
                            if (!isEdgeExec(pred, curr, &cjump_fold)) {
                                if (cjump_fold) had_cjump_fold = true;
                                continue;
                            }
                            // Apply fresh-temp substitution for constant inputs.
                            auto key = make_pair(arg.first->num, pred);
                            int src = phi_subst.count(key) ? phi_subst.at(key) : arg.first->num;
                            new_args->push_back({new Temp(src), new Label(pred)});
                        }
                    }

                    if (new_args->empty()) break; // no valid inputs; skip phi

                    if (new_args->size() == 1) {
                        int input_num = new_args->at(0).first->num;
                        bool is_live = live_temps.count(phi_dst) > 0;
                        RtValue input_val = getRtValue(input_num);
                        // Convert to MOVE only in special cases:
                        // (a) input is NO_VALUE and result is live (undefined var propagated), OR
                        // (b) the single-input situation arose from a CJUMP fold and result is live.
                        // Otherwise keep as a single-input phi (expected output format).
                        bool to_move = is_live &&
                            (had_cjump_fold || input_val.getType() == ValueType::NO_VALUE);
                        if (to_move) {
                            QuadTerm *src = (input_val.getType() == ValueType::ONE_VALUE)
                                ? new QuadTerm(input_val.getIntValue())
                                : new QuadTerm(new QuadTemp(new Temp(input_num), phi->temp_exp->type));
                            new_quadlist->push_back(new QuadMove(phi->temp_exp->clone(), src, nullptr, nullptr));
                        } else {
                            // Keep as single-input phi (dead result, or MANY_VALUES input without fold).
                            new_quadlist->push_back(new QuadPhi(phi->temp_exp->clone(), new_args, nullptr, nullptr));
                        }
                    } else {
                        // Multiple inputs: keep phi with updated args.
                        auto *np = static_cast<QuadPhi*>(stm->clone());
                        np->args = new_args;
                        new_quadlist->push_back(np);
                    }
                    break;
                }
                case QuadKind::RETURN: {
                    auto *ret = static_cast<QuadReturn*>(stm->clone());
                    ret->exp = rewriteTerm(this, ret->exp);
                    new_quadlist->push_back(ret);
                    break;
                }
                case QuadKind::PTR_CALC: {
                    auto *pc = static_cast<QuadPtrCalc*>(stm->clone());
                    pc->ptr = rewriteTerm(this, pc->ptr);
                    pc->offset = rewriteTerm(this, pc->offset);
                    new_quadlist->push_back(pc);
                    break;
                }
                default:
                    new_quadlist->push_back(static_cast<QuadStm*>(stm->clone()));
                    break;
            }
        }

        new_blocks->push_back(new QuadBlock(new_quadlist, block->entry_label, new_exits));
    }
    func->quadblocklist = new_blocks;
    // Reserve 2 extra temp numbers as fixed overhead per function.
    func->last_temp_num += 2;
}

QuadFuncDecl* Opt::optFunc() {
    func = func->clone();
    label2block.clear();
    block_executable.clear();
    temp_value.clear();

    // Initialize temp_value for parameters (assume unknown value coming in from caller).
    if (func->params != nullptr) {
        for (auto *param : *func->params)
            if (param != nullptr) temp_value[param->num] = RtValue(ValueType::MANY_VALUES);
    }

    // Initialize block_executable: only the entry block is executable at the start; others will be discovered by BT. Also build label2block for quick block lookup by label.
    if (func->quadblocklist != nullptr) {
        for (auto *block : *func->quadblocklist) {
            if (block == nullptr || block->entry_label == nullptr) continue;
            label2block[block->entry_label->num] = block;
            block_executable[block->entry_label->num] = false;
        }
        if (!func->quadblocklist->empty()) {
            auto *entry_block = func->quadblocklist->front();
            if (entry_block != nullptr && entry_block->entry_label != nullptr)
                block_executable[entry_block->entry_label->num] = true;
        }
    }

    calculateBT();
    modifyFunc();
    return func;
}

QuadProgram* optProg(QuadProgram* prog) {
    QuadProgram* newProg = new QuadProgram(new vector<QuadFuncDecl*>(), prog->last_label_num, prog->last_temp_num);
    for (int i=0; i < prog->quadFuncDeclList->size(); i++) {
        Opt optthis(prog->quadFuncDeclList->at(i));
        newProg->quadFuncDeclList->push_back(optthis.optFunc());
    }
    return newProg;
}
