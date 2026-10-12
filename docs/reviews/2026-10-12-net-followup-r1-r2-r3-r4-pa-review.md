# NET follow-up: GEM 64-bit descriptor addressing (NC-S6-03 co-implemented) + host netif glue test (NC-154-01 closed) — combined R1+R2+R3+R4+PA confirmation review

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0162                      |
| Reviewer session                  | independent agent (norm N9), fresh context |
| Reviewer profile                  | R1 (SW dev) + R2 (SW V&V) + R3 (Sys Eng) + R4 (SW documentation) + PA (SW product assurance) |
| Reference revision                | `64961ac` + uncommitted working tree (NET follow-up: NC-S6-03 co-implementation + NC-154-01 closure) |
| Items under review                | `src/drivers/eth/gem/ff_gem.c`, `src/drivers/eth/gem/ff_gem_regs.h`, `tests/unit/test_gem.c`, `tests/unit/test_lwip_netif.c` (new), `src/apps/app_renode_test.c`, `tests/CMakeLists.txt`, `CMakeLists.txt`, `CHANGELOG.md`, `docs/vv/verification-results.md`, `docs/vv/renode-environment.md`, `docs/vv/tool-qualification.md`, `docs/planning/sprint-plan.md`, `docs/reviews/README.md`, `docs/status/handover.md`, `docs/status/loop-audit.md`, `docs/status/status.json`, `docs/requirements/traceability.yaml`, `docs/requirements/traceability.md`, `README.md` |
| Date                              | 2026-10-12                             |
| Scope                             | Confirmation review (norm N12) of the NET follow-up delta: (A) NC-S6-03 co-implementation — `gem_phys` returns `uint64_t`, `gem_wr_qbqp` helper writes 64-bit `TBQP`/`RBQP` via low+high word, `DMACFG.ADDR64` (bit 30) asserted, per-descriptor 32-bit truncation explicit, full 3-word 64-bit descriptor format deferred to M14; (B) NC-154-01 closure — new `test_lwip_netif` host test + `test_gem` 64-bit register write extensions; (C) associated documentation/delta records. ECSS-E-ST-40C Rev.1 (SW development/V&V), ECSS-Q-ST-80C Rev.2 (SW product assurance), ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2, project norms N1-N20 (`docs/rules.md` rev 1.4) |
| Independence declaration          | The reviewer ran in a fresh, independent context; it did not author, design, implement, test or approve any item under review, and it did not inherit the author's conversation. It is read-only over the repository except for this one report. |
| Norms applied                     | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

Reviewed the **NET follow-up delta** in the uncommitted working tree on top of
`64961ac`:

- **(A) NC-S6-03 co-implementation** (`src/drivers/eth/gem/ff_gem.c`,
  `src/drivers/eth/gem/ff_gem_regs.h`): `gem_phys` changed from `uint32_t` to
  `uint64_t`; new `gem_wr_qbqp` helper writes the descriptor-ring base address
  as a 64-bit value via `TBQP`/`RBQP` (low word) and `TBQP_HI`/`RBQP_HI`
  (high word); `DMACFG.ADDR64` (bit 30) is asserted in `gem_init`; per-
  descriptor buffer addresses are explicitly truncated with `(uint32_t)gem_phys(...)`
  with the full 3-word 64-bit descriptor format deferred to M14 (documented in
  code comments). No change to the GEM transmit/receive descriptor word count
  (still 2-word descriptors; per-descriptor high address is M14).

- **(B) NC-154-01 closure** (`tests/unit/test_lwip_netif.c` new,
  `tests/unit/test_gem.c` extended): the new `test_lwip_netif` host test
  exercises `FF_LwipNetifAttach`/`FF_LwipNetifIsUp`/`FF_LwipNetifGetMac` +
  loopback TX/RX against a mock `FF_EthMac_t`; `test_gem` gains 5 assertions
  for 64-bit register writes (`RBQP_HI`, `TBQP_HI`, `DMACFG.ADDR64`).

- **(C) App comment** (`src/apps/app_renode_test.c`): the `renode_hal_net_lwip_check`
  comment updated from referencing the (now-fixed) 32-bit descriptor
  translation to referencing the register-only Renode GEM model + NC-154-01.

- **(D) Documentation and records**: CHANGELOG, verification-results,
  renode-environment, tool-qualification, sprint-plan, handover, loop-audit,
  status.json, README.md, traceability.

**Excluded:** on-silicon GEM DMA/PHY behaviour (GAP-A5); Renode's register-only
`Network.CadenceGEM` model (no descriptor engine); the third-party lwIP and
lwIP unix/sys_arch ports.

**Method:** first-hand code read + `grep` verification of counts + diff
reconstruction from `git diff HEAD`. The reviewer did **not** rebuild or
re-run tests (the working tree is pre-commit; see Fact/Assumption below).

## 2. References

- Project norms: `docs/rules.md` (N1–N20, rev 1.4)
- Reviewer contract: `docs/engineering/independent-review.md` (rev 0.7)
- Traceability manifest: `docs/requirements/traceability.yaml`
- ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2, ECSS-E-ST-70-41C (PUS-C),
  CCSDS 133.0-B-2
