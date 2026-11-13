# Independent Avionics / Systems R1-R3 Correction Re-review — PUS S1 Ingestion Failures

| Field | Value |
|---|---|
| Review ID | SYS-R1R3-20261113-PUS-S1-INGEST-RR |
| Review date | 2026-11-13 |
| Correction target | `SYS-R1R3-20261112-PUS-S1-INGEST`, finding SYS-R1R3-01 (Major) |
| Scope | Focused re-review of the candidate test, TS-1 rev 1.12, traceability manifest, verification results, loop audit, SRS and ICD; no external policy decision or NC closure |
| Result | **PASS WITH COMMENTS — Major correction substantiated.** `N22-TS1-23` is no longer represented as requirement-covered. The two documentation-synchronization observations were subsequently confirmed corrected; current r4 evidence is noted with its limits below. |

> **Authority limitation.** This report is an independent engineering review, not an approval or delegated design/safety authority. It does not approve a PUS profile, hazard classification, operational use, qualification, release, mission acceptance or flight readiness. It does not close NC-EXT-04, NC-EXT-13, NC-EXT-18, NC-EXT-241, or any other NC.

## 1. Conclusion and disposition

**SYS-R1R3-01 — Major — RESOLVED IN SUBSTANCE.** The original defect was a coverage-attribution overclaim: the then-`host covered` `N22-TS1-23` treated the illegal-APID rejection and acceptance-stage TM(1,2) as grounded in SRS-REQ-005/017 and ICD text, although the APID receive-acceptance policy was explicitly unresolved. The candidate correction removes that claim rather than altering requirements or implementation:

- TS-1 rev 1.12 labels `N22-TS1-23` `gap`; the row explicitly says SRS-REQ-017 lists APID values but does not establish the rejection/acceptance-failure policy, and ICD §1 leaves it open under NC-EXT-13.
- The observed return code, TM(1,2), lack of TM(1,8), non-dispatch, and payload/code observations are expressly designated characterization. The 9-octet layout is attributed to implementation and NC-EXT-241; the failure code is provisional under NC-EXT-18; the `FF_PusIngest` return is characterization.
- `N22-TS1-104` remains `gap`; its ack-zero assertions are called characterization, not requirement-derived oracles, and are linked to NC-EXT-04 / `N22-TS1-22`.
- The TS-1 total is 104 cases, 76 covered + 28 gap. This is consistent with the previous 77/27 classification after moving `N22-TS1-23` from covered to gap and adding no new case in this correction.
- `traceability.yaml` now says SRS-REQ-017 derives the configured APID values from tailoring but that the current rejection behavior is characterized by code/test and the acceptance-policy basis remains open under NC-EXT-13. It does not present the policy as controlled by SRS-REQ-017.
- The later independent synchronization confirmation (`SYS-R1R3-20261113-PUS-S1-INGEST-SYNC-CONFIRMATION`) verifies that traceability.md rev 0.54 mirrors that mapping and the loop-audit §11 points at r4. The two Minor observations in the initial correction re-review are therefore **resolved in substance**.
- Test comments and `verification-results.md` are consistent with the bounded characterization claim and retain the layout/code/ack-zero qualifiers. The source path examined (`src/pus/pus.c`) does reject any non-TC APID before dispatch, and emits the acceptance-failure report only when acceptance ack is set; this is an observation of the current implementation, not a policy endorsement.

Accordingly, the original Major finding's requested correction—retain useful test evidence while removing requirement-coverage credit for unresolved policy—is achieved. This is a pass of that correction only, not a general approval of the test increment or project gates.

## 2. Bounded system / avionics assessment and residual ambiguity

The test is useful as host implementation characterization: it exercises a validly encoded TC with APID `0x011`, acceptance ack set, and registered ping handler; it checks one TM(1,2) with the current symbolic code, no TM(1,8), no acceptance-success TM, and no ping reply. The ack-zero cases record current no-TM behavior for non-TC APID `0x020` and unknown subservice (17,9). The assertions do not establish how an approved ground/spacecraft profile should route, reject, acknowledge, retry, or time out these commands.

Residual interface and operational questions remain authority-owned. A wrong/nonconfigured APID may represent misaddressing, a valid packet for another endpoint, or an invalid command at this endpoint; the baseline's reject-before-dispatch behavior and acceptance-stage report do not resolve that interpretation. Negative-verification stage, destination/Request-ID interpretation, and what the ground system does when ack is not requested remain open. Separately, SRS-REQ-005's unconditional completion-failure wording conflicts with ack-gated implementation behavior; `N22-TS1-104` appropriately records that gap. Depending on the mission command-state model, absent or differently staged verification could affect ground awareness and recovery, but the reviewed evidence does not establish operational timing, retry policy, hazard consequence, or severity. No hazard classification is assigned.

