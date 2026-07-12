# SysY2022 AArch64 Compiler Report

This report documents the current `contest` branch after the AArch64-only
migration.

## Entry

The contest executable is built as `build/compiler` from
`tools/compiler/main.cc`.

Supported assembly invocation:

```sh
compiler -S -o output.s input.sy
```

The compiler accepts `--target aarch64` as a compatibility no-op. ARM32 targets
and bridge backends are rejected because the branch now has a single production
backend: native AArch64.

Debug and analysis modes:

- `--dump-tokens`
- `--dump-ast`
- `--check-sysy`
- `--dump-parallel-plan`

## Frontend

The SysY frontend lives under `include/sysy` and `lib/sysy`.

- `lexer.cc` tokenizes SysY2022 source, including integer constants, floating
  constants, identifiers, keywords, comments, operators, and string literals
  used by `putf`.
- `parser.cc` is a recursive-descent parser for `CompUnit`, declarations,
  function definitions, blocks, statements, expressions, array dimensions, and
  initialization lists.
- `semantics.cc` checks scopes, redeclarations, function signatures, `main`,
  const assignment, `break`/`continue` placement, return constraints, array
  parameter compatibility, builtin runtime signatures, and `putf` format
  arguments.

The frontend emits a SysY AST rather than reusing the old FDMJ AST. This keeps
the language boundary explicit: SysY has top-level functions, C-style block
scopes, `int`/`float`/`void`, and multi-dimensional arrays; it has no classes,
objects, inheritance, fields, methods, or dynamic dispatch.

## Lowering

`lib/sysy/lower_tree.cc` lowers SysY AST to the shared Tree IR.

Important lowering rules:

- global scalars and arrays are emitted into data sections;
- local arrays are represented by stack allocations;
- array subscripts are lowered to row-major linearized address arithmetic;
- array parameters are passed as addresses, with the first dimension omitted
  according to SysY rules;
- `float` values use IEEE-754 32-bit payloads in IR temps and memory;
- implicit `int`/`float` conversions are made explicit before backend emission;
- `starttime()` and `stoptime()` lower to runtime calls with source line
  numbers;
- `putf` string literals are emitted into `.rodata`, and `%f` arguments are
  promoted to double for the AAPCS64 vararg call.

The lowering stage also consumes loop parallelization plans when native
parallel mode is enabled.

## IR And Optimizations

The migrated middle-end remains organized as:

- Tree IR: `include/ir`, `lib/ir`
- Quad IR and SSA: `include/quad`, `lib/quad`
- data/control-flow analysis: `include/quadflow`, `lib/quadflow`
- optimizations: `include/opt`, `lib/opt`

Supported optimization modes:

- `none` / `-O0`
- `const`
- `loop1`
- `loop2`
- `allloop`
- `allopt` / `-O1` / `-O2`

The optimizer pipeline includes SSA construction, sparse conditional constant
propagation, loop-invariant code motion, induction-variable analysis, strength
reduction, and cleanup. Unsupported or unsafe cases are handled conservatively;
correctness is prioritized over applying a transformation.

The safe default additions are integer AlgebraSimp, a complete-CFG guarded
bitwise-helper recognizer, typed GVN, CopyProp/conservative DCE, and restricted
straight-line scalar-int inlining. The bitwise recognizer replaces the exact
32-round software `and`/`or`/`xor` idiom only after both operands pass a
nonnegative runtime guard; signed inputs execute the untouched original CFG.
Imported FuncSpec, MemOpt, LoopSimplify, LoopUnroll, and trace layout exist for
continued development but are selected only through
`SYSY_EXPERIMENTAL_PASSES` (`funcspec`, `memopt`, `loopsimplify`, `loopunroll`,
or `traceblock`).

## AArch64 Backend

The backend interface is `include/backend/backend_driver.hh`; the implementation
is `lib/backend/backend_driver.cc`.

The current backend path is:

```text
Tree -> canonical Tree -> Quad -> basic blocks -> flow -> SSA
     -> selected optimizations -> final Quad -> AArch64 assembly
```

