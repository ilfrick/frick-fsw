# Development-Loop Conformance Audit (norm N19)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-AUDIT-001                        |
| Revision                          | 0.3                                    |
| Date                              | 2026-10-11                             |
| Baseline commit                   | e1def89 (audited revision)             |
| Norms                             | N19 (this document), N4/N5/N8/N16      |

At the start of every development/verification loop (norm **N19**) this document
compares the **declared objectives** (milestones/OBJs) and **adherence to the
non-derogable rules N1-N20** against the **actual implementation**, and records
each gap with a disposition (closed in the loop, or tracked with owner and
milestone). No gap is left silent.

## 1. Objectives / milestones audit

Declared in `docs/development-plan.md` / `docs/roadmap.md`.

| Objective / milestone | Declared intent | Actual implementation | Gap | Disposition |
| --- | --- | --- | --- | --- |
| M1 baseline | cFS-style core (ES/SB/EVS/TBL/Time/PUS/Link), host sim | delivered | none | closed |
| M2 time base | PPS, CUC, time sources | delivered | none | closed |
| M3 HAL interfaces + ECSS docs | HAL contracts + SDD/CM/PA/… | delivered | none | closed |
| M4 MPFS low-level layers + 1 Gb/s Ethernet | drivers behind HAL (MMUART/CAN/SPI/I2C/QSPI/MMC/GPIO/…) + GEM | **GEM/TSN, GPIO, MMUART, MEM pool done; CAN/SPI/I2C/QSPI/MMC/eMMC/storage/PCIe/SpaceWire pending** | partial | tracked: M4/M11 (remaining classes) |
| M5 documentation/status program | living docs | in progress | partial | tracked: M5 |
| M6 libm | facade | delivered | none | closed |
| M7 SDLS + key mgmt | SDLS | delivered | none | closed |
| M8 gPTP + fabric CoreTSN | 802.1AS + multi-port/FRER | planned | none yet | tracked: M8 |
| M9 MISRA + coding style | style + MISRA process | declared; **certified checker absent** | GAP-A4 | tracked: GAP-A4 |
| M10 emulated target | Renode/QEMU | **Renode real run PASS; QEMU pending** | partial | tracked: M10 |
| M11 Renode HAL coverage | all HAL classes exercised | **Peripheral-bound: GPIO/SERIAL/MEM/ETH done; NET `FF_NetStack_t` contract check (loopback, not peripheral-bound) done; 9 classes pending** | partial | tracked: M11 |
| M12 OBCP engine (uPython) | n7s-obcp / ECSS engine | planned | none yet | tracked: M12 |
| M13 reference mission services | equipment manager + fault mgmt | planned | none yet | tracked: M13 |
| **M14 multicore/SMP** | SMP on the 4×U54 (after all others) | not started | none yet | tracked: M14/OBJ-22 (after all others) |
| **M15 in-house fundamental PUS-C set (+ CFDP + VFS)** | OBJ-23 (fundamental PUS-C behaviour + CFDP + virtual filestore + S23) | planned | none yet | tracked: M15 (after M14) |
| OBJ-05 pluggable TCP/IP+TSN | Tuxera/lwIP/libbsd/custom | lwIP integrated; Tuxera = user adapter (documented) | none | closed (documented) |
| **PUS-C ASN.1 (N7/ESA) as default** | integrate N7/ESA PUS-C library as the open/default PUS basis | **only fetched (`third_party/asn1-pusc-lib`); NOT wired into build/runtime; default stays the local CCSDS "open" backend** | **GAP (see §3)** | **reconcile this loop** (wire as default) |

## 2. Rules audit (N1-N20)

