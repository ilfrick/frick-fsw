# Escalations and Owner Decisions (open)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-ESCAL-001                        |
| Revision                          | 1.7                                   |
| Date                              | 2026-10-21                             |
| Norms                             | N5 (criticality/risk inputs), N7 (no self-approved gate), N12, N20 |
| Status of every item below        | Owner responses recorded 2026-10-21; implementation/verification and separate gates remain open |

This file exists because the NC register asserted that safety-relevant findings
were "escalated to the authorized human owner" while no escalation record with a
recipient, date, question set and response existed anywhere in the repository
(FRICK-REVIEW-0406 F-06 / NC-EXT-172). Recording a question here does **not**
decide it, close the NC, accept risk, or authorize any work, release or commit.

The FSW engineer may not: set safety objectives or acceptable risk, assign
ecss criticality/DAL, approve tailoring, waive a requirement, authorize a gate,
or direct irreversible operational action. Items listed here are questions for
the authority named, not recommendations of acceptance.

## 1. Owner responses received (not gate decisions)

On 2026-10-21 at 15:49, the project owner responded via Telegram 1317/1318 and
authorized proceeding with the bounded first implementation increment. These
are engineering-scope decisions only: they do not close NCs, approve risk,
accept safety behavior, pass a verification/review gate, or authorize
commit/push. The 8 s watchdog timeout remains an arbitrary numeric placeholder,
not a mission-justified or validated value.

| ESC | Recorded decision | Implementation/evidence status |
|---|---|---|
| ESC-1 | Bound console + UART panic reporting; hardware watchdog is independent of the CPU and handles a total CPU hang. | Shared 250 ms panic-report deadline is authorized; not yet implemented/verified end-to-end. |
| ESC-2 | Propagate real arm/feed errors. No flight development/degraded mode, test bypass or interlock; emulate externally at SUT hardware/PAL interfaces. | Arm/feed errors are now propagated by the PAL source delta; error injection and full application behavior tests remain. |
| ESC-3 | First perform a bounded audit/reuse comparison of existing Renode model; no new emulator/model project or image installation. | The pinned board description contains five generic MPFS watchdog models. Owner supplied public Microchip HAL header as an audit lead; it describes five instances/MVRP behavior and illustrative AXI/256 timing but is mutable `main` and not target-datasheet/BSP binding evidence. RTPFS160ZT mapping, clock/reset dependencies and configuration remain open; see ADR-0021. |
| ESC-4 | Retain truthful diagnostic `FF_PalWatchdogCheck` contract; no extra supervisor. | Existing comments/source not yet reconciled in this increment. |
| ESC-5 | SRS-REQ-074 initial design/verification target: 250 ms for the whole pre-reset attempt on one absolute monotonic deadline, including formatting, console/UART write-completion-or-timeout and all retries; mandatory S5 content not truncated; no remote-delivery assumption; failures cannot prevent reset. | Requirement wording updated; implementation and timing/fault tests are incomplete. |
| ESC-6 | Use absolute monotonic deadlines for feeds with execution/jitter margin and coherent CUC/PPS mapping; do not wait against steppable CUC. Preserve actual inter-feed ≤ T/4; audit earliest valid watchdog refresh window before selecting cadence. | PAL absolute-deadline wait added; full feed-loop timing margins/PPS-fault tests remain. |
| ESC-7 | 8 s is arbitrary placeholder, no mission justification. | Recorded in ADR-0021; no threshold ratification/acceptance implied. |
| ESC-8 | Existing gates remain unchanged. | No gate/NC closure or commit/push authorization. |

## 1b. Owner decisions received 2026-10-21 (later session)

