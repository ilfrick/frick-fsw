#!/usr/bin/env bash
# frick-fsw - fetch pinned third-party components (not committed).
#
# Populates third_party/{lwip,asn1-pusc-lib} at the pinned versions recorded in
# third_party/README.md and docs/quality/licensing.md (norm N13/N8).
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
DEST="${ROOT}/third_party"
mkdir -p "${DEST}"
MODE="${1:-all}"
if [[ "${MODE}" != "all" && "${MODE}" != "lwip-only" && "${MODE}" != "asn1-only" ]]; then
  echo "usage: $0 [all|lwip-only|asn1-only]" >&2
  exit 2
fi

# lwIP (BSD-3-Clause), pinned by both release tag and immutable commit.
LWIP_COMMIT="77dcd25a72509eb83f72b033d219b1d40cd8eb95"
ASN1_COMMIT="2126df11cfe664b9f6f9d7d7a49312bb70768e75"

verify_checkout() {
  local repo="$1" expected="$2" label="$3" actual status
  if [[ ! -e "${repo}/.git" ]]; then
    echo "[fetch] error: ${label} repository metadata missing: ${repo}" >&2
    return 1
  fi
  actual="$(git -C "${repo}" rev-parse HEAD)"
  if [[ "${actual}" != "${expected}" ]]; then
    echo "[fetch] error: ${label} revision mismatch: expected ${expected}, found ${actual}" >&2
    return 1
  fi
  # Treat ignored content as modified too: an ignored object under a reused
  # dependency checkout could otherwise shadow or influence the pinned source.
  # Generated outputs belong outside these immutable source repositories.
  status="$(git -C "${repo}" status --porcelain --untracked-files=all --ignored=matching)"
  if [[ -n "${status}" ]]; then
    echo "[fetch] error: ${label} checkout is modified: ${repo}" >&2
    printf '%s\n' "${status}" >&2
    return 1
  fi
}

# Reference TCP/IP stack (FF_NetStack_t). Reused checkouts fail closed if their
# commit or working-tree state differs from the controlled dependency identity.
if [[ "${MODE}" != "asn1-only" ]] && [ ! -e "${DEST}/lwip/.git" ]; then
  echo "[fetch] lwIP STABLE-2_2_1_RELEASE"
  git clone --depth 1 --branch STABLE-2_2_1_RELEASE \
    https://github.com/lwip-tcpip/lwip.git "${DEST}/lwip"
fi
if [[ "${MODE}" != "asn1-only" ]]; then
  verify_checkout "${DEST}/lwip" "${LWIP_COMMIT}" "lwIP"
fi

# N7 Space / ESA asn1-pusc-lib (GPL-3.0), pinned commit. PUS-C types + CCSDS.
if [[ "${MODE}" != "lwip-only" ]] && [ ! -e "${DEST}/asn1-pusc-lib/.git" ]; then
  echo "[fetch] n7space/asn1-pusc-lib @2126df1"
  git clone https://github.com/n7space/asn1-pusc-lib.git "${DEST}/asn1-pusc-lib"
  git -C "${DEST}/asn1-pusc-lib" checkout --quiet "${ASN1_COMMIT}"
fi
if [[ "${MODE}" != "lwip-only" ]]; then
  verify_checkout "${DEST}/asn1-pusc-lib" "${ASN1_COMMIT}" "asn1-pusc-lib"
fi

echo "[fetch] done (${MODE}): ${DEST}"
