# Comprehensive Test Specification

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-TSPEC-001                     |
| Revision                         | 0.2                                    |
| Date                             | 2026-10-14                             |
| Norms                            | N6, N10, N11, N15, N21, N22            |
| Applicable standards             | ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2 |
| Baseline                         | Uncommitted working tree at HEAD `57974d2` + current changes |
| Companion                        | `docs/vv/system-verification-architecture.md` (FRICK-VV-SVA-001) |

This document defines the **comprehensive test specification** required by norm
**N22** and by the owner directive that tests shall be thoroughly comprehensive.
It is the controlled test baseline against which the existing suites are mapped
and new tests are designed.

> **Authority statement.** This specification defines verification *methods and
> coverage intent* for the current project baseline. It does not assign mission
> criticality, accept residual risk, qualify hardware/tools, or approve release.
> Those remain with the authorized authorities (see norm N5 applicability note).

## 1. Scope and test philosophy

### 1.1 Mandated comprehensiveness (owner directive)

The test set shall:

1. exercise **all possible I/Os individually and combined** — every HAL
   peripheral contract and every reference driver, alone and in combination;
2. exercise **all of the above in nominal and out-of-bound conditions** —
   nominal (in-envelope) testing and robustness (out-of-bound) testing;
3. exercise **all features implemented up to that point** according to the
   controlled requirements (`docs/requirements/SRS.md`) and the project goals
   (`docs/roadmap.md`, `docs/development-plan.md`);
4. include **software-only tests** that do not require an I/O or a functional
   condition — probing memory safety, overflow protections, and the standard
   robustness checks expected of flight software.

### 1.2 Definitions (aligned with N22)

- **Nominal test** — applies valid inputs, states, sequencing and interface
  conditions within the specified operating envelope and checks the specified
  observable result.
- **Out-of-bound / robustness test** — applies conditions outside that envelope:
  range-violating or malformed inputs, stale/invalid data, prohibited
  sequencing, resource exhaustion, interface faults, and injected failures. The
  expected outcome (rejection, saturation, safe state, error code) must be
  defined by the requirement or interface contract before it can serve as an
  oracle. Where the contract is silent, the case is recorded as an NC/TBD, not
  invented.
- **Oracle** — the concrete expected observable result (return value, output
  buffer bytes, state change, emitted packet, error code, timing bound), not a
  mere "PASS" marker or absence of a crash.
- **I/O** — a peripheral interface exposed by a HAL contract
  (`src/hal/hal_*.h`) or a reference driver (`src/drivers/**`).

### 1.3 Test environments

| Environment | Abbreviation | Establishes | Cannot establish |
| --- | --- | --- | --- |
| Native host simulator | SIL | Logic, algorithms, protocol encode/decode, bounds, memory-safety tooling (ASan/UBSan) | Real interrupt/timing, device register behavior |
| Renode emulated PolarFire SoC (`beaglev-fire`) | PIL | Boot, HAL register round-trips, RTEMS integration, aggregate functional markers | DMA data paths the model omits, silicon analog/electrical, real timing |
| QEMU `virt` (generic RISC-V) | PIL | Cross-BSP portability, generic boot | MPFS-specific peripherals |
| Target silicon (RTPFS160ZT) | HIL | Device+board+environment behavior, WCET, radiation | (gated by availability; NC-EXT-31) |

Per N22, evidence from an emulator (Renode/QEMU) establishes only the behavior
that the model represents and the test exercises; it does not substitute for
target evidence where hardware-specific behavior is material (GAP-A5).

## 2. Requirements traceability base

The traceability manifest (`docs/requirements/traceability.yaml`) currently
converges at **117 requirements (73 SRS, 30 SSS, 14 GAP), 51 Lean theorems**
(`make trace` PASS). Every SRS requirement must map to at least one nominal and
(at most) the applicable out-of-bound test cases below.

## 3. Test categories

| ID | Category | Purpose | Primary environment |
| -- | -------- | ------- | ------------------- |
| TC-REQ | Requirement-based | One case per SRS clause; nominal + out-of-bound; explicit oracle | SIL (+PIL where interface-bound) |
| TC-INT | Integration | Component↔component and component↔platform across defined interfaces | SIL + PIL |
| TC-IO | I/O unit & combined | Each HAL/driver I/O individually, then combined | SIL (mock MMIO) + PIL |
| TC-MEM | Memory safety | Overflow, use-after-free, double-free, OOB, uninitialized, alignment, stack | SIL with ASan/UBSan + static analysis |
| TC-NUM | Numeric robustness | Integer wrap, underflow, saturation, NaN/Inf, cancellation, quantization | SIL + Lean |
| TC-CONC | Concurrency & timing | Races, deadlock, priority inversion, ISR/task hazards, watchdog | SIL + PIL |
| TC-FAULT | Fault injection | Bus loss, corruption, stale data, clock drift, reset/brownout | SIL + PIL (+HIL) |
| TC-RES | Resource bounds | CPU/WCET, memory pools, queues, stack depth, bandwidth, NVM wear | SIL (measurement) + PID/analysis |
| TC-SEC | Security | Auth, replay, key lifecycle, SDLS, secure update (scope per threat model) | SIL + review |

