#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
final_dir="$root/final"
fmjcc="${FMJCC:-$final_dir/build/fmjcc}"
cc="${ARM_CC:-arm-linux-gnueabihf-gcc}"
qemu="${QEMU_ARM:-qemu-arm}"
libsysy="${LIBSYSY32:-$root/HW12/vendor/libsysy/libsysy32.s}"
work_root="${RUN_WORK:-/tmp/final_run_one}"
k="${K:-9}"

if [[ $# -ne 1 || -z "$1" ]]; then
    echo "Usage: make -C final run-one path/to/file.fmj" >&2
    exit 2
fi

src="$1"
if [[ ! -f "$src" && -f "$root/$src" ]]; then
    src="$root/$src"
elif [[ ! -f "$src" && -f "$final_dir/$src" ]]; then
    src="$final_dir/$src"
fi

if [[ ! -f "$src" ]]; then
    echo "Error: FMJ file not found: $1" >&2
    exit 2
fi

case "$src" in
    *.fmj) ;;
    *) echo "Error: expected a .fmj file: $src" >&2; exit 2 ;;
esac

mkdir -p "$work_root"
base="$(basename "$src" .fmj | tr -c 'A-Za-z0-9_' '_')"
work="$(mktemp -d "$work_root/$base.XXXXXX")"
cp "$src" "$work/$base.fmj"

echo "Compiling $src"
"$fmjcc" --k "$k" "$work/$base.fmj"

run_asm="$work/$base.runone.s"
awk '
    /^[[:space:]]*\.global[[:space:]]+main[[:space:]]*$/ {
        print ".global fmj_user_main";
        next;
    }
    /^[[:space:]]*\.type[[:space:]]+main,[[:space:]]*%function[[:space:]]*$/ {
        print ".type fmj_user_main, %function";
        next;
    }
    /^[[:space:]]*main:[[:space:]]*$/ {
        print "fmj_user_main:";
        next;
    }
    /^[[:space:]]*\.size[[:space:]]+main,[[:space:]]*\.-main[[:space:]]*$/ {
        print ".size fmj_user_main, .-fmj_user_main";
        next;
    }
    { print }
' "$work/$base.s" > "$run_asm"

harness="$work/run_one_harness.c"
cat > "$harness" <<'HARNESS_C'
#include <stdio.h>

extern int fmj_user_main(void);

int main(void) {
    int rc = fmj_user_main();
    fprintf(stderr, "\n[fmj return] %d\n", rc);
    return 0;
}
HARNESS_C

echo "Linking $work/$base.arm"
"$cc" -mcpu=cortex-a72 -Wall -Wextra -Wl,-z,noexecstack --static \
    -o "$work/$base.arm" "$run_asm" "$harness" "$libsysy" -lm

echo "Running $src"
set +e
"$qemu" "$work/$base.arm"
rc=$?
set -e

if [[ "$rc" -ne 0 ]]; then
    echo "[fmj process exit] $rc" >&2
    exit "$rc"
fi
