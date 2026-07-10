# Parallelization Implementation Notes

This note records the current parallelization implementation after the
AArch64-only migration.

## Status Summary

- The production path is native AArch64 assembly.
- `-O1`, `-O2`, and `allopt` enable native loop parallelization by default.
- `--no-parallel-native` disables native loop parallelization for debugging.
- The old ARM32 per-function register-allocation parallelism was removed with
  the ARM32 backend.
- The earlier C++ prototype emitter and C runtime scaffold have been removed;
  loop planning is now used by native lowering and by `--dump-parallel-plan`.

## Shared Loop Plan

Loop recognition is implemented in:

- `include/sysy/parallel_plan.hh`
- `lib/sysy/parallel_plan.cc`

Native lowering consumes this plan, and `--dump-parallel-plan` exposes the same
analysis for inspection. This keeps the safety policy independent from the
final code generator.

The analysis recognizes canonical SysY loops of this form:

```c
i = begin;
while (i < end) {
    body;
    i = i + 1;
}
```

Supported idioms:

- row/array initialization
- array copy or map-style updates with disjoint per-iteration writes
- integer sum reductions with deterministic final addition
- outer row loops around sequential inner loops

Conservative rejection cases:

- calls inside the candidate loop body
- `break`, `continue`, or `return` inside the body
- writes to non-private scalars except recognized integer reductions
- array writes that cannot be proven affine in the induction variable
- read/write conflicts on the same array without a partitioned first dimension
- multiple reductions in the same loop
- float reductions

Profitability rules:

- integer reductions are allowed because the helper amortizes work over the
  range;
- loops containing nested loops are allowed because each outer iteration is
  relatively expensive;
- otherwise the AST-level cost estimate must meet the static threshold;
- generated runtime helpers still check the dynamic trip count before creating
  worker threads.

The plan dump interface is:

```sh
build/compiler --dump-parallel-plan test/performance_final/2025-MYO-20.sy
```

Each JSONL record includes source location, validity, rejection reason,
captures, reductions, estimated cost, lowering kind, and runtime symbol.

## Native AArch64 Lowering

Native lowering happens in `lib/sysy/lower_tree.cc`. When a valid profitable
plan is selected, lowering emits:

1. a worker function for the loop body;
2. a context object containing captured scalars and array bases;
3. a call to one of the AArch64 runtime entry points:

```c
void __sysy_parallel_for_range(int begin, int end, void *ctx, worker);
int __sysy_parallel_reduce_int_range(int begin, int end, void *ctx, worker);
```

The AArch64 backend embeds the pthread runtime implementation directly into the
generated `.s` file only when these runtime symbols are actually called. The
run scripts detect those calls and add `-pthread` during AArch64 linking.

Context layout after the AArch64 migration:

- pointer and function-pointer fields are 8 bytes;
- pointer fields are 8-byte aligned;
- scalar `int` and `float` captures remain 4-byte payloads;
- worker function pointers use AAPCS64 `x` registers.

The runtime defaults to two worker chunks and applies a dynamic minimum trip
count gate. If `pthread_create` fails or the trip count is too small, it falls
back to sequential worker execution over the whole range.

## Native Regression

The native AArch64 parallel regression target is:

```sh
THREADS=2 make sysy-parallel-native-regression
```

It scans `test/performance_final`, compiles with `--parallel-native -O1`,
selects cases whose AArch64 assembly contains generated parallel workers,
links with `clang --target=aarch64-linux-gnu`, runs under `qemu-aarch64`, and
compares exact output against `.out` files.

## Current Boundary

The correctness path is complete for AArch64. Remaining work is performance
engineering:

- replace the AArch64 stack-code backend with register allocation;
- add AArch64 addressing-mode and peephole optimizations;
- tune loop profitability on Cortex-A53 contest hardware;
- investigate NEON/vector lowering for array-heavy kernels;
- broaden reductions only when floating-point reproducibility requirements are
  clear.
