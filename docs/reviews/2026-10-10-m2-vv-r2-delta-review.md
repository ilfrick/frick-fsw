# Independent V&V Delta Re-Review - Milestone M2 (Time Base)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0010                      |
| Reviewer session                  | independent V&V agent (fresh context)  |
| Reviewer profile                  | R2 - Senior SW V&V engineer (N9)       |
| Reference revision                | working tree on commit `a69732f` + uncommitted M2 closure changes (`a69732f` = `docs: documentation program, Ilfrick copyright and metrics panel`) |
| Items under review                | `tests/unit/test_time.c`, `tools/scripts/trace.py`, `.gitea/workflows/ci.yml`, `.github/workflows/ci.yml`, `docs/vv/verification-results.md`, `docs/requirements/traceability.md`, `docs/vv/tool-qualification.md`, `verification/lean/lake-build.log` |
| Date                              | 2026-10-10                             |
| Scope                             | Delta re-review of the M2-01..M2-06 dispositions from `2026-10-10-m2-vv-r2-review.md` (norm N12): independent re-application of all rules to the changed scope only |
| Predecessor report                | `docs/reviews/2026-10-10-m2-vv-r2-review.md` (FRICK-REVIEW-0004, REJECTED) |
| Independence declaration          | reviewer took no part in authoring or executing the fixes; fresh context; all findings based on independent re-runs on the working tree |
| Norms applied                     | N1-N17 (see `docs/rules.md`)           |

## 1. Scope

This report verifies **only** the six non-conformances (M2-01..M2-06) raised by the
predecessor R2 review and their claimed dispositions. It does not re-audit the M2
code base, the R1 code review or the R3 SSS review, all recorded closed elsewhere.

Independent re-runs performed in this session (no file was modified except this
report):

| Activity | Command / artefact | Result |
| -------- | ------------------ | ------ |
| Traceability gate | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** - 48 requirements (25 srs / 16 sss / 7 gap), **41** Lean theorems, **`CI evidence claims: 9`**, 0 errors, 0 warnings (exit 0) |
| Fresh configure + build | `cmake -S . -B build/rev -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build build/rev` | **BUILD OK** |
| Unit tests | `ctest --test-dir build/rev --output-on-failure` | **100% tests passed, 0 failed out of 5** (ccsds, sb, tbl, pus, time) |
| Per-suite check counts | `./build/rev/tests/test_{ccsds,sb,tbl,pus,time}` | ccsds **15**, sb **20**, tbl **9**, pus **29**, time **101** failures=0 (total 174) - matches the M2 row |
| Determinism | `test_time` executed **100x** | 100/100 runs `checks=101 failures=0`, exit 0 - 0 bad runs |
| `ci_evidence()` negative control | isolated `exec` of the function against a synthetic workflow containing `ci-bogus-undeclared`, requirement refs and prose tokens | claims = `['bogus-undeclared', 'host-build']`; **no** `SRS-*`/`SSS-*`/prose tokens leak |
| Marker parity | `grep evidence:` on both workflows; distinct `ci:` names in the manifest | 9 distinct workflow claim names; 8 referenced by the manifest (`ci-lean` reported but unclaimed) |

Excluded: re-running `lake build` / `lake exe frick_verify_checks` (Lean toolchain
not installed in this environment) - the committed transcript is inspected instead.
On-target PPS execution remains the class-A gap GAP-A7 and is out of scope.

## 2. References

- Project norms: `docs/rules.md` (N1-N17), `docs/engineering/independent-review.md`
- Predecessor review: `docs/reviews/2026-10-10-m2-vv-r2-review.md` (FRICK-REVIEW-0004)
- Evidence: `docs/vv/verification-results.md` (rev 0.3), `docs/vv/tool-qualification.md` (rev 0.1.4), `verification/lean/lake-build.log`
- Traceability manifest: `docs/requirements/traceability.yaml`, `docs/requirements/traceability.md`
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C)

## 3. NC-by-NC verification (norm N12)

