# S4c/S4d and default-configuration evidence (2026-10-15)

## Scope and status

This note records host, emulation, and cross-build results for the uncommitted
S4c/S4d increment and ADR-0018 default changes. It is not an independent review,
qualification record, certification claim, release authorization, or flight
readiness finding.

Delivered implementation scope:

- generic fabric-window HAL/backend and CAN-FD fabric-window reference backend;
- documented, allocation-free, single-instance CANopen subset;
- bounded RS-485 DE/RE control through the serial contract;
- PUSopen adapter seam tested against an in-repository API test double;
- open CCSDS PUS backend default, optional N7/ESA ASN.1 backend, lwIP default-on
  with pinned dependency fetch before default CMake configure.

## Configuration and identities

- Candidate baseline: mutable uncommitted tree based on HEAD `6ce6bd7` at cycle
  start; no full source/configuration manifest or immutable container supply
  chain is asserted.
- Final default cache: `build/ci/CMakeCache.txt`;
  `CMAKE_BUILD_TYPE=Debug`, `FF_USE_LWIP=ON`, `FF_USE_PUS_ASN1=OFF`. The earlier
  `build/s4-default` tree is stale relative to the final test source and is not
  cited as final-run evidence.
- `frick-vv:local` image ID:
  `sha256:5cbc72aa4125cfdbd8c0baa4ac2a1cb9d1447ff3147ac0b94e2ed0d918b83e91`.
- `frick-rtems-toolchain:local` image ID:
  `sha256:90f81d0806112a0ddd95f60c9e9cc857de4acf8e71e3dbc9434f6612dcc2f9a0`.
- SHA-256 of host executable `build/ci/frick_fsw`:
  `140cf791c22ff1aff9cfeb8157b210ecfc5ba22a5d6c5bdd8460ef1b7cb20a90`.
- SHA-256 of Renode MPFS image `build/renode/frick_fsw_renode.elf`:
  `54636b0f78edf355473904e448a00dcb7cd9bb3c05971a7460da7032ac10ef98`.
- SHA-256 of QEMU image `build/rtems-generic/frick_fsw`:
  `2ada5ba3132b43f3ba8d89457a1909d452d79dc1c8913e1e878c8cc0cbce0f1a`.

These hashes identify observed outputs, but not all source, environment,
dependency, or toolchain inputs; reproducibility remains unestablished.

## Results

| Check | Result | Evidence boundary |
| --- | --- | --- |
| `make ci-local` | PASS; build, 36/36 CTest, Lean, trace | Latest full rerun after final review-record updates; Debug SB/EVS sweep 17.71 s; Docker-based local pipeline |
| Default Debug configuration | PASS; final `build/ci` 36/36 | CMake cache confirms lwIP ON, PUS ASN.1 OFF |
| Release build/CTest | PASS; 36/36 | Latest rerun; structural sweep 18.82 s |
| AddressSanitizer build/CTest | PASS; 36/36 | Latest rerun; structural sweep 102.14 s |
| Lean build | PASS, Lean 4.10.0 | Pure contract models only; no C refinement proof |
| Traceability | PASS; 117 requirements (73 SRS/30 SSS/14 GAP), 62 theorem links | Does not establish requirement compliance by itself |
| C formatting and whitespace | PASS; repository C/H dry-run and `git diff --check` | Local source checks |
| `make docs` | PASS | Doxygen output generated; existing warnings not treated as qualification evidence |
| `make gsw-selfcheck` | PASS; 5 GSW + 9 metrics + 14 license tests | Plus CLI packet self-check |
| `make smoke` | PASS; six applications registered | Native simulator smoke test |
| `make renode-env` | PASS | Renode starts and PolarFire SoC platform loads |
| RTEMS MPFS cross-build | PASS in `frick-rtems-toolchain:local` | RTEMS 6.2 toolchain container |
| `make renode` | PASS, `RENODE_TEST: PASS` | MPFS emulation integration run; no CAN-FD fabric peripheral model |
| `make qemu` | PASS; banner and six applications | Generic RTEMS/QEMU `virt` image, not MPFS hardware evidence |

