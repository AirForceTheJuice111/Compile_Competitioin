#!/usr/bin/env bash
set -euo pipefail
PROJECT_ROOT=${PROJECT_ROOT:-"$(cd "$(dirname "$0")/.." && pwd)"}
COMPILER=${COMPILER:-"$PROJECT_ROOT/build/compiler"}
BASELINE_COMPILER=${BASELINE_COMPILER:-}
TEST_ROOT=${TEST_ROOT:-"$PROJECT_ROOT/bench_local"}
OUT_DIR=${OUT_DIR:-/tmp/compiler_h_static_regression}
AARCH64_CC=${AARCH64_CC:-clang}
WORKERS=${WORKERS:-2}
SYSY_OPT=${SYSY_OPT:--O1}
PARALLEL_MODE=${PARALLEL_MODE:-official}
EXCLUDE_REGEX=${EXCLUDE_REGEX:-}
[[ -x "$COMPILER" ]] || { echo "compiler not found: $COMPILER" >&2; exit 2; }
[[ -d "$TEST_ROOT" ]] || { echo "test root not found: $TEST_ROOT" >&2; exit 2; }
command -v "$AARCH64_CC" >/dev/null || { echo "assembler driver not found: $AARCH64_CC" >&2; exit 2; }
case "$PARALLEL_MODE" in
 official) extra_args=() ;;
 serial) extra_args=(--no-parallel-native) ;;
 parallel) extra_args=(--parallel-native) ;;
 *) echo "PARALLEL_MODE must be official, serial, or parallel" >&2; exit 2 ;;
esac
rm -rf "$OUT_DIR"; mkdir -p "$OUT_DIR/optimized" "$OUT_DIR/baseline"
find "$TEST_ROOT" -type f -name '*.sy' | sort > "$OUT_DIR/all-files.txt"
if [[ -n "$EXCLUDE_REGEX" ]]; then grep -Ev "$EXCLUDE_REGEX" "$OUT_DIR/all-files.txt" > "$OUT_DIR/files.txt" || true; else cp "$OUT_DIR/all-files.txt" "$OUT_DIR/files.txt"; fi
[[ -s "$OUT_DIR/files.txt" ]] || { echo "no .sy files selected" >&2; exit 2; }
compile_tree(){
 local compiler=$1 target=$2
 export compiler target TEST_ROOT SYSY_OPT AARCH64_CC EXTRA_ARGS="${extra_args[*]}"
 tr '\n' '\0' < "$OUT_DIR/files.txt" | xargs -0 -P "$WORKERS" -I{} /bin/bash -lc '
  set -euo pipefail; f="$1"; rel="${f#$TEST_ROOT/}"; stem="${rel%.sy}"; mkdir -p "$target/$(dirname "$stem")";
  read -r -a opt_args <<< "$SYSY_OPT"; read -r -a extra <<< "$EXTRA_ARGS";
  "$compiler" "$f" -S -o "$target/$stem.s" "${opt_args[@]}" "${extra[@]}";
  "$AARCH64_CC" --target=aarch64-linux-gnu -c "$target/$stem.s" -o "$target/$stem.o"
 ' _ {}
}
compile_tree "$COMPILER" "$OUT_DIR/optimized"
if [[ -n "$BASELINE_COMPILER" ]]; then [[ -x "$BASELINE_COMPILER" ]] || { echo "bad baseline compiler" >&2; exit 2; }; compile_tree "$BASELINE_COMPILER" "$OUT_DIR/baseline"; fi
python3 - "$OUT_DIR" "$PARALLEL_MODE" <<'PY'
from pathlib import Path
import csv,re,sys
root=Path(sys.argv[1]); mode=sys.argv[2]
def metrics(p):
 ins=[]
 for line in p.read_text(errors='ignore').splitlines():
  s=line.strip()
  if not s or s.startswith(('.', '//','#')) or s.endswith(':'): continue
  if re.match(r'^[A-Za-z][A-Za-z0-9.]*\s',s): ins.append(s)
 return {'instructions':len(ins),'branches':sum(bool(re.match(r'^(b(?:\.[a-z]+)?\s|bl\s|blr\s|br\s|cb\w*\s|tb\w*\s)',x)) for x in ins),'loads':sum(bool(re.match(r'^(ldr|ldp|ldur)\b',x)) for x in ins),'stores':sum(bool(re.match(r'^(str|stp|stur)\b',x)) for x in ins),'calls':sum(bool(re.match(r'^(bl|blr)\b',x)) for x in ins),'assembly_bytes':p.stat().st_size}
rows=[]; opt=root/'optimized'; base=root/'baseline'
for p in sorted(opt.rglob('*.s')):
 rel=p.relative_to(opt); om=metrics(p); bp=base/rel; bm=metrics(bp) if bp.exists() else None; r={'case':str(rel),'mode':mode}
 for k,v in om.items(): r['optimized_'+k]=v; r['baseline_'+k]='' if bm is None else bm[k]; r['delta_'+k]='' if bm is None else v-bm[k]
 rows.append(r)
with (root/'case-results.tsv').open('w',newline='') as f: w=csv.DictWriter(f,delimiter='\t',fieldnames=list(rows[0])); w.writeheader(); w.writerows(rows)
keys=['instructions','branches','loads','stores','calls','assembly_bytes']
with (root/'summary.tsv').open('w',newline='') as f:
 w=csv.writer(f,delimiter='\t'); w.writerow(['mode','metric','baseline','optimized','delta','delta_percent'])
 for k in keys:
  o=sum(int(r['optimized_'+k]) for r in rows); vals=[r['baseline_'+k] for r in rows]
  if any(v=='' for v in vals): w.writerow([mode,k,'',o,'',''])
  else:
   b=sum(int(v) for v in vals); d=o-b; w.writerow([mode,k,b,o,d,f'{d*100/b:.2f}%'])
regs=[r for r in rows if r['delta_instructions']!='' and int(r['delta_instructions'])>0]
print(f'validated {len(rows)} AArch64 files'); print((root/'summary.tsv').read_text(),end=''); print(f'instruction-count regressions: {len(regs)}')
PY
