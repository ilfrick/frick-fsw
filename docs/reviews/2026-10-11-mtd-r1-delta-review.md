# Review Report — NOR/QSPI MTD backend (SRS-REQ-056), R1 delta re-review (N12)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0096                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (Senior SW dev / Flight Software)   |
| Reference revision                | commit `cb2576d` ("MTD increment: remove vacuous Renode check, commit reviews, correct QSPI claims"), branch `main`, on top of the reviewed revision `a3cdd68` |
| Items under review                | Delta (N12) of findings F1–F4 of FRICK-REVIEW-0091: `src/apps/app_renode_test.c` (the retired `hal_mtd` clause); `docs/requirements/traceability.yaml` + `.md`; `docs/design/software-design.md` §6; `docs/vv/verification-results.md`; `docs/vv/tool-qualification.md`; `docs/status/{handover.md,loop-audit.md}`; `README.md`; `CHANGELOG.md`; `docs/planning/sprint-plan.md`; `config/platform/platform_mpfs.h`; `src/drivers/mtd/mpfs/ff_mtd_mpfs.{c,h}`; `tests/unit/test_mtd.c`; the on-SoC QSPI tracking |
| Date                              | 2026-10-11                             |
| Scope                             | Whether `cb2576d` closes F1–F4 of FRICK-REVIEW-0091; whether the vacuous `hal_mtd` Renode clause is removed (no residual QSPI/NOR claim); whether SRS-REQ-056 evidence is host-only across all records; whether the manifest/SDD/records no longer overstate; whether the on-SoC QSPI gap is tracked as an M11 task; whether the driver/test is unchanged and correct; whether the board-repl statement is reflected. Gates re-run read-only. ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025). |
| Independence declaration          | Reviewer took no part in authoring, implementing, executing or reviewing the items; fresh independent session with no prior author context. Read-only for the repository: only this report file was created; no tracked file was modified (working tree clean of tracked changes before and after). Build/ctest/trace were run into the git-ignored `build/`; the board-repl check ran in the pinned Renode container. |
| Norms applied                     | N1–N20 (see `docs/rules.md`); focus N2, N4, N8, N9, N11, N12, N15, N18 |

## 1. Scope

This is the norm-N12 **R1 delta re-review** of FRICK-REVIEW-0091, which had
rated the NOR/QSPI MTD backend increment (SRS-REQ-056) at `a3cdd68`
**APPROVED WITH COMMENTS** with four Minor findings:
**F1** the `hal_mtd` Renode clause read `0x21000000`, unmapped in the loaded
CPU-only platform `@platforms/cpus/polarfire-soc.repl`, so it passed vacuously;
**F2** the manifest `lean_note` overstated the evidence as a write/read check;
**F3** the SDD design matrix omitted `mtd/mpfs` / SRS-REQ-056;
**F4** the unit suite missed the strict `off > size` and `size > FF_MTD_MAX_SIZE`
cases and the mock clamped silently.
The integrator applied fixes at **`cb2576d`** and requested this delta re-review.

**Included:** closure status of F1–F4; the completeness of the retirement of the
vacuous Renode clause (code, manifest, records, comments); the honesty of the
host-only scoping of SRS-REQ-056 evidence; the on-SoC QSPI M11 tracking; and
whether the driver/test source is unchanged and still correct.

**Excluded:** the PA (N20) delta review of FRICK-REVIEW-0094 and the sibling R2
(V&V, 0092) and R4 (documentation, 0093) reviews' findings; the internal
correctness of the driver/test is re-confirmed but not re-derived in full; the
Lean proofs; and on-target/silicon NOR behaviour (GAP-A5 by the increment's own
scope).

**Method.** Read `git show cb2576d` and every touched artifact; compared the
pre/post driver and test blobs by object hash; grepped the working tree for
residual `QSPI`/`MTD`/Renode-exercise claims; re-ran `trace.py`, `metrics.py
--check` and `ctest`; and inspected the board repls inside the pinned Renode
image (digest `sha256:7905c3…fb061`).

**Working-tree note.** An **untracked** sibling report
`docs/reviews/2026-10-11-mtd-pa-delta-review.md` (FRICK-REVIEW-0095, PA N20)
was present in the working tree when this review began; it is not committed and
was not modified. Its findings (DF1–DF4) overlap this review's and are
cross-referenced below. The present review uses the requested unique id 0096.

## 2. References

- Predecessor: `docs/reviews/2026-10-11-mtd-r1-review.md` (FRICK-REVIEW-0091,
  APPROVED WITH COMMENTS; F1–F4).
