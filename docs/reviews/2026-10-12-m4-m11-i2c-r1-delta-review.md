# Independent Review Report — R1 delta re-review (norm N12)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | **FRICK-REVIEW-0134**                  |
| Reviewer session                  | independent agent (fresh context, norm N9) |
| Reviewer profile                  | **R1** (Senior FSW / real-time / space software developer) |
| Reference revision                | `db8261f` + **uncommitted** Sprint-3 working tree (fixed revision) |
| Items under review                | `src/drivers/i2c/mpfs/ff_i2c_mpfs.{c,h}` (new); `tests/unit/test_i2c.c` (new); I2C check in `src/apps/app_renode_test.c`; `config/platform/platform_mpfs.h`; records `docs/requirements/SRS.md` (SRS-REQ-058), `docs/requirements/traceability.{yaml,md}`, `docs/vv/{verification-results.md,renode-environment.md,tool-qualification.md}`, `docs/quality/class-a-gaps.md` (GAP-A5), `CHANGELOG.md`, `Makefile` |
| Date                              | 2026-10-12                             |
| Scope                             | Delta re-review (norm N12) of the Sprint-3 MSS I2C increment after the initial R1 review `FRICK-REVIEW-0131`; verify closure of R1-01…R1-03 and R1-C1…R1-C9, look for regressions and new findings, against ECSS-E-ST-40C (SW development/V&V), ECSS-Q-ST-80C (SW PA), the norms N1–N20 (`docs/rules.md`), and the HAL contract `FF_I2c_t` (`src/hal/hal_spi.h`). |
| Independence declaration          | The reviewer took no part in authoring, implementing, building-for-delivery, or previously reviewing the items; this review ran in a fresh context and every declared claim was re-executed/read first-hand. |
| Norms applied                     | N1–N20 (`docs/rules.md` 1.4)           |

## 1. Scope

**In scope (verified first-hand on 2026-10-12):**

- The fixed driver `src/drivers/i2c/mpfs/ff_i2c_mpfs.c` / `.h` — specifically the
  changes made in response to `FRICK-REVIEW-0131`: the documented minimum
  achievable SCL, the fire-and-forget STOP limitation, the 7-bit-only limitation,
  the non-re-entrancy limitation, the self-test side-effect limitation, the
  NULL-bus-hook rejection, and the comment corrections.
- The extended unit test `tests/unit/test_i2c.c` (mock bus + register-based slave
  model; 12 test functions, now **144 checks**): the added 1 MHz divider-code-5
  case, the 104166/104167 Hz minimum-SCL boundary cases, and the NULL-bus-hook
  rejection case; the qualified mock/oracle disclaimer comment.
- The I2C functional check in `src/apps/app_renode_test.c`
  (`renode_hal_i2c_check`, `hal_i2c=PASS`) and its inclusion in the
  `RENODE_TEST` verdict.
- The record deltas: SRS-REQ-058 text, `traceability.yaml`/`traceability.md`,
  `verification-results.md`, `renode-environment.md`, `tool-qualification.md`
  (T3/T14/T17), `class-a-gaps.md` (GAP-A5), `CHANGELOG.md`, and the new
  `Makefile` `misra-docker` target.
- Independent re-execution of the local gate set (see the Evidence ledger).

**Out of scope / excluded (bounded):**

- On-target silicon behaviour (real APB clock, real SCL accuracy/eye, real-slave
  timing, clock stretching, bus recovery, arbitration) — tracked to GAP-A5 and
  explicitly not verifiable here.
- The R2/R3/R4/PA reviews of the same increment and their (non-R1) findings,
  except where a fix also bears on an R1 finding.
- Unrelated doc-only deltas present in the same working tree
  (`docs/architecture/network-stack.md`, `docs/engineering/*`, etc.).

**Assumption recorded:** the assigned document id `FRICK-REVIEW-0134` was
supplied by the review lead; the ids already observed in `docs/reviews/` are
0129 (R2), 0131 (R1), 0133 (PA), so the id is taken as assigned and not
re-derived here.

## 2. References

- Project norms: `docs/rules.md` (N1–N20); reviewer contract:
  `docs/engineering/independent-review.md` (rev 0.7).
- Report format: `docs/reviews/review-template.md`.
- Prior review: `docs/reviews/2026-10-12-m4-m11-i2c-r1-review.md`
  (`FRICK-REVIEW-0131`).
