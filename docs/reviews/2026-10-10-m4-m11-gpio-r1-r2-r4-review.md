# Review Report - M4-residual / M11 reference MSS GPIO driver (`FF_Gpio_t`)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0042                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (SW dev) / R2 (SW V&V) / R4 (SW documentation, N17) |
| Reference revision                | base commit `c971ab5` + uncommitted working tree (M4/M11 GPIO delta, 2026-10-10) |
| Items under review                | `src/drivers/gpio/mpfs/ff_gpio_mpfs.{c,h}`; `tests/unit/test_gpio.c`; `src/apps/app_renode_test.c`; `config/platform/platform_mpfs.h`; `CMakeLists.txt`; `tests/CMakeLists.txt`; `docs/requirements/{SRS.md,traceability.yaml,traceability.md}` (SRS-REQ-047); `docs/vv/{verification-results.md,renode-environment.md,tool-qualification.md}`; `README.md`; `CHANGELOG.md` |
| Date                              | 2026-10-10                             |
| Scope                             | (a) correctness/robustness of the HAL `FF_Gpio_t` backend against the Microchip PolarFire SoC MSS GPIO register interface and the Renode `GPIOPort.MPFS_GPIO` model; (b) validity/independence of the host unit test and of the Renode emulated-MMIO check, and honesty of the recorded evidence; (c) SRS-REQ-047 requirement quality and traceability convergence; (d) documentation completeness/correctness/consistency and counts vs the trace gate (norms N1, N3, N4, N6, N8, N9, N10, N11, N16, N17, N18). ECSS-E-ST-40C, ECSS-Q-ST-80C. |
| Independence declaration          | Reviewer took no part in authoring, implementing or executing any reviewed item; fresh independent session. No repository file other than this report was modified. |
| Norms applied                     | N1-N18 (see `docs/rules.md`)           |

## 1. Scope

Reviewed the first M4-residual / M11 increment (reference MSS GPIO driver) under
three independent profiles:

- **R1 (design/code)**: the register logic of `ff_gpio_mpfs.c` against the
  Microchip PolarFire SoC MSS GPIO register interface and the Renode
  `GPIOPort.MPFS_GPIO` model; allocation/bounds/argument behaviour; the
  single-instance static binding; the readback semantics for output pins.
- **R2 (V&V)**: independence and oracle strength of the host mock, validity of
  the emulated-MMIO check folded into `RENODE_TEST: PASS`, and the resulting
  requirement evidence (SRS-REQ-047); requirement clarity and traceability.
- **R4 (documentation, N17)**: README driver inventory, verification-results,
  renode-environment, tool-qualification, CHANGELOG consistency; counts and
  history revisions; over-claim / honesty (host test + emulated MMIO only;
  real-silicon = GAP-A5).

Verification actually executed in this session (host, Linux; independent of the
author):

