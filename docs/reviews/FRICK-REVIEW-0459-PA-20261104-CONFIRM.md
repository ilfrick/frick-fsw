# FRICK-REVIEW-0459 — Independent PA/N20 delta confirmation (2026-11-04)

| Field | Value |
| --- | --- |
| Document ID | **FRICK-REVIEW-0459** |
| Profile | **PA / N20** (Senior SW Product Assurance), independent sub-agent |
| Reviewer session | `ses_e5a4ecb0effetawalmRd4UDzgi` |
| Date | 2026-11-04 |
| Repository | `/home/nicola/fsw/frick-fsw` (branch `main`) |
| Base commit | `41dcbe21435e14b5aaff06c63d9df6f0de04df54` — all reviewed content is uncommitted |
| Manifest | `docs/vv/evidence/2026-11-04-re-review/candidate-manifest.sha256` |
| Manifest sha256 | `135cb893a6a092520e9db49a628aeb9a3a4a0da0220da5a492ac2f9963d12f87` (matches the value supplied with the task) |
| Manifest verification | `sha256sum -c` → **exit 0, 174 OK, 0 FAILED, 174 lines** (run first, before any other check) |
| Scope | Narrow delta confirmation only: closure of PA-0457-01..08, plus a consistency/config-management sweep for regressions introduced by the correction |
| Out of scope | Full re-review; re-litigation of PA-0452..0456 findings; N18/target/HIL; human qualification; tool qualification decisions; release/commit authority; the withdrawn `test_pus_mutation.c`; `third_party/n7s-obcp`; `build*/`, `.opencode/`, `opencode.json`, `tests/node/` |

**Verdict: PASS WITH COMMENTS — 0 Blocker, 0 Major, 2 Minor, 3 Observations.**

Six of the eight round-three findings are fully resolved, one is partially resolved
(PA-0457-01: the same stale-revision defect survives on a third row of the same identity
table), and one is resolved with a new — smaller — accuracy defect in the rationale it
added (PA-0457-06 → PA-0459-02). No NC is closed, no sprint is closed, and this report is
**not** release approval.

## 1. Independence declaration

- I am an independent sub-agent session (`ses_e5a4ecb0effetawalmRd4UDzgi`). I did not
  author, edit or direct any part of this candidate, and I have no schedule, financial or
  reporting-line interest in its acceptance.
- I did not modify tracked content. This report is the only file I created. Nothing was
  committed or pushed.
- I did not close, downgrade or dispose any NC, finding or risk, and I did not qualify any
  tool. The verdict is a recommendation to the human authority, who alone may accept it.
- Limits of this pass (deliberate, narrow scope): I did **not** re-run configure/build/
  CTest/Lean/trace/format/smoke/RTEMS build from scratch. I verified the retained evidence
  by hash and re-executed four independent checks that are cheap and decisive: the
  candidate manifest, the prior `assets.sha256`, the RTEMS `nm` inventory inside the
  pinned toolchain image, and the two gate10 test binaries. Binary-to-source binding for
  the rest rests on the retained build logs, not on a fresh build of my own.

## 2. Findings

