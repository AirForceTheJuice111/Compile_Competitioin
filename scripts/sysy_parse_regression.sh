#!/usr/bin/env bash
set -euo pipefail

COMPILER=${COMPILER:-"$(pwd)/build/compiler"}
SYSY_TEST_ROOT=${SYSY_TEST_ROOT:-"$(pwd)/test"}

if [[ ! -x "$COMPILER" ]]; then
  echo "missing compiler: $COMPILER" >&2
  exit 2
fi

if [[ ! -d "$SYSY_TEST_ROOT" ]]; then
  echo "missing SysY test root: $SYSY_TEST_ROOT" >&2
  exit 2
fi

total=0
passed=0
failed=0

mapfile -t cases < <(find "$SYSY_TEST_ROOT" -type f -name '*.sy' | sort)

for sy in "${cases[@]}"; do
  total=$((total + 1))
  rel=${sy#"$SYSY_TEST_ROOT"/}
  printf '[%03d] %-55s ' "$total" "$rel"
  if "$COMPILER" --dump-ast "$sy" >/dev/null; then
    echo "PASS"
    passed=$((passed + 1))
  else
    echo "PARSE_FAIL"
    failed=$((failed + 1))
  fi
done

echo "summary: total=$total pass=$passed parse_fail=$failed"
if [[ "$failed" -ne 0 ]]; then
  exit 1
fi
