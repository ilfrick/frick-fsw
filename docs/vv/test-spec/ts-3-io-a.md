# Test Specification TS-3 — I/O group A (GPIO, Serial/MMUART, Mem/DMA, GEM Ethernet, netstack/lwIP, TSN, gPTP)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-TSPEC-TS3                     |
| Revision                         | 0.1                                    |
| Date                             | 2026-10-14                             |
| Norms                            | N10, N11, N22                          |
| Umbrella                         | `docs/vv/comprehensive-test-spec.md` (FRICK-VV-TSPEC-001) |
| Work-package index               | `docs/vv/test-spec/README.md` (FRICK-VV-TSPEC-002), work package TS-3 |
| Companion                        | `docs/vv/system-verification-architecture.md` (FRICK-VV-SVA-001) |
| Scope                            | I/O group A: MSS GPIO, MSS MMUART serial, static RAM pool + DMA helper, Cadence GEM Ethernet MAC, loopback/lwIP network stack, GEM native TSN, 802.1AS gPTP |
| Baseline                         | Working tree at HEAD `57974d2` as read 2026-10-14 (uncommitted changes possible) |
| Modules under test               | `src/drivers/gpio/mpfs/ff_gpio_mpfs.c`, `src/drivers/serial/mpfs/ff_serial_mpfs.c`, `src/drivers/mem/ff_mem_static.c`, `src/drivers/eth/gem/ff_gem.c`, `src/drivers/eth/gem/ff_gem_tsn.c`, `src/drivers/eth/gptp/ff_gptp.c`, `src/drivers/net/ff_net_loopback.c`, `src/hal/net/lwip/ff_net_lwip.c`, `src/hal/net/lwip/ff_lwip_netif.c`, `src/hal/hal.c` |
| Interfaces cited                 | `src/hal/hal.h`, `src/hal/hal_gpio.h`, `src/hal/hal_serial.h`, `src/hal/hal_mem.h`, `src/hal/hal_eth.h`, `src/hal/hal_net.h`, `src/hal/hal_tsn.h`, `config/platform/platform_mpfs.h` |

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

`SRS-REQ-025` (HAL registry), `SRS-REQ-026` (Ethernet MAC / net interface
contract), `SRS-REQ-027` (pluggable network-stack contract), `SRS-REQ-028` (TSN
backend contract), `SRS-REQ-030` (serial RS-422/RS-485 contract), `SRS-REQ-031`
(SPI/I2C/PCIe/GPIO contracts; GPIO ISR→task rule), `SRS-REQ-032`
(SpaceWire/memory-storage contracts), `SRS-REQ-036` (Cadence GEM reference MAC
driver), `SRS-REQ-038` (GEM native TSN backend), `SRS-REQ-039` (reference
loopback network stack), `SRS-REQ-040` (lwIP adapter + netif glue),
`SRS-REQ-043`/`SRS-REQ-044` (emulated-target verification / Renode functional
environment), `SRS-REQ-047` (reference MSS GPIO driver), `SRS-REQ-048`
(reference MSS MMUART serial driver), `SRS-REQ-049` (reference static RAM pool +
DMA helper), `SRS-REQ-066` (GEM TSN Qci/Qav/Qbb completion), `SRS-REQ-067`
(generic fabric-device contract, planned), `SRS-REQ-068` (gPTP 802.1AS), and
`SSS-REQ-010` (static, allocation-free storage).

Out of scope (owned by other work packages): I/O group B — CAN/CAN-FD, I2C,
SPI, MTD, Block, PCIe, SpaceWire, NVM (TS-4); link layer and SDLS (TS-2); core
services SB/TBL/EVS/ES/scheduler/HS-FDIR/math (TS-5); software-only robustness
(memory safety, numeric, concurrency, fault injection, resources) (TS-6). Where
a combined case touches a module from another package, the module is a fixture
and the oracle is grounded only in a requirement of this package.

### 1.2 Class, scope, environment and status conventions

- **Class**: `nominal` = a legal input inside the specified operating envelope
  (this includes legal minimum/maximum boundary values); `OOB` = a
  range-violating, malformed, out-of-envelope, resource-exhausted or
  prohibited-ordering input.
- **Scope**: `individual` = one I/O backend / one module surface is under test
  (other modules are only fixtures); `combined` = the case deliberately asserts
  coupling across two or more modules/backends in one scenario (e.g. GPIO +
  Serial on one MMIO bus, Mem/DMA + GEM, GEM + netstack, TSN + gPTP).
- **Env**: `SIL` = native host unit/integration suite (`make test`, ctest);
  `PIL` = Renode emulated PolarFire SoC; `HIL` = target silicon
  (RTPFS160ZT). Per N22 and FRICK-VV-TSPEC-001 §1.3, an emulator establishes
  only what its model represents.
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
| `hal.h`, `hal.c` (SRS-REQ-025) | bounded allocation-free registry; NULL name/backend → `FF_INVALID_ARGUMENT`, duplicate name → `FF_ALREADY_EXISTS`, full class → `FF_ERROR`, per-class count ≤ `FF_HAL_MAX_BACKENDS` |
| `hal_gpio.h` (SRS-REQ-031) | `FF_Gpio_t` vtable `init/configure/write/read/attach_irq/detach_irq`; the GPIO interrupt contract states an ISR defers work to a task; direction enum `FF_GPIO_OUT`/`FF_GPIO_IN` |
| `ff_gpio_mpfs.h` | `GPIO_CFG[pin]` at `0x00 + pin*4`, `FF_GPIO_CFG_OUT_EN` bit 0, `FF_GPIO_CFG_IN_EN` bit 1, `GPIO_IN` `0x84`, `GPIO_OUT` `0x88`, `FF_GPIO_MPFS_PINS` 32; setup rejects NULL/0/> 32 pins |
| `hal_serial.h` (SRS-REQ-030) | `FF_Serial_t` vtable `init/write/read`; RS-422/RS-485 direction control; `write(NULL,0)` legal, `write(NULL,len≠0)` rejected, `read(NULL,…)` rejected |
| `ff_serial_mpfs.h` | 4-byte-spaced registers THR `0x00`, LCR `0x0C` (WLEN bits 0–1, STOP2 bit 2, PEN bit 3, EPS bit 4), LSR `0x14` (DATA_READY `0x01`, THR_EMPTY `0x20`, TX_EMPTY `0x40`), SCR `0x1C`; `init` rejects `data_bits < 5`/`> 8` and `parity > 2` |
| `hal_mem.h`, `ff_mem_static.h` (SRS-REQ-032) | `FF_MemPool_t` `init/alloc/reset` and `FF_Dma_t` `virt_to_phys`/`phys_to_virt` + cache-maintenance seam; `FF_MEM_ALIGN` 8; bounded bump allocator, single instance |
| `hal_eth.h` (SRS-REQ-026) | `FF_EthMac_t` vtable `init/get_mac_address/transmit/set_rx_callback/get_stats` and `mtu`; `transmit(NULL)`/len 0 → `FF_BAD_SIZE` |
| `ff_gem.h`, `ff_gem_regs.h` | `FF_GEM_MTU` 1536; `FF_GEM_TX_DESC`/`FF_GEM_RX_DESC` 4; `FF_GEM_TX_POLL` 100000; NCR RE/TE, NCFGR GBE/FD; SA1B `0x22110002`/SA1T `0x4433`; `DMACFG.ADDR64` bit 30; RBQP/TBQP + RBQP_HI/TBQP_HI; TSR.COMP; TX USED/WRAP/LAST bits |
| `hal_tsn.h` (SRS-REQ-028) | `FF_TsnBackend_t` `init/set_vlan/configure_tas/get_time_ns/set_stream_filter/configure_cbs/enable_pfc`; API `(vid, pcp)`, `(traffic_class, dest_mac, vid, ethertype, enable)` argument order |
| `ff_gem_regs.h` (GEM TSN block) | SCRT2 `0x0540` (QUEUE shift 0, VLANPR shift 4, VLANEN shift 8); ENST_CONTROL `0x0880`, ENST_ON_TIME(q) `0x0820+q*4`, ENST_OFF_TIME(q) `0x0840+q*4`, 17-bit `FF_GEM_ENST_TIME_MASK` `0x1FFFF`, `FF_GEM_ENST_START_NSEC_MASK` `0x3FFFFFFF`; SCRT1 `FF_GEM_SCRT1_ENTRY_COUNT` 4; TSH `0x01C0`/TSL `0x01D0`/TN `0x01D4` |
| `ff_gptp.h` (SRS-REQ-068) | `FF_GptpPort_t` `state`/`role`/`servo`/`last_sync_ns`; `FF_GPTP_SM_*`, `FF_GPTP_ROLE_*`; message types SYNC `0x00`, FOLLOW_UP `0x08`, PDELAY_REQ `0x02`, PDELAY_RESP `0x03`, PDELAY_FUP `0x0A`; `FF_GPTP_ETHERTYPE` `0x88F7`; `FF_GPTP_HDR_SIZE` 34; PI servo clamped ±100000 ppb |
| `hal_net.h` (SRS-REQ-027) | `FF_NetStack_t` socket-like vtable `init/open/connect/bind/listen/accept/send/recv/close`; `FF_NetSock_t` signed, `open`/`accept` return negative on failure; `bind` fails if the address/port is in use |
| `ff_net_loopback.h` | bounded 4 sockets; single-byte port addressing in `addr[0]`; 256-byte peer ring; unconnected send → error; non-blocking `recv` returns 0 bytes |
| `ff_net_lwip.h`, `ff_lwip_netif.h` | 6-octet address (IPv4 4 + port 2, big-endian); `addr_len < 6`/NULL → `FF_INVALID_ARGUMENT`; `FF_NetIf_t` attach requires `init/get_mac_address/transmit/set_rx_callback`; `FF_LWIP_NETIF_FRAME_MAX` 1536; netif IP MTU 1500; singleton `g_attached` |
| `config/platform/platform_mpfs.h` | `FF_GPIO0_PINS` 14, `FF_GPIO0_BASE` `0x20120000`, `FF_GEM0_BASE` `0x20110000`, `FF_GEM_MODID_OFF` `0xFC`, `FF_MMUART0_BASE` `0x20000000` |

