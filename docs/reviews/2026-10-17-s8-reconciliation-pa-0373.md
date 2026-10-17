# Software Product Assurance assessment — S8 reconciliation delta (bounded)

**Review ID:** FRICK-REVIEW-0373  
**Profile:** PA (independent, norm N20)  
**Date:** 2026-10-17  
**Phase boundary:** N20 product assurance only (not approval, not certification, not release)  
**Assessor:** aeronautics-space-sw-pa-engineer (independent)

## 1. Scope and artifacts inspected

This bounded assessment covers the reconciliation delta specified: sprint-plan rev 1.68 (narrow closures recorded for NC-EXT-88/89/23/24; NC-EXT-17 bounded; new NC-EXT-92 registered), docs/vv/test-spec/ts-1-protocol-core.md rev 1.8, docs/vv/test-spec/ts-5-core-services.md rev 1.2, docs/status/loop-audit.md rev 10.6. Evidence linkage and identity: docs/vv/evidence/ manifests, 2026-10-16-s8-pus-scheduler-delta-identity.md (19-file), 2026-10-16-s8-current-candidate-manifest.md (49-file), CI-local logs and hashes referenced therein. Review evidence: FRICK-REVIEW-0355 (R1), 0356 (R2), 0354 (PA), 0360 (PA), 0361 (PA), 0364 (PA), and related R1/R4 in the same sequence.

**Artifacts consulted (full paths relative to repo):**
- docs/reviews/README.md (historical index up to 2026-10-16)
- docs/reviews/2026-10-16-s8-pus-r1-review-0349.md (R1); 2026-10-16-s8-pus-r1-rereview-0355.md (R1)
- docs/reviews/2026-10-16-s8-pus-r2-review-0350.md (R2); 2026-10-16-s8-pus-r2-rereview-0356.md (R2)
- docs/reviews/2026-10-16-s8-pus-pa-delta-0354.md (PA)
- docs/reviews/2026-10-16-s8-sched-pa-delta-0360.md (PA)
- docs/reviews/2026-10-16-s8-pus-scheduler-delta-pa-identity.md (PA)
- docs/reviews/2026-10-16-s8-current-candidate-pa-identity-0361.md (PA)
- docs/reviews/2026-10-16-s8-final-ci-pa-rereview-0364.md (PA)
- docs/vv/evidence/2026-10-16-s8-current-candidate-manifest.md (49-file; product digest 1a84475…, 49 paths, excludes status/planning/review/build/evidence)
- docs/vv/evidence/2026-10-16-s8-pus-scheduler-delta-identity.md (19-file; digest 272543e…, narrow delta scope; claim explicitly not complete candidate)
- docs/planning/sprint-plan.md (rev 1.68, NC table and revision history record NC-EXT-88/89/23/24/17/90/91/92; S8-PUS-FU/S8-SCH/S8-VV-FU text)
- docs/vv/test-spec/ts-1-protocol-core.md (rev 1.8), docs/vv/test-spec/ts-5-core-services.md (rev 1.2), docs/vv/test-spec/ts-6-sw-robustness.md, docs/vv/test-spec/ts-4-io-b.md (worktree and baseline b8c3aee), docs/status/loop-audit.md (rev 10.6), docs/vv/verification-results.md (current), docs/status/status.json (meta note), docs/architecture/pus-layer.md, docs/architecture/scheduling.md, ADR-0020, tests/unit/test_pus_fu.c, test_pus_n7_adapter.c, test_sched.c, src/core/sched/ff_sched.{c,h}, src/pus/pus*.c/.h

Baseline comparison used git: b8c3aee. Local worktree (uncommitted deltas) as-is.

## 2. Assessment against required questions

### (a) Reconciliation delta scope vs underlying review evidence; any over-closure or contradictions

**Evidence comparison:**

