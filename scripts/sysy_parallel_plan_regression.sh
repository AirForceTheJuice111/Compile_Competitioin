#!/usr/bin/env bash
set -euo pipefail

COMPILER=${COMPILER:-"$(pwd)/build/compiler"}
TEST_ROOT=${SYSY_TEST_ROOT:-"$(pwd)/test/functional"}
WORK_DIR=${WORK_DIR:-/tmp/sysy_parallel_plan_regression}

rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"

if [[ ! -x "$COMPILER" ]]; then
  echo "missing compiler: $COMPILER" >&2
  exit 2
fi

assert_plan() {
  local case_name=$1
  local loop_line=$2
  local expected_valid=$3
  local expected_reject=${4:-}
  local expected_private=${5:-}
  local expected_private_inclusive=${6:-}
  local source=${7:-"$TEST_ROOT/$case_name.sy"}
  local dump="$WORK_DIR/$case_name.jsonl"
  "$COMPILER" --dump-parallel-plan "$source" >"$dump"
  python3 - "$dump" "$loop_line" "$expected_valid" "$expected_reject" \
    "$expected_private" "$expected_private_inclusive" <<'PY'
import json
import sys

(
    dump,
    loop_line,
    expected_valid,
    expected_reject,
    expected_private,
    expected_private_inclusive,
) = sys.argv[1:]
loop_line = int(loop_line)
expected_valid = expected_valid == "true"
rows = [json.loads(line) for line in open(dump, encoding="utf-8") if line.strip()]
matches = [row for row in rows if row["loop_line"] == loop_line]
if len(matches) != 1:
    raise SystemExit(f"{dump}: expected one plan at loop line {loop_line}, got {len(matches)}")
row = matches[0]
if row["valid"] != expected_valid:
    raise SystemExit(f"{dump}:{loop_line}: valid={row['valid']}, expected {expected_valid}")
if expected_reject and expected_reject not in row["reject"]:
    raise SystemExit(
        f"{dump}:{loop_line}: reject={row['reject']!r}, expected substring {expected_reject!r}"
    )
if expected_private:
    matches = [
        private
        for private in row.get("privatized_scalars", [])
        if private.get("name") == expected_private
    ]
    if len(matches) != 1:
        raise SystemExit(
            f"{dump}:{loop_line}: expected one privatized scalar {expected_private!r}, "
            f"got {matches!r}"
        )
    private = matches[0]
    if private.get("type") != "int" or private.get("kind") != "canonical_nested_iv":
        raise SystemExit(
            f"{dump}:{loop_line}: unexpected privatized scalar metadata {private!r}"
        )
    if expected_private_inclusive:
        expected_inclusive = expected_private_inclusive == "true"
        if private.get("inclusive_end") != expected_inclusive:
            raise SystemExit(
                f"{dump}:{loop_line}: private inclusive_end="
                f"{private.get('inclusive_end')!r}, expected {expected_inclusive}"
            )
PY
}

assert_workers() {
  local case_name=$1
  local expected=$2
  local source=${3:-"$TEST_ROOT/$case_name.sy"}
  local asm="$WORK_DIR/$case_name.s"
  "$COMPILER" --parallel-native -O1 -S -o "$asm" "$source"
  local actual
  actual=$(grep -E -c '^[[:space:]]*\.global[[:space:]]+__sysy_parallel_worker_' "$asm" || true)
  if [[ "$actual" -ne "$expected" ]]; then
    echo "$case_name: workers=$actual, expected=$expected" >&2
    exit 1
  fi
}

