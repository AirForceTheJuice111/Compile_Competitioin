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

constexpr std::size_t kMaxSpecializations = 2;
constexpr std::size_t kMaxSpecializedStatements = 128;

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

bool functionHasConditionalUse(quad::QuadFuncDecl* function,
                               const vector<int>& constantIndices) {
    if (function == nullptr || function->params == nullptr ||
        !function->quadblocklist) return false;
    set<int> constantParams;
    for (int index : constantIndices) {
        if (index >= 0 && index < static_cast<int>(function->params->size()) &&
            (*function->params)[index] != nullptr) {
            constantParams.insert((*function->params)[index]->num);
        }
    }
    if (constantParams.empty()) return false;
    for (auto* block : *function->quadblocklist) {
        if (!block || !block->quadlist) continue;
        for (auto* statement : *block->quadlist) {
            if (statement == nullptr || statement->kind != quad::QuadKind::CJUMP)
                continue;
            auto* jump = static_cast<quad::QuadCJump*>(statement);
            for (auto* term : {jump->left, jump->right}) {
                if (term && term->kind == quad::QuadTermKind::TEMP &&
                    term->get_temp() && term->get_temp()->temp &&
                    constantParams.count(term->get_temp()->temp->num)) {
                    return true;
                }
            }
        }
    }
    return false;
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
        auto* newParamTypes = clone->param_types != nullptr &&
                                      clone->param_types->size() ==
                                          clone->params->size()
                                  ? new vector<quad::QuadType>()
                                  : nullptr;
        set<int> constIdxSet(constIdx.begin(), constIdx.end());
        for (size_t i = 0; i < clone->params->size(); i++) {
            if (!constIdxSet.count((int)i)) {
                newParams->push_back((*clone->params)[i]);
                if (newParamTypes != nullptr)
                    newParamTypes->push_back((*clone->param_types)[i]);
            }
        }
        clone->params = newParams;
        clone->param_types = newParamTypes;
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
    set<string> specializedOriginals;
    std::size_t specializedStatements = 0;

    for (auto& cs : sites) {
        if (specialized >= static_cast<int>(kMaxSpecializations)) break;
        auto it = nameMap.find(cs.calleeName);
        if (it == nameMap.end()) continue;
        if (!functionHasConditionalUse(it->second, cs.constArgIdx)) continue;
        std::size_t bodySize = 0;
        if (it->second->quadblocklist) {
            for (auto* block : *it->second->quadblocklist)
                if (block && block->quadlist) bodySize += block->quadlist->size();
        }
        if (bodySize == 0 || specializedStatements + bodySize >
                                kMaxSpecializedStatements) continue;

        // Build a signature string for dedup
        ostringstream sig;
        sig << cs.calleeName;
        for (size_t i = 0; i < cs.constArgIdx.size(); i++)
            sig << "$" << cs.constArgIdx[i] << "$" << cs.constArgVal[i];

        if (generatedSpecs.count(sig.str())) continue;
        generatedSpecs.insert(sig.str());
        specializedStatements += bodySize;

        auto* spec = specializeFunc(it->second, cs.constArgIdx,
                                     cs.constArgVal, gt);
        // Add to program
        prog->quadFuncDeclList->push_back(spec);
        nameMap[spec->funcname] = spec;
        userFuncs.insert(spec->funcname);
        specializedOriginals.insert(cs.calleeName);
        specialized++;
    }

    // Replace call sites with specialized versions. The clone removed the
    // specialized parameters, so the corresponding constant arguments must
    // also be removed at every call site. Keep the remaining arguments in
    // their original order.
    for (auto& cs : sites) {
        ostringstream sig;
        sig << cs.calleeName;
        for (size_t i = 0; i < cs.constArgIdx.size(); i++)
            sig << "$" << cs.constArgIdx[i] << "$" << cs.constArgVal[i];

        string specName = sig.str();
        if (!userFuncs.count(specName)) continue;

        auto* stm = (*cs.block->quadlist)[cs.stmIdx];
        if (!stm) continue;

        auto removeConstantArguments = [&](vector<quad::QuadTerm*>* args) {
            if (args == nullptr) return;
            for (auto it = cs.constArgIdx.rbegin();
                 it != cs.constArgIdx.rend(); ++it) {
                if (*it >= 0 && *it < static_cast<int>(args->size()))
                    args->erase(args->begin() + *it);
            }
        };

        if (stm->kind == quad::QuadKind::MOVE_EXTCALL) {
            auto* me = static_cast<quad::QuadMoveExtCall*>(stm);
            if (me->extcall) {
                me->extcall->extfun = specName;
                removeConstantArguments(me->extcall->args);
            }
        } else if (stm->kind == quad::QuadKind::EXTCALL) {
            auto* e = static_cast<quad::QuadExtCall*>(stm);
            e->extfun = specName;
            removeConstantArguments(e->args);
        } else if (stm->kind == quad::QuadKind::MOVE_CALL) {
            auto* mc = static_cast<quad::QuadMoveCall*>(stm);
            if (mc->call) {
                mc->call->name = specName;
                removeConstantArguments(mc->call->args);
            }
        } else if (stm->kind == quad::QuadKind::CALL) {
            auto* c = static_cast<quad::QuadCall*>(stm);
            c->name = specName;
            removeConstantArguments(c->args);
        }
    }

    // If every direct call to an original was redirected to a clone, the
    // original is dead. Remove only proven-unreferenced non-entry functions;
    // recursive or mixed dynamic/constant call patterns retain the original.
    set<string> referencedFunctions;
    for (auto* function : *prog->quadFuncDeclList) {
        if (!function || !function->quadblocklist) continue;
        for (auto* block : *function->quadblocklist) {
            if (!block || !block->quadlist) continue;
            for (auto* statement : *block->quadlist) {
                string name;
                vector<quad::QuadTerm*>* unusedArgs = nullptr;
                quad::QuadTemp* unusedDestination = nullptr;
                if (statement && statement->kind == quad::QuadKind::MOVE_EXTCALL) {
                    auto* call = static_cast<quad::QuadMoveExtCall*>(statement);
                    if (call->extcall) name = call->extcall->extfun;
                } else if (statement && statement->kind == quad::QuadKind::MOVE_CALL) {
                    auto* call = static_cast<quad::QuadMoveCall*>(statement);
                    if (call->call) name = call->call->name;
                }
                if (!name.empty()) referencedFunctions.insert(name);
            }
        }
    }
    auto keptFunctions = new vector<quad::QuadFuncDecl*>();
    for (auto* function : *prog->quadFuncDeclList) {
        if (function && function->funcname != "main" &&
            specializedOriginals.count(function->funcname) &&
            !referencedFunctions.count(function->funcname)) {
            continue;
        }
        keptFunctions->push_back(function);
    }
    prog->quadFuncDeclList = keptFunctions;

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
