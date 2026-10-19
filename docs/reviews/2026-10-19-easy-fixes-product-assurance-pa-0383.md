|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0383                      |
| Reviewer session                  | Independent subagent context, fresh, norm N9 (no prior context of the batch) |
| Reviewer profile                  | PA (SW product assurance)              |
| Reference revision                | `7e93325` (HEAD, `main`, pushed to `origin` and `gitea`); reviewed batch = `318f49b` (engineering baseline), range `e7e48ee..318f49b` (27 files); status-only follow-ups `b6e1f61`, `7e93325` |
| Items under review                | The "easy-fixes batch" and its assurance record: `docs/vv/evidence/2026-10-19-easy-fixes-*.log`, `docs/vv/evidence/2026-10-19-s8-vv-fu-mutant-m3-discrimination.log`, `docs/vv/evidence/2026-10-17-s8-reconciliation-candidate-manifest.md` (2026-10-19 addendum), `docs/vv/verification-results.md` rev 2.91, `docs/vv/test-spec/ts-5-core-services.md`, `docs/vv/test-spec/ts-6-sw-robustness.md`, `docs/vv/verification-plan.md` §9, `docs/planning/sprint-plan.md` rev 1.72 (NC register), `CHANGELOG.md`, `README.md`, `docs/status/handover.md` §4, `docs/status/loop-audit.md` (N7/N11/N15/N17/N18/N20 rows), `tests/CMakeLists.txt`, `tests/unit/test_decoder_corpus.c`, and the carried reviews `FRICK-REVIEW-0378`/`0379`/`0380` |
| Date                              | 2026-10-19                             |
| Scope                             | Post-hoc N20 product assurance of a **published** batch (NC-EXT-99 deferral): evidence integrity and identity, candidate-manifest digest reproduction, NC-register integrity, gate honesty (N7/N11/N15/N17/N20), claim-vs-evidence audit, configuration management, and PA-record coherence. Against project norms `docs/rules.md` and the ECSS-Q-ST-80C / ECSS-E-ST-40C context invoked by `docs/quality/product-assurance.md` (ECSS Category B is a project target, not an assigned classification — NC-EXT-11). |
| Independence declaration          | The reviewer took no part in authoring, executing, reviewing or approving the batch and had no prior context; no conflict of interest is known. This review was performed by an AI assurance assistant, **not** a licensed/certified engineer, designated certification representative, auditor of record, or approval authority. |
| Norms applied                     | N4, N6, N8, N11, N15, N16, N17, N18, N19, N20, N21, N22 (see `docs/rules.md`) |

## 1. Scope

**Performed (PA remit, own commands, cheap verification only).** Independent
recomputation of every recorded artifact hash; independent reproduction of the
candidate-manifest addendum digest; inspection of each retained log against the
claims made for it; NC-register integrity review of the batch's NC entries;
gate-honesty review of `318f49b`; a bounded claim-vs-evidence audit on four named
claims; configuration-management checks; and a coherence check of the PA records.

**Explicitly excluded.** (a) Re-running `ci-local`, Docker or RTEMS builds, and any
mutation testing — excluded by the review assignment. (b) Deep audit of prose
wording (R4 review 0381) and re-derivation of the test-oracle logic (R2/R3 review
0382) — both are running concurrently; nothing here duplicates or pre-empts them.
(c) Assessment of the *technical* correctness of the `N22-TS6-57` vector, which is
R2/R3's to confirm. PA verifies that what the batch **claims** is supported by
evidence it **retains**, not that the engineering is optimal.

**This review is that deferred review.** NC-EXT-99 records that `318f49b` was
published without its own batch-scoped reviews. This report is the N20 PA part of
that deferred set. It is performed **post-hoc on the published tree**; it cannot
retrofit a pre-push gate and does not claim to.

## 2. References

- Project norms: `docs/rules.md` (N1–N22); `docs/quality/product-assurance.md`
  (FRICK-QUAL-PA-001 rev 0.4); `docs/vv/verification-plan.md` §9 (named sprint
  verification set S1–S10); `docs/quality/configuration-management.md`.
- Traceability manifest: `docs/requirements/traceability.yaml`.
- Reviews carried in the batch: `FRICK-REVIEW-0378` (R4), `FRICK-REVIEW-0379` +
  `0380` (R2/R3) — both made against the **pre-correction** tree `e7e48ee`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.
