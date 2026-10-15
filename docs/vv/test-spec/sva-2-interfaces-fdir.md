# Interfaces and FDIR Strategy (N22 work package SVA-2)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-SVA-WP2                       |
| Revision                         | 0.1                                    |
| Date                             | 2026-10-14                             |
| Norms                            | N5, N10, N11, N15, N21, N22            |
| Umbrella                         | `docs/vv/system-verification-architecture.md` (FRICK-VV-SVA-001) §2–3 |
| Test cases (oracles)            | work packages `ts-1-*` … `ts-6-*` (`docs/vv/test-spec/README.md`) |
| Levels / selection               | `docs/vv/test-spec/sva-1-levels-vmodel.md` (FRICK-VV-SVA-WP1) |
| Baseline                         | Working tree at HEAD `57974d2` as read 2026-10-14 |

> **Authority statement.** This work package defines the system-level *interface
> verification* and *FDIR test strategy* for the current baseline. It does **not**
> assign mission criticality, development-assurance level, environmental levels,
> radiation/SEE/SEL behaviour, failure-detection coverage/PSF, fail-operational
> status, residual risk, or flight/readiness — those are authority-owned
> (see §6). Nothing here is an independent human approval or a compliance
> claim. This is engineering analysis recorded for disposition under N4/N12.

## 1. Scope, purpose and terminology

This package is the system-side companion to the per-I/O/per-feature test cases.
It states, per interface family, *what the ICD/contract fixes* (units, ranges,
encoding, timeliness, validity flags, error behaviour, endianness, freshness),
*what the nominal and out-of-bound verification objectives are*, and *where the
expected out-of-bound oracle comes from* (so a case is `covered` vs `gap`).

It then defines the **FDIR test strategy** (detection → isolation/containment →
recovery), the **combined interface-coupling scenarios** that must be tested
together, the **fault-injection catalogue**, and the **"cannot verify yet"**
list. Where a contract is absent or ambiguous, the row is `gap` and references an
open NC/TBD (NC-EXT-04, NC-EXT-13, NC-EXT-31); no oracle is invented.

Terminology follows `docs/vv/system-verification-architecture.md` and
`docs/vv/comprehensive-test-spec.md` (N22): *nominal* = in-envelope, in-sequence,
valid data; *out-of-bound (OOB)* = range violation, malformed/missing/invalid
data, prohibited sequencing, resource exhaustion, injected fault; *oracle* = the
explicit expected observable result.

## 2. Interface-coverage (nominal + OOB intent)

The ICD reference is `docs/interfaces/ICD.md` (FRICK-ICD-001). Each row lists the
clause that grounds the oracle; an OOB row without a grounding clause is `gap`.

### 2.1 CCSDS Space Packet & PUS-C

| Interface family | ICD / contract clause | Nominal intent | OOB intent + oracle-ground | Environment | Level |
| --- | --- | --- | --- | --- | --- |
| Primary/secondary header | FRICK-ICD-001 §2; CCSDS 133.0-B-2 §4 | encode/decode round-trip; version/AEC/typed fields | `ts-1` PUS/TIME rows; bad version/AEC, truncated header, length<4 `gap` (NC-EXT-13) | SIL/PIL | L1/L2 |
| Packet Data Length (PDL) | CCSDS 133.0-B-2 §4.1.1.2; SRS-REQ-003 | PDL = total(2 bytes) − 1; encode 0xFFFF | 16-bit wrap, undersized/oversized `ts-1` SRS-REQ-003 rows | SIL | L1 |
| PUS-C service 1/3/5/6/8/9/17/20 | E-ST-70-41C; SRS-REQ-036..061 | accept→verify→ack/complete; HK; ping; mem | unknown service/subservice, bad T-field `gap` (NC-EXT-04) | SIL/PIL | L1/L2 |
| Time (CUC 301.0-B-4) | FRICK-ICD-001 §3; SRS-REQ-024/068 | CUC coarse+fine; gPTP 802.1AS | leap-at-boundary, drift, source loss `ts-1` time rows | SIL/PIL | L1/L2 |
| SDLS / security | E-ST-90.1-B-3; SRS-REQ-044 | auth + encap + replay window | replayed/old packet rejected `ts-2` SDLS rows | SIL/PIL | L1/L2 |

