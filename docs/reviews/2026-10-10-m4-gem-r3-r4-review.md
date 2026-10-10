# Review Report — Milestone M4 increment-1 (Cadence GEM Ethernet driver + third-party scaffolding)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0026                      |
| Reviewer session                  | (independent agent, norm N9)           |
| Reviewer profile                  | R3 (Senior Systems Engineer, N14) **and** R4 (Senior SW documentation reviewer, N17) |
| Reference revision                | branch `main`, uncommitted working tree on base commit `b4a1e5f` (M4 increment-1 delta) |
| Items under review                | `docs/requirements/SSS.md` (SSS-REQ-020), `docs/requirements/SRS.md` (SRS-REQ-036), `docs/requirements/traceability.{yaml,md}` (counts 37/20/14); `docs/adr/ADR-0014-drivers-and-third-party.md`; `third_party/README.md`; `tools/scripts/fetch_third_party.sh`; `.gitignore`; `docs/quality/licensing.md` (lwIP BSD-3, n7space GPL-3.0 rows); `docs/architecture/hardware-abstraction.md` + `docs/development-plan.md` (TSN correction); `README.md`; `CHANGELOG.md`; `docs/status/status.json`; `docs/roadmap.md`; `docs/vv/verification-results.md`; the delivered driver `src/drivers/eth/gem/ff_gem.{c,h}`, `src/drivers/eth/gem/ff_gem_regs.h`, `src/hal/hal_eth.h`, `tests/unit/test_gem.c`, `CMakeLists.txt`/`tests/CMakeLists.txt`; both CI workflows (unchanged) |
| Date                              | 2026-10-10                             |
| Scope                             | Completeness/correctness of the M4 increment-1 requirement + documentation delta against ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2, the cited Microchip PolarFire SoC references and norms N1–N17 |
| Independence declaration          | Reviewer took no part in authoring, implementing, verifying or executing the M4 items; review ran in a fresh context with only the repository contents, the applicable standards and public IP references. Every claim below is reproducible from the cited evidence |
| Norms applied                     | N1–N17 (see `docs/rules.md`)           |

> Report document id `FRICK-REVIEW-0026` is the next free number in the
> `FRICK-REVIEW-00nn` namespace: `0025` is used by the concurrent R1/R2 M4 report
> (`docs/reviews/2026-10-10-m4-gem-r1-r2-review.md`, present in the tree during
> this review). The id-uniqueness scan confirms `0026` is unused (see §1).

## 1. Scope

Two independent profiles were applied to the M4 increment-1 delta (Cadence GEM
Ethernet driver + third-party integration scaffolding):

- **R3 (Senior Systems Engineer, norm N14):** review of the new system
  requirement `SSS-REQ-020` ("Reference low-level implementations") and its child
  `SRS-REQ-036` — requirement quality (complete, unambiguous, verifiable,
  feasible), the system-level feasibility of the "MPFS low-level layers and
  1 Gb/s Ethernet" objective (OBJ-11) at criticality B, the system-facing
  hardware claim (native GEM TSN), consistency with the target platform
  (RTPFS160ZT/RTEMS 6.2) and traceability.
- **R4 (Senior SW documentation reviewer, norm N17):** completeness and
  correctness of the documentation set touched by the change — requirement
  counts, coverage figures, revision/history coherence, document-id uniqueness,
  link resolution, README metrics panel, licence scoping (N13), English (N2) and
  internal cross-document consistency.

**Included:** the exact working-tree delta (16 modified + 5 untracked paths).
**Excluded:** deep register/descriptor code review is primarily the R1/R2
mandate (`FRICK-REVIEW-0025`); here the code is examined only where it bears on
the *verifiability/correctness of an SRS claim* and on the *system-feasibility*
of `SSS-REQ-020`. On-target PHY/link/board bring-up is the documented class-A
gap GAP-A5 and is not itself a finding.

Tools/checks executed by the reviewer (no repository file was modified except
this report):

