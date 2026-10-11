# Review Report - M4/M11 reference eNVM NVM backend (`FF_Nvm_t`) + Renode `hal_nvm` check

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0081                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (SW dev)                            |
| Reference revision                | commit `0eb009d` ("Sprint 1.1: reference eNVM NVM backend (SRS-REQ-055) + Renode hal_nvm check"), `main`, working tree clean |
| Items under review                | `src/drivers/nvm/mpfs/ff_nvm_mpfs.{c,h}`; `tests/unit/test_nvm.c`; `CMakeLists.txt` (driver source registration); `tests/CMakeLists.txt` (test + ctest registration); `src/apps/app_renode_test.c` (`renode_hal_nvm_check`); `config/platform/platform_mpfs.h` (`FF_ENVM0_BASE`); requirement SRS-REQ-055 |
| Date                              | 2026-10-11                             |
| Scope                             | (a) correctness and bounds of the eNVM key/value backend: length byte, slot cap, id range, `NULL` handling, over-long value rejection, corrupt-length clamp, MMIO self-test; (b) MISRA/style conformance (clang-format gate; static warnings; dead code); (c) meaningfulness (non-tautology) of `test_nvm`; (d) honesty of the Renode presence check (no over-claim of write/endurance); (e) build / ctest / trace gates. ECSS-E-ST-40C, ECSS-Q-ST-80C, SRS-REQ-055 / SSS-REQ-020. |
| Independence declaration          | Reviewer took no part in authoring, implementing or executing the reviewed items; fresh independent session. Only the untracked review report was created; no tracked repository file was modified. Read-only probes were compiled in `/tmp/opencode` against the reviewed sources. |
| Norms applied                     | N1-N20 (see `docs/rules.md`); R1 focus N1, N3, N4, N9, N15 |

## 1. Scope

Reviewed the M4/M11 increment adding the reference eNVM non-volatile-memory
backend behind the HAL contract `FF_Nvm_t` (key/value by id) and the associated
host unit test, CMake registration, Renode functional-test clause and platform
base address. The review is bounded to the items listed above; documentation
files touched by the same commit (SRS.md, CHANGELOG.md, handover.md,
verification-results.md, traceability.*) are cited only where a finding is about
the accuracy of the change's own evidence, and were not reviewed for N17/N20.

Assumptions: the reviewed revision is `0eb009d`; the driver is a *reference*
backend (host-tested register logic; real eNVM endurance/erase semantics are
out of scope and recorded as GAP-A5).

## 2. References

- Project norms: `docs/rules.md` (N1-N20)
- Traceability manifest: `docs/requirements/traceability.yaml` (SRS-REQ-055)
- HAL contract: `src/hal/hal_mem.h` (`FF_Nvm_t`, SRS-REQ-032)
- Sibling reference drivers (same single-instance/injected-bus pattern):
  `src/drivers/serial/mpfs/ff_serial_mpfs.c`, `src/drivers/mem/ff_mem_static.c`
