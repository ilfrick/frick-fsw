# Software Design Document (SDD)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-SDD-001                          |
| Revision                          | 0.10                                   |
| Date                              | 2026-10-18                             |
| Audience                          | software engineers, reviewers, integrators |
| References                        | ECSS-E-ST-40C (software design), `docs/requirements/SRS.md`, `docs/architecture/*`, `docs/interfaces/ICD.md` |

This document is the architectural and detailed design of frick-fsw. It is
**hardware-independent by construction**: the framework depends only on the PAL
(OS primitives) and the HAL (device contracts, `src/hal/`), so the design can be
completed before the flight board is available. Hardware-specific behaviour is
confined to the BSP/driver layer and to the documented class-A gaps.

## 1. Design principles

1. **Layered, downward-only dependencies** (see `docs/architecture/overview.md`):
   applications depend on services; services depend on the transport and the
   PAL; the HAL is the only door to devices; nothing depends on a specific
   hardware.
2. **Deterministic, static allocation**: no dynamic memory after initialisation
   (SRS-REQ-015 / SSS-REQ-010); fixed-size queues and bounded registries.
3. **Backend abstraction**: PUS product, link transport, PPS source, HAL
   backends and (in M3+) network/TSN stacks are vtables, selected at
   configuration/registration time.
4. **Single configuration point**: `config/fsw_config.h` + mission tables.
5. **Fail-safe error propagation**: `FF_Status_t` on every fallible call;
   faults reported to EVS; HS monitors heartbeats.

## 2. Architectural design

| Layer | Module | Responsibility | Key interfaces |
| ----- | ------ | -------------- | -------------- |
| Applications | `src/apps/` | reference apps; the user's mission apps live here | `FF_AppConfig_t`, ES/SB |
| PUS-C | `src/pus/` | TC dispatch, S1 verification, TM emission, services | `FF_PusIngest`, `FF_Pus_Backend_t`, providers |
| Core services | `src/core/{es,evs,sb,tbl,time}` | lifecycle, events, bus, tables, time | public headers of each |
| Transport | `src/transport/{ccsds,link}` | Space Packet codec + CRC; byte link | `FF_Ccsds*`, `FF_Link_t` |
| OS abstraction | `src/core/pal/` | tasks, mutexes, queues, clock, log, panic | `FF_Pal*` |
| Hardware abstraction | `src/hal/` | bus/device/memory contracts + registry | `FF_Hal*`, `FF_EthMac_t`, `FF_NetStack_t`, ... |
| Mathematics | `src/math/` | portable `libm` facade for applications (M6) | `FF_Math*` |
| Board support | `src/bsp/mpfs/` | RTPFS160ZT bring-up behind the HAL | `FF_BoardInit` |

Data flow (nominal): link -> `FF_PusIngest` -> handler -> service action ->
`FF_SbPublish`/`FF_EvsSend` (SB overflow uses `FF_EvsSendDeferred`) ->
`FF_PusSendTm` -> link. Time: PPS backend ->
`FF_TimeToneSignal` -> CUC/UTC -> PUS S9/TM header.

## 3. Detailed design (per module)

- **PAL (`pal.h`/`pal_posix.c`/`pal_rtems.c`)**: POSIX implementation is the
  reference; RTEMS uses the POSIX surface of RTEMS 6. Blocking primitives are
  bounded; the clock is monotonic. Stack/priority are effective on RTEMS.
- **SB (`sb.c`)**: bounded ring-buffer pipes per application; publish fans out
  to subscribers (exact or `FF_SB_WILDCARD`); no allocation.
- **EVS (`evs.c`)**: event types, formatted messages, bounded log, listener to
  forward events on the bus.
- **ES (`es.c`)**: application registry (`FF_AppConfig_t`), start/stop, state.
- **TBL (`tbl.c`)**: load/validate/activate/revert with atomic activation.
- **Time (`time.c`/`pps.c`)**: source selection; tone discipline (strictly
  increasing epoch, one second per tone); leap-at-boundary; CUC 7-octet P+T
  self-describing form and 6-octet PUS on-wire T-field. Tone state is
  mutex-protected and the hook is **task-context only** (ISRs defer to a task).