- Contract: `src/hal/hal_spi.h` (`FF_I2c_t`).
- Requirements: `docs/requirements/SRS.md` (SRS-REQ-058),
  `docs/requirements/SSS.md` (SSS-REQ-020), `docs/requirements/traceability.yaml`.
- Gaps: `docs/quality/class-a-gaps.md` (GAP-A5).
- Evidence: `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md`,
  `docs/vv/renode-environment.md`, `CHANGELOG.md`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Prior findings closure (R1-01…R1-03, R1-C1…R1-C9)

| Finding | Disposition (as declared) | First-hand evidence seen (this session) | Status |
| ------- | ------------------------- | --------------------------------------- | ------ |
| **R1-01** (Minor) — 100 kHz request resolves to 104,166 Hz, returned `FF_SUCCESS` | Documented minimum achievable SCL + boundary tests | `ff_i2c_mpfs.h:40-44,146-149` define `FF_I2C_MIN_SCL_HZ (= FF_I2C_MPFS_CLK_HZ/960)` and state a below-minimum request is not honoured; SRS-REQ-058 (`SRS.md`, rev 1.17) states the same; `test_i2c.c:272-278` exercises `init(104167u)`/`init(104166u)`/`init(100000u)` → `div_code 4`; `class-a-gaps.md` GAP-A5 (rev 0.7) adds the per-rate SCL-accuracy residual; `verification-results.md:46` (rev 0.34) and `CHANGELOG.md` reconciled | **CLOSED** |
| **R1-02** (Minor) — `i2c_stop()` does not confirm STOP completion | Documented as a bounded fire-and-forget limitation + GAP-A5 residual (state machine left unchanged by explicit disposition) | `ff_i2c_mpfs.h:52-56` documents STOP as fire-and-forget with a rationale (PUS inter-transaction gap ≫ STOP duration) and maps completion/bus-recovery to GAP-A5; `ff_i2c_mpfs.c:145-150` unchanged (as dispositioned); `class-a-gaps.md` GAP-A5 records the STOP-completion residual | **CLOSED** (residual tracked in GAP-A5) |
| **R1-03** (Minor) — "1 MHz tested" while 2 MHz executed | Genuine `init(1000000u)` → `div_code 5` case added; records reconciled | `test_i2c.c:279-281` (`init(1000000u)` → `div_code 5`, PCLK/120); runtime `checks=144`; `CHANGELOG.md:22,76,83` and `verification-results.md:46` now say 144 checks and the correct rates | **CLOSED** |
| **R1-C1** (Comment) — mock is not an independent oracle | Test-file header comment qualified | `test_i2c.c:10-18` states the mock is a behavioural/phase-level fake, not an independent oracle, cannot discriminate a wrong CTRL/SI bit assignment, acts on STA/STO before the SI write-gate, and points to `docs/vv/renode-environment.md` | **CLOSED** |
| **R1-C2** (Comment) — `Setup` does not validate `read8`/`write8` | NULL-hook rejection added + test | `ff_i2c_mpfs.c:346-348` returns `FF_INVALID_ARGUMENT` when `read8`/`write8` is NULL; `test_i2c.c:230,235-239` asserts both NULL-hook cases | **CLOSED** |
| **R1-C3** (Comment) — not re-entrant | Documented in header limitations | `ff_i2c_mpfs.h:57-60` states the backend is NOT re-entrant and calls must be serialised | **CLOSED** |
| **R1-C4** (Comment) — self-test configuration side effects | Documented in header limitations | `ff_i2c_mpfs.h:61-63` (and `:196-198`) states the self-test runs `init()` and leaves `ADDR`/`SLAVE1_ADDR` at 0; run before normal operation | **CLOSED** |
| **R1-C7** (Comment) — typos `achieveable`, `CounterI2C` | Fixed | `grep` over `src/ tests/ config/ docs/` finds neither token outside the prior review report; `ff_i2c_mpfs.c:67` reads "achievable"; the header reads "CoreI2C" | **CLOSED** |
| **R1-C8** (Comment) — limitations lacked 10-bit/general-call scope | 7-bit-only limitation added | `ff_i2c_mpfs.h:50-51` states 7-bit addressing only (no 10-bit, no general call); `FF_I2C_ADDR7_MAX` enforced at `ff_i2c_mpfs.c:223,246,275` | **CLOSED** |
| **R1-C9** (Comment) — T14 (cppcheck) not reproducible | Reproducible `make misra-docker` target added; report regenerated; T14 row updated | `Makefile:53-55` runs `tools/scripts/misra.sh` in `ubuntu:24.04` with apt `cppcheck`; I ran it → **13 advisory findings, 0 in any I2C file** (`build/misra-report.txt`); `tool-qualification.md` T14 (rev 0.3.18) points to `make misra-docker` | **CLOSED** (residual noted, §4 D3) |
| **R1-C5** (Comment) — SI poll is a spin count, not a wall-clock timeout | Accepted as documented scope | `ff_i2c_mpfs.h:64-66` documents the bounded spin count and maps on-target worst-case latency to GAP-A5 | **CLOSED (accepted)** |
| **R1-C6** (Comment) — `ARB_LOST` unhandled / no bus recovery | Accepted as polled single-master scope | `ff_i2c_mpfs.h:47-49` documents "no clock stretching / bus-recovery / arbitration-loss handling"; GAP-A5 records the bus-recovery residual | **CLOSED (accepted)** |

