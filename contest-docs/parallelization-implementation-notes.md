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
- The native runtime reuses one persistent two-core helper rather than creating
  and joining a pthread for every profitable range.
- Constant, provably finite generalized strides and exact modular integer
  reductions are production paths; float and multi-variable reductions remain
  conservative boundaries.

## Shared Loop Plan

Loop recognition is implemented in:

- `include/sysy/parallel_plan.hh`
- `lib/sysy/parallel_plan.cc`

Native lowering consumes this plan, and `--dump-parallel-plan` exposes the same
analysis for inspection. This keeps the safety policy independent from the
final code generator.

The analysis starts from canonical SysY loops of this general form:

```c
i = begin;
while (i comparison end) {
    body;
    i = i + step;
}
```

For dynamic invariant integer endpoints, the production fast path remains
`step == 1` with `<` or `<=`. The inclusive form is normalized to the same
half-open runtime interval by evaluating the endpoint once and passing
`end + 1`; `INT_MAX` takes a generated sequential path using the original
comparison.

When `begin`, `end`, and nonzero `step` are compile-time integers, the planner
also accepts `<`, `<=`, `>`, `>=`, and exactly reachable `!=`. It proves that
the source loop terminates without signed overflow, computes a logical trip
count, dispatches `[0, trips)`, maps each logical IV to
`begin + logical_iv * step`, and restores the source IV's exact final value.
Unreachable `!=`, a step pointing away from its endpoint, dynamic generalized
endpoints, and any overflowing final update remain sequential.

Supported idioms:

- row/array initialization
- array copy or map-style updates with disjoint per-iteration writes
- integer sum reductions with deterministic final addition
- exact array-free modular integer reductions with guarded sequential retry
- descending, non-unit-step, and exactly reachable `!=` loops with constant
  iteration spaces
- canonical early `continue` guarded by an immediately preceding exact IV
  update, lowered through a shared worker/fallback latch
- outer row loops around sequential inner loops
- same-rank array-parameter loops guarded by runtime alias checks
- nested row loops reached through blocks or conditional statements
- transitively pure scalar helper calls, including closed recursive call SCCs
  and immutable scalar-constant reads
- direct pure scalar return helpers substituted into affine index checks
- pure integer outer reductions whose body resets one previously declared
  integer scratch IV and immediately runs its canonical nested loop

The scratch-IV case is deliberately narrow. The reset and nested bound must be
integer constants whose trip count is below the 512-iteration scratch
privatization limit, and the reset plus nested loop must be the complete outer
body. Dynamic and large
nested ranges retain inner-loop selection because measured deeper workers were
slower. The worker receives a private scratch temp;
after a nonempty outer range, lowering always writes the deterministic
canonical final IV value back to the source scalar. Empty outer ranges preserve
the scalar's incoming value, so this optimization does not depend on liveness.

The modular form is deliberately exact rather than fast-math. It recognizes
only `sum = (sum + addend) % MOD`, where `MOD` resolves to a positive integer
constant no greater than `INT_MAX/2`, the body is array-free, and `sum` has no
other use. The runtime uses striped dynamic claims to balance uneven pure-call
cost. It accepts the result only when the initial accumulator is in
`[0, MOD)`, every addend is nonnegative, and `accumulator + addend` cannot
overflow. A worker reports `INT_MIN` on an unsafe value; because the accepted
body has no replay-visible effects, lowering then executes the original source
loop sequentially.

Conservative rejection cases:

- runtime, unknown, or transitively impure calls inside the candidate body;
  functions using arrays, mutable globals, nonlocal writes, or I/O are impure
- every call in a loop endpoint, including otherwise pure calls
- non-integer, induction-dependent, mutable, array-valued, or call-valued loop
  endpoints
- candidate-loop `break` or `return`, and any `continue` whose target/update
  behavior has not been proven safe for the worker latch
- writes to non-private scalars except recognized integer reductions
- array writes whose first index is not affine in the induction variable
- array first indices that use a loop-local scalar, such as `a[i + j]`, even
  when the outer-IV coefficient alone is nonzero
- read/write conflicts on the same array unless they use the same first-index
  partition as the write
- array alias cases that cannot be covered by the same-rank runtime guard
- multiple reductions in the same loop
- float reductions
- accumulator expressions that read their reduction variable outside the one
  recognized `sum = sum + value` operand

Profitability rules:

- integer reductions are allowed because the helper amortizes work over the
  range;
- loops containing nested loops are allowed because each outer iteration is
  relatively expensive;
- otherwise the AST-level cost estimate must meet the static threshold;
- compile-time constant ranges are rejected when `trip_count * work_cost` is
  below the runtime work threshold;
- nested bodies receive an eight-iteration work boost per level, while lowering
  discounts helpers nested inside sequential enclosing loops because they are
  invoked repeatedly;
- generated runtime helpers use an overflow-safe 64-bit product and dispatch
  the persistent helper only when total estimated work reaches 16384.

The plan dump interface is:

```sh
build/compiler --dump-parallel-plan test/performance/01_mm1.sy
```