| ID | Severity | File:line | Finding | Required action |
| --- | --- | --- | --- | --- |
| **PA-0459-01** | Minor | `docs/vv/evidence/2026-11-04-re-review/CANDIDATE.md:27` (bound at `candidate-manifest.sha256:108`) | The bound candidate-identity table still cites `docs/vv/tool-qualification.md` at **rev 1.3**, but the manifested file header (`tool-qualification.md:7`) is **rev 1.4**, and the delta itself bumped it (history row 1.4 records the 110-check hash). `verification-results.md:292` in the same bundle already says "tool-qualification.md rev 1.4", so the bundle contradicts itself. This is the same defect class as PA-0457-01, on a row the correction did not touch. The T3 conclusion is unaffected (rev 1.3 and 1.4 both say **NOT FULLY QUALIFIED**), but an auditor following the identity table reads the wrong edition and misses the 1.4 hash binding. | Change to **rev 1.4** (or "1.3+"), and stop claiming in `verification-results.md:290` that PA-0457-01..08 are all "corrected" until this row is fixed. |
| **PA-0459-02** | Minor | `docs/vv/evidence/2026-11-04-re-review/CANDIDATE.md:90-93` | The new out-of-scope entry for `src/main/fsw_main_reqtest.c` concludes "It is excluded because it cannot affect any gate result". That is too strong: `tools/scripts/license_check.py` enumerates all `src/**` managed-suffix files (`MANAGED_ROOTS` includes `src`; 264 files enumerated on this tree, including this one) and `tools/scripts/cppcheck_scan.sh` scans `/workspace/src`. The file is correctly not compiled (`#error` at `fsw_main_reqtest.c:44-45`, no `CMakeLists.txt`/`*.cmake` reference, `FF_ENABLE_TARGET_REQ_TEST` absent from the build system), and it currently passes both checks (it carries `SPDX-License-Identifier: MPL-2.0` at line 27), so **no recorded gate result changes**. But it participates in the N13 licence-check input set while being unbound, so base commit + manifest does not fully reconstruct the configuration that produced that gate. | Either bind the file in the manifest (it is small and test-only), or restate the rationale precisely: "not compiled and not referenced by the build system; scanned by the N13 licence checker and by the advisory cppcheck path set, where it currently passes; left unbound by decision." |
| **PA-0459-O-01** | Observation | `docs/planning/sprint-plan.md:101` | A live task row still says "the current candidate gate is 52/52, **Rev 1.121**". The 52/52 figure is still the current, evidenced result (`gate10-ctest.log`), so nothing is false — the revision label is stale by three revisions. | Re-label as "as of Rev 1.121" or point at the history row. |
| **PA-0459-O-02** | Observation | `docs/vv/verification-results.md:279`, `docs/planning/sprint-plan.md:322` | Both cite `tool-qualification.md` **rev 1.3** for the T3 determination. This is defensible as the revision of record, and rev 1.4 confirms the status is unchanged, but neither says so. | Add "(unchanged at rev 1.4)" where the 1.3 pointer is retained for history. |
| **PA-0459-O-03** | Observation | `docs/reviews/README.md` | The review index does not yet contain a row for FRICK-REVIEW-0459 (or 0458, for which no report exists). This is a bookkeeping consequence of my write scope (`docs/reviews/README.md` is tracked content I was told not to modify), not a candidate defect. | Coordinator adds the 0459 row under N11/N20 when accepting this report. |

## 3. Per-finding closure of PA-0457