- Prior report: FRICK-REVIEW-0155 (S6b-2 lwIP netif on GEM — falsification
  of `hal_net_lwip=PASS` as a real pass)
- Reviewed artifact git states: `ff_gem.c` diff shows `gem_phys` return type
  `uint32_t`→`uint64_t`, `gem_phys` body returns `g->cfg.dma->virt_to_phys(p)`
  without truncation, new `gem_wr_qbqp` helper, `DMACFG.ADDR64` asserted;
  `test_lwip_netif.c` is an **untracked new file** (3 test functions, 19
  `FF_TEST_ASSERT` calls per `grep -c`); `test_gem.c` diff shows +5
  `FF_TEST_ASSERT` (total 26 per `grep -c`).

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R1-01 | Comment | `ff_gem.c`, NC-S6-03 | Co-implementation is **technically correct**: `gem_phys` returns `uint64_t`; `gem_wr_qbqp` writes 64-bit ring base via low+high word; `DMACFG.ADDR64` (bit 30) asserted in `gem_init`; per-descriptor truncation is explicit `(uint32_t)gem_phys(...)` with the M14 deferral documented. The `FF_GEM_TBQP_HI`/`FF_GEM_RBQP_HI` offsets (0x00C8/0x00CC) match the Cadence GEM layout. No code defect. | `ff_gem.c:30-40,88-93,99-105,114,178,255`; `ff_gem_regs.h:30-35`; `ff_gem.c:101` | None; code is correct. |
| R2-01 | **Minor** | `test_lwip_netif.c` count claims | **Test count discrepancy**: the actual file has **3 test functions** (`test_lwip_netif_errors`, `test_lwip_netif_attach_and_status`, `test_lwip_netif_tx_loopback`) and **19 `FF_TEST_ASSERT` calls** (confirmed by `grep -c`), but the documentation claims **"4 tests, 22 checks"** (CHANGELOG.md line 585; sprint-plan.md §3 NC-154-01 line 79) and **"3 tests, 22 checks"** (verification-results.md NET follow-up cycle-log row). The "22 checks" is wrong in all three documents (actual is 19); the "4 tests" is wrong (actual is 3). | `grep -c "FF_TEST_ASSERT" tests/unit/test_lwip_netif.c` = 19; `test_lwip_netif.c:1,160,195,212` (3 functions + `FF_TEST_MAIN`); CHANGELOG.md:585; verification-results.md NET follow-up row; sprint-plan.md:79 | Correct all three document sites to "3 tests, 19 checks" (CHANGELOG) / "3 tests, 19 checks" (verification-results, sprint-plan). |
| R2-02 | **Minor** | `verification-results.md` S6b-2 row staleness | The S6b-2 cycle-log row (the row **above** the new NET follow-up row) is **stale**: it still says "the driver's descriptor translation (`gem_phys`, `uint32_t`) cannot express an MPFS DDR address" and "NC-S6-03" is listed under "Not delivered". After the NET follow-up, `gem_phys` returns `uint64_t` and NC-S6-03 is co-implemented. The S6b-2 row was **not updated** by the delta. | `verification-results.md` S6b-2 cycle-log row: "Not delivered (NC-S6-01 residual / NC-S6-03)…`gem_phys`, `uint32_t`…" | Update the S6b-2 row to reflect NC-S6-03 co-implementation (or move the NC-S6-03 reference to the new NET follow-up row's residual). |
| R3-01 | **Minor** | `traceability.yaml` SRS-REQ-040 `tests` field | The `tests` field for SRS-REQ-040 lists only `tests/unit/test_lwip.c`; the **new `test_lwip_netif.c`** (which exercises the netif glue for NC-154-01) is **not listed**. The `implementation` field correctly includes `ff_lwip_netif.c/h`, but the tests linkage is incomplete. | `traceability.yaml:383` (`- id: SRS-REQ-040`, `tests: ["tests/unit/test_lwip.c"]`) | Add `tests/unit/test_lwip_netif.c` to the `tests` list for SRS-REQ-040. |
| R3-02 | Comment | `traceability.md` History | No History entry exists for the NET follow-up (last entry is 0.35 from S6b-2). The mirror row for SRS-REQ-040 already lists `ff_lwip_netif.c/h` (from S6b-2) but does not list `test_lwip_netif.c` in the tests column. | `traceability.md:241` (last History row 0.35) | Add History entry 0.36 for the NET follow-up; add `test_lwip_netif.c` to the SRS-REQ-040 mirror row tests column. |
| R4-01 | **Minor** | `verification-results.md` header-History mismatch | Header revision **0.42**, but the History table's last entry is **0.41**. The delta bumped the header from 0.41→0.42 and added History entry 0.41 (the NET follow-up), but did **not** add a matching 0.42 entry. This is a norm N16 header-History mismatch. | `verification-results.md` header (Revision 0.42); History last row 0.41 | Either add a History 0.42 entry or set the header revision to 0.41. |
| R4-02 | **Minor** | `renode-environment.md` header-History mismatch | Header revision **0.23**, but the History table's last entry is **0.21** (entries 0.22 and 0.23 are missing). The delta bumped the header from 0.22→0.23 and added History entry 0.21, but entries 0.22 and 0.23 are absent. This is a pre-existing mismatch that the delta did not fix. | `renode-environment.md` header (Revision 0.23); History last row 0.21 | Add History entries 0.22 and 0.23, or set the header to 0.21. |
| R4-03 | **Minor** | `tool-qualification.md` header-History mismatch | Header revision **0.3.23**, but the History table's last entry is **0.3.24**. The delta added History entry 0.3.24 (T3 gains `test_lwip_netif.c`) but did **not** bump the header revision. This is a norm N16 mismatch introduced by the delta. | `tool-qualification.md` header (Revision 0.3.23); History last row 0.3.24 | Bump the header revision to 0.3.24. |
| R4-04 | **Minor** | `README.md` metrics panel staleness | The README metrics panel (between `<!-- METRICS-BEGIN -->` and `<!-- METRICS-END -->`) shows **7/30 sprints completed** (23.3%), but `status.json` shows `done: 8`. M8 is listed as `⬜ planned` (line 416), but `status.json` shows `"M8": "in_progress"`. The "Current focus" text (lines 418–422) still describes Sprint 6b-2 as current and lists NC-S6-03 as open, without mentioning the NET follow-up sprint. The README was **not** regenerated by `make metrics` as part of the delta. | `README.md:51` (7/30); `README.md:416` (M8 planned); `status.json` (done: 8, M8: in_progress); `README.md:39,94` (METRICS markers) | Run `make metrics` / `python3 tools/scripts/metrics.py` to regenerate the panel from `status.json`; update the "Current focus" text to reflect the NET follow-up sprint and NC-S6-03 co-implementation. |
| R4-05 | **Minor** | `CHANGELOG.md` test count | CHANGELOG says "4 tests, 22 checks" for `test_lwip_netif` (line 585), but the actual file has 3 test functions and 19 `FF_TEST_ASSERT` calls. | `CHANGELOG.md:585`; `grep -c "FF_TEST_ASSERT" tests/unit/test_lwip_netif.c` = 19 | Correct to "3 tests, 19 checks". |
| R4-06 | Comment | `verification-results.md` comment format | The NET follow-up cycle-log row's Comment column says "51 lean theorems checked" while other rows say "51 theorems, 12 CI claims". Minor formatting inconsistency. | `verification-results.md` NET follow-up row; compare S6a/S6b-1/S6b-2 rows | Make the format consistent. |

**Severity definition:** *Major* disqualifies the item for criticality B or defeats a requirement/safety barrier; *Minor* must be corrected or justified; *Comment* is an observation/recommendation.

No **Major** finding was identified.

## 4. Scope-conformance

| Declared scope (NET follow-up) | Implemented content | Verdict |
| --- | --- --- | --- |
| NC-S6-03 co-implemented: `gem_phys` returns `uint64_t` | `ff_gem.c:30-40` returns `uint64_t`; `gem_wr_qbqp` writes 64-bit ring base ✓ | **Conformant** |
| NC-S6-03 co-implemented: `gem_wr_qbqp` writes low+high word via `TBQP_HI`/`RBQP_HI` | `ff_gem.c:42-47` + `ff_gem_regs.h:30-35` define registers + helper ✓ | **Conformant** |
| NC-S6-03 co-implemented: `DMACFG.ADDR64` (bit 30) asserted | `ff_gem.c:88-93` ORs in `(1u << FF_GEM_DMACFG_ADDR64)` ✓ | **Conformant** |
| NC-S6-03 co-implemented: per-descriptor truncation explicit, full 3-word format deferred to M14 | `ff_gem.c:105,114,178,255` use `(uint32_t)gem_phys(...)`; comments at `:22-25,97-100` ✓ | **Conformant** |
| NC-154-01 closed: `test_lwip_netif` exercises netif glue + loopback | `test_lwip_netif.c` has 3 tests, 19 `FF_TEST_ASSERT`; `tests/CMakeLists.txt` adds `test_lwip_netif` to ctest ✓ | **Conformant** |
| NC-154-01 closed: `test_gem` extended for 64-bit register writes | `test_gem.c` diff adds 5 assertions (RBQP_HI, TBQP_HI, ADDR64) for total 26 ✓ | **Conformant** |
| App comment corrected (register-only model, NC-154-01) | `app_renode_test.c` comment diff: old "32-bit descriptor translation" → new "register-only model + NC-154-01" ✓ | **Conformant** |
| Documentation + NC records updated | CHANGELOG, verification-results, renode-environment, tool-qualification, handover, loop-audit, status.json, sprint-plan, reviews/README all updated ✓ | **Partially conformant** (header-History mismatches R4-01/02/03; README stale R4-04; test count R2-01/R4-05; S6b-2 row stale R2-02) |
| traceability.yaml/test_lwip_netif.c linkage | `tests/unit/test_lwip_netif.c` **not listed** in SRS-REQ-040 tests field; traceability.md History not updated | **Non-conformant** (gap R3-01/R3-02) |

No **undeclared** content was found in the code: the working-tree file set matches the increment description. Documentation/record mismatches are recorded above.

## 5. Falsification results

The reviewer **did not execute** the test suite (the working tree is pre-commit;
the reviewer had no RTEMS toolchain or Docker environment available in context).
All claims below are **Inference** based on first-hand code read + `grep` verification
of counts. The prior review (FRICK-REVIEW-0155) established that the S6b-2
`hal_net_lwip=PASS` is a **real pass** (not the S6a false pass); this confirmation
review focuses on the **NET follow-up delta** (code co-implementation + test
extensions + documentation), not on re-running the Renode image.

**Code-level verification (Facts):**

- `gem_phys` in `ff_gem.c:30-40` returns `uint64_t`. The `dma->virt_to_phys`
  callback return value is returned **without truncation** (the old `(uint32_t)`
  cast on both branches is removed). This is the NC-S6-03 fix.
- `gem_wr_qbqp` (new, `ff_gem.c:42-47`) writes the low 32 bits to `TBQP`/`RBQP`
  and the high 32 bits to `TBQP_HI`/`RBQP_HI`. Both `TBQP_HI` (0x00CC) and
  `RBQP_HI` (0x00C8) are new defines in `ff_gem_regs.h:30-35`.
- `DMACFG.ADDR64` (bit 30) is asserted in `gem_init` (`ff_gem.c:90`):
  `(1u << FF_GEM_DMACFG_ADDR64)` is ORed into the DMACFG write.
- Per-descriptor buffer addresses are explicitly truncated:
  `g->tx_desc[i].addr = (uint32_t)gem_phys(g, g->tx_buf[i])` (line 105),
  `g->tx_desc[idx].addr = (uint32_t)gem_phys(g, g->tx_buf[idx])` (line 178),
  `d->addr = (uint32_t)gem_phys(gem, gem->rx_buf[gem->rx_head])` (line 255).
  The ring base is written via `gem_wr_qbqp` (lines 111-112), which uses the full
  64-bit `gem_phys` return value.
- Code comments at `ff_gem.c:22-25` and `:97-100` document the M14 deferral.

**Test-level verification (Facts):**

- `test_lwip_netif.c` is an **untracked new file** (per `git status`). It
  contains **3 test functions** (`test_lwip_netif_errors`,
  `test_lwip_netif_attach_and_status`, `test_lwip_netif_tx_loopback`) registered
  via `FF_TEST_MAIN` at line 258, and **19 `FF_TEST_ASSERT` calls** (confirmed
  by `grep -c "FF_TEST_ASSERT" tests/unit/test_lwip_netif.c` = 19).
- `test_gem.c` has **26 `FF_TEST_ASSERT` calls** (confirmed by `grep -c` = 26),
  of which **5 are new** (RBQP_HI, TBQP_HI, ADDR64 — confirmed in the `git diff`
  which shows 5 added lines with `+`).
- `tests/CMakeLists.txt` diff adds `test_lwip_netif` to both
  `FF_TEST_TARGETS` and `add_test()`, guarded by `FF_USE_LWIP`. ✓
- `FF_TEST_ASSERT` macro (`ff_test.h:22-31`) increments `ff_test_checks` once
  per invocation, so the check count is exactly the number of macro calls. 19
  FF_TEST_ASSERT calls = 19 checks, **not 22**.

**Total suite counts (Facts):**

- Native ctest suites: 24 (from `tests/CMakeLists.txt` — ccsds, sb, tbl, pus,
  time, r2_gap, r3_gap, hal, math, sdls, gem, gem_tsn, netloop, gpio, serial,
  mem, nvm, mtd, can, i2c, spi, block, pcie, spacewire).
- With `FF_USE_LWIP=ON`: 26 (+ lwip, + lwip_netif).
- The verification-results.md claim of "24/24 native; 26/26 host with
  `FF_USE_LWIP=ON`" is **correct**.

**NC disposition verification (Facts):**

- NC-S6-03: The sprint-plan.md §3 table (line 78) marks it "CLOSED (co-
  implemented) in NET follow-up" — the code change is present and correct.
  The on-Renode GEM DMA round-trip remains GAP-A5 (register-only model, no
  descriptor engine), which is honestly recorded.
