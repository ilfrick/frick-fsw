# Reference Applications

| Document id | FRICK-ARCH-0005 |
| ----------- | --------------- |
| Revision    | 0.3 |
| Date        | 2026-10-16 |

The framework ships six reference applications registered by Executive
Services in `fsw_main.c`. They demonstrate the architecture and provide the
V&V exerciser (simulator smoke).

| App | Entry | Role |
| --- | ----- | ---- |
| CMD_INGEST | `FF_AppCmdIngestMain` | receives TC frames from the link and feeds `FF_PusIngest` through the single-iteration `FF_AppCmdIngestReceiveOne` path, whose bounded buffer is `FF_CCSDS_MAX_TC_PACKET` |
| TM_OUTPUT  | `FF_AppTmOutputMain`  | drains the PUS TM sink into the link (loopback/UART) |
| SCHEDULER  | `FF_AppSchedulerMain` | table-driven dispatcher: releases configured SB messages through the bounded allocation-free `ff_sched` module (per-entry absolute release targets, atomic table activation, missed-slot/heartbeat/dispatch-failure evidence); native RTEMS retains task selection and preemption (M17/SRS-REQ-071) |
| HK         | `FF_AppHkMain`        | periodic housekeeping via Table/ps platform data |
| HS         | `FF_AppHsMain`        | health & safety heartbeat monitoring |
| SAMPLE     | `FF_AppSampleMain`    | sample telemetry generator (bus subscriber/publisher) |

Each application is an independent task (fixed stack, priority, pipe depth,
see `src/apps/app.h` and `fsw_main.c`).

## Traceability

SRS-REQ-011/014, SSS-REQ-005/008.

The scheduler app releases SB messages through the bounded `ff_sched` module:
each entry has its own absolute next-due instant (no shared-timestamp
starvation), tables are validated then activated atomically, work per poll is
bounded, and missed/late/publish-error counters plus heartbeat/missed-slot EVS
events feed the health-monitoring path. Mission rates, jitter tolerances,
late-slot policy and recovery authority remain controlled inputs, and target
timing evidence is not yet available; see `docs/architecture/scheduling.md`
section 5.1 and sprint S8-SCH.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial reference-applications note (norm N8/N16) |
| 0.2 | 2026-10-13 | frick-fsw | Clarify current scheduler stub limitations and link M17/S8-SCH replacement plan |
| 0.3 | 2026-10-16 | frick-fsw | Update the SCHEDULER row to the delivered ff_sched dispatcher and retain the explicit mission-input/target-evidence limits |
