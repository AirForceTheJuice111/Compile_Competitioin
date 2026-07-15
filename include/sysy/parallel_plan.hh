#pragma once

#include "ast.hh"

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
    enum class Kind {
        Add,
        Min,
        Max,
    };

    std::string var;
    const Node *addend = nullptr;
    // A modular reduction is the exact recurrence
    //   var = (var + addend) % modulus
    // with a positive, sufficiently small constant modulus.  Lowering proves
    // the initial value/addends safe at runtime and otherwise falls back.
    bool modular = false;
    int modulus = 0;
    Kind kind = Kind::Add;
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
    // Source induction update and comparison.  The original canonical form
    // uses step == 1 and comparison "<"/"<=".  Other forms are admitted only
    // when the planner can prove a finite, overflow-free constant iteration
    // space, represented as the logical half-open range [0, logicalTripCount).
    int step = 1;
    std::string comparison = "<";
    int logicalTripCount = -1;
    // Non-unit/decrement/!= loops with dynamic endpoints use a runtime
    // 64-bit trip-count proof.  A negative proof result takes the original
    // sequential loop, preserving wrapping or non-terminating source cases.
    bool dynamicLogicalRange = false;
    int initialIv = 0;
    int finalIv = 0;
    std::vector<const Node *> body;
    // Candidate-loop continues are admitted only in the source shape
    //
    //   i = i + step;
    //   continue;
    //
    // where the assignment is the immediately preceding sibling and refers
    // to the unshadowed canonical induction variable.  Lowering suppresses
    // these source assignments and routes their continues through the same
    // latch as ordinary fallthrough, so both the source and logical IVs move
    // exactly once.
    std::vector<const Node *> canonicalContinueUpdates;
    bool hasArrayWrite = false;
    bool hasNestedLoop = false;
    int estimatedCost = 0;
    int runtimeWorkCost = 1;
    // Plain integer additions may contain more than one independent
    // reduction.  Modular reductions remain single-variable only because
    // their guarded retry protocol has a scalar sentinel/result ABI.
    std::vector<ParallelReduction> reductions;
    std::vector<ParallelPrivatizedScalar> privatizedScalars;
    std::vector<ParallelCapture> captures;
    std::string rejectReason;
};

using ParallelTypeLookup = std::function<std::string(const std::string &)>;
using ParallelConstIntLookup =
    std::function<std::optional<int>(const std::string &)>;

struct ParallelScalarFunctionSummary {
    bool pure = false;
    std::string returnType;
    std::vector<std::string> parameters;
    std::unordered_set<std::string> globalScalarReads;
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
                                         const ParallelFunctionSummaryLookup &lookupFunction = {},
                                         const ParallelConstIntLookup &lookupConstInt = {});
std::string dumpParallelPlansJsonl(const Node &root);

} // namespace sysy
