# cFS Mapping for Reviewers

| Document id | FRICK-ARCH-0007 |
| ----------- | --------------- |
| Revision    | 0.2 |
| Date        | 2026-10-10 |

frick-fsw is inspired by NASA cFS; reviewers familiar with cFS can map the
concepts directly (ADR-0001). No cFS code is copied.

| cFS concept           | frick-fsw equivalent                   | Files |
| --------------------- | -------------------------------------- | ----- |
| OSAL                  | PAL                                    | `src/core/pal/` |
| Software Bus (SB)     | Software Bus (sb)                      | `src/core/sb/` |
| Event Services (EVS)  | Event Services (evs)                   | `src/core/evs/` |
| Executive Services (ES)| Executive Services (es)               | `src/core/es/` |
| Table Services (TBL)  | Table Services (tbl)                   | `src/core/tbl/` |
| Time Services (TIME)  | Time Services (time)                   | `src/core/time/` |
| cFE message ids `0xMMSS` | SB message ids (wildcard 0xFFFF)     | `config/fsw_config.h` |
| App framework + tasks | apps + ES registry                     | `src/apps/`, `src/core/es/` |
| CCSDS / CFDP          | PUS-C + CCSDS Space Packets + CRC      | `src/pus/`, `src/transport/` |

The main divergences: PUS-C is the protocol layer (cFS cores do not mandate
PUS), and Lean formal verification of requirement theorems is built-in
(norm N1).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial cFS mapping note (norm N8/N16) |
| 0.2 | 2026-10-10 | frick-fsw | Assign unique document id FRICK-ARCH-0007 (R4 review DOC-06) |