| Finding | Status | Evidence |
| --- | --- | --- |
| **PA-0457-01** — stale revisions / volatile line anchor | **PARTIALLY RESOLVED** | `CANDIDATE.md:30` now names sprint-plan **Rev 1.124** and handover **rev 3.63**; headers confirm (`sprint-plan.md:7`, `handover.md:7`). The volatile `:764` pointer is gone from the plan: Rev 1.123(b) now uses the stable description "the two former task rows (Rev 1.121 history row and the S8-SCH task row)", and the only remaining `:764` occurrences are the handover 3.62 history row (explicitly "line numbers as of Rev 1.123") and immutable review reports. **But** the same identity table still cites tool-qualification at rev 1.3 against a manifested rev 1.4 → **PA-0459-01**. |
| **PA-0457-02** — review index incomplete | **RESOLVED** | `docs/reviews/README.md` now has rows for **0439** (`:18`), **0440** (`:17`), **0448** (`:16`), the rest of 0441–0455, and **0456/0457** (`:5`, `:6`). All 19 IDs cited by `CANDIDATE.md` are present; every other review cited by the bundle (0407, 0436, 0438) is also indexed. `CANDIDATE.md:31`'s claim "index rows 0439–0457" matches what is actually present. No cited review file is missing from the index. |
| **PA-0457-03** — NC-EXT-236 stated at two scopes | **RESOLVED** | Narrowed statement now in all five required places and mutually identical in substance: 0449 erratum scope correction (`FRICK-REVIEW-0449…:167-176`), handover 3.58 (`:1594`), `sprint-plan.md:323`, `ts-5-core-services.md:177`, `verification-results.md:305-311`. Historical bodies are retained (0449 body + "appended, body retained"; the 3.58 row keeps its original text before the dated annotations). Independent check of the factual basis: `app_tm_output.c:16-25` really does define `ff_to_sink` and install it via `FF_AppTmOutputInit`, and `test_r3_gap.c:520-571` really does exercise listener → `FF_PusEmitEvent` → S5 → TM sink. |
| **PA-0457-04** — duplicate residual headings | **RESOLVED** | Exactly one current section: `verification-results.md:299` "Residual unproven items (not closed by this cycle)". The earlier one is retitled at `:191` as "(historical, superseded)" with a pointer to the current list. The stale "RTEMS cross build … not yet run" line (`:200`) is annotated as superseded. Evidence matches: `rtems-build-exit.txt` = `RTEMS_BUILD_EXIT=0`; `rtems-symbols.log` records ELF sha256 `2c4934d5a862…8a87b`, which I recomputed on the retained ELF — **identical**. |
| **PA-0457-05** — toolchain-specific `nm` counts | **RESOLVED (reproduced)** | `rtems-symbols.log` §2 names `/opt/rtems/6/bin/riscv-rtems6-nm` and §8 states the counts are toolchain-specific, with the host-`nm` value given. I re-ran inside `frick-rtems-toolchain:local` on the retained ELF: full **1450**, `-g` **1085**, `--defined-only` **1450**, seam symbol count **0**; host `nm` → **2323**. The log's numbers reproduce exactly. |
| **PA-0457-06** — excluded reqtest source | **RESOLVED** (new defect in its rationale) | `CANDIDATE.md:90-93` now names `src/main/fsw_main_reqtest.c`, states the `#error` guard and the absence of a build-system reference. Verified: `fsw_main_reqtest.c:44-45` has `#if !FF_ENABLE_TARGET_REQ_TEST` / `#error`; `grep` over `CMakeLists.txt`/`*.cmake` finds no reference and no `FF_ENABLE_TARGET_REQ_TEST`. NC-EXT-236/237 owners now record a role **and** an acceptance authority (`sprint-plan.md:323-324`, `verification-results.md:310-314`: "owner role: FSW engineer; acceptance authority: Nicola"), not just a name. The added clause "cannot affect any gate result" is inaccurate → **PA-0459-02**. |
| **PA-0457-07** — evidence-mapping classification | **RESOLVED** | `CANDIDATE.md:80-86` is a single classification paragraph: bound for reconstruction only, **not** formatting-only, **not** reviewed, must be reviewed or reverted before any commit that includes it. It is not listed under "Manifest-only"; the `:94-95` bullet is a cross-reference, not a second classification. It is bound in the manifest (`:57`). No other document in the bundle classifies it differently. |
| **PA-0457-08** — evidence package not self-contained | **RESOLVED** | Manifest binds **all** cited reviews: 0439, 0440, 0441, 0442, 0443, 0444, 0445, 0446, 0447, 0448, 0449, 0450, 0451, 0452, 0453, 0454, 0455, 0456, 0457 (19 files). No manifested file contains the manifest's own hash (`grep -r 135cb893…` over the tree returns nothing). `candidate-manifest.sha256` and `manifest-verify.log` are the **only** two files in the evidence directory absent from the manifest — exactly as declared. The earlier 2026-11-03 package is bound too (30 files), and `sha256sum -c assets.sha256` now returns **exit 0, 29 OK** (independently re-run), confirming the round-two Major is genuinely fixed rather than re-worded. |

## 4. Status / consistency sweep (item 9)

All records agree on the material facts; I found no contradiction of substance.