```text
python3 tools/scripts/trace.py docs/requirements/traceability.yaml  -> PASS (exit 0)
python3 tools/scripts/metrics.py                                     -> matches README panel byte-for-byte
build/coverage/coverage.json read                                    -> 1421/2219 lines, 192/236 funcs
document-id uniqueness scan (all docs/**/*.md)                       -> 62 declarations, 0 duplicates
markdown local-link scan                                             -> 46 relative links checked, 0 broken
revision-header vs History scan (docs touched by the delta)          -> 5 match; 2 unrecorded changes (R4-1)
GEM register-layout cross-check (macb.h, MPFS binding)               -> 4 static register defects (R3-1)
```

`trace.py` output: **PASS** — `71 requirements (srs=37, sss=20, gap=14)`,
`51 lean theorems`, `CI evidence claims: 9`. This is consistent with every count
stated in `traceability.md`, `README.md`, the README metrics panel, the M4 row of
`verification-results.md`, `status.json` and the M4 ctest arithmetic
(`15+20+9+29+101+31+20+81+18 = 324`, 9/9 targets).

## 2. References

- Project norms: `docs/rules.md` (N1–N17); reviewer profiles:
  `docs/engineering/independent-review.md`.
- Traceability manifest and gate: `docs/requirements/traceability.yaml`,
  `tools/scripts/trace.py`; metrics/coverage: `tools/scripts/metrics.py`,
  `build/coverage/coverage.json`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.
- Microchip PolarFire SoC Product Overview §3.5.1 (Gigabit Ethernet MAC,
  DS60001656) and Radiation-Tolerant PolarFire SoC Product Overview (DS00005395);
  PolarFire SoC MSS TRM (DS60001702); Linux `drivers/net/ethernet/cadence/macb.h`
  (Cadence GEM register/bitfield definitions; the binding used by the MPFS
  `microchip,mpfs-macb` compatible).