## 2. Test cases

Case IDs `N22-TS3-1` … `N22-TS3-90`; groups: `1–10` GPIO + HAL registry,
`11–21` Serial/MMUART, `22–31` Mem/DMA, `32–44` GEM Ethernet, `45–59`
netstack/lwIP, `60–74` TSN, `75–85` gPTP, `86–90` cross-cutting fabric and
environment-bound gaps.

| Case | Requirement | Case | Class | Inputs (boundaries) | Expected result (oracle) | Env | Scope | Status |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| N22-TS3-1 | SRS-REQ-047 | `ff_gpio_mpfs` + `hal_gpio.h` setup and init (14-pin GPIO0 port) | nominal | `FF_GpioMpfsSetup(&drv,&bus,14)`; pin-count boundaries 1 and `FF_GPIO_MPFS_PINS` (32); `init()` | setup = `FF_SUCCESS`; `drv.gpio.init != NULL`; `init()` = `FF_SUCCESS`; the MMIO bus is bound and the valid pin count is 14 (SRS-REQ-047) | SIL | individual | covered |
| N22-TS3-2 | SRS-REQ-047 | `ff_gpio_mpfs` configure per-pin direction (`GPIO_CFG[pin]`) | nominal | `configure(3, FF_GPIO_OUT)`; `configure(5, FF_GPIO_IN)` | `GPIO_CFG[3]` (`0x00 + 3*4`) has `CFG_OUT_EN` (bit 0) set and `CFG_IN_EN` (bit 1) clear; `GPIO_CFG[5]` has `CFG_IN_EN` set and `CFG_OUT_EN` clear; unrelated pins 1/2 remain 0 (SRS-REQ-047) | SIL | individual | covered |
| N22-TS3-3 | SRS-REQ-047 | `ff_gpio_mpfs` output write/read-back and input read | nominal | `write(3,1)`→`read(3)`; `write(3,0)`→`read(3)`; input pin 5 read with the mock `GPIO_IN` bit set | OUT writes set/clear `GPIO_OUT` (`0x88`) bit 3; reading an OUT pin returns the OUT bit (1 then 0); reading an IN pin returns the `GPIO_IN` (`0x84`) bit value (1) (SRS-REQ-047) | SIL | individual | covered |
| N22-TS3-4 | SRS-REQ-047 | `ff_gpio_mpfs` pin index at/over the port boundary and NULL out-pointer | OOB | `read(14,&v)`, `configure(14,FF_GPIO_OUT)`, `write(14,1)` (pin == pins); `read(0xFF,&v)`; `read(0,NULL)` | each = `FF_INVALID_ARGUMENT`; no MMIO write to a register outside the configured 14-pin window (SRS-REQ-047 register-based driver; `pin >= pins` rejected) | SIL | individual | covered |
| N22-TS3-5 | SRS-REQ-047 | `ff_gpio_mpfs` setup argument boundaries | OOB | `setup(NULL,&bus,14)`; `setup(&drv,NULL,14)`; `setup(&drv,&bus,0)`; `setup(&drv,&bus,33)` (boundary 32 accepted, N22-TS3-1) | each invalid setup = `FF_INVALID_ARGUMENT` and leaves the previous binding unchanged; pins > `FF_GPIO_MPFS_PINS` (32) rejected (SRS-REQ-047) | SIL | individual | covered |
| N22-TS3-6 | SRS-REQ-031 | `ff_gpio_mpfs` + `hal_gpio.h` interrupt attach/detach (ISR→task rule) | OOB | `attach_irq(0, FF_GPIO_RISING, cb, ctx)`; `detach_irq(0)` | **gap** — SRS-REQ-031 requires a GPIO interrupt contract where "an ISR defers work to a task", but the reference backend returns `FF_NOT_IMPLEMENTED` and no clause defines the deferred-task hand-off, latency or re-arm semantics. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_NOT_IMPLEMENTED` for both calls. No oracle is asserted here | SIL | individual | gap |
| N22-TS3-7 | SRS-REQ-047 | `ff_gpio_mpfs` MMIO bus fault / unresponsive register bank | OOB | injected bus whose `read32`/`write32` returns a fault value (e.g. all-ones) or does not retain writes on `GPIO_CFG`/`GPIO_OUT` | **gap** — no SRS/interface clause defines the driver behaviour on an MMIO bus fault (read-back mismatch vs bus error; the injected mock bus cannot express a bus error). Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: the driver returns `FF_SUCCESS` and the read-back reflects the fault model. No oracle is asserted here | SIL | individual | gap |
| N22-TS3-8 | SRS-REQ-047, SRS-REQ-048 | GPIO + Serial on one injected MMIO bus (register-spacing isolation) | nominal | shared mock register bank covering `GPIO_CFG[0..13]`/`GPIO_IN`(`0x84`)/`GPIO_OUT`(`0x88`) and UART THR `0x00`/LCR `0x0C`/LSR `0x14`/SCR `0x1C`; set up both, configure GPIO pin 3, init UART 8N1, write 0xDE/0xAD, scratch self-test | GPIO CFG/OUT hold the GPIO values and UART LCR/LSR/SCR hold the serial values with no cross-talk; serial scratch == `0xA5`; the GPIO OUT bit 3 is unchanged by the serial traffic (SRS-REQ-047 + SRS-REQ-048) | SIL | combined | covered |
| N22-TS3-9 | SRS-REQ-025 | `hal.c` registry binds the GPIO and serial I/O backends under distinct names | nominal | `register(GPIO,"gpio0",gpio)`; `register(SERIAL,"uart0",serial)`; retrieve by name/class | both = `FF_SUCCESS`; per-class count == 1 and retrieval returns the exact registered pointers (SRS-REQ-025 `hal.h`) | SIL | combined | covered |
| N22-TS3-10 | SRS-REQ-025 | `hal.c` registry duplicate / invalid class / capacity / NULL boundaries | OOB | duplicate name in one class; out-of-range class enum; `FF_HAL_MAX_BACKENDS+1` registrations in one class; NULL name/backend | duplicate = `FF_ALREADY_EXISTS`; invalid class = `FF_INVALID_ARGUMENT`; the `FF_HAL_MAX_BACKENDS+1`-th = `FF_ERROR` with the count pinned at `FF_HAL_MAX_BACKENDS`; NULL name/backend = `FF_INVALID_ARGUMENT` (SRS-REQ-025) | SIL | individual | covered |
| N22-TS3-11 | SRS-REQ-048, SRS-REQ-030 | `ff_serial_mpfs` setup + init 8N1 programs LCR word length | nominal | `FF_SerialMpfsSetup(&drv,&bus)`; `init(115200, 8, 0, 1)`; data-bit boundaries 5 and 8 | setup = `FF_SUCCESS`; LCR (offset `0x0C`) WLEN bits 0–1 == `0b11` for 8 data bits; parity enable (PEN) clear; STOP2 clear — 8N1 (SRS-REQ-048) | SIL | individual | covered |
| N22-TS3-12 | SRS-REQ-048 | `ff_serial_mpfs` parity/stop-bit patterns (even+2-stop, odd+1-stop) | nominal | `init(115200,8,2,2)`; `init(115200,8,1,1)` | even/2-stop LCR has PEN, EPS and STOP2 set; odd/1-stop LCR has PEN set and EPS clear (SRS-REQ-048 LCR bit fields) | SIL | individual | covered |
| N22-TS3-13 | SRS-REQ-048 | `ff_serial_mpfs` TX bytes drive THR and scratch MMIO self-test | nominal | `write({0xDE,0xAD}, 2)`; `FF_SerialMpfsScratchSelfTest` | write = `FF_SUCCESS` with THR (`0x00`) receiving `0xDE` then `0xAD` in order; scratch self-test = `FF_SUCCESS` and SCR (`0x1C`) == `0xA5` (SRS-REQ-048 4-byte-spaced registers) | SIL | individual | covered |
| N22-TS3-14 | SRS-REQ-048 | `ff_serial_mpfs` RX read with DATA_READY | nominal | LSR == DATA_READY, THR == `0x55`; `read(rx,4,&got,0)` | read = `FF_SUCCESS`; `*got == 1`; `rx[0] == 0x55` (SRS-REQ-048) | SIL | individual | covered |
| N22-TS3-15 | SRS-REQ-048, SRS-REQ-030 | `ff_serial_mpfs` init argument boundaries (`data_bits`, `parity`) | OOB | `init(115200,4,0,1)`; `init(115200,8,3,1)`; boundary `data_bits` 5/8 and `parity` 0/2 accepted (N22-TS3-11/12) | both = `FF_INVALID_ARGUMENT` (`data_bits < 5` and `parity > 2` rejected) with LCR unchanged (SRS-REQ-048, `hal_serial.h`) | SIL | individual | covered |
| N22-TS3-16 | SRS-REQ-048, SRS-REQ-030 | `ff_serial_mpfs` NULL pointer arguments | OOB | `setup(NULL,&bus)`; `setup(&drv,NULL)`; `write(NULL,1,0)`; `read(NULL,4,&got,0)`; boundary `write(NULL,0,0)` | setup/`write(len≠0)`/read with NULL = `FF_INVALID_ARGUMENT`; `write(NULL,0,0)` = `FF_SUCCESS` transmitting 0 bytes (SRS-REQ-030 `hal_serial.h`) | SIL | individual | covered |
| N22-TS3-17 | SRS-REQ-048 | `ff_serial_mpfs` read timeout semantics | OOB | `read` with no DATA_READY and `timeout_ms` = 0, 1, `0xFFFFFFFF`; the loop is bounded by `FF_SERIAL_SPIN_LIMIT` (1 000 000) | **gap** — `timeout_ms` is ignored (`(void)timeout_ms`) and no clause defines the timeout return (`FF_SUCCESS` with `*got == 0` vs a timeout status); the bounded spin is an implementation detail. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_SUCCESS` with `*got == 0`. No oracle is asserted here | SIL | individual | gap |
| N22-TS3-18 | SRS-REQ-048 | `ff_serial_mpfs` baud-rate programming | OOB | `init` with `baud` = 9600 / 115200 / 3000000 | **gap** — the driver ignores `baud` and programs no divisor/latch sequence; real baud programming is GAP-A5 and no clause fixes the register sequence. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: LCR unchanged, no divisor access. No oracle is asserted here | SIL | individual | gap |
| N22-TS3-19 | SRS-REQ-048 | `ff_serial_mpfs` stop-bit values other than 2 | OOB | `init(..., stop_bits = 0, 1, 3)` | **gap** — only `stop_bits == 2` sets STOP2; 0/1/3 silently select one stop bit, but no clause defines behaviour for stop_bits 0/3. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: STOP2 clear for 0/1/3. No oracle is asserted here | SIL | individual | gap |
| N22-TS3-20 | SRS-REQ-048, SRS-REQ-049 | Serial TX buffer drawn from the static pool (combined, shared MMIO bus) | nominal | allocate the 2-byte TX payload from `FF_MemPool_t`, write it via serial, then `reset()` | the payload pointer is 8-byte aligned and inside the pool; serial writes the exact pool bytes to THR; `used` grows by the aligned size and `reset()` restores it — no heap and no buffer aliasing (SRS-REQ-049 + SRS-REQ-048) | SIL | combined | covered |
| N22-TS3-21 | SRS-REQ-048, SRS-REQ-044 | `ff_serial_mpfs` MMUART0 scratch-register round trip on the emulated SoC | nominal | Renode emulated MSS MMUART0 at `FF_MMUART0_BASE` `0x20000000`: scratch self-test then read back | per-check observable `[renode-test] hal_serial=PASS`: `FF_SerialMpfsScratchSelfTest` returns `FF_SUCCESS` and reads back `0xA5` from the emulated scratch register (FRICK-VV-TSPEC-001 §7.6: the per-check marker, not `RENODE_TEST: PASS`, is the observable) | PIL | individual | covered |
| N22-TS3-22 | SRS-REQ-049, SRS-REQ-032 | `ff_mem_static` pool init and aligned non-overlapping allocation | nominal | `init(&pool,"mem0",area,POOL_BYTES)`; `alloc(1)` → a; `alloc(8)` → b | `total == POOL_BYTES`, `used == 0` after init; a,b non-NULL; `FF_MEM_ALIGN == 8`; `(uintptr_t)a % 8 == 0` and `b % 8 == 0`; `b >= a + 8` (bounded, heap-free; SRS-REQ-049) | SIL | individual | covered |
| N22-TS3-23 | SRS-REQ-049 | `ff_mem_static` allocation is writable and read-back consistent | nominal | write `0x5A` through the returned pointer and read it back; then `reset()` and re-allocate the whole region | read-back == `0x5A`; after `reset()` `used == 0` and `alloc(POOL_BYTES) != NULL` (single-region bump allocator) | SIL | individual | covered |
| N22-TS3-24 | SRS-REQ-049 | `ff_mem_static` exact-fit and exhaustion at the region boundary | OOB | `alloc(POOL_BYTES)` then `alloc(1)`; `alloc(0)`; `alloc(2*POOL_BYTES)`; `init(..., 0)` | exact-fit `alloc(POOL_BYTES) != NULL`; the next `alloc(1) == NULL`; `alloc(0) == NULL`; `alloc(2*POOL_BYTES) == NULL` — never overflows the region (SRS-REQ-049 bounded pool) | SIL | individual | covered |
| N22-TS3-25 | SRS-REQ-049 | `ff_mem_static` init NULL/size-0 argument boundaries | OOB | `init(NULL,"mem0",area,n)`; `init(&pool,"mem0",NULL,n)`; `init(&pool,"mem0",area,0)` | all = `FF_INVALID_ARGUMENT` with the previous pool binding unchanged (SRS-REQ-049) | SIL | individual | covered |
| N22-TS3-26 | SRS-REQ-049, SRS-REQ-032 | `FF_DmaIdentity` virtual↔physical translation | nominal | `virt_to_phys(a)`; `phys_to_virt(that)` | `virt_to_phys(a) == (uint64_t)(uintptr_t)a` and `phys_to_virt(...) == a` (identity reference helper; SRS-REQ-049) | SIL | individual | covered |
| N22-TS3-27 | SRS-REQ-049 | `FF_DmaIdentity` cache-maintenance seam | OOB | call the DMA cache flush/invalidate hooks before/after a write | **gap** — the cache hooks are documented as a no-op seam; correct cache maintenance requires the target (GAP-A5), so no host oracle exists. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: no cache operation is performed. No oracle is asserted here | SIL | individual | gap |
| N22-TS3-28 | SRS-REQ-049 | `ff_mem_static` attempted second pool init (single instance) | OOB | `init(&pool,"mem0",areaA,n)` then `init(&pool2,"mem1",areaB,m)` | **gap** — the backend is a single instance and a second init silently rebinds it; no requirement defines multi-pool support or the re-init contract. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: the second init returns `FF_SUCCESS` and the previous pointers remain in the abandoned region. No oracle is asserted here | SIL | individual | gap |
| N22-TS3-29 | SRS-REQ-049, SRS-REQ-036 | Mem/DMA + GEM: frame buffer allocated from the static pool and DMA-translated | nominal | allocate a TX frame from the pool, identity-translate it to a physical address, pass it to `gem.mac.transmit` on the mock bus (boundary: pool buffer within the 32-bit descriptor address range) | the frame bytes appear in the GEM TX buffer unmodified; the GEM descriptor `addr` is the identity physical address of the pool buffer; `used` accounts for the frame — no heap and no aliasing (SRS-REQ-049 + SRS-REQ-036) | SIL | combined | covered |
| N22-TS3-30 | SSS-REQ-010 | Static, allocation-free I/O storage across GPIO/Serial/Mem/GEM | nominal | build the I/O drivers and the static pool; audit the objects/link for a heap allocator reference (`nm`/`objdump`) | the I/O drivers and the static pool draw all storage from caller-provided/bounded static objects; no `malloc`/`calloc`/`free` is referenced from these modules (SSS-REQ-010) | SIL | combined | covered |
| N22-TS3-31 | SRS-REQ-049, SRS-REQ-044 | `ff_mem_static` pool over emulated DDR on Renode | nominal | Renode: init the pool over the emulated DDR window, allocate, write, read back | per-check observable `[renode-test] hal_mem=PASS`: init/alloc/write/read-back over the emulated DDR succeed with aligned, non-overlapping addresses (FRICK-VV-TSPEC-001 §7.6) | PIL | individual | covered |
| N22-TS3-32 | SRS-REQ-036, SRS-REQ-026 | `ff_gem` setup + init: NCR RE/TE and NCFGR gigabit/full-duplex | nominal | `FF_GemSetup(&gem,&cfg)` (bus, MAC, speed 1000, full-duplex); `gem.mac.init()` | init = `FF_SUCCESS`; NCR has RE and TE set; NCFGR has GBE (gigabit) and FD (full duplex) set (SRS-REQ-036 / `hal_eth.h`) | SIL | individual | covered |
| N22-TS3-33 | SRS-REQ-036 | `ff_gem` station-address programming and `get_mac_address` | nominal | `cfg.mac = {02 00 00 12 34 56}`; `get_mac_address(mac)` | SA1B == `0x22110002`, SA1T == `0x4433` (little-endian split); `get_mac_address` returns the same 6 bytes (SRS-REQ-036) | SIL | individual | covered |
| N22-TS3-34 | SRS-REQ-036 | `ff_gem` 64-bit descriptor-ring base (RBQP/TBQP + `_HI`) and `DMACFG.ADDR64` | nominal | init over a mock MMIO bus; read RBQP/TBQP/RBQP_HI/TBQP_HI/DMACFG | RBQP == low 32 bits of `&rx_desc[0]` and RBQP_HI == bits 63..32; TBQP/TBQP_HI likewise for `&tx_desc[0]` (`0x00CC`/`0x00C8`); DMACFG bit 30 (ADDR64) set (SRS-REQ-036 64-bit queue addressing) | SIL | individual | covered |
| N22-TS3-35 | SRS-REQ-036 | `ff_gem` transmit a frame (TSR.COMP) and statistics | nominal | transmit a 42-byte frame; `get_stats` | transmit = `FF_SUCCESS`; TSR.COMP observed; `tx == 1`, `rx == 0`, `err == 0` (SRS-REQ-036) | SIL | individual | covered |
| N22-TS3-36 | SRS-REQ-036 | `ff_gem` RX poll: dispatched count, length, descriptor re-arm | nominal | one `rx_desc` with USED set and length 42 and a registered callback; `FF_GemPollRx` twice | first poll returns 1, the callback receives len 42, `rx_desc[0].addr` USED bit cleared (re-armed); second poll returns 0 (SRS-REQ-036) | SIL | individual | covered |
| N22-TS3-37 | SRS-REQ-036 | `ff_gem` transmit size boundary (NULL / 0 / MTU+1) and error accounting | OOB | `transmit(NULL,len)`; `transmit(frame,0)`; `transmit(frame, FF_GEM_MTU+1 = 1537)`; boundary `transmit(frame, 1536)` | NULL/0/1537 → `FF_BAD_SIZE` and the error counter increments by 1 for the rejected call; the exact-MTU 1536 frame is accepted (`FF_GEM_MTU` contract) | SIL | individual | covered |
| N22-TS3-38 | SRS-REQ-036 | `ff_gem` TX ring-full (all `FF_GEM_TX_DESC` = 4 descriptors DMA-owned) | OOB | mark all 4 tx descriptors DMA-owned (USED == 0) and call `transmit` | transmit = `FF_NOT_READY` with the error counter incremented; no descriptor is overwritten and the frame buffer is untouched (SRS-REQ-036 bounded ring) | SIL | individual | covered |
| N22-TS3-39 | SRS-REQ-036 | `ff_gem` TX timeout: TSR.COMP never set within `FF_GEM_TX_POLL` | OOB | mock bus that never latches TSR.COMP; transmit a valid frame | transmit = `FF_ERROR` after the bounded poll; the descriptor is reclaimed (USED set) and `tx_head` advances so the ring is not permanently wedged; the error counter increments (SRS-REQ-036; driver timeout contract) | SIL | individual | covered |
| N22-TS3-40 | SRS-REQ-036 | `ff_gem` descriptor address field at/above the 32-bit range | OOB | place a TX/RX frame buffer or descriptor ring at an address ≥ `0x10_0000_0000` (4 GiB) and transmit | **gap** — `FF_GemDesc_t.addr` is 32-bit; the 64-bit queue base (RBQP_HI/TBQP_HI + ADDR64) covers the ring base but not a per-descriptor address ≥ 4 GiB. 64-bit descriptor addressing is an M14 deferral (NC-S6-01/NC-S6-03; NC-EXT-47 §7.3); no clause defines the truncation/rejection. Uncontrolled observation only: `addr` = `(uint32_t)` low 32 bits. No oracle is asserted here | SIL | individual | gap |
| N22-TS3-41 | SRS-REQ-036, SRS-REQ-043 | GEM DMA round trip on the emulated SoC | OOB | Renode emulated `Network.CadenceGEM`: arm the descriptor engine and attempt a transmit | **gap** — the Renode `Network.CadenceGEM` model has no DMA descriptor engine (TSTART rejected as an unhandled bit) and the descriptor translation is 32-bit, so a DMA round trip is not observable on the emulator; on-target GEM DMA/PADDR is GAP-A5 (NC-S6-01/NC-S6-03; NC-EXT-47 §7.5). Uncontrolled observation only: no frame is transmitted. No oracle is asserted here | PIL | individual | gap |
| N22-TS3-42 | SRS-REQ-036, SRS-REQ-044 | GEM identification on the emulated SoC (MODID) | nominal | read `FF_GEM_MODID_OFF` (`0xFC`) on the emulated GEM at `FF_GEM0_BASE` `0x20110000` | per-check observable `[renode-test] hal_eth=PASS`: identification bits [27:16] == `0x002` and revision [15:0] == `0x0118` (Cadence defaults); an MMIO presence/identification check, not a DMA/link oracle (FRICK-VV-TSPEC-001 §7.6) | PIL | individual | covered |
| N22-TS3-43 | SRS-REQ-036, SRS-REQ-026 | GEM + netstack loopback: MAC frame egress path | nominal | bound loopback `FF_NetStack_t` and `FF_EthMac_t`; drive a frame from the net layer to the MAC and back through the RX callback | the exact frame bytes reach `mac.transmit`; the mock RX callback delivers the same bytes to the stack with the frame length preserved; the net-interface vtable wires stack↔MAC without core change (SRS-REQ-026 + SRS-REQ-036/039) | SIL | combined | covered |
| N22-TS3-44 | SRS-REQ-040, SRS-REQ-044 | GEM + lwIP netif on the emulated SoC | nominal | Renode: bind `FF_NetIf_t` (lwIP stack) to the emulated GEM MAC, configure the GEM MAC over MMIO, confirm netif up | per-check observable `[renode-test] hal_net_lwip=PASS`: attach succeeds, the netif reports up and its hardware address equals the configured GEM MAC `{02 00 00 12 34 56}`; TX/RX are deliberately not armed (register-only GEM model) (SRS-REQ-040; §7.6) | PIL | combined | covered |
| N22-TS3-45 | SRS-REQ-039, SRS-REQ-027 | `ff_net_loopback` socket stream round trip (both directions) | nominal | open/bind/listen (srv), connect (cli), accept (acc); `send(cli)`→`recv(acc)`, `send(acc)`→`recv(cli)` | bind/listen/connect/accept = `FF_SUCCESS`; recv returns the exact sent bytes with `got == len` in both directions (SRS-REQ-039 `FF_NetStack_t`) | SIL | individual | covered |
| N22-TS3-46 | SRS-REQ-039 | `ff_net_loopback` bounded socket set and ring-buffer wrap | nominal | open 4 sockets then a 5th; send 256 bytes (full 256-byte peer ring, wrapping) then recv 256 | the first 4 opens return valid handles and the 5th is negative; send = `FF_SUCCESS` with `*sent == 256`; recv returns 256 wrapped bytes in order (SRS-REQ-039 bounded/static) | SIL | individual | covered |
| N22-TS3-47 | SRS-REQ-039 | `ff_net_loopback` send/connect without a peer | OOB | send on an unconnected socket; connect with nothing listening | send = `FF_ERROR` (unconnected); connect = `FF_ERROR` (nothing listening); no handle state change (`hal_net.h`/SRS-REQ-039) | SIL | individual | covered |
| N22-TS3-48 | SRS-REQ-039 | `ff_net_loopback` invalid/out-of-range handles and double close | OOB | `close(65536)`, `recv(65536,…)`, `send(65536,…)`; close an already-closed handle | handle 65536 → `FF_INVALID_ARGUMENT` for send/recv/close; the second close → `FF_INVALID_ARGUMENT` (`hal_net.h` handle contract) | SIL | individual | covered |
| N22-TS3-49 | SRS-REQ-039 | `ff_net_loopback` bind/listen argument and duplicate-port boundaries | OOB | `bind(NULL,1)`; `bind(addr,0)`; bind the same port twice (same socket, another socket); `listen` without bind | NULL/len 0 → `FF_INVALID_ARGUMENT`; duplicate bind → `FF_ALREADY_EXISTS` (self and cross-socket); `listen` without bind → `FF_INVALID_ARGUMENT` (`hal_net.h`/SRS-REQ-039) | SIL | individual | covered |
| N22-TS3-50 | SRS-REQ-039 | `ff_net_loopback` non-blocking recv and empty accept queue | OOB | recv on an empty connected socket with timeout 0; accept on an empty accept queue | recv = `FF_SUCCESS` with `*got == 0` (non-blocking, no data); accept returns a negative `FF_NetSock_t` (`hal_net.h`: `open`/`accept` fail negative; N22-TS3-46) | SIL | individual | covered |
| N22-TS3-51 | SRS-REQ-039 | `ff_net_loopback` NULL out-pointer arguments | OOB | `send(a,buf,1,NULL)`; `recv(a,NULL,1,&got,0)`; `recv(a,buf,1,NULL,0)` | each = `FF_INVALID_ARGUMENT` (out-pointer parameters are required; `hal_net.h`) | SIL | individual | covered |
| N22-TS3-52 | SRS-REQ-040 | lwIP adapter TCP round trip over 127.0.0.1 | nominal | addresses as 6 octets (IPv4 4 + port 2, big-endian) `127.0.0.1:12345`; bind/listen/connect/accept; send/recv | all calls = `FF_SUCCESS`; the received bytes equal the sent bytes with `sent`/`got` == message length (SRS-REQ-040) | SIL | individual | covered |
| N22-TS3-53 | SRS-REQ-040 | lwIP adapter address-length and NULL argument boundaries | OOB | `connect(sd,addr,3)`; `bind(sd,addr,0)`; bind/connect with NULL; send/recv with NULL out-pointer | `addr_len < 6` (and 0) → `FF_INVALID_ARGUMENT`; NULL address/out-pointer → `FF_INVALID_ARGUMENT` (`ff_net_lwip.h`/`hal_net.h`) | SIL | individual | covered |
| N22-TS3-54 | SRS-REQ-040 | lwIP adapter recv timeout | OOB | recv with `timeout_ms > 0` and no incoming data (`SO_RCVTIMEO` configured by lwipopts) | **gap** — `timeout_ms <= 0` means "no timeout"; the return on expiry depends on the lwIP/pthread backend (`FF_ERROR` vs `FF_SUCCESS`/0) and no clause fixes it. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: a backend-dependent return. No oracle is asserted here | SIL | individual | gap |
| N22-TS3-55 | SRS-REQ-040 | `ff_lwip_netif` attach with a complete MAC vtable | nominal | attach an `FF_NetIf_t` whose MAC provides `init/get_mac_address/transmit/set_rx_callback` | attach = `FF_SUCCESS`; the netif is bound and brought up (SRS-REQ-040; host-tested, NC-154-01 closed) | SIL | individual | covered |
| N22-TS3-56 | SRS-REQ-040 | `ff_lwip_netif` attach with a partial/NULL vtable and double attach | OOB | attach a `FF_NetIf_t` missing any required operation; attach NULL; attach twice | a missing operation (or NULL) → `FF_INVALID_ARGUMENT` without attaching; a second attach → `FF_ALREADY_EXISTS` (singleton `g_attached`) (SRS-REQ-040) | SIL | individual | covered |
| N22-TS3-57 | SRS-REQ-040, SRS-REQ-026 | `ff_lwip_netif` status getters and the TX path through a mock MAC | nominal | after a successful attach: `IsUp()`; `GetMac(buf)`; `GetMac(NULL)`; `mac.transmit(frame)`; `mac.transmit(NULL,0)` | `IsUp() == 1`; `GetMac` returns the configured 6-byte MAC; `GetMac(NULL)` → `FF_INVALID_ARGUMENT`; the TX frame arrives at the mock MAC unchanged (tx call count + 1, bytes equal); `mac.transmit(NULL,0)` → `FF_BAD_SIZE` (SRS-REQ-040 + SRS-REQ-026) | SIL | individual | covered |
| N22-TS3-58 | SRS-REQ-027, SRS-REQ-039, SRS-REQ-040 | Pluggable stack: the same `FF_NetStack_t` surface drives loopback and lwIP | nominal | run the common socket sequence (open/bind/listen/connect/send/recv/close) against `FF_NetStack_t` = loopback and = lwIP | identical status-code and byte-throughput semantics for both backends with no change to the calling code — the network is reached only through the stack-independent contract (SRS-REQ-027) | SIL | combined | covered |
| N22-TS3-59 | SRS-REQ-039, SRS-REQ-044 | Reference loopback stack exercised on Renode | nominal | Renode `hal_net` check: in-process loopback contract/behaviour sequence | per-check observable `[renode-test] hal_net=PASS`: the loopback `FF_NetStack_t` round trip succeeds on the emulated SoC (in-process, no emulated peripheral bound) (SRS-REQ-039; §7.6) | PIL | individual | covered |
| N22-TS3-60 | SRS-REQ-038, SRS-REQ-028 | `ff_gem_tsn` setup/init and `set_vlan` PCP programming | nominal | `FF_TsnGemSetup(&t,&bus,0,1000)`; `init()`; `set_vlan(100, 5)`; boundary PCP 0 and 7 | init = `FF_SUCCESS`; `set_vlan` = `FF_SUCCESS`; SCRT2 (`0x0540`) VLANPR field == 5 and VLANEN set; PCP 0 and 7 accepted (SRS-REQ-038 802.1Q VLAN/PCP; `hal_tsn.h`) | SIL | individual | covered |
| N22-TS3-61 | SRS-REQ-038 | `ff_gem_tsn` `set_vlan` PCP out of range | OOB | `set_vlan(100, 8)`; `set_vlan(100, 9)` | `FF_INVALID_ARGUMENT` (PCP > 7) with no SCRT2 write (SRS-REQ-038/`hal_tsn.h`) | SIL | individual | covered |
| N22-TS3-62 | SRS-REQ-038 | `ff_gem_tsn` `configure_tas` ENST gate programming and ns→hardware scaling | nominal | one gate queue 3, `start_nsec` 250000, `on_nsec` 1000, `off_nsec` 2000 at 1000 Mb/s | `FF_SUCCESS`; ENST_CONTROL (`0x0880`) bit 3 set; `ENST_ON_TIME(3) == 125`, `ENST_OFF_TIME(3) == 250` (`ns*speed/8000`) (SRS-REQ-038 802.1Qbv) | SIL | individual | covered |
| N22-TS3-63 | SRS-REQ-038 | `ff_gem_tsn` `configure_tas` argument boundaries | OOB | NULL list; count 0; count > 8; queue 8; `start_sec` 4 (> 3); `start_nsec` above `0x3FFFFFFF`; `on_nsec` 2e9 (hardware 2.5e8 > `0x1FFFF`) | each = `FF_INVALID_ARGUMENT` (table bounds; ENST 17-bit scaled time field) (SRS-REQ-038; `ff_gem_regs.h`) | SIL | individual | covered |
| N22-TS3-64 | SRS-REQ-038 | `ff_gem_tsn` `get_time_ns` from the GEM TSU | nominal | TSH seconds 2, TN `500000000` | `FF_SUCCESS`; ns == 2 500 000 000 (`sec*1e9 + (TN & 0x3FFFFFFF)`) (SRS-REQ-038 802.1AS TSU) | SIL | individual | covered |
| N22-TS3-65 | SRS-REQ-038 | `ff_gem_tsn` `get_time_ns` NULL argument | OOB | `get_time_ns(NULL)` | `FF_INVALID_ARGUMENT` (`hal_tsn.h`) | SIL | individual | covered |
| N22-TS3-66 | SRS-REQ-066 | `ff_gem_tsn` `set_stream_filter` (802.1Qci) enable/disable | nominal | `set_stream_filter(2, dest_mac, vid 100, ethertype 0x0800, enable 1)` then `(..., enable 0)` | both = `FF_SUCCESS`; the screener type-1 entry is bound to queue 2 (SRS-REQ-066 802.1Qci entry per traffic-class queue) | SIL | individual | covered |
| N22-TS3-67 | SRS-REQ-066 | `ff_gem_tsn` `set_stream_filter` argument boundaries | OOB | NULL `dest_mac`; queue 8; queue 4..7 (≥ `FF_GEM_SCRT1_ENTRY_COUNT` 4); vid 5000 (> 4094); disable of a non-enabled entry | NULL/queue 8/vid 5000 → `FF_INVALID_ARGUMENT`; queue 4..7 → `FF_NOT_IMPLEMENTED`; disable of a non-enabled entry → `FF_NOT_FOUND` (SRS-REQ-066; `ff_gem_regs.h`) | SIL | individual | covered |
| N22-TS3-68 | SRS-REQ-066 | `ff_gem_tsn` `configure_cbs` (802.1Qav) documented not-implemented path | OOB | `configure_cbs(1, 1000000)` | `FF_NOT_IMPLEMENTED`, as documented by SRS-REQ-066 (GEM register subset; software fallback NC-S8-01). Oracle = the documented not-implemented status | SIL | individual | covered |
| N22-TS3-69 | SRS-REQ-066 | `ff_gem_tsn` `enable_pfc` (802.1Qbb) documented not-implemented path | OOB | `enable_pfc(0x01)` | `FF_NOT_IMPLEMENTED`, as documented by SRS-REQ-066 (fabric CoreTSN switch; NC-S8-02). Oracle = the documented not-implemented status | SIL | individual | covered |
| N22-TS3-70 | SRS-REQ-038 | `ff_gem_tsn` VID-based classification | OOB | `set_vlan` with a non-zero vid; inspect whether the GEM screener VLAN-VID match is programmed | **gap** — the driver programs PCP/VLANEN but not the VID (the mission screener records VID); no clause defines the driver's VID behaviour. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: VLANPR/VLANEN programmed, VID not written. No oracle is asserted here | SIL | individual | gap |
| N22-TS3-71 | SRS-REQ-038 | `ff_gem_tsn` `configure_tas` partial application on a later-gate rejection | OOB | a gate list whose gate[0] is valid and gate[1] is invalid (e.g. `start_sec` 4); inspect ENST registers after the call | **gap** — the function disables ENST_CONTROL and writes gate[0] before returning `FF_INVALID_ARGUMENT` for gate[1]; no clause defines table-reconfiguration atomicity. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: gate[0] registers hold the earlier values with ENST disabled. No oracle is asserted here | SIL | individual | gap |
| N22-TS3-72 | SRS-REQ-038, SRS-REQ-068 | TSN + gPTP combined: the TSU time read feeds the gPTP servo | nominal | both bound to the same mock TSU; gPTP reads `get_time_ns` during Sync/Follow_Up | gPTP uses the exact TSU nanoseconds; the TSN register block and the gPTP frame path are independent seams (SRS-REQ-068 + SRS-REQ-038) | SIL | combined | covered |
| N22-TS3-73 | SRS-REQ-036, SRS-REQ-038 | TSN + GEM register-block isolation on one mock bus | nominal | same mock bus; init GEM (DMACFG.ADDR64 etc.) and TSN (ENST/SCRT2), then read both | the GEM DMA/queue registers and the ENST/SCRT2 TSN registers retain their distinct values with no aliasing (SRS-REQ-036 + SRS-REQ-038) | SIL | combined | covered |
| N22-TS3-74 | SRS-REQ-038, SRS-REQ-044 | GEM native TSN on the emulated SoC | nominal | Renode `hal_tsn` check: `init` + `set_vlan(1,3)` + `get_time_ns` on the emulated Cadence GEM | per-check observable `[renode-test] hal_tsn=PASS`: the calls succeed peripheral-bound; the emulated 802.1Q screener register is not fully modelled (the VLAN/PCP write is a model no-op), so this is a seam/register-presence observable, not a classification oracle (§7.6) | PIL | individual | covered |
| N22-TS3-75 | SRS-REQ-068 | `ff_gptp` init and clock-identity derivation | nominal | `FF_GptpInit(&port,&mac,&tsn)`; derive EUI-64 from the MAC; `port.state`/`port.role` | init = `FF_SUCCESS`; `state == FF_GPTP_SM_UNCALIBRATED`; `role == FF_GPTP_ROLE_UNCERTAIN`; the clock identity is derived from the MAC (EUI-64) (SRS-REQ-068) | SIL | individual | covered |
| N22-TS3-76 | SRS-REQ-068 | `ff_gptp` Sync transition to slave | nominal | valid Sync from a master at local arrival 1.5 s | `state == FF_GPTP_SM_SLAVE`; `role == FF_GPTP_ROLE_SLAVE`; `last_sync_ns == 1500000000` (SRS-REQ-068 port state machine) | SIL | individual | covered |
| N22-TS3-77 | SRS-REQ-068 | `ff_gptp` Sync + Follow_Up time-offset and grandmaster time | nominal | master time 1.7 s, local arrival 1.5 s | `servo.offset_ns == 200000000` (+200 ms, clamped ±100000 ppb); `FF_GptpGetGrandmasterTime` == local + offset (1.8 s) (SRS-REQ-068 PI servo) | SIL | individual | covered |
| N22-TS3-78 | SRS-REQ-068 | `ff_gptp` BMCA selects master when no better clock is heard | nominal | `FF_GptpTick(&port, 0)` with no announce/sync received | `role == FF_GPTP_ROLE_MASTER`; `state == FF_GPTP_SM_MASTER` (SRS-REQ-068 BMCA) | SIL | individual | covered |
| N22-TS3-79 | SRS-REQ-068 | `ff_gptp` PDelay_Req TX and PDelay_Resp/Follow_Up link delay | nominal | `FF_GptpSendPDelayReq(&port, 5e9)`; then PDelay_Resp (`resp_time` 2e9) and PDelay_Follow_Up (2.1e9) | TX injects one frame with EtherType `0x88F7` and message type PDELAY_REQ (`0x02`); PDelay_Resp stores `delay_ns == 2000000000` (SRS-REQ-068 PDelay) | SIL | individual | covered |
| N22-TS3-80 | SRS-REQ-068 | `ff_gptp` init and getter NULL arguments | OOB | `Init(NULL,…)`; `Init(…,NULL,…)`; `Init(…,…,NULL)`; `GetGrandmasterTime(port,NULL)`; `GetGrandmasterTime(NULL,&gm)`; `GetRole(NULL)`; `GetState(NULL)` | Init with a NULL argument → `FF_INVALID_ARGUMENT`; `GetGrandmasterTime` with NULL → `FF_INVALID_ARGUMENT`; `GetRole(NULL)` → `FF_GPTP_ROLE_DISABLED`; `GetState(NULL)` → `FF_GPTP_SM_DISABLED` (`ff_gptp.h`) | SIL | individual | covered |
| N22-TS3-81 | SRS-REQ-068 | `ff_gptp` disabled port rejects processing | OOB | `SetEnabled(&port,0)`; `ProcessFrame(valid Sync)`; `Tick` | `state == FF_GPTP_SM_DISABLED`; `ProcessFrame` on a disabled port → `FF_INVALID_ARGUMENT`; `Tick` is a no-op returning `FF_SUCCESS` (SRS-REQ-068 port state machine) | SIL | individual | covered |
| N22-TS3-82 | SRS-REQ-068 | `ff_gptp` malformed frame rejection | OOB | len 10 (< 14 + 34); wrong EtherType `0x0800`; unknown message type `0x0F`; PDelay_Resp shorter than 14 + 34 + 16 | short frame → `FF_BAD_SIZE`; wrong EtherType → `FF_PARSE_ERROR`; unknown message type → `FF_PARSE_ERROR`; short PDelay_Resp → `FF_BAD_SIZE` (SRS-REQ-068; `ff_gptp.h`) | SIL | individual | covered |
| N22-TS3-83 | SRS-REQ-068 | `ff_gptp` message timeout, sequence jump, duplicate timestamp, domain mismatch | OOB | no Sync for N intervals; Sync with a jumped `sequenceId`; duplicate Sync/Follow_Up; Sync from a foreign `domainNumber` | **gap** — the SRS/contract do not define timeout, sequence-jump, duplicate-timestamp or domain-mismatch handling of the state machine. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: the servo/state reacts as implemented. No oracle is asserted here | SIL | individual | gap |
| N22-TS3-84 | SRS-REQ-068, SRS-REQ-026 | gPTP + GEM MAC frame path | nominal | gPTP TX/RX through a mock `FF_EthMac_t` | gPTP sends/receives PTP frames exactly through the MAC vtable (EtherType `0x88F7`, message type, timestamp offset) with no direct device coupling (SRS-REQ-068 + SRS-REQ-026) | SIL | combined | covered |
| N22-TS3-85 | SRS-REQ-068 | On-target gPTP time accuracy (HIL) | OOB | measure gPTP offset/skew against a reference grandmaster on the RTPFS160ZT board | **gap** — GAP-A5: on-target PTP timing accuracy requires flight hardware and an on-board campaign (NC-EXT-47 §7.5). Uncontrolled observation only: none possible without the target board. No oracle is asserted here | HIL | individual | gap |
| N22-TS3-86 | SRS-REQ-067, SRS-REQ-068 | Fabric CoreTSN / 802.1CB FRER path | OOB | multi-port switch / FRER path reached through the generic fabric-device (FIC/AXI) contract | **gap** — depends on the generic fabric-device contract SRS-REQ-067, which is planned (not delivered) and tracked in NC-S8-02. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: no fabric-device API exists. No oracle is asserted here | SIL | combined | gap |
| N22-TS3-87 | SRS-REQ-043, SRS-REQ-044 | Renode per-check observables vs the aggregate `RENODE_TEST: PASS` marker | OOB | rely on the aggregate `RENODE_TEST: PASS` line alone as the I/O requirement oracle | **gap** — the aggregate marker is not a per-requirement oracle; the per-check lines (`hal_gpio`/`hal_serial`/`hal_mem`/`hal_eth`/`hal_net`/`hal_tsn`/`hal_net_lwip`) are the observables (FRICK-VV-TSPEC-001; NC-EXT-47 §7.6). Uncontrolled observation only: the aggregate `RENODE_TEST: PASS` line itself. No oracle is asserted for the aggregate line | PIL | individual | gap |
| N22-TS3-88 | SRS-REQ-047, SRS-REQ-048 | On-target MSS GPIO / MMUART MMIO behaviour (HIL) | OOB | run the GPIO configure/read/write and MMUART scratch/TX/RX sequence on the RTPFS160ZT silicon | **gap** — GAP-A5 / NC-EXT-31: real-silicon MMIO, electrical timing and interrupt behaviour require the board (NC-EXT-47 §7.5). Uncontrolled observation only: none possible without the target board. No oracle is asserted here | HIL | individual | gap |
| N22-TS3-89 | SRS-REQ-036 | On-target GEM PHY/link and DMA round trip (HIL) | OOB | link up a PHY, transmit/receive a real frame through the GEM DMA on target | **gap** — GAP-A5: PHY/link behaviour and the GEM DMA/PADDR round trip require the board (NC-S6-01/NC-S6-03; NC-EXT-47 §7.5). Uncontrolled observation only: none possible without the target board. No oracle is asserted here | HIL | individual | gap |
| N22-TS3-90 | SRS-REQ-040 | On-target lwIP netif hardware data path (HIL) | OOB | bind the lwIP netif to the real GEM MAC and complete an IP round trip on target hardware | **gap** — GAP-A5: the on-target stack/netif data path requires the board (NC-EXT-47 §7.5). Uncontrolled observation only: none possible without the target board. No oracle is asserted here | HIL | individual | gap |

