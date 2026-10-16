# S8-FU candidate verification manifest (2026-10-15)

| Field | Value |
| --- | --- |
| Candidate | Source HEAD `13ebf92f46775aec5d26cc2fdc5bd47258ed4f1a` plus uncommitted working tree |
| Scope | Software source/test/configuration delta for CCSDS ingest, GSW stream handling, UART framing and PTY coverage |
| Software-source digest | `d5ced57ee9ed2ce85e21b99335fa33a06e08dbe3eb894ce65290bb2fe12bcf83` |
| Identity method | SHA-256 over sorted changed/relevant source paths; for each path hash `UTF-8(path) + NUL + SHA256(file-content) + LF`. Includes tracked modifications and untracked source/test files listed below; excludes documentation and this manifest. |
| Status | Candidate identity aid only; not a complete immutable/reproducible build manifest and not a qualification claim |

## 1. Changed source/test/configuration inventory

| SHA-256 | Path |
| --- | --- |
| `643c1b6b4cdb979731580b4d22b6893a2aa29d7325d23d8e875f15c293b56744` | `config/platform/platform_native.h` |
| `9f6b74897b48d8d1feeadae3052363cf43dd26d587c7fca9184bd9a187dce572` | `src/apps/app.h` |
| `30246693063eaa2cf15c65a6cf64e2bb139206739017260d2ba3acd49eff60d1` | `src/apps/app_cmd_ingest.c` |
| `07466ce3be3ce01acc91b2d5d9d5fa22b3728331a97a3f871cb727174d30e08b` | `src/transport/link/link.h` |
| `2be337f37f94d87c7cb1af005fdfeaa8a84464f3417e6703628216fe193f4b28` | `src/transport/link/link_ip.c` |
| `b7b4ce828018cff509939c5c6569dbb0cd7b15b502c9a681b3ddd96d011cdab1` | `src/transport/link/link_tsn.c` |
| `7e8eeb0582facd0571dba355ddb6e88a6bdeb76b86fed4682255551eea8a9aae` | `src/transport/link/link_uart.c` |
| `42fecfe04e314065fb0cc654de07e38dc86b1e4a5effc82f28aab8c20edb1c28` | `tests/CMakeLists.txt` |
| `8b5cd5a782971a49faab29de430037e9e62d3649d54d4daa71f9b196d15316e1` | `tests/python/test_ff_gsw.py` |
| `bf16d313db8ca02610f1b77d8099d26d80ef4647eb1126a4253782215daab518` | `tests/unit/test_link_tsn.c` |
| `08485a0c2fb49ac7f79b734521c08c02f5b27b8199fe502f7a9651e109130a2f` | `tests/unit/test_link_uart.c` |
| `983eb55d97bfe6196a8ff6abf6854b1f95335d181a0ef89355ee5a7070a70b0d` | `tests/unit/test_r2_gap.c` |
| `2c568011d900d69e089180af918690d7beaebf212a96329d560ec3327d159577` | `tools/gsw/ff_gsw.py` |

## 2. Build configuration and tools observed

- Host V&V image: `frick-vv:local`, image ID
  `sha256:5cbc72aa4125cfdbd8c0baa4ac2a1cb9d1447ff3147ac0b94e2ed0d918b83e91`.
- RTEMS toolchain image: `frick-rtems-toolchain:local`, image ID
  `sha256:90f81d0806112a0ddd95f60c9e9cc857de4acf8e71e3dbc9434f6612dcc2f9a0`.
- `build/ci`: native Debug, `FF_PLATFORM=native`, `FF_USE_LWIP=ON`,
  `FF_USE_PUS_ASN1=OFF`, coverage off; the build was executed in the
  `frick-vv:local` container, not with the host toolchain. On 2026-10-16 the
  same image ID was inspected: GCC 13.3.0, CMake 3.28.3, Python 3.12.3,
  clang-format 18.1.3 and Ninja 1.11.1; Lean 4.10.0. This corrects the initial
  manifest draft, which mistakenly recorded host-side tool versions as the
  build environment. Exact per-run output remains tied to the retained CI log.
