# Driver development and integration guide

| Document id | FRICK-ENG-0003 |
| ----------- | -------------- |
| Revision    | 0.2 |
| Date        | 2026-10-10 |
| Audience    | humans and agents extending frick-fsw with low-level drivers |

This guide explains how to add a **low-level driver** to frick-fsw (a new link,
a board/BSP peripheral such as a PPS input, a new PAL/RTOS port) so that it is
buildable, testable, traceable and verifiable under the project norms (N1-N20).
It mirrors the way NASA cFS separates the OSAL (our PAL) and the PSP/BSP from
the application code.

Read first: `AGENTS.md` (contribution checklist), `docs/rules.md` (norms),
`docs/architecture/overview.md` (context), `docs/tooling.md` (tools).

## 1. Extension points

frick-fsw hides hardware behind three narrow interfaces. Almost every driver is
implemented as a vtable plus a default backend selected by the platform.

| Interface | Header | Purpose | Reference backends |
| --------- | ------ | ------- | ------------------ |
| PAL | `src/core/pal/pal.h` | tasks, mutexes, queues, time, console | `pal_posix.c` (host + RTEMS POSIX), `pal_rtems.c` (RTEMS bootstrap) |
| Link | `src/transport/link/link.h` | move CCSDS frames in/out (UART, SpaceWire, radio, TCP/IP) | `link_loopback.c`, `link_uart.c` |
| Board/BSP | `src/bsp/mpfs/board.h` | board bring-up and platform quirks | `src/bsp/mpfs/board.c` |

The default backend is chosen at build time in `src/transport/link/link.c`
(`FRICK_PLATFORM_MPFS` → UART, otherwise → loopback). The active link can be
swapped at run time with `FF_LinkSelect(&my_backend)`.

## 2. Adding a link driver (the most common case)

A link is any byte-oriented device that carries a CCSDS transfer frame. The
contract is `FF_Link_t` (`src/transport/link/link.h`):

```c
typedef struct {
    const char *name;
    FF_Status_t (*init)(void);
    FF_Status_t (*send)(const uint8_t *data, uint16_t len);
    FF_Status_t (*recv)(uint8_t *data, uint16_t cap, uint16_t *len, uint32_t timeout_ms);
} FF_Link_t;
```

Steps:

1. Create `src/transport/link/link_<name>.c` with a `static` implementation of
   `init`, `send`, `recv` and a public `const FF_Link_t FF_Link<Name>Backend`.
   Follow the existing files as templates (`link_uart.c`, `link_loopback.c`).
2. Keep the driver **non-blocking or bounded**: `recv` must honour `timeout_ms`
   and never spin forever; `send` must not allocate memory after init
   (norm N10: no dynamic allocation in flight code).
3. Add the source file to `FF_CORE_SOURCES` in `CMakeLists.txt`.
4. Optionally make it the default for a platform in `FF_LinkInit`
   (`src/transport/link/link.c`), or leave it opt-in via `FF_LinkSelect`.
5. Add any compile-time knobs to `config/fsw_config.h` and the platform headers
   (`config/platform/platform_native.h`, `platform_mpfs.h`), not to the driver.
6. Add a unit test that exercises it over the loopback shim or with a fake
   register file (see §6).

### 2.1 Worked example (skeleton)

```c
/* src/transport/link/link_mybus.c */
#include "link.h"
#include "fsw_config.h"
#include "core/pal/pal.h"

static FF_Status_t mybus_init(void) { return FF_SUCCESS; }

static FF_Status_t mybus_send(const uint8_t *data, uint16_t len)
{
    /* write len bytes to the device, bounded retries, no malloc */
    (void)data; (void)len;
    return FF_SUCCESS;
}

static FF_Status_t mybus_recv(uint8_t *data, uint16_t cap, uint16_t *len, uint32_t timeout_ms)
{
    (void)data; (void)cap; (void)timeout_ms;
    *len = 0u; /* nothing yet */
    return FF_NOT_READY;
}

const FF_Link_t FF_LinkMybusBackend = { "mybus", mybus_init, mybus_send, mybus_recv };
```

## 3. Adding a board driver (for example a PPS input)

Board-specific peripherals live under `src/bsp/<board>/`. `FF_BoardInit` is
called once from the main sequence (`src/main/fsw_main.c`) before the link and
the applications start.

A **PPS / 1 Hz tone** input is delivered in milestone M2 as a time-base hook
plus a backend vtable (`src/core/time/pps.h`): `FF_PpsRegister(backend)`
selects the active backend, `FF_PpsStart`/`FF_PpsStop` run/stop it, and every
physical or synthetic edge is delivered to `FF_TimeToneSignal(mono_ns)`. The
native **software backend** (`FF_PpsSwBackend`, priority 40, 1000 ms period)
is the reference and the host-simulator/test path.

The board integrator's remaining job (tracked as GAP-A7 in
`docs/quality/class-a-gaps.md`) is to replace the software backend with a
hardware capture on the RTEMS/MPFS target:

