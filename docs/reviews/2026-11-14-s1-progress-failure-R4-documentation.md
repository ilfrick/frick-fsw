# R4 INDEPENDENT REVIEW REPORT — S1 progress-failure emission (NC-EXT-94) and documentation tracking corrections

| Field | Value |
| --- | --- |
| Review ID | FRICK-REVIEW-2026-11-14-S1-PROGRESS-R4 |
| Norm / profile | N17 (documentation review), R4 — Senior Aeronautics and Space Software Documentation Engineer |
| Date | 2026-11-14 |
| Reviewer | Sub-agent, R4 documentation profile (see independence caveat) |
| Candidate | Uncommitted working tree on committed HEAD `fc68b9d121bce2194c1beaaa49a7dc4cdab0552c` |
| Mode | Read-only. No file modified except this report. No commit, push, Docker run or deletion performed. |
| Verdict | **NOT PASSED — 6 Major + 12 Minor findings; corrections and re-review required** |

## Independence caveat (binding, read first)

This is **contextual independence only** (separate session, no shared working memory), **not organisational independence**. The reviewer is an AI assistant with no delegated engineering, safety, certification, records, legal or release authority. This report:

- does **not** close, downgrade or disposition any NC (NC-EXT-43/44/45/94/95/244/245/246 remain as their owners recorded them);
- does **not** approve a baseline, a release, a gate, or a compliance/qualification claim;
- is **not** evidence of product assurance (N20) or of any R1/R2/R3 review;
- is valid only for the exact candidate hashes below. Any subsequent edit invalidates it.

### Binding candidate hashes (verified with `sha256sum` before review — all nine matched)

```
195b8cf5754e7b4043f8b62bcc0c47d06ff2e10e67cdc2ef2ad054dd8c22293b  src/pus/pus.c
33b43b961616b3b5a91925c75982054e07bda8b9b23fd57707ae9a30f3b42a13  tests/unit/test_pus_s11_residual.c
a35e9ea4dead23f3bc29714e62c719699fb6b1df1756ec780d1c7d670de8a4d7  CHANGELOG.md
5b119888eb176e10747459021b54bd5605fc1adf44f8d689bf018e1caa5de9b6  README.md
27a42bd3568a605fedc6b2e8bbad74782ee9a0b5a273b4bcc97ddc577a29985d  docs/planning/sprint-plan.md
ac374ec65e39069cfba39b9f5db0844e0bf507e716c457d9be837e1f4767cc11  docs/status/handover.md
c36a9e645c8795ed008010fc98d1f5d5e16af83dc501922a0934f7cb8d4b2b41  docs/status/loop-audit.md
7ceaa4b5bf403fa72fb3b25cd1fa83e6e1a59f0e2cdfcf7fcd915f649bacce95  docs/vv/test-spec/ts-1-protocol-core.md
8ab0fab7f48b5ded9ffd065bc6124e3f963200d548877ae1e7be6edc010450bd  docs/vv/verification-results.md
```

HEAD verified as `fc68b9d121bce2194c1beaaa49a7dc4cdab0552c`; `origin/main` and `gitea/main` also resolve to `fc68b9d`.

## Scope

| Reviewed | Excluded |
| --- | --- |
| Documentation delta: `CHANGELOG.md` (new §S1 entry), `README.md` (Service-1 row), `docs/planning/sprint-plan.md` (NC-EXT-43/94/95/245/246 rows, history 1.153/1.154), `docs/status/handover.md` (status block, "Still open", history 3.95/3.96), `docs/status/loop-audit.md` (baseline header, M12 row, §13 N19 assessment, N1–N22 rows, history 11.05–11.07), `docs/vv/test-spec/ts-1-protocol-core.md` (§2 range, N22-TS1-105, history 1.14), `docs/vv/verification-results.md` (new §S1 section, gate-limitations block) | Functional adequacy of the `FF_PusIngest` change (R1/R3, V&V); test-oracle technical validity beyond documentation consistency (R2); PA/N20; safety; security; licensing disposition; ICD ownership decisions |
| Documentation↔artefact consistency: every numeric, revision, commit, gate and NC claim in the delta re-derived from the repository, the retained evidence directory and `git` | Whether the ICD §4 S1 row, the NC dispositions or the interface profile are *correct* engineering decisions (interface authority) |
| English language (N2), overclaiming, limitation honesty, revision-history completeness (N16) | Code style, MISRA, coverage targets |