The old ARM32 instruction selection and graph-coloring backend has been
removed. The native AArch64 emitter now combines conservative stack slots with
injective hot-temp residency in callee-saved `x19`-`x28`. Loop depth,
cross-block use, and parameters influence residency; PHI edge copies are
snapshotted before assignment and nearby spills use direct `ldur`/`stur`
addressing. Pointer calculation directly selects
`add xD, xN, wM, sxtw`, and zero comparisons use an immediate, because the
instruction selector owns the scratch-register lifetimes at those points.
Signed division and remainder by an SSA-proven integer constant use exact
power-of-two or magic-number sequences; canonical remainder chains fuse the
quotient with `msub`. The selector preserves truncation toward zero and the
architectural `INT_MIN / -1` wrapped result.

The default optimized pipeline is SCCP, hardened integer AlgebraSimp, the
guarded bitwise idiom pass, the established LICM/induction passes selected by
the optimization mode, typed GVN, CopyProp/conservative DCE, and restricted
straight-line scalar-int inlining. Mutating passes rebuild and verify Quad
def/use, CFG, extent, SSA uniqueness, type, dominance, and PHI-edge metadata.
The imported textual AArch64 peephole is also experimental: although it helped
some cases, controlled profiling found repeatable MYO and 680 regressions.
Construction-time address and compare forms above remain enabled because they
do not depend on textual liveness guesses.

Implemented ABI behavior:

- 32-bit integers use `w` registers and 4-byte memory slots;
- pointers use `x` registers and 8-byte slots;
- scalar floats use `s` registers;
- promoted `double` varargs use `d` registers or 8-byte stack slots;
- return values follow AAPCS64;
- stack frames remain 16-byte aligned across calls;
- global addresses use AArch64 PC-relative addressing.

The backend emits a small pthread runtime in assembly only when native
parallel lowering generates calls to the parallel helper symbols.

## Runtime

The kept SysY runtime files are:

- `vendor/libsysy/sylib.c`
- `vendor/libsysy/sylib.h`

Run scripts link generated AArch64 assembly with `sylib.c` and `-lm`. They add
`-pthread` only for assembly that contains native parallel helper calls.

## Parallelization

Loop analysis is shared by native lowering and the plan dump interface:

- `include/sysy/parallel_plan.hh`
- `lib/sysy/parallel_plan.cc`

The analysis recognizes affine `while` loops with disjoint array writes and
strict integer reductions. Dynamic invariant integer endpoints use the common
unit-step `<` or `<=` path. The inclusive form is normalized to a half-open
AArch64 runtime interval by evaluating the endpoint once and passing `end + 1`;
`INT_MAX` takes a generated sequential path with the original comparison.

Generalized loops are accepted when the initializer, endpoint, and nonzero
step are compile-time integers. The planner proves a finite, overflow-free
logical iteration count for `<`, `<=`, `>`, `>=`, or exactly reachable `!=`,
then workers map logical iteration `k` back to `initial + k * step`. The caller
restores the source induction variable's exact canonical final value. Dynamic
non-unit steps and endpoints remain sequential.

Bounds containing calls, arrays, the induction variable, or scalars modified
by the loop are rejected. Same-array accesses are kept parallel only when reads
and writes stay in the same first-index partition; cross-iteration patterns
such as `a[i] = a[i + 1]` are rejected. A first index that refers to any
loop-local scalar is also rejected even if it contains a nonzero affine
coefficient in the outer IV, closing the overlapping pattern represented by
`a[i + j]`. Same-rank array parameter aliasing no longer forces a static
rejection: lowering emits a runtime pointer guard and falls back to the
original sequential loop when the captured array bases alias.

A candidate-loop early `continue` is accepted only when the immediately
preceding sibling is the exact, unshadowed canonical IV update. The update is
suppressed in the cloned body and both ordinary fallthrough and continue target
one generated latch, which advances source and logical IVs exactly once.
Nested-loop-local break/continue retain their normal targets; outer break,
returns, and unproven continues are rejected.

An interprocedural fixed-point summary permits user helpers proven scalar-only
and side-effect-free, including recursion and immutable scalar constants.
Runtime/unknown calls, arrays, mutable globals, nonlocal writes, and I/O remain
impure. Direct pure return expressions are substituted conservatively into
affine index checks, exposing helpers such as `idx(r, c, n)`. The pass still
rejects endpoint calls, unsafe scalar writes, unguardable array disjointness,
multiple reductions, float reductions, and control transfers not proven local
to a safe worker iteration.

