# R4 Documentation Correction Re-review — S8-PUS-FU S1 Ingestion Failures

| Field | Value |
|---|---|
| Review ID | DOC-R4-20261113-PUS-S1-RR |
| Review date | 2026-11-13 |
| Scope | Focused N17 re-review of corrections to `2026-11-12-r4-doc-review-pus-s1-ingest-verification-failures.md`, including latest R1/R3 and PA reports and the r4 evidence state |
| Candidate | HEAD `e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83` plus current uncommitted working-tree delta |
| Result | **CHANGES REQUIRED** — prior R4 findings are mostly corrected in substance, but current evidence/review-status statements are stale or contradictory and the manifest does not verify against the current tree |

> **Authority statement.** This is an independent documentation assessment artifact. It has no engineering, PA, safety, certification, baseline, release, or commit authority. It closes no NC, accepts no risk, and does not approve a phase or deliverable. The designated project and human authorities retain all decisions. This review is limited to the correction delta and records listed below; it is not a fresh product-wide review.

## 1. Scope, method, and limitations

Read the initial R4 report and its ten findings; current TS-1 rev 1.12, `verification-results.md`, `tool-qualification.md` rev 1.9, `sprint-plan.md` rev 1.139, traceability manifest/mirror, loop-audit rev 10.62, review index, r4 README/manifest, and the current PA-R3 and R1/R3 correction/synchronization reports. Compared the relevant claim wording and review dispositions. Ran the manifest check from the repository root:

```text
sha256sum -c docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4/assets.sha256
```

**Result: 41 entries OK; 4 entries FAILED** (`PA-R3-20261113-s8-pus-s1-ingestion-failure-review.md`, `docs/reviews/README.md`, `docs/status/loop-audit.md`, and `docs/vv/verification-results.md`). The manifest contains 45 entries. This agrees with the latest PA-R3 and R1/R3 reports, which say the post-update final rebind remains pending.

I did not modify product documents, run build/test/Lean/trace/format/Renode, inspect physical hardware, or verify all technical test oracles anew. The only file created by this review is this report under `docs/reviews/`, as constrained. Because the review index is not modified here, this report still requires a corresponding index entry in the project’s follow-up record update.

## 2. Disposition of initial R4 findings

| Initial finding | Re-review disposition | Evidence / residual |
|---|---|---|
| DOC-R4-001 — NC-EXT-240 misfiled | **RESOLVED** | NC-EXT-240 is in the five-column NC register (`sprint-plan.md` §3), with owner FSW and M17 disposition. It is not in the three-column deferral table. The NC remains OPEN. |
| DOC-R4-002 — octet oracle incorrectly grounded in ICD §4 | **RESOLVED IN SUBSTANCE; authority gap remains open** | TS-1 rev 1.12 marks N22-TS1-23 `gap` and characterizes the APID behavior; N22-TS1-20 uses the qualified `host covered (...)` form and explicitly says the octet order is implementation-derived, not ICD-controlled. NC-EXT-241 records the ICD/interface gap. The implementation-derived payload and provisional values are not represented as approved interface requirements. |
| DOC-R4-003 — review evidence untracked/unindexed and excluded as unrelated | **PARTIALLY RESOLVED** | `verification-results.md` now identifies current-cycle reports and states they belong with this delta; the existing PA-R3 and R1/R3 reports have rows in `docs/reviews/README.md`. However, the paragraph still falsely says the r4 manifest is currently generated and verified and says already-added review rows/reports remain to be added. This re-review report itself also needs an index row after completion. See RR-01. |
| DOC-R4-004 — stale tool-qualification header | **RESOLVED** | Header is rev 1.9 dated 2026-11-12 and agrees with the latest history row. T3 remains NOT FULLY QUALIFIED. |
| DOC-R4-005 — ambiguous hash-history correction | **RESOLVED** | Rev 1.8 history now states “three intervening commits (four later revisions … counting the working-tree delta)” and that rev 1.7’s `0b2cac2a…` value matches no committed revision, with the `ea0ac69` rebind history. |
| DOC-R4-006 — sprint-plan revision/history omission | **RESOLVED** | Sprint plan header is rev 1.139; history rows 1.138/1.139 record the S1 NC registrations and format discrepancy. NC-EXT-240..244 remain open as applicable. |
| DOC-R4-007 — overbroad novelty wording | **RESOLVED IN CURRENT CLAIM; historical entry accurately bounded** | Current results text says the existing unregistered `test_pus_s11_residual.c` and `test_decoder_corpus.c` already exercised packet-level rejection; the new increment adds byte-level characterization. TS-1 history 1.11 says no prior test asserted the failure code/payload layout and names the packet-level tests. The old 1.11 account is explicitly historical, not a current blanket claim that no test existed. |
| DOC-R4-008 — gate evidence/order/warnings/artifact binding | **RESOLVED IN SUBSTANCE FOR THE RETAINED r4 SNAPSHOT; CURRENT BINDING NOT VERIFIED** | r4 README identifies the ordered fresh configure/build/CTest/focused-test sequence, qualifies warning claims, identifies the fresh ELF and raw Renode run, and describes the manifest. PA-R3 and R1/R3 independently support the fresh ELF `e36aee30…4843a41` and Renode PASS as emulator-only evidence; the earlier `55188f5a…` run is expressly invalid historical evidence. The current manifest failure means the latest package bytes are not presently integrity-verified. This is a currentness/binding issue, not a reason to revive the stale-ELF finding. |
| DOC-R4-009 — TS-1 row ordering | **TRACKED / OPEN, appropriately not expanded** | NC-EXT-242 records the out-of-order 95–104 block as a deferred presentation issue. No correction is needed to the range or uniqueness claim. |
| DOC-R4-010 — SRS-REQ-017 trace attribution | **RESOLVED** | `traceability.yaml` and `traceability.md` include `src/pus/pus.c` and `tests/unit/test_pus.c`, and state that non-TC-APID rejection is implementation characterization while the acceptance-policy basis remains open under NC-EXT-13. The R1/R3 synchronization confirmation reports PASS for the mirror. |

