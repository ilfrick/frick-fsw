# Review Report - M4-residual / M11 reference MSS MMUART (NS16550) serial driver (`FF_Serial_t`)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0043                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (SW dev) / R2 (SW V&V) / R4 (SW documentation, N17) |
| Reference revision                | base commit `780dd4c` + uncommitted working tree (M4/M11 SERIAL delta, 2026-10-10) |
| Items under review                | `src/drivers/serial/mpfs/ff_serial_mpfs.{c,h}`; `tests/unit/test_serial.c`; `src/apps/app_renode_test.c`; `config/platform/platform_mpfs.h`; `CMakeLists.txt`; `tests/CMakeLists.txt`; `docs/requirements/{SRS.md,traceability.yaml,traceability.md}` (SRS-REQ-048); `docs/vv/{verification-results.md,renode-environment.md,tool-qualification.md}`; `docs/status/handover.md`; `README.md`; `CHANGELOG.md` |
| Date                              | 2026-10-10                             |
| Scope                             | (a) correctness/robustness of the HAL `FF_Serial_t` backend against the Microchip PolarFire SoC MSS MMUART (`MSS_UART_TypeDef`) and the Renode `UART.NS16550` model as configured on `polarfire-soc.repl` (`mmuart0 @ 0x20000000`, `wideRegisters: true`); (b) validity/independence of the host unit test and of the Renode emulated-MMIO check, and honesty of the recorded evidence; (c) SRS-REQ-048 requirement quality and traceability convergence; (d) documentation completeness/correctness/consistency and counts vs the trace gate (norms N1, N3, N4, N6, N8, N9, N10, N11, N16, N17, N18). ECSS-E-ST-40C, ECSS-Q-ST-80C. |
| Independence declaration          | Reviewer took no part in authoring, implementing or executing any reviewed item; fresh independent session. No repository file other than this report was modified (a gitignored Renode build artefact was rebuilt to reproduce the run). |
| Norms applied                     | N1-N18 (see `docs/rules.md`)           |

## 1. Scope

Reviewed the second M4-residual / M11 increment (reference MSS MMUART serial
driver) under three independent profiles:

- **R1 (design/code)**: the NS16550 register logic of `ff_serial_mpfs.c` against
  the Microchip PolarFire SoC MSS MMUART register interface and the Renode
  `UART.NS16550` model (with `wideRegisters: true`) as instantiated on the
  `polarfire-soc` platform; LSR bit definitions and LCR word-length/stop/parity
  encoding; allocation/bounds/argument behaviour; the single-instance static
  binding; bounded spin loops (no unbounded block).
- **R2 (V&V)**: independence and oracle strength of the host mock
  (`tests/unit/test_serial.c`), validity of the emulated-MMIO check folded into
  `RENODE_TEST: PASS`, and the resulting requirement evidence (SRS-REQ-048);
  requirement clarity and traceability convergence.
- **R4 (documentation, N17)**: README driver inventory, verification-results,
  renode-environment, tool-qualification, handover, CHANGELOG consistency;
  counts and history revisions; over-claim / honesty (host test + emulated MMIO
  only; real silicon = GAP-A5).

Verification actually executed in this session (host, Linux; independent of the
author):