Reference artefacts used for cross-check (read-only): `git show HEAD:<file>`, `git log`, `git status`, `docs/interfaces/ICD.md`, `docs/architecture/pus-c-from-scratch-assessment.md`, `docs/requirements/traceability.yaml`, `docs/requirements/SRS.md`, `src/pus/pus_services.c`, `src/pus/pus.c`, `verification/lean/Frick/Pus.lean`, `docs/vv/evidence/2026-11-14-s1-progress-failure/*`, and the already-built test binary `build/doc-update/tests/test_pus_s11_residual` (executed read-only to re-derive the check count).

## Answers to the seven review questions (summary)

| Q | Answer |
| --- | --- |
| 1. Are the listed factual claims true? | **Partly.** True: NC-EXT-246 = N18 Docker deviation; the `vm.c` finding (tracked, `git diff HEAD` empty, last changed `b9681e8`, MIT notice retained); NC-EXT-94/95/245/246 OPEN; revision numbers 3.96 / 11.07 / 1.154 / 1.14; build/CTest/Lean/trace/smoke/gsw outcomes; format-check not passed. **False or unsupported:** executed check count (`1176` vs actual `1206`), TS-1 covered/gap split (`76/29` vs actual `77/28`), "§3.1 totals updated" (not updated), the SRS-REQ-006 traceability premise, the format-gate extent ("three files" vs nine), and the implied N18 disposition. See F-01…F-06. |
| 2. Is the NC-EXT-94 verbatim preservation real? | **Yes at cell-content level.** Programmatic check: HEAD cell body (row minus the trailing table `|`) is an exact prefix of the current cell body; the appended tail begins `**Implementation status appended (2026-11-14):**`. A byte-exact test *including* the table terminator necessarily fails (the closing pipe moved to the end of the cell). **Caveat:** the preserved text now contradicts the appended status (F-10). |
| 3. Are revision histories complete and correctly ordered? | **Complete and ordered** (all four documents carry a row for the change; ascending order preserved), but **two rows describe work that was not done or done differently** (F-01, F-02) and one NC row destroys rather than appends history (F-07). |
| 4. Internal consistency? | **No.** Five documents carry the same wrong TS-1 split; two documents disagree on the baseline commit; two documents disagree on which third file fails the formatter; one cell contradicts itself on the stage→subservice map. |
| 5. Limitations/open gates honest? | **Mostly yes, with one material exception.** smoke, gsw-selfcheck, format-check, T3, NC-EXT-245/246 and the UNVERIFIED reviews are stated honestly and repeatedly. N18 is the exception (F-06). "clean baseline" is correctly withdrawn (loop-audit:9). |
| 6. English / overclaiming? | **English throughout (N2) satisfied.** No "certified", "qualified", "compliant", "flight-proven" or "deterministic" claim was found in the added lines; all hits are explicit disclaimers. Overclaiming occurs only in the numeric/revision statements (F-01…F-06). |
| 7. Do evidence references exist? | **Yes.** All named evidence files exist; see the directory cross-check below. One wording defect: the directory is called "repo-committed" but is untracked (F-12). |

### Evidence-directory cross-check (`docs/vv/evidence/2026-11-14-s1-progress-failure/`)

Present (15 entries): `build.log`, `ctest.log`, `docker-logs.stderr` (0 bytes), `focused-pus.log`, `format-changed-files.log`, `format-full.log`, `gsw.log`, `lean.log`, `renode-process.log`, `smoke-safe.log`, `smoke.log`, `status.env`, `trace.log`, `uart-console.log`.

