#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
work="${WORKDIR:-/tmp/final_runtime_regression}"
fmjcc="${FMJCC:-$root/final/build/fmjcc}"
cc="${ARM_CC:-arm-linux-gnueabihf-gcc}"
qemu="${QEMU_ARM:-qemu-arm}"
libsysy="${LIBSYSY32:-$root/HW12/vendor/libsysy/libsysy32.s}"
timeout_s="${TIMEOUT:-30}"
run_timeout_s="${RUN_TIMEOUT:-2}"
k="${K:-9}"
input="${INPUT:-4 4 4 4 4 4 4 4 4 4 4 4}"

if [[ ! -x "$fmjcc" ]]; then
    echo "Error: compiler not found or not executable: $fmjcc" >&2
    echo "Run: make -C final build" >&2
    exit 2
fi
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

rm -rf "$work"
mkdir -p "$work/src"

map="$work/map.txt"
: > "$map"
i=0
for dir in "$root/HW10/test" "$root/HW12/test"; do
    [[ -d "$dir" ]] || continue
    while IFS= read -r src; do
        i=$((i + 1))
        base="$(basename "$src" .fmj | tr -c 'A-Za-z0-9_' '_')"
        dst="$(printf '%s/src/%04d_%s.fmj' "$work" "$i" "$base")"
        cp "$src" "$dst"
        printf '%s|%s\n' "$dst" "${src#"$root"/}" >> "$map"
    done < <(find "$dir" -maxdepth 1 -type f -name '*.fmj' | sort)
done

pass=0
compile_fail=0
link_fail=0
run_fail=0
failures="$work/failures.txt"
: > "$failures"

while IFS='|' read -r test_file original; do
    base="${test_file%.fmj}"

    set +e
    timeout "$timeout_s" "$fmjcc" --k "$k" "$test_file" > "$base.compile.log" 2>&1
    rc=$?
    set -e
    if [[ "$rc" -ne 0 ]]; then
        compile_fail=$((compile_fail + 1))
        printf 'COMPILE_FAIL rc=%s %s\n' "$rc" "$original" >> "$failures"
        tail -n 30 "$base.compile.log" >> "$failures"
        continue
    fi

    set +e
    "$cc" -mcpu=cortex-a72 -Wall -Wextra -Wl,-z,noexecstack --static \
        -o "$base.arm" "$base.s" "$libsysy" -lm > "$base.gcc.log" 2>&1
    rc=$?
    set -e
    if [[ "$rc" -ne 0 ]]; then
        link_fail=$((link_fail + 1))
        printf 'LINK_FAIL rc=%s %s\n' "$rc" "$original" >> "$failures"
        tail -n 30 "$base.gcc.log" >> "$failures"
        continue
    fi

    set +e
    printf '%s\n' "$input" | timeout "$run_timeout_s" "$qemu" "$base.arm" > "$base.out" 2> "$base.err"
    rc=$?
    set -e
    if [[ "$rc" -eq 124 ]] || grep -qiE 'uncaught target signal|Segmentation fault|Illegal instruction|Bus error|Aborted' "$base.err"; then
        run_fail=$((run_fail + 1))
        printf 'RUN_FAIL rc=%s %s\n' "$rc" "$original" >> "$failures"
        printf 'out=' >> "$failures"
        cat "$base.out" >> "$failures"
        printf '\nerr=' >> "$failures"
        cat "$base.err" >> "$failures"
        printf '\n' >> "$failures"
    else
        pass=$((pass + 1))
    fi
done < "$map"

total=$(wc -l < "$map")
printf 'total=%s pass=%s compile_fail=%s link_fail=%s run_fail=%s workdir=%s\n' \
    "$total" "$pass" "$compile_fail" "$link_fail" "$run_fail" "$work"

if [[ -s "$failures" ]]; then
    cat "$failures"
fi

if [[ "$compile_fail" -ne 0 || "$link_fail" -ne 0 || "$run_fail" -ne 0 ]]; then
    exit 1
fi