- External cross-check: GitHub Actions REST API (public repository
  `ilfrick/frick-fsw`), accessed 2026-10-19, for hosted CI run identity.

## 3. Findings

### 3.1 Evidence identity — all recorded hashes recomputed and matched

```
$ sha256sum docs/vv/evidence/2026-10-19-easy-fixes-*.log \
            docs/vv/evidence/2026-10-19-s8-vv-fu-mutant-m3-discrimination.log
```

| Artifact | Recorded in `verification-results.md` rev 2.91 | Recomputed on disk | Match |
| --- | --- | --- | --- |
| `...-asan.log` | `4e32e21463a363772bbdee364079afbdf60ade155d23516bf46322aea404a054` | same | yes |
| `...-ci-local.log` | `92c19b3e756e1d5660a23608da5b59121702cf0157f2e1b8c8b16aa6180d4c35` | same | yes |
| `...-docs.log` | `c7d1f9a5a8343199ee3ccaab25a107d6e6fdbcc53c6dc28badda6e8e8a6c019a` | same | yes |
| `...-format-check.log` | `da69e9e40937c195c997c92ad9a509240654dbf08859746b565d84a24de6c83e` | same | yes |
| `...-gsw-selfcheck.log` | `d1223ce32dafa765463f60b1390e3c6ff181479e62dd9603f9f77b61c03e74f1` | same | yes |
| `...-smoke.log` | `6e4327b56f09fb19532988b7c96d6f259063d377c35bec58723a0e2c2e343043` | same | yes |
| `...-mutant-m3-discrimination.log` | `70948df006385a0e79f4a00b100d042f8492c3338564cee9f5f31df9085089cb` | same | yes |

**7/7 hashes match. No mismatch.** The same seven hashes also match the rows in
the manifest addendum, so the evidence set is internally consistent across three
independent records.

Each log contains the result claimed for it (checked directly):

| Claim (`verification-results.md` rev 2.91, ts-5/ts-6, `verification-plan.md` §9) | Verified in the retained log |
| --- | --- |
| `make ci-local`: configure + build + **43/43 ctest** + Lean 62 + `trace.py` PASS | `100% tests passed, 0 tests failed out of 43` (line 372); `lean theorems found: 62` (395); `traceability: PASS (117 requirements, 73 srs, 30 sss, 14 gaps, 62 lean theorems checked)` (430). Tool header: docker `frick-vv:local`, Ninja Debug, `FF_USE_PUS_ASN1=ON` — matches §9 S1. |
| ASan leg **43/43 PASS** | `100% tests passed, 0 tests failed out of 43` (line 144); `Total Test time (real) = 94.02 sec` (157). |
| `make smoke` PASS | `[smoke] PASS (banner + 6 applications registered)` — matches §9 S2. |
| `make gsw-selfcheck` OK | 12 + 9 + 14 python tests `OK`; `MPL-2.0 license identity and authored-file SPDX check: OK` — matches §9 S3. |
| `make docs` with output-existence guard | Doxygen 1.9.8 runs to `finished...`; final line is `test -s build/doxygen/html/index.html || { echo "error: ..."; exit 1; }` with no error output — matches §9 S5 (warning count recorded, not "warning-free"). |
| pinned clang-format 18 `format-check` | Command recorded, no diagnostics emitted — consistent with a clean `--dry-run --Werror` (see F-0383-05). Matches §9 S6. |

The recorded **commands** match §9 S1/S2/S3/S5/S6 and the ts-5/ts-6 statements.
The `-fsanitize=address` configuration claim is corroborated by an artefact
outside the log itself:

```
$ grep -iE "CMAKE_C_FLAGS:|sanitize" build/asan/CMakeCache.txt
CMAKE_C_FLAGS:STRING=-fsanitize=address -fno-omit-frame-pointer
CMAKE_EXE_LINKER_FLAGS:STRING=-fsanitize=address
```

and by the as-committed workflows (`.github/workflows/ci.yml:61-65`,
`.gitea/workflows/ci.yml:64-68`), which build `build/asan` with the same flags —
confirming the ts-6 §3.1 "sanitizer leg" statement.

I also re-ran the cheap gate that is re-runnable on this host:

