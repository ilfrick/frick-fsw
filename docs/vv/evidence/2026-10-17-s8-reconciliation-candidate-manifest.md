# S8 reconciliation candidate identity manifest (2026-10-18)

Refreshes the 2026-10-17 candidate inventory after the 2026-10-18 docs-alignment delta and bounded PA review. This identifies the committed candidate snapshot at HEAD `e7e48ee` before this manifest-only refresh.

- Pushed baseline HEAD: `b8c3aeec5484476407367024f3ce25c5af2bd30e`; candidate snapshot HEAD: `e7e48ee` (on `main`, synchronized to `origin/main` and `gitea/main`); 213 differing paths.
- Method: sorted union of `git diff --name-only <baseline> HEAD` and `git ls-files --others --exclude-standard`; no non-ignored untracked files were present. SHA-256 + byte size are recorded per path. Ignored `build/ci/` cache/executable artifacts remain separately linked below and are not counted in the 213-path Git delta.
- Combined digest over the sorted actual `<sha256>  <path>\n` lines for the pre-refresh snapshot: `434260016bae257da8ea25cb5add3292058d6ea6194d4d13aa24d44001f7d2b6`. The pre-refresh on-disk manifest hash used in that digest was `42374fbff510a611d569c4f5a2ab9c8335db5a9921fbf3d1c305995d03d4bf94`. The inventory row below uses the empty-file hash and size 0 by bootstrap convention; this is not a self-hash of the refreshed file.

Refreshed 2026-10-18 to satisfy action A-PA-2026-10-18-01. The earlier bounded PA result at HEAD `5be0e8c` verified 212 paths and digest `4e0f2544856beb0fdb7c59aebc73678f346a0ac0c03062342004d31f6317075e`; the only subsequent committed path is the PA report itself, giving the current 213-path snapshot. The 10 verification-run linkage hashes are preserved unchanged; `build/ci/` remains separately linked.
## Verification-run linkage

- Cycle log (build + 43/43 ctest + Lean 62 + trace PASS): `4cecaa75b86fa54236af6cb7bc32e37243b4976569ce2bc198e88401cb5d53f3`  (`docs/vv/evidence/2026-10-17-s8-reconciliation-ci-local.log`)
- Pinned format-check log (clang-format 18, docker): `c43cc487f79481072975f09f6becf98d6621c85167fa97da90b1c431c2dfc774`  (`docs/vv/evidence/2026-10-17-s8-reconciliation-format-check.log`)
- Doxygen docs log (docker): `f89ee26e06c0db0c012a375463b42db0b33718f0ab0061c58ef17e400459e57d`  (`docs/vv/evidence/2026-10-17-s8-reconciliation-docs.log`)
- GSW/metrics/license self-check log: `c9fbce7e1a7e2fa9b721cf161d156c46068ec767468005e518ec1f0da75292a9`  (`docs/vv/evidence/2026-10-17-s8-reconciliation-gsw-selfcheck.log`)
- Native simulator smoke log: `64b1a189c44a1ea3586e31609d63740ae4f5f415b00f63a03a3a6b7ab987b0cf`  (`docs/vv/evidence/2026-10-17-s8-reconciliation-smoke.log`)
- RTEMS final-tree cross-build log (N18): `1e1b232b312b3f98d097118864c23a069eedbaed5fb5c4141a8895dd6bab1d1a`  (`docs/vv/evidence/2026-10-17-s8-reconciliation-rtems-cross-build.log`)
- Renode final-tree functional validation log (N18): `0a806a8d52b575244eaf4d0bd1b2ffabbe9e4d3309385b8e4482899869db446a`  (`docs/vv/evidence/2026-10-17-s8-reconciliation-renode.log`)
- QEMU final-tree functional check log (M10): `da25f66cc7ae6d8a151322fbd8cbc5b5226b190bfdaae3742f4ca9503f7d1216`  (`docs/vv/evidence/2026-10-17-s8-reconciliation-qemu.log`)
- CMake cache of the cycle build tree: `7f865d42506bf513a1057ceb4f57aafb2e7d87a61c5d58676f71f5cad49da7f6`  (`build/ci/CMakeCache.txt`)
- Executable of the cycle build tree: `7ee78ef375b21cbae7355439679247a4eaa533368491a3cc8eaa82e267256f3d`  (`build/ci/frick_fsw`)

## Differing files (path, SHA-256, size)

