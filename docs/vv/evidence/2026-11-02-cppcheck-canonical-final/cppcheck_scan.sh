#!/usr/bin/env bash
# frick-fsw - reproducible cppcheck inventory for GAP-A4 (advisory static analysis).
#
# Produces the canonical GAP-A4 finding inventory. The invocation is pinned here
# (checker image, option set, template, include roots, path set) so that a scan
# can be re-performed by anyone holding the pinned image, and so that two runs
# are byte-comparable. It is NOT a MISRA compliance scan: use `make misra`
# (tools/scripts/misra.sh, --addon=misra) for the advisory MISRA subset.
#
# Exit codes (fail-closed, never a silent skip):
#   0  scan performed; findings (if any) are in the report file
#   2  the checker cannot be produced (no docker CLI, or the pinned image is
#      absent) - no report is written and no "clean" claim may be made
#   3  usage error
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
IMG="${CPPCHECK_IMAGE:-frick-vv:local}"
OUT=""
INCLUDE_TESTS=0

usage() {
  cat >&2 <<'EOF'
usage: cppcheck_scan.sh [--out FILE] [--include-tests]

  --out FILE       report path (default: <repo>/build/cppcheck-report.txt)
  --include-tests  also scan tests/ (default: src + config only, which is the
                   path set of the 2026-11-01 GAP-A4 baseline inventory)
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --out)
      [[ $# -ge 2 ]] || { usage; exit 3; }
      OUT="$2"
      shift 2
      ;;
    --include-tests)
      INCLUDE_TESTS=1
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      usage
      exit 3
      ;;
  esac
done

OUT="${OUT:-${ROOT}/build/cppcheck-report.txt}"
mkdir -p "$(dirname "${OUT}")"

if ! command -v docker >/dev/null 2>&1; then
  echo "[cppcheck] FAIL: no docker CLI; the pinned checker cannot be produced (GAP-A4)" >&2
  exit 2
fi
if ! docker image inspect "${IMG}" >/dev/null 2>&1; then
  echo "[cppcheck] FAIL: image ${IMG} absent; build it with 'make docker-image' (GAP-A4)" >&2
  exit 2
fi

# The image mount is /workspace, so every path handed to cppcheck is absolute
# inside the container. Keeping them absolute makes the report paths stable and
# independent of the host checkout location.
PATHS=(/workspace/src /workspace/config)
if [[ "${INCLUDE_TESTS}" -eq 1 ]]; then
  PATHS+=(/workspace/tests)
fi

TEMPLATE='{file}:{line}:{column}: {severity}: {message} [{id}]'

CHECKER_VERSION="$(docker run --rm --entrypoint cppcheck "${IMG}" --version 2>/dev/null | head -1)"
if [[ -z "${CHECKER_VERSION}" ]]; then
  echo "[cppcheck] FAIL: the checker did not report a version (GAP-A4)" >&2
  exit 2
fi

echo "[cppcheck] image:   ${IMG}"
echo "[cppcheck] checker: ${CHECKER_VERSION}"
echo "[cppcheck] paths:   ${PATHS[*]}"

docker run --rm -v "${ROOT}":/workspace -w /workspace --entrypoint cppcheck "${IMG}" \
  --enable=style,unusedFunction \
  --std=c99 \
  --inline-suppr \
  --suppress=missingIncludeSystem \
  --quiet \
  --template="${TEMPLATE}" \
  -I/workspace/src -I/workspace/src/core -I/workspace/config \
  "${PATHS[@]}" > "${OUT}" 2>&1
rc=$?

if [[ "${rc}" -ne 0 ]]; then
  echo "[cppcheck] FAIL: the checker exited ${rc}; report is not evidence (GAP-A4)" >&2
  exit 2
fi

n="$(grep -cE '\[[a-zA-Z0-9_]+\]$' "${OUT}" || true)"
echo "[cppcheck] findings: ${n} (report: ${OUT})"
echo "[cppcheck] advisory: a qualified/certified checker remains GAP-A4"
exit 0
