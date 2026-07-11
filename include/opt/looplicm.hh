#ifndef __LOOPLICM_HH__
#define __LOOPLICM_HH__

#include "loopheader.hh"
#include "quad.hh"

quad::QuadFuncDecl *loopHoistFunc(quad::QuadFuncDecl *func, LoopHeaderMap *loopHeaderMap);

#endif
