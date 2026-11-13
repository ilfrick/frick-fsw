# Independent Avionics Systems Review — R1/R3 profile

**Repository:** frick-fsw (branch `main`, HEAD b6bf060)
**Target:** ECSS class B; Microchip RT PolarFire SoC RTPFS160ZT (RISC-V) under RTEMS 6.2
**Change under review:** `src/pus/pus_services.c` — reorder `FF_TimeGetCuc()` before `g_hk_reports[i].enabled = true;` in `svc3_create` and `svc3_enable`
**Reviewer role:** Senior Aeronautics and Space Avionics Systems Engineer (independent review, R1/R3 profile)
**Review date:** 2026-11-12
**Authority statement:** This review is engineering advice only. It does not approve airworthiness, human-rating, safety risk, qualification, release, deviation, waiver, flight readiness, or mission operations. Authorized humans retain all safety-critical and regulatory decisions.

---

## 1. Change summary (verified against working tree)

Only `src/pus/pus_services.c` is modified. The diff reorders two initializations so the current CUC time is captured **before** the report is marked enabled:

| Function | Before (old order) | After (new order) | Lines |
|---|---|---|---|
| `svc3_create` | `enabled = true; period_s = period_s;` then `FF_TimeGetCuc(...)` | `FF_TimeGetCuc(...)` then `enabled = true; period_s = period_s;` | 233–239 |
| `svc3_enable` | `enabled = true;` then `FF_TimeGetCuc(...)` | `FF_TimeGetCuc(...)` then `enabled = true;` | 270–274 |

**Root cause (per change author):** `g_hk_reports` is zero-initialized (`memset`, line 680). `FF_PusEmitPeriodicHk()` (line 429) reads `report->enabled` (guard line 440) and then `report->last_emission_coarse/fine` to compute `time_period_due()` (line 408). If a concurrent call observed `enabled == true` while `last_emission` was still `(0,0)`, then `u = now - 0 = now` would likely exceed `period_s`, causing an immediate spurious HK emission.

**No public API, interface, GNC, safety, FDIR, operations, hardware, or target change.** Single source file only.

---

## 2. Technical assessment

### 2.1 The fix is sound and closes the described race

The writer now publishes `last_emission_coarse/fine` (lines 236–237 / 272–273) before publishing `enabled = true` (lines 238 / 274). A concurrent reader that observes `enabled == true` will therefore also observe a non-zero, current `last_emission`, so `time_period_due()` computes a sane modular interval instead of `now - 0`.

The reader-side guard `report->period_s == 0u` (line 440) plus zero-initialization (line 680) additionally protect the `svc3_create` window where `period_s` is still written after `enabled` (line 239): a concurrent reader sees `period_s == 0` and skips. This residual ordering is benign.

### 2.2 No system-level risk is introduced

- No public API/interface signature changes.
- No ICD, requirements, or traceability change is required for an internal ordering fix.
- Build/test/Lean/trace/format/license are reported by the change author as passing (exit 0). This review does **not** re-execute gates and does not approve verification results (persona boundary §15).
- No NC-PAL-01, GAP-A5, target/hardware, OBCP-engine, PAL, or NC-EXT-19/90/N22-TS5-47 item is touched.

---

## 3. Findings