**Regression check on the fixes:** no regression observed in the touched scope.
The only behavioural code change (R1-C2, `Setup` NULL-hook rejection) affects
only misconfigured callers; the Renode app and every valid caller pass a
fully-populated bus, and all gates re-ran green (see the Evidence ledger). The
remaining changes are comments/records/tests. The R1-C5/R1-C6 acceptances remain
reasonable: both are explicitly disclosed as bounded reference-scope limitations
and carried in GAP-A5.

## 4. New findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| **N-R1-D1** | **Minor** | Records consistency (N8/N11/N16) | **The reconciled check count is inconsistent across the increment's records.** The fix moved the executed test from 135 to **144 checks** (verified: `checks=144`) and updated `CHANGELOG.md`, `verification-results.md` and `tool-qualification.md` to 144, but three current status records still say **135 checks**: `docs/status/status.json` (meta note, "Last cycle … test_i2c 135 checks"), `docs/status/handover.md` (§5 "S3 (MSS I2C) delivered … 135 checks" and History 1.8), and `docs/planning/sprint-plan.md` (History 0.4). The increment's own `CHANGELOG` claims "Records reconciled: `test_i2c` = 144 checks", which these records contradict. | `docs/status/status.json:5`; `docs/status/handover.md:140,204`; `docs/planning/sprint-plan.md:103`; executed `./build/native/tests/test_i2c` → `checks=144`; `verification-results.md:46`; `CHANGELOG.md:22,83` | Update the three records to 144 checks (or restate them as historical snapshots) so the S3 record set is internally consistent before commit/push. |
| **N-R1-D2** | Comment | SRS-REQ-058 limitation wording | The header limitation says general call is "not implemented (an address above `FF_I2C_ADDR7_MAX` is rejected)" (`ff_i2c_mpfs.h:50-51`). The general-call address is **0x00**, which is *not* above `0x7F`, so a `write(0, …)` is **not** rejected — it would emit SLA+W `0x00` (a general-call write). The two statements are not equivalent. | `ff_i2c_mpfs.h:50-51,129`; `ff_i2c_mpfs.c:223` | Reword to "7-bit device addressing only; general-call (address 0) and 10-bit addressing are not specially handled" or reject address 0 explicitly, to keep the limitation accurate. |
| **N-R1-D3** | Comment | Tool evidence (N10/T14) | Residual on the R1-C9 fix: the `misra-docker` path is tag-pinned (`ubuntu:24.04`) but the `cppcheck` revision is fetched at run time via apt (today 2.13.0) and is not pinned; the V&V image declared as the reproducibility anchor (`tools/docker/Dockerfile:15` lists `cppcheck`) is **stale on this host** — `docker run frick-vv:local` has no `cppcheck` — and `tools/scripts/misra.sh:17-20` exits **0** when `cppcheck` is absent, so the MISRA gate can never fail for a missing tool (silent no-op). | `Makefile:53-55`; `tools/docker/Dockerfile:15`; `docker run --rm frick-vv:local … which cppcheck` → absent; `tools/scripts/misra.sh:17-20`; `ubuntu:24.04` apt `cppcheck` → 2.13.0 | Route to R2/PA: pin the checker revision (digest or version) and/or make `misra.sh` exit non-zero when the tool is missing, so a green T14 is meaningful; rebuild the T1 image so it matches its Dockerfile. |
| **N-R1-D4** | Comment | Test clarity | `test_divider_selection` calls `init(100000u)` twice with the same expectation (`test_i2c.c:270-271` and `:277-278`); the second is redundant. | `test_i2c.c:270-271,277-278` | Remove the duplicate or repurpose it to a distinct below-minimum value; cosmetic. |
| **N-R1-D5** | Comment (pre-existing) | Test hygiene | `test_setup_and_init` and `test_self_test` bind a **stack-local** `FF_I2cMpfs_t` as the module-global `g_i2c`, which then dangles after the function returns; this is safe only because every later test re-binds first via `mock_bind()`. Not introduced by the fix, but relevant to V&V independence/robustness. | `test_i2c.c:227,240,466-472`; `ff_i2c_mpfs.c:22,357` | Note for R2: prefer the file-scope instance for binding, or null `g_i2c` on scope exit, to remove the latent dangling pointer. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
  *(None identified.)*