- `build/rtems`: RTEMS MPFS cross-build, `FF_PLATFORM=rtems`,
  `FF_USE_LWIP=OFF`, `FF_USE_PUS_ASN1=ON`; compiler reports GCC 13.3.0
  (RTEMS 6, RSB revision `3814cb0e7f86cca2be403eac831f9bf571984659-modified`,
  Newlib `1b3dcfd`). This is a successful cross-build only.
- `build/renode`: RTEMS MPFS image, `FF_PLATFORM=rtems`, `FF_USE_LWIP=ON`,
  `FF_USE_PUS_ASN1=OFF`, Renode test enabled; Renode result is emulation only.
- Pinned lwIP checkout: commit
  `77dcd25a72509eb83f72b033d219b1d40cd8eb95`; checkout status was clean.
- Docker base package sources/versions and RTEMS image construction inputs are
  mutable; image IDs identify the observed images but do not establish
  bit-for-bit reproducibility (NC-EXT-35). Full provenance, qualification and
  external dependency/legal disposition remain open (NC-EXT-29/43/T23).

## 3. Verification results and artifact identities

- `make ci-local`: PASS; build, 37/37 CTest, Lean build, trace PASS (117
  requirements: 73 SRS, 30 SSS, 14 GAP; 62 Lean theorems checked).
- Full CTest output retained in `2026-10-15-s8-fu-ctest-full.log`,
  SHA-256 `f428bc1beb1e67c7f2fad0fea66fdf7aef872e0ab9ca76a12e68e20e285c4130`.
- Focused `link_uart` executable: 46 checks / 0 failures; output retained in
  `2026-10-15-s8-fu-link-uart.log`, SHA-256
  `204e1fd02f69293354380a4acbbec0549d7a2ac82d9c815f8b568510a80d3d4c`.
- SHA-256 artifacts: `build/ci/frick_fsw` =
  `fd53df4f70709a67b3b887a8ceca8d3019b9a23e4d78a1b26816cf2fb90f2b14`;
  `build/ci/tests/test_link_uart` =
  `16d13734d14e61535db26210cf3cb8cf4157b2e183ea712d8e00a74e060efb8c`;
  `build/ci/tests/test_r2_gap` =
  `6e5b6677964411d25adad6320505d27069463ec3a1904511e62eba368ec7ce00`;
  `build/renode/frick_fsw_renode.elf` =
  `e19da73a393b8a313d11cd093b0525ebcc8e70f1c68be0dfd8a25bd1a29775c8`;
  `build/rtems/frick_fsw` =
  `4778f18f62a646bedfcd87a2c0dae948931d70b1a3654ef3ae30db10a3cb9d14`.
- Separate smoke, GSW/license self-check, metrics, Doxygen, full-source format,
  Renode emulation and RTEMS cross-build results are recorded in
  `docs/vv/verification-results.md`. Renode did not exercise UART receive
  framing; no physical-hardware or target runtime/timing claim is made.

## 4. Residual identity limits

### 4.1 Tool-version correction record (2026-10-16)

The initial draft's host compiler/tool version tuple was not the environment
that produced `build/ci`; those values are withdrawn. The candidate was built in
the image identified above, and its observed versions are now recorded in §2.
The source digest and the listed file/artifact hashes are unchanged by this
metadata correction. This correction does not establish complete reproducible
build provenance or close NC-EXT-29.

The source digest permits comparison of the listed software delta but does not
capture all tracked documentation, host kernel, environment variables, Docker
daemon, mutable package repositories, complete compiler dependencies, or every
generated intermediate. This manifest improves candidate effectivity evidence
but does not close NC-EXT-29, qualify T1/T18/T23, establish reproducibility, or
authorize commit, release, or flight.