The R1/R3 correction re-review reports **PASS WITH COMMENTS**: SYS-R1R3-01 is resolved in substance because N22-TS1-23 is a gap/characterization case; N22-TS1-104 remains a gap; TS-1 totals are 104 = 76 covered + 28 gap. Its two synchronization observations have a separate **PASS** confirmation. Those are bounded review conclusions, not approval of the full candidate or its evidence manifest.

## 3. Current findings

### DOC-R4-RR-01 — Current verification record overstates manifest and review-record currentness (Major)

**Locations:** `docs/vv/verification-results.md` §“Review evidence and residuals” and §“Staging scope”, especially lines 733–765; r4 `assets.sha256`; `docs/status/loop-audit.md` N12 row.

**Evidence.** The 45-entry r4 manifest fails four current file hashes, as listed in §1. Yet `verification-results.md` says at lines 763–765 that “The r4 manifest is now generated and verified.” It also groups the fresh PA/R2/R1-R3/R4 correction re-reviews and their index rows as work “to be added when completed.” PA and V&V/R2 correction re-reviews remain pending, but the R1/R3 correction re-review and its synchronization confirmation are complete and already indexed. The latest PA-R3 report likewise records the manifest as stale after document/report updates and says a final rebind and independent check remain pending. N12 in the loop audit says SYS-R1R3-01 was addressed “pending independent confirmation,” while N14 immediately records that the confirmation passed. These are not a single consistent current status.

**Why it matters.** The controlled results record currently claims verification that does not pass against the current tree and misstates which review outputs/index entries already exist. This is a direct evidence-integrity and review-status issue; a reader cannot reliably determine the current package state from the controlling summary. The fresh Renode identity and run remain supported by their retained identity/status/raw files, but that does not make the stale manifest pass.

**Required bounded correction.** Update the controlled results and audit statements to match the observed current state: manifest final rebind/verification pending; distinguish completed/indexed R1/R3 reports from the PA and V&V/R2 correction re-reviews still pending (and this R4 report, now completed); align N12 with N14 and the R1/R3 confirmation. Then regenerate the manifest over the final intended candidate/evidence/review records, include the applicable new reports and updated index/audit/results, and independently run the check after all included bytes are frozen. Do not claim the final package verifies until that check returns success. The review index also needs a row for this report; that row and any resulting manifest rebind are follow-up work, not performed here.

