# Software Criticality, Safety and FDIR

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-QUAL-SAFE-001                    |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-10                             |
| References                        | ECSS-Q-ST-80C (software criticality, dependability), `docs/rules.md`, `docs/architecture/overview.md` |

## 1. Criticality categorisation

Per ECSS-Q-ST-80C, software criticality derives from the severity of the
system hazards it can cause or fail to mitigate. For this framework the target
is **category B** (a failure can lead to loss of a mission function or degrade
availability, without catastrophic consequences); the framework is the C&DH
backbone, so its failure is mission-critical but not life-critical.

## 2. Software contributions to hazards (preliminary)

| Hazard class | Software contribution | Mitigation in the framework |
| ------------ | --------------------- | --------------------------- |
| Loss of telecommand (TC) | dispatch failure, link loss | PUS S1 verification, bounded queues, EVS events |
| Corrupted TC/TM | bit errors | CRC-16/CCITT-FALSE; length bounds |
| Loss of time reference | PPS loss, wrong source | time-source selection, discipline rules, reportable state |
| Resource exhaustion | queue/memory growth | static allocation, bounded queues (`SSS-REQ-010`) |
| Silent application failure | task crash/hang | HS heartbeats + commandable reset (S8 path) |
| Non-determinism | unbounded latency | bounded work per cycle, no heap after init |

## 3. Failure Detection, Isolation and Recovery (FDIR) concept

- **Detection**: EVS event reporting (S5), HS heartbeat monitoring, status
  telemetry (S3/S5), tone-count/diagnostics.
- **Isolation**: per-application tasks and bounded pipes contain faults; service
  handlers return `FF_Status_t` and never block unboundedly.
- **Recovery**: HS can trigger application restart; a commandable software reset
  is available (S8 path). Time re-correlation via PUS S9,1.

## 4. Limitations and gaps

- This is a **preliminary** software hazard analysis; a full system FMEA/FMECA
  requires the system hazard inputs and is tracked as **GAP-A10**.
- On-target fault-injection campaigns are GAP-A4; HIL recovery scenarios are
  GAP-A5.
- Radiation effects (SEU/SEL) on the RTPFS160ZT are out of software scope
  (GAP-A6); software mitigations (memory scrub, watchdogs) belong to the BSP.

## 5. Traceability

Mitigations map to SRS/SSS (verification by test/analysis) and to the
operational services (PUS S1/S3/S5/S8/S9). The HS application implements the
heartbeat/reset behaviour.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial criticality, safety and FDIR analysis (preliminary) |