## 4. I/O coverage matrix (individually and combined)

Each row is exercised **individually** (isolated) and **combined** (with adjacent
subsystems), in **nominal** and **out-of-bound** conditions, with explicit oracles.

| I/O (HAL contract / driver) | SRS | Individual nominal | Individual OOB | Combined scenarios |
| --- | --- | --- | --- | --- |
| GPIO (`hal_gpio.h`, `ff_gpio_mpfs`) | SRS-REQ-047 | dir set/read/read-back | invalid pin, out-of-range mode, MMIO fault | GPIO + scheduler actuation; GPIO + HS watchdog kick |
| Serial/MMUART (`hal_serial.h`, `ff_serial_mpfs`) | SRS-REQ-048 | TX/RX byte, 4-byte spacing | overrun, framing, TX timeout, bad baud | Serial + TM output; Serial + GSW byte stream |
| Static memory pool + DMA (`hal_mem.h`, `ff_mem_static`) | SRS-REQ-049 | alloc/free, DMA identity | pool exhaustion, double free, alignment, OOB | Pool + link RX buffers; Pool + TM queue |
| Ethernet GEM (`hal_eth.h`, `ff_gem`) | SRS-REQ-036 | MODID, MAC addr, desc ring | 64-bit addr ≥ 0x10_0000_0000, bad desc, timeout | GEM + netstack; GEM + lwIP netif |
| Net stack (`hal_net.h`, loopback/lwIP) | SRS-REQ-039/040 | socket loopback round-trip | connect fail, short read, close-during-RX | Netstack + link_ip; netif + GEM |
| TSN (`hal_tsn.h`, `ff_gem_tsn`) | SRS-REQ-038 | VLAN/PCP, 802.1AS TSU | Qav not-implemented path, bad PCP | TSN + link_tsn; TSN + gPTP |
| gPTP (`ff_gptp`) | SRS-REQ-068 | BMCA, Sync/Follow_Up, PDelay, PI servo | message timeout, bad timestamp, seq jump | gPTP + TSN; gPTP + time base |
| CAN (`hal_can.h`, `ff_can_mpfs`) | SRS-REQ-057 | TX/RX round-robin, accept-all mask | bus-off (not implemented), arg fault, timeout | CAN + command ingest |
| I2C (`ff_i2c_mpfs`) | SRS-REQ-058 | write/read/write-read, divider codes | NACK, timeout, arg fault, min-SCL boundary | I2C + sample app sensors |
| SPI (`ff_spi_mpfs`) | SRS-REQ-069 | TXDATA/RXDATA transfer, CS assert/release | status timeout, bad framesize, divider min | SPI + MTD flash |
| MTD NOR/QSPI (`ff_mtd_mpfs`) | SRS-REQ-056 | byte R/W, sector erase | erase-alignment, program fail, OOB address | MTD + table storage |
| Block/SD (`ff_block_mpfs`) | SRS-REQ-070 | CMD17/18/24/25 buffer mode | status error, timeout, bad block count | Block + file/data services |
| PCIe (`ff_pcie_mpfs`) | SRS-REQ-071 | link/register probe | link-down, config fault | PCIe + board init |
| SpaceWire (`ff_spacewire_mpfs`) | SRS-REQ-072 | contract/register | link fault, bad char | SpaceWire + link layer |
| NVM/eNVM (`ff_nvm_mpfs`) | SRS-REQ-055 | slot write/read (32×32 B) | slot overflow, bad id, wear bounds | NVM + table persistence |
| Link backends (`link_*`) | SRS-REQ-059/060/061 | loopback/IP/TSN/UART round-trip | seg desync, partial frame, oversized PUS | Link + PUS dispatch; Link + GSW |

> Combined scenarios must exercise **cross-I/O interactions** (e.g. SPI→MTD→TBL,
> GEM→netif→link_ip→PUS, I2C→sample→TM output→serial) so that interface and
> resource coupling is covered, not only isolated units.

## 5. Feature coverage (all features implemented to date)

Every delivered milestone feature must have requirement-based tests. See
`docs/roadmap.md` milestones M2–M17 and the SRS clause map. Feature groups:

- M2 time base: source selection, 1 Hz tone discipline, leap-at-boundary, CUC.
- M3 HAL registry + contracts.
- M4 drivers: GEM/TSN/gPTP, CAN, I2C, SPI, MTD, Block, PCIe, SpaceWire, NVM,
  loopback/lwIP; RTEMS integration.