- Decision record: `docs/adr/ADR-0014-drivers-and-third-party.md`; architecture:
  `docs/architecture/hardware-abstraction.md`.

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R3-1 | **Major** | SSS-REQ-020 / SRS-REQ-036 (objective OBJ-11) | **The delivered reference GEM driver does not program the Cadence GEM correctly, so the system requirement for a 1 Gb/s Ethernet MAC is not met and its declared verification (`Tst | Inp`) is not substantiated.** (a) Gigabit mode is never enabled: `FF_GEM_NWCFG_GIGE = 1u<<8` sets the Network Config "receive 1536-byte frames" bit (`MACB_BIG`=8), whereas GEM gigabit enable is NCFGR bit 10 (`GEM_GBE`). (b) Speed/full-duplex are written into Network *Control* bits 8/9 — which are `BP` (back-pressure) and `TSTART` (start transmission) — instead of NCFGR bits 0/1 (`SPD`/`FD`). (c) The DMA receive-buffer size is written to DMACFG bits [4:0] (`FBLDO`, burst length) instead of the `RXBS` field at bits [23:16]; the value `0x4` also contradicts the macro name `_1536` and the inline comment `64*2^4 = 1024`. (d) The RX path never consumes an RX descriptor: it dispatches `rx_buf[0]` with a length read from `RXSTATUS >> 16`, but Receive Status (0x20) defines only bits 0–2; the completed length lives in the RX descriptor control word. The 18-check unit test passes because the mock MMIO bus encodes the same incorrect model (it special-cases the network-control register and injects `0x2 | (42 << 16)`). These are static facts (not deferred PHY/link behaviour) that can be checked now; on target the MAC would not be in gigabit mode and the RX ring would be inert. | `src/drivers/eth/gem/ff_gem_regs.h:40-41,44,51-52`; `src/drivers/eth/gem/ff_gem.c:60-64,84-92,188-211`; `tests/unit/test_gem.c:40-42,140`; `macb.h`: NCR `MACB_BP`=8/`MACB_TSTART`=9, NCFGR `MACB_SPD`=0/`MACB_FD`=1/`MACB_BIG`=8/`GEM_GBE`=10, DMACFG `GEM_RXBS_OFFSET`=16 (size 8). Independent corroboration: concurrent R1/R2 report `FRICK-REVIEW-0025` (R1-02, R1-03, R1-05, R2-01 — graded Major). | Correct the register constants and the NCFGR/NCR programming against the MSS TRM (DS60001702) and the MPFS `macb` layout; implement descriptor-based RX (ownership/wrap/EOF/length); rebuild the mock from an *independent* device model and assert full register/descriptor images; record the `Inp` inspection evidence. Do not close `SSS-REQ-020`/`SRS-REQ-036` until the R1/R2 Majors (`M4-NC-01..05`) are fixed and delta-re-reviewed (norm N12/N14). |
| R3-2 | Comment | SSS-REQ-020 (separable/optional) | **The "separable, optional components" clause is only partially realised.** `SSS-REQ-020` requires reference implementations of the HAL contracts "as separable, optional components, keeping the middleware independent of them". The increment adds a compile-time toggle (`FF_BUILD_DRIVERS`, default `ON`) but `ff_gem.c` is compiled into the monolithic `frick_fsw_core` static library — there is no per-driver target, so a user cannot link the core without the driver object (only rebuild with the option off). `hardware-abstraction.md` §6 still describes per-driver targets as future M4 work. | `docs/requirements/SSS.md:89-92`; `CMakeLists.txt:22,97-100`; `docs/architecture/hardware-abstraction.md:99-106` | Add a per-driver CMake target (e.g. `frick_fsw_driver_gem`) that the core does not link, or reword SSS-REQ-020/"separable" to "compile-time optional" until the partitioning is delivered. |
| R3-3 | Comment | SRS-REQ-036 (scope vs evidence) | **SRS-REQ-036 bundles a TSN capability with no delivered adapter or evidence.** The requirement's last clause ("native GEM TSN (802.1Q/Qbv/Qav/802.1AS) available through `FF_TsnBackend_t`") is not covered by the manifest links (only the GEM source + `test_gem.c`, which never references `FF_TsnBackend_t`). The factual hardware claim itself is **correct** (see "Conformance verified"); the issue is requirement scope vs delivered/verified item for this increment. | `docs/requirements/SRS.md:174-179`; `docs/requirements/traceability.yaml` (SRS-REQ-036); `tests/unit/test_gem.c` (no TSN symbol) | Split the TSN clause into its own requirement when the `FF_TsnBackend_t` adapter lands, or narrow SRS-REQ-036 to the MAC driver for increment 1 (cf. concurrent R2-03 / `M4-NC-12`). |
| R3-4 | Comment | `docs/architecture/hardware-abstraction.md` §7; SRS-REQ-026 | **The HAL architecture note and the SRS-REQ-026 `lean_note` are now stale about the Ethernet backend.** §7 states "SRS-REQ-026..032 are interface declarations … their implementations are M4 and carry their own requirements/tests then", and the manifest note for SRS-REQ-026 says "The drivers behind the contract are M4". Increment 1 now delivers exactly the SRS-REQ-026 backend (the GEM `FF_EthMac_t` implementation). The text still reads as if all HAL backends are unwritten. | `docs/architecture/hardware-abstraction.md:115-117`; `docs/requirements/traceability.yaml` (SRS-REQ-026 `lean_note`) | Update §7 and the note to name the delivered GEM backend (mapped to SRS-REQ-036) and list the still-missing backends (net/TSN/CAN/serial/…). |

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R4-1 | Minor | `docs/architecture/hardware-abstraction.md`; `docs/roadmap.md` (norms N16/N8) | **Two documents changed by the delta carry no revision bump / `History` row, leaving header and history silently stale.** `hardware-abstraction.md` was edited (the TSN resource row was rewritten) but `Revision` stays `0.1` and the last `History` row is still `0.1` ("Initial HAL specification … M3"). `roadmap.md` was edited (M4 `planned` → `in progress`) but `Revision` stays `0.6` and the last `History` row is `0.6` ("Add M7 …"). Every other touched document was correctly bumped: `SSS.md` 0.7, `SRS.md` 0.7, `traceability.md` 0.6, `licensing.md` 0.5, `development-plan.md` 0.5, `verification-results.md` 0.7. | `git diff b4a1e5f -- docs/architecture/hardware-abstraction.md docs/roadmap.md`; `hardware-abstraction.md:7,119-123`; `roadmap.md:5,111-120` | Bump each revision and add an M4 `History` row describing the change (TSN resource correction; M4 status). |
| R4-2 | Minor | `README.md` §Low-level drivers (norms N8/N17) | **The README's driver inventory contradicts this same increment.** It states "No external device drivers (SpaceWire, CAN, Ethernet/IP, TSN, PPS) are implemented yet" and its "Currently available" table lists only PAL/link/board — while the M4 delta adds `src/drivers/eth/gem/` (built by default, `FF_BUILD_DRIVERS=ON`) with its unit test. The status panel and `status.json` simultaneously report M4 *in progress*, so the README is internally inconsistent and under-reports the delivered code. | `README.md:118-135` vs `CMakeLists.txt:22,97-100`, `src/drivers/eth/gem/ff_gem.c`, `tests/unit/test_gem.c`; `docs/status/status.json:12` | Add a "Cadence GEM (Ethernet MAC)" row to the driver table and reword the "no drivers" paragraph to reflect increment-1 scope and what remains (net/TSN/CAN/serial/…). |
| R4-3 | Comment | `AGENTS.md` §2 (repository map); `README.md` §Repository layout (norm N8) | The `src/` repository map omits the new `src/drivers/` tree (and the pre-existing `hal/`, `math/`, `security/`). A fresh agent using the mandated entry point (`AGENTS.md`) cannot locate the driver from the map. | `AGENTS.md:30`; `README.md:249-261`; `src/drivers/eth/gem/` | Add `drivers/` (and the other unlisted directories) to the map/layout table. |
| R4-4 | Comment | `docs/engineering/driver-development.md` (norm N8) | The driver guide still describes "three narrow interfaces" (§1 table: PAL, link, board) and has no HAL `FF_EthMac_t` backend example, although M4 delivers the first HAL device driver. The guide's own §9 checklist requires its `History` to be updated on such changes. | `docs/engineering/driver-development.md:20-27,143-152,177-186` | Add the HAL/`FF_EthMac_t` extension point and the GEM driver as the worked example; bump the guide's revision/History. |
| R4-5 | Comment | `docs/quality/licensing.md`; `third_party/README.md`; `tools/scripts/fetch_third_party.sh` (norms N8/N13) | **Third-party pinning is partly non-immutable and the licence precision could be tighter.** lwIP is pinned by the moving tag `STABLE-2_2_1_RELEASE` rather than an immutable commit, and `fetch_third_party.sh` does not verify the checked-out revision; `asn1-pusc-lib` is pinned by commit (good) but is fetched with a full clone. The licence is stated as "GPL-3.0" without distinguishing `-only`/`-or-later`. (The compatibility direction is correct — see "Conformance verified".) | `third_party/README.md:10-12`; `tools/scripts/fetch_third_party.sh:17-28`; `docs/quality/licensing.md:45-46` | Pin lwIP by commit hash, add a revision/checksum check to the fetch script, and state licence SPDX expressions precisely (`GPL-3.0-only` if applicable). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### Conformance verified (no finding)

