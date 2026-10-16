# S8-FU R1-corrected candidate verification manifest (2026-10-16)

| Field | Value |
| --- | --- |
| Candidate | Source HEAD `13ebf92f46775aec5d26cc2fdc5bd47258ed4f1a` plus current uncommitted software delta |
| Scope | Software/test/configuration delta for CCSDS ingest, GSW stream handling, UART framing and PTY coverage |
| Software-source digest | `8a80a8c4d05b9561ecfd525826ea6fbd213398e7305d9021fcf92d7c08f6e4af` |
| Identity method | SHA-256 over sorted changed/relevant source paths; for each path hash `UTF-8(path) + NUL + SHA256(file-content) + LF`. Includes tracked modifications and untracked source/test files below; excludes documentation and this manifest. |
| Status | Bounded candidate identity aid only; not a complete immutable/reproducible build manifest or qualification claim |

## 1. Changed source/test/configuration inventory

| SHA-256 | Path |
| --- | --- |
| `643c1b6b4cdb979731580b4d22b6893a2aa29d7325d23d8e875f15c293b56744` | `config/platform/platform_native.h` |
| `9f6b74897b48d8d1feeadae3052363cf43dd26d587c7fca9184bd9a187dce572` | `src/apps/app.h` |
| `30246693063eaa2cf15c65a6cf64e2bb139206739017260d2ba3acd49eff60d1` | `src/apps/app_cmd_ingest.c` |
| `07466ce3be3ce01acc91b2d5d9d5fa22b3728331a97a3f871cb727174d30e08b` | `src/transport/link/link.h` |
| `2be337f37f94d87c7cb1af005fdfeaa8a84464f3417e6703628216fe193f4b28` | `src/transport/link/link_ip.c` |
| `b7b4ce828018cff509939c5c6569dbb0cd7b15b502c9a681b3ddd96d011cdab1` | `src/transport/link/link_tsn.c` |
| `a8303602aa302aafc171eed84d91e8ef5ba677c4df1c5056171692694fedb79c` | `src/transport/link/link_uart.c` |
| `5eb92d387e1afd1ee93612fd053a49b1c8f4658242fdc241cee27c6095a6275d` | `tests/CMakeLists.txt` |
| `8b5cd5a782971a49faab29de430037e9e62d3649d54d4daa71f9b196d15316e1` | `tests/python/test_ff_gsw.py` |
| `bf16d313db8ca02610f1b77d8099d26d80ef4647eb1126a4253782215daab518` | `tests/unit/test_link_tsn.c` |
| `a82a2356adc9e1f464e0fcc27ebbb809893b129d2c641f94d1dd1ff01a08ed88` | `tests/unit/test_link_uart.c` |
| `983eb55d97bfe6196a8ff6abf6854b1f95335d181a0ef89355ee5a7070a70b0d` | `tests/unit/test_r2_gap.c` |
| `2c568011d900d69e089180af918690d7beaebf212a96329d560ec3327d159577` | `tools/gsw/ff_gsw.py` |

## 2. Build environments observed

- Host V&V image `frick-vv:local`, image ID
  `sha256:5cbc72aa4125cfdbd8c0baa4ac2a1cb9d1447ff3147ac0b94e2ed0d918b83e91`:
  GCC 13.3.0, CMake 3.28.3, Python 3.12.3, clang-format 18.1.3, Ninja 1.11.1,
  Lean 4.10.0. `build/ci` is Debug, `FF_PLATFORM=native`, lwIP ON, PUS ASN.1
  OFF, coverage OFF, Renode test OFF.
- RTEMS toolchain image `frick-rtems-toolchain:local`, image ID
  `sha256:90f81d0806112a0ddd95f60c9e9cc857de4acf8e71e3dbc9434f6612dcc2f9a0`:
  RISC-V GCC 13.3.0 (RTEMS 6, RSB revision
  `3814cb0e7f86cca2be403eac831f9bf571984659-modified`, Newlib `1b3dcfd`),
  CMake 3.28.3, Ninja 1.11.1.
- `build/rtems`: `FF_PLATFORM=rtems`, lwIP OFF, PUS ASN.1 ON,
  Renode-test OFF. `build/renode`: `FF_PLATFORM=rtems`, lwIP ON,
  PUS ASN.1 OFF, Renode-test ON.
- Pinned lwIP checkout: `77dcd25a72509eb83f72b033d219b1d40cd8eb95`, clean.
- Docker base/package inputs and RTEMS image construction remain mutable; image
  identity is observed, not proof of reproducibility/provenance (NC-EXT-29/35).

## 3. Verification evidence associated with this source digest

- `make ci-local`: PASS, 37/37 CTest, Lean 4.10.0 and trace (117 requirements:
  73 SRS, 30 SSS, 14 GAP; 62 Lean theorems). Output:
  `2026-10-16-s8-fu-r1-ci-local.log`, SHA-256
  `5000429b2e573aca23dd813ed2382c31b2d42a8f625e6f2c1fa9245288633f5d`.
