#!/usr/bin/env bash
set -u

COMPILER=${COMPILER:-"$(pwd)/build/compiler"}
AARCH64_CC=${AARCH64_CC:-clang}
AARCH64_CC_FLAGS=${AARCH64_CC_FLAGS:---target=aarch64-linux-gnu}
QEMU_AARCH64=${QEMU_AARCH64:-qemu-aarch64}
SYSY_AARCH64_SYSROOT=${SYSY_AARCH64_SYSROOT:-/usr/aarch64-linux-gnu}
LIBSYSY_AARCH64_C=${LIBSYSY_AARCH64_C:-"$(pwd)/vendor/libsysy/sylib.c"}
SYSY_TEST_ROOT=${SYSY_TEST_ROOT:-"$(pwd)/test/performance"}
WORK_DIR=${WORK_DIR:-/tmp/sysy_parallel_native_regression}
KEEP_WORK=${KEEP_WORK:-0}
MAX_CASES=${MAX_CASES:-}
THREADS=${THREADS:-2}

rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"

if [[ ! -x "$COMPILER" ]]; then
  echo "missing compiler: $COMPILER" >&2
  exit 2
fi
if [[ ! -f "$LIBSYSY_AARCH64_C" ]]; then
  echo "missing AArch64 SysY runtime C: $LIBSYSY_AARCH64_C" >&2
  exit 2
fi
if [[ ! -d "$SYSY_TEST_ROOT" ]]; then
  echo "missing SysY test root: $SYSY_TEST_ROOT" >&2
  exit 2
fi

total=0
triggered=0
skipped=0
passed=0
compile_fail=0
link_fail=0
run_fail=0
wrong=0

mapfile -t cases < <(find "$SYSY_TEST_ROOT" -type f -name '*.sy' | sort)

for sy in "${cases[@]}"; do
  if [[ -n "$MAX_CASES" && "$triggered" -ge "$MAX_CASES" ]]; then
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
  exe="$WORK_DIR/$safe.aarch64"
  stdout_file="$WORK_DIR/$safe.stdout"
  stderr_file="$WORK_DIR/$safe.stderr"
  actual_norm="$WORK_DIR/$safe.actual"
  expected_norm="$WORK_DIR/$safe.expected"
  input_file="${sy%.sy}.in"

  if ! "$COMPILER" --parallel-native -S -O1 -o "$asm" "$sy" >"$WORK_DIR/$safe.compile.out" 2>"$WORK_DIR/$safe.compile.err"; then
    printf '[%03d] %-45s COMPILE_FAIL\n' "$total" "$rel"
    compile_fail=$((compile_fail + 1))
    continue
  fi

  worker_count=$(grep -E -c '^[[:space:]]*\.global[[:space:]]+__sysy_parallel_worker_' "$asm" || true)
  if [[ "$worker_count" -eq 0 ]]; then
    skipped=$((skipped + 1))
    continue
  fi
  triggered=$((triggered + 1))
  printf '[%03d] %-45s workers=%s ' "$total" "$rel" "$worker_count"

  read -r -a aarch64_cc_flags <<< "$AARCH64_CC_FLAGS"
  if ! "$AARCH64_CC" "${aarch64_cc_flags[@]}" -o "$exe" "$asm" "$LIBSYSY_AARCH64_C" -lm -pthread >"$WORK_DIR/$safe.link.out" 2>"$WORK_DIR/$safe.link.err"; then
    echo "LINK_FAIL"
    link_fail=$((link_fail + 1))
    continue
  fi

  if [[ -f "$input_file" ]]; then
    SYSY_THREADS="$THREADS" "$QEMU_AARCH64" -L "$SYSY_AARCH64_SYSROOT" "$exe" <"$input_file" >"$stdout_file" 2>"$stderr_file"
  else
    SYSY_THREADS="$THREADS" "$QEMU_AARCH64" -L "$SYSY_AARCH64_SYSROOT" "$exe" >"$stdout_file" 2>"$stderr_file"
  fi
  rc=$?
  if [[ "$rc" -gt 255 ]]; then
    echo "RUN_FAIL rc=$rc"
    run_fail=$((run_fail + 1))
    continue
  fi

  tr -d '\r' < "$stdout_file" > "$actual_norm"
  if [[ -s "$actual_norm" ]]; then
    last_byte=$(tail -c 1 "$actual_norm" | od -An -t u1 | tr -d ' ')
    if [[ "$last_byte" != "10" ]]; then
      printf '\n' >>"$actual_norm"
    fi
  fi
  printf '%s\n' "$rc" >>"$actual_norm"

  tr -d '\r' < "$expected_file" > "$expected_norm"
  if [[ -s "$expected_norm" ]]; then
    expected_last_byte=$(tail -c 1 "$expected_norm" | od -An -t u1 | tr -d ' ')
    if [[ "$expected_last_byte" != "10" ]]; then
      printf '\n' >>"$expected_norm"
    fi
  fi

  if cmp -s "$actual_norm" "$expected_norm"; then
    echo "PASS"
    passed=$((passed + 1))
  else
    echo "WRONG"
    wrong=$((wrong + 1))
    diff -u "$expected_norm" "$actual_norm" >"$WORK_DIR/$safe.diff" || true
  fi
done

echo "summary: scanned=$total triggered=$triggered skipped=$skipped pass=$passed compile_fail=$compile_fail link_fail=$link_fail run_fail=$run_fail wrong=$wrong work_dir=$WORK_DIR"
if [[ "$KEEP_WORK" != 1 && "$compile_fail" -eq 0 && "$link_fail" -eq 0 && "$run_fail" -eq 0 && "$wrong" -eq 0 ]]; then
  rm -rf "$WORK_DIR"
else
  echo "work dir: $WORK_DIR"
fi

if [[ "$compile_fail" -ne 0 || "$link_fail" -ne 0 || "$run_fail" -ne 0 || "$wrong" -ne 0 ]]; then
  exit 1
fi
