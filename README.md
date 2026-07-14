# SysY2022 AArch64 Contest Compiler

This branch is the SysY2022 contest migration. The public contest entry is:

```sh
build/compiler -S -o output.s input.sy
```

The old FMJ frontend, AST, interpreter, course harness, ARM32 backend, GCC
bridge, and ARM32 runtime files have been removed from this branch. Assembly
output is AArch64 ARMv8-A only.

## Environment

Expected tools on x86_64 Linux:

- `make`, `cmake`, `ninja`
- `clang` with `--target=aarch64-linux-gnu`
- `qemu-aarch64`
- AArch64 sysroot, defaulting to `/usr/aarch64-linux-gnu`
- `unzip` for optional performance archive regression

## Build

```sh
make build
```

This builds `build/compiler`. The compiler uses the native SysY lexer, parser,
semantic checker, lowering code, Tree/Quad/SSA optimizers, and the native
AArch64 backend. `--target aarch64` is accepted as a compatibility no-op; any
ARM32 target or bridge option is rejected.

Useful frontend/debug commands:

```sh
build/compiler --dump-tokens test/functional/95_float.sy
build/compiler --dump-ast test/functional/95_float.sy
build/compiler --check-sysy test/functional/95_float.sy
build/compiler --dump-parallel-plan test/performance/01_mm1.sy
```

## Run One Program

```sh
make run-one test/functional/95_float.sy
SYSY_OPT='-O1' make run-one test/performance/03_sort2.sy
```

`make run-one` compiles the selected `.sy` file to AArch64 assembly, links it
with `vendor/libsysy/sylib.c`, runs it through `qemu-aarch64 -L
/usr/aarch64-linux-gnu`, prints stdout/stderr/return code, and compares
`stdout + return_code` with a sibling `.out` file when one exists. If a sibling
`.in` file exists, it is used as stdin.

Equivalent manual command:

```sh
build/compiler -S -o /tmp/program.s test/functional/00_main.sy
clang --target=aarch64-linux-gnu \
  -o /tmp/program.a64 /tmp/program.s vendor/libsysy/sylib.c -lm
qemu-aarch64 -L /usr/aarch64-linux-gnu /tmp/program.a64
```

If the generated assembly calls the embedded parallel helpers
`__sysy_parallel_for_range` or `__sysy_parallel_reduce_int_range`, the run
scripts add `-pthread` at link time.

## Regression

Compile every positive `.sy` under `test/`:

```sh
make compile
SYSY_OPT='-O1' make compile
```

Run functional regression:

```sh
make run
make sysy-functional-regression
MAX_CASES=40 make sysy-functional-regression
```

Run parser and semantic checks:

```sh
make sysy-parse-regression
make sysy-semantic-regression
```

Run the canonical preliminary performance regression:

```sh
make sysy-performance-regression
MAX_CASES=3 make sysy-performance-regression
```

An older archive can still be selected explicitly with
`SYSY_PERF_ARCHIVE=/path/to/archive.zip`.

## Optimization Update

The compiler intentionally keeps two entry modes:

- functional/correctness invocation defaults to `-O0`;
- the contest performance invocation passes `-O1`, which selects the full
  optimizer, native AArch64 backend, and guarded two-core loop runtime.

This optimized delivery adds a conservative multi-round inline pipeline,
post-inline SCCP cleanup, whole-program unreachable-function removal, compact
spill-frame allocation, direct use of resident integer/pointer and scalar-float
registers, immediate-aware arithmetic/comparisons, constant-multiply strength
reduction, and folded constant pointer offsets. The existing
profitability-gated native parallel path remains enabled for `-O1`; pass
`--no-parallel-native` to measure the sequential backend alone.

A reproducible static regression is included:

```sh
# Nine focused local cases, optimized compiler only
make local-static-regression SYSY_TEST_ROOT=bench_local

# Compare against another compiler binary
BASELINE_COMPILER=/path/to/original/compiler \
PARALLEL_MODE=serial \
make local-static-regression SYSY_TEST_ROOT=bench_local
```

