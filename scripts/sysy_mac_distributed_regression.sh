#!/usr/bin/env bash
# Run exact-output SysY regression tests in deterministic shards inside an
# ARM64 Multipass VM hosted by a Mac.  This file is also copied into the VM
# payload and acts as the shard runner there.

set -euo pipefail
export LC_ALL=C

die() {
  echo "error: $*" >&2
  exit 2
}

normalize_output() {
  local input=$1
  local output=$2
  tr -d '\r' <"$input" >"$output"
  if [[ -s "$output" ]]; then
    local last_byte
    last_byte=$(tail -c 1 "$output" | od -An -t u1 | tr -d ' ')
    if [[ "$last_byte" != 10 ]]; then
      printf '\n' >>"$output"
    fi
  fi
}

vm_worker() {
  [[ $# -eq 5 ]] || die "internal worker expects CACHE RUN_DIR SHARD COUNT TIMEOUT"
  local cache=$1
  local run_dir=$2
  local shard=$3
  local shard_count=$4
  local case_timeout=$5
  local compiler="$cache/build/compiler"
  local test_root="$cache/tests"
  local runtime_object="$cache/build/sylib.o"
  local sysy_opt=

  if [[ -n "${SYSY_OPT_B64:-}" ]]; then
    sysy_opt=$(printf '%s' "$SYSY_OPT_B64" | base64 -d)
  fi

  [[ -x "$compiler" ]] || die "missing cached compiler: $compiler"
  [[ -f "$runtime_object" ]] || die "missing cached runtime: $runtime_object"
  [[ -d "$test_root" ]] || die "missing cached tests: $test_root"

  rm -rf "$run_dir/cases"
  rm -f "$run_dir/failures.tsv" "$run_dir/summary.env"
  mkdir -p "$run_dir/cases"

  local total=0
  local passed=0
  local compile_fail=0
  local link_fail=0
  local timed_out=0
  local wrong=0
  local eligible=0
  local sy expected_file rel stem safe case_dir asm exe stdout_file stderr_file
  local actual_norm expected_norm input_file rc status
  local -a sysy_opt_args=()
  if [[ -n "$sysy_opt" ]]; then
    read -r -a sysy_opt_args <<<"$sysy_opt"
  fi

  printf 'case\tstatus\treturn_code\n' >"$run_dir/failures.tsv"
  mapfile -d '' -t cases < <(find "$test_root" -type f -name '*.sy' -print0 | sort -z)

  for sy in "${cases[@]}"; do
    expected_file="${sy%.sy}.out"
    [[ -f "$expected_file" ]] || continue

    if (( eligible % shard_count != shard )); then
      eligible=$((eligible + 1))
      continue
    fi

    total=$((total + 1))
    rel=${sy#"$test_root"/}
    stem=${rel%.sy}
    safe=$(printf '%06d_%s' "$eligible" "$stem" | tr -c 'A-Za-z0-9_.-' '_')
    case_dir="$run_dir/cases/$safe"
    mkdir -p "$case_dir"
    asm="$case_dir/program.s"
    exe="$case_dir/program"
    stdout_file="$case_dir/stdout"
    stderr_file="$case_dir/stderr"
    actual_norm="$case_dir/actual"
    expected_norm="$case_dir/expected"
    input_file="${sy%.sy}.in"
    status=PASS
    rc=-

    printf '[shard %02d/%02d case %04d] %-58s ' \
      "$shard" "$shard_count" "$eligible" "$rel"

    if ! "$compiler" -S "${sysy_opt_args[@]}" -o "$asm" "$sy" \
      >"$case_dir/compile.stdout" 2>"$case_dir/compile.stderr"; then
      status=COMPILE_FAIL
      compile_fail=$((compile_fail + 1))
    elif ! cc -o "$exe" "$asm" "$runtime_object" -lm -pthread \
      >"$case_dir/link.stdout" 2>"$case_dir/link.stderr"; then
      status=LINK_FAIL
      link_fail=$((link_fail + 1))
    else
      set +e
      if [[ "$case_timeout" == 0 ]]; then
        if [[ -f "$input_file" ]]; then
          "$exe" <"$input_file" >"$stdout_file" 2>"$stderr_file"
        else
          "$exe" >"$stdout_file" 2>"$stderr_file"
        fi
      elif [[ -f "$input_file" ]]; then
        timeout --kill-after=5 "$case_timeout" "$exe" \
          <"$input_file" >"$stdout_file" 2>"$stderr_file"
      else
        timeout --kill-after=5 "$case_timeout" "$exe" \
          >"$stdout_file" 2>"$stderr_file"
      fi
      rc=$?
      set -e

      if [[ "$case_timeout" != 0 && "$rc" == 124 ]]; then
        status=TIMEOUT
        timed_out=$((timed_out + 1))
      else
        normalize_output "$stdout_file" "$actual_norm"
        printf '%s\n' "$rc" >>"$actual_norm"
        normalize_output "$expected_file" "$expected_norm"
        if cmp -s "$actual_norm" "$expected_norm"; then
          passed=$((passed + 1))
        else
          status=WRONG
          wrong=$((wrong + 1))
          diff -u "$expected_norm" "$actual_norm" >"$case_dir/diff" || true
        fi
      fi
    fi

    echo "$status"
    if [[ "$status" != PASS ]]; then
      printf '%s\t%s\t%s\n' "$rel" "$status" "$rc" >>"$run_dir/failures.tsv"
    elif [[ "${KEEP_PASS_ARTIFACTS:-0}" != 1 ]]; then
      rm -rf "$case_dir"
    fi
    eligible=$((eligible + 1))
  done

  {
    printf 'shard=%s\n' "$shard"
    printf 'total=%s\n' "$total"
    printf 'pass=%s\n' "$passed"
    printf 'compile_fail=%s\n' "$compile_fail"
    printf 'link_fail=%s\n' "$link_fail"
    printf 'timeout=%s\n' "$timed_out"
    printf 'wrong=%s\n' "$wrong"
  } >"$run_dir/summary.env"

  echo "shard summary: shard=$shard total=$total pass=$passed compile_fail=$compile_fail link_fail=$link_fail timeout=$timed_out wrong=$wrong"
  if (( compile_fail + link_fail + timed_out + wrong != 0 )); then
    return 1
  fi
}

summary_value() {
  local key=$1
  local file=$2
  sed -n "s/^${key}=//p" "$file"
}

vm_run() {
  [[ $# -eq 4 ]] || die "internal VM runner expects CACHE RUN_DIR WORKERS TIMEOUT"
  local cache=$1
  local run_dir=$2
  local workers=$3
  local case_timeout=$4
  local start_ns end_ns wall_ms
  local before_mem after_mem before_load after_load
  local total=0 passed=0 compile_fail=0 link_fail=0 timed_out=0 wrong=0
  local max_shard_rss_kib=0 sum_shard_rss_kib=0 rss_kib
  local aggregate_user_seconds aggregate_system_seconds
  local overall_rc=0
  local shard shard_dir pid
  local -a pids=()

  rm -rf "$run_dir" "$run_dir.results.tar.gz"
  mkdir -p "$run_dir/shards"
  before_mem=$(awk '/MemAvailable:/ { print $2 }' /proc/meminfo)
  before_load=$(cut -d ' ' -f 1-3 /proc/loadavg)
  start_ns=$(date +%s%N)

  for ((shard = 0; shard < workers; ++shard)); do
    shard_dir=$(printf '%s/shards/shard-%02d' "$run_dir" "$shard")
    mkdir -p "$shard_dir"
    /usr/bin/time -v -o "$shard_dir/resource.txt" \
      "$0" __vm_worker "$cache" "$shard_dir" "$shard" "$workers" "$case_timeout" \
      >"$shard_dir.log" 2>&1 &
    pids+=("$!")
  done

  for pid in "${pids[@]}"; do
    if ! wait "$pid"; then
      overall_rc=1
    fi
  done

  end_ns=$(date +%s%N)
  wall_ms=$(((end_ns - start_ns) / 1000000))
  after_mem=$(awk '/MemAvailable:/ { print $2 }' /proc/meminfo)
  after_load=$(cut -d ' ' -f 1-3 /proc/loadavg)
  printf 'case\tstatus\treturn_code\tshard\n' >"$run_dir/failures.tsv"

  for ((shard = 0; shard < workers; ++shard)); do
    shard_dir=$(printf '%s/shards/shard-%02d' "$run_dir" "$shard")
    if [[ ! -f "$shard_dir/summary.env" ]]; then
      printf '<shard-%02d>\tHARNESS_FAIL\t-\t%02d\n' "$shard" "$shard" \
        >>"$run_dir/failures.tsv"
      overall_rc=1
      continue
    fi
    total=$((total + $(summary_value total "$shard_dir/summary.env")))
    passed=$((passed + $(summary_value pass "$shard_dir/summary.env")))
    compile_fail=$((compile_fail + $(summary_value compile_fail "$shard_dir/summary.env")))
    link_fail=$((link_fail + $(summary_value link_fail "$shard_dir/summary.env")))
    timed_out=$((timed_out + $(summary_value timeout "$shard_dir/summary.env")))
    wrong=$((wrong + $(summary_value wrong "$shard_dir/summary.env")))
    rss_kib=$(awk -F: '/Maximum resident set size/ { gsub(/^[[:space:]]+/, "", $2); print $2 }' \
      "$shard_dir/resource.txt")
    rss_kib=${rss_kib:-0}
    sum_shard_rss_kib=$((sum_shard_rss_kib + rss_kib))
    if (( rss_kib > max_shard_rss_kib )); then
      max_shard_rss_kib=$rss_kib
    fi
    tail -n +2 "$shard_dir/failures.tsv" | \
      awk -v shard="$shard" 'BEGIN { OFS="\t" } NF { print $0, shard }' \
      >>"$run_dir/failures.tsv"
  done
  aggregate_user_seconds=$(awk -F: '/User time \(seconds\)/ { sum += $2 } END { printf "%.3f", sum }' \
    "$run_dir"/shards/*/resource.txt)
  aggregate_system_seconds=$(awk -F: '/System time \(seconds\)/ { sum += $2 } END { printf "%.3f", sum }' \
    "$run_dir"/shards/*/resource.txt)

  {
    printf 'workers=%s\n' "$workers"
    printf 'total=%s\n' "$total"
    printf 'pass=%s\n' "$passed"
    printf 'compile_fail=%s\n' "$compile_fail"
    printf 'link_fail=%s\n' "$link_fail"
    printf 'timeout=%s\n' "$timed_out"
    printf 'wrong=%s\n' "$wrong"
    printf 'wall_milliseconds=%s\n' "$wall_ms"
    printf 'mem_available_before_kib=%s\n' "$before_mem"
    printf 'mem_available_after_kib=%s\n' "$after_mem"
    printf 'load_before=%s\n' "$before_load"
    printf 'load_after=%s\n' "$after_load"
    printf 'max_shard_rss_kib=%s\n' "$max_shard_rss_kib"
    printf 'sum_shard_peak_rss_kib=%s\n' "$sum_shard_rss_kib"
    printf 'aggregate_user_seconds=%s\n' "$aggregate_user_seconds"
    printf 'aggregate_system_seconds=%s\n' "$aggregate_system_seconds"
  } >"$run_dir/summary.env"

  printf 'summary: workers=%d total=%d pass=%d compile_fail=%d link_fail=%d timeout=%d wrong=%d wall=%.3fs max_shard_rss=%dKiB\n' \
    "$workers" "$total" "$passed" "$compile_fail" "$link_fail" "$timed_out" "$wrong" \
    "$(awk -v ms="$wall_ms" 'BEGIN { printf "%.3f", ms / 1000 }')" "$max_shard_rss_kib"
  if (( overall_rc != 0 )); then
    echo "failure summary:"
    sed -n '1,80p' "$run_dir/failures.tsv"
  fi

  tar -czf "$run_dir.results.tar.gz" -C "$run_dir" .
  return "$overall_rc"
}

usage() {
  cat <<'EOF'
Usage: scripts/sysy_mac_distributed_regression.sh [options]

Options:
  --test-root DIR       Test tree containing matching .sy/.out files
                        (default: test/functional)
  --workers N           Deterministic shard count, 1..16 (default: 16)
  --benchmark           Run a one-worker baseline, then the requested count
  --case-timeout SEC    Per-case timeout; 0 preserves unrestricted semantics
                        (default: 0)
  --result-root DIR     Local result directory (default: /tmp/...)
  --prepare             Install the small build toolchain in the VM first
  --clean-cache         Remove cached payloads/runs in the VM and exit
  --list-cache          Show cached payload/resource usage and exit
  --keep-remote         Retain VM run directories after pulling results
  --help                Show this help

Environment:
  MAC_HOST, VM_NAME, MULTIPASS_BIN, SYSY_OPT, SYSY_THREADS, BUILD_JOBS,
  REMOTE_BASE, KEEP_PASS_ARTIFACTS (0 or 1)
EOF
}

quote_command() {
  local result= arg quoted
  for arg in "$@"; do
    printf -v quoted '%q' "$arg"
    result+="${result:+ }$quoted"
  done
  printf '%s' "$result"
}

controller() {
  local script_path script_dir repo_root
  script_path=$(readlink -f "$0")
  script_dir=$(dirname "$script_path")
  repo_root=$(cd "$script_dir/.." && pwd)

  local mac_host=${MAC_HOST:-wsy@192.168.1.106}
  local vm_name=${VM_NAME:-docker-node}
  local multipass_bin=${MULTIPASS_BIN:-/Library/Application Support/com.canonical.multipass/bin/multipass}
  local remote_base=${REMOTE_BASE:-/home/ubuntu/.cache/sysy-mac-regression}
  local test_root="$repo_root/test/functional"
  local result_root=${RESULT_ROOT:-/tmp/sysy_mac_distributed_regression}
  local workers=16
  local case_timeout=0
  local prepare=0 clean_cache=0 list_cache=0 keep_remote=0 benchmark=0

  while [[ $# -gt 0 ]]; do
    case "$1" in
      --test-root) [[ $# -ge 2 ]] || die "--test-root needs a value"; test_root=$2; shift 2 ;;
      --workers) [[ $# -ge 2 ]] || die "--workers needs a value"; workers=$2; shift 2 ;;
      --case-timeout) [[ $# -ge 2 ]] || die "--case-timeout needs a value"; case_timeout=$2; shift 2 ;;
      --result-root) [[ $# -ge 2 ]] || die "--result-root needs a value"; result_root=$2; shift 2 ;;
      --prepare) prepare=1; shift ;;
      --clean-cache) clean_cache=1; shift ;;
      --list-cache) list_cache=1; shift ;;
      --keep-remote) keep_remote=1; shift ;;
      --benchmark) benchmark=1; shift ;;
      --help|-h) usage; return 0 ;;
      *) die "unknown option: $1" ;;
    esac
  done

  [[ -d "$test_root" ]] || die "missing test root: $test_root"
  case "$remote_base" in
    /home/ubuntu/.cache/*|/tmp/*) ;;
    *) die "REMOTE_BASE must be below /home/ubuntu/.cache or /tmp" ;;
  esac
  [[ "$workers" =~ ^[0-9]+$ ]] && (( workers >= 1 && workers <= 16 )) || \
    die "--workers must be between 1 and 16"
  [[ "$case_timeout" =~ ^[0-9]+$ ]] || die "--case-timeout must be a non-negative integer"

  run_mac() {
    local command
    command=$(quote_command "$@")
    ssh -o BatchMode=yes -o ConnectTimeout=10 "$mac_host" "$command"
  }

  mp() {
    run_mac "$multipass_bin" "$@"
  }

  mp_exec() {
    mp exec "$vm_name" -- "$@"
  }

  if (( prepare )); then
    echo "preparing ARM64 VM '$vm_name' ..."
    mp_exec sudo env DEBIAN_FRONTEND=noninteractive apt-get update
    mp_exec sudo env DEBIAN_FRONTEND=noninteractive apt-get install -y \
      build-essential cmake ninja-build time
  fi

  if (( clean_cache )); then
    mp_exec rm -rf "$remote_base"
    echo "removed VM cache: $vm_name:$remote_base"
    return 0
  fi

  if (( list_cache )); then
    mp_exec bash -lc "if [[ -d '$remote_base' ]]; then du -sh '$remote_base'; find '$remote_base/cache' -mindepth 1 -maxdepth 1 -type d -printf '%f\\n' 2>/dev/null | sort; else echo 'cache is empty'; fi"
    return 0
  fi

  if ! mp_exec bash -lc 'for tool in cc cmake ninja time tar flock; do command -v "$tool" >/dev/null || { echo "missing VM tool: $tool" >&2; exit 2; }; done'; then
    die "VM toolchain is incomplete; rerun with --prepare"
  fi

  local vm_cpu vm_mem_kib vm_disk_kib minimum_mem_kib
  read -r vm_cpu vm_mem_kib vm_disk_kib < <(mp_exec bash -lc \
    "printf '%s ' \"\$(nproc)\"; awk '/MemAvailable:/ { printf \"%s \", \$2 }' /proc/meminfo; df -Pk /home/ubuntu | awk 'NR == 2 { print \$4 }'")
  minimum_mem_kib=$((workers * 1024 * 1024))
  (( vm_cpu >= workers )) || die "VM has $vm_cpu CPUs, fewer than $workers workers"
  (( vm_mem_kib >= minimum_mem_kib )) || \
    die "VM has only $((vm_mem_kib / 1024)) MiB available; need at least $((minimum_mem_kib / 1024)) MiB"
  echo "VM capacity: cpus=$vm_cpu available_memory=$((vm_mem_kib / 1024))MiB available_disk=$((vm_disk_kib / 1024))MiB workers=$workers"

  local staging archive archive_hash cache_dir vm_archive
  staging=$(mktemp -d /tmp/sysy-mac-payload.XXXXXX)
  archive=$(mktemp /tmp/sysy-mac-payload.XXXXXX.tar.gz)
  local cleanup_command
  cleanup_command=$(quote_command rm -rf -- "$staging" "$archive")
  trap "$cleanup_command" EXIT
  mkdir -p "$staging/src" "$staging/tests"
  cp -a "$repo_root/CMakeLists.txt" "$repo_root/include" "$repo_root/lib" \
    "$repo_root/tools" "$repo_root/vendor" "$staging/src/"
  cp -a "$test_root"/. "$staging/tests/"
  cp -a "$script_path" "$staging/harness.sh"
  chmod +x "$staging/harness.sh"
  tar --sort=name --mtime='UTC 1970-01-01' --owner=0 --group=0 --numeric-owner \
    -czf "$archive" -C "$staging" .
  archive_hash=$(sha256sum "$archive" | awk '{ print $1 }')
  cache_dir="$remote_base/cache/$archive_hash"
  vm_archive="/tmp/sysy-regression-$archive_hash-$$.tar.gz"

  local archive_kib required_disk_kib
  archive_kib=$(du -k "$archive" | awk '{ print $1 }')
  required_disk_kib=$((archive_kib * 4 + 1024 * 1024))
  (( vm_disk_kib >= required_disk_kib )) || \
    die "VM disk has only $((vm_disk_kib / 1024)) MiB free; payload needs at least $((required_disk_kib / 1024)) MiB headroom"

  local build_jobs=${BUILD_JOBS:-16}
  [[ "$build_jobs" =~ ^[1-9][0-9]*$ ]] || die "BUILD_JOBS must be a positive integer"
  if (( build_jobs > vm_cpu )); then
    build_jobs=$vm_cpu
  fi

  if mp_exec test -x "$cache_dir/build/compiler"; then
    echo "reusing VM payload cache ${archive_hash:0:12}"
  else
    echo "syncing one payload ($(du -h "$archive" | awk '{ print $1 }'), hash ${archive_hash:0:12}) ..."
    mp_exec rm -f "$vm_archive"
    mp_exec sh -c 'umask 077; cat > "$1"' sh "$vm_archive" <"$archive"
    mp_exec bash -s -- "$vm_archive" "$cache_dir" "$build_jobs" <<'VM_BUILD'
set -euo pipefail
archive=$1
cache_dir=$2
build_jobs=$3
mkdir -p "$(dirname "$cache_dir")"
exec 9>"$cache_dir.lock"
flock 9
if [[ ! -x "$cache_dir/build/compiler" ]]; then
  tmp="$cache_dir.tmp.$$"
  rm -rf "$tmp"
  mkdir -p "$tmp"
  tar -xzf "$archive" -C "$tmp"
  cmake -S "$tmp/src" -B "$tmp/build" -G Ninja -DCMAKE_BUILD_TYPE=Release
  cmake --build "$tmp/build" --parallel "$build_jobs"
  cc -O2 -c "$tmp/src/vendor/libsysy/sylib.c" -o "$tmp/build/sylib.o"
  rm -rf "$cache_dir"
  mv "$tmp" "$cache_dir"
fi
rm -f "$archive"
VM_BUILD
    echo "built compiler once in VM cache"
  fi

  mkdir -p "$result_root"
  local sysy_opt_b64
  sysy_opt_b64=$(printf '%s' "${SYSY_OPT:-}" | base64 -w0)
  local sysy_threads=${SYSY_THREADS:-2}
  [[ "$sysy_threads" =~ ^[1-9][0-9]*$ ]] || die "SYSY_THREADS must be a positive integer"
  local keep_pass_artifacts=${KEEP_PASS_ARTIFACTS:-0}
  [[ "$keep_pass_artifacts" == 0 || "$keep_pass_artifacts" == 1 ]] || \
    die "KEEP_PASS_ARTIFACTS must be 0 or 1"
  local -a run_counts=("$workers")
  if (( benchmark && workers != 1 )); then
    run_counts=(1 "$workers")
  fi

  local count run_id vm_run vm_result local_archive local_dir run_rc
  local serial_ms= parallel_ms=
  for count in "${run_counts[@]}"; do
    run_id=$(date -u +%Y%m%dT%H%M%SZ)-${archive_hash:0:12}-w$count-$$
    vm_run="$remote_base/runs/$run_id"
    vm_result="$vm_run.results.tar.gz"
    local_archive="$result_root/$run_id.results.tar.gz"
    local_dir="$result_root/$run_id"
    echo "running $count deterministic shard(s) ..."
    set +e
    mp_exec env SYSY_OPT_B64="$sysy_opt_b64" SYSY_THREADS="$sysy_threads" \
      KEEP_PASS_ARTIFACTS="$keep_pass_artifacts" \
      "$cache_dir/harness.sh" __vm_run "$cache_dir" "$vm_run" "$count" "$case_timeout"
    run_rc=$?
    set -e

    mp_exec cat "$vm_result" >"$local_archive"
    rm -rf "$local_dir"
    mkdir -p "$local_dir"
    tar -xzf "$local_archive" -C "$local_dir"
    rm -f "$local_archive"
    echo "local logs: $local_dir"

    if (( count == 1 )); then
      serial_ms=$(summary_value wall_milliseconds "$local_dir/summary.env")
    else
      parallel_ms=$(summary_value wall_milliseconds "$local_dir/summary.env")
    fi

    if (( ! keep_remote )); then
      mp_exec rm -rf "$vm_run" "$vm_result"
    else
      echo "remote logs: $vm_name:$vm_run"
    fi
    if (( run_rc != 0 )); then
      return "$run_rc"
    fi
  done

  if [[ -n "$serial_ms" && -n "$parallel_ms" ]]; then
    awk -v serial="$serial_ms" -v parallel="$parallel_ms" \
      'BEGIN { printf "benchmark: serial=%.3fs parallel=%.3fs speedup=%.2fx wall_reduction=%.1f%%\n", serial / 1000, parallel / 1000, serial / parallel, (serial - parallel) * 100 / serial }'
  fi
}

case "${1:-}" in
  __vm_worker) shift; vm_worker "$@" ;;
  __vm_run) shift; vm_run "$@" ;;
  *) controller "$@" ;;
esac
