#!/bin/sh
# frick-fsw - regression test for the N18 Renode runner verdict logic
# (norm N11/N22, NC-EXT-199).
#
# The runner must never be able to report PASS from a stale or unwritable
# log path. The cases below drive the verdict unit directly with the GENUINE
# recorded outputs of real runs (no forged product output):
#
#   A  stale PASS marker present, fresh run without the marker  -> FAIL
#   B  missing / unreadable / empty capture                    -> FAIL (fail closed)
#   C  genuine recorded successful run (2026-10-20)            -> PASS
#   D  genuine recorded current-tree run, no marker, startup
#      fatal exit                                               -> FAIL, fatal reason
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0

set -u

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
VERDICT="${ROOT}/tools/scripts/renode_vv_verdict.sh"
EV="${ROOT}/docs/vv/evidence"

WORK="$(mktemp -d "${TMPDIR:-/tmp}/frick-renode-verdict-test.XXXXXX")"
trap 'rm -rf "${WORK}"' EXIT

fails=0
checks=0

report() {
    checks=$((checks + 1))
    if [ "$1" = "0" ]; then
        echo "ok   - $2"
    else
        echo "FAIL - $2"
        fails=$((fails + 1))
    fi
}

# The fixtures are genuine recorded runs retained as N11 evidence. If one is
# missing the case must fail closed with a clear diagnostic, not with raw
# "cp: cannot stat" noise that would hide the real cause.
require_fixture() {
    if [ ! -r "$1" ]; then
        echo "FAIL - required evidence fixture is missing: $1"
        echo "renode_vv_verdict: checks=${checks} failures=$((fails + 1))"
        exit 1
    fi
}
for fx in \
    "${EV}/2026-10-20-n18-process.log" \
    "${EV}/2026-10-20-n18-console.log" \
    "${EV}/2026-10-21-s8-fdir-n18-current-process.log" \
    "${EV}/2026-10-21-s8-fdir-n18-current-uart-console.log"; do
    require_fixture "${fx}"
done

run_verdict() {
    out="$(sh "${VERDICT}" "$1" 2>&1)"
    rc=$?
    echo "${out}"
    return "${rc}"
}

# ---------------------------------------------------------------- A
# A stale PASS marker must not be able to make a marker-less run pass.
# The stale file uses the historical fixed log name and is placed where a
# TMPDIR-based path would resolve, so the case fails if the verdict ever
# consults anything outside the run directory it was given.
A="${WORK}/caseA"
LEGACY="${WORK}/legacy"
mkdir -p "${A}" "${LEGACY}"
# The stale artefact is a COPY of the genuine recorded successful run: the
# historical mechanism is exactly "an earlier run's log file is still there".
cp "${EV}/2026-10-20-n18-process.log" "${LEGACY}/frick-renode.log"
cat > "${A}/renode-process.log" <<'EOF'
[INFO] System bus created.
[INFO] sysbus: Loading block of 517520 bytes length at 0x1000000000.
EOF
cat > "${A}/uart-console.log" <<'EOF'
frick-fsw: 8 applications registered
[FF-PANIC] watchdog: init failed
*** EXIT STATUS NOT ZERO ***
EOF
out="$(TMPDIR="${LEGACY}" run_verdict "${A}")"; rc=$?
report "$([ "${rc}" -ne 0 ] && echo 0 || echo 1)" "A1 stale PASS marker does not turn a marker-less run into PASS (rc=${rc})"
case "${out}" in
  *"marker absent"*) report 0 "A2 the FAIL reason names the absent marker" ;;
  *) report 1 "A2 the FAIL reason names the absent marker" ;;
esac
report "$([ -s "${LEGACY}/frick-renode.log" ] && echo 0 || echo 1)" "A3 the stale marker file was still present during the case"

# ---------------------------------------------------------------- B
# B1 missing capture
B1="${WORK}/caseB1"
mkdir -p "${B1}"
cat > "${B1}/uart-console.log" <<'EOF'
some console text
EOF
out="$(run_verdict "${B1}")"; rc=$?
report "$([ "${rc}" -ne 0 ] && echo 0 || echo 1)" "B1 missing process log fails closed (rc=${rc})"
case "${out}" in
  *"run capture incomplete"*) report 0 "B1b the reason names the incomplete capture" ;;
  *) report 1 "B1b the reason names the incomplete capture" ;;
esac

