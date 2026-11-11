# Independent Avionics Systems Review — R1/R3 Profile

**Repository:** frick-fsw (branch `main`, HEAD `0cc455f`)
**Target:** Microchip RT PolarFire SoC RTPFS160ZT (RISC-V) under RTEMS 6.2, ECSS class B
**Review item:** `tools/scripts/run_sim.sh` CMake generator fix (NC-S4b-09)
**Review date:** 2026-11-11
**Reviewer profile:** Senior Aeronautics and Space Avionics Systems Engineer (R1/R3)
**Authority statement:** This review provides engineering evidence only. It does not certify, qualify, accept, close any NC, or authorize commit/push. Authorized humans retain all safety-critical and regulatory decisions.

---

## 1. Review scope and method

Scope: whether the committed `run_sim.sh` fix (remove forced `-G Ninja`; reuse the CMake generator cached in `BUILD_DIR`) introduces any system-level risk, whether NC-PAL-01 / GAP-A5 / any target/hardware item is affected, whether GNC / safety / FDIR / operations interfaces are impacted, cross-disciplinary consistency, and any findings that block commit/push or NC-S4b-09 closure.

Method:
- Inspected `tools/scripts/run_sim.sh` at HEAD and in the working tree (`git diff HEAD -- tools/scripts/run_sim.sh` = empty; fix IS committed).
- Inspected `Makefile` smoke/check chain (`Makefile:43`, `105`, `143-144`).
- Searched repository for `NC-S4b-09`, `NC-PAL-01`, `GAP-A5`, `run_sim`.
- Cross-checked `docs/status/handover.md`, `docs/status/loop-audit.md`, `docs/status/status.json`.
- Attempted local smoke run `BUILD_DIR=build/host bash tools/scripts/run_sim.sh`; build stopped on a **pre-existing third-party dependency gap** (missing `third_party/n7s-obcp/micropython`, required by the `pus-asn1`/`n7s-obcp` chain via `Makefile:143-144`); this is unrelated to `run_sim.sh`.
- Context-reported evidence accepted: smoke gate verified PASS (`BUILD_DIR=build/host bash tools/scripts/run_sim.sh` → PASS, exit 0) in an environment with third-party dependencies fetched.

---

## 2. Summary of verified facts

| Fact | Evidence |
|---|---|
| `run_sim.sh` fix is committed at HEAD | `git diff HEAD -- tools/scripts/run_sim.sh` empty; committed version contains no `-G Ninja`, reuses cached generator, references NC-S4b-09 at `tools/scripts/run_sim.sh:20-24` |
| Fix is host-only tooling | `run_sim.sh` boots the native simulator only; no target hardware, watchdog, FDIR, GNC, safety, or operations interfaces |
| NC-PAL-01 unaffected | NC-PAL-01 (MPFS MSS flight watchdog binding NOT WIRED) tracked in `src/core/pal/pal_rtems.c:34`, `src/core/pal/pal.h:123`, `src/core/pal/pal_watchdog.c:13`, `tests/python/test_qemu_runner_failclosed.py:64`; `run_sim.sh` touches none of these |
| GAP-A5 unaffected | GAP-A5 (target/HIL evidence) is a target-evidence gap; `run_sim.sh` is host simulator smoke tooling |
| GNC/safety/FDIR/operations unaffected | No SRS/SSS/ICD/traceability.yaml entry references `run_sim.sh`; no GNC loop, FDIR watchdog binding, safety analysis, or operations/interface change |
| No interface/requirements/traceability change needed | Change confined to build tooling comments + cmake invocation (`tools/scripts/run_sim.sh:19-25`) |
| Smoke gate PASS (context-verified) | `BUILD_DIR=build/host bash tools/scripts/run_sim.sh` → PASS, exit 0 (local re-run blocked only by pre-existing unfetched third-party micropython dep, not by `run_sim.sh`) |
| NC-S4b-09 status | OPEN pending independent closure (per context and `docs/status/loop-audit.md:191`) |

