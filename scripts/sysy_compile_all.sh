#!/usr/bin/env bash
set -euo pipefail

COMPILER=${COMPILER:-"$(pwd)/build/compiler"}
SYSY_TEST_ROOT=${SYSY_TEST_ROOT:-"$(pwd)/test"}
OUT_DIR=${OUT_DIR:-"$(pwd)/output"}
SYSY_OPT=${SYSY_OPT:-"-O0"}

if [[ ! -x "$COMPILER" ]]; then
  echo "missing compiler: $COMPILER" >&2
  exit 2
fi

if [[ ! -d "$SYSY_TEST_ROOT" ]]; then
  echo "missing SysY test root: $SYSY_TEST_ROOT" >&2
  exit 2
fi

rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR"

total=0
passed=0
failed=0

mapfile -t cases < <(find "$SYSY_TEST_ROOT" -type f -name '*.sy' | sort)

for sy in "${cases[@]}"; do
  if sed -n '1,5p' "$sy" | grep -q 'EXPECT:[[:space:]]*FAIL'; then
    continue
  fi
  total=$((total + 1))
  rel=${sy#"$SYSY_TEST_ROOT"/}
  stem=${rel%.sy}
  out="$OUT_DIR/$stem.s"
  mkdir -p "$(dirname "$out")"
  printf '[%03d] %-55s ' "$total" "$rel"
  if "$COMPILER" -S "$SYSY_OPT" -o "$out" "$sy" >"$out.compile.out" 2>"$out.compile.err"; then
    echo "PASS"
    passed=$((passed + 1))
  else
    echo "COMPILE_FAIL"
    failed=$((failed + 1))
  fi
done

echo "summary: total=$total pass=$passed compile_fail=$failed out_dir=$OUT_DIR"
if [[ "$failed" -ne 0 ]]; then
  exit 1
fi
