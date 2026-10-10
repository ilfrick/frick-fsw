# Reference Applications

| Document id | FRICK-ARCH-0005 |
| ----------- | --------------- |
| Revision    | 0.1 |
| Date        | 2026-10-10 |

The framework ships six reference applications registered by Executive
Services in `fsw_main.c`. They demonstrate the architecture and provide the
V&V exerciser (simulator smoke).

| App | Entry | Role |
| --- | ----- | ---- |
| CMD_INGEST | `FF_AppCmdIngestMain` | receives TC frames from the link and feeds `FF_PusIngest` |
| TM_OUTPUT  | `FF_AppTmOutputMain`  | drains the PUS TM sink into the link (loopback/UART) |
| SCHEDULER  | `FF_AppSchedulerMain` | cyclic wake-up / tick service scheduling |
| HK         | `FF_AppHkMain`        | periodic housekeeping via Table/ps platform data |
| HS         | `FF_AppHsMain`        | health & safety heartbeat monitoring |
| SAMPLE     | `FF_AppSampleMain`    | sample telemetry generator (bus subscriber/publisher) |

Each application is an independent task (fixed stack, priority, pipe depth,
see `src/apps/app.h` and `fsw_main.c`).

## Traceability

SRS-REQ-011/014, SSS-REQ-005/008.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial reference-applications note (norm N8/N16) |