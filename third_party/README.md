# Third-party components

This directory holds **fetched, pinned** third-party sources used behind the
project's abstraction contracts. They are **not committed** (see `.gitignore`);
run `tools/scripts/fetch_third_party.sh` to populate them. Pinned versions and
licences are recorded here and in `docs/quality/licensing.md` (norm N13).

| Component | Purpose | Version / commit | Licence |
| --------- | ------- | ---------------- | ------- |
| [lwIP](https://github.com/lwip-tcpip/lwip) | reference TCP/IP stack behind `FF_NetStack_t` | `STABLE-2_2_1_RELEASE`, commit `77dcd25a72509eb83f72b033d219b1d40cd8eb95` | BSD-3-Clause |
| [n7space/asn1-pusc-lib](https://github.com/n7space/asn1-pusc-lib) | ESA/N7 Space PUS-C ASN.1/ACN types + CCSDS utilities behind `FF_Pus_Backend_t` | `2126df11cfe664b9f6f9d7d7a49312bb70768e75` | GPL-3.0 |
| PUSopen(R) | commercial PUS adapter (user-supplied; never fetched) | n/a | commercial |

## Fetch

```bash
tools/scripts/fetch_third_party.sh [mode]   # all | lwip-only | asn1-only
```

## Build switches

- `FF_USE_LWIP=ON`  → build the lwIP `FF_NetStack_t` adapter.
- `FF_USE_PUS_ASN1=ON` (opt-in; default OFF) → build the optional N7/ESA PUS-C
  ASN.1 backend (build-time ASN1SCC generation, `make pus-asn1`).
  The open CCSDS backend is the default.

`FF_USE_LWIP` defaults ON and requires the pinned lwIP tree to be fetched before
CMake configure. The fetch script verifies exact commits and rejects modified
existing checkouts. `FF_USE_PUS_ASN1` defaults OFF; explicitly enabling it
requires generated sources, and CMake fails rather than silently changing the
requested backend if they are absent.
