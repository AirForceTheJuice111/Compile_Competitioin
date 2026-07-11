#!/usr/bin/env bash
set -euo pipefail

SYSY_PERF_ARCHIVE=${SYSY_PERF_ARCHIVE:-/tmp/compiler2025/ARM-性能.zip}
WORK_DIR=${WORK_DIR:-/tmp/sysy_performance_regression}
EXTRACT_DIR="$WORK_DIR/extracted"

if [[ ! -f "$SYSY_PERF_ARCHIVE" ]]; then
  echo "missing performance archive: $SYSY_PERF_ARCHIVE" >&2
  exit 2
fi

rm -rf "$WORK_DIR"
mkdir -p "$EXTRACT_DIR"
unzip -q "$SYSY_PERF_ARCHIVE" -d "$EXTRACT_DIR"

COMPILER=${COMPILER:-"$(pwd)/build/compiler"} \
AARCH64_CC=${AARCH64_CC:-clang} \
AARCH64_CC_FLAGS=${AARCH64_CC_FLAGS:---target=aarch64-linux-gnu} \
QEMU_AARCH64=${QEMU_AARCH64:-qemu-aarch64} \
SYSY_AARCH64_SYSROOT=${SYSY_AARCH64_SYSROOT:-/usr/aarch64-linux-gnu} \
LIBSYSY_AARCH64_C=${LIBSYSY_AARCH64_C:-"$(pwd)/vendor/libsysy/sylib.c"} \
SYSY_TEST_ROOT="$EXTRACT_DIR" \
WORK_DIR="$WORK_DIR/run" \
KEEP_WORK=${KEEP_WORK:-1} \
MAX_CASES=${MAX_CASES:-} \
SYSY_OPT=${SYSY_OPT:-} \
bash scripts/sysy_functional_regression.sh
