#include "gvn.hh"

#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "quad.hh"
#include "flowinfo.hh"

using namespace std;

namespace {

// ---------------------------------------------------------------------------
// Value numbering helpers
// ---------------------------------------------------------------------------

// Follow the value-number chain to find the canonical temp.
int canonicalTemp(const map<int, int> &vn, int tempNum) {
    auto it = vn.find(tempNum);
    if (it == vn.end()) return tempNum;
    return it->second;
}

// Produce a string representation of a term suitable for hashing.
// Constants are prefixed with "C", temps with "T" (after canonicalisation),
// names with the actual name string.  Using the name string avoids
// incorrectly merging different global symbols (e.g. two distinct
// string constants).
string termHash(const map<int, int> &vn, quad::QuadTerm *term) {
    if (!term) return "N";
    if (term->kind == quad::QuadTermKind::CONST)
        return "C" + to_string(term->get_const());
    if (term->kind == quad::QuadTermKind::TEMP)
        return "T" + to_string(canonicalTemp(vn, term->get_temp()->temp->num));
    // NAME: include the actual name to distinguish different symbols
    return "N:" + term->get_name();
}

// Produce a string representation of a Temp (canonicalised through vn).
string tempHash(const map<int, int> &vn, Temp *temp) {
    if (!temp) return "N";
    return "T" + to_string(canonicalTemp(vn, temp->num));
}

// ---------------------------------------------------------------------------
// Core GVN on a single function (SSA form required)
// ---------------------------------------------------------------------------
void gvnFunction(quad::QuadFuncDecl *func, ControlFlowInfo *cfi,
                 int &freshTempNum, int &eliminated) {
    if (!func || !cfi) return;

    // Value table:  hash -> canonical temp number
    map<string, int> valueTable;
    // Scope stack: keys added in the current block scope (for rollback)
    vector<string> scopeKeys;
    // Value number map: temp -> canonical temp (redundant ones point elsewhere)
    map<int, int> vn;
    // Set of redundant temps that should be deleted after rewriting
    set<int> redundantTemps;
    // Replacement map: old temp -> new temp (for all uses)
    map<int, int> replacements;
    // Temps that have appeared as operands (uses).  If one of these is
    // later re-defined (SCCP may break SSA single-def property), we must
    // treat the redefinition as unique to avoid incorrect merging.
    set<int> usedTemps;

    // Helper: collect all operand temp numbers from a statement
    auto collectUses = [&](quad::QuadStm *stm) {
        if (!stm || !stm->use) return;
        for (auto *t : *stm->use)
            if (t) usedTemps.insert(t->num);
    };

    // --- dominator-tree preorder walk with scoping ------------------------
    function<void(int)> walkBlock = [&](int label) {
        auto blockIt = cfi->labelToBlock.find(label);
        if (blockIt == cfi->labelToBlock.end()) return;
        auto *block = blockIt->second;
        if (!block || !block->quadlist) return;

        size_t scopeStart = scopeKeys.size();

        for (auto *stm : *block->quadlist) {
            if (!stm) continue;

            string hash;
            int dstNum = -1;
            bool canDedup = false;

            switch (stm->kind) {
            case quad::QuadKind::MOVE: {
                auto *move = static_cast<quad::QuadMove *>(stm);
                dstNum = move->dst->temp->num;
                hash = "MV:" + termHash(vn, move->src);
                canDedup = true;
                break;
            }
            case quad::QuadKind::MOVE_BINOP: {
                auto *binop = static_cast<quad::QuadMoveBinop *>(stm);
                dstNum = binop->dst->temp->num;
                string lh = termHash(vn, binop->left);
                string rh = termHash(vn, binop->right);

                // Normalize commutative operations
                bool comm = (binop->binop == "+" || binop->binop == "*" ||
                             binop->binop == "==" || binop->binop == "!=" ||
                             binop->binop == "&" || binop->binop == "|" ||
                             binop->binop == "^");
                if (comm && lh > rh) swap(lh, rh);

                hash = "BIN:" + binop->binop + ":" + lh + ":" + rh;
                canDedup = true;
                break;
            }
            case quad::QuadKind::PHI: {
                auto *phi = static_cast<quad::QuadPhi *>(stm);
                dstNum = phi->temp_exp->temp->num;
                // Build a deterministic hash from (predLabel, argHash) pairs,
                // sorted by predLabel.
                vector<pair<int, string>> pairs;
                if (phi->args) {
                    for (auto &arg : *phi->args) {
                        int predLabel = arg.second ? arg.second->num : -1;
                        string argHash = tempHash(vn, arg.first);
                        pairs.emplace_back(predLabel, argHash);
                    }
                    sort(pairs.begin(), pairs.end());
                }
                ostringstream os;
                os << "PHI:";
                for (auto &p : pairs)
                    os << ":" << p.first << ":" << p.second;
                hash = os.str();
                canDedup = true;
                break;
            }
            case quad::QuadKind::PTR_CALC: {
                auto *pc = static_cast<quad::QuadPtrCalc *>(stm);
                if (pc->dst && pc->dst->kind == quad::QuadTermKind::TEMP) {
                    dstNum = pc->dst->get_temp()->temp->num;
                    hash = "PTRC:" + termHash(vn, pc->ptr) + ":" +
                           termHash(vn, pc->offset);
                    canDedup = true;
                }
                break;
            }
            case quad::QuadKind::LOAD: {
                auto *load = static_cast<quad::QuadLoad *>(stm);
                dstNum = load->dst->temp->num;
                // Without MemSSA we cannot prove two LOADs return the same
                // value even from the same address.  Give each a unique hash.
                hash = "LD:" + to_string(dstNum);
                canDedup = false;
                break;
            }
            case quad::QuadKind::MOVE_CALL: {
                auto *mc = static_cast<quad::QuadMoveCall *>(stm);
                dstNum = mc->dst->temp->num;
                // Calls may have side effects; never deduplicate.
                hash = "CALL:" + to_string(dstNum);
                canDedup = false;
                break;
            }
            case quad::QuadKind::MOVE_EXTCALL: {
                auto *mec = static_cast<quad::QuadMoveExtCall *>(stm);
                dstNum = mec->dst->temp->num;
                hash = "EXTC:" + to_string(dstNum);
                canDedup = false;
                break;
            }
            default:
                break;
            }

            if (dstNum < 0) continue;

            // Every defining instruction gets a value number.
            int myVn = dstNum; // default: map to self

            // If this temp was already used as an operand in a previous
            // instruction, we are seeing a redefinition (SCCP may break SSA
            // single-def property).  Treat redefinitions as unique so that
            // they are never incorrectly merged with earlier definitions.
            if (canDedup && usedTemps.count(dstNum)) {
                canDedup = false;
                hash = "R" + to_string(dstNum);
            }

            // Record operand temps BEFORE updating vn, so that the
            // redefinition guard above sees the correct usedTemps state.
            collectUses(stm);

            if (canDedup) {
                auto found = valueTable.find(hash);
                if (found != valueTable.end()) {
                    // Redundant: replace with canonical temp
                    myVn = found->second;
                    replacements[dstNum] = myVn;
                    redundantTemps.insert(dstNum);
                } else {
                    valueTable[hash] = dstNum;
                    scopeKeys.push_back(hash);
                }
            } else {
                // Non-deduplicable: always record as unique
                valueTable[hash] = dstNum;
                scopeKeys.push_back(hash);
            }

            vn[dstNum] = myVn;
        }

        // Walk children in dominator tree
        auto children = cfi->domTree.find(label);
        if (children != cfi->domTree.end()) {
            for (int childLabel : children->second) {
                walkBlock(childLabel);
            }
        }

        // Pop scope: remove entries added in this block
        while (scopeKeys.size() > scopeStart) {
            valueTable.erase(scopeKeys.back());
            scopeKeys.pop_back();
        }
    };

    if (cfi->entryBlock >= 0) {
        walkBlock(cfi->entryBlock);
    }

    // --- Phase 2: Apply replacements to all instructions ------------------
    if (replacements.empty()) return;

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

            // Rewrite uses
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
                if (phi->args) {
                    for (auto &arg : *phi->args) {
                        auto rit = replacements.find(arg.first->num);
                        if (rit != replacements.end()) {
                            arg.first = new Temp(rit->second);
                        }
                    }
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

    // --- Phase 3: Remove dead instructions (redundant definitions) --------
    // Also filter out PHI nodes that became trivial (all same input)
    for (auto *block : *func->quadblocklist) {
        if (!block || !block->quadlist) continue;
        auto *newList = new vector<quad::QuadStm *>();
        for (auto *stm : *block->quadlist) {
            if (!stm) continue;

            // Remove redundant defining instructions
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
            case quad::QuadKind::PHI:
                defTemp = static_cast<quad::QuadPhi *>(stm)
                              ->temp_exp->temp->num;
                break;
            case quad::QuadKind::LOAD:
                defTemp =
                    static_cast<quad::QuadLoad *>(stm)->dst->temp->num;
                break;
            case quad::QuadKind::MOVE_CALL:
                defTemp = static_cast<quad::QuadMoveCall *>(stm)
                              ->dst->temp->num;
                break;
            case quad::QuadKind::MOVE_EXTCALL:
                defTemp = static_cast<quad::QuadMoveExtCall *>(stm)
                              ->dst->temp->num;
                break;
            case quad::QuadKind::PTR_CALC: {
                auto *pc = static_cast<quad::QuadPtrCalc *>(stm);
                if (pc->dst && pc->dst->kind == quad::QuadTermKind::TEMP)
                    defTemp = pc->dst->get_temp()->temp->num;
                break;
            }
            default:
                break;
            }

            if (defTemp >= 0 && redundantTemps.count(defTemp)) {
                eliminated++;
                continue; // skip: redundant definition
            }

            // Simplify PHI: if all arguments are the same temp, replace with
            // a MOVE
            if (stm->kind == quad::QuadKind::PHI) {
                auto *phi = static_cast<quad::QuadPhi *>(stm);
                if (phi->args && !phi->args->empty()) {
                    int firstVn = -1;
                    bool allSame = true;
                    for (auto &arg : *phi->args) {
                        int v = canonicalTemp(vn, arg.first->num);
                        if (firstVn < 0)
                            firstVn = v;
                        else if (v != firstVn) {
                            allSame = false;
                            break;
                        }
                    }
                    if (allSame && firstVn >= 0) {
                        // Replace phi with a simple MOVE
                        auto *src = new quad::QuadTerm(new quad::QuadTemp(
                            new Temp(firstVn), phi->temp_exp->type));
                        auto *move = new quad::QuadMove(
                            phi->temp_exp->clone(), src,
                            new set<Temp *>(),
                            new set<Temp *>());
                        // Set def/use properly
                        move->def->insert(new Temp(phi->temp_exp->temp->num));
                        move->use->insert(new Temp(firstVn));
                        newList->push_back(move);
                        continue;
                    }
                }
            }

            newList->push_back(stm);
        }
        block->quadlist = newList;
    }

    freshTempNum = max(freshTempNum, func->last_temp_num);
}

} // namespace

