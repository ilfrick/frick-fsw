# Development roadmap

| Document id | FRICK-RM-0001 |
| ----------- | ------------- |
| Revision    | 0.5 |
| Date        | 2026-10-10 |

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

## M3 — Middleware integration & hardware-abstraction interfaces (in progress)

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

## M4 — MPFS low-level layers and 1 Gb/s Ethernet (planned)

Reference implementations behind the M3 contracts:

- Native MSS peripheral layers in `src/bsp/mpfs/` (MMUART/RS-422/RS-485, CAN
  2.0, SPI, I2C, QSPI NOR MTD, MMC/eMMC/SD block device, GPIO/IRQ, DDR/DMA,
  RTC/timer).
- A complete **1 Gb/s Ethernet driver** on the GEM, integrated with the
  middleware link/network layer, with pluggable `FF_NetStack_t` adapters
  (Tuxera, lwIP, RTEMS libbsd) and the `FF_TsnBackend_t` seam.
- Generic fabric-device (FIC/AXI) backend path for fabric-attached resources
  (CAN-FD, SpaceWire/RMAP, TSN), no FPGA IP.
- Absorbed from the former roadmap: IP + TSN over PUS-C (segmentation and
  reassembly, TCP/IP and TSN link backends) and the on-board file transfer set
  (virtual filestore, PUS service 23, CFDP).

## M6 — Mathematics library integration (libm) (in progress)

A documented, portable mathematics facade over the platform `libm`
(`src/math/ff_math.{c,h}`, `FF_USE_LIBM`), for applications that need
floating-point math, with the determinism policy (no use in hard real-time
critical paths without a work-package analysis) and the licensing analysis
documented. Requirements SSS-REQ-018 / SRS-REQ-033; ADR-0012;
`docs/architecture/numerics.md`.

## M5 — Documentation and status program (in progress)

- README index, documentation index and repository map (norm N8).
- PUS services catalogue and low-level driver inventory in README.
- `docs/engineering/driver-development.md` — human + agent guide.
- `docs/engineering/service-development.md` — human + agent guide.
- `docs/engineering/user-integration-manual.md` — human + agent guide.
- `docs/development-plan.md` — milestone/objective plan.
- README metrics panel: coverage plus review/V&V traffic lights
  (red = failures, yellow = issues deferred by the user, green = all pass).

## History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial roadmap: M2 (PPS/CUC), M3 (filestore/CFDP/service 23), M4 (IP+TSN over PUS-C), M5 (docs) |
| 0.2 | 2026-10-10 | frick-fsw | Milestone M2 delivered (time source, tone discipline, leap-at-boundary, CUC framing, PPS backend vtable) |
| 0.3 | 2026-10-10 | frick-fsw | Re-plan: M3 = HAL interfaces + integration manuals; M4 = MPFS BSP + 1 Gb/s Ethernet (absorbs filestore/CFDP/IP/TSN); pointer to `docs/development-plan.md` |
| 0.4 | 2026-10-10 | frick-fsw | M3 status set to in progress (HAL interfaces delivered) |
| 0.5 | 2026-10-10 | frick-fsw | Add M6 (mathematics library integration, libm); M3 delivered interfaces |
