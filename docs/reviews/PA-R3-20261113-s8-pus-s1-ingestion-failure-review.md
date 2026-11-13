# Independent N20 Product Assurance Review — S8-PUS-FU S1 Ingestion-Failure Test

| Field | Value |
|---|---|
| Review ID | PA-N20-20261113-S8-PUS-S1-R3 |
| Review date | 2026-11-13 |
| Baseline | HEAD `e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83` plus the current uncommitted candidate |
| Decision scope | Commit readiness of this bounded S1 ingestion-failure test/documentation delta only; not general release readiness |
| Result | **NOT READY FOR COMMIT on the evidence record reviewed.** One Major evidence-integrity finding remains open. The stale-ELF defect is corrected by the fresh Renode build/run evidence subsequently inspected; complete source-to-artifact manifest binding remains open. This is an assurance recommendation, not an authorization or risk decision. |

> **Authority and independence limits.** This report is an AI-assisted N20 assurance artifact, not approval authority. It does not qualify T3, close any NC, accept safety/security risk, authorize commit/release, or declare flightworthiness. The authorized project and human PA authorities retain those decisions.

## 1. Scope and review basis

Reviewed the requested candidate artifacts: `tests/unit/test_pus.c`; TS-1 rev 1.12; the current S1 section in `docs/vv/verification-results.md`; `docs/vv/tool-qualification.md`; `docs/requirements/traceability.yaml`; sprint-plan NC entries and `docs/status/loop-audit.md`; the r3 evidence package; review index; and the prior PA, V&V, R4 and R1/R3 reports for this increment. Checked working-tree status, source/document hashes, evidence filenames/timestamps, selected log contents and `git diff --check`. After the initial assessment, inspected the added `renode-run/` artifacts and the updated README/results text against the exact ELF hash and statuses supplied in the review update.

The assessment is bounded to the test-only candidate and its assurance records. No independent test execution, full standard-clause review, target/HIL run, hazard analysis, or product-wide release assessment is claimed.

### Configuration identity observed

At review time, `tests/unit/test_pus.c` SHA-256 was `dc8ea3dbae907c43f5897daa799ad792165604eca8cddd119612b2b3a6d889c7`; TS-1 was `a3b8e70e32388a49181726abea39446c5db7bb8aba75463391045e3a413d4245`; `verification-results.md` was `2cb139c24742a0bc55c461c0d12b1af695a21e32921ae290764d31637b4a9543`; `tool-qualification.md` was `1901674a5902a12be0f8b96d38900ed983de5b4eb3c5658daf1ad7541b9bcec9`; and `traceability.yaml` was `61c5e28d0e5f95e54d6fb44b3085cdb599f9d2ace4e86ca044d786e9ef16576f`.

## 2. Positive confirmations / requested checks

1. **Case status and counts are correctly conservative.** TS-1 rev 1.12 marks `N22-TS1-23` as `gap` with explicitly labeled implementation-characterization evidence, not requirement coverage, pending NC-EXT-13. `N22-TS1-104` is a `gap` under NC-EXT-04; its ack-zero observations are characterized and not presented as requirement-derived expected results. The stated totals, 104 = 76 covered + 28 gap, agree with the case-table recount recorded in the candidate documents. No case-count or coverage inflation was found.
2. **Traceability wording is bounded.** The SRS-REQ-017 mapping now includes `src/pus/pus.c` and `tests/unit/test_pus.c`, and explicitly says non-TC-APID rejection is current implementation behavior while acceptance-policy basis remains open under NC-EXT-13. This does not silently turn characterization into a requirement.
3. **No hidden qualification or closure claim found.** T3 remains `NOT FULLY QUALIFIED`; the current test hash is recorded accurately in tool-qualification rev 1.9. NC-EXT-240 through NC-EXT-244 remain open in the sprint plan and loop audit. The 57/57 CTest statement is bounded to registered tests and discloses the unregistered/failing `test_pus_s11_residual` under NC-EXT-240. The records do not claim a general release, safety, or flight approval.
4. **Test scope appears internally coherent.** The changed test drives the ingestion-owned paths, checks TM counts and selected payload fields, checks no dispatch/acceptance-success for the specified case, and marks the ack-zero assertions as characterization. No production source change is represented. This review found no basis to call the test itself a functional defect; this is not a formal test-script qualification.
5. **Prior-review status is not misrepresented as fully closed.** The PA re-review reported its prior findings addressed in substance; the original R4 report required correction re-review; the R1/R3 report identified the coverage-basis issue now corrected in the candidate. Current `verification-results.md` says fresh correction re-reviews remain pending. The index includes the initial reports, not a completed R4/R1-R3 correction re-review.

