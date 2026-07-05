# SysY2022 Contest Compiler Branch

This repository root is the 2026 compiler-contest migration branch. The public
contest entry is:

```sh
build/compiler -S -o output.s input.sy
```

The repository still contains the old FMJ implementation files during the
migration, but the default contest-facing executable, tests, runtime library,
and regression targets are now SysY2022-oriented.

## Environment

Expected tools on x86_64 Linux:

- `make`, `cmake`, `ninja`
- `flex`, `bison`
- `arm-linux-gnueabihf-gcc`
- `qemu-arm`
- `unzip` for performance archive regression

## Build

```sh
make build
```

This builds:

- `build/compiler`: SysY2022 contest entry, accepting `-S -o out.s in.sy`.
- `build/fmjcc` and `build/fmjinterp` only when `BUILD_LEGACY_FMJ=ON`; these
  legacy FMJ tools are kept as migration scaffolding.

The current SysY entry is a functional migration bridge: it normalizes SysY2022
source where C99 differs from the SysY language model, injects the official
runtime declarations, and asks the ARM GCC toolchain to emit ARM assembly. In
particular it handles the contest runtime functions, `starttime`/`stoptime`
macros, SysY scalar `const int` values used in array dimensions, and SysY's
single-precision floating literal semantics.

The native SysY frontend is being built in parallel under `include/sysy` and
`lib/sysy`. It now includes a lexer, parser, semantic checker, and an
experimental Tree/Quad/backend path. Current debug hooks:

```sh
build/compiler --dump-tokens test/functional/95_float.sy
build/compiler --dump-ast test/functional/95_float.sy
build/compiler --check-sysy test/functional/95_float.sy
make sysy-parse-regression
make sysy-semantic-regression
```

The native ARM backend can be exercised explicitly:

```sh
build/compiler --native-backend -S -O0 -o /tmp/native.s test/functional/00_main.sy
```

This path lowers SysY AST directly into the migrated Tree/Quad/SSA/optimizer/
instruction-selection/register-allocation backend. It currently covers an
integer scalar subset: local scalar declarations, functions, calls, arithmetic,
comparisons, short-circuit conditions, `if`, `while`, `break`, `continue`,
`return`, integer scalar globals, and integer runtime calls such as `getint`,
`putint`, and `putch`. Unsupported features such as arrays, strings/`putf`, and
floats are rejected explicitly by the lowering stage. The default contest
invocation does not use this incomplete native path yet; it keeps the bridge so
the full official functional suite continues to pass.

## Compile SysY Tests

```sh
make compile
```

This recursively compiles every `.sy` file under `test/` and writes ARM
assembly under `output/`. The default optimization flag passed to the bridge is
`SYSY_OPT=-O0`; it can be changed for toolchain-level experiments:

```sh
make compile SYSY_OPT=-O2
```

## Compile One SysY Program

```sh
make build
build/compiler -S -o /tmp/program.s test/functional/00_main.sy
arm-linux-gnueabihf-gcc -static -mcpu=cortex-a72 \
  -o /tmp/program.arm /tmp/program.s vendor/libsysy/libsysy_arm.a -lm
qemu-arm /tmp/program.arm
```

The Makefile wrapper can compile, link, run, and compare one test case:

```sh
make run-one test/functional/95_float.sy
```

If a sibling `.in` file exists, it is fed to qemu. If a sibling `.out` file
exists, the script compares stdout plus the process return-code line against
that expectation.

The vendored runtime files are copied from the official `compiler2025`
repository:

- `vendor/libsysy/libsysy_arm.a`
- `vendor/libsysy/sylib.c`
- `vendor/libsysy/sylib.h`

## SysY Tests

`test/` contains the official functional SysY2022 tests extracted from
`functional.zip`:

- `test/functional`: 100 normal functional cases.
- `test/h_functional`: 40 hidden-style functional cases.

The old FMJ `.fmj` corpus has been removed from `test/` on this branch.

Run all vendored functional tests. `make run` is an alias for the same
regression:

```sh
make run
make sysy-functional-regression
```

The regression script compiles each `.sy`, links with `libsysy_arm.a`, runs the
ARM binary under qemu, appends the process return code as the official harness
does, and compares exact output with the corresponding `.out` file.

To run only part of the set while debugging:

```sh
MAX_CASES=10 make sysy-functional-regression
```

## Performance Archives

Large performance inputs from the official repository are not expanded into
`test/`, because the ARM/RISC-V archives contain hundreds of MB of input/output
data. The regression target consumes an official archive on demand:

```sh
make sysy-performance-regression SYSY_PERF_ARCHIVE=/tmp/compiler2025/ARM-性能.zip
```

Use `MAX_CASES=N` for smoke testing:

```sh
MAX_CASES=3 make sysy-performance-regression
```

## Current Verification

Current checked result on this branch:

```text
make build
build/ contains compiler only by default.

make compile
summary: total=140 pass=140 compile_fail=0 out_dir=.../output

make run-one test/functional/00_main.sy
return_code: 3
expect: PASS

make run
summary: total=140 pass=140 compile_fail=0 link_fail=0 run_fail=0 wrong=0

make sysy-parse-regression
summary: total=151 pass=151 parse_fail=0

make sysy-semantic-regression
summary: total=151 pass=140 expected_fail=11 semantic_fail=0

native backend smoke subset
summary: total=15 pass=14 expected_native_unsupported=1 fail=0

MAX_CASES=3 make sysy-performance-regression
summary: total=3 pass=3 compile_fail=0 link_fail=0 run_fail=0 wrong=0
```

## Migration Status

This branch has not yet completed the full native rewrite described in
`contest-docs/SysY2022-vs-FDMJ2026-migration-plan.md`. The native FMJ parser,
AST, IR, optimizer, and backend files remain in the tree and still need to be
reworked into a self-contained SysY compiler. The current default `compiler`
executable uses native SysY lexing/parsing/semantic validation, but default code
generation still uses the functional bridge. A reusable backend driver and an
experimental `--native-backend` lowering path are present for the integer scalar
subset including scalar globals, and are the base for finishing native arrays,
float/VFP ABI, and full SysY runtime support.