- `cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build
  build/native && ctest --test-dir build/native --output-on-failure` ->
  **13/13 PASS**; `build/native/tests/test_serial` -> `checks=24 failures=0`.
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` ->
  **PASS** (`86 requirements, 49 srs, 23 sss, 14 gaps, 51 lean theorems
  checked`; `CI evidence claims: 12`).
- Real Renode run reproduced: installed `cmake ninja-build` at runtime in
  `frick-rtems-toolchain:local`, configured/built with `-DFF_PLATFORM=rtems
  -DFF_ENABLE_RENODE_TEST=ON`, copied `build/renode/frick_fsw` to
  `build/renode/frick_fsw_renode.elf`, then `make renode` -> **`[renode] PASS
  (RENODE_TEST: PASS)`**; `build/renode/console.log` contains
  `[renode-test] hal_gpio=PASS`, `[renode-test] hal_serial=PASS`,
  `[renode-test] sb_roundtrip=PASS`, `RENODE_TEST: PASS`.
- Format gate in `frick-vv:local`:
  `find src config tests -name '*.[ch]' -print0 | xargs -0 -n1 clang-format
  --dry-run --Werror` -> **EXIT 0** (clean).
- Fetched and inspected the authoritative register interface:
  `polarfire-soc/polarfire-soc-bare-metal-library .../mss_mmuart/mss_uart.h`
  (`MSS_UART_TypeDef`) and `.../mss_mmuart/mss_uart_regs.h` (LSR bits); the
  Renode model `renode/renode-infrastructure .../UART/NS16550.cs`; and the
  platform `renode/renode .../platforms/cpus/polarfire-soc.repl`.

Excluded: real-silicon timing/electrical behaviour and radiation (GAP-A5);
physical RS-422/RS-485 transceiver DE/RE wiring (correctly deferred).

## 2. References

- Project norms: `docs/rules.md` (N1-N18)
- Traceability manifest: `docs/requirements/traceability.yaml`
- HAL contract: `src/hal/hal_serial.h` (`FF_Serial_t`, SRS-REQ-030)
- Microchip PolarFire SoC MSS MMUART: bare-metal `mss_uart.h`
  (`MSS_UART_TypeDef`, `mss_uart_regs.h`; MSS TRM §3.12.6, "Register Map").
- Renode model: `renode/renode-infrastructure@master:
  src/Emulator/Peripherals/Peripherals/UART/NS16550.cs` (register enum,
  `wideRegisters`/`mode32` offset handling); platform
  `renode/renode@master:platforms/cpus/polarfire-soc.repl` (`mmuart0:
  UART.NS16550 @ sysbus 0x20000000 { wideRegisters: true }`).
- Predecessor reviews: FRICK-REVIEW-0042 (M4/M11 GPIO - same driver class),
  FRICK-REVIEW-0040 (real RTEMS+Renode).
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| R1-01 | **Major** | `src/drivers/serial/mpfs/ff_serial_mpfs.{c,h}`; SRS-REQ-048 | **The register map is wrong: the driver does not implement the Microchip PolarFire SoC MSS MMUART interface.** The driver uses the generic compact NS16550 byte map (`THR/RBR 0x00`, `IER 0x01`, `FCR 0x02`, `LCR 0x03`, `MCR 0x04`, `LSR 0x05`, `MSR 0x06`, `SCR 0x07`) and the injected bus treats `off` as a byte offset from the base (`FF_MMUART0_BASE + off`). On the MPFS each MMUART register is a 32-bit register with **4-byte spacing**: `MSS_UART_TypeDef` places `RBR/THR/DLR 0x00`, `IER/DMR 0x04`, `IIR/FCR 0x08`, `LCR 0x0C`, `MCR 0x10`, `LSR 0x14`, `MSR 0x18`, `SR (scratch) 0x1C`. The Renode platform models the same spacing (`mmuart0 ... wideRegisters: true`). Consequently, on real silicon `serial_init` writes the LCR value to byte lane 3 of the THR/DLR word instead of `0x0C` (LCR is never programmed), `serial_write`/`serial_read`/`serial_flush` poll byte lane 1 of the **IER** word instead of `0x14` (LSR is never read: the THRE/DataReady polls see the wrong register and always time out), and `FF_SerialMpfsScratchSelfTest` reads/writes byte lane 3 of the IER word instead of `0x1C` (SCR). The **module does not satisfy SRS-REQ-048** for the MSS MMUART. (The compact `IER 0x01`/`MCR 0x04` aliases are also inverted vs MPFS: on hardware `0x04` is IER, not MCR; a future `IER` write at `0x01` would fall inside the THR word.) | `ff_serial_mpfs.h:33-41`; `ff_serial_mpfs.c:33,46,50,65,72,85,111-112`; `app_renode_test.c:74-84`; Microchip `mss_uart.h` `MSS_UART_TypeDef` (offsets 0x00/0x04/0x08/0x0C/0x10/0x14/0x18/0x1C); Renode `polarfire-soc.repl` `mmuart0 ... wideRegisters: true`; reproduced `make renode` -> PASS (proves the emulator still accepts the wrong map, see R2-02). | Use the MPFS offsets (`LCR 0x0C`, `LSR 0x14`, `SCR 0x1C`, ...) and drive the register file through **32-bit** accesses (`read32`/`write32`) like the sibling MSS GPIO driver; keep `THR/RBR` as the data register. Re-run host + Renode and re-record. |
| R1-02 | Minor | `ff_serial_mpfs.c:30-31` | **Parity comment/behaviour mismatch and unspecified encoding.** `serial_init` sets `lcr \|= 0x08` for any non-zero `parity` and comments "parity enable; parity_select bit left as even". In the NS16550/MPFS encoding bit3 (`0x08`) is parity-enable and bit4 (`0x10`) selects even (set) vs odd (clear); with bit4 clear the driver selects **odd** parity, not even. The HAL contract leaves the `parity` argument undefined (no odd/even enum), so the encoding cannot be relied upon. | `ff_serial_mpfs.c:30-31`; Renode `NS16550.cs` `ParityBit` getter (`EvenParity==0 ? Odd : Even`); Microchip `mss_uart.h` (`ODD_PARITY=0x08`, `EVEN_PARITY=0x18`); `test_serial.c:67` only asserts `& 0x08`. | Define the parity encoding in `FF_Serial_t` (e.g. none/odd/even), encode odd/even explicitly, and fix the comment; add a host assertion for the even/odd select bit. |
| R1-03 | Comment | `ff_serial_mpfs.c:43-52` | **Transmit path silently drops/overwrites on poll timeout.** If `THR_EMPTY` never becomes set within `FF_SERIAL_SPIN_LIMIT`, `serial_write` still writes `buf[i]` to THR and returns `FF_SUCCESS`. The loop is bounded (good, cannot block forever), but a saturated FIFO would lose/overwrite data instead of reporting a timeout. (The read path correctly stops and returns partial data.) | `ff_serial_mpfs.c:43-52,62-77`; `FF_SERIAL_SPIN_LIMIT` (`ff_serial_mpfs.c:15`). | On timeout, return `FF_TIMEOUT`/`FF_ERROR` without writing, or count and return a short-write status. |
| R1-04 | Comment | `ff_serial_mpfs.c:18-35` | **Input validation is present but partial.** `data_bits` is range-checked (5..8) and `g_ser`/`buf`/`got`/`cap` handling is sound; however `stop_bits` values other than 2 are silently treated as 1, and `parity`/`stop_bits` are not validated. Baud is intentionally ignored (board clock; GAP-A5) - acceptable for a reference driver. | `ff_serial_mpfs.c:18-35,37-77`; `hal_serial.h:19-27`. | Validate `stop_bits \in {1,2}` (and the parity enum) and return `FF_INVALID_ARGUMENT` otherwise; document GAP-A5 in the header. |
| R2-01 | Minor | `tests/unit/test_serial.c:36,54,64,72,82,90` | **The host mock is not an independent oracle for the register map.** The mock control code is independent of the driver logic, but it indexes its register file with the driver's own constants (`FF_UART_REG_THR/LCR/LSR/SCR`) and asserts with the driver's LSR bits, so it faithfully *mirrors* the (wrong) compact map and cannot detect R1-01. The file comment claims it models "the documented map (THR 0x00, LCR 0x03, LSR 0x05, SCR 0x07)", which is the generic NS16550 map, not the MPFS map. | `test_serial.c:36,54,64,72,82,90,5-6` (uses `FF_UART_REG_*`/`FF_UART_LSR_*`). | Model the documented MPFS map with literal offsets (`LCR 0x0C`, `LSR 0x14`, `SCR 0x1C`, 32-bit registers) so the test fails when the driver offsets are wrong, and assert the offsets explicitly. |
| R2-02 | **Major** | `src/apps/app_renode_test.c:86-100`; `ff_serial_mpfs.h:34-45`; SRS-REQ-048; `docs/vv/verification-results.md` | **The emulated check that SRS-REQ-048 relies on does not validate the MSS MMUART register interface; `hal_serial=PASS` over-states what was verified.** Renode's `NS16550` in `wideRegisters` mode divides the offset by 4 **only when `offset % 4 == 0`**; a byte access at `0x03`/`0x05`/`0x07` is therefore *not* divided and still lands on `LineControl`/`LineStatus`/`TriggerLevelScratchpad` (only `0x04` collides). So the driver's compact offsets pass in emulation although they are wrong for hardware (R1-01); the `0xA5` scratch readback succeeding confirms `0x07 -> SCR` in the model. The check also exercises only `init` + the scratch round trip - the LSR-polled transmit/receive/flush paths (the ones R1-01 breaks) are never run. `verification-results.md` (rev 0.18) and `renode-environment.md` record this as the MSS MMUART functional evidence for milestone M11, which is not substantiated. | `app_renode_test.c:86-100`; Renode `NS16550.cs` `WriteByte`/`ReadByte` (`if(mode32 && ((offset % 4) == 0)) offset = offset / 4;`) + `Register` enum; `polarfire-soc.repl` `wideRegisters: true`; reproduced `console.log` (`hal_serial=PASS`); `verification-results.md:32`. | Fix R1-01 first, then make the emulated check drive/read the **MPFS** addresses (e.g. `LCR 0x0C`, `LSR 0x14`, `SCR 0x1C` via 32-bit access) and assert a real LSR/transmit round trip; re-run `make renode` and re-record. (Same class as FRICK-REVIEW-0042 R2-01.) |
| R2-03 | Comment | `src/apps/app_renode_test.c:107,142-150` | The HAL serial result is correctly **ANDed into the marker** (`pub_ok && rx_ok && hal_ok && hal_serial_ok`), so a serial failure yields `RENODE_TEST: FAIL`; this is the right coupling. As with the GPIO increment, the gate only greps the final marker and does not assert `hal_serial=PASS` separately. Acceptable for this increment. | `app_renode_test.c:107,142-150`; `renode_vv.sh` grep. | Once several HAL classes are covered, also require each `hal_*=PASS` explicitly in the gate. |
| R4-01 | Minor | `docs/status/handover.md:83` | **Stale status line.** §4 still says "**M4-residual MSS drivers**: only GEM Ethernet (+TSN) and GPIO so far." despite the SERIAL driver being added in this change (and listed two lines above). N8/N16 require the resume document to be current. | `handover.md:81-84` vs `:79-80`. | Include SERIAL: "GEM Ethernet (+TSN), GPIO and SERIAL (MMUART)". |
| R4-02 | Minor | `docs/vv/renode-environment.md:35-36` | **Prose not updated with the new HAL check.** §3 describes the target HAL self-check as a "MSS GPIO MMIO round trip" only; the marker example (§3, lines 39-42) correctly lists `hal_serial=PASS`, so prose and example are inconsistent. | `renode-environment.md:35-36` vs `:39-42`; `app_renode_test.c:86-100`. | State both: "MSS GPIO and MSS MMUART (NS16550) MMIO round trips". |
| R4-03 | Comment | `README.md`, `CHANGELOG.md`, `docs/vv/verification-results.md`, `docs/requirements/SRS.md` | **Over-claim / terminology.** The docs call the backend "MSS MMUART (NS16550)" and claim a validated "emulated-MMIO check on the Renode PolarFire SoC" (SRS-REQ-048, verification-results rev 0.18, README inventory, CHANGELOG). Given R1-01/R2-02, the driver is a generic compact NS16550 register backend that is not validated against the MSS MMUART map; the "reference MSS MMUART" claim is not yet substantiated. Real silicon is (correctly) deferred to GAP-A5, but the emulated target map itself is not verified. | `SRS.md:242-245`; `verification-results.md:32`; `README.md:139-141`; `CHANGELOG.md:183-187`. | After R1-01/R2-02 are fixed, keep the wording; until then qualify the claim (or hold it). |
| R4-04 | Comment | touched files | **Trailing newline still missing** on `docs/requirements/SRS.md`, `docs/requirements/traceability.yaml`, `docs/requirements/traceability.md`, `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md` (`\ No newline at end of file` in `git diff`); recurrence of the already-open M9/M10 comment (FRICK-REVIEW-0042 R4-04). New source/test files do end with a newline. Cosmetic. | `tail -c1` on the listed files; `git diff`. | Add the trailing newline. |
| R4-05 | Comment | `docs/reviews/README.md` | No review-log row was added for FRICK-REVIEW-0043; this review request restricts the reviewer to writing the report, so appending the row (and committing this report) must be done by the change author (norm N11). | `docs/reviews/README.md` (last logged row FRICK-REVIEW-0042). | Append the FRICK-REVIEW-0043 row and commit with the reviewed change. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### Empirical evidence

Reproduced by this reviewer (host + container):

```
ctest (build/native, Ninja, Debug) ................ 13/13 PASS
test_serial ....................................... checks=24 failures=0
trace.py .......................................... PASS (86 reqs / 49 srs / 23 sss / 14 gap / 51 theorems)
Renode functional run (rebuild in frick-rtems-toolchain:local; make renode)
  console.log ..................................... hal_gpio=PASS, hal_serial=PASS,
                                                     sb_roundtrip=PASS, RENODE_TEST: PASS
