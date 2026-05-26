#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <path-to-HW11-main>"
    exit 1
fi

main_bin="$1"
test_dir="$(cd "$(dirname "$0")" && pwd)"

for input in "$test_dir"/*.4-ssa-withflow-xml.quad; do
    base="${input%.4-ssa-withflow-xml.quad}"
    name="$(basename "$base")"
    echo "Running $name"
    "$main_bin" "$base" > "$test_dir/$name.log"
    diff -u "$base.expected.s" "$base.s"
done

echo "All extra HW11 tests passed."
