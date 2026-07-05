#!/usr/bin/env bash
set -u

COMPILER=${COMPILER:-"$(pwd)/build/compiler"}
ARM_CC=${ARM_CC:-arm-linux-gnueabihf-gcc}
QEMU_ARM=${QEMU_ARM:-qemu-arm}
LIBSYSY_ARM=${LIBSYSY_ARM:-"$(pwd)/vendor/libsysy/libsysy_arm.a"}
SYSY_TEST_ROOT=${SYSY_TEST_ROOT:-/tmp/compiler2025_functional/functional_recover}
WORK_DIR=${WORK_DIR:-/tmp/sysy_functional_regression}
KEEP_WORK=${KEEP_WORK:-0}
MAX_CASES=${MAX_CASES:-}
SYSY_OPT=${SYSY_OPT:-"-O0"}

rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"

if [[ ! -x "$COMPILER" ]]; then
  echo "missing compiler: $COMPILER" >&2
  exit 2
fi

if [[ ! -f "$LIBSYSY_ARM" ]]; then
  echo "missing ARM SysY runtime: $LIBSYSY_ARM" >&2
  exit 2
fi

if [[ ! -d "$SYSY_TEST_ROOT" ]]; then
  echo "missing SysY test root: $SYSY_TEST_ROOT" >&2
  exit 2
fi

total=0
passed=0
compile_fail=0
link_fail=0
run_fail=0
wrong=0

mapfile -t cases < <(find "$SYSY_TEST_ROOT" -type f -name '*.sy' | sort)

for sy in "${cases[@]}"; do
  if [[ -n "$MAX_CASES" && "$total" -ge "$MAX_CASES" ]]; then
    break
  fi

  expected_file="${sy%.sy}.out"
  if [[ ! -f "$expected_file" ]]; then
    continue
  fi

  total=$((total + 1))
  rel=${sy#"$SYSY_TEST_ROOT"/}
  stem=${rel%.sy}
  safe=${stem//\//__}
  asm="$WORK_DIR/$safe.s"
  exe="$WORK_DIR/$safe.arm"
  stdout_file="$WORK_DIR/$safe.stdout"
  stderr_file="$WORK_DIR/$safe.stderr"
  actual_file="$WORK_DIR/$safe.actual"
  input_file="${sy%.sy}.in"

  printf '[%03d] %-55s ' "$total" "$rel"

  if ! "$COMPILER" -S "$SYSY_OPT" -o "$asm" "$sy" >"$WORK_DIR/$safe.compile.out" 2>"$WORK_DIR/$safe.compile.err"; then
    echo "COMPILE_FAIL"
    compile_fail=$((compile_fail + 1))
    continue
  fi

  if ! "$ARM_CC" -static -mcpu=cortex-a72 -o "$exe" "$asm" "$LIBSYSY_ARM" -lm >"$WORK_DIR/$safe.link.out" 2>"$WORK_DIR/$safe.link.err"; then
    echo "LINK_FAIL"
    link_fail=$((link_fail + 1))
    continue
  fi

  if [[ -f "$input_file" ]]; then
    "$QEMU_ARM" "$exe" <"$input_file" >"$stdout_file" 2>"$stderr_file"
  else
    "$QEMU_ARM" "$exe" >"$stdout_file" 2>"$stderr_file"
  fi
  rc=$?
  if [[ "$rc" -gt 255 ]]; then
    echo "RUN_FAIL rc=$rc"
    run_fail=$((run_fail + 1))
    continue
  fi

  cp "$stdout_file" "$actual_file"
  if [[ -s "$actual_file" ]]; then
    last_byte=$(tail -c 1 "$actual_file" | od -An -t u1 | tr -d ' ')
    if [[ "$last_byte" != "10" ]]; then
      printf '\n' >>"$actual_file"
    fi
  fi
  printf '%s\n' "$rc" >>"$actual_file"

  if [[ ! -f "$expected_file" ]]; then
    echo "MISSING_EXPECT"
    wrong=$((wrong + 1))
    continue
  fi

  if cmp -s "$actual_file" "$expected_file"; then
    echo "PASS"
    passed=$((passed + 1))
  else
    echo "WRONG"
    wrong=$((wrong + 1))
    {
      echo "case: $rel"
      echo "expected:"
      sed -n '1,80p' "$expected_file"
      echo "actual:"
      sed -n '1,80p' "$actual_file"
      echo "diff:"
      diff -u "$expected_file" "$actual_file" | sed -n '1,120p'
    } >"$WORK_DIR/$safe.diff"
  fi
done

echo "summary: total=$total pass=$passed compile_fail=$compile_fail link_fail=$link_fail run_fail=$run_fail wrong=$wrong"
if [[ "$KEEP_WORK" != 1 && "$compile_fail" -eq 0 && "$link_fail" -eq 0 && "$run_fail" -eq 0 && "$wrong" -eq 0 ]]; then
  rm -rf "$WORK_DIR"
else
  echo "work dir: $WORK_DIR"
fi

if [[ "$compile_fail" -ne 0 || "$link_fail" -ne 0 || "$run_fail" -ne 0 || "$wrong" -ne 0 ]]; then
  exit 1
fi
