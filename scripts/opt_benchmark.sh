#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
final_dir="$root"
fmjcc="${FMJCC:-$final_dir/build/fmjcc}"
cc="${ARM_CC:-arm-linux-gnueabihf-gcc}"
qemu="${QEMU_ARM:-qemu-arm}"
libsysy="${LIBSYSY32:-$root/vendor/libsysy/libsysy32.s}"
work="${WORKDIR:-/tmp/final_opt_benchmark}"
k="${K:-9}"
runtime_checks="${RUNTIME_CHECKS:-0}"
bench_timeout="${BENCH_TIMEOUT:-300}"
input="${INPUT:-}"
modes=(none const loop1 loop2 allloop allopt)

default_tests=(
    "$final_dir/test/all/bigarray.fmj"
    "$final_dir/test/all/bigloop.fmj"
    "$final_dir/test/all/deepnestedloops.fmj"
    "$final_dir/test/all/bubblesort.fmj"
)

is_truthy() {
    case "$1" in
        1|true|TRUE|yes|YES|on|ON) return 0 ;;
        *) return 1 ;;
    esac
}

resolve_src() {
    local arg="$1"
    if [[ -f "$arg" ]]; then
        printf '%s\n' "$arg"
    elif [[ -f "$root/$arg" ]]; then
        printf '%s\n' "$root/$arg"
    elif [[ -f "$final_dir/$arg" ]]; then
        printf '%s\n' "$final_dir/$arg"
    else
        echo "Error: FMJ file not found: $arg" >&2
        exit 2
    fi
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

sha256_file() {
    sha256sum "$1" | awk '{print $1}'
}

elapsed_ms() {
    local start_ns="$1"
    local end_ns="$2"
    awk -v start="$start_ns" -v end="$end_ns" 'BEGIN { printf "%.3f", (end - start) / 1000000.0 }'
}

for tool in "$fmjcc" "$cc" "$qemu"; do
    if ! command -v "$tool" >/dev/null 2>&1 && [[ ! -x "$tool" ]]; then
        echo "Error: required tool not found: $tool" >&2
        exit 2
    fi
done
if [[ ! -f "$libsysy" ]]; then
    echo "Making libsysy32.s..." >&2
    (cd "$root/vendor/libsysy" && "$cc" -mcpu=cortex-a72 -S libsysy32.c -o libsysy32.s)
fi

if [[ "$#" -eq 0 ]]; then
    set -- "${default_tests[@]}"
fi

rm -rf "$work"
mkdir -p "$work"

printf 'bench_workdir=%s\n' "$work"
printf 'bench_timeout=%s\n' "$bench_timeout"

for arg in "$@"; do
    src="$(resolve_src "$arg")"
    name="$(printf '%s' "$(basename "$src" .fmj)" | tr -c 'A-Za-z0-9_' '_')"
    case_dir="$work/$name"
    mkdir -p "$case_dir"
    cp "$src" "$case_dir/$name.fmj"
    make_harness "$case_dir/harness.c"

    ref_status=""
    ref_stdout_hash=""
    ref_stderr_hash=""
    consistent=1
    printf 'bench_file=%s source=%s\n' "$name" "${src#"$root"/}"

    for mode in "${modes[@]}"; do
        asm="$case_dir/$name.$mode.s"
        run_asm="$case_dir/$name.$mode.run.s"
        arm="$case_dir/$name.$mode.arm"
        compile_log="$case_dir/$name.$mode.compile.log"
        link_log="$case_dir/$name.$mode.link.log"
        stdout_file="$case_dir/$name.$mode.stdout"
        stderr_file="$case_dir/$name.$mode.stderr"
        timeout_err="$case_dir/$name.$mode.timeout.err"

        fmjcc_args=(--k "$k" --opt-mode "$mode" -o "$asm")
        if is_truthy "$runtime_checks"; then
            fmjcc_args+=(--runtime-checks)
        else
            fmjcc_args+=(--no-runtime-checks)
        fi

        set +e
        "$fmjcc" "${fmjcc_args[@]}" "$case_dir/$name.fmj" > "$compile_log" 2>&1
        compile_rc=$?
        set -e
        if [[ "$compile_rc" -ne 0 ]]; then
            printf 'bench mode=%s stage=compile rc=%s file=%s\n' "$mode" "$compile_rc" "$name"
            consistent=0
            continue
        fi

        rewrite_main_symbol "$asm" "$run_asm"
        set +e
        "$cc" -mcpu=cortex-a72 -Wall -Wextra -Wl,-z,noexecstack --static \
            -o "$arm" "$run_asm" "$case_dir/harness.c" "$libsysy" -lm > "$link_log" 2>&1
        link_rc=$?
        set -e
        if [[ "$link_rc" -ne 0 ]]; then
            printf 'bench mode=%s stage=link rc=%s file=%s\n' "$mode" "$link_rc" "$name"
            consistent=0
            continue
        fi

        start_ns="$(date +%s%N)"
        set +e
        if [[ -n "$input" ]]; then
            printf '%s\n' "$input" | timeout "$bench_timeout" "$qemu" "$arm" > "$stdout_file" 2> "$stderr_file"
        else
            timeout "$bench_timeout" "$qemu" "$arm" > "$stdout_file" 2> "$stderr_file"
        fi
        run_rc=$?
        end_ns="$(date +%s%N)"
        set -e

        elapsed="$(elapsed_ms "$start_ns" "$end_ns")"
        fmj_return="$(sed -n 's/.*\[fmj return\] //p' "$stderr_file" | tail -n 1)"
        [[ -n "$fmj_return" ]] || fmj_return="NA"
        stdout_hash="$(sha256_file "$stdout_file")"
        stderr_hash="$(sha256_file "$stderr_file")"
        asm_lines="$(wc -l < "$asm" | tr -d ' ')"
        status="run:$run_rc:$stdout_hash:$stderr_hash"

        if [[ -z "$ref_status" ]]; then
            ref_status="$status"
            ref_stdout_hash="$stdout_hash"
            ref_stderr_hash="$stderr_hash"
        elif [[ "$status" != "$ref_status" ]]; then
            consistent=0
        fi

        if grep -qiE 'uncaught target signal|Segmentation fault|Illegal instruction|Bus error|Aborted' "$stderr_file"; then
            consistent=0
        fi

        : > "$timeout_err"
        printf 'bench mode=%s stage=run rc=%s fmj_return=%s elapsed_ms=%s asm_lines=%s stdout_sha256=%s stderr_sha256=%s file=%s\n' \
            "$mode" "$run_rc" "$fmj_return" "$elapsed" "$asm_lines" "$stdout_hash" "$stderr_hash" "$name"
    done

    printf 'bench_consistent=%s file=%s ref_stdout_sha256=%s ref_stderr_sha256=%s\n' \
        "$consistent" "$name" "$ref_stdout_hash" "$ref_stderr_hash"
done
