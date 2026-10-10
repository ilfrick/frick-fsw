# Software Design Document (SDD)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-SDD-001                          |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-10                             |
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
| Board support | `src/bsp/mpfs/` | RTPFS160ZT bring-up behind the HAL | `FF_BoardInit` |

Data flow (nominal): link -> `FF_PusIngest` -> handler -> service action ->
`FF_SbPublish`/`FF_EvsSend` -> `FF_PusSendTm` -> link. Time: PPS backend ->
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
- **Link (`link.c` + backends)**: `FF_Link_t` vtable; loopback and UART.
- **PUS (`pus.c`/`pus_services.c`)**: dispatch table, S1 verification generated
  for every acknowledged TC, services 1/3/5/6/8/9/11(stub)/17/20; provider
  registration for HK/parameter/function/memory.
- **HAL (`hal.c` + `hal_*.h`)**: bounded registry (`FF_HAL_MAX_BACKENDS` per
  class) and per-resource contracts; see
  `docs/architecture/hardware-abstraction.md`. Design obligations for
  integrators: caller-owned names, buffer ownership, DMA/cache rules, ISR/task
  split, no dynamic allocation.

## 4. Design for determinism and fault handling

- No heap use after init (static pools; assertion/analysis evidence).
- Bounded queues; bounded work per cycle; priorities documented.
- Failures: `FF_Status_t` returned to callers; EVS events for operational
  visibility; HS heartbeat + commandable reset (service 8 path).

## 5. Interfaces

- External: `docs/interfaces/ICD.md` (CCSDS/PUS, CUC, link).
- Internal: public headers under `src/`; the HAL contracts under `src/hal/`.

## 6. Traceability

Design elements trace to SRS/SSS through `docs/requirements/traceability.yaml`
(implementation paths). Requirements whose design is deferred (on-target PPS,
TAI/CCSDS epoch) are the documented gaps GAP-A5/A7.

## 7. Hardware independence and deferred design

The design above is complete and testable on the host simulator. The following
are intentionally deferred to board integration and documented as gaps:
real MSS peripheral register maps and DMA/cache coherency (GAP-A4/A5), the
on-target PPS capture path (GAP-A7), and fabric-attached devices (CAN-FD,
SpaceWire, TSN) as generic fabric-device backends.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial architectural and detailed design (ECSS-E-ST-40C software design output) |
