#pragma once

#include "quad.hh"

#include <cstddef>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace backend {

// Register allocation result consumed by the stack-code emitter.  A temporary
// absent from both home maps retains its ordinary frame slot.  Registers are
// represented by their architectural number (for example, 19 means x19/w19
// in tempToRegister and s19 in tempToFloatRegister).
struct Aarch64RegisterAllocation {
    std::unordered_map<int, int> tempToRegister;
    std::unordered_map<int, int> tempToFloatRegister;
    std::vector<int> usedCalleeSavedRegisters;
    std::vector<int> usedCalleeSavedFloatRegisters;
    std::size_t maxPhiCopies = 0;
    std::size_t intervalCount = 0;
    std::size_t spilledIntervalCount = 0;
};

// A PTR_CALC whose sole observable result is a memory address can be selected
// directly as an AArch64 register-offset/immediate memory operand.  The
// allocator must see the original base and byte offset at the LOAD/STORE use
// point (rather than at PTR_CALC), otherwise a call or register reuse between
// the two statements could destroy an operand before the fused instruction.
struct Aarch64FusedAddress {
    quad::QuadStm *memoryStatement = nullptr;
    quad::QuadPtrCalc *pointerCalculation = nullptr;
};

// Allocate integer/pointer GPR homes and native single-precision FPR homes for
// Quad SSA temporaries.  Non-call-crossing floats prefer caller-saved s16-s29;
// call-crossing floats use the callee-saved low lanes s8-s15.  s0-s7 remain
// reserved for ABI arguments/results and s30-s31 for selector scratch.
//
// rematerializedTemps contains integer constants which the emitter recreates
// at each use and therefore must not occupy a physical register.
Aarch64RegisterAllocation allocateAarch64Gprs(
    quad::QuadFuncDecl *function,
    const std::unordered_map<int, quad::QuadType> &tempTypes,
    const std::unordered_set<int> &rematerializedTemps = {},
    const std::vector<Aarch64FusedAddress> &fusedAddresses = {});

} // namespace backend
