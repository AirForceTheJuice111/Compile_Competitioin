#define DEBUG
#undef DEBUG

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <algorithm>
#include "temp.hh"
#include "quad.hh"
#include "flowinfo.hh"

using namespace std;
using namespace quad;

void ControlFlowInfo::computeAllBlocks() {
    // This one is done for you!
#ifdef DEBUG
    cout << "Computing all blocks in function: " << func->funcname << endl;
    cout << "#blocks = " << func->quadblocklist->size() << endl;
#endif
    // Compute all blocks in the function
    if (func == nullptr || func->quadblocklist == nullptr) {
        return ; // Nothing to do
    }
    //Collect block information
    allBlocks = set<int>(); //empty set
    labelToBlock = map<int, QuadBlock*>(); //empty map

    for (auto block : *func->quadblocklist) {
        if (block->entry_label) {
            allBlocks.insert(block->entry_label->num);
            labelToBlock[block->entry_label->num] = block;
        }
    }
#ifdef DEBUG
    cout << "All blocks in function: " << func->funcname << endl;
    for (auto block : allBlocks) {
        cout << block << " " << labelToBlock[block]->entry_label->str() << endl;
    }
    cout << endl;
#endif
}

void ControlFlowInfo::computeUnreachableBlocks() {
#ifdef DEBUG
    cout << "Computing unreachable blocks in function: " << func->funcname << endl;
#endif
    // BFS from entry block to find all reachable blocks
    unreachableBlocks.clear();
    set<int> reachable;
    queue<int> worklist;
    worklist.push(entryBlock);
    reachable.insert(entryBlock);
    while (!worklist.empty()) {
        auto curr = worklist.front();
        worklist.pop();
        auto block = labelToBlock[curr];
        if (block && block->exit_labels) {
            for (auto label : *block->exit_labels) {
                if (allBlocks.count(label->num) && !reachable.count(label->num)) {
                    reachable.insert(label->num);
                    worklist.push(label->num);
                }
            }
        }
    }
    // Unreachable = allBlocks - reachable
    for (auto b : allBlocks) {
        if (!reachable.count(b)) unreachableBlocks.insert(b);
    }
#ifdef DEBUG
    cout << "Unreachable blocks: ";
    for (auto b : unreachableBlocks) cout << b << " ";
    cout << endl;
#endif
}

void ControlFlowInfo::eliminateUnreachableBlocks() {
#ifdef DEBUG
    cout << "Eliminating unreachable blocks in function: " << func->funcname << endl;
#endif
    // Remove unreachable blocks from the function's block list
    auto it = func->quadblocklist->begin();
    while (it != func->quadblocklist->end()) {
        if (unreachableBlocks.count((*it)->entry_label->num)) it = func->quadblocklist->erase(it);
        else ++it;
    }
    // Clear all computed data since it may need to be recomputed
    allBlocks.clear();
    unreachableBlocks.clear();
    predecessors.clear();
    successors.clear();
    dominators.clear();
    immediateDominator.clear();
    dominanceFrontiers.clear();
    domTree.clear();
    labelToBlock.clear();
    // Recompute allBlocks with the remaining blocks
    computeAllBlocks();
}

void ControlFlowInfo::computePredecessors() {
    // Compute predecessors: inverse of the successor relation
    predecessors.clear();
    for (auto b : allBlocks) predecessors[b] = set<int>();
    for (auto block : *func->quadblocklist) {
        auto id = block->entry_label->num;
        if (block->exit_labels) {
            for (auto label : *block->exit_labels) {
                if (allBlocks.count(label->num)) predecessors[label->num].insert(id);
            }
        }
    }
}

void ControlFlowInfo::computeSuccessors() {
    // Compute successors from exit_labels of each block
    successors.clear();
    for (auto b : allBlocks) successors[b] = set<int>();
    for (auto block : *func->quadblocklist) {
        auto id = block->entry_label->num;
        if (block->exit_labels) {
            for (auto label : *block->exit_labels) {
                if (allBlocks.count(label->num)) successors[id].insert(label->num);
            }
        }
    }
}

void ControlFlowInfo::computeDominators() {
#ifdef DEBUG
    std::cout << "Computing dominators for: " << func->funcname << endl;
#endif
    // Iterative dominator algorithm:
    //   dom(entry) = {entry}
    //   dom(n) = {n} ∪ (∩ dom(p) for all predecessors p of n)
    dominators.clear();
    for (auto b : allBlocks) {
        if (b == entryBlock) dominators[b] = {b};
        else dominators[b] = allBlocks;
    }
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto b : allBlocks) {
            if (b == entryBlock) continue;
            // Intersect dom sets of all predecessors
            auto new_dom = allBlocks;
            for (auto pred : predecessors[b]) {
                set<int> intersection;
                set_intersection(new_dom.begin(), new_dom.end(),
                                 dominators[pred].begin(), dominators[pred].end(),
                                 inserter(intersection, intersection.begin()));
                new_dom = intersection;
            }
            new_dom.insert(b); // n always dominates itself
            if (new_dom != dominators[b]) {
                dominators[b] = new_dom;
                changed = true;
            }
        }
    }
}

void ControlFlowInfo::computeImmediateDominator() {
#ifdef DEBUG
    std::cout << "Start to find immediate dominators for: " << func->funcname << endl;
#endif
    // idom(b) is the strict dominator d of b such that every other strict
    // dominator of b also dominates d (i.e., d is the closest to b).
    immediateDominator.clear();
    for (auto b : allBlocks) {
        if (b == entryBlock) {
            immediateDominator[b] = -1; // entry has no immediate dominator
            continue;
        }
        auto doms = dominators[b];
        doms.erase(b); // strict dominators only
        for (auto d : doms) {
            // Check: every other strict dominator of b must also dominate d
            bool is_idom = true;
            for (auto d2 : doms) {
                if (d2 == d) continue;
                if (dominators[d].count(d2) == 0) {
                    is_idom = false;
                    break;
                }
            }
            if (is_idom) {
                immediateDominator[b] = d;
                break;
            }
        }
    }
}

void ControlFlowInfo::computeDomTree() {
#ifdef DEBUG
    std::cout << "Computing dominator tree for: " << func->funcname << endl;
#endif
    // Build dominator tree from immediate dominators:
    // each node's children are those whose idom is that node.
    domTree.clear();
    for (auto b : allBlocks) domTree[b] = set<int>();
    for (auto b : allBlocks) {
        if (b == entryBlock) continue;
        auto idom = immediateDominator[b];
        if (idom != -1) domTree[idom].insert(b);
    }
}

void ControlFlowInfo::computeDominanceFrontiers() {
#ifdef DEBUG
    std::cout << "Computing dominance frontier for: " << func->funcname << endl;
#endif
    // Algorithm from Cooper, Harvey, Kennedy:
    // For each CFG edge (a, b), walk up the dominator tree from a
    // until we reach idom(b), adding b to each node's DF along the way.
    dominanceFrontiers.clear();
    for (auto b : allBlocks) dominanceFrontiers[b] = set<int>();
    for (auto a : allBlocks) {
        for (auto b : successors[a]) {
            auto runner = a;
            while (runner != immediateDominator[b]) {
                dominanceFrontiers[runner].insert(b);
                runner = immediateDominator[runner];
                if (runner == -1) break; // reached past entry
            }
        }
    }
}