- M6 math facade (`ff_math`).
- M7 CCSDS SDLS + SA/key management.
- M8 PUS-C services (1,3,5,6,8,9,17,20 + N7 backend).
- M9 code-quality gates (format, static analysis, coverage).
- M10 emulated targets (Renode + QEMU).
- M11 peripheral binding (13/13 target).
- S4b/S4c/S4d link + PUS-C over IP/TSN, generic fabric, CAN-FD/CANopen/RS-485,
  PUSopen adapter.

## 6. Software-only robustness specification

Software-only tests require **no I/O and no functional precondition**; they probe
implementation robustness directly.

### 6.1 Memory safety (TC-MEM)

| Check | Method | Acceptance oracle |
| --- | --- | --- |
| Heap/stack/global buffer overflow | ASan | No ASan report on any suite |
| Use-after-free / double-free | ASan | No report |
| Uninitialized read | MSan (where available) | No report |
| Undefined behavior (shifts, signed overflow, alignment) | UBSan | No report |
| Out-of-bounds on all public APIs | Boundary tests (min-1/max+1) | Defined error, no OOB access |
| Stack depth vs configured stack | Static analysis + measurement | ≤ configured bound with margin |
| Alignment/aliasing on packed wire structs | Targeted tests | Correct decode, no misaligned load |

### 6.2 Overflow and numeric protections (TC-NUM)

| Check | Requirement | Oracle |
| --- | --- | --- |
| 16-bit PDL widen/wrap | SRS-REQ-003 | Reject undersized/wrapped PDL |
| Counters mod-16384 wrap | SRS-REQ-007 | Bounded wrap, no corruption |
| CUC fine-fraction bound | SRS-REQ-024 | Fits 16 bits, round-trips |
| Fixed-point/math saturation | SRS-REQ-033 | Saturate or reject per contract |
| NaN/Inf policy | numerics policy | Defined propagation/sanitization |

### 6.3 General robustness

- Bounded init (no heap after init, SRS-REQ-015); idempotent re-init.
- Every public API: null pointer, zero length, max length, aliasing.
- Every queue/ring: full, empty, wrap, single-element.
- Defensive deserialization of all wire formats.
- Watchdog: kick cadence, missed-kick escalation.

## 7. Current gap analysis (NC-EXT-47)

Existing suites (`tests/unit/*.c`, 30 files) provide strong requirement-based and
driver coverage. Gaps against this specification are tracked in **NC-EXT-47** and
include (non-exhaustive, from the current tree):

1. **Combined-I/O scenarios** are not systematically defined for all I/O pairs.
2. **Software-only memory-safety** evidence is incomplete: ASan runs are recorded
   for some cycles, but the current committed CTest configuration has no UBSan
   leg. A historical ASan+UBSan result is not reproducible from that
   configuration; neither sanitizer set is yet a fully controlled,
   requirement-mapped oracle set.
3. **Out-of-bound oracles** for some interfaces depend on unresolved contracts
   (e.g. NC-EXT-04 malformed-TC reporting, NC-EXT-13 primary-header policy).
4. **I/O pairs not yet exercised combined**: PCIe/SpaceWire with link layer;
   Block with table storage; CAN/I2C with end-to-end command paths on target.
5. **Target (HIL) evidence** for hardware-specific behavior remains GAP-A5.
6. **Renode aggregate marker** (`RENODE_TEST: PASS`) is not a per-requirement
   oracle; per-check observables must be recorded (this doc supplies them).

These are recorded, not fixed, under N21; they do not stop unrelated development.

## 8. Sprint test plan (S8-VV-FU)

| Priority | Work item | Env | Requirement link |
| --- | --- | --- | --- |
| 1 | Nominal+OOB case table per SRS clause | SIL | N22, SRS-REQ-001..071 |
| 2 | Combined-I/O scenario matrix | SIL+PIL | N22 §4 |
| 3 | Controlled memory-safety/robustness suite | SIL | N22 §6 |
| 4 | Per-check Renode observables (replace aggregate marker) | PIL | N18/N22 |
| 5 | Fault-injection set (bus, clock, reset) | SIL+PIL | N22 TC-FAULT |
| 6 | WCET/stack/resource measurement | PID+analysis | N22 TC-RES |

## 9. Evidence and automation (N11/N6)

- Suites run via `make test` (ctest), `make renode` (N18), `make ci-local`,
  `make qemu`; ASan/UBSan legs as configured.
- Evidence recorded in `docs/vv/verification-results.md`; tools registered in
  `docs/vv/tool-qualification.md` (N10).
- CI legs: GitHub Actions (`.github/workflows/ci.yml`) and Gitea
  (`tools/docker/act_runner` on `frick-vv:local`).

## History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.2 | 2026-10-14 | frick-fsw | Update unit-suite file count after registering isolated SB/EVS structural stress coverage |
| 0.1 | 2026-10-14 | frick-fsw | Initial comprehensive test specification per N22 and owner comprehensiveness directive; defines I/O (individual/combined), nominal/out-of-bound, feature and software-only robustness coverage; maps gaps to NC-EXT-47 |
