# Review Report — SRS-REQ-056 reference NOR/QSPI MTD backend

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0092                      |
| Reviewer session                  | (independent agent, norm N9)           |
| Reviewer profile                  | R2 (SW V&V)                            |
| Reference revision                | HEAD `a3cdd68`                         |
| Items under review                | `tests/unit/test_mtd.c`; `src/drivers/mtd/mpfs/ff_mtd_mpfs.{c,h}`; `src/apps/app_renode_test.c` (`hal_mtd`); `tools/scripts/renode_vv.sh`; `tools/docker/renode/frick-renode.resc`; `docs/requirements/traceability.yaml` (SRS-REQ-056); `docs/requirements/traceability.md`; `docs/requirements/SRS.md` (SRS-REQ-056); `docs/vv/verification-results.md` (row/0.27); `docs/vv/tool-qualification.md` (T3/T17); `docs/vv/renode-environment.md`; `config/platform/platform_mpfs.h`; `tests/CMakeLists.txt`; `CHANGELOG.md` |
| Date                              | 2026-10-11                             |
| Scope                             | Independent V&V of SRS-REQ-056: unit-test adequacy (read/write/erase, bounds, alignment, error paths, coverage of the driver), honesty of the Renode `hal_mtd` check scope, accuracy of the recorded evidence, and traceability-manifest convergence. Against ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2, ECSS-E-ST-10-02C Rev.1. |
| Independence declaration          | Fresh context; the reviewer authored, implemented, executed and previously reviewed none of the items under review. Read-only for the repository: only this report file was created; nothing else was modified. Gates were executed read-only (ctest outputs under the git-ignored `build/`). |
| Norms applied                     | N1-N20 (see `docs/rules.md`); emphasis N1, N4, N6, N8, N9, N10, N11, N18 |

## 1. Scope

Reviewed the new reference NOR/QSPI MTD backend (SRS-REQ-056) introduced by
HEAD `a3cdd68`: the HAL driver `ff_mtd_mpfs.{c,h}`, the host unit test
`tests/unit/test_mtd.c`, the Renode functional check `hal_mtd` in
`src/apps/app_renode_test.c`, the manifest entry
(`traceability.yaml` SRS-REQ-056 implementation + tests + `ci:renode`), the
records (`verification-results.md` row and 0.27 history, `tool-qualification.md`
T3/T17) and the mirror `traceability.md`.