The full implementation rationale, safety conditions, iteration record, and
validation procedure are in `OPTIMIZATION_IMPLEMENTATION_REPORT.md`. Aggregate
results are in `OPTIMIZATION_STATIC_SUMMARY.tsv`, and per-case results are in
`OPTIMIZATION_CASE_RESULTS.tsv`.

## AArch64 Backend

The backend lowers SysY AST into the migrated Tree/Quad/SSA IR. It supports the
existing SCCP, LICM, and induction-variable optimization modes:

- `-O0` or `--opt-mode none`
- `const`
- `loop1`
- `loop2`
- `allloop`
- `allopt`, `-O1`, `-O2`

The AArch64 emitter uses 64-bit pointers and AAPCS64 calling convention:
integer/pointer arguments use `w/x` registers, scalar floats use `s` registers,
and `%f` varargs are promoted to `double` in `d` registers. SysY globals and
arrays use the official `sylib.c/.h` runtime interface.

Optimized modes run the hardened integer AlgebraSimp, guarded bitwise-helper
specialization, typed GVN, CopyProp/conservative DCE, and restricted scalar-int
inliner by default. The passes rebuild and verify Quad metadata before final
flow analysis and register residency. AlgebraSimp uses defined 32-bit wrapping
and leaves floating-point identities/comparisons unchanged; GVN excludes
loads/calls and keys values by type; the inliner accepts only straight-line,
leaf, single-return scalar-int functions. The bitwise pass matches the complete
32-iteration software `and`/`or`/`xor` helper CFG, emits a native operation only
for nonnegative operands, and preserves the original helper as the signed
fallback. `SYSY_DISABLE_PASSES=bitwise,gvn,copyprop` can isolate stable passes
for debugging.

Imported FuncSpec, MemOpt, LoopSimplify, LoopUnroll, trace block layout, and
the textual AArch64 peephole remain disabled unless named explicitly in
`SYSY_EXPERIMENTAL_PASSES` as `funcspec`, `memopt`, `loopsimplify`,
`loopunroll`, `traceblock`, or `peephole`. Their general correctness or
profitability is not yet strong enough for the default pipeline; in particular,
trace layout and peephole changes previously regressed measured performance.
Whole-program function DCE is a stable optimized-mode default. Even when the
peephole is explicitly enabled, its historically mixed branch-to-next-label
rewrite additionally requires `SYSY_AGGRESSIVE_PEEPHOLE=1`.

The AArch64 emitter assigns up to ten hot, loop-weighted Quad temps injectively
to callee-saved `x19`-`x28` homes. Integer and pointer operations now consume
and produce those homes directly instead of round-tripping through `w9/w10`
and a stack slot. Scalar floats move directly between ABI `s` registers,
resident raw-bit homes, or spill slots. Stack-frame construction allocates
slots only for values that are actually spilled and cannot be rematerialized.

Address and arithmetic forms whose scratch lifetimes are known are selected
directly: add/sub/cmp use encodable immediates (including the shifted 12-bit
form), constant multiplication recognizes `0`, `1`, `-1`, powers of two, and
`2^k +/- 1`, constant pointer offsets fold into add/sub, and variable byte
offsets use `add xD, xN, wM, sxtw`. Signed division and remainder by
compile-time constants retain the exact magic-number, power-of-two, and `msub`
sequences, including negative divisors and the `INT_MIN / -1` wrapping
boundary. Broader scaled-index and pointer-induction folding remains future
work.

Native loop parallelization is enabled automatically for optimized modes unless
`--no-parallel-native` is passed. The lowering stage uses the shared loop plan
analysis in `lib/sysy/parallel_plan.cc`. Dynamic invariant integer bounds retain
the common unit-step `<` and `<=` path. When the initializer, endpoint, and
nonzero step are compile-time integers, the planner also proves finite,
overflow-free `<`, `<=`, `>`, `>=`, and exactly reachable `!=` loops, maps them
to a logical half-open iteration range, and restores the source IV's exact final
value. This covers descending and non-unit-step while/for-like patterns without
changing signed-overflow behavior.

