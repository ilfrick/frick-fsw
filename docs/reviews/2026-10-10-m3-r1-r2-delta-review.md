# Milestone M3 delta re-review (profiles R1 + R2)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0017                      |
| Reviewer session                  | independent subagent, fresh context (norm N9) |
| Reviewer profiles                 | R1 (design/code) **and** R2 (V&V) |
| Reference revision                | branch `main`, uncommitted M3 working tree on base commit `3d3511c` |
| Items under review                | NC dispositions in `docs/reviews/2026-10-10-m3-hal-r1-review.md` (M3-R1-01..14) and `docs/reviews/2026-10-10-m3-vv-r2-review.md` (NC-1..10); fix artifacts `src/hal/*`, `tests/unit/test_hal.c`, `docs/requirements/traceability.{md,yaml}`, `docs/vv/{verification-results,tool-qualification}.md`, `docs/{status/status.json,roadmap.md,development-plan.md}`, `verification/lean/Frick/{Hal,Requirements}.lean` |
| Date                              | 2026-10-10                             |
| Scope                             | Re-apply ALL norms to the affected M3 scope (norm N12) and verify each recorded NC disposition against the working tree; reproduce the trace, build and ctest gates. |
| Independence declaration          | I am a freshly spawned independent reviewer; I took no part in authoring, fixing, building or previously reviewing the M3 items. Every claim below is from the repository contents and reproduced commands only. |
| Norms applied                     | N1-N17 (see `docs/rules.md`), emphasis N4, N8, N10, N11, N12 |

## 1. Scope

Delta re-review of the milestone-M3 NC dispositions of the R1 (design/code) and
R2 (V&V) reviews against the current uncommitted working tree. Verified each NC
fix, re-ran the full gate chain, and independently re-derived the recorded V&V
numbers. Excluded: M4 driver implementations, on-target/HWIL qualification
(GAP-A5), and unrelated M3 documents except where they carry NC evidence.

Reproduced commands (all from `/home/nicola/fsw/frick-fsw`):

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml`
  → **PASS** (63 requirements: 33 srs / 17 sss / 13 gap; 45 Lean theorems; 9 CI
  claims).
- `cmake -S . -B build/drev -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build
  build/drev && ctest --test-dir build/drev --output-on-failure` → **6/6 pass**
  (`ccsds, sb, tbl, pus, time, hal`).
- Per-suite check counts re-derived from the built binaries: ccsds 15, sb 20,
  tbl 9, pus 29, time 101, **hal 31** = **205** total (see finding D-1).
- Lean transcript `verification/lean/lake-build.log` inspected: `Frick.Hal`
  built, 19/19, "Build completed successfully" (`lake` unavailable on host, as in
  the prior reviews).

## 2. References

- Project norms `docs/rules.md` (N1-N17); review template
  `docs/reviews/review-template.md`.
- `docs/reviews/2026-10-10-m3-hal-r1-review.md` (FRICK-REVIEW-0013);
  `docs/reviews/2026-10-10-m3-vv-r2-review.md` (FRICK-REVIEW-0014).
- `docs/requirements/traceability.yaml` / `traceability.md`;
  `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C.

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| D-1 | Minor | `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md` / N11 | The recorded M3 per-suite check counts are **stale by one**: both files state `hal 30` / `204 checks`, but the working tree builds `test_hal` with **31** checks (`205` total). The recorded evidence is therefore not reproducible as written. The extra check is the TSN contract assert added by the NC-4 fix (`test_hal.c:110`), which was not propagated into the V&V records. | `docs/vv/verification-results.md:24` (`204 checks: ... hal 30`), `:156-157` (`hal 30 ... 204 total`); `docs/vv/tool-qualification.md:73` (`30 checks`); reproduced: `ctest --test-dir build/drev -R '^hal$' -V` → `test_hal ... checks=31 failures=0`; per-suite sum 15+20+9+29+101+31 = 205. | Update the two records to `hal 31` / `205 checks` (and re-run `make coverage`/`make metrics` only if the README panel changes; coverage 892/1514 stays consistent). |
| D-2 | Minor | R1 follow-up §6 / N8, N12 | Several M3-R1 Comment dispositions are marked "corrected (fix applied as recommended)" but the changes are **absent or only partial** in the working tree: **R1-10** (`hal_net.h` still has no negative-descriptor error convention — only the opaque-descriptor comment at `hal_net.h:17`); **R1-13** (`hal_spacewire.h:39` still exposes 1-byte `and_mask`/`or_mask` with no tailoring note); **R1-11** (the startup/single-thread note at `hal.h:52-55` documents timing but no architecture/integration doc names the `FF_HalInit` caller); **R1-12** (`hal.h:44-47` documents pointer lifetime/no-copy but no `FF_HAL_NAME_MAX` or explicit NUL-termination obligation). The R1-06 README reconciliation is also incomplete: `README.md:114-115` still lists "HAL interfaces ... (M3)" under "Planned extensions" while `README.md:131-132` states they are delivered. | `src/hal/hal_net.h:1-40`; `src/hal/hal_spacewire.h:39`; `src/hal/hal.h:44-47,52-55`; `docs/architecture/hardware-abstraction.md` (no `FF_HalInit`); `README.md:114-115` vs `:131-132`. | Either apply the recommended edits, or change the disposition wording to "accepted (Comment, no action)" so the table reflects reality. These are Comment-severity (no derogation), so they do not block R1 approval but must not be recorded as applied when they are not. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item, or a defect that can
  defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must be
  corrected or justified.
