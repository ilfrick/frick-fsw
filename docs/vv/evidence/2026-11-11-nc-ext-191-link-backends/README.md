# Evidence: NC-EXT-191 link-backend deadline writers (2026-11-11)

Scope: verification evidence for the IP and TSN deadline-bounded link writers
(`FF_Link_t::send_until`) described in `CHANGELOG.md` (2026-11-11),
`docs/status/handover.md`, TS-2 rev 0.10 (`N22-TS2-82/83`) and
`docs/requirements/traceability.yaml` (SRS-REQ-060/061/074). It records the host
gates, the RTEMS cross build and the Renode (N18) run executed on the same
working tree, plus the mutation control that shows the new cases discriminate.

**Not claimed:** closure of NC-EXT-191 (only its backend-coverage half is
addressed; the UART shared-descriptor concurrency and target readiness-semantics
parts remain open and authority/design owned), T3 qualification (no N10 review of
the changed test scripts at these hashes), a target 250 ms bound (GAP-A5), HIL,
radiation or flight-use authorization.

## Defect and change

`fsw_main.c` installs `FF_LinkSendUntil` as the PUS panic reporter
(SRS-REQ-074). `FF_LinkSendUntil` returns `FF_NOT_IMPLEMENTED` when the selected
backend's `send_until` is `NULL`, and both `link_ip.c` and `link_tsn.c` left it
`NULL`, so the pre-reset S5 report could never be delivered on two of the four
backends (loopback and UART already implemented it).

Both backends now implement the same deadline pattern already used by
`loop_send_until`/`uart_send_until`, with the shared validation factored into
`ip_check_tx()`/`tsn_check_tx()`:

1. validate readiness and arguments (`FF_NOT_READY` / `FF_INVALID_ARGUMENT`);
2. reject an already-expired deadline **before** transmitting (`FF_TIMEOUT`);
3. perform the transfer;
4. re-check the deadline and report `FF_TIMEOUT` on overrun.

Documented limitation (also stated in `link.h`): `FF_NetStack_t::send` and
`FF_EthMac_t::transmit` are synchronous submissions, so a stalled driver cannot
be preempted; the deadline is an entry/post-check bound, not a preemption
guarantee. The `ip_check_tx()` refactor additionally rejects
`data == NULL && len != 0` in the plain `ip_send()` writer, which previously
forwarded that argument to the stack (defect fix; the other backends already
rejected it).

## Artifacts

| File | Content |
| --- | --- |
| `gate-build.log` | `BUILD_DIR=build2/native make build` — exit 0 |
| `gate-ctest.log` | `BUILD_DIR=build2/native make test` — **100% tests passed, 0 failed out of 56**, exit 0 |
| `gate-lean.log` | `lake build` in `verification/lean` — `Build completed successfully`, exit 0 |
| `gate-trace.log` | `tools/scripts/trace.py docs/requirements/traceability.yaml` — PASS (120/76/30/14/68), exit 0 |
| `gate-format.log` | pinned clang-format 18.1.3 (`frick-vv:local`) `--dry-run --Werror` over `src config tests` — exit 0 |
| `gate-license-gsw.log` | `make gsw-selfcheck` (GSW/metrics/N13 license policy) — exit 0 |
| `gate-smoke.log` | `BUILD_DIR=build2/native make smoke` — `[smoke] PASS (banner + 7 applications registered)` |
| `gate-coverage-metrics.log` | `make metrics` (coverage build + CTest 56/56 + `coverage.json`) — exit 0 |
| `gate-rtems-cross-build.log` | RTEMS 6.2 `riscv/mpfs64imafdc` cross build in `frick-rtems-toolchain:local`, `FF_ENABLE_RENODE_TEST=ON` — exit 0, `frick_fsw` linked |
| `gate-renode-n18.log` | `make renode` on the freshly built ELF — `[renode] PASS (RENODE_TEST: PASS; … exit code=0)` |
| `mutation-check.log` | entry-check and post-check removals each produce 1 failure per binary; restored sources 0 failures |

## Commands and results

