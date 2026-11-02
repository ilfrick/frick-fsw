# Verification Results: test_renode_vv_verdict.sh

## Test Execution (HEAD 0ba7459)

- **SHA-256:** 8787631746b00446c842136b140d59c9bf63572e275e542b81ecb865d275e00c ✓
- **Source integrity:** tools/scripts/renode_vv_verdict.sh unchanged vs 0ba7459 ✓

### Container execution (frick-vv:local)
```
17 checks, 0 failures
ok   - C2 the PASS line carries the marker
ok   - D1 current-tree run without the marker fails (rc=1)
ok   - D2 the FAIL reason distinguishes the startup fatal exit
ok   - E1 missing run directory is a usage error (rc=2)
ok   - F1 the runner does not reference the historical fixed log paths
ok   - F2 the runner creates a fresh per-run directory with mktemp -d
ok   - F3 the runner delegates the verdict to the fail-closed unit
renode_vv_verdict: checks=17 failures=0
```

### Host direct execution
```
Identical output: 17 checks, 0 failures
Last 8 lines: same as container path
```

## Mutations (sandbox, /tmp, not in repo)

### (a) Remove chmod 711 "${B2}"
- B2 directory: default 775 permissions (vs expected 711)
- Non-root user: can traverse, cannot read chmod 000 uart-console.log
- Verdict: `"[renode] FAIL: run capture unreadable ... fail closed"`
- B2 discrimination between execution environments affected, but fail-closed behavior preserved

### (b) Force-setprev probe always fail
- setpriv as current user: `setresuid failed: Operazione non permessa` (exit code 127)
- Direct verdict run (non-root path): correctly fails closed
- If root with setpriv failing: test SKIPS B2/B2b with explicit message
- "Different B2 outcome": skip vs pass/fail reporting

## Verdict
**PASS WITH COMMENTS** — test passes identically on container and host (17 checks, 0 failures). Mutations confirm fail-closed design and correct B2 discrimination logic. No repo file writes.

---

# Verification Results: XW-2 target contract (2026-11-02)

| Field | Value |
| --- | --- |
| Document ID | FRICK-VV-XW2-20261102 |
| Revision | 1.0 |
| Date | 2026-11-02 |
| Author | FSW engineer |
| Norms | N8 (docs current), N11 (evidence), N15 (cycle), N22 (requirement-based) |
| Base HEAD | `65e4a91` (the committed XW-2 increment) |

Work package: `docs/vv/test-spec/ts-7-xw2-target-contract.md` (TS-7, Rev 1.1).
Requirement links: `SRS-REQ-073`, `SRS-REQ-075`.

> **NOT norm N18.** The three PAL device hooks (`ff_pal_wd_hw_arm/feed/reset`)
> are substituted at link time by the validated double
> `tests/stubs/ff_test_extwd_model.c` through `tests/stubs/ff_test_extwd_pal.c`.
> A PASS here proves the middleware's obligations under the abstract contract
> on the RTEMS build; it does not establish a device, a real reset, reset scope,
> electrical/clock independence, timing/WCET or mission timeout. N18 remains
> **FAIL** (NC-EXT-198, root cause NC-PAL-01).

## XW-1 host counterpart (SIL)

| Target | Result |
| ------ | ------ |
| `test_extwd_model` | `checks=73 failures=0`, exit 0 |
| `test_extwd_middleware` | `checks=49 failures=0`, exit 0 |
| CTest `extwd_model_contract`, `extwd_middleware_contract` | PASS (labels `n22;fdir;watchdog;xw1`) |
| Full host CTest (`build/xw1-host`) | **50/50 PASS**, `ctest` exit 0 |

## XW-2 emulated target (`make xw2` / `tools/scripts/qemu_xw2_vv.sh`)

- **Command:** `bash tools/scripts/qemu_xw2_vv.sh > build/gate-evidence/xw2-run.log 2>&1` (direct full-log redirection; exit captured in `build/gate-evidence/xw2-run.exit`; no `tee`/`tail` in the capture path).
- **Exit status:** `0`.
- **Run directory (evidence of record):** `build/xw2-run-20261101T172050Z`.
- **Base HEAD at execution:** `65e4a91` (working tree dirty; this run matches the committed increment).
- **ELF (positive, wrapped):** `0972d96a0e4f549ae3c956af2edb4c98c94ea03c657ef0a2c5a60fe3c710f251`
- **ELF (no-wrap control):** `b695b1ad4c92647f28017c1e765f456a9dcae0b6bfec1d6cad7e3223d6011926`
- **Backend:** `docker` (recorded in run directory `backend.txt`).
- **Toolchain image:** `frick-rtems-toolchain:local` `sha256:8cdb7b38d6b2696cc8c00acf6b622f5f8d9c3728fbf6be3811df28533843a896` (recorded in `toolchain-id.txt`).
- **PUS ASN.1 backend:** `OFF` (contract-equivalent; recorded in `pus-asn1.txt`).
- **QEMU timeout:** `25` s (recorded in `qemu-timeout.txt`).
- **Positive elapsed (wall-clock):** recorded in `positive-elapsed-ns.txt`.

```
Per requirement:
  SRS-REQ-073: PASS=12
  SRS-REQ-075: PASS=1
COVERAGE: 13/13 required cases
QEMU-PROCESS: exit 124 (accepted)
WHOLE-RUN: PASS (no panic/fatal marker in the console)
RESULT: PASS
[xw2] no-wrap control: EXPECTED FAILURE reproduced: watchdog init panic + component verifier exit 1
```

The no-wrap control is a **separate CMake target** (`frick_fsw_xw2_nowrap`,
`FF_XW2_NOWRAP_CONTROL=ON`), not the same binary unwrapped. Wrapper segregation
(`nm`/`objdump` counts **asserted**: 3 wrapper call sites, 0 real-hook call sites
in the positive image; 0 wrappers and 3 real-hook call sites in the
flight-shaped image) is in `nm-segregation.txt` in the run directory.

### Retained evidence archive

The complete run directory (38 files, all hashes verified) is archived at
`docs/vv/evidence/2026-11-01-xw2-target-final/` with `INDEX.md` and
`assets.sha256`. The prior runs `20261101T170112Z` and `20261101T170759Z` are
retained as superseded history in the same archive.

### XW-1 `format` and `LastTest.log` capture (PA-08)

The `checks=73/49` figures above are corroborated by the transient CTest
artifact `build/xw1-host/Testing/Temporary/LastTest.log` (exit 0, 50/50 PASS),
which is now retained in the evidence archive. `format-apply` and `format-check`
exit statuses are captured explicitly as `format-apply.exit` and
`format-check.exit` (both 0) in the archive; the prior archive's assertion of
exit 0 was not backed by a capture file.

## Verdict

**XW-2 target contract: PASS** for the 13 registered cases and the negative
control, on the test-only emulated-target image. This is `PIL-QEMU`
middleware-contract evidence under `SRS-REQ-073`/`SRS-REQ-075`; it is **not**
N18, not HIL and not device evidence. The 8 s table timeout remains an ESC-7
placeholder.