The report and candidate appropriately leave NC-EXT-13, NC-EXT-04, NC-EXT-241, NC-EXT-18 and related interface/profile decisions open. No external policy or interface decision is inferred here.

## 3. Comments (non-blocking for the Major correction)

| ID | Severity | Observation | Bounded recommendation |
|---|---|---|---|
| SYS-R1R3-RR-01 | Minor — resolved | Initially, traceability.md did not mirror the manifest's PUS paths and policy-open qualification. The independent synchronization confirmation verifies rev 0.54 now matches the manifest and labels APID rejection as implementation characterization with NC-EXT-13 open. | No further action within this comment. |
| SYS-R1R3-RR-02 | Minor — resolved | Initially, loop-audit §11 pointed to r2 while the corrected evidence was r3. The independent synchronization confirmation verifies §11 now points to r4 and identifies the r4 evidence. | No further action within this comment. |
| SYS-R1R3-RR-03 | Observation | Current r4 evidence identifies a fresh RTEMS ELF (SHA-256 `e36aee30f381df32d9b085b11d616e35082a9650263e3b8614ed8f4004843a41`, mtime 2026-11-13 00:42:40 +0100) and a fresh `make renode` run at 2026-11-12T23:43:23Z with `RENODE_TEST: PASS` and container exit 0. The identity, status, raw logs and `gate-renode-fresh.log` agree on those identifiers. This is valid N18 emulated functional-validation evidence for the identified image/run; it is not physical target, HIL, timing, or policy/requirement-coverage evidence. | No bearing on SYS-R1R3-01; retain the expressly limited emulator claim. |
| SYS-R1R3-RR-04 | Observation — evidence-currentness, outside correction scope | The r4 directory contains a 45-entry `assets.sha256`. A check against the current working tree reports four stale entries (`docs/reviews/PA-R3-20261113-s8-pus-s1-ingestion-failure-review.md`, `docs/reviews/README.md`, `docs/status/loop-audit.md`, `docs/vv/verification-results.md`). The loop audit itself says final manifest rebind/verification remains pending after subsequent report/document updates. This does not negate the N18 raw evidence or alter the coverage-basis conclusion, but the manifest should not be described as verifying the latest bytes until rebound. | Refresh and verify the manifest after current report/document updates under the normal evidence-integrity process; not a blocker for this focused R1/R3 correction conclusion. |

The synchronization comments were confirmed resolved by the independently retained confirmation report. The observations above do not reinstate the coverage overclaim or prevent this focused correction re-review from passing; the manifest-currentness observation remains for normal evidence maintenance.

## 4. Evidence and limits

Reviewed: initial report `docs/reviews/2026-11-12-r1-r3-s8-pus-fu-s1-ingestion-failures.md`; `tests/unit/test_pus.c::test_verification_failure_via_ingest` and its packet helper; `src/pus/pus.c::FF_PusIngest`; TS-1 rev 1.12 rows 20, 22, 23, 104, its gap list, count and history; SRS-REQ-005/017; ICD §§1–4; SRS-REQ-017 entries in `traceability.yaml` and traceability.md rev 0.54; verification-results r4 record; loop-audit §11; synchronization confirmation; and the r4 N18 identity/status/log artifacts. I counted the r4 manifest entries and ran `sha256sum -c`; the 45-entry file is present, but four current entries fail after later documentation/report updates, consistent with the audit's final-rebind-pending note. I did not rerun build/test/Renode.

This was a source/document and retained-evidence inspection. The fresh N18 identity and pass status were cross-checked in the retained files, but this does not establish hardware/RTEMS runtime fidelity. I did not re-run tests or Renode, inspect a mission-authorized profile, consult the full ECSS text, analyze ground operations, or perform a new hazard assessment. Confidence is **High** that the original requirement-coverage overclaim has been removed in the reviewed candidate text; **Medium** on operational implications because routing, verification, retry and ground-command assumptions are not provided.

## 5. Overall disposition

**Correction re-review: PASS WITH COMMENTS.** The Major SYS-R1R3-01 correction is substantively effective: `N22-TS1-23` is gap/characterization, `N22-TS1-104` is gap/characterization, the count is 76 covered / 28 gap, and machine- and human-readable traceability do not claim that the open acceptance policy is controlled. The two synchronization comments were independently confirmed resolved. The fresh N18 run is supported as emulator-only evidence for the specified ELF, without changing this requirements-basis conclusion. The r4 manifest has 45 entries but currently reports four stale document/report hashes pending the rebind already recorded in the audit; this is outside the corrected Major. No NC is closed and no safety, profile, qualification, release or flight decision is made.
