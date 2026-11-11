# NC-EXT-98 Residual Evidence — Ordered Reset-Path + Ordering Sensor
**Status:** OPEN — bounded host-evidence recording only (no target timing claim, no qualification claim)
**Date:** 2026-10-31 (per sprint-plan 1.115; evidence recorded same cycle as V&V PA rounds)
**Bounded scope:** Host-only (simulator/container); bounded reset-path test fixture; pre-reset ordering sensor (`FF_PalResetRequest` before failure report, per S5 pre-reset requirement from audit §7 step 5 / sprint-plan S8-FU scope).
**Independent review context:** Recorded during exact-final independent PA session `ses_e6f78c046ffedyWFWi73rdhbt8` (Task B conclusions 4-5, verified in review index `docs/reviews/README.md` line 6 (session `ses_e6f78c046ffedyWFWi73fdhbt8` for PA study review); audit artifact `docs/reviews/2026-10-31-obcp-study-audit-pa-0440.md` records Task A session `ses_e6fd3f841ffe0ogCcaql4KxBLT` — provenance discrepancy noted, not fabricated). Bounded PA confirm session identity verification pending — no review artifact at HEAD for the bounded PA confirm session. Study comparison posture unchanged (comparison-only; no adoption; M12 DECISION-FOR-NICOLA open).
**Evidence retention (N11):** This file is retained with the complete compliant change cycle (commit `f6e5be4`, both remotes verified at `f6e5be4cfe2d464646abb2ade0b986b24621a18e`).
**Content (bounded, not a claim):**
- Reset-path ordering fixture: host-based simulation of ordered reset sequence (`FF_PalResetRequest` issued before failure report emission).
- Ordering sensor assertion: pre-reset ordering verified by test harness observation; no target hardware timing verified.
**Limitations:** No target timing; no qualification; no release/gate approval; N18 FAIL remains (NC-EXT-198); independent PA/N20 re-review of study (G1) still open.
**Model/provider context:** Inking FREE OpenRouter (replacement candidate per supervisor directive; not Nemotron 3.5 Lightning/Ultra; GLM quota exhaustion claim unverified by current provider-state timestamp).
**Next:** Evidence referenced in sprint-plan/register after bounded verification cycle; no further mutation required without concrete corrective reason.
