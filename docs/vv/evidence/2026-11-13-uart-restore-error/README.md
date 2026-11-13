# UART deadline-writer restore and deadline evidence — 2026-11-13

## Scope and candidate identity

This retained evidence covers the current combined uncommitted candidate at
baseline `e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83`; it is not a package-wide
manifest or an approval. The UART delta checks every temporary descriptor-mode
restore result, preserves `write()` errno across `fcntl()`, and checks monotonic
deadline after each positive write, including the final write. N22-TS2-84 tests
EAGAIN + failed restore (FF_ERROR, exactly one write, no output, failed descriptor
state observable) and EAGAIN + successful restore that clobbers errno (retry and
exact payload). N22-TS2-85 tests an accepted final write delayed beyond a future
deadline (FF_TIMEOUT and exact bytes emitted). N22-TS2-86 checks an expired
zero-length request returns FF_TIMEOUT without a write. Timeout in the N22-TS2-85
case does not mean no bytes were transmitted.

## Inputs and binary identity

- `src/transport/link/link_uart.c`: `c39c3f271680d0d0c838d10fc72039e5ea65ee0f2fda8df7c56080200737cc99`
- `src/transport/link/link.h`: `45e3f58337546d7485efd0e477d042a788e6ca0fd7b88e4e6e14d54d122b1961`
- `tests/unit/test_link_uart.c`: `11d18353f7e006a13cf7897055e478e31619adb9ecbd33ecc83de115303f5006`
- `tests/CMakeLists.txt`: `68346402e3fa1ad0cc665669cc410d67165753ddf6d03544f29d4b96e1e7d038`
- Native executable `artifacts/test_link_uart`: `726d1052bde18396860a0fd9d1ed354a534e29ee7d0aadc4bc8ac7d47b041b3d`
- Fresh RTEMS/Renode ELF `artifacts/frick_fsw_renode.elf`: `e36aee30f381df32d9b085b11d616e35082a9650263e3b8614ed8f4004843a41`

## Gate sequence and results

1. `BUILD_DIR=build/uart-full make build`: PASS.
2. `BUILD_DIR=build/uart-full make test`: PASS, CTest 57/57.
3. `./build/uart-full/tests/test_link_uart`: 104 checks, 0 failures.
4. `make verify-lean`: PASS (`lake build`).
5. `make trace`: PASS (120 requirements, 76 SRS, 30 SSS, 14 gaps, 68 Lean theorems).
6. Pinned clang-format 18 changed-file dry run: PASS for `link.h`, `link_uart.c`, `test_link_uart.c`.
7. `make gsw-selfcheck`: PASS, including Python checks and N13 license checker.
8. `BUILD_DIR=build/uart-full make smoke`: PASS (banner + 7 apps).
9. `make rtems-docker`: PASS, RTEMS 6.2 cross-build.
10. `make renode`: PASS (`RENODE_TEST: PASS`, container exit 0); raw process/UART/status captures and ELF are in this directory. Emulator only, not target/HIL/timing.
11. `make docs-container`: PASS.
12. `make metrics`: PASS and regenerated README status panel.
13. Full pinned `make format-check`: FAIL only at three unchanged initializers: `src/transport/link/link_tsn.c:18`, `src/transport/link/link_ip.c:19`, `tests/unit/test_link_tsn.c:24` (NC-EXT-244; no unrelated source changed).

The initial default `make build`/`make smoke` attempts hit an existing `build/native`
CMake cache bound to `/workspace`; preserved logs are labeled as failed environment
attempts. The isolated `build/uart-full` Make target runs passed. No cache was
removed. All successful and relevant failed logs are retained beside this file.

## Limits

T3 remains **NOT FULLY QUALIFIED**; execution/review does not itself qualify the
test/tool. The PTY tests are host/Linux-oriented. A failed `fcntl(F_SETFL)` can
leave the shared descriptor nonblocking; the implementation reports `FF_ERROR`
but cannot guarantee restoration or concurrent-writer safety in that condition.
N22-TS2-85 is a host wall-clock timing check, not deterministic target timing or
WCET evidence. N22-TS2-86 covers only the expired zero-length API edge.
NC-EXT-191 remains OPEN for shared descriptor effects, target
readiness and target 250 ms bound. No SRS-REQ-074 compliance, HIL, physical target,
release, flight or risk-acceptance claim is made. Combined-candidate independent
re-review and package-wide manifest/attestations remain pending.
