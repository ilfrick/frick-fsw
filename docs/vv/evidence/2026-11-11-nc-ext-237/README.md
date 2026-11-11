# Evidence: NC-EXT-237 Service 5 event identity is emitter-owned (2026-11-11)

Scope: verification evidence for the `FF_PusEmitEvent` correction described in
`CHANGELOG.md` (2026-11-11), `docs/status/handover.md` rev 3.68, TS-1 rev 1.10
(`N22-TS1-58`) and TS-5 rev 1.16 (`N22-TS5-47`). It records the host gates and
the mutation control executed on the same working tree.

**Not claimed:** NC-EXT-237 acceptance or closure (acceptance authority: the
project/Software PA), a controlled event-id catalogue (NC-EXT-102), T3
qualification (no N10 review of the new script), target/RTEMS timing, HIL,
radiation or flight-use authorization.

## Defect and change

Before this change `FF_PusEmitEvent(event_id, evs_type, data, len)` executed
`(void)event_id;` and forwarded the caller buffer verbatim, so event identity
was caller-supplied payload only and the argument had no effect. The emitter now
owns the PUS-C Service 5 report header `event id[2, big-endian] + type[1]`
(ICD §4) and encodes it from its own arguments; `data`/`len` carry auxiliary
data only. The encoded octets are identical to what a caller previously produced
by embedding the header itself, so the **wire format did not change** (ICD rev
1.7). Bounds: `len <= FF_PUS_MAX_TM_FRAME - 3` (`FF_BAD_SIZE` above);
`data == NULL` accepted only with `len == 0` (`FF_INVALID_ARGUMENT` otherwise).

## Artifacts

| File | Content |
| --- | --- |
| `gate-build.log` | `BUILD_DIR=build2/native` configure + build — exit 0, no warnings |
| `gate-ctest.log` | `ctest` on `build2/native` — **100% tests passed, 0 failed out of 56**, exit 0 |
| `gate-lean.log` | `lake build` in `verification/lean` — `Build completed successfully`, exit 0 |
| `gate-trace.log` | `tools/scripts/trace.py docs/requirements/traceability.yaml` — PASS (120/76/30/14/68), exit 0 |
| `gate-format.log` | pinned clang-format 18 (`frick-vv:local`) `make format-check` — exit 0 |
| `gate-license.log` | N13 `license_check.py` — OK, exit 0 |
| `mutation-check.log` | pre-fix body → `checks=106 failures=33` (exit 1); fixed body → `checks=106 failures=0` (exit 0) |

## Commands and results

```text
make build BUILD_DIR=build2/native                      # exit 0, no warnings
make test  BUILD_DIR=build2/native                      # 56/56 PASS
cd verification/lean && lake build                      # Build completed successfully
python3 tools/scripts/trace.py docs/requirements/traceability.yaml   # PASS (120/76/30/14/68)
docker run --rm -v "$PWD":/workspace -w /workspace frick-vv:local make format-check   # exit 0
python3 tools/scripts/license_check.py                  # OK
./build2/native/tests/test_pus_event_identity            # checks=106 failures=0
./build2/native/tests/test_evs_hs_health                 # checks=122 failures=0
```

`build2/native` was used because `build/native` in this worktree was configured
inside a container at `/workspace` and cannot be reconfigured from the host
(`CMakeCache.txt` records a different source directory). Format checking ran in
the pinned `frick-vv:local` image because the host `clang-format` is 23.x and the
Makefile rejects anything but 18.

## Discriminating oracle

`tests/unit/test_pus_event_identity.c` (5 cases, 106 checks; ctest
`pus_event_identity`, `TIMEOUT 60`, labels `n22;pus;nc-ext-237`):

1. `test_event_id_is_encoded_from_argument` — two emissions differing only in
   `event_id` produce different frames, each carrying its own id and type.
2. `test_event_id_is_big_endian_and_not_duplicated` — big-endian order; the
   header appears exactly once even when the auxiliary data starts with an
   id-like octet triple.
3. `test_event_id_boundary_values` — `0x0000` and `0xFFFF` are encoded as
   ordinary ids, not treated as sentinels; header-only reports are 3 octets.
4. `test_event_severity_and_type_octet` — INFO/WARNING/CRITICAL → subservice
   1/2/3, unknown → 3, raw severity octet preserved.
5. `test_event_out_of_bound_arguments` — `FF_PUS_MAX_TM_FRAME - 2` and
   `UINT16_MAX` → `FF_BAD_SIZE` with no frame; `NULL` + non-zero length →
   `FF_INVALID_ARGUMENT`; exactly `FF_PUS_MAX_TM_FRAME - 3` accepted with header
   intact and data preserved.

Mutation control: the pre-fix body fails 33 of the 106 checks (payload length
and header-octet assertions), so the oracle discriminates the defect it
documents.

## Identity of the changed files

| File | SHA-256 |
| --- | --- |
| `src/pus/pus.c` | `b0cd7b25b97a61447b9039fc80ec08f1516715e5332c5936ee108997a7129ffa` |
| `src/pus/pus.h` | `7a49f9387ce1d6e11b59af87e54f2abff11095cb1ff67fd4e16074637dfcda07` |
| `src/main/fsw_main.c` | `a2795c0ec67da488cff8ef07721cd149a551f13758c4625039911f33b06c1008` |
| `src/main/fsw_main_reqtest.c` | `e2735a8fd13e069ddfccfff104041b17650d1152cb2371f37d4cd468137534e3` |
| `tests/target/fsw_main_xw2.c` | `b12c9721fbb5d010acb95b1e0cac67d61d36aefce8d1e231cb7fafc71af14dc8` |
| `tests/unit/test_r3_gap.c` | `9cf66b0fd1bc277bc6951e1f59e85a71712b02bee088dc191824ad8cfda64f94` |
| `tests/unit/test_tm_output_chain.c` | `2d3ece75c8bb9445d5a7b3eae97799439eef1152b0beb819ffcbcb0b97d14b72` |
| `tests/unit/test_evs_hs_health.c` | `5310cc3c7399606aa2cf0bb6d04530271ab88c61c5cf69ccd7761f2ec9db277f` |
| `tests/unit/test_pus_event_identity.c` | `828b6a2227315daa4fae2dae804dd5c1360016d1efa8b15863707eac4d9024ac` |
| `tests/CMakeLists.txt` | `e23e23992c80b802a0316c1e8dd74fa29827303b76aa7d4c9ef1e6182eb5c621` |

`src/pus/pus.c` also carries the pre-existing uncommitted
`FF_PusSendVerification` request-ID change; these hashes bind the whole file at
the gate run, not only the NC-EXT-237 hunks.

## Open items

- **NC-EXT-237 OPEN** pending acceptance by the project/Software PA (owner role:
  FSW engineer; acceptance authority: Nicola; no acceptance date).
- NC-EXT-102 (controlled event-id catalogue) remains open.
- T3 **NOT FULLY QUALIFIED** for this candidate: `test_pus_event_identity.c` and
  `test_sched_events.c` have no N10 review (`tool-qualification.md` rev 1.6).
- Target/RTEMS timing, HIL (GAP-A5) and NC-PAL-01 remain open.
- No NC is closed and no commit/push is authorized by this evidence alone.