- **CCSDS (`ccsds.c`)**: primary/secondary headers, length bounds, CRC-16.
- **Link (`link.c` + backends)**: `FF_Link_t` vtable; loopback (`link_loopback.c`),
  UART (`link_uart.c`), PUS-over-IP (`link_ip.c`), TSN (`link_tsn.c`) and
  segmentation/reassembly (`link_seg.c`).
- **PUS (`pus.c`/`pus_services.c`)**: dispatch table, S1 verification generated
  for every acknowledged TC, services 1/3/5/6/8/9/11(stub)/17/20; provider
  registration for HK/parameter/function/memory.
- **HAL (`hal.c` + `hal_*.h`)**: bounded registry (`FF_HAL_MAX_BACKENDS` per
  class) and per-resource contracts; see
  `docs/architecture/hardware-abstraction.md`. Design obligations for
  integrators: caller-owned names, buffer ownership, DMA/cache rules, ISR/task
  split, no dynamic allocation.
- **Math (`src/math/ff_math.c`)**: portable facade over the platform `libm`
  (elementary functions plus angle/clamp helpers and a finiteness check) for
  applications; the framework core uses integer arithmetic only
  (SSS-REQ-018/SRS-REQ-033; `docs/architecture/numerics.md`).

## 4. Design for determinism and fault handling

- No heap use after init (static pools; assertion/analysis evidence).
- Bounded queues; bounded work per cycle; priorities documented.
- Failures: `FF_Status_t` returned to callers; EVS events for operational
  visibility; HS heartbeat + commandable reset (service 8 path).

## 5. Interfaces

- External: `docs/interfaces/ICD.md` (CCSDS/PUS, CUC, link).
- Internal: public headers under `src/`; the HAL contracts under `src/hal/`.

## 6. Design traceability matrix (SRS -> design component)

ECSS SDD DRD (ECSS-E-ST-40C Annex F <6>): every software requirement is allocated
to a design element. The matrix below is the requirements->design trace; the
requirement->code->test trace is the manifest `docs/requirements/traceability.yaml`
(gate `tools/scripts/trace.py`).

| Design component | Design element | SRS requirements |
| ---------------- | -------------- | ---------------- |
| PAL / OS abstraction | `src/core/pal/` (`pal.h`, `pal_posix.c`, `pal_rtems.c`) | SRS-REQ-015, SRS-REQ-016 |
| Software Bus | `src/core/sb/sb.c` | SRS-REQ-001, SRS-REQ-002 |
| CCSDS codec | `src/transport/ccsds/ccsds.c` | SRS-REQ-003, SRS-REQ-004, SRS-REQ-017 |
| PUS-C engine | `src/pus/pus.c`, `pus_services.c`, `adapters/` | SRS-REQ-005, SRS-REQ-006, SRS-REQ-007, SRS-REQ-008, SRS-REQ-009, SRS-REQ-010, SRS-REQ-012, SRS-REQ-018, SRS-REQ-050 |
| Event Services | `src/core/evs/evs.c` | SRS-REQ-011 |
| Executive Services | `src/core/es/es.c`, `src/apps/app_hs.c` | SRS-REQ-014 |
| Table Services | `src/core/tbl/tbl.c` | SRS-REQ-013, SRS-REQ-019 |
| Time Services | `src/core/time/time.c`, `pps.c` | SRS-REQ-021, SRS-REQ-022, SRS-REQ-023, SRS-REQ-024 |
| Link layer | `src/transport/link/link.c` (+ backends) | SRS-REQ-039, SRS-REQ-040 |
| HAL registry + contracts | `src/hal/hal.c`, `hal_*.h` | SRS-REQ-025, SRS-REQ-026, SRS-REQ-027, SRS-REQ-028, SRS-REQ-029, SRS-REQ-030, SRS-REQ-031, SRS-REQ-032 |
| Mathematics facade | `src/math/ff_math.c` | SRS-REQ-033 |
| Security (SDLS) | `src/security/ff_sdls.c` | SRS-REQ-034, SRS-REQ-035 |
| Reference drivers | `src/drivers/{eth/gem,gpio/mpfs,serial/mpfs,mem,nvm/mpfs,mtd/mpfs,can/mpfs,i2c/mpfs,spi/mpfs,block/mpfs,net}` | SRS-REQ-036, SRS-REQ-038, SRS-REQ-040, SRS-REQ-047, SRS-REQ-048, SRS-REQ-049, SRS-REQ-055, SRS-REQ-056, SRS-REQ-057, SRS-REQ-058, SRS-REQ-069, SRS-REQ-070 |
| Board / RTEMS integration | `src/bsp/mpfs/`, `config/rtems/rtems_config.c` | SRS-REQ-037 |
| Quality & process | `.clang-format`, `tools/scripts/misra.sh`, `docs/quality/` | SRS-REQ-041, SRS-REQ-042 |
| Emulated-target V&V | `tools/scripts/emulated_run.sh`, `tools/docker/renode/`, `src/apps/app_renode_test.c` | SRS-REQ-043, SRS-REQ-044 |
| Build / CI / configuration | `CMakeLists.txt`, `.github/workflows`, `.gitea/workflows`, `config/` | FRICK-SRS-REQ-000, SRS-REQ-020 |
| OBCP adapter (M12 candidate) | `src/obcp/ff_obcp.{c,h}` behind `FF_Obcp_t` (selected N7 callback integration; default-built with `FF_USE_N7_OBCP=ON`; deferred, NC-S9-01) | SRS-REQ-045 |
| Mission services (M13) - planned | equipment manager + fault management (TBD) | SRS-REQ-046 |
| Multicore / SMP (M14) - planned | RTEMS SMP path (TBD) | SRS-REQ-051 |
| PUS-C service set (M15) - planned | in-house `FF_Pus_Backend_t` (TBD) | SRS-REQ-052 |
| CFDP + VFS (M15) - planned | file transfer + virtual filestore (TBD) | SRS-REQ-053 |
| In-house OBCP engine (M16) - planned | in-house engine behind `FF_Obcp_t` (TBD) | SRS-REQ-054 |

