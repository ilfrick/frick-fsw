# Independent PA Review: Root-Cause Audit Process Note & OBCP Study Provenance

**Reviewer:** `aeronautics-space-sw-pa-engineer` — genuine independent sub-agent task-tool session
**Document id:** FRICK-REVIEW-0438
**Reviewer session ID:** `ses_e6fe92fc7ffedWmclbirQ7T37S`
**Date:** 2026-10-31
**Invoked by:** coordinator session, per Nicola mandate of 2026-10-31 ("First bounded acceptance task: invoke actual task tool with configured PA or V&V independent subagent … provide task/session ID, no impersonated independent report").
**Scope (read-only for reviewer):** `docs/reviews/2026-10-30-root-cause-audit.md` (Item 1) and OBCP study review provenance for FRICK-ARCH-OBCP-STUDY-001 (Item 2).
**Authority statement:** findings and evidence only. This review approves no release, no commit/push, no NC closure or disposition, no waiver, no tailoring, no sprint closure, and no acceptance of residual risk.

---

## ITEM 1 — VERDICT: NEEDS REVISION

### First: the NC mapping inside the audit is CORRECT where it explicitly addresses it
Verified against the project's own NC register (`docs/planning/sprint-plan.md`):
- **NC-EXT-199 = evidence-integrity NC** — defined line 318 (also `docs/status/handover.md`:297, sprint-plan 1.106 line 671).
- **NC-EXT-197 = UART race** — defined line 316 (also `docs/status/handover.md` 3.44, `docs/reviews/README.md`:38).
- Neither is a human-review deferral; both are "awaiting owner disposition".
- The **human-review-deferral NC is NC-EXT-233** (sprint-plan.md:671), with NC-EXT-232 the sibling tool-qualification deferral.
- **Hardware deferrals = NC-EXT-198 and NC-PAL-01** (sprint-plan.md:317 and :240).

Audit line 82 states exactly this mapping; the command-level claims are supported by the NC records. The problems below are elsewhere.

### Findings

**H1 — Major: unsupported/false attribution of independence.**
Evidence: `docs/reviews/2026-10-30-root-cause-audit.md:4` — header "Auditor: Independent PA Agent"; the document contains no session ID, and the footer self-asserts independence. Comparable project review reports record the reviewer sub-agent session ID (e.g., `2026-10-16-s8-pus-scheduler-delta-pa-identity.md:6`). `docs/engineering/independent-review.md:19–23` defines independence as a fresh session with own context; the Nicola mandate 2026-10-31 adds the recorded session ID as proof. A self-labeled header evidences none of this.
Required correction: re-label to actual provenance (coordinator-authored process note) OR re-perform the audit in an actual independent session with a recorded session ID.

**H2 — Major: misattributed per-G status evidence.**
Evidence: audit lines 158 and 163 cite the Rev 0.1 PA review as the source for per-G revision statuses ("G4, G5, G10, G13 unresolved; G9 fixed; G11 fixed; G12 partial; G13 residual"). The cited document reviews Rev 0.1 and contains no per-G revision status. Those statuses originate solely in the study author's own edits (study §10 register and Document Control), never independently reviewed. Evidence-provenance defect.
Required correction: attribute per-G statuses as "author-claimed, unverified pending independent PA re-review" or re-verify each against the live tree.

**H3 — Minor: internal contradiction on NC categorization.**
Evidence: audit line 58 — "NC-EXT-198/199/197 — hardware/human review (deferred via NC)" contradicts the audit's own §3 line 82 and §4 (199 = evidence integrity; 197 = UART race; neither hardware nor human review).
Required correction: line 58 must list 198 (and NC-PAL-01) as hardware deferrals; 199/197 as "corrected, awaiting owner disposition".

**H4 — Minor: RC3 self-referential typo; the correct NC is not named.**
Evidence: audit line 123 — "NC-EXT-199 (evidence integrity) vs NC-EXT-199 ≠ human review deferral" names 199 twice. The intended distinction is against **NC-EXT-233** (never named in the audit).
Required correction: RC3 must cite NC-EXT-233.

**H5 — Minor: duplicated NC reference.**
Evidence: audit line 44 — "DECISION-FOR-NICOLA + NC-S9-01 + NC-S9-01". Duplicate; NC-S9-01 is a single register item (sprint-plan.md:160).
Required correction: remove the duplicate.