| SHA-256 | Size | Path |
| --- | --- | --- |
| `99e2273c9bec9d4b9a220ed46a0b0a5a8a657016b9e64454b969e54f3fc6b04c` | 14610 | `.gitea/workflows/ci.yml` |
| `412681b3294ada43521df5789b786586fb6d21ac0c2d271d3eda6f469e82d6df` | 12151 | `.github/workflows/ci.yml` |
| `c3c2e15e9b9790287b80aff3eabea70176f44a2c3acc3dd2de911f124c91eca7` | 4533 | `.github/workflows/nightly.yml` |
| `1fd345b63f15abf507a9363459f948e438cedd68418da0da0bffbab0df9e9ee5` | 572 | `.gitignore` |
| `e2e94792b8a9310f2a321080f5d51fcdf5ac1eabd222e1d5599c021caaefb597` | 11753 | `AGENTS.md` |
| `80e4e7ef5e671caf44672125bd77b897ef91eebe694f16c1b8cec75e94c3aa35` | 103946 | `CHANGELOG.md` |
| `db0dc51813920bab13ca55a870fb885155fb6edf63112d02ba2d1ad593596c2e` | 15498 | `CMakeLists.txt` |
| `1c4a0501cdb23539906a04534b0fab8b2bafeafecc3add01ea6c8d1086ceee0f` | 8646 | `Makefile` |
| `59179111644f1f6513fcef7de8af1874bc47a870637a7c3500e390a3e25a9044` | 31597 | `README.md` |
| `7671cc408cfb2c2541cc4a39827b90d87736653bd63459fdb324b06237aae379` | 2981 | `docs/adr/ADR-0016-pus-c-asn1-default.md` |
| `c0517e893e2c1f435f606ebacfb09b0ff677a9bbe9835015435a771bf562d0af` | 1908 | `docs/adr/ADR-0018-open-pus-and-lwip-defaults.md` |
| `700d3df2e533b88dcc5efc9f8849f4037f5e5f300fc1e7c11d772581beb9564a` | 2255 | `docs/adr/ADR-0019-n7-pus-backend-default.md` |
| `76615ce9cae0c88ffa7cf4ac7a4879ed1ca0801f14e5eae5bbd35dc9a23cc1b3` | 5974 | `docs/adr/ADR-0020-provisional-pus-and-scheduler-policies.md` |
| `05ca7708dde299e71099ad69da5fcaf1cf3effb590d68db9ee979919e1343829` | 8877 | `docs/architecture/component-gap-analysis.md` |
| `7cc340bfb5218550528fe33c4b2a06f40cc0e078c4a4f135072d5cf27c3bfd6b` | 8469 | `docs/architecture/obcp-integration-assessment.md` |
| `deb2cd6b27f5ed0bc1018283c88eb3dc191b028d7abec68a1fa923bfbdf5dd28` | 6329 | `docs/architecture/overview.md` |
| `64630c933f13c0c2c09182a42586463a625e81c0d06dccc663c55e4910af9656` | 8680 | `docs/architecture/pus-layer.md` |
| `4c6b3e9dbcc58786f91175f5d5f4bab9df185ce7bb75fdeca362448d3007b4bd` | 5909 | `docs/architecture/rtems-integration.md` |
| `b28c58711d18062f40d563b8acbf6f0a93e1476a94df0829ca2a2d6d50f8bf34` | 14474 | `docs/architecture/scheduling.md` |
| `a8a55d1f43ebe1d90391355a9439c1a6b24b9f2f3cc9650b70e3f06fb24ec659` | 10971 | `docs/design/software-design.md` |
| `eb18e4ec87aa2db926dd446ca4a9205b1de2e4bbdf36c8f71e456344cca693df` | 8464 | `docs/engineering/service-development.md` |
| `86bc82c216a8bdff86a9f07d41745f56871e329ab96ccde929197d2c88023a32` | 41982 | `docs/engineering/software-description.md` |
| `23de5b185d8a06421b7146e982b3330895853c3927a817f39d117745b09ac808` | 27162 | `docs/engineering/user-integration-manual.md` |
| `aff8b36dc2ca1013fcb549f751f0d45a14fafb85e3395d0a4888e375a3057d68` | 15338 | `docs/interfaces/ICD.md` |
| `a6627fcbccd8ca326e78e34c1c990c1b92d840cdf63d36bdc46461388d8a646e` | 120757 | `docs/planning/sprint-plan.md` |
| `1a7e2d8ffd7357fb6fe4692e7fe3e00b18a75a61a6be06c7d4fe751eecff7378` | 13883 | `docs/quality/class-a-gaps.md` |
| `71518f7b5cc8607953dee9b0ef5376ea1d32bdbc36b80f89a024d764117bbef4` | 11583 | `docs/quality/licensing.md` |
| `3f457592b866a10a5107e0355f52b8f141ca01114957403ff03efa3af41ea31c` | 42678 | `docs/requirements/SRS.md` |
| `4341a69a148b7f03b55480ce1e322e55b9d86bdc9a0407d1a7fe268c93b1d3cd` | 15347 | `docs/requirements/SSS.md` |
| `9faf4bda73be2dbae10b073f1b3923cab406441cd2eada6edd973b159b146a44` | 31304 | `docs/requirements/traceability.md` |
| `e692848b2a7ac818f2ac708557bac70b7036af6a021921760a812160d0ea1742` | 56896 | `docs/requirements/traceability.yaml` |
| `487062d408d03c52191db2445d2a1feb826f1640e62cd190855ebb899943f416` | 1084 | `docs/reviews/2026-10-16-n7-generated-tree-pa-verification-0346.md` |
| `4cc1227ae24f10440d7defd896cdf12a4c122d4fa323bcf00ac589b09c023894` | 491 | `docs/reviews/2026-10-16-s8-audit-index-r4-check-0369.md` |
| `3a3ba3c3129411e7d4f8b25652f4d41032e4c941b07773772ea0710ebdfb0548` | 1727 | `docs/reviews/2026-10-16-s8-batch-pa-rereview-0345.md` |
| `d8c15252ec5c24ee76e92b575e264c408cd36f82258df97a6782fa0f7bddb0d8` | 1547 | `docs/reviews/2026-10-16-s8-batch-pa-review-0342.md` |
| `5981e410d4cb2040fd3e1da8c4ebdd7844d9562908f012708e792b05f9aab32b` | 1336 | `docs/reviews/2026-10-16-s8-batch-r4-rereview-0344.md` |
| `3d5db935dacfd5872aafb04692d9d6d6ed1f7bf7f1e2bfcc853a1a36c3cd77a0` | 1706 | `docs/reviews/2026-10-16-s8-batch-r4-review-0341.md` |
| `65c14c9dfadfd797cbb2c0964dd8b0d527e6110aaa2fda88e4eb41a7e3791178` | 1780 | `docs/reviews/2026-10-16-s8-batch-vv-review-0343.md` |
| `d20bedce4738d9794eaa8fea92d282f92e76ab5e3997e962ceb24e0490d45363` | 1652 | `docs/reviews/2026-10-16-s8-current-candidate-pa-identity-0361.md` |
| `22832f1aacb12d9ae172385574587a0f0dbc6d84690f0f9a3277f633267727dc` | 650 | `docs/reviews/2026-10-16-s8-current-docs-r4-rereview-0363.md` |
| `eeff0e9ed702b2729dc9203b732461652bbb538caeb52349a5f4199fd9980be5` | 847 | `docs/reviews/2026-10-16-s8-current-docs-r4-review-0362.md` |
| `e4f81e5000697028c1f13a83b4829563d19f7edcb22b2d3c15b37853cb0eed59` | 596 | `docs/reviews/2026-10-16-s8-currentness-r4-confirmation-0348.md` |
| `7d6af7d9fdfc1ec58379d4db50dac0bec5723a5753bf4fe1ca2cec0e7184fc84` | 1008 | `docs/reviews/2026-10-16-s8-currentness-r4-rereview-0347.md` |
| `a4846f8ecfc9762c53dda49d3d3333933e42da50f7820c6d9cf9992396d1e8b5` | 1326 | `docs/reviews/2026-10-16-s8-docs-r4-review-0357.md` |
| `91fa3cfe649b0cbae12f82c03e4c99c253b229c63f9c61116ed407e125cd4848` | 938 | `docs/reviews/2026-10-16-s8-final-ci-pa-rereview-0364.md` |
| `74d2e9537a492c353545113a6a0022ad32d1e62f6315d39506f7a3962ec4f1ae` | 636 | `docs/reviews/2026-10-16-s8-final-reference-r4-rereview.md` |
| `e8aa769dad35105fe5738456bd0481c112cf3b7b2dc4abcb325043c8ff7c2794` | 548 | `docs/reviews/2026-10-16-s8-handover-ci-hash-r4-rereview-0366.md` |
| `c913db3760cd5d7acab3478f6fcf4c1ba465895418339d9ce0a13ba91d1b0f75` | 615 | `docs/reviews/2026-10-16-s8-handover-ci-hash-r4-review-0365.md` |
| `48efa707cb2c5b051e1fa5832f10d8595a2598d3792eaa30d1d3e498e55e3ecc` | 524 | `docs/reviews/2026-10-16-s8-handover-nextsteps-r4-rereview-0368.md` |
| `8abe2d94933d5427ea59969078c211beaa4dd5bd3cd16940d1a8af329e071771` | 742 | `docs/reviews/2026-10-16-s8-handover-nextsteps-r4-review-0367.md` |
| `774a8af9fb21f783b3232c4ac1d118a874568d856f988c0296bee2a4a4957b6a` | 679 | `docs/reviews/2026-10-16-s8-loop-audit-r4-rereview.md` |
| `8248369ede4b0ab295a8775ec8848ee02e34875226a872e1ce99ab5094c136be` | 4250 | `docs/reviews/2026-10-16-s8-provisional-pus-scheduler-delta-assessment.md` |
| `d67eb4b134ee66c6267207dbcceae9201f5b2521b7a05a038b96301d4ebdc430` | 1984 | `docs/reviews/2026-10-16-s8-pus-obcp-pa-review-0339.md` |
| `b40187ce51c63b3ab933873fe9cb131fd43b0041038560e0996716dde4f25785` | 1578 | `docs/reviews/2026-10-16-s8-pus-obcp-r4-review-0338.md` |
| `2950775cbcdb5c72035d7fc8e48cce3c7f27ed0c941b3dbd85dcc48a138bd717` | 2072 | `docs/reviews/2026-10-16-s8-pus-obcp-vv-review-0340.md` |
| `bf44c31f555476792160d7009a95dd01d5d150f345e2385a2303f634b8442af4` | 1141 | `docs/reviews/2026-10-16-s8-pus-pa-delta-0354.md` |
| `454e1bdbacad74afdaff9aa6b6d62ff5d10690d1c4e750a872873b9d10d25b8b` | 729 | `docs/reviews/2026-10-16-s8-pus-r1-rereview-0355.md` |
| `4ee4c74e78b31b01ac91670ba22014fb7e2857a3e08d0a61d35d86e051bef528` | 1140 | `docs/reviews/2026-10-16-s8-pus-r1-review-0349.md` |
| `f042cbba073240eb06d59af0a8ea6b3b7f55882e6dde60bcb0387936ca9386b2` | 820 | `docs/reviews/2026-10-16-s8-pus-r2-rereview-0356.md` |
| `78daa4707d10393811ffeaaf4a89eb5f5a51566cd5cade65564610e5a7ecb8ec` | 1494 | `docs/reviews/2026-10-16-s8-pus-r2-review-0350.md` |
| `4f84adc86f6855a185395ed45647f7db1fe73dff823ab1b5c361c9af9dcf5c50` | 1641 | `docs/reviews/2026-10-16-s8-pus-scheduler-delta-pa-identity.md` |
| `6c6b74a60b8ccce004bf55d01a4deee39eaf49a4913d1a74696c255e50893d09` | 1570 | `docs/reviews/2026-10-16-s8-sched-pa-delta-0360.md` |
| `26369c48fc8edab75490ca22a46714ec84af36e15083f9eacbad2a15828441e4` | 1206 | `docs/reviews/2026-10-16-s8-sched-r1-review-0351.md` |
| `39ee6ef83412e8951f94cd8bf49cb75f4c14f40429e3eab211336ad642b1b450` | 1351 | `docs/reviews/2026-10-16-s8-sched-r2-rereview-0353.md` |
| `242ec485cb69ca6fba7c9c9798d7945a8bffd7ebc93e41d75ec428063290d41b` | 1131 | `docs/reviews/2026-10-16-s8-sched-r2-review-0352.md` |
| `6879fdd4f5d8474928b6b7ecc8f7b8bfe9033570b48460f688a6b217713463ce` | 487 | `docs/reviews/2026-10-16-s8-ts1-history-r4-review-0359.md` |
| `c76f6953ee22b540193f335fa32ab543dc8fee71b47a2fe04aef7c68c4f5250e` | 882 | `docs/reviews/2026-10-16-s8-ts1-r4-rereview-0358.md` |
| `fb72dc90613527b1acd7b3e027316bc86548f28e37882073acf10d583c237819` | 15076 | `docs/reviews/2026-10-17-s8-current-docs-r4-0372.md` |
| `ed529fc3e0a7ddecbb407c3fa5400c96ec9f1012f3ce715385b1cab5f6f13931` | 24187 | `docs/reviews/2026-10-17-s8-icd-1-3-delta-r3-0375.md` |
| `b2158199067d69c710fddf41e19ba9e3331f1a57ef2fc484bed747df379312cd` | 19939 | `docs/reviews/2026-10-17-s8-reconciliation-docs-r4-rereview-0376.md` |
| `6279919a63ecb873de6ef82b090bd6a4e9d4a45544bed98505d1e17fd13234f1` | 29511 | `docs/reviews/2026-10-17-s8-reconciliation-pa-0373.md` |
| `ee8732796eeb47f6eb6b84dfc2928c3b4e007c0960fee8fa540f7536ca77331e` | 37310 | `docs/reviews/2026-10-17-s8-reconciliation-pa-final-0377.md` |
| `65aa8af25ff72120f61b41db26ed99b1d5da464c20753f7af32cee5b489f7721` | 15282 | `docs/reviews/2026-10-17-s8-requirement-grounding-r3-0371.md` |
| `28a4f859c520e8aa3cdeb5f771bcd956efdea6e47c0fe055ee8f5669ab343810` | 30536 | `docs/reviews/2026-10-17-s8-ts5-77-oracle-r2-0374.md` |
| `daa46319766a299dc022627ed1a860aa621ac0175f4b8f9ba177a435359ff4ed` | 18118 | `docs/reviews/2026-10-17-s8-vv-pus-delta-0370.md` |
| `6c69b60b0642b5fe2f9347a026c6cb29cb7761281f23db92c4b8090905bd8b44` | 12450 | `docs/reviews/2026-10-18-docs-implementation-alignment.md` |
| `34f211789d893305498d73e102bb9bf64b3a4a8ceb5c857982763b9077700d6d` | 2104 | `docs/reviews/2026-10-18-post-docs-alignment-manifest-pa.md` |
| `36fbe7de212b956589609b1dc444f539fe118fe6725f75139f15fe55f98d7ec6` | 87055 | `docs/reviews/README.md` |
| `e0e4ac3fde0eeced7db0a5ce03bb0cf8a0eaf1a21706adb1bb32eddf3ea64dc7` | 85125 | `docs/status/handover.md` |
| `aa5e2f168b355ba776d3b8b7e7fc4938ab4e7e263cd097b133b938a7dd3fae8d` | 78200 | `docs/status/loop-audit.md` |
| `e80a31abe5342b45b8a1e466b2bd09812dcd328ad518a61f03746ce78dd66459` | 4191 | `docs/status/status.json` |
| `dd9666ad5cb09a9ffd71e7f3d17a86122f1dcc0211a53de9264d2ab9d9a8e822` | 22180 | `docs/vv/evidence/2026-10-16-n7-generated-flat-file-hashes.txt` |
| `dc6092966cbeb76e3738c4c1824f7a0f8741510df3e8a91b9c96855b79b04bd9` | 7449 | `docs/vv/evidence/2026-10-16-s8-batch-candidate-manifest.md` |
| `41f95e7a1e9118c3e10cd15513358af1a31b7c9692dfced8c38260ca031fc60e` | 26085 | `docs/vv/evidence/2026-10-16-s8-batch-reviewed-ci-local.log` |
| `ba94445586b20306fcaa728a79380cf3a035fd3edf931925755e02fff20e7822` | 7501 | `docs/vv/evidence/2026-10-16-s8-current-asan.log` |
| `1e5b9d3c2afaa52316480d3404187c1c69b45f0e7618dd3f5ccd36a602906754` | 7811 | `docs/vv/evidence/2026-10-16-s8-current-candidate-manifest.md` |
| `f3fcf299d075207065dfdb78d7f11fb3a91dcddc376fbcdfa41e908b8682eb32` | 175070 | `docs/vv/evidence/2026-10-16-s8-current-docs.log` |
| `eb8008a47fec33c92de4972d5375ca2ed4ef52c5302154119c9a24d05456afd5` | 25849 | `docs/vv/evidence/2026-10-16-s8-current-final-ci-local.log` |
| `8afa247add8acd71605a0b2231545faff34968170ec87ceed09d787a56bddfb9` | 281 | `docs/vv/evidence/2026-10-16-s8-current-format-host-failure.log` |
| `c43cc487f79481072975f09f6becf98d6621c85167fa97da90b1c431c2dfc774` | 92 | `docs/vv/evidence/2026-10-16-s8-current-format.log` |
| `258c12f04b8483e30474e25564f1caa69a65db605106fa0f6ceb9436fdfe5138` | 684 | `docs/vv/evidence/2026-10-16-s8-current-gsw.log` |
| `a11db1e08698c45bb779d462222193a25278b7acd601ceb49ce03a011ab22f0a` | 25854 | `docs/vv/evidence/2026-10-16-s8-current-identity-ci-local.log` |
| `9d010ed81ccfa093691dd8d58772d47b36060932f0d6bdcf0bdb385b89833f4a` | 25859 | `docs/vv/evidence/2026-10-16-s8-current-post-r4-ci-local.log` |
| `f0514bed3605e5452af539923f92d426bc09b2b349b51ea2ecd7175735b9f39a` | 25849 | `docs/vv/evidence/2026-10-16-s8-current-post-r4-final-ci-local.log` |
| `0a806a8d52b575244eaf4d0bd1b2ffabbe9e4d3309385b8e4482899869db446a` | 66 | `docs/vv/evidence/2026-10-16-s8-current-renode.log` |
| `4eaba96ba9c13402bb4331c4b712b7786ab1a470106a2cef99b91fbad13ded59` | 16089 | `docs/vv/evidence/2026-10-16-s8-current-rtems-cross-build.log` |
| `17de9f9a6301dd8960a54286269c344c855a493c4a18ea49c111a9ac48fbdf97` | 287 | `docs/vv/evidence/2026-10-16-s8-current-smoke-gsw-format-docs.log` |
| `64b1a189c44a1ea3586e31609d63740ae4f5f415b00f63a03a3a6b7ab987b0cf` | 2594 | `docs/vv/evidence/2026-10-16-s8-current-smoke.log` |
| `8cc3420e271978a7065ea8456992876518d97a18a867b54a7c11d252a4a99f5f` | 23769 | `docs/vv/evidence/2026-10-16-s8-final-batch-asan-retry.log` |
| `cfa1d6be21d3e9cb0a59b6a44fe8334864af75a0a6b3878c50bcd887a797dd9a` | 27146 | `docs/vv/evidence/2026-10-16-s8-final-batch-asan.log` |
| `8c416fa97704672841be8f63a6de3154d0baec2eafc1bbf97b9da0db3e102ee4` | 25849 | `docs/vv/evidence/2026-10-16-s8-final-batch-ci-local.log` |
| `888b6bf10d014c6a26273c74726ab31df03022b5187d65d0524caa77ead2d3e4` | 170737 | `docs/vv/evidence/2026-10-16-s8-final-batch-docs.log` |
| `b975d14f95e85454f57bc93482327de7b063b5306dda5ffdd10c7c01bb86660e` | 170737 | `docs/vv/evidence/2026-10-16-s8-final-batch-docs2.log` |
| `c43cc487f79481072975f09f6becf98d6621c85167fa97da90b1c431c2dfc774` | 92 | `docs/vv/evidence/2026-10-16-s8-final-batch-format.log` |
| `ccb5018d23ad64371a3296cea066cdbf1f9349d5f16df9c02fb7d631ab5b9554` | 685 | `docs/vv/evidence/2026-10-16-s8-final-batch-gsw.log` |
| `0a806a8d52b575244eaf4d0bd1b2ffabbe9e4d3309385b8e4482899869db446a` | 66 | `docs/vv/evidence/2026-10-16-s8-final-batch-renode.log` |
| `64b1a189c44a1ea3586e31609d63740ae4f5f415b00f63a03a3a6b7ab987b0cf` | 2594 | `docs/vv/evidence/2026-10-16-s8-final-batch-smoke.log` |
| `cd0572e3a5cbb812087712982dd095c4ea565c9208c47f48be1b16490f016ae8` | 26262 | `docs/vv/evidence/2026-10-16-s8-final-format-ci-local.log` |
| `742aed235c713e8f548c7b48197d04f43fe332341a70a92b0a88782fab0f1b2a` | 179763 | `docs/vv/evidence/2026-10-16-s8-final-format-docs.log` |
| `bcecddfc889bc3c0878db6ea29b41d98d962b550df92ff5aff5cb1b802bcdf83` | 25854 | `docs/vv/evidence/2026-10-16-s8-final-loop-audit-ci-local.log` |
| `96adac3903e268e07e27a9e76d1d9c2961adf187731e86b2d5ad323f6d86efd4` | 25854 | `docs/vv/evidence/2026-10-16-s8-final-reference-ci-local.log` |
| `be5721a4f5b5ab0fece1e98865d1a952f58bf7db932347139ce76cdabece0de8` | 25854 | `docs/vv/evidence/2026-10-16-s8-final-status-delta-ci-local.log` |
| `8e4a7556570c4a05e3670a16256c439033d7140064bf06384f30d2ab2eab0f7d` | 172008 | `docs/vv/evidence/2026-10-16-s8-final-status-delta-docs.log` |
| `9bd831d5fca6070b6373c088d2d4d7f552715dc9fd719b2efb4b281727b13d86` | 25864 | `docs/vv/evidence/2026-10-16-s8-post-0366-final-ci-local.log` |
| `d7d16c565148f28dc5b55c9f9e85f732c819bf0d3789a8ca499e2bcff8a88bef` | 4605 | `docs/vv/evidence/2026-10-16-s8-pus-fu-asan.log` |
| `9445f337ea4d2b7912e0d181d161ed75bbe6683640305d08843d98821f533820` | 168884 | `docs/vv/evidence/2026-10-16-s8-pus-fu-aux.log` |
| `03d4b227c4dbb54bfdebcdc6fd39d01fdc248095cc9f3d90b584ab7944b4fb18` | 8546 | `docs/vv/evidence/2026-10-16-s8-pus-fu-ci-local.log` |
| `dfbcb9cdb069cca0316407ed7a37e46884a9d8a92b799ece996ec5d17be72981` | 5237 | `docs/vv/evidence/2026-10-16-s8-pus-fu-coverage.log` |
| `2d9a866080c940829831a000442ebfaaf17d6faacbf953100601419d05e485c3` | 24140 | `docs/vv/evidence/2026-10-16-s8-pus-fu-default-n7-asan.log` |
| `0cc160226785c8c95cd2dc2b29eb3791ca3a0e1b5831e4ae956c923c20e98091` | 253492 | `docs/vv/evidence/2026-10-16-s8-pus-fu-default-n7-aux.log` |
| `69d091afc3874c88b5d88dcb95e6066798724c156eed45c3458757932e26fa9c` | 13690 | `docs/vv/evidence/2026-10-16-s8-pus-fu-default-n7-ci-local-final.log` |
| `a5d9f67ff0b037ff2f8b710fafb05026f01eaddcb37d01d0daa07f18aa1d0c0c` | 30308 | `docs/vv/evidence/2026-10-16-s8-pus-fu-default-n7-ci-local.log` |
| `b5c0e8c7accb6f49a3c24f1ccdae4ad05a8d66639f9c009592458a634d6da73c` | 3123 | `docs/vv/evidence/2026-10-16-s8-pus-fu-default-n7-coverage-stale-cache-failure.log` |
| `82980333b6fbdd63fac0f92dc4f93db7d368e5aa3706642a6388874404469c0b` | 24302 | `docs/vv/evidence/2026-10-16-s8-pus-fu-default-n7-coverage.log` |
| `306780099bd2e6886e27c6a2fc892d649a9c78e8f640186c18895481fd1f7c02` | 168094 | `docs/vv/evidence/2026-10-16-s8-pus-fu-default-n7-docs-final.log` |
| `c40b3e4a80b67f45599b540f1852d27303ff2e610abea67dc7090bee7bea28f8` | 168711 | `docs/vv/evidence/2026-10-16-s8-pus-fu-default-n7-docs-format.log` |
| `c43cc487f79481072975f09f6becf98d6621c85167fa97da90b1c431c2dfc774` | 92 | `docs/vv/evidence/2026-10-16-s8-pus-fu-default-n7-format-final.log` |
| `b5c0e8c7accb6f49a3c24f1ccdae4ad05a8d66639f9c009592458a634d6da73c` | 3123 | `docs/vv/evidence/2026-10-16-s8-pus-fu-default-n7-metrics-stale-cache-failure.log` |
| `3b35f04e705c67b3c46452113bfa7e9f1c4c4282a2ccd6fb214b039f318360b3` | 14515 | `docs/vv/evidence/2026-10-16-s8-pus-fu-default-n7-renode-docker-build.log` |
| `0a806a8d52b575244eaf4d0bd1b2ffabbe9e4d3309385b8e4482899869db446a` | 66 | `docs/vv/evidence/2026-10-16-s8-pus-fu-default-n7-renode.log` |
| `562a7a879477b84c0ce4bdc22f219e665c7a42b655f843ac8f01fe39f9c7fab1` | 1894 | `docs/vv/evidence/2026-10-16-s8-pus-fu-default-n7-trace-final.log` |
| `23ad5206d2bfaa498d6495990e87892f6bfafad91a952ccda6df08b06d52b94e` | 1301 | `docs/vv/evidence/2026-10-16-s8-pus-fu-failed-checks.log` |
| `c43cc487f79481072975f09f6becf98d6621c85167fa97da90b1c431c2dfc774` | 92 | `docs/vv/evidence/2026-10-16-s8-pus-fu-format-check.log` |
| `c99aa65f6070aa4def1097b746d2cbd2840a1d21514c5d3dc98ccf7aa79eb737` | 23564 | `docs/vv/evidence/2026-10-16-s8-pus-fu-open-backend-explicit-check.log` |
| `39eb999e790e2e4083bb04edd0bfde012974cc559cf9dc0913af843af9390617` | 927 | `docs/vv/evidence/2026-10-16-s8-pus-fu-renode-stale-cache-failure.log` |
| `630d05f802bc78a99d64b4c9a136d666c99028636372b9858af58d481fcb3a4f` | 14153 | `docs/vv/evidence/2026-10-16-s8-pus-fu-stale-cache-attempt.log` |
| `562a7a879477b84c0ce4bdc22f219e665c7a42b655f843ac8f01fe39f9c7fab1` | 1894 | `docs/vv/evidence/2026-10-16-s8-pus-fu-trace.log` |
| `0b207eadc611f0003d99aef0c8da8dca2e900f13bd19fafe35a9b5b637b3a998` | 25849 | `docs/vv/evidence/2026-10-16-s8-pus-obcp-review-corrected-ci-local.log` |
| `b651988c4a9ad12a7c1c0ef38a1ea4ce1577553a809bba9f5ee67b3223f91c8f` | 237003 | `docs/vv/evidence/2026-10-16-s8-pus-obcp-review-corrected-docs.log` |
| `3423e5ec21cd5010f106a9d41a7a70470cd60155ffbcce814c9d92ba5eb915dc` | 684 | `docs/vv/evidence/2026-10-16-s8-pus-obcp-review-corrected-gsw.log` |
| `64b1a189c44a1ea3586e31609d63740ae4f5f415b00f63a03a3a6b7ab987b0cf` | 2594 | `docs/vv/evidence/2026-10-16-s8-pus-obcp-review-corrected-smoke.log` |
| `55b4420a729a1b5a0a2619c951a5ac339325dab30c379fc72343b6a5ababe0c0` | 2621 | `docs/vv/evidence/2026-10-16-s8-pus-r1-correction-focused-retry.log` |
| `d4c76d073515e6ad1ad8da40ba8f9bb6bd34a836c4661f63d2d396603ad38e85` | 427 | `docs/vv/evidence/2026-10-16-s8-pus-r1-correction-focused-retry2.log` |
| `8bf4eaba671a6a0077e688c452f4f8448d4ecf4a704ba0335065a6836f2cb6df` | 2309 | `docs/vv/evidence/2026-10-16-s8-pus-r1-correction-focused.log` |
| `f4e3ae5d8d5b81ce0ee8af254938e22126ebdc67bb6c610a8a259b0b67f36260` | 25854 | `docs/vv/evidence/2026-10-16-s8-pus-r1r2-delta-ci-local.log` |
| `5b290949588b77d9e7600ddaeb56e834326901f69d49ca6358c3b697480306ca` | 3552 | `docs/vv/evidence/2026-10-16-s8-pus-sched-aux-host-format-environment-failure.log` |
| `c7f3deab0bfc6983e1f6b7d28b1d929988aacf00effb355943d13da513a44e68` | 25854 | `docs/vv/evidence/2026-10-16-s8-pus-sched-corrected-ci-local.log` |
| `0aa439a9c1a11b53ce7a53a565ffb68ac1cbf843a30968348fe6ace793fc79fb` | 195525 | `docs/vv/evidence/2026-10-16-s8-pus-sched-docs-renode-rtems.log` |
| `fbd7253dd0f71744d2aa93fb5d110df8436903c1ca36e6aaefbb564b63c3abc2` | 3462 | `docs/vv/evidence/2026-10-16-s8-pus-sched-pinned-format-failure.log` |
| `61944010212058025babd544c33c1b40af5c42e392bf11eb2eded1ec8e9b82d4` | 26667 | `docs/vv/evidence/2026-10-16-s8-pus-sched-stale-test-failure-ci-local.log` |
| `2165b478d6413d9e18272d5547c667d65d4eae2ae0c9b8cc8767221bdcd4c2bc` | 3965 | `docs/vv/evidence/2026-10-16-s8-pus-scheduler-delta-identity.md` |
| `b85e91de93abc7bd0826305e632b2e53c8cd990ba9a9a97794e30dcb3d44d7b4` | 25854 | `docs/vv/evidence/2026-10-16-s8-s8-sched-pus-current-ci-local.log` |
| `07e9f5a75740adc7d32fe1ee887cf396f1879450a0c40df9e371f65f4683d7ed` | 423 | `docs/vv/evidence/2026-10-16-s8-sched-r2-correction-focused-retry.log` |
| `337d4dea131f70361fc3560498e771b7555272aee0a5f4476809f723533f585c` | 127 | `docs/vv/evidence/2026-10-16-s8-sched-r2-correction-focused.log` |
| `d4160b4f5b0c3b836b9feff07a8eed5465fe7b0228a99c93c149ae2eb7d4e5dd` | 168321 | `docs/vv/evidence/2026-10-16-s9-docs-final-final.log` |
| `2807228aaf5fd3e71f2376ff93569d226f9249593acb0c384de7ee0882e3c167` | 168321 | `docs/vv/evidence/2026-10-16-s9-docs-final.log` |
| `1a922ab4199f562c609add6a47ba8f0c0e53f049e674d9e1a66c7a6acbcfb223` | 168321 | `docs/vv/evidence/2026-10-16-s9-docs.log` |
| `e9c34fec844941c4f593f3f10e2d51d6eb486635407045101f624ac6ecc482ca` | 424013 | `docs/vv/evidence/2026-10-16-s9-n7s-obcp-upstream-host-tests-retry.log` |
| `56f5b255f455e3c6067c92f9555cde5bc6bff4a29d1a13658e26c9bc9ae0bfb0` | 450 | `docs/vv/evidence/2026-10-16-s9-n7s-obcp-upstream-host-tests.log` |
| `2865bdf5f372f22d20d77e1f18146a6b6522365aee674edf8c1fea1e7789ab0f` | 42645 | `docs/vv/evidence/2026-10-16-s9-obcp-ci-local.log` |
| `4ba5adc2f62dbab928d050e8d4a8d76c2188f0813049593cab690edee49b0286` | 1429 | `docs/vv/evidence/2026-10-16-s9-obcp-connectivity-oracle-failure.log` |
| `9fcf24a2151815084c3b0463a9254bf1f78c7670a1ce4ba8bf0dedd408078e11` | 169312 | `docs/vv/evidence/2026-10-16-s9-obcp-docs.log` |
| `02121122b17fb51e197653379f28c6c78587896cc7e8c4910255157922d9aedd` | 65 | `docs/vv/evidence/2026-10-16-s9-obcp-fetch.log` |
| `c43cc487f79481072975f09f6becf98d6621c85167fa97da90b1c431c2dfc774` | 92 | `docs/vv/evidence/2026-10-16-s9-obcp-format.log` |
| `8c33af03e8d70db2ae8158ad3ecc87103c006f92707eb2f00492129330a15a0b` | 684 | `docs/vv/evidence/2026-10-16-s9-obcp-gsw-selfcheck.log` |
| `8e267f18cb62525513035d01453770d240457da351041cd211a8bb419014c3ed` | 26181 | `docs/vv/evidence/2026-10-16-s9-obcp-renode-build.log` |
| `0a806a8d52b575244eaf4d0bd1b2ffabbe9e4d3309385b8e4482899869db446a` | 66 | `docs/vv/evidence/2026-10-16-s9-obcp-renode.log` |
| `b6e62524946c7b57f02768d49fab17fde2d10580cee1a68a3181d4e85a41db2a` | 29057 | `docs/vv/evidence/2026-10-16-s9-obcp-rtems-cross-build.log` |
| `64b1a189c44a1ea3586e31609d63740ae4f5f415b00f63a03a3a6b7ab987b0cf` | 2594 | `docs/vv/evidence/2026-10-16-s9-obcp-smoke.log` |
| `74277cb2c34dddc7c2e8f8529640268bf58dcb7a547058e5b25b8c2fa0d16cde` | 13515 | `docs/vv/evidence/2026-10-16-s9-s8-candidate-ci-local.log` |
| `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` | 0 | `docs/vv/evidence/2026-10-17-s8-reconciliation-candidate-manifest.md` |
| `4cecaa75b86fa54236af6cb7bc32e37243b4976569ce2bc198e88401cb5d53f3` | 26136 | `docs/vv/evidence/2026-10-17-s8-reconciliation-ci-local.log` |
| `f89ee26e06c0db0c012a375463b42db0b33718f0ab0061c58ef17e400459e57d` | 180627 | `docs/vv/evidence/2026-10-17-s8-reconciliation-docs.log` |
| `c43cc487f79481072975f09f6becf98d6621c85167fa97da90b1c431c2dfc774` | 92 | `docs/vv/evidence/2026-10-17-s8-reconciliation-format-check.log` |
| `c9fbce7e1a7e2fa9b721cf161d156c46068ec767468005e518ec1f0da75292a9` | 684 | `docs/vv/evidence/2026-10-17-s8-reconciliation-gsw-selfcheck.log` |
| `da25f66cc7ae6d8a151322fbd8cbc5b5226b190bfdaae3742f4ca9503f7d1216` | 155 | `docs/vv/evidence/2026-10-17-s8-reconciliation-qemu.log` |
| `0a806a8d52b575244eaf4d0bd1b2ffabbe9e4d3309385b8e4482899869db446a` | 66 | `docs/vv/evidence/2026-10-17-s8-reconciliation-renode.log` |
| `1e1b232b312b3f98d097118864c23a069eedbaed5fb5c4141a8895dd6bab1d1a` | 15903 | `docs/vv/evidence/2026-10-17-s8-reconciliation-rtems-cross-build.log` |
| `64b1a189c44a1ea3586e31609d63740ae4f5f415b00f63a03a3a6b7ab987b0cf` | 2594 | `docs/vv/evidence/2026-10-17-s8-reconciliation-smoke.log` |
| `c21979e28430ea3a8e87b157592e17ee59a2c2b7e594ee9c621c39bbbf450d44` | 61668 | `docs/vv/test-spec/ts-1-protocol-core.md` |
| `b95d5730730b1e2bbe5bec3163354d07823d075160bfe9199e516b4dffcd9941` | 65297 | `docs/vv/test-spec/ts-5-core-services.md` |
| `d3e897cd5def7ed3e3fbf28003de8117c394edcf8c1382c461fe1fcf2626258f` | 59559 | `docs/vv/test-spec/ts-6-sw-robustness.md` |
| `3850b2a4f6045273497931536c38bb91ab60bd5a660f48113df4a5cc07427dd7` | 4200 | `docs/vv/test-spec/ts-9-obcp-integration.md` |
| `6872f88e79609acdbfd1dc88ba0058b9208706e170e9b9841ad65b72fb9100e7` | 36978 | `docs/vv/tool-qualification.md` |
| `32cd5646bdd7db6b590e82186f9b98760dd3939a64188cd1112bf5bfc24d1529` | 13115 | `docs/vv/verification-plan.md` |
| `8c4ad335e357c0e3086f6e890cd60db0bcca75c899dc356431a2de8b807e4988` | 190751 | `docs/vv/verification-results.md` |
| `e68e37772258cabc025156decb77c6509110bd34cdc37fe1d6da16b07cf3b860` | 6306 | `src/core/sched/ff_sched.c` |
| `2cae84b0bb0050a855023f1c5caae341a4d2279ffcd4ab5add33736cfa93cd6c` | 4737 | `src/core/sched/ff_sched.h` |
| `79310a426a0612d77940481114a73dd41671a9373c0272dbc1df75e8baa88fed` | 4408 | `src/main/fsw_main.c` |
| `3169d3ba913890340305dff16d83fdc8fa00a94238ae57b12cb2e53ac2d1df11` | 6263 | `src/obcp/ff_obcp.c` |
| `61caa0f8761596150db89af51ae544b373bb8158962fc2761b59d4598aa9cf69` | 1562 | `src/obcp/ff_obcp.h` |
| `9967b489117bef3cbfd71fae79648ad20c8e3e9a370b2dd617ac5e5002da5c05` | 4682 | `src/pus/adapters/pus_adapter_n7.c` |
| `580ea37164df5ef7e28eedc4f75b79fd7523ac32ff108b8beb22d1dbd244b749` | 7846 | `src/pus/pus.c` |
| `ec4d04782c1911caf1abbf905614dd14b62b3f03c1ea395b42bb166f764cd351` | 4201 | `src/pus/pus.h` |
| `7417bcc56403b30ce4a300102822fb7b1d90daa0facf89dac5a923206b9228bb` | 17362 | `src/pus/pus_services.c` |
| `c0605afbd13a6c3a8a27a4d8398b41469bbcb4d274a853f1874b1d7983d78ab1` | 8473 | `tests/CMakeLists.txt` |
| `fba54865ce562ac885adfeb5c762262dc6d0e3d682db204e58de2c93aa4f8187` | 25752 | `tests/unit/test_decoder_corpus.c` |
| `060256cd8b05bbf1cea98cdaea579f931437a152bd7708088fd33aaa58313770` | 12265 | `tests/unit/test_es_hs_lifecycle.c` |
| `7d31011a339d96e1abc46df7ea0ee04b18abb151110f36f69e7c34e3fd09c16d` | 8252 | `tests/unit/test_obcp_integration.c` |
| `22a2f19fec2ff2417f14a349670d727419bbbca00ff70a28225a51b77b29b9f2` | 12089 | `tests/unit/test_pus.c` |
| `eaf00e51c5476cadb7610bd574b01b20c4997e442c58fc76842b010ecfe87a19` | 26628 | `tests/unit/test_pus_fu.c` |
| `df1a9a81c8b6fa1e49af947f6ad7f137ec24e93d7cdf636d64e468a62067d269` | 5532 | `tests/unit/test_pus_n7_adapter.c` |
| `747b3b9ae40ec93ac08dc45e9a34e6091b923b9ff5ab27ba6b083db797adf609` | 7991 | `tests/unit/test_pusopen.c` |
| `0be9793dc1d0ace0b68b4af7b7fccd9a7434d441c4bc90b97e0f43ad2d2c46e4` | 46971 | `tests/unit/test_r2_gap.c` |
| `d03bc0cdff206f4940669b41053e6bad570c698021d6ebb5da97e4bf6307d8d7` | 29389 | `tests/unit/test_r3_gap.c` |
| `9c29da3d84c9cb1c26c7f1e6b301603e531103cb85e74fc7833e8b81f5086775` | 14435 | `tests/unit/test_sched.c` |
| `3a5921f1458ab39a04e1edf44225fd71bffec5b5c2676ea3d3a7e4aa47ab462d` | 2539 | `third_party/README.md` |
| `10c0ca6d30804879980abbb3ddfa4532aba1a722ce29a313ddadb64390447109` | 2944 | `tools/scripts/ci_rtems_cross_build.sh` |
| `da65b6445ceb3738a9c8374e4e1547ff0381ed6a0623b9e0db150da1132a9571` | 3057 | `tools/scripts/emulated_run.sh` |
| `d3408713bdb6faec2cbe247a93cdbe5bb350e380c09dfdaa47b67ecdf14190c9` | 4206 | `tools/scripts/fetch_third_party.sh` |
| `b1fe665e5af3b1b9b8e6518fe5e413726d5ce2d156f1b3d0620f045550637cef` | 2053 | `tools/scripts/gen_pus_asn1.sh` |
| `b1bf2ac5e424973a7a9acbcdfa48e5768f26fa71c3eca64807437446c74e4463` | 1489 | `tools/scripts/run_sim.sh` |
| `48c1e6d126645d97c8437e1c7f708ef4bc79a20a6951093d12f4b94a04225201` | 2416 | `verification/lean/Frick/S4c.lean` |
> This manifest is a working-tree artifact (norm N11). It is not a signed run attestation
> or a release manifest. Generated-source/tool/dependency provenance (ND-EXT-84) is not established here.

