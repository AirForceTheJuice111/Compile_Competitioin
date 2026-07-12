#include "copyprop.hh"

#include <map>
#include <set>
#include <vector>

#include "quad.hh"
#include "temp.hh"

using namespace std;

namespace {

// ---------------------------------------------------------------------------
// Resolve a copy chain.  Follows copyMap transitively to find the ultimate
// source.  Returns the original temp number if no chain exists.
// ---------------------------------------------------------------------------
int resolveChain(const map<int, int> &copyMap, int tempNum) {
    int current = tempNum;
    set<int> visited;
    while (true) {
        auto it = copyMap.find(current);
        if (it == copyMap.end()) break;
        int next = it->second;
        if (visited.count(next)) break; // cycle guard (shouldn't happen)
        visited.insert(current);
        current = next;
    }
    return current;
}

// ---------------------------------------------------------------------------
// Copy propagation on a single function.
// ---------------------------------------------------------------------------
void copyPropFunction(quad::QuadFuncDecl *func, int &eliminated) {
    if (!func || !func->quadblocklist) return;

    // --- Phase 1: Collect all trivial copies -------------------------------
    // dstTemp -> srcTemp  (both must be temps)
    map<int, int> copyMap;
    // Tracks whether a temp has a non-copy definition too (broken SSA guard)
    map<int, bool> hasNonCopyDef;

    for (auto *block : *func->quadblocklist) {
        if (!block || !block->quadlist) continue;
        for (auto *stm : *block->quadlist) {
            if (!stm) continue;
            if (stm->kind != quad::QuadKind::MOVE) {
                // Record non-copy definitions
                int defTemp = -1;
                if (stm->kind == quad::QuadKind::MOVE_BINOP)
                    defTemp = static_cast<quad::QuadMoveBinop *>(stm)
                                  ->dst->temp->num;
                else if (stm->kind == quad::QuadKind::LOAD)
                    defTemp =
                        static_cast<quad::QuadLoad *>(stm)->dst->temp->num;
                else if (stm->kind == quad::QuadKind::MOVE_CALL)
                    defTemp = static_cast<quad::QuadMoveCall *>(stm)
                                  ->dst->temp->num;
                else if (stm->kind == quad::QuadKind::MOVE_EXTCALL)
                    defTemp = static_cast<quad::QuadMoveExtCall *>(stm)
                                  ->dst->temp->num;
                else if (stm->kind == quad::QuadKind::PHI)
                    defTemp = static_cast<quad::QuadPhi *>(stm)
                                  ->temp_exp->temp->num;
                else if (stm->kind == quad::QuadKind::PTR_CALC) {
                    auto *pc = static_cast<quad::QuadPtrCalc *>(stm);
                    if (pc->dst &&
                        pc->dst->kind == quad::QuadTermKind::TEMP)
                        defTemp = pc->dst->get_temp()->temp->num;
                }
                if (defTemp >= 0) hasNonCopyDef[defTemp] = true;
                continue;
            }

            auto *move = static_cast<quad::QuadMove *>(stm);
            if (!move->src || move->src->kind != quad::QuadTermKind::TEMP)
                continue;
            int dstNum = move->dst->temp->num;
            int srcNum = move->src->get_temp()->temp->num;

            // Don't propagate self-copies (shouldn't exist, but be safe)
            if (dstNum == srcNum) {
                eliminated++;
                continue; // will be removed as dead
            }

            copyMap[dstNum] = srcNum;
        }
    }

    if (copyMap.empty()) return;

    // --- Phase 2: Resolve chains and build replacement map ------------------
    // Also filter out copies where dst has a non-copy definition (broken SSA)
    map<int, int> replacements;
    for (auto &[dst, src] : copyMap) {
        if (hasNonCopyDef.count(dst)) continue; // broken SSA guard
        int canonical = resolveChain(copyMap, dst);
        replacements[dst] = canonical;
    }

    if (replacements.empty()) return;

    // --- Phase 3: Rewrite all uses -----------------------------------------
    auto rewriteTerm = [&](quad::QuadTerm *&term) {
        if (!term || term->kind != quad::QuadTermKind::TEMP) return;
        int n = term->get_temp()->temp->num;
        auto it = replacements.find(n);
        if (it != replacements.end()) {
            term = new quad::QuadTerm(new quad::QuadTemp(
                new Temp(it->second), term->get_temp()->type));
        }
    };

    for (auto *block : *func->quadblocklist) {
        if (!block || !block->quadlist) continue;
        for (auto *stm : *block->quadlist) {
            if (!stm) continue;
            switch (stm->kind) {
            case quad::QuadKind::MOVE: {
                auto *m = static_cast<quad::QuadMove *>(stm);
                rewriteTerm(m->src);
                break;
            }
            case quad::QuadKind::MOVE_BINOP: {
                auto *b = static_cast<quad::QuadMoveBinop *>(stm);
                rewriteTerm(b->left);
                rewriteTerm(b->right);
                break;
            }
            case quad::QuadKind::LOAD: {
                auto *l = static_cast<quad::QuadLoad *>(stm);
                rewriteTerm(l->src);
                break;
            }
            case quad::QuadKind::STORE: {
                auto *s = static_cast<quad::QuadStore *>(stm);
                rewriteTerm(s->src);
                rewriteTerm(s->dst);
                break;
            }
            case quad::QuadKind::CALL: {
                auto *c = static_cast<quad::QuadCall *>(stm);
                rewriteTerm(c->obj_term);
                if (c->args)
                    for (auto *&a : *c->args) rewriteTerm(a);
                break;
            }
            case quad::QuadKind::EXTCALL: {
                auto *e = static_cast<quad::QuadExtCall *>(stm);
                if (e->args)
                    for (auto *&a : *e->args) rewriteTerm(a);
                break;
            }
            case quad::QuadKind::MOVE_CALL: {
                auto *mc = static_cast<quad::QuadMoveCall *>(stm);
                if (mc->call) {
                    rewriteTerm(mc->call->obj_term);
                    if (mc->call->args)
                        for (auto *&a : *mc->call->args) rewriteTerm(a);
                }
                break;
            }
            case quad::QuadKind::MOVE_EXTCALL: {
                auto *me = static_cast<quad::QuadMoveExtCall *>(stm);
                if (me->extcall && me->extcall->args)
                    for (auto *&a : *me->extcall->args) rewriteTerm(a);
                break;
            }
            case quad::QuadKind::CJUMP: {
                auto *cj = static_cast<quad::QuadCJump *>(stm);
                rewriteTerm(cj->left);
                rewriteTerm(cj->right);
                break;
            }
            case quad::QuadKind::RETURN: {
                auto *r = static_cast<quad::QuadReturn *>(stm);
                rewriteTerm(r->exp);
                break;
            }
            case quad::QuadKind::PHI: {
                auto *phi = static_cast<quad::QuadPhi *>(stm);
                if (phi->args)
                    for (auto &arg : *phi->args) {
                        auto it = replacements.find(arg.first->num);
                        if (it != replacements.end())
                            arg.first = new Temp(it->second);
                    }
                break;
            }
            case quad::QuadKind::PTR_CALC: {
                auto *pc = static_cast<quad::QuadPtrCalc *>(stm);
                rewriteTerm(pc->ptr);
                rewriteTerm(pc->offset);
                break;
            }
            default:
                break;
            }
        }
    }

    // --- Phase 4: Remove dead copy instructions (DCE) ----------------------
    // Compute liveness by scanning actual QuadTerms (stm->use may be stale
    // after prior optimizations like SCCP).
    set<int> liveTemps;
    for (auto *block : *func->quadblocklist) {
        if (!block || !block->quadlist) continue;
        for (auto *stm : *block->quadlist) {
            if (!stm) continue;
            // Collect temps from actual operands
            switch (stm->kind) {
            case quad::QuadKind::MOVE: {
                auto *m = static_cast<quad::QuadMove *>(stm);
                if (m->src && m->src->kind == quad::QuadTermKind::TEMP)
                    liveTemps.insert(m->src->get_temp()->temp->num);
                break;
            }
            case quad::QuadKind::MOVE_BINOP: {
                auto *b = static_cast<quad::QuadMoveBinop *>(stm);
                if (b->left && b->left->kind == quad::QuadTermKind::TEMP)
                    liveTemps.insert(b->left->get_temp()->temp->num);
                if (b->right && b->right->kind == quad::QuadTermKind::TEMP)
                    liveTemps.insert(b->right->get_temp()->temp->num);
                break;
            }
            case quad::QuadKind::LOAD: {
                auto *l = static_cast<quad::QuadLoad *>(stm);
                if (l->src && l->src->kind == quad::QuadTermKind::TEMP)
                    liveTemps.insert(l->src->get_temp()->temp->num);
                break;
            }
            case quad::QuadKind::STORE: {
                auto *s = static_cast<quad::QuadStore *>(stm);
                if (s->src && s->src->kind == quad::QuadTermKind::TEMP)
                    liveTemps.insert(s->src->get_temp()->temp->num);
                if (s->dst && s->dst->kind == quad::QuadTermKind::TEMP)
                    liveTemps.insert(s->dst->get_temp()->temp->num);
                break;
            }
            case quad::QuadKind::CALL: {
                auto *c = static_cast<quad::QuadCall *>(stm);
                if (c->obj_term && c->obj_term->kind == quad::QuadTermKind::TEMP)
                    liveTemps.insert(c->obj_term->get_temp()->temp->num);
                if (c->args) for (auto *a : *c->args)
                    if (a && a->kind == quad::QuadTermKind::TEMP)
                        liveTemps.insert(a->get_temp()->temp->num);
                break;
            }
            case quad::QuadKind::EXTCALL: {
                auto *e = static_cast<quad::QuadExtCall *>(stm);
                if (e->args) for (auto *a : *e->args)
                    if (a && a->kind == quad::QuadTermKind::TEMP)
                        liveTemps.insert(a->get_temp()->temp->num);
                break;
            }
            case quad::QuadKind::MOVE_CALL: {
                auto *mc = static_cast<quad::QuadMoveCall *>(stm);
                if (mc->call) {
                    if (mc->call->obj_term && mc->call->obj_term->kind == quad::QuadTermKind::TEMP)
                        liveTemps.insert(mc->call->obj_term->get_temp()->temp->num);
                    if (mc->call->args) for (auto *a : *mc->call->args)
                        if (a && a->kind == quad::QuadTermKind::TEMP)
                            liveTemps.insert(a->get_temp()->temp->num);
                }
                break;
            }
            case quad::QuadKind::MOVE_EXTCALL: {
                auto *me = static_cast<quad::QuadMoveExtCall *>(stm);
                if (me->extcall && me->extcall->args)
                    for (auto *a : *me->extcall->args)
                        if (a && a->kind == quad::QuadTermKind::TEMP)
                            liveTemps.insert(a->get_temp()->temp->num);
                break;
            }
            case quad::QuadKind::CJUMP: {
                auto *cj = static_cast<quad::QuadCJump *>(stm);
                if (cj->left && cj->left->kind == quad::QuadTermKind::TEMP)
                    liveTemps.insert(cj->left->get_temp()->temp->num);
                if (cj->right && cj->right->kind == quad::QuadTermKind::TEMP)
                    liveTemps.insert(cj->right->get_temp()->temp->num);
                break;
            }
            case quad::QuadKind::RETURN: {
                auto *r = static_cast<quad::QuadReturn *>(stm);
                if (r->exp && r->exp->kind == quad::QuadTermKind::TEMP)
                    liveTemps.insert(r->exp->get_temp()->temp->num);
                break;
            }
            case quad::QuadKind::PHI: {
                auto *phi = static_cast<quad::QuadPhi *>(stm);
                if (phi->args) for (auto &arg : *phi->args)
                    if (arg.first) liveTemps.insert(arg.first->num);
                break;
            }
            case quad::QuadKind::PTR_CALC: {
                auto *pc = static_cast<quad::QuadPtrCalc *>(stm);
                if (pc->ptr && pc->ptr->kind == quad::QuadTermKind::TEMP)
                    liveTemps.insert(pc->ptr->get_temp()->temp->num);
                if (pc->offset && pc->offset->kind == quad::QuadTermKind::TEMP)
                    liveTemps.insert(pc->offset->get_temp()->temp->num);
                break;
            }
            default:
                break;
            }
        }
    }

    // Some instructions have side effects and must not be removed even if
    // their result is unused.
    auto hasSideEffects = [](quad::QuadStm *stm) -> bool {
        if (!stm) return false;
        switch (stm->kind) {
        case quad::QuadKind::STORE:
        case quad::QuadKind::CALL:
        case quad::QuadKind::EXTCALL:
        case quad::QuadKind::MOVE_CALL:
        case quad::QuadKind::MOVE_EXTCALL:
        case quad::QuadKind::JUMP:
        case quad::QuadKind::CJUMP:
        case quad::QuadKind::RETURN:
        case quad::QuadKind::LABEL:
            return true;
        default:
            return false;
        }
    };

    for (auto *block : *func->quadblocklist) {
        if (!block || !block->quadlist) continue;
        auto *newList = new vector<quad::QuadStm *>();
        for (auto *stm : *block->quadlist) {
            if (!stm) continue;

            if (hasSideEffects(stm)) {
                newList->push_back(stm);
                continue;
            }

            // Check if this instruction defines a dead temp
            int defTemp = -1;
            switch (stm->kind) {
            case quad::QuadKind::MOVE:
                defTemp =
                    static_cast<quad::QuadMove *>(stm)->dst->temp->num;
                break;
            case quad::QuadKind::MOVE_BINOP:
                defTemp = static_cast<quad::QuadMoveBinop *>(stm)
                              ->dst->temp->num;
                break;
            case quad::QuadKind::LOAD:
                defTemp =
                    static_cast<quad::QuadLoad *>(stm)->dst->temp->num;
                break;
            case quad::QuadKind::PHI:
                defTemp = static_cast<quad::QuadPhi *>(stm)
                              ->temp_exp->temp->num;
                break;
            case quad::QuadKind::PTR_CALC: {
                auto *pc = static_cast<quad::QuadPtrCalc *>(stm);
                if (pc->dst &&
                    pc->dst->kind == quad::QuadTermKind::TEMP)
                    defTemp = pc->dst->get_temp()->temp->num;
                break;
            }
            default:
                break;
            }

            if (defTemp >= 0 && !liveTemps.count(defTemp)) {
                eliminated++;
                continue; // dead: remove
            }

            newList->push_back(stm);
        }
        block->quadlist = newList;
    }
}

} // namespace

// ---------------------------------------------------------------------------
// Public entry point
// ---------------------------------------------------------------------------
namespace quad {

QuadProgram *copyPropProg(QuadProgram *prog, int *eliminatedOut) {
    if (!prog) return prog;

    int totalEliminated = 0;
    auto *newFuncs = new vector<QuadFuncDecl *>();
    int lastLabel = prog->last_label_num;
    int lastTemp = prog->last_temp_num;

    for (auto *funcDecl : *prog->quadFuncDeclList) {
        if (!funcDecl) continue;
        int eliminated = 0;
        copyPropFunction(funcDecl, eliminated);
        totalEliminated += eliminated;
        newFuncs->push_back(funcDecl);
    }

    if (eliminatedOut) *eliminatedOut = totalEliminated;
    return new QuadProgram(newFuncs, lastLabel, lastTemp);
}

} // namespace quad
