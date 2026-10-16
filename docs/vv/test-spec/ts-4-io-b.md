# Test Specification TS-4 — I/O group B (CAN, CAN-FD/CANopen, serial, I2C, SPI, MTD NOR/QSPI, Block/SD, PCIe, SpaceWire, NVM/eNVM, fabric)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-TSPEC-TS4                     |
| Revision                         | 0.8                                    |
| Date                             | 2026-10-15                             |
| Norms                            | N10, N11, N22                          |
| Umbrella                         | `docs/vv/comprehensive-test-spec.md` (FRICK-VV-TSPEC-001) |
| Work-package index               | `docs/vv/test-spec/README.md` (FRICK-VV-TSPEC-002), work package TS-4 |
| Companion                        | `docs/vv/system-verification-architecture.md` (FRICK-VV-SVA-001) |
| Scope                            | I/O group B: MSS CAN (classic), fabric CAN-FD, CANopen subset, MSS RS-485 DE/RE, generic fabric window contract, MSS I2C/SPI/MTD/Block/PCIe/SpaceWire/NVM |
| Baseline                         | Committed HEAD `13ebf92` (S4c/S4d/defaults); GitHub Actions run `38793643786` passed; Gitea Actions status unverified |
| Modules under test               | `src/drivers/can/{mpfs,canopen,fabric}`, `src/drivers/fabric/mpfs/ff_fabric_mpfs.c`, `src/drivers/serial/mpfs/ff_serial_mpfs.c`, and the previously listed group-B modules |
| Interfaces cited                 | `src/hal/hal_can.h`, `src/hal/hal_serial.h`, `src/hal/hal_fabric.h`, plus existing I/O group-B HAL contracts |

> **Authority statement.** This work package defines verification *cases and
> oracles* for the current baseline. It does not assign criticality, accept
> residual risk, qualify tools, close an NC, or approve release/flight
> readiness. Requirement/interface gaps are recorded (never resolved by
> inventing an oracle); disposition and closure require the authorized
> authorities.

## 1. Scope and method

### 1.1 In scope

Cases derived from the following requirements, which are the ones in this
work package's area and present in `docs/requirements/traceability.yaml`:

`SRS-REQ-029` (CAN/CAN-FD/CANopen contract), `SRS-REQ-031` (SPI/I2C/PCIe/GPIO
contracts), `SRS-REQ-032` (SpaceWire/RMAP and memory/storage contracts),
`SRS-REQ-043` (emulated-target execution), `SRS-REQ-044` (Renode functional
environment), `SRS-REQ-055` (reference NVM/eNVM backend), `SRS-REQ-056`
(reference NOR/QSPI MTD backend), `SRS-REQ-057` (reference MSS CAN driver),
`SRS-REQ-058` (reference MSS I2C driver), `SRS-REQ-069` (reference MSS SPI
driver), `SRS-REQ-070` (reference MSS block-device driver), the implemented but
bounded contracts `SRS-REQ-062`/`SRS-REQ-063`/`SRS-REQ-064` (CAN-FD / CANopen
subset / RS-485)
and `SRS-REQ-067` (generic fabric device), `SRS-REQ-025` (HAL registry, used
only for the backend-registration combined cases), and `SSS-REQ-010` (static,
allocation-free storage).

**Requirement-id reconciliation (umbrella §4).** The umbrella `docs/vv/comprehensive-test-spec.md` §4
lists PCIe as `SRS-REQ-071` and SpaceWire as `SRS-REQ-072`. Neither mapping is
usable: `SRS-REQ-071` in `traceability.yaml` is the planned M17 RTEMS-backed
schedule dispatch (not PCIe), and `SRS-REQ-072` **does not exist** in
`traceability.yaml`/`SRS.md`. PCIe cases here are therefore grounded in
`SRS-REQ-031` (the SPI/I2C/PCIe/GPIO contract) and SpaceWire cases in
`SRS-REQ-032` (the SpaceWire/RMAP contract). The umbrella §4 cell values are
recorded as an inconsistency; this work package does not edit the umbrella.

Out of scope (owned by other work packages): I/O group A — GPIO, Serial,
Mem/DMA, GEM Ethernet, netstack/lwIP, TSN, gPTP (TS-3); link layer and SDLS
(TS-2); core services SB/TBL/EVS/ES/scheduler/HS-FDIR/math (TS-5); software-only
robustness — memory safety, numeric, concurrency, fault injection, resources
(TS-6). Where a combined case touches a module from another package, the module
is a fixture and the oracle is grounded only in a requirement of this package.

### 1.2 Class, scope, environment and status conventions

- **Class**: `nominal` = a legal input inside the specified operating envelope
  (this includes legal minimum/maximum boundary values); `OOB` = a
  range-violating, malformed, out-of-envelope, resource-exhausted or
  prohibited-ordering input.
- **Scope**: `individual` = one I/O backend / one module surface is under test
  (other modules are only fixtures); `combined` = the case deliberately asserts
  coupling across two or more modules/backends in one scenario (e.g. SPI + MTD,
  MTD + NVM, Block + PCIe, PCIe + SpaceWire, CAN + HAL registry).
- **Env**: `SIL` = native host unit/integration suite (`make test`, ctest);
  `PIL` = Renode emulated PolarFire SoC; `HIL` = target silicon (RTPFS160ZT).
  Per N22 and FRICK-VV-TSPEC-001 §1.3, an emulator establishes only what its
  model represents.
- **Status**: `covered` = the case is specified with an oracle grounded in a
  controlled requirement or interface clause and is executable in the named
  environment (the test may still have to be added to the suite; execution
  status is tracked by the umbrella and `docs/vv/verification-results.md`).
  `gap` = no controlled oracle exists (the requirement/interface is silent,
  contradictory, or the feature is not delivered), so the case is recorded
  against **NC-EXT-47** and **not** executed to a fabricated oracle.
- **Gap discipline (N22)**: every `OOB` row whose expected behaviour is not
  defined by a requirement/interface clause is `gap`; the oracle cell names the
  open NC/TBD and, where applicable, an uncontrolled *observation* (explicitly
  not an oracle). The Gaps list (§3) maps every gap row to an NC-EXT-47
  sub-item (FRICK-VV-TSPEC-001 §7).

### 1.3 Interface clauses used as oracle ground

