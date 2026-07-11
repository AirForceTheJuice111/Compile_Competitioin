#include "parallel_plan.hh"

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace sysy {

namespace {

struct ArrayAccess {
    std::string base;
    const Node *lval = nullptr;
};

struct LexicalInfo {
    std::unordered_set<const Node *> localLvals;
    std::unordered_set<std::string> bodyScopeNames;
};

bool isScalarLVal(const Node &node) {
    return node.kind == NodeKind::LVal && node.children.empty();
}

bool isScalarDef(const Node &def) {
    if (def.kind != NodeKind::VarDef && def.kind != NodeKind::ConstDef) {
        return false;
    }
    for (const auto &child : def.children) {
        if (child->kind == NodeKind::ArrayDim) {
            return false;
        }
    }
    return true;
}

const Node *initializerOf(const Node &def) {
    for (const auto &child : def.children) {
        if (child->kind != NodeKind::ArrayDim) {
            return child.get();
        }
    }
    return nullptr;
}

bool exprContainsCall(const Node &node) {
    if (node.kind == NodeKind::CallExpr) {
        return true;
    }
    for (const auto &child : node.children) {
        if (exprContainsCall(*child)) {
            return true;
        }
    }
    return false;
}

int estimateExprCost(const Node &node) {
    switch (node.kind) {
    case NodeKind::Number:
    case NodeKind::StringLiteral:
    case NodeKind::Identifier:
        return 0;
    case NodeKind::LVal:
        return node.children.empty() ? 1 : 4 + static_cast<int>(node.children.size()) * 2;
    case NodeKind::CallExpr:
        return 64;
    case NodeKind::UnaryExpr:
        return 1 + estimateExprCost(*node.children.at(0));
    case NodeKind::BinaryExpr: {
        int opCost = (node.text == "*" || node.text == "/" || node.text == "%") ? 4 : 1;
        return opCost + estimateExprCost(*node.children.at(0)) +
               estimateExprCost(*node.children.at(1));
    }
    default: {
        int cost = 0;
        for (const auto &child : node.children) {
            cost += estimateExprCost(*child);
        }
        return cost;
    }
    }
}

int estimateStmtCost(const Node &node) {
    switch (node.kind) {
    case NodeKind::AssignStmt:
        return 2 + estimateExprCost(*node.children.at(0)) +
               estimateExprCost(*node.children.at(1));
    case NodeKind::VarDecl:
    case NodeKind::ConstDecl: {
        int cost = 1;
        for (const auto &def : node.children) {
            if (const Node *init = initializerOf(*def)) {
                cost += estimateExprCost(*init);
            }
        }
        return cost;
    }
    case NodeKind::IfStmt:
        return 8 + estimateExprCost(*node.children.at(0));
    case NodeKind::WhileStmt:
        return 64 + estimateExprCost(*node.children.at(0));
    default: {
        int cost = 1;
        for (const auto &child : node.children) {
            cost += estimateStmtCost(*child);
        }
        return cost;
    }
    }
}

bool intConstValue(const Node &node, int &value) {
    if (node.kind != NodeKind::Number) {
        return false;
    }
    const std::string &text = node.text;
    if (text.find('.') != std::string::npos || text.find('e') != std::string::npos ||
        text.find('E') != std::string::npos || text.find('p') != std::string::npos ||
        text.find('P') != std::string::npos) {
        return false;
    }
    char *end = nullptr;
    long parsed = std::strtol(text.c_str(), &end, 0);
    if (end == nullptr || *end != '\0' || parsed < INT_MIN || parsed > INT_MAX) {
        return false;
    }
    value = static_cast<int>(parsed);
    return true;
}

bool affineCoeff(const Node &node, const std::string &var,
                 const std::unordered_set<const Node *> &localLvals, int &coeff) {
    if (node.kind == NodeKind::Number) {
        coeff = 0;
        return true;
    }
    if (node.kind == NodeKind::LVal) {
        coeff = node.text == var && localLvals.find(&node) == localLvals.end() ? 1 : 0;
        return true;
    }
    if (node.kind == NodeKind::UnaryExpr) {
        int inner = 0;
        if (!affineCoeff(*node.children.at(0), var, localLvals, inner)) {
            return false;
        }
        if (node.text == "+") {
            coeff = inner;
            return true;
        }
        if (node.text == "-") {
            coeff = -inner;
            return true;
        }
        return false;
    }
    if (node.kind != NodeKind::BinaryExpr) {
        return false;
    }
    int left = 0;
    int right = 0;
    if (node.text == "+" || node.text == "-") {
        if (!affineCoeff(*node.children.at(0), var, localLvals, left) ||
            !affineCoeff(*node.children.at(1), var, localLvals, right)) {
            return false;
        }
        coeff = node.text == "+" ? left + right : left - right;
        return true;
    }
    if (node.text == "*") {
        int c = 0;
        if (intConstValue(*node.children.at(0), c) &&
            affineCoeff(*node.children.at(1), var, localLvals, right)) {
            coeff = c * right;
            return true;
        }
        if (intConstValue(*node.children.at(1), c) &&
            affineCoeff(*node.children.at(0), var, localLvals, left)) {
            coeff = c * left;
            return true;
        }
    }
    return false;
}

bool lvalFirstIndexIsPartitionedByLoopVar(
    const Node &lval, const std::string &var,
    const std::unordered_set<const Node *> &localLvals) {
    if (lval.kind != NodeKind::LVal || lval.children.empty()) {
        return false;
    }
    int coeff = 0;
    return affineCoeff(*lval.children.front(), var, localLvals, coeff) && coeff != 0;
}

std::string nodeKey(const Node &node) {
    std::ostringstream out;
    out << static_cast<int>(node.kind) << ":" << node.text << "(";
    for (const auto &child : node.children) {
        out << nodeKey(*child) << ",";
    }
    out << ")";
    return out.str();
}

bool sameFirstPartitionIndex(const Node &lhs, const Node &rhs, const std::string &var,
                             const std::unordered_set<const Node *> &localLvals) {
    if (lhs.kind != NodeKind::LVal || rhs.kind != NodeKind::LVal ||
        lhs.children.empty() || rhs.children.empty()) {
        return false;
    }
    int lhsCoeff = 0;
    int rhsCoeff = 0;
    if (!affineCoeff(*lhs.children.front(), var, localLvals, lhsCoeff) ||
        !affineCoeff(*rhs.children.front(), var, localLvals, rhsCoeff) ||
        lhsCoeff == 0 || rhsCoeff == 0) {
        return false;
    }
    return nodeKey(*lhs.children.front()) == nodeKey(*rhs.children.front());
}

bool isIncrementOf(const Node &stmt, const std::string &var) {
    if (stmt.kind != NodeKind::AssignStmt || stmt.children.size() != 2) {
        return false;
    }
    const Node &lhs = *stmt.children.at(0);
    const Node &rhs = *stmt.children.at(1);
    if (!isScalarLVal(lhs) || lhs.text != var || rhs.kind != NodeKind::BinaryExpr ||
        rhs.text != "+") {
        return false;
    }
    int c = 0;
    return ((rhs.children.at(0)->kind == NodeKind::LVal &&
             rhs.children.at(0)->text == var &&
             intConstValue(*rhs.children.at(1), c) && c == 1) ||
            (rhs.children.at(1)->kind == NodeKind::LVal &&
             rhs.children.at(1)->text == var &&
             intConstValue(*rhs.children.at(0), c) && c == 1));
}

bool canonicalWhile(const Node &loop, const ParallelLoopInit &init, const Node *&endExpr,
                    bool &inclusiveEnd, std::vector<const Node *> &body) {
    if (!init.valid || loop.kind != NodeKind::WhileStmt || loop.children.size() != 2) {
        return false;
    }
    const Node &cond = *loop.children.at(0);
    if (cond.kind != NodeKind::BinaryExpr || (cond.text != "<" && cond.text != "<=") ||
        cond.children.at(0)->kind != NodeKind::LVal ||
        cond.children.at(0)->text != init.var ||
        !cond.children.at(0)->children.empty()) {
        return false;
    }
    const Node &bodyNode = *loop.children.at(1);
    if (bodyNode.kind != NodeKind::Block || bodyNode.children.empty()) {
        return false;
    }
    const Node &last = *bodyNode.children.back();
    if (!isIncrementOf(last, init.var)) {
        return false;
    }
    endExpr = cond.children.at(1).get();
    inclusiveEnd = cond.text == "<=";
    body.clear();
    for (std::size_t i = 0; i + 1 < bodyNode.children.size(); ++i) {
        body.push_back(bodyNode.children[i].get());
    }
    return true;
}

bool nameIsLocal(const std::vector<std::unordered_set<std::string>> &scopes,
                 const std::string &name) {
    return std::any_of(scopes.rbegin(), scopes.rend(), [&](const auto &scope) {
        return scope.find(name) != scope.end();
    });
}

void collectLexicalInfo(const Node &node,
                        std::vector<std::unordered_set<std::string>> &scopes,
                        LexicalInfo &info) {
    if (node.kind == NodeKind::LVal) {
        if (nameIsLocal(scopes, node.text)) {
            info.localLvals.insert(&node);
        }
        for (const auto &index : node.children) {
            collectLexicalInfo(*index, scopes, info);
        }
        return;
    }
    if (node.kind == NodeKind::Block) {
        scopes.push_back({});
        for (const auto &stmt : node.children) {
            collectLexicalInfo(*stmt, scopes, info);
        }
        scopes.pop_back();
        return;
    }
    if (node.kind == NodeKind::VarDecl || node.kind == NodeKind::ConstDecl) {
        for (const auto &def : node.children) {
            // Match lowering: each definition becomes visible to its initializer,
            // and definitions in one declaration are processed in source order.
            if (isScalarDef(*def)) {
                scopes.back().insert(def->text);
            }
            for (const auto &child : def->children) {
                collectLexicalInfo(*child, scopes, info);
            }
        }
        return;
    }
    for (const auto &child : node.children) {
        collectLexicalInfo(*child, scopes, info);
    }
}

LexicalInfo buildLexicalInfo(const std::vector<const Node *> &body) {
    LexicalInfo info;
    std::vector<std::unordered_set<std::string>> scopes(1);
    for (const Node *stmt : body) {
        collectLexicalInfo(*stmt, scopes, info);
    }
    info.bodyScopeNames = std::move(scopes.front());
    return info;
}

bool containsScalarDeclarationNamed(const Node &node, const std::string &name) {
    if (node.kind == NodeKind::VarDecl || node.kind == NodeKind::ConstDecl) {
        for (const auto &def : node.children) {
            if (isScalarDef(*def) && def->text == name) {
                return true;
            }
        }
    }
    return std::any_of(node.children.begin(), node.children.end(), [&](const auto &child) {
        return containsScalarDeclarationNamed(*child, name);
    });
}

bool containsScalarWriteNamed(const Node &node, const std::string &name) {
    if (node.kind == NodeKind::AssignStmt && node.children.size() == 2 &&
        isScalarLVal(*node.children.front()) && node.children.front()->text == name) {
        return true;
    }
    return std::any_of(node.children.begin(), node.children.end(), [&](const auto &child) {
        return containsScalarWriteNamed(*child, name);
    });
}

void markPrivatizedScalarLvals(const Node &node, const std::string &name,
                              std::unordered_set<const Node *> &localLvals) {
    if (node.kind == NodeKind::LVal && node.text == name) {
        localLvals.insert(&node);
    }
    for (const auto &child : node.children) {
        markPrivatizedScalarLvals(*child, name, localLvals);
    }
}

std::optional<ParallelPrivatizedScalar> recognizeCanonicalScratchScalar(
    const ParallelLoopPlan &plan, const ParallelTypeLookup &lookupType) {
    if (plan.body.size() != 2) {
        return std::nullopt;
    }

    ParallelLoopInit scratchInit = parseParallelLoopInit(*plan.body.front());
    if (!scratchInit.valid || scratchInit.declaration || scratchInit.initExpr == nullptr ||
        scratchInit.var == plan.init.var || !lookupType ||
        lookupType(scratchInit.var) != "int") {
        return std::nullopt;
    }
    int ignoredInit = 0;
    if (!intConstValue(*scratchInit.initExpr, ignoredInit)) {
        return std::nullopt;
    }

    const Node *scratchEnd = nullptr;
    bool scratchInclusive = false;
    std::vector<const Node *> scratchBody;
    if (!canonicalWhile(*plan.body.back(), scratchInit, scratchEnd,
                        scratchInclusive, scratchBody) || scratchEnd == nullptr) {
        return std::nullopt;
    }

    if (scratchEnd->kind == NodeKind::Number) {
        int ignoredEnd = 0;
        if (!intConstValue(*scratchEnd, ignoredEnd) ||
            (scratchInclusive && ignoredEnd == INT_MAX)) {
            return std::nullopt;
        }
    } else if (!scratchInclusive && scratchEnd->kind == NodeKind::LVal &&
               scratchEnd->children.empty()) {
        if (scratchEnd->text == scratchInit.var || scratchEnd->text == plan.init.var ||
            lookupType(scratchEnd->text) != "int") {
            return std::nullopt;
        }
    } else {
        return std::nullopt;
    }

    for (const Node *stmt : plan.body) {
        if (containsScalarDeclarationNamed(*stmt, scratchInit.var)) {
            return std::nullopt;
        }
    }
    for (const Node *stmt : scratchBody) {
        if (containsScalarWriteNamed(*stmt, scratchInit.var)) {
            return std::nullopt;
        }
    }

    if (scratchEnd->kind == NodeKind::LVal) {
        for (const Node *stmt : plan.body) {
            if (containsScalarWriteNamed(*stmt, scratchEnd->text)) {
                return std::nullopt;
            }
        }
    }

    return ParallelPrivatizedScalar{scratchInit.var, "int", scratchInit.initExpr,
                                    scratchEnd, scratchInclusive};
}

bool containsWhile(const Node &node) {
    if (node.kind == NodeKind::WhileStmt) {
        return true;
    }
    return std::any_of(node.children.begin(), node.children.end(), [](const auto &child) {
        return containsWhile(*child);
    });
}

bool containsArrayLVal(const Node &node) {
    if (node.kind == NodeKind::LVal && !node.children.empty()) {
        return true;
    }
    return std::any_of(node.children.begin(), node.children.end(), [](const auto &child) {
        return containsArrayLVal(*child);
    });
}

bool containsExternalScalarReference(
    const Node &node, const std::string &name,
    const std::unordered_set<const Node *> &localLvals) {
    if (node.kind == NodeKind::LVal && node.children.empty() && node.text == name &&
        localLvals.find(&node) == localLvals.end()) {
        return true;
    }
    return std::any_of(node.children.begin(), node.children.end(), [&](const auto &child) {
        return containsExternalScalarReference(*child, name, localLvals);
    });
}

void collectExternalScalarWrites(
    const Node &node, const std::unordered_set<const Node *> &localLvals,
    std::unordered_set<std::string> &writes) {
    if (node.kind == NodeKind::AssignStmt && node.children.size() == 2) {
        const Node &lhs = *node.children.at(0);
        if (isScalarLVal(lhs) && localLvals.find(&lhs) == localLvals.end()) {
            writes.insert(lhs.text);
        }
    }
    for (const auto &child : node.children) {
        collectExternalScalarWrites(*child, localLvals, writes);
    }
}

bool exprIsProvablyInt(const Node &node, const ParallelTypeLookup &lookupType) {
    if (node.kind == NodeKind::Number) {
        int ignored = 0;
        return intConstValue(node, ignored);
    }
    if (node.kind == NodeKind::LVal) {
        return node.children.empty() && lookupType && lookupType(node.text) == "int";
    }
    if (node.kind == NodeKind::UnaryExpr) {
        return node.children.size() == 1 && exprIsProvablyInt(*node.children.front(), lookupType);
    }
    if (node.kind == NodeKind::BinaryExpr) {
        return node.children.size() == 2 &&
               exprIsProvablyInt(*node.children.at(0), lookupType) &&
               exprIsProvablyInt(*node.children.at(1), lookupType);
    }
    return false;
}

void addReduction(std::vector<ParallelReduction> &reductions, ParallelReduction info) {
    auto found = std::find_if(reductions.begin(), reductions.end(),
                              [&](const ParallelReduction &existing) {
                                  return existing.var == info.var;
                              });
    if (found == reductions.end()) {
        reductions.push_back(info);
    }
}

void collectArrayReads(const Node &node, std::vector<ArrayAccess> &reads) {
    if (node.kind == NodeKind::LVal && !node.children.empty()) {
        reads.push_back(ArrayAccess{node.text, &node});
    }
    for (const auto &child : node.children) {
        collectArrayReads(*child, reads);
    }
}

void collectArrayWrites(const Node &node, std::vector<ArrayAccess> &writes) {
    if (node.kind == NodeKind::AssignStmt && node.children.size() == 2) {
        const Node *lhs = node.children.at(0).get();
        if (lhs != nullptr && lhs->kind == NodeKind::LVal && !lhs->children.empty()) {
            writes.push_back(ArrayAccess{lhs->text, lhs});
        }
    }
    for (const auto &child : node.children) {
        collectArrayWrites(*child, writes);
    }
}

void noteCapture(ParallelLoopPlan &plan, const std::string &name, const std::string &type,
                 bool array, bool read, bool write) {
    auto found = std::find_if(plan.captures.begin(), plan.captures.end(),
                              [&](const ParallelCapture &capture) {
                                  return capture.name == name && capture.array == array;
                              });
    if (found == plan.captures.end()) {
        plan.captures.push_back(ParallelCapture{name, type, array, read, write});
        return;
    }
    if (found->type.empty()) {
        found->type = type;
    }
    found->read = found->read || read;
    found->write = found->write || write;
}

bool isReductionVar(const ParallelLoopPlan &plan, const std::string &name) {
    return std::any_of(plan.reductions.begin(), plan.reductions.end(),
                       [&](const ParallelReduction &reduction) {
                           return reduction.var == name;
                       });
}

void collectCapturesFromExpr(const Node &node, const std::string &loopVar,
                             const std::unordered_set<const Node *> &localLvals,
                             const ParallelTypeLookup &lookupType,
                             ParallelLoopPlan &plan) {
    if (node.kind == NodeKind::LVal) {
        if (!node.children.empty()) {
            noteCapture(plan, node.text, lookupType ? lookupType(node.text) : "", true, true, false);
            for (const auto &index : node.children) {
                collectCapturesFromExpr(*index, loopVar, localLvals, lookupType, plan);
            }
            return;
        }
        if (node.text != loopVar && localLvals.find(&node) == localLvals.end() &&
            !isReductionVar(plan, node.text)) {
            noteCapture(plan, node.text, lookupType ? lookupType(node.text) : "", false, true, false);
        }
        return;
    }
    for (const auto &child : node.children) {
        collectCapturesFromExpr(*child, loopVar, localLvals, lookupType, plan);
    }
}

void collectCapturesFromStmt(const Node &node, const std::string &loopVar,
                             const std::unordered_set<const Node *> &localLvals,
                             const ParallelTypeLookup &lookupType,
                             ParallelLoopPlan &plan) {
    if (node.kind == NodeKind::LVal || node.kind == NodeKind::BinaryExpr ||
        node.kind == NodeKind::UnaryExpr || node.kind == NodeKind::CallExpr ||
        node.kind == NodeKind::InitList) {
        collectCapturesFromExpr(node, loopVar, localLvals, lookupType, plan);
        return;
    }
    if (node.kind == NodeKind::AssignStmt && node.children.size() == 2) {
        const Node &lhs = *node.children.at(0);
        if (lhs.kind == NodeKind::LVal && !lhs.children.empty()) {
            noteCapture(plan, lhs.text, lookupType ? lookupType(lhs.text) : "", true, false, true);
            for (const auto &index : lhs.children) {
                collectCapturesFromExpr(*index, loopVar, localLvals, lookupType, plan);
            }
        } else if (lhs.kind == NodeKind::LVal && lhs.text != loopVar &&
                   localLvals.find(&lhs) == localLvals.end() &&
                   !isReductionVar(plan, lhs.text)) {
            noteCapture(plan, lhs.text, lookupType ? lookupType(lhs.text) : "", false, false, true);
        }
        collectCapturesFromExpr(*node.children.at(1), loopVar, localLvals, lookupType, plan);
        return;
    }
    if (node.kind == NodeKind::VarDecl || node.kind == NodeKind::ConstDecl) {
        for (const auto &def : node.children) {
            if (const Node *init = initializerOf(*def)) {
                collectCapturesFromExpr(*init, loopVar, localLvals, lookupType, plan);
            }
        }
        return;
    }
    for (const auto &child : node.children) {
        collectCapturesFromStmt(*child, loopVar, localLvals, lookupType, plan);
    }
}

bool sameArrayReadsStayInWrittenPartition(const Node &writeLval, const Node &rhs,
                                          const std::string &loopVar,
                                          const std::unordered_set<const Node *> &localLvals) {
    std::vector<ArrayAccess> reads;
    collectArrayReads(rhs, reads);
    for (const ArrayAccess &read : reads) {
        if (read.base != writeLval.text) {
            continue;
        }
        if (read.lval == nullptr ||
            !sameFirstPartitionIndex(writeLval, *read.lval, loopVar, localLvals)) {
            return false;
        }
    }
    return true;
}

bool analyzeNode(const Node &node, const std::string &loopVar,
                 const std::unordered_set<const Node *> &localLvals,
                 const ParallelTypeLookup &lookupType,
                 ParallelLoopPlan &plan) {
    if (exprContainsCall(node)) {
        plan.rejectReason = "call in loop body";
        return false;
    }
    switch (node.kind) {
    case NodeKind::BreakStmt:
    case NodeKind::ContinueStmt:
    case NodeKind::ReturnStmt:
        plan.rejectReason = "control transfer in loop body";
        return false;
    case NodeKind::VarDecl:
    case NodeKind::ConstDecl:
        for (const auto &def : node.children) {
            if (!isScalarDef(*def)) {
                plan.rejectReason = "non-scalar local declaration";
                return false;
            }
        }
        return true;
    case NodeKind::AssignStmt: {
        const Node &lhs = *node.children.at(0);
        if (isScalarLVal(lhs)) {
            if (localLvals.find(&lhs) != localLvals.end()) {
                return true;
            }
            if (lhs.text == loopVar) {
                plan.rejectReason = "loop induction variable write";
                return false;
            }
            if (lookupType && lookupType(lhs.text) == "int") {
                const Node *add = parallelReductionAddend(node, lhs.text);
                if (add != nullptr &&
                    !containsExternalScalarReference(*add, lhs.text, localLvals)) {
                    addReduction(plan.reductions, ParallelReduction{lhs.text, add});
                    return true;
                }
            }
            plan.rejectReason = "unsafe scalar write";
            return false;
        }
        if (!lvalFirstIndexIsPartitionedByLoopVar(lhs, loopVar, localLvals)) {
            plan.rejectReason = "non-affine array write";
            return false;
        }
        if (!sameArrayReadsStayInWrittenPartition(lhs, *node.children.at(1), loopVar,
                                                  localLvals)) {
            plan.rejectReason = "same-array read is not partitioned";
            return false;
        }
        plan.hasArrayWrite = true;
        return true;
    }
    default:
        for (const auto &child : node.children) {
            if (!analyzeNode(*child, loopVar, localLvals, lookupType, plan)) {
                return false;
            }
        }
        return true;
    }
}

bool validateReductionUses(
    const Node &node, const std::string &var,
    const std::unordered_set<const Node *> &localLvals) {
    if (node.kind == NodeKind::AssignStmt && node.children.size() == 2) {
        const Node &lhs = *node.children.at(0);
        if (isScalarLVal(lhs) && lhs.text == var &&
            localLvals.find(&lhs) == localLvals.end()) {
            const Node *addend = parallelReductionAddend(node, var);
            return addend != nullptr &&
                   !containsExternalScalarReference(*addend, var, localLvals);
        }
    }
    if (node.kind == NodeKind::LVal && node.children.empty() && node.text == var &&
        localLvals.find(&node) == localLvals.end()) {
        return false;
    }
    return std::all_of(node.children.begin(), node.children.end(), [&](const auto &child) {
        return validateReductionUses(*child, var, localLvals);
    });
}

bool boundReferencesWrittenScalar(const Node &node,
                                  const std::unordered_set<std::string> &writes) {
    if (node.kind == NodeKind::LVal && node.children.empty() &&
        writes.find(node.text) != writes.end()) {
        return true;
    }
    return std::any_of(node.children.begin(), node.children.end(), [&](const auto &child) {
        return boundReferencesWrittenScalar(*child, writes);
    });
}

bool validateLoopBound(ParallelLoopPlan &plan, const LexicalInfo &lexical,
                       const ParallelTypeLookup &lookupType) {
    if (plan.endExpr == nullptr) {
        plan.rejectReason = "missing loop endpoint";
        return false;
    }
    std::string ivType = plan.init.declaration
                             ? plan.init.type
                             : (lookupType ? lookupType(plan.init.var) : std::string{});
    if (ivType != "int") {
        plan.rejectReason = "non-int induction variable";
        return false;
    }
    if (exprContainsCall(*plan.endExpr)) {
        plan.rejectReason = "call in loop endpoint";
        return false;
    }
    if (containsArrayLVal(*plan.endExpr)) {
        plan.rejectReason = "array access in loop endpoint";
        return false;
    }
    static const std::unordered_set<const Node *> noLocals;
    if (containsExternalScalarReference(*plan.endExpr, plan.init.var, noLocals)) {
        plan.rejectReason = "induction variable in loop endpoint";
        return false;
    }
    std::unordered_set<std::string> writes;
    for (const Node *stmt : plan.body) {
        collectExternalScalarWrites(*stmt, lexical.localLvals, writes);
    }
    if (boundReferencesWrittenScalar(*plan.endExpr, writes)) {
        plan.rejectReason = "loop endpoint is modified in body";
        return false;
    }
    if (!exprIsProvablyInt(*plan.endExpr, lookupType)) {
        plan.rejectReason = "non-int loop endpoint";
        return false;
    }
    if (plan.inclusiveEnd) {
        int endpoint = 0;
        if (intConstValue(*plan.endExpr, endpoint) && endpoint == INT_MAX) {
            plan.rejectReason = "inclusive endpoint may overflow";
            return false;
        }
    }
    return true;
}

std::optional<int> constantTripCount(const ParallelLoopPlan &plan) {
    if (plan.init.initExpr == nullptr || plan.endExpr == nullptr) {
        return std::nullopt;
    }
    int init = 0;
    int end = 0;
    if (!intConstValue(*plan.init.initExpr, init) || !intConstValue(*plan.endExpr, end)) {
        return std::nullopt;
    }
    long long count = static_cast<long long>(end) - static_cast<long long>(init);
    if (plan.inclusiveEnd) {
        ++count;
    }
    if (count < 0) {
        count = 0;
    }
    if (count > INT_MAX) {
        count = INT_MAX;
    }
    return static_cast<int>(count);
}

bool profitableParallelLoop(const ParallelLoopPlan &plan) {
    if (!plan.valid) {
        return false;
    }
    constexpr int kNativeParallelThreadThreshold = 512;
    if (std::optional<int> tripCount = constantTripCount(plan);
        tripCount.has_value() && *tripCount < kNativeParallelThreadThreshold) {
        return false;
    }
    if (!plan.reductions.empty()) {
        return true;
    }
    if (plan.hasNestedLoop) {
        return true;
    }
    return plan.estimatedCost >= 48;
}

} // namespace

std::string firstWord(const std::string &text) {
    std::size_t pos = text.find(' ');
    return pos == std::string::npos ? text : text.substr(0, pos);
}

std::string secondWord(const std::string &text) {
    std::size_t pos = text.find(' ');
    if (pos == std::string::npos) {
        return {};
    }
    std::size_t begin = text.find_first_not_of(' ', pos);
    if (begin == std::string::npos) {
        return {};
    }
    std::size_t end = text.find(' ', begin);
    return text.substr(begin, end == std::string::npos ? std::string::npos : end - begin);
}

std::string jsonEscape(const std::string &text) {
    std::ostringstream out;
    for (char ch : text) {
        switch (ch) {
        case '\\':
            out << "\\\\";
            break;
        case '"':
            out << "\\\"";
            break;
        case '\n':
            out << "\\n";
            break;
        case '\r':
            out << "\\r";
            break;
        case '\t':
            out << "\\t";
            break;
        default:
            out << ch;
            break;
        }
    }
    return out.str();
}

class ParallelPlanJsonDumper {
public:
    std::string dump(const Node &root) {
        scopes_.clear();
        nextId_ = 0;
        loopDepth_ = 0;
        out_.str("");
        out_.clear();
        pushScope();
        visitRoot(root);
        popScope();
        return out_.str();
    }

private:
    void pushScope() {
        scopes_.push_back({});
    }

    void popScope() {
        scopes_.pop_back();
    }

    void declare(const std::string &name, const std::string &type) {
        if (!scopes_.empty() && !name.empty()) {
            scopes_.back()[name] = type;
        }
    }

    std::string lookupType(const std::string &name) const {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return found->second;
            }
        }
        return {};
    }

    void declareDecl(const Node &node) {
        if (node.kind != NodeKind::VarDecl && node.kind != NodeKind::ConstDecl) {
            return;
        }
        for (const auto &def : node.children) {
            declare(def->text, node.text);
        }
    }

    void visitRoot(const Node &node) {
        for (const auto &child : node.children) {
            if (child->kind == NodeKind::FuncDef) {
                visitFunction(*child);
            } else {
                declareDecl(*child);
            }
        }
    }

    void visitFunction(const Node &node) {
        pushScope();
        for (const auto &child : node.children) {
            if (child->kind == NodeKind::FuncParam) {
                declare(secondWord(child->text), firstWord(child->text));
            }
        }
        for (const auto &child : node.children) {
            if (child->kind == NodeKind::Block) {
                visitBlock(*child);
            }
        }
        popScope();
    }

    void visitBlock(const Node &node) {
        pushScope();
        for (std::size_t i = 0; i < node.children.size(); ++i) {
            if (i + 1 < node.children.size() &&
                parseParallelLoopInit(*node.children[i]).valid &&
                node.children[i + 1]->kind == NodeKind::WhileStmt) {
                ParallelLoopPlan plan = analyzeParallelLoopPair(
                    *node.children[i], *node.children[i + 1],
                    [this](const std::string &name) { return lookupType(name); });
                emitPlan(plan, node.children[i]->loc, node.children[i + 1]->loc);
            }
            visitStmt(*node.children[i]);
        }
        popScope();
    }

    void visitStmt(const Node &node) {
        switch (node.kind) {
        case NodeKind::VarDecl:
        case NodeKind::ConstDecl:
            declareDecl(node);
            return;
        case NodeKind::Block:
            visitBlock(node);
            return;
        case NodeKind::IfStmt:
            if (node.children.size() > 1) {
                visitStmt(*node.children[1]);
            }
            if (node.children.size() > 2) {
                visitStmt(*node.children[2]);
            }
            return;
        case NodeKind::WhileStmt:
            if (node.children.size() > 1) {
                ++loopDepth_;
                visitStmt(*node.children[1]);
                --loopDepth_;
            }
            return;
        default:
            return;
        }
    }

    void emitPlan(const ParallelLoopPlan &plan, SourceLocation initLoc, SourceLocation loopLoc) {
        std::string loweringKind;
        std::string runtimeSymbol;
        if (plan.valid) {
            if (plan.reductions.empty()) {
                loweringKind = "parallel_for";
                runtimeSymbol = "__sysy_parallel_for_range";
            } else {
                loweringKind = "parallel_reduce_int";
                runtimeSymbol = "__sysy_parallel_reduce_int_range";
            }
        }
        out_ << "{\"id\":" << nextId_++
             << ",\"init_line\":" << initLoc.line
             << ",\"init_column\":" << initLoc.column
             << ",\"loop_line\":" << loopLoc.line
             << ",\"loop_column\":" << loopLoc.column
             << ",\"loop_depth\":" << loopDepth_
             << ",\"valid\":" << (plan.valid ? "true" : "false")
             << ",\"init_var\":\"" << jsonEscape(plan.init.var) << "\""
             << ",\"init_decl\":" << (plan.init.declaration ? "true" : "false")
             << ",\"init_type\":\"" << jsonEscape(plan.init.type) << "\""
             << ",\"inclusive_end\":" << (plan.inclusiveEnd ? "true" : "false")
             << ",\"estimated_cost\":" << plan.estimatedCost
             << ",\"has_nested_loop\":" << (plan.hasNestedLoop ? "true" : "false")
             << ",\"has_array_write\":" << (plan.hasArrayWrite ? "true" : "false")
             << ",\"body_stmts\":" << plan.body.size()
             << ",\"lowering_kind\":\"" << loweringKind << "\""
             << ",\"runtime_symbol\":\"" << runtimeSymbol << "\""
             << ",\"captures\":[";
        for (std::size_t i = 0; i < plan.captures.size(); ++i) {
            const ParallelCapture &capture = plan.captures[i];
            if (i != 0) {
                out_ << ",";
            }
            std::string access;
            if (capture.read) {
                access += "R";
            }
            if (capture.write) {
                access += "W";
            }
            out_ << "{\"name\":\"" << jsonEscape(capture.name)
                 << "\",\"type\":\"" << jsonEscape(capture.type)
                 << "\",\"array\":" << (capture.array ? "true" : "false")
                 << ",\"access\":\"" << access << "\"}";
        }
        out_ << "],\"reductions\":[";
        for (std::size_t i = 0; i < plan.reductions.size(); ++i) {
            if (i != 0) {
                out_ << ",";
            }
            out_ << "{\"var\":\"" << jsonEscape(plan.reductions[i].var) << "\"}";
        }
        out_ << "],\"privatized_scalars\":[";
        for (std::size_t i = 0; i < plan.privatizedScalars.size(); ++i) {
            if (i != 0) {
                out_ << ",";
            }
            const ParallelPrivatizedScalar &scalar = plan.privatizedScalars[i];
            out_ << "{\"name\":\"" << jsonEscape(scalar.var)
                 << "\",\"type\":\"" << jsonEscape(scalar.type)
                 << "\",\"kind\":\"canonical_nested_iv\""
                 << ",\"inclusive_end\":"
                 << (scalar.inclusiveEnd ? "true" : "false") << "}";
        }
        out_ << "],\"reject\":\"" << jsonEscape(plan.rejectReason) << "\"}\n";
    }

    std::vector<std::unordered_map<std::string, std::string>> scopes_;
    int nextId_ = 0;
    int loopDepth_ = 0;
    std::ostringstream out_;
};

