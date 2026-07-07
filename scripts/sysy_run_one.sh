#!/usr/bin/env bash
set -euo pipefail

src=${1:-}
if [[ -z "$src" ]]; then
  echo "usage: make run-one path/to/file.sy" >&2
  exit 2
fi

COMPILER=${COMPILER:-"$(pwd)/build/compiler"}
ARM_CC=${ARM_CC:-arm-linux-gnueabihf-gcc}
QEMU_ARM=${QEMU_ARM:-qemu-arm}
LIBSYSY_ARM=${LIBSYSY_ARM:-"$(pwd)/vendor/libsysy/libsysy_arm.a"}
RUN_WORK=${RUN_WORK:-/tmp/sysy_run_one}
SYSY_OPT=${SYSY_OPT:-}

if [[ ! -f "$src" ]]; then
  echo "missing source: $src" >&2
  exit 2
fi

if [[ ! -x "$COMPILER" ]]; then
  echo "missing compiler: $COMPILER" >&2
  exit 2
fi

if [[ ! -f "$LIBSYSY_ARM" ]]; then
  echo "missing ARM SysY runtime: $LIBSYSY_ARM" >&2
  exit 2
fi

rm -rf "$RUN_WORK"
mkdir -p "$RUN_WORK"

base=$(basename "$src" .sy)
asm="$RUN_WORK/$base.s"
exe="$RUN_WORK/$base.arm"
stdout_file="$RUN_WORK/$base.stdout"
stderr_file="$RUN_WORK/$base.stderr"
actual_file="$RUN_WORK/$base.actual"
input_file="${src%.sy}.in"
expected_file="${src%.sy}.out"

echo "source: $src"
read -r -a sysy_opt_args <<< "$SYSY_OPT"
"$COMPILER" -S "${sysy_opt_args[@]}" -o "$asm" "$src"
"$ARM_CC" -static -mcpu=cortex-a72 -o "$exe" "$asm" "$LIBSYSY_ARM" -lm

if [[ -f "$input_file" ]]; then
  set +e
  "$QEMU_ARM" "$exe" <"$input_file" >"$stdout_file" 2>"$stderr_file"
  rc=$?
  set -e
else
  set +e
  "$QEMU_ARM" "$exe" >"$stdout_file" 2>"$stderr_file"
  rc=$?
  set -e
fi

cp "$stdout_file" "$actual_file"
if [[ -s "$actual_file" ]]; then
  last_byte=$(tail -c 1 "$actual_file" | od -An -t u1 | tr -d ' ')
  if [[ "$last_byte" != "10" ]]; then
    printf '\n' >>"$actual_file"
  fi
fi
printf '%s\n' "$rc" >>"$actual_file"

echo "stdout:"
sed -n '1,120p' "$stdout_file"
if [[ -s "$stderr_file" ]]; then
  echo "stderr:"
  sed -n '1,120p' "$stderr_file"
fi
echo "return_code: $rc"

if [[ -f "$expected_file" ]]; then
  if cmp -s "$actual_file" "$expected_file"; then
    echo "expect: PASS"
  else
    echo "expect: WRONG"
    diff -u "$expected_file" "$actual_file" | sed -n '1,120p'
    exit 1
  fi
fi
