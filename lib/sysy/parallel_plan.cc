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

const ParallelScalarFunctionSummary *lookupPureFunction(
    const ParallelFunctionSummaryLookup &lookupFunction, const std::string &name) {
    if (!lookupFunction) {
        return nullptr;
    }
    const ParallelScalarFunctionSummary *summary = lookupFunction(name);
    return summary != nullptr && summary->pure ? summary : nullptr;
}

bool exprContainsUnsafeCall(const Node &node,
                            const ParallelFunctionSummaryLookup &lookupFunction) {
    if (node.kind == NodeKind::CallExpr &&
        lookupPureFunction(lookupFunction, node.text) == nullptr) {
        return true;
    }
    for (const auto &child : node.children) {
        if (exprContainsUnsafeCall(*child, lookupFunction)) {
            return true;
        }
    }
    return false;
}

bool exprContainsAnyCall(const Node &node) {
    if (node.kind == NodeKind::CallExpr) {
        return true;
    }
    return std::any_of(node.children.begin(), node.children.end(), [](const auto &child) {
        return exprContainsAnyCall(*child);
    });
}

bool callReadsWrittenScalar(
    const Node &node, const std::unordered_set<std::string> &writes,
    const ParallelFunctionSummaryLookup &lookupFunction) {
    if (node.kind == NodeKind::CallExpr) {
        const ParallelScalarFunctionSummary *summary =
            lookupPureFunction(lookupFunction, node.text);
        if (summary == nullptr) {
            return true;
        }
        for (const std::string &read : summary->globalScalarReads) {
            if (writes.find(read) != writes.end()) {
                return true;
            }
        }
    }
    return std::any_of(node.children.begin(), node.children.end(), [&](const auto &child) {
        return callReadsWrittenScalar(*child, writes, lookupFunction);
    });
}