Components marked *planned* have no code in this baseline; the design intent is
recorded here and detailed at their milestone (ECSS allows a phased design; the
requirement->code trace is deferred until implementation, tracked in norm N19).

## 7. Traceability

Design elements trace to SRS/SSS through the matrix in §6 and through
`docs/requirements/traceability.yaml` (implementation paths). Requirements whose
design is deferred (on-target PPS, TAI/CCSDS epoch) are the documented gaps
GAP-A5/A7.

## 7.1 Detailed design-to-code traceability (ECSS Annex M <4.4>a.1)

The following matrix maps each design element (per §3 and §5) to its implementing
source files. This satisfies Annex M <4.4>a.1 detailed design ↔ code traceability.

| Design element | Source file(s) | Notes |
| -------------- | -------------- | ----- |
| PAL — Portable Abstraction Layer | `src/core/pal/pal.h` | Public interface: tasks, mutexes, queues, time, console, panic |
|  | `src/core/pal/pal_posix.c` | POSIX implementation (host + RTEMS) |
|  | `src/core/pal/pal_rtems.c` | RTEMS bootstrap (POSIX keys, clock, panic hook) |
|  | `src/core/pal/pal_watchdog.c` | Watchdog abstraction (independent HW backend) |
| Software Bus | `src/core/sb/sb.h` | Public interface: publish/subscribe, pipes, wildcard |
|  | `src/core/sb/sb.c` | Bounded ring-buffer pipes, publish fan-out, no allocation |
| Event Services | `src/core/evs/evs.h` | Public interface: event types, formatted messages, log |
|  | `src/core/evs/evs.c` | Event log, listeners, PUS S5 forwarding |
| Executive Services | `src/core/es/es.h` | Public interface: app registry, lifecycle, reset counters |
|  | `src/core/es/es.c` | App registry, start/stop, state, heartbeat |
| Table Services | `src/core/tbl/tbl.h` | Public interface: load/validate/activate/revert/dump |
|  | `src/core/tbl/tbl.c` | Load/validate/atomic activate/revert, mission tables |
| Time Services | `src/core/time/time.h` | Public interface: MET/UTC, tone discipline, CUC |
|  | `src/core/time/time.c` | MET/UTC, source selection, tone discipline, PPS |
|  | `src/core/time/pps.c` | 1 Hz tone backend (software) |
| CCSDS Codec | `src/transport/ccsds/ccsds.h` | Public interface: Space Packet encode/decode, CRC |
|  | `src/transport/ccsds/ccsds.c` | Primary/secondary headers, length bounds, CRC-16 |
| PUS-C Engine | `src/pus/pus.h` | Public interface: TC ingest, TM emission, providers |
|  | `src/pus/pus.c` | TC ingest, S1 verification, TM emission, dispatch |
|  | `src/pus/pus_services.c` | Services 1,3,5,6,8,9,11,17,20; provider hooks |
|  | `src/pus/adapters/pus_adapter_open.c` | Open CCSDS backend (FF_PusBackendOpen) |
|  | `src/pus/adapters/pus_adapter_n7.c` | N7/ESA ASN.1 backend (FF_PusBackendN7) |
|  | `src/pus/adapters/pus_adapter_pusopen.c` | PUSopen commercial adapter (FF_PusBackendPusopen) |
| Link Layer | `src/transport/link/link.h` | Public interface: FF_Link_t vtable, run-time selection |
|  | `src/transport/link/link.c` | Loopback, UART, IP, TSN backends, runtime selection |
|  | `src/transport/link/link_loopback.c` | In-memory loopback (simulator/tests) |
|  | `src/transport/link/link_uart.c` | UART backend (host `/dev/ttyUSB0` or RTEMS console) |
|  | `src/transport/link/link_ip.c` | IP backend (UDP default + TCP over FF_NetStack_t) |
|  | `src/transport/link/link_tsn.c` | TSN backend (EtherType 0x88F5, GEM MAC) |
|  | `src/transport/link/link_seg.c` | Segmentation/reassembly (SRS-REQ-059) |
| HAL Registry + Contracts | `src/hal/hal.c` | Bounded registry (FF_HAL_MAX_BACKENDS per class) |
|  | `src/hal/hal.h` | Public registry interface |
|  | `src/hal/hal_eth.h` | FF_EthMac_t, FF_NetIf_t (SRS-REQ-026) |
|  | `src/hal/hal_net.h` | FF_NetStack_t (SRS-REQ-027) |
|  | `src/hal/hal_tsn.h` | FF_TsnBackend_t (SRS-REQ-028) |
|  | `src/hal/hal_can.h` | FF_CanBackend_t (SRS-REQ-029) |
|  | `src/hal/hal_serial.h` | FF_Serial_t (SRS-REQ-030) |
|  | `src/hal/hal_spi.h` | FF_Spi_t (SRS-REQ-031) |
|  | `src/hal/hal_mem.h` | FF_MemPool_t, FF_Dma_t, FF_Nvm_t, FF_Mtd_t, FF_BlockDev_t (SRS-REQ-032) |
|  | `src/hal/hal_gpio.h` | FF_Gpio_t (SRS-REQ-047) |
|  | `src/hal/hal_spacewire.h` | FF_SpaceWire_t, FF_Rmap_t (SRS-REQ-032) |
|  | `src/hal/hal_pcie.h` | FF_Pcie_t (SRS-REQ-031/032) |
| Reference Drivers | `src/drivers/eth/gem/ff_gem.c` | GEM Ethernet MAC (FF_EthMac_t) |
|  | `src/drivers/eth/gem/ff_gem_tsn.c` | GEM native TSN (FF_TsnBackend_t) |
|  | `src/drivers/eth/gptp/ff_gptp.c` | 802.1AS gPTP (SRS-REQ-068) |
|  | `src/drivers/net/ff_net_loopback.c` | Loopback network stack (FF_NetStack_t) |
|  | `src/hal/net/lwip/ff_net_lwip.c` | lwIP adapter (FF_NetStack_t) |
|  | `src/drivers/gpio/mpfs/ff_gpio_mpfs.c` | MSS GPIO (FF_Gpio_t) |
|  | `src/drivers/serial/mpfs/ff_serial_mpfs.c` | MSS MMUART (FF_Serial_t) |
|  | `src/drivers/mem/ff_mem_static.c` | Static RAM pool + DMA (FF_MemPool_t, FF_Dma_t) |
|  | `src/drivers/nvm/mpfs/ff_nvm_mpfs.c` | MSS eNVM (FF_Nvm_t) |
|  | `src/drivers/mtd/mpfs/ff_mtd_mpfs.c` | MSS QSPI NOR (FF_Mtd_t) |
|  | `src/drivers/can/mpfs/ff_can_mpfs.c` | MSS CAN (FF_CanBackend_t) |
|  | `src/drivers/can/fabric/ff_canfd_fabric.c` | CAN-FD fabric (FF_CanBackend_t) |
|  | `src/drivers/can/canopen/ff_canopen.c` | CANopen subset (FF_CanBackend_t) |
|  | `src/drivers/i2c/mpfs/ff_i2c_mpfs.c` | MSS I2C (FF_I2c_t) |
|  | `src/drivers/spi/mpfs/ff_spi_mpfs.c` | MSS SPI (FF_Spi_t) |
|  | `src/drivers/block/mpfs/ff_block_mpfs.c` | MSS MMC/SD (FF_BlockDev_t) |
|  | `src/drivers/pcie/mpfs/ff_pcie_mpfs.c` | MSS PCIe (FF_Pcie_t) |
|  | `src/drivers/spacewire/mpfs/ff_spacewire_mpfs.c` | SpaceWire contract-level (FF_SpaceWire_t, FF_Rmap_t) |
|  | `src/drivers/fabric/mpfs/ff_fabric_mpfs.c` | Generic fabric device contract |
| BSP / RTEMS Integration | `src/bsp/mpfs/board.c` | RTPFS160ZT bring-up (FF_BoardInit) |
|  | `config/rtems/rtems_config.c` | RTEMS application configuration table |
| Mathematics Facade | `src/math/ff_math.h` | Public interface: portable libm facade |
|  | `src/math/ff_math.c` | Integer arithmetic + elementary functions for apps |
| Security (SDLS) | `src/security/ff_sdls.c` | CCSDS SDLS + key management/rotation |
|  | `src/security/ff_crypto_test.c` | Test backend (deterministic) |
| OBCP Adapter (M12) | `src/obcp/ff_obcp.c` | N7 Space / MicroPython callback integration |
|  | `n7s-obcp/flat/*.c` | Pinned N7 Space / MicroPython VM (vendor source) |
| Build / CI / Configuration | `CMakeLists.txt` | Build system (host + RTEMS cross) |
|  | `config/fsw_config.h` | Single configuration point |
|  | `config/tables/fsw_tables.c` | Mission tables |
|  | `config/platform/*.h` | Platform headers (native, MPFS, QEMU) |
|  | `.github/workflows/ci.yml` | GitHub Actions CI |
|  | `.gitea/workflows/ci.yml` | Gitea Actions CI |
|  | `tools/docker/Dockerfile` | V&V Docker image (frick-vv:local) |
|  | `tools/scripts/*.sh` | V&V scripts (T1..T21) |

