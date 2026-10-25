# N18 emulated-target start-up failure: watchdog arm path and bounded integration proposal

|                                   |                                        |
| --------------------------------- | :------------------------------------- |
| Project                           | frick-fsw                              |
| Document id                       | FRICK-ARCH-0023                        |
| Revision                          | 1.5                                    |
| Date                              | 2026-10-22                             |
| Status                            | **Proposal for owner architecture decision — nothing implemented** |
| Norms                             | N2 (English), N8 (docs), N16 (history), N21 (findings) |

## 1. Why this document exists

The N18 Renode functional validation of the current tree **fails**: the
`RENODE_TEST: PASS` marker is absent because the target software aborts during
start-up. The failure was reproduced three times (see
`docs/vv/evidence/2026-10-21-s8-fdir-gate-verification.log`) and is **not** a
runner artefact: the corrected runner reports

```
[renode] FAIL: marker absent and the target reported a startup fatal exit
('EXIT STATUS NOT ZERO'); container status=exited exit=0
```

The agent has **not** changed the watchdog behaviour, has **not** added a
degraded or test flight mode, has **not** fabricated a marker, has **not**
changed requirements and has **not** implemented a hardware binding. This
document records the exact trace, the two bounded integration options and the
decisions the owner must take.

## 2. Exact trace: from the gate to the missing binding

| Step | Location | Behaviour |
| --- | --- | --- |
| 1 | `Makefile:129-133` (`renode-image`) | `FF_PLATFORM=rtems`, `FF_ENABLE_RENODE_TEST=ON`, `FF_USE_PUS_ASN1=ON`, toolchain `cmake/toolchains/rtems-riscv-mpfs.cmake` (BSP `mpfs64imafdc`, `-march=rv64imafdc -mabi=lp64d -mcmodel=medany`, `-qrtems`) |
| 2 | `tools/docker/renode/frick-renode.resc` | loads `platforms/boards/beaglev-fire.repl` → `platforms/cpus/polarfire-soc.repl`; captures the console from `sysbus.mmuart1`; runs **only u54_1** (`e51`, `u54_2/3/4` halted) |
| 3 | `src/apps/app_wd.c:85-87` | `FF_AppWdMain`: `if (FF_PalWatchdogInit(timeout_ms) != FF_SUCCESS) FF_PalPanic("watchdog: init failed")` |
| 4 | `src/core/pal/pal_watchdog.c:45-48` | `FF_PalWatchdogInit` propagates the backend result (the owner-approved ESC-2 change) |
| 5 | `src/core/pal/pal_rtems.c:73-77` | **`ff_pal_wd_hw_arm` returns `FF_NOT_IMPLEMENTED`** ("NOT WIRED on RTEMS … NC-PAL-01") — this is the exact missing binding |
| 6 | `src/core/pal/pal_posix.c:682,692` | the panic requests the reset (`FF_PalWatchdogReset`, also `FF_NOT_IMPLEMENTED` at `pal_rtems.c:84-88`) and then `abort()` |
| 7 | target | RTEMS fatal exit (`exit source: 5 (RTEMS_FATAL_SOURCE_EXIT)`, `*** EXIT STATUS NOT ZERO ***`) → the Renode test application never reaches its marker |

Console excerpt of the failing run
(`docs/vv/evidence/2026-10-21-s8-fdir-n18-current-uart-console.log`):

```
frick-fsw: 8 applications registered
app RENODE_TEST started
watchdog: NOT WIRED on RTEMS (timeout 8000 ms) - NC-PAL-01
[FF-PANIC] watchdog: init failed
watchdog: reset requested but MPFS binding NOT WIRED - NC-PAL-01
*** EXIT STATUS NOT ZERO ***
```

The 2026-10-20 tree **passed** the same gate because it predates the
arm-error propagation: `FF_PalWatchdogInit` then discarded the backend result
and returned success, so the feeder continued and the test application emitted
its marker. The pass therefore never evidenced watchdog behaviour — it is not
evidence for the current tree.

## 3. Does the pinned emulator already contain an applicable model?

