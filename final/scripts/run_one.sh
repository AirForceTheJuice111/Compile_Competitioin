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
runtime_checks="${RUNTIME_CHECKS:-0}"
opt_mode="${OPT_MODE:-${MODE:-allopt}}"
run_all_modes="${RUN_ALL_MODES:-0}"
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
modes=(none const loop1 loop2 allloop allopt)

usage() {
    echo "Usage: make -C final run-one path/to/file.fmj [OPT_MODE=none|const|loop1|loop2|allloop|allopt]" >&2
    echo "       make -C final run-one-all-mode path/to/file.fmj" >&2
}

is_truthy() {
    case "$1" in
        1|true|TRUE|yes|YES|on|ON) return 0 ;;
        *) return 1 ;;
    esac
}

valid_mode() {
    case "$1" in
        none|const|loop1|loop2|allloop|allopt) return 0 ;;
        *) return 1 ;;
    esac
}

resolve_src() {
    local arg="$1"
    local src="$arg"
    if [[ ! -f "$src" && -f "$root/$src" ]]; then
        src="$root/$src"
    elif [[ ! -f "$src" && -f "$final_dir/$src" ]]; then
        src="$final_dir/$src"
    fi
    if [[ ! -f "$src" ]]; then
        echo "Error: FMJ file not found: $arg" >&2
        exit 2
    fi
    case "$src" in
        *.fmj) ;;
        *) echo "Error: expected a .fmj file: $src" >&2; exit 2 ;;
    esac
    printf '%s\n' "$src"
}

make_harness() {
    local harness="$1"
    cat > "$harness" <<'HARNESS_C'
#include <stdio.h>

extern int fmj_user_main(void);

int main(void) {
    int rc = fmj_user_main();
    fprintf(stderr, "\n[fmj return] %d\n", rc);
    return 0;
}
HARNESS_C
}

rewrite_main_symbol() {
    local in_asm="$1"
    local out_asm="$2"
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
    ' "$in_asm" > "$out_asm"
}

result_field() {
    local mode="$1"
    local field="$2"
    local file="$work/$base.$mode.$field"
    [[ -f "$file" ]] && cat "$file"
}

result_status() {
    local mode="$1"
    result_field "$mode" status
}

result_stage() {
    local mode="$1"
    result_field "$mode" stage
}

result_rc() {
    local mode="$1"
    result_field "$mode" rc
}

run_mode() {
    local mode="$1"
    local quiet="${2:-0}"
    local asm="$work/$base.$mode.s"
    local run_asm="$work/$base.$mode.runone.s"
    local arm="$work/$base.$mode.arm"
    local harness="$work/run_one_harness.c"
    local compile_log="$work/$base.$mode.compile.log"
    local link_log="$work/$base.$mode.link.log"
    local stdout_file="$work/$base.$mode.stdout"
    local stderr_file="$work/$base.$mode.stderr"
    local stage_file="$work/$base.$mode.stage"
    local rc_file="$work/$base.$mode.rc"
    local status_file="$work/$base.$mode.status"

    [[ "$quiet" -eq 1 ]] || echo "Compiling [$mode] $src"

    local fmjcc_args=(--k "$k" --opt-mode "$mode" -o "$asm")
    if is_truthy "$runtime_checks"; then
        fmjcc_args+=(--runtime-checks)
    else
        fmjcc_args+=(--no-runtime-checks)
    fi

    set +e
    "$fmjcc" "${fmjcc_args[@]}" "$work/$base.fmj" > "$compile_log" 2>&1
    local compile_rc=$?
    set -e
    if [[ "$compile_rc" -ne 0 ]]; then
        printf 'compile\n' > "$stage_file"
        printf '%s\n' "$compile_rc" > "$rc_file"
        printf 'compile:%s\n' "$compile_rc" > "$status_file"
        : > "$stdout_file"
        cp "$compile_log" "$stderr_file"
        return 0
    fi

    rewrite_main_symbol "$asm" "$run_asm"
    make_harness "$harness"

    [[ "$quiet" -eq 1 ]] || echo "Linking [$mode] $arm"
    set +e
    "$cc" -mcpu=cortex-a72 -Wall -Wextra -Wl,-z,noexecstack --static \
        -o "$arm" "$run_asm" "$harness" "$libsysy" -lm > "$link_log" 2>&1
    local link_rc=$?
    set -e
    if [[ "$link_rc" -ne 0 ]]; then
        printf 'link\n' > "$stage_file"
        printf '%s\n' "$link_rc" > "$rc_file"
        printf 'link:%s\n' "$link_rc" > "$status_file"
        : > "$stdout_file"
        cp "$link_log" "$stderr_file"
        return 0
    fi

    echo "Running [$mode] $src"
    set +e
    : > "$stdout_file"
    : > "$stderr_file"
    if [[ "$input_set" -eq 1 ]]; then
        if [[ "$timeout_set" -eq 1 ]]; then
            printf '%s\n' "$input" | timeout "$timeout_s" "$qemu" "$arm" > "$stdout_file" 2> "$stderr_file"
        else
            printf '%s\n' "$input" | "$qemu" "$arm" > "$stdout_file" 2> "$stderr_file"
        fi
    else
        if [[ "$timeout_set" -eq 1 ]]; then
            timeout --foreground "$timeout_s" "$qemu" "$arm" 2> >(tee "$stderr_file" >&2)
        else
            "$qemu" "$arm" 2> >(tee "$stderr_file" >&2)
        fi
    fi
    local run_rc=$?
    set -e
    printf 'run\n' > "$stage_file"
    printf '%s\n' "$run_rc" > "$rc_file"
    printf 'run:%s\n' "$run_rc" > "$status_file"
}

