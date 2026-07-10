#define DEBUG
#undef DEBUG

#include <string>
#include <stack>
#include <variant>
#include <vector>
#include <set>
#include <algorithm>
#include <map>
#include "quad.hh"
#include "flowinfo.hh"
#include "looplicm.hh"

using namespace std;
using namespace quad;

namespace {

int blockLabel(QuadBlock *block) {
    if (block == nullptr || block->entry_label == nullptr) return -1;
    return block->entry_label->num;
}

bool isPureHoistCandidate(QuadStm *stm) {
    if (stm == nullptr) return false;
    if (stm->kind != QuadKind::MOVE_BINOP) return false;

    auto *binop = dynamic_cast<QuadMoveBinop*>(stm);
    if (binop == nullptr) return false;

    // On the current ARM backend, hoisting cheap arithmetic or address
    // calculations often lengthens live ranges enough to create spill traffic.
    // Keep LICM conservative and only hoist expensive invariant integer divides.
    return binop->binop == "/";
}

set<int> tempNums(set<Temp*> *temps) {
    set<int> nums;
    if (temps == nullptr) return nums;
    for (auto temp : *temps) {
        if (temp != nullptr) nums.insert(temp->num);
    }
    return nums;
}

bool operandsAreInvariant(QuadStm *stm, const set<int> &loopDefs, const set<int> &invariantDefs) { // return true if all operands of the statement are invariant or defined outside the loop
    for (int use : tempNums(stm->use)) {
        if (loopDefs.count(use) && !invariantDefs.count(use)) return false;
    }
    return true;
}

QuadBlock *findBlock(QuadFuncDecl *func, int label) {
    for (auto block : *func->quadblocklist) {
        if (blockLabel(block) == label) return block;
    }
    return nullptr;
}

QuadBlock *findPreheader(QuadFuncDecl *func, const set<int> &bodyBlocks, int headerLabel) { // return the block that has an exit edge to the header and is not in the loop body
    QuadBlock *header = findBlock(func, headerLabel);
    if (header == nullptr) return nullptr;

    for (auto block : *func->quadblocklist) {
        int label = blockLabel(block);
        if (bodyBlocks.count(label) || block == nullptr || block->exit_labels == nullptr) continue;
        for (auto exitLabel : *block->exit_labels) {
            if (exitLabel != nullptr && exitLabel->num == headerLabel) return block;
        }
    }
    return nullptr;
}

vector<LoopHeader*> sortedLoops(LoopHeaderMap *loopHeaderMap, QuadFuncDecl *func) { // return loops sorted by size (smallest first) and then by header label (smallest first)
    vector<LoopHeader*> loops;
    if (!loopHeaderMap->funcLoopHeaders.count(func)) return loops;
    for (auto loop : loopHeaderMap->funcLoopHeaders[func]) loops.push_back(loop);
    sort(loops.begin(), loops.end(), [] (LoopHeader *a, LoopHeader *b) {
        if (a->bodyBlocks.size() != b->bodyBlocks.size()) return a->bodyBlocks.size() < b->bodyBlocks.size();
        return a->headerLabel < b->headerLabel;
    });
    return loops;
}

set<int> collectLoopDefs(QuadFuncDecl *func, const set<int> &bodyBlocks) {
    set<int> defs;
    for (auto block : *func->quadblocklist) {
        if (!bodyBlocks.count(blockLabel(block)) || block->quadlist == nullptr) continue;
        for (auto stm : *block->quadlist) {
            set<int> stmDefs = tempNums(stm->def);
            defs.insert(stmDefs.begin(), stmDefs.end());
        }
    }
    return defs;
}

bool hoistOneLoop(QuadFuncDecl *func, LoopHeader *loop) {
    QuadBlock *preheader = findPreheader(func, loop->bodyBlocks, loop->headerLabel);
    if (preheader == nullptr || preheader->quadlist == nullptr) return false; // can't hoist if no preheader or preheader has no quadlist

    set<int> loopDefs = collectLoopDefs(func, loop->bodyBlocks);
    set<int> invariantDefs;
    vector<pair<QuadBlock*, QuadStm*>> toHoist;
    bool changed = true;

    // calculate invariant statements and hoist them iteratively until no more can be hoisted
    while (changed) {
        changed = false;
        for (auto block : *func->quadblocklist) {
            if (!loop->bodyBlocks.count(blockLabel(block)) || block->quadlist == nullptr) continue;
            for (auto stm : *block->quadlist) {
                if (!isPureHoistCandidate(stm) || !operandsAreInvariant(stm, loopDefs, invariantDefs)) continue;

                set<int> defs = tempNums(stm->def);
                bool alreadyHoisted = false;
                for (int def : defs) {
                    if (invariantDefs.count(def)) alreadyHoisted = true;
                }
                if (alreadyHoisted) continue;

                toHoist.push_back({block, stm});
                invariantDefs.insert(defs.begin(), defs.end());
                changed = true;
            }
        }
    }

    if (toHoist.empty()) return false;

    auto insertPos = preheader->quadlist->end();
    if (insertPos != preheader->quadlist->begin()) { // if preheader has statements, insert before the last jump/cjump/return if it exists
        auto last = insertPos;
        --last;
        if ((*last)->kind == QuadKind::JUMP || (*last)->kind == QuadKind::CJUMP || (*last)->kind == QuadKind::RETURN) {
            insertPos = last;
        }
    }

    for (auto item : toHoist) {
        auto &quadlist = *item.first->quadlist;
        auto found = find(quadlist.begin(), quadlist.end(), item.second);
        if (found == quadlist.end()) continue;
        quadlist.erase(found);
        insertPos = preheader->quadlist->insert(insertPos, item.second);
        ++insertPos;
    }

    return true;
}

}

// Main entry point for loop optimization
// Complete the function!!

QuadFuncDecl* loopHoistFunc(QuadFuncDecl* func, LoopHeaderMap *loopHeaderMap) {
    if (func == nullptr || func->quadblocklist == nullptr || loopHeaderMap == nullptr) {
        return func;
    }

    vector<LoopHeader*> loops = sortedLoops(loopHeaderMap, func);
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto loop : loops) {
            if (hoistOneLoop(func, loop)) changed = true;
        }
    }

    return func;
}