```text
BUILD_DIR=build2/native make build        # exit 0
BUILD_DIR=build2/native make test         # 56/56 PASS
cd verification/lean && lake build        # Build completed successfully
python3 tools/scripts/trace.py docs/requirements/traceability.yaml   # PASS (120/76/30/14/68)
docker run --rm --user $(id -u):$(id -g) -v "$PWD":/workspace -w /workspace frick-vv:local \
  bash -c 'clang-format --dry-run --Werror …'                       # exit 0 (18.1.3)
make gsw-selfcheck                        # OK
BUILD_DIR=build2/native make smoke        # PASS (7 applications)
make metrics                              # exit 0, 56/56, coverage.json written
docker run --rm --user $(id -u):$(id -g) -v "$PWD":/workspace -w /workspace \
  frick-rtems-toolchain:local cmake --build build/renode …          # exit 0
make renode                               # RENODE_TEST: PASS
```

`build2/native` was used because `build/native` in this worktree was configured
inside a container at `/workspace`. `build/renode` is the container-configured
RTEMS tree (source mounted at `/workspace`), rebuilt for this change so the N18
run does not rest on a stale ELF.

## Discriminating oracle

New cases (registered in the existing binaries, so the CTest count stays 56):

- `tests/unit/test_link_tsn.c::test_tsn_send_until_deadline` (149 checks total):
  `send_until != NULL`; in-budget delivery (60-octet padded frame reaches the
  mock MAC); expired-on-entry `FF_TIMEOUT` with **no** transmission; post-transfer
  overrun `FF_TIMEOUT` with the frame transmitted; `NULL` data / `len == 0` /
  `len > FF_LINK_TSN_RX_FRAMESZ` → `FF_INVALID_ARGUMENT`.
- `tests/unit/test_link_ip.c::test_ip_send_until_deadline` (93 checks total):
  same matrix for the UDP datagram path, plus the `NULL` + non-zero-length
  rejection in both `send_until` and `send`.

Determinism: the tests freeze the monotonic clock with the host-only
`FF_PalMonotonicTestSet()` seam and the mocks advance it during the transfer
(`g_mock_transmit_advance_ns` / `g_mock_net_send_advance_ns`), so no wall-clock
timing is involved. Both cases unfreeze the clock at the end.

Mutation control (`mutation-check.log`):

| Variant | `test_link_tsn` | `test_link_ip` |
| --- | --- | --- |
| unmutated | 149 checks, 0 failures | 93 checks, 0 failures |
| entry deadline check removed | 1 failure (`g_captured_len == 0`) | 1 failure (`g_mock_net_tx_len == 0`) |
| post-transfer re-check removed | 1 failure (`… == FF_TIMEOUT`) | 1 failure (`… == FF_TIMEOUT`) |
| sources restored | 149 checks, 0 failures | 93 checks, 0 failures |

## Identity of the changed files

| File | SHA-256 |
| --- | --- |
| `src/transport/link/link.h` | `c06d2fadb2e1051961caf0df02b3dd5540639ffd84ebde6eb413c2aaaa699bda` |
| `src/transport/link/link_ip.c` | `c62f99751b77e8dd2258c4c032d3b0278fd4a5c12c854616753381c064461324` |
| `src/transport/link/link_tsn.c` | `d019bb9dd21d353806ca2ec2d8324cd4c3563ed48c2d3d2f05356ebd7fea56e0` |
| `tests/unit/test_link_ip.c` | `8153e6e140ae0aa1228c092638e16cae760e4a4fdbeb3e3ad27f00269bfcba20` |
| `tests/unit/test_link_tsn.c` | `0b2cac2ac9dbc36291adfa71faf710a8df821455fa73f0784c9e24baebe4e382` |
| `build/renode/frick_fsw_renode.elf` (N18 run) | `55188f5a460bf7c2a97f4540bd8a12f571da56d276394c68a1be25b6a9251f84` |
| `build/coverage/coverage.json` | `02d82862c7da0b2d24c1e24ebcedfbe9c81e8ae08095eed759f6d5973f72760d` |

Coverage after this change: 15328/32948 statement lines (46.5%) — a project
indicator, not an ECSS threshold (GAP-A4).

## Open items

- **NC-EXT-191 remains OPEN**: the backend-coverage half is implemented and
  host-verified; the UART shared-descriptor concurrency and target
  readiness-semantics parts are unchanged and not claimed.
- Target 250 ms panic-report bound, on-target link behavior and timing remain
  GAP-A5 / N22-TS2-78 (HIL scope).
- T3 **NOT FULLY QUALIFIED** for this candidate: `test_link_ip.c` and
  `test_link_tsn.c` have no N10 review at these hashes
  (`tool-qualification.md` rev 1.7).
- NC-PAL-01 and NC-EXT-239 are untouched by this increment.
- No NC is closed and no release/flight approval is implied by this evidence.
