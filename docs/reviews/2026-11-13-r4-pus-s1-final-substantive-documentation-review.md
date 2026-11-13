# R4 Substantive Documentation Review — S8-PUS-FU S1 Frozen Candidate

| Field | Value |
|---|---|
| Review ID | DOC-R4-20261113-PUS-S1-FINAL-SUBSTANTIVE |
| Review date | 2026-11-13 |
| Candidate | HEAD `e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83` plus the current working-tree delta and retained evidence/reports |
| Scope | Substantive N17 review of the candidate’s changed product documentation, review/evidence records, report index, candidate identity, and intended r4 package-manifest inventory boundary |
| Verdict | **CHANGES REQUIRED for final package-integrity readiness.** The technical-status wording is substantially consistent and appropriately bounded; the current on-disk manifest is an obsolete 45-entry snapshot and does not establish integrity of this frozen candidate. No final manifest result is asserted. |

> **Authority statement.** This is an AI-assisted documentation review, not engineering, PA, CM, safety, certification, release, or commit approval. It closes no NC, accepts no risk, and does not declare sprint completion. Qualified authorized humans retain those decisions.

## 1. Scope and method

Reviewed current HEAD and working-tree identity; the tracked diff; `CHANGELOG.md`; `docs/reviews/README.md`; `docs/vv/verification-results.md`; `docs/status/loop-audit.md`; TS-1 rev 1.12; the machine-readable traceability and human-readable mirror; tool qualification rev 1.9; sprint-plan rev 1.139 and its NC entries; the r4 evidence README, identity records, package manifest, CTest inventory/submanifest, retained gate and Renode evidence; and the current initial/correction R1/R2/PA/R4 reports, including `VV-R2-20261113-s8-pus-s1-manifest-scope-method-review.md`. Compared cross-document status and candidate claims. This review did not modify product documentation, run builds/tests/Lean/trace/Renode, or assess hardware or external-standard clause compliance.

The tracked working-tree diff contains ten paths: `CHANGELOG.md`, `docs/planning/sprint-plan.md`, `docs/requirements/traceability.md`, `docs/requirements/traceability.yaml`, `docs/reviews/README.md`, `docs/status/loop-audit.md`, `docs/vv/test-spec/ts-1-protocol-core.md`, `docs/vv/tool-qualification.md`, `docs/vv/verification-results.md`, and `tests/unit/test_pus.c`. Candidate identity records the same ten paths against the stated HEAD and their SHA-256 values. `src/pus/pus.c` is not modified and is identified as unchanged. Other untracked review reports and the r4 evidence tree are separate package/evidence inputs, not additional tracked product-delta paths.

I ran the existing package check from the repository root:

```text
sha256sum -c docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4/assets.sha256
```

The existing file has **45 entries; 41 report OK and 4 fail**: `docs/reviews/PA-R3-20261113-s8-pus-s1-ingestion-failure-review.md`, `docs/reviews/README.md`, `docs/status/loop-audit.md`, and `docs/vv/verification-results.md`. This is the status of the obsolete/currently-on-disk snapshot only. No proposed final manifest has been generated or verified in this review.

## 2. Substantive content and consistency assessment

### 2.1 Scope, requirements attribution, and TS-1

The current record accurately limits the increment to test/documentation changes; it does not claim production PUS source or requirement changes. TS-1 rev 1.12 states the current candidate baseline, describes implementation characterization separately from requirement-grounded coverage, and identifies the authority gaps. The 104 case IDs reconcile to 76 covered and 28 gaps; the 48 nominal, 51 OOB, 5 mixed; 93 individual and 11 combined; and 101 SIL, 1 PIL, 2 HIL totals are consistent with the independently retained R2 recount.

`N22-TS1-23` is correctly a gap: APID value tailoring does not define rejection/acceptance-report policy. Its observed return/TM/non-dispatch/payload behavior is labeled characterization, the 9-octet layout is attributed to implementation and NC-EXT-241, and the failure-code value remains provisional under NC-EXT-18. `N22-TS1-104` is also a gap: ack-zero behavior is characterization under the unresolved SRS-REQ-005 conflict (NC-EXT-04), not an acceptance oracle. The gap taxonomy now permits a characterization check without crediting it as a requirement-derived oracle. The out-of-order case rows remain explicitly tracked by NC-EXT-242 rather than silently represented as resolved.