| Contract / clause | Content used as oracle ground |
| --- | --- |
| `hal_can.h` (SRS-REQ-029), `ff_canopen.h` (SRS-REQ-063) | `FF_CanBackend_t` vtable `init/configure/transmit/set_rx_callback`, `supports_fd`; `configure(bitrate, fd, data_bitrate)`; `transmit(id, ext, data, len)`; `FF_CanOpenStack_t` and the implemented single-instance CANopen subset (not full CiA 301/302/402 conformance) |
| `ff_can_mpfs.h` (SRS-REQ-057) | `FF_CAN_MPFS_CLK_HZ` 80 MHz; `FF_CAN_SJW` 1, `FF_CAN_TSEG1` 13, `FF_CAN_TSEG2` 2; `FF_CAN_MAX_DLC` 8; 32 TX + 32 RX buffers; `FF_CAN_ID_SHIFT` 3, `FF_CAN_ID_MASK` `0x1FFFFFFF`, `FF_CAN_STD_ID_MAX` `0x7FF`, `FF_CAN_EXT_ID_MAX` `0x1FFFFFFF`; `FF_CAN_ACCEPT_ALL`; Command RUN bit; TX `TxReq`/`DLC`/`IDE`; RX `MsgAv` (w1c)/`RxBufferEbl`; `FF_CanMpfsPoll`; FD → `FF_NOT_IMPLEMENTED` |
| `hal_spi.h` (SRS-REQ-031) | `FF_Spi_t` vtable `init/transfer/cs_assert/cs_release`; `FF_I2c_t` vtable `init/write/read/write_read` |
| `ff_i2c_mpfs.h` (SRS-REQ-058) | `FF_I2C_MPFS_CLK_HZ` 100 MHz; 7-entry divider table (codes, divisors 60..960), `FF_I2C_MIN_SCL_HZ` 104166; `FF_I2C_POLL_MAX` 100000; NACK states `0x20/0x30/0x48`; `FF_I2C_ADDR7_MAX` 0x7F; `FF_I2C_ST_IDLE` 0xF8 |
| `ff_spi_mpfs.h` (SRS-REQ-069) | `FF_SPI_MPFS_CLK_HZ` 100 MHz; SCL = PCLK/(2*(CLKRATE+1)), `FF_SPI_MIN_SCL_HZ` 195312; `FF_SPI_FRAMESIZE_BITS` 8; `FF_SPI_MODE_MAX` 3; `FF_SPI_CS_MAX` 7; `FF_SPI_POLL_MAX` 100000; CONTROL ENABLE/MASTER/MODE; STATUS `RXDATARCVD`/`RXFIFOEMPTY`; CMD `RXFIFORST` |
| `hal_mem.h`, `ff_mtd_mpfs.h` (SRS-REQ-032/056) | `FF_Mtd_t` `size`/`sector_size`/`read`/`write`/`erase`; `FF_MTD_DEFAULT_SECTOR_SIZE` 4096; `FF_MTD_MAX_SIZE` 2 MiB; NOR erase → 0xFF, sector-aligned erase, erase-before-write left to the caller |
| `ff_nvm_mpfs.h` (SRS-REQ-055) | `FF_Nvm_t` `read/write/erase` by id; `FF_NVM_MAX_IDS` 32; `FF_NVM_SLOT_SIZE` 32 (1 length byte + 31 payload); `FF_NVM_MAX_VALUE` 31; corrupt length byte clamped |
| `ff_block_mpfs.h` (SRS-REQ-070) | `FF_BlockDev_t` `block_size`/`num_blocks`/`read`/`write`/`flush`; block size power of two in [512, 2048]; `FF_MMC_BCCT_MAX` 0xFFFF; 32-bit Argument1 LBA; CMD17/18/24/25; `FF_MMC_INT_ERROR_MASK` `0x03FF8000`; `FF_MMC_POLL_MAX` 100000; flush no-op |
| `hal_pcie.h`, `ff_pcie_mpfs.h` (SRS-REQ-031) | `FF_Pcie_t` `init/read_config/write_config/msi_enable/msi_disable`; ECAM offset `(bus<<20)\|(dev<<15)\|(fn<<12)\|off`; only bus/dev/fn 0,0,0 accepted; `rc < 2`; MSI stubbed → `FF_NOT_IMPLEMENTED` |
| `hal_spacewire.h`, `ff_spacewire_mpfs.h` (SRS-REQ-032) | `FF_SpaceWire_t` `init/send/send_pid/set_rx_callback`; `FF_Rmap_t` `init/read/write/rmw`; `FF_SPW_MAX_PAYLOAD` 256; `FF_RMAP_HEADER_SIZE` 16; protocol IDs CCSDS 0x01 / RMAP 0x02; CRC-16 CCITT (poly 0x1021, init 0xFFFF); default target 0xFE, key 0x00, reply = logical address |
| `config/platform/platform_mpfs.h` | MSS peripheral base addresses used by the Renode checks (`FF_CAN0_BASE`, `FF_I2C0_BASE`, `FF_SPI0_BASE`, `FF_MMC0_BASE`, `FF_PCIE0_BASE`, eNVM DATA region) |
| `src/apps/app_renode_test.c` (SRS-REQ-043/044) | per-check observables `[renode-test] hal_nvm/hal_can/hal_i2c/hal_spi/hal_block/hal_pcie=…` and `hal_spacewire=CONTRACT`; the aggregate `RENODE_TEST: PASS` line is **not** a per-requirement oracle |

## 2. Test cases

Case IDs `N22-TS4-1` … `N22-TS4-N`; groups: CAN, I2C, SPI, MTD, Block, PCIe,
SpaceWire, NVM, then cross-cutting fabric/environment rows.