**Disposition:** Open. This finding prevents a positive R4 correction re-review recommendation until the controlled-record text is reconciled and the current evidence package is rebound and verified. It does not invalidate the fresh N18 run or the narrower R1/R3 correction result.

### DOC-R4-RR-02 — Loop-audit N12 contradicts its N14 synchronization disposition (Minor)

**Location:** `docs/status/loop-audit.md:606,608`.

**Evidence.** N12 says the SYS-R1R3-01 correction is pending independent confirmation. N14 says the correction re-review passed with comments and the two synchronization observations were independently confirmed PASS. The retained R1/R3 re-review and synchronization confirmation support the latter status.

**Required bounded correction.** Reconcile N12 with the completed, scoped confirmation and retain the explicit limitation that no broader NC, policy, phase, or release is closed. N12 must not say “pending independent confirmation” for this correction; unrelated open findings remain open.

**Disposition:** Open; status-bookkeeping correction required. This is not a technical disagreement with the R1/R3 Major correction.

## 4. N18 evidence assessment

**Fact, confidence high for the retained run identity:** r4 `gate-renode-image-docker.log`, `gate-renode-fresh.log`, `renode-run/identity.txt`, `status.env`, process log, and UART log identify ELF SHA-256 `e36aee30f381df32d9b085b11d616e35082a9650263e3b8614ed8f4004843a41`; the run records `RENODE_TEST: PASS` and container exit 0. PA-R3 and R1/R3 independently inspected these retained records. The earlier PASS tied to `55188f5a…` is explicitly withdrawn/invalid historical evidence.

**Assessment:** The stale-ELF concern is resolved in substance for this identified fresh emulated run. The evidence supports a Renode emulator functional-validation claim for that ELF/run, not physical target, HIL, hardware timing/WCET, mission policy, requirement coverage, qualification, release, or flight readiness. The current manifest's four failed entries mean the package as currently edited is not fully checksum-verified; this is a separate currentness limitation and must remain explicit.

## 5. Verdict and gate recommendation

**CHANGES REQUIRED for this R4 correction re-review.** The initial R4 technical/document findings are corrected or tracked in substance, and the R1/R3 Major correction and its synchronization comments are independently confirmed. DOC-R4-RR-01 remains a Major because the controlled verification record says the manifest is verified when the current 45-entry check fails, while also carrying obsolete pending-review wording. DOC-R4-RR-02 is a Minor audit-status inconsistency. The evidence package must be reconciled, regenerated, and independently verified before claiming current manifest integrity; the relevant review-status statements must then be updated and re-reviewed as required.

This report does not authorize or prohibit commit/release; it recommends that the applicable documentation/evidence gate remain uncleared until the currentness corrections and required independent correction reviews are completed and recorded. V&V/R2 correction re-review remains pending. No NC is closed, no test/tool qualification is granted, and no sprint or phase completion is asserted.

```text
Product/configuration and decision supported: S8-PUS-FU S1 test/documentation correction candidate; HEAD e5f01d4 plus uncommitted delta.
Information item/status: focused N17 re-review report, DOC-R4-20261113-PUS-S1-RR.
Claim types: Fact (file text, review outcomes, checksum output); Inference (record-integrity impact); Recommendation (reconcile and rebind before claiming currentness).
Applicable baseline: repository norms N8/N11/N12/N15/N16/N17/N19; no clause-level external-standard compliance assessed.
Sources/access: current repository files and retained reports/logs; no external standard text used.
Independent cross-check: direct manifest check and comparison of review reports, index, verification results, and audit.
Confidence: High for manifest failures, documented report/index statuses, and fresh ELF/run identifiers; medium for overall package completeness pending a final frozen rebind.
Limitations: no build/test/Renode rerun; no hardware or mission-authority assessment.
Required authorities: designated FSW owner, independent R4 and V&V/R2 reviewers, and authorized PA/CM authorities as applicable.
Recheck trigger: after all current report/index/results/audit updates and final manifest regeneration.
```

## History

| Rev | Date | Author | Reason |
|---|---|---|---|
| 1.0 | 2026-11-13 | R4 documentation-review profile | Focused correction re-review of the S1 ingestion-failure candidate; dispositioned initial DOC-R4-001..010, assessed latest R1/R3 and PA evidence, identified current manifest/status inconsistencies, and retained CHANGES REQUIRED. No product file modified. |