- Sibling reviews committed with the fix: `...-mtd-r2-review.md` (0092),
  `...-mtd-r4-review.md` (0093), and the PA reviews `...-mtd-pa-review.md`
  (0094, REJECTED Major) / `...-mtd-pa-delta-review.md` (0095, untracked).
- Project norms: `docs/rules.md` (N1–N20), especially N4, N8, N11, N12, N15, N18.
- Traceability manifest: `docs/requirements/traceability.yaml`; human mirror
  `docs/requirements/traceability.md`.
- HAL contract: `src/hal/hal_mem.h` (`FF_Mtd_t`).
- Renode: `tools/docker/renode/frick-renode.resc` loads the SoC-only CPU repl;
  `tools/docker/renode/Dockerfile` pins `antmicro/renode@sha256:7905c3…fb061`
  (Renode v1.16.1.16973).
- ECSS-E-ST-40C Rev.1 (30 Apr 2025); ECSS-Q-ST-80C Rev.2 (30 Apr 2025).

## 3. Verification performed (independent)

### 3.1 Product gates re-run at `cb2576d`

| Gate | Command | Result |
| ---- | ------- | ------ |
| Unit suite (N6) | `cmake --build build/native` + `ctest --test-dir build/native` | **18/18 PASS** (incl. `mtd` 18/18) |
| Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 98 requirements (57 srs / 27 sss / 14 gap), **51** Lean theorems |
| Format (N2/style) | `clang-format --dry-run --Werror` on `app_renode_test.c`, `ff_mtd_mpfs.c/.h`, `test_mtd.c` | **clean** (clang-format 23.1.3) |
| Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0** — "metrics panel: current" |
| Driver/test unchanged | `git rev-parse a3cdd68:<blob> cb2576d:<blob>` | `ff_mtd_mpfs.c` `a5e4cac…` **=** `a5e4cac…`; `test_mtd.c` `c94fb31…` **=** `c94fb31…` → **byte-identical** |
| Vacuous clause retired | `grep -n -i 'mtd\|qspi\|nor\|0x21000000' src/apps/app_renode_test.c` | only the generic file-header word "Renode" (line 5); **no** QSPI/NOR/`hal_mtd` clause |
| Board repl (pinned image) | `docker run antmicro/renode@sha256:7905c3…fb061 grep … platforms/boards/mpfs-icicle-kit.repl` | `boards/mpfs-icicle-kit.repl:37 qspi: SPI.MPFS_QSPI @ sysbus 0x21000000`; `:41 pseFlash: SPI.Micron_MT25Q @ qspi`; `cpus/polarfire-soc.repl` has **no** QSPI/`0x21000000` |
| Commit/remote state (N7) | `git branch -avv` | HEAD `cb2576d` **=** `origin/main` **=** `gitea/main`; no tracked modifications |

### 3.2 Delta closure matrix (F1–F4)

| Finding (0091) | Severity | Verification at `cb2576d` | Status |
| --- | --- | --- | --- |
| **F1** — vacuous `hal_mtd` read of unmapped `0x21000000` | Minor | `git show cb2576d -- src/apps/app_renode_test.c`: the `#include`, `mpfs_mtd_read8/write8`, `renode_hal_mtd_check`, the `hal_mtd_ok` flag, its log line, and its term in the `RENODE_TEST` marker are all **removed** (file: `app_renode_test.c:236–301`). No QSPI/NOR/`0x21000000` text remains. The pinned image confirms the SoC-only CPU repl maps no peripheral there and the board repls (`mpfs-icicle-kit.repl:37,41`; `beaglev-fire.repl:7,11`) carry `SPI.MPFS_QSPI @0x21000000` + `SPI.Micron_MT25Q`. | **CLOSED** (removed, not reworded) |
| **F2** — manifest `lean_note` overstates as write/read check | Minor | `traceability.yaml:530–532`: `tests: ["tests/unit/test_mtd.c"]` (`ci:renode` removed) and `lean_note` reworded to "…register logic host-tested (the on-SoC QSPI check needs the board repl - M11 follow-up)". **But** the human mirror `traceability.md:117` still lists "`test_mtd.c, ci:renode`" and its header Revision is still `0.25`. | **PARTIAL** — yaml closed; mirror residual (Finding 3, = 0095 DF1) |
| **F3** — SDD matrix omits `mtd/mpfs` / SRS-REQ-056 | Minor | `software-design.md` §6 row now `src/drivers/{…,nvm/mpfs,mtd/mpfs,net}` with `SRS-REQ-056`; Revision 0.3→0.4 with a History row; `README.md:54` design traceability back to **57/57 (100%)**. | **CLOSED** |
| **F4** — missing strict `off > size` / `size > FF_MTD_MAX_SIZE` cases; silent-clamping mock | Minor | `test_mtd.c` is byte-identical (`c94fb31…`). Bounds cases remain `read(size,1)` (`:67`, falls to the `len > size-off` disjunct) and `write(size-2,4)` (`:68`); **no** `size + 1` case for read/write/erase; **no** `FF_MtdMpfsSetup(…, FF_MTD_MAX_SIZE + 1, …)` case; the mock still clamps/drops out-of-range accesses (`:27–39`). | **NOT CLOSED** (carried; = 0095 DF4) |