```
$ make trace
traceability: PASS  (117 requirements, 73 srs, 30 sss, 14 gaps, 62 lean theorems checked)
exit=0
```

identical to the figure in the retained `ci-local` log. `python3 tools/scripts/metrics.py --check`
returned `metrics panel: current` (exit 0), confirming the README metrics panel
was refreshed with the batch.

### 3.2 Candidate-manifest addendum digest — reproduced exactly

The addendum's stated method is: sorted union of `git diff --name-only e7e48ee`
and `git ls-files --others --exclude-standard`; `<sha256>  <path>` per path; the
manifest's own row substituted with the empty-file hash (documented bootstrap
convention, not a self-hash). I implemented that method independently and, using
the blob contents of the commit that captures the delta, obtained:

```
union count: 27
digest: 8f7bbb3ffbeb584877b8f1ff44548057f62f9a62af4194ab2516882a5629425c
recorded: 8f7bbb3ffbeb584877b8f1ff44548057f62f9a62af4194ab2516882a5629425c
```

**Exact match. Path count 27 confirmed.** All 27 per-path rows were also checked
individually against `318f49b` blobs: `rows mismatching @318f49b: 0` (hash and
byte size). The method is therefore **repeatable**, and the batch's identity is
**reconstructable by a third party from the repository alone** (see F-0383-04 for
the one caveat about which tree to use).

The addendum's distinction is **clear and honest**: it states that the inventory
above is "the PA-facing candidate snapshot" at `e7e48ee` and that the table below
is "the follow-on batch delta and is **not** independently PA-verified". It
separately flags that "the batch's own batch-scoped independent reviews are
deferred as NC-EXT-99". The two inventories are not conflated anywhere I could
find, and no document presents this addendum as PA verification of the batch.

### 3.3 NC-register integrity (`docs/planning/sprint-plan.md` rev 1.72)

| NC | Description real? | Owner / verifier | Explicit status | Closed? |
| --- | --- | --- | --- | --- |
| `NC-EXT-97` | Yes — concrete `SRS-REQ-018` vs `SRS-REQ-050`/`ADR-0019` backend-default contradiction, with line references and the reason it is not a unilateral edit | requirements authority (R3, N14) with FSW / verifier: independent R3 | **OPEN** | No |
| `NC-EXT-98` | Yes — residual `N22-TS5-50` reset-path case (no `FF_PalPanic` stub exists in `tests/`) and ES/HS anomaly context | FSW/V&V / verifier: independent R2 | **OPEN** | No |
| `NC-EXT-99` | Yes — names the three missing batch-scoped reviews, the norms (N17/N22/N10/N20), and the pre-correction-tree status of the carried reviews | FSW (request/coordinate) / verifier: the respective independent profiles | **OPEN — deferred by explicit owner disposition**; "Closure requires the R4/R2-R3/PA reports to be produced and their findings processed under N12" | **No — correctly not presented as a closure** |
| `NC-EXT-94` (widened) | Yes — widened from "subservice 6 only" to "neither 4 nor 6 has an emission path; emitted set {1,2,3,5,7,8}", with the re-derivation from `src/pus/pus.c` | FSW + interface/requirements authority / verifier: independent R3/R4 | **OPEN** | No |
| `NC-EXT-96` (updated) | Yes — records R4 confirmation complete and **recommends** narrow closure as "an owner disposition, not a self-declared closure" | FSW/documentation / verifier: independent R4 | **OPEN** | No |

**No NC was silently closed.** The batch introduced no `CLOSED` line at all:

```
$ git diff e7e48ee 318f49b -- docs/planning/sprint-plan.md | grep -E "^\+.*CLOSED"
(no output)
```

`GAP-R3-1` and `NC-EXT-25` remain **OPEN** in every record that mentions them. The
register is the correct home for this review's findings (§6 below) and is
maintained under `docs/quality/product-assurance.md` §2 "Non-conformance control".

**Independent corroboration of the NC-EXT-99 factual premise.** The claim that the
carried reviews were made against the *pre-correction* tree is verifiable and I
verified it, rather than trusting the assertion:

```
tests/unit/test_decoder_corpus.c   e7e48ee=fba54865...  318f49b=c789942e...  same=False
docs/vv/test-spec/ts-5-core-services.md   e7e48ee=b95d5730...  318f49b=28b08620...  same=False
docs/vv/test-spec/ts-6-sw-robustness.md   e7e48ee=d3e897cd...  318f49b=8b8ee288...  same=False
tests/unit/test_es_hs_lifecycle.c  e7e48ee=060256cd...  318f49b=060256cd...  same=True
```

