# Non-Conformance: Integration Manual correctness vs codebase (FRICK-REVIEW-0159 / 0160)

| Field | Value |
| --- | --- |
| NC id | NC-INT-MAN-01 |
| Origin | FRICK-REVIEW-0159 (R1 FSW cross-check) + FRICK-REVIEW-0160 (R4 doc review), 2026-10-13 |
| Severity | Blocker (type-name mismatch would prevent compilation); 4 Major; 6 Minor |
| Owner | doc-engineering (author) |
| Target milestone / sprint | S8 (integration-manual refresh, in progress) |
| Norms involved | N8 (current docs), N12 (NC closure), N16 (revision history), N17 (documentation review R4) |
| Conformance class | ECSS Category B (Class-A gaps not applicable; this is a documentation correctness NC) |

## Summary

Two independent reviewers (R1 Senior FSW Engineer and R4 Senior Documentation
Engineer) cross-checked `docs/engineering/user-integration-manual.md` rev 0.6
against the actual source tree (`src/`, `tests/`, `CMakeLists.txt`, `Makefile`).
Both reviewers returned **REJECTED**. The manual contains a type-name mismatch
that would prevent compilation, missing/incorrect function names, wrong file-path
attribution, outdated status labels, and an outdated check count.

## Findings (consolidated)

### Blocker

| ID | Source | Section | Issue | Fix applied |
| --- | --- | --- | --- | --- |
| FSW-REV-01 | FRICK-REVIEW-0159 | §4 | `FF_TblValidateFn_t` does not exist; actual type is `FF_TblValidateFn` (declared in `src/core/tbl/tbl.h`). A customer copying the snippet would fail to compile. | §4 updated to `FF_TblValidateFn` |
| DOC-R4-01 | FRICK-REVIEW-0160 | §4 | Same type-name mismatch. | Same fix |

### Major

| ID | Source | Section | Issue | Fix applied |
| --- | --- | --- | --- | --- |
| FSW-REV-04 | FRICK-REVIEW-0159 | §7.1 | Link backends attributed to `link.c`; actual backends are `link_loopback.c`/`link_uart.c`. | §7.1 corrected to list the three source files |
| DOC-R4-04 | FRICK-REVIEW-0160 | §4 | `FF_TblValidate` listed as a function but does not exist; validation is the `validate` callback in `FF_Tbl_Def_t`, invoked internally by `FF_TblLoad`. | §4 rewritten |
| FSW-REV-05 | FRICK-REVIEW-0159 | §1 | Init sequence omits 6 real functions (`FF_BoardInit`, `FF_MissionTablesInit`, `FF_LinkInit`, `FF_EvsSetListener`, `FF_AppTmOutputInit`, `FF_AppHkRegister`) and misgroups `FF_PusInit`. | §1 rewrite to include the complete sequence |
| DOC-R4-02 | FRICK-REVIEW-0160 | §8 | `GAP-A4` attributed to formal-verification depth; actual is `GAP-A1` (formal C refinement gap). | §8 corrected to `GAP-A1` |
| DOC-R4-03 | FRICK-REVIEW-0160 | §8.1 | Address format described as "6 octets" without qualifying that the convention is adapter-specific; the loopback backend uses a 1-byte port. | §8.1 step 1 rewritten to state adapter convention + backend variance |
| FSW-REV-03 | FRICK-REVIEW-0159 | §7 | PUS backend fallback (ASN.1 → open) not documented. | §7 backend note added |
| DOC-R4-05 | FRICK-REVIEW-0160 | §8.1 | lwIP adapter file list missing `ff_lwip_netif.{c,h}`. | §8.1 step 2 + §10 updated |

### Minor

| ID | Source | Section | Issue | Fix applied |
| --- | --- | --- | --- | --- |
| FSW-REV-07 | FRICK-REVIEW-0159 | §1 | `stack` field name; actual is `stack_bytes`. | §1 + code example updated |
| FSW-REV-08 | FRICK-REVIEW-0159 | §3 | `FF_EvsEvent`; actual is `FF_EvsEvent_t`. | §3 updated |
| FSW-REV-09 | FRICK-REVIEW-0159 | §9 | `test_gem_tsn.c` "16 checks"; actual is 22 (extended with Qci/Qbb). | §9 table updated to 22 |
| FSW-COMMENT-008 | both | §1 | "six `src/apps/*`" ambiguous (demo + test). | §1 callout clarified |
| FSW-COMMENT-011 | both | §3 | TM_OUTPUT attribution needs clarification. | §3 rewritten |
| FSW-COMMENT-010 | both | §7 | PUS backend fallback note. | §7 backend note added |
| DOC-R4-09 | FRICK-REVIEW-0160 | §9.1 | TSN vtable methods `set_stream_filter`/`enable_pfc` not listed. | §9.1 updated with full method list |
| DOC-R4-07 | FRICK-REVIEW-0160 | §10 | lwIP adapter file list missing `ff_lwip_netif.{c,h}`. | §10 updated |
| DOC-R4-08 | FRICK-REVIEW-0160 | §1 | `FF_PalTaskFunc_t` should reference `pal.h`. | §1 updated with header path |
| DOC-COMMENT-015 | FRICK-REVIEW-0160 | §13 | `make coverage metrics` as a single target; actual is two targets. | §13 split into `make coverage` + `make metrics` |

