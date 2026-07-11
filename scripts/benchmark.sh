#!/usr/bin/env bash
# Performance benchmark script for Compiler-H optimizations.
# Run after each optimization to see cumulative impact.
#
# Usage:
#   bash scripts/benchmark.sh                    # run all perf tests
#   bash scripts/benchmark.sh 2025-45Z-11        # run single test
#   MAX_CASES=3 bash scripts/benchmark.sh         # limit to N cases
set -euo pipefail

COMPILER=${COMPILER:-"$(pwd)/build/compiler"}
AARCH64_CC=${AARCH64_CC:-clang-18}
AARCH64_CC_FLAGS=${AARCH64_CC_FLAGS:---target=aarch64-linux-gnu}
QEMU_AARCH64=${QEMU_AARCH64:-qemu-aarch64}
SYSY_AARCH64_SYSROOT=${SYSY_AARCH64_SYSROOT:-/usr/aarch64-linux-gnu}
LIBSYSY_AARCH64_C=${LIBSYSY_AARCH64_C:-"$(pwd)/vendor/libsysy/sylib.c"}
PERF_DIR=${PERF_DIR:-"$(pwd)/test/performance_final"}
MAX_CASES=${MAX_CASES:-999}
FILTER=${1:-}

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

if [[ ! -x "$COMPILER" ]]; then
    echo "missing compiler: $COMPILER" >&2
    exit 2
fi

run_one() {
    local src="$1" opt="$2"
    local base=$(basename "$src" .sy)
    local work="/tmp/bench_${base}_${opt}"
    rm -rf "$work" && mkdir -p "$work"
    local asm="$work/$base.s"
    local exe="$work/$base.aarch64"

    "$COMPILER" -S -o "$asm" "$opt" "$src" 2>/dev/null
    local needs_parallel=0
    grep -Eq '^[[:space:]]*bl[[:space:]]+__sysy_parallel_(for_range|reduce_int_range)' "$asm" && needs_parallel=1 || true

    local link_cmd=("$AARCH64_CC" $AARCH64_CC_FLAGS -o "$exe" "$asm" "$LIBSYSY_AARCH64_C" -lm)
    [[ "$needs_parallel" == 1 ]] && link_cmd+=(-pthread)
    "${link_cmd[@]}" 2>/dev/null

    local input="${src%.sy}.in"
    if [[ -f "$input" ]]; then
        "$QEMU_AARCH64" -L "$SYSY_AARCH64_SYSROOT" "$exe" < "$input" > /dev/null 2>&1
    else
        "$QEMU_AARCH64" -L "$SYSY_AARCH64_SYSROOT" "$exe" > /dev/null 2>&1
    fi
    # Return just the exit code for correctness
}

time_one() {
    local src="$1" opt="$2"
    local base=$(basename "$src" .sy)
    local work="/tmp/bench_${base}_${opt}"
    rm -rf "$work" && mkdir -p "$work"
    local asm="$work/$base.s"
    local exe="$work/$base.aarch64"

    "$COMPILER" -S -o "$asm" "$opt" "$src" 2>/dev/null || { echo "COMPILE_FAIL"; return; }
    local needs_parallel=0
    grep -Eq '^[[:space:]]*bl[[:space:]]+__sysy_parallel_(for_range|reduce_int_range)' "$asm" && needs_parallel=1 || true

    local link_cmd=("$AARCH64_CC" $AARCH64_CC_FLAGS -o "$exe" "$asm" "$LIBSYSY_AARCH64_C" -lm)
    [[ "$needs_parallel" == 1 ]] && link_cmd+=(-pthread)
    "${link_cmd[@]}" 2>/dev/null || { echo "LINK_FAIL"; return; }

    local input="${src%.sy}.in"
    local start end
    start=$(date +%s%6N)
    if [[ -f "$input" ]]; then
        "$QEMU_AARCH64" -L "$SYSY_AARCH64_SYSROOT" "$exe" < "$input" > /dev/null 2>&1
    else
        "$QEMU_AARCH64" -L "$SYSY_AARCH64_SYSROOT" "$exe" > /dev/null 2>&1
    fi
    end=$(date +%s%6N)
    echo $(( (end - start) / 1000 ))  # microseconds
}