- 0349 (R1): found `svc3_report` overwrote first failure with `FF_NOT_FOUND`. Required conditional update. **Satisfied.**
- 0355 (R1 re-review, bounded): verifies preservation of first failure (status only set if `FF_SUCCESS`); TS1-102 covers both orders; focused test passes. This matches the sprint-plan description of NC-EXT-88 closure (narrowly). **Consistent.**
- 0350 (R2): gaps: capacity+1 and UINT16_MAX rejection, HK zero/full TM lengths, duplicate-retention oracle scope, SRS-REQ-050 trace mapping. **Addressed in 0356.**
- 0356 (R2 re-review, bounded): confirms boundary rejections produce no TM, exact HK lengths, retained service/HK-provider behavior after duplicate rejection (setter/function claim explicitly not asserted), and test_pus_n7_adapter.c added to trace. **Matches NC-EXT-89 closure claim (narrow).**
- 0354 (PA delta): accepts bounded corrections, notes 43/43 CI-local reported at review time and scheduler delta added afterward (so a fresh full cycle/identity required); keeps NC-EXT-88/89 open pending complete evidence/review and NC-owner disposition. The sprint-plan rev 1.68 states NC-EXT-88/89 are "CLOSED narrowly by the independent R1/R2 delta re-reviews 0355/0356" (no phase/compliance claim) — this is a phrasing tension: PA 0354 treats them as still conditionally open until complete evidence/identity/review, while R1/R2 reports mark bounded correction verified. **Inference (medium confidence):** the bounded functional/test-evidence closure is per R1/R2; PA requires full configuration-matched cycle/identity before closure in PA terms. The sprint-plan text reflects the narrow R1/R2 closure with explicit caveats ("no phase/compliance claim; full-cycle/PA/R4/release gates remain separate"). The description is consistent with caveats; no direct contradiction. **Recommendation:** clarify in NC-EXT-88/89 text whether "closed narrowly" means the defect is corrected/verified by focused reviews, while PA acceptance of closure requires configuration-matched full cycle (per 0354). This is a minor wording/closure-status taxonomy issue (not a safety concern).  
- 0360 (PA S8-SCH): acceptable bounded host evidence for per-entry release targets; NC-EXT-90 remains open (time overflow/exhaustion/mission policy/target timing); NC-EXT-19 remains open (mission schedule/time policy, target timing, TBL activation-to-dispatch, late-slot/recovery). Sprint-plan 1.68 aligns: NC-EXT-90 open, NC-EXT-19 not closed by bounded evidence. **Consistent.**
- 0361 (PA identity, 49-file): verified digest and all per-file hashes match; identity-cycle CI-local 43/43 pass; excluded planning/status/reviews/evidence/build. Open: NC-EXT-84 (generator/tool provenance), NC-EXT-88/89/90 open, hosted CI/release gates. **Consistent with scope limit.**
- 0364 (PA run-linkage): hash linkage to log/cache/executable matches; no signed attestation/provenance/reproducibility. **Consistent.**
- PUS/Scheduler delta identity (19-file): verified, digest matches; PA closed NC-EXT-91 narrowly for that delta inventory only; broader identity/provenance/target/release gates remain open. Sprint-plan 1.68 records this narrow closure. **Consistent.**

**Conclusion (a):** The recorded narrow closures for NC-EXT-23/24 (duplicate rejection, provider length bounds) are supported by R2 0356 (focused oracles and cases). NC-EXT-88 (S3 first-failure) supported by R1 0355. NC-EXT-89 supported by R2 0356. NC-EXT-91 narrow closure supported by 19-file identity. NC-EXT-17 bounded correction recorded under NC-EXT-88 context (first-failure/propagation) while broader profile remains open — consistent. No recorded closure exceeds evidence; taxonomy nuance between R1/R2 "narrowly closed" vs PA's full-cycle acceptance is the only ambiguity (non-blocking, see Recommendation).

### (b) Coverage/verification claim changes without independent review?

Changes observed:
- TS-1 rev 1.8: normalized N22-TS1-103 status token from prose to "host covered (...)"; §3.1 counts already stated 77/26/103; table became internally consistent (103 rows → 77 covered/26 gap). **No oracle/input/expected-result changed.** The token normalization aligns table with summary; this is a classification/wording change. No independent review report (R2/R4) explicitly cites this token normalization, but the case's content (test_hk_standard_subservice_assignments in test_pus_fu.c, ECSS-referenced mapping per ADR-0020, qualifiers unchanged) is not a new oracle claim. **Fact (medium confidence):** classification alignment, not a new coverage claim. No requirement violated, but the change should be traceable; absence of explicit review mention for this trivial token normalization is minor documentation/trace gap (not safety).
- TS-5 rev 1.2: renumbered duplicate ID N22-TS5-73 → N22-TS5-77 (fixed prior ID collision). Added N22-TS5-77 as a covered case for "checked release-target/deadline-exhaustion and missed-counter saturation" (ADR-0020 policy, NC-EXT-90 open). Baseline b8c3aee had 76 rows (55/21); worktree has 77 rows (56/21). **Inference (high confidence):** new covered claim introduced. Underlying test behavior is `test_time_exhaustion_does_not_wrap` in test_sched.c (deadline disabling on unrepresentable initial deadline, no wrap, missed saturates at UINT32_MAX). Was this test/oracle independently reviewed? Reviews 0351-0353, 0360 cover scheduler arithmetic and host behavior (per-entry targets, drift/missed, atomic activation) but do **not** explicitly mention N22-TS5-77 or the exhaustion-saturation test. No review report in the reviewed set cites `test_time_exhaustion_does_not_wrap` as assessed. **Recommendation/Open issue:** new covered claim (TS5-77) lacks an explicit independent review of the new test/oracle (N9). This is a documentation/test-evidence review gap. It does not directly invalidate the bounded scheduler fix but undermines the "independent review of that change" requirement for a coverage claim. **Severity: Minor (process/trace)**; NC-EXT-90 remains open anyway.

