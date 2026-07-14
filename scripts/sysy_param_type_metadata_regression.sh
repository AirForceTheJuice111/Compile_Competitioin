#!/usr/bin/env bash
set -euo pipefail

COMPILER=${COMPILER:-"$(pwd)/build/compiler"}
TEST_ROOT=${SYSY_TEST_ROOT:-"$(pwd)/test/functional"}
WORK_DIR=${WORK_DIR:-/tmp/sysy_param_type_metadata_regression}
CASE=${PARAM_METADATA_CASE:-"$TEST_ROOT/149_aarch64_mixed_many_params.sy"}

rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"

if [[ ! -x "$COMPILER" ]]; then
  echo "missing compiler: $COMPILER" >&2
  exit 2
fi
if [[ ! -f "$CASE" ]]; then
  echo "missing parameter metadata case: $CASE" >&2
  exit 2
fi

prefix="$WORK_DIR/mixed-many"
"$COMPILER" --debug-prefix "$prefix" -S -o "$prefix.s" "$CASE"

python3 - "$prefix" <<'PY'
import pathlib
import re
import sys

prefix = pathlib.Path(sys.argv[1])
stages = (".4.quad", ".4-block.quad", ".4-ssa.quad", ".4-ssa-final.quad")
expected = [
    "int", "float", "float",
    "int", "float",
    "int", "float",
    "int", "float",
    "int", "float",
    "int", "float",
    "int", "float",
    "int", "float",
    "int", "float",
    "int", "float", "float",
]

for suffix in stages:
    dump = pathlib.Path(str(prefix) + suffix)
    lines = dump.read_text(encoding="utf-8").splitlines()
    signatures = [
        line for line in lines if line.startswith("Function mixed_many_params(")
    ]
    if len(signatures) != 1:
        raise SystemExit(
            f"{dump}: expected one mixed_many_params signature, got {len(signatures)}"
        )
    actual = re.findall(r"t[0-9]+:(int|float|ptr)", signatures[0])
    if actual != expected:
        raise SystemExit(
            f"{dump}: parameter types changed\n"
            f"  actual:   {actual}\n"
            f"  expected: {expected}"
        )

assembly = pathlib.Path(str(prefix) + ".s").read_text(encoding="utf-8")
caller_match = re.search(
    r"(?ms)^main:\n(?P<body>.*?)^[ \t]*bl[ \t]+mixed_many_params[ \t]*$",
    assembly,
)
if caller_match is None:
    raise SystemExit("assembly: could not find the call to mixed_many_params in main")
caller = caller_match.group("body")
fp_registers = re.findall(r"(?m)^[ \t]*fmov[ \t]+s([0-7]),", caller)
if fp_registers != list("01234567"):
    raise SystemExit(
        "assembly: FP register arguments must consume s0-s7, including unused "
        f"float slots; got {fp_registers}"
    )
fp_stack_offsets = {
    int(offset)
    for offset in re.findall(
        r"(?m)^[ \t]*str[ \t]+s[0-9]+,[ \t]*\[sp,[ \t]*#([0-9]+)\]",
        caller,
    )
}
expected_fp_stack_offsets = {0, 16, 32, 40}
if not expected_fp_stack_offsets.issubset(fp_stack_offsets):
    raise SystemExit(
        "assembly: expected ninth-through-twelfth FP arguments at stack offsets "
        f"{sorted(expected_fp_stack_offsets)}, got {sorted(fp_stack_offsets)}"
    )

callee_match = re.search(
    r"(?ms)^mixed_many_params:\n(?P<body>.*?)"
    r"^mixed_many_params\$L[0-9]+:",
    assembly,
)
if callee_match is None:
    raise SystemExit("assembly: could not isolate mixed_many_params prologue")
callee = callee_match.group("body")
fp_sources = re.findall(
    r"(?m)^[ \t]*fmov[ \t]+(?:s[0-9]+|w[0-9]+),[ \t]*s([0-7])[ \t]*$",
    callee,
)
if fp_sources != list("01234567"):
    raise SystemExit(
        "assembly: callee must consume s0-s7, including unused float slots; "
        f"got {fp_sources}"
    )
stack_fp_loads = {
    int(offset)
    for offset in re.findall(
        r"(?m)^[ \t]*add[ \t]+x16,[ \t]*x29,[ \t]*#([0-9]+)\n"
        r"[ \t]*ldr[ \t]+s[0-9]+,[ \t]*\[x16\]",
        callee,
    )
}
# f10 at x29+48 is intentionally unused.  f11 must remain at x29+56 rather
# than sliding into its slot, which is the failure this regression targets.
expected_stack_fp_loads = {16, 32, 56}
if not expected_stack_fp_loads.issubset(stack_fp_loads):
    raise SystemExit(
        "assembly: callee FP stack positions lost unused-float spacing; "
        f"expected {sorted(expected_stack_fp_loads)}, got {sorted(stack_fp_loads)}"
    )

print(
    "parameter metadata regression: PASS "
    "(Tree/Quad/block/SSA/final SSA/AArch64 ABI)"
)
PY
