# Independent Review — Reference MSS CAN driver (SRS-REQ-057), profile R2 (SW V&V)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                           | frick-fsw                              |
| Document id                       | FRICK-REVIEW-0107                      |
| Reviewer session                  | Independent agent, fresh context (norm N9) |
| Reviewer profile                  | R2 (Senior SW V&V engineer)            |
| Reference revision                | `a95cafb` + uncommitted CAN increment (working tree) |
| Items under review                | `src/drivers/can/mpfs/ff_can_mpfs.{c,h}` (new), `tests/unit/test_can.c` (new), `config/platform/platform_mpfs.h`, `CMakeLists.txt` (driver source), `tests/CMakeLists.txt`, `src/apps/app_renode_test.c` (`renode_hal_can_check()` + `hal_can` marker), V&V docs `docs/vv/verification-results.md` (0.29), `docs/vv/renode-environment.md` (0.11), `docs/vv/tool-qualification.md` (0.3.13), `docs/requirements/traceability.yaml` + `traceability.md` (0.27); requirement **SRS-REQ-057** |
| Date                              | 2026-10-11                             |
| Scope                             | Verification adequacy of the SRS-REQ-057 increment against ECSS-E-ST-40C (verification), ECSS-Q-ST-80C (software PA), ECSS-E-ST-10-02C (verification), ECSS-E-ST-70-41C/CCSDS 133.0-B-2 (context), IEEE 1012 (V&V process framing) |
| Independence declaration          | I did not author, design, execute or disposition this change. I received only the material under review and the repository reference documents; I had no access to the author's conversation, prior reasoning or intent. Every finding below is grounded in an independently reproduced command, a controlled repository artefact, or a publicly available primary source (Renode peripheral model, Microchip MSS CAN API). |
| Norms applied                     | N1–N20 (`docs/rules.md`); focus N4 (traceability), N8 (docs per change), N10/T3+T17 (tool qualification), N11 (evidence), N15 (full cycle), N16 (revision history), N18 (Renode functional validation), N19 (loop audit) |

## 1. Scope

Reviewed: the verification *adequacy* of the new reference MSS CAN driver increment — whether the test evidence actually establishes the claims of SRS-REQ-057; whether the host mock faithfully models the emulated/real controller; whether the Renode check is meaningful and honestly described; whether the committed evidence/records are accurate and internally consistent; and whether the traceability links are correct.

Excluded: R1 code-quality judgement of the driver (structural/style/algorithmic review), the RTEMS/Lean/RTEMS-image build chain, and any R3/PA/R4 remit. I did **not** rebuild the RTEMS image, did not commit or push, and did not modify any tracked file other than this report (build artefacts under `build/` were used privately).

Method: I re-ran the claimed gates from a private build directory, inspected the driver/test/emulator/vendor sources, and diffed the working tree against `a95cafb`.

## 2. References

- Project norms: `docs/rules.md` (N1–N20); `docs/engineering/independent-review.md` (profiles, independence).
- Requirement under review: SRS-REQ-057 (SRS rev 1.15), parent SSS-REQ-020 (*reference low-level implementations*).
- Standards: ECSS-E-ST-40C (software verification/validation), ECSS-Q-ST-80C (software PA, independence), ECSS-E-ST-10-02C (product verification), IEEE 1012 (V&V methods), ECSS-E-ST-70-41C / CCSDS 133.0-B-2 (context only).
- Primary technical references (independently fetched, not taken from the author):
  - Renode peripheral model `src/Emulator/Peripherals/Peripherals/CAN/MPFS_CAN.cs` (renode/renode-infrastructure, MIT) and the pinned image's `platforms/cpus/polarfire-soc.repl` (`can0: CAN.MPFS_CAN @ 0x2010C000`).
  - Microchip `polarfire-soc/platform` `drivers/mss/mss_can/mss_can.h` / `mss_can.c` (the MSS CAN register/filter layout and `MSS_CAN_get_msg_filter_mask()`).
- Gap: `docs/quality/class-a-gaps.md` GAP-A5 (on-target qualification).