Recorded so the review is reproducible.

- **GEM TSN claim — correct.** The correction from the previous "(GEM has no
  TSN) / fabric via FIC" to "native GEM TSN block" is accurate and
  well-sourced: the Microchip PolarFire SoC Product Overview §3.5.1 and the
  Radiation-Tolerant PolarFire SoC Product Overview state that *each GEM*
  supports IEEE 802.3br (frame pre-emption), 802.1Qci, 802.1Qbb, 802.1Q VLAN,
  802.1AS, 802.1Qav, 802.1Qbv and 1588; the MSS TRM additionally describes
  802.1CB stream identification / duplicate elimination. Fabric CoreTSN is needed
  only for multi-port switching/FRER. The rows in
  `docs/architecture/hardware-abstraction.md:51`, `docs/development-plan.md:65`,
  ADR-0014:17-19, `SRS.md:178-179` and `CHANGELOG.md:80-81` are consistent with
  this. (Independently corroborated by `FRICK-REVIEW-0025` C-2.)
- **Traceability gate:** `trace.py` → **PASS**, exit 0, `71/37/20/14`, 51 Lean
  theorems, 9 CI evidence claims; `SSS-REQ-020 <- 1 srs child` in the bottom-up
  hierarchy. Ids are unique in `SSS.md`/`SRS.md` and the gate validates both
  directions.
