#!/usr/bin/env bash

fmj_expect() {
    local file="$1"
    sed -n -E '1,8s/^[[:space:]]*\/\/[[:space:]]*EXPECT:[[:space:]]*(PASS|FAIL).*/\1/p' "$file" | head -n 1
}

is_compile_diagnostic() {
    local log="$1"
    grep -qiE 'error|syntax|parse|semantic|invalid|undefined|not found|cannot|duplicate|must be|not allowed|failed' "$log"
}
