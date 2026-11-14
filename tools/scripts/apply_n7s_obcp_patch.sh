#!/usr/bin/env bash
# frick-fsw - Apply the project-owned N7 Space OBCP VM-budget patch set to the
# fetched pinned dependency.
#
# Applies, idempotently and fail-closed:
#   third_party/n7s-obcp/                     @ 55d6d97e7e7ff08a130698f7a922bcf58acb3d99
#   third_party/n7s-obcp/src/micropython/     @ 647c8b96cae7e202c7a020395b7cfe65e5b8ce04
#
# Patches and overlay sources live in tools/patches/n7s-obcp/ (tracked in the
# root Git tree). The fetched clone third_party/n7s-obcp/ is gitignored and is
# never the sole source of truth.
#
# Usage: tools/scripts/apply_n7s_obcp_patch.sh [--check]
#   (no args)  apply patches; exit 0 on success or "already applied"
#   --check    only verify that patches apply cleanly; do not modify
#
# SPDX-License-Identifier: MPL-2.0
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PATCH_DIR="${ROOT}/tools/patches/n7s-obcp"
CLONE="${N7_OBCP_CLONE:-${ROOT}/third_party/n7s-obcp}"

OBCP_COMMIT="55d6d97e7e7ff08a130698f7a922bcf58acb3d99"
MICROPYTHON_COMMIT="647c8b96cae7e202c7a020395b7cfe65e5b8ce04"

CHECK_ONLY=0
if [[ "${1:-}" == "--check" ]]; then
  CHECK_ONLY=1
fi

# Allow skipping pinned-commit verification when applying against a test
# snapshot (e.g. a pristine git-archive extraction) whose HEAD differs from
# the pinned commit but whose file tree is identical.
SKIP_COMMIT_CHECK=0
if [[ "${N7_OBCP_SKIP_COMMIT_CHECK:-0}" == "1" ]]; then
  SKIP_COMMIT_CHECK=1
fi

# ---------------------------------------------------------------------------
# Preconditions: pinned dependency must be fetched at the exact revisions.
# ---------------------------------------------------------------------------
if [[ ! -e "${CLONE}/.git" ]]; then
  echo "[apply-n7s-obcp-patch] error: n7s-obcp clone not found: ${CLONE}" >&2
  echo "         Run: bash tools/scripts/fetch_third_party.sh obcp-only" >&2
  exit 2
fi

if [[ "${SKIP_COMMIT_CHECK}" -eq 0 ]]; then
  actual_obcp="$(git -C "${CLONE}" rev-parse HEAD 2>/dev/null || true)"
  if [[ "${actual_obcp}" != "${OBCP_COMMIT}" ]]; then
    echo "[apply-n7s-obcp-patch] error: n7s-obcp revision mismatch" >&2
    echo "         expected: ${OBCP_COMMIT}" >&2
    echo "         found:    ${actual_obcp:-<none>}" >&2
    exit 2
  fi
fi

if [[ ! -e "${CLONE}/src/micropython/.git" ]]; then
  echo "[apply-n7s-obcp-patch] error: MicroPython submodule not found" >&2
  exit 2
fi

if [[ "${SKIP_COMMIT_CHECK}" -eq 0 ]]; then
  actual_mpy="$(git -C "${CLONE}/src/micropython" rev-parse HEAD 2>/dev/null || true)"
  if [[ "${actual_mpy}" != "${MICROPYTHON_COMMIT}" ]]; then
    echo "[apply-n7s-obcp-patch] error: MicroPython revision mismatch" >&2
    echo "         expected: ${MICROPYTHON_COMMIT}" >&2
    echo "         found:    ${actual_mpy:-<none>}" >&2
    exit 2
  fi
fi

# Working tree must be pristine before applying (no surprise overlays).
obcp_status="$(git -C "${CLONE}" status --porcelain --untracked-files=all --ignored=matching 2>/dev/null || true)"
if [[ -n "${obcp_status}" ]]; then
  echo "[apply-n7s-obcp-patch] warning: n7s-obcp checkout already has modifications:" >&2
  printf '%s\n' "${obcp_status}" >&2
fi

# ---------------------------------------------------------------------------
# Helper: check whether a patch's project-owned content is already in place.
# Used when git apply neither forwards nor reverses (already-patched checkout
# with unrelated local edits). Returns 0 if content looks already applied.
# ---------------------------------------------------------------------------
patch_already_in_place() {
  local label="$1"
  local patch_file="$2"
  local target_file=""

  case "${label}" in
    *obcpengine.c*)
      target_file="${CLONE}/src/obcp_engine/obcpengine.c"
      # Require ALL of: new signature, budget API set call, budget-exhausted
      # result enum. A partial edit that merely inserts one marker is not
      # sufficient to treat the patch as already applied.
      if grep -q "obcpengine_exec_result_t obcpengine_execute_py" "${target_file}" 2>/dev/null \
         && grep -q "mp_vm_set_instruction_budget" "${target_file}" 2>/dev/null \
         && grep -q "OBCP_ENGINE_EXEC_BUDGET_EXHAUSTED" "${target_file}" 2>/dev/null; then
        return 0
      fi
      ;;
    *obcpengine.h*)
      target_file="${CLONE}/src/obcp_engine/obcpengine.h"
      if grep -q "OBCP_ENGINE_EXEC_BUDGET_EXHAUSTED" "${target_file}" 2>/dev/null \
         && grep -q "obcpengine_exec_result_t" "${target_file}" 2>/dev/null; then
        return 0
      fi
      ;;
    *Makefile*)
      target_file="${CLONE}/Makefile"
      if grep -q "modifications/py/vm.c" "${target_file}" 2>/dev/null \
         && grep -q "cp.*vm.c" "${target_file}" 2>/dev/null; then
        return 0
      fi
      ;;
  esac
  return 1
}