`docs/reviews/2026-10-18-s8-vv-fu-delta-vv-review.md` cites the **`e7e48ee`** hashes
of `test_decoder_corpus.c`, TS-5 and TS-6, and the (unchanged) hash of
`test_es_hs_lifecycle.c`. The reviews genuinely did not see the corrections. The
NC-EXT-99 description is factually accurate, not a self-serving label.

### 3.4 Gate honesty for `318f49b`

| Gate | Status for `318f49b` | Evidence (verified by this review) |
| --- | --- | --- |
| Build + unit tests (`make ci-local`) | **SATISFIED** | `100% tests passed, 0 tests failed out of 43`; Lean 62 theorems; `traceability: PASS`; log hash matches |
| Traceability (N4) | **SATISFIED** | `make trace` re-run at HEAD: PASS, identical counts |
| Lean (N1) | **SATISFIED** | `lean theorems found: 62` in the retained log |
| Format / whitespace (N15) | **SATISFIED** | pinned clang-format 18 log, no diagnostics (record-format caveat F-0383-05) |
| Smoke (N15) | **SATISFIED** | `[smoke] PASS (banner + 6 applications registered)` |
| GSW / metrics / N13 license (N13/N15) | **SATISFIED** | 35 python checks `OK`; MPL-2.0 identity `OK`; `metrics.py --check` → `metrics panel: current` |
| Documentation generation (N8) | **SATISFIED** | Doxygen 1.9.8 to `finished...` + output-existence guard, no error |
| ASan leg | **SATISFIED** | 43/43 in log; flags corroborated by `build/asan/CMakeCache.txt` and both CI workflows |
| RTEMS 6.2 cross-build (N15/§9 S7) | **SATISFIED for this commit, but not by the batch's own retained set** | Hosted CI run `39218060325`, job "RTEMS 6.2 cross build (riscv/mpfs64imafdc)" = `success` (GitHub API, head_sha `318f49be2a31df9079f0677451b3ab38df723b47`). See F-0383-03 |
| Emulated-target functional validation — Renode (N18/§9 S8) | **NOT evidenced for this batch** | No Renode log in the batch; no Renode job exists in `.github/workflows/ci.yml` (8 jobs, none is Renode). Defensible non-application (no `src/` change) but not recorded. See F-0383-03 |
| Hosted CI | **SATISFIED — independently confirmed** | GitHub API: run `39218060325`, `head_sha 318f49be2a31df9079f0677451b3ab38df723b47`, `status completed`, `conclusion success`, `total_count: 8` with all 8 jobs `success` (documentation set, Lean verification, simulator smoke, traceability gate, RTEMS 6.2 cross build, build/unit tests, static analysis, pipeline summary). The "all eight jobs" assertion in `loop-audit.md` N7 is **correct**, and the 8-job count matches the 8 jobs defined in the workflow |
| **Batch-scoped N17 R4 documentation review** | **NOT SATISFIED at commit** | Deferred, `NC-EXT-99` |
| **Batch-scoped N22/N10 R2/R3 V&V confirmation** | **NOT SATISFIED at commit** | Deferred, `NC-EXT-99` |
| **Batch-scoped N20 PA review** | **NOT SATISFIED at commit** | Deferred, `NC-EXT-99`; **this report is that deferred review, performed post-hoc** |

**Is the deviation recorded truthfully and visibly enough? Yes.** It appears, consistently
and without dilution, in **five** independent places, all of which I read:

- `CHANGELOG.md` ("**Recorded deviation — NC-EXT-99**", ending "NC-EXT-99 is **not** closed and no gate, phase, qualification or release approval is implied");
- `docs/planning/sprint-plan.md` NC-EXT-99 row and history row 1.72;
- `docs/status/handover.md` §4 (naming NC-EXT-99 and the resume point);
- `docs/status/loop-audit.md` **N7, N11, N17 and N20 rows**, each stating the batch-scoped reviews were **not** obtained before commit;
- `docs/vv/verification-results.md` rev 2.91 ("Recorded review-coverage deviation").

