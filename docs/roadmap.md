# Development roadmap

| Document id | FRICK-RM-0001 |
| ----------- | ------------- |
| Revision    | 0.1 |
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

## M2 — Time base: PPS capture and CUC timecode (planned)

Add a hardware-disciplined time base, from heritage (NASA cFS Time Services:
1 Hz tone, time source selection, `AddLeapSeconds`) and ECSS/CCSDS practice.

Scope:

- PPS / 1 Hz tone driver abstraction (`FF_TimeTone_t`) with a native software
  backend (simulator/tests) and an RTEMS backend (GPIO/PLIC capture) to be
  wired by the board integrator; see `docs/engineering/driver-development.md`.
- Time source selection (internal CPU / external tone / ground), latching the
  monotonic counter at each tone and steering the UTC correlation.
- CUC encode/decode completion, including the CCSDS P-field, plus leap-second
  application at the tone boundary (pending-leap handling).
- Requirements: new SSS/SRS items; Lean theorems for monotonicity, CUC
  round-trip and leap-second application; unit tests `test_time.c`.
- Documentation: update `docs/architecture/time-and-timecode.md` and the ICD.

## M3 — On-board file transfer: virtual filestore, CFDP, PUS service 23 (planned)

Complete on-board file management and a file-delivery protocol.

Scope:

- **Virtual filestore (`src/fs/`)**: a complete RAM-backed store plus a backend
  vtable (`FF_VfsBackend_t`) so users can plug the mission's real memory
  management system without touching the protocol layers. Documented hooks and
  an integration guide for users.
- **PUS service 23 — File management** (ECSS-E-ST-70-41C): create/delete/copy/
  move/list/attributes over the virtual filestore.
- **CFDP** (CCSDS 727.0-B-5): a class-1 (unacknowledged) and class-2
  (acknowledged) engine performing PUT/GET against the filestore, with fault
  handling and filestore requests; PDUs carried over the link layer.
- Requirements, Lean models (checksum/segment arithmetic where applicable),
  unit tests, and user + integrator guides.

## M4 — IP and TSN transport over PUS-C (planned)

Transparent carriage of PUS-C packets over Ethernet, including segmentation and
reassembly so that PUS messages larger than an Ethernet frame are handled
without the user seeing the fragmentation.

Scope:

- Segmentation/reassembly (SAR) layer for PUS-C packets larger than the link
  MTU, with sequence control, timeout and duplicate handling.
- **TCP/IP link backend** over the existing `FF_Link_t` contract.
- **TSN link backend** (IEEE 802.1 real-time Ethernet: priority/VLAN, time-aware
  scheduling) for deterministic delivery.
- PUS-C encapsulation/decapsulation and service-level transparency.
- Requirements, Lean models (SAR reassembly correctness), unit tests, and user
  + integrator guides.

## M5 — Documentation and status program (in progress)

- README index, documentation index and repository map (norm N8).
- PUS services catalogue and low-level driver inventory in README.
- `docs/engineering/driver-development.md` — human + agent guide.
- `docs/engineering/service-development.md` — human + agent guide.
- README metrics panel: coverage plus review/V&V traffic lights
  (red = failures, yellow = issues deferred by the user, green = all pass).

## History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial roadmap: M2 (PPS/CUC), M3 (filestore/CFDP/service 23), M4 (IP+TSN over PUS-C), M5 (docs) |