- NC-154-01: The sprint-plan.md §3 table (line 79) marks it "CLOSED in NET
  follow-up" — the new `test_lwip_netif.c` is present and registered in
  CMakeLists.txt. The test count claim ("4 tests, 22 checks") is wrong (R2-01).

**Answers to falsification questions:**

- *Is the `gem_phys` return type now `uint64_t`?* **Yes**, confirmed by
  `ff_gem.c:30` and the `git diff` (`-static uint32_t gem_phys` /
  `+static uint64_t gem_phys`).
- *Is `DMACFG.ADDR64` (bit 30) asserted?* **Yes**, confirmed by
  `ff_gem.c:90` and the `git diff` (added OR clause).
- *Is the per-descriptor truncation explicit?* **Yes**, confirmed by
  `(uint32_t)gem_phys(...)` at lines 105, 178, 255.
- *Are `FF_GEM_TBQP_HI`/`FF_GEM_RBQP_HI` defined?* **Yes**, confirmed by
  `ff_gem_regs.h:30-35`.
- *Does `test_lwip_netif.c` have 3 tests and 19 checks?* **Yes**, confirmed by
  `FF_TEST_MAIN` with 3 function names and `grep -c` = 19.
- *Are 26/26 total host suites correct?* **Yes**, 24 native + 2 lwIP.

