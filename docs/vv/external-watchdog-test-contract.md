# External watchdog — abstract test contract (XW-1)

| | |
| --- | --- |
| Document id | FRICK-VV-XW1-001 |
| Revision | 0.8 |
| Date | 2026-11-01 |
| Scope | the **host test contract** between the middleware (SUT) and an external independent watchdog |
| Status | **test contract only** — not a device interface specification, not a mission profile, not HIL evidence |

## 1. Purpose and standing

The current owner direction is an **external independent watchdog device**
(msg2037, 2026-10-26: external independent watchdog mandated; no production/
hardware/device selection made). SRS-REQ-073 requires an *independent* hardware
watchdog, not specifically an external one. The middleware feeds it through the
PAL seam `ff_pal_wd_hw_arm/feed/reset` behind the production API
`FF_PalWatchdogInit/Feed/Check/Reset`.

**External inventory reference (retained, not in-repo):**
`/home/nicola/.openclaw/workspace/jeeves/reports/fsw-target-io-2026-10-25/EXTERNAL-WATCHDOG-INVENTORY.md`,
SHA-256 `f5b587e4597f05fa3ba7d245829971e298421c60be4ece1c7c94e1e42d3d07f3`. This
is a code path/hash reference, not a repository Markdown link.

The earlier internal-MPFS/WDOG0 options are superseded as the *binding premise*
for this contract, but their retirement/re-scope in NC-PAL-01/NC-EXT-198 and any
hardware/architecture choice remain owner decisions. This document selects no
device, bus, protocol, register, reset route or mission value.

This document defines the **abstract contract** that the host test layer
exercises, so that the software side can be tested **before** a device is
selected. It deliberately says nothing about any real device's protocol,
registers, clock, pin or reset routing: those remain TBD (external inventory report §5.2; path/hash in §1) and are
not needed to test the middleware's obligations.

The physical protocol is abstracted as three operations and three observables.
When a device is selected, an adapter (in the PAL backend or a driver) must
satisfy the same contract; the tests in this round are written against the
contract, not against any implementation of it.

## 2. Abstract contract

| # | Requirement on the device (abstract) | Observable |
| - | ------------------------------------ | ---------- |
| C1 | **Arm**: after a successful arm the device is *armed* with a deadline derived from the requested service interval. | `state == ARMED`, `deadline_ms` set |
| C2 | **Service (feed)**: a valid service before the deadline reloads the deadline to `now + window`. | `deadline_ms` advances, `feeds` increments |
| C3 | **Independent deadline**: if the deadline is reached without a valid service, the device **expires by itself**. The SUT is not consulted and cannot prevent it. | `state == EXPIRED → RESET_ASSERTED`, `expiries` increments |
| C4 | **Reset assertion**: at expiry the device asserts the reset itself, **at most once per arm cycle**. | `resets` increments exactly once |
| C5 | **No resurrection**: a service arriving after expiry is refused and does not clear the expiry. | service returns a refusal, state unchanged |
| C6 | **Reported failures**: an arm or service failure is reported to the caller; the device never enters a silent armed state. | non-success return, `state` unchanged |
| C7 | **Explicit reset**: a reset requested by the SUT uses the same single assertion and is also at most once. | `resets` ≤ 1 |

**Boundary semantics (normative for the tests).**

- The deadline is **inclusive**: at exactly `deadline_ms` the device expires.
  At `deadline_ms − 1` it is still armed. Both are asserted (boundary pair).
- Expiry is counted **once per armed cycle**, however far the clock jumps: one
  advance step larger than the window produces exactly one expiry (the model
  exposes the count, not an expiry timestamp - see section 5.1).
- A service at exactly the deadline arrives **after** the expiry (the device has
  already expired).
- `resets` counts **device assertions**, not SUT requests; the two are distinct
  observables and the tests assert both.

## 3. Test parameter set (NOT mission values)

| Parameter | Value | Standing |
| --------- | ----- | -------- |
| Test window `FF_EXTWD_DEFAULT_WINDOW_MS` | 100 ms | **test-contract parameter** chosen to make boundary cases cheap; not a device or mission timeout |
| Windows used by the tests | 40 / 50 / 80 / 100 ms | as above |
| Virtual clock | advanced only by the harness (`ff_extwd_advance_ms`) | test mechanism |
| Clock-jump fault | `FF_EXTWD_FAULT_CLOCK_JUMP` counts an advance step larger than the window (`ff_extwd_jumps`) | test observable, so the fault mode is proven rather than labelled |

**The 8 s in `config/tables/fsw_tables.c` is an ESC-7 placeholder and is NOT
asserted as mission data anywhere in this contract or in the XW-1 tests.** No
mission timeout, cadence, window, margin or device timing is defined here.

## 4. What a device-level PASS does and does not prove

