#ifndef __MEMOPT_HH__
#define __MEMOPT_HH__

#include "quad.hh"

namespace quad {

// Memory Optimization pass (intra-block).
// - Store-to-load forwarding: replace LOAD with stored value when safe
// - Dead store elimination: remove overwritten stores
//
// Returns the number of eliminated instructions through eliminatedOut.
QuadProgram* memOptProg(QuadProgram* prog, int *eliminatedOut = nullptr);

} // namespace quad

#endif
