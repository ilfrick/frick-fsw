# Delta Re-Review Report - M4/M11 reference eNVM NVM backend: disposition of `FRICK-REVIEW-0081` (R1-01..R1-05)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0086                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (SW dev)                            |
| Reference revision                | commit `03962d6` ("NVM backend: close R1/R2/R4/PA review findings (N12)"), branch `main`, working tree clean |
| Items under review                | Delta re-review of the five Minor findings `R1-01`..`R1-05` of `FRICK-REVIEW-0081`: `src/drivers/nvm/mpfs/ff_nvm_mpfs.{c,h}`; `tests/unit/test_nvm.c`; records `CHANGELOG.md`, `docs/vv/verification-results.md`; requirement SRS-REQ-055 |
| Date                              | 2026-10-11                             |
| Scope                             | Whether each `FRICK-REVIEW-0081` Minor is closed at `03962d6` (norm N12), and whether the fixes kept the eNVM backend and its host test correct and gate-clean. ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2, SRS-REQ-055. |
| Independence declaration          | Reviewer took no part in authoring, implementing or executing the reviewed items; fresh independent session. Only this untracked report file was created; no tracked repository file was modified. Read-only probes were compiled in `/tmp/opencode` against the reviewed sources. |
| Norms applied                     | N1-N20 (see `docs/rules.md`); R1 focus N1, N3, N4, N9, N11, N12, N15 |

## 1. Scope

This is the norm-N12 delta re-review that closes the loop on the predecessor R1
review `FRICK-REVIEW-0081` (`docs/reviews/2026-10-11-nvm-r1-review.md`, reviewed
revision `0eb009d`). The fixes are contained in commit `03962d6`, which also
folds the R2/R4/PA findings of the same increment. This report evaluates **only**
the disposition of the five R1 Minors and the regression status of the item they
touch; the R2/R4/PA deltas are owned by their respective reviewers and are not
re-decided here.

Assumptions: the reviewed revision is `03962d6`; the driver remains a *reference*
backend (host-tested register logic; real eNVM endurance/erase semantics remain
GAP-A5).

## 2. References

- Project norms: `docs/rules.md` (N1-N20)
- Predecessor review: `FRICK-REVIEW-0081` (`docs/reviews/2026-10-11-nvm-r1-review.md`)
- Sibling reviews of the same increment: `FRICK-REVIEW-0082` (R2),
  `FRICK-REVIEW-0083` (R4), `FRICK-REVIEW-0084` (PA)
- Traceability manifest: `docs/requirements/traceability.yaml` (SRS-REQ-055)
- HAL contract: `src/hal/hal_mem.h` (`FF_Nvm_t`, SRS-REQ-032)
- ECSS-E-ST-40C Rev.1 (software, 30 Apr 2025); ECSS-Q-ST-80C Rev.2 (software
  product assurance, 30 Apr 2025)

## 3. Findings