| Case | Requirement | Case | Class | Inputs (boundaries) | Expected result (oracle) | Env | Scope | Status |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| N22-TS4-1 | SRS-REQ-057 | `ff_can_mpfs` setup + init (classic CAN 2.0 A/B) | nominal | `FF_CanMpfsSetup(&drv,&bus)`; `can.init()` | setup = `FF_SUCCESS`; `can.name == "mpfs-can"`; `can.supports_fd == 0`; all four vtable entries set; `init()` = `FF_SUCCESS` → Command register `FF_CAN_CMD_RUN` set and InterruptEnable == 0 (SRS-REQ-057) | SIL | individual | covered |
| N22-TS4-2 | SRS-REQ-057 | `ff_can_mpfs` bit-timing configure at standard rates | nominal | `configure(125000,0,0)`, `(250000,0,0)`, `(500000,0,0)`, `(1000000,0,0)`; boundary `bitrate == FF_CAN_MPFS_CLK_HZ/(1+13+2)` | each = `FF_SUCCESS`; CONFIG (`0x018`) SJW field == 1, TSEG2 == 2, TSEG1 == 13 and prescaler field == 40/20/10/5 for the four rates (exact at the 80 MHz reference) (SRS-REQ-057; `ff_can_mpfs.h` bit-timing) | SIL | individual | covered |
| N22-TS4-3 | SRS-REQ-057 | `ff_can_mpfs` transmit classic standard and extended frames; 32-buffer round-robin | nominal | `transmit(0x123,0,d4,4)`; `transmit(0x1ABCDEF,1,d8,8)`; 33 transmits in a row (boundary: buffer 0 and wrap to 0) | each = `FF_SUCCESS`; TX buffer word = `TxReq` + DLC, `IDE` set only for the extended frame, Id register == `id << 3`, data little-endian in DataLo/DataHi; the cursor advances per buffer and wraps at `FF_CAN_TX_BUFFERS` (32) (SRS-REQ-057 bounded 32 TX buffers) | SIL | individual | covered |
| N22-TS4-4 | SRS-REQ-057 | `ff_can_mpfs` RX dispatch via `FF_CanMpfsPoll` (accept-all AMR) | nominal | `set_rx_callback(cb,ctx)`; inject one standard frame (id 0x321, 4 B) and one extended frame (id 0x1ABCDEFF, 8 B); re-poll | AMR == `FF_CAN_ACCEPT_ALL` and `ACR == 0`; `poll` = `FF_SUCCESS`, `*got == 1`, callback receives the exact id/ext/len/data; the buffer `MsgAv` bit is cleared and `RxBufferEbl` retained; a second `poll` returns `*got == 0` (SRS-REQ-057 receive dispatch) | SIL | individual | covered |
| N22-TS4-5 | SRS-REQ-057 | CAN MMIO self-test on the emulated MSS CAN | nominal | Renode: `FF_CanMpfsSelfTest` on MSS CAN at `FF_CAN0_BASE` (Command run bit, Interrupt-enable, TX buffer 0 ID, RX acceptance-mask data round trips; interrupts left disabled) | per-check observable `[renode-test] hal_can=PASS`: every register round trip returns `FF_SUCCESS` and no IRQ is raised (FRICK-VV-TSPEC-001 §7.6: the per-check marker, not `RENODE_TEST: PASS`, is the observable) | PIL | individual | covered |
| N22-TS4-6 | SRS-REQ-057 | `ff_can_mpfs` configure argument boundaries | OOB | `configure(0,0,0)`; `configure(8000000,0,0)` (> clk/16); `configure(100,0,0)` (prescaler > 15 bits); `configure(500000,1,2000000)` (CAN-FD) | `bitrate == 0`, above the bus clock, or needing a > 15-bit prescaler → `FF_INVALID_ARGUMENT`; `fd != 0` → `FF_NOT_IMPLEMENTED` (FD is a fabric device); CONFIG register unchanged on rejection (SRS-REQ-057; `hal_can.h` fd contract) | SIL | individual | covered |
| N22-TS4-7 | SRS-REQ-057 | `ff_can_mpfs` transmit argument boundaries | OOB | `transmit(0x123,0,d8,9)` (len > DLC 8); `transmit(0x123,0,NULL,4)`; `transmit(0x800,0,d4,4)` (std id > 0x7FF); `transmit(0x20000000,1,d4,4)` (ext id > 0x1FFFFFFF); boundary `transmit(0x1,0,NULL,0)` and len == 8 | each invalid call = `FF_INVALID_ARGUMENT` with no TX-buffer write and the cursor unchanged; `len == 0`/NULL and `len == 8` are accepted (SRS-REQ-057; `FF_CAN_MAX_DLC`/id limits) | SIL | individual | covered |
| N22-TS4-8 | SRS-REQ-057 | `ff_can_mpfs` RX DLC clamp and poll argument/NULL-callback behaviour | OOB | inject an RX frame with a DLC field of 15 (corrupt); `FF_CanMpfsPoll(&drv,NULL)`; `FF_CanMpfsPoll(NULL,NULL)`; poll with no callback registered | a DLC field > 8 is clamped to `FF_CAN_MAX_DLC` (8) in the delivered length; `poll(&drv,NULL)` = `FF_SUCCESS`; `poll(NULL,…)` = `FF_INVALID_ARGUMENT`; with no callback the frame is still acknowledged (`MsgAv` cleared) and counted (SRS-REQ-057) | SIL | individual | covered |
| N22-TS4-9 | SRS-REQ-057 | `ff_can_mpfs` bus-off / error-frame / lost-arbitration / RX_MSG_LOSS handling | OOB | force a bus-off/error condition and set InterruptStatus `RX_MSG_LOSS` (bit 10); poll TX/RX | **gap** — `ff_can_mpfs.h` documents "no bus-off / error-frame / lost-arbitration handling and no RX_MSG_LOSS (bit 10) handling"; no SRS/interface clause defines the driver's response. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: the bits are not acted on. No oracle is asserted here | SIL | individual | gap |
| N22-TS4-10 | SRS-REQ-057 | `ff_can_mpfs` TX-pending buffer overwrite (TX status not polled) | OOB | issue 33+ transmits faster than the controller completes them; leave TxStatus showing pending requests | **gap** — the header documents that "the TX status register is not polled, so a still-pending buffer can be overwritten if the caller outruns the controller"; no clause defines flow control or back-pressure. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: the round-robin cursor overwrites buffer 0. No oracle is asserted here | SIL | individual | gap |
| N22-TS4-11 | SRS-REQ-057, SRS-REQ-025 | `ff_can_mpfs` bound through the HAL registry and retrieved by name | nominal | `FF_HalRegister(CAN,"can0",can)`; retrieve by name/class; then `can.transmit(…)` on the retrieved pointer | register = `FF_SUCCESS`; retrieval returns the exact `FF_CanBackend_t` bound by `FF_CanMpfsSetup`; a transmit through the retrieved pointer reaches the same TX buffer (SRS-REQ-057 + SRS-REQ-025 registry) | SIL | combined | covered |
| N22-TS4-12 | SRS-REQ-058 | `ff_i2c_mpfs` setup + init programs the serial-clock divider and ENS1 | nominal | `FF_I2cMpfsSetup(&drv,&bus)`; `init(100000)`; boundary requests 104166 and 2000000 | setup = `FF_SUCCESS`; name "mpfs-i2c"; `init(100000)` → CTRL divider code 4 (PCLK/960, the slowest ≤ 100 kHz request is not available so the min divider is used) and ENS1 set; `init(2000000)` → code 6 (PCLK/60); `ready == 1` (SRS-REQ-058) | SIL | individual | covered |
| N22-TS4-13 | SRS-REQ-058 | `ff_i2c_mpfs` write transaction (START, SLA+W, data, STOP) | nominal | `init(400000)`; `write(0x50,{01,02,03},3)`; boundary `write(0x50,NULL,0)` | each = `FF_SUCCESS`; exactly one START and one STOP; the three data bytes are transmitted in order to the slave register; the bus returns to `FF_I2C_ST_IDLE` (SRS-REQ-058) | SIL | individual | covered |
| N22-TS4-14 | SRS-REQ-058 | `ff_i2c_mpfs` read transaction (SLA+R, ACK all but last) | nominal | `read(0x50,buf,4,&got)`; `read(0x50,buf,1,&got)` (single byte, NACK-last boundary) | = `FF_SUCCESS`; the SLA+R address byte is `(0x50<<1)\|1`; `*got == cap`; bytes match the slave model; the last byte is NACKed and STOP is issued (SRS-REQ-058) | SIL | individual | covered |
| N22-TS4-15 | SRS-REQ-058 | `ff_i2c_mpfs` `write_read` (repeated START, register read) | nominal | `write_read(0x50,{0x10},1,buf,4,&rgot)`; boundary `write_read(0x50,NULL,0,buf,4,&rgot)` | = `FF_SUCCESS`; one START, one repeated START (`FF_I2C_ST_RESTART`), SLA+R, `*rgot == 4`; the `wlen == 0` form delegates to `read` with the same result (SRS-REQ-058) | SIL | individual | covered |
| N22-TS4-16 | SRS-REQ-058 | I2C MMIO self-test on the emulated MSS I2C | nominal | Renode: `FF_I2cMpfsSelfTest` on MSS I2C0 at `FF_I2C0_BASE` (init divider + ENS1, bus idle, ADDR/SLAVE1_ADDR round trips) | per-check observable `[renode-test] hal_i2c=PASS`: ENS1 set, STATUS == `FF_I2C_ST_IDLE`, both slave-address registers round-trip (FRICK-VV-TSPEC-001 §7.6) | PIL | individual | covered |
| N22-TS4-17 | SRS-REQ-058 | `ff_i2c_mpfs` init argument boundaries and below-minimum SCL | OOB | `init(0)`; `init(10000)` (below `FF_I2C_MIN_SCL_HZ` 104166); `write(0x80,…)` (addr > 0x7F); `read` with cap 0 | `init(0)` = `FF_INVALID_ARGUMENT`; a below-minimum request is **not honoured** — the slowest divider (code 4, PCLK/960) is selected and `init` still returns `FF_SUCCESS` (documented contract: caller must request ≥ `FF_I2C_MIN_SCL_HZ`); `addr7 > 0x7F` or `cap == 0` → `FF_INVALID_ARGUMENT` (SRS-REQ-058) | SIL | individual | covered |
| N22-TS4-18 | SRS-REQ-058 | `ff_i2c_mpfs` NACK → `FF_ERROR` | OOB | slave model returns address-NACK (`0x20`) on SLA+W; data-NACK (`0x30`) on a data byte; SLA+R NACK (`0x48`) | each transaction = `FF_ERROR` with STOP still issued on the error path (SRS-REQ-058: NACK → `FF_ERROR`) | SIL | individual | covered |
| N22-TS4-19 | SRS-REQ-058 | `ff_i2c_mpfs` SI-poll timeout → `FF_TIMEOUT` | OOB | mock bus never sets SI (bounded by `FF_I2C_POLL_MAX` 100000) | `write`/`read`/`write_read` = `FF_TIMEOUT` after the bounded poll; STOP is attempted on the timeout path (SRS-REQ-058: SI-poll timeout → `FF_TIMEOUT`) | SIL | individual | covered |
| N22-TS4-20 | SRS-REQ-058 | `ff_i2c_mpfs` pre-init use and NULL out-pointer arguments | OOB | `write`/`read`/`write_read` before `init`; `read(0x50,NULL,…)`; `read(0x50,buf,cap,NULL)`; setup with a missing bus hook | pre-init use → `FF_NOT_READY`; NULL buffer/out-pointer → `FF_INVALID_ARGUMENT`; `FF_I2cMpfsSetup` with NULL `read8`/`write8` hook → `FF_INVALID_ARGUMENT` (SRS-REQ-058; `hal_spi.h` I2C contract) | SIL | individual | covered |
| N22-TS4-21 | SRS-REQ-058 | `ff_i2c_mpfs` STOP completion, bus recovery, clock stretching, arbitration loss | OOB | hold SCL low / never complete STOP / assert arbitration-lost (`0x38`) | **gap** — the header documents "STOP is fire-and-forget: `i2c_stop()` writes STO and does not poll for STOP completion" and "no clock stretching / bus-recovery / arbitration-loss handling"; no clause defines the response. Open TBDs (GAP-A5; NC-EXT-47 §7.3). Uncontrolled observation only: no STOP-completion poll, no recovery sequence. No oracle is asserted here | SIL | individual | gap |
| N22-TS4-22 | SRS-REQ-058, SRS-REQ-069 | I2C + SPI on separate injected buses (register-map isolation) | nominal | one shared mock register bank holding both the CoreI2C byte map (CTRL `0x00`/STATUS `0x04`/DATA `0x08`/ADDR `0x0C`) and the CoreSPI 32-bit map (CONTROL `0x00`/STATUS `0x08`/TXDATA `0x14`/CLKRATE `0x18`); init + transact on each | each backend reflects only its own register map; no cross-talk between the I2C byte accesses and the SPI 32-bit accesses; a write to one map leaves the other unchanged (SRS-REQ-058 + SRS-REQ-069) | SIL | combined | covered |
| N22-TS4-23 | SRS-REQ-069 | `ff_spi_mpfs` setup + init programs CONTROL/FRAMESIZE/CLKRATE | nominal | `FF_SpiMpfsSetup(&drv,&bus)`; `init(1000000,0,0)`; boundary `init(1000000,3,7)` | setup = `FF_SUCCESS`; name "mpfs-spi"; `init(1000000)` → CONTROL ENABLE\|MASTER set, FRAMESIZE == 8, CLKRATE == 49 (PCLK/(2*50)), SSEL == 0; mode 3 / cs 7 accepted (SRS-REQ-069) | SIL | individual | covered |
| N22-TS4-24 | SRS-REQ-069 | `ff_spi_mpfs` full-duplex transfer (TXDATA→RXDATARCVD→RXDATA) | nominal | `cs_assert(0)`; `transfer(tx,rx,3)`; `cs_release(0)` | `transfer` = `FF_SUCCESS`; all three TX frames appear in order on TXDATA; the three received frames are placed in `rx[0..2]`; SSEL is `0x01` during the transfer and `0` after release (SRS-REQ-069) | SIL | individual | covered |
| N22-TS4-25 | SRS-REQ-069 | `ff_spi_mpfs` chip-select assert/release and argument bounds | nominal | `cs_assert(0)`, `cs_assert(7)`; `cs_release(0)`; boundary `cs_assert(7)`/`cs_release(7)` | `cs_assert(n)` writes `1 << n` to SSEL and `cs_release(n)` writes 0; `cs > FF_SPI_CS_MAX` (7) → `FF_INVALID_ARGUMENT` (SRS-REQ-069) | SIL | individual | covered |
| N22-TS4-26 | SRS-REQ-069 | `ff_spi_mpfs` rx-only / tx-only / zero-length transfer and RX drain | nominal | `transfer(NULL,rx,2)`; `transfer(tx,NULL,2)`; `transfer(tx,rx,0)`; `transfer(tx,rx,1)` after a prior transfer | rx-only sends 0x00 frames and fills `rx`; tx-only still reads (and discards) one RXDATA frame per transmitted frame so `RXDATARCVD` is cleared; len 0 = `FF_SUCCESS` with no bus traffic; a following full-duplex transfer is not shifted by a stale frame (SRS-REQ-069; header drain note) | SIL | individual | covered |
| N22-TS4-27 | SRS-REQ-069 | SPI MMIO self-test on the emulated MSS SPI | nominal | Renode: `FF_SpiMpfsSelfTest` on MSS SPI0 at `FF_SPI0_BASE` (CONTROL ENABLE\|MASTER, FRAMESIZE 8, derived CLKRATE 49, STATUS RXFIFOEMPTY; no bus transaction) | per-check observable `[renode-test] hal_spi=PASS`: the register checks pass and no transfer is run (the model has no slave) (FRICK-VV-TSPEC-001 §7.6) | PIL | individual | covered |
| N22-TS4-28 | SRS-REQ-069 | `ff_spi_mpfs` init argument boundaries and minimum-SCL clamp | OOB | `init(0,0,0)`; `init(1000000,4,0)` (mode > 3); `init(1000000,0,8)` (cs > 7); `init(100000,…)` and `init(1,…)` (below `FF_SPI_MIN_SCL_HZ` 195312) | the three argument faults = `FF_INVALID_ARGUMENT`; a below-minimum request selects CLKRATE 255 (PCLK/(2*256)) and `init` still returns `FF_SUCCESS` (documented: caller must request ≥ `FF_SPI_MIN_SCL_HZ`) (SRS-REQ-069) | SIL | individual | covered |
| N22-TS4-29 | SRS-REQ-069 | `ff_spi_mpfs` status-poll timeout and RX FIFO reset | OOB | STATUS never sets `RXDATARCVD` (bounded by `FF_SPI_POLL_MAX` 100000); then a subsequent valid transfer | the transfer = `FF_TIMEOUT`, the CMD `RXFIFORST` bit is written to drain the FIFO, and the following transfer is not shifted by a stale received byte (SRS-REQ-069: status-poll timeout → `FF_TIMEOUT`) | SIL | individual | covered |
| N22-TS4-30 | SRS-REQ-069 | `ff_spi_mpfs` pre-init use | OOB | `transfer(…)` and `cs_assert(0)` before `init` | each = `FF_NOT_READY` (SRS-REQ-069: pre-init use → `FF_NOT_READY`) | SIL | individual | covered |
| N22-TS4-31 | SRS-REQ-069 | `ff_spi_mpfs` STATUS bit-11 name and on-target SCL accuracy | OOB | read STATUS bit 11; measure the achieved SCL at each requested rate on silicon | **gap** — STATUS bit 11's datasheet name is unconfirmed (the Renode field list calls it `TXFIFOFULLNEXT`; the driver does not depend on it) and per-rate/on-target SCL accuracy requires the board (GAP-A5). Open TBD (NC-EXT-47 §7.3/§7.5). Uncontrolled observation only: the driver does not read bit 11. No oracle is asserted here | HIL | individual | gap |
| N22-TS4-32 | SRS-REQ-069, SRS-REQ-056 | SPI + MTD: stage a configuration image over SPI into the NOR window, then read it back through `FF_Mtd_t` | nominal | SPI: `init` + `cs_assert` + `transfer` sends an image byte stream; write those bytes into the MTD window via `mtd.write`; read back via `mtd.read` | the bytes read back through the MTD equal the bytes sent over SPI and written into the window; no heap and no MTD/SPI register aliasing (SRS-REQ-069 + SRS-REQ-056) | SIL | combined | covered |
| N22-TS4-33 | SRS-REQ-056 | `ff_mtd_mpfs` setup + byte read/write round trip | nominal | `FF_MtdMpfsSetup(&drv,&bus,SIZE,4096)`; `write(100,{…},4)`; `read(100,out,4)` | setup = `FF_SUCCESS`; `mtd.size == SIZE`, `sector_size == 4096`; the four bytes read back equal those written (NOR byte-addressed contract; SRS-REQ-056) | SIL | individual | covered |
| N22-TS4-34 | SRS-REQ-056 | `ff_mtd_mpfs` sector erase sets bytes to 0xFF | nominal | `erase(0, 4096)` (one sector); `erase(0, 8192)` (whole device boundary) | each = `FF_SUCCESS`; every byte in the erased range reads 0xFF (NOR erase semantics, SRS-REQ-056) | SIL | individual | covered |
| N22-TS4-35 | SRS-REQ-056 | `ff_mtd_mpfs` setup argument bounds (window size / sector) | OOB | `Setup(&drv,&bus,0,4096)`; `Setup(&drv,&bus,SIZE,0)`; `Setup(&drv,&bus,SIZE+1,4096)` (size not a sector multiple); `Setup(…,(2 MiB)+4096,4096)` (> `FF_MTD_MAX_SIZE`); NULL m/bus | each = `FF_INVALID_ARGUMENT` with the previous binding unchanged (SRS-REQ-056 bounded window) | SIL | individual | covered |
| N22-TS4-36 | SRS-REQ-056 | `ff_mtd_mpfs` address/capacity bounds, erase alignment, NULL/length-0 | OOB | `read(SIZE,out,1)`; `write(SIZE-2,val,4)`; `read(SIZE+1,out,1)`; `read(0,NULL,4)`; `read(0,out,0)`; `erase(1,4096)` (unaligned off); `erase(0,100)` (unaligned len); `erase(0,0)` | out-of-window `off`/`len` → `FF_BAD_SIZE`; NULL buffer or `len == 0` → `FF_INVALID_ARGUMENT`; off/len not a multiple of the sector size → `FF_INVALID_ARGUMENT` (SRS-REQ-056) | SIL | individual | covered |
| N22-TS4-37 | SRS-REQ-056 | MTD scratch self-test (host bring-up check) | nominal | `FF_MtdMpfsSelfTest(&drv)` (write 0x5A at offset 0, read back) | `FF_SUCCESS` and the window byte 0 reads back `0x5A`; `FF_MtdMpfsSelfTest(NULL)` = `FF_INVALID_ARGUMENT` (SRS-REQ-056; `ff_mtd_mpfs.h`) | SIL | individual | covered |
| N22-TS4-38 | SRS-REQ-056 | `ff_mtd_mpfs` NOR program-failure and erase-before-write semantics | OOB | write a byte twice without an intervening erase; force a program-failure status | **gap** — the driver "performs plain byte writes and leaves erase-before-write to the caller" and there is no program-failure status path; SRS-REQ-056 does not define the response and real flash timing/endurance/retention is GAP-A5. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: the second (0→1) write is silently ineffective at the model. No oracle is asserted here | SIL | individual | gap |
| N22-TS4-39 | SRS-REQ-056 | MTD on-SoC QSPI check on the emulated PolarFire SoC | OOB | run the `FF_Mtd_t` path against the emulated MSS QSPI + `Micron_MT25Q` window in Renode | **gap** — SRS-REQ-056 records "the live on-SoC QSPI check is not yet in place (pending QSPI controller command driver)" and there is no `renode-test` MTD/QSPI per-check observable. Open TBD (NC-EXT-47 §7.5). Uncontrolled observation only: none (no emulated MTD check exists). No oracle is asserted here | PIL | individual | gap |
| N22-TS4-40 | SRS-REQ-056, SRS-REQ-055 | MTD + NVM: persist a bounded parameter and mirror it into the NOR window | nominal | write parameter id 3 with a 4-byte value via `nvm.write`; copy the same 4 bytes into a 4 KiB sector of the MTD after `erase`; read both back | NVM read returns the 4 bytes with `len == 4`; MTD read returns the same bytes; the eNVM slot table and the NOR window are independent byte ranges with no aliasing (SRS-REQ-056 + SRS-REQ-055) | SIL | combined | covered |
| N22-TS4-41 | SRS-REQ-070 | `ff_block_mpfs` setup validation + single-block read (CMD17) | nominal | `FF_BlockMpfsSetup(&drv,&bus,512,N)`; `read(2,buf,1)` | setup = `FF_SUCCESS`; `dev.block_size == 512`, `dev.num_blocks == N`; `read` = `FF_SUCCESS`, last command == CMD17, direction read, TBS == 512, block count == 1, buffer == the mock store block 2 (SRS-REQ-070) | SIL | individual | covered |
| N22-TS4-42 | SRS-REQ-070 | `ff_block_mpfs` multi-block write/read and round trip (CMD18/CMD25) | nominal | `write(4,data,2)`; `read(4,in,2)`; boundary `read(N-1,buf,1)` (last valid LBA) | each = `FF_SUCCESS`; commands CMD25/CMD18 with block count == 2; the read-back equals the written data; the last full-capacity request is accepted (SRS-REQ-070) | SIL | individual | covered |
| N22-TS4-43 | SRS-REQ-070 | `ff_block_mpfs` flush (no write-back cache) | nominal | `dev.flush()` | `FF_SUCCESS` (no-op; documented: there is no write-back cache and no CMD12 stop-transmission path) (SRS-REQ-070) | SIL | individual | covered |
| N22-TS4-44 | SRS-REQ-070 | `ff_block_mpfs` setup argument bounds | OOB | `Setup(&drv,&bus,256,N)` (< 512); `Setup(…,0,N)`; `Setup(…,1000,N)` (not a power of two); `Setup(…,8192,N)` (> 2048); `Setup(…,512,0)` (num_blocks 0); NULL d/bus; missing bus hook; boundary `Setup(…,2048,N)` accepted | each invalid = `FF_INVALID_ARGUMENT`; block size must be a power of two in [512, 2048] and `num_blocks > 0`; the 2048 boundary is accepted (SRS-REQ-070) | SIL | individual | covered |
| N22-TS4-45 | SRS-REQ-070 | `ff_block_mpfs` read/write argument bounds (buffer, block count, LBA) | OOB | `read(0,NULL,1)`; `read(0,buf,0)`; `read(0,buf,FF_MMC_BCCT_MAX+1)`; `read(N,buf,1)`; `read(N-1,buf,2)`; `read(0x100000000ULL,buf,1)` | NULL buffer / `blocks == 0` / `blocks > BCCT (0xFFFF)` / LBA ≥ capacity / request past the end / LBA > 32 bits → `FF_INVALID_ARGUMENT`; the exact-capacity single-block read is accepted (SRS-REQ-070) | SIL | individual | covered |
| N22-TS4-46 | SRS-REQ-070 | `ff_block_mpfs` timeout and controller-error status | OOB | controller never sets completion (bounded by `FF_MMC_POLL_MAX`); set an error bit (`FF_MMC_INT_ERROR_MASK`) in the status; leave a stale status bit from a prior transfer | no completion → `FF_TIMEOUT` and the whole status word is cleared; an error bit → `FF_ERROR` and the status word is cleared; a stale error/completion bit cannot mislead the next transfer (SRS-REQ-070: timeout → `FF_TIMEOUT`, error status → `FF_ERROR`) | SIL | individual | covered |
| N22-TS4-47 | SRS-REQ-070 | `ff_block_mpfs` card initialisation, CMD12 abort and error recovery | OOB | issue a transfer without card init (CMD0/CMD8/ACMD41/CMD2/CMD3/CMD7); request a stop-transmission after an error | **gap** — the header documents "no card-initialisation … the driver assumes the card is already initialised" and "no error recovery beyond reporting `FF_ERROR`/`FF_TIMEOUT`: no CMD12 stop-transmission, no bus reset"; on-target card init is GAP-A5. Open TBD (NC-EXT-47 §7.3/§7.5). Uncontrolled observation only: no CMD0/CMD12 is issued. No oracle is asserted here | SIL | individual | gap |
| N22-TS4-48 | SRS-REQ-070 | `ff_block_mpfs` DMA/ADMA data path | OOB | program a DMA/ADMA transfer (DMA enable bit; DMA address registers) and read/write | **gap** — the backend is "polled single/multi-block buffer mode only: no DMA/ADMA/ADMA2 (the DMA address registers are left at reset)"; real SDHCI FIFO handshaking and the DMA path are GAP-A5. Open TBD (NC-EXT-47 §7.5). Uncontrolled observation only: the DMA-enable bit is not set by the driver. No oracle is asserted here | SIL | individual | gap |
| N22-TS4-49 | SRS-REQ-070, SRS-REQ-031 | Block + PCIe: a storage device behind the PCIe root complex | nominal | bind the PCIe backend (config space) and the block backend on two injected buses; probe the device class through PCIe and perform a block read | the PCIe config-space probe returns the class/revision word and the block read returns the mock store block; the two register maps do not alias (SRS-REQ-070 + SRS-REQ-031) | SIL | combined | covered |
| N22-TS4-50 | SRS-REQ-031 | `ff_pcie_mpfs` setup + init enables ECAM | nominal | `FF_PcieMpfsSetup(&drv,&bus,base,0)`; `init(0)`; `init(1)` (wrong rc) | setup = `FF_SUCCESS`; name "mpfs-pcie"; all five vtable entries set; `init(0)` = `FF_SUCCESS` and CTRL has ENABLE\|ECAM_EN set; `init(1)` = `FF_INVALID_ARGUMENT` (SRS-REQ-031) | SIL | individual | covered |
| N22-TS4-51 | SRS-REQ-031 | `ff_pcie_mpfs` read config space (Vendor/Device, Command/Status, Class/Rev) | nominal | `read_config(0,0,0,0x00,&v)`; `(0x04)`; `(0x08)`; boundary `(0x3C)` | each = `FF_SUCCESS`; offset 0x00 returns VendorID\|DeviceID, 0x04 returns Command\|Status, 0x08 returns Class Code (23:8)\|Revision (7:0) from the ECAM window (SRS-REQ-031 ECAM config-space contract) | SIL | individual | covered |
| N22-TS4-52 | SRS-REQ-031 | `ff_pcie_mpfs` write config space (Command register round trip) | nominal | `write_config(0,0,0,0x04,0x00000007)`; `read_config(0,0,0,0x04,&v)` | write = `FF_SUCCESS`; the read-back == the full 32-bit written value; restoring the original value round-trips (SRS-REQ-031) | SIL | individual | covered |
| N22-TS4-53 | SRS-REQ-031 | `ff_pcie_mpfs` unsupported bus/device/function and NULL out-pointer | OOB | `read_config(1,0,0,…)`; `read_config(0,1,0,…)`; `read_config(0,0,1,…)`; `write_config(0,1,0,…)`; `read_config(0,0,0,0,NULL)` | any (bus,dev,fn) != (0,0,0) → `FF_INVALID_ARGUMENT` (the increment handles only the emulated root complex); NULL out-pointer → `FF_INVALID_ARGUMENT` (SRS-REQ-031) | SIL | individual | covered |
| N22-TS4-54 | SRS-REQ-031 | `ff_pcie_mpfs` pre-init access and root-complex index bounds | OOB | `read_config`/`write_config` before `init`; `Setup(&drv,&bus,base,2)` (rc ≥ 2); NULL bus hook | pre-init access → `FF_NOT_READY`; `rc >= 2` → `FF_INVALID_ARGUMENT` at setup; NULL d/bus/hook → `FF_INVALID_ARGUMENT` (SRS-REQ-031) | SIL | individual | covered |
| N22-TS4-55 | SRS-REQ-031 | `ff_pcie_mpfs` MSI enable/disable documented not-implemented path | OOB | `msi_enable(0,0,0,0)`; `msi_disable(0,0,0)` | both = `FF_NOT_IMPLEMENTED` as documented in this increment (MSI stubbed; PLIC lines 149/150 declared for completeness; `hal_pcie.h` notes DMA/full enumeration are residuals). Oracle = the documented not-implemented status | SIL | individual | covered |
| N22-TS4-56 | SRS-REQ-031 | `ff_pcie_mpfs` link training, enumeration, BAR mapping, DMA | OOB | attach a physical endpoint; enumerate the bus; assign BARs; run a DMA transfer | **gap** — `hal_pcie.h`/`ff_pcie_mpfs.h`: "Configuration-space access only: no enumeration, no BAR mapping, no DMA … full PCIe subsystem is GAP-A5"; no clause defines the behaviour. Open TBD (NC-EXT-47 §7.5). Uncontrolled observation only: config-space round trips succeed; nothing else is attempted. No oracle is asserted here | HIL | individual | gap |
| N22-TS4-57 | SRS-REQ-044, SRS-REQ-031 | PCIe `hal_pcie=PASS` on the emulated SoC is not peripheral-bound | OOB | rely on the Renode `hal_pcie=PASS` per-check marker as PCIe peripheral evidence | **gap** — `app_renode_test.c` `renode_hal_pcie_check()` returns `true` without touching the controller because the beaglev-fire platform does not instantiate the PCIe root complex; the marker is therefore not a peripheral-bound observable. Open TBD (NC-EXT-47 §7.6). Uncontrolled observation only: `hal_pcie=PASS` is emitted unconditionally. No oracle is asserted here | PIL | individual | gap |
| N22-TS4-58 | SRS-REQ-031, SRS-REQ-032 | PCIe + SpaceWire on distinct injected buses (contract/register isolation) | nominal | bind PCIe (`read_config`/`write_config`) and SpaceWire (`send`/`rmap.read`) on separate injected buses; exercise both | each backend's config-space/contract state is retained independently; no register or buffer aliasing between the 32-bit ECAM accesses and the byte-oriented SpaceWire frames (SRS-REQ-031 + SRS-REQ-032) | SIL | combined | covered |
| N22-TS4-59 | SRS-REQ-032 | `ff_spacewire_mpfs` setup + init + send/send_pid with protocol ID | nominal | `FF_SpaceWireMpfsSetup(&drv,&bus,0xFE)`; `init()`; `send({0xA1,0xB2},2)`; `send_pid(0x02,{…},2)`; `set_rx_callback(cb,ctx)` | setup = `FF_SUCCESS`; name "mpfs-spacewire"; `init` = `FF_SUCCESS`, `ready == 1`; `send` prefixes protocol ID `FF_SPW_PROTOCOL_ID_CCSDS` (0x01) and `send_pid` uses the given id (0x02); the frame length is 1 + payload; the RX callback flag is set (SRS-REQ-032; ECSS-E-ST-50-51) | SIL | individual | covered |
| N22-TS4-60 | SRS-REQ-032 | `ff_spacewire_mpfs` RMAP read/write/rmw command format | nominal | `rmap.read(0x1000,resp,cap,&len)`; `rmap.write(0x2000,data,4)`; `rmap.rmw(0x3000,and,or,2)` | each = `FF_SUCCESS`; the 16-byte header has protocol ID 0x02, the RMAP command code (read 0x01 / write 0x02 / rmw 0x03), target 0xFE, key 0x00, reply == logical address, a monotonically increasing transaction id and the address/data-length fields set correctly; write/rmw append the payload and a data CRC; read leaves `*len == 0` (SRS-REQ-032; ECSS-E-ST-50-52) | SIL | individual | covered |
| N22-TS4-61 | SRS-REQ-032 | `ff_spacewire_mpfs` RMAP CRC-16 known-answer and corruption detection | nominal | verify the header CRC over bytes 0–13; flip a byte and re-verify; verify a data CRC | a well-formed header passes the CRC check; a single corrupted byte fails it; the data CRC over a known 4-byte payload matches the helper (ECSS-E-ST-50-52 CRC; SRS-REQ-032) | SIL | individual | covered |
| N22-TS4-62 | SRS-REQ-032 | `ff_spacewire_mpfs` send argument boundaries and pre-init use | OOB | `send(NULL,0)`; `send(data,0)`; `send(data,257)` (> `FF_SPW_MAX_PAYLOAD` 256); `send_pid(0x02,NULL,0)`; send before `init`; boundary `send(data,256)` | NULL/len 0/len > 256 → `FF_INVALID_ARGUMENT`; send before `init` → `FF_NOT_READY`; `len == 256` is accepted (SRS-REQ-032) | SIL | individual | covered |
| N22-TS4-63 | SRS-REQ-032 | `ff_spacewire_mpfs` RMAP argument boundaries | OOB | `rmap.read(addr,NULL,cap,&len)`; `cap == 0`; `cap > 256`; `rmap.write(addr,data,0)`; `rmap.write(addr,data,257)`; `rmap.rmw(addr,NULL,or,2)`; `mask_len == 0` / `> 256`; pre-init | NULL/cap-0/cap > 256/len-0/len > 256/mask-len bounds → `FF_INVALID_ARGUMENT`; pre-init → `FF_NOT_READY` (SRS-REQ-032; `hal_spacewire.h`) | SIL | individual | covered |
| N22-TS4-64 | SRS-REQ-032 | `ff_spacewire_mpfs` hardware timing, link-layer errors and timeouts | OOB | inject a disconnect/parity/ESC error; withhold a reply and wait for a timeout | **gap** — the backend documents "No link-layer error injection (disconnect, parity, ESC errors)" and "No timeouts or retry logic"; no clause defines the response. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: no timeout or error is reported. No oracle is asserted here | SIL | individual | gap |
| N22-TS4-65 | SRS-REQ-032 | SpaceWire peripheral-bound validation on the emulated SoC | OOB | bind the SpaceWire backend to an emulated SpaceWire peripheral and run a link transaction | **gap** — there is no emulated SpaceWire peripheral in the Renode beaglev-fire repl; the check emits `[renode-test] hal_spacewire=CONTRACT` (NC-S7-01). Open TBD (NC-EXT-47 §7.5). Uncontrolled observation only: the contract-level self-test result, not a peripheral-bound observable. No oracle is asserted here | PIL | individual | gap |
| N22-TS4-66 | SRS-REQ-032, SRS-REQ-056 | SpaceWire + MTD: RMAP write payload delivered into the NOR window and read back | nominal | build an RMAP write command carrying a data block on the mock link; write the RMAP payload into the MTD window; read back through `FF_Mtd_t` | the bytes read through the MTD equal the RMAP write payload; the RMAP header/CRC and the byte-addressed MTD window are independent ranges (SRS-REQ-032 + SRS-REQ-056) | SIL | combined | covered |
| N22-TS4-67 | SRS-REQ-055 | `ff_nvm_mpfs` setup + slot write/read round trip | nominal | `FF_NvmMpfsSetup(&drv,&bus)`; `write(3,val,4)`; `read(3,out,sizeof,&len)`; boundary `write(0,NULL,0)` | setup = `FF_SUCCESS`; name "mpfs-envm"; write = `FF_SUCCESS`, `present[3] == 1`; read = `FF_SUCCESS`, `len == 4`, bytes match; a zero-length write stores length 0 and reads back `len == 0` (SRS-REQ-055) | SIL | individual | covered |
| N22-TS4-68 | SRS-REQ-055 | `ff_nvm_mpfs` erase and slot bounds (31-byte payload boundary) | nominal | `erase(3)`; boundary `write(1,big,FF_NVM_MAX_VALUE)` (31) then read | erase = `FF_SUCCESS`, `present[3] == 0`, length reads 0; a 31-byte payload round-trips with `len == 31` and equal bytes (SRS-REQ-055; `FF_NVM_SLOT_SIZE` 32) | SIL | individual | covered |
| N22-TS4-69 | SRS-REQ-055 | NVM MMIO self-test on the emulated MSS eNVM | nominal | Renode: `FF_NvmMpfsSelfTest` on the eNVM DATA region (scratch round trip outside the parameter slot table) | per-check observable `[renode-test] hal_nvm=PASS`: the scratch write/read returns `FF_SUCCESS` with the written byte read back (FRICK-VV-TSPEC-001 §7.6) | PIL | individual | covered |
| N22-TS4-70 | SRS-REQ-055 | `ff_nvm_mpfs` id, length and pointer bounds | OOB | `write(FF_NVM_MAX_IDS,…)` (id 32); `write(1,val,FF_NVM_SLOT_SIZE)` (32 > 31); `write(1,NULL,4)`; `read(1,NULL,…)`; `read(1,out,sizeof,NULL)`; `read(FF_NVM_MAX_IDS,…)`; `erase(FF_NVM_MAX_IDS)` | id ≥ 32, `len > FF_NVM_MAX_VALUE` (31), NULL data with `len != 0`, NULL out/len pointers → `FF_INVALID_ARGUMENT` (SRS-REQ-055 bounded ids/values) | SIL | individual | covered |
| N22-TS4-71 | SRS-REQ-055 | `ff_nvm_mpfs` read capacity and corrupt length byte | OOB | `read(2,out,2,&len)` when the stored length is 4 (cap < len); store a corrupt length byte > 31 | `cap < len` → `FF_BAD_SIZE` (and a larger cap succeeds with `len == 4`); a corrupt length byte is defensively clamped to `FF_NVM_MAX_VALUE` (31) and read with a 31-byte cap (SRS-REQ-055) | SIL | individual | covered |
| N22-TS4-72 | SRS-REQ-055 | `ff_nvm_mpfs` wear, endurance and retention | OOB | write/erase one id beyond the eNVM endurance; test data retention after power cycling | **gap** — the reference backend exercises only the logical slot table; eNVM write/endurance/retention on silicon is GAP-A5 and no clause defines a wear model. Open TBD (NC-EXT-47 §7.5). Uncontrolled observation only: none available without the target board. No oracle is asserted here | HIL | individual | gap |
| N22-TS4-73 | SRS-REQ-055, SRS-REQ-070 | NVM + Block: persist a block-device image digest across a power cycle | nominal | write a block image, compute a digest, store the digest in an eNVM slot; reset the block mock, re-read the image, compare against the stored digest | the digest read from eNVM equals the recomputed digest of the block image; the slot table and the block store are independent storage ranges (SRS-REQ-055 + SRS-REQ-070) | SIL | combined | covered |
| N22-TS4-74 | SRS-REQ-025, SRS-REQ-057, SRS-REQ-058, SRS-REQ-069, SRS-REQ-056, SRS-REQ-070, SRS-REQ-031, SRS-REQ-032, SRS-REQ-055 | HAL registry binds the eight group-B I/O backends under distinct names | nominal | register CAN, I2C, SPI, MTD, Block, PCIe, SpaceWire and NVM backends (different classes) and retrieve each by name/class | every registration = `FF_SUCCESS`; per-class count == 1; each retrieval returns the exact registered backend pointer (SRS-REQ-025 registry) | SIL | combined | covered |
| N22-TS4-75 | SRS-REQ-062, SRS-REQ-063, SRS-REQ-064 | CAN-FD register-window/backend, CANopen documented subset, RS-485 DE/RE control | nominal + OOB | `test_canfd` DLC/exact sample-point/config/TX/RX cases; `test_canopen` setup/NMT/heartbeat/SDO/PDO/abort/rollback/index-and-subindex error cases, including the bounded RX-contention regression; `test_serial` DE assert/release, bounded drain, timeout result, release-call hold and invalid-argument cases | explicit host expectations in `tests/unit/test_canfd.c` (328 checks), `test_canopen.c` (178), and `test_serial.c` (67); PASS on host. The CANopen guard's target/ISR behavior and integration policy lack independent delta review (NC-S4c-04). These checks do not establish real CAN-FD IP or electrical DE/RE behavior. CAN-FD emulator/target path remains NC-S4c-01/GAP-A5; full CiA 301/302/402 conformance is not claimed | SIL | combined | covered (host subset) |
| N22-TS4-76 | SRS-REQ-067 | Generic fabric-device (FIC/AXI) contract path | nominal + OOB | `test_fabric` valid/invalid device windows, alignment/range, read/write and self-test restoration; `test_canfd` accesses through the fabric backend | explicit host mock-MMIO expected values and bounds; PASS (`test_fabric.c` 119 checks, `test_canfd.c` 328 checks). No FPGA peripheral is included or emulated; target integration is NC-S4c-01/GAP-A5 | SIL | combined | covered (host contract) |
| N22-TS4-77 | SRS-REQ-057, SRS-REQ-058, SRS-REQ-070, SRS-REQ-055 | Combined pairs that couple group B to modules owned by other work packages | OOB | CAN + command ingest (PUS/TC), I2C + sample-app sensors, Block + file/data services, NVM + table persistence | **gap** — the peer modules are owned by TS-2/TS-5 (link/PUS, TBL, reference apps) and are not in this work package; the umbrella lists "CAN/I2C with end-to-end command paths on target; Block with table storage" as not-yet-exercised. Open TBD (NC-EXT-47 §7.1). No oracle is asserted here | SIL | combined | gap |
| N22-TS4-78 | SRS-REQ-043, SRS-REQ-044 | Renode per-check observables vs the aggregate `RENODE_TEST: PASS` marker | OOB | rely on the aggregate `RENODE_TEST: PASS` line alone as the group-B I/O requirement oracle | **gap** — the aggregate marker is not a per-requirement oracle; the per-check lines (`hal_nvm`/`hal_can`/`hal_i2c`/`hal_spi`/`hal_block`/`hal_pcie`/`hal_spacewire`) are the observables (FRICK-VV-TSPEC-001 §7.6). Uncontrolled observation only: the aggregate `RENODE_TEST: PASS` line itself. No oracle is asserted for the aggregate line | PIL | individual | gap |
| N22-TS4-79 | SRS-REQ-056, SRS-REQ-057, SRS-REQ-058, SRS-REQ-069, SRS-REQ-070, SRS-REQ-031, SRS-REQ-032, SRS-REQ-055 | On-target (HIL) evidence for all group-B I/O | OOB | run every group-B peripheral on the RTPFS160ZT silicon with real devices attached | **gap** — GAP-A5 / NC-EXT-31: real-silicon MMIO, electrical timing, device/card/flash behaviour and the PCIe/SpaceWire link require the board (NC-EXT-47 §7.5). Uncontrolled observation only: none possible without the target board. No oracle is asserted here | HIL | individual | gap |
| N22-TS4-80 | SSS-REQ-010 | Static, allocation-free storage across all group-B I/O | nominal | build the eight backends; audit the objects/link for a heap allocator reference (`nm`/`objdump`) | the group-B drivers draw all storage from caller-provided/bounded static objects; no `malloc`/`calloc`/`free` is referenced from these modules (SSS-REQ-010) | SIL | combined | covered |

