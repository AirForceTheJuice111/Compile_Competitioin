#ifndef QUAD_OPT_LOOP_UNROLL_HH
#define QUAD_OPT_LOOP_UNROLL_HH

#include "quad.hh"

namespace quad {

struct LoopUnrollOptions {
    int maxTripCount = 8;
    int maxBodyInstructions = 24;
    int maxExpandedInstructions = 96;
    int maxAdditionalTemps = 64;
};

struct LoopUnrollStats {
    int functionsVisited = 0;
    int functionsChanged = 0;
    int functionsSkipped = 0;
    int loopsUnrolled = 0;
    int instructionsCloned = 0;
};

// Fully unroll small, exact-trip-count, canonical SSA loops. The transform is
// deliberately conservative and transactional.
QuadProgram *loopUnrollProg(QuadProgram *program,
                            const LoopUnrollOptions &options = {},
                            LoopUnrollStats *stats = nullptr);

} // namespace quad

#endif