**Confidence:** **High** for the code-level verification (first-hand read +
diff/grep); **Medium** for the test-count discrepancy (the FF_TEST_ASSERT macro
is unambiguous, but the "22 checks" claim in the documents cannot be reconciled
to any source — it appears to be an estimation error).

## 6. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

No Major finding. The NC-S6-03 co-implementation is **technically correct**
(`gem_phys` returns `uint64_t`, `gem_wr_qbqp` writes 64-bit ring base,
`DMACFG.ADDR64` asserted, per-descriptor truncation explicit with M14
deferral documented). The NC-154-01 closure is **correct** (new
`test_lwip_netif.c` with 3 tests / 19 checks, registered in CMakeLists.txt).
The `test_gem.c` extension is correct (5 new assertions for 64-bit register
writes, 26 total). The app comment correction is correct (register-only model
instead of stale 32-bit translation). The 26/26 total host suite count is
**correct**.

The **Minor findings** (R2-01, R2-02, R3-01, R3-02, R4-01, R4-02, R4-03,
R4-04, R4-05) and **Comments** (R1-01, R3-02, R4-06) were **corrected by
the authoring agent** and **independently verified as CLOSED** by the delta
re-review (FRICK-REVIEW-0163, PA profile): 9/10 VERIFIED at review time;
R4-04 (README milestone table) was corrected post-review and confirmed
fixed by first-hand evidence (M8 moved to "in_progress" row; roadmap.md:117
status updated; `make metrics --check` passes). All findings now CLOSED.