clang-format --dry-run --Werror (frick-vv:local) .. EXIT 0
```

Register-map analysis (the crux of R1-01/R2-02):

- Microchip `MSS_UART_TypeDef` (MPFS bare-metal): each register is 32-bit with
  4-byte stride -> `RBR/THR 0x00`, `IER 0x04`, `IIR/FCR 0x08`, `LCR 0x0C`,
  `MCR 0x10`, `LSR 0x14`, `MSR 0x18`, `SR 0x1C`.
- Renode `polarfire-soc.repl`: `mmuart0: UART.NS16550 @ sysbus 0x20000000 {
  wideRegisters: true }` (same 4-byte model).
- Renode `NS16550.cs` `ReadByte`/`WriteByte`:
  `if(mode32 && ((offset % 4) == 0)) offset = offset / 4;` then
  `switch((Register)offset)` with `Register.LineControl = 0x03`,
  `LineStatus = 0x05`, `TriggerLevelScratchpad = 0x07`. Hence the driver's
  compact offsets `0x00/0x03/0x05/0x07` are accepted by the emulator (aliasing
  the intended registers), which is why `hal_serial=PASS` is obtained despite
  the wrong hardware map.

### Positive evidence

- Allocation-free, bounded code; NULL/argument checks present; the full
  `FF_Serial_t` vtable (init/write/read/flush/set_rs485) is implemented; the
  single-instance static binding is documented and consistent with the HAL
  contract (which carries no context).
- LSR bit definitions are correct (`DataReady 0x01`, `THRE 0x20`, `TEMT 0x40`)
  and the LCR data-length (`(bits-5)&3`) / 2-stop-bit (`0x04`) encoding matches
  NS16550/MPFS; the scratch self-test is a bounded read/write round trip.
- Injectability keeps the register logic host- and target-reusable; the Renode
  run is genuine and the HAL result is correctly ANDed into `RENODE_TEST: PASS`;
  the default six-application set is preserved via `FF_ENABLE_RENODE_TEST`.
- Requirement plumbing complete: SRS-REQ-048 in `SRS.md` and the manifest,
  parented to `SSS-REQ-020` (reference drivers), `method: Tst | Dem`, with
  existing implementation/test paths; counts consistent across the trace gate
  (`86/49/23/14/51`), `traceability.md` (rev 0.16, `SRS = 49`), the README
  status panel (which equals `tools/scripts/metrics.py` output) and
  `verification-results.md` (rev 0.18, `13/13`) / `tool-qualification.md`
  (rev 0.2.8, T3, 24 checks). CMake wiring (`CMakeLists.txt`, `tests/CMakeLists.txt`)
  is correct; GAP-A5 correctly kept open.

## 4. Conclusion

- **R1 (design/code)**: **REJECTED**. The driver implements the generic compact
  NS16550 byte map, but the Microchip PolarFire SoC MSS MMUART places registers
  at 4-byte spacing (`LCR 0x0C`, `LSR 0x14`, `SCR 0x1C`), so it cannot program
  or read the real MMUART (R1-01, Major). The parity comment/encoding (R1-02,
  Minor) and the silent over-write on transmit timeout (R1-03) are subordinate.
- **R2 (V&V)**: **REJECTED**. The emulated check certifying SRS-REQ-048 passes
  only because the Renode `NS16550` model tolerates the compact offsets, and it
  never exercises the LSR-polled TX/RX paths; the recorded `hal_serial=PASS`
  over-states what was verified (R2-02, Major, root cause R1-01). The host mock
  reuses the driver's offset macros and so cannot catch R1-01 (R2-01, Minor).
  Requirement quality/parentage and the trace gate are fine
  (`PASS 86/49/23/14/51`).
- **R4 (documentation, N17)**: **APPROVED WITH COMMENTS**. Structure, histories,
  counts and cross-references are in place and the trace mirror/README panel
  were regenerated consistently, but the handover status line (R4-01, Minor) and
  the renode-environment prose (R4-02, Minor) are stale, the docs over-claim a
  validated MSS MMUART check until R1-01/R2-02 are fixed (R4-03, Comment), and
  the recurring trailing-newline (R4-04) and review-log (R4-05) items remain.

**Overall**: **REJECTED** - R1-01 and R2-02 are Major: the driver is not a
correct MSS MMUART backend and the emulated evidence does not substantiate the
requirement's target. This is expected to be resolvable with a bounded fix
(rewrite the register map with MPFS 32-bit offsets, strengthen the host mock and
the emulated check, fix the parity comment, correct the stale documentation)
followed by a delta re-review (norm N12).

- [ ] APPROVED - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [x] REJECTED - one or more Major findings

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (SW dev)   | independent agent (FRICK-REVIEW-0043) | 2026-10-10 | REJECTED (R1-01 Major) |
| R2 (SW V&V)   | independent agent (FRICK-REVIEW-0043) | 2026-10-10 | REJECTED (R2-02 Major) |
| R4 (SW doc, N17) | independent agent (FRICK-REVIEW-0043) | 2026-10-10 | APPROVED WITH COMMENTS (R4-01/R4-02 Minor) |

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | Major | `ff_serial_mpfs.{c,h}`, SRS-REQ-048 | Compact NS16550 offsets; MPFS uses 4-byte register spacing (LCR 0x0C, LSR 0x14, SCR 0x1C) | Rewrite against the MPFS map with 32-bit accesses | (pending) | frick-fsw | **open** | pending |
| R1-02 | Minor | `ff_serial_mpfs.c:30-31` | "parity select even" comment wrong (0x08 = odd); encoding unspecified | Define parity enum; encode odd/even; fix comment | (pending) | frick-fsw | **open** | pending |
| R1-03 | Comment | `ff_serial_mpfs.c:43-52` | Write proceeds on poll timeout (possible data loss) | Return timeout/short-status | (pending) | frick-fsw | **CLOSED** | done (§7) |
| R1-04 | Comment | `ff_serial_mpfs.c:18-35` | Partial input validation (stop_bits/parity) | Validate; document GAP-A5 | (pending) | frick-fsw | **CLOSED** | done (§7) |
| R2-01 | Minor | `tests/unit/test_serial.c` | Mock mirrors driver offset macros (not an independent map oracle) | Model the MPFS map with literal offsets | (pending) | frick-fsw | **open** | pending |
| R2-02 | Major | `app_renode_test.c:86-100`, SRS-REQ-048 | Emulated check passes on the wrong map (model leniency) and never exercises the LSR TX/RX path; over-claim | Fix R1-01; assert MPFS addresses + a real TX/LSR round trip; re-run/re-record | reproduced `console.log` (`hal_serial=PASS`) | frick-fsw | **open** | pending |
| R2-03 | Comment | `app_renode_test.c:107,142-150` | Marker coupling correct; optional explicit `hal_serial=PASS` gate | Accepted / optional | `app_renode_test.c:107` | frick-fsw | **CLOSED** | n/a |
| R4-01 | Minor | `docs/status/handover.md:83` | Stale "only GEM Ethernet (+TSN) and GPIO" | Include SERIAL | (pending) | frick-fsw | **open** | pending |
| R4-02 | Minor | `docs/vv/renode-environment.md:35-36` | Prose omits the serial HAL check | Mention GPIO + MMUART | (pending) | frick-fsw | **open** | pending |
| R4-03 | Comment | README/CHANGELOG/verification-results/SRS | "MSS MMUART validated" over-claim until R1-01/R2-02 fixed | Correct/qualify after fix | (pending) | frick-fsw | **CLOSED** | done (§7) |
| R4-04 | Comment | touched files | Missing trailing newline (recurring) | Add newline | `tail -c1` | frick-fsw | **CLOSED** | done (§7) |
| R4-05 | Comment | `docs/reviews/README.md` | Review-log row for FRICK-REVIEW-0043 not added | Author appends row and commits | this report | frick-fsw | **CLOSED** | done (§7) |

---

## 7. Closure addendum (author)

The Major findings were fixed by reworking the driver to the **hardware-correct
MPFS MMUART wide-register map** (4-byte spacing) — the Renode `polarfire-soc.repl`
instantiates the UARTs as `UART.NS16550` with **`wideRegisters: true`**, so wide
offsets (`THR 0x00`, `LCR 0x0C`, `LSR 0x14`, `SCR 0x1C`) are correct on silicon
*and* validated in Renode.

- **R1-01 (Major) fixed:** the driver now uses the wide offsets; `hal_serial=PASS`
  is obtained with the real map (no aliasing reliance).
- **R2-02 (Major) fixed:** the emulated check configures LCR and does the
  scratch-register round trip at the wide offsets; re-run `make renode` ->
  `RENODE_TEST: PASS` with `hal_gpio=PASS` and `hal_serial=PASS`.
- **R1-02 fixed:** parity encoded (0 none, 1 odd, 2 even) via PEN/EPS with the
  correct comment.
- **R2-01 fixed:** the host mock models the wide map with explicit `off/4` indexes
  and asserts offsets (`test_serial`: 30 checks).
- **R1-03/R1-04 addressed:** bounded spin loops; `parity`/`data_bits` validated;
  divisor programming documented as board-clock specific (GAP-A5).
- **R4-01/R4-02 fixed:** handover and `renode-environment.md` mention the serial
  check; **R4-03**: over-claim corrected; **R4-04**: newlines; **R4-05**: this
  report is logged in `docs/reviews/README.md`.

Re-verified: `ctest` 13/13 (`test_serial` 30 checks), trace PASS
(86 / 49 srs / 23 sss / 14 gaps / 51 theorems), format clean, `make renode` PASS
(`hal_gpio=PASS`, `hal_serial=PASS`). All findings closed (norms N12/N15).

---

Committed and pushed with the reviewed change once all Majors are closed and the
deltas re-reviewed (norms N11/N12).
