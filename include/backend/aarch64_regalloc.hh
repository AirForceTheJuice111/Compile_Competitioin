#pragma once

#include "quad.hh"

#include <cstddef>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace backend {

// GPR allocation result consumed by the stack-code emitter.  A temporary not
// present in tempToRegister retains its ordinary frame slot.  Registers are
// represented by their architectural number (for example, 19 means x19/w19).
struct Aarch64RegisterAllocation {
    std::unordered_map<int, int> tempToRegister;
    std::vector<int> usedCalleeSavedRegisters;
    std::size_t maxPhiCopies = 0;
    std::size_t intervalCount = 0;
    std::size_t spilledIntervalCount = 0;
};

// Allocate integer/pointer GPR homes for Quad SSA temporaries.  FLOAT values
// are also eligible as their existing raw 32-bit representation; this keeps
// the current ABI until a dedicated FP allocator is introduced.
//
// rematerializedTemps contains integer constants which the emitter recreates
// at each use and therefore must not occupy a physical register.
Aarch64RegisterAllocation allocateAarch64Gprs(
    quad::QuadFuncDecl *function,
    const std::unordered_map<int, quad::QuadType> &tempTypes,
    const std::unordered_set<int> &rematerializedTemps = {});

} // namespace backend
