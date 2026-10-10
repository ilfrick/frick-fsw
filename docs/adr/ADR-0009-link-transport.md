# ADR-0009 - Decoupled link and transport layers

| Status | Accepted |
| ------ | -------- |
| Date   | 2026-10-10 |

## Context
The transport codec (CCSDS/PUS) must stay independent of the physical link so
future SpaceWire/radio back-ends can be added without touching the protocol
logic.

## Decision
- `src/transport/ccsds/` - Space Packet encode/decode + CRC (pure, testable,
  Lean-mirrored).
- `src/transport/link/` - link abstraction (`link.h`) with `link_loopback.c`
  (host/simulator) and `link_uart.c` (RTEMS `/dev/console`).
- PUS layer consumes frame bytes via `FF_LinkRxHandler`; TM produced by the
  PUS backend is handed to the link for emission.

## Consequences
The loopback link enables on-host CI without hardware; the UART target link is
used on the RTPFS160ZT board (GAP-A5 for on-target qualification).

## Traceability
SRS-REQ-012, SSS-REQ-014.