#include <stack>
#include <map>
#include <set>
#include <algorithm>
#include <vector>
#include "loopheader.hh"
#include "flowinfo.hh"
#include "quad.hh"

using namespace std;
using namespace quad;

LoopHeaderMap* findLoopHeadersWithFlow(QuadFuncDecl* func, ControlFlowInfo* flowInfo) {
    if (func == nullptr || flowInfo == nullptr) {
        return new LoopHeaderMap();  // Return empty map if missing required info
    }

    LoopHeaderMap* loopHeaderMap = new LoopHeaderMap();
    loopHeaderMap->initFunc(func);

    // A natural loop is recognized from a back edge tail -> header.
    // In a reducible CFG, the edge is a back edge exactly when header dominates tail.
    // Multiple back edges may point to the same header, so loops are first merged by
    // header label before being converted to LoopHeader objects.
    map<int, set<int>> loops;
    for (auto edgeFrom : flowInfo->successors) {
        int tail = edgeFrom.first;
        for (int header : edgeFrom.second) {
            if (!flowInfo->dominators.count(tail) || !flowInfo->dominators.at(tail).count(header)) continue;

            // Collect the natural loop body by walking predecessors backwards from
            // the back-edge tail. The header is included immediately and stops
            // backward expansion, matching the standard Tiger-book algorithm.
            set<int> bodyBlocks;
            stack<int> worklist;
            bodyBlocks.insert(header);
            bodyBlocks.insert(tail);
            worklist.push(tail);

            while (!worklist.empty()) {
                int block = worklist.top();
                worklist.pop();
                if (!flowInfo->predecessors.count(block)) continue;
                for (int pred : flowInfo->predecessors.at(block)) {
                    if (bodyBlocks.count(pred)) continue;
                    bodyBlocks.insert(pred);
                    if (pred != header) worklist.push(pred);
                }
            }
            loops[header].insert(bodyBlocks.begin(), bodyBlocks.end());
        }
    }

    set<LoopHeader*> loopHeaders;
    for (auto loop : loops) loopHeaders.insert(new LoopHeader(loop.first, loop.second));
    loopHeaderMap->addLoopHeader(func, loopHeaders);

    return loopHeaderMap;
}
