# Review Report — SRS-REQ-055 reference eNVM NVM backend

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0082                      |
| Reviewer session                  | (independent agent, norm N9)           |
| Reviewer profile                  | R2 (SW V&V)                            |
| Reference revision                | HEAD `0eb009d`                         |
| Items under review                | `tests/unit/test_nvm.c`; `src/drivers/nvm/mpfs/ff_nvm_mpfs.{c,h}`; `config/platform/platform_mpfs.h`; `docs/requirements/traceability.yaml` (SRS-REQ-055); `docs/requirements/traceability.md`; `docs/vv/verification-results.md` (row/0.26); `docs/vv/tool-qualification.md` (T3/T17); `src/apps/app_renode_test.c` (`hal_nvm`); `tools/scripts/renode_vv.sh`; `tests/CMakeLists.txt`; `CMakeLists.txt`; `.github/workflows/ci.yml` |
| Date                              | 2026-10-11                             |
| Scope                             | Independent V&V of SRS-REQ-055: unit-test adequacy (register/layout logic, bounds, error paths), Renode `hal_nvm` scope honesty, coverage/evidence-claim accuracy, and traceability-manifest convergence. Against ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2, ECSS-E-ST-10-02C Rev.1. |
| Independence declaration          | Fresh context; the reviewer authored/executed none of the items under review and read only the repository at `0eb009d`. |
| Norms applied                     | N1-N20 (see `docs/rules.md`); emphasis N4, N6, N9, N10, N11, N18 |

## 1. Scope

Reviewed the complete reference eNVM NVM backend introduced at HEAD `0eb009d`:
the driver (`src/drivers/nvm/mpfs/ff_nvm_mpfs.c/.h`) against the `FF_Nvm_t`
HAL contract (`src/hal/hal_mem.h:54-62`), its host unit test
(`tests/unit/test_nvm.c`), the Renode `hal_nvm` presence check
(`src/apps/app_renode_test.c:209-236, 289`), the CI/Renode wrapper
(`tools/scripts/renode_vv.sh`), and the four evidence records that carry the
claim: `docs/requirements/traceability.yaml` (SRS-REQ-055),
`docs/requirements/traceability.md:115`, `docs/vv/verification-results.md`
(lines 38 and 394), and `docs/vv/tool-qualification.md` (lines 27, 41, 107).

Excluded: real-silicon behaviour, eNVM flash endurance/retention (documented
GAP-A5), the other M11 peripheral classes, and the Lean proofs (no Lean theorem
is linked to SRS-REQ-055; `lean_note` is provided, which satisfies norm N1/N3
for this entry).