I swept the whole batch diff for overclaiming language (`fully reviewed`, `all
gates passed`, `N20 complete`, `phase approval`, `qualification complete`,
`release approved`, `compliant with`, `zero defects`). **No overclaim was
detected.** Every match was a sentence *disclaiming* approval. `loop-audit.md` N20
states in terms that "the batch-scoped PA review was **not** obtained before
commit". The deviation is recorded truthfully and is visible enough that a reader
entering at any of the five documents reaches the same conclusion.

### 3.5 Claim-vs-evidence audit

| Claim tested | Method | Result |
| --- | --- | --- |
| "43 registered ctest tests; 41 without lwIP" (ts-6 §3.1) | `grep -c add_test tests/CMakeLists.txt` → `43`; `ctest -N --test-dir build/native` → `Total Tests: 43`; lwIP tests behind `if(FF_USE_LWIP)` (`tests/CMakeLists.txt:115-118,204-206`) → 2 tests | **SUPPORTED.** 43 − 2 = 41. The 14-unconditional + 29-guarded breakdown also reconciles (20-test `FF_BUILD_DRIVERS` set + hal/math/sdls/link_uart/obcp/n7/pusopen = 27, + 2 lwIP = 29; + 14 = 43). The two identical `93.10 sec*proc` label entries are the *same* test listed under two labels (`sb_evs_structural_sweep` carries `LABELS "tool-qualification;sb-evs"`, `tests/CMakeLists.txt:145`), **not** a duplicate registration — `add_test` names are unique |
| README metrics panel currency | `python3 tools/scripts/metrics.py --check` → `metrics panel: current`, exit 0 | **SUPPORTED** |
| "the corrected min-PDL sub-case is discriminating" | Read the retained mutation log end to end | **SELF-CONSISTENT and AUDITABLE.** Header records date, tool (`host build/native (Debug, gcc)`), norms, the originating finding (`FRICK-REVIEW-0379 F-0379-01`), the exact mutation (`remove the 'declared_data_len < minimum_data_len' check in ff_ccsds_packet_data_length()`), expected outcome, and restoration. Body contains exactly **15** `FAIL` lines and a closing `checks=14941 failures=15` — the claimed failure count matches the log's own content. The claimed context `template=5 iter=5` is present. Restoration is corroborated: `src/transport/ccsds/ccsds.c` still contains `if (declared_data_len < minimum_data_len)` (line 54) and `src/` contributes **0** of the 27 changed paths. I did not re-run the mutation (excluded by scope) |
| ASan `-fsanitize=address` configuration | `build/asan/CMakeCache.txt` + both committed CI workflows | **SUPPORTED** (see §3.1) |

### 3.6 Configuration management

- `git diff --name-only e7e48ee 318f49b` → **27 paths**, all tracked and committed.
  `git ls-files --others --exclude-standard` → **empty**. No evidence is dangling
  outside git; every log cited is itself in the manifest.
- Working tree clean at HEAD apart from this report (verified before and after
  writing; §7).
- `git diff --name-only 318f49b 7e93325` → `docs/status/handover.md`,
  `docs/status/loop-audit.md` **only**. Each of `b6e1f61` and `7e93325`
  individually touches exactly those two files (`2 files changed` each). The
  "status-only" characterization is **true**.
- `318f49b` is an ancestor of both `origin/main` and `gitea/main`
  (`git merge-base --is-ancestor`), and both remotes are at `7e93325`. The dual-remote
  claim is true.

