#!/usr/bin/env bash
# frick-fsw - MISRA C:2012 scan (open-source cppcheck MISRA addon).
#
# Advisory scanner used in CI (norm N10): writes build/misra-report.txt and
# prints a summary. Not a qualified MISRA checker; see docs/quality/misra-c.md.
# Exit code: 0 on success (findings are advisory and written to the report),
# 2 if the cppcheck checker is absent (fail-closed: evidence cannot be produced).
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: AGPL-3.0-only
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUT="${ROOT}/build/misra-report.txt"
mkdir -p "${ROOT}/build"

if ! command -v cppcheck > /dev/null 2>&1; then
  echo "[misra] cppcheck not installed; install it (apt-get install cppcheck) to run the MISRA scan" >&2
  echo "[misra] FAIL: static-analysis evidence cannot be produced without the checker" >&2
  exit 2
fi

cppcheck --addon=misra --std=c99 --inline-suppr \
  --suppress=missingIncludeSystem --suppress=unusedFunction \
  --quiet --template='{file}:{line}: {id}: {message}' \
  -I"${ROOT}/src" -I"${ROOT}/src/core" -I"${ROOT}/config" \
  "${ROOT}/src" "${ROOT}/config" "${ROOT}/tests" 2> "${OUT}"

n=$(grep -c 'misra' "${OUT}" || true)
echo "[misra] findings: ${n} (report: ${OUT})"
echo "[misra] advisory: see docs/quality/misra-c.md (certified checker required for compliance)"
exit 0