Commands executed independently on 2026-10-11:

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` →
  `traceability: PASS (97 requirements, 56 srs, 27 sss, 14 gaps, 51 lean
  theorems checked)` — matches the claimed `97/56/27/14/51`.
- `ctest --test-dir build/native` → `100% tests passed, 0 tests failed out of
  17`.
- `make coverage` → `line coverage: 2898/4061 (71.4%)`. `gcov` on the new
  driver was inspected directly.

## 2. References

- Project norms: `docs/rules.md` (N1-N20)
- Traceability manifest: `docs/requirements/traceability.yaml` (SRS-REQ-055)
- ECSS-E-ST-40C Rev.1 (software verification), ECSS-Q-ST-80C Rev.2 (software
  product assurance, tool qualification), ECSS-E-ST-10-02C Rev.1 (verification)
- HAL contract: `src/hal/hal_mem.h`

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| 1 | Minor | Evidence accuracy (SRS-REQ-055; norm N11) | The executed-check count for the new suite is misstated as **14** in every record; the harness actually executes **29** assertions (and prints `checks=29`). This is a factual evidence record, not a stylistic value. | `tests/unit/test_nvm.c:46-86` (29 `FF_TEST_ASSERT`); binary output `.../test_nvm.c checks=29 failures=0`; `docs/vv/verification-results.md:38` ("test_nvm 14 checks"); commit `0eb009d` message ("test_nvm.c (14 checks)") | Correct the row and commit/CHANGELOG text to `29 checks`. |
| 2 | Minor | Self-test (norm N4/N8) | `FF_NvmMpfsSelfTest` is documented as the *"MMIO presence/round-trip self-test used by the Renode functional test"*, but the Renode `hal_nvm` check never calls it (it only does `setup`+`read`). Separately, its in-code comment *"last slot (not part of the value space)"* is **false**: `FF_NVM_MAX_IDS = 32`, so ids `0..31` are all valid and the self-test overwrites payload slot 31 (writing length byte `1` and payload `0xA5`). | `src/drivers/nvm/mpfs/ff_nvm_mpfs.h:45-47`; `src/drivers/nvm/mpfs/ff_nvm_mpfs.c:84-88`; `src/apps/app_renode_test.c:225-236`; `ff_nvm_mpfs.h:31-33` | Fix the comment/claim and either reserve slot 31 explicitly or use an out-of-range scratch region; do not wire this self-test to Renode/real eNVM until the destructive write to a valid slot is removed (eNVM writes are non-volatile and wear-limited). |
| 3 | Minor | Coverage of the new unit (norm N6) | Two code paths on the new driver are unexecuted: `nvm_init` is dead (defined, never referenced in the vtable, `0% called`) and the defensive "corrupt length byte" clamp (`ff_nvm_mpfs.c:54-55`) is never taken. No test injects a length byte `> FF_NVM_MAX_VALUE`, and the successful write at the maximum legal length (`len == FF_NVM_MAX_VALUE`) is not exercised. | `gcov` (`build/coverage/gcov/...ff_nvm_mpfs.c.gcov`): `function nvm_init called 0 returned 0%`; `#####: 55` (clamp never executed). `tests/unit/test_nvm.c` asserts only the over-limit reject (`:71`), not the max-legal accept. | Remove `nvm_init` or wire it; add a test that stores a corrupt length byte through the mock and asserts the clamp/read behaviour; add the `len == 31` boundary case. |
| 4 | Minor | T17 register consistency (norm N10) | The T17 register row still lists the HAL checks as `hal_gpio`/`hal_serial`/`hal_mem`/`hal_eth`/`hal_net` — `hal_nvm` is missing — while the revision-history entry `0.3.9` states T17 "gains `hal_nvm`". The register is self-inconsistent. | `docs/vv/tool-qualification.md:41` vs `docs/vv/tool-qualification.md:107` | Add `hal_nvm` to the T17 row (and the evidence-claims mapping row at `:44` if it enumerates HAL checks). |
| 5 | Comment | CI provenance (norm N6/N4) | The `ci-renode` evidence marker does not list SRS-REQ-055 in its parenthetical requirement set (it lists 036/039/044/047/048/049/050 + SSS-013/020). The gate (`trace.py`) only checks that the *claim name* exists, so the manifest's `ci:renode` for SRS-REQ-055 converges structurally but the per-requirement CI provenance is incomplete. | `.github/workflows/ci.yml:80`; `.gitea/workflows/ci.yml:83`; `tools/scripts/trace.py:160-184, 287-294` | Add `SRS-REQ-055` to the `ci-renode` marker parentheticals in both workflows. |
| 6 | Comment | Renode `hal_nvm` scope | The Renode check reads the first length byte and returns `FF_SUCCESS` whenever that byte is `<= 4` (a zero-filled or aliased read both pass); unlike the GEM MODID check there is no distinguishing identifier. The scope is *honestly documented* as "presence/identification" (`app_renode_test.c:221-224`) and the record says "MMIO presence check" (`verification-results.md:38`), so this is not a misclaim — but the check cannot distinguish "eNVM mapped" from "address reads zeros". | `src/apps/app_renode_test.c:225-236`; `docs/vv/verification-results.md:38`; `docs/vv/handover.md:148-149` | Consider asserting a distinguishable value or a bounded scratch write/read-back (documented endurance = GAP-A5) to make presence a positive result; otherwise keep the "presence only, not a write/endurance test" wording explicit wherever the class is counted (M11 "5/13 peripheral-bound"). |
| 7 | Comment | Host mock robustness | The mock `read8`/`write8` index with `off % sizeof(m->mem)`, which silently aliases any out-of-range offset instead of failing. For the current driver all offsets stay `<= 1023 < 1024` (`ff_nvm_mpfs.c:34-37`, `:85-87`), so no bug is masked today, but the modulo weakens the mock as an independent oracle. | `tests/unit/test_nvm.c:27, 33` | Replace the modulo with a hard bounds tripwire (e.g. assert `off < sizeof(m->mem)`), so a future offset bug cannot alias into a false PASS. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### Assessment against the review questions