print_mode_result() {
    local mode="$1"
    local stdout_file="$work/$base.$mode.stdout"
    local stderr_file="$work/$base.$mode.stderr"
    local fmj_return
    fmj_return="$(sed -n 's/.*\[fmj return\] //p' "$stderr_file" | tail -n 1)"
    [[ -n "$fmj_return" ]] || fmj_return="NA"

    echo "== $mode =="
    echo "stage=$(result_stage "$mode") process_rc=$(result_rc "$mode") fmj_return=$fmj_return"
    echo "stdout:"
    cat "$stdout_file"
    echo
    echo "stderr:"
    cat "$stderr_file"
    echo
}

compare_modes() {
    local ref="${modes[0]}"
    local ok=1
    for mode in "${modes[@]:1}"; do
        if [[ "$(result_status "$mode")" != "$(result_status "$ref")" ]] ||
           ! cmp -s "$work/$base.$mode.stdout" "$work/$base.$ref.stdout" ||
           ! cmp -s "$work/$base.$mode.stderr" "$work/$base.$ref.stderr"; then
            ok=0
            echo "DIFF mode=$mode ref=$ref status=$(result_status "$mode") ref_status=$(result_status "$ref")" >&2
        fi
    done

    if [[ "$ok" -eq 1 ]]; then
        echo "all_modes_consistent=1 file=$src stage=$(result_stage "$ref") rc=$(result_rc "$ref")"
        return 0
    fi

    echo "all_modes_consistent=0 file=$src" >&2
    return 1
}

if [[ $# -ne 1 || -z "$1" ]]; then
    usage
    exit 2
fi

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

src="$(resolve_src "$1")"
if ! valid_mode "$opt_mode"; then
    echo "Error: unknown OPT_MODE=$opt_mode" >&2
    usage
    exit 2
fi

mkdir -p "$work_root"
base="$(basename "$src" .fmj | tr -c 'A-Za-z0-9_' '_')"
work="$(mktemp -d "$work_root/$base.XXXXXX")"
cp "$src" "$work/$base.fmj"

if [[ "$timeout_set" -eq 1 ]]; then
    printf 'RUN_TIMEOUT=%s seconds\n' "$timeout_s"
else
    printf 'RUN_TIMEOUT=disabled\n'
fi

if is_truthy "$run_all_modes"; then
    echo "Running all optimization modes for $src"
    echo "workdir=$work"
    for mode in "${modes[@]}"; do
        run_mode "$mode" 1
        print_mode_result "$mode"
    done
    compare_modes
else
    echo "opt_mode=$opt_mode"
    echo "workdir=$work"
    run_mode "$opt_mode" 0
    cat "$work/$base.$opt_mode.stdout"
    cat "$work/$base.$opt_mode.stderr" >&2
    rc="$(result_rc "$opt_mode")"
    if [[ "$(result_stage "$opt_mode")" != "run" ]]; then
        echo "[$(result_stage "$opt_mode") failed] rc=$rc" >&2
        exit "$rc"
    fi
    if [[ "$rc" -ne 0 ]]; then
        echo "[fmj process exit] $rc" >&2
        exit "$rc"
    fi
fi
