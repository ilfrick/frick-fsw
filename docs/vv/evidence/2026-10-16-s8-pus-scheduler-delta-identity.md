# S8 PUS/Scheduler delta identity aid — 2026-10-16

| Field | Value |
| --- | --- |
| Candidate base | `b8c3aeec5484476407367024f3ce25c5af2bd30e` plus uncommitted worktree |
| Scope | PUS/Scheduler implementation, selected build inputs, policy/architecture/test artifacts for the ADR-0020 delta; **not** the complete S8/OBCP product set |
| Inventory | 19 listed files below |
| Digest | `272543ef10c5d1bf65bc7be65936101e52a30dd99cdb8fb283bed8584bf9cf9a` |
| Digest method | SHA-256 over sorted UTF-8 `path + NUL + raw SHA-256(contents) + LF` records |
| Claim limit | Author-generated delta identity aid only. It is not independent PA verification, source-to-binary provenance, signed run attestation, reproducible build evidence, qualification, phase acceptance, or release authorization. NC-EXT-91 remains open pending PA review. |

## Build/run references

- Latest `make ci-local` retained log:
  `2026-10-16-s8-final-format-ci-local.log`, SHA-256
  `cd0572e3a5cbb812087712982dd095c4ea565c9208c47f48be1b16490f016ae8`.
- Host `build/ci/CMakeCache.txt` SHA-256:
  `7f865d42506bf513a1057ceb4f57aafb2e7d87a61c5d58676f71f5cad49da7f6`.
- Host `build/ci/frick_fsw` SHA-256:
  `7ee78ef375b21cbae7355439679247a4eaa533368491a3cc8eaa82e267256f3d`.
- Configuration: native Debug; default N7/ESA ASN.1 backend enabled; OBCP
  engine and tests enabled. Exact CMake options are in the cache. `make ci-local`
  passed 43/43 CTest, Lean build and trace after the final formatting and
  Makefile correction.

## Inventory

| SHA-256 | Path |
| --- | --- |
| `db0dc51813920bab13ca55a870fb885155fb6edf63112d02ba2d1ad593596c2e` | `CMakeLists.txt` |
| `1c4a0501cdb23539906a04534b0fab8b2bafeafecc3add01ea6c8d1086ceee0f` | `Makefile` |
| `8f0dfebd66061a6160041326fc3dbac386de2119df61f3b2d6e6a591d7b84459` | `config/fsw_config.h` |
| `76615ce9cae0c88ffa7cf4ac7a4879ed1ca0801f14e5eae5bbd35dc9a23cc1b3` | `docs/adr/ADR-0020-provisional-pus-and-scheduler-policies.md` |
| `981ae54b0701bfe1831602a706f736f5fe1c5fe4bca29f856f817118fc138371` | `docs/architecture/pus-layer.md` |
| `b28c58711d18062f40d563b8acbf6f0a93e1476a94df0829ca2a2d6d50f8bf34` | `docs/architecture/scheduling.md` |
| `63bdd5da0313039207b6848d95bd6c615784c4120968e646cf8de38287c52152` | `docs/vv/test-spec/ts-1-protocol-core.md` |
| `a3ffd80a76ffb81a27aeafe6c35937466070d0909c3d820c0fa597d8469479b4` | `docs/vv/test-spec/ts-5-core-services.md` |
| `e68e37772258cabc025156decb77c6509110bd34cdc37fe1d6da16b07cf3b860` | `src/core/sched/ff_sched.c` |
| `2cae84b0bb0050a855023f1c5caae341a4d2279ffcd4ab5add33736cfa93cd6c` | `src/core/sched/ff_sched.h` |
| `580ea37164df5ef7e28eedc4f75b79fd7523ac32ff108b8beb22d1dbd244b749` | `src/pus/pus.c` |
| `ec4d04782c1911caf1abbf905614dd14b62b3f03c1ea395b42bb166f764cd351` | `src/pus/pus.h` |
| `7417bcc56403b30ce4a300102822fb7b1d90daa0facf89dac5a923206b9228bb` | `src/pus/pus_services.c` |
| `c0605afbd13a6c3a8a27a4d8398b41469bbcb4d274a853f1874b1d7983d78ab1` | `tests/CMakeLists.txt` |
| `22a2f19fec2ff2417f14a349670d727419bbbca00ff70a28225a51b77b29b9f2` | `tests/unit/test_pus.c` |
| `eaf00e51c5476cadb7610bd574b01b20c4997e442c58fc76842b010ecfe87a19` | `tests/unit/test_pus_fu.c` |
| `0be9793dc1d0ace0b68b4af7b7fccd9a7434d441c4bc90b97e0f43ad2d2c46e4` | `tests/unit/test_r2_gap.c` |
| `d03bc0cdff206f4940669b41053e6bad570c698021d6ebb5da97e4bf6307d8d7` | `tests/unit/test_r3_gap.c` |
| `9c29da3d84c9cb1c26c7f1e6b301603e531103cb85e74fc7833e8b81f5086775` | `tests/unit/test_sched.c` |

## Open limits

The existing 49-file candidate manifest remains the broader identity aid and
does not subsume this list. Conversely, this 19-file delta inventory does not
identify all current OBCP/S8/V&V changes. PA must independently verify these
file hashes/digest and correlate the successful run, configuration and binary
before NC-EXT-91 can be dispositioned. Mission PUS-profile acceptance,
scheduler time-domain approval, target timing and physical-hardware evidence
remain separate open gates.