The SRS-REQ-017 YAML entry and traceability mirror agree on implementation/test paths and the policy-open qualification. The mapping does not turn observed rejection into a controlled requirement. Trace PASS is recorded as a gate result, not as proof that the open policy is resolved.

### 2.2 Evidence, run identity, and claims

The r4 results and README distinguish the 57/57 registered CTest result from the unregistered `test_pus_s11_residual` executable (10/1164 direct checks, NC-EXT-240); they do not claim every declared test target passes. The retained inventory says 57 registered entries and 55 distinct executable copies, and the separate CTest binary submanifest is reported as 55/55 verified. The focused test is reported as 322 checks, 0 failures. The successful isolated `make build`/`make test` legs and failed default-cache-path attempts are distinguished rather than conflated.

The N18 record consistently identifies the fresh Docker-built ELF as `e36aee30f381df32d9b085b11d616e35082a9650263e3b8614ed8f4004843a41`, with a fresh Renode PASS and retained identity/status/process/UART records. The earlier PASS associated with `55188f5a…` is explicitly invalid historical evidence. Claims remain emulator-only, without physical-target, HIL, timing/WCET, hardware-fidelity, or flight-readiness claims. The full pinned clang-format 18 failure on three unchanged initializers is recorded as a failure and NC-EXT-244, while the changed test file’s pinned-format pass is kept distinct. T3 remains **NOT FULLY QUALIFIED**.

The controlled results, current audit addendum, README, and review index consistently disclose that the package-level manifest is stale/unverified and that the final rebind is pending. R1/R3 correction and synchronization results are recorded as bounded PASS WITH COMMENTS/PASS; PA and V&V/R2 evidence findings remain open pending their required final delta work. The R4 correction delta records DOC-R4-RR-02 resolved and DOC-R4-RR-01 partially resolved/open. No NC closure, qualification, release approval, or sprint completion is claimed. The sprint plan’s earlier S8-PUS-FU implementation-complete status is scoped to that earlier implementation milestone and does not claim this current evidence/review cycle is complete.

## 3. Manifest inventory and exclusion assessment

The declared boundary in r4 `README.md`, `docs/reviews/README.md`, `verification-results.md`, and the loop-audit addendum is directionally consistent: bind candidate source/document content, identity records, retained evidence, and completed substantive reviews, including this substantive R4 report and the completed R2 scope-method report; do not recursively hash the manifest or the three post-manifest hash-only attestations. The R2 scope-method report is present and indexed, but is not yet bound by a final manifest. The three and only three attestation exclusions are stated by exact path:

1. `docs/reviews/PA-N20-20261113-s8-pus-s1-final-manifest-attestation.md`
2. `docs/reviews/VV-R2-20261113-s8-pus-s1-final-manifest-attestation.md`
3. `docs/reviews/R4-N17-20261113-s8-pus-s1-final-manifest-attestation.md`

Those reports are pre-indexed as checksum/inventory attestations, not substantive reviews. They are excluded from the manifest they attest; their later bytes are intended to be bound by the final Git commit. They must not substitute for this substantive N17 review. The manifest itself is the sole other exclusion. Unrelated pre-existing worktree material is outside the candidate definition, not an additional manifest exclusion; it must not be silently included or represented as part of this candidate.

The candidate/evidence inventory that the final manifest must actually bind includes, at minimum:

- the ten changed tracked candidate paths above, plus the unchanged source/build inputs whose identity is used to support this increment (including the recorded PUS implementation, test registration/build configuration and relevant configuration inputs);
- the candidate identity and working-tree patch, the r4 README, CMake cache and executable identity records;
- the current controlled test/results/audit/traceability/tool-qualification/sprint/change-history documents and review index;
- the relevant retained configure/build/CTest/focused-test, Make-target, CI-local, Lean/receipt/trace, format, license/GSW, smoke, Doxygen, Renode-environment, mutation, and operator-attempt records, including successful isolated Make logs and disclosed failed/interrupted attempts that form part of the reviewed evidence chronology;
- the CTest invocation inventory, binary hash list and verification log, all 55 retained test executable copies, focused `test_pus`, fresh Renode ELF, and raw Renode identity/status/process/UART/stderr records;
- every completed substantive review report for this increment, including the initial PA/V&V/R4/R1-R3 reports, correction/re-review and delta reports, PA-R3, R1/R3 correction and synchronization confirmations, the R2 scope-method review, and this final substantive R4 report.

The candidate identity’s ten-path list is suitable for the tracked product delta, but is not itself the full evidence/package inventory. The present 45-entry manifest is not a valid final inventory: in addition to its four failed current hashes, it omits the newly retained r4 `artifacts/` inventory and executable copies, the successful isolated Make-target logs, the r4 README, and subsequent completed substantive review reports/index/control-record bytes, including the R2 scope-method report. It also still names transient `build/evidence-20261112-pus-s1-r4/...` and `build/renode/...` paths even though the r4 README says transient `build/` paths are not manifest inputs and exact artifacts are retained under r4. The new R2 scope-method review independently calls out this intended-scope/current-manifest mismatch. The review index and loop-audit also use present-tense wording that the substantive R4 report “is included” in the manifest, while the final manifest has not yet been generated and the current manifest does not include it. These statements should be prospective until actual inclusion is independently confirmed. The current controlled results and r4 README otherwise disclose the pending final rebind. The 45-entry file therefore cannot establish that the stated “all current candidate sources/docs/evidence and substantive reviews” scope is bound. The broad phrases “supplemental gate logs” and “completed substantive review reports” are acceptable scope descriptions only if the generated manifest is checked against a concrete, exhaustive file inventory; they are not evidence that all expected paths were included.

## 4. Prior R4 finding dispositions

| Earlier finding | Current disposition |
|---|---|
| DOC-R4-001 — NC-EXT-240 misfiled/absent from register | **Resolved in substance.** NC-EXT-240 is in the five-column NC register with owner/disposition; it remains OPEN. |
| DOC-R4-002 — S1 payload octet oracle attributed to ICD §4 | **Resolved in documentation; authority gap remains open.** TS-1 and results attribute layout to implementation and identify NC-EXT-241; no interface decision or NC closure is inferred. |
| DOC-R4-003 — current review reports untracked/unindexed and called unrelated | **Resolved in content/index, final package binding pending.** Current-cycle reports are indexed and declared in the final package scope; the final manifest must bind all applicable reports, including this report. |
| DOC-R4-004 — stale tool-qualification header | **Resolved.** Header rev/date match the latest history row; T3 remains NOT FULLY QUALIFIED. |
| DOC-R4-005 — ambiguous historical hash correction | **Resolved.** Rev 1.8 history now states the intervening revisions and that the old TSN hash matched no committed revision; rev 1.9 re-binds the current candidate test hash. |
| DOC-R4-006 — sprint-plan revision/history and changelog omission | **Resolved.** Sprint-plan rev 1.139/history and the current CHANGELOG entry record the relevant updates. |
| DOC-R4-007 — overbroad novelty claim | **Resolved in current claim.** Results acknowledge prior packet-level tests and describe the new byte-level characterization; older report statements remain point-in-time records. |
| DOC-R4-008 — gate evidence/order/warnings/artifact binding | **Resolved in substance for execution descriptions; final package binding remains open.** The ordered run, fresh ELF and limits are disclosed. The final current candidate/evidence set is not yet bound by a verified package manifest. |
| DOC-R4-009 — TS-1 nonnumeric ordering | **Tracked/open** as NC-EXT-242, appropriately deferred; uniqueness/range claims are not undermined. |
| DOC-R4-010 — SRS-REQ-017 trace attribution | **Resolved.** YAML and mirror now match and correctly distinguish APID tailoring, characterization, and unresolved policy. |
| DOC-R4-RR-01 — verification record overstates manifest/review currentness | **Partially resolved; remains OPEN/Major for package binding.** False-currentness wording is corrected and the 55-binary persistence evidence is retained, but the current manifest is stale and incomplete. |
| DOC-R4-RR-02 — loop-audit N12/N14 contradiction | **Resolved in substance.** N12 and N14 both reflect the completed scoped R1/R3 correction and synchronization results, without implying broader closure. |