- **Comment**: observation, recommendation, no derogation.

## 4. Conclusion

- **R1 (design/code): APPROVED WITH COMMENTS.** All seven Minor NCs
  (M3-R1-01..07) are substantively corrected and verified: the `hal_i2c.h`
  dangling reference is gone, `hal_net.h` self-typedefs `FF_NetStack_t` and
  `hal_eth.h` includes it, `test_hal.c` includes every contract header and
  instantiates all 18 contract types, the registry concurrency contract is
  documented, the V&V/tool-qualification registers and milestone status are
  updated, and the Lean theorems are renamed with `req_srs025_*` left as
  wrappers (45 unique names now agree with the gate). Residual: finding D-2
  (overstated/partial Comment dispositions, incl. the R1-06 README wording).
- **R2 (V&V): APPROVED WITH COMMENTS.** Both Major findings are resolved: NC-1
  (matrices §1-4 now carry SSS-REQ-017, SRS-REQ-025..032, GAP-A8..A13; counts
  33/17/13/45 match the gate) and NC-2 (an M3 cycle row and M3 notes exist,
  with Lean 19/19, trace 63/45, coverage 892/1514). NC-3..NC-5 are corrected.
  Comments NC-6..NC-10 confirmed. Residual: finding D-1 (recorded `hal 30`/204
  must be corrected to `hal 31`/205).

No Major finding remains. Milestone-M3 closure is unblocked; D-1 and D-2 are
Minor and must be corrected before the records are considered final (norm N12).

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | Independent reviewer agent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS (D-2) |
| R2 (N9)       | Independent reviewer agent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS (D-1) |

## 6. Follow-up records

### 6.1 R1 delta — M3-R1-01..14

