#include "gvn.hh"

#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "quad_metadata.hh"

namespace quad {
namespace {

int canonicalTemp(const std::map<int, int> &values, int temp) {
    std::set<int> visited;
    int current = temp;
    while (visited.insert(current).second) {
        auto found = values.find(current);
        if (found == values.end() || found->second == current) break;
        current = found->second;
    }
    return current;
}

std::string typeKey(QuadType type) {
    if (type == QuadType::FLOAT) return "F";
    if (type == QuadType::PTR) return "P";
    return "I";
}

std::string termKey(const std::map<int, int> &values, QuadTerm *term,
                    QuadType contextualType) {
    std::ostringstream out;
    out << typeKey(contextualType) << ':';
    if (term == nullptr) return out.str() + "null";
    if (term->kind == QuadTermKind::CONST) return out.str() + "C" + std::to_string(term->get_const());
    if (term->kind == QuadTermKind::NAME) return out.str() + "N" + term->get_name();
    auto *temp = term->get_temp();
    if (temp == nullptr || temp->temp == nullptr || temp->type != contextualType) return out.str() + "invalid";
    return out.str() + "T" + std::to_string(canonicalTemp(values, temp->temp->num));
}

bool commutative(const std::string &op) {
    return op == "+" || op == "*" || op == "xor" || op == "&" || op == "|" || op == "^";
}

int runGvn(QuadFuncDecl *func, ControlFlowInfo *flow) {
    if (func == nullptr || flow == nullptr || !verifySsaFunction(func, flow, nullptr)) return 0;
    std::map<std::string, int> available;
    std::map<int, int> values;
    std::map<int, int> replacements;
    std::set<int> redundant;
    std::vector<std::string> scope;

    std::function<void(int)> visit = [&](int label) {
        auto blockFound = flow->labelToBlock.find(label);
        if (blockFound == flow->labelToBlock.end() || blockFound->second == nullptr ||
            blockFound->second->quadlist == nullptr) return;
        std::size_t scopeBegin = scope.size();
        for (auto *stm : *blockFound->second->quadlist) {
            if (stm == nullptr) continue;
            int dst = -1;
            std::string key;
            if (stm->kind == QuadKind::MOVE_BINOP) {
                auto *bin = static_cast<QuadMoveBinop *>(stm);
                if (bin->dst == nullptr || bin->dst->temp == nullptr || bin->dst->type != QuadType::INT) continue;
                dst = bin->dst->temp->num;
                std::string left = termKey(values, bin->left, QuadType::INT);
                std::string right = termKey(values, bin->right, QuadType::INT);
                if (commutative(bin->binop) && right < left) std::swap(left, right);
                key = "BIN:" + typeKey(bin->dst->type) + ':' + bin->binop + ':' + left + ':' + right;
            } else if (stm->kind == QuadKind::PTR_CALC) {
                auto *calc = static_cast<QuadPtrCalc *>(stm);
                if (calc->dst == nullptr || calc->dst->kind != QuadTermKind::TEMP ||
                    calc->dst->get_temp() == nullptr || calc->dst->get_temp()->temp == nullptr ||
                    calc->dst->get_temp()->type != QuadType::PTR) continue;
                dst = calc->dst->get_temp()->temp->num;
                key = "PTR:P:" + termKey(values, calc->ptr, QuadType::PTR) + ':' +
                      termKey(values, calc->offset, QuadType::INT);
            } else {
                continue;
            }
            auto found = available.find(key);
            if (found != available.end()) {
                int canonical = canonicalTemp(values, found->second);
                values[dst] = canonical;
                replacements[dst] = canonical;
                redundant.insert(dst);
            } else {
                available[key] = dst;
                values[dst] = dst;
                scope.push_back(key);
            }
        }
        auto children = flow->domTree.find(label);
        if (children != flow->domTree.end()) for (int child : children->second) visit(child);
        while (scope.size() > scopeBegin) {
            available.erase(scope.back());
            scope.pop_back();
        }
    };
    if (flow->entryBlock >= 0) visit(flow->entryBlock);
    if (replacements.empty()) return 0;

    rewriteQuadUses(func, replacements);
    int eliminated = 0;
    for (auto *block : *func->quadblocklist) {
        if (block == nullptr || block->quadlist == nullptr) continue;
        auto *kept = new std::vector<QuadStm *>();
        for (auto *stm : *block->quadlist) {
            int def = -1;
            if (stm != nullptr && stm->kind == QuadKind::MOVE_BINOP) {
                auto *dst = static_cast<QuadMoveBinop *>(stm)->dst;
                if (dst != nullptr && dst->temp != nullptr) def = dst->temp->num;
            } else if (stm != nullptr && stm->kind == QuadKind::PTR_CALC) {
                auto *dst = static_cast<QuadPtrCalc *>(stm)->dst;
                if (dst != nullptr && dst->kind == QuadTermKind::TEMP && dst->get_temp() != nullptr && dst->get_temp()->temp != nullptr) def = dst->get_temp()->temp->num;
            }
            if (def >= 0 && redundant.count(def) != 0) ++eliminated;
            else kept->push_back(stm);
        }
        block->quadlist = kept;
    }
    rebuildQuadFunctionMetadata(func);
    return eliminated;
}

} // namespace

QuadProgram *gvnProg(QuadProgram *prog, std::set<FuncFlowInfo *> *flowInfo,
                     int *eliminatedOut) {
    (void)flowInfo;
    if (prog == nullptr || prog->quadFuncDeclList == nullptr) return prog;
    auto *functions = new std::vector<QuadFuncDecl *>();
    int eliminated = 0;
    for (auto *original : *prog->quadFuncDeclList) {
        if (original == nullptr) continue;
        auto *candidate = static_cast<QuadFuncDecl *>(original->clone());
        rebuildQuadFunctionMetadata(candidate);
        ControlFlowInfo flow(candidate);
        flow.computeEverything();
        rebuildQuadFunctionMetadata(candidate);
        int functionEliminated = runGvn(candidate, &flow);
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
