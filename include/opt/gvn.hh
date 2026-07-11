#ifndef __GVN_HH__
#define __GVN_HH__

#include <set>
#include "quad.hh"
#include "flowinfo.hh"

namespace quad {
    QuadProgram* gvnProg(QuadProgram* prog, std::set<FuncFlowInfo*>* flowInfo,
                         int *eliminatedOut = nullptr);
}

#endif