### 3.7 Findings table

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| F-0383-01 | **Minor** | `docs/vv/verification-results.md` rev 2.91 (§"Easy-fixes batch", ASan bullet; history row 2.91) | **Unsupported numeric claim in a retained evidence record.** The ASan leg is recorded as "**43/43 PASS** (166.36 s)", but the retained log ends `Total Test time (real) = 94.02 sec`, and the string `166.36` occurs in **no** retained evidence file (`grep -rl "166.36" docs/vv/evidence/` → empty). The substantive claim (43/43 PASS) *is* supported and the log hash matches; the defect is an attribute of the run that the evidence does not carry. | `grep -n "166" docs/vv/evidence/2026-10-19-easy-fixes-asan.log` → no match; `grep -n -A12 "Label Time Summary" ...asan.log` → `Total Test time (real) = 94.02 sec` | Correct the figure to the logged value, or record the full-leg wall time in the log itself. An unsupported number in a retained V&V record is not acceptable even when the pass/fail result is sound; if it had concerned the result or a hash it would be Major. |
| F-0383-02 | **Minor** | `docs/vv/verification-results.md` rev 2.91, section heading + first bullet | **Stale currency: the evidence record still describes a published batch as uncommitted.** Heading reads "(2026-10-19; uncommitted)" and the scope bullet reads "Working-tree-only batch on top of the pushed baseline `e7e48ee`", although the batch is published as `318f49b`. `handover.md` (3.08/3.09) and `loop-audit.md` (10.10/10.11) were both repointed to `318f49b`; `verification-results.md` was not. A reader reconciling the evidence record to a published commit is misled about the product state. | `sed -n '25,30p' docs/vv/verification-results.md`; `git merge-base --is-ancestor 318f49b origin/main` → yes | Update the heading and scope bullet to name the publishing commit `318f49b`, preserving the point-in-time "uncommitted when the evidence was generated" note as history. |
| F-0383-03 | **Minor** | `docs/vv/verification-plan.md` §9 vs the batch evidence set; `docs/status/loop-audit.md` N15 row | **Every-sprint verification set is not fully accounted for.** §9 requires that "a member that cannot run on a given host is recorded as SKIP with the reason and is never counted as a pass". The batch retains no RTEMS (S7) or Renode (S8/N18) result and records no non-application rationale, while the N15 row lists "Renode and RTEMS 6.2 cross-build pass" as current — those entries are inherited from the 2026-10-17 cycle, not re-run for this batch. RTEMS is in fact covered for this commit by hosted CI run `39218060325` (verified). **Renode/N18 is covered nowhere for this batch.** Non-application is defensible — the batch changes **0** `src/` paths, so the emulated target image is unaffected — but that argument is not written down, and N18 is stated as applying to *every* change. | `sed -n '24,110p' docs/vv/verification-results.md \| grep -niE "renode\|rtems\|SKIP"` → no match; GitHub API job list for run `39218060325` (8 jobs, no Renode job); `git diff --name-only e7e48ee 318f49b \| grep -c "src/"` → `0` | Record S7 as satisfied-by-hosted-CI with the run id, and record S8/N18 as explicitly non-applicable (no product-source change) or as SKIP with the reason, per §9. |
| F-0383-04 | Comment | candidate-manifest addendum, "Method" bullet | The addendum records the baseline (`e7e48ee`) and the digest but does not name the commit that captures the delta. Because the digest is computed over **working-tree** content, it is **not stable at HEAD**: regenerating at `7e93325` yields `ec3a4882ec702e11e043409de0b7831d940c762c55950bac4d34e9f9c9e05dd0` (the two status-only commits changed `handover.md`/`loop-audit.md`, both in the set). I reproduced the recorded value only by taking the manifest's own containing commit as the tree. The method is repeatable, but only with a tree identity the addendum does not state. | `python3` re-run at HEAD → `ec3a4882...`; re-run at `318f49b` → `8f7bbb3f...`; `rows mismatching @HEAD(7e93325): 2 ['docs/status/handover.md', 'docs/status/loop-audit.md']` | Record `318f49b` in the addendum (or the companion `verification-results.md` entry) as the tree the digest binds to, so a third party reproduces it first time. |
| F-0383-05 | Comment | `...-format-check.log`, `...-docs.log` | The format-check log records the tool, the command and nothing else — no exit status, no "PASS" marker; the docs log's last line is the output-guard command with no output. The PASS attribution rests on the absence of output. This matches the project's established convention (prior format logs are 92 bytes), so it is an observation on a weak record format, **not** a defect introduced by this batch, and I found no contrary signal. | `cat docs/vv/evidence/2026-10-19-easy-fixes-format-check.log` (4 lines); manifest rows for `2026-10-16-*-format.log` (92 B) | Consider appending `echo "exit=$?"` (or an explicit `PASS`) to such logs so the record is positively self-verifying rather than inferentially so. |

