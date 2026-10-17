# Current sprint-candidate product-set identity — 2026-10-16

| Field | Value |
| --- | --- |
| Candidate base | `b8c3aeec5484476407367024f3ce25c5af2bd30e` plus uncommitted working tree |
| Scope | S8-SCH/S8-VV-FU/S8-PUS-FU and selected N7 OBCP integration only; no full sprint/mission-use claim |
| Product-set digest | `1a844759a3857cca64ac51ddf63820887cab44881ed2e55b42cd8bb4c46fe8e4` |
| Inventory | 49 selected changed product/source/configuration/specification paths; see table below |
| Method | SHA-256 over sorted UTF-8 path + NUL + SHA-256(contents) + LF records; excludes status, planning, review, generated build output and evidence records |
| Claim limit | Product-set identity aid only; not a full repository snapshot, provenance record, qualification, phase acceptance or release authorization |

## Verification configuration

- `HEAD`: `b8c3aeec5484476407367024f3ce25c5af2bd30e`.
- `build/ci/CMakeCache.txt` SHA-256: `7f865d42506bf513a1057ceb4f57aafb2e7d87a61c5d58676f71f5cad49da7f6`.
- Host executable `build/ci/frick_fsw` SHA-256: `777cb93df70a6c9c9c25a49d07863906d8fea1f87a5810c58dc8a709ab598578`.
- Native Debug build; N7 ASN.1 backend, N7 OBCP, lwIP and tests enabled; exact options are recorded in CMakeCache.
- External N7 OBCP and MicroPython pins are listed in `third_party/README.md`; generated-code provenance NC-EXT-84 remains open.

## Selected product inventory