| A PASS in this contract proves | A PASS does **not** prove |
| ------------------------------ | ------------------------- |
| The middleware arms, services and reacts through the PAL seam; its calls reach the device abstraction | that any real device exists, is wired, or obeys this contract |
| Expiry is the **device's**, not the software's opinion (the clock is harness-owned) | electrical independence, clock accuracy, pin polarity, reset scope |
| Failures are surfaced and never silently degraded | the mission timeout value or cadence compliance |
| The SUT's reset path is single and at-most-once at the device | HIL, emulated-target, N18 or all-I/O coverage |
| The abstract contract is consistent with the SRS clauses in §5 | that the physical protocol was implemented correctly |

## 5. Mapping to SRS-REQ-073/074/075, with GAPs

| Requirement clause | Covered by | Status |
| ------------------ | ---------- | ------ |
| 073: arm an independent hardware watchdog with a timeout from the configuration table | `test_mw_arm_reaches_the_device` (arm reaches device; the SUT timeout becomes the device window). The table provenance of that timeout is exercised by `test_pal_watchdog.c::test_watchdog_plan_from_mission_table`, not by XW-1 | **partly covered here, partly by cross-reference** (device = model) |
| 073: feed from a dedicated task at ≤ T/4 | `test_pal_watchdog.c::test_watchdog_plan_from_mission_table` (derivation) + `test_mw_feed_reloads_the_device_deadline` (reload works) | **partially covered**: the *cadence* under real scheduling is not testable here |
| 073: a missing feed causes a hardware reset independent of the software it resets | `test_mw_stopped_feeds_expire_the_device`, `test_extwd_expiry_at_boundary` | **covered at the abstract layer**; **GAP for hardware**: no device, so no real reset |
| 073: initialisation order (init + reporter, then arm) | existing `test_es_hs_lifecycle.c` + `fsw_main.c` inspection | **covered elsewhere**; not re-asserted here |
| 074: exactly one pre-reset S5 report, ≤250 ms, lock-free | existing N22-TS5-50 evidence (`test_es_hs_lifecycle.c`), `test_link_uart.c` | **not re-asserted in XW-1** — cross-reference only; the target 250 ms remains a gap (N22-TS5-83..87) |
| 074: report failure must not prevent the reset | existing N22-TS5-50 | **covered elsewhere**; **GAP on target** |
| 075: single reset path, at most one reset per occurrence | `test_mw_single_reset_path_at_most_once`, `test_extwd_reset_at_most_once`, `test_mw_reset_after_expiry_shares_the_mechanism` (expiry then explicit request: same mechanism, device answers already-asserted) | **covered at the abstract layer.** Note: production `FF_PalWatchdogReset()` has no at-most-once guard of its own - it forwards and counts every request; the device enforces one assertion. Whether the requirement's requested-at-most-once must also be enforced software-side is an authority interpretation, not settled here |
| 075: truncated/unknown S8 command rejected without reset | existing N22-TS5-50 out-of-bound cases | **covered elsewhere** |
| Health/supervision rule that would *withhold* a feed (e.g. HS heartbeat gating) | — | **GAP**: no requirement governs it; the tests do not invent one (external inventory report §4; path/hash in §1). If the authority defines it, a case must be added |
| Device protocol, timeout value, reset routing, status readback | — | **TBD** (device selection); out of scope of this contract |

## 5.1 Model scope notes (what the model does *not* model)

- `FF_EXTWD_EXPIRED` exists as a state value but is transient: expiry moves
  straight to `RESET_ASSERTED`, because the contract couples the two (C3/C4).
- A reset requested from `DISARMED` is honoured (`RESET_ASSERTED`) and therefore
  blocks a later `arm` until the harness resets the model - a device-side
  shortcut the contract does not otherwise constrain.
- The model exposes **counters**, not timestamps: expiry-at-the-deadline is
  proven by the per-cycle count, not by an expiry instant.
- `settle()` also runs inside `ff_extwd_advance_ms()`, so an armed device is
  already expired when a service arrives at or after the deadline; the ordering
  *inside* the service call is therefore not separately observable (the
  observable property - the service is refused and nothing reloads - is pinned).
- The two device models in the tree are deliberately different:
  `ff_test_wd_model` (internal MPFS register semantics, `test_wd_host_contract`)
  and `ff_test_extwd_model` (this abstract external device, XW-1); the internal
  premise is superseded (see the external inventory report, path/hash in §1, §5.2).

## 6. Test assets and their isolation

| Asset | Path | Isolated how |
| ----- | ---- | ------------ |
| Abstract model | `tests/stubs/ff_test_extwd_model.{c,h}` | test double; no production include, no MMIO, no clock source |
| Model validation | `tests/unit/test_extwd_model.c` | links the model only (no middleware); expectations are authored literals |
| Test-only PAL adapter | `tests/stubs/ff_test_extwd_pal.c` | provides `__wrap_ff_pal_wd_hw_*`; selected with `-Wl,--wrap=` in one test binary |
| Middleware contract tests | `tests/unit/test_extwd_middleware.c` | links the **production** `pal_watchdog.c` unchanged; only the three hooks are substituted |

Production files, the production backends (`pal_posix.c`, `pal_rtems.c`), the
flight main and the mission tables are **not** modified and are not reachable
from these tests except through the public PAL API.

## 6.1 Emulated-target extension (XW-2, test-only image)

