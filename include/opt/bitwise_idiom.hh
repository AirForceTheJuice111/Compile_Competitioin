#ifndef SYSY_BITWISE_IDIOM_HH
#define SYSY_BITWISE_IDIOM_HH

#include "quad.hh"

namespace quad {

// Recognize the exact 32-iteration arithmetic emulations of integer AND, OR,
// and XOR used by several contest workloads. A native operation is selected
// only when both arguments are nonnegative; all other inputs retain the
// original function body because SysY division truncates toward zero.
QuadProgram *specializeBitwiseIdioms(QuadProgram *program,
                                     int *specializedOut = nullptr);

} // namespace quad

#endif