## 3. Coverage assessment (SRS-REQ-057 claimed behaviours vs `tests/unit/test_can.c`)

Legend: **C** covered, **W** weakly covered (asserts register state/tautology rather than behaviour), **M** missing.

| # | Claimed behaviour (SRS-REQ-057 / header) | Cov. | Evidence / gap |
| - | ---------------------------------------- | ---- | -------------- |
| 1 | `FF_CanMpfsSetup` bind + NULL argument rejection | C | test lines 153–158; `supports_fd == 0`, `name`, `init` non-NULL |
| 2 | `init()` sets run bit, clears interrupt enable | C | lines 161–163 |
| 3 | `configure()` classic accepted, bit-timing register fields | C | lines 166–175 (SJW/TSEG1/TSEG2/prescaler) |
| 4 | `configure()` CAN-FD rejected | C | line 167 → `FF_NOT_IMPLEMENTED` |
| 5 | `configure()` bitrate = 0 rejected | C | line 166 |
| 6 | `configure()` bitrate > bus clock rejected (`ff_can_mpfs.c:68`) | M | no test; gcov line 68 `#####` |
| 7 | `configure()` prescaler > 0x7FFF rejected (`:76`) / presc==0→1 (`:73`) | M | no test; gcov lines 73, 76 `#####` |
| 8 | transmit standard frame, 4 B, ID/DLC/payload/TX_MSG | C | lines 178–184 |
| 9 | transmit extended frame, 8 B, IDE bit | C | lines 191–196 |
| 10 | transmit `len > 8` rejected | C | line 199 (9 B) |
| 11 | transmit NULL data with `len > 0` rejected | C | line 200 |
| 12 | transmit NULL data with `len == 0` allowed (code path `:100`) | M | no test; 0-length frame untested |
| 13 | transmit standard ID > 0x7FF rejected | C | line 201 |
| 14 | transmit extended ID > 0x1FFFFFFF rejected | C | line 202 |
| 15 | rejected transmit sends nothing | C | `tx_count == 2` after 4 rejects (line 203) |
| 16 | TX buffer round-robin selection and 32-buffer wraparound | W/M | `can_tx_off` 2 calls (gcov); the test asserts frame *content* only, never the buffer index; wraparound at 32 never exercised |
| 17 | `set_rx_callback` stores cb, configures accept-all RX buffer | W | lines 206–210: asserts `AMR == FF_CAN_ACCEPT_ALL` literally (tautology) — **does not verify accept-all behaviour** (see R2-01); ACR/AMR_DATA/ACR_DATA writes not asserted |
| 18 | `FF_CanMpfsPoll` dispatch std frame, clear MsgAv, keep buffer enabled | C | lines 213–232 |
| 19 | `FF_CanMpfsPoll` dispatch ext frame, 8 B, IDE flag | C | lines 235–245 |
| 20 | poll with no message → `got == 0` | C | lines 229–231 |
| 21 | poll NULL instance rejected / NULL counter allowed | C | lines 248–249 |
| 22 | poll return codes for all paths | W | only `FF_SUCCESS` / `FF_INVALID_ARGUMENT`; the `rx_cb == NULL` dispatch path (`:175`) untested |
| 23 | RX `dlc > 8` clamp (`ff_can_mpfs.c:164-166`) | M | no test; gcov line 165 `#####` |
| 24 | RX callback dispatch order across multiple enabled buffers | M | only buffer 0 used; RX_STATUS multi-bit iteration untested |
| 25 | `FF_CanMpfsSelfTest` register round trips + NULL rejection | C | lines 253–254; also exercised in Renode |
| 26 | re-init via `init()` after Setup (idempotency) | M | not exercised |
| 27 | single-instance rebind (`g_can` overwritten by a 2nd Setup) | M | not exercised (documented limitation) |
| 28 | classic 2.0 A/B frame receive on real controller (filtering path) | M | mock has **no acceptance filtering**; emulator has no medium → receive filtering is never validated (R2-01) |

