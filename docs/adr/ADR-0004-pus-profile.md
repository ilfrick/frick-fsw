# ADR-0004 - PUS-C / CCSDS Space Packet profile

| Status | Accepted |
| ------ | -------- |
| Date   | 2026-10-10 |

## Context
Ground-segment interoperability requires a well-defined TM/TC packet profile.

## Decision
- CCSDS Space Packets (CCSDS 133.0-B-2) wrapped by the PUS-C layer
  (ECSS-E-ST-70-41C): PUS version 2, mission identifier `0x01`, TC APID `0x010`,
  TM APID `0x020`.
- CRC-16/CCITT-FALSE trailer on TM/TC frames (`FF_FRAME_WITH_CRC=1`).
- UART byte-stream transport on target (`/dev/console`, 115200 baud);
  `loopback` link on the host simulator. Full field map: `docs/interfaces/ICD.md`.

## Consequences
Tuning is concentrated in `config/fsw_config.h` (SRS-REQ-019); the codec is
mirrored by the Lean model (SRS-REQ-003/004).

## Traceability
SRS-REQ-003, SRS-REQ-004, SRS-REQ-017, SSS-REQ-003, SSS-REQ-014.