### 3.3 Residual claim/flag scan (new probes)

`grep -rn -i "exercised on the emulated SoC\|exercised against the emulated SoC\|Renode functional test" src/drivers/mtd/ config/platform/platform_mpfs.h tests/unit/test_mtd.c` — the fix retired the Renode clause but **left the surrounding prose claiming the driver is exercised in Renode**:

| Probe | Result |
| ----- | ------ |
| `src/drivers/mtd/mpfs/ff_mtd_mpfs.h:6–8` | "…testable on the host and **exercised on the emulated SoC in the Renode functional test (milestone M11)**" → **stale/false** |
| `src/drivers/mtd/mpfs/ff_mtd_mpfs.h:49–51` | "**The Renode check is a read-presence check** (the XIP window is read-only in the model)." → references a **removed** check |
| `tests/unit/test_mtd.c:5–7` | "The same driver is **exercised against the emulated SoC MMIO in the Renode functional test (milestone M11)**." → **stale/false** |
| `config/platform/platform_mpfs.h:50–52` | "MSS QSPI NOR flash … **exercised on the emulated SoC in the Renode functional test (M11)**." → **stale/false** (this header is listed as SRS-REQ-056 `implementation` in the manifest) |
| `docs/vv/verification-results.md:410` | History 0.27 still: "…`test_mtd` (ctest 18/18), **`hal_mtd=PASS` in Renode**" (cycle row `:39` corrected to "proxy"/host-only) → **stale** (0095 DF2) |
| `docs/requirements/traceability.md:117` | still "`test_mtd.c, ci:renode`" → **stale** (0095 DF1) |
| `config/platform/platform_mpfs.h:53` | `FF_QSPI0_BASE` now only defined, never used → dead define (0095 DF3) |

Note: the equivalent "exercised on the emulated SoC" comments for **GPIO/SERIAL/NVM**
(`platform_mpfs.h:31,41,46`) remain **true** — those checks still run — so the
defect is specific to the MTD/QSPI text.

