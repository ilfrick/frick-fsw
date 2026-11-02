# XW-2 target contract — post-correction evidence of record

| Field | Value |
| --- | --- |
| Document ID | FRICK-EVID-XW2-POSTFIX-20261102 |
| Revision | 1.0 |
| Date | 2026-11-02 |
| Run directory | `build/xw2-run-20261102T011328Z` |
| Script version | `tools/scripts/qemu_xw2_vv.sh` = `671805b827e4566556f7962054a96eb0d7dd3a7c26c7dd86d2cfad210ffde3e0` |
| Manifest version | `tools/scripts/xw2_manifest.json` = `8d1fdbc50b5816d42f2fad110b4d760d6212336a5abda23d379e9bab40438819` |
| Backend | `docker` (recorded in `backend.txt`) |
| Toolchain image | `frick-rtems-toolchain:local` `sha256:8cdb7b38d6b2696cc8c00acf6b622f5f8d9c3728fbf6be3811df28533843a896` |
| Config | `FF_USE_PUS_ASN1=OFF` (contract-equivalent, no PUS codec in XW-2 image), QEMU timeout 25 s |
| Commit | `ae278ef8734d12e69a20d41ba367c12c16155afe` (XW-2 commit on both remotes) |

## Result

| Item | Value |
| --- | --- |
| Overall verdict | **PASS** |
| Required cases | 13/13 (SRS-REQ-073 PASS=12, SRS-REQ-075 PASS=1) |
| Whole-run | PASS (no panic/fatal marker) |
| QEMU exit | 124 (accepted) |
| Elapsed wall-clock | 25,360,537,941 ns (recorded in `positive-elapsed-ns.txt`) |
| Control (no-wrap) | **EXPECTED FAILURE** — watchdog init panic + component verifier exit 1 |
| Control passing cases | XW-T8 only |

## Wrapper segregation (asserted)

| Image | Wrap call sites | Real-hook call sites |
| --- | --- | --- |
| XW test image | 3 | 0 |
| Flight-shaped image | 0 | 3 |

## Supersession

This run supersedes the pre-correction runs:
- `build/xw2-run-20261101T170112Z` (lacked `backend.txt`, `toolchain-id.txt`, `pus-asn1.txt`, `qemu-timeout.txt`, `positive-elapsed-ns.txt`, asserted nm counts)
- `build/xw2-run-20261101T170759Z`
- `build/xw2-run-20261101T172050Z` (cited in committed docs but produced by pre-correction runner)

All three prior runs and their archive `docs/vv/evidence/2026-11-01-xw2-target-final/` are retained as **superseded history**. The evidence of record for the committed XW-2 increment is this directory.

## Contents

All files from the run directory plus this README and `assets.sha256`.
