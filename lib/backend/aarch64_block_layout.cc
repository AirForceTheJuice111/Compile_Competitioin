#include "aarch64_block_layout.hh"

#include <algorithm>
#include <map>
#include <set>

#include "cfg_transform.hh"

namespace backend {
namespace {

int blockLabel(quad::QuadBlock *block) {
    return block == nullptr || block->entry_label == nullptr
        ? -1 : block->entry_label->num;
}

} // namespace

std::vector<quad::QuadBlock *> buildAarch64TraceLayout(
    quad::QuadFuncDecl *func) {
    std::vector<quad::QuadBlock *> original;
    if (func == nullptr || func->quadblocklist == nullptr) return original;
    original = *func->quadblocklist;
    if (original.size() < 2) return original;

    ControlFlowInfo *cfi = quad::buildControlFlowInfo(func);
    if (cfi == nullptr) return original;
    std::map<int, int> index;
    std::map<int, int> loopDepth;
    for (std::size_t i = 0; i < original.size(); ++i) {
        index[blockLabel(original[i])] = static_cast<int>(i);
    }
    for (const auto &loop : quad::findNaturalLoopInfo(func, cfi)) {
        for (int label : loop.blocks) ++loopDepth[label];
    }

    auto better = [&](int left, int right) {
        if (right < 0) return true;
        if (loopDepth[left] != loopDepth[right]) {
            return loopDepth[left] > loopDepth[right];
        }
        bool leftReturns = cfi->successors[left].empty();
        bool rightReturns = cfi->successors[right].empty();
        if (leftReturns != rightReturns) return !leftReturns;
        return index[left] < index[right];
    };

    std::vector<quad::QuadBlock *> result;
    std::set<int> placed;
    auto appendTrace = [&](int start) {
        int current = start;
        while (current >= 0 && placed.insert(current).second) {
            auto block = cfi->labelToBlock.find(current);
            if (block == cfi->labelToBlock.end()) break;
            result.push_back(block->second);
            int next = -1;
            for (int successor : cfi->successors[current]) {
                if (placed.count(successor) == 0 && better(successor, next)) {
                    next = successor;
                }
            }
            current = next;
        }
    };

    appendTrace(cfi->entryBlock);
    while (result.size() < original.size()) {
        int nextStart = -1;
        for (auto *block : original) {
            int label = blockLabel(block);
            if (label >= 0 && placed.count(label) == 0 && better(label, nextStart)) {
                nextStart = label;
            }
        }
        if (nextStart < 0) break;
        appendTrace(nextStart);
    }
    return result.size() == original.size() ? result : original;
}

} // namespace backend