| Rule | Requirement | Actual | Gap | Disposition |
| --- | --- | --- | --- | --- |
| N1 | every generated artifact checked with Lean | pure-logic modules mirrored + 51 theorems; rest `lean_note` | full C refinement absent | GAP-A1 (tracked) |
| N2 | English only | compliant | none | closed |
| N3 | SRS/SSS for all code | compliant (94 reqs) | planned reqs documented-only | tracked: M12-M14 |
| N4 | bidirectional traceability converged | `trace.py` PASS | none | closed |
| N5 | class-B deliverable; class-A gaps documented | class B + GAP-A1..A14 | A1..A14 open | tracked: class-a-gaps |
| N6 | full SW V&V; CI/CD via Docker | CI + V&V image + Renode | RTEMS per-commit proxy; nightly real | documented |
| N7 | push to both remotes | compliant | none | closed |
| N8 | complete/current docs | compliant | some stale-word recurrences found by reviews | closed per review |
| N9 | independent reviews | R1/R2/R3/R4 + PA per change | none | closed |
| N10 | tools qualified | registry T1-T19 | new tools each change | closed |
| N11 | scripts/reviews/results committed | compliant | none | closed |
| N12 | NC close-out loop | compliant | none | closed |
| N13 | AGPL-3.0; licensing reviewed | compliant (+ Eärendil disclaimer) | none | closed |
| N14 | SSS reviewed by R3 | compliant | none | closed |
| N15 | full cycle each change | compliant (native+trace+format+Renode) | Lean/RTEMS proxies | documented |
| N16 | revision history | compliant | none | closed |
| N17 | documentation review (R4) | per change; one N20-change review was initially missed (FRICK-REVIEW-0056 DOC-10) then performed | none | closed |
| N18 | Renode functional validation | **in force** (`make renode` PASS) | per-commit SKIP proxy (no image) | documented |
| N19 | this audit each loop | **introduced now** | none | closed (this doc) |
| N20 | every phase product PA-reviewed; findings fixed and reviews repeated until pass | in force; FRICK-REVIEW-0052/0053 findings closed, PA PASS (FRICK-REVIEW-0063) | R1-02 bounded residual (tracked) | closed (residual tracked: loopback R1-02) |

## 3. The PUS-C ASN.1 gap: root cause and reconciliation

**Gap:** the project decision records repeatedly state intent to *integrate the
N7 Space / ESA `asn1-pusc-lib` (ASN.1/ACN PUS-C) as the open/default PUS basis*,
but only `tools/scripts/fetch_third_party.sh` (fetch, pinned) was ever added. The
library is **not referenced** by `src/`, `config/` or `CMakeLists.txt`, and the
default PUS backend is the project's own CCSDS `FF_PusBackendOpen`.

**Root cause (why a gap like this arises):**
1. The decision was recorded in prose (plan/ADR) and in a **fetch** script, but
   never converted into a **testable requirement + milestone deliverable + gate**:
   no SRS-REQ pointed at the integration, and the traceability gate only checks
   that listed implementation paths *exist* — it cannot detect an *absent*
   integration that was never required.
2. `asn1-pusc-lib` is **not a C runtime library**: it is an **ASN.1/ACN model**
   (312 `.asn1` + 311 `.acn`) plus small C wrappers that themselves depend on
   **ASN1SCC-generated** headers (`asn1crt.h`, `BasicTypes.h`). "Wiring it" is
   therefore a **toolchain integration** (ASN1SCC code generation) whose
   enabler was not made explicit; without a requirement/gate, the fetch was
   mistaken for the integration.
3. No **loop audit** existed to compare declared intent with implementation
   (which is exactly what norm **N19** now mandates).

**Reconciliation (this loop):** wire PUS-C ASN.1 (N7/ESA) into build and runtime
and make it the **default** PUS basis:
- Fetch/build **ASN1SCC** (ESA, GPL-with-runtime-exception; generated code and
  runtime are not GPL-restricted) and generate C from `asn1-pusc-lib`
  (`tools/scripts/gen_pus_asn1.sh`, tool T19).
