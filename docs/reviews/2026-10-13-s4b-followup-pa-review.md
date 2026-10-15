# Review Report

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0169                      |
| Reviewer session                  | independent agent (norm N9/N20), fresh context |
| Reviewer profile                  | **PA** — Senior SW Product Assurance engineer |
| Reference revision                | S4b **follow-up** — uncommitted working-tree increment (12 files) atop `57974d2`; the S4b link backends were committed at `e65fc1b` |
| Items under review                | `tests/unit/test_link_ip.c`, `tests/unit/test_link_tsn.c`; `CHANGELOG.md`, `README.md`; `docs/planning/sprint-plan.md`; `docs/requirements/traceability.md`, `docs/requirements/traceability.yaml`; `docs/quality/class-a-gaps.md`; `docs/status/loop-audit.md`, `docs/status/status.json`; `docs/vv/tool-qualification.md`, `docs/vv/verification-results.md` |
| Date                              | 2026-10-13                             |
| Scope                             | Independent **Product-Assurance review of every phase product** (norm N20) of the "S4b follow-up" increment (NC-S4b-01..05 closure + host `pus_ip`/`pus_tsn` PUS-C TM/TC demonstration). The review (a) confirms the two new test functions and their PUS-C round-trip demonstration by first-hand source read; (b) independently re-executes every gate the increment claims (build, ctest, trace, metrics, clang-format, `-Wall -Wextra -Werror`); (c) audits the engineering data set touched by the increment (CHANGELOG, README metrics, sprint-plan, traceability, class-a-gaps, status, verification-results, tool-qualification) for **evidence-truthfulness, record conformance, revision history and NC control**; and (d) checks scope/claim conformance vs ECSS-Q-ST-80C Rev.2 (records, tool qualification, NC control, independence) and the project norms N1–N20. Assessed against ECSS-E-ST-40C, ECSS-Q-ST-80C Rev.2, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2 and `docs/rules.md` (rev 1.4). |
| Independence declaration          | The reviewer had **no part** in authoring, building, or previously reviewing the items under review; this review ran in a fresh context and every declared claim was read first-hand from the source/test/document files or confirmed by re-running the referenced gate. The reviewer wrote exactly one file — this report — and modified no other repository file (no source, test, doc or config file was touched; gate runs regenerated only git-ignored `build/` artifacts). |
| Norms applied                     | N1–N20 (see `docs/rules.md`)           |

**Point-in-time counts (audit D1).** The counts recorded in this report (e.g.
`test_link_ip` 77 checks) are the values the reviewer measured at review time,
before the `-Wall -Wextra -Werror` corrective delta added the IP send-leg and
PUS-TM-sink assertions; the current values are `test_link_ip` 20 funcs / **79**
checks and `test_time` 99 checks. A review report is an immutable record of the
state it examined and is deliberately not retro-edited; the current counts are
carried by `docs/vv/verification-results.md` (rev 0.45+) and by the delta
re-review `FRICK-REVIEW-0169-D1`.

## 1. Scope

**In scope (verified first-hand):**

