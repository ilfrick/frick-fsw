# SYS-REVIEW-R1-R3-20261112 — Avionics/Systems Independent Review

**Reviewer profile:** R1/R3 — Senior Aeronautics and Space Avionics Systems Engineer (independent review)
**Repository:** /home/nicola/fsw/frick-fsw
**Branch / HEAD:** main / c3fe024
**Target / ECSS criticality:** Microchip RT PolarFire SoC RTPFS160ZT (RISC-V) under RTEMS 6.2; ECSS class B
**Review date:** 2026-11-12
**Change under review:** `tests/unit/test_pus.c` — added `test_service1_progress_success` verifying PUS service 1 subservice 5 (TC[17,1] with progress ack → TM[1,5]) end-to-end through `FF_PusIngest`.
**Scope of change:** One test file only (`tests/unit/test_pus.c`). No `src/` change. No API, interface, traceability, target, or hardware change. Host-executable only.

---

## 1. Change summary (technical basis for the review)

The new test `test_service1_progress_success` (`tests/unit/test_pus.c:558-599`) exercises the PUS verification path with exactly one acknowledgement flag:

- Builds a CCSDS TC[17,1] (ping) with `tc.ack_flags = FF_PUS_ACK_PROGRESS` only (progress, no acceptance/start/completion).
- Feeds it through the full TC ingestion path: `FF_CcsdsEncodeTc` → `FF_PusIngest` (`src/pus/pus.c:390`, declaration `src/pus/pus.h:56`).
- Asserts the emitted telemetry stream contains exactly one `TM[1,5]` (verification progress success) and exactly one `TM[17,2]` (ping reply), and contains no `TM[1,1]` (acceptance) and no `TM[1,7]` (completion) since those flags were not requested.
- Uses the same harness pattern as the existing tests: `FF_PalInit`/`FF_EvsInit`/`FF_TimeInit`/`FF_PusInit`/`FF_PusSetBackend(&FF_PusBackendOpen)`/`FF_PusSetTmSink(tm_sink)`, `stream_has`/`stream_count` oracles (`tests/unit/test_pus.c:154-185`).

The production functions on the path (`FF_PusIngest` at `src/pus/pus.c:390`; `FF_PusSendVerification` at `src/pus/pus.c:298`, including the `FF_PUS_STAGE_PROGRESS` emission at `src/pus/pus.c:456`; declarations `src/pus/pus.h:56,76`) are **not modified**. No wire layout, ICD, public signature, or behavior of production code changes.

Coverage context:
- `test_ping_roundtrip` (`tests/unit/test_pus.c:187-248`) covers acceptance (`TM[1,1]`), start (`TM[1,3]`) and completion (`TM[1,7]`) plus ping reply (`TM[17,2]`), but explicitly not progress (`TM[1,5]`).
- `test_verification_missing_subservices` (`tests/unit/test_pus.c:601+`) covers start/progress **failure** emission.
- The new test covers progress **success** emission — the one un-covered success branch of Service 1 verification staging.

Registered in `FF_TEST_MAIN` at `tests/unit/test_pus.c:1274`. `FF_TEST_ASSERT` counts failures and continues (does not abort) — `tests/ff_test.h:17-24`.

---

## 2. Findings

| ID | Classification | Blocks commit/push? | Finding (one line) |
|----|----------------|---------------------|--------------------|
| SYS-R1/R3-001 | Minor | No | System-level risk of adding `test_service1_progress_success`: none introduced |
| SYS-R1/R3-002 | Minor | No | NC-PAL-01 / GAP-A5 / target hardware: unaffected |
| SYS-R1/R3-003 | Minor | No | GNC / safety / FDIR / operations interfaces: not impacted |
| SYS-R1/R3-004 | Minor | No | Cross-disciplinary consistency: consistent with existing `test_pus.c` patterns |
| SYS-R1/R3-005 | Minor | No | S1 subservice 5 progress-success coverage: appropriate gap-fill |
| SYS-R1/R3-006 | Minor | No | Blocking systems findings: none |

---

### SYS-R1/R3-001 — System-level risk of the test addition: none introduced (Minor)

**Reference:** `tests/unit/test_pus.c:558-599`; production path `src/pus/pus.c:298,390,456` (unchanged).

**Assessment.** This is a **test-only, host-executable** addition. It calls only public/test-facing APIs (`FF_PusInit`, `FF_PusSetBackend`, `FF_PusIngest`, `FF_CcsdsEncodeTc`, `FF_PusSetTmSink`) and inspects an in-process TM sink buffer. It writes no register, no file, no hardware, no shared global state outside `g_rx`/`g_rx_count` (already used by every other test in the file). Because no `src/` production code changes, it cannot introduce runtime system-level risk on target (flight or ground). The assertions are passive (presence/absence/count of TM service/subservice pairs).

**Confidence:** High — directly observed in the diff; single new test function (~36 lines) plus one registration line; all gates reported green by the change owner (build, CTest including `test_pus.c`, Lean, trace, format, license).

**Blocks commit/push:** No.

---

### SYS-R1/R3-002 — Target/hardware and open NC items: not affected (Minor)

**Reference:** `tests/unit/test_pus.c:558-599`; open items tracked project-wide (NC-PAL-01, GAP-A5, NC-EXT-19/90, N22-TS5-47).

**Assessment.** Per the stated scope and AGENTS.md ground rules, the change is host-executable only: no target build, no BSP/PAL/device-tree/register access, no hardware interaction, no RTEMS configuration change. Consequently:

- **NC-PAL-01** (MPFS/MSS watchdog not wired / N18 gate) — untouched.
- **GAP-A5** residuals (HWIL/target items such as `N22-TS5-47`, `N22-TS2-78`, GEM PHY/link, PCIe enumeration, on-target timing) — untouched.
- **OBCP engine**, **NC-EXT-19/90**, **N22-TS5-47** — untouched and remain OPEN.

No target or hardware item is affected, and no existing open NC is opened, closed, or reclassified by this change.

**Blocks commit/push:** No.

---

### SYS-R1/R3-003 — GNC, safety, FDIR, operations interfaces: not impacted (Minor)

**Reference:** `tests/unit/test_pus.c:558-599`; ICD §4 Service 1 rows (`docs/interfaces/ICD.md`); `SRS-REQ-073/074/075` (watchdog/FDIR per `traceability.yaml`).

**Assessment.** PUS Service 1 verification TM is a ground command-acknowledgment / ground visibility path. Adding a host test that observes `TM[1,5]` does not alter:

- control authority or GNC/control loops;
- FDIR behavior (watchdog arm/feed, pre-reset event, single reset path — separate from PUS verification staging);
- dispatch/degraded-operation or maintenance interfaces;
- the wire layout emitted on the wire (unchanged; no production code modified).

No public API/interface change; no ICD row modified by this change; no safety-analysis delta required from systems (test evidence only).

**Blocks commit/push:** No.

---

### SYS-R1/R3-004 — Cross-disciplinary consistency: consistent with existing `test_pus.c` patterns (Minor)

**Reference:** `tests/unit/test_pus.c:154-185,187-248,558-599,601+,1274`; `tests/ff_test.h:17-24`.

**Assessment.** The new test is consistent across disciplines:

- **FSW / test pattern:** identical fixture sequence to `test_ping_roundtrip` (`tests/unit/test_pus.c:189-194`) and `test_verification_missing_subservices` (`tests/unit/test_pus.c:608-613`); same `stream_has`/`stream_count` oracles (`tests/unit/test_pus.c:154-185`); same `FF_PusBackendOpen` backend; same `g_rx_count` reset idiom.
- **Requirements/traceability:** behavior-preserving (observes existing production behavior, does not change it); no new or modified requirement; no traceability-matrix delta; trace gate reported green.
- **Lean/formal:** no Lean-proved function affected; no theorem delta.
- **V&V/oracle:** nominal success case with explicit expected results (presence + exact counts + absence of non-requested TM), consistent with N22 nominal coverage; complements the existing failure-case coverage in `test_verification_missing_subservices`.
- **PA/CM:** single test-file change; configuration identification updated by the normal commit record; no waiver/deviation.

**Blocks commit/push:** No.

---

### SYS-R1/R3-005 — S1 subservice 5 progress-success coverage: appropriate gap-fill (Minor)

**Reference:** `tests/unit/test_pus.c:558-599` vs `test_ping_roundtrip` (`tests/unit/test_pus.c:187-248`) and `test_verification_missing_subservices` (`tests/unit/test_pus.c:601+`).

**Assessment.** Adding coverage for Service 1 subservice 5 (progress success) is appropriate:

- The verification staging machine emits distinct TM subservice codes per stage/success (acceptance 1, start 3, progress 5, completion 7 on success; 8 on failure). `test_ping_roundtrip` requested acceptance|start|completion and covered `TM[1,1]/TM[1,3]/TM[1,7]`; `test_verification_missing_subservices` covered failure emission for start/progress. The progress **success** path (`TM[1,5]`) was not previously asserted as a standalone success case.
- The test isolates progress by requesting only `FF_PUS_ACK_PROGRESS` and explicitly asserts `TM[1,1]` and `TM[1,7]` are absent, which guards against stage-bleed (e.g., progress flag implicitly triggering acceptance/completion TM) — a meaningful regression guard for FDIR-visible ground acknowledgments.
- It is bounded (exactly one `TM[1,5]`, exactly one `TM[17,2]`) and host-executable, so it adds assurance without target cost or hardware dependency.

No systems objection to the coverage addition; it strengthens the verification evidence envelope for Service 1 ground acknowledgments.

**Blocks commit/push:** No.

---

### SYS-R1/R3-006 — Blocking systems findings: none (Minor)

**Reference:** findings SYS-R1/R3-001 through SYS-R1/R3-005 above.

**Assessment.** No Major systems finding exists. The change neither introduces nor depends on any unresolved safety/security/FDIR/hardware/environment item. Pre-existing open NCs (NC-PAL-01, NC-EXT-19/90, GAP-A5 residuals, N22-TS5-47) are unrelated to `tests/unit/test_pus.c` and remain tracked in their existing locations; this change neither opens nor closes them.

**Blocks commit/push:** No.

---

## 3. Overall determination

**No avionics/systems-level finding blocks commit or push.**

The addition of `test_service1_progress_success` (`tests/unit/test_pus.c:558-599`, registered at `tests/unit/test_pus.c:1274`) is a benign, host-executable, test-only gap-fill for PUS Service 1 subservice 5 (progress success). It introduces no system-level risk, touches no target/hardware item (NC-PAL-01/GAP-A5 unaffected), impacts no GNC/safety/FDIR/operations interface, is consistent with existing `test_pus.c` patterns, and requires no interface/requirements/traceability change.

This systems determination is independent of, and does not substitute for, the project's separate gates owned by the designated agents: FSW build/test, Lean N1, trace N4, V&V N10/N11/N18/N22, PA N20, documentation N17, and the commit/push authority N7/N15/N21.

---

**Reviewed by:** Senior Aeronautics and Space Avionics Systems Engineer (R1/R3 independent review)
**Date:** 2026-11-12
**Disposition:** No systems objections; commit/push not blocked by systems review.