# ---------------------------------------------------------------------------
# Helper: apply one unified diff patch idempotently.
# Returns 0 if applied or already applied; 2 on mismatch.
# ---------------------------------------------------------------------------
apply_patch() {
  local patch_file="$1"
  local label="$2"

  if [[ ! -f "${patch_file}" ]]; then
    echo "[apply-n7s-obcp-patch] error: patch not found: ${patch_file}" >&2
    exit 2
  fi

  if git -C "${CLONE}" apply --check "${patch_file}" >/dev/null 2>&1; then
    if [[ "${CHECK_ONLY}" -eq 1 ]]; then
      echo "[apply-n7s-obcp-patch] OK: ${label} applies cleanly"
      return 0
    fi
    if ! git -C "${CLONE}" apply "${patch_file}" >/dev/null 2>&1; then
      echo "[apply-n7s-obcp-patch] error: failed to apply ${label}" >&2
      exit 2
    fi
    echo "[apply-n7s-obcp-patch] applied: ${label}"
    return 0
  fi

  # Patch does not apply forwards. Check if it is already applied (reverse applies).
  if git -C "${CLONE}" apply --reverse --check "${patch_file}" >/dev/null 2>&1; then
    echo "[apply-n7s-obcp-patch] already applied: ${label}"
    return 0
  fi

  # Neither forward nor reverse applies. This can happen when the checkout has
  # been patched already plus unrelated local edits, or when a prior CMake
  # configure already ran the patch step. If the target file(s) already contain
  # the expected project-owned content, treat as already applied (idempotent).
  if patch_already_in_place "${label}" "${patch_file}"; then
    echo "[apply-n7s-obcp-patch] already in place (content-verified): ${label}"
    return 0
  fi

  echo "[apply-n7s-obcp-patch] error: patch mismatch for ${label}" >&2
  echo "         patch: ${patch_file}" >&2
  echo "         The checkout does not match the expected pre-patch state." >&2
  echo "         Aborting fail-closed; no partial changes are acceptable." >&2
  return 2
}

# ---------------------------------------------------------------------------
# Apply patches to tracked files in the clone.
# ---------------------------------------------------------------------------
apply_patch "${PATCH_DIR}/Makefile.patch"                "Makefile (clone target copies vm.c)"
apply_patch "${PATCH_DIR}/src/obcp_engine/obcpengine.h.patch"   "obcpengine.h (budget result enum + API)"
apply_patch "${PATCH_DIR}/src/obcp_engine/obcpengine.c.patch"   "obcpengine.c (budget-aware execute_py)"

# ---------------------------------------------------------------------------
# Overlay the new VM source file (project-owned modification).
# Idempotent: copy only if the clone copy differs from the tracked source.
# ---------------------------------------------------------------------------
TRACKED_VM="${PATCH_DIR}/src/modifications/py/vm.c"
CLONE_VM="${CLONE}/src/modifications/py/vm.c"

if [[ ! -f "${TRACKED_VM}" ]]; then
  echo "[apply-n7s-obcp-patch] error: tracked VM source not found: ${TRACKED_VM}" >&2
  exit 2
fi

if [[ -f "${CLONE_VM}" ]]; then
  if cmp -s "${TRACKED_VM}" "${CLONE_VM}"; then
    echo "[apply-n7s-obcp-patch] already in place: src/modifications/py/vm.c"
  else
    echo "[apply-n7s-obcp-patch] warning: clone vm.c differs from tracked source; overwriting with tracked version" >&2
    if [[ "${CHECK_ONLY}" -eq 1 ]]; then
      echo "[apply-n7s-obcp-patch] error: vm.c mismatch in --check mode" >&2
      exit 2
    fi
    cp "${TRACKED_VM}" "${CLONE_VM}"
    echo "[apply-n7s-obcp-patch] overlaid: src/modifications/py/vm.c"
  fi
else
  if [[ "${CHECK_ONLY}" -eq 1 ]]; then
    echo "[apply-n7s-obcp-patch] OK: src/modifications/py/vm.c would be created"
  else
    cp "${TRACKED_VM}" "${CLONE_VM}"
    echo "[apply-n7s-obcp-patch] created: src/modifications/py/vm.c"
  fi
fi

echo "[apply-n7s-obcp-patch] done: patch set applied to pinned n7s-obcp @ ${OBCP_COMMIT}"