Each JSONL record includes source location, validity, rejection reason,
comparison, step, logical trip count, captures, reductions, canonical
privatized scalars, estimated cost, lowering kind, and runtime symbol.

## Native AArch64 Lowering

Native lowering happens in `lib/sysy/lower_tree.cc`. When a valid profitable
plan is selected, lowering emits:

1. a worker function for the loop body;
2. a context object containing captured scalars and array bases;
3. optional alias guards and a sequential fallback for guarded parameter-array
   cases;
4. a call to one of the AArch64 runtime entry points:

```c
void __sysy_parallel_for_range(int begin, int end, void *ctx, worker,
                               int work_cost);
int __sysy_parallel_reduce_int_range(int begin, int end, void *ctx, worker,
                                     int work_cost);
int __sysy_parallel_reduce_mod_int_range(int begin, int end, void *ctx,
                                         worker, int work_cost, int modulus);
```

The AArch64 backend embeds the pthread runtime implementation directly into the
generated `.s` file only when these runtime symbols are actually called. The
run scripts detect those calls and add `-pthread` during AArch64 linking.

Context layout after the AArch64 migration:

- pointer and function-pointer fields are 8 bytes;
- pointer fields are 8-byte aligned;
- scalar `int` and `float` captures remain 4-byte payloads;
- worker function pointers use AAPCS64 `x` registers.

The runtime lazily starts one persistent helper pthread. Ordinary for/reduction
calls divide work between the caller and helper; modular reductions use striped
dynamic claims to avoid a heavy contiguous half. A short atomic spin handles
dense dispatch, condition variables put the helper to sleep between bursts,
and an atomic busy flag forces nested or concurrent calls down the direct path
rather than allowing the shared job record to be overwritten. Empty ranges,
invalid costs, and a failed initial `pthread_create` also take the direct path.
Modular workers use `INT_MIN` only as an internal unsafe sentinel that triggers
the lowering-generated sequential retry.

The AArch64 backend emits 64-bit `x` register comparisons for pointer-valued
conditional jumps, which is required by the runtime alias guards.

## Native Regression

The native AArch64 parallel regression target is:

```sh
THREADS=2 make sysy-parallel-native-regression
make sysy-parallel-plan-regression
```

It scans the canonical preliminary suite in `test/performance`, compiles with
`--parallel-native -O1`,
selects cases whose AArch64 assembly contains generated parallel workers,
links with `clang --target=aarch64-linux-gnu`, runs under `qemu-aarch64`, and
compares exact output against `.out` files.

The plan regression additionally checks focused validity/rejection reasons,
worker counts, recursive nested-loop selection, dynamic-inclusive overflow
fallback, generalized strides and unreachable `!=`, modular accept/reject
cases, alias fallback, and loop-local-index overlap rejection. Native execution
tests cover the persistent runtime's nested re-entry fallback.

## Adjacent Optimizer And AArch64 Status

Several non-parallel changes materially affect worker and sequential code:

- the default guarded bitwise idiom pass recognizes the complete 32-round
  software `and`/`or`/`xor` helper CFG, takes a native fast path only when both
  inputs are nonnegative, and keeps the original signed fallback;
- imported LoopSimplify, LoopUnroll, and trace block layout are present but
  require `SYSY_EXPERIMENTAL_PASSES=loopsimplify,loopunroll,traceblock` as
  applicable; LoopSimplify regressed EB0 in controlled profiling and unrolling
  did not trigger on the measured performance corpus;
- the imported textual AArch64 peephole is also experimental because it
  regressed MYO and 680 even after unsound liveness-dependent patterns were
  removed;
- pointer-plus-signed-offset and compare-with-zero forms are constructed in
  instruction selection, where scratch lifetimes are known, so those safe
  addressing improvements do not depend on the text peephole.
- exact signed constant division/remainder lowering uses power-of-two and
  magic-number sequences plus fused `msub`; millions of differential operand
  pairs, including `INT_MIN/-1`, were checked against dynamic `sdiv`/`msub`.

All Mac timing samples and session boundaries are recorded in
`contest-docs/performance-timings.md`.

## Current Boundary

The guarded correctness path is in place for the supported loop subset.
Remaining work is broader parallel coverage and backend performance:

- add correct support for multi-variable reductions;
- define whether floating-point reductions may change association, then either
  implement an explicit fast-math mode or keep them sequential;
- broaden safe loop-local control flow beyond the canonical early-continue
  latch while continuing to reject transfers that leave the candidate loop;
- support dynamic generalized steps/endpoints only when termination and signed
  overflow can be proven;
- improve alias analysis for different-rank parameter/global interactions;
- extend the current ten-register injective hot-temp residency into a
  liveness-based allocator with FP-register residency;
- add scaled-index/pointer-induction addressing and post-allocation peepholes;
- tune loop profitability on Cortex-A53 contest hardware;
- harden and selectively enable imported loop transforms and trace layout only
  where measurements show a net win;
- investigate NEON/vector lowering for array-heavy kernels.