- **Is the test evidence adequate for the requirement (register logic, bounds,
  error paths)?** Substantially yes for host-level logic: the suite covers the
  write/read round trip, zero-length values, erase, id bound (`FF_NVM_MAX_IDS`),
  length bound (`> FF_NVM_MAX_VALUE`), NULL-argument rejection on all entry
  points, the short-capacity `FF_BAD_SIZE` path, and the exact-capacity accept.
  It is *not* complete: the corrupt-length clamp, the max-legal-length success,
  and (by design) any eNVM-mediated write are unverified (findings 3, 6).
- **Is the Renode presence check honestly scoped?** Yes. The in-code comment and
  the verification-results row both state it is a presence/readability check,
  and explicitly defer write/erase register logic to the host test and
  endurance to GAP-A5. It is not presented as a write/endurance test.
- **Are the coverage/evidence claims accurate?** No, one is not: the "14 checks"
  figure is wrong (29). The trace counts (`97/56/27/14/51`), `ctest 17/17`, and
  "ctest 16→17" are all accurate; the "M11 5/13" and "Renode MMIO presence
  check" statements are accurate as scoped.
- **Does the manifest converge?** Yes. `trace.py` is green at `97/56/27/14/51`;
  SRS-REQ-055 lists existing implementation paths and existing test/CI evidence,
  and its `lean_note` satisfies N1/N3. The only weakness is the CI-provenance
  parenthetical (finding 5), which the gate does not currently enforce.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

No Major findings. Four Minor findings (evidence-count accuracy, self-test
doc/in-code inconsistency and destructive slot write, new-driver coverage gaps,
T17 register inconsistency) must be corrected/justified; three Comments are
advisory. The functionally observable behaviour required by SRS-REQ-055
(bounded ids/slots, `FF_Nvm_t` conformance, argument/error handling) is
demonstrated by the host suite and the Renode presence check within the honestly
stated evidence envelope.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R2 (N9)       | DeepSeek V4.1 Flash (independent SW V&V agent) | 2026-10-11 | APPROVED WITH COMMENTS |

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-01 | Minor | SRS-REQ-055 evidence | `test_nvm` recorded as 14 checks; actual 29 | Correct record to 29 | `test_nvm` prints `checks=29`; `verification-results.md:38` | frick-fsw | open | pending |
| NC-02 | Minor | `ff_nvm_mpfs.c/.h` self-test | Doc says used by Renode (it is not); comment "not part of value space" false; overwrites valid slot 31 | Fix comment/claim; reserve or relocate scratch slot; do not wire to Renode until non-destructive | re-review of `ff_nvm_mpfs.h:45-47`, `ff_nvm_mpfs.c:84-88` | frick-fsw | open | pending |
| NC-03 | Minor | new-driver coverage | `nvm_init` dead (0%); corrupt-length clamp unexecuted; max-legal-length success untested | Remove/wire `nvm_init`; add clamp + boundary tests | updated `gcov` + new asserts | frick-fsw | open | pending |
| NC-04 | Minor | `tool-qualification.md` T17 | T17 row omits `hal_nvm` though revision 0.3.9 adds it | Add `hal_nvm` to T17 row | `tool-qualification.md:41` vs `:107` | frick-fsw | open | pending |

---

Committed and pushed with the reviewed change (norm N11).