Referenced by the delta and verified present: `smoke-safe.log` (handover:23, CHANGELOG:88, verification-results:1001). Directory referenced at loop-audit:447, verification-results:1011, sprint-plan:333, handover:25. `RENODE_TEST: PASS` found in `uart-console.log`; `CONTAINER_EXIT=0` in `status.env`; `100% tests passed, 0 tests failed out of 60` in `ctest.log`; `BUILD_EXIT=0`, `LEAN_EXIT=0`, `FOCUSED_EXIT=0`, `FORMAT_CHANGED_EXIT=0`, `GSW_EXIT=2`, `SMOKE_EXIT=2`, `FORMAT_FULL_EXIT=2`. **No file referenced by name is missing.**

## Findings

### Major

| ID | Severity | Finding | Location |
| --- | --- | --- | --- |
| **F-01** | **Major** | **TS-1 §3.1 was not updated, and history row 1.14 says it was.** §3.1 still reads `Total cases 104`, `covered 76`, `gap 28`, while §2 states the range `N22-TS1-1 … N22-TS1-105` and the case table contains 105 rows. Parsing all 105 status cells gives **77 covered-family + 28 gap**. History row 1.14 asserts "§3.1 totals updated (105 cases = 76 `covered` + 29 `gap`)" — a false statement about work performed. Adding one `host covered` row to a 76/28 baseline yields 77/28, not 76/29. | `ts-1-protocol-core.md:293,297,298` (§3.1) vs `:87` (§2 range) and `:349` (history 1.14) |
| **F-02** | **Major** | **The wrong 76/29 split is propagated to four further documents.** Every occurrence must become `105 cases = 77 covered + 28 gap` (or the split re-derived after §3.1 is fixed). | `ts-1-protocol-core.md:349`; `verification-results.md:993-994`; `loop-audit.md:458` (N22), `:802` (N22 addendum); `handover.md:1899` (history 3.95) |
| **F-03** | **Major** | **The traceability justification is factually wrong, and it suppresses a real N3/N4 gap.** The delta states "SRS-REQ-006 already lists `tests/unit/test_pus_s11_residual.c` (line 106 of traceability.yaml), so no traceability mapping change was required". `traceability.yaml:106` is **SRS-REQ-008**'s `tests` list; **SRS-REQ-006** (`traceability.yaml:85-90`) lists only `tests/unit/test_pus.c` and `tests/unit/test_r3_gap.c`. The new oracle `N22-TS1-105` is linked to `SRS-REQ-006` in TS-1 and implemented by `test_pus_s11_residual.c`, so the manifest trace SRS-REQ-006 → test is **absent** and N4 bidirectional traceability is incomplete for this change. `make trace` passing does not detect this (it validates manifest→code/test presence, not TS-1-case coverage). SRS-REQ-006 ("every acknowledged stage (acceptance/start/progress/completion) … exactly one service 1 telemetry packet", `SRS.md:65-69`) is exactly the requirement the new case exercises. | `verification-results.md:985-990`; `loop-audit.md:440` (N3/N4 row); `traceability.yaml:89-90`, `:106` |
| **F-04** | **Major** | **The executed check count is stale and cannot describe the reviewed tree.** The delta reports `checks=1176`. The reviewed test source adds 28 unconditional `FF_TEST_ASSERT` statements over HEAD (122 → 150). Running the already-built binary (mtime 12:44, source mtime 12:21, i.e. built from the reviewed source) reports `checks=1206 failures=0`. `1176` is the previous cycle's figure carried forward. No retained log in the evidence directory records any check count, so the number was not evidenced at all. | `verification-results.md:1050`; `loop-audit.md:795` (N15) |
| **F-05** | **Major** | **The failed format gate is materially understated.** The delta says the full pinned check "fails on three *unchanged* initializer files" (`link_tsn.c:18`, `link_ip.c:19`, `test_pus_s18.c:347/351/352`). The retained log `format-full.log` lists **nine** files: `tests/unit/test_pus_s18.c` (11 violations), `src/obcp/ff_obcp_inhouse.c` (8), `tests/unit/test_obcp_budget.c` (5), `tests/unit/test_obcp_inhouse.c` (3), `tests/unit/test_link_tsn.c` (3), `src/transport/link/link_tsn.c` (3), `src/transport/link/link_ip.c` (3), `tests/unit/test_obcp_abort.c` (2), `src/pus/pus_services.c:753` (1). Two of them are production sources, and none is an "initializer". The third file also contradicts NC-EXT-244, which names `tests/unit/test_link_tsn.c:24`. The recorded exit code also disagrees: the text says "exit 123" while the retained log records `FORMAT_FULL_EXIT=2` (123 is the inner `make` error). | `verification-results.md:1003`; `handover.md:66-67`; `sprint-plan.md:331` (NC-EXT-244) vs `evidence/…/format-full.log` |
| **F-06** | **Major** | **N18 is presented in a passing state although the retained run cannot cover this change.** All three status documents record "N18 Renode functional PASS retained from prior run". The retained run is timestamped **11:26:43–11:26:56** (`renode-process.log`), while `src/pus/pus.c` was last written at **12:21:53** — the run predates the change it would have to validate. No ELF sha256, build identity or RTEMS image digest is retained for this cycle (the previous cycle retained one). Under N18 ("functional validation of every change"), the honest disposition is "N18 not satisfied for this change"; instead the N1–N22 outcome table shows a PASS row next to rows that explicitly say "not claimed passed". | `loop-audit.md:454`; `handover.md:25`; `CHANGELOG.md:84-85` |