Verification performed (read-only):

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` →
  `PASS (98 requirements, 57 srs, 27 sss, 14 gaps, 51 lean theorems checked)`.
- `ctest --test-dir build/native` → `100% tests passed, 0 tests failed out of 18`
  (`mtd` test 18/18 PASS).
- `./build/native/tests/test_mtd` → `checks=4123 failures=0`.
- Per-file gcov inspection of the driver
  (`build/coverage/gcov/…ff_mtd_mpfs.c.gcov`, from the committed coverage build).

Excluded: re-execution of the Renode functional run (requires Docker + the RTEMS
toolchain image; the reviewer assessed the check's scope and the record's claim
against the code, not by re-running it). No hardware/silicon evidence exists
(GAP-A5), which is declared.

Assumption: the `build/native` and `build/coverage` trees inspected are those of
the checked-out HEAD `a3cdd68`.

## 2. References

- Project norms: `docs/rules.md` (N1-N20)
- Traceability manifest: `docs/requirements/traceability.yaml`
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-10-02C Rev.1, CCSDS 133.0-B-2

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| 1 | Minor | Erase bounds (SRS-REQ-056 "bounded window") | The erase out-of-window `FF_BAD_SIZE` path is **never exercised**. `mtd_erase`'s bounds block is dead in every run (`#####:60` in the gcov), and neither the `off > size` nor the `len > size-off` sub-condition is ever true for erase. The test only covers alignment errors and in-bounds erases, so the "bounded window" attribute of the requirement is verified for read/write but not for erase. | `src/drivers/mtd/mpfs/ff_mtd_mpfs.c:59-60`; `tests/unit/test_mtd.c:73-85`; gcov `…ff_mtd_mpfs.c.gcov` line 60 `#####` and line 59 branches `taken 0` | Add negative cases: `erase(0, size + sector_size)` and `erase(size, sector_size)` must return `FF_BAD_SIZE`; re-run coverage. |
| 2 | Minor | Renode evidence claim (N4/N8) | The manifest `lean_note` states a **"write/read check on the emulated SoC (hal_mtd)"**, but the Renode check is a **read-only XIP presence check**: `renode_hal_mtd_check` only calls `drv.mtd.read(...)` and the injected `write8` is a no-op there (the model's XIP window is read-only). The driver header, the CHANGELOG and the verification record all say "read" / "read-only"; the manifest is the odd one out, overstating the evidence. | `docs/requirements/traceability.yaml:532` vs `src/apps/app_renode_test.c:249-263`; `src/drivers/mtd/mpfs/ff_mtd_mpfs.h:49-51`; `CHANGELOG.md:15`; `docs/vv/verification-results.md:39` | Reword the manifest `lean_note` to "XIP read-presence check on the emulated SoC (hal_mtd)" so it converges with the implementation and the record (mirror `traceability.md` if the wording propagates). |
| 3 | Comment | Bounds — `off > size` guard | The `off > g_mtd->mtd.size` sub-condition in `mtd_read`/`mtd_write` is never true in any test (gcov branch `taken 0` at lines 22 and 38): the test exercises only `off == size` (which falls to the `len > size-off` disjunct). The strictly-greater guard is therefore unverified. | `src/drivers/mtd/mpfs/ff_mtd_mpfs.c:22,38`; `tests/unit/test_mtd.c:67-68`; gcov branches `taken 0` | Add `read(size + 1, out, 1)` and `write(size + 1, val, 1)` negative cases so both bounds disjuncts are covered. |
| 4 | Comment | Write negative path | `write` with `len == 0` is never tested (gcov line 35 branch `taken 0`); only the read `len == 0` case is present. The symmetric invalid-argument case is missing. | `src/drivers/mtd/mpfs/ff_mtd_mpfs.c:35`; `tests/unit/test_mtd.c:71` | Add `write(0, val, 0) == FF_INVALID_ARGUMENT`. |
| 5 | Comment | Test intent vs assertion | The comment "The erased sector leaves the second sector untouched" is not substantiated by the assertions: sector 1 is first written *after* `erase(0, 4096)`, so an erroneous over-erase of sector 1 would still leave the 4200-byte round trip passing. | `tests/unit/test_mtd.c:81-84` | Write a sentinel at 4200 *before* the sector-0 erase and re-read it after, or reword the comment to match what is checked. |
| 6 | Comment | Evidence count honesty | The "4123 checks" figure (`verification-results.md:39`, `CHANGELOG.md:14`) is dominated by the 4096-iteration `for` loop that verifies the erased sector (`test_mtd.c:78-80`); the number of distinct behavioural assertions is ~27. A raw check count can overstate coverage. | `tests/unit/test_mtd.c:78-80`; `docs/vv/verification-results.md:39` | Keep the figure but note it includes the 4096-byte erase sweep (or report distinct cases), as done for other suites. |
| 7 | Comment | Renode environment doc (N8) | `renode-environment.md` §3 lists the HAL checks `hal_gpio`/`hal_serial`/`hal_mem`/`hal_eth`/`hal_net` but **not** `hal_nvm` or `hal_mtd`; it also does not state the MTD check's read-presence scope. This overlaps the already-open PA-89-02 for `hal_nvm`. | `docs/vv/renode-environment.md:38-46`; `docs/reviews/2026-10-11-nvm-pa-final-review.md:126` (PA-89-02) | Add `hal_nvm`/`hal_mtd` to the marker list and one line stating the MTD check is an XIP read-presence check (emulator-dependent). |
| 8 | Comment | `ci:renode` cadence (N18) | The manifest `tests` field lists `ci:renode` for SRS-REQ-056 without noting that the per-commit run is a **SKIP proxy** when the RTEMS/Renode ELF is absent (`renode_vv.sh:30-33`, exit 0); the real functional `hal_mtd=PASS` occurs in the weekly `nightly.yml` job. The record row honestly marks it REAL, but the manifest does not scope the cadence. | `tools/scripts/renode_vv.sh:30-33,56-58`; `.github/workflows/nightly.yml`; `docs/requirements/traceability.yaml:530` | Document that `ci:renode` evidence for `hal_mtd` is per-commit-proxy / weekly-real, as already stated for the Renode gate generally. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must be
  corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### 3.1 Confirmations (no finding)

- **Requirement text / allocation.** `SRS.md:285-289` states "byte-addressed
  read/write, sector-aligned erase … bounded window", matching `FF_Mtd_t`
  (`src/hal/hal_mem.h:44-52`) and the driver's vtable completion
  (`ff_mtd_mpfs.c:81-97`). `parent_sss: SSS-REQ-020` is correct
  (`traceability.yaml:527`; `traceability.md:48`).
- **Coverage of the stated behaviours.** Read/write round trip
  (`test_mtd.c:61-64`), NULL/zero-length invalid arguments (69-71), alignment
  errors (74-76), erase sets `0xFF` (77-80), whole-device erase (85) and the
  MMIO self-test (87-90) are present. The mock byte array (27-39) is an
  independent oracle (no reuse of driver logic).
- **Honest NOR scoping.** The header (`ff_mtd_mpfs.h:10-14`) records that the
  reference backend performs plain byte writes and leaves erase-before-write to
  the caller, and that real timing/endurance/retention is GAP-A5; the test does
  not over-claim 1→0 NOR semantics. The self-test is correctly **not** invoked
  from the Renode read-only path.
- **Renode check scope in code.** `app_renode_test.c:249-253` documents the
  read-only XIP window; the combined `RENODE_TEST: PASS` marker includes
  `hal_mtd` (330-333).
- **Records.** `tool-qualification.md:27` (T3 gains `test_mtd`) and `:41`
  (T17 includes `hal_mtd`), history `:109`; `verification-results.md:39` (row)
  and `:406` (0.27 history) are present and consistent with the artefacts.
- **Manifest convergence.** `trace.py` is green (98/57/27/14/51); the
  `implementation`/`tests` file lists for SRS-REQ-056 resolve.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

SRS-REQ-056 is implemented correctly for the host-verifiable attributes, the
unit test is meaningful and its Renode scope is honestly documented in the code,
CHANGELOG and record. Two Minor findings (F1 erase-bounds coverage; F2 manifest
`lean_note` evidence overstatement) must be corrected or justified; the remaining
items are advisory Comments. Verdict: **APPROVED WITH COMMENTS**.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R2 (N9)       | Senior SW V&V reviewer (independent) | 2026-10-11 | APPROVED WITH COMMENTS |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| F1 | Minor | `ff_mtd_mpfs.c:59-60`; SRS-REQ-056 erase bounds | Erase `FF_BAD_SIZE` path untested (gcov `#####`) | Correct/justify | pending | frick-fsw | open | pending |
| F2 | Minor | `traceability.yaml:532`; SRS-REQ-056 Renode scope | `lean_note` says "write/read check" vs read-only XIP presence check | Correct | pending | frick-fsw | open | pending |
| F3-F8 | Comment | bounds/write-0/intent/count/doc/cadence | advisory | Correct/justify as convenient | pending | frick-fsw | open | pending |

---

Committed and pushed with the reviewed change (norm N11).