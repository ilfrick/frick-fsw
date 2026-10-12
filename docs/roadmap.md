# Development roadmap

| Document id | FRICK-RM-0001 |
| ----------- | ------------- |
| Revision    | 0.19 |
| Date        | 2026-10-12 |

This roadmap records the planned development milestones of frick-fsw beyond the
v0.1.0 baseline. Every milestone is delivered through the same non-derogable
cycle (norm N15): requirements (norm N3) → implementation → Lean model (norm N1)
→ tests (norm N11) → traceability (norm N4) → documentation (norm N8) →
independent review where criticality requires it (norm N9) → full cycle green →
commit and push to both remotes (norm N7).

Status legend: **done**, **in progress**, **planned**.

## M1 — Baseline (done)

cFS-inspired core: PAL, Software Bus, EVS, ES, TBL, Time (MET/UTC + CUC), CCSDS
codec, link layer (loopback/UART), PUS-C services 1/3/5/6/8/9/11/17/20, six
applications, Lean requirement models, CI/CD on GitHub and Gitea. Released as
v0.1.0.

## M2 — Time base: PPS capture and CUC timecode (done)

Hardware-disciplined time base delivered, from heritage (NASA cFS Time
Services: 1 Hz tone, time source selection, `AddLeapSeconds`) and ECSS/CCSDS
practice.

Delivered scope (commit M2, 2026-10-10):

- PPS backend vtable (`src/core/time/pps.h`) with the native software backend
  task (`FF_PpsSwBackend`, priority 40, 1000 ms period) and an RTEMS/MPFS
  GPIO/PLIC capture hook to be wired by the board integrator (GAP-A7); see
  `docs/engineering/driver-development.md`.
- Time source selection `FF_TimeSetSource`/`FF_TimeGetSource` (internal / tone /
  ground), latching the monotonic counter at each tone
  (`FF_TimeToneSignal`) and steering UTC without drift between tones.
- CUC encode/decode completion including the CCSDS P-field (`0x2E`), plus
  leap-second application at the tone boundary (`FF_TimeScheduleLeap`,
  pending-leap handling, single second per change).
- Requirements SSS-REQ-016, SRS-REQ-021..024; Lean theorems in `Frick.Time`
  (source-set disjointness, tone discipline, leap-at-boundary, CUC round-trip,
  fine-field bound, coarse byte decomposition); unit tests `test_time.c`
  (7 cases, ctest `time`).
- Documentation: `docs/architecture/time-and-timecode.md` rev 0.2, ICD rev 0.2,
  `docs/quality/class-a-gaps.md` GAP-A7.

## M3 — Middleware integration & hardware-abstraction interfaces (done)

Deliver the **interfaces and documentation** that let users integrate their own
applications, drivers and third-party stacks:

- HAL contract headers under `src/hal/` for every bus and memory resource
  (Ethernet + pluggable TCP/IP and TSN, CAN/CANopen/CAN-FD, RS-422/485, SPI,
  I2C, PCIe, GPIO, SpaceWire ECSS 50-12/51/52/53, RAM/DDR, NOR/NVM, eMMC/SD,
  SSD block devices), header-only, no drivers.
- Repository/publication partitioning so low-level parts are opt-in (selectable
  CMake targets and directories), and the pluggable-backend philosophy for
  every resource.
- **User & Integration Manual** and the completed **PUS service extension
  guide**.
- Complete **V&V approach** documentation and corrected **architecture
  diagrams** focused on the software layers running on the PolarFire SoC,
  marking which layers are implemented in this repository and which are not.
- See `docs/development-plan.md` (FRICK-PLAN-0001) for the full objective set.

## M4 — MPFS low-level layers and 1 Gb/s Ethernet (in progress)

Reference implementations behind the M3 contracts:

- Native MSS peripheral layers in `src/bsp/mpfs/` (MMUART/RS-422/RS-485, CAN
  2.0, SPI, I2C, QSPI NOR MTD, MMC/eMMC/SD block device, GPIO/IRQ, DDR/DMA,
  RTC/timer).
