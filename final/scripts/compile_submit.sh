#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
final_dir="$root/final"
fmjcc="${FMJCC:-$final_dir/build/fmjcc}"
test_dir="${TEST_DIR:-$final_dir/test}"
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
test_dir_abs="$(cd "$test_dir" && pwd)"
out_dir_abs="$(cd "$out_dir" && pwd)"

find_sources() {
    if [[ "$out_dir_abs" == "$test_dir_abs" || "$out_dir_abs" == "$test_dir_abs"/* ]]; then
        find "$test_dir_abs" -path "$out_dir_abs" -prune -o -type f -name '*.fmj' -print | sort
    else
        find "$test_dir_abs" -type f -name '*.fmj' -print | sort
    fi
}

total=0
compiled=0
compile_fail=0
for opt in "${modes[@]}"; do
    mode_dir="$out_dir_abs/$opt"
    rm -rf "$mode_dir"
    mkdir -p "$mode_dir"
    map="$mode_dir/map.txt"
    results="$mode_dir/compile-results.txt"
    failures="$mode_dir/compile-failures.txt"
    : > "$map"
    : > "$results"
    : > "$failures"

    while IFS= read -r src; do
        [[ -n "$src" ]] || continue
        rel="${src#"$test_dir_abs"/}"
        name="${rel%.fmj}"
        safe="${name//\//__}"
        dst="$mode_dir/$safe.fmj"
        cp "$src" "$dst"
        base="${dst%.fmj}"
        asm="$base.$opt.s"
        set +e
        "$fmjcc" --k "$k" --opt-mode "$opt" "${runtime_arg[@]}" -o "$asm" "$dst" > "$base.compile.log" 2>&1
        rc=$?
        set -e
        total=$((total + 1))
        if [[ "$rc" -eq 0 ]]; then
            compiled=$((compiled + 1))
            printf '%s|%s|%s\n' "$base" "$asm" "$src" >> "$map"
            printf 'COMPILE_OK mode=%s src=%s asm=%s\n' "$opt" "$src" "$asm" >> "$results"
        else
            compile_fail=$((compile_fail + 1))
            printf 'COMPILE_FAIL mode=%s rc=%s src=%s log=%s\n' "$opt" "$rc" "$src" "$base.compile.log" >> "$results"
            printf 'COMPILE_FAIL mode=%s rc=%s src=%s\n' "$opt" "$rc" "$src" >> "$failures"
            tail -n 30 "$base.compile.log" >> "$failures"
            printf '\n' >> "$failures"
        fi
    done < <(find_sources)
done

printf 'total=%s compiled=%s compile_fail=%s modes=%s test_dir=%s out_dir=%s\n' \
    "$total" "$compiled" "$compile_fail" "${modes[*]}" "$test_dir_abs" "$out_dir_abs"
