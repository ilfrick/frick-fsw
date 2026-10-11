# Review Report — Reference NOR/QSPI MTD backend (SRS-REQ-056), R1 code review

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0091                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (Senior SW dev / Flight Software) |
| Reference revision                | commit `a3cdd68` ("Sprint 1.2: reference NOR/QSPI MTD backend (SRS-REQ-056)"), branch `main` |
| Items under review                | `src/drivers/mtd/mpfs/ff_mtd_mpfs.{c,h}`; `tests/unit/test_mtd.c`; CMake registration (`CMakeLists.txt`, `tests/CMakeLists.txt`); the `renode_hal_mtd_check` clause in `src/apps/app_renode_test.c`; `FF_QSPI0_BASE` in `config/platform/platform_mpfs.h`; requirement SRS-REQ-056 / SSS-REQ-020 and the associated traceability records (`docs/requirements/traceability.yaml`, `docs/requirements/traceability.md`, `README.md`, `CHANGELOG.md`, `docs/vv/verification-results.md` 0.27) |
| Date                              | 2026-10-11                             |
| Scope                             | Correctness and bounds of the injected-bus NOR MTD backend (off/len overflow, sector alignment, size bound, NULL, zero len), NOR erase semantics (0xFF), MISRA/coding style (clang-format), test meaningfulness (non-tautological), CMake registration, and the honesty of the Renode `hal_mtd` read-presence clause. ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025). Gates re-run read-only. |
| Independence declaration          | Reviewer took no part in authoring, implementing, executing or reviewing the items; fresh independent session with no prior author context. Read-only for the repository: only this report file was created; no tracked file was modified (working tree clean before and after). Build/ctest/trace were run into the git-ignored `build/`; the Renode probe ran in an ephemeral container on the pinned image. |
| Norms applied                     | N1–N20 (see `docs/rules.md`); focus N2, N4, N8, N9, N11, N15, N18 |

## 1. Scope

Reviewed the milestone M4/M11 increment that adds the reference MSS QSPI NOR
flash MTD backend behind the HAL `FF_Mtd_t` contract (SRS-REQ-056, child of
SSS-REQ-020) and a `hal_mtd` clause to the Renode functional test, as
delivered at commit `a3cdd68`. The review covered: (a) static correctness and
bounds analysis of `ff_mtd_mpfs.{c,h}`; (b) code style / MISRA posture; (c)
meaningfulness of `tests/unit/test_mtd.c`; (d) build/test registration; and (e)
the evidentiary honesty of the Renode clause. Requirement/design mirror
documents were inspected as adjacent records. Hardware NOR timing/endurance/
retention (GAP-A5) and the HAL registry wiring of the `FF_Mtd_t` instance are
**out of scope** (the latter is a caller responsibility).

## 2. References