- **Minor**: non-conformity that does not invalidate the requirement but must be
  corrected or justified. *(N-R1-D1.)*
- **Comment**: observation/recommendation, no derogation. *(N-R1-D2…N-R1-D5.)*

## 5. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — all prior R1 findings R1-01…R1-03 and
  R1-C1…R1-C9 are **CLOSED** (or accepted as documented scope), with no
  regression; one new **Minor** finding (N-R1-D1) is raised and must be closed
  per norm N12, and four Comments (N-R1-D2…N-R1-D5) are advisory.
- [ ] REJECTED — one or more Major findings

The three Minor findings from `FRICK-REVIEW-0131` are resolved by documentation
and tests that I re-executed/read first-hand; the driver's polled CoreI2C state
machine, bounds, HAL-contract compliance and error behaviour are unchanged and
sound. The only substantive code change (R1-C2) is a correct, well-tested
hardening. The increment is suitable to proceed once N-R1-D1 (records
consistency) is corrected; no R1 blocker remains.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | Senior FSW engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** (R1-01…R1-03, R1-C1…R1-C9 closed; N-R1-D1 to closure per N12) |

## 7. Follow-up records

(norm N12: NCs are resolved by re-applying all norms to the affected scope and
re-reviewing the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | Minor | SRS-REQ-058 divider | 100 kHz request yields 104,166 Hz, returned as `FF_SUCCESS` | Documented minimum achievable SCL (`FF_I2C_MIN_SCL_HZ`) + boundary tests | `ff_i2c_mpfs.h:40-44,146-149`; `test_i2c.c:272-278`; SRS-REQ-058; GAP-A5 | frick-fsw | **closed** | this report |
| R1-02 | Minor | SRS-REQ-058 STOP | `i2c_stop()` does not confirm STOP completion | Documented fire-and-forget limitation + GAP-A5 residual | `ff_i2c_mpfs.h:52-56`; GAP-A5 | frick-fsw | **closed** | this report |
| R1-03 | Minor | Evidence (N11) | "1 MHz" claimed; 2 MHz executed | 1 MHz (code 5) case added; records reconciled | `test_i2c.c:279-281`; runtime `checks=144`; `CHANGELOG.md`; `verification-results.md:46` | frick-fsw | **closed** | this report |
| R1-C1 | Comment | V&V independence | mock not an independent oracle | Qualified in the test header | `test_i2c.c:10-18` | frick-fsw | **closed** | this report |
| R1-C2 | Comment | Robustness | `Setup` did not validate bus hooks | NULL-hook rejection + test | `ff_i2c_mpfs.c:346-348`; `test_i2c.c:235-239` | frick-fsw | **closed** | this report |
| R1-C3 | Comment | Concurrency | not re-entrant | Documented limitation | `ff_i2c_mpfs.h:57-60` | frick-fsw | **closed** | this report |
| R1-C4 | Comment | Self-test | configuration side effects | Documented limitation | `ff_i2c_mpfs.h:61-63` | frick-fsw | **closed** | this report |
| R1-C5 | Comment | Timing bound | SI poll is a spin count | Accepted; GAP-A5 | `ff_i2c_mpfs.h:64-66` | frick-fsw | **closed (accepted)** | this report |
| R1-C6 | Comment | Multi-master | `ARB_LOST` unhandled | Accepted (polled single-master); GAP-A5 | `ff_i2c_mpfs.h:47-49` | frick-fsw | **closed (accepted)** | this report |
| R1-C7 | Comment | Quality (N2) | two typos | Fixed | `grep` over `src/ tests/ config/` finds neither token | frick-fsw | **closed** | this report |
| R1-C8 | Comment | Limitations scope | 10-bit/general call absent | 7-bit-only limitation added | `ff_i2c_mpfs.h:50-51` | frick-fsw | **closed** | this report |
| R1-C9 | Comment | Tool evidence (N10) | T14 not reproducible | `make misra-docker` added; report regenerated | `Makefile:53-55`; `make misra-docker` → 13 findings, 0 I2C; `tool-qualification.md` T14 | frick-fsw | **closed** (residual N-R1-D3) | this report |
| N-R1-D1 | Minor | Records (N8/N11/N16) | status records still say 135 checks vs executed 144 | (to be corrected) | `docs/status/status.json:5`; `docs/status/handover.md:140,204`; `docs/planning/sprint-plan.md:103` | frick-fsw (R1 delta) | **open** | pending |
| N-R1-D2 | Comment | SRS-REQ-058 wording | general-call statement imprecise | (to be reworded) | `ff_i2c_mpfs.h:50-51` | frick-fsw | open (advisory) | — |
| N-R1-D3 | Comment | Tool evidence (N10/T14) | checker revision unpinned; V&V image stale; `misra.sh` no-op on missing tool | route to R2/PA | `tools/scripts/misra.sh:17-20`; `tools/docker/Dockerfile:15` | frick-fsw / R2 / PA | open (advisory) | — |
| N-R1-D4 | Comment | Test clarity | duplicate `init(100000u)` case | (cosmetic) | `test_i2c.c:270-271,277-278` | frick-fsw | open (advisory) | — |
| N-R1-D5 | Comment | Test hygiene | stack-local instance bound as `g_i2c` | note for R2 | `test_i2c.c:227,466-472` | frick-fsw / R2 | open (advisory) | — |

## 8. Evidence ledger (all re-executed/obtained first-hand on 2026-10-12)

| Claim | Method | Result |
| ----- | ------ | ------ |
| Native build | `cmake -S . -B build/native && cmake --build build/native` | BUILD OK |
| Unit tests | `ctest --test-dir build/native` | **20/20 pass** (incl. `i2c`) |
| `test_i2c` checks | `./build/native/tests/test_i2c` | **`checks=144 failures=0`** |
| Sanitizers | `-fsanitize=address,undefined` + `ctest` | **20/20 pass** |
| Formatting | `clang-format --dry-run --Werror` on the 4 changed C/H files | clean (4/4) |
| Warning hygiene | strict warning set on `ff_i2c_mpfs.c` | clean |
| Traceability | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** (100 reqs / 59 srs / 27 sss / 14 gaps, 51 theorems) |
| Lean | `lake build` (host lacks `lake`; run in `frick-vv:local`) | Build completed successfully |
| Renode functional run | `make renode` | **PASS** (`RENODE_TEST: PASS`, incl. `hal_i2c=PASS`) |
| Smoke | `make smoke` | PASS (banner + 6 apps) |
| MISRA/T14 | `make misra-docker` | 13 advisory findings, **0 in any I2C file** |
| `cppcheck` availability | host + `frick-vv:local` probe; `ubuntu:24.04` apt | host absent; `frick-vv:local` absent; `ubuntu:24.04` → 2.13.0 |
| Divider numerics | hand computation at 100 MHz | 100000→code4/104166, 104166→code4, 104167→code4, 1000000→code5/833333, 400000→code0/390625, 500000→code1/446428, 2000000→code6/1666666 |
| Check-count records | `grep` across `docs/` + `CHANGELOG.md` | 144 in CHANGELOG/verification-results/tool-qualification; **135 in status.json/handover.md/sprint-plan.md** (N-R1-D1) |
| Typo scan | `grep -rn "achieveable\|CounterI2C"` | none in sources |

---

## Facts, Assumptions, Inferences, Recommendations, Confidence, Open gaps

**Facts** (verified first-hand, this session)

- The driver implements the `FF_I2c_t` contract without deviation and is
  unchanged except for the `Setup` NULL-bus-hook rejection (`ff_i2c_mpfs.c:346-348`).
- `ff_i2c_mpfs.h` now documents: the minimum achievable SCL
  (`FF_I2C_MIN_SCL_HZ = 104166`, `:146-149`), 7-bit-only addressing (`:50-51`),
  the fire-and-forget STOP (`:52-56`), non-re-entrancy (`:57-60`), and the
  self-test side effects (`:61-63`).
- `test_i2c.c` adds a real `init(1000000u)`→`div_code 5` case, the 104166/104167
  minimum-SCL boundary cases, and NULL-bus-hook rejection cases; runtime
  `checks=144`, `failures=0`.
- The test-file header qualifies the mock as a behavioural/phase-level fake, not
  an independent oracle (`test_i2c.c:10-18`).
- The two cited typos are gone from `src/`, `tests/` and `config/`.
- Gates: native `ctest` 20/20; ASan/UBSan 20/20; `clang-format` clean; strict
  warnings clean; `trace.py` PASS (100/59/27/14, 51 theorems); `make renode`
  PASS with `hal_i2c=PASS`; `make smoke` PASS; `lake build` success.
- `make misra-docker` reproduces the T14 scan: 13 findings, 0 in the I2C files.
- `cppcheck` is absent from the host and from `frick-vv:local`, though
  `tools/docker/Dockerfile:15` lists it; `misra.sh` exits 0 when it is missing.
- Three current status records (`status.json`, `handover.md`, `sprint-plan.md`)
  still report "135 checks" while the executed test and the other records report
  144.

**Assumptions** (owner, impact, validation)

- *Document id `FRICK-REVIEW-0134`* — owner: review lead; impact: report identity
  only; validation: lead renumbers if needed.
- *`FF_I2C_MPFS_CLK_HZ = 100 MHz` is the reference APB clock* — owner:
  hardware/mission (GAP-A5); validation: confirm the configured APB clock against
  the RTPFS160ZT datasheet and re-derive the divider.
- *The uncommitted working-tree delta is the fixed revision under review atop
  `db8261f`*.

**Inferences**

- Because the fix changed the executed check count 135→144 and updated only some
  records, the status/handover/sprint-plan "135" entries are stale rather than
  intentionally historical → N-R1-D1 (Minor).
- Because `misra.sh` returns 0 when `cppcheck` is missing and the V&V image here
  lacks `cppcheck`, a green T14 could in principle be a no-op → N-R1-D3
  (Comment, for R2/PA).

**Recommendations** (decision owner, prerequisites, trade-offs)

1. Correct the three status records to 144 checks (or mark them historical) and
   re-run the delta through R1/PA (N12/N20). Owner: authoring agent.
2. Reword the general-call limitation (N-R1-D2). Owner: authoring agent.
3. Route N-R1-D3 to R2/PA under N10: pin the checker revision and make the MISRA
   gate fail closed when the tool is absent; rebuild the T1 image.
4. Close N-R1-D4/N-R1-D5 as advisory test-quality items (owner: authoring agent /
   R2).

**Confidence:** **High.** Every gate was re-executed first-hand and is internally
consistent; the closure evidence for R1-01…R1-03 and R1-C1…C9 rests on direct
source/doc reads plus the executed test output (144 checks). The one new Minor
finding rests on a direct, reproducible text comparison (135 vs 144).

**Open gaps**

- **N-R1-D1** — records inconsistency (Minor), to be closed.
- **T14/misra reproducibility residual** — N-R1-D3 (Comment).
- **On-target evidence absent** — GAP-A5 (per-rate SCL accuracy, STOP completion,
  bus recovery).
- **No independent oracle** for the transaction path — disclosed (R1-C1) and for
  R2/V&V.
- **Document id** `FRICK-REVIEW-0134` retained as assigned.

---

*This report is authored to be committed and pushed with the reviewed change
(norm N11). The review was read-only: no repository file was modified or
created other than this report, and nothing was committed or pushed. The
`docs/reviews/README.md` log row for this report was intentionally not added, as
the task scoped this reviewer to writing exactly one file.*