**Yes — the model exists; only the software-side binding is missing.** In the
prescribed image (`antmicro/renode@sha256:7905c32…`), the platform
`platforms/cpus/polarfire-soc.repl` (imported by `beaglev-fire.repl`) declares
five watchdog peripherals:

| Model instance | Base address | IRQ wiring | Relevance to the N18 run |
| --- | --- | --- | --- |
| `wdog0` | `0x20001000` | `RefreshEnable → plic@100 \| e51@26`, `Trigger → plic@105 \| e51@25` | E51 (monitor) — not the running hart |
| `wdog1` | `0x20101000` | `RefreshEnable → plic@101 \| u54_1@26`, `Trigger → plic@106 \| u54_1@25 \| e51@24` | **the instance whose IRQs reach the running hart u54_1** |
| `wdog2` | `0x20103000` | `… \| u54_2@26` | u54_2 — halted by the `.resc` |
| `wdog3` | `0x20105000` | `… \| u54_3@26` | u54_3 — halted |
| `wdog4` | `0x20107000` | `… \| u54_4@26` | u54_4 — halted |

> **Issuer fact that bounds this table (FRICK-ARCH-0024 F-18, MSS Technical
> Reference Manual rev S §3.12.12):** "**Only watchdog_0 can reset the MSS.**
> The other watchdogs can only generate interrupts to the E51 core." The table
> above is therefore an **emulator** fact about interrupt routing — **not** a
> protection choice. A watchdog owned by a U54 core can *detect* a hang of that
> core but **cannot reset the MSS**; the only MSS-reset-capable watchdog is
> `wdog0`, the E51's.

All five use `Timers.MPFS_Watchdog` with `frequency: 156250 //this value is
estimated from the comments in the code`.

**Limits of that evidence (must not be over-read):**

- the model frequency is an **estimate** in the platform file, not a
  datasheet value, so any timing derived from it is not a target timing;
- the upstream model carries a register-locking TODO, so its register semantics
  are not proven equivalent to the MSS watchdog in the RTPFS160ZT;
- the model's presence says nothing about the **software** register sequence
  (TIME write-once, `REFRESH` key `0xDEADC0DE`, `FORCE` key `0xDEAD`, the
  optional MVRP forbidden window) documented in the public Microchip
  `mss_watchdog.h` header — that header is on a mutable `main` branch and is
  an audit lead, not a controlled baseline;
- nothing in the current tree programs these registers: the whole MSS watchdog
  path is `FF_NOT_IMPLEMENTED` (NC-PAL-01).

## 4. Two bounded integration options

### Option A — wire the MSS watchdog binding against the existing model (recommended end state)

Implement `ff_pal_wd_hw_arm/_feed/_reset` in `src/core/pal/pal_rtems.c` for the
**MSS watchdog instance that the owner selects** (FRICK-ARCH-0024 rev 1.5,
agenda item C). The earlier instance recommendation (`wdog1`, the one whose
IRQs reach the running hart in the N18 configuration) is **withdrawn**: per the
issuer's MSS Technical Reference Manual rev S, **only `wdog0` (the E51's
watchdog) can reset the MSS**, while a watchdog owned by a U54 core can only
interrupt the E51. For the **emulator**, `wdog1` (`0x20101000`) is the instance
whose IRQs reach the running hart — an emulator routing fact, not a protection
choice. The register sequence itself is the
public Microchip HAL header — `TRIGGER` (timeout value), `MSVP`
(forbidden-window value) and `TIME` (reload value) **in that order** (the
cited source states that a write into the 0x00–0x0c range locks `TRIGGER`,
`MSVP` and `TIME`, and the header ties the freeze to the `TIME` write, so the
order is what makes the configuration effective), then `CONTROL`
(forbidden-window enable), then `REFRESH` with the key `0xDEADC0DE` for a feed
— and validate it
first against the emulator model, then on target silicon/HIL. A software reset
request is **not** an immediate reset: the documented `FORCE` semantics apply
only after a timeout, and any other `FORCE` value raises the watchdog NMI
sequence. The exact sequence, the counter encoding and the reset scope are
specified in FRICK-ARCH-0024 rev 1.5 and remain owner decisions (agenda items C, D and E).