### 2.2 Link layer & segmentation

| Interface family | ICD / contract clause | Nominal intent | OOB intent + oracle-ground | Environment | Level |
| --- | --- | --- | --- | --- | --- |
| Loopback | `src/transport/link/link_loopback.c`; FRICK-ICD-001 §5 | loopback TX→RX round-trip, length mirror | zero-length, oversized `ts-2` SRS-REQ-059 | SIL/PIL | L1/L2 |
| IP (UDP/TCP) | `src/transport/link/link_ip.c`; FRICK-ICD-001 §5.1 | PUS-C round-trip over IP | desync, partial frame `ts-2` SRS-REQ-060 | SIL/PIL | L1/L2 |
| TSN (0x88F5) | `src/transport/link/link_tsn.c`; FRICK-ICD-001 §5.2 | EtherType 0x88F5, 60-byte pad, 16-slot ring | bad PCP/VID, RX over-length `ts-2` SRS-REQ-061 | PIL-Renode | L1/L2 |
| Segmentation/reassembly | `src/transport/link/link_seg.c`; SRS-REQ-059 | 2-byte BE prefix, max 1024, reasm | desync, oversized `ts-2` SRS-REQ-059/060/061 | SIL/PIL | L1/L2 |
| UART | `src/transport/link/link_uart.c`; FRICK-ICD-001 §5 | TM/TC byte-stream framing | offset parse, malformed `ts-2`, `test_netloop` | SIL/PIL | L1/L2 |

### 2.3 HAL peripheral contracts

Contract source: `src/hal/hal_*.h` + each `src/drivers/**/ff_*_mpfs.*`. Per-I/O
cases live in `ts-3-io-a.md` and `ts-4-io-b.md`; this table summarizes the
contract-grounded OOB intents and the open-NC where the contract is silent.

| I/O | Contract header | Nominal intent | OOB intent + oracle-ground | Open NC if silent |
| --- | --- | --- | --- | --- |
| GPIO | `hal_gpio.h` | dir set/read-back | pin ≥14/32/33, bad mode (SRS-REQ-047) | none |
| Serial | `hal_serial.h` | TX/RX byte, 4-byte spacing | data_bits 4/5/8, bad parity/baud (SRS-REQ-048) | none |
| Mem/DMA | `hal_mem.h` | alloc/free, DMA address | pool exhaustion, double free, 64-bit ≥0x10_0000_0000 (SRS-REQ-049) | none |
| GEM Ethernet | `hal_eth.h` | desc ring + MAC | MTU 1536/1537, 64-bit addr (SRS-REQ-036) | none |
| TSN screener | `hal_tsn.h` | VLAN/PCP 802.1Qci | Qav not-implemented path (SRS-REQ-038) → `gap` (documented) | none |
| gPTP | `ff_gptp.h` | 802.1AS BMCA/Sync/PI | msg timeout, bad timestamp (SRS-REQ-068) | none |
| CAN | `hal_can.h` | TX/RX, accept mask | bus-off (NI), NACK, timeout (SRS-REQ-057) → `gap` (bus-off NI) | none |
| I2C | `ff_i2c_mpfs.c` | write/read/write-read | NACK, timeout, divider min (SRS-REQ-058) | none |
| SPI | `ff_spi_mpfs.c` | TXDATA/RXDATA, CS | status timeout, bad framesize (SRS-REQ-069) | none |
| MTD | `ff_mtd_mpfs.c` | byte R/W, sector erase | alignment, OOB address (SRS-REQ-056) | none |
| Block | `ff_block_mpfs.c` | CMD17/18/24/25 | status error, bad count (SRS-REQ-070) | none |
| PCIe | `ff_pcie_mpfs.c` | link probe | link-down, config fault (SRS-REQ-071) → `gap` (link-down behavior) | none |
| SpaceWire | `ff_spacewire_mpfs.c` | contract/register ops | link fault, bad char (SRS-REQ-072) → `gap` | none |
| NVM | `ff_nvm_mpfs.c` | slot write/read | slot overflow, bad id (SRS-REQ-055) | none |

