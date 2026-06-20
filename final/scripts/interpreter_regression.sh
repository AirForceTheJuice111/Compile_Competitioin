#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
work="${WORKDIR:-/tmp/final_interpreter_regression}"
fmjcc="${FMJCC:-$root/final/build/fmjcc}"
fmjinterp="${FMJINTERP:-$root/final/build/fmjinterp}"
cc="${ARM_CC:-arm-linux-gnueabihf-gcc}"
qemu="${QEMU_ARM:-qemu-arm}"
libsysy="${LIBSYSY32:-$root/HW12/vendor/libsysy/libsysy32.s}"
timeout_s="${TIMEOUT:-30}"
run_timeout_s="${RUN_TIMEOUT:-2}"
k="${K:-9}"
input="${INPUT:-4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4}"
runtime_checks="${RUNTIME_CHECKS:-0}"
source "$root/final/scripts/test_expect.sh"

case "$runtime_checks" in
    1|true|TRUE|yes|YES|on|ON) runtime_checks_enabled=1 ;;
    *) runtime_checks_enabled=0 ;;
esac

for tool in "$fmjcc" "$fmjinterp"; do
    if [[ ! -x "$tool" ]]; then
        echo "Error: tool not found or not executable: $tool" >&2
        echo "Run: make -C final build" >&2
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
    (cd "$root/HW12/vendor/libsysy" && "$cc" -mcpu=cortex-a72 -S libsysy32.c -o libsysy32.s)
fi

rm -rf "$work"
mkdir -p "$work/src"

map="$work/map.txt"
: > "$map"
i=0
while IFS= read -r src; do
    [[ -n "$src" ]] || continue
    i=$((i + 1))
    base="$(basename "$src" .fmj | tr -c 'A-Za-z0-9_' '_')"
    dst="$(printf '%s/src/%04d_%s.fmj' "$work" "$i" "$base")"
    cp "$src" "$dst"
    printf '%s|%s\n' "$dst" "${src#"$root"/}" >> "$map"
done < <("$root/final/scripts/collect_tests.sh")

run_match=0
reject_match=0
timeout_match=0
mismatch=0
compile_only_reject=0
interp_only_reject=0
link_fail=0
run_fail=0
timeout_count=0
runtime_error_skip=0
optional_semantics_skip=0
expect_pass_match=0
expect_reject_match=0
stress_runtime_skip=0
failures="$work/failures.txt"
: > "$failures"

is_stress_runtime_test() {
    case "$1" in
        *bigloop*.fmj|*linkedlist*.fmj) return 0 ;;
        *) return 1 ;;
    esac
}