## 3. Gaps (mapping to NC-EXT-47 sub-items)

All gaps are recorded, not resolved (N21). Each row below maps a `gap` case to
the NC-EXT-47 sub-item in FRICK-VV-TSPEC-001 §7 that it belongs to, and names
the specific open NC where one exists. Gap rows assert **no** oracle.

**NC-EXT-47 §7.3 — out-of-bound oracles not defined by a controlled clause
(unresolved/absent contract).**

- `N22-TS3-6` — GPIO interrupt contract: the ISR→task rule of SRS-REQ-031 is not
  implemented (TBD; NC-EXT-47 §7.3).
- `N22-TS3-17` — serial read timeout semantics: `timeout_ms` ignored and no
  timeout-return contract (TBD; NC-EXT-47 §7.3).
- `N22-TS3-18` — serial baud-rate divisor programming (GAP-A5; NC-EXT-47 §7.3).
- `N22-TS3-19` — serial stop-bit values other than 2 (TBD; NC-EXT-47 §7.3).
- `N22-TS3-27` — DMA cache-maintenance seam; real cache ops are on-target work
  (GAP-A5; NC-EXT-47 §7.3).
- `N22-TS3-28` — static-pool second-init rebinding / multi-pool contract (TBD;
  NC-EXT-47 §7.3).