### 2.4 Ground interface (TM/TC byte stream, GSW)

| Interface family | ICD / contract clause | Nominal intent | OOB intent + oracle-ground | Environment | Level |
| --- | --- | --- | --- | --- | --- |
| TM output | FRICK-ICD-001 §4 | packetized TM, monotonic counters | resync, offset parse, malformed `ts-2`, `test_netloop` | SIL/PIL | L2 |
| TC ingest | FRICK-ICD-001 §4 | accept→process→complete | malformed header/LEN `gap` (NC-EXT-04) | SIL/PIL | L1/L2 |
| GSW ground | `tools/gsw/ff_gsw.py` | ping/pong decode | malformed framing (NC-EXT-04) → `gap` | SIL | adjunct |

## 3. Interface-coupled (combined) scenarios

These cross-interface interactions must be tested as **combined** cases (each is
covered by one or more `Scope = combined` rows spread across `ts-1`…`ts-6`; the
table lists the scenario and where its oracles live):

| Scenario (combined) | Interfaces coupled | Oracle location | Note |
| --- | --- | --- | --- |
| TM pipeline | GEM → netif → link_ip → PUS-C → serial TM | `ts-3/ts-1` GEM+netif, `ts-2` TM output | verifies framing across boundaries |
| Command pipeline | TC ingress (IP/TSN/UART) → link_seg → PUS-C accept → SB → app action → TM confirm | `ts-2` seg/IP/TSN, `ts-1` PUS, `ts-5` SB | end-to-end command→telemetry |
| SPI→flash→table | SPI → MTD → TBL (table storage) | `ts-4` SPI/MTD/TBL, `ts-5` TBL staging | validates load/activate boundary |
| NVM persist | TBL/NVM write → power-cycle/reset → read-back | `ts-4` NVM, `ts-5` TBL | `gap`: reset-recovery oracle pending NC-EXT-31 |
| Time sync→TSN | gPTP 802.1AS → TSN screener time-gate | `ts-3` gPTP/TSN | emulator timing not timing evidence |
| I2C sensor→sample→TM | I2C → app_sample → TM output → serial | `ts-4` I2C, `ts-5` apps, `ts-2` TM | end-to-end data path |
| CAN command→exec→HK | CAN → cmd ingest → SB → HS watchdog kick → HK TM | `ts-4` CAN, `ts-5` SB/HS/apps | EVS event-not-raised finding (see §5) |

## 4. FDIR strategy

FDIR verification follows **detect → isolate/contain → recover**, with explicit
separation of detection from response selection and recovery confirmation
(`docs/vv/system-verification-architecture.md` §3). Monitors map to the failure
hypotheses; until those are authority-approved (NC-EXT-31), tests use documented
project assumptions and record the unresolved contract — no invented oracles.

| FDIR stage | Test intent | Oracle | Authority / project ownership |
| --- | --- | --- | --- |
| Detection | assert each monitor trips on its injected fault; measure latency vs threshold + debounce | monitor fires, expected event/counter within latency; `ts-6` watchdog/concurrency rows | project owns monitors/tests; failure hypothesis/PSF owned by authority (NC-EXT-31) |
| Isolation/containment | fault must not spread beyond its fault-containment region; resource limits, partition/mode behavior | only the FCR crosses a boundary; counters/queues bounded | project; FCR model and PSF owned by authority |
| Recovery | safe-state entry, graceful degradation, restart, ground command; no recovery-loop | defined safe state reached; observable; bounded retries | project owns recovery paths; safe-state definition owned by authority |
| Confirmation / escalation | verify recovery took effect; escalate unrecoverable | post-recovery state + TM; escalation to ground/safing | project; abort/recovery policy owned by authority |
| Common cause | shared clock/power/bus/code/tool/compiler analysis | independence argument evidenced | shared-resource analysis: project; independence standard: authority |