to_ms() {
    # Convert microseconds to human-readable
    local us=$1
    if (( us >= 1000000 )); then
        echo "$(bc <<< "scale=2; $us/1000000")s"
    elif (( us >= 1000 )); then
        echo "$(bc <<< "scale=1; $us/1000")ms"
    else
        echo "${us}us"
    fi
}

# Collect test cases
mapfile -t cases < <(find "$PERF_DIR" -maxdepth 1 -name '*.sy' | sort)
if [[ -n "$FILTER" ]]; then
    filtered=()
    for c in "${cases[@]}"; do
        [[ "$c" == *"$FILTER"* ]] && filtered+=("$c")
    done
    cases=("${filtered[@]}")
fi

# Header
echo ""
echo -e "${CYAN}══════════════════════════════════════════════════════════════════${NC}"
echo -e "${CYAN}  Compiler-H Performance Benchmark${NC}"
echo -e "${CYAN}  $(date '+%Y-%m-%d %H:%M:%S')  |  Branch: $(git branch --show-current 2>/dev/null || echo unknown)${NC}"
echo -e "${CYAN}══════════════════════════════════════════════════════════════════${NC}"
echo ""

# Optimization summary first
echo -e "${YELLOW}── Optimization Summary ──${NC}"
echo -n "  GVN:        "
BACKEND_PROFILE=1 "$COMPILER" -S -o /dev/null const "$PERF_DIR/2025-45Z-11.sy" 2>&1 | grep "gvn eliminated" || echo "N/A"
echo -n "  CopyProp:   "
BACKEND_PROFILE=1 "$COMPILER" -S -o /dev/null const "$PERF_DIR/2025-45Z-11.sy" 2>&1 | grep "copyprop eliminated" || echo "N/A"
echo ""

# Table header
printf "  %-30s %12s %12s %12s %8s\n" "Test Case" "noopt" "const" "allopt" "Speedup"
printf "  %-30s %12s %12s %12s %8s\n" "------------------------------" "------------" "------------" "------------" "--------"

total_noopt=0
total_allopt=0
count=0
passed=0

for sy in "${cases[@]}"; do
    ((count >= MAX_CASES)) && break
    base=$(basename "$sy" .sy)
    name="${base:0:30}"

    # Quick correctness check
    rc_noopt=$(run_one "$sy" "noopt" && echo $? || echo "FAIL")
    rc_allopt=$(run_one "$sy" "allopt" && echo $? || echo "FAIL")
    if [[ "$rc_noopt" != "$rc_allopt" ]]; then
        printf "  ${RED}%-30s %12s %12s %12s %8s${NC}\n" "$name" "CORRECTNESS" "MISMATCH" "--" "--"
        continue
    fi

    t_noopt=$(time_one "$sy" "noopt")
    t_const=$(time_one "$sy" "const")
    t_allopt=$(time_one "$sy" "allopt")

    if [[ "$t_noopt" == *"FAIL"* || "$t_allopt" == *"FAIL"* ]]; then
        printf "  ${RED}%-30s %12s %12s %12s %8s${NC}\n" "$name" "$t_noopt" "$t_const" "$t_allopt" "FAIL"
        continue
    fi

    total_noopt=$((total_noopt + t_noopt))
    total_allopt=$((total_allopt + t_allopt))
    count=$((count + 1))
    passed=$((passed + 1))

    # Speedup: noopt / allopt
    if (( t_allopt > 0 )); then
        sp=$(bc <<< "scale=2; $t_noopt / $t_allopt")
    else
        sp="1.00"
    fi

    if (( $(bc <<< "$sp >= 1.05") )); then
        color="$GREEN"
    elif (( $(bc <<< "$sp >= 0.95") )); then
        color="$NC"
    else
        color="$RED"
    fi

    printf "  %-30s %12s %12s %12s ${color}%7sx${NC}\n" \
        "$name" "$(to_ms $t_noopt)" "$(to_ms $t_const)" "$(to_ms $t_allopt)" "$sp"
done

# Footer
echo ""
echo -e "${YELLOW}── Summary ──${NC}"
if (( passed > 0 )); then
    avg_sp=$(bc <<< "scale=2; $total_noopt / $total_allopt")
    echo "  Cases: $passed  |  Total noopt: $(to_ms $total_noopt)  |  Total allopt: $(to_ms $total_allopt)"
    echo -e "  ${GREEN}Geometric speedup: ${avg_sp}x${NC}"
fi
echo ""
