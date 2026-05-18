#include "loopinductionopt.hh"
#include "defusechain.hh"
#include <queue>
#include <algorithm>

using namespace std;
using namespace quad;

namespace {

bool isPureDef(QuadStm* stm) {
    if (stm == nullptr) return false;
    return stm->kind == QuadKind::MOVE ||
           stm->kind == QuadKind::MOVE_BINOP ||
           stm->kind == QuadKind::PTR_CALC ||
           stm->kind == QuadKind::PHI;
}

bool hasSideEffect(QuadStm* stm) {
    if (stm == nullptr) return true;
    return stm->kind == QuadKind::STORE ||
           stm->kind == QuadKind::CALL ||
           stm->kind == QuadKind::MOVE_CALL ||
           stm->kind == QuadKind::EXTCALL ||
           stm->kind == QuadKind::MOVE_EXTCALL ||
           stm->kind == QuadKind::JUMP ||
           stm->kind == QuadKind::CJUMP ||
           stm->kind == QuadKind::RETURN ||
           stm->kind == QuadKind::LABEL;
}

}

QuadFuncDecl* eliminateUnusedInductionVars(QuadFuncDecl* func) {
    if (func == nullptr || func->quadblocklist == nullptr) {
        return func;
    }
    bool changed = true;
    while (changed) {
        changed = false;
        DefUseChain du(func);
        set<QuadStm*> useful;
        queue<QuadStm*> worklist;

        for (auto block : *func->quadblocklist) {
            if (block == nullptr || block->quadlist == nullptr) continue;
            for (auto stm : *block->quadlist) {
                if (!hasSideEffect(stm)) continue;
                useful.insert(stm);
                worklist.push(stm);
            }
        }

        while (!worklist.empty()) {
            QuadStm* stm = worklist.front();
            worklist.pop();
            for (int usedTemp : du.getUsesBy(stm)) {
                auto def = du.getDef(usedTemp);
                if (def == nullptr || def->defStm == nullptr || useful.count(def->defStm)) continue;
                useful.insert(def->defStm);
                worklist.push(def->defStm);
            }
        }

        for (auto block : *func->quadblocklist) {
            if (block == nullptr || block->quadlist == nullptr) continue;
            auto oldSize = block->quadlist->size();
            block->quadlist->erase(remove_if(block->quadlist->begin(), block->quadlist->end(), [&] (QuadStm* stm) {
                return isPureDef(stm) && !useful.count(stm);
            }), block->quadlist->end());
            if (block->quadlist->size() != oldSize) changed = true;
        }
    }

    return func;
}