- Add `FF_PusBackendN7` on the generated codecs + wrappers
  (packet error control/CRC, ISO checksum, packet length, UInt16 coder).
- Make it the **default** backend (`FF_PusBackendDefault`), keeping the previous
  open backend and PUSopen as alternatives.
- Requirement SRS-REQ-050; verify (host + Renode). Blocked on the ASN1SCC
  toolchain in the V&V image (see §4).

**Status update (validated this loop):** the ASN1SCC toolchain **builds**
(`tools/docker/asn1scc/Dockerfile`, pinned esa/asn1scc `3124590a…`; T19) and
**generation is proven** — `asn1scc -c -ACN -typePrefix Pusc_` over
`asn1-pusc-lib` produces `BasicTypes.h`, `asn1crt.c/h` and the module codecs.
Remaining: commit/consume the generated code in CMake, implement `FF_PusBackendN7`
and switch the default; then verify (host + Renode) and close SRS-REQ-050.

**Reconciled (build-time, ADR-0016):** `FF_PusBackendN7` is the default
(`FF_USE_PUS_ASN1=ON`, build-time generation via `make pus-asn1`; T19), verified on
host (14/14) and on the emulated SoC (`make renode` PASS). SRS-REQ-050 implemented
for the field/error-control layer; the deep per-service ASN.1 type codecs and the
fundamental service behaviour are **M15**. Falls back to the open backend with a
warning when the generated code is absent.

## 4. Other gaps of the same class (declared-but-not-delivered)

Found by this audit (not previously tracked as such):

1. **PUS-C ASN.1 default** — §3 (being reconciled this loop).
2. **Multicore/SMP** — never planned despite the 4×U54 target; now **M14/OBJ-22**
   (user directive; after all other milestones).
3. **QEMU** emulated target (M10) — script is a proxy; QEMU run pending.
4. **Certified MISRA checker** (M9/GAP-A4) — process only.
5. **Full PUS-C service coverage** — partial (GAP-A2).
6. **Full formal C refinement** — GAP-A1 (Lean model only).
These are now explicitly tracked (M10/M14/GAP-A1/A2/A4/M11/M13).

## 5. Dispositions summary

| Gap | Disposition | Owner | Milestone |
| --- | --- | --- | --- |
| PUS-C ASN.1 not wired/default | **reconciled**: `FF_PusBackendN7` default (build-time; ADR-0016); host `test_pus` + Renode | frick-fsw | M4/now |
| Multicore/SMP | tracked | frick-fsw | **M14** |
| In-house fundamental PUS-C service set (+ CFDP + virtual filestore) | tracked (assessment FRICK-ARCH-0015, ~34 PM; hybrid) | frick-fsw | **M15** |
| QEMU target | tracked | frick-fsw | M10 |
| Certified MISRA checker | tracked | frick-fsw | GAP-A4 |
| PUS-C service coverage | tracked | frick-fsw | GAP-A2 / M13 |
| Formal C refinement | tracked | frick-fsw | GAP-A1 / M8 |
| Loopback relay/`pending` orphan (FRICK-REVIEW-0032 R1-02; `backlog` ignored, second `connect` overwrites `pending`) | tracked (bounded 4-slot reference backend; `init()` resets) | frick-fsw | M4/M11 |
| PA-review dispositions (FRICK-REVIEW-0052/0053) | tracked | frick-fsw | now |

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-11 | frick-fsw | First N19 loop audit: objective/rule conformance; PUS-C ASN.1 gap root cause + reconciliation; multicore M14 |
| 0.2 | 2026-10-11 | frick-fsw | Re-baselined at e1def89: counts 94/54/26/14/51, registry T1-T19, N9 row + PA, N20 in progress, R1-02 residual tracked (PA-review loop) |
| 0.3 | 2026-10-11 | frick-fsw | N19 reconciliation: add the M15 objective row, refresh M14 wording, correct the M11 pending-class count (9) to match development-plan/roadmap |
