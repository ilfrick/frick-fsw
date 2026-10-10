#!/usr/bin/env bash
# frick-fsw - fetch pinned third-party components (not committed).
#
# Populates third_party/{lwip,asn1-pusc-lib} at the pinned versions recorded in
# third_party/README.md and docs/quality/licensing.md (norm N13/N8).
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: AGPL-3.0-only
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
DEST="${ROOT}/third_party"
mkdir -p "${DEST}"

# lwIP (BSD-3-Clause), pinned tag. Reference TCP/IP stack (FF_NetStack_t).
if [ ! -d "${DEST}/lwip/.git" ]; then
  echo "[fetch] lwIP STABLE-2_2_1_RELEASE"
  git clone --depth 1 --branch STABLE-2_2_1_RELEASE \
    https://github.com/lwip-tcpip/lwip.git "${DEST}/lwip"
fi

# N7 Space / ESA asn1-pusc-lib (GPL-3.0), pinned commit. PUS-C types + CCSDS.
if [ ! -d "${DEST}/asn1-pusc-lib/.git" ]; then
  echo "[fetch] n7space/asn1-pusc-lib @2126df1"
  git clone https://github.com/n7space/asn1-pusc-lib.git "${DEST}/asn1-pusc-lib"
  git -C "${DEST}/asn1-pusc-lib" checkout --quiet \
    2126df11cfe664b9f6f9d7d7a49312bb70768e75
fi

echo "[fetch] done: ${DEST}"