### Minor

| ID | Severity | Finding | Location |
| --- | --- | --- | --- |
| **F-07** | Minor | **NC-EXT-95 history was overwritten, not appended.** NC-EXT-94 is correctly preserved verbatim with an appended status; NC-EXT-95's *problem statement* was replaced ("NO PRODUCTION CODE MISSING: …"), so the original finding text is no longer in the ledger (only in git). Two adjacent NCs in the same delta treated inconsistently; N16/history integrity expects append-and-annotate. The replacement text is factually correct (verified: `ICD.md:123` is the TC[20,1] row; `svc20_set` at `pus_services.c:635`, registered at `:834`; `test_param_set_bounded` exists). | `sprint-plan.md:233` vs `git show HEAD:docs/planning/sprint-plan.md` row `NC-EXT-95` |
| **F-08** | Minor | **Two controlled documents are cited at a revision their own headers do not declare.** Five passages cite "ICD rev 1.9 §4", but `docs/interfaces/ICD.md` header still reads `Revision 1.8` / `Date 2026-11-13` (only its history row `1.9` exists, at `:255`). Two passages cite "`pus-c-from-scratch-assessment.md` rev 0.4", whose header reads `Revision 0.3` / `Date 2026-10-19` (history row `0.4` at `:284`). Root cause is committed HEAD `fc68b9d` (history rows added without header bumps), but this delta asserts the un-declared revisions as authority for the emission. | `ICD.md:6-7`,`:255`; `pus-c-from-scratch-assessment.md:7-8`,`:284`; citations `CHANGELOG.md:59,61`; `loop-audit.md:9,422`; `verification-results.md:1016,1018`; `sprint-plan.md:233` |
| **F-09** | Minor | **Handover names the wrong base commit.** "base committed and pushed … (commit `b9681e8`)" — the actual committed HEAD (also pushed to `origin/main` and `gitea/main`) is `fc68b9d`. `loop-audit.md:9` states `fc68b9d` correctly, so the two status documents contradict each other on baseline identity. `b9681e8` is two commits behind. | `handover.md:18` vs `loop-audit.md:9` |
| **F-10** | Minor | **The preserved NC-EXT-94 text now contradicts the appended status inside the same cell.** Preserved text: "maps `FF_PUS_STAGE_PROGRESS` to subservice 5 only" / "subservice 6 is not mapped at all". Appended text: "the stage→subservice map already covers PROGRESS→{5,6}". The map was **already** `success ? 5u : 6u` at HEAD (`pus.c:313-315`), so the preserved mechanism description was inaccurate even when written; the true defect was the hard-coded `success=1u` call site. Verbatim preservation is correct, but a one-line reconciliation note is missing, leaving a reader with two mutually exclusive statements. | `sprint-plan.md:232`; `src/pus/pus.c:313-315` |
| **F-11** | Minor | **The N22-TS1-105 oracle rationale misattributes the error source.** The row says "handler `FF_TimeSetUtcNs` returns `FF_BAD_SIZE`". TC[9,1] dispatches to `svc9_set_time`, which returns `FF_BAD_SIZE` at its own `len != 6u` guard *before* calling `FF_TimeSetUtcNs`; the latter is never reached for a 7-octet payload. The oracle (one TM[1,6], code 20) is unaffected; the explanation is. | `ts-1-protocol-core.md:147`; `src/pus/pus_services.c:586-588`,`:822` |
| **F-12** | Minor | **"repo-committed evidence directory" is not committed.** The directory is untracked in the working tree (`git status`: `?? docs/vv/evidence/2026-11-14-s1-progress-failure/`). N11 requires retention *and* publication with the compliant change; describing it as committed pre-empts that gate. | `verification-results.md:1011` |
| **F-13** | Minor | **The new S1 material is inserted inside the previous section.** The traceability paragraph and the entire "Honest gate status for this cycle" block (`:985-1009`) sit under the `## ICD §4 S3/S6 dispatch-contract correction — 2026-11-13` heading; the new `## S1 progress-failure emission (NC-EXT-94) — 2026-11-14` heading only starts at `:1013`. Readers will attribute the smoke/GSW/format failures and the "UNVERIFIED" review statements to the 2026-11-13 ICD cycle. The lead-in at `:1011` also ends with a colon but is followed by a heading, not the promised list. | `verification-results.md:985-1013` |
| **F-14** | Minor | **The "7/7" focused-test enumeration names only four of the seven tests.** `focused-pus.log` runs `pus`, `pus_fu`, `pus_s11_residual`, `pus_n7_adapter_contract`, `pusopen_adapter_contract`, `pus_event_identity`, `decoder_corpus`. Naming four and reporting "7/7" invites a miscount; either list all seven or state "7 CTest entries matching the filter". | `verification-results.md:1049`; `loop-audit.md:795` |
| **F-15** | Minor | **"Configure exit 0" and "0 compiler warnings" are not supported by the retained logs.** `build.log` contains only `[ nn%] Built target …` lines — no `Building C object`, no configure output — so it is an up-to-date incremental invocation and cannot demonstrate that the changed files compiled warning-free. No configure log exists in the evidence directory. (The subsequent `-Werror` build that produced the 12:44 binary presumably did compile, but that invocation was not retained.) | `verification-results.md:1046-1047`; `handover.md:21`; `evidence/…/build.log` |
| **F-16** | Minor | **N1 is dispositioned "N/A" without recording the formal-model coverage gap.** The Lean model's `verificationCount` counts acceptance/start/completion and **omits the progress stage entirely**, and models no success/failure subservice selection; its own header comment claims "service 1 verification is emitted for every acknowledged stage". "N/A — no Lean requirement theorems changed" is literally true but implies no N1/N3 obligation; the changed behaviour has no formal model, and the gap is not recorded as an NC/gap with owner. | `loop-audit.md:438`; `verification/lean/Frick/Pus.lean:5-6`,`:31-35` |
| **F-17** | Minor | **TS-1 §2 range and group enumeration disagree.** The range was advanced to `…N22-TS1-105`, but the group enumeration still ends at "`104` PUS S1 ack = 0 variants …". The new case is not described in the group list. | `ts-1-protocol-core.md:90` vs `:96-97` |
| **F-18** | Minor | **The README status panel was not refreshed.** It still states "last updated 2026-11-13" although `README.md` changed in this cycle and the panel's own note says it is "refreshed every cycle (norms N8/N11/N19)". `make metrics` was not run (consistent with the gsw-selfcheck failure) but the limitation is not stated. | `README.md:54` |

