#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
out="$root/final/test/all"

rm -rf "$out"
mkdir -p "$out"

while IFS= read -r file; do
    rel="${file#"$root"/}"
    name="${rel//\//__}"
    cp "$file" "$out/$name"
done < <(find "$root" -path "$root/final" -prune -o -type f -name '*.fmj' -print | sort)

find "$out" -type f -name '*.fmj' | sort