assert_plan 101_parallel_le 4 true
assert_plan 104_parallel_bound_invariance 5 false "induction variable in loop endpoint"
assert_plan 105_parallel_reduction_constraints 6 false "unsafe scalar write"
assert_plan 105_parallel_reduction_constraints 13 false "non-reduction use"
assert_plan 106_parallel_shadow_scope 6 false "unsafe scalar write"
assert_plan 107_parallel_nested_if 5 true
assert_plan 108_parallel_float_bound 5 false "non-int loop endpoint"
assert_plan 109_parallel_dynamic_le 6 true
assert_plan 110_parallel_bound_effects 11 false "modified in body"
assert_plan 110_parallel_bound_effects 26 false "array access in loop endpoint"
assert_plan 110_parallel_bound_effects 38 false "call in loop endpoint"
assert_plan 111_parallel_scratch_iv 6 true "" j false
assert_plan 111_parallel_scratch_iv 22 true "" j true
assert_plan 111_parallel_scratch_iv 38 true "" j false
assert_plan 112_parallel_scratch_iv_reject 6 false "unsafe scalar write"
assert_plan 113_parallel_pure_calls 17 true
assert_plan 114_parallel_impure_calls 14 false "unsafe call in loop body"
assert_plan 114_parallel_impure_calls 22 false "unsafe call in loop body"
assert_plan 114_parallel_impure_calls 30 false "unsafe call in loop body"
assert_plan 115_parallel_cost_expensive 5 true
assert_plan 116_parallel_cost_cheap 4 true
assert_plan 117_parallel_step 4 true
assert_plan 118_parallel_decrement 4 true
assert_plan 119_parallel_ne 4 true
assert_plan 119_parallel_ne 18 false "!= endpoint is not reached exactly"
assert_plan 119_parallel_ne 28 true
assert_plan 119_parallel_ne 38 false "may overflow"
assert_plan 127_parallel_general_alias 3 true
assert_plan 129_parallel_mod_reduction 10 true
assert_plan 129_parallel_mod_reduction 20 true
assert_plan 130_parallel_mod_reject 18 false "unsafe scalar write"
assert_plan 130_parallel_mod_reject 23 false "unsafe scalar write"
assert_plan 130_parallel_mod_reject 28 false "array-free"
assert_plan 130_parallel_mod_reject 33 false "unsafe call"
assert_plan 130_parallel_mod_reject 38 false "call reads loop-written scalar"
assert_plan 133_parallel_local_partition_overlap 5 false "non-affine array write"
assert_plan 134_parallel_affine_partition_control 5 true
assert_plan 135_parallel_continue 6 true
assert_plan 135_parallel_continue 23 true
assert_plan 135_parallel_continue 37 true
assert_plan 136_parallel_continue_reject 5 false "canonical induction update"
assert_plan 136_parallel_continue_reject 17 false "canonical induction update"
assert_plan 136_parallel_continue_reject 30 false "canonical induction update"
assert_plan 136_parallel_continue_reject 44 false "break exits candidate loop"
assert_plan 136_parallel_continue_reject 56 false "return in loop body"
assert_plan 136_parallel_continue_reject 68 false "return in loop body"
assert_plan 137_write_only_global 6 true
assert_plan 157_parallel_multi_reduction 10 true
assert_plan 158_parallel_nested_array_iv 8 true
assert_plan 159_parallel_multi_nested 8 true
assert_plan 160_parallel_column_partition 5 true
assert_plan 161_parallel_dynamic_stride 4 true
assert_plan 161_parallel_dynamic_stride 18 true
assert_plan 161_parallel_dynamic_stride 32 true
assert_plan 162_parallel_minmax_reduction 5 true
assert_plan 162_parallel_minmax_reduction 21 true
assert_plan 163_parallel_float_reduction 19 true
assert_plan 164_parallel_dynamic_step 5 true
assert_plan 164_parallel_dynamic_step 16 true
assert_plan 165_parallel_mod_chain 6 true
assert_plan 2025-O30-49 24 true "" "" "" \
  "$TEST_ROOT/../performance_final/2025-O30-49.sy"
assert_plan 2025-MYO-20 92 true "" i false \
  "$TEST_ROOT/../performance_final/2025-MYO-20.sy"
assert_plan 2025-680-52 76 true "" j false \
  "$TEST_ROOT/../performance_final/2025-680-52.sy"
assert_plan 2025-D6H-55 69 true "" "" "" \
  "$TEST_ROOT/../performance_final/2025-D6H-55.sy"
assert_plan 2025-D6H-55 75 true "" "" "" \
  "$TEST_ROOT/../performance_final/2025-D6H-55.sy"
assert_plan 2025-4W1-32 58 true "" "" "" \
  "$TEST_ROOT/../performance_final/2025-4W1-32.sy"