while IFS='|' read -r test_file original; do
    base="${test_file%.fmj}"
    expect="$(fmj_expect "$test_file")"

    set +e
    timeout "$timeout_s" "$fmjinterp" --check "$test_file" > "$base.interp.check.out" 2> "$base.interp.check.err"
    interp_check_rc=$?
    fmjcc_args=(--k "$k")
    if [[ "$runtime_checks_enabled" -eq 1 ]]; then
        fmjcc_args+=(--runtime-checks)
    else
        fmjcc_args+=(--no-runtime-checks)
    fi
    timeout "$timeout_s" "$fmjcc" "${fmjcc_args[@]}" "$test_file" > "$base.compile.log" 2>&1
    compile_rc=$?
    set -e

    if [[ "$interp_check_rc" -eq 124 || "$compile_rc" -eq 124 ]]; then
        timeout_count=$((timeout_count + 1))
        mismatch=$((mismatch + 1))
        printf 'TIMEOUT check compile_rc=%s interp_rc=%s %s\n' "$compile_rc" "$interp_check_rc" "$original" >> "$failures"
        continue
    fi

    if [[ "$expect" == "FAIL" ]]; then
        if [[ "$interp_check_rc" -ne 0 && "$compile_rc" -ne 0 ]]; then
            reject_match=$((reject_match + 1))
            expect_reject_match=$((expect_reject_match + 1))
        else
            mismatch=$((mismatch + 1))
            printf 'EXPECT_FAIL_MISMATCH compile_rc=%s interp_rc=%s %s\n' "$compile_rc" "$interp_check_rc" "$original" >> "$failures"
            if [[ "$compile_rc" -eq 0 ]]; then
                printf 'compiler accepted expected-fail program\n' >> "$failures"
            else
                tail -n 30 "$base.compile.log" >> "$failures"
            fi
            if [[ "$interp_check_rc" -eq 0 ]]; then
                printf 'interpreter accepted expected-fail program\n' >> "$failures"
            else
                tail -n 20 "$base.interp.check.err" >> "$failures"
            fi
            printf '\n' >> "$failures"
        fi
        continue
    fi

    if [[ "$expect" == "PASS" ]]; then
        if [[ "$interp_check_rc" -ne 0 || "$compile_rc" -ne 0 ]]; then
            mismatch=$((mismatch + 1))
            printf 'EXPECT_PASS_MISMATCH compile_rc=%s interp_rc=%s %s\n' "$compile_rc" "$interp_check_rc" "$original" >> "$failures"
            if [[ "$compile_rc" -ne 0 ]]; then tail -n 30 "$base.compile.log" >> "$failures"; fi
            if [[ "$interp_check_rc" -ne 0 ]]; then tail -n 20 "$base.interp.check.err" >> "$failures"; fi
            printf '\n' >> "$failures"
            continue
        fi
        expect_pass_match=$((expect_pass_match + 1))
    fi

    if [[ "$interp_check_rc" -ne 0 && "$compile_rc" -ne 0 ]]; then
        reject_match=$((reject_match + 1))
        continue
    fi
    if [[ "$interp_check_rc" -ne 0 && "$compile_rc" -eq 0 ]]; then
        interp_only_reject=$((interp_only_reject + 1))
        mismatch=$((mismatch + 1))
        printf 'INTERP_ONLY_REJECT %s\n' "$original" >> "$failures"
        tail -n 20 "$base.interp.check.err" >> "$failures"
        continue
    fi
    if [[ "$interp_check_rc" -eq 0 && "$compile_rc" -ne 0 ]]; then
        compile_only_reject=$((compile_only_reject + 1))
        mismatch=$((mismatch + 1))
        printf 'COMPILE_ONLY_REJECT rc=%s %s\n' "$compile_rc" "$original" >> "$failures"
        tail -n 30 "$base.compile.log" >> "$failures"
        continue
    fi

    set +e
    "$cc" -mcpu=cortex-a72 -Wall -Wextra -Wl,-z,noexecstack --static \
        -o "$base.arm" "$base.s" "$libsysy" -lm > "$base.link.log" 2>&1
    link_rc=$?
    set -e
    if [[ "$link_rc" -ne 0 ]]; then
        link_fail=$((link_fail + 1))
        mismatch=$((mismatch + 1))
        printf 'LINK_FAIL rc=%s %s\n' "$link_rc" "$original" >> "$failures"
        tail -n 30 "$base.link.log" >> "$failures"
        continue
    fi

    if is_stress_runtime_test "$test_file"; then
        set +e
        printf '%s\n' "$input" | timeout "$run_timeout_s" "$qemu" "$base.arm" > "$base.compiled.out" 2> "$base.compiled.err"
        compiled_rc=$?
        set -e

        if grep -qiE 'uncaught target signal|Segmentation fault|Illegal instruction|Bus error|Aborted' "$base.compiled.err"; then
            run_fail=$((run_fail + 1))
            mismatch=$((mismatch + 1))
            printf 'STRESS_COMPILED_RUNTIME_ERROR rc=%s %s\n' "$compiled_rc" "$original" >> "$failures"
            cat "$base.compiled.err" >> "$failures"
            printf '\n' >> "$failures"
        else
            stress_runtime_skip=$((stress_runtime_skip + 1))
            if [[ "$compiled_rc" -eq 124 ]]; then
                timeout_count=$((timeout_count + 1))
            fi
        fi
        continue
    fi

    set +e
    printf '%s\n' "$input" | timeout "$run_timeout_s" "$fmjinterp" \
        --runtime-status "$base.interp.status" "$test_file" > "$base.interp.out" 2> "$base.interp.err"
    interp_rc=$?
    set -e

    interp_exit_taken=0
    interp_optional_semantics=0
    interp_runtime_reason=
    interp_optional_reasons=
    if [[ -f "$base.interp.status" ]]; then
        interp_exit_taken="$(sed -n 's/^exit_taken=//p' "$base.interp.status" | head -n 1)"
        [[ -n "$interp_exit_taken" ]] || interp_exit_taken=0
        interp_optional_semantics="$(sed -n 's/^optional_semantics=//p' "$base.interp.status" | head -n 1)"
        [[ -n "$interp_optional_semantics" ]] || interp_optional_semantics=0
        interp_runtime_reason="$(sed -n 's/^runtime_reason=//p' "$base.interp.status" | head -n 1)"
        interp_optional_reasons="$(sed -n 's/^optional_reasons=//p' "$base.interp.status" | head -n 1)"
    fi

    if [[ "$interp_rc" -ne 124 && "$runtime_checks_enabled" -eq 0 && "$interp_optional_semantics" -eq 1 ]]; then
        optional_semantics_skip=$((optional_semantics_skip + 1))
        if [[ "$interp_exit_taken" -eq 1 ]]; then
            runtime_error_skip=$((runtime_error_skip + 1))
        fi
        printf 'OPTIONAL_SEMANTICS_SKIP runtime_reason=%s optional_reasons=%s %s\n' \
            "$interp_runtime_reason" "$interp_optional_reasons" "$original" >> "$base.skip.log"
        continue
    fi

    set +e
    printf '%s\n' "$input" | timeout "$run_timeout_s" "$qemu" "$base.arm" > "$base.compiled.out" 2> "$base.compiled.err"
    compiled_rc=$?
    set -e

    if [[ "$compiled_rc" -eq 124 && "$interp_rc" -eq 124 ]]; then
        timeout_match=$((timeout_match + 1))
        timeout_count=$((timeout_count + 1))
        continue
    fi

    if [[ "$compiled_rc" -eq 124 || "$interp_rc" -eq 124 ]]; then
        timeout_count=$((timeout_count + 1))
        run_fail=$((run_fail + 1))
        mismatch=$((mismatch + 1))
        printf 'RUN_TIMEOUT compiled_rc=%s interp_rc=%s %s\n' "$compiled_rc" "$interp_rc" "$original" >> "$failures"
        continue
    fi

    if grep -qiE 'uncaught target signal|Segmentation fault|Illegal instruction|Bus error|Aborted' "$base.compiled.err"; then
        run_fail=$((run_fail + 1))
        mismatch=$((mismatch + 1))
        printf 'COMPILED_RUNTIME_ERROR rc=%s %s\n' "$compiled_rc" "$original" >> "$failures"
        cat "$base.compiled.err" >> "$failures"
        printf '\n' >> "$failures"
        continue
    fi

    if [[ "$compiled_rc" -eq "$interp_rc" ]] && cmp -s "$base.compiled.out" "$base.interp.out"; then
        run_match=$((run_match + 1))
    else
        mismatch=$((mismatch + 1))
        printf 'OUTPUT_MISMATCH compiled_rc=%s interp_rc=%s %s\n' "$compiled_rc" "$interp_rc" "$original" >> "$failures"
        printf 'compiled_stdout=' >> "$failures"
        od -An -tx1 -v "$base.compiled.out" | sed -n '1,12p' >> "$failures"
        printf 'interp_stdout=' >> "$failures"
        od -An -tx1 -v "$base.interp.out" | sed -n '1,12p' >> "$failures"
        printf 'compiled_text=' >> "$failures"
        cat "$base.compiled.out" >> "$failures"
        printf '\ninterp_text=' >> "$failures"
        cat "$base.interp.out" >> "$failures"
        printf '\n' >> "$failures"
    fi
done < "$map"

total=$(wc -l < "$map")
printf 'total=%s run_match=%s reject_match=%s timeout_match=%s runtime_error_skip=%s optional_semantics_skip=%s stress_runtime_skip=%s expect_pass_match=%s expect_reject_match=%s mismatch=%s compile_only_reject=%s interp_only_reject=%s link_fail=%s run_fail=%s timeout=%s runtime_checks=%s workdir=%s\n' \
    "$total" "$run_match" "$reject_match" "$timeout_match" "$runtime_error_skip" "$optional_semantics_skip" "$stress_runtime_skip" "$expect_pass_match" "$expect_reject_match" "$mismatch" "$compile_only_reject" "$interp_only_reject" "$link_fail" "$run_fail" "$timeout_count" "$runtime_checks_enabled" "$work"

if [[ -s "$failures" ]]; then
    cat "$failures"
fi

if [[ "$mismatch" -ne 0 || "$link_fail" -ne 0 || "$run_fail" -ne 0 ]]; then
    exit 1
fi
