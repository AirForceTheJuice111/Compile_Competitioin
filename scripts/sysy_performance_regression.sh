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
ARM_CC=${ARM_CC:-arm-linux-gnueabihf-gcc} \
QEMU_ARM=${QEMU_ARM:-qemu-arm} \
LIBSYSY_ARM=${LIBSYSY_ARM:-"$(pwd)/vendor/libsysy/libsysy_arm.a"} \
SYSY_TEST_ROOT="$EXTRACT_DIR" \
WORK_DIR="$WORK_DIR/run" \
KEEP_WORK=${KEEP_WORK:-1} \
MAX_CASES=${MAX_CASES:-} \
SYSY_OPT=${SYSY_OPT:-"-O0"} \
bash scripts/sysy_functional_regression.sh