**No Major finding. No false evidence claim about a pass/fail result. No hash or
identity mismatch. No silently-closed NC.** I state this plainly rather than
manufacturing severity.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] **APPROVED WITH COMMENTS** - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**What this conclusion covers.** The evidence identity, integrity and
reproducibility of the easy-fixes batch are **sound**: all 7 recorded artifact
hashes match the files on disk, the candidate digest reproduces exactly
(`8f7bbb3ffbeb584877b8f1ff44548057f62f9a62af4194ab2516882a5629425c`, 27 paths,
27/27 per-path rows correct), the NC register is honest (nothing silently closed,
`NC-EXT-99` correctly not a closure), the batch is fully committed with a clean
tree, and the recorded review-coverage deviation is truthful and visible in five
places. The batch is **fit to stand as the engineering baseline** on the
understanding that its own batch-scoped reviews were not obtained before publish,
exactly as `NC-EXT-99` states.

Three Minor and two Comment findings are tracked to closure (§6). None of them
invalidates a recorded result; each is a correction to the assurance record.

### Confidence

```text
Decision/product and configuration: easy-fixes batch, engineering baseline `318f49b`,
  published on `origin`/`gitea`; PA scope only (evidence identity, NC register, gate
  honesty, claim-vs-evidence, CM).
Claim type: Fact (hash/digest reproduction, log content, NC status, CI API result,
  git state) + Recommendation (findings) + Open issue (F-0383-01/-02/-03).
Applicable baseline and approved tailoring: `docs/rules.md` N1-N22; FRICK-QUAL-PA-001
  rev 0.4; `verification-plan.md` §9 S1-S10; ECSS Category B is a project target, not
  an assigned classification (NC-EXT-11) - no mission/customer tailoring was supplied
  to this review.
Evidence consulted: 7 evidence logs (full text), candidate manifest + addendum (full
  text), sprint-plan rev 1.72 NC register, verification-results rev 2.91, ts-5/ts-6,
  verification-plan §9, CHANGELOG, handover §4, loop-audit N-rows, tests/CMakeLists.txt,
  tests/unit/test_decoder_corpus.c, src/transport/ccsds/ccsds.c, both CI workflows,
  build/asan/CMakeCache.txt, and the GitHub Actions REST API (2026-10-19).
Cross-check and reconciliation: manifest digest and all 27 rows reproduced independently
  and matched; hosted CI claim cross-checked against an independent authoritative source
  (GitHub API) rather than accepted from the docs; carried-review tree state
  cross-checked against blob hashes.
Confidence: High.
Basis and detection limits: hashes, the digest, log contents, git state and the CI run
  are directly recomputed, so identity/integrity conclusions are high-confidence. Limits:
  (a) I did not re-run ci-local/Docker/RTEMS or the mutation (excluded by scope), so a
  passing run is attested by the retained log + its hash + hosted CI, not by my own
  execution; (b) ASan flag corroboration via build/asan/CMakeCache.txt is local,
  gitignored working state, not a committed artefact; (c) test-oracle correctness (R2/R3
  0382) and prose wording (R4 0381) were out of scope and are not covered here.
Missing/inaccessible evidence: no Renode/N18 record for this batch (F-0383-03); Gitea
  Actions remains unverified (host is not reachable as an independent source from here).
Assumptions and expiry/recheck date: assumes `docs/rules.md` and FRICK-QUAL-PA-001 rev
  0.4 are the approved baseline (no customer tailoring was supplied). Recheck at the next
  cycle, or on any change to the batch, the evidence logs or the NC register.
Consequences if wrong: a mis-recorded ASan duration (F-0383-01) is cosmetic unless
  extended to pass/fail claims; stale "uncommitted" wording (F-0383-02) could cause a
  reader to reconcile evidence to the wrong tree; an undocumented Renode
  non-application (F-0383-03) weakens the N18 argument for future changes.
Required independent human reviewer/authority: the owner/authorized authority for any
  closure disposition, release, phase or gate decision, and the requirements authority
  (R3/N14) for NC-EXT-97.
```

### What this review does NOT approve

This report is **not** and must not be read as:

- **closure of `NC-EXT-99`** — closure requires the batch-scoped R4 (0381) and R2/R3 (0382)
  reports to be produced and *all* their findings processed under N12, plus an owner
  disposition. I cannot close it and do not recommend closing it on this report alone;
- **release, flight, qualification, tool-qualification, phase or sprint approval** —
  I have no such authority and grant none;
