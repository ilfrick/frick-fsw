# Session handover — GSW UART/PUS host E2E

| Field | Value |
| --- | --- |
| Project | frick-fsw |
| Date | 2026-11-15 |
| Scope | User request m00806: close available end-to-end coverage gaps, execute tests, then resume normal development |
| Baseline | `fc68b9d121bce2194c1beaaa9a7dc4cdab0552c`; existing S1 progress-failure/SPDX work remains uncommitted |

## Delivered in this loop

- `tools/gsw/ff_gsw.py` now implements documented `--ack` stage selection (including `none`), sets PUS TC ACK bits, and waits for/validates all requested positive S1 ACKs with the S17.2 ping response.
- Added `tests/integration/gsw_uart_e2e_server.c`, `tests/python/stubs/serial.py`, and `tests/python/test_gsw_uart_e2e.py`; native CTest runs nominal and OOB (`gsw_uart_e2e`, `gsw_uart_e2e_missing_ack`) through actual GSW CLI ↔ bridged POSIX PTYs/test-only serial adapter ↔ production `FF_LinkUartBackend`/PUS. Nominal asserts exact TC bytes `1810c000000a2b1101010201020304f72b`, TMs `[1,1],[1,3],[17,2],[1,7]`, ACK payloads, S17 echo `01020304`, CRC and modulo-16384 PUS secondary-header counter. OOB drops TM[1,1] on the GSW leg and asserts CLI exit 1 with the exact missing-response error while server captured all four TMs.
- TS-2 rev0.15 marks N22-TS2-75 nominal and N22-TS2-87 OOB host-covered (87 total: 44 nominal, 41 OOB, 2 mixed; 48 covered, 7 host covered, 31 gaps, 1 characterization). This is host SIL only; it is not pyserial, external ground segment, physical UART, RTEMS, HIL, Renode or SDLS evidence.
- `docs/vv/tool-qualification.md` rev2.3 records exact T3/T7 hashes and execution. Both remain NOT FULLY QUALIFIED until independent exact-hash N10 review.
- N19 audit `docs/status/loop-audit.md` rev11.14 records this loop and its remaining gates; NC-EXT-245 remains open for late audit sequencing.

## Verification and retained evidence

All evidence is under `docs/vv/evidence/2026-11-15-gsw-uart-e2e/`.

- Focused CTest (`gsw_uart_e2e`, `gsw_uart_e2e_missing_ack`): 2/2 PASS (0.94 s).
- Python GSW tests: 14/14 PASS.
- Full host CTest: 62/62 PASS, 19.71 s, including nominal and OOB integration tests.
- `make verify-lean`, initial `make trace`, `make gsw-selfcheck`, and `BUILD_DIR=build/s1-review make smoke` passed. After adding the test-to-SRS entries in both traceability files, `make trace` was rerun and passed; its retained artifact is `trace-post-mapping.log` (120 requirements; 76 SRS, 30 SSS, 14 gaps; 68 Lean theorems).
- The default `make build` and `make smoke` each initially failed before compile/boot because ignored `build/native/CMakeCache.txt` was generated under `/workspace`. Using `BUILD_DIR=build/s1-review` passed; no destructive cache cleanup was performed. Exact diagnostic is retained in `stale-build-cache-failure.md`.
- `tests/integration/gsw_uart_e2e_server.c` initially required clang-format 23.1.3 correction; the initial failure and correction are retained in `initial-failures.md`. Final build/test/Lean/trace/GSW/smoke logs are retained for the current nominal+OOB candidate.

## Open gates / next actions

1. E2E cycle closed: `make trace` passed after the traceability mapping, `logs.sha256` includes `trace-post-mapping.log` and verifies, and final `git diff --check` passes.
2. Continue normal S1/NC-EXT-251 work only after finishing this E2E evidence cycle. No ACK behavior was changed. The option-2 emission contention path can reject an ACK while SRS-REQ-006 requires exactly one per requested stage; the previously sent direct notification requires owner disposition before altering that contract.
3. NC-EXT-246 still bars Docker/Renode pending safe preflight; NC-EXT-247 means N18 is not evidenced. SDLS N22-TS2-73, Renode 76/77, and target/HIL 78–80 remain open; do not claim coverage without hardware/authority.
4. Corrected-tree R1/R2/R3/R4, PA, and N17 documentation reviews remain pending. T3/T7 qualification remains open. No commit/push until all applicable gates and the full sprint scope pass under N7.
5. If any subagent fails, stop immediately and notify the user; use `hermes-consult_notify_nicola` for direct MCP notification as directed.

No release, qualification, flight, safety, security, waiver or residual-risk acceptance is implied.