### Observations (no action required for this cycle; record or track at will)

| ID | Observation |
| --- | --- |
| O-1 | `format-changed-files.log` contains a single line, `FORMAT_CHANGED_EXIT=0`. No command line, no `clang-format --version`, no per-file output. The claim "pinned clang-format 18 PASS on the two changed files" therefore rests on an exit marker alone — weak for N11 evidence. Recommended: retain the command line and version banner. |
| O-2 | A Docker container (`frick-vv:local`) was used for the changed-file format check in the same cycle in which NC-EXT-246 records a Docker preflight blocker. The blocker is scoped to the N18 Renode runner, but the format-check invocation's preflight status is not recorded. |
| O-3 | `loop-audit.md:482` (history 11.06) records NC-EXT-246 as the licensing NC and is explicitly marked "**Superseded by 11.07**". Acceptable as history, but the same table now contains both allocations of the identifier; a reader scanning the history alone can be misled. |
| O-4 | Three history rows (11.05/11.06/11.07) were added for one uncommitted cycle and the header jumped 11.05 → 11.07. This is disclosed and internally consistent, but revision 11.06 never existed as a published state. |
| O-5 | `ICD.md:255` (history 1.9) states "Wire behaviour changed: TM[1,6] is now emitted" — at committed HEAD the emission did not exist (the code change is uncommitted). This delta discloses the uncommitted state (`loop-audit.md:9`), but the committed ICD text itself overstates implemented behaviour at HEAD. Interface-authority disposition territory; noted, not decided here. |
| O-6 | `loop-audit.md:484` contains a stray duplicate table separator (`| --- | ---------- | --------- | ------ |`) inside the history table — pre-existing, not introduced here. |
| O-7 | `tests/unit/test_pus_s11_residual.c` has no trailing newline — pre-existing, not introduced here. |
| O-8 | The `ICD.md` §4 S1 row now correctly splits subservices 4 (`reserved, not emitted`) and 6 (`progress failure`), and the README Service-1 row (`README.md:184`) correctly describes the new emission including the absence of a start-failure path. Both are consistent with `src/pus/pus.c`. |

