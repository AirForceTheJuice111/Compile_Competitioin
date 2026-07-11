#ifndef __COPYPROP_HH__
#define __COPYPROP_HH__

#include "quad.hh"

namespace quad {

// SSA-based copy propagation: eliminate trivial MOVE instructions
//   t_dst = t_src   (both temps)
// by replacing all uses of t_dst with t_src and removing the copy.
// Includes a simple dead-code elimination pass.
//
// Returns the number of eliminated instructions through eliminatedOut.
QuadProgram* copyPropProg(QuadProgram* prog,
                          int *eliminatedOut = nullptr);

} // namespace quad

#endif