assert_workers 101_parallel_le 1
assert_workers 104_parallel_bound_invariance 0
assert_workers 105_parallel_reduction_constraints 0
assert_workers 106_parallel_shadow_scope 0
assert_workers 107_parallel_nested_if 2
assert_workers 108_parallel_float_bound 0
assert_workers 109_parallel_dynamic_le 1
assert_workers 110_parallel_bound_effects 0
assert_workers 111_parallel_scratch_iv 3
assert_workers 112_parallel_scratch_iv_reject 0
assert_workers 113_parallel_pure_calls 1
assert_workers 114_parallel_impure_calls 0
assert_workers 115_parallel_cost_expensive 1
assert_workers 116_parallel_cost_cheap 2
assert_workers 117_parallel_step 1
assert_workers 118_parallel_decrement 1
assert_workers 119_parallel_ne 1
assert_workers 127_parallel_general_alias 2
# Both loops are still recognized in the plan dump above.  The second has a
# compile-time empty range (`i == limit == 2`), so SCCP removes its runtime call
# and the default whole-program DCE correctly drops the now-unreferenced worker.
assert_workers 129_parallel_mod_reduction 1
assert_workers 130_parallel_mod_reject 0
assert_workers 133_parallel_local_partition_overlap 0
assert_workers 134_parallel_affine_partition_control 1
assert_workers 135_parallel_continue 3
assert_workers 136_parallel_continue_reject 0
assert_workers 137_write_only_global 0
assert_workers 157_parallel_multi_reduction 1
assert_workers 158_parallel_nested_array_iv 2
assert_workers 159_parallel_multi_nested 1
assert_workers 160_parallel_column_partition 1
assert_workers 161_parallel_dynamic_stride 3
assert_workers 162_parallel_minmax_reduction 2
assert_workers 163_parallel_float_reduction 1
assert_workers 164_parallel_dynamic_step 2
assert_workers 165_parallel_mod_chain 1
assert_workers 138_write_only_global_reject 0
assert_workers 2025-O30-49 1 \
  "$TEST_ROOT/../performance_final/2025-O30-49.sy"
assert_workers 2025-D6H-55 2 \
  "$TEST_ROOT/../performance_final/2025-D6H-55.sy"
assert_workers 2025-4W1-32 3 \
  "$TEST_ROOT/../performance_final/2025-4W1-32.sy"

# The dynamic <= worker must guard normalization overflow and preserve the
# original comparator on its sequential INT_MAX path.
if ! grep -Eq 'movk[[:space:]]+w[0-9]+, #32767, lsl #16' "$WORK_DIR/109_parallel_dynamic_le.s" ||
   ! grep -Eq 'b\.le[[:space:]]' "$WORK_DIR/109_parallel_dynamic_le.s"; then
  echo "109_parallel_dynamic_le: missing INT_MAX/original-<= fallback guard" >&2
  exit 1
fi

# Runtime profitability uses a 64-bit trip*cost product, so expensive short
# ranges can create a worker while cheap dynamic ranges retain direct fallback.
if ! grep -Eq 'umull[[:space:]]+x[0-9]+, w[0-9]+, w[0-9]+' \
      "$WORK_DIR/115_parallel_cost_expensive.s" ||
   ! grep -Eq 'movz[[:space:]]+x[0-9]+, #16384' \
      "$WORK_DIR/116_parallel_cost_cheap.s"; then
  echo "parallel work gate: missing 64-bit product/threshold" >&2
  exit 1
fi

# Fully-indexed, effect-free stores to an otherwise unobserved global array
# must disappear before parallel worker capture, together with the storage.
# Reads, address escapes, and effectful store expressions retain their arrays.
if grep -q '__sysy_global_write_only' \
      "$WORK_DIR/137_write_only_global.s" ||
   grep -q '__sysy_parallel_worker_' \
      "$WORK_DIR/137_write_only_global.s"; then
  echo "137_write_only_global: dead storage or worker survived" >&2
  exit 1
fi
for symbol in readback escaped effect_only; do
  if ! grep -Eq "^[[:space:]]*\\.global[[:space:]]+__sysy_global_${symbol}$" \
        "$WORK_DIR/138_write_only_global_reject.s"; then
    echo "138_write_only_global_reject: eliminated live/effectful $symbol" >&2
    exit 1
  fi
done

# Exact emitted costs cover both the boosted depth-0 model and discounting for
# helpers repeatedly invoked inside sequential loops.
if ! grep -Eq 'movz[[:space:]]+w4, #166' \
      "$WORK_DIR/115_parallel_cost_expensive.s" ||
   ! grep -Eq 'movz[[:space:]]+w4, #7' \
      "$WORK_DIR/116_parallel_cost_cheap.s" ||
   ! grep -Eq 'movz[[:space:]]+w4, #67' \
      "$WORK_DIR/2025-4W1-32.s" ||
   ! grep -Eq 'movz[[:space:]]+w4, #73' \
      "$WORK_DIR/2025-D6H-55.s"; then
  echo "parallel work gate: unexpected emitted cost/depth discount" >&2
  exit 1
fi

echo "parallel plan regression: PASS"