- ECSS-E-ST-40C (software), ECSS-Q-ST-80C (software product assurance)
- Predecessor reviews: FRICK-REVIEW-0043 (M4/M11 SERIAL), FRICK-REVIEW-0044
  (M4/M11 MEM)

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| R1-01 | Minor | `src/drivers/nvm/mpfs/ff_nvm_mpfs.c:84-88`; `ff_nvm_mpfs.h:45-47`; SRS-REQ-055 | **`FF_NvmMpfsSelfTest` uses slot `FF_NVM_MAX_IDS - 1` (= 31) as its scratch area and the comment calls it "not part of the value space", but id 31 is a valid id** (ids are `0 .. FF_NVM_MAX_IDS-1 = 0..31`). The self-test writes the length byte and one payload byte of slot 31, destroying any stored parameter with `id == 31`. Confirmed empirically: after writing 31 payload bytes to id 31, `FF_NvmMpfsSelfTest` leaves the slot reading `len=1, byte0=0xA5` (value lost). No runtime caller exists (only `test_nvm.c`), so there is no flight defect today, but the comment is factually wrong and the helper is unsafe if ever wired into a diagnostic/health path. | `ff_nvm_mpfs.c:84-88`; `ff_nvm_mpfs.h:31-33,45-47`; independent probe output (self-test destroyed id 31) | Reserve a dedicated out-of-range scratch location (e.g. `nvm_off(FF_NVM_MAX_IDS)` in the not-yet-used region up to the eNVM page) or document explicitly that the self-test clobbers id 31 and must never be called while parameter 31 is live. |
| R1-02 | Minor | `src/drivers/nvm/mpfs/ff_nvm_mpfs.c:20-23`; `CMakeLists.txt:201-203`; `CMakePresets.json` | **Dead code / build not warning-clean.** `nvm_init()` is a `static` function that is never referenced (the HAL `FF_Nvm_t` contract has no `init` member). `gcc -Wall` reports `warning: 'nvm_init' defined but not used [-Wunused-function]`. The default `native` preset builds without `-Wall`/`-Wextra` on `frick_fsw_core` and has `FF_ENABLE_WERROR=OFF`, so the default and `native-release` gates cannot catch it; the defect is silent. | `ff_nvm_mpfs.c:20-23`; `grep -rn nvm_init` (single hit = definition); `gcc -Wall` probe; `CMakeLists.txt:201-203`; `CMakePresets.json:22` | Remove `nvm_init` (or wire it into the contract if intended) and add `-Wall -Wextra` to `frick_fsw_core` so this class of defect fails the build automatically. |
| R1-03 | Minor | `tests/unit/test_nvm.c`; `src/drivers/nvm/mpfs/ff_nvm_mpfs.c:54-56`; SRS-REQ-055 | **The corrupt-length clamp is not exercised by the unit test.** The scope explicitly requires verifying the clamp; the logic is correct (probe: a length byte of `200` with `cap=64` returns `FF_SUCCESS, len=31`; with `cap=10` returns `FF_BAD_SIZE`), but no test writes a length byte `> FF_NVM_MAX_VALUE`, so a regression in `ff_nvm_mpfs.c:54-56` would pass CI undetected. | `ff_nvm_mpfs.c:53-56`; `test_nvm.c` (no corrupt-length case); independent probe | Add a case that seeds the mock length byte to e.g. `0xFF`/`200` and asserts `len == FF_NVM_MAX_VALUE` (and `FF_BAD_SIZE` for a smaller `cap`). |
| R1-04 | Minor | `tests/unit/test_nvm.c:24-34` | **The mock bus masks the offset (`off % sizeof(m->mem)`), so the oracle cannot detect an out-of-bounds access.** A bounds regression in `nvm_off`/the loop would alias onto a valid location and the test would still pass. The current driver performs no OOB access (verified: a probe recording `off >= sizeof(mem)` observes zero OOB over all valid operations), so this is a test-strength defect, not a driver defect. | `test_nvm.c:27,33`; independent probe (`oob=0` for all valid ops) | Assert `off < sizeof(m->mem)` in the mock (failing the test on OOB) instead of wrapping, or bound-check explicitly. |
| R1-05 | Minor | `CHANGELOG.md:12-16`; `docs/vv/verification-results.md:38`; commit `0eb009d` message | **The recorded test count is wrong.** The change claims "`test_nvm` … (14 checks)"; the harness (`tests/ff_test.h`) counts executed `FF_TEST_ASSERT` macros and the binary prints `checks=29 failures=0` (29 `FF_TEST_ASSERT` in `test_nvm.c`). The evidence record is therefore inaccurate (norms N11/N16). | `./build/native/tests/test_nvm` -> `checks=29 failures=0`; `grep -c FF_TEST_ASSERT tests/unit/test_nvm.c` = 29; `CHANGELOG.md:15`; `verification-results.md:38`; `git show --stat 0eb009d` | Correct the count to 29 in the CHANGELOG and verification-results row (and avoid a hard-coded count in the commit text). |
| R1-06 | Comment | `src/apps/app_renode_test.c:209-236`; SRS-REQ-055 | **The Renode `hal_nvm` check is honestly scoped as a presence check** (comment states it is MMIO presence/identification; write/erase register logic is host-tested and endurance is GAP-A5) - no over-claim of write or endurance, which is correct. Two weaknesses: it reads value-space slot 0 (rather than a fixed identification/scratch location) and returns `FF_SUCCESS` only if the emulator's stored length byte is `<= cap` (`buf[4]`); an erased (0xFF) emulated eNVM would clamp to 31 and yield `FF_BAD_SIZE`, i.e. the presence result depends on the emulator initialising the region to zero. | `app_renode_test.c:221-236`; `ff_nvm_mpfs.c:53-58` | Probe a fixed scratch/identification offset (or accept any `FF_Status_t` as "bus responded") and note in `renode-environment.md` that the check relies on zero-initialised emulated eNVM. |
| R1-07 | Comment | `src/drivers/nvm/mpfs/ff_nvm_mpfs.c:25-41,67-75`; SRS-REQ-055 | **Write/erase have no atomicity or real-flash semantics.** The length byte is programmed before the payload (an interrupted write leaves a length that claims unwritten payload); `erase` only writes the length byte to 0 and never restores the payload region to the erased (0xFF) state; eNVM byte programming can only clear bits (1->0), so rewriting a slot without an erase is not modeled. This is acceptable for a *reference* backend, but it is only implied by "GAP-A5", not stated at the interface. | `ff_nvm_mpfs.c:35-38,72`; `ff_nvm_mpfs.h:10-12` | Add a header note that the backend assumes host/RAM-like storage and that power-fail atomicity, erase-before-program and endurance are integrator obligations (GAP-A5). |
| R1-08 | Comment | `src/drivers/nvm/mpfs/ff_nvm_mpfs.h:39`; `ff_nvm_mpfs.c:39,73,100` | `present[]` is maintained by the driver but is not used by any driver code path (only read by the unit test). It is dead state unless intended as a health/telemetry source. | `ff_nvm_mpfs.h:39`; `ff_nvm_mpfs.c:39,73,100`; only read at `test_nvm.c:53,65` | Either declare it explicitly as debug/health state in the header, or drop it and have the test derive presence from a successful read. |
| R1-09 | Comment | `src/drivers/nvm/mpfs/ff_nvm_mpfs.c:30,49`; `hal_mem.h:54-62` | Interface asymmetry: `nvm_write` accepts `buf == NULL` when `len == 0`, but `nvm_read` rejects `buf == NULL` unconditionally (even to query the length of a zero-length value); there is no "length-only" query. Acceptable, but undocumented. | `ff_nvm_mpfs.c:30,49` | Document the `NULL`-buffer contract for read/write in the header. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### Empirical evidence