- Focused UART executable: 51 checks / 0 failures. Output:
  `2026-10-16-s8-fu-uart-ptys.log`, SHA-256
  `92e36c7aa75914e61cb3dc4d13750b8940aec62d0145dcae71d41ea3c5a0e220`.
- Other checks and retained logs:
  `2026-10-16-s8-fu-r1-aux-checks.log`, SHA-256
  `f350d55c528a874e5ab80aab9fdee14e12254d217a1d3c072b45b556813820b8`;
  focused CTest passed 1/1, smoke/GSW/license/metrics/docs/format/diff checks
  passed; Doxygen emitted 112 warnings. `make rtems-docker` cross-build passed;
  Renode image rebuilt and `make renode` passed its exercised emulator checks.
- Artifact SHA-256 at the recorded build:
  `build/ci/frick_fsw` = `fd53df4f70709a67b3b887a8ceca8d3019b9a23e4d78a1b26816cf2fb90f2b14`;
  `build/ci/tests/test_link_uart` = `103f6b876ac01b1cd68b2d7ff37383c5e77c2891daa88dd82e9bdadd703541a0`;
  `build/rtems/frick_fsw` = `d5122d106d316c66af1eda5adcb76b184ddf1d7798f58a4d880de873729b0c38`;
  `build/renode/frick_fsw_renode.elf` = `6b02ccdcdf1f390e0cdec6554560a9e0ab3bb07927551c2243a93ed8f45f7002`.
- Host PTY tests accept only enumerated Linux behavior. RTEMS cross-build is
  compile/link evidence; Renode is emulation and does not exercise UART receive.
  No target runtime, timing, peripheral, physical UART, qualification, or
  external ground-segment E2E is established.

### Final current-document cycle (2026-10-16)

After the TS-2/SVA/ICD/status and R4/PA correction records were updated, the
local applicable cycle was repeated. `make ci-local` passed 37/37 CTest, Lean
and trace; retained output is `2026-10-16-s8-fu-final-ci-local.log`, SHA-256
`0f8178c39b0b60c9712f95f8a36abe30a83d068bf8f4931837645f97c9dac951`.
Auxiliary smoke, GSW/license, metrics (35/35 coverage CTest), docs, full-source
format, diff and JSON checks passed; log `2026-10-16-s8-fu-final-aux-rerun.log`
SHA-256 `492796283ef9bdf4388919ecd1534cbee538db7c60a11af9b205fb2739c624b6`.
The focused UART PTY executable reported 51 checks/0 failures; log SHA-256
`92e36c7aa75914e61cb3dc4d13750b8940aec62d0145dcae71d41ea3c5a0e220`.
RTEMS cross-build and Renode emulation passed. Doxygen emitted 112 warnings;
they are recorded, not represented as warning-free. These remain local-only
results and do not resolve authority, external, target-runtime, physical UART,
provenance, qualification, phase or release gates.

### Final repeated cycle after status/evidence recording

The complete applicable cycle was repeated after updating verification, status
and handover records. Retained outputs:

| Evidence | Result | SHA-256 |
| --- | --- | --- |
| `2026-10-16-s8-fu-final-final-ci-local.log` | `make ci-local`; 37/37 CTest, Lean and trace pass | `97ba4113e31631ea64c2a8a0e47e9908897b12b309037ed654d2cf1c82cb384d` |
| `2026-10-16-s8-fu-final-final-aux.log` | smoke, GSW/license, metrics, docs, format, diff/JSON pass; 112 Doxygen warnings | `767604775339162e32d3560cfb643da055aaa637eee7ad8739168110abffd798` |
| `2026-10-16-s8-fu-final-final-rtems.log` | RTEMS 6 cross-build pass | `3ca3a64f24b094deb2213162523579ab88c102e0ee79c65355df26c03ebcd4de` |
| `2026-10-16-s8-fu-final-final-renode.log` | Renode emulator check pass | `0a806a8d52b575244eaf4d0bd1b2ffabbe9e4d3309385b8e4482899869db446a` |

The focused UART result remains 51 checks/0 failures as recorded above. These
runs are local and do not change authority, external-E2E, hardware or release
limitations.

## 4. Residual identity and policy limits

The digest covers the listed changed software paths; it omits documentation,
host kernel, full environment variables, Docker daemon, all package-repository
inputs and every generated intermediate. The manifest does not close NC-EXT-29,
qualify T1/T18/T23, establish reproducibility, or authorize commit/release.
NC-EXT-07 also remains open: a malformed but plausible in-range PDL candidate
can encompass a following valid TC, and no authority-approved discard/recovery
policy is available. No guessed inner boundary is implemented.
