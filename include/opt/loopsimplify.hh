#ifndef QUAD_OPT_LOOP_SIMPLIFY_HH
#define QUAD_OPT_LOOP_SIMPLIFY_HH

#include "quad.hh"

namespace quad {

struct LoopSimplifyStats {
    int functionsVisited = 0;
    int functionsChanged = 0;
    int functionsSkipped = 0;
    int preheadersCreated = 0;
    int latchesCreated = 0;
    int exitEdgesSplit = 0;
};

// Canonicalize reducible SSA loops transactionally. Unsupported or invalid
// functions are returned unchanged.
QuadProgram *loopSimplifyProg(QuadProgram *program,
                              LoopSimplifyStats *stats = nullptr);

} // namespace quad

#endif
