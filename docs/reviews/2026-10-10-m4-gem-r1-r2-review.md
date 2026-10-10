# Milestone M4 (Cadence GEM Ethernet driver, increment 1) - Independent R1/R2 Review

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0025                      |
| Reviewer session                  | (independent agent, norm N9)           |
| Reviewer profile                  | R1 (SW dev) **and** R2 (SW V&V)        |
| Reference revision                | branch `main`, working tree on base commit `b4a1e5f` (uncommitted M4 increment-1 delta) |
| Items under review                | `src/drivers/eth/gem/ff_gem.{h,c}`, `src/drivers/eth/gem/ff_gem_regs.h`, `src/hal/hal_eth.h` (`FF_EthMac_t` vtable), `tests/unit/test_gem.c`, `CMakeLists.txt` (`FF_BUILD_DRIVERS`), `tests/CMakeLists.txt`, `docs/requirements/traceability.yaml` (SSS-REQ-020, SRS-REQ-036), `docs/adr/ADR-0014-drivers-and-third-party.md`, `third_party/README.md`, `tools/scripts/fetch_third_party.sh`, and the doc deltas (SRS/SSS, development-plan, roadmap, hardware-abstraction, licensing, verification-results, README, status) |
| Date                              | 2026-10-10                             |
| Scope                             | Correctness and defensive programming of the register/descriptor logic, static/no-alloc behaviour, bounded polling, the `FF_EthMac_t` contract change, the honesty of the doc claims (native GEM TSN, third-party licences), conformance to norms N1-N17 and ECSS-E-ST-40C / ECSS-Q-ST-80C, traceability, and V&V/test correctness. Acceptability of the `lean_note` for SRS-REQ-036. |
| Independence declaration          | The reviewer had no part in authoring, designing, verifying or executing the reviewed items; this review ran in a fresh context with only the repository and public standards/IP references (norm N9). |
| Norms applied                     | N1-N17 (see `docs/rules.md`, including interpretation notes 1-3) |

## 1. Scope

This review covers the M4 increment-1 delta as found in the uncommitted working
tree on base commit `b4a1e5f`: the Cadence GEM Ethernet driver behind the
`FF_EthMac_t` HAL contract, the vtable signature change to `void *self`, the
host mock-MMIO unit test, the build wiring (`FF_BUILD_DRIVERS`), the
requirements/traceability entries (SSS-REQ-020, SRS-REQ-036), ADR-0014, the
third-party fetch scaffolding (lwIP, N7/ESA `asn1-pusc-lib`) and the associated
documentation deltas. Both the R1 (design/code) and R2 (V&V) profiles were
applied to the same delta.

**Excluded:** real board bring-up (PHY/link validation is the documented
class-A gap GAP-A5) is not counted as a finding by itself; this review is about
whether the delivered register/descriptor logic, its error handling and its
verification are correct and honest as delivered. The lwIP/N7 adapters
(`FF_NetStack_t`, `FF_Pus_Backend_t`) are not delivered in this increment.
The R3 (N14) and R4 (N17) reviews are separate.

### Verification evidence executed (independent re-run)

