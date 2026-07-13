#!/usr/bin/env bash
set -euo pipefail

src=${1:-}
if [[ -z "$src" ]]; then
  echo "usage: make run-one path/to/file.sy" >&2
  exit 2
fi

COMPILER=${COMPILER:-"$(pwd)/build/compiler"}
AARCH64_CC=${AARCH64_CC:-$(command -v clang || command -v clang-18 || command -v clang-17 || true)}
AARCH64_CC_FLAGS=${AARCH64_CC_FLAGS:---target=aarch64-linux-gnu}
QEMU_AARCH64=${QEMU_AARCH64:-qemu-aarch64}
SYSY_AARCH64_SYSROOT=${SYSY_AARCH64_SYSROOT:-/usr/aarch64-linux-gnu}
LIBSYSY_AARCH64_C=${LIBSYSY_AARCH64_C:-"$(pwd)/vendor/libsysy/sylib.c"}
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

if [[ -z "$AARCH64_CC" || -z "$(command -v "$AARCH64_CC" 2>/dev/null || true)" ]]; then
  echo "missing AArch64 C compiler: set AARCH64_CC (tried clang, clang-18, clang-17)" >&2
  exit 2
fi

if [[ ! -f "$LIBSYSY_AARCH64_C" ]]; then
  echo "missing AArch64 SysY runtime C: $LIBSYSY_AARCH64_C" >&2
  exit 2
fi

rm -rf "$RUN_WORK"
mkdir -p "$RUN_WORK"

base=$(basename "$src" .sy)
asm="$RUN_WORK/$base.s"
exe="$RUN_WORK/$base.aarch64"
stdout_file="$RUN_WORK/$base.stdout"
stderr_file="$RUN_WORK/$base.stderr"
actual_file="$RUN_WORK/$base.actual"
input_file="${src%.sy}.in"
expected_file="${src%.sy}.out"

echo "source: $src"
read -r -a sysy_opt_args <<< "$SYSY_OPT"
"$COMPILER" -S "${sysy_opt_args[@]}" -o "$asm" "$src"
needs_parallel_runtime=0
if grep -Eq '^[[:space:]]*bl[[:space:]]+__sysy_parallel_(for_range|reduce_int_range)' "$asm"; then
  needs_parallel_runtime=1
fi
read -r -a aarch64_cc_flags <<< "$AARCH64_CC_FLAGS"
link_cmd=("$AARCH64_CC" "${aarch64_cc_flags[@]}" -o "$exe" "$asm" "$LIBSYSY_AARCH64_C" -lm)
if [[ "$needs_parallel_runtime" == 1 ]]; then
  link_cmd+=(-pthread)
fi
"${link_cmd[@]}"

if [[ -f "$input_file" ]]; then
  set +e
  "$QEMU_AARCH64" -L "$SYSY_AARCH64_SYSROOT" "$exe" <"$input_file" >"$stdout_file" 2>"$stderr_file"
  rc=$?
  set -e
else
  set +e
  "$QEMU_AARCH64" -L "$SYSY_AARCH64_SYSROOT" "$exe" >"$stdout_file" 2>"$stderr_file"
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