| Fact | Where checked | Result |
| --- | --- | --- |
| Test script hash `621a0fe1…442d` | `sha256sum tests/unit/test_evs_hs_health.c`; `CANDIDATE.md:22`; `ts-5:177`; `tool-qualification.md` hist. 1.4; `handover.md:1600`; `sprint-plan.md:700` | Agrees, and equals the working-tree file |
| 110 checks / 0 failures | `gate10-new-tests.log`; re-ran `build/gate10/tests/test_evs_hs_health` → `checks=110 failures=0`, exit 0 | Agrees |
| 4325 checks (frozen seam) | `gate10-new-tests.log`; re-ran binary → `checks=4325 failures=0`, exit 0 | Agrees |
| `gate10` authoritative | `README.md` §2.2 (gate7/8/9 superseded with reasons); `verification-results.md:293-297` | Agrees; `gate10-ctest.log` = `100% tests passed, 0 tests failed out of 52`, `gate10-build.log` = `[402/402]`, 403 log lines, smoke = "banner + 7 applications registered", trace = PASS 120/76/30/14/68, cppcheck = 163 (advisory) |
| T3 **NOT FULLY QUALIFIED** | `tool-qualification.md:26` and hist. 1.3/1.4; `CANDIDATE.md:27`; `sprint-plan.md:322`; `handover.md:1598,1600`; `verification-results.md:279`; `README.md` §5 | Agrees everywhere; no live "T3 QUALIFIED" claim remains |
| No NC closed, no sprint closed | `CANDIDATE.md:102-103`; `verification-results.md:316`; `README.md` §5; `sprint-plan.md:700`; `handover.md:1600` | Agrees; NC-EXT-19/236/237 explicitly open |
| N18 still FAIL | `verification-results.md:301`; `README.md` §5; `handover.md:38,70,115` | Agrees; no live N18 PASS claim outside dated history |
| 108-check figure | Only in `new-tests-run.log` (108) and labelled "intermediate revision" / superseded by `gate10-*` in `README.md` §2.2 and `ts-5:177` | Correctly quarantined as history |

One caveat, not a finding: the bundle's flat sentence at `verification-results.md:290`
("PA-0457-01..08 are corrected in this revision") is **stronger than the tree supports**
while PA-0459-01 stands. The per-item tables are accurate; only the blanket sentence is not.

## 5. Configuration-management judgement (item 10)

Test of the reconstruction property: for every file with a managed suffix
(`.c/.h/.py/.sh/.lean/.cmake/.dts/.resc`) that is modified or untracked, is it either
represented by the base commit or bound by the manifest?

- All **18** modified tracked files are bound (checked one by one against the manifest).
- All **12** untracked `.py` gate inputs and all **6** untracked `.sh`/`.resc` files inside
  `MANAGED_ROOTS` are bound, as `CANDIDATE.md:50-61` states. The `.mjs` helper is bound
  although it is outside the licence checker's suffix set.
- The four pre-existing formatting-only test modifications are bound, so the tested test
  binaries are reconstructable.
- **One unbound managed file remains**: `src/main/fsw_main_reqtest.c` (not compiled; but
  inside `src`, therefore enumerated by `license_check.py`, and inside the cppcheck path
  set). See PA-0459-02. It changes no recorded result today, but the N13 input set is not
  fully reconstructed by base commit + manifest.
- Remaining exclusions are **honest**: `docs/vv/evidence/2026-11-02-cppcheck-canonical/
  cppcheck_scan.sh` and the two `docs/vv/evidence/2026-10-28-resume-gates/**/*.resc` are
  under `docs/`, which is not a `MANAGED_ROOTS` entry, so no gate scans them; they are
  historical evidence, not candidate inputs. `build2/`, `build3/` are build outputs (not
  managed roots, and regenerable); `third_party/n7s-obcp` is a dirty submodule; `.opencode/`,
  `opencode.json`, `tests/node/` and the two new untracked design notes do not feed any
  executed gate.
- The two `nm`-count flavours are now correctly attributed to their binutils, which removes
  the last place where a reader could mistake a host-tool artefact for a product defect.

