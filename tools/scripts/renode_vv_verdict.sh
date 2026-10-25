#!/bin/sh
# frick-fsw - N18 Renode run verdict (norm N18), factored for testability.
#
# Reads ONLY the artifacts of ONE run directory and decides the gate verdict:
#
#   0  PASS  - the "RENODE_TEST: PASS" marker is present in this run's capture
#   1  FAIL  - the marker is absent, or the run's own capture is unusable
#   2  ERROR - usage problem (no run directory given)
#
# The run directory is expected to contain, all created by the run itself:
#   renode-process.log  the Renode/docker process log of THIS run
#   uart-console.log    the UART console capture of THIS run
#   status.env          optional key=value status of THIS run
#                       (CONTAINER_STATUS, CONTAINER_EXIT, WAIT_RESULT)
#
# Design rules (NC-EXT-199):
#   * only files inside the given run directory are ever read, so a stale or
#     unwritable fixed log path can never influence the verdict;
#   * a missing, unreadable or empty capture fails closed (FAIL), it is never
#     treated as "no marker found" and never as a skip;
#   * the absence of the marker is distinguished between a target startup fatal
#     exit and a bounded-window timeout, because they have different causes.
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0

set -u

MARKER="RENODE_TEST: PASS"

usage() {
    echo "usage: renode_vv_verdict.sh <run-dir>" >&2
    exit 2
}

[ "$#" -eq 1 ] || usage
RUN_DIR="$1"
[ -d "${RUN_DIR}" ] || usage

PROCESS_LOG="${RUN_DIR}/renode-process.log"
CONSOLE_LOG="${RUN_DIR}/uart-console.log"
STATUS_FILE="${RUN_DIR}/status.env"

# ---- capture integrity: fail closed -------------------------------------
for f in "${PROCESS_LOG}" "${CONSOLE_LOG}"; do
    if [ ! -f "${f}" ]; then
        echo "[renode] FAIL: run capture incomplete (missing ${f}) - fail closed"
        exit 1
    fi
    if [ ! -r "${f}" ]; then
        echo "[renode] FAIL: run capture unreadable (${f}) - fail closed"
        exit 1
    fi
    if [ ! -s "${f}" ]; then
        echo "[renode] FAIL: run capture empty (${f}) - fail closed"
        exit 1
    fi
done

CONTAINER_STATUS="unknown"
CONTAINER_EXIT="unknown"
WAIT_RESULT="unknown"
if [ -r "${STATUS_FILE}" ]; then
    # The status file is parsed with a whitelist: it is never sourced, so a
    # crafted value cannot introduce command execution here.
    _v="$(grep -E '^CONTAINER_STATUS=' "${STATUS_FILE}" 2>/dev/null | tail -n 1 | cut -d= -f2-)"
    case "${_v}" in
        ""|*[!A-Za-z0-9_.-]*) : ;;
        *) CONTAINER_STATUS="${_v}" ;;
    esac
    _v="$(grep -E '^CONTAINER_EXIT=' "${STATUS_FILE}" 2>/dev/null | tail -n 1 | cut -d= -f2-)"
    case "${_v}" in
        ""|*[!A-Za-z0-9_.-]*) : ;;
        *) CONTAINER_EXIT="${_v}" ;;
    esac
    _v="$(grep -E '^WAIT_RESULT=' "${STATUS_FILE}" 2>/dev/null | tail -n 1 | cut -d= -f2-)"
    case "${_v}" in
        ""|*[!A-Za-z0-9_.-]*) : ;;
        *) WAIT_RESULT="${_v}" ;;
    esac
fi

# ---- marker: this run's captures only -----------------------------------
if grep -aq "${MARKER}" "${PROCESS_LOG}" "${CONSOLE_LOG}"; then
    echo "[renode] PASS (${MARKER}; container status=${CONTAINER_STATUS} container exit code=${CONTAINER_EXIT})"
    exit 0
fi

# ---- no marker: classify the cause --------------------------------------
fatal_hit=""
for pat in "EXIT STATUS NOT ZERO" "exit source:" "exit code:" "[FF-PANIC]" "rtems_fatal" "Fatal error"; do
    if grep -aqF -- "${pat}" "${PROCESS_LOG}" "${CONSOLE_LOG}"; then
        fatal_hit="${pat}"
        break
    fi
done

if [ -n "${fatal_hit}" ]; then
    echo "[renode] FAIL: marker absent and the target reported a startup fatal exit ('${fatal_hit}'); container status=${CONTAINER_STATUS} container exit code=${CONTAINER_EXIT}"
    exit 1
fi

if [ "${WAIT_RESULT}" = "running" ] || [ "${CONTAINER_STATUS}" = "running" ]; then
    echo "[renode] FAIL: marker absent and the container was still running when the bounded window ended; container status=${CONTAINER_STATUS} container exit code=${CONTAINER_EXIT}"
    exit 1
fi

echo "[renode] FAIL: marker absent; container status=${CONTAINER_STATUS} container exit code=${CONTAINER_EXIT}"
exit 1
