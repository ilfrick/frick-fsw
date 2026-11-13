# SYS-REVIEW-R1-R3-20261112 — Avionics/Systems Independent Review

**Reviewer profile:** R1/R3 — Senior Aeronautics and Space Avionics Systems Engineer (independent review)
**Repository:** /home/nicola/fsw/frick-fsw
**Branch / HEAD:** main / b6bf060
**Target / ECSS criticality:** Microchip RT PolarFire SoC RTPFS160ZT (RISC-V) under RTEMS 6.2; ECSS class B
**Review date:** 2026-11-12
**Change under review:** `src/pus/pus.c` line 301 — `uint8_t payload[7];` → `uint8_t payload[7] = {0};` in `FF_PusSendVerification()`
**Scope of change:** One source file only (`src/pus/pus.c`). No API, interface, traceability, or target change.

---

## 1. Change summary (technical basis for the review)

`FF_PusSendVerification()` builds the PUS Service 1 (verification) TM payload:

- `payload[0..5]` are explicitly assigned at `src/pus/pus.c:335-342` (Request ID: source_id[2], destination_id[2], sequence_count[2] big-endian).
- `payload[6] = tc->service;` at line 343.
- `memcpy(extended, payload, 6)` copies `payload[0..5]` → `extended[0..5]` at line 349.
- `extended[6] = service`, `extended[7] = subservice`, `extended[8] = code` at lines 350-352.
- `FF_PusSendTm(FF_PUS_SERVICE_VERIFICATION, subservice, extended, 9)` sends 9 octets at line 353.

All seven `payload` bytes are already written before use in the current control flow (every `switch` case sets `subservice` and `break`s; `default` returns early), so the wire output is deterministic today. The `= {0}` initializer adds **defense-in-depth**: it guarantees all stack bytes are defined if a future maintenance change adds a switch case/code path that does not fully write `payload`, and it removes any static-analysis flag on the stack array. It does **not** alter current wire behavior, output length, encoding, or the public contract.

Public signature: `src/pus/pus.h:76-77` — unchanged. No new symbol. No ICD wire-layout change (Request ID[6] + service + subservice + code = 9 octets remains).

---

## 2. Findings

| ID | Classification | Blocks commit/push? | Finding (one line) |
|----|----------------|---------------------|--------------------|
| SYS-R1/R3-001 | Minor | No | Pure zero-initialization; no system-level risk introduced |
| SYS-R1/R3-002 | Minor | No | NC-PAL-01 / GAP-A5 / target hardware unaffected |
| SYS-R1/R3-003 | Minor | No | GNC, safety, FDIR, operations interfaces not impacted |
| SYS-R1/R3-004 | Minor | No | No interface/requirements/traceability change needed |
| SYS-R1/R3-005 | — (none) | No | No blocking systems finding |

---

### SYS-R1/R3-001 — System-level risk of the initialization fix: none introduced (Minor)

**Reference:** `src/pus/pus.c:301`; public contract `src/pus/pus.h:76-77`.

**Assessment.** The change is a behavior-preserving stack-array zero-initialization. It cannot introduce system-level risk: it only removes stack indeterminism and adds a maintainability guard against a future incomplete-write regression. There is no public API/interface change (signature unchanged, no new symbol), no wire-format change (9-octet verification TM unchanged), and no traceability-requirement delta.

**Confidence:** High — directly observed in source; single-line diff; build/test/Lean/trace gates reported green by the change owner (build exit 0 / 0 warnings, CTest exit 0 including `test_pus.c`, Lean lake build, trace PASS 120/76/30/14/68, format-check PASS, license OK).

**Blocks commit/push:** No.

---

### SYS-R1/R3-002 — Target/hardware and open NC items: not affected (Minor)

**Reference:** `src/pus/pus.c:298-354`; open items tracked project-wide.

