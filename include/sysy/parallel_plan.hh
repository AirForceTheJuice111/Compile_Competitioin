#pragma once

#include "ast.hh"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace sysy {

struct ParallelLoopInit {
    bool valid = false;
    bool declaration = false;
    std::string type;
    std::string var;
    const Node *initExpr = nullptr;
};

struct ParallelReduction {
    std::string var;
    const Node *addend = nullptr;
};

struct ParallelCapture {
    std::string name;
    std::string type;
    bool array = false;
    bool read = false;
    bool write = false;
};

struct ParallelPrivatizedScalar {
    std::string var;
    std::string type;
    const Node *initExpr = nullptr;
    const Node *endExpr = nullptr;
    bool inclusiveEnd = false;
};

struct ParallelLoopPlan {
    bool valid = false;
    ParallelLoopInit init;
    const Node *endExpr = nullptr;
    bool inclusiveEnd = false;
    std::vector<const Node *> body;
    bool hasArrayWrite = false;
    bool hasNestedLoop = false;
    int estimatedCost = 0;
    int runtimeWorkCost = 1;
    std::vector<ParallelReduction> reductions;
    std::vector<ParallelPrivatizedScalar> privatizedScalars;
    std::vector<ParallelCapture> captures;
    std::string rejectReason;
};

using ParallelTypeLookup = std::function<std::string(const std::string &)>;

struct ParallelScalarFunctionSummary {
    bool pure = false;
    std::string returnType;
    std::vector<std::string> parameters;
    // Present only for a direct, single-expression return.  The expression is
    // owned by the source AST and can be substituted into affine index checks.
    const Node *affineReturnExpr = nullptr;
};

using ParallelFunctionSummaries =
    std::unordered_map<std::string, ParallelScalarFunctionSummary>;
using ParallelFunctionSummaryLookup =
    std::function<const ParallelScalarFunctionSummary *(const std::string &)>;

ParallelLoopInit parseParallelLoopInit(const Node &node);
const Node *parallelReductionAddend(const Node &assign, const std::string &var);
ParallelFunctionSummaries summarizeParallelScalarFunctions(const Node &root);
ParallelLoopPlan analyzeParallelLoopPair(const Node &initStmt, const Node &loopStmt,
                                         const ParallelTypeLookup &lookupType,
                                         const ParallelFunctionSummaryLookup &lookupFunction = {});
std::string dumpParallelPlansJsonl(const Node &root);

} // namespace sysy
