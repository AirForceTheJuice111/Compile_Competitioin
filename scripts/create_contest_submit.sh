#!/usr/bin/env bash
# Create or refresh a source-only contest-submit branch without switching the
# current worktree.  The first snapshot is an orphan root commit; later
# snapshots keep that isolated history unless --recreate is requested.

set -euo pipefail
export LC_ALL=C

die() {
  echo "error: $*" >&2
  exit 2
}

usage() {
  cat <<'EOF'
Usage: scripts/create_contest_submit.sh [options]

Options:
  --source REF       Snapshot this committed ref (default: HEAD)
  --branch NAME      Local submission branch (default: contest-submit)
  --message TEXT     Snapshot commit message
  --recreate         Replace the branch with a new orphan root commit
  --push             Push after creating the local snapshot
  --remote NAME      Push remote (default: contest-submit; implies --push)
  --dry-run          Show the source-only tree without updating refs
  --help             Show this help

Environment:
  SUBMIT_GIT_NAME    Commit author/committer name (default: git user.name)
  SUBMIT_GIT_EMAIL   Commit author/committer email (default: git user.email)

The snapshot contains only:
  .clang-format .gitignore CMakeLists.txt Makefile README.md
  include/ lib/ scripts/ tools/ vendor/

Examples:
  scripts/create_contest_submit.sh
  scripts/create_contest_submit.sh --push
  scripts/create_contest_submit.sh --source contest --push
  scripts/create_contest_submit.sh --recreate --push
EOF
}

source_ref=HEAD
branch=contest-submit
message="Refresh contest compiler source snapshot"
remote=contest-submit
push=0
recreate=0
dry_run=0

while (($#)); do
  case "$1" in
  --source)
    (($# >= 2)) || die "--source requires a ref"
    source_ref=$2
    shift 2
    ;;
  --branch)
    (($# >= 2)) || die "--branch requires a name"
    branch=$2
    shift 2
    ;;
  --message)
    (($# >= 2)) || die "--message requires text"
    message=$2
    shift 2
    ;;
  --recreate)
    recreate=1
    shift
    ;;
  --push)
    push=1
    shift
    ;;
  --remote)
    (($# >= 2)) || die "--remote requires a name"
    remote=$2
    push=1
    shift 2
    ;;
  --dry-run)
    dry_run=1
    shift
    ;;
  --help|-h)
    usage
    exit 0
    ;;
  *)
    die "unknown option: $1"
    ;;
  esac
done

git_dir=$(git rev-parse --git-dir 2>/dev/null) || die "not inside a Git repository"
repo_root=$(git rev-parse --show-toplevel)
cd "$repo_root"

git check-ref-format --branch "$branch" >/dev/null ||
  die "invalid branch name: $branch"
source_commit=$(git rev-parse --verify "$source_ref^{commit}" 2>/dev/null) ||
  die "source is not a commit: $source_ref"

target_ref="refs/heads/$branch"
old_commit=$(git rev-parse --verify "$target_ref^{commit}" 2>/dev/null || true)

if git worktree list --porcelain |
    grep -Fqx "branch $target_ref"; then
  die "$branch is checked out in a worktree; detach or switch it first"
fi

if ((push)); then
  git remote get-url "$remote" >/dev/null 2>&1 ||
    die "unknown push remote: $remote"
fi

# This explicit allowlist is the submission boundary.  Paths not committed in
# source_commit cannot enter the snapshot, and tests/docs/build artifacts are
# intentionally absent.
include_paths=(
  .clang-format
  .gitignore
  CMakeLists.txt
  Makefile
  README.md
  include
  lib
  scripts
  tools
  vendor
)

tmp_index=$(mktemp "${TMPDIR:-/tmp}/contest-submit-index.XXXXXX")
rm -f "$tmp_index"
cleanup() {
  rm -f "$tmp_index"
}
trap cleanup EXIT

GIT_INDEX_FILE="$tmp_index" git read-tree --empty
git ls-tree -r -z "$source_commit" -- "${include_paths[@]}" |
  GIT_INDEX_FILE="$tmp_index" git update-index -z --index-info
snapshot_tree=$(GIT_INDEX_FILE="$tmp_index" git write-tree)

snapshot_roots=$(GIT_INDEX_FILE="$tmp_index" git ls-files -z |
  while IFS= read -r -d '' path; do
    printf '%s\n' "${path%%/*}"
  done | sort -u)
[[ -n "$snapshot_roots" ]] || die "source-only snapshot is empty"

echo "source:  $source_ref ($source_commit)"
echo "branch:  $branch"
echo "tree:    $snapshot_tree"
echo "roots:"
sed 's/^/  - /' <<<"$snapshot_roots"

if ((dry_run)); then
  echo "dry-run: no commit or ref was changed"
  exit 0
fi

if [[ -n "$old_commit" ]] && ((recreate == 0)); then
  old_tree=$(git rev-parse "$old_commit^{tree}")
  if [[ "$old_tree" == "$snapshot_tree" ]]; then
    echo "$branch is already up to date at $old_commit"
    new_commit=$old_commit
  fi
fi

if [[ -z "${new_commit:-}" ]]; then
  author_name=${SUBMIT_GIT_NAME:-$(git config user.name || true)}
  author_email=${SUBMIT_GIT_EMAIL:-$(git config user.email || true)}
  [[ -n "$author_name" ]] || die "set git user.name or SUBMIT_GIT_NAME"
  [[ -n "$author_email" ]] || die "set git user.email or SUBMIT_GIT_EMAIL"

  parent_args=()
  if [[ -n "$old_commit" ]] && ((recreate == 0)); then
    parent_args=(-p "$old_commit")
  fi
  new_commit=$(
    printf '%s\n' "$message" |
      GIT_AUTHOR_NAME="$author_name" GIT_AUTHOR_EMAIL="$author_email" \
      GIT_COMMITTER_NAME="$author_name" GIT_COMMITTER_EMAIL="$author_email" \
      git commit-tree "$snapshot_tree" "${parent_args[@]}"
  )

  if [[ -n "$old_commit" ]]; then
    git update-ref -m "contest source snapshot" \
      "$target_ref" "$new_commit" "$old_commit"
  else
    git update-ref -m "contest source snapshot" \
      "$target_ref" "$new_commit" ""
  fi
  echo "updated $branch: ${old_commit:-<new>} -> $new_commit"
fi

if ((push)); then
  push_refspec="$target_ref:$target_ref"
  if ((recreate)) && [[ -n "$old_commit" ]]; then
    remote_commit=$(git ls-remote --heads "$remote" "$target_ref" |
      awk 'NR == 1 { print $1 }')
    if [[ -n "$remote_commit" ]]; then
      git push --force-with-lease="$target_ref:$remote_commit" \
        "$remote" "$push_refspec"
    else
      git push "$remote" "$push_refspec"
    fi
  else
    git push "$remote" "$push_refspec"
  fi
fi

echo "snapshot ready: $new_commit"