## 3. Gaps (mapping to NC-EXT-47 sub-items)

All gaps are recorded, not resolved (N21). Each row below maps a `gap` case to
the NC-EXT-47 sub-item in FRICK-VV-TSPEC-001 §7 that it belongs to, and names
the specific open NC where one exists. Gap rows assert **no** oracle.

**NC-EXT-47 §7.3 — out-of-bound oracles not defined by a controlled clause
(unresolved/absent contract).**

- `N22-TS4-9` — CAN bus-off / error-frame / lost-arbitration / RX_MSG_LOSS
  (documented limitation; TBD; NC-EXT-47 §7.3).
- `N22-TS4-10` — CAN TX-pending buffer overwrite (TX status not polled; TBD;
  NC-EXT-47 §7.3).
- `N22-TS4-21` — I2C STOP completion / bus recovery / clock stretching /
  arbitration loss (GAP-A5; NC-EXT-47 §7.3).
- `N22-TS4-31` — SPI STATUS bit-11 name and on-target SCL accuracy (GAP-A5;
  NC-EXT-47 §7.3/§7.5).
- `N22-TS4-38` — MTD NOR program-failure / erase-before-write semantics (TBD;
  NC-EXT-47 §7.3).
- `N22-TS4-47` — Block card initialisation / CMD12 abort / error recovery
  (GAP-A5; NC-EXT-47 §7.3/§7.5).