The report index’s status distinction is substantively correct: a substantive R4 review evaluates documentation and status; the three later reports attest hashes/inventory only. A checksum attestation must not be described as passing or replacing this content review.

## 5. Finding and disposition

### DOC-R4-FINAL-01 — Major — Declared final inventory is not yet demonstrated by the package manifest

**Evidence:** the current 45-entry `assets.sha256` check returns 41 OK and four FAILED, and does not include the new retained CTest inventory/binaries, isolated Make-target logs, r4 README, or all subsequently completed substantive reports/control-record bytes. It includes transient `build/` paths contrary to the stated final scope that retained r4 copies are authoritative. This substantive R4 and the R2 scope-method report now exist and are indexed, but neither has been bound by a final manifest. The current review index/loop-audit present-tense inclusion wording is therefore not yet evidenced. The README and controlled results disclose that final rebind is pending; this review does not treat the existing manifest or a future checksum attestation as proof of completeness.

**Required disposition:** reconcile present-tense “is included” wording to prospective language until verified; freeze the full candidate/evidence/review set; generate the package manifest over all intended current sources, documents, evidence and completed substantive reports, including this report and `VV-R2-20261113-s8-pus-s1-manifest-scope-method-review.md`; exclude only the manifest itself and the three exact pre-indexed attestation paths above; ensure transient `build/` paths are not included under the stated rule; independently compare generated paths against the concrete inventory, then run and retain `sha256sum -c` from repository root. Only after successful generation/verification may the three hash-only reports attest the exact digest and full inventory/exclusions. Any covered-file change requires a rebind. Do not state package-manifest success before that check.

**Status:** Open. This is an evidence/document-control readiness finding, not a finding that the reported test or Renode execution failed. It does not itself grant or deny commit/release authority.

## 6. Verdict and limits

The changed controlled content is substantially coherent, current for its stated snapshot, and conservative about policy gaps, test coverage, tool qualification, test registration, format failure, and emulator evidence. Earlier R4 findings are resolved as listed or remain explicitly tracked; RR-01 and the package-inventory finding remain open until the package-level binding is actually generated and checked. Accordingly, **the current package is not ready for a positive final integrity conclusion**. This is not a claim that the final manifest has been generated or failed; it has not been generated as the required final binding. PA/R2 follow-up and their applicable gates remain independent and open as recorded.

```text
Product/configuration and decision supported: S8-PUS-FU S1 candidate; HEAD e5f01d4 plus current working-tree delta/evidence.
Information item/status: substantive N17 review, DOC-R4-20261113-PUS-S1-FINAL-SUBSTANTIVE.
Claim type: Facts (inspected files, hashes, retained reports/logs); assessment; Recommendation (freeze, enumerate, bind, verify).
Applicable baseline: repository N8/N11/N12/N16/N17/N19/N20; no external-standard clause interpretation performed.
Sources/access: repository documents, current working tree, retained evidence and review reports; no external standard text used.
Independent cross-check: tracked diff and candidate identity; review index/results/audit/report comparisons; direct existing-manifest checksum check.
Confidence: High for candidate paths, 45-entry checksum result, wording/status comparisons, and stated review outcomes; medium for completeness of the final future manifest until it is generated and compared with the full inventory.
Limitations: No test/build/Lean/trace/format/Renode execution; no hardware or mission authority review; no final-manifest success asserted.
Required authorities: designated project owner and authorized CM/PA/review authorities as applicable.
Recheck trigger: after final evidence/review freeze, manifest generation, and independent inventory/checksum verification.
```

## History

| Rev | Date | Author | Reason |
|---|---|---|---|
| 1.0 | 2026-11-13 | Senior SW Documentation reviewer profile | Substantive N17 review of frozen pre-manifest S1 candidate, records, review status, and package-inventory boundary. No product documentation modified. |
