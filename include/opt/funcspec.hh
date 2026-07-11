#ifndef __FUNCSPEC_HH__
#define __FUNCSPEC_HH__

#include "quad.hh"

namespace quad {

// Function Specialization pass.
// Creates specialized versions of functions when called with constant
// arguments, enabling further constant folding within the clone.
//
// Returns the number of specializations created through eliminatedOut.
QuadProgram* funcSpecProg(QuadProgram* prog,
                          int *eliminatedOut = nullptr);

} // namespace quad

#endif
