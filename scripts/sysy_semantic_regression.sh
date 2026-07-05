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
expected_fail_passed=0

mapfile -t cases < <(find "$SYSY_TEST_ROOT" -type f -name '*.sy' | sort)

for sy in "${cases[@]}"; do
  total=$((total + 1))
  rel=${sy#"$SYSY_TEST_ROOT"/}
  expect=PASS
  if sed -n '1,5p' "$sy" | grep -q 'EXPECT:[[:space:]]*FAIL'; then
    expect=FAIL
  fi
  printf '[%03d] %-55s ' "$total" "$rel"
  if "$COMPILER" --check-sysy "$sy" >/dev/null 2>&1; then
    if [[ "$expect" == "FAIL" ]]; then
      echo "UNEXPECTED_PASS"
      failed=$((failed + 1))
    else
      echo "PASS"
      passed=$((passed + 1))
    fi
  else
    if [[ "$expect" == "FAIL" ]]; then
      echo "EXPECTED_FAIL"
      expected_fail_passed=$((expected_fail_passed + 1))
    else
      echo "SEMANTIC_FAIL"
      failed=$((failed + 1))
    fi
  fi
done

echo "summary: total=$total pass=$passed expected_fail=$expected_fail_passed semantic_fail=$failed"
if [[ "$failed" -ne 0 ]]; then
  exit 1
fi
