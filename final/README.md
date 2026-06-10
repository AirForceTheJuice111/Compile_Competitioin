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

By default this recursively compiles every `.fmj` file under `test/` in all
required optimization modes and writes results under `output/<mode>/`.

The default modes are:

- `none`: no optimization
- `const`: Constant Propagation
- `loop1`: Loop Invariant Hoisting
- `loop2`: Induction Variable & Strength Reduction
- `allloop`: Loop Invariant Hoisting plus Induction Variable & Strength Reduction
- `allopt`: all optimizations

Useful variables:

```sh
make compile TEST_DIR=test OUT_DIR=output K=9
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

The following targets recursively compile every `.fmj` file under `TEST_DIR`
and run each successfully compiled program using qemu. Each target prints a
per-program result line, followed by non-empty stdout/stderr blocks. Programs
rejected by the compiler are reported as `COMPILE_FAIL`.

```sh
make run          # no optimization
make run-const    # Constant Propagation
make run-loop1    # Loop Invariant Hoisting
make run-loop2    # Induction Variable & Strength Reduction
make run-allloop  # both loop optimizations
make run-allopt   # all optimizations
```

By default, batch `run*` targets use their own stdin for qemu. If stdin is a
pipe or redirected file, the script buffers it once and replays the same input
to every program, avoiding pipe read-ahead between qemu processes. For
non-interactive repeated input, the same input can also be supplied with:

```sh
make run-allopt INPUT="4 4 4 4"
```

Directory run and regression qemu timeout defaults to 2 seconds and can be
overridden with:

```sh
make run-allopt RUN_TIMEOUT=5
make run-allopt RUN_TIMEOUT=   # disable timeout
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
make run-one path/to/program.fmj OPT_MODE=none
make run-one path/to/program.fmj OPT_MODE=const
make run-one path/to/program.fmj OPT_MODE=loop1
make run-one path/to/program.fmj OPT_MODE=loop2
make run-one path/to/program.fmj OPT_MODE=allloop
make run-one path/to/program.fmj OPT_MODE=allopt
```

To run one FMJ file in all optimization modes and compare process return code,
stdout, stderr, and the printed FMJ return value:

```sh
make run-one-all-mode path/to/program.fmj
```

`run-one` and `run-one-all-mode` do not provide default stdin. Piped or
redirected stdin is also buffered and replayed to every optimization mode. For
input programs, pass input explicitly:

```sh
make run-one path/to/program.fmj INPUT="1 2 3"
make run-one-all-mode path/to/program.fmj INPUT="1 2 3"
```

`run-one` and `run-one-all-mode` also do not wrap qemu in `timeout` by default.

## Regression Tests

The following targets are not required by the final-project Makefile contract,
but are kept for validation:

```sh
make compile-regression
make interpreter-regression
make runtime-regression
make fuzz-regression
make all-mode-regression
```

`test/` is the default final-project corpus for `compile` and `run*`. It
contains the collected HW/PARSING tests, submit examples, regression cases, and
programs that are expected to be rejected by the compiler.
