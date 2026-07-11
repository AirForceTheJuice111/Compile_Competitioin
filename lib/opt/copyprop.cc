#include "copyprop.hh"

#include <map>
#include <set>
#include <string>
#include <vector>

#include "flowinfo.hh"
#include "quad_metadata.hh"

namespace quad {
namespace {

int resolveCopy(const std::map<int, int> &copies, int temp) {
    std::set<int> visited;
    int current = temp;
    while (visited.insert(current).second) {
        auto found = copies.find(current);
        if (found == copies.end() || found->second == current) break;
        current = found->second;
    }
    return current;
}

int propagateCopies(QuadFuncDecl *func) {
    if (func == nullptr || func->quadblocklist == nullptr) return 0;
    rebuildQuadFunctionMetadata(func);
    ControlFlowInfo flow(func);
    flow.computeEverything();
    rebuildQuadFunctionMetadata(func);
    std::string reason;
    if (!verifySsaFunction(func, &flow, &reason)) return 0;

    std::map<int, int> copies;
    for (auto *block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) continue;
        for (auto *stm : *block->quadlist) {
            if (stm == nullptr || stm->kind != QuadKind::MOVE) continue;
            auto *move = static_cast<QuadMove *>(stm);
            if (move->dst == nullptr || move->dst->temp == nullptr || move->src == nullptr ||
                move->src->kind != QuadTermKind::TEMP || move->src->get_temp() == nullptr ||
                move->src->get_temp()->temp == nullptr ||
                move->dst->type != move->src->get_temp()->type) {
                continue;
            }
            int dst = move->dst->temp->num;
            int src = move->src->get_temp()->temp->num;
            if (dst != src) copies[dst] = src;
        }
    }
    if (copies.empty()) return eliminateDeadPureQuadDefs(func);

    std::map<int, int> replacements;
    for (const auto &[dst, src] : copies) {
        int canonical = resolveCopy(copies, src);
        if (canonical != dst) replacements[dst] = canonical;
    }
    rewriteQuadUses(func, replacements);

    int eliminated = 0;
    for (auto *block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) continue;
        auto *kept = new std::vector<QuadStm *>();
        for (auto *stm : *block->quadlist) {
            bool remove = false;
            if (stm != nullptr && stm->kind == QuadKind::MOVE) {
                auto *move = static_cast<QuadMove *>(stm);
                remove = move->dst != nullptr && move->dst->temp != nullptr &&
                         replacements.count(move->dst->temp->num) != 0;
            }
            if (remove) ++eliminated;
            else kept->push_back(stm);
        }
        block->quadlist = kept;
    }
    rebuildQuadFunctionMetadata(func);
    eliminated += eliminateDeadPureQuadDefs(func);
    return eliminated;
}

} // namespace

QuadProgram *copyPropProg(QuadProgram *prog, int *eliminatedOut) {
    if (prog == nullptr || prog->quadFuncDeclList == nullptr) return prog;
    auto *functions = new std::vector<QuadFuncDecl *>();
    int eliminated = 0;
    for (auto *original : *prog->quadFuncDeclList) {
        if (original == nullptr) continue;
        auto *candidate = static_cast<QuadFuncDecl *>(original->clone());
        int functionEliminated = propagateCopies(candidate);
        rebuildQuadFunctionMetadata(candidate);
        ControlFlowInfo verifyFlow(candidate);
        verifyFlow.computeEverything();
        rebuildQuadFunctionMetadata(candidate);
        if (!verifySsaFunction(candidate, &verifyFlow, nullptr)) {
            auto *fallback = static_cast<QuadFuncDecl *>(original->clone());
            rebuildQuadFunctionMetadata(fallback);
            functions->push_back(fallback);
        } else {
            functions->push_back(candidate);
            eliminated += functionEliminated;
        }
    }
    auto *result = new QuadProgram(functions, prog->last_label_num, prog->last_temp_num);
    rebuildQuadProgramMetadata(result);
    if (eliminatedOut != nullptr) *eliminatedOut = eliminated;
    return result;
}

} // namespace quad
