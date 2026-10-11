#!/usr/bin/env bash
# frick-fsw - generate C from the N7/ESA asn1-pusc-lib ASN.1/ACN PUS-C model.
#
# Reproducible PUS-C ASN.1 code generation (norm N19 reconciliation of the
# PUS-C ASN.1 default): builds the pinned ASN1SCC compiler image (once), fetches
# the pinned asn1-pusc-lib (once), and generates the C for the common, CCSDS and
# PUS service modules into third_party/asn1-pusc-lib-generated/ (git-ignored).
#
# Output is consumed by the PUS-C ASN.1 default backend (FF_PusBackendN7).
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: AGPL-3.0-only
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
LIB="${ROOT}/third_party/asn1-pusc-lib"
OUT="${ROOT}/third_party/asn1-pusc-lib-generated"
IMG="${ASN1SCC_IMAGE:-frick-asn1scc:local}"

if [[ ! -d "${LIB}" ]]; then
  echo "[pus-asn1] fetching third-party tree"
  bash "${ROOT}/tools/scripts/fetch_third_party.sh"
fi
if ! docker image inspect "${IMG}" > /dev/null 2>&1; then
  echo "[pus-asn1] building ASN1SCC image ${IMG} (long)"
  docker build -t "${IMG}" "${ROOT}/tools/docker/asn1scc" || exit 1
fi

mkdir -p "${OUT}"
rc=0
gen() { # gen <subdir>
  local d="$1"
  echo "[pus-asn1] $d"
  docker run --rm -v "${LIB}:/pusclib:ro" -v "${OUT}:/out" "${IMG}" \
    asn1scc -c -ACN -typePrefix Pusc_ -o "/out/${d}" \
      "/pusclib/${d}"/*.asn1 "/pusclib/${d}"/*.acn || rc=1
}
gen common
gen ccsds
for d in "${LIB}"/service-*; do
  [[ -d "${d}" ]] && gen "$(basename "${d}")"
done

echo "[pus-asn1] generated into ${OUT} (rc=${rc})"
ls "${OUT}" | head
exit "${rc}"
