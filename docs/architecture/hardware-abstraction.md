# Hardware / OS Abstraction Layer (HAL)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-ARCH-0008                        |
| Revision                          | 0.2                                    |
| Date                              | 2026-10-10                             |
| Norms                             | N1, N3, N4, N8                         |
| Requirements                      | SSS-REQ-017, SRS-REQ-025..032          |

## 1. Purpose

The HAL is the single seam between the framework and every bus, device and
memory resource (milestone M3). The framework depends only on the HAL contracts,
never on a specific controller, PHY, stack or board, so that:

- a user can integrate their **own drivers and third-party stacks** without
  modifying the framework;
- the low-level implementations are **optional and separable** (a user takes
  only what they need, objective OBJ-07);
- the design is **hardware-independent** and testable on the host simulator.

The OS primitives (tasks, mutexes, queues, clock) are abstracted by the **PAL**
(`src/core/pal/pal.h`); the HAL covers everything below that: buses, memory and
fabric-attached devices. See `docs/design/software-design.md`.

## 2. Layering

```
 mission apps (user)            │  user-provided
 reference apps                 │  this repo (examples)
 PUS / core services / transport│  this repo (middleware)
 PAL (OS abstraction)           │  this repo (POSIX + RTEMS)
 HAL registry + contracts       │  this repo (interfaces; M3)
 BSP/drivers (M4)               │  reference impls (optional, separable)
 MPFS MSS peripherals / fabric  │  hardware / FPGA IP (not in this repo)
```

## 3. Contract catalogue

One header per resource family under `src/hal/`. "MPFS" is the Microchip RT
PolarFire SoC RTPFS160ZT; MSS = native subsystem, fabric = FPGA fabric reached
through the FIC.

| Family | Header | Contract type | MPFS resource | Native/fabric |
| ------ | ------ | ------------- | ------------- | ------------- |
| Core registry | `hal.h` | `FF_HalClass_t`, `FF_HalRegister/Get/Count` | — | — |
| Ethernet | `hal_eth.h` | `FF_EthMac_t`, `FF_NetIf_t` | GEM x2 | MSS |
| Network stack | `hal_net.h` | `FF_NetStack_t` (socket-like) | — (stack) | pluggable |
| TSN | `hal_tsn.h` | `FF_TsnBackend_t` | GEM internal TSN block (802.1Q/Qbv/Qav/Qci/Qbb/802.1AS/1588/CB) | MSS (fabric CoreTSN only for multi-port switch/FRER) |
| CAN | `hal_can.h` | `FF_CanBackend_t`, `FF_CanOpenStack_t` | CAN x2 (classic) | MSS; FD = fabric |
| Serial | `hal_serial.h` | `FF_Serial_t` (RS-485 DE) | MMUART x5 | MSS |
| SPI / I2C | `hal_spi.h` | `FF_Spi_t`, `FF_I2c_t` | SPI x2, I2C x2 | MSS |
| PCIe | `hal_pcie.h` | `FF_Pcie_t` | PCIe Gen2 (XCVR0) | hard block |
| GPIO / IRQ | `hal_gpio.h` | `FF_Gpio_t` | GPIO (MSS x3 + fabric) | MSS + fabric |
| SpaceWire | `hal_spacewire.h` | `FF_SpaceWire_t`, `FF_Rmap_t` | — | fabric |
| Memory / storage | `hal_mem.h` | `FF_MemPool_t`, `FF_Dma_t`, `FF_Mtd_t`, `FF_Nvm_t`, `FF_BlockDev_t` | DDR, QSPI NOR, eNVM, MMC | MSS |

The **registry** (`hal.c`) is bounded and allocation-free: at most
`FF_HAL_MAX_BACKENDS` backends per class; duplicate names and invalid classes are
rejected. Registration names are caller-owned (typically string literals).

## 4. Obligations the middleware places on integrators

A conforming backend must respect, and document, the following (these are the
"specificities" the framework needs and they are part of the contract review):

1. **Buffer ownership** — who allocates/frees a buffer, and for how long it must
   remain valid after a call returns (e.g. TX buffers until completion).
2. **DMA and cache coherence** — on RISC-V, cache flush/invalidate around DMA;
   the `FF_Dma_t` contract exposes `virt_to_phys`, `cache_flush`,
   `cache_invalidate`.
3. **ISR vs task context** — ISRs do the minimum and defer work to a task; a
   mutex is not ISR-safe (see the time base). The `FF_Gpio_t` interrupt
   contract and the driver guide state this.
4. **No dynamic allocation** — after initialisation, no heap use (SSS-REQ-010);
   pools are static and bounded.
5. **Bounded latency and work** — no unbounded loops or blocking in a callback.
6. **Endianness and wire format** — network/big-endian on the wire; explicit
   byte ordering at the contract boundary.
7. **Error model** — every fallible operation returns `FF_Status_t`; faults are
   reportable (EVS).
8. **Half-duplex / RS-485 direction** — the serial contract exposes
   `set_rs485(en, de_assert_us)`; the backend must release the driver enable
   within the bound after the last byte.
9. **Packet vs stream framing** — CAN/SpaceWire are frame/packet oriented while
   UART is a byte stream; the contract states which, and boundary framing is the
   backend's responsibility.

## 5. Pluggable third-party stacks (example: TCP/IP and TSN)

The framework addresses the network only through `FF_NetStack_t`
(SRS-REQ-027) and `FF_TsnBackend_t` (SRS-REQ-028). A user therefore adopts
**Tuxera TCP/IP**, **lwIP**, **RTEMS libbsd** or their own stack by writing a
thin adapter that implements the vtable; the core, PUS and applications are
unchanged. The same principle applies to CANopen, TSN and storage backends.

## 6. Partitioning and build

- Contracts ship always (headers, no cost).
- The registry implementation is toggled by the `FF_BUILD_HAL` CMake option
  (default ON). M4 will add per-driver targets so a user links only the
  backends they need.
- `src/hal/` (interfaces + registry), `src/bsp/mpfs/` (board), and (M4)
  per-driver directories form the separable low-level partition.

## 7. Verification

- SRS-REQ-025 (registry) is verified by test (`tests/unit/test_hal.c`) and by
  Lean: the model is `Frick.Hal`
  (`hal_registry_count_bound`, `hal_dup_registration_unchanged`), wrapped as
  `req_srs025_registry_bound` / `req_srs025_dup_unchanged` in
  `Frick.Requirements`.
- SRS-REQ-026..032 are interface declarations (non-executable) verified by
  inspection and the contract-presence test; their implementations are M4 and
  carry their own requirements/tests then.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial HAL specification and contract catalogue (milestone M3) |
| 0.2 | 2026-10-10 | frick-fsw | M4: correct TSN resource (GEM native TSN block) |