- **Counts (71/37/20/14/51/9) mutually consistent** across
  `docs/requirements/traceability.md:18`, `README.md:72,244`, the README metrics
  panel, `verification-results.md:27` and `trace.py`; the M4 ctest arithmetic
  (`9/9`, `324 = 306 + 18`, `gem 18`) reconciles.
- **README metrics panel vs `metrics.py`:** byte-for-byte identical;
  `build/coverage/coverage.json` = `1421/2219` lines, `192/236` funcs, matching
  the panel and `verification-results.md`.
- **Document ids unique:** 62 `Document id` declarations across `docs/**/*.md`
  (including the concurrent `FRICK-REVIEW-0025`), **0 duplicates**;
  `FRICK-REVIEW-0026` is free and used here. ADRs carry `Status`/`Date` (the
  documented ADR convention), not a `Document id`.
- **Links resolve:** 46 relative markdown links checked, 0 broken.
- **Licences coherent (N13):** lwIP (BSD-3-Clause) and n7space/`asn1-pusc-lib`
  (GPL-3.0) are both compatible with the project `AGPL-3.0-only`; GPL-3.0/AGPL-3.0
  combination is permitted and a BSD-3 aggregate requires attribution only. The
  pinned commit `2126df11cfe664b9f6f9d7d7a49312bb70768e75` exists and the trees
  are correctly `.gitignore`d / not committed; the entry points stay user
  adapters. (Precision remark in R4-5.)
- **CI unchanged and consistent:** both workflows are untouched by the delta;
  the 9 `# evidence:` claim names (8 referenced by the manifest, `ci-lean` extra)
  match `traceability.md`'s statement.
- **Revision histories** of the five other touched documents match their
  headers and describe the M4 change; `hardware-abstraction.md` and `roadmap.md`
  are the sole exceptions (R4-1). Status is coherent (`status.json`,
  `roadmap.md`, `development-plan.md` all show M4 *in progress*).
- **English (N2):** the delta is in English; only intentional non-ASCII
  characters (em dash, `Eärendil`, arrows).
- **SPDX headers:** all new source/header/test files carry
  `SPDX-License-Identifier: AGPL-3.0-only`; new engineering docs carry no SPDX
  header, consistent with the N13 scoping.

## 4. Conclusion

### R3 — Senior Systems Engineer (N14)

- [ ] APPROVED — no findings or Comments only
- [ ] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [x] **REJECTED** — one or more Major findings

`SSS-REQ-020` is well-formed and traceable, and the system-facing hardware
correction (native GEM TSN) is **verified correct** against the PolarFire SoC
Product Overview §3.5.1 and the MSS TRM. However, the increment does not deliver
a GEM MAC that meets the objective it is meant to serve: the register/descriptor
programming is incorrect against the Cadence GEM layout used by the MPFS
`microchip,mpfs-macb` binding, so gigabit mode, speed, duplex and the RX DMA
buffer are not configured and the RX descriptor ring is inert (R3-1, **Major**);
the mock-based test cannot detect this, so `SRS-REQ-036`'s `Tst | Inp`
verification is not substantiated. This defect is independent of the deferred
PHY/link GAP-A5. R3-2, R3-3 and R3-4 are Comments. The system requirement set is
sound but **cannot be accepted for this increment** until R3-1 (and the
corroborating R1/R2 Majors) are corrected and delta-re-reviewed (norm N12/N14).