- `N22-TS4-64` — SpaceWire hardware timing / link-layer errors / timeouts
  (TBD; NC-EXT-47 §7.3).
- `N22-TS4-75/76` — target/emulator peripheral fidelity remains open under
  NC-S4c-01/GAP-A5; host contract tests do not assert a target oracle.

**NC-EXT-47 §7.1 — combined-I/O scenarios.**

- `N22-TS4-77` — combined pairs coupling group B to modules owned by other work
  packages (CAN + command ingest, I2C + sensors, Block + file services, NVM +
  table persistence) are recorded under TS-2/TS-5, not here (TBD;
  NC-EXT-47 §7.1).

**NC-EXT-47 §7.4 — I/O pairs not yet exercised combined.**

- The umbrella §7 lists "PCIe/SpaceWire with link layer; Block with table
  storage; CAN/I2C with end-to-end command paths on target" as not-yet-exercised
  pairs. The in-package members of those pairs are covered here
  (`N22-TS4-58`, `N22-TS4-49`); the link-layer/table/target peers are recorded
  under TS-2/TS-5 and `N22-TS4-77`/`N22-TS4-79` (NC-EXT-47 §7.4/§7.1).

**NC-EXT-47 §7.5 — target (HIL) evidence remains GAP-A5 / NC-EXT-31.**