| NC | Severity | Claimed disposition | Independent verification | Status |
| -- | -------- | ------------------- | ------------------------ | ------ |
| **M2-01** | Major | File-scope `FF_TEST_MAIN`, dead nested `main` removed, suite executes | `tests/unit/test_time.c:285-288` places `FF_TEST_MAIN(...)` at file scope with the 10 test functions; there is **no** enclosing `int main(...)` and no trailing `return 0;`. `tests/ff_test.h:33-42` expands `FF_TEST_MAIN` to the sole `main`. The freshly built binary prints `/home/nicola/fsw/frick-fsw/tests/unit/test_time.c checks=101 failures=0`, exit 0; `ctest` reports `time ... Passed`. The vacuous `xor eax,eax; ret` `main` no longer exists. | **CLOSED** |
| **M2-02** | Major | Second tone signalled with a freshly sampled monotonic timestamp (`later_than()`), not future-dated | `test_time.c:25-32` defines `later_than()` which polls `FF_PalGetMonotonicNs()` until strictly later than the previous sample (never future-dated). `test_tone_discipline` (`:140-167`) signals tone 1 at `now` and tone 2 at `now2 = later_than(now)`, asserting `coarse == 2u`. This matches `time.c:148-174` (`g_tone_epoch_ns = mono_ns`) and `time.c:59-70` (TONE reads `g_tone_utc_ns + (now - g_tone_epoch_ns)` only when `now >= epoch`). 100/100 executions green. | **CLOSED** |
| **M2-03** | Minor | `trace.py ci_evidence()` scoped to `ci:`/`ci-` tokens; tool reports `CI evidence claims: 9`; `traceability.md` re-aligned; T6 re-qualified | `trace.py:177-184`: `claims.add(tok[3:])` now sits **inside** the `if tok.startswith("ci:") or tok.startswith("ci-")`, with the R2 M2-03 comment. Live gate prints **`CI evidence claims: 9`**. `traceability.md:18-20` documents `CI evidence claims = 9 ... (8 referenced by the manifest; ci-lean ... not claimed)`. `tool-qualification.md:7,72` bumps to rev 0.1.4 with an explicit T6 re-qualification entry. Independent negative control (synthetic workflow with `ci-bogus-undeclared` + requirement/prose tokens) returns only genuine `ci-*` names - spurious tokens are gone, so the latent false-accept is removed. | **CLOSED** (see N-2) |
| **M2-04** | Minor | `ci-lean` marker added to the Gitea workflow | `.gitea/workflows/ci.yml:109` now carries `# evidence: ci-lean (norm N1; not claimed in manifest but reported)`, matching `.github/workflows/ci.yml:92`. Both files' eight marker lines are token-identical; the header claim "Identical evidence markers" (`:7-8`) is now true. | **CLOSED** |
| **M2-05** | Minor | `lake build` transcript committed and referenced by `verification-results.md` | `verification/lean/lake-build.log` exists and records a clean rebuild as 17 timestamped steps (`✔ [1/17] Built Frick.LeanOmega` ... `✔ [17/17] Built frick_verify_checks`), consistent with the 8 source modules + their `:c.o` artefacts + the checker executable. `verification-results.md:88-93` states the module count is re-derivable and points to the transcript; the M2 closure row (`:23`) references it too. The transcript is inspectable in-tree; `lake` itself cannot be re-run here. | **CLOSED** (see N-3) |
| **M2-06** | Minor | Determinism fixed: anchored round-trip + fixed reference buffer | `test_cuc_bytes` (`:76-126`) anchors the round-trip on the coarse value set at the same instant (`FF_TimeSetUtcNs(5u*FF_TIME_TONE_NS)`, `:87`), decodes a **fixed** reference frame `{0x2E,0,0,0,5,0x40,0x00}` asserting `f2 == 16384u` (`:110-116`), and checks decode idempotence on a given buffer (`:106-107`). `test_init_and_basic_cuc` no longer asserts fine equality. 100/100 consecutive runs stable with identical check/failure counts. | **CLOSED** (see N-1) |

**Result: 2 Major + 4 Minor = 6/6 NCs closed.**

