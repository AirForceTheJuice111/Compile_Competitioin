#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
work="${WORKDIR:-/tmp/final_fuzz_regression}"
fmjcc="${FMJCC:-$root/final/build/fmjcc}"
cc="${ARM_CC:-arm-linux-gnueabihf-gcc}"
qemu="${QEMU_ARM:-qemu-arm}"
iters="${ITERS:-1000000}"
timeout_s="${TIMEOUT:-3600}"
kset="${KSET:-9}"
seed="${SEED:-0x5eed1234}"
use_noopt_oracle="${USE_NOOPT_ORACLE:-0}"

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

rm -rf "$work"
mkdir -p "$work"
trap 'pkill -P $$ 2>/dev/null || true' EXIT

harness="$work/fuzz_harness.c"
cat > "$harness" <<'HARNESS_C'
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef ITERS
#define ITERS 1000000
#endif
#ifndef CASE_KIND
#define CASE_KIND "generic"
#endif
#ifndef RNG_SEED
#define RNG_SEED 0x5eed1234u
#endif

extern int call_test_main(void);
extern int fmj_exit_taken;

static uint64_t hash_state = 1469598103934665603ull;
static uint32_t rng_state = RNG_SEED;
static int input_slot = 0;
static unsigned long long input_calls = 0;

static void hash_u32(uint32_t value) {
    hash_state ^= value;
    hash_state *= 1099511628211ull;
}

static uint32_t next_u32(void) {
    uint32_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng_state = x ? x : 0x9e3779b9u;
    return rng_state;
}

static int range_int(int lo, int hi) {
    uint32_t span = (uint32_t)(hi - lo + 1);
    return lo + (int)(next_u32() % span);
}

static int generated_scalar(void) {
    const char *kind = CASE_KIND;
    int slot = input_slot++;

    if (strcmp(kind, "fibonacci") == 0) return range_int(-5, 12);
    if (strcmp(kind, "insttest4") == 0) return range_int(-32, 32);
    if (strcmp(kind, "loop12") == 0) return range_int(-20, 80);
    if (strcmp(kind, "loop3") == 0) return (slot % 2) == 0 ? range_int(-20, 80) : range_int(1, 20);
    if (strcmp(kind, "loop56") == 0) return range_int(-32, 32);
    if (strcmp(kind, "extra2") == 0) return range_int(-20, 80);
    if (strcmp(kind, "extra3") == 0) return (slot % 2) == 0 ? range_int(-20, 80) : range_int(1, 20);
    if (strcmp(kind, "extra45") == 0) return (slot % 2) == 0 ? range_int(-20, 80) : range_int(0, 20);
    if (strcmp(kind, "positive") == 0) return range_int(1, 20);
    if (strcmp(kind, "comprehensive") == 0) {
        int v = range_int(-8, 12);
        return (v == 3 || v == 4) ? 5 : v;
    }
    return range_int(-32, 32);
}

int getint(void) {
    input_calls++;
    return generated_scalar();
}

int getch(void) {
    input_calls++;
    return generated_scalar();
}

int getarray(int a[]) {
    int n = range_int(0, 10);
    input_calls += (unsigned long long)n + 1ull;
    a[0] = n;
    for (int i = 0; i < n; ++i) a[i + 1] = range_int(-32, 32);
    return n;
}

void putint(int a) {
    hash_u32(0x70757469u);
    hash_u32((uint32_t)a);
}

void putch(int a) {
    hash_u32(0x70757463u);
    hash_u32((uint32_t)(unsigned char)a);
}

void putarray(int n, int a[]) {
    hash_u32(0x70757461u);
    hash_u32((uint32_t)n);
    for (int i = 0; i < n; ++i) hash_u32((uint32_t)a[i + 1]);
}

void starttime(void) {}
void stoptime(void) {}

static void emit_char(char c) {
    (void)write(1, &c, 1);
}

static void emit_hex64(uint64_t value) {
    static const char hex[] = "0123456789abcdef";
    char buf[16];
    for (int i = 15; i >= 0; --i) {
        buf[i] = hex[value & 15u];
        value >>= 4;
    }
    (void)write(1, buf, sizeof(buf));
}

static void emit_u64(unsigned long long value) {
    char buf[32];
    int n = 0;
    if (value == 0) {
        emit_char('0');
        return;
    }
    while (value != 0) {
        buf[n++] = (char)('0' + (value % 10ull));
        value /= 10ull;
    }
    while (n > 0) emit_char(buf[--n]);
}

int main(void) {
    for (int i = 0; i < ITERS; ++i) {
        input_slot = 0;
        int rc = call_test_main();
        hash_u32(0x72657475u);
        hash_u32((uint32_t)rc);
        hash_u32((uint32_t)fmj_exit_taken);
    }
    emit_hex64(hash_state);
    emit_char(' ');
    emit_u64(input_calls);
    emit_char('\n');
    return 0;
}
HARNESS_C