## 3. Findings

Severity here reflects assurance-record impact, not a product hazard classification.

### PA-N20-R3-01 — Major — r3 package asserts byte binding, but the referenced manifest is absent and the gate sequence is not bound to this candidate

**Locations:** `docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r3/README.md` (“Integrity binding”); `docs/vv/verification-results.md` in the “Executed evidence” section; `docs/status/loop-audit.md` N15 row.

**Evidence observed:**

- The r3 README says `assets.sha256` binds source, documents, cache, executable and logs, and instructs reviewers to verify it. The r3 directory contains no `assets.sha256` (directory listing at review time contains `README.md` and logs only). Thus the stated verification mechanism cannot be performed.
- Current source/document hashes above are not present in a retained manifest. The README does not itself list their complete hashes.
- `gate-configure.log` is timestamped 2026-11-12 21:43 local, whereas the current r3 build/CTest/test logs were written 2026-11-13 00:34–00:35 local. The build record starts “Recovery build started”; a separate `gate-build-interrupted.log` documents an interrupted attempt. The retained configure output therefore does not establish a configure→successful build→CTest sequence for the current recovery run.
- The candidate’s claims of source/build/test-binary/document hash binding in `verification-results.md` and the r3 README are not corroborated by the missing manifest. Test output may be genuine, but the evidence chain does not identify the exact source and binary configuration it proves.

**Consequence / assessment:** This is a configuration/provenance gap affecting N11 evidence integrity and the central commit-readiness decision. The 322/0 focused test and 57/57 registered CTest results are not rejected as false; rather, the retained package does not presently demonstrate byte identity between those executions and all current candidate inputs as claimed.

**Disposition:** **Open; blocks a positive commit-readiness recommendation.** Recreate or restore a manifest covering the exact candidate source/doc hashes, configure/build/test inputs, executable(s), and retained logs; verify it. Reconcile the interrupted and successful build chronology and retain an explicit configure record for the successful build tree, or accurately qualify the sequence and its limitations. Do not describe unbound evidence as hash-bound.

### PA-N20-R3-02 — Major (re-evaluated: stale-ELF gap corrected) — the original cited Renode PASS used a stale ELF; fresh build/run now supersedes it

**Locations:** `docs/vv/verification-results.md` N18 row; r3 `README.md`; `gate-renode.log`, `gate-renode-image.log`, `gate-renode-image-recovery.log`, `gate-renode-image-docker.log`, and `gate-renode-fresh.log` in the r3 evidence directory.

**Initial finding:**

- The N18 table cites `gate-renode.log`, timestamped 2026-11-12T23:37Z, which records a PASS but no ELF identity.
- Subsequent image-build attempts in `gate-renode-image.log` and `gate-renode-image-recovery.log` failed (old CMake cache path mismatch; then missing `/opt/rtems/6/bin/riscv-rtems6-gcc`). Those logs recorded the prior ELF hash `55188f5a…` unchanged after the failures.
- A later Docker build log, `gate-renode-image-docker.log`, records exit 0 and an ELF SHA-256 `e36aee30f381df32d9b085b11d616e35082a9650263e3b8614ed8f4004843a41`. `gate-renode-fresh.log` then records Renode PASS against an ELF with that hash and its mtime. This is materially newer evidence and appears to address the stale-ELF concern, but neither the results table nor README identifies it as the governing run, and there is no manifest/source-hash linkage from that ELF build to the exact reviewed candidate.
- The first Renode PASS cited by the results record therefore could not be treated as evidence of the newly rebuilt image.

**Re-review of supplied update and retained artifacts (2026-11-13):**

- `gate-renode-image-docker.log` now records a successful Docker image build (`RENODE_IMAGE_DOCKER_EXIT=0`) and the newly produced ELF identity: SHA-256 `e36aee30f381df32d9b085b11d616e35082a9650263e3b8614ed8f4004843a41`, size 4,133,912 bytes, mtime 2026-11-13 00:42:40 +0100.
- `renode-run/identity.txt` records that same ELF hash, mtime and size; the toolchain and Renode image digests are recorded there. `status.env` records container status `exited`, container exit 0 and `WAIT_RESULT=exited`.
- `gate-renode-fresh.log` records the fresh invocation at 2026-11-12T23:43:23Z and `RENODE_TEST: PASS`; `renode-process.log` and `uart-console.log` are retained under `renode-run/`. Their file hashes were directly computed during this re-review.
- The r3 README and verification-results now explicitly identify this as the fresh N18 run and mark the earlier `gate-renode.log` / ELF hash `55188f5a…` as invalid historical evidence, not a current-candidate pass.