Net: the suite is a good *register-logic* test (60 assertions, all passing), but it does not establish the **receive-filtering** claim, and it leaves several driver branches and error paths unexecuted (12 uncovered lines in `ff_can_mpfs.c`: 41, 68, 73, 76, 91, 120, 165, 203, 207, 214, 222, 228).

## 4. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R2-01 | **Major** | SRS-REQ-057 (frame *receive*); `ff_can_mpfs.c:116,127`; `tests/unit/test_can.c:208` | The "accept-all" RX acceptance mask does **not** accept every identifier. The driver writes `FF_CAN_ACCEPT_ALL = 0x1FFFFFFF` **un-shifted** to the AMR register, while the AMR identifier-mask field is bits **3–31** (29 bits, left-aligned) — the same file left-aligns the ID (`<< FF_CAN_ID_SHIFT`, line 105) but not the mask. Under the mask semantics used by the emulator (`(~AcceptanceMask & (Id ^ AcceptanceCode)) == 0`, mask=1 ⇒ don't-care) the effective mask covers only ID bits 0–25, so extended identifiers with bits 26–28 set (Id ≥ 0x04000000) are silently rejected. The Microchip API confirms the layout twice over: `mss_can_filterobject.AMR { N_A:1; RTR:1; IDE:1; ID:29 }` (bits 3–31) and `MSS_CAN_get_msg_filter_mask()` which left-aligns the ID (`id <<= 3`) and additionally sets the RTR/IDE mask bits — none of which this write does. The header comment stating "the identifier filtering behaves as accept-all either way" is therefore factually wrong for the emulator and the hardware. The **test cannot detect this**: it asserts only the literal constant (`... == FF_CAN_ACCEPT_ALL`) and the mock implements no acceptance filtering at all. This is a latent receive defect that the current V&V gives false confidence in. | `src/drivers/can/mpfs/ff_can_mpfs.c:116,127`; `tests/unit/test_can.c:208`; Renode `MPFS_CAN.cs` (AMR field `WithReservedBits(0,1).WithTag("RTR",1,1).WithTag("IDE",2,1).WithValueField(3,29,...)`, filter `(~AcceptanceMask & (message.Id ^ AcceptanceCode)) == 0`); Microchip `mss_can.h`/`mss_can.c` (`MSS_CAN_get_msg_filter_mask`) | Write the left-aligned don't-care mask (e.g. `(FF_CAN_ID_MASK << FF_CAN_ID_SHIFT) | (RTR_mask | IDE_mask)`, or derive it via the vendor `MSS_CAN_get_msg_filter_mask(0,1,1)` semantics), correct the header comment, and add a test that verifies the mask in the identifier domain (or a mock that models filtering) so the accept-all behaviour — not just the literal write — is verified. Re-run the full N15 cycle. |
| R2-02 | Minor | SRS-REQ-057; T3 | Missing negative/boundary tests for documented driver branches: `configure()` bitrate exceeding the bus clock (`:68`), prescaler overflow (`:76`), presc rounding edge (`:73`); `transmit(id,ext,NULL,0)`; `FF_CanMpfsPoll` with `rx_cb == NULL`; RX `dlc > 8` clamp (`:165`); `init()` idempotency. | gcov `ff_can_mpfs.c.gcov`: lines 68, 73, 76, 165 `#####`; test has no such cases | Add the boundary/negative cases; the file is small and the branches are cheap to reach. |
| R2-03 | Minor | SRS-REQ-057 (round-robin TX) | The round-robin TX cursor is not verified: the test never asserts *which* buffer a frame went to, and the 32-buffer wraparound (and the "don't overwrite a still-pending buffer" concern) is untested. | `tests/unit/test_can.c:178-203` (content only); `ff_can_mpfs.c:99,111` | Assert the buffer index/base per transmit (the mock already keys capture on the write offset) and add a wraparound case. |
| R2-04 | Minor | SRS-REQ-057; T3 (mock fidelity) | The host mock is **more permissive** than the emulated controller in three ways, so a driver defect in those areas would pass: (a) any `TxReq` write is treated as a successful send and always sets TX_MSG, whereas the emulator requires a loaded message (`HasValidData`) and otherwise leaves `IsRequestPending` set and raises no TX_MSG; (b) `RX_STATUS` (0x008) and `TX_STATUS` (0x00C) are **writable** in the mock but read-only in the emulator; (c) RX_CMD and the interrupt-status register accept arbitrary bit writes while the emulator treats several fields as reserved/tagged. | `tests/unit/test_can.c:59-105`; Renode `MPFS_CAN.cs` (`TX_STATUS`/`RX_STATUS` `FieldMode.Read`; `RequestSendingMessage`/`UnloadMessage`) | Harden the mock: ignore writes to the read-only status registers, require data before TX_MSG, and mirror the reserved/tagged fields. |
| R2-05 | Minor | SRS-REQ-057; oracle independence | The oracle is not independent for byte order: both the mock's TX decode (`mock_write32`) and the RX injection helper (`inject_rx`) re-implement the driver's little-endian packing convention, so a systematic endianness/bit-order defect would be invisible. (The convention does happen to match the emulator default `SWAP_ENDIAN=0`, but nothing in the suite proves that independently.) | `tests/unit/test_can.c:74-79,129-134` vs `ff_can_mpfs.c:27-36,169-174` | Add at least one known-answer vector derived from an independent source (vendor/emulator byte order), or assert against literal register words. |
| R2-06 | Minor | N8/N16/N19 (records currency) | The change touched status docs but left them partially stale: `docs/development-plan.md:302` still reads "in progress (5/13 peripheral-bound)" and lists CAN as pending; `docs/status/handover.md:97,106-107` still says "5/13" and lists CAN as remaining; `docs/status/loop-audit.md:34` still says "8 classes pending" and `:49` still counts "98 reqs". The updated rows (`verification-results.md`, `renode-environment.md`, `tool-qualification.md`, `traceability.md`) are internally correct at 6/13, so the drift is cross-document only. | `git diff` of the working tree; files above | Update the three stale status/plan statements to 6/13, CAN done, 7 classes pending, 99 reqs (norm N8/N16/N19). |
| R2-07 | Minor | SRS-REQ-057 wording vs HAL contract | SRS-REQ-057 requires a "HAL CAN backend … frame transmit/receive and set-up", but `FF_CanBackend_t` (`src/hal/hal_can.h`) has no receive operation — receive is exposed only through the driver-specific `FF_CanMpfsPoll`, which the requirement text does not mention. The tests verify the driver-specific path, so the delivered behaviour is real, but requirement-to-contract wording is inconsistent. | `src/hal/hal_can.h:21-30`; `src/drivers/can/mpfs/ff_can_mpfs.h:146-150`; SRS.md SRS-REQ-057 | Clarify the requirement (backend + driver-specific poll) or extend the HAL contract; keep traceability consistent. |
| R2-08 | Observation | N18 / Renode check honesty | The Renode `hal_can` check is honest as written but must not be over-read: `renode_hal_can_check()` proves only that MMIO at `0x2010C000` is mapped to the emulated `CAN.MPFS_CAN` and that three register round trips (Command run bit, Interrupt-enable, TX-buffer-0 ID, RX AMR_DATA) behave. It does **not** exercise TX/RX, acceptance filtering, IRQ, bit timing or off-chip behaviour (no CAN medium is attached; `Config.CFG_BITRATE` is `WithTag`-unimplemented in the model). `renode-environment.md §5` ("MMIO identification/round-trip checks; full driver … path host-tested") and `verification-results.md` ("MMIO register round trips … M11 6/13") state this correctly and bind it to GAP-A5. | `src/apps/app_renode_test.c` `renode_hal_can_check`; `ff_can_mpfs.c:190-234`; `renode-environment.md:91-108`; `verification-results.md:41`; Renode `MPFS_CAN.cs` | No action; retain the GAP-A5 linkage. Note the register check cannot substitute for a data-path test on the emulated controller. |
| R2-09 | Observation | `FF_CanMpfsPoll` robustness | The poll acknowledge writes back the previously-read RX CMD word, so it can clobber a reconfiguration performed inside the RX callback for the same buffer; the callback-dispatch order across multiple buffers and stale/invalid-data handling are also untested. | `ff_can_mpfs.c:178-182` | Consider re-reading/only clearing MsgAv, and add a callback-reentrancy test. |

