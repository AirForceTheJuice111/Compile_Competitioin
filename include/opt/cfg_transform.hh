#ifndef QUAD_OPT_CFG_TRANSFORM_HH
#define QUAD_OPT_CFG_TRANSFORM_HH

#include <set>
#include <string>
#include <utility>
#include <vector>

#include "flowinfo.hh"
#include "quad.hh"

namespace quad {

// Canonical description of one reducible natural loop. Loops that share a
// header are merged so consumers do not have to reason about individual back
// edges independently.
struct NaturalLoopInfo {
    int header = -1;
    std::set<int> blocks;
    std::set<int> outsidePredecessors;
    std::set<int> backedgePredecessors;
    std::set<std::pair<int, int>> exitEdges;
};

ControlFlowInfo *buildControlFlowInfo(QuadFuncDecl *func);
QuadBlock *findQuadBlock(QuadFuncDecl *func, int label);
std::vector<NaturalLoopInfo> findNaturalLoopInfo(QuadFuncDecl *func,
                                                 ControlFlowInfo *cfi);

// Retarget a real CFG edge in both the terminator and the cached block exit
// labels. Phi nodes are intentionally handled separately by the caller.
bool retargetQuadEdge(QuadFuncDecl *func, int predecessor, int oldTarget,
                      int newTarget);

// Replace a phi predecessor label without changing the incoming SSA value.
bool replacePhiPredecessor(QuadBlock *target, int oldPredecessor,
                           int newPredecessor);

// Validate block/terminator/phi structure and then the existing SSA dominance
// invariants. Callers should rebuild metadata before invoking this routine.
bool verifyQuadSsaCfg(QuadFuncDecl *func, std::string *reason = nullptr);

} // namespace quad

#endif