- Project norms: `docs/rules.md` (N1-N20)
- Traceability manifest: `docs/requirements/traceability.yaml` (SRS-REQ-056, SSS-REQ-020)
- HAL contract: `src/hal/hal_mem.h` (`FF_Mtd_t`, lines 42-52)
- Renode pinned image: `antmicro/renode@sha256:7905c3…fb061`, Renode 1.16.1
  (verified by `docker run … renode --console`)
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| 1 | Minor | SRS-REQ-056 / N18 / N11 (Renode `hal_mtd`) | The Renode check reads `0x21000000`, but the platform loaded by the functional test (`@platforms/cpus/polarfire-soc.repl`, pinned Renode 1.16.1) has **no peripheral mapped at `0x21000000`**. The read is served by the sysbus "non existing peripheral" path (warning + `0x00`), so the clause passes vacuously and proves nothing about QSPI/NOR presence. The code comment "the memory-mapped XIP window is **read-only** in the platform model" is therefore inaccurate (the window is *absent*, not read-only). | `src/apps/app_renode_test.c:249-264` (comment + `renode_hal_mtd_check`); `tools/docker/renode/frick-renode.resc:14` loads the CPU repl; `tools/docker/renode/Dockerfile:10` pins the image; empirical probe: Renode 1.16.1 `sysbus ReadByte 0x21000000` → `[WARNING] sysbus: ReadByte from non existing peripheral at 0x21000000.` `0x00`; the CPU repl contains no `qspi`/`0x2100…` line, whereas `platforms/boards/beaglev-fire.repl:7` and `platforms/boards/mpfs-icicle-kit.repl:37` map `qspi: SPI.MPFS_QSPI @ sysbus 0x21000000` | Either load a board repl that maps QSPI (`beaglev-fire.repl`, `mpfs-icicle-kit.repl`) and assert a QSPI-specific observable, or explicitly state that `0x21000000` is not modeled, downgrade the claim to "an in-range read does not fault the driver", and record it honestly in the code comment and the enumerated records. Note the board-model `0x21000000` is the **SPI controller** (size `0x1000`), not a true XIP flash window; a real XIP window would need its own read-only mapping. |
| 2 | Minor | N4 / N8 (traceability `lean_note`) | `lean_note` for SRS-REQ-056 claims "write/read check on the emulated SoC (hal_mtd)". The clause performs **no write** and does **not verify any read value** — it only checks the driver returns `FF_SUCCESS`. The note overstates the evidence and contradicts the honest wording used in the commit message / CHANGELOG ("XIP read presence"). | `docs/requirements/traceability.yaml:530`; contrast `CHANGELOG.md` (MTD entry) and `src/apps/app_renode_test.c:263` (`return drv.mtd.read(...) == FF_SUCCESS;`) | Reword the `lean_note` to "in-range XIP read presence; the QSPI XIP window is not modeled on the emulated SoC (see FRICK-REVIEW-0091)", and keep it consistent with the code comment. |
| 3 | Minor | N8 (design mirror) | The SDD design-allocation row for the reference drivers was not updated: the row lists `src/drivers/{eth/gem,gpio/mpfs,serial/mpfs,mem,nvm/mpfs,net}` with SRS-REQ-036/038/040/047/048/049/055 and **omits `mtd/mpfs` and SRS-REQ-056**. `git show a3cdd68` does not touch `docs/design/software-design.md`. | `docs/design/software-design.md:113`; commit stat | Add `mtd/mpfs` to the path set and SRS-REQ-056 to the row's requirement list; update the SDD revision history. |
| 4 | Minor | SRS-REQ-056 / test coverage | The unit suite does not exercise (a) the strict out-of-range path `off > size` for read/write/erase, nor (b) the setup bound `size > FF_MTD_MAX_SIZE`; and the mock **clamps/silently drops** out-of-range accesses, so a hypothetical bounds regression would not be caught by the mock (no canary/poison). | `tests/unit/test_mtd.c:27-39` (clamping mock), `:67-71` (only `off == size`), `:55` (only `4097` vs sector) | Add cases for `off = size + 1` (read/write/erase) and `FF_MtdMpfsSetup(..., FF_MTD_MAX_SIZE + 1, …)`; have the mock set an out-of-range flag asserted false at the end. |
| 5 | Comment | Test clarity | Comment "The erased sector leaves the second sector untouched." is not demonstrated by the following lines: they write then read offset 4200 (sector 1), which proves write/read, not that sector 0's erase left sector 1 intact. | `tests/unit/test_mtd.c:81-84` | Either assert sector-1 content is unchanged across the sector-0 erase (e.g. prime it before the erase) or reword the comment. |
| 6 | Comment | Robustness | `FF_MtdMpfsSetup` accepts a `bus` with NULL `read8`/`write8` (returns `FF_SUCCESS`), after which `FF_Mtd_t.read/write` would dereference a NULL function pointer. A second `FF_MtdMpfsSetup` silently rebinds the single global `g_mtd` (documented single-instance limitation). | `src/drivers/mtd/mpfs/ff_mtd_mpfs.c:84-96`, `:13`, `:26`, `:42` | Add `(bus->read8 == NULL) || (bus->write8 == NULL)` to the setup guard; optionally record the rebinding behaviour as an explicit precondition. |
| 7 | Comment | Self-test semantics | `FF_MtdMpfsSelfTest` writes offset 0 (mutating NOR) and returns `FF_ERROR` on write-protected/read-only media; it is compiled into the production driver (the header documents "host unit test (not part of the HAL contract)"). It is correctly **not** called by the Renode clause. | `src/drivers/mtd/mpfs/ff_mtd_mpfs.c:68-79`; `src/drivers/mtd/mpfs/ff_mtd_mpfs.h:49-52` | Ensure it is never invoked on target (or gate it out of target builds); the header warning is adequate but worth enforcing by construction. |
| 8 | Comment | MISRA evidence | The MISRA scan (`tools/scripts/misra.sh`, cppcheck) could not be run in this environment (cppcheck absent), so MISRA/static evidence was not independently reproduced. The file is clang-format clean and visually consistent with the codebase style. | `tools/scripts/misra.sh:20-21` | No action required for this increment; note as an advisory confidence limitation. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### 3.1 Positive verification (no finding)

- **Bounds / overflow (clean).** The guard `(off > size) || (len > (size - off))`
  (`ff_mtd_mpfs.c:22,38,59`) short-circuits before the subtraction, so no
  `uint32_t` underflow occurs; `off == size` with `len > 0` returns
  `FF_BAD_SIZE`; `NULL`/zero-`len` returns `FF_INVALID_ARGUMENT`. With
  `size ≤ FF_MTD_MAX_SIZE = 2 MiB`, `off + i` cannot wrap. Confirmed.
