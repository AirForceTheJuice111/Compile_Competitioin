#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
final_dir="$root"
work="${WORKDIR:-/tmp/final_all_mode_regression}"
fmjinterp="${FMJINTERP:-$final_dir/build/fmjinterp}"
fmjcc="${FMJCC:-$final_dir/build/fmjcc}"
cc="${ARM_CC:-arm-linux-gnueabihf-gcc}"
qemu="${QEMU_ARM:-qemu-arm}"
libsysy="${LIBSYSY32:-$root/vendor/libsysy/libsysy32.s}"
timeout_s="${TIMEOUT:-30}"
run_timeout_s="${RUN_TIMEOUT:-2}"
k="${K:-9}"
runtime_checks="${RUNTIME_CHECKS:-0}"
input="${INPUT:-4 4 4 4 4 4 4 4 4 4 4 4}"
source "$final_dir/scripts/test_expect.sh"

case "$runtime_checks" in
    1|true|TRUE|yes|YES|on|ON) runtime_checks_enabled=1 ;;
    *) runtime_checks_enabled=0 ;;
esac

for tool in "$fmjcc" "$fmjinterp"; do
    if [[ ! -x "$tool" ]]; then
        echo "Error: tool not found or not executable: $tool" >&2
        echo "Run: make build" >&2
        exit 2
    fi
done
for tool in "$cc" "$qemu"; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "Error: required tool not found: $tool" >&2
        exit 2
    fi
done
if [[ ! -f "$libsysy" ]]; then
    echo "Making libsysy32.s..." >&2
    (cd "$root/vendor/libsysy" && "$cc" -mcpu=cortex-a72 -S libsysy32.c -o libsysy32.s)
fi

rm -rf "$work"
mkdir -p "$work/logs" "$work/runs"

map="$work/map.txt"
: > "$map"

while IFS= read -r src; do
    [[ -n "$src" ]] || continue
    printf '%s|%s\n' "$src" "${src#"$root"/}" >> "$map"
done < <("$final_dir/scripts/collect_tests.sh")

if [[ -d "$final_dir/test/submit" ]]; then
    while IFS= read -r src; do
        [[ -n "$src" ]] || continue
        printf '%s|%s\n' "$src" "${src#"$root"/}" >> "$map"
    done < <(find "$final_dir/test/submit" -type f -name '*.fmj' | sort)
fi

total=0
run_consistent=0
reject_consistent=0
timeout_consistent=0
optional_semantics_skip=0
expect_pass_consistent=0
expect_reject_consistent=0
mismatch=0
interp_timeout=0
unexpected_compile_reject=0
compiler_accept_invalid=0
failures="$work/failures.txt"
: > "$failures"

extract_field() {
    local key="$1"
    local line="$2"
    sed -n "s/.* $key=\\([^ ]*\\).*/\\1/p" <<< "$line"
}

all_modes_reached_run() {
    local log="$1"
    [[ "$(grep -c '^stage=run ' "$log" || true)" -eq 6 ]]
}

uses_optional_runtime_semantics() {
    local src="$1"
    local status="$2"
    local out="$3"
    local err="$4"

    if [[ "$runtime_checks_enabled" -eq 1 ]]; then
        return 1
    fi

    set +e
    printf '%s\n' "$input" | timeout "$run_timeout_s" "$fmjinterp" \
        --runtime-status "$status" "$src" > "$out" 2> "$err"
    local rc=$?
    set -e

    if [[ "$rc" -eq 124 || ! -f "$status" ]]; then
        return 1
    fi

    local optional
    optional="$(sed -n 's/^optional_semantics=//p' "$status" | head -n 1)"
    [[ "$optional" == "1" ]]
}

is_stress_runtime_test() {
    case "$(basename "$1")" in
        bigloop.fmj|linkedlist.fmj) return 0 ;;
        *) return 1 ;;
    esac
}

is_nonterminating_test() {
    case "$(basename "$1")" in
        newtest10.fmj|semant_test26_no_continue_outside_loop.fmj) return 0 ;;
        *) return 1 ;;
    esac
}

run_timeout_for() {
    case "$(basename "$1")" in
        newtest10.fmj|semant_test26_no_continue_outside_loop.fmj)
            printf '%s\n' "${NONTERMINATING_RUN_TIMEOUT:-5}"
            ;;
        *)
            printf '%s\n' "$run_timeout_s"
            ;;
    esac
}