## 4. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| 1 | Minor | N8 / N4 — residual over-claim in MTD artifacts | The Renode clause was removed, but **three artifacts still assert the MTD driver is exercised on the emulated SoC in the Renode functional test**: the driver header (`ff_mtd_mpfs.h:6–8`), the unit-test header comment (`test_mtd.c:5–7`), and the platform header (`platform_mpfs.h:50–52`). `ff_mtd_mpfs.h:49–51` additionally describes a "Renode check" that no longer exists. The fix therefore did **not** fully retire the QSPI/NOR claim from the SRS-REQ-056 implementation set — the very class F1 targeted. The PA delta scan (0095) probed only for `hal_mtd`/`renode_hal_mtd`/`FF_QSPI0_BASE` tokens and so missed the prose. | `src/drivers/mtd/mpfs/ff_mtd_mpfs.h:6–8,49–51`; `tests/unit/test_mtd.c:5–7`; `config/platform/platform_mpfs.h:50–52`; `traceability.yaml:529` (header listed as implementation) | Reword the three comments to state the driver is **host-tested** and the **on-SoC check is pending (M11, board repl)**; delete the "Renode check is a read-presence check" sentence. Re-grep for the phrase, not just the token. |
| 2 | Minor | SRS-REQ-056 / test coverage (0091-F4, 0092-F1) | Carried, **not closed**: `test_mtd.c` is untouched, so neither the strict `off > size` disjunct (read/write/**erase**) nor the `size > FF_MTD_MAX_SIZE` setup bound is exercised, and the mock still silently clamps/drops out-of-range accesses (no out-of-range canary). The requirement attributes "bounded window" and a 2 MiB cap remain host-unverified at the boundary. | `tests/unit/test_mtd.c:27–39,55,67–68` (unchanged `c94fb31…`); `ff_mtd_mpfs.c:22,38,59,84` | Add `read/write/erase(size + 1, …) == FF_BAD_SIZE` and `FF_MtdMpfsSetup(…, FF_MTD_MAX_SIZE + 1, …) == FF_INVALID_ARGUMENT`; assert an out-of-range flag from the mock is false at the end. |
| 3 | Minor | N4 / N8 — traceability mirror divergence | The authoritative `traceability.yaml` removed `ci:renode` for SRS-REQ-056, but the human mirror `traceability.md:117` still lists it and the mirror is unchanged since `a3cdd68` (Revision still `0.25`). The document states it "mirrors its *current* content"; `trace.py` and the CI docs gate check convergence of the yaml only, so the divergence passes uncaught. This re-asserts the retired Renode evidence in a second record — the F2 over-claim class surviving in the mirror. | `docs/requirements/traceability.md:117` vs `traceability.yaml:530`; `trace.py` (checks yaml↔SRS/SSS/GAP only); `traceability.md:12` | Update `traceability.md:117` to `test_mtd.c` and bump its Revision; reconcile the mirror. |
| 4 | Comment | Records consistency (N11/N16/N8) | Additional stale records survive the fix and are cross-referenced to 0095: `verification-results.md:410` History 0.27 still claims "`hal_mtd=PASS` in Renode" (DF2); `handover.md:126` still lists "NOR MTD" among next classes and `:63` "(14/14)" and `:97` "5/13" remain (§4 stale); `README.md:137` still lists `QSPI/NOR` as **Planned** and `:408` focus sentence is stale; `renode-environment.md:88` still "Peripheral-bound (4/13)". | See each file:line | Correct each and delta-verify (N12). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### 4.1 Positive verification (no finding)

- **F1 correctly retired.** The vacuous clause was **deleted**, not reworded:
  the read at the unmapped `0x21000000` no longer exists, and the `RENODE_TEST`
  marker no longer includes `hal_mtd`. Directly confirmed in the pinned image
  that the loaded `cpus/polarfire-soc.repl` maps no peripheral at `0x21000000`,
  while `boards/mpfs-icicle-kit.repl:37,41` maps `SPI.MPFS_QSPI @ sysbus
  0x21000000` + `SPI.Micron_MT25Q @ qspi` — my FRICK-REVIEW-0091 statement is
  **confirmed and now reflected** in the commit message, `CHANGELOG.md`, the
  `loop-audit.md` disposition, `sprint-plan.md` S1, `README.md:186` and
  `verification-results.md:39`.
- **on-SoC QSPI gap tracked.** `loop-audit.md` dispositions row:
  "On-SoC QSPI NOR write/erase check (MTD) - needs the board repl
  `MPFS_QSPI`+`Micron_MT25Q` + flash command path | tracked | frick-fsw | **M11**";
  `sprint-plan.md:42` S1 exit criterion agrees. Confirmed.
- **Evidence honestly scoped as host-only (primary records).** `CHANGELOG.md`,
  `README.md:186`, `verification-results.md:39` (cycle row) and the
  `traceability.yaml` `lean_note` all state host-only + M11 pending. Confirmed.
- **Driver/test unchanged and correct.** Blobs byte-identical to `a3cdd68`
  (`ff_mtd_mpfs.c` `a5e4cac…`, `test_mtd.c` `c94fb31…`). Independent re-read:
  the guard `(off > size) || (len > (size - off))` (`ff_mtd_mpfs.c:22,38,59`)
  short-circuits before the subtraction (no `uint32_t` underflow); `off == size`
  with `len > 0` → `FF_BAD_SIZE`; `NULL`/`len == 0` → `FF_INVALID_ARGUMENT`;
  erase requires alignment and writes `0xFF`; setup rejects `NULL`, `size == 0`,
  `size > FF_MTD_MAX_SIZE`, `sector_size == 0`, non-multiple size. The four
  residual Comments of 0091 (F5–F8) remain advisory and carry no new evidence.
- **Gates.** ctest **18/18**, trace **98/57/27/14/51**, clang-format clean,
  metrics current, HEAD = both remotes.

## 5. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**FRICK-REVIEW-0091 status:**

- **R1-MTD-01 (F1, Renode vacuous clause) — CLOSED**: removed, and the
  board-repl statement is confirmed and reflected.
- **R1-MTD-02 (F2, manifest wording) — PARTIAL**: the authoritative
  `traceability.yaml` is corrected, but the `traceability.md` mirror still
  asserts `ci:renode` (Finding 3); the "evidence is host-only" claim is not yet
  true across all records.
- **R1-MTD-03 (F3, SDD) — CLOSED**.
- **R1-MTD-04 (F4, tests) — OPEN**: `test_mtd.c` untouched; boundary/erase
  bounds and the setup size cap remain unexercised (Finding 2).

Because F4 remains open and F2 is only partially closed (with the residual
over-claim of Finding 1 and the mirror of Finding 3), **FRICK-REVIEW-0091 is
NOT closed**; it remains APPROVED WITH COMMENTS with tracked Minors. No Major
finding exists: the material defect (F1, the vacuous on-SoC QSPI evidence) is
correctly and completely removed at the source, and all gates are green at
`cb2576d`. The verdict of this delta review is **APPROVED WITH COMMENTS**.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent agent, FRICK-REVIEW-0096 | 2026-10-11 | **APPROVED WITH COMMENTS** (F1/F3 closed; F2 partial, F4 open; 3 Minor + 1 Comment) |

## 7. Uncertainty and confidence

- **Facts:** the clause removal, the byte-identical driver/test blobs, the
  manifest/SDD edits, the board-repl contents in the pinned image, the gate
  results, and the residual stale comments/records are directly observed.
- **Assumptions:** the reviewed revision is `cb2576d` and the `build/native`
  tree inspected corresponds to it (rebuilt with "no work to do").
- **Inferences:** the `traceability.md` mirror is hand-maintained (no generator
  found; `trace.py` verifies only the yaml), so its divergence will persist
  until edited.
- **Recommendations:** close Findings 1–3 and the Comment 4 records, then
  delta-verify; the increment loop cannot close (N12) until F4 is corrected.
- **Confidence:** **High** on F1/F3 closure and on Findings 1–3 (direct,
  configuration-matched evidence); **High** on the board-repl confirmation
  (pinned-digest container); **Medium** that no other stale claim survives
  outside the greps used.
- **Open gaps:** MISRA/static scan not reproduced (cppcheck absent, as in 0091);
  target NOR timing/endurance/retention remains GAP-A5; an untracked sibling
  PA report (0095) exists in the working tree and is not part of the baseline.

## 8. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| 0091-F1 / R1-MTD-01 | Minor | Renode `hal_mtd` | Vacuous read at unmapped `0x21000000` | Closed: clause deleted; board repl confirmed | `app_renode_test.c` (no clause); pinned-image repl grep | frick-fsw | **closed** | 0096 (this) |
| 0091-F2 / R1-MTD-02 | Minor | `traceability.yaml` `lean_note` | Overstated as write/read check | yaml closed; mirror residual open | `traceability.yaml:530–532`; `traceability.md:117` | frick-fsw | **partial** | 0096 |
| 0091-F3 / R1-MTD-03 | Minor | SDD §6 | `mtd/mpfs` + SRS-REQ-056 missing | Closed: added; design 57/57 | `software-design.md` §6 + History; `README.md:54` | frick-fsw | **closed** | 0096 |
| 0091-F4 / R1-MTD-04 | Minor | `tests/unit/test_mtd.c` | `off > size` / `size > FF_MTD_MAX_SIZE` cases missing; clamping mock | Open: test untouched | `test_mtd.c` (sha `c94fb31…`); bounds cases `:55,67–68` | frick-fsw | **open** | 0096 |
| 0096-01 | Minor | `ff_mtd_mpfs.h:6–8,49–51`; `test_mtd.c:5–7`; `platform_mpfs.h:50–52` | Stale "exercised on the emulated SoC in the Renode functional test" claims; references a removed "Renode check" | Reword to host-tested + M11 pending | updated comments; grep for the phrase | frick-fsw | **open** | pending |
| 0096-02 | Minor | `docs/requirements/traceability.md:117` | Mirror still lists `ci:renode`; Revision 0.25 | Correct the mirror | updated `traceability.md`; yaml/md reconciled | frick-fsw | **open** | pending |
| 0096-03 | Comment | `verification-results.md:410`; `handover.md:63,97,126`; `README.md:137,408`; `renode-environment.md:88` | Residual stale records (History 0.27 `hal_mtd=PASS`; handover "(14/14)"/"5/13"/"NOR MTD next"; README scope table/focus; renode-env "(4/13)") | Correct each (overlaps 0095 DF1/DF2/DF4) | updated records | frick-fsw | **open** | pending |

---

Prepared as the R1 delta re-review (FRICK-REVIEW-0096); committed and pushed
with the reviewed change per norm N11.