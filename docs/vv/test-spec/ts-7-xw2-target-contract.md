# Test Specification TS-7 — XW-2 target contract (abstract external watchdog, emulated target)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                           | frick-fsw                              |
| Document id                       | FRICK-VV-TSPEC-TS7                     |
| Revision                          | 1.1                                    |
| Date                              | 2026-11-02                             |
| Norms                             | N8, N10, N11, N22                      |
| Umbrella                          | `docs/vv/comprehensive-test-spec.md` (FRICK-VV-TSPEC-001), §7 |
| Work-package index                | `docs/vv/test-spec/README.md` (FRICK-VV-TSPEC-002), work package TS-7 |
| Companion                         | `docs/vv/external-watchdog-test-contract.md` (FRICK-VV-XW1-001, XW-1/XW-2) |
| Scope                             | The middleware's obligations under the abstract external-watchdog contract, exercised on a **test-only** RTEMS/QEMU image with the three PAL device hooks substituted at link time |
| Baseline                          | Working tree 2026-11-01; N22 addendum to the 2026-10-26 XW-2 run |

### Revision history

| Rev | Date       | Author    | Changes |
| --- | ---------- | --------- | ------- |
| 1.0 | 2026-11-01 | frick-fsw | Initial issue: 13 cases (XW-T1..T12, XW-B1) for SRS-REQ-073/075; `PIL-QEMU` environment; manifest, verifiers, CMake wiring; non-claims explicit. |
| 1.1 | 2026-11-02 | frick-fsw | PA review corrections (PA-01…PA-13): `XW-G1..G5` gap table added (§5.1); enforcement split stated (§6.1); `nm` segregation counts asserted; "proven"→"asserted/observed"; QEMU timeout parameterised (`XW2_QEMU_TIMEOUT`), elapsed recorded; `FF_USE_PUS_ASN1=OFF` documented as contract-equivalent; T25 deviation `DEV-T25-01` cross-referenced. No test case or oracle change. |

> **Authority statement.** This work package defines verification *cases and
> oracles* for the current baseline. It does not assign criticality, accept
> residual risk, qualify tools, close an NC, or approve release/flight
> readiness. It is **not** norm N18 evidence: N18 remains **FAIL**
> (NC-EXT-198/NC-PAL-01). Requirement/interface gaps are recorded, never
> resolved by inventing an oracle.

## 1. Scope and method

### 1.1 In scope

Cases derived from `SRS-REQ-073` (independent hardware watchdog: arm with a
configured timeout, feed from a dedicated task, a missed feed causes a hardware
reset independent of the software it resets) and `SRS-REQ-075` (single reset
path, at most one reset per occurrence), as exercised through the **production**
API `FF_PalWatchdogInit/Feed/Check/Reset` in `src/core/pal/pal_watchdog.c`.

The SUT is the unchanged production path. Only the three platform hooks
`ff_pal_wd_hw_{arm,feed,reset}` are replaced, at LINK time, by the validated
test double `tests/stubs/ff_test_extwd_model.c` through
`tests/stubs/ff_test_extwd_pal.c` (`-Wl,--wrap=`). No file under `src/` or
`config/` is modified and no `#if` is added to production code.

### 1.2 Out of scope

Device selection, protocol, registers, timeout value, reset routing, status
readback, electrical/clock independence, reset scope, HIL, real-time behaviour
and N18. All remain TBD or open NCs (NC-EXT-198, NC-PAL-01); see §5.

### 1.3 Method

A separate test-only executable (`frick_fsw_xw2`, CMake option
`FF_ENABLE_EXTWD_TARGET_TEST`, default OFF) mirrors the flight start-up
sequence and runs:

1. `XW-T1` … `XW-T12` in the single-threaded init context, against an
   **authored clock** (the harness owns it), so every expectation is exact;
2. `FF_EsStartAll()`, where the **production** feeder arms the device;
3. `XW-B1` from a task, asserting feed **counts** only — never a deadline —
   because from that point the SUT and the harness share the scheduler.

Expectations are derived from the SUT's own configured timeout
(`FF_GetMissionCfg()->watchdog_timeout_s`) as a **test parameter**; no mission
value is asserted. The 8 s table entry remains an ESC-7 placeholder.

A **no-wrap control** (same sources, no substitution) must reproduce
`[FF-PANIC] watchdog: init failed`. It is recorded as an expected failure and is
never a pass; it discriminates the substituted-seam image from the production
path.

## 2. Environment and fidelity limits

| Item | Value |
| ---- | ----- |
| Environment | `PIL-QEMU` (generic RTEMS 6.2 `rv64imafdc` BSP on QEMU `virt`) |
| Image | `frick_fsw_xw2` (test-only; not `frick_fsw`) |
| Clock | model clock advanced only by the harness in the init thread; `XW-B1` uses the RTEMS scheduler |
| Substituted | `ff_pal_wd_hw_arm`, `ff_pal_wd_hw_feed`, `ff_pal_wd_hw_reset` (link-time `--wrap`) |
| Not substituted | `pal_watchdog.c`, `pal_rtems.c`, `app_wd.c`, `fsw_main.c`, RTEMS configuration, mission tables |
| Establishes | the middleware's obligations C1–C7 of the abstract contract, on the RTEMS build, and the discriminator provided by the no-wrap control |
| Cannot establish | a device, a real reset, reset scope, electrical/clock independence, timing/WCET, HIL, N18, mission timeout |

