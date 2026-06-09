#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    echo "Usage: scripts/run_submit.sh <none|const|loop1|loop2|allloop|allopt>" >&2
    exit 2
fi

mode="$1"
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
final_dir="$root/final"
out_dir="${OUT_DIR:-$final_dir/output}"
mode_dir="$out_dir/$mode"
map="$mode_dir/map.txt"
cc="${ARM_CC:-arm-linux-gnueabihf-gcc}"
qemu="${QEMU_ARM:-qemu-arm}"
libsysy="${LIBSYSY32:-$root/HW12/vendor/libsysy/libsysy32.s}"
input="${INPUT:-4 4 4 4 4 4 4 4 4 4 4 4}"
timeout_s="${RUN_TIMEOUT:-10}"

case "$mode" in
    none|const|loop1|loop2|allloop|allopt) ;;
    *) echo "Error: unknown run mode: $mode" >&2; exit 2 ;;
esac

for tool in "$cc" "$qemu"; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "Error: required tool not found: $tool" >&2
        exit 2
    fi
done
if [[ ! -f "$libsysy" ]]; then
    echo "Making libsysy32.s..." >&2
    (cd "$root/HW12/vendor/libsysy" && "$cc" -mcpu=cortex-a72 -S libsysy32.c -o libsysy32.s)
fi

if [[ ! -f "$map" ]]; then
    MODE="$mode" "$final_dir/scripts/compile_submit.sh"
fi

pass=0
link_fail=0
run_fail=0
failures="$mode_dir/run-failures.txt"
: > "$failures"

while IFS='|' read -r base asm src; do
    [[ -n "$base" ]] || continue
    arm="$base.$mode.arm"
    set +e
    "$cc" -mcpu=cortex-a72 -Wall -Wextra -Wl,-z,noexecstack --static \
        -o "$arm" "$asm" "$libsysy" -lm > "$base.link.log" 2>&1
    link_rc=$?
    set -e
    if [[ "$link_rc" -ne 0 ]]; then
        link_fail=$((link_fail + 1))
        printf 'LINK_FAIL rc=%s %s\n' "$link_rc" "$src" >> "$failures"
        tail -n 30 "$base.link.log" >> "$failures"
        continue
    fi

    set +e
    printf '%s\n' "$input" | timeout "$timeout_s" "$qemu" "$arm" > "$base.run.out" 2> "$base.run.err"
    run_rc=$?
    set -e
    if [[ "$run_rc" -eq 124 ]] || grep -qiE 'uncaught target signal|Segmentation fault|Illegal instruction|Bus error|Aborted' "$base.run.err"; then
        run_fail=$((run_fail + 1))
        printf 'RUN_FAIL rc=%s %s\n' "$run_rc" "$src" >> "$failures"
        printf 'stdout=' >> "$failures"
        cat "$base.run.out" >> "$failures"
        printf '\nstderr=' >> "$failures"
        cat "$base.run.err" >> "$failures"
        printf '\n' >> "$failures"
    else
        pass=$((pass + 1))
    fi
done < "$map"

total=$(wc -l < "$map")
printf 'mode=%s total=%s pass=%s link_fail=%s run_fail=%s out_dir=%s\n' \
    "$mode" "$total" "$pass" "$link_fail" "$run_fail" "$mode_dir"

if [[ -s "$failures" ]]; then
    cat "$failures"
fi

if [[ "$link_fail" -ne 0 || "$run_fail" -ne 0 ]]; then
    exit 1
fi