- Other TS changes (TS-6 rev 1.4): CRC oracle wording clarified (internal consistency check, not independent oracle); scheduler missed-slot vector aligned; no count change affecting a new requirement claim. **Consistent.**

**Conclusion (b):** The TS-5 addition of a covered case (TS5-77) introduces a new coverage/verification claim without an explicit independent review of that specific test/oracle. TS-1 token normalization is a classification alignment (no new claim). The TS-5 unreviewed new covered claim is an **open issue (process)** and should be tracked (e.g. a minor NC against review closure) even though NC-EXT-90 remains open.

### (c) NC-EXT-92: registration, scope, owner/verifier, and baseline claim

**Registration details (from sprint-plan 1.68, §3):**
- ID: NC-EXT-92
- Source: TS-1/TS-4/TS-5 case-table self-audit (2026-10-16/17 reconciliation)
- Severity: Minor (documentation/coverage-consistency)
- Description: TS-4 §3.2 (metrics summary, §3.1 in TS-4 is the Coverage summary; see below) reports covered=62 and gap=18, but table has 64 rows with covered-style status (62 plain + N22-TS4-75 "covered (host subset)" + N22-TS4-76 "covered (host contract)") and 16 gap rows; §3 additionally lists N22-TS4-75/76 under gaps. Counts disagree by 2.  
- Disposition must decide whether §3.1 summary + §3 gap listing or row statuses are authoritative, reconcile all three, obtain independent R2/R4 review before any coverage number changes. Owner: FSW/test-spec author; sprint: later disposition under N21; verifier: independent R2/R4. No target/qualification/phase claim implied.
- Pre-existing at b8c3aee: "the claim that it pre-exists at b8c3aee? Verify that claim yourself against the committed file (git show b8c3aee:docs/vv/test-spec/ts-4-io-b.md) and report the exact counts you observe at the baseline and in the working tree."

**Independent verification (PA):**
- Baseline (b8c3aee), TS-4-io-b.md: 80 rows total (N22-TS4-1..N22-TS4-80). Row statuses: 62 "covered", 16 "gap", 1 "covered (host subset)" (N22-TS4-75), 1 "covered (host contract)" (N22-TS4-76) → covered-style = 64, gap = 16. Metrics summary (baseline §3.1 Coverage summary, line ~260): Total 80, Nominal 40, OOB 40, Individual 69, Combined 11, `covered` = 62, `gap` = 18, SIL/PIL/HIL 68/8/4. **Confirmed baseline discrepancy: 64 covered-style rows vs 62 claimed covered; 16 gap rows vs 18 claimed gap. Sums: 62+18=80 (summary) vs 64+16=80 (table) — different split, same total.** Yes, pre-existing at b8c3aee.  
- Worktree (current), TS-4-io-b.md: identical to baseline for these counts. 80 rows, 64 covered-style, 16 gap. Summary still 62/18. No change. **Confirmed identical in worktree.**