Nested-loop profitability detection is recursive through blocks and
conditionals. Reduction validation also rejects self-dependent accumulators and
uses of a partial accumulator value elsewhere in the loop body.

In addition to plain integer sums, the exact recurrence
`sum = (sum + addend) % MOD` has a dedicated modular worker. `MOD` must resolve
to a positive integer constant no greater than `INT_MAX/2`; the body must be
array-free and replay-safe, and all uses of `sum` must be the same recurrence.
A striped dynamic schedule balances O30/SFX's recursive addend cost. The caller
uses the parallel result only when the initial accumulator is a canonical
residue and every worker observes a nonnegative addend that cannot overflow the
source signed addition. Otherwise an `INT_MIN` sentinel triggers the original
sequential loop, preserving exact SysY remainder and overflow behavior.

Pure outer integer reductions may also privatize one canonical nested-loop
scratch IV when the complete outer body is an unconditional constant reset
followed immediately by that IV's constant-bound unit-step loop, provided its
trip count is below the pthread threshold. Dynamic or large nested ranges keep
the simpler measured inner-loop choice. Workers bind the scratch name to a
private temp. The caller writes its deterministic final IV value back after
every nonempty parallel range and preserves the incoming value for an empty
range, avoiding any assumption that the scalar is dead.

Optimized modes enable native parallel lowering by default. The generated
AArch64 assembly contains worker functions, 8-byte pointer-safe context
layouts, and runtime helper implementations when needed. The runtime lazily
creates one persistent helper pthread and reuses it for later range calls.
Short atomic spinning handles dense dispatch, condition variables are the
sleeping fallback, and a busy guard routes nested or concurrent re-entry to
direct execution so the shared slot cannot deadlock or be overwritten.

Each helper receives a conservative per-iteration work estimate. Nested bodies
are boosted and helpers inside sequential enclosing loops are depth-discounted;
compile-time and dynamic profitability use an overflow-safe 64-bit
`trip_count * work_cost` threshold of 16384. This admits coarse short ranges
without dispatching cheap dynamic reductions or repeatedly invoked inner
workers.

## Build And Test

Build:

```sh
make build
```

Compile all positive SysY tests:

```sh
make compile
SYSY_OPT='-O1' make compile
```

Run all executable tests:

```sh
make run
SYSY_OPT='-O1' make run
make sysy-parallel-plan-regression
```

Run one test:

```sh
make run-one test/functional/95_float.sy
```

Run native parallel regression:

```sh
THREADS=2 make sysy-parallel-native-regression
```

The run scripts compile to AArch64 assembly, link with
`clang --target=aarch64-linux-gnu`, execute using `qemu-aarch64 -L
/usr/aarch64-linux-gnu`, append the process return code, and compare exact
output against sibling `.out` files.

Mac/Linux-ARM timing history is kept in
`contest-docs/performance-timings.md`. That ledger records revision, affinity,
raw samples, and session boundaries so unrelated host-load samples are not
presented as optimization speedups.
`test/performance` is the canonical preliminary performance root. The Mac
distributed regression script builds once, deterministically shards cases into
16 isolated workers inside the 32-vCPU ARM64 VM, and merges exact-output logs;
this avoids both repeated builds and shared `/tmp` collisions.

## Remaining Work

The branch is now correctness-oriented AArch64 native. The main remaining work
is performance:

- extend injective register residency into liveness-based allocation and keep
  float values in FP registers;
- add scaled-index, pointer-induction, and post-allocation peepholes;
- support multi-reduction loops with a richer native runtime ABI;
- define an explicit reassociation mode before parallelizing floating-point
  reductions; exact-output mode keeps them sequential;
- generalize non-unit-step lowering beyond compile-time iteration spaces and
  broaden safe loop-local control flow beyond canonical early continue without
  admitting candidate-loop exits;
- improve range/alias reasoning for different-rank parameters, globals, and
  provably disjoint affine partitions;
- harden and profitably select the imported loop transforms, trace layout, and
  textual peepholes before enabling any of them by default;
- tune the remaining dynamic profitability model on Cortex-A53 hardware;
- consider loop unrolling and NEON lowering.