**Combined-scenario FDIR**: a fault in one interface must not silently corrupt an
adjacent one — the command pipeline (§3) is the canonical combined-FDIR case.

## 5. Fault-injection catalogue

For each injected fault, the *expected observable* column is the oracle; `gap`
marks faults whose expected platform response is not yet defined by a controlled
clause (NC-linked). Environment: SIL unless marked PIL/HIL.

| Fault injected | Where injected | Expected observable (oracle) | Status | NC / TBD |
| --- | --- | --- | --- | --- |
| Bus-off (CAN) | CAN driver | monitor/event, graceful degradation | `gap` | bus-off not implemented (SRS-REQ-057) |
| Corrupt packet on IP path | link_ip RX | reject + CRC/Checksum fail event | covered | — (`ts-2`, CCSDS CRC) |
| Corrupt packet on TSN path | link_tsn RX | drop frame, length mismatch event | covered | — (`ts-2`) |
| Stale sensor (I2C) | I2C read | stale-data flag / timeout path | covered | — (`ts-4`) |
| Clock loss (gPTP/PPS) | time source | time-validity flag, source-switch | covered | — (`ts-3`, `ts-1`) |
| Reset/brownout mid-TM burst | TM output path | incomplete packet dropped, TM seq correct after | `gap` | reset-recovery oracle pending NC-EXT-31 |
| Table write interrupted (NVM flash) | TBL→NVM write | table remains in prior valid state, rollback | `gap` | interrupted-write handling policy NC-EXT-04/31 |
| Queue/ring saturation | SB/SB-IPC queues | drop oldest or block per policy, counter | covered | — (`ts-5`) |
| Memory pool exhaustion | mem pool allocator | allocation fails graceful, no OOB | covered | — (`ts-6`, `ts-3/4`) |
| Counter wrap (SB stats) | SB counters | wrap mod-2^N, no corruption | covered | — (`ts-5` SRS-REQ-007; `N22-TS5-70`) |
| Descriptor ring overrun (GEM) | GEM RX/TX | descriptor drop + counter, no OOB | covered | — (`ts-3`) |

## 6. Cannot verify yet (explicit limits)

- **Failure hypotheses / detection coverage / PSF** — not yet authority-approved
  (NC-EXT-31); tests use documented project assumptions and record the gap.
- **Silicon-specific behavior** (GEM DMA engine, SD card, PCIe root complex,
  SpaceWire, analog/RAM/ROM ECC) — present on the RTPFS160ZT but absent from the
  Renode/QEMU emulators (GAP-A5). These interfaces are `covered` in the emulator
  for protocol logic only.
- **Reset/brownout recovery and interrupted-write roll-back** — oracles pending
  NC-EXT-04/31; several rows above are `gap` for this reason.
- **Timing as evidence** — emulator timing is not silicon timing evidence
  (NC-EXT-29); timing/determinism evidence requires HIL (GAP-A4).

## 7. Ownership summary

- **Project owns**: interface contracts, nominal + OOB test design, monitors,
  recovery paths, combined-coupling scenarios, tool qualification evidence.
- **Authority owns**: mission criticality / DAL (NC-EXT-11), environmental &
  radiation levels (GAP-A6), failure hypotheses & detection coverage (NC-EXT-31),
  safe-state policy & risk acceptance, and all release/flight-readiness go/no-go.

## History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-14 | frick-fsw | Initial interface-coverage and FDIR-strategy work package for N22; ICD/OOB intents per clause, combined-coupling scenarios, fault-injection catalogue with oracle/gap discipline, limits and ownership summary |
