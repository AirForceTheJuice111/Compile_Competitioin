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
addressing.

The default optimized pipeline is SCCP, hardened integer AlgebraSimp, loop
passes, typed GVN, CopyProp/conservative DCE, and restricted straight-line
scalar-int inlining. Mutating passes rebuild and verify Quad def/use, CFG,
extent, SSA uniqueness, type, dominance, and PHI-edge metadata. The imported
FuncSpec and MemOpt prototypes stay experimental until their call-arity and
alias/join behavior is fully hardened.

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

The analysis recognizes affine `while` loops with `<` or `<=` invariant integer
upper bounds, disjoint array writes, and strict integer sum reductions. The
inclusive `<=` form is normalized to the half-open AArch64 runtime interval by
evaluating the end expression once and passing `end + 1` to the helper; dynamic
endpoints equal to `INT_MAX` take a generated sequential path with the original
comparison. Bounds containing calls, arrays, the induction variable, or
scalars modified by the loop are rejected. Same-array accesses are
kept parallel only when reads and writes stay in the same first-index partition;
cross-iteration patterns such as `a[i] = a[i + 1]` are rejected. Same-rank
array parameter aliasing no longer forces a static rejection: lowering emits a
runtime pointer guard and falls back to the original sequential loop when the
captured array bases alias. An interprocedural fixed-point summary permits
user helpers proven scalar-only and side-effect-free, including recursion and
immutable scalar constants. Runtime/unknown calls, arrays, mutable globals,
nonlocal writes, and I/O remain impure. Direct pure return expressions are
substituted conservatively into affine index checks, exposing helpers such as
`idx(r, c, n)`. The pass still rejects endpoint calls, control-flow exits,
unsafe scalar writes, unguardable array disjointness, multiple reductions, and
float reductions.

Nested-loop profitability detection is recursive through blocks and
conditionals. Reduction validation also rejects self-dependent accumulators and
uses of a partial accumulator value elsewhere in the loop body.

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
layouts, and runtime helper implementations when needed. Each helper receives
a conservative per-iteration work estimate. Nested bodies are boosted and
helpers inside sequential enclosing loops are depth-discounted; compile-time
and dynamic profitability use an overflow-safe 64-bit `trip_count * work_cost`
threshold of 16384. This admits coarse short ranges without spawning threads
for cheap dynamic reductions or repeatedly invoked inner workers.

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

## Remaining Work

The branch is now correctness-oriented AArch64 native. The main remaining work
is performance:

- extend injective register residency into liveness-based allocation and keep
  float values in FP registers;
- add scaled-index, pointer-induction, and post-allocation peepholes;
- support multi-reduction loops with a richer native runtime ABI;
- support decrement/non-unit-step loops and safe loop-local control flow;
- amortize repeated parallel dispatch with a persistent two-core worker;
- tune the remaining dynamic profitability model on Cortex-A53 hardware;
- consider loop unrolling and NEON lowering.
