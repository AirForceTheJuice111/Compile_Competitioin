#ifndef OPT_FUNCTION_DCE_HH
#define OPT_FUNCTION_DCE_HH
#include "quad.hh"
namespace quad {
QuadProgram *eliminateDeadFunctions(QuadProgram *program,
                                    int *removedOut = nullptr);
}
#endif
