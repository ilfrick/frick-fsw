# Milestone M4 increment-1 (Cadence GEM Ethernet driver) — delta re-review — profiles R1/R2/R3/R4

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0027                      |
| Reviewer session                  | independent subagent, fresh context (norm N9) |
| Reviewer profiles                 | R1 (design/code), R2 (V&V), R3 (systems, N14), R4 (documentation, N17) |
| Reference revision                | branch `main`, uncommitted M4 working tree on base commit `b4a1e5f` |
| Items under review                | NC dispositions in `docs/reviews/2026-10-10-m4-gem-r1-r2-review.md` (FRICK-REVIEW-0025: R1-01..R1-12, R2-01..R2-06) and `docs/reviews/2026-10-10-m4-gem-r3-r4-review.md` (FRICK-REVIEW-0026: R3-1..R3-4, R4-1..R4-5); fix artifacts `src/drivers/eth/gem/ff_gem.{c,h}`, `src/drivers/eth/gem/ff_gem_regs.h`, `tests/unit/test_gem.c`, `CMakeLists.txt`, `tests/CMakeLists.txt`, `src/hal/hal_eth.h`, `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md`, `README.md`, `CHANGELOG.md`, `docs/requirements/{SRS,SSS,traceability}.*`, `docs/architecture/{hardware-abstraction,platform}.md`, `docs/roadmap.md`, `AGENTS.md` |
| Date                              | 2026-10-10                             |
| Scope                             | Re-apply all norms to the affected M4 increment-1 scope (norm N12) and verify every recorded NC disposition against the working tree; reproduce the trace/build/ctest/ASan gates, re-check the GEM register/bitfield and descriptor layout against the authoritative Cadence GEM reference (Linux `macb.h`/`macb_main.c`), and re-derive the V&V counts and documentation records |
| Independence declaration          | I am a freshly spawned independent reviewer with no prior context. I took no part in authoring, fixing, building, executing or previously reviewing any item under review; every claim below derives from the repository contents and reproduced commands. I modified no file except this report. |
| Norms applied                     | N1–N17 (see `docs/rules.md`), emphasis N8, N9, N10, N11, N12, N14, N16, N17 |

## 1. Scope

Delta re-review of the milestone-M4 increment-1 Cadence GEM Ethernet driver NCs
of the R1/R2 review (FRICK-REVIEW-0025, **REJECTED**: R1-01..R1-04 and R2-01
Major) and the R3/R4 review (FRICK-REVIEW-0026, **R3 REJECTED**: R3-1 Major; R4
**APPROVED WITH COMMENTS**: R4-1/R4-2 Minor) against the current uncommitted
working tree on base commit `b4a1e5f`. Every recorded fix was located in the
tree and checked against the authoritative Cadence GEM layout as used by the
Linux `macb` driver (`drivers/net/ethernet/cadence/macb.h` and `macb_main.c`,
the binding behind `microchip,mpfs-macb`), not against the driver's own
assumptions. The traceability, Debug build, ctest, ASan/UBSan and coverage gates
were reproduced independently.

**Included:** the GEM register/descriptor logic, its host mock-MMIO test, the
`FF_EthMac_t` contract change, the build wiring, the M4 requirements
(SSS-REQ-020, SRS-REQ-036) and the documentation records touched by the delta.
**Excluded:** on-target PHY/link/board bring-up (documented class-A gap GAP-A5)
is not itself a finding; the lwIP `FF_NetStack_t` and N7/ESA `FF_Pus_Backend_t`
adapters are not delivered in this increment. No repository file was modified
except this report; build artifacts were produced under `build/d4*` (ignored).

### Verification evidence executed (independent re-run)