## Addendum — 2026-10-19 easy-fixes batch candidate identity

The inventory above identifies the **S8 reconciliation candidate snapshot at HEAD
`e7e48ee`** (PA action A-PA-2026-10-18-01). This addendum records the identity of the
**subsequent, separate** delta carried by the 2026-10-19 easy-fixes batch, which was
produced on the working tree on top of that pushed baseline. The two inventories are
disjoint in purpose: the section above is the PA-facing candidate snapshot; the table
below is the follow-on batch delta and is **not** independently PA-verified.

- Baseline for this addendum: `e7e48ee` (pushed to `origin` and `gitea`); the delta was
  uncommitted when this addendum was generated.
- Method: sorted union of `git diff --name-only e7e48ee` and
  `git ls-files --others --exclude-standard`; SHA-256 + byte size recorded per path;
  no ignored `build/` artifact is counted.
- Delta paths: 27; combined digest over the sorted `<sha256>  <path>` lines:
  `8f7bbb3ffbeb584877b8f1ff44548057f62f9a62af4194ab2516882a5629425c`
- Bootstrap convention: this manifest file is itself part of the delta, so its own row
  uses the empty-file hash and size 0 (as in the inventory above); the addendum text
  and header are therefore not represented by a self-referential hash.
- Cycle evidence for this delta is retained under
  `docs/vv/evidence/2026-10-19-easy-fixes-*.log`, the mutant-m3 discrimination probe
  under `docs/vv/evidence/2026-10-19-s8-vv-fu-mutant-m3-discrimination.log`, and both
  are referenced from `docs/vv/verification-results.md`. The batch's own batch-scoped
  independent reviews are deferred as NC-EXT-99 (see `docs/planning/sprint-plan.md`).

