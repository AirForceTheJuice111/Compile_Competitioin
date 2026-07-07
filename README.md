# SysY2022 Contest Compiler Branch

This repository root is the 2026 compiler-contest migration branch. The public
contest entry is:

```sh
build/compiler -S -o output.s input.sy
```

The old FMJ frontend, AST, interpreter, and course-only tools have been removed
from this branch. The contest-facing executable, tests, runtime library, and
regression targets are SysY2022-oriented.

## Environment

Expected tools on x86_64 Linux:

- `make`, `cmake`, `ninja`
- `arm-linux-gnueabihf-gcc`
- `qemu-arm`
- `unzip` for performance archive regression

## Build

```sh
make build
```

This builds `build/compiler`, the SysY2022 contest entry accepting
`-S -o out.s in.sy`.

The current SysY entry uses the native SysY lexer, parser, semantic checker,
lowering code, and the migrated Tree/Quad/SSA/ARM backend by default. Current
debug hooks:

```sh
build/compiler --dump-tokens test/functional/95_float.sy
build/compiler --dump-ast test/functional/95_float.sy
build/compiler --check-sysy test/functional/95_float.sy
make sysy-parse-regression
make sysy-semantic-regression
```

The default ARM backend path can be exercised directly:

```sh
build/compiler -S -o /tmp/native.s test/functional/00_main.sy
```

This path lowers SysY AST directly into the migrated Tree/Quad/SSA/optimizer/
instruction-selection/register-allocation backend. It covers the official
functional SysY subset, including scalar and array `int`/`float`, implicit
int/float conversions, float arithmetic and comparisons, user function calls
with float parameters/returns, and the runtime calls `getfloat`, `getfarray`,
`putfloat`, `putfarray`, and `putf` format strings. Float values are represented
inside Tree/Quad as 32-bit IEEE-754 bit patterns; instruction selection lowers
float operations through the ARM EABI helpers, bridges hard-float runtime calls
with VFP moves, and promotes `putf` `%f` varargs to ARM AAPCS double-word
arguments. The old ARM GCC bridge is still available for debugging:

```sh
build/compiler --gcc-bridge -S -O0 -o /tmp/bridge.s test/functional/00_main.sy
```

## Compile SysY Tests

```sh
make compile
```

This recursively compiles every `.sy` file under `test/` and writes ARM
assembly under `output/`. `SYSY_OPT` is empty by default. Use it only when an
extra compiler flag or the bridge fallback is needed:

```sh
make compile SYSY_OPT="--gcc-bridge -O2"
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
`functional.zip`, local semantic rejects, and the ARM final performance cases:

- `test/functional`: 100 normal functional cases plus local `putf` coverage.
- `test/h_functional`: 40 hidden-style functional cases.
- `test/performance_final`: 60 ARM final performance cases.
- `test/reject`: 12 negative semantic tests marked with `EXPECT: FAIL`.

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

## Performance Tests

`test/performance_final/` contains the ARM final performance cases currently
available from the contest reference bundle. The regression target can also
consume an official archive on demand:

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
summary: total=201 pass=201 compile_fail=0 out_dir=.../output

make run-one test/functional/100_putf.sy
return_code: 9
expect: PASS

make run
summary: total=201 pass=201 compile_fail=0 link_fail=0 run_fail=0 wrong=0

make sysy-parse-regression
summary: total=213 pass=213 parse_fail=0

make sysy-semantic-regression
summary: total=213 pass=201 expected_fail=12 semantic_fail=0

native backend path
SYSY_TEST_ROOT=.../test bash scripts/sysy_functional_regression.sh
summary: total=201 pass=201 compile_fail=0 link_fail=0 run_fail=0 wrong=0

SYSY_PERF_ARCHIVE=/tmp/compiler2025/ARM-性能.zip make sysy-performance-regression
summary: total=59 pass=59 compile_fail=0 link_fail=0 run_fail=0 wrong=0

SYSY_PERF_ARCHIVE=/tmp/compiler2025/ARM决赛性能用例.zip make sysy-performance-regression
summary: total=60 pass=60 compile_fail=0 link_fail=0 run_fail=0 wrong=0
```

## Migration Status

The contest-facing path is native SysY by default. The old FMJ parser, AST,
interpreter, XML AST bridge, and course harness scripts have been removed;
remaining work is mainly deeper optimization tuning.
