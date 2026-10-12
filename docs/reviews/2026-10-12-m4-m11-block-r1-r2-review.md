# Review Report

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0146                      |
| Reviewer session                  | independent agent (norm N9), fresh context |
| Reviewer profile                  | **R1 + R2 combined** — R1 (Senior FSW / real-time / space software developer) **and** R2 (Senior SW V&V engineer) |
| Reference revision                | `737c805` (HEAD) + **uncommitted** Sprint-5 working-tree increment (reference MSS block-device backend, SRS-REQ-070) |
| Items under review                | `src/drivers/block/mpfs/ff_block_mpfs.{c,h}` (new); `tests/unit/test_block.c` (new); the `hal_block` deltas in `src/apps/app_renode_test.c`; `config/platform/platform_mpfs.h` (`FF_MMC0_BASE`); `CMakeLists.txt`, `tests/CMakeLists.txt`; requirement **SRS-REQ-070** and its chain in `docs/requirements/{SRS.md,traceability.yaml,traceability.md}`; `docs/design/software-design.md`; `docs/vv/{verification-results.md,renode-environment.md,tool-qualification.md}`; `docs/quality/class-a-gaps.md`; `CHANGELOG.md`; `README.md`; `docs/status/*`; `docs/engineering/software-description.md`; the `ci-renode` markers in both CI workflows. |
| Date                              | 2026-10-12                             |
| Scope                             | Independent combined R1+R2 (norm N9) review of the reference MSS MMC/SD (SDHCI-style) HAL block-device backend and its verification evidence, against ECSS-E-ST-40C (SW development/V&V), ECSS-Q-ST-80C (SW PA), the project norms N1–N20 (`docs/rules.md`), the HAL contract `FF_BlockDev_t` (`src/hal/hal_mem.h`), and the authoritative register map of the Renode `SD.MPFS_SDController` model. |
| Independence declaration          | The reviewer had no part in authoring, building-for-delivery, or previously reviewing the items under review. This review ran in a fresh context and every declared claim was re-executed or read first-hand (see §9). |
| Norms applied                     | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

**In scope (verified first-hand):**

- The new driver `src/drivers/block/mpfs/ff_block_mpfs.c` / `.h`: the register map, offsets and bit positions (BlockSizeBlockCount/Argument1/CommandTransferMode/DataBuffer/ErrorNormalInterruptStatus/Capabilities1/HostControl2) against the authoritative Renode `SD.MPFS_SDController` model; the `read`/`write`/`flush` vtable; the read/write command sequence (CMD17/18 read, CMD24/25 write) and the command-word encoding; the bounds (`lba+blocks <= num_blocks`, 16-bit BCCT, 32-bit Argument1, `block_size` power-of-two in `[512,4096]`); bounded/allocation-free coding; strict-warning cleanliness; NULL/argument validation (including the injected `FF_BlockBus_t` hooks); pre-setup (`FF_NOT_READY`); the timeout and error paths; the status poll/clear logic; the MMIO self-test and its side effects; the documented limitations.
- The new unit test `tests/unit/test_block.c` (32-bit behavioural mock + 12 test functions, 81 checks): whether it genuinely discriminates the faults it claims (timeout, error status, out-of-range, block-count programming) and whether the mock is honestly declared (not an independent oracle).
- The block-device functional check in `src/apps/app_renode_test.c` (`mpfs_mmc_read32/write32`, `renode_hal_block_check`, `hal_block=PASS`) and its term in the `RENODE_TEST` verdict — specifically that the Renode check is register round-trips only (no SD card is attached to the emulated board).
- Build/test wiring: `CMakeLists.txt` (`FF_BUILD_DRIVERS`) and `tests/CMakeLists.txt` (`test_block` target + ctest `block`).
- Platform constant `FF_MMC0_BASE` against the authoritative memory map.
- Requirement/evidence chain: SRS-REQ-070 text and parent (SSS-REQ-020), `traceability.yaml`/`traceability.md`, the `verification-results.md` row, `tool-qualification.md` (T3/T17), `class-a-gaps.md` (GAP-A5), `CHANGELOG.md`, `README.md`, `docs/status/*`, `docs/engineering/software-description.md`, `docs/design/software-design.md`, and the `ci-renode` markers in both workflows.
- Independent re-execution of the full local gate set (see §9).
- **Scope conformance** (standing rule): the implemented block-device backend and its documentation vs the documented scope (`src/hal/hal_mem.h` `FF_BlockDev_t`, SRS-REQ-032, SRS-REQ-070, `docs/roadmap.md` M4, `docs/development-plan.md` OBJ-10/OBJ-19) — see §4.

