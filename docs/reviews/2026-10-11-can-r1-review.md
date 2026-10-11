# Review Report - Reference MSS CAN driver (SRS-REQ-057), R1 code review

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0106                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (Senior FSW dev / Flight Software) |
| Reference revision                | a95cafb + uncommitted CAN increment |
| Items under review                | `src/drivers/can/mpfs/ff_can_mpfs.{c,h}`; `tests/unit/test_can.c`; CMake registration (`CMakeLists.txt`, `tests/CMakeLists.txt`); `renode_hal_can_check()` in `src/apps/app_renode_test.c`; `FF_CAN0_BASE` in `config/platform/platform_mpfs.h`; SRS-REQ-057/SSS-REQ-020 and traceability (`docs/requirements/traceability.yaml`, `docs/requirements/traceability.md`); docs updated (`docs/design/software-design.md`, `docs/vv/*`, `CHANGELOG.md`, `README.md`, `docs/status/*`) |
| Date                              | 2026-10-11                             |
| Scope                             | Software development aspects: HAL contract compliance (`src/hal/hal_can.h`), register-map correctness vs header, static/allocation-free claims, bounds checking (DLC/len, IDs std vs ext, buffer counts), error handling/return codes, ISR/polling and concurrency assumptions, TX round-robin and RX accept-all/poll semantics, self-test honesty and consistency, app integration, MISRA C:2012-oriented style and `.clang-format`. Gates re-run read-only. |
| Independence declaration          | Reviewer took no part in authoring, implementing, executing or reviewing the items; fresh independent session with no prior author context. Read-only for the repository: only this report file was created; no tracked file was modified. ctest/run_sim/trace read from `build/` (git-ignored). |
| Norms applied                     | N1–N20 (see `docs/rules.md`); focus N2, N4, N8, N9, N11, N15, N18 |

## 1. Scope

Reviewed the uncommitted increment adding the reference MSS CAN driver (classic CAN 2.0 A/B) behind `FF_CanBackend_t` (SRS-REQ-057, child of SSS-REQ-020). The review covered implementation correctness (bounds, ID limits, packing/unpacking, MsgAv write-1-to-clear semantics in poll path), host unit test with independent mock (60 assertions), CMake/test registration, Renode `hal_can` integration, traceability/doc updates, and honesty of stated limitations (CAN-FD rejected, reference bit timing GAP-A5, Renode AMR model note).

## 2. References