When a loop is parallelized, the AArch64 assembly embeds worker functions,
8-byte pointer-safe context structs, and a small pthread runtime in the same
`.s` file. Parameter array aliasing is handled with a runtime guard for
same-rank array parameters: aliasing calls take a generated sequential
fallback, while non-aliasing calls use the parallel worker. Loop endpoints
must be invariant integer expressions; dynamic inclusive endpoints guard
`INT_MAX` and use the original sequential comparison on the overflow path.
Partition checks reject first-index expressions that depend on loop-local
scalars, even when a superficial coefficient in the outer IV is nonzero,
because such expressions can overlap between workers.
Candidate-loop early `continue` is accepted only when its immediately preceding
sibling is the exact unshadowed source-IV update. Worker and sequential fallback
lowering suppress that source update and route the transfer through one shared
latch, so source and logical IVs advance exactly once. Outer `break`, returns,
bare continues, and wrong-step/shadowed updates remain sequential.
Loop bodies may call user helpers proven transitively scalar-only and pure,
including self-recursive helpers and reads of immutable scalar constants.
Runtime/unknown calls, mutable globals, and every hidden array access remain
sequential; simple pure return expressions can also expose affine indices such
as `idx(r, c, n)` to the partition checker.
Pure integer reductions can additionally select an outer loop whose complete
body resets one previously declared integer scratch IV and runs its canonical
nested loop. The scratch is worker-private, but its source-visible final IV
value is still restored for nonempty outer ranges; empty ranges leave it
unchanged. This coarse form is selected only when the nested trip count is a
compile-time constant below the pthread threshold; dynamic or large inner
ranges retain the simpler inner-loop worker selected by measurements.
Native profitability uses a fifth runtime argument containing a conservative
per-iteration work estimate. Nested bodies are boosted, helpers emitted inside
sequential enclosing loops are depth-discounted. The exact integer recurrence
`sum = (sum + addend) % MOD` can additionally use a striped modular-reduction
worker when `MOD` is a positive constant no greater than `INT_MAX/2`, the loop
has no replay-visible effects, and the ordinary reduction constraints hold.
Runtime checks reject an unsafe initial residue, negative addend, or possible
signed-add overflow and retry the original sequential loop, preserving SysY
signed remainder behavior.

The embedded two-core runtime lazily creates one persistent helper thread and
reuses it across range calls. A short atomic rendezvous keeps dense dispatches
fast, condition variables provide the sleeping fallback, and an atomic busy
guard makes nested or concurrent re-entry run directly instead of corrupting
the shared job slot or deadlocking. The overflow-safe 64-bit
`trip_count * work_cost` gate requires a total estimated cost of 16384. Cheap
dynamic reductions therefore retain direct execution, while sufficiently
coarse short ranges can parallelize.

## Native Parallel Check

The production native parallel path can be checked with:

```sh
THREADS=2 make sysy-parallel-native-regression
make sysy-parallel-plan-regression
```

Mac two-core measurements, including compiler revisions and comparability
notes, are maintained in `contest-docs/performance-timings.md`.
The canonical preliminary performance suite is `test/performance`. A cached
16-worker native Mac regression is available as:

```bash
scripts/sysy_mac_distributed_regression.sh \
  --test-root test/performance --workers 16
```

## Runtime Files

Only the official SysY source runtime is kept:

- `vendor/libsysy/sylib.c`
- `vendor/libsysy/sylib.h`

Legacy `libsysy32.*`, `libsysy64.*`, and `libsysy_arm.a` were removed during
the AArch64-only migration.

## Tests

`test/` contains official functional SysY2022 tests, hidden-style functional
tests, local semantic rejects, final performance cases, and additional
FINALREPORT tests. The old FMJ `.fmj` corpus is not part of this contest
branch.