- **Evidence available:** the model exists in the pinned image with the exact
  instance/address/IRQ wiring; the vendor HAL header documents the register
  block and the refresh/force keys; the PAL already has the portable arm/feed/
  expiry/reset contract and a simulated backend.
- **Required owner decisions:** which instance is the mission watchdog
  (note that only `wdog0`/E51 can reset the MSS and a U54-owned watchdog can only
  interrupt the E51, so this is an **architectural** choice, not a numbering one —
  FRICK-ARCH-0024 agenda item C);
  whether the MVRP forbidden window is enabled and with which values; the
  counter/timeout encoding (the model frequency is an estimate — the register
  values must come from the **target** datasheet, not from the model); the
  reset scope the software may request; and the verification plan (emulator
  first, then HIL) with its acceptance criteria.
- **Explicit limits:** this is new hardware-binding work and **is not
  implemented here**; it needs a controlled target datasheet/BSP reference
  before any register value is chosen; the model alone cannot establish
  fidelity, and the target/HIL evidence remains absent until then; the 8 s
  timeout and the inter-feed cadence stay unjustified placeholders until the
  refresh-window and margin questions (NC-EXT-187) are answered.
- **Effect on N18:** the target would arm the modelled watchdog and survive
  start-up, so the framework's emulated functional path becomes testable
  again — but the marker would then evidence a *bound* watchdog, not a proven
  one, until HIL evidence exists.

### Option B — keep the N18 gate blocked until the hardware inputs exist (no degraded operation)

Keep the binding unwired and keep the watchdog's fatal behaviour exactly as
approved: the emulated target continues to abort at start-up because the MPFS
binding is unimplemented, the `RENODE_TEST: PASS` marker stays absent, and the
N18 gate stays **FAIL** until the controlled target inputs exist and the
binding is implemented and verified. This option changes no flight behaviour,
introduces no "capability absent ⇒ non-fatal" mode and fabricates no marker.