## 3. Cases

| Case | Requirement | Case | Class | Inputs (boundaries) | Expected result (oracle) | Env | Scope | Status |
| ---- | ----------- | ---- | ----- | ------------------- | ------------------------ | --- | ----- | ------ |
| XW-T1 | SRS-REQ-073 | Arm with the SUT-derived timeout | nominal | `FF_PalWatchdogInit(T)` with `T` = configured timeout in ms | returns `FF_SUCCESS`; device `state == ARMED`, `window_ms == T`, `deadline_ms == T`, and `FF_PalWatchdogGetTimeoutMs() == T` | PIL-QEMU | individual | covered |
| XW-T2 | SRS-REQ-073 | Service reloads the deadline | nominal | arm with `T`; advance `F = T/4`; `FF_PalWatchdogFeed()` | returns `FF_SUCCESS`; `deadline_ms == F + T`; device `feeds == 1` and `FF_PalWatchdogGetFeeds() == 1` | PIL-QEMU | individual | covered |
| XW-T3 | SRS-REQ-073 | Armed one tick before the deadline | OOB (boundary) | arm with `T`; advance `T−1` | `state == ARMED` and `expiries == 0` (no early expiry) | PIL-QEMU | individual | covered |
| XW-T4 | SRS-REQ-073 | Expiry at the deadline is inclusive and resets once | OOB (boundary) | continue from XW-T3: advance `1` | `state == RESET_ASSERTED`, `expiries == 1`, `resets == 1` | PIL-QEMU | individual | covered |
| XW-T5 | SRS-REQ-073 | One clock step across many windows yields one expiry | OOB | arm with `T`; advance `2T + 5` in a single step | `expiries == 1`, `state == RESET_ASSERTED` (no repeated expiry per window) | PIL-QEMU | individual | covered |
| XW-T6 | SRS-REQ-073 | Late service is refused and does not resurrect the expiry | OOB | arm with `T`; advance to expiry; `FF_PalWatchdogFeed()` | feed returns non-success; device `feeds` and SUT feed counter unchanged; `state == RESET_ASSERTED` | PIL-QEMU | individual | covered |
| XW-T7 | SRS-REQ-075 | One device assertion for two reset requests | OOB | arm with `T`; `FF_PalWatchdogReset()` twice | first returns `FF_SUCCESS`, second non-success; device `resets == 1`, SUT request counter `== 2` | PIL-QEMU | individual | covered |
| XW-T8 | SRS-REQ-073 | Arm failure is reported, never a silent armed state | OOB | inject `ARM_FAIL`; `FF_PalWatchdogInit(T)`; clear fault; `FF_PalWatchdogFeed()` | init returns non-success; `state == DISARMED`; feed returns `FF_NOT_READY` | PIL-QEMU | individual | covered |
| XW-T9 | SRS-REQ-073 | Failed service does not reload; expiry still follows | OOB | arm with `T`; advance `F`; inject `FEED_FAIL`; feed; clear fault; advance to the unchanged deadline | feed returns non-success; `deadline_ms` unchanged; `expiries == 1` | PIL-QEMU | individual | covered |
| XW-T10 | SRS-REQ-073 | A "successful" service cannot move the deadline | OOB | arm with `T`; inject `STUCK_FEED`; feed; advance `T + F`; clear fault | feed returns `FF_SUCCESS` but `expiries == 1` and `feeds == 1` (deadline unchanged by the service) | PIL-QEMU | individual | covered |
| XW-T11 | SRS-REQ-073 | Clock-jump mode is observable and yields one expiry | OOB | arm with `T`; inject `CLOCK_JUMP`; advance `T + 1`; clear fault | `jumps == 1` and `expiries == 1` | PIL-QEMU | individual | covered |
| XW-T12 | SRS-REQ-073 | SUT diagnostic check runs on the RTEMS clock | nominal | arm with `T`; `FF_PalWatchdogCheck()` | returns `FF_SUCCESS`; this is a diagnostic-contract case, **not** a deadline assertion | PIL-QEMU | individual | covered |
| XW-B1 | SRS-REQ-073 | Production feeder reaches the device after start-up | nominal | after `FF_EsStartAll()`, delay 3000 ms in a task | device `state == ARMED`; device feed count advanced by ≥ 1; device feed count equals `FF_PalWatchdogGetFeeds()` | PIL-QEMU | combined | covered |

## 4. Coverage and mapping

| Requirement | Cases | Level claimed |
| ----------- | ----- | ------------- |
| SRS-REQ-073 | XW-T1, T2, T3, T4, T5, T6, T8, T9, T10, T11, T12, B1 | middleware obligations C1–C4/C6 on the RTEMS build; **not** device independence |
| SRS-REQ-075 | XW-T7 | at-most-once device assertion; production `FF_PalWatchdogReset()` has **no** software at-most-once guard of its own (authority interpretation open, contract §5) |

