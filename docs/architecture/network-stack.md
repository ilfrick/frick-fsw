# Network Stack Abstraction

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-ARCH-0012                        |
| Revision                          | 0.4                                    |
| Date                              | 2026-10-12                             |
| Norms                             | N3, N4, N8                             |
| Requirements                      | SRS-REQ-027, SRS-REQ-039, SRS-REQ-040, SRS-REQ-059, SRS-REQ-060, SRS-REQ-061 |

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
| lwIP | `src/hal/net/lwip/ff_net_lwip.{c,h}` | adapter over the lwIP socket API (SRS-REQ-040); lwIP is a third-party **BSD-3-Clause** stack, fetched pinned to the release tag **`STABLE-2_2_1_RELEASE`** (`third_party/lwip`, `contrib` included in the lwIP tree; not committed) |

Tuxera TCP/IP and any other closed stack plug in through the same vtable as
user-supplied adapters.

## 4. Build

- Loopback backend: always built with `FF_BUILD_DRIVERS=ON`.
- lwIP backend: `FF_USE_LWIP=ON` (default OFF); requires the fetched lwIP
  (`tools/scripts/fetch_third_party.sh`, release tag `STABLE-2_2_1_RELEASE`).
  CMake builds a `frick_lwip` target
  (lwIP core + the unix port `sys_arch`) and a separate `frick_net_lwip` adapter
  target. lwIP sources are not compiled with the project's `-Werror`.

## 5. Verification

- `tests/unit/test_netloop.c` (328 checks): the loopback contract (incl. boundary/negative paths, descriptor exhaustion, full-ring partial send, cross-socket duplicate bind).
- `tests/unit/test_lwip.c` (19 checks, `FF_USE_LWIP`): a real TCP round trip
  over the lwIP loopback interface (127.0.0.1), plus negative address handling.
- CI: a job fetches lwIP, builds with `FF_USE_LWIP=ON` and runs the lwIP test.

## 6. Status — PUS-C over IP and TSN

**PUS-C over IP is not implemented end-to-end in this revision.** The contract
above is the delivered seam; the TCP/IP and TSN **link** backends (`FF_Link_t`)
and the PUS-C-over-IP segmentation/reassembly are **planned** (explicit
M4-residual scope, SRS-REQ-059/060/061: `docs/roadmap.md`,
`docs/development-plan.md`, `docs/planning/sprint-plan.md`).
The ICD has no TCP/UDP/IPv4 or EtherType allocation yet (`docs/interfaces/ICD.md`),
and the supplied ground station is **serial-only** (V&V tool T7,
`docs/vv/tool-qualification.md`). On-target netif/PHY and stack qualification is
**GAP-A5** (`docs/quality/class-a-gaps.md`).

**TSN** is a MAC-level capability: the GEM native backend
(`src/drivers/eth/gem/ff_gem_tsn.{c,h}`, SRS-REQ-038) implements 802.1Q
(VLAN/PCP), 802.1Qbv (ENST) and the 802.1AS time-read seam (a hardware clock
read, not the gPTP protocol); 802.1Qav returns
`FF_NOT_IMPLEMENTED`; the full gPTP stack and the fabric CoreTSN path are
milestone **M8** (planned). On-target TSN timing is **GAP-A5**.

The implemented / planned / external / third-party matrix, the "which TCP/IP
stack is included" pronouncement and the "how to use" steps (what to call, what
build flags, what is user-supplied) are in
`docs/engineering/user-integration-manual.md` (§8-§10, incl. §8.1 and §9.1),
with a summary in `docs/engineering/software-description.md` (§3.7).

## 7. Gaps

On-target hardware netif/PHY and stack timing are GAP-A5; the adapter assumes a
mission-provided netif on the target (`attach_netif`).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial network-stack abstraction note (loopback + lwIP adapter) |
| 0.2 | 2026-10-12 | frick-fsw | Add status section: PUS-C over IP and TSN implemented vs planned, with pointers to the ICD, development plan and engineering manuals |
| 0.3 | 2026-10-12 | frick-fsw | State the exact lwIP pin (release tag `STABLE-2_2_1_RELEASE`) and BSD-3-Clause licence in §3/§4; cross-reference the user manual's "how to use" steps (§8.1/§9.1) |
| 0.4 | 2026-10-12 | frick-fsw | Update §6: the PUS-C-over-IP/TSN link backends are now the explicit M4-residual scope (SRS-REQ-059/060/061), not an "absorbed" note |
