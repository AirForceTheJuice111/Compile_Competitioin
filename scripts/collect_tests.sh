#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
out="$root/test/all"
lock="${TMPDIR:-/tmp}/final_collect_tests.lock"

exec 9>"$lock"
flock 9

if [[ -d "$out" ]]; then
    find "$out" -type f -name '*.fmj' | sort
    exit 0
fi

find "$root/test" -type f -name '*.fmj' | sort
