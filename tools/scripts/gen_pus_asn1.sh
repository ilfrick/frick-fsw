#!/usr/bin/env bash
# frick-fsw - generate the ASN1SCC common runtime/types required by the N7 PUS adapter.
#
# The current adapter consumes the N7-generated BasicTypes widths plus the
# ASN1SCC runtime; service-specific generated codecs remain outside the adapter
# integration scope (M15). Outputs are ignored build products, never committed.
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
LIB="${ROOT}/third_party/asn1-pusc-lib"
OUT="${ROOT}/third_party/asn1-pusc-lib-generated"
IMG="${ASN1SCC_IMAGE:-frick-asn1scc:local}"

if [[ ! -d "${LIB}/common" ]]; then
  echo "[pus-asn1] fetching pinned N7/ESA source"
  bash "${ROOT}/tools/scripts/fetch_third_party.sh" asn1-only
fi
if ! docker image inspect "${IMG}" >/dev/null 2>&1; then
  echo "[pus-asn1] building ASN1SCC image ${IMG}"
  docker build -t "${IMG}" "${ROOT}/tools/docker/asn1scc"
fi

TMP="$(mktemp -d "${ROOT}/third_party/asn1-pusc-lib-generated.tmp.XXXXXX")"
cleanup() { rm -rf "${TMP}"; }
trap cleanup EXIT
mkdir -p "${TMP}/common"

# Expand input globs inside the container. Host-side expansion against the
# container-only /pusclib mount silently passes literal wildcard paths.
docker run --rm --user "$(id -u):$(id -g)" \
  -v "${LIB}:/pusclib:ro" -v "${TMP}:/out" "${IMG}" \
  bash -lc 'shopt -s nullglob; files=(/pusclib/common/*.asn1 /pusclib/common/*.acn); \
    if ((${#files[@]} == 0)); then echo "no common ASN.1/ACN inputs" >&2; exit 2; fi; \
    exec asn1scc -c -ACN -typePrefix Pusc_ -o /out/common "${files[@]}"'

for required in BasicTypes.h BasicTypes.c asn1crt.h asn1crt.c asn1crt_encoding.c asn1crt_encoding_acn.c; do
  if [[ ! -s "${TMP}/common/${required}" ]]; then
    echo "[pus-asn1] required generated file missing: ${required}" >&2
    exit 1
  fi
done

rm -rf "${OUT}"
mv "${TMP}" "${OUT}"
trap - EXIT
echo "[pus-asn1] generated common ASN1SCC runtime/types into ${OUT}/common"