- A complete **1 Gb/s Ethernet driver** on the GEM, integrated with the
  middleware link/network layer, with pluggable `FF_NetStack_t` adapters
  (Tuxera, lwIP, RTEMS libbsd) and the `FF_TsnBackend_t` seam.
- Generic fabric-device (FIC/AXI) backend path for fabric-attached resources
  (CAN-FD, SpaceWire/RMAP, TSN), no FPGA IP (SRS-REQ-067).
- **PUS-C over IP and TSN** (restored original M4 scope; OBJ-25, SSS-REQ-028): bounded
  segmentation/reassembly (SRS-REQ-059) and reference `FF_Link_t` backends for
  TCP/IP (SRS-REQ-060) and TSN (SRS-REQ-061), selectable with `FF_LinkSelect`.
- **Extended fieldbus** (OBJ-26/OBJ-27/OBJ-28, SSS-REQ-029): the generic fabric
  contract (SRS-REQ-067), CAN-FD (SRS-REQ-062), a CANopen (CiA 301/302/402) stack
  (SRS-REQ-063) and RS-485 half-duplex direction control (SRS-REQ-064); plus a
  documented **PUSopen(R) adapter** (OBJ-30, SRS-REQ-065).
- The on-board **file transfer set** (virtual filestore, PUS service 23, CFDP)
  is planned in **M15** (OBJ-23).

## M5 — Documentation and status program (in progress)

- README index, documentation index and repository map (norm N8).
- PUS services catalogue and low-level driver inventory in README.
- `docs/engineering/driver-development.md` — human + agent guide.
- `docs/engineering/service-development.md` — human + agent guide.
- `docs/engineering/user-integration-manual.md` — human + agent guide.
- `docs/development-plan.md` — milestone/objective plan.
- README metrics panel: coverage plus review/V&V traffic lights
  (red = failures, yellow = issues deferred by the user, green = all pass).

## M6 — Mathematics library integration (libm) (done)

A documented, portable mathematics facade over the platform `libm`
(`src/math/ff_math.{c,h}`, `FF_USE_LIBM`), for applications that need
floating-point math, with the determinism policy (no use in hard real-time
critical paths without a work-package analysis) and the licensing analysis
documented. Requirements SSS-REQ-018 / SRS-REQ-033; ADR-0012;
`docs/architecture/numerics.md`.

## M7 — CCSDS SDLS (Space Data Link Security) (done)

Middleware-side link-layer security: SDLS processing, security-association
database and key management/rotation, with a pluggable cryptographic backend
(`src/security/ff_sdls.{c,h}`). Requirements SSS-REQ-019 / SRS-REQ-034/-035;
ADR-0013; `docs/architecture/security-sdls.md`; GAP-A14 (algorithm/HSM/ground).

## M8 — Deterministic Ethernet completion: gPTP (802.1AS) and fabric CoreTSN (in progress)

Complete the deterministic-Ethernet feature set on the PolarFire SoC:

- **Full 802.1AS (gPTP) stack**: BMCA, Sync/Follow_Up, PDelay, and a servo
  disciplining the GEM TSU clock from the grandmaster; CMLDS/message-stamp
  handling. The middleware exposes the timestamp/time access; the PTP state
  machine is added in software (`FF_TsnBackend_t` + a PTP module).
- **Fabric CoreTSN path**: a generic fabric-device backend (via the FIC) for a
  multi-port TSN switch / Frame Replication and Elimination (802.1CB) when the
  mission routes TSN through fabric IP instead of the GEM endpoint.
- **GEM native TSN shaping/filtering completion**: 802.1Qav (credit-based
  shaping), 802.1Qci (per-stream filtering/policing) and 802.1Qbb (priority flow
  control) on the GEM TSN block (SRS-REQ-066, OBJ-29); the full gPTP stack and
  fabric CoreTSN are captured by SRS-REQ-068.
- Requirements, Lean models (PTP state machine where feasible), unit tests,
  documentation, and independent review. On-target timing/PTP accuracy remains
  GAP-A5.

## M9 — Code quality: MISRA C:2012 and coding style (in progress)

