#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
final_dir="$root/final"
fmjcc="${FMJCC:-$final_dir/build/fmjcc}"
test_dir="${TEST_DIR:-$final_dir/test/submit}"
out_dir="${OUT_DIR:-$final_dir/output}"
k="${K:-9}"
runtime_checks="${RUNTIME_CHECKS:-0}"
mode="${MODE:-all}"

case "$mode" in
    all) modes=(none const loop1 loop2 allloop allopt) ;;
    none|const|loop1|loop2|allloop|allopt) modes=("$mode") ;;
    *) echo "Error: unknown MODE=$mode" >&2; exit 2 ;;
esac

if [[ ! -x "$fmjcc" ]]; then
    echo "Error: compiler not found or not executable: $fmjcc" >&2
    echo "Run: make build" >&2
    exit 2
fi
if [[ ! -d "$test_dir" ]]; then
    echo "Error: test directory not found: $test_dir" >&2
    exit 2
fi

case "$runtime_checks" in
    1|true|TRUE|yes|YES|on|ON) runtime_arg=(--runtime-checks) ;;
    *) runtime_arg=(--no-runtime-checks) ;;
esac

mkdir -p "$out_dir"

total=0
for opt in "${modes[@]}"; do
    mode_dir="$out_dir/$opt"
    rm -rf "$mode_dir"
    mkdir -p "$mode_dir"
    map="$mode_dir/map.txt"
    : > "$map"

    while IFS= read -r src; do
        [[ -n "$src" ]] || continue
        rel="${src#"$test_dir"/}"
        name="${rel%.fmj}"
        safe="${name//\//__}"
        dst="$mode_dir/$safe.fmj"
        cp "$src" "$dst"
        base="${dst%.fmj}"
        asm="$base.$opt.s"
        "$fmjcc" --k "$k" --opt-mode "$opt" "${runtime_arg[@]}" -o "$asm" "$dst" > "$base.compile.log" 2>&1
        printf '%s|%s|%s\n' "$base" "$asm" "$src" >> "$map"
        total=$((total + 1))
    done < <(find "$test_dir" -type f -name '*.fmj' | sort)
done

printf 'compiled=%s modes=%s test_dir=%s out_dir=%s\n' "$total" "${modes[*]}" "$test_dir" "$out_dir"
