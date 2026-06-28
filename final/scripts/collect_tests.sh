#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
out="$root/final/test/all"
lock="${TMPDIR:-/tmp}/final_collect_tests.lock"

exec 9>"$lock"
flock 9

rm -rf "$out"
mkdir -p "$out"

source_priority() {
    local rel="$1"
    case "$rel" in
        FINALREPORTtests/newtests.fmj/*) printf '000\n' ;;
        FINALREPORTtests/oldtests.fmj/*) printf '001\n' ;;
        FINALREPORTtests/*) printf '002\n' ;;
        HW12/*) printf '010\n' ;;
        HW11/*) printf '011\n' ;;
        HW10/*) printf '012\n' ;;
        HW9/*) printf '013\n' ;;
        HW8/*) printf '014\n' ;;
        HW7/*) printf '015\n' ;;
        HW6/*) printf '016\n' ;;
        HW5/*) printf '017\n' ;;
        HW4/*) printf '018\n' ;;
        HW3/*) printf '019\n' ;;
        HW2/*) printf '020\n' ;;
        HW1/*) printf '021\n' ;;
        PARSING/*) printf '030\n' ;;
        *) printf '090\n' ;;
    esac
}

declare -A chosen_file
declare -A chosen_rel
declare -A chosen_priority

while IFS= read -r file; do
    rel="${file#"$root"/}"
    name="$(basename "$file")"
    priority="$(source_priority "$rel")"

    if [[ -z "${chosen_file[$name]:-}" ||
          "$priority" < "${chosen_priority[$name]}" ||
          ( "$priority" == "${chosen_priority[$name]}" && "$rel" < "${chosen_rel[$name]}" ) ]]; then
        chosen_file[$name]="$file"
        chosen_rel[$name]="$rel"
        chosen_priority[$name]="$priority"
    fi
done < <(find "$root" -path "$root/final" -prune -o -type f -name '*.fmj' -print | sort)

while IFS= read -r name; do
    cp "${chosen_file[$name]}" "$out/$name"
done < <(printf '%s\n' "${!chosen_file[@]}" | sort)

find "$out" -type f -name '*.fmj' | sort