| NC | Severity | Item | Delta verdict | Evidence |
| -- | -------- | ---- | ------------- | -------- |
| M3-R1-01 | Minor | `hal.h` banner/enum | **CLOSED** | `hal.h:6-7` (`hal_spi.h [SPI + I2C]`), `:29` (`I2C master (hal_spi.h)`); no `hal_i2c.h` reference. |
| M3-R1-02 | Minor | `test_hal.c` contract presence | **CLOSED** | `test_hal.c:13-23` includes all 11 contract headers; `:68-112` instantiates all 18 types incl. TSN/I2c/NetIf/CanOpen/Dma/Mtd/Nvm/Rmap; `hal_tsn.h` now compiled. |
| M3-R1-03 | Minor | `hal_net.h` self-containment | **CLOSED** | `hal_net.h:20` `typedef struct FF_NetStack FF_NetStack_t;`; `hal_eth.h:16` includes `hal_net.h`. |
| M3-R1-04 | Minor | registry concurrency | **CLOSED** | `hal.h:50-55` documents start-up single-threaded registration and lock-free read-only lookups. |
| M3-R1-05 | Minor | V&V records | **CLOSED (with D-1)** | `verification-results.md:24,136-180` M3 row + notes; `tool-qualification.md:7,73` rev 0.1.5. Count defect D-1. |
| M3-R1-06 | Minor | status/README | **CLOSED (residual)** | `status.json:11` M3 "in progress"; `roadmap.md:49`; `development-plan.md:172`. README planned wording incomplete (D-2). |
| M3-R1-07 | Minor | Lean naming | **CLOSED** | `Hal.lean:30,43` descriptive names; `Requirements.lean:160,165` wrappers; trace.py 45 unique. |
| M3-R1-08 | Comment | `FF_BUILD_HAL=OFF` scope | **CLOSED** | `hardware-abstraction.md:93-100` documents contracts-always, registry toggled by `FF_BUILD_HAL` (default ON). |
| M3-R1-09 | Comment | Lean abstraction note | **PARTIAL** | `formal-verification.md:27,60` describe `Frick.Hal` scope; exclusions not enumerated. Non-derogatory. |
| M3-R1-10 | Comment | `hal_net.h` error convention | **NOT CLOSED** | No negative-descriptor convention; `hal_net.h:17` only. See D-2. |
| M3-R1-11 | Comment | `FF_HalInit` caller | **PARTIAL** | `hal.h:52-55` documents start-up timing; no architecture/integration doc names the caller. See D-2. |
| M3-R1-12 | Comment | name bound / NUL | **PARTIAL** | `hal.h:44-47` documents pointer lifetime; no `FF_HAL_NAME_MAX`/explicit NUL obligation. See D-2. |
| M3-R1-13 | Comment | RMAP 1-byte masks | **NOT CLOSED** | `hal_spacewire.h:39` unchanged, no tailoring note. See D-2. |
| M3-R1-14 | Comment | `FF_MemPool` semantics | **CLOSED** | `hal_mem.h:6,19` static/bounded pool, no dynamic allocation. |

### 6.2 R2 delta — NC-1..10

| NC | Severity | Item | Delta verdict | Evidence |
| -- | -------- | ---- | ------------- | -------- |
| NC-1 | Major | `traceability.md` matrices | **CLOSED** | §1 `:42` SSS-REQ-017 + 8 children; §2 `:73-80` SRS-REQ-025..032; §3 `:107` SSS-REQ-017; §4 `:120-125` GAP-A8..A13; counts match `trace.py` (33/17/13). |
| NC-2 | Major | `verification-results.md` M3 record | **CLOSED (with D-1)** | M3 cycle row `:24` and M3 notes `:136-180` added; gate reproduced. Recorded counts stale (D-1). |
| NC-3 | Minor | `tool-qualification.md` T3 | **CLOSED (with D-1)** | Rev 0.1.5 `:7,73` records `test_hal.c`; the "30 checks" figure is D-1. |
| NC-4 | Minor | SRS-REQ-026..032 test evidence | **CLOSED** | `test_hal.c` now includes all contract headers and instantiates every contract type (see M3-R1-02). |
| NC-5 | Minor | `hal.h` `hal_i2c.h` | **CLOSED** | See M3-R1-01. |
| NC-6 | Comment | `lean_note` inspection evidence | **CLOSED** | `verification-results.md:159-165` "Inspection evidence (methods Inp)" records the header-inspection rationale and contract-presence test. |
| NC-7 | Comment | status/plan/roadmap | **CLOSED** | `status.json:11`; `roadmap.md:49`; `development-plan.md:172` all "in progress". |
| NC-8 | Comment | `Frick.Hal` model abstraction | **CLOSED** | `formal-verification.md:27,60`; `Hal.lean:3-8` scope statement. |
| NC-9 | Comment | capacity test coupling | **CLOSED** | Accepted; `test_hal.c:51-66` bound to configured `FF_HAL_MAX_BACKENDS` as dispositioned. |
| NC-10 | Comment | Lean gate reproducibility | **CLOSED** | `verification/lean/lake-build.log` 19/19 incl. `Frick.Hal`, "Build completed successfully". |

---

Committed and pushed with the reviewed change (norm N11).