bool isStartTimingStmt(const Node &node) {
    return node.kind == NodeKind::ExprStmt && node.children.size() == 1 &&
           node.children.front()->kind == NodeKind::CallExpr &&
           (node.children.front()->text == "starttime" ||
            node.children.front()->text == "_sysy_starttime");
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

int saturatingWorkAdd(int lhs, int rhs) {
    long long value = static_cast<long long>(lhs) + static_cast<long long>(rhs);
    return value > INT_MAX ? INT_MAX : static_cast<int>(value);
}

int saturatingWorkMultiply(int value, int multiplier) {
    long long product = static_cast<long long>(value) * multiplier;
    return product > INT_MAX ? INT_MAX : static_cast<int>(product);
}

int estimateRuntimeStmtCost(const Node &node) {
    switch (node.kind) {
    case NodeKind::AssignStmt:
    case NodeKind::VarDecl:
    case NodeKind::ConstDecl:
        return std::max(1, estimateStmtCost(node));
    case NodeKind::IfStmt: {
        int cost = 8 + estimateExprCost(*node.children.at(0));
        for (std::size_t i = 1; i < node.children.size(); ++i) {
            cost = saturatingWorkAdd(cost, estimateRuntimeStmtCost(*node.children[i]));
        }
        return std::max(1, cost);
    }
    case NodeKind::WhileStmt: {
        // Dynamic nested bounds are unknown at compile time.  Eight iterations
        // is a conservative profitability weight: it exposes genuinely heavy
        // outer iterations without claiming the full dynamic trip count.
        int bodyCost = node.children.size() > 1
                           ? estimateRuntimeStmtCost(*node.children.at(1))
                           : 1;
        int loopCost = 64 + estimateExprCost(*node.children.at(0));
        return saturatingWorkAdd(loopCost, saturatingWorkMultiply(bodyCost, 8));
    }
    default: {
        int cost = 1;
        for (const auto &child : node.children) {
            cost = saturatingWorkAdd(cost, estimateRuntimeStmtCost(*child));
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

struct AffineSubstitutions {
    const AffineSubstitutions *parent = nullptr;
    std::unordered_map<std::string, const Node *> arguments;
};

const Node *substitutedNode(const Node &node, const AffineSubstitutions *substitutions,
                            const AffineSubstitutions *&parent) {
    if (node.kind == NodeKind::LVal && node.children.empty() && substitutions != nullptr) {
        auto found = substitutions->arguments.find(node.text);
        if (found != substitutions->arguments.end()) {
            parent = substitutions->parent;
            return found->second;
        }
    }
    parent = substitutions;
    return &node;
}

bool affineIntConstValue(const Node &node, const AffineSubstitutions *substitutions,
                         int &value) {
    const AffineSubstitutions *resolvedSubstitutions = substitutions;
    const Node *resolved = substitutedNode(node, substitutions, resolvedSubstitutions);
    if (resolved != &node) {
        return affineIntConstValue(*resolved, resolvedSubstitutions, value);
    }
    return intConstValue(node, value);
}

bool safeExprReferencesLoopVar(
    const Node &node, const std::string &var,
    const std::unordered_set<const Node *> &localLvals,
    const ParallelFunctionSummaryLookup &lookupFunction,
    const AffineSubstitutions *substitutions, bool &references) {
    const AffineSubstitutions *resolvedSubstitutions = substitutions;
    const Node *resolved = substitutedNode(node, substitutions, resolvedSubstitutions);
    if (resolved != &node) {
        return safeExprReferencesLoopVar(*resolved, var, localLvals, lookupFunction,
                                         resolvedSubstitutions, references);
    }
    if (node.kind == NodeKind::CallExpr &&
        lookupPureFunction(lookupFunction, node.text) == nullptr) {
        return false;
    }
    if (node.kind == NodeKind::LVal && node.children.empty() && node.text == var &&
        localLvals.find(&node) == localLvals.end()) {
        references = true;
    }
    for (const auto &child : node.children) {
        if (!safeExprReferencesLoopVar(*child, var, localLvals, lookupFunction,
                                       substitutions, references)) {
            return false;
        }
    }
    return true;
}

bool affineCoeffImpl(
    const Node &node, const std::string &var,
    const std::unordered_set<const Node *> &localLvals,
    const ParallelFunctionSummaryLookup &lookupFunction,
    const AffineSubstitutions *substitutions,
    std::unordered_set<std::string> &activeFunctions, int &coeff) {
    const AffineSubstitutions *resolvedSubstitutions = substitutions;
    const Node *resolved = substitutedNode(node, substitutions, resolvedSubstitutions);
    if (resolved != &node) {
        return affineCoeffImpl(*resolved, var, localLvals, lookupFunction,
                               resolvedSubstitutions, activeFunctions, coeff);
    }

    bool referencesLoopVar = false;
    if (!safeExprReferencesLoopVar(node, var, localLvals, lookupFunction,
                                   substitutions, referencesLoopVar)) {
        return false;
    }
    if (!referencesLoopVar) {
        coeff = 0;
        return true;
    }
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
        if (!affineCoeffImpl(*node.children.at(0), var, localLvals, lookupFunction,
                             substitutions, activeFunctions, inner)) {
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
    if (node.kind == NodeKind::CallExpr) {
        const ParallelScalarFunctionSummary *summary =
            lookupPureFunction(lookupFunction, node.text);
        if (summary == nullptr || summary->affineReturnExpr == nullptr ||
            summary->parameters.size() != node.children.size() ||
            activeFunctions.find(node.text) != activeFunctions.end()) {
            return false;
        }
        AffineSubstitutions callSubstitutions;
        callSubstitutions.parent = substitutions;
        for (std::size_t i = 0; i < summary->parameters.size(); ++i) {
            callSubstitutions.arguments.emplace(summary->parameters[i],
                                                node.children[i].get());
        }
        activeFunctions.insert(node.text);
        bool affine = affineCoeffImpl(*summary->affineReturnExpr, var, localLvals,
                                      lookupFunction, &callSubstitutions,
                                      activeFunctions, coeff);
        activeFunctions.erase(node.text);
        return affine;
    }
    if (node.kind != NodeKind::BinaryExpr) {
        return false;
    }
    int left = 0;
    int right = 0;
    if (node.text == "+" || node.text == "-") {
        if (!affineCoeffImpl(*node.children.at(0), var, localLvals, lookupFunction,
                             substitutions, activeFunctions, left) ||
            !affineCoeffImpl(*node.children.at(1), var, localLvals, lookupFunction,
                             substitutions, activeFunctions, right)) {
            return false;
        }
        coeff = node.text == "+" ? left + right : left - right;
        return true;
    }
    if (node.text == "*") {
        int c = 0;
        if (affineIntConstValue(*node.children.at(0), substitutions, c) &&
            affineCoeffImpl(*node.children.at(1), var, localLvals, lookupFunction,
                             substitutions, activeFunctions, right)) {
            coeff = c * right;
            return true;
        }
        if (affineIntConstValue(*node.children.at(1), substitutions, c) &&
            affineCoeffImpl(*node.children.at(0), var, localLvals, lookupFunction,
                             substitutions, activeFunctions, left)) {
            coeff = c * left;
            return true;
        }
    }
    return false;
}

bool affineCoeff(const Node &node, const std::string &var,
                 const std::unordered_set<const Node *> &localLvals,
                 const ParallelFunctionSummaryLookup &lookupFunction, int &coeff) {
    std::unordered_set<std::string> activeFunctions;
    return affineCoeffImpl(node, var, localLvals, lookupFunction, nullptr,
                           activeFunctions, coeff);
}

bool lvalFirstIndexIsPartitionedByLoopVar(
    const Node &lval, const std::string &var,
    const std::unordered_set<const Node *> &localLvals,
    const ParallelFunctionSummaryLookup &lookupFunction) {
    if (lval.kind != NodeKind::LVal || lval.children.empty()) {
        return false;
    }
    int coeff = 0;
    return affineCoeff(*lval.children.front(), var, localLvals, lookupFunction, coeff) &&
           coeff != 0;
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
                             const std::unordered_set<const Node *> &localLvals,
                             const ParallelFunctionSummaryLookup &lookupFunction) {
    if (lhs.kind != NodeKind::LVal || rhs.kind != NodeKind::LVal ||
        lhs.children.empty() || rhs.children.empty()) {
        return false;
    }
    int lhsCoeff = 0;
    int rhsCoeff = 0;
    if (!affineCoeff(*lhs.children.front(), var, localLvals, lookupFunction, lhsCoeff) ||
        !affineCoeff(*rhs.children.front(), var, localLvals, lookupFunction, rhsCoeff) ||
        lhsCoeff == 0 || rhsCoeff == 0) {
        return false;
    }
    return nodeKey(*lhs.children.front()) == nodeKey(*rhs.children.front());
}

bool signedIntConstValue(const Node &node, int &value) {
    if (intConstValue(node, value)) {
        return true;
    }
    if (node.kind != NodeKind::UnaryExpr || node.children.size() != 1 ||
        (node.text != "+" && node.text != "-")) {
        return false;
    }
    int inner = 0;
    if (!intConstValue(*node.children.front(), inner) ||
        (node.text == "-" && inner == INT_MIN)) {
        return false;
    }
    value = node.text == "-" ? -inner : inner;
    return true;
}

bool inductionStepOf(const Node &stmt, const std::string &var, int &step) {
    if (stmt.kind != NodeKind::AssignStmt || stmt.children.size() != 2) {
        return false;
    }
    const Node &lhs = *stmt.children.at(0);
    const Node &rhs = *stmt.children.at(1);
    if (!isScalarLVal(lhs) || lhs.text != var || rhs.kind != NodeKind::BinaryExpr) {
        return false;
    }
    int c = 0;
    if (rhs.text == "+") {
        if (rhs.children.at(0)->kind == NodeKind::LVal &&
            rhs.children.at(0)->text == var && rhs.children.at(0)->children.empty() &&
            signedIntConstValue(*rhs.children.at(1), c)) {
            step = c;
        } else if (rhs.children.at(1)->kind == NodeKind::LVal &&
                   rhs.children.at(1)->text == var && rhs.children.at(1)->children.empty() &&
                   signedIntConstValue(*rhs.children.at(0), c)) {
            step = c;
        } else {
            return false;
        }
    } else if (rhs.text == "-" &&
               rhs.children.at(0)->kind == NodeKind::LVal &&
               rhs.children.at(0)->text == var && rhs.children.at(0)->children.empty() &&
               signedIntConstValue(*rhs.children.at(1), c) && c != INT_MIN) {
        step = -c;
    } else {
        return false;
    }
    return step != 0;
}

bool canonicalWhile(const Node &loop, const ParallelLoopInit &init, const Node *&endExpr,
                    bool &inclusiveEnd, int &step, std::string &comparison,
                    std::vector<const Node *> &body) {
    if (!init.valid || loop.kind != NodeKind::WhileStmt || loop.children.size() != 2) {
        return false;
    }
    const Node &cond = *loop.children.at(0);
    if (cond.kind != NodeKind::BinaryExpr ||
        (cond.text != "<" && cond.text != "<=" && cond.text != ">" &&
         cond.text != ">=" && cond.text != "!=") ||
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
    if (!inductionStepOf(last, init.var, step)) {
        return false;
    }
    endExpr = cond.children.at(1).get();
    comparison = cond.text;
    inclusiveEnd = cond.text == "<=" || cond.text == ">=";
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
    int scratchBegin = 0;
    if (!intConstValue(*scratchInit.initExpr, scratchBegin)) {
        return std::nullopt;
    }

    const Node *scratchEnd = nullptr;
    bool scratchInclusive = false;
    int scratchStep = 1;
    std::string scratchComparison;
    std::vector<const Node *> scratchBody;
    if (!canonicalWhile(*plan.body.back(), scratchInit, scratchEnd,
                        scratchInclusive, scratchStep, scratchComparison, scratchBody) ||
        scratchEnd == nullptr || scratchStep != 1 ||
        (scratchComparison != "<" && scratchComparison != "<=")) {
        return std::nullopt;
    }

    int scratchEndValue = 0;
    if (!intConstValue(*scratchEnd, scratchEndValue) ||
        (scratchInclusive && scratchEndValue == INT_MAX)) {
        return std::nullopt;
    }
    long long scratchTripCount = static_cast<long long>(scratchEndValue) -
                                 static_cast<long long>(scratchBegin) +
                                 (scratchInclusive ? 1LL : 0LL);
    if (scratchTripCount < 0) {
        scratchTripCount = 0;
    }
    constexpr long long kNativeParallelThreadThreshold = 512;
    if (scratchTripCount >= kNativeParallelThreadThreshold) {
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

bool exprIsProvablyInt(const Node &node, const ParallelTypeLookup &lookupType,
                       const ParallelFunctionSummaryLookup &lookupFunction) {
    if (node.kind == NodeKind::Number) {
        int ignored = 0;
        return intConstValue(node, ignored);
    }
    if (node.kind == NodeKind::LVal) {
        return node.children.empty() && lookupType && lookupType(node.text) == "int";
    }
    if (node.kind == NodeKind::CallExpr) {
        const ParallelScalarFunctionSummary *summary =
            lookupPureFunction(lookupFunction, node.text);
        return summary != nullptr && summary->returnType == "int";
    }
    if (node.kind == NodeKind::UnaryExpr) {
        return node.children.size() == 1 &&
               exprIsProvablyInt(*node.children.front(), lookupType, lookupFunction);
    }
    if (node.kind == NodeKind::BinaryExpr) {
        return node.children.size() == 2 &&
               exprIsProvablyInt(*node.children.at(0), lookupType, lookupFunction) &&
               exprIsProvablyInt(*node.children.at(1), lookupType, lookupFunction);
    }
    return false;
}

std::optional<int> reductionConstInt(const Node &node,
                                     const ParallelConstIntLookup &lookupConstInt) {
    int literal = 0;
    if (intConstValue(node, literal)) {
        return literal;
    }
    if (node.kind == NodeKind::LVal && node.children.empty() && lookupConstInt) {
        return lookupConstInt(node.text);
    }
    return std::nullopt;
}

std::optional<ParallelReduction> modularReduction(
    const Node &assign, const std::string &var,
    const ParallelConstIntLookup &lookupConstInt) {
    if (assign.kind != NodeKind::AssignStmt || assign.children.size() != 2) {
        return std::nullopt;
    }
    const Node &lhs = *assign.children.at(0);
    const Node &rhs = *assign.children.at(1);
    if (!isScalarLVal(lhs) || lhs.text != var || rhs.kind != NodeKind::BinaryExpr ||
        rhs.text != "%" || rhs.children.size() != 2) {
        return std::nullopt;
    }
    std::optional<int> modulus =
        reductionConstInt(*rhs.children.at(1), lookupConstInt);
    // Keeping both signed residues in (-m,m) makes their sum overflow-free.
    // m <= INT_MAX/2 is deliberately a little stricter than necessary.
    if (!modulus || *modulus <= 0 || *modulus > INT_MAX / 2) {
        return std::nullopt;
    }
    const Node &sum = *rhs.children.at(0);
    if (sum.kind != NodeKind::BinaryExpr || sum.text != "+" ||
        sum.children.size() != 2) {
        return std::nullopt;
    }
    const Node *left = sum.children.at(0).get();
    const Node *right = sum.children.at(1).get();
    const Node *addend = nullptr;
    if (isScalarLVal(*left) && left->text == var) {
        addend = right;
    } else if (isScalarLVal(*right) && right->text == var) {
        addend = left;
    }
    if (addend == nullptr) {
        return std::nullopt;
    }
    return ParallelReduction{var, addend, true, *modulus};
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
                                          const std::unordered_set<const Node *> &localLvals,
                                          const ParallelFunctionSummaryLookup &lookupFunction) {
    std::vector<ArrayAccess> reads;
    collectArrayReads(rhs, reads);
    for (const ArrayAccess &read : reads) {
        if (read.base != writeLval.text) {
            continue;
        }
        if (read.lval == nullptr ||
            !sameFirstPartitionIndex(writeLval, *read.lval, loopVar, localLvals,
                                     lookupFunction)) {
            return false;
        }
    }
    return true;
}

bool analyzeNode(const Node &node, const std::string &loopVar,
                 const std::unordered_set<const Node *> &localLvals,
                 const ParallelTypeLookup &lookupType,
                 const ParallelFunctionSummaryLookup &lookupFunction,
                 const ParallelConstIntLookup &lookupConstInt,
                 ParallelLoopPlan &plan) {
    if (exprContainsUnsafeCall(node, lookupFunction)) {
        plan.rejectReason = "unsafe call in loop body";
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
                if (std::optional<ParallelReduction> reduction =
                        modularReduction(node, lhs.text, lookupConstInt)) {
                    if (!containsExternalScalarReference(*reduction->addend, lhs.text,
                                                         localLvals)) {
                        addReduction(plan.reductions, *reduction);
                        return true;
                    }
                }
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
        if (!lvalFirstIndexIsPartitionedByLoopVar(lhs, loopVar, localLvals,
                                                  lookupFunction)) {
            plan.rejectReason = "non-affine array write";
            return false;
        }
        if (!sameArrayReadsStayInWrittenPartition(lhs, *node.children.at(1), loopVar,
                                                  localLvals, lookupFunction)) {
            plan.rejectReason = "same-array read is not partitioned";
            return false;
        }
        plan.hasArrayWrite = true;
        return true;
    }
    default:
        for (const auto &child : node.children) {
            if (!analyzeNode(*child, loopVar, localLvals, lookupType,
                             lookupFunction, lookupConstInt, plan)) {
                return false;
            }
        }
        return true;
    }
}

bool validateReductionUses(
    const Node &node, const ParallelReduction &reduction,
    const std::unordered_set<const Node *> &localLvals,
    const ParallelConstIntLookup &lookupConstInt) {
    if (node.kind == NodeKind::AssignStmt && node.children.size() == 2) {
        const Node &lhs = *node.children.at(0);
        if (isScalarLVal(lhs) && lhs.text == reduction.var &&
            localLvals.find(&lhs) == localLvals.end()) {
            const Node *addend = nullptr;
            if (reduction.modular) {
                std::optional<ParallelReduction> current =
                    modularReduction(node, reduction.var, lookupConstInt);
                if (current && current->modulus == reduction.modulus) {
                    addend = current->addend;
                }
            } else {
                addend = parallelReductionAddend(node, reduction.var);
            }
            return addend != nullptr &&
                   !containsExternalScalarReference(*addend, reduction.var, localLvals);
        }
    }
    if (node.kind == NodeKind::LVal && node.children.empty() &&
        node.text == reduction.var &&
        localLvals.find(&node) == localLvals.end()) {
        return false;
    }
    return std::all_of(node.children.begin(), node.children.end(), [&](const auto &child) {
        return validateReductionUses(*child, reduction, localLvals, lookupConstInt);
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
                       const ParallelTypeLookup &lookupType,
                       const ParallelFunctionSummaryLookup &lookupFunction) {
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
    if (exprContainsAnyCall(*plan.endExpr)) {
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
    if (!exprIsProvablyInt(*plan.endExpr, lookupType, lookupFunction)) {
        plan.rejectReason = "non-int loop endpoint";
        return false;
    }
    if (plan.comparison == "<=") {
        int endpoint = 0;
        if (intConstValue(*plan.endExpr, endpoint) && endpoint == INT_MAX) {
            plan.rejectReason = "inclusive endpoint may overflow";
            return false;
        }
    }
    return true;
}

bool comparisonHolds(long long lhs, long long rhs, const std::string &comparison) {
    if (comparison == "<") return lhs < rhs;
    if (comparison == "<=") return lhs <= rhs;
    if (comparison == ">") return lhs > rhs;
    if (comparison == ">=") return lhs >= rhs;
    return comparison == "!=" && lhs != rhs;
}

bool deriveLogicalIterationSpace(ParallelLoopPlan &plan) {
    // Preserve the established dynamic unit-stride lowering.  It has runtime
    // overflow handling for <= and does not need a logical-IV mapping.
    if (plan.step == 1 && (plan.comparison == "<" || plan.comparison == "<=")) {
        return true;
    }

    int initial = 0;
    int endpoint = 0;
    if (plan.init.initExpr == nullptr || plan.endExpr == nullptr ||
        !signedIntConstValue(*plan.init.initExpr, initial) ||
        !signedIntConstValue(*plan.endExpr, endpoint)) {
        plan.rejectReason = "generalized loop needs constant endpoints";
        return false;
    }

    const long long begin = initial;
    const long long end = endpoint;
    long long trips = 0;
    if (!comparisonHolds(begin, end, plan.comparison)) {
        trips = 0;
    } else if (plan.comparison == "<" || plan.comparison == "<=") {
        if (plan.step <= 0) {
            plan.rejectReason = "loop step does not approach endpoint";
            return false;
        }
        const long long distance = end - begin;
        trips = plan.comparison == "<"
                    ? (distance + static_cast<long long>(plan.step) - 1) / plan.step
                    : distance / plan.step + 1;
    } else if (plan.comparison == ">" || plan.comparison == ">=") {
        if (plan.step >= 0) {
            plan.rejectReason = "loop step does not approach endpoint";
            return false;
        }
        const long long magnitude = -static_cast<long long>(plan.step);
        const long long distance = begin - end;
        trips = plan.comparison == ">"
                    ? (distance + magnitude - 1) / magnitude
                    : distance / magnitude + 1;
    } else {
        const long long distance = end - begin;
        if ((distance > 0 && plan.step <= 0) ||
            (distance < 0 && plan.step >= 0) ||
            distance % static_cast<long long>(plan.step) != 0) {
            plan.rejectReason = "!= endpoint is not reached exactly";
            return false;
        }
        trips = distance / static_cast<long long>(plan.step);
    }

    const long long offset = trips * static_cast<long long>(plan.step);
    const long long finalValue = begin + offset;
    if (trips < 0 || trips > INT_MAX || offset < -static_cast<long long>(INT_MAX) ||
        offset > INT_MAX || finalValue < INT_MIN || finalValue > INT_MAX ||
        comparisonHolds(finalValue, end, plan.comparison)) {
        plan.rejectReason = "generalized loop may overflow or not terminate";
        return false;
    }
    plan.logicalTripCount = static_cast<int>(trips);
    plan.initialIv = initial;
    plan.finalIv = static_cast<int>(finalValue);
    return true;
}

std::optional<int> constantTripCount(const ParallelLoopPlan &plan) {
    if (plan.logicalTripCount >= 0) {
        return plan.logicalTripCount;
    }
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
    constexpr long long kNativeParallelWorkThreshold = 16384;
    if (std::optional<int> tripCount = constantTripCount(plan);
        tripCount.has_value()) {
        long long work = static_cast<long long>(*tripCount) *
                         std::max(1, plan.runtimeWorkCost);
        if (work < kNativeParallelWorkThreshold) {
            return false;
        }
    }
    if (!plan.reductions.empty()) {
        return true;
    }
    if (plan.hasNestedLoop) {
        return true;
    }
    return plan.estimatedCost >= 48;
}

std::string summaryFirstWord(const std::string &text) {
    std::size_t pos = text.find(' ');
    return pos == std::string::npos ? text : text.substr(0, pos);
}

std::string summarySecondWord(const std::string &text) {
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

bool declarationIsArray(const Node &def) {
    return std::any_of(def.children.begin(), def.children.end(), [](const auto &child) {
        return child->kind == NodeKind::ArrayDim;
    });
}

struct GlobalPuritySymbol {
    bool immutableScalar = false;
};

struct FunctionPurityCandidate {
    ParallelScalarFunctionSummary summary;
    bool structurallyPure = true;
    std::unordered_set<std::string> callees;
};

bool directReturnIsAffineTemplate(const Node &node,
                                  const std::unordered_set<std::string> &parameters) {
    switch (node.kind) {
    case NodeKind::Number:
        return true;
    case NodeKind::LVal:
        return node.children.empty() && parameters.find(node.text) != parameters.end();
    case NodeKind::UnaryExpr:
    case NodeKind::BinaryExpr:
        return std::all_of(node.children.begin(), node.children.end(),
                           [&](const auto &child) {
                               return directReturnIsAffineTemplate(*child, parameters);
                           });
    default:
        return false;
    }
}

class FunctionPurityScanner {
public:
    explicit FunctionPurityScanner(
        const std::unordered_map<std::string, GlobalPuritySymbol> &globals)
        : globals_(globals) {}

    FunctionPurityCandidate scan(const Node &function) {
        candidate_ = {};
        candidate_.summary.returnType = summaryFirstWord(function.text);
        scopes_.clear();
        scopes_.push_back({});
        std::unordered_set<std::string> parameters;
        for (const auto &child : function.children) {
            if (child->kind != NodeKind::FuncParam) {
                continue;
            }
            std::string name = summarySecondWord(child->text);
            candidate_.summary.parameters.push_back(name);
            parameters.insert(name);
            scopes_.back().insert(name);
            if (declarationIsArray(*child)) {
                candidate_.structurallyPure = false;
            }
        }
        for (const auto &child : function.children) {
            if (child->kind == NodeKind::Block) {
                scanBlock(*child, false);
                if (child->children.size() == 1 &&
                    child->children.front()->kind == NodeKind::ReturnStmt &&
                    child->children.front()->children.size() == 1) {
                    const Node *returnExpr = child->children.front()->children.front().get();
                    if (directReturnIsAffineTemplate(*returnExpr, parameters)) {
                        candidate_.summary.affineReturnExpr = returnExpr;
                    }
                }
            }
        }
        return candidate_;
    }

private:
    bool local(const std::string &name) const {
        return std::any_of(scopes_.rbegin(), scopes_.rend(), [&](const auto &scope) {
            return scope.find(name) != scope.end();
        });
    }

    void scanBlock(const Node &block, bool scoped) {
        if (scoped) {
            scopes_.push_back({});
        }
        for (const auto &child : block.children) {
            scanNode(*child);
        }
        if (scoped) {
            scopes_.pop_back();
        }
    }

    void scanDeclaration(const Node &declaration) {
        for (const auto &def : declaration.children) {
            if (declarationIsArray(*def)) {
                candidate_.structurallyPure = false;
            } else {
                // Match frontend visibility: a scalar definition is in scope in
                // its own initializer and in later definitions.
                scopes_.back().insert(def->text);
            }
            for (const auto &child : def->children) {
                if (child->kind != NodeKind::ArrayDim) {
                    scanNode(*child);
                }
            }
        }
    }

    void scanNode(const Node &node) {
        if (node.kind == NodeKind::Block) {
            scanBlock(node, true);
            return;
        }
        if (node.kind == NodeKind::VarDecl || node.kind == NodeKind::ConstDecl) {
            scanDeclaration(node);
            return;
        }
        if (node.kind == NodeKind::AssignStmt && node.children.size() == 2) {
            const Node &lhs = *node.children.front();
            if (lhs.kind != NodeKind::LVal || !lhs.children.empty() || !local(lhs.text)) {
                candidate_.structurallyPure = false;
            }
            for (const auto &index : lhs.children) {
                scanNode(*index);
            }
            scanNode(*node.children.at(1));
            return;
        }
        if (node.kind == NodeKind::LVal) {
            if (!node.children.empty()) {
                candidate_.structurallyPure = false;
            } else if (!local(node.text)) {
                auto found = globals_.find(node.text);
                // Reading a scalar global has no call effect.  Such a value is
                // stable for the duration of a parallel region because the
                // planner separately rejects every non-reduction scalar write
                // in its body.  Array/global writes and unknown symbols remain
                // impure.
                if (found == globals_.end()) {
                    candidate_.structurallyPure = false;
                } else {
                    candidate_.summary.globalScalarReads.insert(node.text);
                }
            }
            for (const auto &index : node.children) {
                scanNode(*index);
            }
            return;
        }
        if (node.kind == NodeKind::CallExpr) {
            candidate_.callees.insert(node.text);
        }
        if (node.kind == NodeKind::StringLiteral) {
            candidate_.structurallyPure = false;
        }
        for (const auto &child : node.children) {
            scanNode(*child);
        }
    }

    const std::unordered_map<std::string, GlobalPuritySymbol> &globals_;
    std::vector<std::unordered_set<std::string>> scopes_;
    FunctionPurityCandidate candidate_;
};

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

ParallelFunctionSummaries summarizeParallelScalarFunctions(const Node &root) {
    std::unordered_map<std::string, GlobalPuritySymbol> globals;
    for (const auto &child : root.children) {
        if (child->kind != NodeKind::VarDecl && child->kind != NodeKind::ConstDecl) {
            continue;
        }
        for (const auto &def : child->children) {
            globals[def->text] = GlobalPuritySymbol{
                child->kind == NodeKind::ConstDecl && !declarationIsArray(*def)};
        }
    }

    std::unordered_map<std::string, FunctionPurityCandidate> candidates;
    FunctionPurityScanner scanner(globals);
    for (const auto &child : root.children) {
        if (child->kind != NodeKind::FuncDef) {
            continue;
        }
        std::string name = summarySecondWord(child->text);
        if (!name.empty()) {
            candidates[name] = scanner.scan(*child);
        }
    }

    ParallelFunctionSummaries summaries;
    for (const auto &entry : candidates) {
        ParallelScalarFunctionSummary summary = entry.second.summary;
        summary.pure = entry.second.structurallyPure;
        summaries.emplace(entry.first, std::move(summary));
    }

    // Start optimistic and remove functions that reach an impure or unknown
    // callee.  This greatest fixed point keeps closed recursive SCCs pure.
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto &entry : candidates) {
            auto summary = summaries.find(entry.first);
            if (summary == summaries.end() || !summary->second.pure) {
                continue;
            }
            bool callsOnlyPureUserFunctions =
                std::all_of(entry.second.callees.begin(), entry.second.callees.end(),
                            [&](const std::string &callee) {
                                auto found = summaries.find(callee);
                                return found != summaries.end() && found->second.pure;
                            });
            if (!callsOnlyPureUserFunctions) {
                summary->second.pure = false;
                changed = true;
            }
        }
    }
    // Carry transitive scalar reads through pure call chains and recursive
    // SCCs.  The loop planner uses this to reject a seemingly pure helper that
    // observes a scalar concurrently reduced by the loop.
    changed = true;
    while (changed) {
        changed = false;
        for (const auto &entry : candidates) {
            auto summary = summaries.find(entry.first);
            if (summary == summaries.end() || !summary->second.pure) {
                continue;
            }
            for (const std::string &callee : entry.second.callees) {
                auto calleeSummary = summaries.find(callee);
                if (calleeSummary == summaries.end() || !calleeSummary->second.pure) {
                    continue;
                }
                for (const std::string &read : calleeSummary->second.globalScalarReads) {
                    if (summary->second.globalScalarReads.insert(read).second) {
                        changed = true;
                    }
                }
            }
        }
    }
    return summaries;
}

class ParallelPlanJsonDumper {
public:
    std::string dump(const Node &root) {
        scopes_.clear();
        functionSummaries_ = summarizeParallelScalarFunctions(root);
        nextId_ = 0;
        loopDepth_ = 0;
        out_.str("");
        out_.clear();
        globalConstInts_.clear();
        for (const auto &child : root.children) {
            if (child->kind != NodeKind::ConstDecl || child->text != "int") {
                continue;
            }
            for (const auto &def : child->children) {
                if (!isScalarDef(*def)) {
                    continue;
                }
                const Node *init = initializerOf(*def);
                int value = 0;
                if (init != nullptr && intConstValue(*init, value)) {
                    globalConstInts_[def->text] = value;
                }
            }
        }
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
            std::size_t loopIndex = i + 1;
            if (i + 2 < node.children.size() &&
                isStartTimingStmt(*node.children[i + 1])) {
                loopIndex = i + 2;
            }
            if (loopIndex < node.children.size() &&
                parseParallelLoopInit(*node.children[i]).valid &&
                node.children[loopIndex]->kind == NodeKind::WhileStmt) {
                ParallelLoopPlan plan = analyzeParallelLoopPair(
                    *node.children[i], *node.children[loopIndex],
                    [this](const std::string &name) { return lookupType(name); },
                    [this](const std::string &name)
                        -> const ParallelScalarFunctionSummary * {
                        auto found = functionSummaries_.find(name);
                        return found == functionSummaries_.end() ? nullptr : &found->second;
                    },
                    [this](const std::string &name) -> std::optional<int> {
                        auto found = globalConstInts_.find(name);
                        return found == globalConstInts_.end()
                                   ? std::nullopt
                                   : std::optional<int>(found->second);
                    });
                emitPlan(plan, node.children[i]->loc, node.children[loopIndex]->loc);
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
            } else if (plan.reductions.front().modular) {
                loweringKind = "parallel_reduce_mod_int_dynamic";
                runtimeSymbol = "__sysy_parallel_reduce_mod_int_range";
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
             << ",\"comparison\":\"" << jsonEscape(plan.comparison) << "\""
             << ",\"step\":" << plan.step
             << ",\"logical_trip_count\":" << plan.logicalTripCount
             << ",\"estimated_cost\":" << plan.estimatedCost
             << ",\"runtime_work_cost\":" << plan.runtimeWorkCost
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
            out_ << "{\"var\":\"" << jsonEscape(plan.reductions[i].var)
                 << "\",\"modular\":"
                 << (plan.reductions[i].modular ? "true" : "false")
                 << ",\"modulus\":" << plan.reductions[i].modulus << "}";
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
    ParallelFunctionSummaries functionSummaries_;
    std::unordered_map<std::string, int> globalConstInts_;
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
                                         const ParallelTypeLookup &lookupType,
                                         const ParallelFunctionSummaryLookup &lookupFunction,
                                         const ParallelConstIntLookup &lookupConstInt) {
    ParallelLoopPlan plan;
    plan.init = parseParallelLoopInit(initStmt);
    if (!plan.init.valid) {
        plan.rejectReason = "not a canonical loop initializer";
        return plan;
    }
    if (!canonicalWhile(loopStmt, plan.init, plan.endExpr, plan.inclusiveEnd,
                        plan.step, plan.comparison, plan.body)) {
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
        plan.runtimeWorkCost = saturatingWorkAdd(
            plan.runtimeWorkCost, estimateRuntimeStmtCost(*stmt));
        plan.hasNestedLoop = plan.hasNestedLoop || containsWhile(*stmt);
    }
    if (!validateLoopBound(plan, lexical, lookupType, lookupFunction)) {
        plan.valid = false;
        return plan;
    }
    if (!deriveLogicalIterationSpace(plan)) {
        plan.valid = false;
        return plan;
    }
    for (const Node *stmt : plan.body) {
        if (!analyzeNode(*stmt, plan.init.var, lexical.localLvals, lookupType,
                         lookupFunction, lookupConstInt, plan)) {
            plan.valid = false;
            return plan;
        }
    }
    for (const ParallelReduction &reduction : plan.reductions) {
        for (const Node *stmt : plan.body) {
            if (!validateReductionUses(*stmt, reduction, lexical.localLvals,
                                       lookupConstInt)) {
                plan.valid = false;
                plan.rejectReason = "reduction variable has non-reduction use";
                return plan;
            }
        }
    }
    std::unordered_set<std::string> scalarWrites;
    for (const Node *stmt : plan.body) {
        collectExternalScalarWrites(*stmt, lexical.localLvals, scalarWrites);
    }
    for (const Node *stmt : plan.body) {
        if (callReadsWrittenScalar(*stmt, scalarWrites, lookupFunction)) {
            plan.valid = false;
            plan.rejectReason = "call reads loop-written scalar";
            return plan;
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
    if (!plan.reductions.empty() && plan.reductions.front().modular &&
        (!reads.empty() || !writes.empty())) {
        plan.valid = false;
        plan.rejectReason = "dynamic modular reduction requires an array-free body";
        return plan;
    }
    for (const ArrayAccess &write : writes) {
        for (const ArrayAccess &otherWrite : writes) {
            if (&write == &otherWrite || write.base != otherWrite.base ||
                write.lval == nullptr || otherWrite.lval == nullptr) {
                continue;
            }
            if (!sameFirstPartitionIndex(*write.lval, *otherWrite.lval, plan.init.var,
                                         lexical.localLvals, lookupFunction)) {
                plan.valid = false;
                plan.rejectReason = "same-array write partitions overlap";
                return plan;
            }
        }
        for (const ArrayAccess &read : reads) {
            if (write.base == read.base && read.lval != nullptr &&
                !sameFirstPartitionIndex(*write.lval, *read.lval, plan.init.var,
                                         lexical.localLvals, lookupFunction)) {
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