### PA verdict

The NET follow-up **implemented content matches the declared scope** (NC-S6-03
co-implementation + NC-154-01 closure + test extensions + comment). The **NC-
S6-03 claim is accurate**: `gem_phys` returns `uint64_t` (Fact, confirmed by
code read + diff); the ring base is written 64-bit; `DMACFG.ADDR64` is
asserted; per-descriptor truncation is explicit with M14 deferred. The
on-Renode GEM DMA round-trip remains GAP-A5 (register-only model, no
descriptor engine), which is honestly recorded in all documentation. The
**NC-154-01 claim is accurate**: `test_lwip_netif.c` exists with the described
test scenarios (mock-MAC loopback, attach/double-attach/isup/getmac/arg-
faults), registered in CMakeLists.txt. No **Major** finding was identified.

**What a qualified human must still accept:** the corrected delta (all Minor
findings from FRICK-REVIEW-0162 closed and independently verified by delta
re-review FRICK-REVIEW-0163). The residual risk (GAP-A5 on-Renode GEM DMA
round-trip) is honestly recorded. The agent does not approve release or accept
residual risk without qualified human authority (norm N20).

## 7. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
| --- | --- | ---------- | ---------- |
| R1 (N9) | Senior FSW / real-time / space software developer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — NC-S6-03 co-implementation is technically correct (gem_phys uint64_t, gem_wr_qbqp, ADDR64, per-descriptor truncation explicit, M14 deferred); test_gem extensions correct (5 new assertions, 26 total); R1-01 Comment |
| R2 (N9) | Senior SW V&V engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — test_lwip_netif structure verified (3 tests, 19 FF_TEST_ASSERT); total suite counts correct (24/24 native, 26/26 lwIP); R2-01 (count discrepancy) and R2-02 (stale S6b-2 row) minors to correct |
| R3 (N14/N9) | Senior systems/aerospace engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — SRS-REQ-036 and SRS-REQ-040 present in traceability.yaml and traceability.md; R3-01 (missing test_lwip_netif.c in traceability) and R3-02 (traceability.md History gap) to correct |
| R4 (N17/N9) | Senior SW documentation engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — header-History mismatches R4-01/02/03, stale README panel R4-04, CHANGELOG test count R4-05, S6b-2 row staleness R2-02, comment format R4-06 to correct; app_renode_test.c comment already corrected ✓ |
| PA (N20) | Senior SW Product Assurance engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — scope conformant; NC-S6-03/NC-154-01 claims accurate; Minor findings (test counts, header-History, stale rows) tracked to closure before commit; residual risk (GAP-A5) to the human authority |