The same abstract contract is executed on the generic RTEMS/QEMU target by a
**separate test-only executable** (`frick_fsw_xw2`, CMake option
`FF_ENABLE_EXTWD_TARGET_TEST`, default OFF). Only the three device hooks are
substituted, at link time (`-Wl,--wrap=ff_pal_wd_hw_{arm,feed,reset}` →
`tests/stubs/ff_test_extwd_pal.c` → `tests/stubs/ff_test_extwd_model.c`); the
production `pal_watchdog.c`, `pal_rtems.c`, `app_wd.c`, `fsw_main.c`, the RTEMS
configuration and the mission tables are unchanged and unreferenced-hook
sections of `pal_rtems.o` stay linked for `POSIX_Init`.

Cases XW-T1…XW-T12 run in the single-threaded init context (exact, authored
clock); XW-B1 runs after `FF_EsStartAll()` and asserts feed **counts** only.
Expectations are derived from the SUT's own configured timeout as a **test
parameter**; no mission value is asserted (8 s remains an ESC-7 placeholder).
The no-wrap control (same sources, no `--wrap`) must reproduce
`[FF-PANIC] watchdog: init failed`; it is recorded as an expected failure and is
never a pass.

This extension proves the contract on the RTEMS build and nothing else: no
device, no real reset, no electrical independence, no HIL, no N18, no
timing/WCET, no mission timeout.

The build is now reproducible from the tracked tree: the CMake block in the root
`CMakeLists.txt` creates `frick_fsw_xw2` (and `frick_fsw_xw2_nowrap` under
`FF_XW2_NOWRAP_CONTROL`), the Makefile target `xw2` runs
`tools/scripts/qemu_xw2_vv.sh`, and the 13 cases are registered as work package
TS-7 (`docs/vv/test-spec/ts-7-xw2-target-contract.md`) with `PIL-QEMU` as the
environment. The runner selects an explicit `docker` or `host` backend and
records it in the run directory; a missing prerequisite exits 2, never SKIP.

## 7. Non-claims

This contract and its tests are **host evidence**, and since XW-2 they also run
on a test-only emulated-target image with the device seam substituted (§6.1). They
are not hardware, not a device, not HIL, not N18 and not all-I/O coverage; they do not
select a device, define a mission timeout, or approve the watchdog design. NC-
PAL-01 / NC-EXT-198 (the internal MPFS watchdog gate) remain open and are not
re-labelled by this document.

## History

| Rev | Date | Author | Change |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-26 | frick-fsw | Initial abstract external-watchdog host test contract: abstract operations/observables, boundary semantics, test parameters distinct from mission values, SRS-REQ-073/074/075 mapping with explicit GAPs, asset isolation and non-claims |
| 0.2 | 2026-10-26 | frick-fsw | Apply the XW-1 review: the exact-deadline service requirement is pinned by a model case and a middleware case; the clock-jump fault has a declared observable (`ff_extwd_jumps`); the expiry statement is narrowed to what the model exposes; the SRS-REQ-073 table-provenance cross-reference and the SRS-REQ-075 same-mechanism case are recorded, including that production has no software at-most-once guard; model scope notes added |
| 0.3 | 2026-10-26 | frick-fsw | Record-control fix (no content change beyond 0.2): the revision is bumped so the R3 body is not published under the same identifier as the R2 body (XW1R3-N2) |
| 0.4 | 2026-10-26 | frick-fsw | Record the owner-approved XW-2 emulated-target extension: separate test-only executable, link-time substitution of the three hooks, case set XW-T1…XW-T12 + XW-B1, expectations derived from the SUT's own timeout (test parameter), no-wrap expected-failure control, and the unchanged non-claims |
| 0.5 | 2026-10-26 | frick-fsw | XW-2 review: §7 qualified so it no longer contradicts §6.1 (host evidence plus the test-only emulated-target extension; still not device/HIL/N18) |
| 0.6 | 2026-10-26 | frick-fsw | Correct the external-premise citation defect: state the current owner direction (msg2037) and SRS-REQ-073's "independent" wording instead of citing `OWNER-WATCHDOG-OPTIONS.md` §5 (which recommends the internal WDOG0 option); record that internal-premise retirement/re-scope in NC-PAL-01/NC-EXT-198 and all hardware/architecture choices remain authority-owned. No device/protocol/reset/mission selection |
| 0.7 | 2026-10-26 | frick-fsw | MSG2040 pointer correction: add the retained external inventory report path and SHA-256 at the citation sites (not a repo link), keep the focused 0.6 premise correction, and record the authority-owned WDOG0 retirement/re-scope. No device/protocol/reset/mission selection |
| 0.8 | 2026-11-01 | frick-fsw | Record reproducibility: the XW-2 build is now produced from the tracked CMake block (`FF_ENABLE_EXTWD_TARGET_TEST`, `FF_XW2_NOWRAP_CONTROL`) and the `make xw2` target, the 13 cases are registered as work package TS-7 with explicit `PIL-QEMU` environment, and the runner records its `docker`/`host` backend while failing closed (exit 2) on missing prerequisites. No change to the contract, cases, oracles or non-claims; still not device/HIL/N18 |
