# Final FMJ Compiler

This directory is the submission package root for the final project. It contains
the integrated FMJ compiler, tests, build scripts, and report.

## Environment

The expected environment is a 64-bit Ubuntu 20.04 or newer Linux system on x86,
with the course toolchain installed:

- `make`, `cmake`, `ninja`
- `flex`, `bison`
- `arm-linux-gnueabihf-gcc`
- `qemu-arm`

## Build

```sh
make build
```

This builds:

- `build/fmjcc`: FMJ compiler
- `build/fmjinterp`: interpreter used by regression tests

## Compile Tests

```sh
make compile
```

By default this compiles every `.fmj` file under `test/submit` in all required
optimization modes and writes results under `output/<mode>/`.

The default modes are:

- `none`: no optimization
- `const`: Constant Propagation
- `loop1`: Loop Invariant Hoisting
- `loop2`: Induction Variable & Strength Reduction
- `allloop`: Loop Invariant Hoisting plus Induction Variable & Strength Reduction
- `allopt`: all optimizations

Useful variables:

```sh
make compile TEST_DIR=test/submit OUT_DIR=output K=9
```

Each compiled source produces, at minimum:

- `.1.fmj`: source print with comments removed
- `.2-semant.ast`: AST with semantic information
- `.3.irp`: IR+ tree
- `.4.quad`: Quad program
- `.4-ssa.quad`: SSA Quad program
- `.4-ssa-final-<mode>.quad`: final Quad after the requested optimization mode
- `.<mode>.s`: final ARM assembly for that mode

Additional XML and diagnostic files are also emitted.

## Run Tests

The following targets compile and run `TEST_DIR` using qemu:

```sh
make run          # no optimization
make run-const    # Constant Propagation
make run-loop1    # Loop Invariant Hoisting
make run-loop2    # Induction Variable & Strength Reduction
make run-allloop  # both loop optimizations
make run-allopt   # all optimizations
```

Input for programs using `getint`, `getch`, or `getarray` can be supplied with:

```sh
make run-allopt INPUT="4 4 4 4"
```

## Compile One Program

```sh
build/fmjcc --k 9 --opt-mode allopt path/to/program.fmj
build/fmjcc --k 9 --opt-mode none path/to/program.fmj
```

Supported `--opt-mode` values are `none`, `const`, `loop1`, `loop2`, `allloop`,
and `allopt`. `--no-opt` is an alias for `--opt-mode none`.

To run one FMJ file through the compiler, linker, and qemu:

```sh
make run-one path/to/program.fmj
```

## Regression Tests

The following targets are not required by the final-project Makefile contract,
but are kept for validation:

```sh
make compile-regression
make interpreter-regression
make runtime-regression
make fuzz-regression
```

`test/all` contains the full collected HW/PARSING test corpus, including tests
that are expected to be rejected. `test/submit` contains valid programs used by
the default final-project `compile` and `run*` targets.