**Revised assessment:** **The stale-ELF gap is adequately corrected for the narrow question of whether the PASS came from the old ELF.** The newly built ELF has a different identity and explicit successful build record, and the rerun identity/status/process records corroborate execution and PASS for that hash. The prior PASS is expressly superseded. This establishes a fresh emulated N18 run; it is not evidence of physical target, hardware timing, HIL, or flight qualification.

**Residual limitation:** exact source/configuration-to-ELF provenance still depends on the missing r3 `assets.sha256` manifest and the source-binding issue in PA-N20-R3-01. I do not independently infer from the ELF digest alone that every image build input matches the current source candidate. This residual does not reinstate the stale-ELF defect; it remains within PA-N20-R3-01.

**Disposition:** **Closed in substance for stale-ELF use/current-run identity; no longer a separate blocker.** Keep the fresh log/artifacts authoritative and the old PASS explicitly invalidated. Candidate source/configuration binding remains open under PA-N20-R3-01. No NC is closed by this review update.

## 4. Decision and outstanding review gates

**Decision:** For this bounded candidate only, the technical/test-spec correction is substantively conservative, and the fresh Renode record adequately corrects the stale-ELF issue. **Commit readiness is still not demonstrated** because PA-N20-R3-01 remains open: the r3 package's claimed `assets.sha256` is absent and the overall gate/source/artifact binding is incomplete. This report does not close PA-N20-R3-01 or any existing NC.

The candidate must also complete its separately applicable review loop: the initial R4 report is **CHANGES REQUIRED** and explicitly calls for correction re-review; the R1/R3 correction confirmation is pending. Those reports are evidence of the previous review state, not approval of this exact rev 1.12/r3 candidate. The project’s designated authorities decide whether all required review gates and findings are acceptably dispositioned.

Existing NCs were not independently closed or dispositioned here:

- **NC-EXT-240:** test target registration/oracle defect; OPEN, owner FSW, M17.
- **NC-EXT-241:** S1 payload interface/layout authority gap; OPEN, owner interface authority.
- **NC-EXT-242:** TS-1 row ordering; OPEN/deferred.
- **NC-EXT-243:** unsynchronized sequence counter; OPEN/deferred.
- **NC-EXT-244:** full pinned clang-format discrepancy; OPEN/deferred.

No additional product NC is proposed by this review; the two findings above concern candidate evidence integrity/currentness. No requirement or profile decision is made.

## 5. Uncertainty and confidence

```text
Decision/product and configuration: Commit readiness for uncommitted S8-PUS-FU S1 test/documentation delta; HEAD e5f01d4 plus candidate, hashes listed in §1.
Claim type: Facts (file contents, hashes, timestamps, log outcomes); Inference (assurance significance); Recommendation (evidence repair and gate hold).
Applicable baseline and approved tailoring: Repository norms N8/N11/N15/N19/N20/N22; no external standard clause compliance assessed.
Evidence consulted: Current source/docs, r3 logs and README, review index, PA-R2, V&V-R2, initial R4 and R1/R3 reports, sprint plan and loop audit; accessed 2026-11-13.
Cross-check and reconciliation: Direct directory listing and SHA-256; log chronology and explicit exit/status lines compared with README/results claims.
Confidence: High for the missing manifest, timestamps, stale cited log, and open prior-review states; Medium for the extent to which later Renode evidence can be candidate-bound, because the source/configuration binding is absent.
Basis and detection limits: Review was read-only apart from this report. Did not independently build/run CTest or Renode, inspect the complete build environment, or independently validate all case-table rows.
Missing/inaccessible evidence: r3/assets.sha256 is absent; exact candidate-source-to-N18-ELF linkage is absent; fresh R4 and R1/R3 correction re-reviews are pending.
Assumptions and expiry/recheck date: Findings apply to the observed worktree/evidence at review time; recheck after any evidence regeneration or candidate change.
Consequences if wrong: A stale or mismatched execution could be attributed to a candidate it did not verify, undermining the commit evidence record.
Required independent human reviewer/authority: Designated FSW commit authority and qualified human Software PA; R4 and R1/R3 reviewers for their correction gates.
```

## History