while IFS='|' read -r src original; do
    [[ -n "$src" ]] || continue
    total=$((total + 1))
    safe="$(printf '%04d_%s' "$total" "$(basename "$src" .fmj | tr -c 'A-Za-z0-9_' '_')")"
    check_out="$work/logs/$safe.interp.check.out"
    check_err="$work/logs/$safe.interp.check.err"
    status_file="$work/logs/$safe.interp.status"
    status_out="$work/logs/$safe.interp.out"
    status_err="$work/logs/$safe.interp.err"
    run_log="$work/logs/$safe.allmodes.log"
    expect="$(fmj_expect "$src")"
    case_run_timeout="$(run_timeout_for "$src")"

    set +e
    timeout "$timeout_s" "$fmjinterp" --check "$src" > "$check_out" 2> "$check_err"
    interp_check_rc=$?
    RUN_ALL_MODES=1 RUN_WORK="$work/runs" RUN_TIMEOUT="$case_run_timeout" INPUT="$input" \
        FMJCC="$fmjcc" ARM_CC="$cc" QEMU_ARM="$qemu" LIBSYSY32="$libsysy" \
        K="$k" RUNTIME_CHECKS="$runtime_checks" \
        bash "$final_dir/scripts/run_one.sh" "$src" > "$run_log" 2>&1
    run_one_rc=$?
    set -e

    if [[ "$interp_check_rc" -eq 124 ]]; then
        interp_timeout=$((interp_timeout + 1))
        mismatch=$((mismatch + 1))
        printf 'INTERP_CHECK_TIMEOUT %s\n' "$original" >> "$failures"
        continue
    fi

    if [[ "$run_one_rc" -ne 0 ]]; then
        if [[ "$interp_check_rc" -eq 0 ]] &&
           all_modes_reached_run "$run_log" &&
           uses_optional_runtime_semantics "$src" "$status_file" "$status_out" "$status_err"; then
            optional_semantics_skip=$((optional_semantics_skip + 1))
            continue
        fi
        mismatch=$((mismatch + 1))
        printf 'ALL_MODE_MISMATCH %s\n' "$original" >> "$failures"
        cat "$run_log" >> "$failures"
        printf '\n' >> "$failures"
        continue
    fi

    summary="$(grep '^all_modes_consistent=1 ' "$run_log" | tail -n 1 || true)"
    if [[ -z "$summary" ]]; then
        mismatch=$((mismatch + 1))
        printf 'MISSING_ALL_MODE_SUMMARY %s\n' "$original" >> "$failures"
        cat "$run_log" >> "$failures"
        printf '\n' >> "$failures"
        continue
    fi

    stage="$(extract_field stage "$summary")"
    rc="$(extract_field rc "$summary")"

    if [[ "$expect" == "FAIL" ]]; then
        if [[ "$stage" == "compile" ]]; then
            reject_consistent=$((reject_consistent + 1))
            expect_reject_consistent=$((expect_reject_consistent + 1))
        else
            compiler_accept_invalid=$((compiler_accept_invalid + 1))
            mismatch=$((mismatch + 1))
            printf 'EXPECT_FAIL_ACCEPTED stage=%s rc=%s %s\n' "$stage" "$rc" "$original" >> "$failures"
            cat "$run_log" >> "$failures"
            printf '\n' >> "$failures"
        fi
        continue
    fi

    if [[ "$expect" == "PASS" ]]; then
        if [[ "$interp_check_rc" -ne 0 ]]; then
            mismatch=$((mismatch + 1))
            printf 'EXPECT_PASS_INTERP_REJECT interp_rc=%s stage=%s rc=%s %s\n' "$interp_check_rc" "$stage" "$rc" "$original" >> "$failures"
            cat "$check_err" >> "$failures"
            printf '\n' >> "$failures"
            continue
        fi
        if [[ "$stage" != "run" ]]; then
            unexpected_compile_reject=$((unexpected_compile_reject + 1))
            mismatch=$((mismatch + 1))
            printf 'EXPECT_PASS_NOT_RUN stage=%s rc=%s %s\n' "$stage" "$rc" "$original" >> "$failures"
            cat "$run_log" >> "$failures"
            printf '\n' >> "$failures"
            continue
        fi
        expect_pass_consistent=$((expect_pass_consistent + 1))
    fi

    if [[ "$interp_check_rc" -ne 0 ]]; then
        if [[ "$stage" == "compile" ]]; then
            reject_consistent=$((reject_consistent + 1))
        else
            compiler_accept_invalid=$((compiler_accept_invalid + 1))
            mismatch=$((mismatch + 1))
            printf 'COMPILER_ACCEPT_INVALID stage=%s rc=%s %s\n' "$stage" "$rc" "$original" >> "$failures"
            cat "$run_log" >> "$failures"
            printf '\n' >> "$failures"
        fi
        continue
    fi

    if [[ "$stage" != "run" ]]; then
        unexpected_compile_reject=$((unexpected_compile_reject + 1))
        mismatch=$((mismatch + 1))
        printf 'VALID_NOT_RUN stage=%s rc=%s %s\n' "$stage" "$rc" "$original" >> "$failures"
        cat "$run_log" >> "$failures"
        printf '\n' >> "$failures"
        continue
    fi

    if ! is_stress_runtime_test "$src" &&
       ! is_nonterminating_test "$src" &&
       uses_optional_runtime_semantics "$src" "$status_file" "$status_out" "$status_err"; then
        optional_semantics_skip=$((optional_semantics_skip + 1))
        continue
    fi

    if [[ "$rc" == "124" ]]; then
        timeout_consistent=$((timeout_consistent + 1))
    else
        run_consistent=$((run_consistent + 1))
    fi
done < "$map"

printf 'total=%s run_consistent=%s reject_consistent=%s timeout_consistent=%s optional_semantics_skip=%s expect_pass_consistent=%s expect_reject_consistent=%s mismatch=%s interp_timeout=%s unexpected_compile_reject=%s compiler_accept_invalid=%s runtime_checks=%s workdir=%s\n' \
    "$total" "$run_consistent" "$reject_consistent" "$timeout_consistent" "$optional_semantics_skip" "$expect_pass_consistent" "$expect_reject_consistent" "$mismatch" "$interp_timeout" "$unexpected_compile_reject" "$compiler_accept_invalid" "$runtime_checks_enabled" "$work"

if [[ -s "$failures" ]]; then
    cat "$failures"
fi

if [[ "$mismatch" -ne 0 ]]; then
    exit 1
fi
