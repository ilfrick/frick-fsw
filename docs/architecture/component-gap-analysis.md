# Component Gap Analysis (vs cFS and other FSW)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-ARCH-0014                        |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-10                             |
| Norms                             | N3, N4, N8                             |
| Requirements                      | SSS-REQ-022, SSS-REQ-023, SRS-REQ-045, SRS-REQ-046 |

## 1. Purpose

Position frick-fsw against the **NASA core Flight System (cFS)** and other open
flight-software frameworks, list the components that are **missing or partial**,
and record the decisions taken in the development plan (milestones M12/M13).

## 2. frick-fsw vs cFS (component by component)

cFS = cFE (core: ES/SB/EVS/TBL/TIME) + OSAL + PSP, plus apps (CF, CS, CI, DS, FM,
HK, HS, LC, MD, MM, SCH, SC, SAMPLE, TO), libraries and interfaces (ECI, SBN, SIL).

| cFS element | Function | frick-fsw status | Where / milestone |
| ----------- | -------- | ---------------- | ----------------- |
| cFE ES | executive/startup/registry | **present** | `src/core/es/` |
| cFE SB | software bus messaging | **present** | `src/core/sb/` |
| cFE EVS | event services | **present** | `src/core/evs/` |
| cFE TBL | table services | **present** | `src/core/tbl/` |
| cFE FS | file services (volumes/files) | **missing** | M4 (filestore) |
| cFE TIME | time services | **present** | `src/core/time/` (+ PPS) |
| OSAL | OS abstraction | **present (PAL)** | `src/core/pal/` |
| PSP | platform/board support | **present (BSP + HAL)** | `src/bsp/mpfs/`, `src/hal/` |
| PUS (ECSS-E-ST-70-41C) | TM/TC services | **present (subset)** | `src/pus/` |
| Mission transport (CFDP, CCSDS) | space packet / link / file | **partial** (CCSDS + link; CFDP planned) | `src/transport/`, M4 |
| **LC** Limit Checker | telemetry limit monitoring + action | **missing** | M13 (PUS S12) |
| **SC** Stored Command | absolute/relative command sequences | **partial** (scheduler only) | M13 (PUS S11) |
| **MM** Memory Manager | memory load/dump | **missing** | M13 (PUS S6) |
| **MD** Memory Dwell | telemetry of memory | **missing** | M13 (PUS S6) |
| **CS** Checksum | memory/table/file integrity | **missing** | M13 |
| **FM** File Manager | on-board file system ops | **missing** | M4 (filestore) |
| **DS** Data Storage | onboard data logging | **missing** | M4/M13 |
| **CF** CFDP | file transfer to/from ground | **missing** | M4 |
| **SBN** Software Bus Network | bus over network | **missing** | M4 (net) |
| **ECI** External Code Interface | wrap external task code | **missing** | M13 |
| **Equipment manager** | device command/monitor/reconfig, redundancy | **missing** (esp. HK only) | M13 |
| HK / HS | housekeeping / health & safety | **present (reference apps)** | `src/apps/app_hk.c`, `app_hs.c` |
| SCH / SAMPLE | scheduler / sample | **present (reference apps)** | `app_scheduler.c`, `app_sample.c` |
| Command Ingest / Telemetry Output | TM/TC front end | **present (reference apps)** | `app_cmd_ingest.c`, `app_tm_output.c` |

Additional frick-fsw elements beyond the cFS open set: **HAL** contracts
(`src/hal/`), pluggable **TCP/IP + TSN** (lwIP + GEM native TSN, M4/M8),
**libm** facade (M6), **CCSDS SDLS** with key management (M7).

## 3. Other flight-software frameworks (inspiration)

- **NASA F′ (F Prime, JPL)** — component-based, model-driven (XML → C++/Python);
  useful reference for FPP modelling and autocoded dictionaries.
- **ESA TASTE** — model-based (ASN.1 + SDL) with cFS/F′ integration; reference
  for interface modelling.
- **N7 Space MBEP** (Model-Based Execution Platform, ESA contract) — includes the
  **n7s-obcp** OBCP engine (MicroPython); direct source for M12.
