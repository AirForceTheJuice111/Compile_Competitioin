#ifndef __INLINE_HH__
#define __INLINE_HH__

#include "quad.hh"

namespace quad {

// Function Inlining pass.
// Inlines small leaf functions at their call sites to reduce call overhead
// and expose further optimization opportunities.
//
// Returns the number of inlined call sites through eliminatedOut.
QuadProgram* inlineProg(QuadProgram* prog, int *eliminatedOut = nullptr);

} // namespace quad

#endif