| ESC | Recorded decision | Implementation/evidence status |
|---|---|---|
| ESC-9 | **Path A authorized** (owner Nicola, Telegram 223694038, message 1406, 2026-10-21): wire the MPFS/MSS watchdog, **starting with a controlled specification for owner approval before any implementation or register write**. The scope of that session is documentation, research and review only. **This authorization is procedural only** — it approves no hardware parameter, counter value, timeout, instance, cadence or margin, and it does not authorize the binding itself. A second message in the same exchange (1407) concerned the review/agent model to use; **it is not a further watchdog approval** and carries no parameter. | Recorded as FRICK-ARCH-0024 (specification rev 1.5) with its reference inventory, verified facts, recommendations, the grouped owner input register (G-A..G-F) and the owner decision agenda (A, B1/B2, C, D, E, F — all OPEN). Independently reviewed: FRICK-REVIEW-0424/0425 for rev 1.0, FRICK-REVIEW-0426/0427 for the final delta, and the dated **confirmation addenda FRICK-REVIEW-0428 (embedded safety) and 0429 (V&V)** for the corrected body (rev 1.4), whose findings are itemised against the corrections in rev 1.5. The issuer's official documents were retrieved in this phase and **supersede the rev 1.3 instance recommendation** (only WDOG0/E51 can reset the MSS); **no parameter, clock, timeout, instance or reset policy is adopted**, and every owner decision remains OPEN. **No binding, no register write, no production change.** The controlled target documents remain an owner input (M-1). |
| ESC-10 | **Operating without a watchdog is rejected.** The viable alternative to implementing the binding is to keep the N18 gate blocked and FAIL until the controlled hardware inputs exist — not a degraded or "capability absent => non-fatal" mode. | The earlier proposal variant offering a non-fatal unimplemented-binding mode is **withdrawn** (FRICK-ARCH-0023 rev 1.1/1.2; NC-EXT-198 option (c) corrected). The fatal arm behaviour is unchanged. N18 remains FAIL (marker absent, the target aborts at start-up). |

> **Evidence limitation (NC-EXT-172, OPEN):** the question set, receiving
> authority and response of ESC-9/ESC-10 are recorded here from the owner's own
> report in the task channel; the underlying message set is not retained in this
> repository, so the same "asserted, not evidenced" limitation that
> NC-EXT-172 registers for the earlier escalations continues to apply.

## 2. Open escalations