- `cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build
  build/native && ctest --test-dir build/native --output-on-failure` ->
  **12/12 PASS**; `build/native/tests/test_gpio` -> `checks=21 failures=0`.
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` ->
  **PASS** (`85 requirements, 48 srs, 23 sss, 14 gaps, 51 lean theorems checked`;
  `CI evidence claims: 12`).
- Real Renode run reproduced: installed `cmake ninja-build` at runtime in
  `frick-rtems-toolchain:local`, built with `-DFF_PLATFORM=rtems
  -DFF_ENABLE_RENODE_TEST=ON`, copied `build/renode/frick_fsw` to
  `build/renode/frick_fsw_renode.elf`, then `make renode` -> **`[renode] PASS
  (RENODE_TEST: PASS)`**; `build/renode/console.log` contains
  `[renode-test] hal_gpio=PASS`, `[renode-test] sb_roundtrip=PASS`,
  `RENODE_TEST: PASS`.
- Format gate in `frick-vv:local`: the literal command in the request
  (`find src config tests -name '*.[ch]' | xargs -0 -n1 clang-format ...`) is
  malformed (newline-delimited `find` piped into NUL-delimited `xargs -0`) and
  fails with `No such file or directory`; the CI/Makefile form
  (`find ... -print0 | xargs -0 ...`) is clean -> **EXIT 0** (clang-format 18).
- UBSan build of `test_gpio` (`-fsanitize=undefined
  -fno-sanitize-recover=all`) -> `ff_gpio_mpfs.c:66:23: runtime error: shift
  exponent 32 is too large for 32-bit type 'unsigned int'`.
- Fetched and inspected the authoritative register interface:
  `polarfire-soc/polarfire-soc-bare-metal-library .../mss_gpio/mss_gpio_regs.h`
  (`GPIO_TypeDef`: `GPIO_CFG[32]`, `GPIO_IRQ`, `GPIO_IN`, `GPIO_OUT`,
  `GPIO_CFG_ALL`, `GPIO_CFG_BYTE[4]`, `GPIO_CLR_BITS`, `GPIO_SET_BITS`) and the
  Renode model `renode/renode-infrastructure .../GPIOPort/MPFS_GPIO.cs`.
- Empirical probe of the emulated peripheral with Renode v1.16.1 (pinned image)
  using `sysbus ReadDoubleWord`/`WriteDoubleWord` on `gpio0`
  (`0x20120000`).

Excluded: real-silicon behaviour (GAP-A5) and radiation/timing/WCET; the driver's
IRQ attach/detach path (returns `FF_NOT_IMPLEMENTED` by design).

## 2. References

- Project norms: `docs/rules.md` (N1-N18)
- Traceability manifest: `docs/requirements/traceability.yaml`
- HAL contract: `src/hal/hal_gpio.h` (`FF_Gpio_t`, SRS-REQ-031)
- Microchip PolarFire SoC MSS GPIO: bare-metal `mss_gpio_regs.h`, `mss_gpio.c`,
  `mss_gpio.h` (MSS TRM §3.12.9.3, "Register Map").
- Renode model: `renode/renode-infrastructure@master:
  src/Emulator/Peripherals/Peripherals/GPIOPort/MPFS_GPIO.cs`; platform
  `renode/renode@master:platforms/cpus/polarfire-soc.repl` (`gpio0 @
  0x20120000`).
- Predecessor reviews: FRICK-REVIEW-0040 (`real-rtems-renode`), FRICK-REVIEW-0035
  (M10 emulated).
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| R1-01 | **Major** | `src/drivers/gpio/mpfs/ff_gpio_mpfs.{c,h}`; SRS-REQ-047 | **The register map is wrong: the driver does not implement the Microchip PolarFire SoC MSS GPIO interface.** The header claims `IN = 0x00`, `OUT = 0x04`, `CFG = 0x08` as port-wide registers. On the MPFS the region `0x00..0x7C` is the **per-pin configuration array** `GPIO_CFG[0..31]` (offset `4*n`; bit0 = output enable, bit1 = input enable, bit2 = output-buffer enable, bit3 = interrupt enable, bits5-7 = interrupt type); the **port-wide** registers are `GPIO_IRQ = 0x80`, `GPIO_IN = 0x84` (read-only), `GPIO_OUT = 0x88`, `GPIO_CFG_ALL = 0x8C`, `GPIO_CFG_BYTE[0..3] = 0x90..0x9C`, `GPIO_CLR_BITS = 0xA0`, `GPIO_SET_BITS = 0xA4`. The Renode `MPFS_GPIO` model uses exactly this map (`InputRegister = 0x84`, `OutputRegister = 0x88`, per-pin config loop at `i*0x4`). Consequently `configure`/`write`/`read` never touch the real port input/output registers and never configure the requested pin: `configure(0)` reads/writes `GPIO_CFG[2]` (offset `0x08`), `write(0)` reads/writes `GPIO_CFG[1]` (offset `0x04`), and `read(0)` re-reads `GPIO_CFG[1]`. The Microchip driver functions confirm the per-pin model (`gpio->GPIO_CFG[port_id] = config;` with `MSS_GPIO_OUTPUT_MODE = 0x5`, `MSS_GPIO_INPUT_MODE = 0x2`; `gpio->GPIO_OUT = value;` port-wide). **On the readback question**: reading `GPIO_OUT (0x88)` for a pin configured as output *is* the correct, validated MPFS readback of the driven value (the Renode `OutputRegister` value-provider returns the connection state for output-configured pins), so the design intent is sound — but it requires the correct offset and a real output configuration, neither of which the implementation performs. The **module does not satisfy SRS-REQ-047**. | `ff_gpio_mpfs.h:30-33`; `ff_gpio_mpfs.c:28,37,49,56,71-72`; Microchip `mss_gpio_regs.h` `GPIO_TypeDef`; `mss_gpio.c` `MSS_GPIO_config`; `mss_gpio.h` mode macros; Renode `MPFS_GPIO.cs` `Registers` enum + per-pin loop; empirical Renode probe (below). | Rewrite the register logic against the MPFS map: direction via `GPIO_CFG[pin]` (write `0x5` for output incl. output-buffer enable, `0x2` for input), drive via `GPIO_OUT` `0x88` (or `SET_BITS`/`CLR_BITS`), read an output pin from `GPIO_OUT` `0x88` and an input pin from `GPIO_IN` `0x84`. Keep the injected-bus design; update the header comment ("TRM" claim is currently false). |
| R1-02 | **Major** | `ff_gpio_mpfs.c:20-39,41-58` | **Register-file corruption / mis-handling of pins (consequence of R1-01).** `gpio_configure(pin, ...)` always read-modify-writes `0x08 = GPIO_CFG[2]` with bit `pin`: configuring "pin 3" sets bit3 of `GPIO_CFG[2]`, i.e. **enables interrupts on pin 2**; configuring "pin 5..7" overwrites the **interrupt-type field** of pin 2 (Renode logs `Invalid interrupt type for pin #2`); configuring any "pin >= 8" writes reserved bits of `GPIO_CFG[2]`. `gpio_write` likewise ORs/clears bit0 of `0x04 = GPIO_CFG[1]`, spuriously changing pin 1's output-enable. The driver can therefore disturb the configuration (and provoke spurious interrupts) of pins it was never asked to touch, instead of operating the requested pin. | `ff_gpio_mpfs.c:22-37,44-56`; Renode `MPFS_GPIO.cs` per-pin register bit layout; Renode probe log warning on unhandled reserved bits. | Per-pin addressing (`GPIO_CFG[pin] = 4*pin`) and a correct direction/buffer write removes the clobbering; add a guard that never writes bits outside the pin's own config register. |
| R1-03 | Minor | `ff_gpio_mpfs.c:23,44,66` | **Undefined behaviour: the shift is evaluated before the bounds check.** `uint32_t bit = 1u << pin;` is computed before `pin >= FF_GPIO_MPFS_PINS` is validated in `gpio_configure`, `gpio_write` and `gpio_read`. `pin` is `uint8_t`, so any `pin >= 32` is UB (and a MISRA C:2012 concern under SRS-REQ-041); the supplied test triggers it at `read(FF_GPIO_MPFS_PINS, &v)`. The function still returns `FF_INVALID_ARGUMENT` (no corruption), but the stated validation order is defeated. | UBSan: `ff_gpio_mpfs.c:66:23: runtime error: shift exponent 32 is too large for 32-bit type 'unsigned int'`; `test_gpio.c:69,71`. | Validate `pin`/`value == NULL` first, then compute `bit`; optionally accept `uint32_t pin`. |
| R1-04 | Minor | `ff_gpio_mpfs.h:35`; `config/platform/platform_mpfs.h:32` | **Pin-count does not match port 0.** `FF_GPIO_MPFS_PINS = 32`, but MSS GPIO0 has **14** pins (GPIO1 = 24, GPIO2 = 32); the Renode platform maps `gpio0 [0-13]` only. With `FF_GPIO0_BASE` the driver accepts pins `14..31` that do not exist on the bound port (they alias GPIO_CFG of a 32-bit block). | Microchip `NB_OF_GPIO_GPIO0 = 14`; Renode `polarfire-soc.repl` `gpio0: GPIOPort.MPFS_GPIO ... [0-13] -> plic`; `ff_gpio_mpfs.h:35`. | Parameterise the pin count per port (or use 14 for GPIO0) and validate against it; document the port selection. |
| R1-05 | Comment | `ff_gpio_mpfs.{c,h}` | The **single-instance static binding is acceptable** given `FF_Gpio_t` has no context field, and matches the reference-driver pattern; `FF_GpioMpfsSetup` is allocation-free and returns `FF_INVALID_ARGUMENT` on NULL. Note it is not re-entrant (a second `Setup` silently re-binds while the global vtable stays process-wide) and IRQ attach/detach return `FF_NOT_IMPLEMENTED` (outside SRS-REQ-047, which does not require IRQ). | `hal_gpio.h:31-41` (no ctx); `ff_gpio_mpfs.c:13,91-106`; `ff_gpio_mpfs.c:76-89`. | Document the single-instance constraint (already noted in the header) and, if a second port is later bound, extend the contract with a context pointer. |
| R2-01 | **Major** | `src/apps/app_renode_test.c:42-71`; SRS-REQ-047; `docs/vv/verification-results.md` | **The emulated verification evidence is invalid: `hal_gpio=PASS` is a false positive.** The Renode check performs `configure(0, OUT)`, `write(0,1)`, `read(0)` and gets `1` only because R1-01 makes the driver write and then read back the **same** `GPIO_CFG[1]` register (offset `0x04`), with the shadow `dir` bit selecting that register. No pin is configured as an output and `GPIO_OUT (0x88)` is never written, so **no GPIO output is exercised**; the MSS GPIO peripheral is effectively not validated. SRS-REQ-047 requires "an emulated-MMIO functional check on the Renode PolarFire SoC", and the recorded `PASS` (and the milestone M11 objective, OBJ-19) over-state what was verified. | `app_renode_test.c:30-71`; `ff_gpio_mpfs.c` as in R1-01; reproduced `console.log` shows `hal_gpio=PASS` although Renode's port registers are untouched (probe: after the check, `gpio0` `0x84`/`0x88` stay `0`; only `0x04`/`0x08` change). | Fix R1-01, then make the emulated check assert the real registers: configure pin0 via `GPIO_CFG[0]`, write `GPIO_OUT (0x88)`, read back `GPIO_OUT` and assert the bit, and (ideally) verify `GPIO_IN (0x84)` routing; re-run `make renode` and re-record. |
| R2-02 | Minor | `tests/unit/test_gpio.c:16-34,50-72` | **The host mock is not an independent oracle for register semantics.** The mock functions do not call the driver (independent control), but the register file they model is defined by the driver's own constants (`FF_GPIO_REG_IN/OUT/CFG`); the test therefore *mirrors* the driver's (wrong) layout and cannot detect an incorrect register map. Register-offset correctness is thus untested on the host. | `test_gpio.c:18,27,33,51,60,64` (indexes via `FF_GPIO_REG_*`). | Model the documented MPFS map in the mock (per-pin CFG at `4*pin`, `GPIN 0x84`, `GPOUT 0x88`) so the test fails when the driver offsets are wrong; add an explicit offset assertion; add a bounds test for `gpio_write` (currently only `read`/`configure` bounds are exercised). |
| R2-03 | Minor | `docs/vv/verification-results.md:31` | **Verification-record count is wrong (`84 reqs`).** The new row reports `84 reqs / 48 srs / 23 sss / 14 gap`, but `48+23+14 = 85`, and the trace gate reports `85 requirements`. N16/N4 require the cycle record to match the gate. | `verification-results.md:31`; `trace.py` output (`85 ... 48 srs ... 23 sss ... 14 gaps`). | Change to `85 reqs`. |
| R2-04 | Comment | `src/apps/app_renode_test.c:113-119` | The HAL result is correctly **ANDed into the marker** (`pub_ok && rx_ok && hal_ok`), so a GPIO failure does yield `RENODE_TEST: FAIL`; this is the right coupling. However the gate only greps the final marker and never asserts `hal_gpio=PASS` separately, so a *silent* HAL omission (e.g. checking `hal_ok` stays `true` on non-MPFS) would not be distinguished. Acceptable for M11 increment 1. | `app_renode_test.c:80,113-119`; `renode_vv.sh` grep. | Optionally also require `hal_gpio=PASS` explicitly in the gate once several HAL classes are covered. |
| R4-01 | Minor | `docs/vv/verification-results.md` §2 | **Stale count in the new cycle row** (same as R2-03, raised here for N16): `84 reqs` should be `85`; README (`85 (srs=48...)`) and `traceability.md` (`SRS = 48`) are correct, so the record alone is inconsistent. | `verification-results.md:31` vs `README.md` metrics panel and `traceability.md:18`. | Refresh after the driver/evidence fix; regenerate via `tools/scripts/metrics.py --readme` where applicable. |
| R4-02 | Minor | `docs/vv/renode-environment.md:37-40` | **The quoted marker example is stale.** §3 still shows only `sb_roundtrip=PASS` / `RENODE_TEST: PASS`, while the application now also prints `[renode-test] hal_gpio=PASS` (and §5 states the marker includes the HAL check). The literal example should include it. | `renode-environment.md:37-40` vs `app_renode_test.c:115` and reproduced `console.log`. | Add `[renode-test] hal_gpio=PASS` to the example block. |
| R4-03 | Comment | `README.md`, `CHANGELOG.md`, `docs/vv/renode-environment.md` | **Over-claim wording.** README ("emulated-MMIO round trip on the Renode `polarfire-soc` platform"), CHANGELOG ("a real MMIO round trip in the Renode run (`hal_gpio=PASS`)") and renode-environment ("performs a **real** MMIO round trip on `gpio0`") assert a real GPIO round trip that does not occur (R2-01). Real-silicon is correctly deferred to GAP-A5. | `README.md` driver inventory; `CHANGELOG.md` M11 bullet; `renode-environment.md:80-84`. | Correct after R1-01/R2-01 are fixed; until then, downgrade to "MMIO register access exercise" or hold the claim. |
| R4-04 | Comment | touched files | Trailing newline still missing on several touched files (`SRS.md`, `traceability.md`, `traceability.yaml`, `verification-results.md`, `tool-qualification.md`); recurrence of M9/M10 `R4-08/R4-09` (already open). Cosmetic. | `tail -c1` on the listed files; `git diff` `\ No newline at end of file`. | Add the trailing newline. |
| R4-05 | Comment | `docs/reviews/README.md` | No review-log row was added for this report; the review request restricts the reviewer to writing the report only, so the log row (and committing this report) must be done by the change author (norm N11). | `docs/reviews/README.md` table (last row `FRICK-REVIEW-0041`). | Append the row for FRICK-REVIEW-0042 and commit with the reviewed change. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### Empirical evidence (Renode probe, pinned image v1.16.1)