Declare and enforce a single C coding style (`.clang-format` normative) applied
codebase-wide, and drive MISRA C:2012 compliance with an open-source scan
(cppcheck addon, advisory → blocking) and a documented deviation process.
`docs/engineering/coding-style.md`, `docs/quality/misra-c.md`; certified MISRA
checker + formal report are GAP-A4.

## M10 — Emulated-target verification: Renode (PolarFire SoC) and QEMU (in progress)

Run the actual RTEMS target binary on an emulated PolarFire SoC to verify boot,
PAL/RTEMS and drivers without hardware. **Renode** has official `polarfire-soc`
/ `icicle-kit` platforms (5 CPUs, UART, GEM, CAN, SPI, I2C, GPIO, PCIe, SD/eMMC,
QSPI flash, eNVM, timers); **QEMU** runs the RTEMS `riscv/rv64imafdc` BSP. This
**partially mitigates GAP-A5**; tool qualification (N10) and CI wiring included.
See `docs/vv/emulated-target.md`; requirement SRS-REQ-043; objective OBJ-18.

## M11 — Renode HAL coverage: exercise every HAL resource class on the emulated SoC (in progress)

Extend the Renode functional test (norm N18) so that **every HAL resource class**
is bound to its emulated peripheral and exercised on the `polarfire-soc` platform:
ETH, NET/TCP-IP, TSN, CAN, SERIAL (UART/RS-422/485), SPI, I2C, PCIE, GPIO/IRQ,
SPACEWIRE (+RMAP), MEM (RAM/DMA), BLOCK (eMMC/SD/SSD), NVM. Each class gets its
backend behind the `hal_*.h` contract (the M4-residual MSS peripherals where still
missing), a binding in `frick-renode.resc`, and a checkable clause in the marker.
Objective OBJ-19; builds on M10 and the M4-residual drivers.

## M12 — OBCP support: integrate an OBCP engine (planned)

Integrate an engine that executes On-Board Control Procedures written in
**uPython/MicroPython** (uploaded on board, resource/safety bounded). First
candidate: **`n7space/n7s-obcp`** (MicroPython-based OBCP engine, MIT, ESA-funded)
behind an `FF_Obcp_t` contract with SB/EVS/Time/PUS bindings. See
`docs/architecture/component-gap-analysis.md §5`.
Requirement SRS-REQ-045 / SSS-REQ-022; objective OBJ-20. The **in-house**
ECSS-compliant engine (fallback/hardening, to remove a permanent third-party
dependency) is delivered by **M16**.

## M13 — Reference mission services: equipment manager + fault management (planned)

Optional reference services on the PUS: an **equipment manager** (device
command/monitoring/reconfiguration, redundancy) and **fault management** (limit
checking, stored command sequences, memory/file management). Users may build
their own on the SB/PUS/HAL seams. See
`docs/architecture/component-gap-analysis.md §4`. Requirement SRS-REQ-046 /
SSS-REQ-023; objective OBJ-21.

## M14 — Multicore / SMP (planned, after all other milestones)

Enable and validate RTEMS **SMP** on the PolarFire SoC 4×U54 cluster: SMP tasking
and priorities, per-core interrupts/timers, cache coherency for shared data and
DMA, and a shared-memory Software Bus/queues path. Depends on every earlier
milestone; do not start before they are complete. Requirement SRS-REQ-051 /
SSS-REQ-024; objective OBJ-22.

## M15 — In-house fundamental PUS-C service set (planned, after M14)

Implement, in MISRA-clean in-house C behind `FF_Pus_Backend_t`, the fundamental
PUS-C service behaviour (S1/3/5/6/8/9/11/12/17/18-PUS-surface/19/20) on the
`asn1-pusc-lib` default data model, with a bounded on-board parameter datapool;
cover every in-scope subservice with verification and requirement-level Lean
theorems. Also: the CFDP entity (CCSDS 727.0-B) and the **virtual filestore (VFS)**
it operates on (pluggable storage: RAM/flash/eMMC/SSD), on which PUS-C S23 file
management is built. Assessment FRICK-ARCH-0015 (est. ~34 PM; hybrid
recommendation). Requirement SRS-REQ-052/053 / SSS-REQ-025/026; objective OBJ-23.

