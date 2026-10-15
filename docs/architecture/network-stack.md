# Network Stack Abstraction

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-ARCH-0012                        |
| Revision                          | 0.9                                    |
| Date                              | 2026-10-13                             |
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
| lwIP | `src/hal/net/lwip/ff_net_lwip.{c,h}`, `ff_lwip_netif.{c,h}` | adapter over the lwIP socket API + netif glue (netif bound to `FF_EthMac_t`) (SRS-REQ-040); lwIP is a third-party **BSD-3-Clause** stack, fetched pinned to the release tag **`STABLE-2_2_1_RELEASE`** (`third_party/lwip`, `contrib` included in the lwIP tree; not committed) |

Tuxera TCP/IP and any other closed stack plug in through the same vtable as
user-supplied adapters.

## 4. Build

- Loopback backend: always built with `FF_BUILD_DRIVERS=ON`.
- lwIP backend: `FF_USE_LWIP=ON` (default OFF); requires the fetched lwIP
  (`tools/scripts/fetch_third_party.sh`, release tag `STABLE-2_2_1_RELEASE`).
  CMake builds a `frick_lwip` target (lwIP core + the port `sys_arch`) and a
  separate `frick_net_lwip` adapter target. On the **host** the lwIP `unix` port
  is used; for the **RTEMS target** the in-repo port
  `src/hal/net/lwip/rtems/` (`sys_arch` over the RTEMS classic API) is selected
  by `FF_PLATFORM=rtems` — the pinned lwIP ships no RTEMS port, so this port is
  required to build the stack for the target (S6b-1; the runtime object sizing is
  wired in `config/rtems/rtems_config.c`, S6b-2). The adapter is linked only into
  the **lwIP-enabled** image, so `FF_USE_LWIP=OFF` output is unchanged.
  lwIP sources are not compiled with the project's `-Werror`.
- lwIP netif glue (S6b-2): `src/hal/net/lwip/ff_lwip_netif.{c,h}` registers an
  lwIP `struct netif` whose `linkoutput` transmits via the framework Ethernet MAC
  (`FF_EthMac_t`) and whose RX callback feeds `tcpip_input`; on the lwIP-enabled
  emulated image the check binds the netif to the emulated GEM and asserts
  netif-up (`hal_net_lwip`). The full TX/RX packet round-trip and the emulated GEM
  DMA remain open (NC-S6-01/GAP-A5: the Renode GEM model is register-only and does
  not exercise the DMA packet path). NC-S6-03's 64-bit GEM base-address handling
  was co-implemented; it is not the current root cause of the remaining gap.

## 5. Verification

- `tests/unit/test_netloop.c` (328 checks): the loopback contract (incl. boundary/negative paths, descriptor exhaustion, full-ring partial send, cross-socket duplicate bind).
- `tests/unit/test_lwip.c` (19 checks, `FF_USE_LWIP`): a real TCP round trip
  over the lwIP loopback interface (127.0.0.1), plus negative address handling.
- CI: a job fetches lwIP, builds with `FF_USE_LWIP=ON` and runs the lwIP test.

## 6. Status — PUS-C over IP and TSN

**PUS-C over IP/TSN is implemented (host-verified).** The framework ships the
PUS-C-over-IP segmentation/reassembly layer `src/transport/link/link_seg.{c,h}`
(2-byte BE length-prefix, allocation-free; SRS-REQ-059), the IP **link** backend
`src/transport/link/link_ip.{c,h}` (UDP default + TCP with `link_seg` framing
over any `FF_NetStack_t`; SRS-REQ-060, `FF_LinkIpBackend`) and the TSN **link**
backend `src/transport/link/link_tsn.{c,h}` (Ethernet frames over the GEM MAC,
EtherType 0x88F5; SRS-REQ-061, `FF_LinkTsnBackend`). Evidence:
`tests/unit/test_link_ip.c` (20 test functions incl. the `test_pus_ip_roundtrip`
PUS-C TM/TC round-trip demonstration) and `tests/unit/test_link_tsn.c` (15 test
functions incl. `test_pus_tsn_roundtrip`); the `FF_NetStack_t` contract above
remains the delivered stack seam.

The TM/TC ICD has no TCP/UDP/IPv4 or EtherType allocation yet (mission input;
`docs/interfaces/ICD.md`), and the supplied ground station is **serial-only**
(V&V tool T7, `docs/vv/tool-qualification.md`). On-target netif/PHY and link/stack
qualification is **GAP-A5** (`docs/quality/class-a-gaps.md`).

**TSN** is a MAC-level capability: the GEM native backend
(`src/drivers/eth/gem/ff_gem_tsn.{c,h}`, SRS-REQ-038) implements 802.1Q
(VLAN/PCP), 802.1Qbv (ENST), the 802.1AS time-read seam (a hardware clock
read) and 802.1Qci screener-type-1 filtering (SRS-REQ-066); 802.1Qav (CBS) and
802.1Qbb (PFC) return `FF_NOT_IMPLEMENTED` (NC-S8-01/NC-S8-02). The full gPTP
protocol stack (802.1AS: BMCA, Sync/Follow_Up, PDelay, PI servo disciplining the
GEM TSU) is **implemented** (`src/drivers/eth/gptp/`, SRS-REQ-068, milestone M8);
the fabric CoreTSN path (multi-port switch / 802.1CB FRER) is **deferred to M14**
(SRS-REQ-068, requiring the SRS-REQ-067 fabric contract). On-target TSN timing is
**GAP-A5**.

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
| 0.5 | 2026-10-12 | frick-fsw | §4: document the lwIP port selection — unix port on the host, the in-repo RTEMS `sys_arch` port (`src/hal/net/lwip/rtems/`) on the target (S6b-1); the pinned lwIP ships no RTEMS port |
| 0.6 | 2026-10-12 | frick-fsw | §4: document the lwIP netif glue (`ff_lwip_netif.{c,h}`, netif bound to `FF_EthMac_t`; `hal_net_lwip` on the emulated GEM, S6b-2) and the open packet-round-trip/GEM-DMA residuals (NC-S6-01/NC-S6-03) |
| 0.7 | 2026-10-13 | frick-fsw | §6: correct the status to the delivered S4b scope (FRICK-REVIEW-0168 DOC-R4-025) — the PUS-C-over-IP segmentation/reassembly (`link_seg.{c,h}`, SRS-REQ-059) and the IP/TSN `FF_Link_t` backends (`link_ip.{c,h}` SRS-REQ-060, `link_tsn.{c,h}` SRS-REQ-061) are implemented (host `test_link_ip`/`test_link_tsn`, incl. the `pus_*` round-trip demonstrations); the ICD TCP/UDP/IPv4/EtherType allocation remains absent (mission input) and the on-target residual stays GAP-A5 |
| 0.8 | 2026-10-13 | frick-fsw | §6: reconcile the TSN/gPTP milestone statement with the authoritative SRS (NC-S4b-07) — the full 802.1AS gPTP stack is implemented (`src/drivers/eth/gptp/`, SRS-REQ-068, M8) and the fabric CoreTSN path is deferred to M14 (not "M8 planned"); 802.1Qci is recorded as implemented and 802.1Qav/802.1Qbb as `FF_NOT_IMPLEMENTED` (NC-S8-01/NC-S8-02) |
| 0.9 | 2026-10-13 | frick-fsw | §6 milestone change is NC-S4b-07; document the corrected NC-S6-03/GAP-A5 root cause for the remaining GEM DMA path; preserve the unresolved on-target residual and pending independent NC closure accurately. |
