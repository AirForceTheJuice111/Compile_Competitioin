#ifndef BACKEND_AARCH64_BLOCK_LAYOUT_HH
#define BACKEND_AARCH64_BLOCK_LAYOUT_HH

#include <vector>

#include "quad.hh"

namespace backend {

// Build a deterministic static trace order. This changes physical emission
// order only; the Quad CFG and SSA graph remain untouched.
std::vector<quad::QuadBlock *> buildAarch64TraceLayout(
    quad::QuadFuncDecl *func);

} // namespace backend

#endif
