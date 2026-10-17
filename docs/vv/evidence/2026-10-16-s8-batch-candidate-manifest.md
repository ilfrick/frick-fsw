# S8 batch candidate evidence manifest — 2026-10-16

| Field | Value |
| --- | --- |
| Candidate base | `b8c3aeec5484476407367024f3ce25c5af2bd30e` plus current uncommitted tree |
| Scope | S8-SCH, S8-VV-FU, S8-PUS-FU corrections and selected pinned N7 OBCP S9 build/connectivity integration; this manifest does not assert full sprint completion |
| Candidate product-set digest | `6a94057d7f3b905f0b73fe54d6b686d3a5b3e5f7a2f972308a0f9e5d4657d035` |
| Digest method | SHA-256 over the 23 explicitly enumerated changed source/test/configuration/requirements/specification files in §3, sorted by path; for each path: UTF-8(path) + NUL + SHA256(file contents) + LF. Excludes status/history/review/evidence records; this is a product-set identity, not a full repository snapshot. |
| Status | Candidate identity aid for the recorded local evidence, not a complete immutable build/provenance record, qualification, phase approval or release authorization |

## Configuration and generated-input identity

- `HEAD`: `b8c3aeec5484476407367024f3ce25c5af2bd30e`.
- Native `build/ci`: Debug, `FF_PLATFORM=native`, tests ON, lwIP ON,
  `FF_USE_N7_OBCP=ON`, `FF_USE_PUS_ASN1=ON`. `CMakeCache.txt` SHA-256:
  `7f865d42506bf513a1057ceb4f57aafb2e7d87a61c5d58676f71f5cad49da7f6`.
- V&V container `frick-vv:local` image ID:
  `sha256:5cbc72aa4125cfdbd8c0baa4ac2a1cb9d1447ff3147ac0b94e2ed0d918b83e91`.
- N7 OBCP engine commit:
  `55d6d97e7e7ff08a130698f7a922bcf58acb3d99`; MicroPython commit:
  `647c8b96cae7e202c7a020395b7cfe65e5b8ce04`.
- Generated flat-source set: 221 files; digest of sorted per-file SHA-256
  records: `dd9666ad5cb09a9ffd71e7f3d17a86122f1dcc0211a53de9264d2ab9d9a8e822`.
  Recompute at repository root with:
  `find build/ci/n7s-obcp/flat -type f -print0 | sort -z | xargs -0 sha256sum`
  and hash that text file; the retained per-file list is
  `docs/vv/evidence/2026-10-16-n7-generated-flat-file-hashes.txt` (221 rows,
  SHA-256 equal to the digest above).
  PA 0346 verified the 221-path per-file list and aggregate digest against the
  observed tree. The `make flat` generator/tool version is not independently
  recorded or qualified; NC-EXT-84 remains OPEN for provenance. The hash list
  identifies this observed generated tree, not reproducible generation.

## Results associated with this candidate

| Result | Outcome | SHA-256 / identity |
| --- | --- | --- |
| `make ci-local` (`2026-10-16-s8-final-batch-ci-local.log`) | PASS, 43/43 CTest, build, Lean and trace | `8c416fa97704672841be8f63a6de3154d0baec2eafc1bbf97b9da0db3e102ee4` |
| Host executable `build/ci/frick_fsw` | built by the above cycle | `5139bd28f9bd1a56ec97fab34cc3048fd4991c033140bd57522256ee580d28cf` |
| `test_pus_fu` | included in 43/43 suite | `7a2c0cb0727d412389d64f206e0289f634fd20f67ec9dfc9f7e0ae1bafc6bebe` |
| `test_decoder_corpus` | included in 43/43 suite | `9df52a46689328e7db92b0615b552901b145782f6e3f864df1d8cd8182a1612a` |
| `test_obcp_integration` | included in 43/43 suite | `5238cc9aea301bcdd32a8eaede1caca869db39f04b8cb6e5a23637829deea6f6` |
| `make smoke` | PASS (banner + 6 applications) | `2026-10-16-s8-final-batch-smoke.log`, `64b1a189c44a1ea3586e31609d63740ae4f5f415b00f63a03a3a6b7ab987b0cf` |
| `make gsw-selfcheck` | PASS (Python tests, license check, packet self-check) | `2026-10-16-s8-final-batch-gsw.log`, `ccb5018d23ad64371a3296cea066cdbf1f9349d5f16df9c02fb7d631ab5b9554` |
| `make format-check` | PASS | `2026-10-16-s8-final-batch-format.log`, `c43cc487f79481072975f09f6becf98d6621c85167fa97da90b1c431c2dfc774` |
| `make docs` | PASS, Doxygen emits existing warnings | `2026-10-16-s8-final-batch-docs2.log`, `b975d14f95e85454f57bc93482327de7b063b5306dda5ffdd10c7c01bb86660e` |
| `make renode` | PASS (`RENODE_TEST: PASS`) | `2026-10-16-s8-final-batch-renode.log`, `0a806a8d52b575244eaf4d0bd1b2ffabbe9e4d3309385b8e4482899869db446a` |
| ASan full CTest run | PASS, 43/43 tests | `2026-10-16-s8-final-batch-asan-retry.log`, `8cc3420e271978a7065ea8456992876518d97a18a867b54a7c11d252a4a99f5f` |
| N7 generated flat source file list | 221 per-file SHA-256 records; digest reproduced | `2026-10-16-n7-generated-flat-file-hashes.txt`, `dd9666ad5cb09a9ffd71e7f3d17a86122f1dcc0211a53de9264d2ab9d9a8e822` |