- **Erase semantics (clean).** Erase requires `off % sector == 0`,
  `len % sector == 0`, `len > 0`, and in-bounds; it writes `0xFF` byte-wide and
  the unit test verifies all 4096 bytes of the erased sector. Confirmed
  (`ff_mtd_mpfs.c:47-66`, `test_mtd.c:73-80`).
- **Setup validation (clean).** Rejects `m == NULL`, `bus == NULL`, `size == 0`,
  `size > FF_MTD_MAX_SIZE`, `sector_size == 0`, and `size % sector_size != 0`
  (`ff_mtd_mpfs.c:84-87`).
- **Tests are meaningful, not tautological.** Round-trip through an independent
  mock byte array, byte-level `0xFF` verification, alignment rejection, bounds
  rejection, whole-device erase, and self-test; `test_mtd` reports
  `checks=4123 failures=0` (`tests/unit/test_mtd.c:41-91`).
- **Registration correct.** `CMakeLists.txt:130` adds the source to
  `frick_fsw_core` under `FF_BUILD_DRIVERS`; `tests/CMakeLists.txt:45-46` builds
  `test_mtd` and `:81` registers ctest `mtd`.
- **Gates (re-run at `a3cdd68`).** `cmake --build build/native` clean;
  `ctest --test-dir build/native` → **18/18 passed** (incl. `mtd`);
  `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` →
  `98 requirements, 57 srs, 27 sss, 14 gaps, 51 lean theorems` PASS;
  `clang-format --dry-run --Werror` on the four changed files → clean.

### 3.2 Confidence and remaining gaps

- **Facts:** the bounds/erase/setup logic, the test suite, the CMake
  registration, the gate results, and the Renode platform contents (verified
  empirically in the pinned image) are directly observed.
- **Assumptions:** the Renode `RENODE_TEST: PASS` / `hal_mtd=PASS` result was
  reported in the commit and verification-results but could not be re-executed
  here (the target ELF was not built for this review); the finding on F1 rests
  on the platform file contents, which are deterministic and version-pinned.
- **Inferences:** because the address is unmapped, the clause would pass for any
  in-range address, so it carries no QSPI-specific evidence.
- **Recommendations:** F1–F4 require correction before the increment's Renode
  evidence can be described as validating the QSPI/NOR path; F5–F8 are
  discretionary.
- **Confidence:** **High** for the code-correctness and gate findings; **High**
  for F1 (pinned image inspected directly); **Medium** that a future Renode
  image upgrade will not silently change the unmapped-read behaviour.
- **Open gaps:** MISRA scan not reproduced (cppcheck absent); target NOR
  timing/endurance/retention remains GAP-A5.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

The backend is functionally correct, well bounded, statically safe against
`uint32_t` overflow, style-clean, and covered by meaningful (non-tautological)
host tests; all required gates pass (18/18, trace 98/57/27/14/51). No Major
finding. The increment is **APPROVED WITH COMMENTS**: the four Minor findings
(F1–F4) must be tracked to closure per norm N12, with **F1** (the Renode
`hal_mtd` evidence is vacuous and its surrounding justification is inaccurate)
being the most material.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent agent, FRICK-REVIEW-0091 | 2026-10-11 | **APPROVED WITH COMMENTS** (4 Minor, 4 Comment; no Major) |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | ------ | --------------- |
| R1-MTD-01 | Minor | SRS-REQ-056 / N18 (Renode) | `0x21000000` unmapped in the loaded CPU repl; read served by the "non existing peripheral" path → vacuous `hal_mtd=PASS`; "read-only XIP window" comment inaccurate | open | — | frick-fsw | open | pending |
| R1-MTD-02 | Minor | N4/N8 | `traceability.yaml:530` `lean_note` overstates the Renode evidence as a write/read check | open | — | frick-fsw | open | pending |
| R1-MTD-03 | Minor | N8 | SDD reference-drivers row omits `mtd/mpfs` / SRS-REQ-056 | open | — | frick-fsw | open | pending |
| R1-MTD-04 | Minor | SRS-REQ-056 (test) | No `off > size` or `size > FF_MTD_MAX_SIZE` cases; clamping mock can mask bounds regressions | open | — | frick-fsw | open | pending |
| R1-MTD-05..08 | Comment | clarity / robustness / MISRA | see F5–F8 | advisory | — | frick-fsw | open | — |

---

Committed and pushed with the reviewed change (norm N11).