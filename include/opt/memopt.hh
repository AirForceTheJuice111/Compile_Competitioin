#ifndef __MEMOPT_HH__
#define __MEMOPT_HH__
#include <set>
#include "quad.hh"
#include "flowinfo.hh"
namespace quad {
QuadProgram* memOptProg(QuadProgram* prog, std::set<FuncFlowInfo*>* flow, int *eo=nullptr);
}
#endif