## 3. Product-set file inventory for digest recomputation

| SHA-256 | Path |
| --- | --- |
| `db0dc51813920bab13ca55a870fb885155fb6edf63112d02ba2d1ad593596c2e` | `CMakeLists.txt` |
| `043f844906b747c3cb8cb609aacc21af93118e74bf125360d21cf2d9ee400fe9` | `Makefile` |
| `7cc340bfb5218550528fe33c4b2a06f40cc0e078c4a4f135072d5cf27c3bfd6b` | `docs/architecture/obcp-integration-assessment.md` |
| `335c58c236b550a1c69e712ea1c0055e6ab13d8f1ce0107320aad05a3c506ffe` | `docs/architecture/pus-layer.md` |
| `9faf4bda73be2dbae10b073f1b3923cab406441cd2eada6edd973b159b146a44` | `docs/requirements/traceability.md` |
| `199fa892a5cb29688455afc612c81439db89abd06332467e1a3b3c6d69668051` | `docs/requirements/traceability.yaml` |
| `f18d1f568889de32a43241a01072b5b39adf81b9fcca89ee44b869d34d1e20a1` | `docs/vv/test-spec/ts-1-protocol-core.md` |
| `d20eb203115cbbadc9cd7e604fb1f22276d483c809c8c945642e851f4400c480` | `docs/vv/test-spec/ts-6-sw-robustness.md` |
| `5bd02e7dff01ecda44fb8685073a9c82e40fe4fc961c9544e0acb1d495a8428e` | `docs/vv/test-spec/ts-9-obcp-integration.md` |
| `79310a426a0612d77940481114a73dd41671a9373c0272dbc1df75e8baa88fed` | `src/main/fsw_main.c` |
| `3169d3ba913890340305dff16d83fdc8fa00a94238ae57b12cb2e53ac2d1df11` | `src/obcp/ff_obcp.c` |
| `61caa0f8761596150db89af51ae544b373bb8158962fc2761b59d4598aa9cf69` | `src/obcp/ff_obcp.h` |
| `9967b489117bef3cbfd71fae79648ad20c8e3e9a370b2dd617ac5e5002da5c05` | `src/pus/adapters/pus_adapter_n7.c` |
| `580ea37164df5ef7e28eedc4f75b79fd7523ac32ff108b8beb22d1dbd244b749` | `src/pus/pus.c` |
| `1fa834dab74e7edcf2c985d3c1f2d62a662cbff78295cf40965a78c55fbc9aed` | `src/pus/pus.h` |
| `02c03566beae6ac90788421e669c820026a6f53df8990a24b637d9a57cb1e952` | `src/pus/pus_services.c` |
| `c0605afbd13a6c3a8a27a4d8398b41469bbcb4d274a853f1874b1d7983d78ab1` | `tests/CMakeLists.txt` |
| `fba54865ce562ac885adfeb5c762262dc6d0e3d682db204e58de2c93aa4f8187` | `tests/unit/test_decoder_corpus.c` |
| `7d31011a339d96e1abc46df7ea0ee04b18abb151110f36f69e7c34e3fd09c16d` | `tests/unit/test_obcp_integration.c` |
| `abcc33b13ed5bd7765ed5ebbe01ae749f152bd4fda5f11897f6e43ad53449408` | `tests/unit/test_pus.c` |
| `4347591b411a3f5c8baade6fb372b72d08f91bdf05437a047cb04e3a9b90fbb3` | `tests/unit/test_pus_fu.c` |
| `83dde356dbd47f2cfbd264cce88f1a1fde38c27a52d950c1a6a5e3d28dda6302` | `tests/unit/test_pus_n7_adapter.c` |
| `48c1e6d126645d97c8437e1c7f708ef4bc79a20a6951093d12f4b94a04225201` | `verification/lean/Frick/S4c.lean` |

These results are host, documentation, smoke, sanitizer, or emulator evidence as named.
Renode does not execute an OBCP procedure; none establishes physical-target
runtime, mission profile compliance, OBCP arbitrary-script bounds, production
service bindings, PUS S18 lifecycle, or mission-use suitability. The first ASan
attempt failed due root-owned stale build outputs; that failure is retained in
`2026-10-16-s8-final-batch-asan.log`; the corrected-permission retry passed as
listed. PA review FRICK-REVIEW-0345 accepted product-set identity for the 23
enumerated files and listed local outputs only; this is not an N20 phase/release
approval. NC-EXT-84 remains open for generator/tool provenance and does not imply
that observed generated files failed their hash check.