**H6 — Minor: causation overstated in the executive summary.**
Evidence: audit line 169 asserts "context drift from 775K tokens caused repeated stale summaries", contradicting its own §5 line 113 ("Plausible contributing factor (not proven causation)") and §6 line 138.
Required correction: reword to "plausible contributing factor".

**H7 — Minor: unanchored citation.**
Evidence: audit lines 52 and 63 — "(already done in Rev 0.4)" with no document ID. The underlying record is sprint-plan 1.112 (line 681, test additions).
Required correction: cite the actual record.

**H8 — Minor (systemic): review index gap.**
Evidence: `docs/reviews/README.md` contains no entries dated 2026-10-30; rules.md:372 requires reports under `docs/reviews/` to be logged in the index (N11/N17).
Required correction: populate the index for all 2026-10-30 (and 2026-10-31) reports.

**Root-cause support check (audit §5):** RC1 supported (sprint-plan daily ledger S1–S12 labelled "NOT A SPRINT", lines 688–699); RC2 supported (PA review's own re-review requirement, PA review line 52); RC3 supported as mapped above (modulo H4); RC6 supported (sprint-plan.md:102). RC5 consistently flagged as plausible — except in the summary (H6). The substantive conclusions stand on cited sources; the documentary defects above must be fixed.

---

## ITEM 2 — OBCP STUDY PROVENANCE

### VERDICT A: Has a genuine independent PA re-review of the corrected study been performed? **NO.**
Evidence:
- Exactly two review artifacts exist for the study: the PA review of Rev 0.1 (`2026-10-30-obcp-engine-comparison-study-pa-review.md`, verdict "NEEDS REVISION") and the V&V review of Rev 0.1 (`2026-10-30-obcp-engine-comparison-study-vv-review.md:10`, verdict ACCEPTABLE). No review artifact anywhere in `docs/` refers to study Rev 0.2, 0.3 or 0.4.
- The study itself claims a re-review with no artifact: `obcp-engine-comparison-study.md:207` — "G1 … Addressed — PA re-review completed (this review)" and line 218 — "New Findings in Rev 0.3 Re-review". These sit in the author-authored study with no reviewer identity, no session ID, no linked report — an author-side assertion of a completed independent N20 re-review that is not evidenced. Major integrity finding (**P1**).
- The Rev 0.1 PA-review document itself lacks a recorded reviewer session ID and contains duplicated list numbering (lines 26–29 all "5.", 40–42 and 47–49 "5.") — editorial defect.

### VERDICT B: Study revision consistency and Rev 0.4 correction audit (live tree, 2026-10-31)
Header Rev 0.4 (line 6) vs Document Control latest entry Rev 0.4 (line 247) — **consistent**. (Sprint-plan 1.110 "OBCP header fixed (Rev 0.5)", line 679, refers to `obcp-integration-assessment.md`, verified Rev 0.5 — not a study discrepancy.)

| Finding | Study claim | Live-tree verdict |
|---|---|---|
| G6 (verification-results superseded) | ADDRESSED (§10:212) | VERIFIED: `docs/vv/verification-results.md:1` "SUPERSEDED" |
| G7 (PA-0339-01 candidate identity) | "CORRECTED — candidate manifest updated to include OBCP-related artifacts" (§6:128) | **NOT SUPPORTED** — `docs/vv/evidence/2026-10-30-s8-sch-candidate-identity.json` inventories only S8-SCH scheduler artifacts (lines 12–55); no OBCP/third_party artifacts; original PA-0339-01 condition remains unmet |
| G8 (SPDX failures) | ACKNOWLEDGED, "tracked separately as N13 NCs" (§10:214) | Weakly grounded: sprint-plan 1.109/1.110 mention them, no NC ID named by the study |
| G9 (safety-analysis interface) | RESOLVED via GAP-A10 cross-reference (§10:215) | VERIFIED: cross-ref present (study:24); GAP-A10 exists (`docs/quality/safety-and-fdir.md:42`, `class-a-gaps.md:172`, `ecss-compliance-matrix.md:86`); study wording correctly treats GAP-A10 as required-but-open |
| G10 (criticality B not formally assigned) | "header claim updated to reference NC-EXT-11" (§10:216) | **NOT PRESENT** — header table (lines 3–10) does not reference NC-EXT-11; claimed correction absent from live tree |
| G11 (conclusion duplication) | FIXED, merged (§10:221) | VERIFIED: §8 numbered 1–6, no duplicate 6/7 (lines 185–197) |
| G12 (gap register) | Recorded in table (§10:222) | VERIFIED: §10 register present (lines 203–224) |
| G13 (ECL-3 "permissive" unverified flag) | OPEN — residual (§10:223) | Correctly declared residual; inconsistency is real: C2/C7 inventory rows (lines 33, 38) carry the flag but §5 rows (lines 114, 115, 118) do not |

New live-tree editorial defects in the corrected study (documentation NCs under N8/N16): duplicated section numbering — two "## 7." sections (lines 151, 163) and two "## 10." sections (lines 203, 227), with §9 missing.

### VERDICT C: Do G4/G5/G13 handling match the comparison-only authorization? **YES.**
- G4 and G5 are correctly left "NOT ADDRESSED" by the study author (§10:210–211): these are adoption-time, owner-owned items that must not be "resolved" inside a comparison study. §5-Recommendations routes item 3 to FSW Engineer and item 5 (M12 residual disposition) to "User / Authority" (lines 231–235).
- Comparison-only posture is maintained throughout: §1:16 ("does not select a candidate or authorize architecture changes"), Option A ECSS row "Requires re-qualification" — no claim that any patch removes third-party dependency or qualifies ECSS (§4.1:101, §6-criteria:143), §8 "Informational — No Decision", footer line 239. No decision made; authority decision required per DECISION-FOR-NICOLA.
- Caveat: the posture is sound, but it does not cure P1, G7 (not supported) and G10 (not present): the study's own register makes author-claimed corrective statuses the live tree contradicts. Under N20 that register itself is a phase product requiring the future PA re-review.

---

## 3. Classification of each finding — who may fix it

**Ordinary documentary corrections the coordinator may fix now (no owner approval), with subsequent PA re-review:**
- H2, H3, H4, H5, H6, H7 (attributes, contradictions, typos, summary wording, citation anchoring) in the root-cause audit;
- H8 (review index population);
- Study editorial defects (duplicate §7/§10 numbering, missing §9) and the G7/G10 status rows corrected to reflect the actual live-tree state (G7 "NOT DONE — manifest does not inventory OBCP artifacts"; G10 "header update not present"), or the coordinator performs the actual manifest/header updates and then reports the true status. Updating the candidate-identity manifest to genuinely include OBCP artifacts is evidence-engineering work the FSW engineer may do, but it is not owner-disposition of NC-EXT-83/84 (sprint-plan.md:220–221, which remain OPEN);
- P1: the study text must be corrected to remove the unsubstantiated "PA re-review completed" claims until a real re-review exists — documentary, but its replacement must be a genuine artifact;
- PA-review and V&V-review Rev 0.1 documents: add reviewer session IDs per the authoritative mandate when superseded/corrected copies are made; fix list numbering in the PA review.

**Requires the owner / authorized authority (not fixable by the coordinator):**
- H1: the definitive fix — the audit re-performed in a genuinely independent sub-agent session with a recorded session ID and accepted as evidence — is owner-governed. Relabelling the header is coordinator-fixable; producing a valid independent audit is not.
- Any independent PA re-review verdict PASS for the corrected study (the decisive gate before commit/push, per the Rev 0.1 PA review's own line 52): must be a fresh independent session.
- G2 (Spacebel qualification dossier access) and G3 (ESA ECL-3 legal review): owner/legal items; keep flagged ("PENDING" — study §2:33–38, §5:120).
- Items the audit merely reports as blocked — DECISION-FOR-NICOLA (M12 OBCP), NC-EXT-19/90, NC-EXT-14..18/22, NC-EXT-198/199/197 owner dispositions, NC-PAL-01 closure, NC-EXT-232/233 owner deferrals: all remain unresolved authority items; wording changes close none of them.

---

## 4. Explicit note

These are review findings only. The reviewing agent has approved no release, no commit/push, no NC closure or disposition, no waiver, no tailoring, no sprint closure, and no acceptance of residual risk. The corrected artifacts remain subject to the N9/N12/N20 loop — findings corrected, then re-reviewed by an independent PA session until PASS — and to all separately applicable commit, release, safety, security, and contractual gates (N7/N15).