ParallelLoopInit parseParallelLoopInit(const Node &node) {
    ParallelLoopInit init;
    if (node.kind == NodeKind::AssignStmt && node.children.size() == 2 &&
        isScalarLVal(*node.children.at(0))) {
        init.valid = true;
        init.var = node.children.at(0)->text;
        init.initExpr = node.children.at(1).get();
        return init;
    }
    if (node.kind == NodeKind::VarDecl && node.children.size() == 1 &&
        isScalarDef(*node.children.at(0))) {
        const Node *initExpr = initializerOf(*node.children.at(0));
        if (initExpr != nullptr) {
            init.valid = true;
            init.declaration = true;
            init.type = node.text;
            init.var = node.children.at(0)->text;
            init.initExpr = initExpr;
        }
    }
    return init;
}

const Node *parallelReductionAddend(const Node &assign, const std::string &var) {
    if (assign.kind != NodeKind::AssignStmt || assign.children.size() != 2) {
        return nullptr;
    }
    const Node &lhs = *assign.children.at(0);
    const Node &rhs = *assign.children.at(1);
    if (!isScalarLVal(lhs) || lhs.text != var || rhs.kind != NodeKind::BinaryExpr ||
        rhs.text != "+") {
        return nullptr;
    }
    const Node *left = rhs.children.at(0).get();
    const Node *right = rhs.children.at(1).get();
    if (left->kind == NodeKind::LVal && left->text == var && left->children.empty()) {
        return right;
    }
    if (right->kind == NodeKind::LVal && right->text == var && right->children.empty()) {
        return left;
    }
    return nullptr;
}