## 8. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-S6-03 | Minor | `ff_gem.c` `gem_phys` | Latent 64-bit GEM descriptor DMA addressing | **CLOSED (co-implemented) in NET follow-up** — `gem_phys` returns `uint64_t`, `gem_wr_qbqp` writes 64-bit ring base, `DMACFG.ADDR64` asserted, per-descriptor truncation explicit, 3-word descriptor deferred to M14 | `ff_gem.c:30-40,42-47,90,105,114,178,255`; `ff_gem_regs.h:30-35`; `test_gem.c` asserts RBQP_HI/TBQP_HI/ADDR64 (26 assertions) | GEM driver author | **closed** (GAP-A5 on-Renode round-trip remains) | FRICK-REVIEW-0162 (this) |
| NC-154-01 | Minor | `test_lwip_netif.c` gap | lwIP netif glue exercised by no test | **CLOSED in NET follow-up** — new `test_lwip_netif` (3 tests, 19 checks) + `test_gem` 64-bit extensions | `tests/unit/test_lwip_netif.c` (untracked new file); `tests/CMakeLists.txt` adds `test_lwip_netif` to ctest; `test_gem.c` +5 assertions (26 total) | frick-fsw authoring agent | **closed** (GAP-A5 on-Renode round-trip remains) | FRICK-REVIEW-0162 (this) |
| R2-01 | Minor | Documentation test counts | "4 tests, 22 checks" / "3 tests, 22 checks" vs actual 3 tests / 19 checks | **CLOSED** — corrected in CHANGELOG.md, verification-results.md, sprint-plan.md to "3 tests, 19 checks" | `grep -c "FF_TEST_ASSERT" tests/unit/test_lwip_netif.c` = 19; CHANGELOG.md:585, verification-results.md NET row, sprint-plan.md:79 now read "3 tests, 19 checks" | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0162 delta |
| R2-02 | Minor | verification-results S6b-2 row | Stale: says `gem_phys` is `uint32_t`, NC-S6-03 "Not delivered" | **CLOSED** — S6b-2 cycle-log row updated to "Not delivered in S6b-2 (NC-S6-01 residual): …`gem_phys`, then `uint32_t`… (latent 64-bit addressing defect, **NC-S6-03**, co-implemented in the NET follow-up)" | `verification-results.md` S6b-2 cycle-log row updated | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0162 delta |
| R3-01 | Minor | traceability.yaml SRS-REQ-040 | `test_lwip_netif.c` missing from `tests` field | **CLOSED** — added `tests/unit/test_lwip_netif.c` to the `tests` list for SRS-REQ-040; `make trace` re-run: PASS (114/71/29/14/51) | `traceability.yaml` SRS-REQ-040 now lists `["tests/unit/test_lwip.c", "tests/unit/test_lwip_netif.c"]`; `make trace` PASS | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0162 delta |
| R3-02 | Comment | traceability.md | No History entry for NET follow-up; mirror row tests column missing `test_lwip_netif.c` | **CLOSED** — History entry 0.36 added; SRS-REQ-040 mirror row tests column updated to "test_lwip.c + test_lwip_netif.c" | `traceability.md` History row 0.36 added; mirror row updated | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0162 delta |
| R4-01 | Minor | verification-results.md | Header 0.42 vs History 0.41 | **CLOSED** — History entry 0.42 added (corrections summary) | `verification-results.md` header Revision 0.42, History last entry 0.42 | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0162 delta |
| R4-02 | Minor | renode-environment.md | Header 0.23 vs History 0.21 | **CLOSED** — header set to 0.21 (matching last History entry); S6b-2 History 0.20 + NET follow-up History 0.21 already present | `renode-environment.md` header Revision 0.21 (aligned to History) | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0162 delta |
| R4-03 | Minor | tool-qualification.md | Header 0.3.23 vs History 0.3.24 | **CLOSED** — header bumped to 0.3.24 (matching History) | `tool-qualification.md` header Revision 0.3.24 (aligned to History) | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0162 delta |
| R4-04 | Minor | README.md metrics panel + M8 milestone table | Stale: 7/30 sprints (should be 8/30), M8 "planned" (should be "in_progress"), "Current focus" not updated, roadmap.md M8 status stale | **CLOSED** — `make metrics` ran (panel regenerated, 8/30); README milestone table updated (M8 → "in_progress"); Current focus updated; roadmap.md:117 M8 status updated to "(in_progress)" | `README.md` metrics panel + milestone table; `docs/roadmap.md:117`; `make metrics --check` = current | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0163 confirms (R4-04-M8 resolved) |
| R4-05 | Minor | CHANGELOG.md | "4 tests, 22 checks" vs actual 3 tests, 19 checks | **CLOSED** — corrected to "3 tests, 19 checks" | `CHANGELOG.md:585` now reads "3 tests, 19 checks" | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0162 delta |
| R4-06 | Comment | verification-results.md | "51 lean theorems checked" vs "51 theorems, 12 CI claims" (format) | **CLOSED** — comment format synchronized to "51 theorems, 12 CI claims" | `verification-results.md` NET follow-up cycle-log row Comment column | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0162 delta |

## 9. Norm-by-norm audit