**Out of scope / excluded (recorded so the scope is bounded):**

- On-target silicon behaviour (the configured SD clock, bus width, card initialisation/type, real card timing, the on-target data path and wall-clock latency) — tracked to GAP-A5 and explicitly not verifiable here.
- The R3 (systems), R4 (documentation) and PA (N20) reviews of the same increment.
- The uncommitted doc-only deltas that carry no block content and belong to other review scopes; unrelated increments re-run only as aggregate regressions.

**Assumption recorded:** the assigned document id `FRICK-REVIEW-0146` was supplied by the review lead; the highest committed id observed in `docs/reviews/` is `FRICK-REVIEW-0145`, so the id is taken as assigned and not re-derived here.

## 2. References

- Project norms: `docs/rules.md` (N1–N20); reviewer contract: `docs/engineering/independent-review.md` (rev 0.7).
- Report format: `docs/reviews/review-template.md`.
- Contract: `src/hal/hal_mem.h` (`FF_BlockDev_t`, SRS-REQ-032).
- Requirements: `docs/requirements/SRS.md` (SRS-REQ-070, SRS-REQ-032), `docs/requirements/SSS.md` (SSS-REQ-020), `docs/requirements/traceability.yaml`.
- Scope documents: `docs/roadmap.md` (M4, M11), `docs/development-plan.md` (OBJ-10, OBJ-19), `docs/planning/sprint-plan.md` (S5), `docs/vv/renode-environment.md`.
- Gaps: `docs/quality/class-a-gaps.md` (GAP-A5).
- Evidence: `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md` (T3/T17), `CHANGELOG.md`.
- Authoritative hardware oracle: Renode `SD.MPFS_SDController` model (`renode-infrastructure`, `src/Emulator/Peripherals/Peripherals/SD/MPFS_SDController.cs`), register enum and bit fields; instance `mmc @ 0x20008000`, size `0x2000`, IRQ `plic@88`; **no SD card attached** in the `beaglev-fire` repl.
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation (required correction) |
| - | ------------------------------ | ------------------ | ------- | -------- | ------------------------------------ |
| **R1-01** | **Major** | SRS-REQ-070 — `Setup()` block-size contract vs the 12-bit TBS field | **`block_size == 4096` is a documented-supported value but cannot be represented in the 12-bit TBS field, and the driver silently truncates it to 0.** `block_issue()` programs `g_block->block_size & FF_MMC_TBS_MASK` into the BlockSizeBlockCount register (`ff_block_mpfs.c:76-77`), where `FF_MMC_TBS_MASK = 0x0FFF` (12 bits, `ff_block_mpfs.h:177`) and the oracle confirms TBS is b0-11 (`SD.MPFS_SDController`: `.WithValueField(0, 12, name: "Transfer Block Size (TBS)")`). Since `4096 = 0x1000` needs bit 12, **`4096 & 0x0FFF = 0`** (re-verified): a 4096-byte-block transfer programs TBS = 0 (an invalid/zero transfer block size), so the payload move reads/writes data the controller never transferred — silent data corruption. `Setup()` accepts 4096 (`FF_MMC_BLOCK_SIZE_MAX = 4096`, `ff_block_mpfs.h:228`) and the test **asserts that 4096 is accepted** (`test_block.c:254-255`), so the contract genuinely promises it; the mock derives `xfer_len = tbs * block_count` from the same truncated field (`test_block.c:85-87`) and **no test transfers with block_size 4096**, so the defect is undetected. The driver carries `FF_STATIC_ASSERT(FF_MMC_BCCT_MAX <= FF_MMC_BCCT_MASK, …)` but **no analogous assert** `FF_MMC_BLOCK_SIZE_MAX <= FF_MMC_TBS_MASK`, which would have failed at compile time. | `ff_block_mpfs.c:76-77`; `ff_block_mpfs.h:177,227-228`; `test_block.c:85-87,254-255`; oracle TBS b0-11 / model `.WithValueField(0,12)`; `4096 & 0x0FFF == 0` | Do not accept a block size the TBS field cannot represent: reject 4096 in `Setup()` (e.g. `FF_MMC_BLOCK_SIZE_MAX = 2048`), or confirm/handle the real MPFS field width. Add `FF_STATIC_ASSERT(FF_MMC_BLOCK_SIZE_MAX <= FF_MMC_TBS_MASK, block_size_fits_field)` and a discriminating host test (R2-01). |
| **R1-02** | Minor | SRS-REQ-070 — timeout path leaves the status register uncleared | **A timed-out transfer clears nothing, so a stale completion/error bit can mis-lead the next transfer.** `block_wait_done()` clears the status on the success path (`FF_MMC_INT_DONE`) and on the error path (`st`), but on the **timeout** path it returns `FF_TIMEOUT` with the register untouched (`ff_block_mpfs.c:50-67`); the driver also never clears the status *before* issuing a command. A completion bit left set by a timed-out command makes the next transfer's poll return immediately (premature `FF_SUCCESS` if both completion bits are set), and a stale error bit makes it return a spurious `FF_ERROR`. This is the same failure class the SPI backend fixed by draining/resetting on the error path (Sprint-4 R1-01); here the analogous cleanup is missing on the timeout path. The mock cannot discriminate it: `never_ready` keeps `err_status` at 0 (`test_block.c:92-95`), so no stale bit is modelled. | `ff_block_mpfs.c:50-67,141-166`; `test_block.c:383-391`; `ff_spi_mpfs.c:122-129` (Sprint-4 R1-01 fix) | Clear the status register on the timeout path (and/or before issuing a command), or document the stale-status hazard as an accepted limitation; add a regression test with a pre-set stale status bit that fails on the current driver (R2-01). |
| **R1-03** | Minor | SRS-REQ-070 — completion poll relies on an unprogrammed enable register | **The driver polls `ErrorNormalInterruptStatus` (0x230) for completion but never programs `ErrorNormalStatusEnable` (0x234).** On the emulated `SD.MPFS_SDController`, the command callback raises the completion bits only when the matching enable bit is set (`irqManager.SetInterrupt(Interrupts.CommandComplete, irqManager.IsEnabled(Interrupts.CommandComplete))`), so a real command on the model would never complete; the transfer path cannot be exercised there (consistent with it being host-tested only). On real SDHCI the normal-status bits are set regardless of the enable register (enable only gates the IRQ signal), so the driver is believed correct on target — but this is an unstated reliance that should be made explicit and confirmed on silicon. | `ff_block_mpfs.c:50-67`; `ff_block_mpfs.h:46-48`; model command write-callback (`SetInterrupt(..., IsEnabled(...))`) | Program `ErrorNormalStatusEnable` (0x234) for the completion/error bits as a defensive measure (it also makes the emulated model's transfer path consistent), or document the reliance on the SDHCI "status set regardless of enable" property; confirm on target under GAP-A5. |
| R1-C1 | Comment | SRS-REQ-070 — error-bit map | The model defines `ResponseError` at **b27**, which lies outside the driver's error mask `FF_MMC_INT_ERROR_MASK = 0x03FF8000` (bits 15-25, `ff_block_mpfs.h:214`); a response error would therefore be missed. Per the stated oracle ("errors b15-25") the mask is correct; the b27 definition is a model-only extra. | `ff_block_mpfs.h:214`; model `Interrupts` enum (`ResponseError = 27`) | Confirm the MPFS error-bit map against `mss_sd_regs.h` on target (GAP-A5); extend the mask if b27 is a real error bit. |
| R1-C2 | Comment | SRS-REQ-070 — completion model | The driver waits for **both** CommandComplete and TransferComplete and only then moves the whole payload through the DataBuffer. This is incompatible with a FIFO-based SDHCI (which requires BufferReadReady/BufferWriteReady handshaking while the transfer is in flight) and with the emulated model, where `TransferComplete` for a read is raised only *after* the buffer is drained (`ReadBuffer`) and for a write only after it is filled (`WriteBuffer`). Declared as a simplification in the header. | `ff_block_mpfs.c:61-64,96-122`; `ff_block_mpfs.h:79-86`; model `ReadBuffer`/`WriteBuffer` | Accepted as a declared limitation; the on-target data path and timing remain GAP-A5. |
| R1-C3 | Comment | SRS-REQ-070 — error not re-checked after the data move | The error status is checked only *before* the payload move; a data error raised during/after the move (e.g. a data CRC error) is not re-checked and the call returns `FF_SUCCESS`. Declared under the "no error recovery" limitation. | `ff_block_mpfs.c:141-193`; `ff_block_mpfs.h:94-96` | Accepted as a declared limitation; consider a post-move status re-check for data-integrity reporting. |
| **R2-01** | Minor | V&V evidence — test discrimination | **The host mock cannot discriminate R1-01 (4096-byte block) or R1-02 (stale status).** No test transfers with `block_size = 4096` (so the TBS truncation is invisible: the mock computes `xfer_len` from the same truncated field), and no test pre-sets a stale status bit (`never_ready` keeps `err_status = 0`). The mock is honestly declared a behavioural fake, not an independent oracle (`test_block.c:14-18`) — the defect is the untested behaviour, not the honesty. | `test_block.c:14-18,85-95,254-255,383-403` | Add a `block_size = 4096` transfer test and a stale-status/timeout-recovery test; they must fail against the current driver. |
| R2-02 | Comment | V&V evidence — error-injection granularity | The error injection sets the **whole** mask (`m->err_status = FF_MMC_INT_ERROR_MASK`, `test_block.c:96-99`), so the test cannot discriminate a wrong mask bit or a single-bit error; combined with the mock reusing the driver's own constants, the error-bit assignment is not independently checked. | `test_block.c:96-99`; `ff_block_mpfs.h:214` | Recorded for V&V/PA; obtain an independent oracle (datasheet-derived golden vectors) for the transfer/status path. |
| **SCOPE-01 (NC-01)** | Minor | Scope conformance / N8 / N16 / N19 | **`docs/development-plan.md` still reports BLOCK as pending, contradicting the implemented increment.** The §6 status table is stale: the M4 row (line 310) lists "…MSS I2C, MSS SPI done; **MMC/eMMC/storage**/PCIe/SpaceWire **pending**" and the M11 row (line 317) says "**in progress (8/13 peripheral-bound)**" with "Pending: …**BLOCK**", while SRS-REQ-070 is implemented and every other status document in this same increment records **M11 9/13** and BLOCK done (README, `handover.md`, `loop-audit.md`, `sprint-plan.md`, `status.json`, `software-description.md`, `verification-results.md`, `renode-environment.md`). `development-plan.md` was **not modified** in the increment. This is the **same doc-currency deviation** as the Sprint-4 SPI review's SCOPE-01 (closed at `development-plan.md` rev 0.21) and has re-opened for BLOCK (N8/N16/N19). The implemented feature itself is within the documented M4/OBJ-10/OBJ-19 scope; only the plan *status* contradicts it. | `docs/development-plan.md:310,317`; `git status --short docs/development-plan.md` (unmodified); README/handover/loop-audit M11 9/13 rows | Update the `development-plan.md` M4 and M11 status rows (BLOCK done; M11 9/13; BLOCK removed from "pending") in the corrective commit, and add the revision-history entry (N16). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable criticality, or a defect that can defeat a requirement or a safety barrier. *(R1-01.)*
- **Minor**: non-conformity that does not invalidate the requirement but must be corrected or justified. *(R1-02, R1-03, R2-01, SCOPE-01.)*
- **Comment**: observation/recommendation, no derogation.

## 4. Scope conformance (standing rule)

**Implemented content vs documented scope.**

- **Within scope, no over-implementation.** The driver implements exactly the `FF_BlockDev_t` contract (`name`, `block_size`, `num_blocks`, `read`, `write`, `flush`) plus a driver-specific `Setup`/`SelfTest` (outside the HAL vtable, consistent with the sibling backends). No DMA/ADMA, command queuing, SDIO, PHY tuning or card-init path is implemented — matching SRS-REQ-070 and the header limitations. The additional register `#define`s are documentation of the map only and unused. The MMIO self-test is a documented bring-up check with disclosed side effects.
- **Contract conformance.** `src/hal/hal_mem.h` `FF_BlockDev_t` (read/write/flush, `block_size`, `num_blocks`) is satisfied; the SRS-REQ-070 failure mapping (timeout → `FF_TIMEOUT`, error → `FF_ERROR`, invalid args → `FF_INVALID_ARGUMENT`, pre-setup → `FF_NOT_READY`) is implemented and host-tested. `docs/roadmap.md` M4 and `docs/development-plan.md` OBJ-10/OBJ-19 list the MMC/eMMC/SD block-device class; the implementation matches that scope. **One contract caveat:** the header/SRS-supported `block_size = 4096` is not implementable (R1-01).
- **Deviation found → NC-01 (SCOPE-01).** The **plan status** contradicts the implementation: `docs/development-plan.md` §6 (lines 310, 317) still reports BLOCK "pending" and M11 "8/13", while the increment records M11 9/13 and BLOCK done everywhere else. `development-plan.md` was not updated in the increment (N8/N16/N19). See §3 SCOPE-01 and §7 follow-up.

No other scope deviation was identified.

## 5. Conclusion

- [ ] APPROVED — no findings or Comments only
- [ ] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [x] **REJECTED** — one or more Major findings

The increment is **structurally sound and well-documented for the declared reference scope**: the register map, offsets and bit positions match the authoritative Renode `SD.MPFS_SDController` oracle exactly (BlockSizeBlockCount `0x204` TBS b0-11/BCCT b16-31, Argument1 `0x208`, CommandTransferMode `0x20C`-`0x20F`, DataBuffer `0x220`, ErrorNormalInterruptStatus `0x230`, Capabilities1 `0x240`, HostControl2 `0x22C`); the command-word encoding (data-present b21, response type 48-bit at b16-17, command index b24-29, data direction b4, multi-block b5, block-count-enable b1) is correct; the read/write command selection (CMD17/18/24/25) is correct; the bounds checks (`lba+blocks <= num_blocks`, 16-bit BCCT, 32-bit Argument1) are correct; all loops are statically bounded and allocation-free (no `while`/`goto`/`malloc`/recursion); the driver compiles clean under a strict warning set; `FF_BlockMpfsSetup` validates the instance, the bus and both bus hooks; the HAL binding and build/test wiring are correct; the Renode check is correctly limited to register round-trips (no SD card attached); and all gates pass (22/22 native and ASan, `test_block` 81 checks, `trace.py` PASS 114/71/29/14, `metrics --check` current, clang-format clean, Renode `hal_block=PASS`, QEMU PASS, Lean PASS, RTEMS cross-build PASS).

However, the **`block_size == 4096` path carries a real defect (R1-01, Major)**: the value is a documented-supported input (and is explicitly asserted as accepted by the host test) but cannot be represented in the 12-bit TBS field, and the driver silently truncates it to 0, mis-programming the transfer block size; the host mock cannot discriminate it (R2-01). Because a Major finding is open, the review is **REJECTED** and, per norm N12, blocks commit/push until R1-01 is resolved and the delta re-reviewed. R1-01 is bounded and has a small, safe fix (exclude 4096 or confirm the field width, plus the missing `FF_STATIC_ASSERT`).

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 + R2 (N9)  | Senior FSW engineer **and** Senior SW V&V engineer (independent context) | 2026-10-12 | **REJECTED** — Major R1-01 open; Minors R1-02/R1-03/R2-01/SCOPE-01 to closure (N12) |

## 7. Follow-up records

(norm N12: NCs are resolved by re-applying all norms to the affected scope and re-reviewing the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review | Scheduled sprint |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- | ---------------- |
| R1-01 | **Major** | SRS-REQ-070 `Setup` block size | `block_size = 4096` accepted but truncated to TBS = 0 (12-bit field); silent data-path mis-programming | Reject 4096 (or confirm the field width/handle it); add `FF_STATIC_ASSERT(FF_MMC_BLOCK_SIZE_MAX <= FF_MMC_TBS_MASK)`; add the discriminating test (R2-01) | (delta) `test_block` 4096-block regression + full gate set + Renode run | frick-fsw (authoring agent) | **open** | pending | **S5 delta (pre-commit)** |
| R2-01 | Minor | V&V test discrimination | Mock cannot discriminate the 4096-block truncation or a stale status bit | Add a 4096-block transfer test + a stale-status/timeout-recovery test that fail pre-fix | (delta) new test cases fail pre-fix, pass post-fix | frick-fsw (authoring agent) | open | pending | **S5 delta (pre-commit)** |
| R1-02 | Minor | SRS-REQ-070 timeout path | Timed-out transfer leaves the status register uncleared → stale completion/error bit mis-leads the next transfer | Clear the status on the timeout path (and/or before a command) or document; add a regression test | (delta) code + test | frick-fsw (authoring agent) | open | pending | **S5 delta (pre-commit)** |
| R1-03 | Minor | SRS-REQ-070 completion poll | Polls the completion status without programming `ErrorNormalStatusEnable` (0x234); the emulated model gates the status on the enable | Program 0x234 (defensive) or document the reliance; confirm on target | (delta) code/doc + GAP-A5 note | frick-fsw (authoring agent) | open | pending | **S5 delta (pre-commit)** |
| SCOPE-01 (NC-01) | Minor | Scope conformance (N8/N16/N19) | `development-plan.md` M4/M11 status rows still list BLOCK pending and M11 8/13 | Update the two rows (BLOCK done; M11 9/13) + revision-history entry | (delta) `development-plan.md` diff | frick-fsw (authoring agent) | open | pending | **S5 delta (pre-commit)** |
| R1-C1..C3, R2-02 | Comment | — | Advisory observations (§3) | Accepted / fixed where cheap | — | frick-fsw | n/a | — | — |

> Norm N20 (PA) and norm N17 (R4) reviews of this increment remain **out of scope of this combined R1+R2 report** and are to be run separately before commit/push. The review-log entry in `docs/reviews/README.md` is to be added by the authoring agent/coordinator (norms N11/N20) — this reviewer wrote only this report.

## 8. FSW / V&V review questions

**Q1 — Register map and bit positions vs the oracle.** **Match exactly.** Offsets GeneralInfo `0x000` … SlotInterruptStatus `0x0FC`, SDMASystemAddressArgument2 `0x200`, BlockSizeBlockCount `0x204` (TBS b0-11, BCCT b16-31), Argument1 `0x208`, CommandTransferMode `0x20C`-`0x20F`, Response1..4 `0x210`-`0x21C`, DataBuffer `0x220`, PresentState `0x224`, HostControl1 `0x228`, HostControl2 `0x22C` (ICS b1), ErrorNormalInterruptStatus `0x230` (write-1-to-clear), ErrorNormalStatusEnable `0x234`, ErrorNormalSignalEnable `0x238`, Capabilities1 `0x240` (SDMA b22, VS33 b24), Capabilities2..4 `0x244`-`0x24C`, ForceEvent `0x250`, ADMAErrorStatus `0x254`, DmaSystemAddressLow/High `0x258`/`0x25C`, PresetValue `0x260`-`0x26C` — all match the model's `Registers` enum. CommandTransferMode byte fields match (`0x20C` DMA/b1 BCE/b4 dir/b5 multi, `0x20E` response-type b0-1/b5 data-present, `0x20F` command index b0-5). The two completion bits (b0/b1) and the error mask (bits 15-25) match the model's `Interrupts` enum (only the model-only `ResponseError = 27` is outside the mask — R1-C1).

**Q2 — Command encoding and sequence.** **Correct.** `cmd = DATA_PRESENT | (RESP_48 << 16) | (cmd_index << 24)`, plus `BLOCK_COUNT_ENABLE | MULTI_BLOCK` for multi-block and `DATA_DIR_READ` for reads (`ff_block_mpfs.c:76-88`); single-block uses CMD17/CMD24, multi-block CMD18/CMD25 (`ff_block_mpfs.c:158-159,185-186`). Verified by hand for a single-block read (`0x11200010`: index 17, response type 2, data present, direction read) and a multi-block read (adds `0x22`). Sequence: program BlockSizeBlockCount → Argument1 → CommandTransferMode → poll → move payload. Correct.

**Q3 — Status poll / clear.** The poll bit and error mask **match the oracle**, and the success/error paths clear correctly (write-1-to-clear). **Two issues:** (a) the completion status is polled without programming `ErrorNormalStatusEnable` (R1-03); (b) the **timeout path clears nothing**, so a stale status bit can mis-lead the next transfer (R1-02). An error is **not missed** in the poll (error checked before completion), but a late data error after the payload move is not re-checked (R1-C3).

**Q4 — Bounds.** `block_check_args()` rejects `blocks == 0`, `blocks > 0xFFFF`, `lba > 0xFFFFFFFF`, `lba >= num_blocks`, and `blocks > num_blocks - lba` (`ff_block_mpfs.c:126-139`) — i.e. `lba + blocks <= num_blocks` and both fields fit their registers. Correct. **Block-size bound defect:** the accepted maximum 4096 exceeds the 12-bit TBS field (R1-01).

**Q5 — Bounded / allocation-free / warnings.** All loops are statically `for` loops bounded by `FF_MMC_POLL_MAX` (100000) or the byte count; no `while`, `goto`, `malloc`/`calloc`/`realloc`/`free`, recursion or delay primitive; deterministic and allocation-free. The driver and test compile clean under the strict warning set (`-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wundef -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes -fsyntax-only`).

**Q6 — Validation and pre-setup.** `FF_BlockMpfsSetup` rejects `NULL` `d`/`bus` and either missing bus hook, a non-power-of-two `block_size` outside `[512,4096]`, and `num_blocks == 0`; `read`/`write` return `FF_NOT_READY` before `Setup` (via `block_check_ready`), `FF_INVALID_ARGUMENT` for a `NULL` buffer or invalid range, and are host-tested. The `g_block == NULL` guard is defensive (an unbound vtable has no callable pointer).

**Q7 — Self-test.** Round-trips BlockSizeBlockCount (0x204) and Argument1 (0x208), checks Capabilities1 (SDMA + VS33) and HostControl2 (ICS); side effects (leaves BlockSizeBlockCount/Argument1 cleared) disclosed; runs register round-trips only, matching the "no SD card attached" constraint. Verified PASS on the emulated SoC (`hal_block=PASS`).

**Q8 — HAL contract compliance.** Verified against `src/hal/hal_mem.h`; the `FF_BlockDev_t dev` member is populated by `Setup()`; the driver-specific `Setup`/`SelfTest` are outside the vtable. No deviation (except the block-size caveat, R1-01).

**Q9 — V&V evidence.** The mock is a behavioural fake, honestly declared not an independent oracle (good); it genuinely discriminates timeout, error status, out-of-range and block-count programming, but **cannot discriminate R1-01 or R1-02** (R2-01) and sets the whole error mask (R2-02). The Renode check is register round-trips only (correct: no SD card). `trace.py` PASS with SRS-REQ-070 → SSS-REQ-020; T3 gains `test_block.c`; T17 gains `hal_block`; the `ci-renode` markers in both workflows name SRS-REQ-070.

**Q10 — Build/test wiring.** `CMakeLists.txt` adds the driver under `FF_BUILD_DRIVERS`; `tests/CMakeLists.txt` adds `test_block` + ctest `block`. Verified 22/22.

## 9. Evidence ledger (all re-executed/obtained first-hand on 2026-10-12)

| Claim | Method | Result |
| ----- | ------ | ------ |
| Native build | `cmake --build build/native` | up to date (green) |
| Native ctest | `ctest --test-dir build/native` | **22/22 pass**, incl. `block` |
| ASan/UBSan ctest | `ctest --test-dir build/asan` | **22/22 pass**, incl. `block` |
| `test_block` checks | `./build/native/tests/test_block` | `checks=81 failures=0` |
| Traceability | `trace.py docs/requirements/traceability.yaml` | **PASS (114 requirements, 71 srs, 29 sss, 14 gaps, 51 lean theorems)** |
| Metrics panel | `metrics.py --check` | `metrics panel: current` |
| Formatting | `clang-format --dry-run --Werror` on the changed C files | clean (exit 0) |
| Warning hygiene | strict set `-fsyntax-only` on driver + test (with `-I src -I src/core -I config`) | clean (exit 0) |
| Renode functional run (N18) | `make renode` | **PASS** (`RENODE_TEST: PASS`, incl. `hal_block=PASS`) |
| QEMU emulated target | `make qemu` | PASS (banner + 6 apps) |
| Lean (N1) | `lake build` via `frick-vv:local` | Build completed successfully |
| RTEMS cross build | `cmake --build build/rtems` via `frick-rtems-toolchain:local` | linked (green; block driver compiles) |
| Register map | oracle (`SD.MPFS_SDController`) vs `ff_block_mpfs.h` | exact match (b27 only — R1-C1) |
| TBS field width | model `.WithValueField(0,12)` + oracle "b0-11" | 12 bits; `4096 & 0x0FFF == 0` (R1-01) |
| FORBIDDEN constructs | `grep -nE '\bwhile\b\|\bgoto\b\|\bmalloc\b\|\bcalloc\b\|\brealloc\b\|\bfree\('` | none in the driver |
| SPDX headers (N13) | head of both new files | `AGPL-3.0-only` present |
| Scope conformance | `development-plan.md` M4/M11 rows vs implementation | stale (SCOPE-01) |

> Note: the gate commands regenerate build artifacts under `build/` (git-ignored) only; **no tracked source or documentation file was modified** by this review.

---

## Facts, Assumptions, Inferences, Recommendations, Confidence, Open gaps

**Facts** (verified first-hand, this session)

- The register map, offsets and bit positions of `ff_block_mpfs.h` match the authoritative Renode `SD.MPFS_SDController` oracle exactly (only the model-only `ResponseError = 27` is outside the driver's mask).
- The command-word encoding (CMD17/18/24/25, response type 48, data-present, direction, multi-block) is correct.
- `FF_MMC_TBS_MASK = 0x0FFF` (12 bits) while `FF_MMC_BLOCK_SIZE_MAX = 4096`; `4096 & 0x0FFF == 0`; `Setup()` accepts 4096 and `test_block.c:254-255` asserts it; no test transfers with 4096; the driver has a BCCT static assert but no TBS static assert.
- The completion status is polled without programming `ErrorNormalStatusEnable` (0x234); the model's command callback raises the completion bits only when the enable bit is set.
- `block_wait_done` clears the status on the success and error paths but not on the timeout path; the mock cannot model a stale status bit.
- All loops are bounded; no forbidden constructs; strict-warning clean.
- All gates pass: 22/22 native and ASan, `test_block` 81 checks, `trace.py` PASS 114/71/29/14 (51 theorems), `metrics --check` current, clang-format clean, Renode `hal_block=PASS`, QEMU PASS, Lean PASS, RTEMS cross-build PASS.
- `docs/development-plan.md` §6 M4/M11 status rows are stale (BLOCK "pending", M11 8/13) and were not updated in the increment.

**Assumptions** (owner, impact, validation)

- *Document id `FRICK-REVIEW-0146`* — owner: review lead; impact: report identity only; validation: lead renumbers if needed.
- *The uncommitted working-tree delta is the item under review atop `737c805`.*
- *On real SDHCI the normal/error status bits are set regardless of the enable register (enable only gates the IRQ signal)* — owner: hardware/mission (GAP-A5); validation: confirm against the Microchip MPFS SDHC datasheet/`mss_sd_regs.h`.
- *The 12-bit TBS width is authoritative (model + stated oracle)* — validation: confirm on silicon (GAP-A5).

**Inferences**

- Because `block_size & 0x0FFF` is used to program TBS and 4096 does not fit 12 bits, a 4096-byte-block transfer programs TBS = 0 → the controller transfers no data while the driver still moves `blocks * 4096` bytes → silent corruption (R1-01). Because the mock derives `xfer_len` from the same truncated field and no test uses 4096, the defect reaches the working tree undetected (R2-01).
- Because the timeout path clears nothing, a timed-out command can leave a stale completion/error bit that makes the next transfer's poll return immediately (R1-02).
- Because every other status document records M11 9/13 while `development-plan.md` records 8/13 and BLOCK pending, the plan status is inconsistent with the implemented content (SCOPE-01).

**Recommendations** (decision owner, prerequisites, trade-offs)

1. **Resolve R1-01 before commit** (Major, blocks per N12): reject `block_size = 4096` in `Setup()` (or confirm the real field width and handle it), add `FF_STATIC_ASSERT(FF_MMC_BLOCK_SIZE_MAX <= FF_MMC_TBS_MASK, …)`, add the discriminating test (R2-01) and re-run the full gate set + Renode run. Owner: authoring agent; sprint: **S5 delta (pre-commit)**.
2. Resolve R1-02, R1-03, R2-01 and SCOPE-01 (Minors) in the same corrective commit; update the `development-plan.md` status rows and add the revision-history entries (N8/N16/N19).
3. Route the oracle-independence concern (R2-02) to R2/PA; keep the GAP-A5 on-target residuals (SD clock/bus-width, card init, error-bit map incl. b27, wall-clock latency) tracked.
4. Run the R3/R4/PA (N14/N17/N20) reviews and add the `docs/reviews/README.md` log row before commit/push (norms N11/N20).

**Confidence:** **High** for the register-map/bit match, the command encoding, the bounded/allocation-free structure, the strict-warning cleanliness and the gate results (all reproduced first-hand and internally consistent). **High** for R1-01: it rests on a direct source read plus the authoritative 12-bit TBS field and simple arithmetic (`4096 & 0x0FFF == 0`), corroborated by the absent `FF_STATIC_ASSERT`. **Medium** for R1-02/R1-03: they rest on the model's status-enable gating and on the timeout path's behaviour, with the on-target controller semantics a GAP-A5 residual.

**Open gaps**

- **R1-01 (Major) open** — `block_size = 4096` mis-programming; blocks commit/push (N12).
- **R2-01** — no host test covers a 4096-byte block or a stale status bit.
- **SCOPE-01** — `development-plan.md` M11/M4 status not reconciled (N8/N16/N19).
- **No independent oracle** for the transfer/status path (R2-02); register offsets only are cross-checked (by Renode).
- **On-target evidence absent** — SD clock/bus-width, card init, error-bit map (incl. b27), wall-clock latency (GAP-A5).
- **R3/R4/PA reviews and the review-log row** for this increment are not part of this combined R1+R2 report.
- **Document id** `FRICK-REVIEW-0146` retained as assigned.

---

*This report is authored to be committed and pushed with the reviewed change (norm N11). The review was read-only with respect to repository content: the reviewer wrote exactly one file (this report); no other tracked file was modified, created or deleted, and nothing was committed or pushed. The gate commands regenerate only git-ignored `build/` artifacts.*
