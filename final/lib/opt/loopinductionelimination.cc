#include "loopinductionopt.hh"
#include "defusechain.hh"
#include <queue>
#include <algorithm>

using namespace std;
using namespace quad;

namespace {

bool isPureDef(QuadStm* stm) {
    // These statements only define temps and have no observable effect by
    // themselves. If their definitions cannot reach a useful side-effecting
    // statement, they can be deleted safely.
    if (stm == nullptr) return false;
    return stm->kind == QuadKind::MOVE ||
           stm->kind == QuadKind::MOVE_BINOP ||
           stm->kind == QuadKind::PTR_CALC ||
           stm->kind == QuadKind::PHI;
}

bool hasSideEffect(QuadStm* stm) {
    // Treat labels and control-flow statements as roots. Calls and stores are
    // conservatively rooted because they may observe or change program state.
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

bool hasSelfDefUse(QuadStm* stm) {
    if (stm == nullptr || stm->def == nullptr || stm->use == nullptr) return false;
    for (auto *def : *stm->def) {
        if (def == nullptr) continue;
        for (auto *use : *stm->use) {
            if (use != nullptr && use->num == def->num) return true;
        }
    }
    return false;
}

}

QuadFuncDecl* eliminateUnusedInductionVars(QuadFuncDecl* func) {
    // 前面只删除了旧Derived-IV的定义，但它的旧Basic-IV定义和新PHI定义仍然可能变成死代码。这个函数通过标记所有有副作用的语句及其依赖的定义为有用，然后删除不在这些有用语句依赖链上的纯定义来清理这些死代码。
     if (func == nullptr || func->quadblocklist == nullptr) {
        return func;
    }
    if (func == nullptr || func->quadblocklist == nullptr) {
        return func;
    }
    bool changed = true;
    while (changed) { // actually one-pass is sufficient, but we keep the fixed-point loop anyways. (for future extensions)
        changed = false;
        DefUseChain du(func);
        set<QuadStm*> useful;
        queue<QuadStm*> worklist;

        // Mark observable statements as roots of usefulness.
        for (auto block : *func->quadblocklist) {
            if (block == nullptr || block->quadlist == nullptr) continue;
            for (auto stm : *block->quadlist) {
                if (!hasSideEffect(stm)) continue;
                useful.insert(stm);
                worklist.push(stm);
            }
        }

        // Walk backwards through SSA def-use edges. If a useful statement uses a
        // temp, the statement defining that temp is also useful.
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

        // Remove pure definitions that are not reachable from any useful root.
        // The outer fixed point matters for PHI/update self-cycles: removing one
        // dead layer may expose another layer as dead in the next iteration.
        for (auto block : *func->quadblocklist) {
            if (block == nullptr || block->quadlist == nullptr) continue;
            auto oldSize = block->quadlist->size();
            block->quadlist->erase(remove_if(block->quadlist->begin(), block->quadlist->end(), [&] (QuadStm* stm) {
                return isPureDef(stm) && !hasSelfDefUse(stm) && !useful.count(stm);
            }), block->quadlist->end());
            if (block->quadlist->size() != oldSize) changed = true;
        }
    }

    return func;
}