- `N22-TS4-39` — MTD on-SoC QSPI check not yet in place (QSPI command driver
  pending; NC-EXT-47 §7.5).
- `N22-TS4-48` — Block DMA/ADMA data path (GAP-A5; NC-EXT-47 §7.5).
- `N22-TS4-56` — PCIe link training / enumeration / BAR mapping / DMA (GAP-A5;
  NC-EXT-47 §7.5).
- `N22-TS4-65` — SpaceWire peripheral-bound validation; no emulated peripheral
  (NC-S7-01; NC-EXT-47 §7.5).
- `N22-TS4-72` — NVM/eNVM wear, endurance and retention (GAP-A5;
  NC-EXT-47 §7.5).
- `N22-TS4-79` — all group-B I/O on target silicon (GAP-A5 / NC-EXT-31;
  NC-EXT-47 §7.5).

**NC-EXT-47 §7.6 — emulated-target aggregate marker is not a per-requirement
oracle.**

- `N22-TS4-57` — the PCIe `hal_pcie=PASS` marker is emitted by a check that
  returns `true` without touching the controller (root complex not instantiated
  in the beaglev-fire platform); it is not peripheral-bound (TBD;
  NC-EXT-47 §7.6).
- `N22-TS4-78` — `RENODE_TEST: PASS` does not evidence the group-B requirements;
  the per-check `[renode-test] …=PASS`/`=CONTRACT` lines in
  `N22-TS4-5/16/27/69` (+ Block, PCIe, SpaceWire) are the observables (TBD;
  NC-EXT-47 §7.6).