| # | Severity (Major/Minor/Comment) | Prior NC / Item | Disposition at `03962d6` | Evidence | Recommendation |
| - | ------------------------------ | --------------- | ------------------------ | -------- | -------------- |
| D-R1-01 | Closed | R1-01 / SRS-REQ-055 | **Closed.** The self-test no longer touches a valid id. `FF_NVM_SCRATCH_OFF` (= `FF_NVM_MAX_IDS * FF_NVM_SLOT_SIZE` = 1024) is defined in the header (`ff_nvm_mpfs.h:34-35`) and used by `FF_NvmMpfsSelfTest` (`ff_nvm_mpfs.c:79-83`); the false "not part of the value space" comment is gone, replaced by "outside the parameter slot table (id space untouched)". Independent probe: after `write(31, 31 bytes)` then `FF_NvmMpfsSelfTest`, id 31 reads back intact (`preserved=1`, `memcmp` match); the self-test's maximum touched offset is 1025, `oob=0`. | `ff_nvm_mpfs.h:34-35`; `ff_nvm_mpfs.c:79-83`; `/tmp/opencode/nvm_probe` (`preserved=1`, `max_off_seen=1025`, `oob=0`) | None. See D-02 (test-fidelity consequence of moving the scratch out of the mock). |
| D-R1-02 | Closed (defect); Recommendation residual | R1-02 / `ff_nvm_mpfs.c` | **Closed for the defect.** `nvm_init` is removed: `grep -rn nvm_init src tests` returns no hit; the translation unit no longer contains any unreferenced `static`. Both the driver and the test compile clean under `gcc -Wall -Wextra -Wpedantic -std=c99` (exit 0, no diagnostics). The secondary recommendation (add `-Wall -Wextra` to `frick_fsw_core`) is **not** implemented - see D-03. | `grep nvm_init` = no hit; `gcc -Wall -Wextra -Wpedantic` on `ff_nvm_mpfs.c` and `test_nvm.c` = exit 0; `CMakeLists.txt:201-203` (only conditional `-Werror`) | Implement D-03 (warning flags) so this class cannot silently recur. |
| D-R1-03 | Closed | R1-03 / SRS-REQ-055 | **Closed.** The previously unexecuted corrupt-length clamp is now exercised: `test_nvm.c:98-102` seeds the id 1 length byte to `200`, asserts `FF_BAD_SIZE` for `cap = 8` and `FF_SUCCESS, len == FF_NVM_MAX_VALUE` for `cap = 31`. The max-legal-length success is added at `test_nvm.c:84-96` (write/read 31 bytes, `memcmp`). The suite prints `checks=36 failures=0`. Independent probe reproduces both clamp branches. | `test_nvm.c:84-102`; `./build/native/tests/test_nvm` -> `checks=36 failures=0`; `grep -c FF_TEST_ASSERT` = 36; `/tmp/opencode/nvm_probe` (`len=200 cap=64 -> SUCCESS len=31`; `cap=10 -> FF_BAD_SIZE`) | None. |
| D-R1-04 | **Open** (restated, Minor) | R1-04 / `test_nvm.c` | **Not addressed.** The host mock still masks the offset (`m->mem[off % sizeof(m->mem)]`, `test_nvm.c:27,33`); no hard bounds tripwire and no recorded justification were added. The finding is now **more** material: the R1-01 fix places the self-test at offsets 1024/1025 (`FF_NVM_SCRATCH_OFF` + 1), i.e. outside the 1024-byte mock (`MockNvm_t.mem[FF_NVM_MAX_IDS * FF_NVM_SLOT_SIZE] = 1024`). A strict-bounds variant of the current mock records **3** accesses at/above the mock size during `FF_NvmMpfsSelfTest`, all silently aliased to slot 0 - exactly the masking the predecessor flagged. As worded, the original recommendation (assert `off < sizeof(m->mem)`) cannot be applied verbatim: it would now fail the self-test unless the mock is enlarged to cover the scratch region. | `test_nvm.c:18,24-34`; `/tmp/opencode/nvm_scratch_probe` -> `sizeof(mock)=1024 scratch_off=1024 selftest=0 strict_oob_touches=3` | Close one of two ways: (a) enlarge the mock to the whole region actually exercised (`FF_NVM_SCRATCH_OFF + 2`) and replace the modulo with an `off < sizeof` tripwire (restores the oracle); or (b) record an explicit, signed justification reclassifying the item as an advisory Comment (consistent with `FRICK-REVIEW-0082` Comment 7). No justification exists at HEAD, so the Minor stands. |
| D-R1-05 | Closed | R1-05 / norm N11/N16 | **Closed.** The records now state the measured count: `CHANGELOG.md:15` = "`test_nvm` (mock MMIO, 36 checks)"; `docs/vv/verification-results.md:38` = "`test_nvm` 36 checks"; the commit subject body records "test_nvm 14 -> 36 checks". This matches the measured `checks=36`. | `CHANGELOG.md:15`; `docs/vv/verification-results.md:38`; `git show 03962d6`; `./build/native/tests/test_nvm` -> `checks=36` | None. |
| D-02 | Comment | New (consequence of R1-01) | The unit-test mock does not model the region the driver's self-test now uses: `MockNvm_t` is exactly the slot table (1024 B) while `FF_NVM_SCRATCH_OFF = 1024`, so the self-test's two writes and one read are aliased to `mem[0..1]` by the modulo and never validated against a faithful backing store. The self-test still returns `FF_SUCCESS` on the host, but the host evidence for the scratch round trip is an artefact of the masking, not of an out-of-table store. (The driver behaviour on silicon is unaffected: 1024 lies well inside the eNVM aperture `FF_ENVM0_BASE`.) | `test_nvm.c:18,105`; `/tmp/opencode/nvm_scratch_probe` (`strict_oob_touches=3`) | Enlarge the mock to `FF_NVM_SCRATCH_OFF + 2` bytes and assert `off < sizeof(m->mem)`; then the self-test is genuinely exercised out-of-table on the host. |
| D-03 | Comment | New (R1-02 recommendation) | The preventive part of R1-02 is not implemented: `frick_fsw_core` (which owns `ff_nvm_mpfs.c`) still carries no `-Wall`/`-Wextra`; `CMakeLists.txt:201-203` only appends `-Werror` when `FF_ENABLE_WERROR=ON` (off in the default `native` preset and only reachable in `native-release`). The CI "static analysis" job's `-Wall -Wextra` text is a fallback `echo` (`ci.yml:106`), not an actual build; static evidence rides on `clang-tidy`. So an unused-`static`/unused-variable regression in the core would still pass the default and `native-release` gates silently. | `CMakeLists.txt:172,201-203`; `CMakePresets.json` (`native` has no `FF_ENABLE_WERROR`); `.github/workflows/ci.yml:106` | Add `-Wall -Wextra` to `frick_fsw_core` (and, if project policy allows, `-Werror` in the CI build), as originally recommended by R1-02. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### Independent verification executed at `03962d6`

