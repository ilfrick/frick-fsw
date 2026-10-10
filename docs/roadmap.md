# Roadmap

| Document id | FRICK-RMAP-001 |
| ----------- | -------------- |
| Revision    | 0.1 |
| Date        | 2026-10-10 |

## Mission

Reach a deliverable ECSS class-B baseline (v1.0.0) with the documented path to
class A (norm N5). All work items are tracked against the norms and the
traceability manifest; gate = `make trace` green.

## Releases

| Version | Objective | Norms exercised |
| ------- | --------- | --------------- |
| v0.1.0 (current) | Framework baseline: core services, PUS-C layer, simulator, Lean model, traceability, Docker CI | N1-N14 |
| v0.2.0 | Full PUS-C service set S1-S20 on the open backend; per-service unit tests; UART link qualification harness | N6, N10, N11 |
| v0.3.0 | RTEMS 6.2 on-target bring-up (RTPFS160ZT), BSP package, HWIL smoke; start GAP-A5 closure | N6, N10 |
| v1.0.0 | Class-B baseline book: SES records, independent review closure, qualification evidence, MISRA clean gate | N9, N12, N14 |

## Work packages (next)

| WP | Work | GAP/Link |
| -- | ---- | -------- |
| WP-FV-A | Arbitrary-length CRC induction, full C dispatch embedding in Lean | GAP-A1 |
| WP-PUS-A | S2, S11, S12, S13, S15, S18, S19, S20 services + malformed-frame tests | GAP-A2 |
| WP-HS-T | HS unit tests (heartbeat, app failure detection) and fault injection | SRS-REQ-014 |
| WP-RTEMS-B | Board package, boot up to `fsw_main`, UART TM/TC on target | GAP-A5 |
| WP-MISRA | MISRA-C:2012 audit and clean gate; WCET/stack report for the BSP | GAP-A4 |
| WP-SES | SES/QA organisational records + independence evidence per ECSS-Q-ST-80 | GAP-A3 |

## Review cadence

- On each SSS change: R3 systems review (norm N14).
- On each release milestone: R2 independent V&V review and R1 code review;
  NCs closed per norm N12.
- On each change to V&V scripts/tools: re-qualification review (norm N10).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial roadmap |