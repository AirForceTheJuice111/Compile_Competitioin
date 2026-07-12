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
build/compiler --dump-parallel-plan test/performance_final/2025-MYO-20.sy
```

## Run One Program

```sh
make run-one test/functional/95_float.sy
SYSY_OPT='-O1' make run-one test/performance_final/2025-3Z0-43.sy
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

Run performance archive regression:

```sh
make sysy-performance-regression SYSY_PERF_ARCHIVE=/tmp/compiler2025/ARM-性能.zip
MAX_CASES=3 make sysy-performance-regression
```

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

Optimized modes run the hardened integer AlgebraSimp, typed GVN,
CopyProp/conservative DCE, and restricted scalar-int inliner by default. The
passes rebuild and verify Quad metadata before final flow analysis and register
residency. AlgebraSimp uses defined 32-bit wrapping and leaves floating-point
identities/comparisons unchanged; GVN excludes loads/calls and keys values by
type; the inliner accepts only straight-line, leaf, single-return scalar-int
functions. `SYSY_DISABLE_PASSES=gvn,copyprop` can isolate a stable pass for
debugging. The imported FuncSpec and MemOpt prototypes remain disabled unless
named explicitly in `SYSY_EXPERIMENTAL_PASSES` because their general forms are
not yet correctness-hardened.

The AArch64 emitter assigns up to ten hot, loop-weighted Quad temps injectively
to callee-saved `x19`-`x28` homes, including pointer values and raw float bits.
It preserves those registers under AAPCS64, snapshots parallel PHI copies, and
uses direct frame-relative addressing for nearby spills.

Native loop parallelization is enabled automatically for optimized modes unless
`--no-parallel-native` is passed. The lowering stage uses the shared loop plan
analysis in `lib/sysy/parallel_plan.cc`; current canonical loops may use either
`while (i < end)` or `while (i <= end)` with unit increments. When a loop is
parallelized, the AArch64 assembly embeds worker functions, 8-byte pointer-safe
context structs, and a small pthread runtime in the same `.s` file. Parameter
array aliasing is handled with a runtime guard for same-rank array parameters:
aliasing calls take a generated sequential fallback, while non-aliasing calls
use the parallel worker. Loop endpoints must be invariant integer expressions;
dynamic inclusive endpoints guard `INT_MAX` and use the original sequential
comparison on the overflow path.
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
sequential enclosing loops are depth-discounted, and the two-core runtime
creates a pthread only when the overflow-safe 64-bit `trip_count * work_cost`
reaches 16384. Cheap dynamic reductions therefore retain direct execution,
while sufficiently coarse short ranges can parallelize.

## Native Parallel Check

The production native parallel path can be checked with:

```sh
THREADS=2 make sysy-parallel-native-regression
make sysy-parallel-plan-regression
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