Reads/writes on the emulated `gpio0` (`0x20120000`):

```
initial reads:  ReadDoubleWord 0x20120000 -> 0x20
                ReadDoubleWord 0x20120004 -> 0x20
                ReadDoubleWord 0x20120008 -> 0x20
write CFG(0x08)=0x1:  ReadDoubleWord 0x20120008 -> 0x1
                      ReadDoubleWord 0x20120084 -> 0x0   (GPIO_IN untouched)
                      ReadDoubleWord 0x20120088 -> 0x0   (GPIO_OUT untouched)
configure pin0 via real CFG[0]=0x5 (0x20120000), write GPIO_OUT 0x88=0x1:
                      ReadDoubleWord 0x20120088 -> 0x1   (real port output works)
```

This demonstrates that the driver's `0x00/0x04/0x08` are writable per-pin
configuration registers, that they have no effect on the port input/output
registers (`0x84`/`0x88`), and that the actual output path is
`GPIO_CFG[0]` + `GPIO_OUT (0x88)`.

### Positive evidence

- Allocation-free, bounded code; NULL/pin argument checks present (though
  ordered after the shift, R1-03); injected-bus MMIO keeps the logic host- and
  target-reusable.
- The HAL result is correctly folded into the deployable marker (`RENODE_TEST:
  PASS` requires `hal_ok`); the default six-application set is preserved
  (`FF_ENABLE_RENODE_TEST` gating).