1. In `src/bsp/mpfs/board.c` register the interrupt (RTEMS `rtems_interrupt_*`
   on the PLIC/GPIO) or timer-capture. The ISR must do the **minimum** (latch
   the monotonic counter / post it to a queue) and must **not** call
   `FF_TimeToneSignal` directly: the tone state is protected by a PAL mutex and
   the hook runs in **task context** (a mutex is not ISR-safe). A dedicated
   time-tone task dequeues the captured timestamp and calls
   `FF_TimeToneSignal(captured_mono_ns)`.
2. Do the minimum in the ISR; do the UTC correlation and the state update in the
   time-tone task context.
3. Keep the hook registered through a vtable so the simulator can inject a
   synthetic tone (see §6).

Device-tree and timing constants live in `board/mpfs-rtpfs160zt.dts` and the
platform header (`config/platform/platform_mpfs.h`).

## 4. Adding a PAL/RTOS port

If you target a new RTOS, implement the surface of `src/core/pal/pal.h`
(tasks, mutexes, queues, time, console) in a new `pal_<rtos>.c` and select it in
`CMakeLists.txt`. `pal_posix.c` is the reference (POSIX + RTEMS 6 both use it);
`pal_rtems.c` shows the RTEMS-specific bootstrap. Keep the semantics
deterministic: fixed-size queues, monotonic time, bounded blocking.

## 5. Configuration and build integration

- `config/fsw_config.h` is the single configuration point; add tunables there.
- Platform selection is `-DFF_PLATFORM=rtems` (sets `FRICK_PLATFORM_MPFS`) or
  the default `native` (see `CMakeLists.txt`).
- Register new sources in `FF_CORE_SOURCES`; tests in `tests/CMakeLists.txt`.
- Reference implementations live under `src/drivers/` (e.g. the Cadence GEM MAC
  `src/drivers/eth/gem/`, the GEM TSN backend, and the loopback `FF_NetStack_t`)
  and implement the HAL contracts in `src/hal/`; they are separable via the
  `FF_BUILD_DRIVERS` option.
- Run the format gate locally before committing:

```bash
docker run --rm -v "$PWD":/workspace frick-vv:local \
  bash -lc 'cd /workspace && find src config tests -name "*.[ch]" -print0 | xargs -0 -n1 clang-format --dry-run --Werror'
```

## 6. Testing a driver

1. **Unit test** (`tests/unit/`): instantiate the vtable directly, drive
   `send`/`recv`, assert on bytes and error codes. Use the loopback shim
   (`FF_LinkLoopbackPush`) for link-level round-trips; use a fake register file
   for MMIO drivers so tests run on the host.
2. **Simulator smoke**: the native build must still boot and register the
   applications (`make run`, CI `smoke` job).
3. **Injection hooks**: expose a small `..._inject()`/setter for hardware edges,
   guarded so it is inert on the real target (pattern used by the time tone).

## 7. Traceability and requirements

Every driver delivers behaviour that must be traceable (norm N3/N4):

1. Add or refine an **SRS** item in `docs/requirements/SRS.md` (with an SSS
   parent where applicable).
2. Add the implementation and test paths to
   `docs/requirements/traceability.yaml` for that item.
3. If the driver has pure logic (framing, arithmetic, state machines), add a
   **Lean** model under `verification/lean/Frick/` and link its theorem in the
   manifest (norm N1).
4. Keep `docs/requirements/traceability.md` in sync (generated view).

## 8. V&V and tool qualification

- Re-run the **full cycle** on every change (norm N15): `make ci-local`.
- Record the run in `docs/vv/verification-results.md` (norm N11).
- If the change touches a V&V script or tool, update
  `docs/vv/tool-qualification.md` (norm N10).
- If the criticality requires independent review, follow
  `docs/engineering/independent-review.md` and store the report in
  `docs/reviews/` (norm N9).

## 9. Checklist

- [ ] Interface implemented behind a vtable, no new global coupling.
- [ ] No dynamic allocation after init; bounded ISR.
- [ ] Source added to `CMakeLists.txt`; tunables in config/platform headers.
- [ ] Unit test added and green; simulator smoke green.
- [ ] SRS/SSS + `traceability.yaml` updated; Lean model if there is logic.
- [ ] `docs/architecture/overview.md` / ICD updated if the interface changed.
- [ ] `CHANGELOG.md` + this document's History updated.
- [ ] Full cycle (`make ci-local`) green; committed and pushed to both remotes.

## 10. Heritage references

- NASA cFS: OSAL (our PAL), PSP/BSP layers, `CFE_TIME` tone handling.
- RTEMS 6.2 `riscv/mpfs64imafdc` BSP and Microchip RTPFS160ZT datasheet.
- CCSDS 133.0-B-2 (Space Packet), CCSDS 232.0-B-4 (TC Space Data Link).

## History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial driver development/integration guide (norm N8) |
| 0.2 | 2026-10-10 | frick-fsw | Milestone M2: PPS backend vtable + software backend delivered; board hook = GAP-A7 |
