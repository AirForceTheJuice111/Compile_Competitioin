#pragma once

#include "ast.hh"

#include <functional>
#include <string>
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
    std::vector<ParallelReduction> reductions;
    std::vector<ParallelPrivatizedScalar> privatizedScalars;
    std::vector<ParallelCapture> captures;
    std::string rejectReason;
};

using ParallelTypeLookup = std::function<std::string(const std::string &)>;

ParallelLoopInit parseParallelLoopInit(const Node &node);
const Node *parallelReductionAddend(const Node &assign, const std::string &var);
ParallelLoopPlan analyzeParallelLoopPair(const Node &initStmt, const Node &loopStmt,
                                         const ParallelTypeLookup &lookupType);
std::string dumpParallelPlansJsonl(const Node &root);

} // namespace sysy