- Project norms: `docs/rules.md` (N1-N20)
- Traceability manifest: `docs/requirements/traceability.yaml` (SRS-REQ-057)
- HAL contract: `src/hal/hal_can.h`
- Register map/semantics: `src/drivers/can/mpfs/ff_can_mpfs.h`, Microchip PolarFire SoC MSS CAN (TRM/MSS HAL)
- Renode model: `/tmp/mpfs_can.cs` (MPFS_CAN, only AMR bits 0-15 of AMR_DATA implemented; emulated platform used in tests)
- ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025), CCSDS 133.0-B-2

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R1-01 | Comment | SRS-REQ-057, return code semantics | `can_configure()` returns `FF_NOT_IMPLEMENTED` when `fd != 0` (line 64 in `ff_can_mpfs.c`). The header/comment state “CAN-FD is rejected…” which matches the HAL contract semantics. `FF_Status_t` defines `FF_NOT_IMPLEMENTED = -5` (core/common/ff_types.h) which is consistent. No issue; clarity note only. | `ff_can_mpfs.c:52-81`, `ff_can_mpfs.h:5-11`, `core/common/ff_types.h:23-29` | None required. Documented as intended. |
| R1-02 | Minor | Bounds/ID validation | `can_transmit()` checks `id > FF_CAN_STD_ID_MAX` (0x7FF) for std and `id > FF_CAN_EXT_ID_MAX` (0x1FFFFFFF) for ext (line 96). Also checks `len > FF_CAN_MAX_DLC` (8) and NULL/data consistency. This matches classic CAN 2.0 A/B. OK. Consider noting that `ext` is 0/1 (contract passes uint8_t). | `ff_can_mpfs.c:83-113`, `ff_can_mpfs.h:100-109`, `test_can.c:199-203` | Acceptable as-is. No change required. |
| R1-03 | Comment | Packing/unpacking | `can_pack()` packs up to 4 bytes into 32-bit little-endian (low byte -> bits 0-7). TX uses `lo` for data[0..3], `hi` for data[4..7]. RX unpacks symmetrically in the poll loop (lines 167-174). This matches the register layout described (DataLo/DataHi). Test with 4 and 8 bytes confirms. | `ff_can_mpfs.c:27-36, 100-111, 163-174`, `test_can.c:176-196, 213-245` | Correct for 32-bit double-word MMIO view. |
| R1-04 | Minor | RX MsgAv write-1-to-clear semantics | Poll loop writes `(cmd & ~FF_CAN_RX_CMD_MSGAV) | FF_CAN_RX_CMD_MSGAV` to the RX CMD register to acknowledge (lines 181-183), asserting MsgAv bit in the write to clear it (write-1-to-clear). This matches the mock’s behavior (mock clears RX status bit on MsgAv w1c, line 91-97) and is consistent with controller semantics. | `ff_can_mpfs.c:179-186`, `test_can.c:90-97, 224-231` | Correct. No change needed. |
| R1-05 | Minor | AMR/accept-all honesty | Driver sets `AMR = FF_CAN_ACCEPT_ALL` (0x1FFFFFFFu), `AMR_DATA = 0xFFFFu`, ACR/ACR_DATA zero in `can_set_rx_callback()` (lines 124-133). Header documents this is identifier-only mask (RTR/IDE bits 0-2 not implemented in emulated model) and notes benign emulator warning. The implementation is correct for identifier accept-all as modeled. | `ff_can_mpfs.c:124-133`, `ff_can_mpfs.h:111-117, 225-233`, `/tmp/mpfs_can.cs` (AMR field at bits 3-29 in model) | Honesty is good. No code change required. |
| R1-06 | Comment | Bit timing calculation | Reference timing: `tq = 1 + TSEG1 + TSEG2 = 16`, prescaler rounded (`(FF_CAN_MPFS_CLK_HZ + divisor/2)/divisor`). Validates `bitrate != 0`, bounds `presc <= 0x7FFF`, and rejects if `bitrate > FF_CAN_MPFS_CLK_HZ/tq`. Header marks GAP-A5 (on-target confirmation). Logic is sound for a reference. | `ff_can_mpfs.c:50-81`, `ff_can_mpfs.h:31-36, 118-126`, platform `FF_CPU_HZ` context | Acceptable. GAP-A5 is properly tracked. |
| R1-07 | Minor | Poll API contracts | `FF_CanMpfsPoll()` checks `c == NULL` -> `FF_INVALID_ARGUMENT`; allows `got == NULL`. Scans all RX buffers (0..31), extracts ID from RX_ID shifted, DLC from CMD field (masked/clamped to <=8), unpacks data, calls callback if set, then clears MsgAv. Returns `FF_SUCCESS`. Test covers std/ext, multiple frames conceptually via injection, NULL got case. | `ff_can_mpfs.c:135-188`, `test_can.c:212-250, 248-249` | API contract is clear and defensive. |
| R1-08 | Comment | Self-test scope | `FF_CanMpfsSelfTest()` probes command/run, interrupt-enable register (round trip with bits 0 and 11), TX buffer 0 ID register, and RX AMR_DATA (0xFFFF). Intentionally does not touch interrupt status (leaves disabled). Comment explains why AMR_DATA used (emulator limitation). Renode uses same path (`hal_can=PASS`). | `ff_can_mpfs.c:190-234`, `test_can.c:251-255`, `src/apps/app_renode_test.c:236-264` | Honest about emulator limitation; matches test expectations. |
| R1-09 | Comment | Allocation-free/static claims | No `malloc`/dynamic allocation in implementation. All state in `FF_CanMpfs_t` (contains `FF_CanBackend_t` as first member, bus, cb/ctx, next_tx). Single global `g_can` bound in `FF_CanMpfsSetup()` (documented as single instance). Matches other drivers’ pattern. | `ff_can_mpfs.c:14, 236-253`, header lines 9, 35 | Claims satisfied. |
| R1-10 | Minor | TX round-robin | `next_tx` initialized to 0 in setup, incremented modulo `FF_CAN_TX_BUFFERS` (32) after each transmit. No checking of TX buffer busy status (no read of TX status register) — driver assumes mission-level flow control or that caller doesn’t overrun; this is a design choice consistent with poll/ISR model documented as mission-specific. | `ff_can_mpfs.c:99, 111`, `ff_can_mpfs.h:133`, test uses single buffer writes | Design choice is acceptable and documented. |
| R1-11 | Minor | App integration | Renode check creates static `FF_CanMpfs_t`, uses `FF_CAN0_BASE` (0x2010C000) with direct MMIO read/write (volatile), calls `FF_CanMpfsSetup()` then `FF_CanMpfsSelfTest()`. Runs after other HAL checks; included in `RENODE_TEST` aggregate. Matches platform header definition. | `src/apps/app_renode_test.c:236-264, 315-334`, `config/platform/platform_mpfs.h:50-53` | Integration is correct and minimal (register-level only, as documented). |
| R1-12 | Minor | Test coverage quality | `test_can` has 60 assertions covering: setup arg validation (NULL cases), init state (run set, ints disabled), configure (zero bitrate invalid, FD rejected, bit-timing fields set), transmit std/ext with correct packing, transmit validation (bad len/data/IDs), set_rx_callback (AMR/ACR/enable), poll with std and ext frames (dispatch, got count, MsgAv cleared, buffer remains enabled), poll arg validation (NULL got allowed, NULL instance rejected), self-test (success + NULL). Mock models TxReq self-clear + sets TX_MSG, MsgAv w1c clears RX status bit. Coverage is meaningful. | `tests/unit/test_can.c:144-255`, `CMakeLists.txt`, `tests/CMakeLists.txt` | Good coverage for register-level behavior. |
| R1-13 | Comment | Style/conventions | Files are clang-format clean. Code follows project style (4 spaces, braces placement, pointer style). Public API uses `FF_` prefix; types as specified. Comments are English and explain rationale (timing, emulator limitation). Header has SPDX + copyright. Include guards present. | `ff_can_mpfs.c/h`, `.clang-format` | Conforms to coding style. |
| R1-14 | Comment | MISRA posture | No MISRA findings reported in new CAN files by cppcheck MISRA addon (per problem statement: 13 findings repo-wide, 0 in new CAN files/test). Implementation uses simple loops, fixed sizes, no dynamic allocation, defensive checks. No VLAs, uses fixed-width types appropriately. | Problem statement + code inspection | MISRA posture acceptable for advisory scan. |
| R1-15 | Minor | Traceability/docs | SRS updated to 1.15 with SRS-REQ-057; traceability.yaml/md updated (counts to 58 srs/99 reqs). SDD matrix includes SRS-REQ-057; VV docs (verification-results 0.29, renode-environment 0.11, tool-qualification 0.3.13) updated; README/CHANGELOG/status/handover updated. All affected docs touched as required (norm N8). `make trace` reports PASS (99/58/27/14/51). | Git diffs + trace output | Traceability is complete and consistent. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### 3.1 Positive verification (no finding)

