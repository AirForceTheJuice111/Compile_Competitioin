#include "cfg_transform.hh"

#include <algorithm>
#include <map>
#include <stack>

#include "quad_metadata.hh"

namespace quad {
namespace {

int labelNumber(QuadBlock *block) {
    return block == nullptr || block->entry_label == nullptr
        ? -1 : block->entry_label->num;
}

std::set<int> exitNumbers(QuadBlock *block) {
    std::set<int> result;
    if (block == nullptr || block->exit_labels == nullptr) return result;
    for (auto *label : *block->exit_labels) {
        if (label != nullptr) result.insert(label->num);
    }
    return result;
}

std::set<int> terminatorTargets(QuadBlock *block) {
    std::set<int> result;
    if (block == nullptr || block->quadlist == nullptr || block->quadlist->empty()) {
        return result;
    }
    QuadStm *last = block->quadlist->back();
    if (last == nullptr) return result;
    if (last->kind == QuadKind::JUMP) {
        auto *jump = static_cast<QuadJump *>(last);
        if (jump->label != nullptr) result.insert(jump->label->num);
    } else if (last->kind == QuadKind::CJUMP) {
        auto *jump = static_cast<QuadCJump *>(last);
        if (jump->t != nullptr) result.insert(jump->t->num);
        if (jump->f != nullptr) result.insert(jump->f->num);
    }
    return result;
}

} // namespace

ControlFlowInfo *buildControlFlowInfo(QuadFuncDecl *func) {
    if (func == nullptr || func->quadblocklist == nullptr ||
        func->quadblocklist->empty()) {
        return nullptr;
    }
    auto *cfi = new ControlFlowInfo(func);
    cfi->computeAllBlocks();
    cfi->computePredecessors();
    cfi->computeSuccessors();
    cfi->computeDominators();
    cfi->computeImmediateDominator();
    cfi->computeDomTree();
    cfi->computeDominanceFrontiers();
    return cfi;
}

QuadBlock *findQuadBlock(QuadFuncDecl *func, int label) {
    if (func == nullptr || func->quadblocklist == nullptr) return nullptr;
    for (auto *block : *func->quadblocklist) {
        if (labelNumber(block) == label) return block;
    }
    return nullptr;
}

std::vector<NaturalLoopInfo> findNaturalLoopInfo(QuadFuncDecl *func,
                                                 ControlFlowInfo *cfi) {
    std::vector<NaturalLoopInfo> result;
    if (func == nullptr || cfi == nullptr) return result;

    std::map<int, NaturalLoopInfo> byHeader;
    for (const auto &entry : cfi->successors) {
        int tail = entry.first;
        for (int header : entry.second) {
            auto dom = cfi->dominators.find(tail);
            if (dom == cfi->dominators.end() || dom->second.count(header) == 0) {
                continue;
            }
            auto &loop = byHeader[header];
            loop.header = header;
            loop.backedgePredecessors.insert(tail);
            loop.blocks.insert(header);
            loop.blocks.insert(tail);
            std::stack<int> work;
            // A self-loop already consists of its header.  Walking the
            // header's predecessors here would incorrectly absorb entry-side
            // predecessors into the natural loop.
            if (tail != header) work.push(tail);
            while (!work.empty()) {
                int block = work.top();
                work.pop();
                auto preds = cfi->predecessors.find(block);
                if (preds == cfi->predecessors.end()) continue;
                for (int pred : preds->second) {
                    if (!loop.blocks.insert(pred).second || pred == header) continue;
                    work.push(pred);
                }
            }
        }
    }

    for (auto &entry : byHeader) {
        auto &loop = entry.second;
        for (int pred : cfi->predecessors[loop.header]) {
            if (loop.blocks.count(pred) == 0) loop.outsidePredecessors.insert(pred);
        }
        for (int block : loop.blocks) {
            for (int successor : cfi->successors[block]) {
                if (loop.blocks.count(successor) == 0) {
                    loop.exitEdges.insert({block, successor});
                }
            }
        }
        result.push_back(loop);
    }
    std::sort(result.begin(), result.end(), [](const NaturalLoopInfo &left,
                                               const NaturalLoopInfo &right) {
        if (left.blocks.size() != right.blocks.size()) {
            return left.blocks.size() < right.blocks.size();
        }
        return left.header < right.header;
    });
    return result;
}

bool retargetQuadEdge(QuadFuncDecl *func, int predecessor, int oldTarget,
                      int newTarget) {
    QuadBlock *block = findQuadBlock(func, predecessor);
    if (block == nullptr || block->quadlist == nullptr ||
        block->exit_labels == nullptr) {
        return false;
    }
    bool foundExit = false;
    for (auto *&label : *block->exit_labels) {
        if (label != nullptr && label->num == oldTarget) {
            label = new Label(newTarget);
            foundExit = true;
        }
    }
    if (!foundExit) return false;

    if (!block->quadlist->empty()) {
        QuadStm *last = block->quadlist->back();
        if (last != nullptr && last->kind == QuadKind::JUMP) {
            auto *jump = static_cast<QuadJump *>(last);
            if (jump->label != nullptr && jump->label->num == oldTarget) {
                jump->label = new Label(newTarget);
            }
        } else if (last != nullptr && last->kind == QuadKind::CJUMP) {
            auto *jump = static_cast<QuadCJump *>(last);
            if (jump->t != nullptr && jump->t->num == oldTarget) {
                jump->t = new Label(newTarget);
            }
            if (jump->f != nullptr && jump->f->num == oldTarget) {
                jump->f = new Label(newTarget);
            }
        }
    }
    return true;
}

bool replacePhiPredecessor(QuadBlock *target, int oldPredecessor,
                           int newPredecessor) {
    if (target == nullptr || target->quadlist == nullptr) return false;
    bool changed = false;
    for (auto *stm : *target->quadlist) {
        if (stm == nullptr || stm->kind == QuadKind::LABEL) continue;
        if (stm->kind != QuadKind::PHI) break;
        auto *phi = static_cast<QuadPhi *>(stm);
        if (phi->args == nullptr) continue;
        for (auto &arg : *phi->args) {
            if (arg.second != nullptr && arg.second->num == oldPredecessor) {
                arg.second = new Label(newPredecessor);
                changed = true;
            }
        }
    }
    return changed;
}

bool verifyQuadSsaCfg(QuadFuncDecl *func, std::string *reason) {
    auto fail = [&](const std::string &message) {
        if (reason != nullptr) *reason = message;
        return false;
    };
    if (func == nullptr || func->quadblocklist == nullptr ||
        func->quadblocklist->empty()) {
        return fail("missing function blocks");
    }

    std::set<int> labels;
    for (auto *block : *func->quadblocklist) {
        int label = labelNumber(block);
        if (label < 0 || block->quadlist == nullptr ||
            !labels.insert(label).second) {
            return fail("missing or duplicate block label");
        }
        if (block->quadlist->empty() || block->quadlist->front() == nullptr ||
            block->quadlist->front()->kind != QuadKind::LABEL ||
            static_cast<QuadLabel *>(block->quadlist->front())->label == nullptr ||
            static_cast<QuadLabel *>(block->quadlist->front())->label->num != label) {
            return fail("block entry label does not match first statement");
        }
        bool sawOrdinary = false;
        for (std::size_t i = 1; i < block->quadlist->size(); ++i) {
            auto *stm = block->quadlist->at(i);
            if (stm == nullptr) return fail("null statement");
            if (stm->kind == QuadKind::PHI) {
                if (sawOrdinary) return fail("phi appears after an ordinary statement");
            } else {
                sawOrdinary = true;
            }
            if ((stm->kind == QuadKind::JUMP || stm->kind == QuadKind::CJUMP ||
                 stm->kind == QuadKind::RETURN) && i + 1 != block->quadlist->size()) {
                return fail("terminator is not the last statement");
            }
        }
    }
    for (auto *block : *func->quadblocklist) {
        QuadStm *last = block->quadlist->empty()
                            ? nullptr
                            : block->quadlist->back();
        std::set<int> exits = exitNumbers(block);
        if (last != nullptr && last->kind == QuadKind::RETURN && !exits.empty()) {
            return fail("return block has cached successors");
        }
        if (last != nullptr && last->kind == QuadKind::JUMP &&
            static_cast<QuadJump *>(last)->label == nullptr) {
            return fail("jump has no target");
        }
        if (last != nullptr && last->kind == QuadKind::CJUMP) {
            auto *jump = static_cast<QuadCJump *>(last);
            if (jump->t == nullptr || jump->f == nullptr) {
                return fail("conditional jump has a missing target");
            }
        }
        for (int target : exitNumbers(block)) {
            if (labels.count(target) == 0) return fail("edge targets a missing block");
        }
        std::set<int> targets = terminatorTargets(block);
        if (!targets.empty() && targets != exits) {
            return fail("terminator and exit labels disagree");
        }
        if (targets.empty() && block->exit_labels != nullptr &&
            block->exit_labels->size() > 1) {
            return fail("unterminated block has multiple successors");
        }
    }

    ControlFlowInfo *cfi = buildControlFlowInfo(func);
    if (cfi == nullptr) return fail("failed to build control-flow information");
    for (auto *block : *func->quadblocklist) {
        int label = labelNumber(block);
        for (auto *stm : *block->quadlist) {
            if (stm == nullptr || stm->kind == QuadKind::LABEL) continue;
            if (stm->kind != QuadKind::PHI) break;
            auto *phi = static_cast<QuadPhi *>(stm);
            std::set<int> phiPreds;
            if (phi->args == nullptr) return fail("phi has no argument vector");
            for (const auto &arg : *phi->args) {
                if (arg.first == nullptr || arg.second == nullptr ||
                    !phiPreds.insert(arg.second->num).second) {
                    return fail("malformed or duplicate phi predecessor");
                }
            }
            if (phiPreds != cfi->predecessors[label]) {
                return fail("phi predecessors do not match the CFG");
            }
        }
    }
    std::string ssaReason;
    if (!verifySsaFunction(func, cfi, &ssaReason)) {
        return fail("SSA verification failed: " + ssaReason);
    }
    return true;
}

} // namespace quad