Earlier point-in-time cycles recorded 16.52/18.48/101.00 s, 16.57/19.79/99.28 s,
20.16/24.51/130.43 s, and 17.27/18.21/98.44 s. The values in the table above
(17.71/18.82/102.14 s) come from the latest full cycle after review-record
updates. These are distinct CTest runs, not conflicting measurements.

Final emulator run logs are retained as `2026-10-15-renode-console.log`,
`2026-10-15-renode-process.log`, and `2026-10-15-qemu-console.log`. Their
SHA-256 values from the latest rerun are `a0ae13c65695bef6480488ff92b1e487bbd198cb596a3b2cbdd16bc0517d85c7`,
`ce2f24e0d7cc9d72fe1849ef6523587cc3de7493b0e75e688b27273685cafd0c`, and
`e07c9b6fa946418d30853a5ddfc824de6d5cbe76f1a19a8d77d2ffcfee73f442`.

Focused component test counts after the review-driven corrections: fabric 119
checks, CAN-FD 328, CANopen 171, serial 67, and PUSopen adapter 33. The final
Debug CTest suite includes these as passing tests. `test_canopen` was run directly
from the final Debug, Release, and ASan build trees and reported 171 checks,
zero failures. Exact missing-index/subindex upload/download response oracles assert
fresh transmissions. The PUSopen test uses `tests/stubs/pusopen.h`; it does not
establish compatibility with or qualification of the proprietary vendor SDK.
CANopen coverage is limited to the SRS-documented subset, not complete CiA
301/302/402 conformance. CAN-FD target behavior remains open under
NC-S4c-01/GAP-A5; RS-485 electrical timing and board behavior remain GAP-A5.

## Failed attempts and recovery

1. Initial direct `make renode-image` failed because the host lacked
   `/opt/rtems/6/bin/riscv-rtems6-gcc`.
2. An initial containerized retry found a CMake cache configured using the host
   source path. A stale generic-QEMU cache had the same host/container path
   mismatch. The stale caches were removed; final MPFS and generic RTEMS images
   were freshly configured/built in dedicated `build/s4-rtems`, `build/s4-renode`
   and `build/rtems-generic` trees inside `frick-rtems-toolchain:local`.
3. Direct host CTest against root-owned `build/ci` could not write
   `Testing/Temporary/LastTest.log`, and a direct host rebuild later found the
   container-path cache and root-owned Ninja lock. These attempts are not counted
   as passes; final CI-local configure/build/test/Lean/trace ran in-container and
   passed 36/36.
4. A host `make docs` attempt could not remove root-owned Doxygen output. The
   output tree was removed/regenerated in `frick-vv:local`, the index existence
   guard passed, and ownership was restored; containerized Doxygen 1.9.8 passed.

Final Renode/QEMU logs are retained above and do not substitute for failed
attempt logs. Failed attempts are not counted as passing evidence.
The fail-closed ASN.1 and dirty-lwIP probe outputs, including command dispositions,
are retained in `2026-10-15-s4c-config-probes.md`. Other failed build/permission
attempts are summarized above; raw logs are not retained for those attempts.

## Open actions

- Required independent FSW/V&V, systems, documentation, and PA reviews have
  occurred (FRICK-REVIEW-0288..0292 and bounded delta reviews 0293..0307). Functional
  corrections received bounded review, while integration/authority, dependency
  evidence, provenance, hosted-CI, target-CAN-FD, and vendor-SDK findings remain
  open; record and disposition these before any closure claim.
- Run hosted GitHub and Gitea CI only after the applicable reviews and gates;
  neither hosted result is represented by these local tests.
- Preserve emulator/target limits; no Renode CAN-FD result or vendor PUSopen
  compatibility result exists.
- Do not commit or push until all applicable gates, review dispositions, and
  sprint scope are complete under AGENTS.md. This note does not authorize a
  release, flight, or operational action.