---

## 3. Findings

### SYS-R1/R3-001 — No system-level risk from the run_sim.sh generator fix (Minor)

**Description.** The fix changes only host-side CMake generator selection in `tools/scripts/run_sim.sh:19-25`: it removes the unconditional `-G Ninja` and reuses the generator already cached in `BUILD_DIR` (e.g. Unix Makefiles from `make configure`/`make build`). This is build-tooling behavior for the native simulator smoke test. It does not alter any flight function, timing, power, thermal, radiation, redundancy, partitioning, FDIR, GNC, safety, or operations interface.

**Classification:** Minor.

**File:line:** `tools/scripts/run_sim.sh:19-25` (cmake reconfigure/build); `Makefile:43` (smoke target); `Makefile:105` (check bundle).

**Blocks commit/push?** No.

**Blocks NC-S4b-09 closure?** No — closure is a review/authority action, not blocked by this finding.

---

### SYS-R1/R3-002 — NC-PAL-01, GAP-A5, and all target/hardware items are NOT affected (Minor / informational)

**Description.** NC-PAL-01 (flight MPFS MSS watchdog binding NOT WIRED; `FF_NOT_IMPLEMENTED` hooks), GAP-A5 (target/HIL evidence), and every target/hardware item (RTPFS160ZT registers, clocks, reset, MSS watchdog, RTEMS BSP, harness, installation) remain exactly as before. `run_sim.sh` is host simulator tooling and references none of them. No register write, no binding, no production change, no target evidence is introduced or removed.

**Classification:** Minor / informational (confirmation of non-impact).

**File:line:** `src/core/pal/pal_rtems.c:34,99,108,284,290,302`; `src/core/pal/pal.h:123`; `src/core/pal/pal_watchdog.c:13`; `docs/status/handover.md` (NC-PAL-01 register); `tools/scripts/run_sim.sh` (no target references).

**Blocks commit/push?** No.

**Blocks NC-S4b-09 closure?** No.

---

### SYS-R1/R3-003 — GNC, safety, FDIR, and operations interfaces are NOT impacted (Minor / informational)

**Description.** No GNC control loop, safety analysis input, FDIR watchdog/reset-recovery path, or operations/ICD interface is touched by the fix. `run_sim.sh` asserts only banner print + 7 registered applications + loopback EVS→PUS S5 telemetry on the host simulator. No SRS/SSS/ICD/traceability.yaml entry references `run_sim.sh`; no hazard, failure hypothesis, DAL/criticality, or assurance allocation changes.

**Classification:** Minor / informational (confirmation of non-impact).

**File:line:** `docs/requirements/traceability.yaml` (no run_sim.sh entry); `docs/interfaces/ICD.md` (no run_sim.sh entry); `tools/scripts/run_sim.sh:30-36` (smoke assertions only).

**Blocks commit/push?** No.

**Blocks NC-S4b-09 closure?** No.

---

### SYS-R1/R3-004 — Cross-disciplinary consistency; no interface/requirements/traceability change needed (Minor / informational)

**Description.** The change is confined to build tooling (`tools/scripts/run_sim.sh:20-24` comments and cmake invocation). No SRS, SSS, ICD, traceability.yaml, design document, test specification, or V&V tool record requires updating. Cross-disciplinary dependencies (FSW, PAL, FPGA/electronics, safety/dependability, security, EMC/environment/radiation/thermal/mechanical, V&V/lab, PA/quality/CM, certification/customer) see no delta from this fix.

**Classification:** Minor / informational.

**File:line:** `tools/scripts/run_sim.sh:20-24`; `docs/requirements/traceability.yaml` (unchanged); `docs/vv/tool-qualification.md` (unchanged by this fix).

**Blocks commit/push?** No.

**Blocks NC-S4b-09 closure?** No.

---

### SYS-R1/R3-005 — NC-S4b-09 remains OPEN pending independent closure (Minor)