- **Spacebel MicroPython OBCP** (DASIA 2018; ESA qualification work on a
  MicroPython-based VM/OBCP engine) — heritage behind n7s-obcp.
- **ESA OPS-SAT / NanoSat MO Framework (NMF)** — PUS-based on-board apps and
  autonomy; reference for mission services.
- **N7 / ESA**: N7 Space is the ESA-funded supplier of the PUS-C/SMP2 and OBCP
  tooling; "N7" in our context = N7 Space (not a standard). The applicable
  standards are **ECSS-E-ST-70-41C** (PUS-C) and the **ECSS OBCP** guidance
  (ECSS-E-ST-70 series).

## 4. Equipment manager — decision

An **equipment manager** is **not** a standard cFS app; it is mission-specific
device management. It is, however, a recurring need (device command/monitoring,
mode/redundancy selection, reconfiguration). **Decision:** provide it as an
**optional reference service** on top of the middleware (PUS S2 device command
verification, S3 housekeeping, S8 function management, S12 on-board monitoring),
not as a mandatory framework core. Tracked as **milestone M13** (SSS-REQ-023 /
SRS-REQ-046); users may build their own on the SB/PUS/HAL seams.

## 5. OBCP engine — decision (milestones M12 and M16)

> M12 integrates an existing engine (first candidate `n7space/n7s-obcp`); **M16**
> develops an in-house, ECSS-compliant engine behind the same `FF_Obcp_t`
> contract as the fallback/hardened engine (requirement SRS-REQ-054 / SSS-REQ-027).

Requirement: execute **On-Board Control Procedures written in uPython** (a subset
of Python for embedded targets — MicroPython), uploaded on board, with tight
resource and safety bounds, ECSS-compliant.

**First integration candidate (open source, lightweight):**
[`n7space/n7s-obcp`](https://github.com/n7space/n7s-obcp) — OBCP engine **based on
MicroPython** (derived from the `embed` port), **MIT licence**, developed by
**N7 Space** under an **ESA** contract ("Model-Based Execution Platform for Space
Applications"). It exposes a small C interface (`obcpengine.h`:
`obcpengine_init`, `obcpengine_execute_py`, `obcpengine_execute_mpy`,
`obcpengine_provide_buffer`) with callbacks supplied by the embedding
application, and provides the OBCP modules `obcpio`, `obcpdatapool`,
`obcpevents`, `obcpcontrol`, `obcptime`, `obcppackets`. It supports a concurrent
mode (one task per OBCP) and is designed to be embedded in a larger C flight
application. Upstream targets are x86-64 and ARM Cortex-M7; the RISC-V/RTEMS port
is to be validated at integration (M12). Callbacks map to frick-fsw SB/EVS/Time/PUS.

**Fallback:** if n7s-obcp is not integrable (licence/qualification/portability),
**develop an ECSS-compliant engine** (MicroPython-based, same C interface and
module surface) with the ECSS OBCP guidance (ECSS-E-ST-70 series) and MISRA C.

Work (M12): vendored pinned under `third_party/` (not committed, like lwIP/PUS-C);
adapter behind a `FF_Obcp_t` contract; callbacks to SB/EVS/Time/PUS; the OBCP
modules mapped to frick-fsw services (datapool → parameters, events → EVS/PUS S5,
time → Time Services, packets → PUS); resource bounds and safety (stack/heap,
no dynamic growth, watchdog, execution time budget, OBCP state reporting via
PUS S18); ECSS/Renode validation (extend the N18 functional test).

## 6. Gap linkage

Missing mission services (LC/SC/MM/CS/FM/DS/CF/SBN/ECI, equipment manager) and the
OBCP engine are **scope/mission features**, not defects of the framework core;
they are planned (M4/M12/M13) and recorded as requirements SSS-REQ-022/023 and
SRS-REQ-045/046. Real-silicon residual remains GAP-A5.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial cFS/other-FSW component gap analysis; OBCP engine (n7s-obcp) and equipment-manager decisions |