## 8. Hardware independence and deferred design

The design above is complete and testable on the host simulator. The following
are intentionally deferred to board integration and documented as gaps:
real MSS peripheral register maps and DMA/cache coherency (GAP-A4/A5), the
on-target PPS capture path (GAP-A7), and fabric-attached devices (CAN-FD,
SpaceWire, TSN) as generic fabric-device backends.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial architectural and detailed design (ECSS-E-ST-40C software design output) |
| 0.2 | 2026-10-11 | frick-fsw | Add the SRS->design-component traceability matrix (ECSS Annex F <6>) addressing NC PA-67-01 (closure pending review) |
| 0.3 | 2026-10-11 | frick-fsw | Allocate SRS-REQ-055 (reference eNVM backend) in the design matrix |
| 0.4 | 2026-10-11 | frick-fsw | Allocate SRS-REQ-056 (reference NOR/QSPI MTD backend) in the design matrix |
| 0.5 | 2026-10-11 | frick-fsw | Allocate SRS-REQ-057 (reference MSS CAN driver) in the design matrix |
| 0.6 | 2026-10-12 | frick-fsw | Allocate SRS-REQ-058 (reference MSS I2C driver) in the design matrix |
| 0.7 | 2026-10-12 | frick-fsw | Allocate SRS-REQ-069 (reference MSS SPI driver) in the design matrix (M4/M11) |
| 0.8 | 2026-10-12 | frick-fsw | Allocate SRS-REQ-070 (reference MSS block-device driver) in the design matrix (M4/M11) |
| 0.9 | 2026-10-14 | frick-fsw | Clarify that SB overflow uses explicit deferred EVS send while ordinary EVS sends remain synchronous |
| 0.10 | 2026-10-18 | frick-fsw | Record the delivered `src/obcp/` N7 OBCP adapter candidate as implementation present (default-built `FF_USE_N7_OBCP=ON`; deferred, NC-S9-01) instead of *planned*; list the IP/TSN/segmentation link backends in §3 |