- `N22-TS3-40` — GEM 64-bit descriptor addressing below the 4 GiB boundary
  (NC-S6-01/NC-S6-03; NC-EXT-47 §7.3).
- `N22-TS3-54` — lwIP adapter recv-timeout return (TBD; NC-EXT-47 §7.3).
- `N22-TS3-70` — TSN driver VID-based classification not programmed (TBD;
  NC-EXT-47 §7.3).
- `N22-TS3-71` — `configure_tas` reconfiguration atomicity on a later-gate
  rejection (TBD; NC-EXT-47 §7.3).
- `N22-TS3-83` — gPTP message-timeout/sequence-jump/duplicate-timestamp/domain
  handling (TBD; NC-EXT-47 §7.3).
- `N22-TS3-86` — fabric CoreTSN / 802.1CB FRER path with the planned
  fabric-device contract (SRS-REQ-067 planned; NC-S8-02; NC-EXT-47 §7.3).

**NC-EXT-47 §7.2 — software-only robustness oracles not specified as a
controlled, requirement-mapped set.**

- `N22-TS3-7` — `ff_gpio_mpfs` MMIO bus-fault behaviour (TBD;
  NC-EXT-47 §7.2/§7.3).

**NC-EXT-47 §7.1 — combined-I/O scenarios.**