- **New test evidence (the increment's primary product).** `tests/unit/test_link_ip.c` — new `test_pus_ip_roundtrip` (PUS-C S17 ping TC received over the IP `FF_Link_t` backend, ingested by PUS, TM reply pushed back over the same link); `tests/unit/test_link_tsn.c` — new `test_pus_tsn_roundtrip` (same over the TSN backend, mock MAC looped back). Called-counts, check-counts, and the `Dem` claim are verified by reading the files and running the tests.
- **Independent re-execution of every claimed gate:** `cmake --build build/native` (PASS); `ctest` (27/27 PASS); `test_link_ip` (20 funcs / 77 checks, 0 failures); `test_link_tsn` (15 funcs / 135 checks, 0 failures); `python3 tools/scripts/trace.py` (PASS, 114/71/29/14/51); `python3 tools/scripts/metrics.py --check` (panel current); `clang-format --dry-run --Werror` on the two changed test files (clean); `gcc -Wall -Wextra -Werror -fsyntax-only` on the authored files (see finding PA-MN-01); `make verify-lean` (see PA-MN-07).
- **PA evidence-record conformance** across the increment's data set: `CHANGELOG.md` follow-up bullet; `docs/status/status.json` `note`; `docs/vv/verification-results.md` S4b + delta + follow-up rows and `## History`; `docs/vv/tool-qualification.md` (T3, §History); `docs/planning/sprint-plan.md` §3 NC register + §History; `docs/requirements/traceability.{md,yaml}`; `docs/quality/class-a-gaps.md` (GAP-A5); `docs/status/loop-audit.md`; `docs/requirements/SRS.md` §History; `docs/engineering/user-integration-manual.md` §7/§8.
- **NC control for NC-S4b-01..05** (the deviations the increment claims to close).
- **Cross-review consistency** with the same increment's other independent reviews: FRICK-REVIEW-0167 (R1+R2, combined, `2026-10-13-s4b-followup-r1-r2-review.md`) and FRICK-REVIEW-0168 (R4, documentation, `2026-10-13-s4b-followup-r4-review.md`) — see §3.1. Both are APPROVED WITH COMMENTS.

**Out of scope / excluded (recorded so the scope is bounded):**

- Engineering content of the S4b link backends already reviewed by R1+R2 (FRICK-REVIEW-0165/-D1) and PA (FRICK-REVIEW-0166/-D1) — this report verifies the **follow-up** and its records, it does not re-derive `link_seg`/`link_ip`/`link_tsn` numerics. No `src/` file changed in the increment.
- On-target silicon behaviour (Renode N18 SKIP — host-only increment; GAP-A5).
- This report's own `docs/reviews/README.md` log row (owner: coordinator, norm N11) — to be appended by the coordinator after this report is finalised.

## 2. References

- Project norms: `docs/rules.md` (N1–N20, rev 1.4, 2026-10-11).
- Reviewer contract: `docs/engineering/independent-review.md` (R1–R4 + PA profiles, norm N9/N20).
- Report format: `docs/reviews/review-template.md`.
- Prior reviews (S4b): FRICK-REVIEW-0164 (R4, integration manual); FRICK-REVIEW-0165 and -D1 (R1+R2); FRICK-REVIEW-0166 and -D1 (PA).
- Sibling reviews of this increment: FRICK-REVIEW-0167 (R1+R2 combined, `2026-10-13-s4b-followup-r1-r2-review.md`); FRICK-REVIEW-0168 (R4 documentation, `2026-10-13-s4b-followup-r4-review.md`).
- NC register: `docs/planning/sprint-plan.md` §3 (NC-S4b-01..05).
- Traceability: `docs/requirements/traceability.yaml` (SRS-REQ-059/060/061); `docs/requirements/traceability.md` (mirror rows + §History).
- Requirements: `docs/requirements/SRS.md` (SRS-REQ-059/060/061 + §History).
- V&V records: `docs/vv/verification-results.md` (FRICK-VV-RES-001); `docs/vv/tool-qualification.md` (FRICK-VV-TOOLS-001, T3).
- Class-A gaps: `docs/quality/class-a-gaps.md` (FRICK-QUAL-A-001, GAP-A5).
- Build configuration: `CMakeLists.txt` (lines 17, 187, 240, 248–250); `tests/CMakeLists.txt`; `build/native/**/flags.make`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C Rev.2, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| 1 | **Minor** | N6/N8/N11 — evidence truthfulness | **The "`-Wall -Wextra -Werror` clean / all PASS" claim is false.** Independent compilation of the increment's authored test file `tests/unit/test_link_tsn.c` with `gcc -std=gnu99 -Wall -Wextra -Werror -fsyntax-only` **fails** with two `[-Werror=sign-compare]` errors (`tests/unit/test_link_tsn.c:327` and `:336`, both in `test_tsn_ring_full_drop`: `for (int i = 0; i < FF_LINK_TSN_RX_SLOTS; i++)` compares signed `int` to the unsigned `FF_LINK_TSN_RX_SLOTS`). The claim is asserted in five places in this increment's records: `CHANGELOG.md` follow-up bullet (line 71: "`-Wall -Wextra -Werror` all PASS"), `docs/vv/verification-results.md:56` ("`-Wall -Wextra -Werror` clean (host)"), `docs/status/status.json` `note`, `docs/vv/tool-qualification.md:134` context and `docs/requirements/SRS.md:462`. Separately, the native build **never applies** `-Wall -Wextra` to the test targets (`build/native/tests/CMakeFiles/test_link_tsn.dir/flags.make` → `C_FLAGS = -g -std=gnu99`; `frick_fsw_core` receives `-Werror` only when the default-OFF `FF_ENABLE_WERROR` is set, and never `-Wall -Wextra` — `CMakeLists.txt:17,248–250`), so the claim is not produced by any gate. This is a recurring evidence-truthfulness defect (the same claim was accepted in FRICK-REVIEW-0166-D1 §3, whose command list silently omitted `test_link_tsn.c`). | `gcc … -Wall -Wextra -Werror -fsyntax-only tests/unit/test_link_tsn.c` → 2 errors; `CMakeLists.txt:17,187,248–250`; `build/native/tests/CMakeFiles/test_link_tsn.dir/flags.make`; `CHANGELOG.md:71`; `verification-results.md:56`; `status.json:5`; `SRS.md:462` | Fix the two sign-compare comparisons (use an `unsigned`/`size_t` loop index). Either apply `-Wall -Wextra -Werror` to the test targets in CMake so the gate is real, or stop making the claim. Correct the false "clean/PASS" statements in CHANGELOG, verification-results, status.json and SRS. |
| 2 | **Minor** | N8/N16 — documentation currency | **`docs/engineering/user-integration-manual.md` was not updated for the increment and carries stale data.** Header Revision is `0.8` (line 7) while its own `## History` last row is `0.9` (line 419) — a header/history mismatch (same defect class as NC-S4b-03). The §8 status table still records the **pre-follow-up** counts: line 203 "`test_link_ip.c` (19 test functions: 10 seg + 9 IP)" and line 204 "`test_link_tsn.c` (14 test functions)", whereas the increment brings them to **20 / 15** functions (measured). The manual is a change-affected document (its §8 counts and §8.2 round-trip narrative describe exactly the artifacts the increment changed) so N8 requires it to be updated in the same change. | `user-integration-manual.md:7` (0.8) vs `:419` (0.9); `:203`,`:204` (19/14) vs measured 20/15; not present in the increment's file set | Advance the header to `0.9` (or a new rev) and correct the §8 rows to 20 / 15 functions; add a `## History` row for the follow-up demonstration. |
| 3 | **Minor** | N8/N16 — documentation currency | **`docs/requirements/SRS.md` was not updated for the increment.** Its `## History` stops at `1.23` (delta cycle); there is no entry recording the S4b follow-up (`test_pus_ip_roundtrip` / `test_pus_tsn_roundtrip`, the `Dem` demonstration of SRS-REQ-060/061). Additionally history row `1.22` (line 461) still states the **stale** counts "host unit tests test_link_ip (19) + test_link_tsn (14)". | `SRS.md:461` ("(19) + (14)"), `:462` (last history row = 1.23); no follow-up entry | Add a `1.24` history row for the follow-up (and its `Dem` evidence); correct the stale counts in `1.22` or annotate them as superseded. |
| 4 | **Minor** | N16 — revision history | **`docs/vv/verification-results.md` header `Revision 0.43` and its `## History` were not advanced for the S4b delta cycle nor for this follow-up.** The increment appended the follow-up cycle row (`:56`) but the document header stays `0.43` (line 7) and the `## History` table still ends at `0.43` "S4b delivered" (line 460): there is **no** `0.44` entry for the S4b delta re-review (NF-1/NF-2/NF-3 closure, row `:55`) and no `0.45` entry for the follow-up (row `:56`). The follow-up itself added a cycle row without the mandated history bump. | `verification-results.md:7` (Revision 0.43), `:413` (`## History`), `:460` (last row 0.43); rows `:55`,`:56` present without history entries | Add `0.44` (delta) and `0.45` (follow-up) history rows and advance the header Revision accordingly. |
| 5 | **Minor** | N10/N16 — tool/V&V-script qualification | **`docs/vv/tool-qualification.md` history `0.3.25` self-contradicts.** The same entry states that T3 evidence was extended ("`test_link_ip` gains `test_pus_ip_roundtrip` … `test_link_tsn` gains `test_pus_tsn_roundtrip`") **and** "no V&V tool changed". T3 is defined as `tests/unit/*.c` (`:27`), so modifying `test_link_ip.c` / `test_link_tsn.c` *is* a T3 test-script change; the project's own established precedent records every such T3 test-file change as a **"Re-qualification"** (e.g. `:90,94–100,112,117`). | `tool-qualification.md:27` (T3 = `tests/unit/*.c`), `:134` (0.3.25 both claims), `:90,94,112,117` (re-qualification precedent) | Replace "no V&V tool changed" with a re-qualification record for T3 (the two changed test scripts: what changed, re-run evidence) per N10. |
| 6 | **Minor** | N11/N12 — non-conformance control | **NC-S4b-01..05 have no review-report follow-up record.** The five deviations are traceable only to `docs/planning/sprint-plan.md` §3 (the *sprint view*), yet that same section states "The status of every NC remains in its review report's follow-up table (norms N11/N12); this table is the sprint view only" (`:86`). No review report/§6 follow-up table exists for the NC-S4b series (grep of `docs/reviews/` returns nothing) — the NCs were closed in the sprint register alone, without the review-report follow-up record the project's own rule mandates. | `sprint-plan.md:77–86` (NC-S4b-01..05 + the "review report's follow-up table" rule); no `NC-S4b` reference in any `docs/reviews/*.md` | Record NC-S4b-01..05 (criterion, observed evidence, disposition, re-verification evidence, owner, status, closure authority) in a review-report follow-up table for this increment (this report's §6 carries the PA-raised findings; the N19-audit NCs need their own closure record). |
| 7 | **Minor** | N6/N11 — evidence provenance/consistency | **The increment's cycle row asserts "Lean … PASS (`lake build` 21/21)" for a gate that is not reproducible in this environment and that the same cycle's review recorded as SKIP.** `make verify-lean` fails here (`cd verification/lean && lake build` → `/bin/sh: 1: lake: not found`, exit 127); the only Lean evidence is a committed `verification/lean/lake-build.log` dated 2026-10-11 (`Build completed successfully`, 21/21), i.e. from a **prior** cycle. FRICK-REVIEW-0166-D1 §3 recorded `verify-lean` for the same S4b cycle as **SKIP** ("Lean toolchain not installed"). The follow-up row (`verification-results.md:56`) and status.json nonetheless list Lean as PASS without distinguishing a current-cycle run from a carried-over/CI result. Per N6 (V&V) and N11 (records) the record must state provenance, not report unreproduced evidence as a fresh PASS. | `make verify-lean` → `lake: not found` (exit 127); `verification/lean/lake-build.log` (2026-10-11); `verification-results.md:56` (Lean PASS); FRICK-REVIEW-0166-D1 §3 (`verify-lean` SKIP); `status.json:5` ("Lean … PASS") | Mark the Lean gate as `SKIP (toolchain absent)` / "carried over from cycle X (log attached)" instead of PASS, or re-run `lake build` in a Lean-capable environment and record the run. Apply the same provenance rule to Renode (below). |
| 8 | **Comment** | SRS-REQ-060/061 — V&V method (Dem) | **The declared `Dem` method evidence is a host unit test.** SRS-REQ-060/061 declare `method: Tst | Dem`; the follow-up satisfies the `Dem` leg with `test_pus_ip_roundtrip` / `test_pus_tsn_roundtrip`, i.e. host unit tests over a mock net stack / mock MAC — the same artifacts already serving the `Tst` leg. This is acceptable only if "demonstration" is explicitly defined here as a host reference-backend functional demonstration (on-target qualification is deferred to GAP-A5, which the increment correctly records). As written, the `Dem` basis is asserted but not justified. | `SRS.md:358–367` (`method: Tst | Dem`); `test_link_ip.c` `test_pus_ip_roundtrip` (`cfg.mode = FF_LINK_IPMODE_UDP`); `test_link_tsn.c` `test_pus_tsn_roundtrip`; `traceability.yaml` SRS-REQ-060/061 `lean_note` | State the `Dem` basis explicitly (host reference-backend demonstration; operational-environment demonstration = GAP-A5) in the SRS/traceability note, or defer the `Dem` leg until target evidence exists. |
| 9 | **Comment** | N8/N11 — status semaphores | **The published semaphores are not cycle-accurate for the increment.** `README.md` §Semaphores and `docs/status/status.json` show `product_assurance_review: green`, `reviews: green`, `lean_verification: green` and `renode_functional_validation: green` while: this N20 PA review of the follow-up is **open**, the Lean gate is not reproducible (finding 7), and the S4b/S4b-follow-up rows themselves record Renode as **SKIP** (host-only code). The increment modified `status.json`'s `note` but left the semaphores green. The semantics (last-real-run vs current-cycle) should be explicit. | `README.md:80–90` (semaphores incl. PA/renode green); `status.json:30–40`; `verification-results.md:54,56` (Renode SKIP); finding 7 (Lean) | Document the semaphore semantics in `status.json`, and set `product_assurance_review`/`reviews` to the pending state until this N20 review closes; annotate `renode`/`lean` as "last real run / SKIP-this-cycle". |
| 10 | **Comment** | N8/N11/N19 — change-scope record | **The `README.md` metrics-panel edit is not recorded in any change note.** `README.md` is modified in the increment (coverage rows `5552/7779 → 5618/7877`, `678/698 → 682/702`) but the follow-up entries in `CHANGELOG.md`, `sprint-plan.md` (rev 0.17), `loop-audit.md` (0.25) and `status.json` do not mention it. (The panel itself is consistent — `metrics.py --check` reports current — and the delta **is** attributable to the instrumented test sources: `build/coverage/gcov/…test_link_ip.c.gcov` / `…test_link_tsn.c.gcov` exist and `tools/scripts/coverage.py:26,42` aggregates **all** `.gcov`, not only `src/`; so this is not the "environment drift" suspected by FRICK-REVIEW-0167 R2-F01, but it is still an unrecorded scope change.) | `git diff README.md`; `CHANGELOG.md:48–71` (no README mention); `tools/scripts/coverage.py:26,42`; `build/coverage/gcov/*test_link_*.gcov` | Record the README metrics refresh in the change note (as baseline commit `57974d2` did), or revert `README.md`; note that coverage aggregates test sources into the figure. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

**No Major finding.** The increment's product (the two new test functions and the PUS-C round-trip demonstration) is correct and reproducible, the `src/` code is unchanged, and the closed deviation NC-S4b-02 is re-evidenced by tests that actually execute (see §4). All findings above are Minor (record/currency/tool-qualification/NC-control, plus the false `-Werror` claim and the Lean provenance) or Comments (method labelling, semaphore semantics, README change-scope record).

### 3.1 Cross-review consistency with FRICK-REVIEW-0167 (R1+R2) and FRICK-REVIEW-0168 (R4)

This PA review ran independently but converged with the sibling reviews on the increment's factual record; the mapping below shows agreement, avoids silent contradiction, and isolates the PA-only findings. Where I state agreement, I re-verified the item first-hand.

| Sibling finding | Item | This PA review | Agreement |
| --------------- | ---- | -------------- | --------- |
| 0167 **R2-F02** | `-Wall -Wextra -Werror` claim not reproducible for `test_link_tsn.c` (sign-compare 327/336) | **PA-MN-01** (Minor) — same failure reproduced, claim traced across five records, and the build never applies the flags | **agree** |
| 0167 **R2-F04** | T3 entry 0.3.25 "no V&V tool changed" despite changed test scripts | **PA-MN-05** (Minor) — same, cross-checked against T3 = `tests/unit/*.c` (`:27`) | **agree** |
| 0167 **R2-F06** | `SRS.md` history not updated for the follow-up (latest row 1.22, counts 19/14) | **PA-MN-03** (Minor) — same | **agree** |
| 0167 **R1-F01** | IP `Dem` send leg not asserted (`g_mock_net_tx` unread) | Acknowledged; PA adds **PA-MN-08** (Comment) on the `Dem` method basis itself | **agree / complementary** |
| 0168 findings 8 | Integration manual §8 counts (19/14) + header 0.8 vs History 0.9 | **PA-MN-02** (Minor) — same | **agree** |
| 0168 finding 9 | `status.json`/README review semaphores green while reviews open | **PA-MN-09** (Comment) — same, extended to Lean/Renode | **agree** |
| 0168 findings 1–7,10 | Other stale documents (software-description, network-stack, README scope matrix, development-plan, handover, sprint-plan History 0.14) | Out of this increment's changed-file scope; noted as documentation-loop residuals to be closed under N17 (see Appendix B, N17) | **agree (deferred to R4)** |
| 0167 **R2-F01** | `README.md` metrics edit / "environment drift" | **PA-MN-10** (Comment) — the delta **is** attributable (coverage aggregates test sources); the real defect is the unrecorded change scope | **partially differ** (see PA-MN-10) |
| — | Lean PASS asserted for a SKIP/unreproducible gate | **PA-MN-07** (Minor) — PA-only (not raised by 0167/0168) | **new (PA)** |
| — | NC-S4b-01..05 have no review-report follow-up record | **PA-MN-06** (Minor) — PA-only | **new (PA)** |

## 4. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**Rationale.** The increment is a competent, narrowly-scoped corrective change targeting the N19 conformance-audit deviations. First-hand re-execution confirms every *behavioural* claim:

- `cmake --build build/native` PASS; `ctest` **27/27** PASS.
- `test_link_ip` **20 funcs / 77 checks, 0 failures**; `test_link_tsn` **15 funcs / 135 checks, 0 failures** — the increment's declared counts (`verification-results.md:56`, `CHANGELOG.md`, `status.json`) are **accurate**.
- `trace.py` PASS (114 reqs / 71 srs / 29 sss / 14 gaps / 51 theorems); `metrics.py --check` → "panel current"; `clang-format --dry-run --Werror` on both changed test files → clean; `README.md` coverage panel (5618/7877, 682/702) is consistent with the generated output.
- The `test_pus_ip_roundtrip` / `test_pus_tsn_roundtrip` functions do carry a real PUS-C S17 exchange over the IP and TSN `FF_Link_t` backends, so the NC-S4b-02 exit-criterion/Dem gap is addressed for the host reference backend (on-target = GAP-A5, correctly recorded). The `src/transport/link/*` backends are `-Wall -Wextra -Werror` clean.

The findings are therefore **evidence-record and documentation-currency defects**, not product defects: (1) a **false compile-cleanliness claim** repeated across four documents and not produced by any gate; (2–4) the integration manual, SRS and verification-results were not brought current (stale counts, header/history mismatches, missing history rows); (5) a tool-qualification entry that contradicts itself ("no V&V tool changed" while T3 test scripts changed); (6) the NC-S4b-01..05 deviations closed without the review-report follow-up record the project's own rule requires; (7) a Lean PASS asserted for a gate that is SKIP/unreproducible and was recorded as SKIP in the same cycle's review; plus three Comments (host-only `Dem` labelling; semaphore cycle-accuracy; unrecorded README change scope).

Per norm N12 these Minors must be corrected and re-verified; per norm N20 the PA review is repeated over the corrective delta and the affected scope re-audited until it passes. Per norm N11 the NCs are resolved by re-applying **all** rules and re-reviewing the deltas. This review does **not** block the technical merit of the increment, but the false evidence claims (finding 1) and the missing NC records (findings 4 and 6) must be closed before this increment's records can be certified as conformant.

**Confidence: High** for findings 1–5 and 8–9 (each verified by direct file read and, for finding 1, by reproducing the failure) and for all behavioural results (re-executed). **High** for finding 6 (the absent review-report record was confirmed by a repository-wide search). **Medium** for finding 7: the Lean gate's historical pass is evidenced by a committed build log but cannot be reproduced in this environment, so the defect is recorded as an evidence-provenance inconsistency rather than a proven false artefact.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| **PA (N20)** | Senior SW Product Assurance engineer (independent context, norm N9/N20) | 2026-10-13 | **APPROVED WITH COMMENTS** — no Major finding; 7 Minor (PA-MN-01..07) + 3 Comments (PA-MN-08/09/10) tracked to closure. Increment's behavioural claims verified by re-execution; open items are evidence-record/documentation-currency defects. |

## 6. Follow-up records

(norm N12: NCs are resolved by re-applying ALL norms to the affected scope and re-reviewing the deltas until full compliance. Owner of every corrective action: **frick-fsw authoring agent**.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| **PA-MN-01** | Minor | N6/N8/N11 — evidence truthfulness | False "`-Wall -Wextra -Werror` clean/all PASS" claim; `test_link_tsn.c:327,336` fail `-Werror=sign-compare` | Fix the two comparisons; apply `-Wall -Wextra -Werror` to test targets (or drop the claim); correct CHANGELOG/verification-results/status.json/SRS | `clang-format` + `gcc -Wall -Wextra -Werror` clean over `tests/unit/test_link_ip.c` + `tests/unit/test_link_tsn.c`; corrected records | frick-fsw authoring agent | **corrected; OPEN pending independent delta review** | pending |
| **PA-MN-02** | Minor | N8/N16 — integration manual | Header rev 0.8 ≠ history 0.9; §8 rows 203/204 stale (19/14 vs 20/15) | Advance header; correct §8 counts; add follow-up `## History` row | Manual diff + count cross-check | frick-fsw authoring agent | **corrected; OPEN pending independent delta review** | pending |
| **PA-MN-03** | Minor | N8/N16 — SRS | No follow-up history entry; history 1.22 stale counts "(19) + (14)" | Add `1.24` row; correct/annotate `1.22` counts | SRS diff | frick-fsw authoring agent | **corrected; OPEN pending independent delta review** | pending |
| **PA-MN-04** | Minor | N16 — verification-results | Header 0.43 + `## History` not advanced for delta (0.44) / follow-up (0.45) | Add `0.44`/`0.45` history rows; advance header Revision | verification-results diff | frick-fsw authoring agent | **corrected; OPEN pending independent delta review** | pending |
| **PA-MN-05** | Minor | N10/N16 — tool qualification | 0.3.25 says "no V&V tool changed" while T3 `test_link_ip.c`/`test_link_tsn.c` changed | Replace with a T3 re-qualification record (precedent: `:90,94,112,117`) | tool-qualification diff + T3 re-run | frick-fsw authoring agent | **corrected; OPEN pending independent delta review** | pending |
| **PA-MN-06** | Minor | N11/N12 — NC control | NC-S4b-01..05 have no review-report follow-up record (sprint view only) | Record NC-S4b-01..05 closure in a review-report §6 follow-up table (criterion/evidence/disposition/owner/status) | NC register + review report §6 | frick-fsw authoring agent | **corrected; OPEN pending independent delta review** | pending |
| **PA-MN-07** | Minor | N6/N11 — Lean evidence provenance | Cycle row asserts Lean PASS (`lake build` 21/21); gate SKIP/unreproducible and recorded SKIP in FRICK-REVIEW-0166-D1 | Mark Lean `SKIP`/carried-over with provenance, or re-run in a Lean-capable env | `lake build` log of the current cycle, or corrected record | frick-fsw authoring agent | **corrected; OPEN pending independent delta review** | pending |
| **PA-MN-08** | Comment | SRS-REQ-060/061 — `Tst \| Dem` | `Dem` evidence is a host unit test (same artifacts as `Tst`); basis not justified | State the `Dem` basis (host reference-backend demonstration; on-target = GAP-A5) or defer `Dem` | SRS/traceability note | frick-fsw authoring agent | **corrected; OPEN pending independent delta review** | pending |
| **PA-MN-09** | Comment | N8/N11 — semaphores | PA/reviews/Lean/Renode semaphores green while PA review open, Lean SKIP, Renode SKIP | Document semaphore semantics; set PA/reviews pending; annotate Lean/Renode | status.json/README diff | frick-fsw authoring agent | **corrected; OPEN pending independent delta review** | pending |
| **PA-MN-10** | Comment | N8/N11/N19 — change-scope record | `README.md` metrics-panel edit not recorded in any change note (coverage 5552/7779→5618/7877); attributable to the instrumented test sources (not drift) | Record the README refresh in the change note, or revert `README.md` | README/CHANGELOG diff + `coverage.py` check | frick-fsw authoring agent | **corrected; OPEN pending independent delta review** | pending |

### 6.1 NC closure records (authoring agent, 2026-10-13)

**Findings PA-MN-01..10** have author corrections recorded in the disposition
column above, but remain **OPEN pending independent delta review**. The table's
former `closed` labels were erroneous: authoring correction is not NC closure.
The affected files are `tests/CMakeLists.txt`,
`tests/unit/*.c`, `CHANGELOG.md`, `README.md`,
`docs/vv/verification-results.md`, `docs/vv/tool-qualification.md`,
`docs/requirements/SRS.md`, `docs/requirements/traceability.yaml`,
`docs/engineering/user-integration-manual.md`,
`docs/engineering/software-description.md`, `docs/architecture/network-stack.md`,
`docs/development-plan.md`, `docs/status/handover.md`, `docs/status/status.json`,
`docs/status/loop-audit.md` and `docs/planning/sprint-plan.md`.
Re-verification evidence available at the time included the local pipeline
(build, ctest 27/27, trace, clang-format, metrics gate, smoke, GSW); this does not
substitute for an independent delta review and is not evidence for later edits.

**Norm-N19 audit NCs** — raised by `docs/status/loop-audit.md` §1 and registered
in `docs/planning/sprint-plan.md` §3. The authoring agent originally labeled
these closed, but that was unsupported: the independent follow-up reports
`FRICK-REVIEW-0167`/`0168`/`0169` state that delta closure remains pending. They
remain OPEN here until their required independent review is recorded.

| NC | Severity | Criterion (what was non-conformant) | Evidence of closure | Owner | Status |
| -- | -------- | ----------------------------------- | ------------------- | ----- | ------ |
| NC-S4b-01 | Minor | GAP-A5 did not enumerate the IP/TSN link-level on-target residual although the records claim it | `docs/quality/class-a-gaps.md` GAP-A5 (rev 0.11) carries the bullet | frick-fsw authoring agent | OPEN pending independent delta review |
| NC-S4b-02 | **Major** | The S4b exit criterion (`pus_ip`/`pus_tsn`) was unevidenced, and the `Dem` method of SRS-REQ-060/061 was unmet | `test_pus_ip_roundtrip` / `test_pus_tsn_roundtrip` (host PUS-C TM/TC round-trips); on-target residual = GAP-A5 | frick-fsw authoring agent | OPEN pending independent delta review |
| NC-S4b-03 | Minor | Stale suite/check counts, `Verfied` typo, stale header revisions | `verification-results.md`, `traceability.yaml`, sprint-plan/loop-audit headers corrected | frick-fsw authoring agent | OPEN pending independent delta review |
| NC-S4b-04 | Minor | `traceability.md` SRS-REQ-059 mirror row listed `test_link_tsn.c` | mirror row corrected to `test_link_ip.c` | frick-fsw authoring agent | OPEN pending independent delta review |
| NC-S4b-05 | Minor | Stale M4 pending/remaining lists | loop-audit §1 M4 row and `verification-results.md` "Remaining (M4)" reconciled with the delivered state | frick-fsw authoring agent | OPEN pending independent delta review |

> This historical report's original statement that NC-S4b-01..05 were closed was corrected by the authoring agent after independent PA finding PA-01/NC-EXT-28. The NCs remain open pending independent delta review. This correction does not constitute approval of this report or of the current change set.

---

## Appendix A — Evidence ledger (all verified first-hand, this session)

| Claim | Method | Result |
| ----- | ------ | ------ |
| Build | `cmake --build build/native -j4` | **PASS** (exit 0; all targets incl. `test_link_ip`, `test_link_tsn`) |
| ctest | `ctest` in `build/native` | **PASS 27/27**, 0 failed |
| `test_link_ip` counts | run `./tests/test_link_ip` | **checks=77 failures=0**; 20 funcs in `FF_TEST_MAIN` — matches the increment's claim |
| `test_link_tsn` counts | run `./tests/test_link_tsn` | **checks=135 failures=0**; 15 funcs in `FF_TEST_MAIN` — matches the increment's claim |
| Traceability | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** (114 reqs / 71 srs / 29 sss / 14 gaps / 51 theorems) |
| Metrics panel | `python3 tools/scripts/metrics.py --check` | **"metrics panel: current"** (exit 0) |
| clang-format | `clang-format --dry-run --Werror tests/unit/test_link_ip.c tests/unit/test_link_tsn.c` | **clean** (exit 0, clang-format 23.1.3) |
| `-Wall -Wextra -Werror` (finding 1) | `gcc -std=gnu99 -Wall -Wextra -Werror -fsyntax-only` (project include paths) over `link_seg.c`, `link_ip.c`, `link_tsn.c`, `test_link_ip.c`, `test_link_tsn.c` | src + `test_link_ip.c` **clean**; **`test_link_tsn.c` FAILS — 2 × `-Werror=sign-compare` at `:327`,`:336`** |
| Lean gate (finding 7) | `make verify-lean` | **FAIL/not-reproducible** — `lake: not found` (exit 127); `lake-build.log` dated 2026-10-11 (prior cycle) |
| Renode gate | read of `verification-results.md:54,56` | **SKIP** (host-only code, N18 proxy) — not treated as PASS |
| cppcheck/MISRA | no claim made in this increment | not run this cycle (nothing claimed) |
| `test_pus_ip_roundtrip` substance | source read `tests/unit/test_link_ip.c` | genuine PUS S17 ping TC → PUS ingest → TM reply over the IP `FF_Link_t` backend (UDP mode) |
| `test_pus_tsn_roundtrip` substance | source read `tests/unit/test_link_tsn.c` | genuine PUS S17 ping TC → PUS ingest → TM reply over the TSN backend (mock MAC loop-backed) |
| Increment scope | `git status --short`, `git diff --stat` | 12 files; **no `src/` change**; baseline `57974d2` |
| NC-S4b record search (finding 6) | `grep -rn "NC-S4b"` across `docs/` (excl. sprint-plan) and all `docs/reviews/*.md` | matches only in CHANGELOG-adjacent docs + sprint-plan; **no review report carries them** |
| README coverage attribution (finding 10) | inspect `build/coverage/gcov/` + `tools/scripts/coverage.py` | test sources **are** instrumented (`…tests#unit#test_link_ip.c.gcov`, `…test_link_tsn.c.gcov`); `coverage.py:26,42` aggregates all `.gcov` → the panel delta is attributable, not drift |
| Sibling reviews | read FRICK-REVIEW-0167 (R1+R2), FRICK-REVIEW-0168 (R4) | both **APPROVED WITH COMMENTS**; agreement on the `-Werror` claim, T3 entry, SRS history, integration-manual counts, semaphores (see §3.1) |
| Data-set checks | read CHANGELOG/README/status.json/verification-results/tool-qualification/SRS/integration manual/traceability/class-a-gaps/sprint-plan/loop-audit | findings 1–9 confirmed (header/history mismatches, stale counts, false claim, self-contradicting tool-qual entry, semaphores) |

## Appendix B — Norm-by-norm audit (N1–N20)

| Norm | Requirement | Verification | Status |
| ---- | ----------- | ------------ | ------ |
| N1 | Lean verification of generated code | `lean: []` with documented `lean_note` for SRS-REQ-059/060/061; trace gate counts 51 theorems. `lake build` not reproducible here (finding 7). | partial (PA-MN-07) |
| N2 | English only | All new/changed content English. | compliant |
| N3 | Backwards ECSS SRS/SSS | SRS-REQ-059/060/061 present, parented to SSS-REQ-028. | compliant |
| N4 | Bidirectional traceability | `trace.py` PASS; SRS-REQ-059 mirror row corrected (NC-S4b-04); SRS-REQ-060/061 notes carry the new tests. | compliant |
| N5 | Class B target; class-A gaps documented | GAP-A5 gains the IP/TSN on-target residual (rev 0.11); ECSS Class B baseline. | compliant |
| N6 | Full SW V&V; Docker-deployed environments | build + ctest 27/27 + trace + smoke + GSW PASS; Renode SKIP; Lean not reproducible (finding 7); false `-Werror` claim (finding 1). | partial (PA-MN-01/07) |
| N7 | Dual push | not performed — pre-commit review gate; commit/push is the human/authority action. | n/a |
| N8 | Complete, current, deployable documentation | Integration manual stale (PA-MN-02); SRS not updated for the follow-up (PA-MN-03); false `-Werror` claims (PA-MN-01); semaphores (PA-MN-09). | partial |
| N9 | Independent review agents | R1/R2, R4, PA reviews independent; this PA review independent. | compliant |
| N10 | Test-script/V&V-tool review & qualification | T3 test scripts changed but recorded "no V&V tool changed" (PA-MN-05). | partial (PA-MN-05) |
| N11 | Scripts + review results in the repo | Review reports present; NC-S4b-01..05 lack a review follow-up record (PA-MN-06); Lean provenance (PA-MN-07). | partial |
| N12 | NC resolution to full compliance | 7 Minor + 3 Comments open; no Major — does not block commit, but must be closed and re-reviewed. | open |
| N13 | AGPL-3.0-only, SPDX | New test code carries SPDX/License headers; no licensing defect in the increment. | compliant |
| N14 | SSS review by Senior Systems Engineer | No SSS change in the increment; not triggered. | not triggered |
| N15 | Full cycle on every change | Cycle re-run: build + ctest + trace + metrics + clang-format PASS; Lean not reproducible (PA-MN-07). | partial |
| N16 | Detailed revision history | CHANGELOG/SRS/integration-manual/verification-results/tool-qualification history deficits (PA-MN-02/03/04/05). | partial |
| N17 | R4 documentation review before commit | R4 review (FRICK-REVIEW-0164) APPROVED WITH COMMENTS; but the follow-up changed docs *after* that review without a new R4 pass → re-trigger required. | **re-trigger needed** |
| N18 | Renode functional validation | SKIP proxy correctly recorded (host-only code); not treated as PASS. | compliant (SKIP) |
| N19 | Loop audit vs objectives/norms | `loop-audit.md` history 0.25 records the follow-up; M4 lists reconciled (NC-S4b-05). | compliant |
| N20 | Independent PA review of every phase product | This report — APPROVED WITH COMMENTS (7 Minor + 3 Comments tracked to closure). | **open comments** |

## Appendix C — Facts, assumptions, open issues

**Facts**

- The increment contains 12 uncommitted files; no `src/` file changed (baseline `57974d2`).
- `test_link_ip` = 20 funcs / 77 checks; `test_link_tsn` = 15 funcs / 135 checks (measured).
- `gcc -std=gnu99 -Wall -Wextra -Werror -fsyntax-only tests/unit/test_link_tsn.c` fails with `-Werror=sign-compare` at lines 327 and 336.
- The native build applies `-Wall -Wextra` to no target; `frick_fsw_core` gets `-Werror` only under `FF_ENABLE_WERROR` (default OFF).
- `make verify-lean` fails in this environment (`lake: not found`, exit 127).
- `verification-results.md` header Revision = 0.43 and `## History` ends at 0.43.
- `user-integration-manual.md` header Revision = 0.8; `## History` last row = 0.9; §8 counts = 19/14.
- `SRS.md` last history row = 1.23; row 1.22 counts = "(19) + (14)".
- `tool-qualification.md` 0.3.25 states both "T3 evidence extended" and "no V&V tool changed".
- No `docs/reviews/*.md` references `NC-S4b`.
- `status.json`/`README.md` semaphores are all green, including PA/reviews/Lean/Renode.

**Open issues (claim type: Open issue)**

- Whether `Dem` for SRS-REQ-060/061 is acceptable as a host reference-backend demonstration, or must await target evidence (PA-MN-08) — requires the PA/authority to define the `Dem` basis.
- Whether the Lean "PASS" carries forward from a real cycle (log attached) or must be re-run (PA-MN-07).
- Whether the follow-up's post-R4 documentation changes re-trigger the N17 R4 pass (Appendix B, N17).

**Assumptions**

- *The committed `verification/lean/lake-build.log` (2026-10-11) is an authentic prior-cycle `lake build` output.* — owner: V&V; impact: PA-MN-07 severity (Medium confidence).
- *Host unit tests over a mock/loopback stack are the project's accepted proxy for the reference-backend `Dem` leg (on-target = GAP-A5).* — owner: PA/V&V; impact: PA-MN-08 (Comment only).
- *Semaphore states are intended as "last real run" rather than "current cycle".* — owner: coordinator; impact: PA-MN-09 (Comment only).

**Recommendations (ordered)**

1. Fix the two sign-compare comparisons in `test_link_tsn.c` and make the `-Wall -Wextra -Werror` gate real (add the flags in CMake) **or** withdraw the claim everywhere (PA-MN-01).
2. Bring the four documents current: integration manual (§8 counts, header rev, history), SRS (follow-up history entry, stale counts), verification-results (header rev + `0.44`/`0.45` history), tool-qualification (T3 re-qualification) (PA-MN-02/03/04/05).
3. Record the NC-S4b-01..05 closure in a review-report follow-up table (PA-MN-06).
4. Correct the Lean provenance and the semaphore semantics (PA-MN-07, PA-MN-09).
5. Justify or defer the `Dem` leg for SRS-REQ-060/061 (PA-MN-08).
6. Re-trigger the R4 (N17) documentation pass over the corrected docs, then repeat the PA (N20) delta review over the corrective delta until no finding is open, before commit/push.

---

## Norms applied — closing note (PA)

This review is an independent product-assurance assessment, not a certification or approval. It does not accept residual risk, close non-conformances, or authorise release. Per norm N12 the corrective delta must re-apply **all** norms to the affected scope; per norm N20 the PA review repeats until it passes; per norm N11 the NCs above are resolved by re-reviewing the deltas. Where this report and a primary source disagree, the primary source governs.