| Command | Result |
| ------- | ------ |
| `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 71 requirements (srs=37, sss=20, gap=14), 51 Lean theorems, 9 CI claims (`EXIT=0`) |
| `cmake -S . -B build/d4 -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build build/d4` | **PASS**, no warnings |
| `ctest --test-dir build/d4 --output-on-failure` | **PASS** 9/9 (`ccsds, sb, tbl, pus, time, hal, math, sdls, gem`) |
| per-suite check counts | 15+20+9+29+101+31+20+81+**23** = **329**; `gem` = **23** (`tests/unit/test_gem.c checks=23 failures=0`) |
| ASan+UBSan build (`-fsanitize=address,undefined`) + ctest | **PASS** 9/9, clean |
| `python3 tools/scripts/coverage.py build/d4cov` (fresh `FF_ENABLE_COVERAGE=ON`) | **1434/2239 lines (64.0%)**, functions **193/237** (gcc/gcov 15.2.0) — the recorded README panel is 1437/2239 / 194/237 |
| Register/bitfield cross-check against `macb.h` (fetched from torvalds/linux master) | all used offsets/bits confirmed; **one gap** (TX WRAP, see NC-07) |

## 2. References

- Project norms: `docs/rules.md` (N1–N17); profiles: `docs/engineering/independent-review.md`.
- Traceability manifest/gate: `docs/requirements/traceability.yaml`, `tools/scripts/trace.py`.
- V&V records: `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md`, `build/coverage/coverage.json`.
- External, authoritative: Linux `drivers/net/ethernet/cadence/macb.h` and
  `macb_main.c` (Cadence GEM register/bitfield/descriptor definitions, used by
  the `microchip,mpfs-macb` binding); Microchip PolarFire SoC MSS TRM
  (DS60001702) §3.1.2.5.
- Predecessor reviews: FRICK-REVIEW-0025, FRICK-REVIEW-0026.

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| D-1 | **Minor** | `docs/vv/verification-results.md`; `CHANGELOG.md` (norms N8/N11/N16) | **The M4 verification records were not updated with the rewritten driver and are now internally inconsistent.** `verification-results.md` M4 row and notes still record `gem 18` checks / 324 total and coverage `1421/2219 (64.0%)`, functions `192/236`; `CHANGELOG.md` still says "`test_gem`, 18 checks". The delivered test has 23 checks (total 329) and the regenerated README panel records `1437/2239`, functions `194/237`; `tool-qualification.md` correctly says 23. | `docs/vv/verification-results.md:27,254,261,317`; `CHANGELOG.md:76`; `README.md:76`; `docs/vv/tool-qualification.md` rev 0.1.8; reproduced `gem 23`, total 329 | Update the M4 row/notes and CHANGELOG to `gem 23`/329 and the regenerated coverage; keep one authoritative metric |
| D-2 | **Minor** | SRS-REQ-036; verification method `Inp` (norms N10/N12) | **Residual verification gaps after the fixes (carried from R2-02):** the cache-maintenance path is never exercised (no mock `FF_Dma_t` is passed, so `cfg.dma == NULL` in every test and `cache_flush`/`cache_invalidate` are not called), and there is no test for TX ring wrap/multi-frame, TX timeout/recovery, TX ring-full (`FF_NOT_READY`), NULL arguments, `set_link_up`, or the 100 Mbps `SPD` path. No dedicated `Inp` inspection record accompanies SRS-REQ-036's `Inp` method (only a TRM pointer in the notes). | `tests/unit/test_gem.c` (23 checks, 3 functions); `src/drivers/eth/gem/ff_gem.c:38-50`; traceability `SRS-REQ-036 method: Tst | Inp` | Add a mock `FF_Dma_t` recording flush/invalidate; add the negative/boundary tests; record the `Inp` evidence. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

**No Major finding was raised at any profile in this delta.** All five
predecessor Major defects (R1-01..R1-04, R2-01 / R3-1) are fixed in the tree and
verified against the authoritative Cadence GEM reference; the remaining items
are the Minors above, base Minors with residuals (NC-07, NC-08, NC-10, NC-11,
NC-14) and non-derogatory Comments.

**Register/descriptor conformance verified (no finding):**

- **NCR**: no software-reset bit exists; `NCR` bit 15 is `SRTSM` and `macb.h`
  defines no reset. The driver defines `LLB=1, RE=2, TE=3, MPE=4, CLRSTAT=5,
  BP=8, TSTART=9, THALT=10` (`ff_gem_regs.h:36-43`) and `gem_init` sequences
  `CLRSTAT` strobe (`ff_gem.c:63`) then `RE|TE|MPE` (`:103`); TX start uses
  `TSTART` (`:163`). Matches `macb.h` exactly.
- **NCFGR**: `SPD=0`, `FD=1` (`ff_gem_regs.h:46-47`); gigabit is `GBE=10`
  (`:49`); `gem_init` sets `FD`, `BIG`, `DRFCSDBW` and adds `GBE` for 1000 Mbps
  or `SPD` for 100 Mbps (`ff_gem.c:65-75`). No speed/duplex is written to NCR.
  Matches `GEM_GBE_OFFSET=10`, `MACB_SPD_OFFSET=0`, `MACB_FD_OFFSET=1`.
- **DMACFG**: `RXBS` at bits 16..23 (`ff_gem_regs.h:55`), value
  `(MTU+63)/64 = 24` (`ff_gem.c:17-18,78`). Matches `GEM_RXBS_OFFSET=16`.
- **RX path**: `FF_GemPollRx` scans `rx_desc[rx_head]` for the address-word
  `USED` bit, takes the length from the descriptor control word
  (`& 0xFFF`), invalidates the descriptor and buffer, dispatches, re-arms with
  the `WRAP` bit on the last descriptor and advances `rx_head`
  (`ff_gem.c:203-237`); the ring is armed in `init` (`:92-98`). This matches
  `MACB_RX_USED_OFFSET=0`, `MACB_RX_WRAP_OFFSET=1`, `MACB_RX_FRMLEN_MASK=0xFFF`
  and `macb_main.c`'s "Setting addr clears RX_USED and allows reception" model.
- **TSR**: `COMP=5` (`ff_gem_regs.h:63`) matches `MACB_COMP_OFFSET=5`.
- **Cache maintenance**: `gem_flush`/`gem_inval` call the injected
  `FF_Dma_t` around the TX buffer+descriptor before the `TSTART` kick and after
  completion, and around the RX descriptor+buffer (`ff_gem.c:38-50,157,161,174,
  214,222,232`) — the R1-04 defect is fixed in code (but see D-2 on testing).
- **Traceability**: `trace.py` PASS, 71 reqs (37/20/14), 51 theorems, 9 CI
  claims; `SSS-REQ-020 <- 1 srs child`; SRS-REQ-036 reworded so TSN is
  explicitly deferred to a separate planned increment (`SRS.md:174-179`).

## 4. Conclusion (per profile)

### R1 — Design/code (senior real-time/space SW developer)

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

The four R1 Majors are closed by correct, reference-conformant code: R1-01
(no reset bit; `CLRSTAT` strobe only), R1-02 (`GBE`/`SPD`/`FD` in NCFGR, no
speed/duplex in NCR), R1-03 (descriptor-based RX with `USED` + descriptor
length) and R1-04 (DMA flush/invalidate around buffers/descriptors). Minors
remain: **NC-07 (TX `WRAP` never defined/set)** — only the RX ring wraps, so a
correctly-sized TX ring cannot wrap (`macb_main.c` sets `MACB_BIT(TX_WRAP)` on
the last TX descriptor and preserves it); **NC-08 (TX timeout still wedges the
ring)** — `ff_gem.c:169-172` returns `FF_ERROR` before restoring `USED`/advancing
`tx_head`; **NC-10 (partial validation)** — `FF_GemSetup` still returns
`FF_SUCCESS` for a NULL bus. None is a Major.

### R2 — Software V&V (senior SW V&V engineer)

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

R2-01's Major is closed: the mock no longer encodes the driver's own (wrong)
hardware model — it models documented GEM behaviour (`NCR TSTART → TSR COMP`,
RX frame length and `USED` in the descriptor) and the assertions now cover the
fixed items (`GBE`, `FD`) and the descriptor re-arm, so the previously live
Majors would now be caught. The gates reproduce (trace 71 reqs; 9/9 ctest;
`gem` 23; ASan clean). Residual Minors: **NC-14** (the recorded coverage claim
is still not exactly reproducible — README 1437/2239 vs my 1434/2239 with
gcc/gcov 15.2.0 — and the toolchain/version is still unstated) and **D-2**
(coverage gaps, no mock DMA, no `Inp` record). Also **D-1** (stale V&V records).

### R3 — Senior Systems Engineer (N14)

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

R3-1's Major is closed: `SSS-REQ-020`/`SRS-REQ-036` are now served by a GEM MAC
that programs gigabit (`GBE`), speed, duplex and the DMA RX buffer size per the
MPFS `macb` layout and consumes the RX descriptor ring; the system-facing TSN
correction remains verified. Comments **R3-2** ("separable" is only the
`FF_BUILD_DRIVERS` compile toggle; `ff_gem.c` is still linked into the monolithic
`frick_fsw_core`, `CMakeLists.txt:97-100`) and **R3-4** (`hardware-abstraction.md`
§7 and the `SRS-REQ-026` note still say the backend implementations "are M4")
remain open and non-blocking. No Major.

### R4 — Senior SW documentation reviewer (N17)

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

R4-1 is closed (`hardware-abstraction.md` rev 0.2, `platform.md` rev 0.2,
`roadmap.md` rev 0.7, each with an M4 `History` row); R4-2 is closed (README
driver table now lists "Ethernet (GEM)" and the "no drivers" paragraph is
corrected); R4-3 is addressed (AGENTS.md and the README layout list
`drivers`/`hal`/`math`/`security`). New Minor **D-1** (stale M4 check counts and
coverage in `verification-results.md`/`CHANGELOG.md`). Comments **R4-4**
(`driver-development.md` still documents three interfaces, no HAL/`FF_EthMac_t`
GEM example, History not bumped) and **R4-5** (lwIP still pinned by the moving
tag `STABLE-2_2_1_RELEASE`, fetch script verifies no revision, licence stated as
`GPL-3.0` rather than an SPDX `-only`/`-or-later` expression) remain open and
non-blocking. **0 duplicate `Document id` declarations**; `FRICK-REVIEW-0027` is
free and used here.

**Overall for the M4 increment-1 delta: the R1/R2/R3 rejections are lifted** —
no Major finding remains, and every predecessor Major is verified fixed against
the authoritative Cadence GEM reference. Closure is blocked only on Minor
findings (NC-07, NC-08, NC-10, NC-11, NC-14, D-1, D-2) and the non-blocking
Comments (R3-2, R3-4, R4-4, R4-5), which norm N12 requires to be dispositioned
before the M4 records are final. Because the records are still uncommitted, the
commit/push step (norm N11) must also log both predecessor reports and this
delta report in `docs/reviews/README.md` and reconcile the M4 rows/wording in
`verification-results.md`.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent subagent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS (NC-07, NC-08, NC-10 Minor) |
| R2 (N9)       | independent subagent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS (NC-14, D-2 Minor; D-1) |
| R3 (N9/N14)   | independent subagent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS (R3-1 closed; R3-2/R3-4 Comments) |
| R4 (N9/N17)   | independent subagent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS (D-1 Minor; R4-4/R4-5 Comments) |

## 6. Follow-up records

Norm N12: NCs are resolved by re-applying ALL rules to the affected scope and
re-reviewing the deltas until full compliance. This report must be committed with
the reviewed change and a row added to `docs/reviews/README.md` (norm N11).

### 6.1 R1/R2 base NCs (FRICK-REVIEW-0025)

| NC | Ref | Severity | Item / Requirement | Delta verdict | Evidence |
| -- | --- | -------- | ------------------ | ------------- | -------- |
| M4-NC-01 | R1-01 | Major | Reset written to a non-existent NCR bit; `init` fails on target | **CLOSED** | No reset bit defined (`ff_gem_regs.h:35-43`); `gem_init` writes only the `CLRSTAT` strobe (`ff_gem.c:63`) and no reset poll; `macb.h` bit 15 = `SRTSM`; the mock no longer special-cases NCR. |
| M4-NC-02 | R1-02 | Major | Gigabit/speed/duplex in wrong registers/bits | **CLOSED** | `GBE=10`, `FD=1`, `SPD=0` (`ff_gem_regs.h:46-49`); NCFGR built accordingly, no speed/duplex in NCR (`ff_gem.c:65-75`); test asserts `GBE`+`FD` (`test_gem.c:84-85`). |
| M4-NC-03 | R1-03 | Major | RX not descriptor-based; length from fake RSR field | **CLOSED** | `FF_GemPollRx` polls descriptor `USED`, length `= ctrl & 0xFFF`, re-arms with `WRAP`, advances head (`ff_gem.c:203-237`); ring armed in `init` (`:92-98`); test driven by descriptor USED+length (`test_gem.c:136-142`). |
| M4-NC-04 | R1-04 | Major | No DMA cache maintenance | **CLOSED (code)** | `gem_flush`/`gem_inval` around TX buf+desc before the `TSTART` kick and after completion, and around RX desc+buf (`ff_gem.c:38-50,157,161,174,214,222,232`). Residual: not exercised by a mock DMA (D-2). |
| M4-NC-05 | R2-01 | Major | Suite passes with live defects; mock is not an independent oracle | **CLOSED (residual D-2)** | Mock now models documented behaviour (`TSTART→TSR COMP`, `test_gem.c:41-44`; RX descriptor `USED`+length) rather than the driver internals; assertions cover `GBE`/`FD`/`SA1B`/`SA1T`/`RBQP`/`TBQP`/re-arm. Residual coverage gaps in D-2. |
| M4-NC-06 | R1-05 | Minor | `DMACFG` RX buffer value in wrong field / misnamed | **CLOSED** | `RXBS=16` (`ff_gem_regs.h:55`); value `(MTU+63)/64 = 24` written at bits 16..23 (`ff_gem.c:17-18,78`); matches `GEM_RXBS_OFFSET=16`. |
| M4-NC-07 | R1-06 | Minor | Descriptor `WRAP` bits undefined/unset | **NOT CLOSED** | RX `WRAP` is set (`ff_gem.c:95,230`), but there is **no `FF_GEM_TX_WRAP`** (grep: none) and the last TX descriptor never gets it (`:88-91,160`), so the TX ring cannot wrap; `macb_main.c` sets `MACB_BIT(TX_WRAP)` on the last TX descriptor and preserves it. |
| M4-NC-08 | R1-07 | Minor | TX timeout permanently wedges the ring | **NOT CLOSED** | On `TSR.COMP` timeout `ff_gem.c:169-172` increments `errors` and returns `FF_ERROR` before `ctrl |= USED`/`tx_head++`; the descriptor stays DMA-owned → the same index returns `FF_NOT_READY` forever; no recovery path. |
| M4-NC-09 | R1-08 | Minor | `FF_GemBaseAddress` strong/unused vs header claim | **CLOSED** | Symbol removed (grep: none in `src/drivers`); header comment corrected (`ff_gem.h:78-80`). |
| M4-NC-10 | R1-09 | Minor | No bus/`self` validation outside `init` | **PARTIAL** | `FF_GemSetup` validates `gem`/`cfg` (`ff_gem.c:241`), `init` validates bus (`:58`), `get_mac_address` validates `out` (`:116`), `PollRx` guards NULL (`:207`); but `FF_GemSetup` still returns `FF_SUCCESS` for a NULL bus and `transmit`/`set_link_up`/`get_stats` still do not guard `self`. |
| M4-NC-11 | R2-02 | Minor | Thin coverage; no `Inp` record | **PARTIAL (D-2)** | Added: zero-length `FF_BAD_SIZE` (`test_gem.c:112`), empty `PollRx` (`:134`), descriptor re-arm (`:142`); still missing: DMA path, TX wrap/multi-frame, timeout/recovery, ring-full, NULL args, `set_link_up`, 100 Mbps `SPD`; no dedicated `Inp` record. |
| M4-NC-12 | R2-03 | Minor | TSN clause bundled without delivered evidence | **CLOSED** | `SRS-REQ-036` reworded: TSN is delivered through `FF_TsnBackend_t` "in a separate planned increment" (`SRS.md:174-179`); manifest `lean_note` consistent. |
| M4-NC-13 | R2-04 | Minor | `test_gem.c` not registered/re-qualified | **CLOSED** | `tool-qualification.md` rev **0.1.8**; M4 `History` row records `tests/unit/test_gem.c` (23 checks, mock MMIO). |
| M4-NC-14 | R2-05 | Minor | Coverage claim not exactly reproducible; toolchain unstated | **NOT CLOSED** | README/coverage.json 1437/2239 / 194/237; my fresh gcov 15.2.0 run gives 1434/2239 / 193/237; `verification-results.md` still records 1421/2219 / 192/236 (stale, D-1); no compiler/gcov version recorded. |
| M4-NC-15 | R1-10..R1-12, R2-06, C-1..C-4 | Comment | 32-bit truncation, 12.5 KB stack object, register naming, records vs review state, `lean_note` wording, licence precision, fetch hardening | **OPEN (non-blocking)** | Carried as observations; no required change in this increment (see also R4-5). |

### 6.2 R3/R4 base NCs (FRICK-REVIEW-0026)

| NC | Severity | Item | Delta verdict | Evidence |
| -- | -------- | ---- | ------------- | -------- |
| R3-1 | Major | GEM register/descriptor programming incorrect; `Tst\|Inp` unsubstantiated | **CLOSED** | Same code evidence as NC-01..NC-06; register/bitfield/descriptor layout now matches `macb.h`; residual defect is Minor NC-07 (TX `WRAP`). |
| R3-2 | Comment | "separable" only a compile toggle; driver linked into monolithic core | **OPEN (non-blocking)** | `CMakeLists.txt:97-100` still appends `ff_gem.c` to `FF_CORE_SOURCES`; no per-driver target. |
| R3-3 | Comment | TSN clause bundled without evidence | **CLOSED** | Same as NC-12 (`SRS.md:174-179`). |
| R3-4 | Comment | Stale `hardware-abstraction.md` §7 / `SRS-REQ-026` note | **OPEN (non-blocking)** | `hardware-abstraction.md:115-116` still says implementations "are M4"; the `SRS-REQ-026` note is unchanged. |
| R4-1 | Minor | M4 edits without revision bump / `History` | **CLOSED** | `hardware-abstraction.md` rev 0.2 + History; `platform.md` rev 0.2 + History; `roadmap.md` rev 0.7 + History. |
| R4-2 | Minor | README driver inventory contradicts the delivered driver | **CLOSED** | README driver table adds "Ethernet (GEM)"; "no drivers" paragraph reworded (`README.md:129-135`). |
| R4-3 | Comment | `src/drivers/` missing from repo maps | **CLOSED** | `AGENTS.md:30` and README layout list `hal`/`drivers`/`math`/`security`. |
| R4-4 | Comment | `driver-development.md` lacks HAL/GEM example; History stale | **OPEN (non-blocking)** | Guide §1 still lists three interfaces (PAL/link/board); no `FF_EthMac_t` example; History not bumped. |
| R4-5 | Comment | lwIP tag-pinned, no fetch verification, licence `-only`/`-or-later` unspecified | **OPEN (non-blocking)** | `third_party/README.md` still pins `STABLE-2_2_1_RELEASE`; `fetch_third_party.sh` verifies no revision; `licensing.md` states `GPL-3.0`. |

### 6.3 New delta non-conformances

| NC | Severity | Profile | Summary | Status |
| -- | -------- | ------- | ------- | ------ |
| D-1 | Minor | R4 | Stale M4 records: `verification-results.md` (`gem 18`/324, coverage 1421/2219, funcs 192/236) and `CHANGELOG.md` (`test_gem` 18 checks) vs actual `gem 23`/329 and regenerated 1437/2239 | open |
| D-2 | Minor | R2 | Residual verification gaps: no mock `FF_Dma_t` (cache path unexercised), no TX wrap/multi-frame, timeout/recovery, ring-full, NULL-arg, `set_link_up` or 100 Mbps tests; no `Inp` record | open |

### 6.4 Conformance verified (no finding)

- **Traceability gate:** `trace.py` exit 0 — 71 requirements (37 srs / 20 sss /
  14 gap), 51 Lean theorems, 9 CI claims; `SSS-REQ-020 <- 1 srs child`.
- **Build/test/ASan:** Debug build clean (no warnings); 9/9 ctest pass;
  `test_gem` `checks=23 failures=0`; total 329 checks
  (15+20+9+29+101+31+20+81+23); ASan+UBSan clean.
- **Register/bitfield/descriptor layout** verified against `macb.h`
  (`NCR` `CLRSTAT`/`BP`/`TSTART`; `NCFGR` `SPD`/`FD`/`BIG`/`GBE`/`DRFCS`/`DBW`;
  `DMACFG` `RXBS`; `TSR` `COMP`; `RX` `USED`/`WRAP`/`FRMLEN`) and against
  `macb_main.c` for the RX `USED` ownership semantics and the TX `WRAP` handling.
- **README metrics panel** matches `metrics.py` byte-for-byte
  (1437/2239 lines, 194/237 functions, `build/coverage/coverage.json`).
- **Document ids:** 0 duplicate `Document id` declarations (including
  `FRICK-REVIEW-0025`/`-0026`); `FRICK-REVIEW-0027` is free and used here.
- **Docs coherence:** SRS 0.7, SSS 0.7, traceability.md 0.6, licensing.md 0.5,
  development-plan 0.5, verification-results 0.7, tool-qualification 0.1.8,
  hardware-abstraction 0.2, platform 0.2, roadmap 0.7; `status.json`,
  `roadmap.md` and `development-plan.md` all show M4 *in progress*; TSN
  correction consistent across ADR-0014, development-plan, hardware-abstraction,
  platform and SRS.
- **SPDX:** new sources/headers/test carry `SPDX-License-Identifier:
  AGPL-3.0-only`; fetched third-party trees remain `.gitignore`d and uncommitted.

---

Committed and pushed with the reviewed change (norm N11). Proposed review-log row
for `docs/reviews/README.md`:
`| 2026-10-10 | M4 GEM driver delta | R1+R2+R3+R4 | APPROVED WITH COMMENTS | 2026-10-10-m4-gem-delta-review.md |`.