### Positive verifications (independently reproduced)

Author's recorded evidence was reproduced end-to-end: native ctest **19/19**; `test_can` **60 checks / 0 failures**; ASan+UBSan ctest **19/19**; `trace.py` **PASS (99 reqs / 58 srs / 27 sss / 14 gaps, 51 theorems, 12 CI claims)**; `metrics.py --check` current; `clang-format` clean; `run_sim.sh` smoke **PASS**; Renode `make renode` **PASS** (`hal_can=PASS`, `RENODE_TEST: PASS`) and `make renode-env` **PASS**. The V&V-document edits (verification-results 0.29, renode-environment 0.11, tool-qualification 0.3.13/T3+T17, traceability 0.27) are arithmetically consistent with the tool outputs. The register map, buffer geometry, TX self-clear and MsgAv write-1-to-clear semantics claimed in the header **match the Renode model** (verified against the model source), except for the AMR masking logic of R2-01.

## 5. Conclusion

- [ ] APPROVED
- [ ] APPROVED WITH COMMENTS
- [x] **REJECTED** — one or more Major findings (R2-01)

The increment is well-engineered at the register-logic level and its recorded evidence reproduces cleanly, but SRS-REQ-057's **receive** claim is not established: the accept-all acceptance mask is mis-encoded (R2-01, confirmed against both the Renode model and the Microchip MSS CAN API), and the test suite cannot detect this because the mock implements no filtering and the assertion is a tautology on the written constant. Per norm N12 a REJECTED review blocks commit/push until every Major finding is resolved; the remaining findings are tracked to closure. The fix is small (mask constant + comment + a behaviour-level test), but the verification gap — not just the code defect — must be closed so that a receive-filtering error cannot pass again.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R2 (SW V&V, N9) | Independent agent (fresh context) | 2026-10-11 | REJECTED (R2-01 Major; R2-02..R2-07 Minor; R2-08/R2-09 Observations) |