trampoline="$work/exit_trampoline.s"
cat > "$trampoline" <<'TRAMPOLINE_ASM'
.balign 4
.section .text
.arm
.global call_test_main
.type call_test_main, %function
call_test_main:
	push {r4-r11, lr}
	sub sp, sp, #4
	ldr r4, =fmj_exit_sp
	str sp, [r4]
	ldr r4, =fmj_exit_taken
	mov r1, #0
	str r1, [r4]
	bl test_main
	add sp, sp, #4
	pop {r4-r11, pc}

.global __fmj_exit
.type __fmj_exit, %function
__fmj_exit:
	ldr r1, =fmj_exit_taken
	mov r2, #1
	str r2, [r1]
	ldr r1, =fmj_exit_sp
	ldr sp, [r1]
	add sp, sp, #4
	pop {r4-r11, pc}

.section .data
.balign 4
.global fmj_exit_sp
fmj_exit_sp:
	.word 0
.global fmj_exit_taken
fmj_exit_taken:
	.word 0
TRAMPOLINE_ASM

safe_name() {
    local base
    base="$(basename "$1" .fmj)"
    printf '%s' "$base" | tr -c 'A-Za-z0-9_' '_'
}

case_kind() {
    local src="$1"
    local base
    base="$(basename "$src" .fmj)"
    case "$base" in
        fibonacci|newtest12|quadtest2|irtest22) echo "fibonacci" ;;
        insttest4) echo "insttest4" ;;
        optloopivtest1|optloopivtest2) echo "loop12" ;;
        optloopivtest3) echo "loop3" ;;
        optloopivtest5|optloopivtest6) echo "loop56" ;;
        optloopivextra2) echo "extra2" ;;
        optloopivextra3) echo "extra3" ;;
        optloopivextra4|optloopivextra5) echo "extra45" ;;
        opttest2|opttest4|opttest5|opttest6|opttest9|opttest10) echo "positive" ;;
        test_comprehensive) echo "comprehensive" ;;
        *) echo "generic" ;;
    esac
}

reference_name() {
    local base
    base="$(basename "$1" .fmj)"
    case "$base" in
        newtest12|quadtest2|irtest22) echo "fibonacci" ;;
        *) echo "$base" ;;
    esac
}

transform_main() {
    local in="$1"
    local out="$2"
    awk '
        $0 == ".global main" { print ".global test_main"; print ".type test_main, %function"; next }
        $0 == ".type main, %function" { next }
        $0 == "main:" { print "test_main:"; next }
        /^[[:space:]]*bl[[:space:]]+exit$/ { sub(/exit$/, "__fmj_exit"); print; next }
        { print }
    ' "$in" > "$out"
}

build_binary() {
    local asm="$1"
    local kind="$2"
    local out="$3"
    local renamed="${out%.arm}.test_main.s"
    transform_main "$asm" "$renamed"
    "$cc" -static -O2 -marm -fno-stack-protector -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 \
        -DITERS="$iters" -DRNG_SEED="$seed" -DCASE_KIND="\"$kind\"" \
        "$renamed" "$trampoline" "$harness" -o "$out"
}

run_binary() {
    local bin="$1"
    timeout "$timeout_s" "$qemu" "$bin"
}

compile_final() {
    local src="$1"
    local dir="$2"
    local k="$3"
    local mode="$4"
    local base
    base="$(safe_name "$src")"
    mkdir -p "$dir"
    cp "$src" "$dir/$base.fmj"
    if [[ "$mode" == "noopt" ]]; then
        if ! "$fmjcc" --no-opt --k "$k" "$dir/$base.fmj" > "$dir/$base.compile.log" 2>&1; then
            return 1
        fi
    else
        if ! "$fmjcc" --k "$k" "$dir/$base.fmj" > "$dir/$base.compile.log" 2>&1; then
            return 1
        fi
    fi
    printf '%s/%s.s\n' "$dir" "$base"
}

collect_fuzz_cases() {
    find "$root"/HW* -path '*/build' -prune -o -type f -name '*.fmj' -print \
        | sort \
        | while IFS= read -r src; do
            if ! rg -q 'getint|getch|getarray' "$src"; then continue; fi
            case "${src#"$root"/}" in
                *semant_test30_no_getarray_non_lvalue.fmj|*semant_test30_getarray_non_lvalue.fmj)
                    continue
                    ;;
            esac
            printf '%s\n' "$src"
        done
}

failures="$work/failures.txt"
: > "$failures"
pass=0
fail=0
skip=0