- Register addresses match MSS CAN map (TX base 0x020, stride 0x10; RX base 0x220, stride 0x20; CMD/ID/DataLo/DataHi/AMR/ACR/AMR_DATA/ACR_DATA) as documented.
- MsgAv acknowledgment preserves RX buffer enable/DLC/IDE bits (masks with `~FF_CAN_RX_CMD_MSGAV` and ORs back only the w1c bit as written) — preserves state correctly.
- TX command sets DLC shift (16), IDE bit (20), TxReq (0); data packing covers both 32-bit words for 8-byte frames.
- NULL callback handling is safe (guarded). Poll handles NULL `got` parameter.
- Renode `hal_can=PASS` observed in `build/renode/console.log`; ctest 19/19 PASS including `test_can` (60 checks).
- clang-format clean across all new/modified C/H files.

## 4. Conclusion

- [x] **APPROVED WITH COMMENTS** - Minor findings tracked to closure (R1-02,R1-04,R1-05,R1-07,R1-10,R1-11,R1-12 are Minor observations; R1-01,R1-03,R1-06,R1-08,R1-09,R1-13,R1-14,R1-15 are Comments only). All Minor items are acceptable as implemented; no code changes required. The implementation satisfies SRS-REQ-057 with documented limitations (GAP-A5) and emulator constraints honestly stated.

**One-line rationale:** Implementation is correct, defensive, allocation-free, matches HAL contract, test coverage is thorough (mock + Renode), traceability/docs are complete and consistent; minor observations are design/clarity notes only.

## 5. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
| -----| ---------------- | ---- | ---------- |
| R1 (SW dev) | Senior FSW reviewer (independent context) | 2026-10-11 | APPROVED WITH COMMENTS |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
| --- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | ------ | --------------- |
| (none) | — | — | — | — | — | — | closed | n/a |

## Evidence consulted

- Commands run (read-only): `git status --short`, `git log --oneline -3`, `git diff` on specified paths, `ls` of new files, `grep` for types/status codes, `clang-format --dry-run -Werror` on 4 files, `ctest --test-dir build/native` (tail), `python3 tools/scripts/trace.py` (tail), `tail` of `build/renode/console.log`.
- Files read: `docs/engineering/independent-review.md`, `AGENTS.md`, `src/drivers/can/mpfs/ff_can_mpfs.{c,h}`, `src/hal/hal_can.h`, `tests/unit/test_can.c`, diffs of platform/CMake/app/test CMake, traceability/SRS/SDD/VV docs, `CHANGELOG.md`, `README.md`, status docs, review template, `/tmp/mpfs_can.cs`, Renode resc files, coding style, driver guide.
- Test results: native ctest 19/19 (including test_can: 60 checks) — PASS; `make trace` PASS (99/58/27/14/51); `clang-format` clean; Renode functional: `hal_can=PASS`, `RENODE_TEST: PASS` in `build/renode/console.log`.

Committed and pushed with the reviewed change (norm N11). (This report is the deliverable; no other tracked files modified.)