### 3.1 Coverage summary

| Metric | Count |
| --- | --- |
| Total cases | 80 |
| Nominal | 40 |
| Out-of-bound | 40 |
| Individual | 69 |
| Combined | 11 |
| `covered` (oracle grounded in a requirement/interface clause) | 62 |
| `gap` (no controlled oracle; recorded under NC-EXT-47) | 18 |
| SIL / PIL / HIL | 68 / 8 / 4 |

Requirement coverage in this scope: `SRS-REQ-025/029/031/032/043/044/055/056/
057/058/062/063/064/067/069/070` and `SSS-REQ-010` all have at least one case;
every group-B I/O (CAN, I2C, SPI, MTD, Block, PCIe, SpaceWire, NVM) has an
individual nominal case, an individual out-of-bound case and at least one
combined case with an adjacent subsystem, except where the combined case is the
cross-package/environment gap (`N22-TS4-77`). The combined cases are the
in-package pairs `N22-TS4-11/22/32/40/49/58/66/73`, the registry case
`N22-TS4-74`, the cross-package pair `N22-TS4-77` (gap) and the static-all case
`N22-TS4-80`. The
PCIe/SpaceWire requirement-id mismatch with the umbrella §4 and the
`hal_pcie=PASS` marker discrepancy are recorded, not resolved by a fabricated
oracle.