## Verified-true claims (for the record)

The following claims in the change description were independently confirmed and are **not** findings:

1. **NC-EXT-246 = N18 Docker runner preflight/cleanup deviation** — consistently allocated in all current rows (`sprint-plan.md:333`, `loop-audit.md:454`, `handover.md:25,70`, `verification-results.md:1006`); no current row associates it with licensing.
2. **`vm.c` SPDX observation** — `tools/patches/n7s-obcp/src/modifications/py/vm.c` is tracked; `git diff HEAD` is empty; `git log -1` gives `b9681e8`; the file contains the upstream MicroPython MIT notice and zero `SPDX` occurrences; `tools/patches/n7s-obcp/README.md:12` declares `SPDX-License-Identifier: MPL-2.0 (project-owned patches; …)`; `docs/quality/licensing.md` §2 (line 58) is the external-components section. Appended to NC-EXT-43 and cross-referenced to NC-EXT-44 as described.
3. **NC-EXT-94/95/245/246 remain OPEN** — all four rows read `OPEN`/`REMAINS OPEN`; none is marked closed or corrected-to-closed.
4. **Revision numbers** — handover `3.96` (`:7`), loop-audit `11.07` (`:7`), sprint-plan `1.154` (`:7`), TS-1 `1.14` (`:7`); each has a history row for the change (`handover.md:1899-1900`, `loop-audit.md:481-483`, `sprint-plan.md:739-740`, `ts-1:349`).
5. **Gate outcomes** — `BUILD_EXIT=0`, CTest `60/60` (`100% tests passed, 0 tests failed out of 60`), focused `7/7` (`FOCUSED_EXIT=0`), `make verify-lean` PASS (`LEAN_EXIT=0`), `make trace` PASS (`requirements in manifest: 120`, `srs=76 sss=30 gap=14`, `lean theorems found: 68`), changed-file format `FORMAT_CHANGED_EXIT=0`, default `make smoke` FAILED with the documented CMake `/workspace` vs local path mismatch (`SMOKE_EXIT=2`), `BUILD_DIR=build/smoke-safe` PASSED (`[smoke] PASS (banner + 7 applications registered)`, `SMOKE_SAFE_EXIT=0`), `make gsw-selfcheck` FAILED on exactly `tools/patches/n7s-obcp/src/modifications/py/vm.c: missing MPL-2.0 SPDX header` (`GSW_EXIT=2`), full `make format-check` NOT passed.
6. **NC-EXT-94 verbatim preservation** — cell-body prefix match confirmed programmatically (see Q2).
7. **Uncommitted state** — the S1 source/test edits are uncommitted; `fc68b9d` touched only `docs/interfaces/ICD.md` and `docs/architecture/pus-c-from-scratch-assessment.md` (no source code), exactly as `loop-audit.md:9` states.
8. **No "clean baseline" claim** — the adjective was withdrawn and start-of-loop cleanliness is marked UNVERIFIED (`loop-audit.md:9`), with the deviation registered as NC-EXT-245.
9. **English (N2)** — all added prose in the nine files is English. The only non-English text in the tree is `make`'s Italian locale output inside retained logs (`Errore 1`), which is not document content.
10. **README Service-1 row** — consistent with the implementation and with the ICD S1 rows.

