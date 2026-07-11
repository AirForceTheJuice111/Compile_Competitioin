#ifndef __ALGEBRASIMP_HH__
#define __ALGEBRASIMP_HH__

#include "quad.hh"

namespace quad {

// Algebra Simplification + Constant Folding pass.
// Simplifies expressions like x+0→x, x*1→x, x*0→0, 3+5→8, x==x→1, etc.
//
// Returns the number of simplifications applied through eliminatedOut.
QuadProgram* algebraSimpProg(QuadProgram* prog,
                             int *eliminatedOut = nullptr);

} // namespace quad

#endif