## 7. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R2-01 | Major | SRS-REQ-057 / `ff_can_mpfs.c:116,127` | accept-all AMR mis-encoded (R2-01) | open — correct mask + comment + behaviour test, re-run cycle | required: re-run `make renode`, `test_can`, `make trace` | frick-fsw | open | pending |
| R2-02 | Minor | SRS-REQ-057 / T3 | missing negative/boundary tests | open | required: new assertions + gcov | frick-fsw | open | pending |
| R2-03 | Minor | SRS-REQ-057 | TX round-robin/wraparound unverified | open | required: buffer-index assertions | frick-fsw | open | pending |
| R2-04 | Minor | T3 (mock fidelity) | mock more permissive than emulator | open | required: mock hardening | frick-fsw | open | pending |
| R2-05 | Minor | T3 (oracle) | byte-order oracle non-independence | open | required: known-answer vector | frick-fsw | open | pending |
| R2-06 | Minor | N8/N16/N19 | stale status/plan docs (5/13, CAN pending, 98 reqs) | open | required: doc edits | frick-fsw | open | pending |
| R2-07 | Minor | SRS-REQ-057 wording | requirement text vs HAL contract boundary | open | required: wording/contract clarification | frick-fsw | open | pending |
| R2-08 | Observation | N18 | Renode check scope (no action) | closed (informational) | — | frick-fsw | closed | — |
| R2-09 | Observation | `FF_CanMpfsPoll` | poll ack clobber / dispatch-order coverage | open | optional | frick-fsw | open | — |

## 8. Evidence consulted (exact commands and results)

Reference revision check:
- `git -C /home/nicola/fsw/frick-fsw status` / `git diff --stat` — branch `main` at `a95cafb`; 15 modified files + untracked `src/drivers/can/`, `tests/unit/test_can.c`.