# B2 unreadable capture
B2="${WORK}/caseB2"
mkdir -p "${B2}"
cp "${EV}/2026-10-21-s8-fdir-n18-current-process.log" "${B2}/renode-process.log"
cp "${EV}/2026-10-21-s8-fdir-n18-current-uart-console.log" "${B2}/uart-console.log"
chmod 000 "${B2}/uart-console.log"
out="$(run_verdict "${B2}")"; rc=$?
chmod 644 "${B2}/uart-console.log"
report "$([ "${rc}" -ne 0 ] && echo 0 || echo 1)" "B2 unreadable capture fails closed (rc=${rc})"
case "${out}" in
  *"run capture unreadable"*) report 0 "B2b the reason names the unreadable capture" ;;
  *) report 1 "B2b the reason names the unreadable capture" ;;
esac

# B3 empty capture
B3="${WORK}/caseB3"
mkdir -p "${B3}"
: > "${B3}/renode-process.log"
cp "${EV}/2026-10-21-s8-fdir-n18-current-uart-console.log" "${B3}/uart-console.log"
out="$(run_verdict "${B3}")"; rc=$?
report "$([ "${rc}" -ne 0 ] && echo 0 || echo 1)" "B3 empty capture fails closed (rc=${rc})"
case "${out}" in
  *"run capture empty"*) report 0 "B3b the reason names the empty capture" ;;
  *) report 1 "B3b the reason names the empty capture" ;;
esac

# ---------------------------------------------------------------- C
# C genuine recorded successful run (2026-10-20) must PASS
C="${WORK}/caseC"
mkdir -p "${C}"
cp "${EV}/2026-10-20-n18-process.log" "${C}/renode-process.log"
cp "${EV}/2026-10-20-n18-console.log" "${C}/uart-console.log"
out="$(run_verdict "${C}")"; rc=$?
report "$([ "${rc}" -eq 0 ] && echo 0 || echo 1)" "C1 genuine recorded successful run passes (rc=${rc})"
case "${out}" in
  *"RENODE_TEST: PASS"*) report 0 "C2 the PASS line carries the marker" ;;
  *) report 1 "C2 the PASS line carries the marker" ;;
esac

# ---------------------------------------------------------------- D
# D genuine recorded current-tree run (no marker, startup fatal exit)
D="${WORK}/caseD"
mkdir -p "${D}"
cp "${EV}/2026-10-21-s8-fdir-n18-current-process.log" "${D}/renode-process.log"
cp "${EV}/2026-10-21-s8-fdir-n18-current-uart-console.log" "${D}/uart-console.log"
out="$(run_verdict "${D}")"; rc=$?
report "$([ "${rc}" -ne 0 ] && echo 0 || echo 1)" "D1 current-tree run without the marker fails (rc=${rc})"
case "${out}" in
  *"startup fatal exit"*) report 0 "D2 the FAIL reason distinguishes the startup fatal exit" ;;
  *) report 1 "D2 the FAIL reason distinguishes the startup fatal exit" ;;
esac

# ---------------------------------------------------------------- E
# E usage: no run directory
sh "${VERDICT}" > /dev/null 2>&1; rc=$?
report "$([ "${rc}" -eq 2 ] && echo 0 || echo 1)" "E1 missing run directory is a usage error (rc=${rc})"

# ---------------------------------------------------------------- F
# F static guard on the runner: it must not read a fixed log path again, and
# it must create its per-run directory with mktemp -d. This pins the exact
# NC-EXT-199 mechanism at the runner level.
RUNNER="${ROOT}/tools/scripts/renode_vv.sh"
# Comment lines are stripped first: the runner may *document* the historical
# paths, but no executable statement may reference them.
if grep -vE '^[[:space:]]*#' "${RUNNER}" | grep -qE '/tmp/frick-renode\.log|/tmp/frick-console\.log'; then
    report 1 "F1 the runner does not reference the historical fixed log paths"
else
    report 0 "F1 the runner does not reference the historical fixed log paths"
fi
if grep -q 'mktemp -d' "${RUNNER}"; then
    report 0 "F2 the runner creates a fresh per-run directory with mktemp -d"
else
    report 1 "F2 the runner creates a fresh per-run directory with mktemp -d"
fi
if grep -q 'renode_vv_verdict.sh' "${RUNNER}"; then
    report 0 "F3 the runner delegates the verdict to the fail-closed unit"
else
    report 1 "F3 the runner delegates the verdict to the fail-closed unit"
fi

echo "renode_vv_verdict: checks=${checks} failures=${fails}"
[ "${fails}" -eq 0 ] || exit 1
exit 0