| ID | Question for the owner | Raised by | Receiving authority | Status |
|---|---|---|---|---|
| ESC-1 | **Blocking pre-reset path (NC-EXT-168, Blocker).** | PA FRICK-REVIEW-0406 F-01 | Authorized human owner (mission/safety authority with FSW) | **DECISION RECEIVED; implementation/evidence open** |
| ESC-2 | **Watchdog arm result and the "WD armed" statement (NC-EXT-169, PA F-02 / R2 C-02).** | R2 FRICK-REVIEW-0405 C-02; PA FRICK-REVIEW-0406 F-02 | Authorized human owner | **DECISION RECEIVED; tests/disposition open** |
| ESC-3 | **Hardware watchdog binding (NC-PAL-01).** The MPFS MSS watchdog binding remains deliberately unwired (`ff_pal_wd_hw_arm/_feed/_reset` return `FF_NOT_IMPLEMENTED`). | NC-PAL-01 | Owner + avionics authority | **AUDIT AUTHORIZED; compatibility decision remains open** |
| ESC-4 | **Dead deadline-check layer (NC-EXT-129).** `FF_PalWatchdogCheck()` has no production call site although `pal.h` comments imply feeder use. | R2 FRICK-REVIEW-0405 C-01 | Authorized human owner | **DECISION RECEIVED; truthful contract delta pending** |
| ESC-5 | **Pre-reset timing bound (NC-EXT-123).** | R2 FRICK-REVIEW-0405 M-03; R3 | Requirements authority | **250 ms initial requirement/design target received; implementation and validation open** |
| ESC-6 | **Feed-period semantics (NC-EXT-166, PA F-03).** | PA FRICK-REVIEW-0406 F-03 | Requirements authority + FSW | **Absolute monotonic deadline decision received; earliest valid hardware refresh window remains to audit** |
| ESC-7 | **8 s timeout / 2 s feed basis.** ADR-0021's 8 s default has no mission basis. | R3 (requirements/system); ADR-0021 | Owner + systems/safety authority | **Placeholder status received; mission justification/independence evidence open** |
| ESC-8 | **Commit/push authorization (N7).** | this cycle | Owner | **No commit/push decision; existing gates remain unchanged** |
| ESC-HK-01 | **PUS `TC[3,1]` wire format (NC-EXT-17).** The implemented housekeeping-report-create payload is 6 octets (`report_id`, `provider_id`, `period_s[4]`); `docs/interfaces/ICD.md` §4 defines 2 octets. Which form is the controlled interface, and may the periodic-period field exist at all? No ECSS compliance is claimed for the implemented private extension. | Independent V&V `FRICK-REVIEW-HK` 2026-10-24; interface authority item | Interface authority + requirements authority | **AWAITING OWNER** |
| ESC-HK-02 | **Periodic HK activation/caller.** `FF_PusEmitPeriodicHk()` has no scheduler or application caller, so periodic generation is inert in the flight build. Which component owns the cadence, at what priority, and is a missed-period or send-failure policy required? Note: PA records the wrap-unsafe comparison (PA-HK-F01) as a **blocker for activation**. | Independent V&V/PA delta reviews 2026-10-24 | Systems/architecture authority + owner | **AWAITING OWNER** |
| ESC-HK-03 | **Hazard analysis for periodic HK emission.** None exists. Measured hazards from the 2026-10-24 reviews are `H_SW_CADENCE_HALVED` (double-count defect, now fixed: sustained 50 % HK sample-rate loss), `H_SW_TM_FLOOD` (wrap-unsafe comparison plus unvalidated `period_s`), `H_SW_PREMATURE_EMIT`, and the create/enable publish-before-init concurrency window. A formal hazard analysis and FDIR linkage are required. | PA final delta review 2026-10-24 | Safety authority | **AWAITING OWNER** |
| ESC-HK-04 | **Ground-segment interpretation risk of the private HK wire format.** A ground station built to ICD §4 cannot parse the 6-octet form; a mismatched period is not detectable on the wire. | PA final delta review 2026-10-24 | Interface authority + operations | **AWAITING OWNER** |
| ESC-HK-05 | **Requirements basis for periodic HK emission.** No SRS/SSS requirement and no `traceability.yaml` entry exist for periodic emission, so no coverage or verification claim is possible. | Independent V&V delta reviews 2026-10-24 | Requirements authority | **AWAITING OWNER** |
| ESC-HK-06 | **Time-base contract for periodic emission.** The CUC correlation epoch is undefined, the 32-bit coarse wrap representation and behaviour are unspecified, and permitted backward steps/leap handling, the maximum permissible `period_s`, and the missed-period/send-failure policy are all undecided. `period_s` is currently accepted unvalidated. | PA final delta review 2026-10-24 | Systems/time authority + requirements authority | **AWAITING OWNER** |

## 3. What this document does not do

It does not accept risk, close an NC, waive a gate, approve a phase, qualify a
tool or item, certify anything, or authorize commit/push or flight use. When the
owner answers an item, the answer must be recorded here with the date, the
authority, and the resulting register/ADR change, and then verified under N12.

## History

| Revision | Date       | Author    | Change |
| -------- | ---------- | --------- | ------ |
| 1.0      | 2026-10-21 | frick-fsw | Created to satisfy PA FRICK-REVIEW-0406 F-06 / NC-EXT-172: records the open safety and requirement escalations with their receiving authority, all AWAITING OWNER |
| 1.1      | 2026-10-21 | frick-fsw | Record owner responses Telegram 1317/1318 (15:49), accepted bounded increment and the 250 ms local attempt target; distinguish decisions from implementation, NC closure and gate status |
| 1.2      | 2026-10-22 | frick-fsw | Record the owner's path-A authorization (ESC-9) and the rejection of operating without a watchdog (ESC-10), with the NC-EXT-172 evidence limitation stated; the implementation remains behind the controlled specification FRICK-ARCH-0024 |
| 1.3      | 2026-10-22 | frick-fsw | Sharpen the ESC-9/ESC-10 wording: the path-A authorization is procedural (specification first) and approves no hardware parameter; the second message of the exchange (1407) is a model/review request, not a further watchdog approval |
| 1.7      | 2026-10-24 | frick-fsw | Register ESC-HK-01..06 for the S8-PUS-FU HK periodic-emission increment (NC-EXT-17), closing the PA-HK-F03 gap where those escalations existed only inside review reports. Each item states its receiving authority and is AWAITING OWNER: this record decides nothing, closes no NC and authorizes no activation, release or commit/push |