| Rev | Date | Author | Reason |
|---|---|---|---|
| 1.0 | 2026-11-13 | aeronautics-space-sw-pa-engineer profile | Fresh N20 review of the S1 ingestion-failure candidate; two Major evidence-integrity findings; no NC closure or commit/release authorization. |
| 1.1 | 2026-11-13 | aeronautics-space-sw-pa-engineer profile | Re-evaluated PA-N20-R3-02 after inspection of the fresh Docker-built ELF identity and retained Renode run artifacts. Stale-ELF gap closed in substance; source/artifact manifest finding PA-N20-R3-01 remains open and commit readiness remains unestablished. |

## Evidence update — r4 host gate package (2026-11-13)

The newly provided `r4/` package materially improves the host test evidence chain relative to r3. Its configure log records HEAD `e5f01d4…`, the candidate `test_pus.c` hash (`dc8ea3d…89c7`) and production `src/pus/pus.c` hash (`1eea40c…5401`) before configuring the named fresh build directory. The successful build log records the test hash before build; the ordered logs then show build exit 0, CTest 57/57, and the focused executable hash followed by `checks=322 failures=0`. `CMakeCache.txt` for that r4 build directory is retained. These artifacts provide substantially better direct candidate linkage for the host configure/build/test results and address the r3 chronology weakness for those gates.

**PA-N20-R3-01 status: partially corrected; still OPEN pending final integrity check.** No `r4/assets.sha256` exists yet, so the README's claimed complete hash binding cannot currently be verified. Also, `docs/vv/verification-results.md` still identifies the r3 package in its current S1 section while the r4 README declares r4 authoritative; the controlled results record must be reconciled to the final authoritative package. The r4 N18 evidence is copied from r3; the stale-ELF concern remains corrected by the fresh build/run evidence assessed under PA-N20-R3-02, but the final r4 manifest should bind the fresh Docker build log, exact ELF identity, Renode raw artifacts, and current candidate/document hashes. Reassess after manifest generation and the results-record update; until then, no final closure or commit-readiness recommendation is given.

| Rev | Date | Author | Reason |
|---|---|---|---|
| 1.2 | 2026-11-13 | aeronautics-space-sw-pa-engineer profile | Assessed r4 host configure/build/CTest/focused-test chronology as materially stronger candidate binding than r3. PA-N20-R3-01 remains open pending generated and verified manifest plus reconciliation of the controlled S1 results record from r3 to r4. |

## Final-manifest delta review — PA-N20-R3-01 (2026-11-13)

I independently reran from the repository root:

```text
sha256sum -c docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4/assets.sha256
```

**Result: PASS, 45/45 entries.** Every listed file reported `OK`, including the current `docs/status/loop-audit.md`, `docs/vv/verification-results.md`, the exact test and production source hashes, fresh r4 CMake cache and executable, all listed r4 gate logs, the fresh Renode ELF, all five raw `renode-run/` artifacts, the current-cycle review reports, and `docs/reviews/README.md`. The verification-results and loop-audit now use r4 for the current candidate and disclose the manifest/review status. The r4 raw Renode files are present and hash-verified. The manifest contains 45 paths and intentionally excludes itself and the r4 README, as the README says.

The earlier 43/45 result recorded in this report was against the preceding manifest revision and is superseded. The two document hashes then mismatched because those files had changed after that manifest was generated. This latest manifest has current hashes for both and verifies successfully.

### Disposition of PA-N20-R3-01

**Resolved in substance for the current r4 evidence set.** The original concerns are addressed by: (1) an ordered fresh host configure/build/CTest/focused-test run with candidate source identity recorded in the sequence; (2) a manifest that binds the current candidate, required evidence artifacts and current-cycle review records; (3) successful independent verification of every manifest entry; (4) current controlled-result and loop-audit pointers to r4; and (5) a separately supported fresh-ELF N18 run with the ELF identity and raw Renode records included in the verified manifest. This disposition is limited to PA-N20-R3-01 and does not qualify T3, close NC-EXT-240..244, approve a phase/release, or accept safety risk.

**Rebinding condition:** the manifest currently includes this report's prior hash. This requested disposition changes the report bytes, so the just-verified manifest will no longer match this report after this edit. Before treating the evidence set as final, regenerate the manifest to include this updated report (and any forthcoming correction-review reports/index or controlled-document updates), then independently rerun `sha256sum -c` from repository root. Any changes made after the new check require another rebind and verification. The remaining PA/N20 disposition is therefore no longer an open finding for the inspected manifest, but the *post-report-update final manifest verification* remains pending.

