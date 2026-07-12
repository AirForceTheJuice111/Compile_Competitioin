#include "loopsimplify.hh"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

#include "cfg_transform.hh"
#include "quad_metadata.hh"

namespace quad {
namespace {

std::set<Temp *> *emptyTemps() {
    return new std::set<Temp *>();
}

QuadBlock *makeJumpBlock(int label, int target,
                         const std::vector<QuadStm *> &prefix = {}) {
    auto *statements = new std::vector<QuadStm *>();
    statements->push_back(new QuadLabel(new Label(label), emptyTemps(), emptyTemps()));
    statements->insert(statements->end(), prefix.begin(), prefix.end());
    statements->push_back(new QuadJump(new Label(target), emptyTemps(), emptyTemps()));
    auto *exits = new std::vector<Label *>();
    exits->push_back(new Label(target));
    return new QuadBlock(statements, new Label(label), exits);
}

void insertBefore(QuadFuncDecl *func, int beforeLabel, QuadBlock *block) {
    auto found = std::find_if(func->quadblocklist->begin(), func->quadblocklist->end(),
        [&](QuadBlock *candidate) {
            return candidate != nullptr && candidate->entry_label != nullptr &&
                   candidate->entry_label->num == beforeLabel;
        });
    func->quadblocklist->insert(found, block);
}

bool isDedicatedPredecessor(ControlFlowInfo *cfi, const std::set<int> &preds,
                            int target) {
    if (cfi == nullptr || preds.size() != 1) return false;
    int pred = *preds.begin();
    auto successors = cfi->successors.find(pred);
    return successors != cfi->successors.end() && successors->second.size() == 1 &&
           successors->second.count(target) != 0;
}

struct PhiMergePlan {
    QuadPhi *headerPhi = nullptr;
    std::vector<std::pair<Temp *, Label *>> retained;
    std::vector<std::pair<Temp *, Label *>> incoming;
    Temp *joinedValue = nullptr;
    QuadPhi *joinPhi = nullptr;
};

bool mergeEdgesThroughBlock(QuadFuncDecl *func, int target,
                            const std::set<int> &predecessors, int newLabel) {
    QuadBlock *targetBlock = findQuadBlock(func, target);
    if (targetBlock == nullptr || targetBlock->quadlist == nullptr ||
        predecessors.empty()) {
        return false;
    }

    std::vector<PhiMergePlan> plans;
    for (auto *stm : *targetBlock->quadlist) {
        if (stm == nullptr || stm->kind == QuadKind::LABEL) continue;
        if (stm->kind != QuadKind::PHI) break;
        auto *phi = static_cast<QuadPhi *>(stm);
        if (phi->temp_exp == nullptr || phi->temp_exp->temp == nullptr ||
            phi->args == nullptr) {
            return false;
        }
        PhiMergePlan plan;
        plan.headerPhi = phi;
        std::set<int> found;
        for (const auto &arg : *phi->args) {
            if (arg.first == nullptr || arg.second == nullptr) return false;
            if (predecessors.count(arg.second->num) != 0) {
                plan.incoming.push_back({new Temp(arg.first->num),
                                         new Label(arg.second->num)});
                found.insert(arg.second->num);
            } else {
                plan.retained.push_back({new Temp(arg.first->num),
                                         new Label(arg.second->num)});
            }
        }
        if (found != predecessors) return false;

        int first = plan.incoming.front().first->num;
        bool allSame = std::all_of(plan.incoming.begin(), plan.incoming.end(),
            [&](const std::pair<Temp *, Label *> &arg) {
                return arg.first != nullptr && arg.first->num == first;
            });
        if (allSame) {
            plan.joinedValue = new Temp(first);
        } else {
            int fresh = ++func->last_temp_num;
            plan.joinedValue = new Temp(fresh);
            plan.joinPhi = new QuadPhi(
                new QuadTemp(new Temp(fresh), phi->temp_exp->type),
                new std::vector<std::pair<Temp *, Label *>>(plan.incoming),
                emptyTemps(), emptyTemps());
        }
        plans.push_back(plan);
    }

    for (int pred : predecessors) {
        if (!retargetQuadEdge(func, pred, target, newLabel)) return false;
    }

    std::vector<QuadStm *> joinPhis;
    for (auto &plan : plans) {
        auto *args = new std::vector<std::pair<Temp *, Label *>>(plan.retained);
        args->push_back({new Temp(plan.joinedValue->num), new Label(newLabel)});
        plan.headerPhi->args = args;
        if (plan.joinPhi != nullptr) joinPhis.push_back(plan.joinPhi);
    }
    insertBefore(func, target, makeJumpBlock(newLabel, target, joinPhis));
    return true;
}

bool splitEdge(QuadFuncDecl *func, int predecessor, int target, int newLabel) {
    QuadBlock *targetBlock = findQuadBlock(func, target);
    if (targetBlock == nullptr ||
        !retargetQuadEdge(func, predecessor, target, newLabel)) {
        return false;
    }
    replacePhiPredecessor(targetBlock, predecessor, newLabel);
    insertBefore(func, target, makeJumpBlock(newLabel, target));
    return true;
}

bool canonicalizeOne(QuadFuncDecl *func, LoopSimplifyStats &stats) {
    rebuildQuadFunctionMetadata(func);
    ControlFlowInfo *cfi = buildControlFlowInfo(func);
    if (cfi == nullptr) return false;
    std::vector<NaturalLoopInfo> loops = findNaturalLoopInfo(func, cfi);
    for (const auto &loop : loops) {
        if (loop.outsidePredecessors.empty() || loop.backedgePredecessors.empty()) {
            continue;
        }
        if (!isDedicatedPredecessor(cfi, loop.outsidePredecessors, loop.header)) {
            int label = ++func->last_label_num;
            if (mergeEdgesThroughBlock(func, loop.header,
                                       loop.outsidePredecessors, label)) {
                ++stats.preheadersCreated;
                return true;
            }
            return false;
        }
        if (!isDedicatedPredecessor(cfi, loop.backedgePredecessors, loop.header)) {
            int label = ++func->last_label_num;
            if (mergeEdgesThroughBlock(func, loop.header,
                                       loop.backedgePredecessors, label)) {
                ++stats.latchesCreated;
                return true;
            }
            return false;
        }

        for (const auto &edge : loop.exitEdges) {
            int source = edge.first;
            int target = edge.second;
            bool hasOutsidePred = false;
            for (int pred : cfi->predecessors[target]) {
                if (loop.blocks.count(pred) == 0) {
                    hasOutsidePred = true;
                    break;
                }
            }
            if (!hasOutsidePred) continue;
            int label = ++func->last_label_num;
            if (splitEdge(func, source, target, label)) {
                ++stats.exitEdgesSplit;
                return true;
            }
            return false;
        }
    }
    return false;
}

} // namespace

QuadProgram *loopSimplifyProg(QuadProgram *program, LoopSimplifyStats *statsOut) {
    if (program == nullptr || program->quadFuncDeclList == nullptr) return program;
    LoopSimplifyStats stats;
    auto *functions = new std::vector<QuadFuncDecl *>();
    int lastLabel = program->last_label_num;
    int lastTemp = program->last_temp_num;

    for (auto *original : *program->quadFuncDeclList) {
        ++stats.functionsVisited;
        if (original == nullptr) {
            functions->push_back(original);
            ++stats.functionsSkipped;
            continue;
        }
        auto *candidate = static_cast<QuadFuncDecl *>(original->clone());
        candidate->last_label_num = std::max(candidate->last_label_num, lastLabel);
        candidate->last_temp_num = std::max(candidate->last_temp_num, lastTemp);
        rebuildQuadFunctionMetadata(candidate);
        std::string reason;
        if (!verifyQuadSsaCfg(candidate, &reason)) {
            functions->push_back(original);
            ++stats.functionsSkipped;
            continue;
        }

        LoopSimplifyStats candidateStats;
        int before = 0;
        for (int iteration = 0; iteration < 64; ++iteration) {
            if (!canonicalizeOne(candidate, candidateStats)) break;
        }
        rebuildQuadFunctionMetadata(candidate);
        int after = candidateStats.preheadersCreated + candidateStats.latchesCreated +
                    candidateStats.exitEdgesSplit;
        if (after == before) {
            functions->push_back(original);
            continue;
        }
        if (!verifyQuadSsaCfg(candidate, &reason)) {
            functions->push_back(original);
            ++stats.functionsSkipped;
            continue;
        }
        ++stats.functionsChanged;
        stats.preheadersCreated += candidateStats.preheadersCreated;
        stats.latchesCreated += candidateStats.latchesCreated;
        stats.exitEdgesSplit += candidateStats.exitEdgesSplit;
        lastLabel = std::max(lastLabel, candidate->last_label_num);
        lastTemp = std::max(lastTemp, candidate->last_temp_num);
        functions->push_back(candidate);
    }
    auto *result = new QuadProgram(functions, lastLabel, lastTemp);
    rebuildQuadProgramMetadata(result);
    if (statsOut != nullptr) *statsOut = stats;
    return result;
}

} // namespace quad