- All seven I/O surfaces of this package have at least one combined case
  (`N22-TS3-8/9/20/29/30/43/44/58/72/73/84`); the residual combined pairs that
  involve modules owned by other packages (GPIO + watchdog/actuation, serial +
  GSW byte stream, TSN + `link_tsn`, netstack + `link_ip`) are recorded under
  TS-2/TS-4/TS-5, not here (NC-EXT-47 §7.1).

**NC-EXT-47 §7.5 — target (HIL) evidence remains GAP-A5 / NC-EXT-31.**

- `N22-TS3-41` — GEM DMA round trip on the emulated SoC (register-only GEM
  model; NC-S6-01/NC-S6-03; NC-EXT-47 §7.5).
- `N22-TS3-85` — on-target gPTP timing accuracy (GAP-A5).
- `N22-TS3-88` — on-target MSS GPIO/MMUART MMIO behaviour (GAP-A5 /
  NC-EXT-31).
- `N22-TS3-89` — on-target GEM PHY/link and DMA round trip (GAP-A5).
- `N22-TS3-90` — on-target lwIP netif hardware data path (GAP-A5).

**NC-EXT-47 §7.6 — emulated-target aggregate marker is not a per-requirement
oracle.**

- `N22-TS3-87` — `RENODE_TEST: PASS` does not evidence the I/O requirements;
  the per-check `[renode-test] …=PASS` lines in `N22-TS3-21/31/42/44/59/74` are
  the observables (TBD; NC-EXT-47 §7.6).