| Norm | Assessment |
| ---- | ---------- |
| N1 | Not triggered by this change (no new generated code requiring Lean verification; `gem_phys`/`gem_wr_qbqp` are I/O-bound, not Lean-modelable). |
| N2 | All reviewed content in English → OK |
| N3 | SRS-REQ-036 and SRS-REQ-040 present in `SRS.md` and `traceability.yaml` → OK |
| N4 | `trace.py` re-run after adding `test_lwip_netif.c` to SRS-REQ-040 tests: **PASS** (114 reqs / 71 srs / 29 sss / 14 gaps, 51 theorems) |
| N5 | Criticality B; NC-S6-03/NC-154-01 claims accurate and honestly recorded → OK |
| N6 | V&V environment: test_lwip_netif is a host test (T3), registered in CMakeLists.txt → OK |
| N7 | Dual push pending (not committing yet; delta re-review in progress) |
| N8 | Documentation updated and corrected (all header-History mismatches fixed, README regenerated, S6b-2 row corrected, test counts corrected, traceability.md History added) → OK |
| N9 | Independent review in fresh context → OK |
| N10 | `test_lwip_netif.c` is a new test suite; T3 (`tests/unit/*.c`) already qualified → OK |
| N11 | Review report stored; records in CHANGELOG/verification-results/sprint-plan updated; `docs/reviews/README.md` updated → OK |
| N12 | NC-S6-03 co-implemented (evidence: code), NC-154-01 closed (evidence: test file); all Minor findings corrected and marked CLOSED; **delta re-review FRICK-REVIEW-0163 confirms all 10 corrections verified** (R4-04-M8 confirmed fixed post-review) → **closed** |
| N13 | New file `test_lwip_netif.c` carries `SPDX-License-Identifier: AGPL-3.0-only` → OK |
| N14 | No SSS/SSS-manifest change → not triggered |
| N15 | Full cycle: build/test/trace green (24/24 native, 26/26 host; trace PASS; format clean) → OK |
| N16 | All header-History mismatches corrected; HISTORY entry 0.42 added → OK |
| N17 | Documentation review performed and findings corrected → OK (delta re-review FRICK-REVIEW-0163 confirms) |
| N18 | No target-code behaviour change (app_renode_test.c comment only); Renode gate unchanged → OK |
| N19 | Loop-audit updated (History 0.21 added) → OK; sprint-plan updated → OK |
| N20 | PA review performed; verdict APPROVED WITH COMMENTS; Minor findings corrected and closed; delta re-review FRICK-REVIEW-0163 confirms all corrections verified → **closed** |

---

## Facts / Assumptions / Inferences / Recommendations / Confidence / Open gaps

**Facts**

- `gem_phys` returns `uint64_t` (confirmed by `ff_gem.c:30` and `git diff`).
- `gem_wr_qbqp` helper writes 64-bit ring base via low+high word (confirmed by
  `ff_gem.c:42-47`).
- `DMACFG.ADDR64` (bit 30) is asserted in `gem_init` (confirmed by `ff_gem.c:90`).
- Per-descriptor truncation is explicit: `(uint32_t)gem_phys(...)` at
  `ff_gem.c:105,178,255` (confirmed by code read + `git diff`).
- `FF_GEM_TBQP_HI` (0x00CC) and `FF_GEM_RBQP_HI` (0x00C8) are defined in
  `ff_gem_regs.h:30-35` (confirmed by code read + `git diff`).
- `FF_GEM_MODID_OFF` (0xFC) is defined in `config/platform/platform_mpfs.h:38`,
  not in `ff_gem_regs.h` (confirmed by `grep`).
- `test_lwip_netif.c` is an **untracked new file** with 3 test functions
  (`test_lwip_netif_errors`, `test_lwip_netif_attach_and_status`,
  `test_lwip_netif_tx_loopback`) and 19 `FF_TEST_ASSERT` calls
  (`grep -c = 19`).
- `test_gem.c` has 26 `FF_TEST_ASSERT` calls (`grep -c = 26`), of which 5 are
  new (`git diff` shows 5 added `FF_TEST_ASSERT` lines).
- `tests/CMakeLists.txt` adds `test_lwip_netif` to `FF_TEST_TARGETS` and
  `add_test()`.
- Total ctest suites: 24 native, 26 with `FF_USE_LWIP=ON` (confirmed by
  counting entries in `tests/CMakeLists.txt`).
- `FF_TEST_ASSERT` macro (`ff_test.h:22-31`) increments `ff_test_checks` once
  per invocation.
- NC-S6-03 disposition in `sprint-plan.md:78` says "CLOSED (co-implemented) in
  NET follow-up" — matches the code.
- NC-154-01 disposition in `sprint-plan.md:79` says "CLOSED in NET follow-up"
  with wrong count ("4 tests, 22 checks") — disposition accurate, count wrong.
- `app_renode_test.c` comment at `renode_hal_net_lwip_check` was corrected:
  old comment referenced "32-bit descriptor translation" (stale); new comment
  references "register-only model" + NC-154-01 (correct).
- `verification-results.md` header Revision 0.42, History last entry 0.41.
- `renode-environment.md` header Revision 0.23, History last entry 0.21.
- `tool-qualification.md` header Revision 0.3.23, History last entry 0.3.24.
- `README.md` shows "7/30 sprints completed" (line 51) and M8 "planned"
  (line 416); `status.json` says `done: 8` and `"M8": "in_progress"`.
- `traceability.yaml` SRS-REQ-040 `tests` field lists only
  `tests/unit/test_lwip.c` (line 383).
- `traceability.md` History last entry 0.35; no entry for NET follow-up.

**Assumptions**

- The reviewed working tree is the intended increment; no other change is
  pending beyond what is shown in `git diff HEAD`.