| SHA-256 | Bytes | Path |
| ------- | ----: | ---- |
| `7bdbbe34ab24f1e034b4da22282322b59a1f947ca73bd0e325a38f11399737b0` | 109269 | `CHANGELOG.md` |
| `dfe155454062ac505f70f539fcb7522fcd668fd61ee04047c8243a6a1036db40` | 32084 | `README.md` |
| `2d84df7af23527a3651fb25946aba93bd7fcaf8e7dcba08b6f4719205dd21ab9` | 9136 | `docs/architecture/component-gap-analysis.md` |
| `97871d11089379921f5c21fe79699f42b1171d2966cacac632ee9c1c9616a43a` | 18952 | `docs/architecture/pus-c-from-scratch-assessment.md` |
| `b09ad64ab55b0a3b8f7ccf5de8bda768de208cf8d5eeca442e275b1c1cd990b6` | 27587 | `docs/engineering/user-integration-manual.md` |
| `f86404c00ffe8c1975f858504e539566f44e66251b9e132e490f95e38600c4de` | 126986 | `docs/planning/sprint-plan.md` |
| `fbbab0535061680460101988f3a53d665ed0ee14d0eab5455223ac50f0fea9b2` | 11841 | `docs/quality/licensing.md` |
| `73761651468a8372ff968a1e732aff168278bf479d97f032204380be01f4d073` | 43093 | `docs/requirements/SRS.md` |
| `190a10eb1ecadccb3d9c9dd3a6c8d16f74cac7733106975a90bc8af5d656fb08` | 15638 | `docs/requirements/SSS.md` |
| `a4e720e4b32370e605dc5cfd2f8e1163c1cdfae2ea65301cdd84bdf68d268340` | 27620 | `docs/reviews/2026-10-18-docs-alignment-r4-confirmation.md` |
| `4d9c1c1815718e8df532735b7a1737c7d08cccad5e3f0e2b4e317ea85977b156` | 39879 | `docs/reviews/2026-10-18-s8-vv-fu-delta-vv-review.md` |
| `daaf6f32c907c7ecc1807f240454f75abcc6017711363ff356d9f27498277f17` | 89531 | `docs/reviews/README.md` |
| `426d8218c2a23ca0389e71061dd3bcd8fe12113820f1bc4500c5cb152bd86efd` | 87664 | `docs/status/handover.md` |
| `a68d9a655a38698355cb7ac4b49e3731b663e5c02338a9ac1087d970a6cb6200` | 80514 | `docs/status/loop-audit.md` |
| `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` | 0 | `docs/vv/evidence/2026-10-17-s8-reconciliation-candidate-manifest.md` |
| `4e32e21463a363772bbdee364079afbdf60ade155d23516bf46322aea404a054` | 7714 | `docs/vv/evidence/2026-10-19-easy-fixes-asan.log` |
| `92c19b3e756e1d5660a23608da5b59121702cf0157f2e1b8c8b16aa6180d4c35` | 26366 | `docs/vv/evidence/2026-10-19-easy-fixes-ci-local.log` |
| `c7d1f9a5a8343199ee3ccaab25a107d6e6fdbcc53c6dc28badda6e8e8a6c019a` | 183821 | `docs/vv/evidence/2026-10-19-easy-fixes-docs.log` |
| `da69e9e40937c195c997c92ad9a509240654dbf08859746b565d84a24de6c83e` | 252 | `docs/vv/evidence/2026-10-19-easy-fixes-format-check.log` |
| `d1223ce32dafa765463f60b1390e3c6ff181479e62dd9603f9f77b61c03e74f1` | 768 | `docs/vv/evidence/2026-10-19-easy-fixes-gsw-selfcheck.log` |
| `6e4327b56f09fb19532988b7c96d6f259063d377c35bec58723a0e2c2e343043` | 2664 | `docs/vv/evidence/2026-10-19-easy-fixes-smoke.log` |
| `70948df006385a0e79f4a00b100d042f8492c3338564cee9f5f31df9085089cb` | 3915 | `docs/vv/evidence/2026-10-19-s8-vv-fu-mutant-m3-discrimination.log` |
| `28b08620839a437169953ef9cda478f3b4cfeeaee4916bfe6c4ebd69f0ac5028` | 66476 | `docs/vv/test-spec/ts-5-core-services.md` |
| `8b8ee288a78ac0f6b0202f68b83fed94d1d8e31b4bf375e68a1dbac4d65be231` | 61663 | `docs/vv/test-spec/ts-6-sw-robustness.md` |
| `7feb003a6c7f9639498942925609d13f1c5a021f6f156edf6f546edf742e100e` | 197726 | `docs/vv/verification-results.md` |
| `0215d919c3590e50d1fcd72cc6ccd3b388e1c8cc29ace354bd411d5695b4dc08` | 8824 | `tests/CMakeLists.txt` |
| `c789942e83c974aba30e8717c0e1fa909e6601453d5067641f05ca9a90be0a6e` | 27157 | `tests/unit/test_decoder_corpus.c` |