ParallelLoopPlan analyzeParallelLoopPair(const Node &initStmt, const Node &loopStmt,
                                         const ParallelTypeLookup &lookupType) {
    ParallelLoopPlan plan;
    plan.init = parseParallelLoopInit(initStmt);
    if (!plan.init.valid) {
        plan.rejectReason = "not a canonical loop initializer";
        return plan;
    }
    if (!canonicalWhile(loopStmt, plan.init, plan.endExpr, plan.inclusiveEnd, plan.body)) {
        plan.rejectReason = "not a canonical while loop";
        return plan;
    }

    plan.valid = true;
    LexicalInfo lexical = buildLexicalInfo(plan.body);
    if (lexical.bodyScopeNames.find(plan.init.var) != lexical.bodyScopeNames.end()) {
        plan.valid = false;
        plan.rejectReason = "loop increment is shadowed";
        return plan;
    }
    if (std::optional<ParallelPrivatizedScalar> scratch =
            recognizeCanonicalScratchScalar(plan, lookupType)) {
        plan.privatizedScalars.push_back(*scratch);
        for (const Node *stmt : plan.body) {
            markPrivatizedScalarLvals(*stmt, scratch->var, lexical.localLvals);
        }
    }
    for (const Node *stmt : plan.body) {
        plan.estimatedCost += estimateStmtCost(*stmt);
        plan.hasNestedLoop = plan.hasNestedLoop || containsWhile(*stmt);
    }
    if (!validateLoopBound(plan, lexical, lookupType)) {
        plan.valid = false;
        return plan;
    }
    for (const Node *stmt : plan.body) {
        if (!analyzeNode(*stmt, plan.init.var, lexical.localLvals, lookupType, plan)) {
            plan.valid = false;
            return plan;
        }
    }
    for (const ParallelReduction &reduction : plan.reductions) {
        for (const Node *stmt : plan.body) {
            if (!validateReductionUses(*stmt, reduction.var, lexical.localLvals)) {
                plan.valid = false;
                plan.rejectReason = "reduction variable has non-reduction use";
                return plan;
            }
        }
    }
    for (const Node *stmt : plan.body) {
        collectCapturesFromStmt(*stmt, plan.init.var, lexical.localLvals, lookupType, plan);
    }

    std::vector<ArrayAccess> reads;
    std::vector<ArrayAccess> writes;
    for (const Node *stmt : plan.body) {
        collectArrayReads(*stmt, reads);
        collectArrayWrites(*stmt, writes);
    }
    for (const ArrayAccess &write : writes) {
        for (const ArrayAccess &otherWrite : writes) {
            if (&write == &otherWrite || write.base != otherWrite.base ||
                write.lval == nullptr || otherWrite.lval == nullptr) {
                continue;
            }
            if (!sameFirstPartitionIndex(*write.lval, *otherWrite.lval, plan.init.var,
                                         lexical.localLvals)) {
                plan.valid = false;
                plan.rejectReason = "same-array write partitions overlap";
                return plan;
            }
        }
        for (const ArrayAccess &read : reads) {
            if (write.base == read.base && read.lval != nullptr &&
                !sameFirstPartitionIndex(*write.lval, *read.lval, plan.init.var,
                                         lexical.localLvals)) {
                plan.valid = false;
                plan.rejectReason = "loop-carried array dependence";
                return plan;
            }
        }
    }
    if (!plan.hasArrayWrite && plan.reductions.empty()) {
        plan.valid = false;
        plan.rejectReason = "no parallel side effect";
        return plan;
    }
    if (plan.reductions.size() > 1) {
        plan.valid = false;
        plan.rejectReason = "multiple reductions";
        return plan;
    }
    if (!plan.privatizedScalars.empty() &&
        (plan.reductions.size() != 1 || plan.hasArrayWrite ||
         plan.reductions.front().var == plan.privatizedScalars.front().var)) {
        plan.valid = false;
        plan.rejectReason = "scratch scalar privatization requires a pure reduction";
        return plan;
    }
    if (!profitableParallelLoop(plan)) {
        plan.valid = false;
        plan.rejectReason = "not profitable";
    }
    return plan;
}

std::string dumpParallelPlansJsonl(const Node &root) {
    ParallelPlanJsonDumper dumper;
    return dumper.dump(root);
}

} // namespace sysy