// ---------------------------------------------------------------------------
// Public entry point
// ---------------------------------------------------------------------------
namespace quad {

QuadProgram *gvnProg(QuadProgram *prog,
                     set<FuncFlowInfo *> *flowInfo,
                     int *eliminatedOut) {
    if (!prog || !flowInfo) return prog;

    int totalEliminated = 0;

    auto *newFuncs =
        new vector<QuadFuncDecl *>();
    int lastLabel = prog->last_label_num;
    int lastTemp = prog->last_temp_num;

    for (auto *ffi : *flowInfo) {
        if (!ffi || !ffi->cfi || !ffi->cfi->func) continue;
        auto *func = ffi->cfi->func;

        int freshTempNum = func->last_temp_num;
        int eliminated = 0;
        gvnFunction(func, ffi->cfi, freshTempNum, eliminated);
        totalEliminated += eliminated;
        func->last_temp_num = freshTempNum;

        newFuncs->push_back(func);
        if (ffi->programLastLabelNum >= 0)
            lastLabel = ffi->programLastLabelNum;
        if (ffi->programLastTempNum >= 0)
            lastTemp = ffi->programLastTempNum;
    }

    if (eliminatedOut) *eliminatedOut = totalEliminated;
    return new QuadProgram(newFuncs, lastLabel, lastTemp);
}

} // namespace quad