**Assessment.** This is a **source-code-only, host-executable** fix (per AGENTS.md ground rules and stated scope). There is no register write, no BSP/PAL/device-tree change, no target build, and no hardware interaction. Consequently:

- **NC-PAL-01** (MPFS/MSS watchdog not wired / N18 gate) — untouched.
- **GAP-A5** residuals (HWIL/target items such as `N22-TS5-47`, `N22-TS2-78`, GEM PHY/link, PCIe enumeration, on-target timing) — untouched.
- **OBCP engine**, **NC-EXT-19/90** — untouched.

No target or hardware item is affected, and no existing open NC is opened, closed, or reclassified by this change.

**Blocks commit/push:** No.

---

### SYS-R1/R3-003 — GNC, safety, FDIR, operations interfaces: not impacted (Minor)

**Reference:** `src/pus/pus.c:298-354`; ICD §4 Service 1 rows (`docs/interfaces/ICD.md:94-106`); `SRS-REQ-073/074/075` (watchdog/FDIR, per `traceability.yaml:681-699`).

**Assessment.** PUS Service 1 verification TM is a ground command-acknowledgment path. Zero-initializing its payload does not alter:

- control authority or GNC/control loops;
- FDIR behavior (watchdog arm/feed, pre-reset event, single reset path are separate — `SRS-REQ-073/074/075` are not traced to this PUS function);
- dispatch/degraded-operation or maintenance interfaces;
- the wire layout emitted on the wire (Request ID[6] + service + subservice + code = 9 octets, unchanged).

No public API/interface change; no ICD row modified by this change.

**Blocks commit/push:** No.

---

### SYS-R1/R3-004 — Cross-disciplinary consistency: no interface/requirements/traceability change required (Minor)

**Reference:** `src/pus/pus.c:298-354`; `tests/unit/test_pus.c:409-480`; `verification/lean/`; `docs/requirements/traceability.yaml`.

**Assessment.** The fix is consistent across disciplines:

- **FSW:** function-local only; no signature/symbol/behavior change to the public contract.
- **Requirements/traceability:** behavior-preserving for the current flow (all payload bytes written before use); no new or modified requirement; no traceability-matrix delta.
- **Lean/formal:** `FF_PusSendVerification` is not a Lean-proved function affected by this change; no theorem delta.
- **V&V/test:** `test_pus.c` already exercises `FF_PusSendVerification` across stages (acceptance/start/progress/completion, success/fail) per N22 nominal + out-of-bound coverage; no test-script/procedure/oracle change required.
- **PA/CM:** single-file source change; configuration identification updated by the normal commit record; no waiver/deviation.

**Blocks commit/push:** No.

---

### SYS-R1/R3-005 — Blocking systems findings: none (No finding)

**Assessment.** No Major systems finding exists. The change neither introduces nor depends on any unresolved safety/security/FDIR/hardware/environment item. Pre-existing open NCs (NC-PAL-01, NC-EXT-198/N18, GAP-A5 residuals) are unrelated to `src/pus/pus.c` and remain tracked in their existing locations; this change neither opens nor closes them.

**Blocks commit/push:** No.

---

## 3. Overall determination

**No avionics/systems-level finding blocks commit or push.**

The change `src/pus/pus.c:301` (`uint8_t payload[7] = {0};`) is a benign, behavior-preserving initialization fix. It introduces no system-level risk, touches no target/hardware item (NC-PAL-01/GAP-A5 unaffected), impacts no GNC/safety/FDIR/operations interface, and requires no interface/requirements/traceability change.

This systems determination is independent of, and does not substitute for, the project's separate gates owned by the designated agents: FSW build/test, Lean N1, trace N4, V&V N10/N11/N18/N22, PA N20, documentation N17, and the commit/push authority N7/N15/N21.

---

**Reviewed by:** Senior Aeronautics and Space Avionics Systems Engineer (R1/R3 independent review)  
**Date:** 2026-11-12  
**Disposition:** No systems objections; commit/push not blocked by systems review.