## History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-14 | frick-fsw | Initial TS-4 work package per N22 and the FRICK-VV-TSPEC-002 chunk plan: nominal + out-of-bound case table with explicit oracles for I/O group B (MSS CAN, MSS I2C, MSS SPI, MSS QSPI NOR MTD, MSS MMC/SD block, MPFS PCIe, SpaceWire/RMAP, MSS eNVM); gaps mapped to NC-EXT-47 sub-items with the open NC/TBD named; PCIe/SpaceWire requirement-id reconciliation against the umbrella §4 recorded |
| 0.2 | 2026-10-15 | frick-fsw | Add implemented S4c host-oracle cases for CAN-FD/CANopen/RS-485/fabric; explicitly retain the missing CAN-FD emulator/target path as NC-S4c-01/GAP-A5, not an NC-EXT phantom or Renode PASS |
| 0.3 | 2026-10-15 | frick-fsw | Reconcile test case ID uniqueness and verify exact post-review counts/oracles for CAN-FD timing and CANopen abort/rollback cases; mark S4c contracts implemented with bounded scope |
| 0.4 | 2026-10-15 | frick-fsw | Clarify release-call hold parameter semantics for RS-485 and ensure the PUSopen test-case ID is unique across TS-1 |
| 0.5 | 2026-10-15 | frick-fsw | Correct TS-4 interface-ground wording to reflect implemented bounded CANopen subset; update current final test counts |
| 0.6 | 2026-10-15 | frick-fsw | Add exact CAN-ID/length/index/subindex checks to absent-index SDO download oracle; update CANopen count to 171 |
| 0.7 | 2026-10-15 | frick-fsw | Correct TS-4 fabric row's CAN-FD check count to 328 and reconcile final full local cycle and test evidence |
| 0.8 | 2026-10-15 | frick-fsw | Reconcile baseline to committed HEAD 13ebf92 and GitHub CI run 38793643786; update CANopen evidence to 178 checks and retain NC-S4c-04 independent-review/integration limitation |