| SHA-256 | Path |
| --- | --- |
| `99e2273c9bec9d4b9a220ed46a0b0a5a8a657016b9e64454b969e54f3fc6b04c` | `.gitea/workflows/ci.yml` |
| `412681b3294ada43521df5789b786586fb6d21ac0c2d271d3eda6f469e82d6df` | `.github/workflows/ci.yml` |
| `c3c2e15e9b9790287b80aff3eabea70176f44a2c3acc3dd2de911f124c91eca7` | `.github/workflows/nightly.yml` |
| `1fd345b63f15abf507a9363459f948e438cedd68418da0da0bffbab0df9e9ee5` | `.gitignore` |
| `e2e94792b8a9310f2a321080f5d51fcdf5ac1eabd222e1d5599c021caaefb597` | `AGENTS.md` |
| `5b546fd7ef8f961e600a3d06cc6f13a331653572b2eb9b8f62294afca1eae7ca` | `CHANGELOG.md` |
| `db0dc51813920bab13ca55a870fb885155fb6edf63112d02ba2d1ad593596c2e` | `CMakeLists.txt` |
| `043f844906b747c3cb8cb609aacc21af93118e74bf125360d21cf2d9ee400fe9` | `Makefile` |
| `3fdcdd39da5116e891d481bec09f6353aed45a2df76e6db0dd594a307fc2c539` | `README.md` |
| `7671cc408cfb2c2541cc4a39827b90d87736653bd63459fdb324b06237aae379` | `docs/adr/ADR-0016-pus-c-asn1-default.md` |
| `c0517e893e2c1f435f606ebacfb09b0ff677a9bbe9835015435a771bf562d0af` | `docs/adr/ADR-0018-open-pus-and-lwip-defaults.md` |
| `700d3df2e533b88dcc5efc9f8849f4037f5e5f300fc1e7c11d772581beb9564a` | `docs/adr/ADR-0019-n7-pus-backend-default.md` |
| `e8e2559234e3c04b4d66f6496d54ce1cada58a01d4bddd564b84a201b242aec5` | `docs/architecture/component-gap-analysis.md` |
| `7cc340bfb5218550528fe33c4b2a06f40cc0e078c4a4f135072d5cf27c3bfd6b` | `docs/architecture/obcp-integration-assessment.md` |
| `deb2cd6b27f5ed0bc1018283c88eb3dc191b028d7abec68a1fa923bfbdf5dd28` | `docs/architecture/overview.md` |
| `335c58c236b550a1c69e712ea1c0055e6ab13d8f1ce0107320aad05a3c506ffe` | `docs/architecture/pus-layer.md` |
| `4c6b3e9dbcc58786f91175f5d5f4bab9df185ce7bb75fdeca362448d3007b4bd` | `docs/architecture/rtems-integration.md` |
| `86bc82c216a8bdff86a9f07d41745f56871e329ab96ccde929197d2c88023a32` | `docs/engineering/software-description.md` |
| `1a7e2d8ffd7357fb6fe4692e7fe3e00b18a75a61a6be06c7d4fe751eecff7378` | `docs/quality/class-a-gaps.md` |
| `8c746ca34cc32d814eb2d41e0a737b264de678ead7a4c4a91a605164ffdf3e54` | `docs/quality/licensing.md` |
| `18cdf4e81a3d07e96453fcba4c7bce836c4c6b812609e207533af9ab9f897070` | `docs/requirements/SRS.md` |
| `0dfc06481450a5d7aaea66bc19126266cfbe483dc82ed840e9759377361deb55` | `docs/requirements/SSS.md` |
| `9faf4bda73be2dbae10b073f1b3923cab406441cd2eada6edd973b159b146a44` | `docs/requirements/traceability.md` |
| `e692848b2a7ac818f2ac708557bac70b7036af6a021921760a812160d0ea1742` | `docs/requirements/traceability.yaml` |
| `519e7d593e99a5aae8c5d1b9f99def242615078556fd132092d47caaa203811b` | `docs/vv/test-spec/ts-1-protocol-core.md` |
| `f83c3198792b65a7d008c27fad408a25fc6c2b9d86089cc872a489bb7b516ea3` | `docs/vv/test-spec/ts-5-core-services.md` |
| `f119065d7599337032831e69d4194884040a8c8359a9e114be6b786cae0101e7` | `docs/vv/test-spec/ts-6-sw-robustness.md` |
| `5bd02e7dff01ecda44fb8685073a9c82e40fe4fc961c9544e0acb1d495a8428e` | `docs/vv/test-spec/ts-9-obcp-integration.md` |
| `79310a426a0612d77940481114a73dd41671a9373c0272dbc1df75e8baa88fed` | `src/main/fsw_main.c` |
| `3169d3ba913890340305dff16d83fdc8fa00a94238ae57b12cb2e53ac2d1df11` | `src/obcp/ff_obcp.c` |
| `61caa0f8761596150db89af51ae544b373bb8158962fc2761b59d4598aa9cf69` | `src/obcp/ff_obcp.h` |
| `9967b489117bef3cbfd71fae79648ad20c8e3e9a370b2dd617ac5e5002da5c05` | `src/pus/adapters/pus_adapter_n7.c` |
| `580ea37164df5ef7e28eedc4f75b79fd7523ac32ff108b8beb22d1dbd244b749` | `src/pus/pus.c` |
| `1fa834dab74e7edcf2c985d3c1f2d62a662cbff78295cf40965a78c55fbc9aed` | `src/pus/pus.h` |
| `3f91d54c65342d4ed9de374e8ed5dc9a0eeed682951cb506dbba1ee500cccab5` | `src/pus/pus_services.c` |
| `c0605afbd13a6c3a8a27a4d8398b41469bbcb4d274a853f1874b1d7983d78ab1` | `tests/CMakeLists.txt` |
| `fba54865ce562ac885adfeb5c762262dc6d0e3d682db204e58de2c93aa4f8187` | `tests/unit/test_decoder_corpus.c` |
| `7d31011a339d96e1abc46df7ea0ee04b18abb151110f36f69e7c34e3fd09c16d` | `tests/unit/test_obcp_integration.c` |
| `abcc33b13ed5bd7765ed5ebbe01ae749f152bd4fda5f11897f6e43ad53449408` | `tests/unit/test_pus.c` |
| `7637c2f870222097644344727063e59e754af1cd543e71ba2e31c18b630db689` | `tests/unit/test_pus_fu.c` |
| `83dde356dbd47f2cfbd264cce88f1a1fde38c27a52d950c1a6a5e3d28dda6302` | `tests/unit/test_pus_n7_adapter.c` |
| `f99a5480b3d351267af7da00518f785acff2f4606603c79ff9c878f5c696e44c` | `tests/unit/test_sched.c` |
| `3a5921f1458ab39a04e1edf44225fd71bffec5b5c2676ea3d3a7e4aa47ab462d` | `third_party/README.md` |
| `10c0ca6d30804879980abbb3ddfa4532aba1a722ce29a313ddadb64390447109` | `tools/scripts/ci_rtems_cross_build.sh` |
| `da65b6445ceb3738a9c8374e4e1547ff0381ed6a0623b9e0db150da1132a9571` | `tools/scripts/emulated_run.sh` |
| `d3408713bdb6faec2cbe247a93cdbe5bb350e380c09dfdaa47b67ecdf14190c9` | `tools/scripts/fetch_third_party.sh` |
| `b1fe665e5af3b1b9b8e6518fe5e413726d5ce2d156f1b3d0620f045550637cef` | `tools/scripts/gen_pus_asn1.sh` |
| `b1bf2ac5e424973a7a9acbcdfa48e5768f26fa71c3eca64807437446c74e4463` | `tools/scripts/run_sim.sh` |
| `48c1e6d126645d97c8437e1c7f708ef4bc79a20a6951093d12f4b94a04225201` | `verification/lean/Frick/S4c.lean` |

## Evidence boundaries

- Local verification result hashes are in `docs/vv/verification-results.md`.
- Evidence applies only to the listed source/configuration and named environments. Renode is emulation; RTEMS cross-build is not target runtime evidence.
- PA 0345 accepted an earlier 23-file product identity; it does not apply to this newly enumerated candidate. PA 0361 independently verified the 49 listed files and digest. Run-to-configuration linkage and generated-tool provenance remain limited/open; this is not release or phase approval.

## Current CI-local run linkage

- Command: `make ci-local`, latest run retained in
  `2026-10-16-s8-post-0366-final-ci-local.log` (SHA-256
  `9bd831d5fca6070b6373c088d2d4d7f552715dc9fd719b2efb4b281727b13d86`).
- The run reports build success, 43/43 CTest, Lean build success and trace PASS.
- Immediately after completion, observed hashes were `build/ci/CMakeCache.txt`
  SHA-256 `7f865d42506bf513a1057ceb4f57aafb2e7d87a61c5d58676f71f5cad49da7f6`
  and `build/ci/frick_fsw` SHA-256
  `777cb93df70a6c9c9c25a49d07863906d8fea1f87a5810c58dc8a709ab598578`.
- This is a post-run correlation to the same observed working tree/product-set
  digest, not a signed build attestation, independent reproducibility result, or
  complete compiler/tool provenance record.