**Assessment of NC-EXT-92 registration:**
- Adequate source: cites specific section/row numbers and the numeric inconsistency (by 2). References the conflicting elements (summary vs row statuses vs §3 gap listing).  
- Severity rationale: "Minor (documentation/coverage-consistency)". The numeric inconsistency is documentation level; it affects coverage reporting clarity but no oracle or requirement content changed in a way that asserts new coverage. The sprint-plan explicitly states "no target, qualification or phase claim is implied" and owner is FSW/test-spec author. **Appropriate.**  
- Owner: FSW/test-spec author — **appropriate** (document owner).  
- Verifier: independent R2/R4 — **appropriate** given N9 (independent review) for test-spec consistency.  
- Scope: narrowly defined (reconciliation decision between summary vs row statuses vs gap listing), with explicit instruction not to change coverage numbers unilaterally and to obtain independent review. **Adequate.**  
- Consistency with pre-existing claim: the "pre-existing at b8c3aee" claim is **factually true** by independent count.

**Conclusion (c):** NC-EXT-92 is adequately registered with source, severity, scope, owner, verifier. The baseline claim is verified true. No phase/release implication. Minor note: the sprint-plan text cites "TS-4 §3.2" in the NC row but the metrics are in §3.1 (Coverage summary); the revision history says "§3.1/§3.2". This is a section reference typo in the NC row (non-blocking).

### (d) Evidence artifacts tied to an identifiable configuration; missing for phase/release-relevant identity

