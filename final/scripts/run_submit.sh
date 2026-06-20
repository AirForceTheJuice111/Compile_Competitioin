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
compile_results="$mode_dir/compile-results.txt"
cc="${ARM_CC:-arm-linux-gnueabihf-gcc}"
qemu="${QEMU_ARM:-qemu-arm}"
libsysy="${LIBSYSY32:-$root/HW12/vendor/libsysy/libsysy32.s}"
timeout_set=0
timeout_s=""
if [[ -v RUN_TIMEOUT && -n "$RUN_TIMEOUT" ]]; then
    timeout_set=1
    timeout_s="$RUN_TIMEOUT"
fi
input_set=0
input=""
if [[ -v INPUT ]]; then
    input_set=1
    input="$INPUT"
fi
stdin_file=""
if [[ "$input_set" -eq 0 && ! -t 0 ]]; then
    stdin_file="$(mktemp "${TMPDIR:-/tmp}/fmj-run-stdin.XXXXXX")"
    cat > "$stdin_file"
    trap 'rm -f "$stdin_file"' EXIT
fi

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

print_stream() {
    local label="$1"
    local file="$2"
    if [[ -s "$file" ]]; then
        printf '%s_BEGIN\n' "$label"
        cat "$file"
        printf '\n%s_END\n' "$label"
    fi
}

print_compile_failures() {
    [[ -f "$compile_results" ]] || return 0
    while IFS= read -r line; do
        [[ "$line" == COMPILE_FAIL* ]] || continue
        printf '%s\n' "$line"
    done < "$compile_results"
}

pass=0
compile_fail=0
link_fail=0
run_fail=0
failures="$mode_dir/run-failures.txt"
: > "$failures"

print_compile_failures
if [[ -f "$compile_results" ]]; then
    compile_fail=$(grep -c '^COMPILE_FAIL ' "$compile_results" || true)
fi

while IFS='|' read -r base asm src <&3; do
    [[ -n "$base" ]] || continue
    arm="$base.$mode.arm"
    set +e
    "$cc" -mcpu=cortex-a72 -Wall -Wextra -Wl,-z,noexecstack --static \
        -o "$arm" "$asm" "$libsysy" -lm > "$base.link.log" 2>&1
    link_rc=$?
    set -e
    if [[ "$link_rc" -ne 0 ]]; then
        link_fail=$((link_fail + 1))
        printf 'mode=%s result=LINK_FAIL rc=%s src=%s\n' "$mode" "$link_rc" "$src"
        printf 'LINK_FAIL rc=%s %s\n' "$link_rc" "$src" >> "$failures"
        tail -n 30 "$base.link.log" >> "$failures"
        print_stream "link_stderr" "$base.link.log"
        continue
    fi

    set +e
    if [[ "$input_set" -eq 1 ]]; then
        if [[ "$timeout_set" -eq 1 ]]; then
            printf '%s\n' "$input" | timeout "$timeout_s" "$qemu" "$arm" > "$base.run.out" 2> "$base.run.err"
        else
            printf '%s\n' "$input" | "$qemu" "$arm" > "$base.run.out" 2> "$base.run.err"
        fi
    elif [[ -n "$stdin_file" ]]; then
        if [[ "$timeout_set" -eq 1 ]]; then
            timeout "$timeout_s" "$qemu" "$arm" < "$stdin_file" > "$base.run.out" 2> "$base.run.err"
        else
            "$qemu" "$arm" < "$stdin_file" > "$base.run.out" 2> "$base.run.err"
        fi
    else
        if [[ "$timeout_set" -eq 1 ]]; then
            timeout "$timeout_s" "$qemu" "$arm" > "$base.run.out" 2> "$base.run.err"
        else
            "$qemu" "$arm" > "$base.run.out" 2> "$base.run.err"
        fi
    fi
    run_rc=$?
    set -e

    if [[ "$run_rc" -eq 124 ]] || grep -qiE 'uncaught target signal|Segmentation fault|Illegal instruction|Bus error|Aborted' "$base.run.err"; then
        run_fail=$((run_fail + 1))
        printf 'mode=%s result=RUN_FAIL rc=%s src=%s\n' "$mode" "$run_rc" "$src"
        printf 'RUN_FAIL rc=%s %s\n' "$run_rc" "$src" >> "$failures"
        printf 'stdout=' >> "$failures"
        cat "$base.run.out" >> "$failures"
        printf '\nstderr=' >> "$failures"
        cat "$base.run.err" >> "$failures"
        printf '\n' >> "$failures"
    else
        pass=$((pass + 1))
        printf 'mode=%s result=PASS rc=%s src=%s\n' "$mode" "$run_rc" "$src"
    fi
    print_stream "stdout" "$base.run.out"
    print_stream "stderr" "$base.run.err"
done 3< "$map"

total_compiled=$(wc -l < "$map")
total=$((total_compiled + compile_fail))
printf 'mode=%s total=%s compiled=%s pass=%s compile_fail=%s link_fail=%s run_fail=%s out_dir=%s\n' \
    "$mode" "$total" "$total_compiled" "$pass" "$compile_fail" "$link_fail" "$run_fail" "$mode_dir"

if [[ -s "$failures" ]]; then
    cat "$failures"
fi