### R4 — Senior SW documentation reviewer (N17)

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

All quantitative claims are derivable and mutually consistent (71/37/20/14
requirements, 51 theorems, 9 CI claims, coverage 1421/2219 / 192/236), the trace
gate passes, document ids are unique, links resolve, the README panel matches
`metrics.py`, and the licence analysis is coherent with N13. Two Minor
documentation non-conformities must be corrected before commit (norm N8/N12/N17):
R4-1 (unrecorded revision/history for `hardware-abstraction.md` and `roadmap.md`)
and R4-2 (README driver inventory contradicts the delivered GEM driver); R4-3,
R4-4 and R4-5 are Comments. No Major finding **in the documentation scope**.

> Note: although the documentation review is APPROVED WITH COMMENTS, the M4
> increment as a whole is **not** ready to commit — the concurrent R1/R2 review
> (`FRICK-REVIEW-0025`) and this review's R3-1 have open Major findings
> (norm N12/N14/N17).

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R3 (N9/N14)   | independent agent (fresh context) | 2026-10-10 | REJECTED (1 Major + 3 Comments open) |
| R4 (N9/N17)   | independent agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS (2 Minor + 3 Comments open) |

## 6. Follow-up records

Norm N12/N14/N17: every finding is tracked to closure; the delta is re-reviewed
from fresh context after correction. This report must be committed with the
reviewed change and a row added to `docs/reviews/README.md` (norm N11).

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R3-1 | **Major** | SSS-REQ-020 / SRS-REQ-036 | GEM register/descriptor programming incorrect (GBE/SPD/FD/DMACFG RXBS; RX not descriptor-based); `Tst|Inp` evidence not substantiated | (to dispose) | corrected constants + device-faithful register/descriptor assertions; `Inp` record | frick-fsw | **open** | required |
| R4-1 | Minor | `hardware-abstraction.md`, `roadmap.md` (N16) | M4 edits with no revision bump / `History` row | (to dispose) | bumped revisions + M4 History rows | frick-fsw | **open** | required |
| R4-2 | Minor | `README.md` §Low-level drivers (N8) | "No Ethernet driver implemented" + table omits the new GEM driver | (to dispose) | README driver row + reworded paragraph | frick-fsw | **open** | required |
| R3-2 | Comment | SSS-REQ-020 | "separable" only a compile toggle; driver linked into monolithic core lib | (to dispose) | per-driver target or rewording | frick-fsw | **open** | optional |
| R3-3 | Comment | SRS-REQ-036 | TSN clause bundled without delivered adapter/evidence | (to dispose) | split/retarget requirement | frick-fsw | **open** | optional |
| R3-4 | Comment | `hardware-abstraction.md` §7 / SRS-REQ-026 | Stale "implementations are M4" text now that the GEM backend is delivered | (to dispose) | §7/`lean_note` updated | frick-fsw | **open** | optional |
| R4-3 | Comment | `AGENTS.md`, `README.md` maps | `src/drivers/` (and hal/math/security) missing from repo maps | (to dispose) | maps updated | frick-fsw | **open** | optional |
| R4-4 | Comment | `driver-development.md` | No HAL/`FF_EthMac_t` extension point or GEM example; History stale | (to dispose) | guide extended + bumped | frick-fsw | **open** | optional |
| R4-5 | Comment | `licensing.md`, `third_party/README.md`, `fetch_third_party.sh` | lwIP pinned by tag; no fetch verification; licence `-only`/`-or-later` unspecified | (to dispose) | commit pin + verification + SPDX precision | frick-fsw | **open** | optional |

---

Committed and pushed with the reviewed change (norm N11).