**Current identity state (inspected):**
- 49-file candidate manifest (2026-10-16-s8-current-candidate-manifest.md): digest 1a84475…, 49 paths. Method: SHA-256 over sorted UTF-8 path+NUL+SHA-256(contents)+LF; excludes status/planning/reviews/evidence/build outputs. Build/run references: HEAD b8c3aee (stated), build/ci/CMakeCache.txt hash, build/ci/frick_fsw hash, log reference. Claims product-set identity aid only; not full repo snapshot/provenance/qualification/phase/release.  
- 19-file PUS/Scheduler delta inventory (2026-10-16-s8-pus-scheduler-delta-identity.md): digest 272543e…, narrow scope. Verified by PA (narrow closure of NC-EXT-91). Explicitly not complete candidate.  
- PA 0361 verified 49-file manifest matches working tree at review time; PA 0364 verified run-linkage. However, the reconciliation delta modified files after those checks: ts-1 (rev 1.8) and ts-5 (rev 1.2) changed, test_sched.c changed (exhaustion test), test_pus_fu.c changed (new cases), etc. Independent hash comparison against the listed manifest hashes shows: ts-1 519e7d59 (manifest) vs 978c6ba6 (current) — **STALE** (49-file manifest); ts-5 f83c3198 vs 109738fc — **STALE**; test_pus_fu.c 7637c2f8 vs eaf00e51 — **STALE**; test_sched.c f99a5480 vs 9c29da3d — **STALE**; Makefile 043f8449 vs 1c4a0501 — **STALE** (also differs between 49-file and 19-file in stated history); pus_services.c matches 19-file inventory but 49-file manifest had 3f91d54c vs current matches 19-file 7417bcc5? Wait: 19-file says pus_services.c is 7417bcc5 and current is 7417bcc5 — OK (matches 19-file). 49-file says 3f91d54c — so 49-file manifest hash for pus_services.c is stale relative to current. Yes, confirmed: 49:STALE 19:OK for pus_services.c. Similarly pus.h 49 stale vs 19 OK; Makefile both stale relative? Current Makefile hash 1c4a0501 matches 19-file inventory (1c4a0501) but 49-file manifest says 043f8449 — so 49-file manifest is stale for Makefile; 19-file inventory is current for Makefile (matches). ts-1/ts-5 are stale in both relative to current (both inventories were generated earlier).  
- Untracked files exist (not in manifests): tests/unit/test_pus_fu.c, test_pus_n7_adapter.c, test_obcp_integration.c, src/obcp/, ts-9-obcp-integration.md, ADR-0019, ADR-0020 — some appear in inventories (ADR-0020 in 19-file; test_pus_fu.c, test_pus_n7_adapter.c, ts-9, ADR-0019, obcp files in 49-file? 49-file lists src/obcp/ff_obcp.c/h, tests/unit/test_pus_fu.c, test_pus_n7_adapter.c, ts-9-obcp-integration.md, ADR-0019 — yes, all listed with hashes; check current vs manifest: test_pus_fu.c is stale in 49-file vs current (eaf00e51 current vs 7637c2f8 manifest). test_pus_n7_adapter.c: current 83dde356 matches 49-file manifest (83dde356) — OK. ts-9: matches. ADR-0019 matches. obcp .c/.h: manifest has 3169d3ba/61caa0f8 — let’s quickly sanity check a couple: current ff_obcp.c? The script checked a subset; easier to trust manifest includes them. But test_pus_fu.c is the key untracked/tracked-changed file backing new cases and its manifest hash is stale.

**Findings (d):**
- Neither the 49-file manifest nor the 19-file inventory describes the current working tree in full (multiple file hashes differ; TS-1/TS-5 differ from both). The earlier verified identities (0361/0364) do not cover the reconciliation delta as committed to the working tree now. **Open issue (high significance for identity).**  
- Identity is bounded (product set, excludes planning/status/reviews/evidence/build) and explicitly not a full repo snapshot/provenance/qualification/phase/release. This is appropriate given scope, but it means phase/release-relevant identity is incomplete. **Missing:** an updated candidate manifest (or refreshed digests) covering the current working tree state that backs the claimed covered cases (especially TS-1/TS-5/test_pus_fu.c/test_sched.c), plus a correlation to the final CI-local run/cache/executable used for this delta. Also missing: complete generated-source/tool provenance (NC-EXT-84 remains open).  
- Run-to-configuration linkage remains limited (no signed attestation). Evidence artifacts exist (logs, manifests) but are point-in-time and not updated to current tree state. **Missing for phase/release:** immutable source snapshot (hash tree of all files that affect behavior/evidence), build environment manifest (compiler/tool versions, flags), deterministic/reproducible build evidence, and source-to-binary trace binding the run to that snapshot.  
- NC-EXT-88/89/90/84 remain open; hosted CI unverified; Renode/RTEMS cross-build evidence exists but not target runtime qualification. These are consistent with "not phase/release approval".

**Conclusion (d):** The evidence artifacts are tied to identifiable (bounded) configurations at the time of review, but they are **not current** for the reconciliation delta in the working tree. No earlier bounded identity acceptance covers the current tree. Phase/release-relevant identity is missing (updated manifest/digest set covering current tree, refreshed run correlation, and generated/tool provenance closure). **Open issue.**

### (e) Immediate safety/security concern?

Reviewed scope: PUS service behavior corrections (first-failure preservation), scheduler per-entry targets (addresses starvation), decoder corpus/ES-HS lifecycle test additions, identity/trace updates. No code changes observed that introduce unsafe memory operations, privilege escalation, unsafe crypto, or new external interfaces. Changes are host-testable logic and documentation/spec reconciliation. No new dependencies introduced in a way that changes attack surface; OBCP defaults remain pinned with documented limitations (NC-S9-01 deferred). Safety/security-relevant NCs remain open but not newly created in this delta (NC-EXT-90 involves time arithmetic overflow policy — a robustness concern, but no immediate hazard identified; it remains open with policy required). No evidence of backdoors, altered evidence, or blocked access. **No immediate safety/security concern requiring escalation.** However, the time overflow policy gap (NC-EXT-90) is a robustness concern that must be closed before operational use; it is appropriately tracked, not immediate in this host-bounded scope.

## 3. TS-4 count observation (baseline vs worktree)

As verified in (c):
- Baseline b8c3aee (git show): 80 rows, N22-TS4-1..N22-TS4-80. Status distribution: 62 "covered", 16 "gap", 1 "covered (host subset)" (75), 1 "covered (host contract)" (76). So covered-style = 64, gap = 16. Metrics §3.1: covered=62, gap=18, total 80. **Discrepancy exists pre-baseline.**
- Worktree: identical to baseline — 80 rows, same status split, same metrics. No delta touched TS-4. **No change.**

Exact observation: baseline and worktree both show table covered-style count = 64 (62+2) with 16 gap rows (total 80), while §3.1 summary states covered=62, gap=18. This matches NC-EXT-92’s description exactly. NC-EXT-92 is registered as pre-existing at b8c3aee (confirmed).

## 4. Findings (numbered)

### F-0373-01 (Medium, process/closure taxonomy)
**Criterion/source:** PA 0354 states NC-EXT-88/89 remain open pending complete evidence/review and NC-owner disposition; R1 0355/R2 0356 state bounded corrections verified and defect closed narrowly. Sprint-plan 1.68 says "CLOSED narrowly by the independent R1/R2 delta re-reviews 0355/0356".  
**Observed:** Ambiguity in closure status taxonomy between review verdict ("narrowly closed") and PA acceptance of closure ("remains open pending full cycle/identity").  
**Affected:** NC-EXT-88/89 tracking, review log/PA gate clarity.  
**Gap:** No single authoritative closure state.  
**Risk:** Confusion at phase gates; could misrepresent what is verified.  
**Owner:** PA lead + R1/R2 leads (governance).  
**Due:** Before N20 phase gate.  
**Closure evidence:** Updated NC register entry clarifying: (a) functional/test defect corrected and verified by focused reviews (narrow closure of defect), (b) PA acceptance of full closure requires configuration-matched full cycle and identity; both states recorded separately with timestamps.

### F-0373-02 (Minor, unreviewed coverage claim)
**Criterion/source:** N9, N22; TS-5 rev 1.2 adds N22-TS5-77 as covered case (deadline/exhaustion) with new test `test_time_exhaustion_does_not_wrap`.  
**Observed:** No independent review report cites this specific test/oracle. 0360 mentions NC-EXT-90 requires tests but does not assess this case.  
**Affected:** Coverage claim TS5-77.  
**Gap:** Coverage increment lacks independent review.  
**Risk:** Low (host robustness test; NC-EXT-90 remains open).  
**Owner:** R2 (V&V) with PA oversight.  
**Due:** Before relying on this covered case for any gate closure; can be tracked under N21.  
**Closure evidence:** Independent R2 delta review report assessing N22-TS5-77, the test code, oracles (deadline disabling, saturation at UINT32_MAX, no wrap), and test execution results tied to configuration.

### F-0373-03 (Minor, section reference typo in NC record)
**Criterion/source:** Sprint-plan 1.68 NC-EXT-92 row cites "TS-4 §3.2"; metrics are in TS-4 §3.1 (Coverage summary). Revision history says §3.1/§3.2.  
**Observed:** Inaccurate section reference.  
**Affected:** NC-EXT-92 traceability.  
**Gap:** Documentation accuracy.  
**Risk:** None (semantic content is clear).  
**Owner:** FSW/test-spec author.  
**Due:** Next doc update.  
**Closure evidence:** Corrected NC-EXT-92 text to cite TS-4 §3.1 (Coverage summary).

### F-0373-04 (High for identity, but bounded scope)
**Criterion/source:** N11/N20 identity requirements; manifests vs current tree.  
**Observed:** 49-file manifest and 19-file inventory are out of date relative to current tree (multiple file hashes stale). PA 0361/0364 verified earlier states; current state unverified by those identity checks.  
**Affected:** Configuration identity for the reconciliation delta.  
**Gap:** Updated manifest covering current tree (ts-1, ts-5, test_pus_fu.c, test_sched.c, Makefile, pus.h, pus_services.c, pus-layer.md as applicable) with refreshed digest, and correlation to current CI-local run/cache/executable.  
**Risk:** Cannot assert the evidence set corresponds to the current working tree without re-verification.  
**Owner:** FSW/CM + PA.  
**Due:** Before any phase/release-relevant decision using this delta.  
**Closure evidence:** New candidate manifest (or addendum) with SHA-256 of all behavior-affecting files in scope, digest computed by stated method, verified against working tree, and PA identity review report covering the current state (or explicit delta of changed files).

### F-0373-05 (Medium, provenance/tooling)
**Criterion/source:** N20, NC-EXT-84.  
**Observed:** NC-EXT-84 remains open (generated-tree identity/generator/tool provenance/reproducible generation for N7/OBCP generated code). Untracked-generated material implications.  
**Affected:** Provenance chain.  
**Gap:** Generator version, inputs, reproduction recipe, qualification implications.  
**Risk:** Low if bounded to host scope, but blocks phase/release.  
**Owner:** FSW/tooling.  
**Due:** Per NC-EXT-84.  
**Closure evidence:** As specified in NC-EXT-84.

## 5. Residual provenance/identity gaps with owners

| Gap | Description | Owner | Due/target | Status |
|---|---|---|---|---|
| ID-G1 | Updated candidate manifest/digest for current working tree (refresh ts-1/ts-5/test_pus_fu.c/test_sched.c/pus*.c/h/Makefile/pus-layer.md and verify unchanged others). | FSW/CM + PA | Before phase/release decision | OPEN |
| ID-G2 | Correlation of current CI-local run (log/cache/executable hashes) to the refreshed manifest/snapshot. | PA + FSW | Before phase/release decision | OPEN |
| PROV-G1 | NC-EXT-84: generator/tool provenance for N7 generated tree (version, inputs, reproducibility). | FSW/tooling | Per NC-EXT-84 | OPEN |
| REV-G1 | Independent review of TS-5-77 test/oracle (F-0373-02). | R2 + PA | Before relying on covered claim | OPEN |
| NC-G1 | NC-EXT-90: time overflow domain/policy/exhaustion response and tests (includes exhaustion case). | Systems + FSW + V&V | Before operational use | OPEN |
| NC-G2 | NC-EXT-88/89: full PA acceptance pending full cycle/identity per 0354 (defect narrowly closed by R1/R2). | PA + FSW | Before phase/release | OPEN |
| NC-G3 | NC-EXT-19: mission schedule/time policy, target timing, TBL activation-to-dispatch, late-slot/recovery. | Systems + FSW | Per plan | OPEN |
| NC-G4 | NC-EXT-91: narrowly closed for 19-file delta only; broader identity remains. | PA + CM | Contextual | CLOSED (narrow) |
| NC-G5 | NC-EXT-92: reconcile summary vs row statuses (pre-existing); decide authoritative source, R2/R4 review. | FSW/test-spec author + R2/R4 | Later under N21 | OPEN |
| ID-G3 | OBCP mission-use gaps (NC-S9-01 deferred) and production service bindings (S9-S10). | FSW + OBCP authority | Later | OPEN |

## 6. Explicit conclusion

The reconciliation delta is **consistent** with the bounded review evidence for the enumerated narrow closures (NC-EXT-23/24/88/89/91) and with open items (NC-EXT-90/19/84/92). However, identity coverage for the current working tree is incomplete (earlier verified manifests are stale relative to the reconciliation delta). A new coverage claim (TS5-77, deadline/exhaustion test) lacks an explicit independent review. Closure taxonomy between R1/R2 (narrow defect closure) and PA full-cycle acceptance is ambiguous (F-0373-01). 

**Overall conclusion for this bounded PA assessment:** The delta is acceptable as a bounded reconciliation of reviewed corrections **only if** (a) an updated identity manifest covering current tree is produced and verified, (b) TS5-77 receives independent review, and (c) the open NCs listed remain tracked. No immediate safety/security concern is identified.

## 7. No N20 phase/release approval granted

**Explicit statement:** This is an independent Software Product Assurance assessment (profile PA, norm N20). It does **not** constitute N20 phase/release approval, does **not** certify compliance, does **not** approve safety case/residual risk, does **not** close nonconformances, does **not** qualify tools, does **not** authorize release, and does **not** declare flightworthiness. All decisions requiring authorized human approval remain with the designated independent qualified human authorities. The bounded identity acceptance from prior reviews does not cover the current working tree; phase/release-relevant identity and provenance remain incomplete.

**Confidence summary (per §14 block):**
- Decision/product/config: Uncommitted reconciliation delta in frick-fsw at working tree state (post b8c3aee), bounded scope as defined.
- Claim type: Open issue/Inference/Fact mix (as labeled above).
- Applicable baseline/tailoring: ECSS-Q-ST-80C/ECSS-E-ST-40C context per project docs; exact tailoring invoked not re-validated here (beyond inspection of referenced artifacts).
- Evidence consulted: As listed in §1 (reviews 0349/0350/0354/0355/0356/0360/0361/0364, 19-file/49-file manifests, sprint-plan 1.68, TS specs, loop-audit 10.6, baseline TS-4).
- Cross-check/reconciliation: Baseline-vs-worktree TS-4 counts confirmed identical (64 covered-style/16 gap vs 62/18 summary; pre-existing). Manifest hash deltas computed.
- Confidence: Medium (bounded inspection of primary artifacts; some manifests stale relative to current tree; review coverage for TS5-77 is inferred absent).
- Basis/detection limits: Direct inspection of specified files and review texts; hash comparisons for a subset; could not re-run full CI cycle as part of this assessment.
- Missing/inaccessible evidence: Updated identity manifest for current tree; independent review of TS5-77; complete generator provenance (NC-EXT-84).
- Assumptions/expiry: Assumes review texts are accurate as written; recheck after refreshed identity and TS5-77 review (expiry: before any gate closure using this delta).
- Consequences if wrong: Misleading closure state or unverified identity could propagate to phase gate; low immediate safety impact but process integrity risk.
- Required human reviewer/authority: Designated PA authority, R2 lead (V&V), R1 lead (FSW), systems authority (NC-EXT-90), CM/tooling leads.

**End of report FRICK-REVIEW-0373.**