### 3.1 Coverage summary

| Metric | Count |
| --- | --- |
| Total cases | 90 |
| Nominal | 45 |
| Out-of-bound | 45 |
| Individual | 78 |
| Combined | 12 |
| `covered` (oracle grounded in a requirement/interface clause) | 71 |
| `gap` (no controlled oracle; recorded under NC-EXT-47) | 19 |
| SIL / PIL / HIL | 78 / 8 / 4 |

Requirement coverage in this scope: `SRS-REQ-025/026/027/028/030/031/032/036/
038/039/040/043/044/047/048/049/066/067/068` and `SSS-REQ-010` all have at least
one case; every I/O surface (GPIO, Serial, Mem/DMA, GEM, netstack/lwIP, TSN,
gPTP) has an individual nominal case, an individual out-of-bound case and at
least one combined case with an adjacent subsystem, except where the combined
case is the fabric/environment gap (`N22-TS3-86/87`). The GEM 64-bit/descriptor,
TSN table-atomicity, gPTP state-machine and cache-maintenance gaps are recorded
under NC-EXT-47, not resolved by a fabricated oracle.

## History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-14 | frick-fsw | Initial TS-3 work package per N22 and the FRICK-VV-TSPEC-002 chunk plan: nominal + out-of-bound case table with explicit oracles for I/O group A (MSS GPIO, MSS MMUART serial, static RAM pool + DMA helper, Cadence GEM Ethernet, loopback/lwIP network stack, GEM native TSN, 802.1AS gPTP); gaps mapped to NC-EXT-47 sub-items with the open NC/TBD named |