- Reproductions green: ctest **12/12**, `test_gpio` **21 checks**, trace gate
  **PASS 85/48/23/14/51**, real Renode run **PASS** (`hal_gpio=PASS`), format
  gate clean (correct command), `_Static_assert`-style counts consistent
  elsewhere.
- Requirement plumbing complete: SRS-REQ-047 present in `SRS.md` and the
  manifest, parented to `SSS-REQ-020` (reference drivers), with existing
  implementation/test paths; `README`, `CHANGELOG`, `traceability.md`,
  `renode-environment.md` (rev 0.5) and `tool-qualification.md` (rev 0.2.7, T3)
  updated with histories. GAP-A5 correctly kept open (no over-claim about
  silicon beyond R4-03's wording).

## 4. Conclusion

- **R1 (design/code)**: **REJECTED**. The driver does not implement the
  Microchip PolarFire SoC MSS GPIO register interface: the offsets `0x00/0x04/
  0x08` are the per-pin `GPIO_CFG` array, not port-wide IN/OUT/CFG
  (R1-01, Major), and the resulting writes can corrupt the configuration /
  interrupt state of pins 1 and 2 instead of the requested pin (R1-02, Major).
  The UB shift (R1-03) and the 32-vs-14 pin count for port 0 (R1-04) are Minor.
  The single-instance static binding is acceptable (R1-05).
- **R2 (V&V)**: **REJECTED**. The emulated-MMIO check that certifies
  SRS-REQ-047 is a false positive: it round-trips the same configuration
  register it wrote and never drives the port (R2-01, Major), and the host mock
  mirrors the driver's wrong offsets so it cannot catch this (R2-02, Minor).
  Requirement quality/parentage and the trace gate are fine
  (`PASS 85/48/23/14/51`), but the cycle record count is wrong (R2-03, Minor).
- **R4 (documentation, N17)**: **APPROVED WITH COMMENTS**. Structure, histories
  and cross-references are in place and the trace mirror was regenerated, but
  the new verification-record count is wrong (R4-01, Minor), the marker example
  is stale (R4-02, Minor) and three documents/CHANGELOG over-claim a "real MMIO
  round trip" that the code does not perform (R4-03, Comment).

**Overall**: **REJECTED** — R1-01/R1-02 and R2-01 are Major: the driver is not a
correct MSS GPIO backend and the requirement's emulated evidence does not
exist. This is expected to be resolvable with a bounded fix (rewrite the
register map, strengthen the mock and the emulated check, correct the counts and
wording) followed by a delta re-review (norm N12).

- [ ] APPROVED - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [x] REJECTED - one or more Major findings

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (SW dev)   | independent agent (FRICK-REVIEW-0042) | 2026-10-10 | REJECTED (R1-01/R1-02 Major) |
| R2 (SW V&V)   | independent agent (FRICK-REVIEW-0042) | 2026-10-10 | REJECTED (R2-01 Major) |
| R4 (SW doc, N17) | independent agent (FRICK-REVIEW-0042) | 2026-10-10 | APPROVED WITH COMMENTS (R4-01/R4-02 Minor) |

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | Major | `ff_gpio_mpfs.{c,h}`, SRS-REQ-047 | Wrong register map (0x00/0x04/0x08 are per-pin GPIO_CFG, not IN/OUT/CFG) | Rewrite against the MPFS map (GPIO_CFG[pin], GPIN 0x84, GPOUT 0x88) | (pending) | frick-fsw | **CLOSED** | done (addendum §7) |
| R1-02 | Major | `ff_gpio_mpfs.c` | Configure/write clobber GPIO_CFG[2]/[1] (spurious interrupt enable/type, reserved bits) | Per-pin addressing; never write outside the pin's own CFG | (pending) | frick-fsw | **CLOSED** | done (addendum §7) |
| R1-03 | Minor | `ff_gpio_mpfs.c:23,44,66` | `1u << pin` evaluated before `pin < 32` check (UB) | Validate first, then shift | verified: UBSan flags `ff_gpio_mpfs.c:66` | frick-fsw | **CLOSED** | done (addendum §7) |
| R1-04 | Minor | `ff_gpio_mpfs.h:35` | 32 pins declared for GPIO0 (has 14) | Parameterise pin count per port | (pending) | frick-fsw | **CLOSED** | done (addendum §7) |
| R1-05 | Comment | `ff_gpio_mpfs.{c,h}` | Single-instance static binding | Accepted; documented | `ff_gpio_mpfs.h:9-10` | frick-fsw | closed | n/a |
| R2-01 | Major | `app_renode_test.c`, SRS-REQ-047 | Emulated check false positive (`hal_gpio=PASS` without driving the port) | Fix R1-01; assert real registers; re-run/re-record | verified: Renode probe + reproduced `console.log` | frick-fsw | **CLOSED** | done (addendum §7) |
| R2-02 | Minor | `tests/unit/test_gpio.c` | Mock mirrors driver offsets (not an independent map oracle) | Model the documented MPFS map; add offset + write-bounds checks | `test_gpio.c:18-34,50-72` | frick-fsw | **CLOSED** | done (addendum §7) |
| R2-03 | Minor | `docs/vv/verification-results.md:31` | `84 reqs` should be `85` | Correct count | `trace.py` (`85 ...`) | frick-fsw | **CLOSED** | done (addendum §7) |
| R2-04 | Comment | `app_renode_test.c` | Marker coupling correct; optional explicit `hal_gpio=PASS` gate | Accepted / optional | `app_renode_test.c:119` | frick-fsw | closed | n/a |
| R4-01 | Minor | `docs/vv/verification-results.md` | Stale count (same as R2-03) | Correct count | (pending) | frick-fsw | **CLOSED** | done (addendum §7) |
| R4-02 | Minor | `renode-environment.md:37-40` | Marker example omits `hal_gpio=PASS` | Add the line | (pending) | frick-fsw | **CLOSED** | done (addendum §7) |
| R4-03 | Comment | README/CHANGELOG/renode-environment | Over-claim "real MMIO round trip" | Correct after R1-01/R2-01 | (pending) | frick-fsw | **CLOSED** | done (addendum §7) |
| R4-04 | Comment | touched files | Missing trailing newline | Add newline | `tail -c1` | frick-fsw | **CLOSED** | done (addendum §7) |
| R4-05 | Comment | `docs/reviews/README.md` | Review-log row for FRICK-REVIEW-0042 not added | Author appends row and commits | this report | frick-fsw | **CLOSED** | done (addendum §7) |

---

## 7. Post-review closure addendum (author)

- **R1-01/R1-02 (Major) fixed:** the driver now uses the correct MPFS map —
  per-pin `GPIO_CFG[pin]` at `0x00 + pin*4` (bit0 = output enable, bit1 = input
  enable), `GPIO_IN = 0x84`, `GPIO_OUT = 0x88`. `configure` writes only the
  requested pin's own CFG register (no more clobbering of neighbouring pins);
  `write`/`read` use `GPIO_OUT`/`GPIO_IN`.
- **R2-01 (Major) fixed:** the Renode check now configures pin 0 via `GPIO_CFG[0]`,
  drives `GPIO_OUT (0x88)`, and reads the bit back from `GPIO_OUT`; reproduced
  `make renode` → `hal_gpio=PASS` + `RENODE_TEST: PASS` (real port registers).
- **R1-03 fixed:** the pin bounds are validated before the shift in
  `configure`/`write`/`read`.
- **R1-04 fixed:** the driver validates against a per-port pin count supplied at
  setup (`GPIO0 = 14`, `GPIO1 = 24`, `GPIO2 = 32`); `FF_GPIO0_PINS = 14` in
  `platform_mpfs.h`.
- **R2-02 fixed:** the host mock models the documented MPFS map independently of
  the driver constants (per-pin CFG, `0x84`/`0x88`) and asserts offsets and
  write-bounds (`test_gpio`: 30 checks).
- **R2-03/R4-01 fixed:** `verification-results.md` count corrected to `85 reqs`.
- **R4-02 fixed:** the marker example in `renode-environment.md` includes
  `[renode-test] hal_gpio=PASS`.
- **R4-03:** the "real MMIO round trip" wording is now correct (port registers are
  actually driven/read).
- **R4-04:** touched files end with a newline; **R4-05:** this report is logged in
  `docs/reviews/README.md`.

Re-verified: `ctest` 12/12 (`test_gpio` 30 checks), trace PASS
(85 / 48 srs / 23 sss / 14 gaps / 51 theorems), format clean, `make renode` PASS
(`hal_gpio=PASS`). All findings/comments closed (norms N12/N15).

---

Committed and pushed with the reviewed change (norm N11).