### Comments (addressed)

| ID | Source | Section | Note | Action |
| --- | --- | --- | --- | --- |
| FSW-COMMENT-009 | FRICK-REVIEW-0159 | §3 | Event→PUS-S5 attribution. | §3 rewritten |
| DOC-R4-006 | FRICK-REVIEW-0160 | §7 | `FF_LinkSelect` NULL check not documented. | §7.1 documents NULL guard |
| DOC-R4-010 | FRICK-REVIEW-0160 | §1 | `FF_PalTaskFunc_t` reference. | §1 now references `src/core/pal/pal.h` |
| DOC-COMMENT-017 | FRICK-REVIEW-0160 | §8.1 | "address = 6 octets" wording. | §8.1 qualified as adapter convention |
| DOC-COMMENT-016 | FRICK-REVIEW-0160 | §9 | gPTP status "Planned (M8)". | §9 updated to "Implemented (M8)" |

## M8 status update (new since rev 0.6)

The rev 0.6 manual predated the M8/SPRINT S8 implementation. The following
status labels were stale and are now updated:

- §9 table: "Full 802.1AS **gPTP** stack — Planned (M8)" → **Implemented (M8)**
  with evidence: `src/drivers/eth/gptp/ff_gptp.{c,h}` (SRS-REQ-068), `test_gptp.c`
  (58 checks).
- §9 table: `test_gem_tsn.c` "16 checks" → "22 checks".
- §9.1: Added `set_stream_filter` (802.1Qci) and `enable_pfc` (802.1Qbb) to the
  method list; documented `configure_cbs` returns `FF_NOT_IMPLEMENTED`.
- §9.1: Added gPTP usage instructions (§4: `FF_GptpInit` / `FF_GptpSetEnabled` /
  `FF_GptpTick` / `FF_GptpSendPDelayReq` / `FF_GptpGetGrandmasterTime`).

## NC disposition

All findings corrected in `docs/engineering/user-integration-manual.md` rev **0.7**
(2026-10-13). A new R4 documentation re-review (FRICK-REVIEW-0164) verified
convergence before this NC can be closed (norm N12).

| Finding | Status |
| --- | --- |
| FSW-REV-01 / DOC-R4-01 (Blocker) | FIXED |
| FSW-REV-03..05 / DOC-R4-02..04 (Major) | FIXED |
| FSW-REV-07..09 / FSW-COMMENT-008/010/011 / DOC-R4-06..09 / DOC-COMMENT-015/017 (Minor) | FIXED |
| FSW-COMMENT-009 / DOC-R4-005/006 (Comment) | FIXED |
| M8 stale status labels | FIXED |
| S4b doc delta (§7.1 link backends, §8 status table, §8.2 usage, §10 note) | FIXED (rev 0.8) |

**Re-open condition:** the R4 re-review (FRICK-REVIEW-0164) has returned APPROVED
WITH COMMENTS; this NC is closed (norm N12).

## References

- `docs/reviews/2026-10-13-integration-manual-fsw-review.md` (FRICK-REVIEW-0159, R1)
- `docs/reviews/2026-10-13-integration-manual-doc-review.md` (FRICK-REVIEW-0160, R4)
- `docs/reviews/2026-10-13-integration-manual-delta-r1-review.md` (R1 delta, APPROVED)
- `docs/reviews/2026-10-13-integration-manual-delta-r4-review.md` (R4 delta rev 0.7, APPROVED)
- `docs/reviews/2026-10-13-s4b-integration-manual-r4-review.md` (FRICK-REVIEW-0164, R4, APPROVED WITH COMMENTS)
- `docs/engineering/user-integration-manual.md` (FRICK-ENG-0005) rev 0.8
- `docs/requirements/traceability.yaml` (SRS-REQ-059/060/061 updated to implemented)
- `docs/requirements/SRS.md` (SRS-REQ-066/068/059/060/061)
- `src/transport/link/link_ip.h` / `link_ip.c` (SRS-REQ-060)
- `src/transport/link/link_tsn.h` / `link_tsn.c` (SRS-REQ-061)
- `src/transport/link/link_seg.h` / `link_seg.c` (SRS-REQ-059)