Verification gates (independently reproduced):
- `cmake -S . -B build/rev-r2-can -DCMAKE_BUILD_TYPE=Debug` → OK; `cmake --build build/rev-r2-can -j8` → OK.
- `ctest --test-dir build/rev-r2-can` → **100% tests passed, 0 failed out of 19**.
- `./build/rev-r2-can/tests/test_can` → `checks=60 failures=0`.
- `cmake -S . -B build/rev-r2-asan -DCMAKE_C_FLAGS="-fsanitize=address,undefined …"` + build + `ctest` → **19/19 passed** (ASan+UBSan).
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` → `CI evidence claims: 12`; `traceability: PASS (99 requirements, 58 srs, 27 sss, 14 gaps, 51 lean theorems checked)`; SRS-REQ-057 → SSS-REQ-020 confirmed.
- `python3 tools/scripts/metrics.py --check` → `metrics panel: current`.
- `clang-format --dry-run --Werror src/drivers/can/mpfs/ff_can_mpfs.{c,h} tests/unit/test_can.c` → clean.
- `bash tools/scripts/run_sim.sh` → `[smoke] PASS (banner + 6 applications registered)`.
- `make renode` (`tools/scripts/renode_vv.sh`) → `[renode] PASS (RENODE_TEST: PASS)`; log contains `[renode-test] hal_can=PASS` and `RENODE_TEST: PASS` (ELF `build/renode/frick_fsw_renode.elf` already present and containing `FF_CanMpfs*` symbols — image not rebuilt).
- `make renode-env` → `[renode-env] PASS: Renode runs and the PolarFire SoC board platform loads`.
- `cat build/coverage/gcov/…ff_can_mpfs.c.gcov` → 12 uncovered lines (41, 68, 73, 76, 91, 120, 165, 203, 207, 214, 222, 228).
- MISRA: `bash tools/scripts/misra.sh` → cppcheck not installed on this host, so **not re-run**; the author's artefact `build/misra-report.txt` (docker `/workspace`) contains no entry for `ff_can_mpfs.c`/`test_can.c` — consistent with, but not independently confirming, the "0 findings" claim.

Primary technical references (independently fetched):
- `docker run --rm antmicro/renode:latest` → `platforms/cpus/polarfire-soc.repl` (`can0: CAN.MPFS_CAN @ sysbus 0x2010C000`) and `platforms/boards/beaglev-fire.repl`.
- `MPFS_CAN.cs` (renode/renode-infrastructure): register map, TX `TxReq`/`HasValidData`/`UnloadMessage`, RX `MsgAv` write-1-to-clear, read-only `TX_STATUS`/`RX_STATUS`, AMR `ID[3:31]`, `AMR_DATA[0:15]`, filter `(~AcceptanceMask & (Id ^ AcceptanceCode)) == 0`.
- Microchip `polarfire-soc/platform` `drivers/mss/mss_can/mss_can.h` (`mss_can_filterobject.AMR {N_A:1;RTR:1;IDE:1;ID:29}`) and `mss_can.c` (`MSS_CAN_get_msg_filter_mask()` left-aligns the ID and sets the RTR/IDE mask bits) — the basis of R2-01.

Documents read: SRS rev 1.15, SSS-REQ-020, `traceability.yaml`/`traceability.md` 0.27, `verification-results.md` 0.29, `renode-environment.md` 0.11, `tool-qualification.md` 0.3.13, `software-design.md` 0.5, `loop-audit.md`, `handover.md`, `development-plan.md`, `class-a-gaps.md` (GAP-A5), `docs/rules.md`, `docs/engineering/independent-review.md`, `docs/reviews/review-template.md`.

---

Uncertainty statement (R2): facts above are traceable to the commands/artefacts listed. The R2-01 rejection rests on the *filter mask semantics* shared by the Renode model and the Microchip MSS CAN driver API; the absolute register-level confirmation on silicon is governed by GAP-A5 and must be closed by a qualified human before flight. `tests/unit/test_can.c` passing proves the driver's register writes under the mock's conditions, not absence of defects in untested behaviours (notably receive filtering). Confidence: **High** for R2-01, R2-02/03/04/05 (reproduced or source-grounded); **Medium** for the cross-document drift in R2-06 (textual evidence only). This report is an AI analysis and carries no approval, qualification or certification authority.