**Conclusion:** base commit + manifest reconstructs the build/test/RTEMS configuration that
produced the recorded gate results. It does not *quite* reconstruct the N13 licence-check
input set (one unbound but passing file). That is a documentation/CM-hygiene defect, not an
integrity failure: the manifest verifies clean at 174/174 and the evidence package excludes
only its own two byproducts.

## 6. Residual open items

1. **PA-0459-01** — stale `tool-qualification.md` revision (rev 1.3 vs manifested 1.4) in
   `CANDIDATE.md:27`; also fix the blanket "all corrected" sentence at
   `verification-results.md:290`.
2. **PA-0459-02** — unbound `src/main/fsw_main_reqtest.c` participates in the N13/cppcheck
   input sets; either bind it or restate the exclusion rationale.
3. **PA-0459-O-01/O-02** — stale revision labels in `sprint-plan.md:101` and the 1.3
   pointers at `verification-results.md:279` / `sprint-plan.md:322`.
4. **PA-0459-O-03** — the review index needs a 0459 row (coordinator action).
5. **Carried over, unchanged:** T3 is **NOT FULLY QUALIFIED**; NC-EXT-19, NC-EXT-236,
   NC-EXT-237, NC-EXT-198/199, NC-PAL-01, NC-EXT-207/208/211 remain **open**; **N18 is
   FAIL**; the evidence-mapping document (`docs/vv/evidence/2026-11-02-n22-test-evidence-mapping.md`)
   is still an unreviewed content change that must be reviewed or reverted before any
   commit; hardware and human reviews are deferred under NCs.
6. **Explicit statements:** **no NC is closed** by this candidate or by this review; **no
   sprint is closed**; this report is **not release approval**, **not** a flightworthiness
   statement, **not** a tool qualification and **not** an independence certification. No
   commit or push was performed or authorised.

## 7. Confidence

```text
Decision/product and configuration: frick-fsw working tree on base 41dcbe2,
  manifest 135cb893…d12f87 (174 entries, verified exit 0)
Claim type: Fact (verification results) + Recommendation (closure verdict)
Applicable baseline and approved tailoring: N9/N10/N11/N15/N20/N21/N22 as
  recorded in AGENTS.md and docs/rules.md; ECSS-Q-ST-80C / ECSS-E-ST-40C
  invoked by the project, not re-derived here
Evidence consulted: candidate manifest + manifest-verify transcript; assets.sha256
  (re-run, 29 OK); gate10 logs/exit files; rtems-build-exit.txt and rtems-symbols.log;
  re-execution of riscv-rtems6-nm in frick-rtems-toolchain:local; re-execution of the
  two gate10 test binaries; CANDIDATE.md/README.md/plan/handover/TS-5/tool-qualification/
  verification-results/review index
Cross-check and reconciliation: manifest vs task-supplied hash (equal); test hash vs
  working tree (equal); ELF hash vs log (equal); nm counts vs log (equal); 110/4325
  check counts vs log (equal); review index vs cited IDs (complete)
Confidence: High for the eight closure verdicts and the manifest/evidence facts;
  Medium for anything depending on builds I did not re-run (build, CTest, Lean, trace,
  format, smoke, RTEMS link)
Basis and detection limits: no fresh full build; container-built binaries executed
  as retained; paywalled standards not consulted (not needed for a delta check)
Missing/inaccessible evidence: pre-format hash of test_evs_hs_health.c (declared
  unretained by the candidate); first failing format-check transcript (declared lost)
Assumptions and expiry/recheck date: verdict valid only for manifest
  135cb893…d12f87; re-check on any edit to CANDIDATE.md, tool-qualification.md,
  the manifest, or the test files
Consequences if wrong: a stale edition pointer or an unbound licence-check input
  would let a later reader audit the wrong configuration; neither changes a gate result
Required independent human reviewer/authority: the designated FSW engineer for the
  corrections, and the human owner/acceptance authority for any disposition
```

---

**End of FRICK-REVIEW-0459.** This report records findings and a recommendation only. It
does not close an NC, does not qualify a tool, and does not approve release.