| Command | Result |
| ------- | ------ |
| `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** - 71 requirements, 37 srs, 20 sss, 14 gaps, 51 Lean theorems, 9 CI claims (`EXIT=0`) |
| `cmake -S . -B build/grev -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build build/grev` | **PASS** (no warnings) |
| `ctest --test-dir build/grev --output-on-failure` | **PASS** 9/9 (`ccsds, sb, tbl, pus, time, hal, math, sdls, gem`) |
| `test_gem` check count | `gem 18` - matches the claimed 18 checks (324 total = 306 + 18) |
| ASan+UBSan build (`-fsanitize=address,undefined`) + ctest | **PASS** 9/9, gem clean |
| `python3 tools/scripts/coverage.py build/gem_cov` (fresh `FF_ENABLE_COVERAGE=ON`) | **1418/2219 lines (63.9%), functions 191/236** - see R2-05 (recorded claim is 1421/2219, 192/236) |
| `clang-format --dry-run --Werror` on the new files | not executable in this environment (binary absent) - claim not independently re-verified |

The requested gates pass exactly as stated (9/9, `gem` 18 checks, trace 71 reqs).
**The defects below are not detected by these gates**, because the mock encodes
the same (incorrect) hardware assumptions as the driver (finding R2-01).

## 2. References

- Project norms: `docs/rules.md` (N1-N17, interpretation notes 1-3)
- Traceability manifest: `docs/requirements/traceability.yaml`
- Design/decision: `docs/adr/ADR-0014-drivers-and-third-party.md`, `docs/architecture/hardware-abstraction.md`, `docs/development-plan.md`
- V&V records: `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md`
- Licensing: `docs/quality/licensing.md`, `third_party/README.md`, `tools/scripts/fetch_third_party.sh`
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2
- Cadence GEM IP references (external, authoritative): Microchip PolarFire SoC
  Product Overview §3.5.1 (DS60001656); Microchip GMAC Network Control Register
  map (onlinedocs); Linux `drivers/net/ethernet/cadence/macb.h` (Cadence
  GEM register/bitfield definitions); iPXE `drivers/net/cgem.c`; PolarFire SoC
  MSS TRM DS60001702 §3.1.2.5 (write-back data cache)

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R1-01 | **Major** | SRS-REQ-036; defensive programming; bounded polling | **The MAC "soft reset" is written to a non-existent register bit, so `init` would fail on real hardware.** `gem_init` writes `FF_GEM_NWCTRL_RST (1u<<15)` and then polls for that bit to self-clear. On the Cadence GEM the Network Control register bit 15 is `SRTSM` (Store Receive Timestamp to Memory) - a sticky PTP configuration bit; **there is no software reset in the MAC** (the reset is driven from outside the block). On the target the poll exhausts `FF_GEM_RESET_POLL` (1000) and the driver returns `FF_ERROR`; the mock hides this by special-casing `NWCTRL` to clear bit 15. | `src/drivers/eth/gem/ff_gem.c:49-58` (write + poll), `src/drivers/eth/gem/ff_gem_regs.h:39`; hidden by the mock at `tests/unit/test_gem.c:40-42`. Microchip GMAC NCR map: bit 15 = SRTSM, bits 8/9/10 = BP/TSTART/THALT; iPXE `cgem.c`: "There is no software-driven reset capability in the hardware." | Remove the bogus bit-15 reset and poll; perform the documented reset sequence (write the expected reset values, as iPXE does) or defer reset to the BSP/reset controller. Add a test that the reset path is exercised against an independent register model, including the failure path. |
| R1-02 | **Major** | SRS-REQ-036 (MAC configuration) | **Gigabit mode, speed and duplex are programmed into the wrong registers/bits, so the MAC is misconfigured.** Speed and full-duplex are Network *Config* bits 0/1 (`MACB_SPD`/`MACB_FD`); gigabit enable is Network *Config* bit 10 (`GEM_GBE`). The driver instead sets `SPEED100 = NCR bit 8` (Network Control bit 8 = **back-pressure**) and `FULLD = NCR bit 9` (**start transmission**), and sets `GIGE = NCFGR bit 8` (which is `MACB_BIG`, "receive 1536-byte frames") rather than bit 10. Gigabit mode is therefore never enabled and speed/duplex are never configured. | `src/drivers/eth/gem/ff_gem.c:60-61, 84-92`; `ff_gem_regs.h:40-41, 44`; compared against `macb.h` NCR `MACB_BP`/`MACB_TSTART`, NCFGR `MACB_SPD`=0, `MACB_FD`=1, `GEM_GBE`=10, `MACB_BIG`=8; Microchip GMAC NCR map. The test only asserts RXEN, never GBE/SPD/FD (`tests/unit/test_gem.c:89`). | Program NCFGR = `GIGE(bit10) | SPD | FD | (DBW)` and NCR = `RXEN|TXEN|MDEN` only; add register-image assertions to the test. |
| R1-03 | **Major** | SRS-REQ-036 (reference MAC implementation); defensive programming | **The receive path is not a real implementation: the RX descriptor ring is never consumed and the frame length is read from a non-existent register field.** `FF_GemPollRx` always dispatches `rx_buf[0]` with `len = status >> 16`. Receive Status (offset 0x20) defines only bits 0-2 (BNA/REC/OVR) - there is **no length field at [31:16]**; the completed length/buffer are held in the RX descriptor control word (`FRMLEN` bits [11:0], after the DMA clears the software ownership/`USED` bit). The driver initialises `rx_desc[i].ctrl = 0` (never offering the buffers to the DMA by setting the ownership bit) and never reads any `rx_desc`/`rx_head`. The RX ring programmed into `RXQBASE` is therefore inert. | `src/drivers/eth/gem/ff_gem.c:77-81` (init), `:188-211` (poll), `ff_gem.h:53-55`; `macb.h` RSR bits 0-2, `MACB_RX_USED_OFFSET 0`, `MACB_RX_FRMLEN_OFFSET 0`, `MACB_RX_EOF_OFFSET 15`; the mock fabricates the length at `tests/unit/test_gem.c:140`. | Implement descriptor-based RX: set the ownership bit on each buffer, set the wrap bit on the last descriptor, poll the descriptor `USED`/EOF state, take the length from descriptor word 2, invalidate the cache, advance `rx_head`, dispatch each completed frame. This is the core of a "reference MAC driver". |
| R1-04 | **Major** | SRS-REQ-036; portability/hardware assumptions (RISC-V) | **No DMA cache maintenance; the injected `FF_Dma_t` is used only for address translation.** The U54 data cache is 32 KB 8-way write-back (MSS TRM §3.1.2.5), and the HAL `FF_Dma_t` exposes `cache_flush`/`cache_invalidate` precisely for this. `gem_phys()` only calls `virt_to_phys` (`ff_gem.c:28-34`); the driver never flushes the TX descriptor/frame before kicking the DMA and never invalidates RX buffers/descriptors after completion. On the target the DMA can read stale descriptors/data and software can read stale received data. No host test can detect this (no `FF_Dma_t` is passed). | `src/drivers/eth/gem/ff_gem.c:28-34, 126-164`; `src/hal/hal_mem.h` `struct FF_Dma` (`cache_flush`, `cache_invalidate`); MSS TRM DS60001702 §3.1.2.5. | Flush descriptors + TX buffer before the transmit kick; invalidate RX buffer + descriptor after completion. Add a mock `FF_Dma_t` that records the calls and assert flush-before-kick / invalidate-after-completion in the suite. |
| R1-05 | Minor | SRS-REQ-036 (DMA configuration) | **`FF_GEM_DMACFG_RXBUF_1536` lands in the wrong DMACFG field and its value contradicts its name/comment.** In the GEM, DMACFG bits [4:0] are `FBLDO` (DMA fixed burst length); the DMA receive buffer size is `RXBS` at bits [23:16]. Writing `0x00000004` sets the burst length, not the buffer size. The comment "64 * 2^4 = 1024" also contradicts the macro name `_1536` (and 1024 < MTU 1536). | `ff_gem_regs.h:51-52`; `ff_gem.c:64`; `macb.h` `GEM_FBLDO_OFFSET 0` (size 5), `GEM_RXBS_OFFSET 16` (size 8). | Write the required buffer size into the `RXBS` field (or drop the write and document the default), and make the macro name/comment self-consistent. |
| R1-06 | Minor | SRS-REQ-036; defensive programming | **The descriptor ring wrap bits are never set (and not defined).** The DMA only wraps to the ring base if the last descriptor carries the wrap bit (`GEM_TX_WRAP`, TX ctrl bit 30; `MACB_RX_WRAP`, RX ctrl bit 1). Neither is defined or written, so even a corrected ring would not wrap. | `ff_gem_regs.h:54-57`; `ff_gem.c:73-82`; `macb.h` `MACB_TX_WRAP_OFFSET 30`, `MACB_RX_WRAP_OFFSET 1`. | Define `*_WRAP` and set it on `tx_desc[FF_GEM_TX_DESC-1]` / `rx_desc[FF_GEM_RX_DESC-1]`. |
| R1-07 | Minor | SRS-REQ-036; fault containment | **A single transmit timeout permanently wedges the TX path (no recovery).** On completion timeout, `gem_transmit` increments `errors` and returns `FF_ERROR` *before* restoring the descriptor to free (`ctrl |= USED`) and before advancing `tx_head`. The descriptor keeps `USED=0` (DMA-owned), so the next call for the same index returns `FF_NOT_READY` forever; there is no reset/reclaim path. | `src/drivers/eth/gem/ff_gem.c:137-140, 149-161`. | On timeout, reset/recover the MAC and reclaim the descriptor; expose a recovery path and test it. |
| R1-08 | Minor | Contract/doc accuracy | **`FF_GemBaseAddress` is a strong, unused symbol while the header claims "weak defaults provided".** A board override would collide at link time; the helper is never called (the driver uses `cfg->base`). | `ff_gem.h:88-90`; `ff_gem.c:237-241`; no `__attribute__((weak))` anywhere in the driver (`grep weak src/drivers`). | Either mark it weak and use it, or remove it and correct the comment. |
| R1-09 | Minor | Defensive programming | **Vtable entry points do not validate `self`/bus callbacks; `FF_GemSetup` accepts a NULL bus and returns `FF_SUCCESS`.** Only `gem_init` checks `bus.read32/write32`; `transmit`/`set_link_up`/`get_mac_address`/`get_stats`/`FF_GemPollRx` dereference the (possibly NULL) bus through `gem_rd`/`gem_wr` if `init` was not called or failed, and none checks `self != NULL`. | `ff_gem.c:45-47` (only check), `:213-234` (Setup no validation), `:18-26, 99-186, 188-211`. | Validate the bus in `FF_GemSetup` (reject NULL) and/or add NULL guards in each entry point. |
| R1-10 | Comment | Defensive programming | `gem_phys` silently truncates the 64-bit `virt_to_phys` result to `uint32_t` (`ff_gem.c:31`). GEM descriptor addresses are 32-bit, so buffers above 4 GiB cannot be used; this should be validated/rejected rather than silently truncated. (MPFS DDR is below 4 GiB, so low present-day risk.) | `ff_gem.c:28-34`; `src/hal/hal_mem.h` `virt_to_phys` returns `uint64_t`. | Range-check the translated address; fail loudly if it does not fit 32 bits. |
| R1-11 | Comment | Static/no-alloc and stack usage | `sizeof(FF_Gem_t) = 12504` bytes (tx/rx buffers 12288), yet the tests declare it on the stack in three functions. On RTEMS this is a task-stack hazard for a component meant to be a static instance. | Measured against the reviewed headers; `tests/unit/test_gem.c:70, 99, 123`. | Document "one static instance per MAC, not on a task stack"; consider a smaller default ring/MTU or a pool. |
| R1-12 | Comment | Register-map naming accuracy | Minor naming inaccuracies: `FF_GEM_NWCTRL_LOOPBACK` is bit 1 = local loopback (`LBL`); `FF_GEM_TXSTATUS_TXBUSY` is bit 3 = `TGO` (transmit go), not a busy flag. Also the NCFGR full write clears reset-value fields (e.g. `DBW` data-bus width) because it is not a read-modify-write. | `ff_gem_regs.h:35, 49, 44`; `ff_gem.c:61`; `macb.h` `MACB_LLB`, `MACB_TGO`, `MACB_DBW`. | Align names with the IP; set DBW explicitly as the Linux GEM driver does. |
| R2-01 | **Major** | V&V confidence (norm N6/N10) | **The unit suite passes (18 checks) while the Major code defects above are live; the mock is not an independent oracle.** It reproduces the implementation's own (incorrect) hardware model rather than the device: `mock_write32` special-cases `FF_GEM_NWCTRL` to clear the bogus RST bit (`test_gem.c:40-42`), the RX test injects `0x2 | (42<<16)` into RXSTATUS asserting the fabricated length field (`:140`), no `FF_Dma_t` is passed so cache maintenance is never required, and the MAC assertions check only RXEN, never GBE/SPD/FD/DBW or the descriptor contents. Expected values derived from the implementation cannot detect implementation errors. | `tests/unit/test_gem.c:30-58, 89-94, 140`; defects R1-01..R1-04. | Rebuild the mock from an independent register/descriptor model (Cadence GEM / MSS TRM); assert full expected register images and descriptor contents (ownership, wrap, length, USED); add a mock DMA recording flush/invalidate; add the negative paths in R2-02. |
| R2-02 | Minor | SRS-REQ-036; test coverage | **Only happy-path coverage; the negative and boundary paths are untested.** Not covered: reset timeout/`FF_ERROR`, TX ring-full (`FF_NOT_READY`) and timeout/recovery, RX oversize clamp and zero length, `rx_cb == NULL`, NULL arguments (`FF_GemSetup`, `get_mac_address(NULL)`, `get_stats` with NULLs), `set_link_up`, the DMA translation path (`cfg.dma != NULL`), `FF_GemBaseAddress`, multi-frame and ring-wrap. `SRS-REQ-036` declares `method: Tst | Inp`, but no inspection record accompanies the `Inp` method. | `tests/unit/test_gem.c` (18 checks, all happy path); `traceability.yaml` SRS-REQ-036. | Add the negative/boundary cases and record the inspection evidence for `Inp`. |
| R2-03 | Minor | SRS-REQ-036 (scope vs evidence) | **SRS-REQ-036 bundles TSN ("native GEM TSN (802.1Q/Qbv/Qav/802.1AS) available through `FF_TsnBackend_t`"), but this increment delivers no TSN adapter, no `FF_TsnBackend_t` usage and no evidence.** The manifest links SRS-REQ-036 solely to the GEM driver and `test_gem.c`, which never references `FF_TsnBackend_t`. The requirement as worded is broader than the delivered/verified item. (Echoes the earlier M3 R2 NC-4 on `test_hal.c`/`FF_TsnBackend_t`.) | `docs/requirements/SRS.md` SRS-REQ-036; `traceability.yaml` SRS-REQ-036; `tests/unit/test_gem.c` has no TSN reference. | Split the TSN clause into its own requirement with its own evidence when the backend lands, or narrow SRS-REQ-036 to the MAC driver for this increment. |
| R2-04 | Minor | norm N10/N11 (tool qualification) | **`test_gem.c` was not registered/re-qualified.** `docs/vv/tool-qualification.md` is at rev 0.1.7 (M7) and has no M4 entry recording the new test script; the established practice records each new `tests/unit/*.c` in the T3 history. N10/N11 require the register to be updated in the same commit. | `docs/vv/tool-qualification.md:75` (last history entry M7); no M4/`test_gem` row. | Add a T3/M4 entry (`tests/unit/test_gem.c`, 18 checks) with the source-review disposition. |
| R2-05 | Minor | norm N10/N11 (reproducibility) | **The recorded coverage metric is not exactly reproducible and does not state the toolchain.** README/`verification-results.md` claim 1421/2219 lines (64.0%), functions 192/236; a fresh `FF_ENABLE_COVERAGE=ON` build + `ctest` + `coverage.py` in this environment yields 1418/2219 lines (63.9%) and 191/236 functions. The delta is small and compiler/gcov-version dependent, but N10/N11 expect recorded evidence to be reproducible and the tool/version to be identified. | fresh `build/gem_cov/coverage.json` vs `README.md` metrics panel and `docs/vv/verification-results.md:27`. | Record the compiler/gcov version with the metric, re-generate on the qualified image, or state the claim as "≥63.9%". |
| R2-06 | Comment | norm N8/N11 (records vs review state) | The `verification-results.md` M4 row states a full green gate ("clang-format clean") while the M4 independent reviews are not yet committed and the defects above are open; the M4 reviews (this report, R3, R4) must be committed with log rows before the change may be pushed (norm N11/N12/N14/N17). The `gem` "18 checks" and "324 total" claims are verified correct. | `docs/vv/verification-results.md:27`; `docs/reviews/README.md` has no M4 row; this report is untracked. | Reconcile the record with the review state and commit the M4 reports with log rows in the same cycle. |

**Positive observations (no action):**

- The requested gates pass exactly as stated: trace **PASS (71 reqs)**, Debug
  build clean, **9/9** tests, `gem` = **18 checks**, and an independent
  ASan+UBSan run is clean. The check accounting reconciles (306 + 18 = 324).
- The driver is genuinely allocation-free (no `malloc`); waits are bounded by
  `FF_GEM_RESET_POLL`/`FF_GEM_TX_POLL`; all buffers are fixed-size arrays; the
  vtable uses a valid offset-0 container downcast and is self-consistent with
  its only caller set (no other `FF_EthMac_t` implementation exists in-tree).
- SPDX/AGPL headers are present on all new files; the fetched third-party trees
  are correctly `.gitignore`d and not committed.
- The requirement/doc bookkeeping is consistent: SRS/SSS revision histories
  bumped, traceability manifest updated, counts (71/37/20/14, 51 theorems)
  coherent across README, traceability.md and verification-results.

**Findings graded Comment (no derogation, tracked as observations):**

- **C-1 (`lean_note` for SRS-REQ-036) - acceptable under norm N1.** The note
  (`traceability.yaml`) is explicit and justifies verification by host unit
  tests plus inspection with the target board deferred to GAP-A5; interpretation
  note 1 permits this with no silent waiver. Remark: the pure ring-index
  arithmetic, bounds and clamp logic are Lean-modelable; a small `Frick.Gem`
  theorem would strengthen N1 beyond the note in a later increment, and the
  note's wording should not conflate the deferred PHY/link behaviour with the
  register/descriptor logic that the tests claim to verify (which they currently
  do not - R2-01).
- **C-2 (TSN doc claim) - verified honest.** The ADR/development-plan correction
  from "(GEM has no TSN)" to "native GEM TSN block (802.1Q/Qbv/Qav/Qci/Qbb/
  802.1AS/1588/CB)" is corroborated by the PolarFire SoC Product Overview
  §3.5.1, which lists exactly those IEEE 802 standards on the GEM; the fabric
  CoreTSN is only for multi-port switching/FRER. This is an accurate correction,
  not an overclaim.
- **C-3 (third-party licences) - verified, with a precision remark.** lwIP
  (BSD-3-Clause) and `n7space/asn1-pusc-lib` (GPL-3.0, confirmed from the
  project page) are correctly identified; the AGPL-3.0 compatibility direction is
  right. The licensing doc should distinguish "aggregate" (mere aggregation)
  from "combined work" and, if a binary linked with `FF_USE_N7PUS` is ever
  distributed, state the GPL-3.0 corresponding-source obligation. Both default
  OFF.
- **C-4 (fetch script) - good practice.** `fetch_third_party.sh` pins lwIP by tag
  and `asn1-pusc-lib` by commit; consider verifying the fetched revision/checksum
  and recording whether the script is in the N10 tool scope (it is not V&V
  evidence today).

## 4. Conclusion

### R1 - Design/code (Senior real-time/space SW developer profile)

- [ ] APPROVED - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [x] **REJECTED** - one or more Major findings

Justification: four Major non-conformities are confirmed against the
authoritative Cadence GEM/MSS references - R1-01 (software reset written to the
non-existent `SRTSM` bit; `init` fails on real hardware), R1-02 (gigabit / speed
/ duplex bits programmed into the wrong registers/bits; gigabit never enabled),
R1-03 (the RX path does not implement descriptor-based reception and fabricates
the frame length from a non-existent status field) and R1-04 (no DMA cache
maintenance despite the injected `FF_Dma_t` and the write-back U54 dCache).
These are register/descriptor-logic defects that can defeat the SRS-REQ-036
requirement on the target and are independent of the deferred PHY/link GAP-A5.
The remaining design items are Minor/Comment. Norms N1-N17 are not satisfied for
this artifact until the Majors are fixed and re-reviewed (norm N12).

### R2 - Software V&V (Senior SW V&V engineer profile)

- [ ] APPROVED - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [x] **REJECTED** - one or more Major findings

Justification: the requested gates pass (9/9, `gem` 18 checks, trace 71 reqs),
but the V&V evidence does not establish the required confidence: R2-01 (Major)
shows the mock is built from the implementation's own (incorrect) hardware model,
so all four live Major code defects pass green - the suite cannot detect them.
R2-02..R2-05 are Minor (coverage thin, requirement scope vs evidence, tool
qualification not updated, coverage claim not exactly reproducible). V&V cannot
be accepted until the tests are rebuilt against an independent register/descriptor
model with the negative paths, the defects are re-verified after fix, and the
qualification/reporting records are updated (norm N12).

Overall for the M4 increment-1 delta: **REJECTED** pending closure of R1-01,
R1-02, R1-03, R1-04 and R2-01, and re-review of the deltas (norm N12/N15).

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 / R2 (N9)  | independent agent (this session) | 2026-10-10 | REJECTED (R1: 4 Major, 8 Minor/Comment; R2: 1 Major, 4 Minor, 1 Comment) |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| M4-NC-01 | Major | SRS-REQ-036 | R1-01 reset on non-existent NCR bit 15 (`SRTSM`); init fails on target | Remove the bogus reset/poll; perform the documented reset sequence or defer to the BSP; test the failure path against an independent model | `init` succeeds against a device-faithful mock; reset path test | frick-fsw | open | pending |
| M4-NC-02 | Major | SRS-REQ-036 | R1-02 gigabit/speed/duplex in wrong registers/bits | Program NCFGR `GIGE(bit10)|SPD|FD|DBW` and NCR `RXEN|TXEN|MDEN`; add register-image assertions | Full expected NCFGR/NCR image asserted in test | frick-fsw | open | pending |
| M4-NC-03 | Major | SRS-REQ-036 | R1-03 RX path not descriptor-based; length from fake RSR field | Implement descriptor ownership/wrap/EOF/length handling, advance `rx_head`, dispatch per frame | RX test driven by a descriptor model; multi-frame + wrap cases | frick-fsw | open | pending |
| M4-NC-04 | Major | SRS-REQ-036 | R1-04 no DMA cache maintenance | Flush TX descriptor/buffer before kick; invalidate RX after completion | Mock `FF_Dma_t` records and asserts flush/invalidate | frick-fsw | open | pending |
| M4-NC-05 | Major | norm N6/N10 | R2-01 suite passes with live Major defects; mock is not an independent oracle | Rebuild mock from Cadence GEM/MSS TRM; assert full register/descriptor images; add negative paths and mock DMA | New tests fail before fix / pass after; independent-oracle review | frick-fsw | open | pending |
| M4-NC-06 | Minor | SRS-REQ-036 | R1-05 `DMACFG` RX buffer value in wrong field/misnamed | Write `RXBS` field (or drop); fix macro name/comment | Register-image assertion | frick-fsw | open | pending |
| M4-NC-07 | Minor | SRS-REQ-036 | R1-06 descriptor WRAP bits undefined/unset | Define and set `*_WRAP` on the last descriptor | Descriptor-content assertion | frick-fsw | open | pending |
| M4-NC-08 | Minor | SRS-REQ-036 | R1-07 TX timeout permanently wedges the ring | Reclaim/reset on timeout; expose recovery | Timeout-then-recover test | frick-fsw | open | pending |
| M4-NC-09 | Minor | contract/doc | R1-08 `FF_GemBaseAddress` not weak/unused vs header claim | Make weak+use, or remove and fix comment | Header/code consistency | frick-fsw | open | pending |
| M4-NC-10 | Minor | defensive programming | R1-09 no bus/`self` validation outside `init` | Validate bus in Setup and/or guard entries | Invalid-argument tests | frick-fsw | open | pending |
| M4-NC-11 | Minor | SRS-REQ-036 | R2-02 thin coverage; no `Inp` record | Add negative/boundary tests; record inspection | Extended suite; inspection note | frick-fsw | open | pending |
| M4-NC-12 | Minor | SRS-REQ-036 | R2-03 TSN clause not covered by evidence | Split TSN into its own requirement or narrow SRS-REQ-036 | Manifest/SRS update | frick-fsw | open | pending |
| M4-NC-13 | Minor | norm N10/N11 | R2-04 `test_gem.c` not registered/re-qualified | Add T3/M4 entry in `tool-qualification.md` | Register rev bump | frick-fsw | open | pending |
| M4-NC-14 | Minor | norm N10/N11 | R2-05 coverage claim not exactly reproducible; toolchain unstated | Record compiler/gcov version and re-generate, or restate the metric | Reproduced metric on the qualified image | frick-fsw | open | pending |
| M4-NC-15 | Comment | norms | R1-10..R1-12, R2-06, C-1..C-4 observations (32-bit truncation, 12.5 KB stack object, register naming, records vs review state, lean_note wording, licence precision, fetch-script hardening) | Consider in a later increment | n/a | frick-fsw | open | n/a |

---

Reviewer note on repository hygiene: this report must be committed together with
the reviewed change and a row added to `docs/reviews/README.md` (norm N11). Per
the subagent instruction for this review, no file other than this report was
modified; the coverage/build artifacts used as evidence were produced under
`build/gem_cov` (ignored) and `/tmp/opencode/`. The proposed log row is:
`| 2026-10-10 | M4 GEM driver code/V&V review | R1+R2 | REJECTED | 2026-10-10-m4-gem-r1-r2-review.md |`.