Host counterpart (XW-1, `SIL`): `tests/unit/test_extwd_model.c` (test-double
validation) and `tests/unit/test_extwd_middleware.c` (production
`pal_watchdog.c` + substituted hooks), registered with CTest as
`extwd_model_contract` and `extwd_middleware_contract`.

## 5. Gaps and non-claims

- **No device.** The contract is executed against a test double; device
  selection, protocol, registers, timeout value and reset routing remain TBD.
- **No independence.** C3 ("the device expires by itself, the SUT is not
  consulted") is *modelled*; electrical and clock independence, and the reset
  scope (mission function vs hart), are not established (FRICK-ARCH-0023
  F-11; M-2/M-7/M-8).
- **No timing.** No WCET, jitter, interrupt latency or on-target cadence is
  asserted; `XW-B1` asserts counts, not deadlines.
- **Not N18.** N18 exercises the production path and remains **FAIL** on the
  current tree (start-up panic, NC-EXT-198, root cause NC-PAL-01). A PASS on
  this work package must never be reported as an N18 result.
- **Mission value.** The 8 s timeout remains an ESC-7 placeholder; the cases
  derive every expectation from whatever the table configures.
- **At-most-once in software.** SRS-REQ-075's "requested at most once" is
  enforced by the device in this model; whether the production PAL must also
  enforce it is an authority interpretation, not settled here.

### 5.1 Declared gaps (manifest `XW-G1`…`XW-G5`)

The manifest carries five declared gaps. They are **not executable cases**: the
verifier rejects an *undeclared* gap line but never requires a declared gap to
appear in the console, so no console line is expected for any of them. They are
recorded here so each gap has a work-package identity, a reason and the authority
that owns its closure.

| Gap | Requirement | Why it is a gap on this environment | Closure authority |
| --- | ----------- | ----------------------------------- | ----------------- |
| `XW-G1` | SRS-REQ-073 | Fatal arm/feed policy (what the SUT must do when arming fails) is not fixed by the contract. | System/FSW authority |
| `XW-G2` | SRS-REQ-074 | The pre-reset report ≤250 ms obligation needs a real reset; this image substitutes the reset hook at link time. | System/FSW authority + hardware |
| `XW-G3` | SRS-REQ-073 | Cadence, jitter and timing-margin acceptance values are authority inputs, not derived from the SRS. | System authority |
| `XW-G4` | SRS-REQ-073 | The SRS states no requirement for *withholding* the feed; the behaviour is therefore unasserted. | Requirements authority |
| `XW-G5` | SRS-REQ-073 | Reset scope (mission function vs hart) and electrical/clock independence require a device. | Hardware/safety authority |

## 6. Evidence and automation

| Item | Location |
| ---- | -------- |
| Runner | `tools/scripts/qemu_xw2_vv.sh` (`make xw2`); knobs `XW2_USE_PUS_ASN1`, `XW2_QEMU_TIMEOUT` |
| Manifest | `tools/scripts/xw2_manifest.json` (13 required cases) |
| Verifiers | `tools/scripts/target_req_verify.py`, `tools/scripts/target_run_verdict.py` |
| Image | `frick_fsw_xw2` (CMake `FF_ENABLE_EXTWD_TARGET_TEST=ON`) |
| Control | `frick_fsw_xw2_nowrap` (CMake `FF_XW2_NOWRAP_CONTROL=ON`), expected failure |
| Retained result | `docs/vv/verification-results.md`, "XW-2 target contract cycle" |

### 6.1 Enforcement split (what the automation actually checks)

| Layer | What is checked | What is **not** checked |
| ----- | --------------- | ----------------------- |
| `target_req_verify.py` | that each required case ID appears with a `PASS` token; that no undeclared `GAP`/`SKIP` line is present; per-requirement PASS counts | the *content* of a case's `detail` field on PASS (it is echoed only for FAIL/SKIP) |
| `target_run_verdict.py` | run nonce binding, QEMU exit ∈ {0, 124}, 13/13 coverage, whole-run token | wall-clock timing, boot margin, mission timeout |
| `qemu_xw2_vv.sh` | positive/control build, `nm`/`objdump` counts **asserted** (3/0 and 0/3), control shape, console presence | dynamic behaviour of the substituted hooks |
| `tests/target/app_xw2_test.c` | the actual per-case oracles (state, window, deadline, counts) | anything not expressible on the test image |

Consequently the "Expected result (oracle)" column in §3 is the oracle the case
*asserts inside the image*; it is not re-derived from the console text by the
verifier, which only carries the PASS/FAIL token.

## History

| Rev | Date | Author | Change |
| --- | ---- | ------ | ------ |
| 1.0 | 2026-11-01 | frick-fsw | First issue: 13 cases registered from the XW-2 test-only image, with the environment/fidelity limits, the no-wrap control, and the non-claims (not a device, not N18, no independence/timing). Requirement mapping added to `traceability.yaml` (`SRS-REQ-073`/`SRS-REQ-075`). |