**Description.** The code fix is committed at HEAD and the smoke gate passes (context evidence). However, NC-S4b-09 is a non-conformance record, not a code change. Its closure requires independent review acceptance (this report is the review input) and action by the authorized human authority; the NC register and closure disposition remain open. The fix itself does not block commit/push.

**Classification:** Minor.

**File:line:** `docs/status/loop-audit.md:191` ("NC-S4b-09 open pending delta review"); `tools/scripts/run_sim.sh:20-23` (fix references NC-S4b-09); `docs/status/handover.md` (NC register).

**Blocks commit/push?** No.

**Blocks NC-S4b-09 closure?** The NC remains open until the closure authority accepts this review and dispositions the NC; this finding records that state, it does not add a new blocking condition.

---

### SYS-R1/R3-006 — Smoke-gate PASS is conditional on third-party dependencies being fetched (Minor, observation; pre-existing, not introduced by this fix)

**Description.** `make smoke` (and therefore `run_sim.sh`'s build step) depends on `pus-asn1` → `fetch-third-party` (`Makefile:43`, `143-144`). In this environment the build stopped on missing `third_party/n7s-obcp/micropython` files (micropython `src/flat/*.h` absent), a pre-existing third-party fetch gap unrelated to `run_sim.sh`. This is not a regression from the generator fix; it is an environment precondition that existed before and after the fix.

**Classification:** Minor (observation; pre-existing; not introduced by the fix).

**File:line:** `Makefile:43` (smoke: pus-asn1); `Makefile:143-144` (pus-asn1: fetch-third-party); `tools/scripts/run_sim.sh:24-25` (cmake reconfigure/build).

**Blocks commit/push?** No — environment precondition, not a `run_sim.sh` defect.

**Blocks NC-S4b-09 closure?** No.

---

## 4. Cross-disciplinary consistency statement

No interface, requirement, or traceability entry is added, changed, or deleted by this fix. The change is host-only build tooling. All dependent disciplines (FSW, PAL/watchdog, hardware/device, safety/dependability, security, EMC/environment/radiation/thermal/mechanical, V&V/lab, PA/quality/CM, certification/customer) observe zero delta. No ICD, SRS, SSS, traceability.yaml, tool-qualification, or safety-analysis update is required.

---

## 5. Commit/push and NC-S4b-09 closure determination

| Question | Determination | Rationale |
|---|---|---|
| Does the fix introduce system-level risk? | **No** | Host-only CMake generator selection; no flight function/interface touched (`tools/scripts/run_sim.sh:19-25`). |
| Are NC-PAL-01 / GAP-A5 / target/hardware affected? | **No** | Confirmed by repository search; NC-PAL-01 remains OPEN and authority-owned (`src/core/pal/pal_rtems.c:34`, `pal.h:123`). |
| Are GNC / safety / FDIR / operations impacted? | **No** | No SRS/SSS/ICD/traceability entry references `run_sim.sh`; no watchdog/FDIR/GNC/safety change. |
| Is a traceability/requirements change needed? | **No** | Change confined to build tooling; no doc change required (`tools/scripts/run_sim.sh:20-24`). |
| Does anything block commit/push? | **No** | All five findings are Minor/informational; none is a blocking system-level condition. Commit/push remains subject to the separately applicable N7/N15 gates and authorized-agent authority (not decided here). |
| Does anything block NC-S4b-09 closure? | **No new blocker** | Fix committed; smoke PASS (context-verified); NC remains OPEN pending independent review acceptance + authority disposition — this report is the review input, not the closure decision. |

---

## 6. Reviewer note

This is an R1/R3 independent avionics systems review. It records evidence and findings; it does not close NC-S4b-09, does not authorize commit/push, and does not waive any gate. NC-PAL-01, GAP-A5, NC-EXT-19/90, NC-EXT-239, and other authority-owned items remain OPEN and unaffected by this review.

**Report ID:** SYS-REVIEW-R1-R3-20261111-run_sim-sh-NC-S4b-09  
**Retain in:** `docs/reviews/` (norm N11)