```
cmake --build build/native ................. clean (forced rebuild of ff_nvm_mpfs.c + test_nvm.c; no warning)
ctest --test-dir build/native .............. 100% tests passed, 0 failed out of 17
build/native/tests/test_nvm ................ checks=36 failures=0
python3 tools/scripts/trace.py docs/requirements/traceability.yaml
  ......................................... PASS (97 requirements, 56 srs, 27 sss, 14 gaps, 51 lean theorems checked)
clang-format (frick-vv:local, v18.1.3) --dry-run --Werror over "find src config tests" ... EXIT 0 (clean)
gcc -Wall -Wextra -Wpedantic -std=c99 (driver + test) ... EXIT 0 (no diagnostics)
grep -rn nvm_init src tests ............... no hit
/tmp/opencode/nvm_probe ................... write31/selftest/read31 -> id 31 preserved; scratch_off=1024;
                                            max_off_seen=1025; oob=0; clamp 200 cap64 -> SUCCESS len31,
                                            cap10 -> FF_BAD_SIZE; max-len 31 round-trips; len=32 rejected
/tmp/opencode/nvm_scratch_probe ........... sizeof(mock)=1024, strict_oob_touches=3 (self-test aliased by modulo)
```

Bounds reviewed by construction (unchanged logic): `nvm_off(id) = id*32`,
`id <= 31`, maximum slot offset `31*32 + 1 + 30 = 1023 < 1024`; the self-test
scratch `[1024, 1025]` is disjoint from the slot table; the corrupt-length byte is
clamped to `FF_NVM_MAX_VALUE` before the `cap < n` check. No unbounded access,
arithmetic overflow, or regression of the round trip was found.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

No Major finding. The R1-01/R1-02/R1-03/R1-05 fixes are correct and verified with
independent evidence, the backend is still correct and bounded, and all gates
reproduce at `03962d6` (`ctest` 17/17, `trace` 97/56/27/14/51, `clang-format`
clean in the pinned V&V image, `-Wall -Wextra -Wpedantic` clean on the touched
translation units, `test_nvm` `checks=36`).

**`FRICK-REVIEW-0081` is NOT fully closed.** Four of the five Minors are closed
(`R1-01`, `R1-02` defect, `R1-03`, `R1-05`); **`R1-04` remains open** - the mock
still masks out-of-range offsets and, after the R1-01 fix, silently aliases the
self-test's out-of-table accesses (D-02). Under norm N12 the item must be either
corrected (D-02/D-R1-04 option (a)) or explicitly justified/reclassified
(option (b)) before `FRICK-REVIEW-0081` can be declared closed. Two non-blocking
Comments (`D-02`, `D-03`) are also recorded. `FRICK-REVIEW-0081` is therefore
closeable **only** once `R1-04` is dispositioned.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | aeronautics-space-fsw-engineer (independent session) | 2026-10-11 | APPROVED WITH COMMENTS (R1-04 open; `FRICK-REVIEW-0081` not fully closed) |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-0081-01 | Minor | `ff_nvm_mpfs.c`/`.h`; SRS-REQ-055 | Self-test clobbers valid id 31 | Use an out-of-table scratch offset | `ff_nvm_mpfs.h:34-35`; `ff_nvm_mpfs.c:79-83`; probe: id 31 preserved, `oob=0` | frick-fsw | **closed** | this report (D-R1-01) |
| NC-0081-02 | Minor | `ff_nvm_mpfs.c:20-23` | `nvm_init` dead code | Remove it (done) | `grep nvm_init` no hit; `-Wall -Wextra` clean | frick-fsw | **closed** | this report (D-R1-02) |
| NC-0081-03 | Minor | `test_nvm.c`; clamp | Corrupt-length clamp untested; max-legal length untested | Add clamp + boundary cases | `test_nvm.c:84-102`; `checks=36`; probe clamp branches | frick-fsw | **closed** | this report (D-R1-03) |
| NC-0081-04 | Minor | `test_nvm.c:27,33` | Mock masks offset; oracle cannot detect OOB | Add bounds tripwire and cover the scratch region, or justify | mock unchanged (`off % sizeof`); `strict_oob_touches=3` for the self-test | frick-fsw | **open** | this report (D-R1-04 / D-02) |
| NC-0081-05 | Minor | `CHANGELOG.md:15`; `verification-results.md:38` | Recorded 14 checks vs actual 36 | Correct the count | records now read "36 checks"; measured `checks=36` | frick-fsw | **closed** | this report (D-R1-05) |
| NC-0086-01 | Comment | `test_nvm.c:18,105`; N6 | Mock does not cover the self-test scratch region | Enlarge mock + tripwire | `nvm_scratch_probe` | frick-fsw | open (advisory) | this report (D-02) |
| NC-0086-02 | Comment | `CMakeLists.txt:201-203`; N6 | `-Wall -Wextra` not applied to `frick_fsw_core` | Add warning flags | `CMakeLists.txt`; `ci.yml:106` | frick-fsw | open (advisory) | this report (D-03) |

---

Committed and pushed with the reviewed change (norm N11).