# Network Stack Abstraction

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-ARCH-0012                        |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-10                             |
| Norms                             | N3, N4, N8                             |
| Requirements                      | SRS-REQ-027, SRS-REQ-039, SRS-REQ-040  |

## 1. Purpose

The framework addresses the network only through a **stack-independent
contract**, `FF_NetStack_t` (`src/hal/hal_net.h`), so a mission can plug its own
TCP/IP stack without changing the middleware. This note describes the contract,
the two reference backends and how they are verified.

## 2. Contract and addressing

`FF_NetStack_t` is a socket-like vtable: `init`, `attach_netif`, `open`,
`connect`, `bind`, `listen`, `accept`, `send`, `recv`, `close`. Addresses in the
contract are **6 octets**: IPv4 (4, big-endian) followed by port (2,
big-endian). `send` may be partial (`*sent`); `recv` semantics are
backend-defined: the loopback backend is non-blocking, the lwIP backend is
blocking unless `timeout_ms > 0` (which sets `SO_RCVTIMEO`).

## 3. Reference backends

| Backend | File | Notes |
| ------- | ---- | ----- |
| Loopback | `src/drivers/net/ff_net_loopback.{c,h}` | bounded, static, in-process; exercises the contract in CI (SRS-REQ-039) |
| lwIP | `src/hal/net/lwip/ff_net_lwip.{c,h}` | adapter over the lwIP socket API (SRS-REQ-040); lwIP is a third-party BSD-3 stack, fetched pinned (`third_party/lwip`, `contrib` included in the lwIP tree) |

Tuxera TCP/IP and any other closed stack plug in through the same vtable as
user-supplied adapters.

## 4. Build

- Loopback backend: always built with `FF_BUILD_DRIVERS=ON`.
- lwIP backend: `FF_USE_LWIP=ON` (default OFF); requires the fetched lwIP
  (`tools/scripts/fetch_third_party.sh`). CMake builds a `frick_lwip` target
  (lwIP core + the unix port `sys_arch`) and a separate `frick_net_lwip` adapter
  target. lwIP sources are not compiled with the project's `-Werror`.

## 5. Verification

- `tests/unit/test_netloop.c` (65 checks): the loopback contract (incl. boundary/negative paths, descriptor exhaustion, full-ring partial send, cross-socket duplicate bind).
- `tests/unit/test_lwip.c` (15 checks, `FF_USE_LWIP`): a real TCP round trip
  over the lwIP loopback interface (127.0.0.1), plus negative address handling.
- CI: a job fetches lwIP, builds with `FF_USE_LWIP=ON` and runs the lwIP test.

## 6. Gaps

On-target hardware netif/PHY and stack timing are GAP-A5; the adapter assumes a
mission-provided netif on the target (`attach_netif`).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial network-stack abstraction note (loopback + lwIP adapter) |
