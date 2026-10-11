# ADR-0001 - cFS-inspired layered architecture

| Status | Accepted |
| ------ | -------- |
| Date   | 2026-10-10 |

## Context
A generic flight software framework for a small/medium spacecraft OBC must be
easy to port, to verify and to reuse across missions. NASA cFS provides a
proven blueprint (PAL/OSAL, Software Bus, Event Services, Executive Services,
Table Services, Time Services).

## Decision
Adopt the cFS-inspired mapping for frick-fsw:

- PAL (`src/core/pal`) <-> OSAL: platform abstraction (POSIX host / RTEMS).
- Software Bus (`src/core/sb`) <-> SB: publish/subscribe message bus.
- Event Services (`src/core/evs`) <-> EVS: on-board events.
- Executive Services (`src/core/es`) <-> ES: application registry/lifecycle.
- Table Services (`src/core/tbl`) <-> TBL: load/validate/activate tables.
- Time Services (`src/core/time`) <-> TIME: UTC correlation and CUC encoding.

## Consequences
Ports reduce to the PAL; applications are independent tasks; the cFS-style
naming helps reviewers familiar with cFS. No cFS code is copied (norm N13
licence compatibility).

## Traceability
SSS-REQ-002..010; SRS-REQ-001..016.
