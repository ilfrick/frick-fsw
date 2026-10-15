# System Verification Architecture

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-SVA-001                       |
| Revision                         | 0.1                                    |
| Date                             | 2026-10-14                             |
| Norms                            | N5, N6, N10, N11, N15, N21, N22        |
| Applicable standards             | ECSS-E-ST-10-02C Rev.1, ECSS-E-ST-10-03C Rev.1, ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2 |
| Baseline                         | Uncommitted working tree at HEAD `57974d2` + current changes |
| Companion                        | `docs/vv/comprehensive-test-spec.md` (FRICK-VV-TSPEC-001) |

This document defines the **system-level verification architecture** for
frick-fsw from an avionics systems-engineering perspective: test levels, V-model
mapping, interface coverage, fault-management strategy, timing/determinism,
hardware/environmental qualification, security, and tool qualification.

> **Authority statement.** This is engineering analysis and draft verification
> architecture. It does not assign mission criticality, environmental levels,
> radiation requirements, launch/operational readiness, or accept risk. The
> project states an ECSS **class-B target**; no controlled mission authority has
> assigned that level (NC-EXT-11). Environmental/qualification levels and
> applicable tailoring require program/customer authority.

## 1. Verification levels and V-model mapping

| Level | Scope | ECSS method(s) | Environment | Norm |
| --- | --- | --- | --- | --- |
| L1 Unit/component | functions, modules, contracts | Test (Tst), Analysis | SIL | N22/N15 |
| L2 Integration | component↔component, component↔platform/ground | Test, Demonstration (Dem) | SIL + PIL | N22/N6 |
| L3 System | full boot, modes, end-to-end TM/TC | Demonstration, Test | PIL + target | N18/N6 |
| L4 Qualification | hardware, environment, timing toward target | Test, Analysis, Inspection | Target | GAP-A5/A6 |
| L5 Acceptance | release/flight readiness | Review, Inspection, Demonstration | As-configured | N7/N20 |

Verification-method legend (ECSS): **Test** (measurable), **Analysis**,
**Inspection**, **Review of design**, **Demonstration**.

## 2. Interface test coverage (ICD-driven)

Source: `docs/interfaces/ICD.md` (FRICK-ICD-001), `config/fsw_config.h`,
`src/hal/**`. Every interface gets nominal and out-of-bound cases with explicit
oracles (see companion §4).

| Interface group | Key interfaces | Nominal | Out-of-bound |
| --- | --- | --- | --- |
| CCSDS Space Packet (133.0-B-2) | primary header, sec. header, PDL, CRC trailer | encode/decode round-trip, CRC | undersized PDL, 16-bit wrap, coalesced/trailing bytes, bad CRC (NC-EXT-01/13) |
| PUS-C (E-ST-70-41C) | S1,3,5,6,8,9,17,20; N7 backend | accept→verify→complete, HK, ping, mem dump, time | unknown service/subservice, illegal APID, bad T-field length (NC-EXT-04) |
| Time (CUC 301.0-B-4) | PPS, CUC, leap, gPTP | tone discipline, leap-at-boundary | clock drift, leap edge, source loss |
| Link layer | loopback, IP/UDP/TCP, TSN (0x88F5), UART | PUS round-trip per backend | seg desync, partial/oversized frame |
| HAL contracts | GPIO, UART, SPI, I2C, CAN, ETH, TSN, PCIe, SpaceWire, NVM, MTD, Block, Mem | per-contract nominal ops | arg faults, timeouts, range violations |
| Ground interface | TM/TC byte stream, GSW | ping/pong decode | resync, offset parsing, malformed |

## 3. Fault-management (FDIR) test strategy

| FDIR stage | Test intent | Oracle |
| --- | --- | --- |
| Detection | inject each failure hypothesis; confirm monitor trips | expected monitor/event within latency |
| Isolation/containment | resource limits, partition/mode behavior | no spread beyond containment region |
| Recovery | safe-mode entry, degradation, restart, ground command | defined safe state reached, observable |
| Common cause | shared clock/power/bus/code/compiler analysis | independence argument evidenced |

Open dependencies: failure hypotheses and detection coverage are pending
authority disposition (NC-EXT-04/13/31). Until then, tests use documented
project assumptions and record the unresolved contract (no invented oracles).

## 4. Timing and determinism verification

| Aspect | Method | Status |
| --- | --- | --- |
| WCET | measurement (SIL/PIL) + static analysis | GAP-A4; not yet qualified |
| Schedulability | fixed-priority preemptive analysis | to be evidenced |
| End-to-end latency | sensor→process→actuator/TM chains | to be evidenced |
| Jitter / ISR latency | PIL measurement | to be evidenced |
| Multicore interference | 5×U54 + 1×E51, shared L2/mem controller | design-stage; not characterized |

Determinism claims are not made without target WCET and interference evidence.
"Average load is low" is not timing assurance.

## 5. Environmental and hardware qualification

| Item | Requirement | Status |
| --- | --- | --- |
| Radiation (SEE/SEL/SEFI) | per mission environment | not characterized (GAP-A6) |
| Temperature/vibration/shock/EMC | per mission environment | not defined (authority) |
| POR/brownout/clock-fail/watchdog | on silicon | design only; target test pending |

All of the above are authority-owned levels; the project provides the test
architecture and records gaps; it does not set levels.

## 6. Security test requirements

- Secure boot chain (ROM→FSBL→SSBL→application): integrity/authenticity checks
  (scope per approved threat model; not yet authority-agreed).
- Command authentication/authorization, replay protection, rate limiting: tests
  to be defined with the security authority.
- Key management, anti-rollback, secure update: pending NC-EXT-43/44 disposition.
- SPDX license compliance (T21 checker): legal/provenance concern, kept separate
  from functional security; T21 is NOT FULLY QUALIFIED.

## 7. Test tool qualification (N10)

Tools T1..T21 registered in `docs/vv/tool-qualification.md`. At class-B target
the minimum is source review + inspection + CM; independent review where the
criticality/risk requires. Current open qualification items: **T1, T13, T18,
T21** (NOT FULLY QUALIFIED). Any change to a recorded V&V script invalidates its
qualification and forces re-qualification before its evidence is reused.

## 8. Integration with the N22 test specification

The SW V&V comprehensive test specification (FRICK-VV-TSPEC-001) provides the
**test cases and oracles**; this document provides the **system context** —
levels, environments, interface scope, fault/timing/environmental architecture,
and tool qualification — into which those cases are placed. Both are controlled
under N8/N16 and reviewed under N17/N20.

## 9. Open items (recorded, not blocking under N21)

| ID | Item | Owner | Disposition |
| --- | --- | --- | --- |
| NC-EXT-47 | Test-evidence mapping to N22 | FSW/V&V | later disposition |
| GAP-A4 | WCET/latency/stack/multicore evidence | V&V | class-A plan |
| GAP-A5 | On-target peripheral behavior | HW/V&V | class-A plan |
| GAP-A6 | Environmental/radiation qualification | program | authority |
| NC-EXT-11 | Criticality assignment | authority | authority |
| NC-EXT-31 | Primary-header/failure-hypothesis criteria | authority | authority |

## History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-14 | frick-fsw | Initial system verification architecture per N22 and owner comprehensiveness directive; V-model levels, ICD-driven interface coverage, FDIR, timing, environmental, security and tool-qualification architecture; open items recorded |
