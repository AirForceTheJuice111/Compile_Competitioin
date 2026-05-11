#include <stack>
#include <map>
#include <set>
#include "loopopt.hh"
#include "flowinfo.hh"
#include "quad.hh"

// Function to find loop headers in a function and populate the LoopHeaderMap
// Complete the function!!

LoopHeaderMap *findLoopHeaders(QuadFuncDecl* func, FuncFlowInfo *ffi) {
    LoopHeaderMap *loopHeaderMap = new LoopHeaderMap();
    loopHeaderMap->initFunc(func);
    if (func == nullptr || ffi == nullptr || ffi->cfi == nullptr) {
        return loopHeaderMap;
    }

    ControlFlowInfo *cfi = ffi->cfi;
    map<int, set<int>> loops; // header -> body blocks

    for (auto edgeFrom : cfi->successors) {
        int tail = edgeFrom.first;
        for (int header : edgeFrom.second) {
            if (!cfi->dominators.count(tail) || !cfi->dominators[tail].count(header)) continue;

            set<int> bodyBlocks;
            stack<int> worklist;
            bodyBlocks.insert(header);
            bodyBlocks.insert(tail);
            worklist.push(tail);

            while (!worklist.empty()) {
                int block = worklist.top();
                worklist.pop();
                for (int pred : cfi->predecessors[block]) {
                    if (bodyBlocks.count(pred)) continue;
                    bodyBlocks.insert(pred);
                    if (pred != header) worklist.push(pred);
                }
            }

            loops[header].insert(bodyBlocks.begin(), bodyBlocks.end());
        }
    }

    set<LoopHeader*> loopHeaders;
    for (auto loop : loops) {
        loopHeaders.insert(new LoopHeader(loop.first, loop.second));
    }
    loopHeaderMap->addLoopHeader(func, loopHeaders);

    return loopHeaderMap;
}