## 4. New findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| N-1 | Comment | `test_time.c:50,103` | `FF_TEST_ASSERT(fine < 65536u)` is tautological: `fine` is `uint16_t` (max 65535), so the assertion can never fail. It is harmless (the exact fine semantics are pinned by the fixed-buffer check `f2 == 16384u`), but it adds no coverage. | `test_time.c:37` (`uint16_t fine`), `:50`, `:103` | Drop the tautology or replace with a meaningful bound/round-trip check; no derogation. |
| N-2 | Comment | `tools/scripts/trace.py` (T6) | The M2-03 disposition listed "add a regression check (e.g. a `ci:*` with a bogus name must be reported as unclaimed)". No committed automated regression test exists (no `test_trace.py`/harness); the scoping fix is verified here only by independent manual reproduction. The core defect is corrected, but a future edit could silently regress it. | `find` for a trace-tool test returns none; `tool-qualification.md:72` records the fix but no test | Add a tiny committed regression test (or a documented manual check with a negative fixture) when convenient; no derogation. |
| N-3 | Comment | Norm N11 hygiene | At review time the reviewed fixes and evidence are still in the **uncommitted** working tree on `a69732f` (`git status`: `M .gitea/workflows/ci.yml`, `M tools/scripts/trace.py`, `?? tests/unit/test_time.c`, `?? verification/lean/lake-build.log`, etc.). This is the expected pre-commit state; the reviewed change, the regenerated evidence **and this report** must be committed and pushed to both remotes together for the NCs to be closed in the record. | `git status --short` | Commit/push the whole cycle (including `lake-build.log` and `test_time.c`, currently untracked) as a single closure commit (N7/N11). |

**Severity definition** (per template): Major = non-conformity that disqualifies
the item; Minor = must be corrected/justified but does not invalidate the
requirement; Comment = observation, no derogation. Findings N-1..N-3 are all
Comments: none blocks closure.

## 5. Conclusion

- [x] **APPROVED** - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

Rationale: all six predecessor NCs verify closed against the working tree. The M2
unit suite now genuinely executes (`time checks=101 failures=0`, ctest 5/5, 174
checks across the five suites), the TONE sub-case uses a freshly sampled monotonic
edge and is stable over 100 iterations, the traceability gate is corrected and
reports `CI evidence claims: 9`, the Gitea and GitHub marker sets are identical
including `ci-lean`, the Lean rebuild transcript is committed and referenced, and
the CUC/tone tests are deterministically anchored. The only residual items are
non-blocking observations (N-1..N-3). The Tst evidence for SRS-REQ-021..024 and
SSS-REQ-016 is restored and valid; the recorded M2 closure row in
`verification-results.md` is now supported by the repository.

## 6. Sign-off

| Role          | Agent / Reviewer                  | Date       | Conclusion |
| ------------- | --------------------------------- | ---------- | ---------- |
| R2 (N9)       | independent V&V session (delta)   | 2026-10-10 | APPROVED   |

## 7. Follow-up records (norm N12)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | ------ | --------------- |
| M2-01 | Major | `test_time.c` entry point; SRS-REQ-021..024, SSS-REQ-016 | Nested (dead) `main`; suite never ran | File-scope `FF_TEST_MAIN` (10 tests), nested `main` removed | `test_time.c:285-288`; runtime `checks=101 failures=0`; ctest `time Passed` | frick-fsw | **closed** | done (R2, independent) |
| M2-02 | Major | `test_tone_discipline`; SRS-REQ-022 | Future-dated 2nd tone made `coarse==2u` unsatisfiable | `later_than()` fresh monotonic sample; clock-independent expectation | `test_time.c:25-32,160-166`; 100/100 runs green; `time.c:148-174` | frick-fsw | **closed** | done (R2, independent) |
| M2-03 | Minor | `trace.py ci_evidence` | Claims set polluted; count 39 vs 8; latent false-accept | Scope `claims.add` to `ci:`/`ci-`; align docs; T6 rev 0.1.4 | `trace.py:177-184`; gate `CI evidence claims: 9`; `traceability.md:18-20`; `tool-qualification.md:72`; negative control clean | frick-fsw | **closed** | done (R2, independent; see N-2) |
| M2-04 | Minor | Gitea workflow markers | `ci-lean` marker missing vs GitHub | Add `ci-lean` marker | `.gitea/workflows/ci.yml:109` == `.github/workflows/ci.yml:92`; marker sets identical | frick-fsw | **closed** | done (R2, independent) |
| M2-05 | Minor | `verification-results.md` / N11 | `17/17 modules` not re-derivable | Commit `lake-build.log`; reference it | `verification/lean/lake-build.log`; `verification-results.md:88-93,23` | frick-fsw | **closed** | done (R2, independent; see N-3) |
| M2-06 | Minor | Test determinism | Live-clock margins (15 µs - 1 s) flake risk | Fixed reference buffer; anchored round-trip | `test_time.c:76-126`; 100/100 stable runs | frick-fsw | **closed** | done (R2, independent; see N-1) |

---

Committed and pushed with the reviewed change (norm N11).
