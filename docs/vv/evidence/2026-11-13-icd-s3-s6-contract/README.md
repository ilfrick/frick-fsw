# Evidence — ICD §4 S3/S6 dispatch-contract correction (2026-11-13)

## Candidate scope

Documentation/interface-contract correction with **no production wire-behaviour change**:
`docs/interfaces/ICD.md` rev 1.8 (§4 Service 3 = 1 enable / 2 disable / 3 periodic-alias / 4 one-shot
request / 25 report TM / 128 create / 129 delete; Service 6 dump = TC[6,2]) plus aligned citing text
(`src/pus/pus_services.c` comments, `docs/architecture/pus-layer.md` rev 0.1.9, `README.md`,
`docs/adr/ADR-0020` rev 0.4 label note, TS-1 rev 1.13, `docs/vv/target-io-coverage.md`,
`docs/vv/xw3-tc-oracle-matrix.md`, `tools/scripts/target_req_manifest.json`,
`tools/scripts/xw3_manifest.json`, `src/apps/app_req_test.c` gap string, `tests/unit/test_pus.c` /
`tests/unit/test_r3_gap.c` comments) and the rewritten residual dispatch test
`tests/unit/test_pus_s11_residual.c` (1176 checks, registered CTest `pus_s11_residual`).

NC-EXT-16/17/18 remain **CLOSED for implementation only**; formal mission ICD entry remains M15 scope.
ESC-HK-01/NC-EXT-210 create width, ESC-HK-02 (no periodic production caller) and NC-EXT-208 / ESC-HK-05/06
period bounds remain authority-owned. No NC is closed; no authority decision is taken; no compliance,
qualification, release or flight claim is implied.

## Gates executed on this exact tree (raw logs in this directory)

| Gate | Result | Log |
|---|---|---|
| Configure + build (`BUILD_DIR=build/uart-full`) | exit 0 | build.log |
| CTest | 58/58 PASS | ctest.log |
| Focused `pus_s11_residual` | 1176 checks, 0 failures | pus-s11-residual.log |
| Lean 4 | `lake build` exit 0 | lean.log |
| Traceability | PASS (120/76/30/14/68) | trace.log |
| GSW / metrics / N13 license | OK | gsw-selfcheck.log |
| Smoke (native simulator) | PASS, 7 applications | smoke.log |
| Coverage / metrics | 15924/33832 lines (47.1%), 1796/2640 functions (68.0%) | metrics.log |
| Docs (Doxygen) | exit 0, index non-empty | docs-container.log |
| RTEMS 6.2 cross build | exit 0, `frick_fsw` linked | rtems-docker.log |
| Renode N18 | `RENODE_TEST: PASS`, container exit 0 | renode.log + renode-run/ |
| Renode ELF sha256 | `41acb18583ac25a6deb671274e1400fe4db8b84f6c105958f22959db0660cb20` | renode-elf.sha256 |
| Python verdict fixtures | 17 tests OK | python-fixtures.log |
| Changed-file clang-format (23.1.3) | exit 0 (pinned 18 unavailable) | format-changed-files.log |

The Renode run used the RTEMS image rebuilt from the current tree in this cycle (fresh ELF above); it is
emulator-only evidence, not physical target, HIL, timing/WCET, qualification or flight evidence.

## Limitations

- Full pinned clang-format 18 `make format-check` remains blocked: only clang-format 23.1.3 is installed
  on the host, and three unchanged initializer files (`link_tsn.c:18`, `link_ip.c:19`,
  `test_link_tsn.c:24`) violate it (NC-EXT-244, deferred under N21). Changed C files pass 23.1.3.
- `make metrics` refreshed the README status panel; semaphores remain yellow while any program-wide
  review/release-evidence obligation is pending.
- T3 remains **NOT FULLY QUALIFIED**; successful execution is not qualification.
- NC-EXT-191 (shared-descriptor concurrency / target readiness / 250 ms bound) remains OPEN.
- **Independent reviews (R1/R3, R2, R4/N17, PA/N20) were attempted via the configured review subagents
  but could not be obtained in this session (external model usage limit). They are PENDING and must be
  completed before any package attestation, release or flight claim. No review reports were fabricated.**

## Candidate manifest

`assets.sha256` is a **candidate** digest (not attested). The three PA/R2/R4 post-manifest attestation
reports are pre-registered in `docs/reviews/README.md` but not yet produced because the independent
reviews above are pending. Any covered-byte change invalidates the manifest and requires a new
freeze/review cycle.

## Related historical evidence (unchanged, annotated where used as fixtures)

- `docs/vv/evidence/2026-11-13-uart-restore-error/` — UART `send_until()` increment (2026-11-13), previously gated.
- `docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/` — S1 ingestion-failure increment (2026-11-12/13).
- Retained 2026-10-26 console transcripts inside `tests/python/test_target_run_verdict.py` and
  `tests/python/test_qemu_runner_failclosed.py` are historical fixtures, annotated as such; the L9 gap
  wording predates ICD rev 1.8 and is matched by requirement/case only.

## Preserved unrelated untracked paths (not part of this candidate)

- `.opencode/plugins/`, `opencode.json`, `tests/unit/test_sched_events.c.bak`.