Executed by this reviewer (host, Linux; independent of the author):

```
cmake --build build/native .......................... clean (forced rebuild of ff_nvm_mpfs.c; no warning under default flags)
ctest --test-dir build/native ....................... 100% tests passed, 0 failed out of 17
build/native/tests/test_nvm ......................... checks=29 failures=0
python3 tools/scripts/trace.py docs/requirements/traceability.yaml
  ................................................... PASS (97 requirements, 56 srs, 27 sss, 14 gaps, 51 lean theorems checked)
clang-format --dry-run --Werror (frick-vv:local, find src config tests) ... EXIT 0 (clean)
gcc -Wall -Wextra ff_nvm_mpfs.c ..................... warning: 'nvm_init' defined but not used [-Wunused-function]
independent bounds probe (/tmp/opencode/nvm_probe) ... corrupt len 200 cap 64 -> SUCCESS len 31; cap 10 -> BAD_SIZE;
                                                       SelfTest destroys id 31; no OOB offset over valid ops
```

Bounds reviewed by construction: `nvm_off(id) = id*32`, `id <= 31`, max write
offset `31*32 + 1 + 30 = 1023 < 32*32 = 1024`; the length field is written as
`(uint8_t)len` with `len <= 31` so it round-trips exactly; read is guarded by
`id < FF_NVM_MAX_IDS`, `buf != NULL`, `len != NULL`, the clamp `n <= 31` and the
`cap < n` size check. No unbounded access or arithmetic overflow was found.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

No Major finding. The backend is correct and bounded, the host test is
meaningful (real round trips, boundaries, negative cases - not tautological),
clang-format is clean, and `cmake --build` / `ctest` (17/17) / `trace.py`
(97/56/27/14/51) all reproduce. The Renode `hal_nvm` check is honestly scoped as
a presence check and does not over-claim write/endurance. The five Minor findings
(R1-01 dead-space self-test, R1-02 dead code, R1-03 untested clamp, R1-04 masked
oracle, R1-05 wrong evidence count) must be corrected or justified before closure
(norm N12).

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | aeronautics-space-fsw-engineer (independent session) | 2026-10-11 | APPROVED WITH COMMENTS |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-0081-01 | Minor | `ff_nvm_mpfs.c:84-88`; SRS-REQ-055 | Self-test clobbers valid slot id 31; comment claims "not part of the value space" | open | - | - | open | pending |
| NC-0081-02 | Minor | `ff_nvm_mpfs.c:20-23` | `nvm_init` dead code; not caught by build warnings | open | - | - | open | pending |
| NC-0081-03 | Minor | `test_nvm.c`; `ff_nvm_mpfs.c:54-56` | Corrupt-length clamp untested | open | - | - | open | pending |
| NC-0081-04 | Minor | `test_nvm.c:27,33` | Mock bus masks offset; oracle cannot detect OOB | open | - | - | open | pending |
| NC-0081-05 | Minor | `CHANGELOG.md:15`; `verification-results.md:38` | Recorded "14 checks" vs actual 29 | open | - | - | open | pending |

---

Committed and pushed with the reviewed change (norm N11).