Other independent correction reviews/index rows identified as pending by the project remain separate gates. No conclusion about completion of those reviews is made here. No commit, release or flight authorization is given.

| Rev | Date | Author | Reason |
|---|---|---|---|
| 1.5 | 2026-11-13 | aeronautics-space-sw-pa-engineer profile | Final-manifest delta review: independently verified latest r4 manifest 45/45; resolved PA-N20-R3-01 in substance for the inspected evidence set. Report update invalidates its included hash, requiring one final manifest rebind/verification after all review outputs are finalized. |

## Evidence update — required Make targets (2026-11-13)

Reviewed the new r4 records for the required host Make targets. `gate-make-build-isolated.log` records `make build` with `BUILD_DIR=build/evidence-20261112-pus-s1-make` and `MAKE_BUILD_EXIT=0`; `gate-make-test-isolated.log` records `make test` against that isolated directory and `MAKE_TEST_EXIT=0` (CTest completes with exit 0). These provide successful execution evidence for both required targets using the non-conflicting isolated build directory.

The retained `gate-make-build.log` and `gate-make-test.log` show the default `build/native` attempts failed at CMake path validation because the existing cache was configured under `/workspace`; they do not indicate a build/test product failure. The reported preservation of the pre-existing root-owned cache is consistent with the recorded failure and isolated recovery approach. The deviation and successful override are disclosed in the r4 README/results/audit. I did not clean or modify the cache or independently rerun the targets.

**Manifest/currentness status:** the prior 45-entry manifest that I verified in the preceding review is no longer a valid checksum pass for the current tree. On recheck, `sha256sum -c` fails four entries: `docs/reviews/PA-R3-20261113-s8-pus-s1-ingestion-failure-review.md`, `docs/reviews/README.md`, `docs/status/loop-audit.md`, and `docs/vv/verification-results.md`. The newly added successful Make-target logs are also not among that manifest's entries. This is expected after the new evidence and record updates, but means the current enlarged r4 evidence set has not yet been integrity-verified. The host target execution claim is supported by its retained logs; final assurance binding is pending.

**Disposition:** PA-N20-R3-01 was resolved in substance for the earlier inspected r4 package, but final-current-package binding is **pending again** after this evidence/document/report delta. Regenerate the manifest over the final intended evidence set (including both successful isolated Make logs, failed default-attempt logs, updated documents/reports and current review index) and obtain an independent 45+-entry verification after all review outputs are incorporated. The isolated `make build` / `make test` evidence does not change NC status, tool qualification, or release/commit authority. No NC is closed.

| Rev | Date | Author | Reason |
|---|---|---|---|
| 1.6 | 2026-11-13 | aeronautics-space-sw-pa-engineer profile | Reviewed isolated required Make-target evidence (both exit 0) and recorded that the earlier manifest is stale against four modified files and does not include the newly added logs. Final-current-package rebinding/verification is pending. |

## Current-cycle review update — R1/R3 correction re-review (2026-11-13)

Considered `docs/reviews/SYS-R1R3-20261113-PUS-S1-INGEST-CORRECTION-REREVIEW.md` as current-cycle evidence. Its conclusion is **PASS WITH COMMENTS** for the focused correction: the Major coverage-attribution overclaim is resolved in substance by keeping `N22-TS1-23` as a gap/characterization case; `N22-TS1-104` remains a gap; totals remain 104 = 76 covered + 28 gap; the machine-readable and human-readable traceability qualifications are aligned. The report keeps policy/interface NCs open and does not turn its fresh N18 observation into hardware or release evidence. It also records the later synchronization confirmation for its two Minor comments. I find this disposition consistent with the candidate records inspected; it does not resolve unrelated N20 evidence-integrity status or the pending R4/V&V correction reviews.

The R1/R3 report's manifest-currentness observation matches my independent recheck: the present 45-entry manifest has four checksum failures (`PA-R3` report, review index, loop audit, verification results), and it does not include the newly added R1/R3 correction re-review report. Thus the previous finding disposition for the then-verified manifest remains valid for those exact bytes, but **final current-package binding is still pending**. Rebind after the R4/V&V reports/index and all controlled records are final, include this PA report and the R1/R3 correction report, then independently verify all entries.

| Rev | Date | Author | Reason |
|---|---|---|---|
| 1.7 | 2026-11-13 | aeronautics-space-sw-pa-engineer profile | Recorded the R1/R3 correction re-review as a current-cycle PASS WITH COMMENTS, agreeing with its bounded Major correction disposition; final manifest remains stale and does not yet include that new report. |

