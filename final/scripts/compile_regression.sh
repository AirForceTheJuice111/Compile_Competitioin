#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
work="${WORKDIR:-/tmp/final_compile_regression}"
fmjcc="${FMJCC:-$root/final/build/fmjcc}"
timeout_s="${TIMEOUT:-30}"
k="${K:-9}"

if [[ ! -x "$fmjcc" ]]; then
    echo "Error: compiler not found or not executable: $fmjcc" >&2
    echo "Run: make -C final build" >&2
    exit 2
fi

rm -rf "$work"
mkdir -p "$work/src"

map="$work/map.txt"
: > "$map"

i=0
while IFS= read -r src; do
    [[ -n "$src" ]] || continue
    i=$((i + 1))
    base="$(basename "$src" .fmj | tr -c 'A-Za-z0-9_' '_')"
    dst="$(printf '%s/src/%04d_%s.fmj' "$work" "$i" "$base")"
    cp "$src" "$dst"
    printf '%s|%s\n' "$dst" "$src" >> "$map"
done < <("$root/final/scripts/collect_tests.sh")

pass=0
reject=0
crash=0
timeout_count=0
failures="$work/failures.txt"
: > "$failures"

while IFS='|' read -r test_file original; do
    log="$test_file.log"
    set +e
    timeout "$timeout_s" "$fmjcc" --k "$k" "$test_file" > "$log" 2>&1
    rc=$?
    set -e

    if [[ "$rc" -eq 0 ]]; then
        pass=$((pass + 1))
    elif [[ "$rc" -eq 124 ]]; then
        timeout_count=$((timeout_count + 1))
        crash=$((crash + 1))
        printf 'TIMEOUT %s\n' "$original" >> "$failures"
    elif grep -qiE 'error|syntax|parse|semantic|invalid|undefined|not found|cannot|duplicate|must be|not allowed|failed' "$log"; then
        reject=$((reject + 1))
    else
        crash=$((crash + 1))
        printf 'CRASH rc=%s %s\n' "$rc" "$original" >> "$failures"
        tail -n 30 "$log" >> "$failures"
    fi
done < "$map"

empty_labels="$work/empty-labels.txt"
set +e
rg -n 'Entry Label:\s*$|LABEL\s*;|JUMP\s*;|CJUMP.*\?\s*:' "$work/src" -g '*.quad' > "$empty_labels"
empty_label_hits=$(wc -l < "$empty_labels")
set -e

total=$(wc -l < "$map")
printf 'total=%s pass=%s reject=%s crash=%s timeout=%s empty_label_hits=%s workdir=%s\n' \
    "$total" "$pass" "$reject" "$crash" "$timeout_count" "$empty_label_hits" "$work"

if [[ -s "$failures" ]]; then
    cat "$failures"
fi

if [[ "$empty_label_hits" -ne 0 ]]; then
    head -n 80 "$empty_labels"
fi

if [[ "$crash" -ne 0 || "$empty_label_hits" -ne 0 ]]; then
    exit 1
fi