> **Correction (2026-10-21, after the owner's decision Telegram 1406/1407):**
> an earlier revision of this proposal offered a variant that made the
> unimplemented binding non-fatal so that the emulated target could start and
> the framework's functional path could be tested. **The owner has rejected
> starting without a watchdog**, so that variant is withdrawn and is not an
> option. The only viable alternative to implementing the binding is to keep
> the N18 gate blocked until the hardware inputs exist.

- **Evidence available:** the current start-up failure is fully characterized
  (FRICK-ARCH-0024 §2, §3 F-9); the corrected N18 runner reports it as FAIL
  with a distinct reason, so the blockage is visible and not a tooling
  artefact.
- **Required owner decisions:** that the N18 gate is expected to remain FAIL
  for the duration (and what interim evidence is acceptable in its place);
  the schedule/priority of the controlled target-document collection
  (FRICK-ARCH-0024 §6, M-1..M-10); and that no partial or "framework-only"
  PASS may be recorded for the N18 gate in the meantime.
- **Explicit limits:** the emulated target never reaches the framework's
  functional path, so the N18 gate provides **no** functional evidence while
  this option stands; the flight target still has no hardware watchdog
  (NC-PAL-01) and no HIL evidence; the watcher must not be weakened to make
  the gate pass.
- **Effect on N18:** the gate stays **FAIL**, deliberately and visibly, until
  path A is implemented and verified.

### Recommendation

**Option A is the required end state and is now the authorized path** (owner
Telegram 1406/1407, 2026-10-21), to be executed **starting from the
controlled specification** `docs/architecture/mpfs-mss-watchdog-integration-spec.md`
(FRICK-ARCH-0024) and the owner's approval of its decision table. **Option B is
the only alternative and means the N18 gate stays blocked and FAIL** until the
hardware inputs exist — the owner has rejected starting without a watchdog, so
no degraded or non-fatal "capability absent" operation is on the table.

## 5. What is explicitly not decided here

- the report obligation, mandatory-text envelope and the bounded-formatting
  strategy (NC-EXT-184/185) — unchanged and still open;
- the 250 ms local-attempt target and its target/WCET evidence
  (NC-EXT-186) — unchanged;
- the watchdog timeout value, the feed cadence, the refresh window and the
  execution/jitter margin (NC-EXT-187) — unchanged;
- the hardware binding, the register values and the verification plan — **not**
  implemented and **not** selected here;
- the N18 result on the current tree — **FAIL**, and it stays FAIL until an
  owner decision is taken and the corresponding work is done and reviewed.

## 6. Evidence index

| Artifact | Content |
| --- | --- |
| `docs/vv/evidence/2026-10-21-s8-fdir-gate-verification.log` | the false-PASS transcript (stale 2026-10-20 log), the truthful re-execution, the corrected-runner run, the Lean gate and the NC-EXT-197 before/after TSan pair |
| `docs/vv/evidence/2026-10-21-s8-fdir-n18-current-process.log` | the current run's Renode process log (no marker) |
| `docs/vv/evidence/2026-10-21-s8-fdir-n18-current-uart-console.log` | the current run's UART console (the fatal exit) |
| `docs/vv/evidence/2026-10-20-n18-process.log`, `-console.log` | the genuine recorded 2026-10-20 successful run (used as the positive regression fixture) |
| `tools/scripts/renode_vv.sh`, `tools/scripts/renode_vv_verdict.sh` | the corrected runner and its fail-closed verdict unit |
| `tests/shell/test_renode_vv_verdict.sh` | the regression cases (stale marker, missing/unreadable/empty capture, recorded success, recorded startup fatal exit) |

## History

| Revision | Date | Change |
| --- | --- | --- |
| 1.0 | 2026-10-21 | Initial record of the N18 start-up failure, the exact arm-path trace, the emulator model inventory, and the two bounded integration options with the required owner decisions and limits. Nothing implemented; no policy decided. |
| 1.1 | 2026-10-21 | Owner decision Telegram 1406/1407 applied: path A is authorized starting from the controlled specification (FRICK-ARCH-0024); the former Option B variant that made the unimplemented binding non-fatal is **withdrawn** (the owner rejects starting without a watchdog) and Option B now means keeping the N18 gate blocked and FAIL until the hardware inputs exist. No implementation, register write or policy change. |
| 1.2 | 2026-10-22 | Correct the Option A sequence description (the register write order, and the fact that a software reset request is not an immediate reset) and point the detailed specification, counter encoding and reset scope to FRICK-ARCH-0024 rev 1.1, which carries the owner decision table. No implementation, register write or policy decision. |
| 1.3 | 2026-10-22 | Parent acceptance correction: the cross-references to the specification are aligned to its rev 1.2 and to the consolidated owner agenda (items A–F instead of the former D-numbers), and the Option A sequence wording is unchanged in substance. The specification's rev 1.2 records the retained pinned-image model probe, so the emulator's usable scope is now measured rather than asserted. No implementation, register write, production change or policy decision. |
| 1.4 | 2026-10-22 | Final-delta correction after FRICK-REVIEW-0426: the register-lock wording now follows the cited sources (the lock follows a write into the 0x00–0x0c range and the header ties the freeze to the `TIME` write) instead of the earlier "lock after the first write"; the specification cross-reference is aligned to rev 1.3 and to the A–F agenda. No implementation, register write, production change or policy decision. |
| 1.5 | 2026-10-22 | Correction after the confirmation addenda of FRICK-ARCH-0024 rev 1.4 (FRICK-REVIEW-0428 embedded safety, FRICK-REVIEW-0429 V&V): the superseded instance selection is **withdrawn** from Option A — per the issuer's MSS TRM rev S only `wdog0` (E51) can reset the MSS, and a U54-owned watchdog can only interrupt the E51, so the instance is now an **owner decision** (FRICK-ARCH-0024 rev 1.5, agenda item C) and the emulator's `wdog1` routing is stated as an emulator fact only; the issuer fact is quoted next to the emulator instance table; the specification cross-reference is aligned to rev 1.5. No implementation, register write, production change or policy decision. |