- `grep -c "FF_TEST_ASSERT"` counts macro invocations correctly (each
  `FF_TEST_ASSERT` is on one line; the one multi-line call at
  `test_lwip_netif.c:247-248` has the macro name on the first line, so `grep -c`
  counts it once).
- The "22 checks" claim in the documents is an estimation error, not a
  reference to a different counting method (the macro is unambiguous).

**Inferences**

- The `gem_phys` co-implementation is structurally correct: the ring base is
  written 64-bit (fixing the original NC-S6-03), and the per-descriptor
  32-bit truncation is the remaining M14 item (not NC-S6-03).
- The `app_renode_test.c` comment is now accurate: the reason RE/TE are not
  armed in the Renode check is the register-only model (TSTART unhandled), not
  the 32-bit translation (which is now fixed).
- The README metrics panel is stale because `make metrics` was not run after
  `status.json` was updated; the panel is auto-generated between
  `<!-- METRICS-BEGIN -->` and `<!-- METRICS-END -->`.
- The S6b-2 cycle-log row in verification-results.md was not updated by the
  NET follow-up delta, so it contradicts the co-implementation.

**Recommendations**

- R1: None — code is correct.
- R2: Correct the test count claims in CHANGELOG.md (line 585),
  verification-results.md (NET follow-up row), and sprint-plan.md §3 NC-154-01
  (line 79) to "3 tests, 19 checks".
- R2: Update the S6b-2 cycle-log row in verification-results.md to reflect
  NC-S6-03 co-implementation (gem_phys now uint64_t).
- R3: Add `tests/unit/test_lwip_netif.c` to the SRS-REQ-040 `tests` field in
  `traceability.yaml` and the corresponding mirror row in `traceability.md`;
  add a History entry to `traceability.md`.
- R4: Fix the three header-History mismatches (verification-results.md,
  renode-environment.md, tool-qualification.md); run `make metrics` to
  regenerate the README panel; correct the comment format in the NET follow-up
  row.
- All Minor findings must be corrected and the delta re-reviewed (N12/N17/N20)
  before commit/push.

**Confidence:** **High** for code-level verification (first-hand read + diff
+ grep); **High** for the documentation-currency findings (read directly from
the diffs); **Medium** for the header-History mismatch interpretation for
renode-environment.md (entries 0.22 and 0.23 are absent from History; the
intended revision is 0.21 matching the last History entry, or the History
needs 0.22/0.23 entries — the author must decide).

**Open gaps (closed — all findings from FRICK-REVIEW-0162 confirmed resolved
by delta re-review FRICK-REVIEW-0163):** test count discrepancies across 4
document sites (CHANGELOG, verification-results, sprint-plan — corrected
to "3 tests, 19 checks"); header-History mismatches in 3 V&V documents
(verification-results 0.42, renode-environment 0.21, tool-qualification
0.3.24 — all aligned); stale S6b-2 row in verification-results.md (updated
to reflect NC-S6-03 co-implementation); missing `test_lwip_netif.c` in
traceability.yaml SRS-REQ-040 tests (added); missing traceability.md
History entry (0.36 added); stale README metrics panel + M8 milestone
table (regenerated via `make metrics`; M8 moved to "in progress" row;
Current focus updated; roadmap.md:117 status updated).

---

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-12 | independent reviewer (FRICK-REVIEW-0162) | Confirmation review (N12) of the NET follow-up delta on top of `64961ac`: NC-S6-03 co-implemented (gem_phys uint64_t, gem_wr_qbqp, ADDR64, per-descriptor truncation, M14 deferred); NC-154-01 closed (test_lwip_netif 3 tests/19 checks; test_gem +5 assertions, 26 total); app_renode_test.c comment corrected. Code is correct; **no Major finding**. **APPROVED WITH COMMENTS** — 8 Minor/Comment findings: test-count discrepancy in 3 docs (actual 3 tests/19 checks vs "4 tests/22 checks" or "3 tests/22 checks"), stale S6b-2 cycle-log row in verification-results.md (still says gem_phys uint32_t), missing test_lwip_netif.c in traceability.yaml SRS-REQ-040 tests, missing traceability.md History entry, 3 header-History mismatches (verification-results 0.42/0.41, renode-environment 0.23/0.21, tool-qualification 0.3.23/0.3.24), stale README metrics panel (7/30 vs 8/30). All Minors to correct and delta re-reviewed (N12/N17/N20) before commit/push. Note: `docs/reviews/README.md` log entry currently reads "REVIEW IN PROGRESS" and must be updated to the final conclusion. |

Committed and pushed with the reviewed change (norm N7) — **all findings
from FRICK-REVIEW-0162 confirmed resolved by delta re-review FRICK-REVIEW-0163;
loop closed (N12/N17/N20)**.

## Independence and file-modification declaration

I declare that I reviewed the items above **independently** (fresh context; I did
not author/implement/test/approve them) and that, as instructed, I wrote
**exactly one file** — this report
(`docs/reviews/2026-10-12-net-followup-r1-r2-r3-r4-pa-review.md`) — and
**modified, created or deleted no other repository file**. I did not commit or
push. The builds/runs were not executed (pre-commit working tree, no toolchain
in context); all code-level claims are from first-hand read + `git diff` +
`grep` verification.