| ID | Classification | File:line | Blocks commit/push? | Finding |
|---|---|---|---|---|
| SYS-R1/R3-001 | Minor (favorable) | `src/pus/pus_services.c:236-238, 272-274` | No | The reorder correctly closes the `enabled=true` / stale `last_emission=(0,0)` race that could cause a spurious immediate periodic HK emission in `FF_PusEmitPeriodicHk()`. No system-level risk introduced. |
| SYS-R1/R3-002 | Minor | `src/pus/pus_services.c:238-239` (create), `:440` (reader guard), `:680` (zero-init) | No | Residual nuance: in `svc3_create`, `period_s` is still written after `enabled=true`. This is benign because the struct is zero-initialized and the reader skips `period_s == 0`. Documented for completeness; no interface change. |
| SYS-R1/R3-003 | **Major** | `src/pus/pus_services.c:235, 271` (comments cite `NC-EXT-211`); `docs/planning/sprint-plan.md:338` | **Yes — until NC-EXT-211 is dispositioned** | `NC-EXT-211` is defined in `sprint-plan.md:338` as exactly this latent-concurrency ordering NC. It is tracked **OPEN / DEFERRED under N21**, authority-owned, and per sprint-plan requires "concurrency/architecture agreement" plus independent V&V before activation (`ESC-HK-02`). Existing PA/independent reviews (FRICK-REVIEW-0440/0442/0451/0457) repeatedly state `NC-EXT-207/208/211` remain OPEN and "must not be read as progress on NC-EXT-211."<br/><br/>This change's code comments cite `NC-EXT-211`, which may be read as claiming closure/progress. Under N9 (independent review), N12 (NCs closed only by re-applying all rules to full compliance), and N20 (PA independent review), an NC is **not closed by an author's fix alone** — it requires authority agreement and independent V&V sign-off.<br/><br/>Disposition options: (a) route this change through independent review (R1/R3), PA (N20), and V&V (N10/N18 — concurrency/timing fix warrants emulated-target/Renode evidence), obtain authority agreement on the ordering fix and/or a single-threaded-access claim, then close `NC-EXT-211`; or (b) remove the `NC-EXT-211` closure claim/citation from the change and frame it as a defensive ordering improvement that does **not** close `NC-EXT-211`, leaving the NC open and authority-owned. Either way, committing while representing `NC-EXT-211` as resolved without the required disposition violates NC discipline and blocks commit/push. |
| SYS-R1/R3-004 | Minor (confirmatory) | `src/pus/pus_services.c` only (full file) | No | Confirmed: no impact on NC-PAL-01, GAP-A5, target/hardware, PAL, OBCP engine, NC-EXT-19/90, or N22-TS5-47. No GNC, safety, FDIR, or operations interface impact. No requirements/traceability change needed. |

---

## 4. Cross-disciplinary consistency

- **Interfaces/ICD:** none changed; no ICD or traceability (`docs/requirements/traceability.yaml`) update required.
- **FSW:** internal to `pus_services.c`; `FF_PusEmitPeriodicHk()` behavior unchanged except the race is removed.
- **Safety/FDIR:** no hazard-class, DAL, criticality, or failure-hypothesis change; no FDIR interface impact.
- **GNC/operations:** no command/telemetry frame, period, or provider interface change.
- **Hardware/target:** source-only, host-executable per change scope; no part, radiation, derating, or device-tree impact.
- **PA/CM:** single-file change; configuration identification unaffected except the `NC-EXT-211` status question in finding SYS-R1/R3-003.

---

## 5. Systems conclusion and recommendation

The **technical fix is correct and low risk**; it introduces no system-level hazard and no interface change. The only item that blocks commit/push is **SYS-R1/R3-003** (Major): the `NC-EXT-211` configuration-management disposition.

**Recommended path before commit/push:**
1. Decide whether this change is represented as closing `NC-EXT-211` (option a) or as defensive-only leaving it open (option b).
2. If (a): obtain concurrency/architecture agreement, independent V&V evidence (Renode/emulated-target, norm N18), PA review (N20), and authority agreement; then close `NC-EXT-211` and update `sprint-plan.md`.
3. If (b): remove/clarify the `NC-EXT-211` citation so the change does not claim closure of an open, authority-owned NC.
4. Re-run the full applicable cycle (build, test, Lean, trace, format, license) per N15 and record evidence in `docs/vv/verification-results.md` (N11) before commit/push.

Until SYS-R1/R3-003 is dispositioned, **this change must not be committed or pushed** if it is represented as resolving `NC-EXT-211`.

---

*End of report. English. Findings classified Major/Minor with file:line references and commit/push blocking status.*
