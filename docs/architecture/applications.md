# Reference Applications

| Document id | FRICK-ARCH-0005 |
| ----------- | --------------- |
| Revision    | 0.2 |
| Date        | 2026-10-13 |

The framework ships six reference applications registered by Executive
Services in `fsw_main.c`. They demonstrate the architecture and provide the
V&V exerciser (simulator smoke).

| App | Entry | Role |
| --- | ----- | ---- |
| CMD_INGEST | `FF_AppCmdIngestMain` | receives TC frames from the link and feeds `FF_PusIngest` through the single-iteration `FF_AppCmdIngestReceiveOne` path, whose bounded buffer is `FF_CCSDS_MAX_TC_PACKET` |
| TM_OUTPUT  | `FF_AppTmOutputMain`  | drains the PUS TM sink into the link (loopback/UART) |
| SCHEDULER  | `FF_AppSchedulerMain` | reference scheduler stub; planned replacement is the bounded SB schedule dispatcher (M17/SRS-REQ-071), with native RTEMS retaining task scheduling |
| HK         | `FF_AppHkMain`        | periodic housekeeping via Table/ps platform data |
| HS         | `FF_AppHsMain`        | health & safety heartbeat monitoring |
| SAMPLE     | `FF_AppSampleMain`    | sample telemetry generator (bus subscriber/publisher) |

Each application is an independent task (fixed stack, priority, pipe depth,
see `src/apps/app.h` and `fsw_main.c`).

## Traceability

SRS-REQ-011/014, SSS-REQ-005/008.

The current scheduler app is demonstration code, not mission-ready: it uses a
shared last-run timestamp and does not yet implement controlled table activation,
missed-slot handling, or scheduler-fault recovery. See
`docs/architecture/scheduling.md` and sprint S8-SCH for the planned replacement.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial reference-applications note (norm N8/N16) |
| 0.2 | 2026-10-13 | frick-fsw | Clarify current scheduler stub limitations and link M17/S8-SCH replacement plan |
