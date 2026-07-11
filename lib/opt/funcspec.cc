#include "funcspec.hh"

#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "quad.hh"
#include "temp.hh"

using namespace std;

namespace {

set<Temp*>* es() { return new set<Temp*>(); }
set<Temp*>* os(int n) { auto* s = new set<Temp*>(); s->insert(new Temp(n)); return s; }

// Check if a QuadTerm is a constant.
bool isConst(quad::QuadTerm* t) {
    return t && t->kind == quad::QuadTermKind::CONST;
}

// Collect call sites with constant arguments.
// Returns vector of {caller_func, block, stm_index, callee_name, {const_arg_indices}}
struct CallSite {
    quad::QuadFuncDecl* caller;
    quad::QuadBlock*    block;
    int                 stmIdx;      // index in block->quadlist
    string              calleeName;
    vector<int>         constArgIdx; // which args are constant
    vector<int>         constArgVal; // their values
};

vector<CallSite> findConstCallSites(quad::QuadProgram* prog,
                                     const set<string>& userFuncs) {
    vector<CallSite> sites;
    if (!prog || !prog->quadFuncDeclList) return sites;

    for (auto* caller : *prog->quadFuncDeclList) {
        if (!caller || !caller->quadblocklist) continue;
        for (auto* block : *caller->quadblocklist) {
            if (!block || !block->quadlist) continue;
            for (size_t i = 0; i < block->quadlist->size(); i++) {
                auto* stm = (*block->quadlist)[i];
                if (!stm) continue;

                string cn;
                vector<quad::QuadTerm*>* args = nullptr;
                if (stm->kind == quad::QuadKind::MOVE_EXTCALL) {
                    auto* me = static_cast<quad::QuadMoveExtCall*>(stm);
                    if (me->extcall) {
                        cn = me->extcall->extfun;
                        args = me->extcall->args;
                    }
                } else if (stm->kind == quad::QuadKind::EXTCALL) {
                    auto* e = static_cast<quad::QuadExtCall*>(stm);
                    cn = e->extfun;
                    args = e->args;
                } else if (stm->kind == quad::QuadKind::MOVE_CALL) {
                    auto* mc = static_cast<quad::QuadMoveCall*>(stm);
                    if (mc->call) {
                        cn = mc->call->name;
                        args = mc->call->args;
                    }
                } else if (stm->kind == quad::QuadKind::CALL) {
                    auto* c = static_cast<quad::QuadCall*>(stm);
                    cn = c->name;
                    args = c->args;
                }

                if (cn.empty() || !userFuncs.count(cn) || !args) continue;

                CallSite cs;
                cs.caller     = caller;
                cs.block      = block;
                cs.stmIdx     = (int)i;
                cs.calleeName = cn;
                for (size_t j = 0; j < args->size(); j++) {
                    if (isConst((*args)[j])) {
                        cs.constArgIdx.push_back((int)j);
                        cs.constArgVal.push_back((*args)[j]->get_const());
                    }
                }
                if (!cs.constArgIdx.empty()) sites.push_back(cs);
            }
        }
    }
    return sites;
}

// Create a specialized clone of a function with constant args replaced.
quad::QuadFuncDecl* specializeFunc(quad::QuadFuncDecl* original,
                                    const vector<int>& constIdx,
                                    const vector<int>& constVal,
                                    int& globalTemp) {
    auto* clone = original->clone();

    // Build new name
    ostringstream oss;
    oss << original->funcname << "$spec";
    for (size_t i = 0; i < constIdx.size(); i++)
        oss << "$" << constIdx[i] << "$" << constVal[i];
    clone->funcname = oss.str();

    // Replace constant params with their values in the body
    map<int, int> constParamVal; // paramTempNum -> constValue
    if (original->params) {
        for (size_t i = 0; i < constIdx.size(); i++) {
            int idx = constIdx[i];
            if (idx < (int)original->params->size())
                constParamVal[(*original->params)[idx]->num] = constVal[i];
        }
    }

    for (auto* block : *clone->quadblocklist) {
        if (!block || !block->quadlist) continue;
        auto* nl = new vector<quad::QuadStm*>();
        for (auto* stm : *block->quadlist) {
            if (!stm) continue;

            // Replace const param uses in this stm
            // For MOVE, replace src if it references a const param
            bool pushedAsMove = false;
            if (stm->kind == quad::QuadKind::MOVE) {
                auto* mv = static_cast<quad::QuadMove*>(stm);
                if (mv->src && mv->src->kind == quad::QuadTermKind::TEMP) {
                    int sn = mv->src->get_temp()->temp->num;
                    auto it = constParamVal.find(sn);
                    if (it != constParamVal.end()) {
                        // Replace MOVE dst <- param with MOVE dst <- Const
                        mv = static_cast<quad::QuadMove*>(stm->clone());
                        mv->src = new quad::QuadTerm(it->second);
                        mv->use = es();
                        mv->def = os(mv->dst->temp->num);
                        nl->push_back(mv);
                        pushedAsMove = true;
                    }
                }
            }
            if (pushedAsMove) continue;

            // For MOVE_BINOP, replace operand temps that are const params
            if (stm->kind == quad::QuadKind::MOVE_BINOP) {
                auto* bp = static_cast<quad::QuadMoveBinop*>(stm);
                bool changed = false;
                if (bp->left && bp->left->kind == quad::QuadTermKind::TEMP) {
                    int ln = bp->left->get_temp()->temp->num;
                    if (constParamVal.count(ln)) {
                        bp = static_cast<quad::QuadMoveBinop*>(stm->clone());
                        bp->left = new quad::QuadTerm(constParamVal[ln]);
                        changed = true;
                    }
                }
                if (bp->right && bp->right->kind == quad::QuadTermKind::TEMP) {
                    int rn = bp->right->get_temp()->temp->num;
                    if (constParamVal.count(rn)) {
                        if (!changed) bp = static_cast<quad::QuadMoveBinop*>(stm->clone());
                        bp->right = new quad::QuadTerm(constParamVal[rn]);
                        changed = true;
                    }
                }
                if (changed) {
                    bp->def = os(bp->dst->temp->num);
                    bp->use = es();
                    if (bp->left && bp->left->kind == quad::QuadTermKind::TEMP)
                        bp->use->insert(new Temp(bp->left->get_temp()->temp->num));
                    if (bp->right && bp->right->kind == quad::QuadTermKind::TEMP)
                        bp->use->insert(new Temp(bp->right->get_temp()->temp->num));
                }
                nl->push_back(bp);
                continue;
            }

            // For CJUMP, replace operands
            if (stm->kind == quad::QuadKind::CJUMP) {
                auto* cj = static_cast<quad::QuadCJump*>(stm);
                bool changed = false;
                if (cj->left && cj->left->kind == quad::QuadTermKind::TEMP) {
                    int ln = cj->left->get_temp()->temp->num;
                    if (constParamVal.count(ln)) {
                        if (!changed) cj = static_cast<quad::QuadCJump*>(stm->clone());
                        cj->left = new quad::QuadTerm(constParamVal[ln]);
                        changed = true;
                    }
                }
                if (cj->right && cj->right->kind == quad::QuadTermKind::TEMP) {
                    int rn = cj->right->get_temp()->temp->num;
                    if (constParamVal.count(rn)) {
                        if (!changed) cj = static_cast<quad::QuadCJump*>(stm->clone());
                        cj->right = new quad::QuadTerm(constParamVal[rn]);
                        changed = true;
                    }
                }
                if (changed) {
                    cj->use = es();
                    if (cj->left && cj->left->kind == quad::QuadTermKind::TEMP)
                        cj->use->insert(new Temp(cj->left->get_temp()->temp->num));
                    if (cj->right && cj->right->kind == quad::QuadTermKind::TEMP)
                        cj->use->insert(new Temp(cj->right->get_temp()->temp->num));
                }
                nl->push_back(cj);
                continue;
            }

            // For RETURN, replace exp
            if (stm->kind == quad::QuadKind::RETURN) {
                auto* ret = static_cast<quad::QuadReturn*>(stm);
                if (ret->exp && ret->exp->kind == quad::QuadTermKind::TEMP) {
                    int en = ret->exp->get_temp()->temp->num;
                    if (constParamVal.count(en)) {
                        ret = static_cast<quad::QuadReturn*>(stm->clone());
                        ret->exp = new quad::QuadTerm(constParamVal[en]);
                        ret->use = es();
                    }
                }
                nl->push_back(ret);
                continue;
            }

            // STORE
            if (stm->kind == quad::QuadKind::STORE) {
                auto* st = static_cast<quad::QuadStore*>(stm);
                bool changed = false;
                auto replace = [&](quad::QuadTerm*& t) {
                    if (t && t->kind == quad::QuadTermKind::TEMP) {
                        int n = t->get_temp()->temp->num;
                        if (constParamVal.count(n)) {
                            if (!changed) st = static_cast<quad::QuadStore*>(stm->clone());
                            t = new quad::QuadTerm(constParamVal[n]);
                            changed = true;
                        }
                    }
                };
                replace(st->src);
                replace(st->dst);
                nl->push_back(st);
                continue;
            }

            nl->push_back(stm);
        }
        block->quadlist = nl;
    }

    // Remove const params from the parameter list
    if (clone->params) {
        auto* newParams = new vector<Temp*>();
        set<int> constIdxSet(constIdx.begin(), constIdx.end());
        for (size_t i = 0; i < clone->params->size(); i++)
            if (!constIdxSet.count((int)i))
                newParams->push_back((*clone->params)[i]);
        clone->params = newParams;
    }

    clone->last_temp_num = max(clone->last_temp_num, globalTemp);
    return clone;
}

void funcSpecProgram(quad::QuadProgram* prog, int& specialized) {
    if (!prog || !prog->quadFuncDeclList) return;

    // Build function name set
    set<string> userFuncs;
    map<string, quad::QuadFuncDecl*> nameMap;
    for (auto* f : *prog->quadFuncDeclList) {
        if (!f) continue;
        userFuncs.insert(f->funcname);
        nameMap[f->funcname] = f;
    }

    auto sites = findConstCallSites(prog, userFuncs);
    if (sites.empty()) return;

    int gt = prog->last_temp_num;
    set<string> generatedSpecs; // avoid duplicate specializations

    for (auto& cs : sites) {
        auto it = nameMap.find(cs.calleeName);
        if (it == nameMap.end()) continue;

        // Build a signature string for dedup
        ostringstream sig;
        sig << cs.calleeName;
        for (size_t i = 0; i < cs.constArgIdx.size(); i++)
            sig << "$" << cs.constArgIdx[i] << "$" << cs.constArgVal[i];

        if (generatedSpecs.count(sig.str())) continue;
        generatedSpecs.insert(sig.str());

        auto* spec = specializeFunc(it->second, cs.constArgIdx,
                                     cs.constArgVal, gt);
        // Add to program
        prog->quadFuncDeclList->push_back(spec);
        nameMap[spec->funcname] = spec;
        userFuncs.insert(spec->funcname);
        specialized++;
    }

    // Replace call sites with specialized versions
    for (auto& cs : sites) {
        ostringstream sig;
        sig << cs.calleeName;
        for (size_t i = 0; i < cs.constArgIdx.size(); i++)
            sig << "$" << cs.constArgIdx[i] << "$" << cs.constArgVal[i];

        string specName = sig.str();
        if (!userFuncs.count(specName)) continue;

        auto* stm = (*cs.block->quadlist)[cs.stmIdx];
        if (!stm) continue;

        if (stm->kind == quad::QuadKind::MOVE_EXTCALL) {
            auto* me = static_cast<quad::QuadMoveExtCall*>(stm);
            if (me->extcall) me->extcall->extfun = specName;
        } else if (stm->kind == quad::QuadKind::EXTCALL) {
            auto* e = static_cast<quad::QuadExtCall*>(stm);
            e->extfun = specName;
        } else if (stm->kind == quad::QuadKind::MOVE_CALL) {
            auto* mc = static_cast<quad::QuadMoveCall*>(stm);
            if (mc->call) mc->call->name = specName;
        } else if (stm->kind == quad::QuadKind::CALL) {
            static_cast<quad::QuadCall*>(stm)->name = specName;
        }
    }

    prog->last_temp_num = gt;
}

} // namespace

namespace quad {
QuadProgram* funcSpecProg(QuadProgram* prog, int* eo) {
    if (!prog) return prog;
    int e = 0;
    funcSpecProgram(prog, e);
    if (eo) *eo = e;
    return prog;
}
} // namespace quad