## M16 — In-house OBCP engine development (planned, after M12)

Develop, within the project, an **in-house, ECSS-compliant OBCP engine** behind
the `FF_Obcp_t` contract: bounded resource accounting (stack/heap), isolation,
execution-time budget, abort and reporting; OBCP state/management via PUS **S18**.
It is the fallback / hardened engine to the integrated third-party engine (M12),
removes a permanent third-party dependency, and reuses the M12 `FF_Obcp_t`
contract and SB/EVS/Time/PUS bindings. Requirement SRS-REQ-054 / SSS-REQ-027;
objective OBJ-24. **Execution order: after M12, before M13**; M14/M15 remain the
last milestones.

## History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial roadmap: M2 (PPS/CUC), M3 (filestore/CFDP/service 23), M4 (IP+TSN over PUS-C), M5 (docs) |
| 0.2 | 2026-10-10 | frick-fsw | Milestone M2 delivered (time source, tone discipline, leap-at-boundary, CUC framing, PPS backend vtable) |
| 0.3 | 2026-10-10 | frick-fsw | Re-plan: M3 = HAL interfaces + integration manuals; M4 = MPFS BSP + 1 Gb/s Ethernet (absorbs filestore/CFDP/IP/TSN); pointer to `docs/development-plan.md` |
| 0.4 | 2026-10-10 | frick-fsw | M3 status set to in progress (HAL interfaces delivered) |
| 0.5 | 2026-10-10 | frick-fsw | Add M6 (mathematics library integration, libm); M3 delivered interfaces |
| 0.6 | 2026-10-10 | frick-fsw | Add M7 (CCSDS SDLS security and key management) |
| 0.7 | 2026-10-10 | frick-fsw | M4 status set to in progress (Ethernet GEM driver) |
| 0.8 | 2026-10-10 | frick-fsw | Add M8 (deterministic Ethernet completion: gPTP 802.1AS + fabric CoreTSN) |
| 0.9 | 2026-10-10 | frick-fsw | Add M9 (code quality: MISRA C:2012 + coding style) |
| 0.10 | 2026-10-10 | frick-fsw | Add M10 (emulated-target verification: Renode PolarFire SoC + QEMU) |
| 0.11 | 2026-10-10 | frick-fsw | Add M11 (Renode HAL coverage: extend the functional test to every HAL resource class) |
| 0.12 | 2026-10-10 | frick-fsw | Add M12 (OBCP engine: uPython/MicroPython OBCPs) and M13 (reference mission services: equipment manager + fault management); component gap analysis |
| 0.13 | 2026-10-11 | frick-fsw | Add M14 (multicore/SMP, after all other milestones) |
| 0.14 | 2026-10-11 | frick-fsw | Add M15 (in-house fundamental PUS-C service set) from FRICK-ARCH-0015 |
| 0.15 | 2026-10-11 | frick-fsw | M15 scope extended with CFDP + virtual filestore (VFS) and PUS-C S23 |
| 0.16 | 2026-10-11 | frick-fsw | Reconcile heading statuses with the actual state (N19): M3/M6/M7 done, M10/M11 in progress; M5 section moved into milestone order |
| 0.17 | 2026-10-11 | frick-fsw | Add M16 (in-house OBCP engine development, after M12); M12 reframed as engine integration (user directive) |
| 0.18 | 2026-10-12 | frick-fsw | Restore explicit M4 scope (N19 audit): PUS-C over IP/TSN + extended fieldbus (CAN-FD/CANopen/RS-485 DE) + PUSopen adapter made explicit deliverables (were only prose "absorbed"); M8 gains GEM TSN 802.1Qav/Qci/Qbb |
| 0.19 | 2026-10-12 | frick-fsw | NET follow-up: M8 status updated from "planned" to "in progress" (S8 = M8 gPTP + CoreTSN now the active sprint per sprint-plan); NC-S6-03 co-implemented, NC-154-01 closed |