- **approval of any open NC**, in particular `GAP-R3-1`, `NC-EXT-25`, `NC-EXT-84`,
  `NC-EXT-90`, `NC-EXT-92`, `NC-EXT-97`, `NC-EXT-98`, `NC-EXT-99`, `NC-S9-01`, or the
  authority-owned items — all remain OPEN;
- **a PUS-C compliance claim, a target/HIL evidence claim, or a safety/security
  acceptance**;
- **retrofitting the pre-push gate** that NC-EXT-99 records as missing. The deviation
  happened; this review examines it afterwards;
- **approval of the test-oracle logic or of prose wording**, which are the concurrent
  R2/R3 (0382) and R4 (0381) scopes.

Under N21, all findings below are non-functional and recorded for later disposition;
that does not close, waive or accept any of them.

## 5. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
| ---- | ---------------- | ---- | ---------- |
| PA (SW product assurance, N20) | independent subagent, fresh context (FRICK-REVIEW-0383) | 2026-10-19 | **APPROVED WITH COMMENTS** — 3 Minor + 2 Comment; no Major; no NC closed; no gate, phase, qualification or release approval |

## 6. Follow-up records

Norm N12: NCs are resolved by re-applying **all** rules to the affected scope and
re-reviewing the deltas until full compliance. The home for these entries is the
NC register in `docs/planning/sprint-plan.md` §3.2 (NC-EXT series), maintained
under `docs/quality/product-assurance.md` §2. **This report file is the only file
this review created; the NC rows below are proposed for the owner/FSW engineer to
register in the sprint plan — they are not yet registered.** That is itself
recorded so the gap is not lost.

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| *(proposed, owner to assign)* | Minor | `docs/vv/verification-results.md` rev 2.91 — ASan bullet + history row | F-0383-01: recorded ASan duration "(166.36 s)" is supported by no retained log; the log records 94.02 s | Correct to the logged value, or record the full-leg wall time in the log | Re-inspect the corrected line against the log; recompute the log hash if the log is touched | FSW / documentation | **OPEN** | R4/N17 review of the corrected document |
| *(proposed, owner to assign)* | Minor | `docs/vv/verification-results.md` rev 2.91 — section heading + scope bullet | F-0383-02: batch still described as "uncommitted"/"working-tree-only" after publication as `318f49b` | Repoint to `318f49b`, preserving the point-in-time note as history | R4 confirmation that the record names the publishing commit | FSW / documentation | **OPEN** | R4/N17 review of the corrected document |
| *(proposed, owner to assign)* | Minor | `docs/vv/verification-plan.md` §9 S7/S8; `docs/status/loop-audit.md` N15 | F-0383-03: no RTEMS/Renode result and no explicit non-application rationale for the batch; N15 row presents inherited entries as current | Record S7 as satisfied-by-hosted-CI (run `39218060325`) and S8/N18 as non-applicable (0 `src/` paths) or SKIP with reason | The recorded rationale/SKIP in `verification-results.md`; consistency with the N15 row | FSW / V&V | **OPEN** | R4/N17 + PA confirmation |
| *(proposed, owner to assign)* | Comment | candidate-manifest addendum "Method" bullet | F-0383-04: digest is tree-bound but the binding commit is not stated; not stable at HEAD | Record `318f49b` as the tree the digest binds to | Independent third-party reproduction to the recorded digest | FSW / PA | **OPEN** | Bounded PA re-verification |
| *(proposed, owner to assign)* | Comment | `...-format-check.log`, `...-docs.log` record format | F-0383-05: PASS attributed from absence of output; no exit status recorded | Append `exit=$?`/`PASS` to such logs in a future cycle | Inspection of the next cycle's log | FSW / V&V (N10 tooling) | **OPEN** | Tool-qualification note under N10 if the log format changes |
| `NC-EXT-99` (existing) | Process/review-coverage | 2026-10-19 easy-fixes batch, `318f49b` | Batch published without its own batch-scoped N17 R4, N22/N10 R2/R3 and N20 PA reviews | **Not closable by this report.** Requires the R4 (0381) and R2/R3 (0382) reports plus processing of their findings under N12, then an authorized owner disposition | This report (N20 PA part) + `2026-10-18-s8-vv-fu-delta-vv-review.md`-style outputs for 0381/0382 | FSW (request/coordinate); verifier: respective independent profiles | **OPEN** | Yes — batch-scoped R4 + R2/R3 still outstanding |