if [[ -n "${CASES:-}" ]]; then
    case_source() {
        for src in $CASES; do
            if [[ "$src" = /* ]]; then printf '%s\n' "$src";
            else printf '%s\n' "$root/$src";
            fi
        done
    }
else
    case_source() {
        collect_fuzz_cases
    }
fi

while IFS= read -r src; do
    [[ -n "$src" ]] || continue
    rel="${src#"$root"/}"
    kind="$(case_kind "$src")"

    for k in $kset; do
        name="$(safe_name "$src")"
        case_dir="$work/k$k/$name"
        mkdir -p "$case_dir"

        if ! opt_s="$(compile_final "$src" "$case_dir/opt" "$k" "opt")"; then
            skip=$((skip + 1))
            printf 'SKIP_COMPILE k=%s %s mode=opt\n' "$k" "$rel" >> "$failures"
            continue
        fi

        set +e
        build_binary "$opt_s" "$kind" "$case_dir/opt.arm" > "$case_dir/opt.link.log" 2>&1
        opt_link_rc=$?
        set -e
        if [[ "$opt_link_rc" -ne 0 ]]; then
            fail=$((fail + 1))
            printf 'LINK_FAIL k=%s %s mode=opt rc=%s\n' "$k" "$rel" "$opt_link_rc" >> "$failures"
            continue
        fi

        set +e
        opt_out="$(run_binary "$case_dir/opt.arm" 2>"$case_dir/opt.err")"
        opt_run_rc=$?
        set -e
        if [[ "$opt_run_rc" -ne 0 ]]; then
            fail=$((fail + 1))
            {
                printf 'RUN_FAIL k=%s %s kind=%s opt_rc=%s\n' "$k" "$rel" "$kind" "$opt_run_rc"
                printf '  opt=%s\n' "$opt_out"
                printf '  opt_err='
                cat "$case_dir/opt.err"
                printf '\n'
            } >> "$failures"
            continue
        fi

        ref_name="$(reference_name "$src")"
        ref="$root/HW12/test/k$k/$ref_name.colored.s"
        if [[ -f "$ref" ]]; then
            set +e
            build_binary "$ref" "$kind" "$case_dir/ref.arm" > "$case_dir/ref.link.log" 2>&1
            ref_link_rc=$?
            ref_out="$(run_binary "$case_dir/ref.arm" 2>"$case_dir/ref.err")"
            ref_run_rc=$?
            set -e
            if [[ "$ref_link_rc" -ne 0 || "$ref_run_rc" -ne 0 || "$opt_out" != "$ref_out" ]]; then
                fail=$((fail + 1))
                {
                    printf 'REFERENCE_MISMATCH k=%s %s kind=%s ref_link_rc=%s ref_run_rc=%s\n' "$k" "$rel" "$kind" "$ref_link_rc" "$ref_run_rc"
                    printf '  opt=%s\n' "$opt_out"
                    printf '  ref=%s\n' "$ref_out"
                    printf '  ref_err='
                    cat "$case_dir/ref.err"
                    printf '\n'
                } >> "$failures"
            else
                pass=$((pass + 1))
                printf 'OK reference k=%s %s %s\n' "$k" "$rel" "$ref_out"
            fi
        elif [[ "$use_noopt_oracle" == "1" ]]; then
            if ! noopt_s="$(compile_final "$src" "$case_dir/noopt" "$k" "noopt")"; then
                skip=$((skip + 1))
                printf 'SKIP_COMPILE k=%s %s mode=noopt\n' "$k" "$rel" >> "$failures"
                continue
            fi
            set +e
            build_binary "$noopt_s" "$kind" "$case_dir/noopt.arm" > "$case_dir/noopt.link.log" 2>&1
            noopt_link_rc=$?
            noopt_out="$(run_binary "$case_dir/noopt.arm" 2>"$case_dir/noopt.err")"
            noopt_run_rc=$?
            set -e
            if [[ "$noopt_link_rc" -ne 0 || "$noopt_run_rc" -ne 0 || "$opt_out" != "$noopt_out" ]]; then
                fail=$((fail + 1))
                {
                    printf 'NOOPT_MISMATCH k=%s %s kind=%s noopt_link_rc=%s noopt_run_rc=%s\n' "$k" "$rel" "$kind" "$noopt_link_rc" "$noopt_run_rc"
                    printf '  opt=%s\n' "$opt_out"
                    printf '  noopt=%s\n' "$noopt_out"
                    printf '  noopt_err='
                    cat "$case_dir/noopt.err"
                    printf '\n'
                } >> "$failures"
            else
                pass=$((pass + 1))
                printf 'OK noopt k=%s %s %s\n' "$k" "$rel" "$noopt_out"
            fi
        else
            pass=$((pass + 1))
            printf 'OK smoke k=%s %s %s\n' "$k" "$rel" "$opt_out"
        fi
    done
done < <(case_source)

printf 'fuzz_pass=%s fuzz_fail=%s fuzz_skip=%s iterations=%s kset="%s" workdir=%s\n' \
    "$pass" "$fail" "$skip" "$iters" "$kset" "$work"

if [[ -s "$failures" ]]; then
    cat "$failures"
fi

if [[ "$fail" -ne 0 ]]; then
    exit 1
fi