## Evidence persistence update — retained executable artifacts (2026-11-13)

Inspected the new r4 `artifacts/` files and `artifacts/identities.txt` and independently hashed the files. Results match the stated identities:

| Retained artifact | SHA-256 | Cross-check |
|---|---|---|
| `r4/artifacts/test_pus` | `4be7c2b1e12fd50b20e057096b4fe68a01acd0ceea57c4cfc82d26eed1997ddb` | Matches `build/evidence-20261112-pus-s1-r4/tests/test_pus` and the focused-test log identity. |
| `r4/artifacts/frick_fsw_renode.elf` | `e36aee30f381df32d9b085b11d616e35082a9650263e3b8614ed8f4004843a41` | Matches `build/renode/frick_fsw_renode.elf` and the fresh Renode identity/build records. |

These retained copies address the durability gap for the exact focused `test_pus` executable and the fresh Renode ELF, avoiding dependence on their gitignored `build/` originals for later byte-level identity checks. `artifacts/identities.txt` documents the copy sources and hashes. The r4 CMake cache is also retained under the evidence package.

**Residual for full CTest:** this does not persist the complete set of registered CTest executables. The r4 build directory currently contains the suite binaries, but those are under ignored `build/`; the retained evidence package does not yet contain copies or a complete per-executable SHA-256 inventory tied to the 57/57 CTest run. The CTest transcript and build output establish the reported run and targets, but without retained hashes/copies they do not preserve byte identity for every executable used. For a durable full-suite identity chain, add a hash inventory captured from the exact r4 build directory after the successful build and before CTest (or retain the binaries/archive if repository policy and size permit), and bind that inventory plus `gate-ctest.log` in the final manifest. This is a bounded evidence-completeness residual; it does not invalidate the focused-test or Renode identities.

The current final manifest is still pending as stated by the owner; I have not assessed whether its next revision includes these new artifacts or the full-CTest executable inventory. Because this report has changed and is intended to be manifest-bound, regenerate and independently verify the manifest after the report, review index and remaining R4/V&V correction reports are final.

| Rev | Date | Author | Reason |
|---|---|---|---|
| 1.8 | 2026-11-13 | aeronautics-space-sw-pa-engineer profile | Confirmed retained focused-test and Renode binary hashes against their build outputs; recorded residual absence of a complete persistent CTest executable identity inventory and deferred final manifest assessment. |

## Evidence persistence delta review — complete CTest executable inventory (2026-11-13)

Inspected `artifacts/ctest-list.json`, `artifacts/ctest-binaries.sha256`, `artifacts/ctest-inventory-summary.txt`, and the retained `artifacts/ctest-binaries/` set.

- `ctest-list.json` contains 57 registered CTest entries. Its command list has 55 distinct compiled-test executable paths plus the non-compiled `python3` and `sh` commands.
- Exactly 55 retained executable files are present in `artifacts/ctest-binaries/`; their names match the compiled executable set (no missing or extra binary copies).
- Running `sha256sum -c ctest-binaries.sha256` from `r4/artifacts/` reports **55/55 OK**.
- Independently compared each retained copy's SHA-256 to the corresponding executable in `build/evidence-20261112-pus-s1-r4/tests/`; all 55 match. The CTest inventory's executable paths point to that same r4 build directory.
- The package README, verification results and loop audit now disclose that the CTest executable inventory is retained. The separate focused `test_pus` and fresh Renode ELF copies remain present with the previously checked hashes.

**Disposition:** the prior residual concern about durability/identity of the complete compiled CTest executable set is **resolved in substance** for this r4 run. The command list appropriately distinguishes 57 CTest cases from 55 compiled binaries; the shell/Python invocations do not require additional compiled-product copies. This closes only the persistence/identity residual, not N10 tool qualification or the validity of every test oracle.

The package-level `assets.sha256` remains pending/stale as the owner states; this review did not treat it as verified. Include the CTest JSON, per-executable hash list, summary, and all 55 binaries in the eventual package manifest and independently check after all review/index/document updates. This report has changed and remains a manifest input, so regenerate/reverify after this update.

| Rev | Date | Author | Reason |
|---|---|---|---|
| 1.9 | 2026-11-13 | aeronautics-space-sw-pa-engineer profile | Verified the 57-case CTest inventory maps to 55 retained compiled executables; per-binary manifest passes 55/55 and copies hash-match the r4 build. Resolved the previously recorded CTest binary persistence residual; package-level manifest remains pending. |
