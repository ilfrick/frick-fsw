# Third-party components

This directory holds **fetched, pinned** third-party sources used behind the
project's abstraction contracts. They are **not committed** (see `.gitignore`);
run `tools/scripts/fetch_third_party.sh` to populate them. Pinned versions and
licences are recorded here and in `docs/quality/licensing.md` (norm N13).

| Component | Purpose | Version / commit | Licence |
| --------- | ------- | ---------------- | ------- |
| [lwIP](https://github.com/lwip-tcpip/lwip) | reference TCP/IP stack behind `FF_NetStack_t` | `STABLE-2_2_1_RELEASE` | BSD-3-Clause |
| [n7space/asn1-pusc-lib](https://github.com/n7space/asn1-pusc-lib) | ESA/N7 Space PUS-C ASN.1/ACN types + CCSDS utilities behind `FF_Pus_Backend_t` | `2126df11cfe664b9f6f9d7d7a49312bb70768e75` | GPL-3.0 |
| PUSopen(R) | commercial PUS adapter (user-supplied; never fetched) | n/a | commercial |

## Fetch

```bash
tools/scripts/fetch_third_party.sh          # into third_party/{lwip,asn1-pusc-lib}
```

## Build switches

- `FF_USE_LWIP=ON`  → build the lwIP `FF_NetStack_t` adapter.
- `FF_USE_PUS_ASN1=ON` (default) → build the N7/ESA PUS-C ASN.1
  `FF_Pus_Backend_t` default backend (build-time ASN1SCC generation, `make pus-asn1`).
  With `-DFF_USE_PUS_ASN1=OFF` the open CCSDS backend is used.

`FF_USE_LWIP` defaults OFF; `FF_USE_PUS_ASN1` defaults ON and falls back to the
open backend (with a warning) when the generated code is absent, so the
middleware builds and its tests pass either way.