## Verdict

**NOT PASSED.** The change is documentation-honest in tone and unusually careful about open gates, but it fails N17 on substance:

- two coverage figures (executed checks, TS-1 covered/gap split) are wrong and propagated across five documents;
- one revision history claims an edit that was not made (TS-1 §3.1);
- the traceability justification rests on a misread manifest line and suppresses a genuine N3/N4 gap for `N22-TS1-105` ↔ `SRS-REQ-006`;
- the failed format gate is described as three files when the retained evidence shows nine;
- N18 is shown as a PASS row although the retained Renode run predates the change and carries no ELF identity.

Under N12 these must be corrected by re-applying the rules to full compliance, not by adding disclaimers. Recommended correction set (all documentation-only; no source/test change needed for F-01…F-18 except the traceability manifest entry required by F-03):

1. F-01/F-02: fix TS-1 §3.1 (105 total, 77 covered, 28 gap, mixed 6) and every propagated figure; correct history row 1.14's claim.
2. F-03: add `tests/unit/test_pus_s11_residual.c` to the `SRS-REQ-006` `tests` list in `traceability.yaml` and the mirror (N3/N4), then re-run `make trace`; or record an explicit NC if the manifest change is deferred.
3. F-04: replace `1176` with the re-derived count from a retained log (retain the actual `checks=` line).
4. F-05: restate the format gate from `format-full.log` (nine files, 37 violations) and reconcile NC-EXT-244's file list and exit code.
5. F-06: disposition N18 for this change explicitly as "not validated in Renode for this change" (or re-run under the approved preflight once NC-EXT-246 is cleared).
6. F-07…F-18: bounded text corrections in the same pass.

A re-review is required on the corrected tree. This report does not close any NC and does not authorise commit, release or any compliance statement.

## Residual risks

| Risk | Consequence | Owner |
| --- | --- | --- |
| Coverage numbers (checks, TS-1 split) are wrong in five controlled records | Downstream package attestations, manifests and review reports that copy these figures inherit the error; a future reader cannot distinguish a stale number from a measured one | FSW engineer / TS-1 author |
| `SRS-REQ-006` has no manifest trace to the test that now provides its progress-failure oracle | N4 bidirectional traceability is incomplete for the change; a future `trace.py`-green state will not reveal it; R3/N14 review would likely raise it | FSW engineer + requirements authority |
| N18 is recorded in a passing posture for a change it does not cover | A reader may conclude the S1 change is Renode-validated; NC-EXT-246 already blocks future Docker use, so the gate may stay open across several cycles | FSW engineer; authorised human for gate disposition |
| ICD §4 and the assessment are cited at revisions their headers do not declare (1.9 / 0.4 vs 1.8 / 0.3) | Interface-authority disposition of NC-EXT-94/95 will reference a revision that does not formally exist; a later header bump will not match the citations made here | Interface authority / document owner |
| The preserved NC-EXT-94 mechanism description contradicts the implementation | A reader trying to understand *why* TM[1,6] was absent may conclude the map changed in this delta, which is false | FSW engineer (add a reconciliation note) |
| Formal model (Lean) does not cover progress emission or the 5/6 selection | N1/N3 coverage claim is weaker than the N1 "N/A" row suggests; a future refactor could regress the emission undetected by the formal layer | FSW engineer / formal-methods owner |
| Evidence quality (bare exit markers, no configure log, no ELF identity) | Gates that pass cannot be re-derived from retained artefacts alone; an auditor must re-run the build | V&V tool owner (N10) |

---

*End of report. Reviewer: R4 documentation profile (contextual independence only). This report is review evidence, not approval. NCs are not closed by it.*
