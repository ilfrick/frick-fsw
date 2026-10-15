# User & Integration Manual

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-ENG-0005                         |
| Revision                          | 0.11                                   |
| Date                              | 2026-10-13                             |
| Audience                          | humans and agents integrating applications, drivers and stacks on the frick-fsw middleware |

This manual explains how to use frick-fsw as a **middleware**: you write your
own mission applications on top of it, integrate your own hardware drivers, and
optionally plug third-party stacks. Read first `AGENTS.md`, `README.md`,
`docs/development-plan.md` (product boundary and milestones) and
`docs/rules.md` (norms N1-N21).

> **frick-fsw is a framework, not an application.** The six `src/apps/*` are
> reference applications (demonstration and V&V), not mission software.

## 1. The application model

Every application is a **task** registered with Executive Services (ES) and
communicating over the Software Bus (SB). An application:

1. has an entry point `void AppMain(void *arg)` (`FF_PalTaskFunc_t`
   compatible, declared in `src/core/pal/pal.h`; see `src/apps/app.h` for the
   six reference entry-point pattern);
2. is described by an `FF_AppConfig_t` (name, entry, **`stack_bytes`**,
   priority, queue depth; `src/core/es/es.h`);
3. is registered at start-up with `FF_EsRegisterApp(&cfg, &id)`, optionally
   gets a pipe with `FF_SbCreatePipe(id, depth)`, subscribes with
   `FF_SbSubscribe(msg_id, id)`, and is started by `FF_EsStartAll()`.

The reference wiring is in `src/main/fsw_main.c`: it initialises the core
(`FF_PalInit` → `FF_SbInit` → `FF_EvsInit` → `FF_EsInit` → `FF_TblInit` →
`FF_TimeInit` → `FF_BoardInit` → `FF_MissionTablesInit`), initialises PUS
(`FF_PusInit` + `FF_PusSetBackend(&FF_PusBackendN7)`), starts the link
(`FF_LinkInit`), installs the event listener (`FF_EvsSetListener`), registers
the TM sink and HK providers (`FF_AppTmOutputInit`, `FF_AppHkRegister`), then
registers and starts the applications with `FF_EsStartAll()`.

### Minimal application

```c
#include "core/es/es.h"
#include "core/sb/sb.h"

static void MyAppMain(void *arg)
{
    (void)arg;
    FF_AppId_t me = FF_EsGetId("MY_APP");
    FF_SbSubscribe(0x1234u, me);          /* your message id(s) */
    for (;;) {
        FF_Msg_t msg;
        if (FF_SbReceive(me, &msg, 1000u) == FF_SUCCESS) {
            /* handle msg.payload[0..msg.length-1] */
        }
    }
}

static const FF_AppConfig_t my_cfg = {
    .name = "MY_APP", .entry = MyAppMain, .stack_bytes = 4096u,
    .priority = 50u, .queue_depth = 8u};
/* register: FF_EsRegisterApp(&my_cfg, &id); FF_SbCreatePipe(id, 8u); */
```

## 2. Software Bus (SB) — `src/core/sb/sb.h`

- `FF_SbPublish(const FF_Msg_t *msg)` — publish to all subscribers of
  `msg->msg_id` (wildcard `FF_SB_WILDCARD` = `0xFFFF` matches every id).
- `FF_SbSubscribe` / `FF_SbUnsubscribe` — per-application subscriptions.
- `FF_SbReceive(app, &msg, timeout_ms)` — bounded blocking receive from the
  application pipe (no dynamic allocation; fixed `FF_SB_MSG_PAYLOAD_MAX`).
- `FF_Msg_t`: `{ msg_id, apid, time_ns, length, payload[] }`.

## 3. Events (EVS) — `src/core/evs/evs.h`

- `FF_EvsSend(event_id, type, app, fmt, ...)` — emit an event (formatted
  message); `FF_EvsEvent_t`/`FF_EvsGetLog`/`FF_EvsGetTotalCount` for the log.
- Register a listener with `FF_EvsSetListener()` to forward events on the bus;
  the listener converts events to PUS service 5 telemetry (the reference
  `ff_event_listener` in `src/main/fsw_main.c`), and the `TM_OUTPUT` application
  installs the TM sink that emits the encoded packets over the link.

## 4. Tables (TBL) — `src/core/tbl/tbl.h`

`FF_TblRegister`, `FF_TblLoad`, `FF_TblActivate`,
`FF_TblRevert`, `FF_TblGet`/`FF_TblDump`. Tables are validated before atomic
activation via the `validate` callback registered in `FF_Tbl_Def_t`, which is
invoked internally by `FF_TblLoad` (the callback type is `FF_TblValidateFn`,
declared in `src/core/tbl/tbl.h`).

## 5. PUS providers — `src/pus/pus.h`

Register the data your application exposes to the ground:

- Housekeeping: `FF_PusRegisterHkProvider(id, provider)` (service 3).
- Parameters: `FF_PusRegisterParamProvider` (report) and
  `FF_PusRegisterParamSetter` (set) (service 20).
- Functions: `FF_PusRegisterFunction(id, fn)` (service 8).
- Memory regions: `FF_PusRegisterMemoryRegion(base, size)` (service 6).

The reference `HK` application shows the pattern. To add a **new/standardised
PUS service**, follow `docs/engineering/service-development.md`.

## 6. Time — `src/core/time/time.h`

- `FF_TimeGetMetNs`/`FF_TimeGetMetMs` — mission elapsed time.
- `FF_TimeGetUtcNs` — UTC (correlated); `FF_TimeSetUtcNs` — correlation
  (PUS service 9,1); `FF_TimeSetSource`/`FF_TimeGetSource` — source selection.
- `FF_TimeGetCuc`/`FF_TimeCucEncode`/`FF_TimeCucDecode` — CCSDS CUC timecode.
- A PPS backend (`src/core/time/pps.h`) disciplines the tone; see
  `docs/architecture/time-and-timecode.md`.

## 7. Link and PUS dispatch

- The **link layer** (`src/transport/link/link.h`) moves CCSDS frames over a
  transport (loopback, UART, IP, TSN). See §7.1 and §8 for the available
  backends.
- Received frames are fed to `FF_PusIngest(raw, len)`; telemetry is emitted
  through `FF_PusSendTm(...)` and the TM sink. Program against
  `FF_Pus_Backend_t` to swap the PUS product. The **default** backend is the
  N7/ESA ASN.1 one (`FF_PusBackendN7`, build-time generation `make pus-asn1`,
  `FF_USE_PUS_ASN1`); select the open backend explicitly if needed with
  `FF_PusSetBackend(&FF_PusBackendOpen)`.
- Secure links: the CCSDS **SDLS** sub-layer (`src/security/`) wraps the
  telemetry/telecommand with a pluggable cryptographic backend
  (`FF_CryptoBackend_t`); enable/configure it where the mission link requires it.
- The status of the **TCP/IP-over-PUS-C** and **TSN** paths — what is
  implemented in this repository versus planned, external or third-party — is
  stated in §8 and §9 below; §10 answers which TCP/IP stack is actually
  included.

### 7.1 PUS-C transport channels

**PUS-C is transport-agnostic.** ECSS-E-ST-70-41C defines the packet/service
semantics (the PUS-C TM/TC), not the physical link; the same PUS-C packets can
ride any byte-stream transport. In frick-fsw the TM/TC frames are carried by the
framework's **own link abstraction** `FF_Link_t` (`src/transport/link/link.h`),
a vtable of `init` / `send` / `recv` operating on **opaque CCSDS frames**.

**Channels implemented today.** Four reference link backends ship
(`src/transport/link/`):

- **Loopback** — `FF_LinkLoopbackBackend` (in-process; host simulator and unit
  tests);
- **UART** — `FF_LinkUartBackend` (byte-stream serial);
- **IP** — `FF_LinkIpBackend` (UDP default, TCP with seg framing; SRS-REQ-060);
- **TSN** — `FF_LinkTsnBackend` (GEM MAC, EtherType 0x88F5; SRS-REQ-061).

The link-management API (`FF_LinkInit`, `FF_LinkSelect`, `FF_LinkGet`,
`FF_LinkSend`, `FF_LinkRecv`) lives in `src/transport/link/link.c`; the backends
are in `src/transport/link/link_loopback.c`, `link_uart.c`, `link_ip.c` and
`link_tsn.c` (`CMakeLists.txt` lists them as separate sources).

**Selection.** Install a backend at run-time with `FF_LinkSelect()`, which calls
the backend's `init`. The default is the platform default (loopback on host /
UART on RTEMS); select another backend at start-up with `FF_LinkSelect()`.

> **Backend note (PUS-C).** The default PUS-C backend is the N7/ESA ASN.1
> stack (`FF_PusBackendN7`, `FF_USE_PUS_ASN1=ON` by default, built by
> `make pus-asn1`). If the generated ASN.1 code is absent (first-time checkout
> without running `make pus-asn1`), CMake emits a warning and falls back to the
> open CCSDS backend (`FF_PusBackendOpen`); select it explicitly with
> `FF_PusSetBackend(&FF_PusBackendOpen)` or `FF_USE_PUS_ASN1=OFF` (see
> `docs/interfaces/ICD.md` §PUS-C backend and `CMakeLists.txt` §PUS-C).

## 8. TCP/IP over PUS-C — status

The framework ships **reference link backends** that carry PUS-C TM/TC CCSDS
frames over UDP/TCP (`FF_LinkIpBackend`, SRS-REQ-060) and TSN-scheduled Ethernet
(`FF_LinkTsnBackend`, SRS-REQ-061), in addition to loopback and UART. Segmentation/
reassembly (2-byte BE length-prefix, SRS-REQ-059) provides stream framing for TCP.
The ICD has no EtherType or port allocation yet (mission input); on-target
netif/PHY/stack qualification is GAP-A5.

**Framework or separate?** The framework owns only the *contracts*, never a
stack:

- `FF_NetStack_t` — the stack-independent, socket-like vtable
  (`src/hal/hal_net.h`, SRS-REQ-027); and
- `FF_EthMac_t` / `FF_NetIf_t` — the Ethernet MAC and net-interface contracts
  that bind a MAC to a stack (`src/hal/hal_eth.h`, SRS-REQ-026).

Every TCP/IP **stack** is separate and optional: **lwIP** is the only stack
bundled here (third-party, opt-in, default off) and **Tuxera TCP/IP** and
**RTEMS `libbsd`** are **user-supplied**. The framework core links no stack.

**What is implemented today:** the two contracts above, a loopback reference
stack, the opt-in lwIP adapter, and the reference **IP link backend**
(`FF_LinkIpBackend`, SRS-REQ-060) and **TSN link backend** (`FF_LinkTsnBackend`,
SRS-REQ-061) that carry PUS-C CCSDS frames over a socket / GEM MAC. What remains
outside the framework: any EtherType/port allocation in the ICD and the
on-target netif/PHY/stack qualification (GAP-A5).

| Capability | Status | Evidence / source |
| ---------- | ------ | ----------------- |
| `FF_NetStack_t` contract (socket-like vtable; address = 6 octets, IPv4 big-endian + port) | **Implemented** | `src/hal/hal_net.h` (SRS-REQ-027) |
| Ethernet MAC / net-if contracts (`FF_EthMac_t`, `FF_NetIf_t`; `attach_netif`) | **Implemented** | `src/hal/hal_eth.h` (SRS-REQ-026) |
| Loopback network-stack reference backend (`FF_NetStackLoopback`) | **Implemented** | `src/drivers/net/ff_net_loopback.{c,h}` (SRS-REQ-039); `tests/unit/test_netloop.c` (328 checks) |
| lwIP adapter (`FF_NetStackLwip`, opt-in) | **Implemented** (adapter over a third-party stack) | `src/hal/net/lwip/ff_net_lwip.{c,h}` + `ff_lwip_netif.{c,h}` (SRS-REQ-040); `FF_USE_LWIP=ON` (default OFF); pinned lwIP (`STABLE-2_2_1_RELEASE`, BSD-3-Clause) fetched by `tools/scripts/fetch_third_party.sh` |
| Segmentation/reassembly layer (`FF_LinkSeg_t`, 2-byte BE length-prefix for TCP) | **Implemented** | `src/transport/link/link_seg.{c,h}` (SRS-REQ-059); used by `FF_LinkIpBackend` in TCP mode |
| IP **link** backend (`FF_Link_t`, UDP + TCP over `FF_NetStack_t`) | **Implemented** | `src/transport/link/link_ip.{c,h}` (SRS-REQ-060); `tests/unit/test_link_ip.c` (20 test functions: 10 seg + 9 IP + 1 PUS-C `test_pus_ip_roundtrip` TM/TC round-trip demo) |
| TSN **link** backend (`FF_Link_t`, GEM MAC, EtherType 0x88F5) | **Implemented** | `src/transport/link/link_tsn.{c,h}` (SRS-REQ-061); `tests/unit/test_link_tsn.c` (15 test functions, incl. the `test_pus_tsn_roundtrip` TM/TC round-trip demo) |
| EtherType / port allocation for PUS-C over IP | **Absent from ICD** | `docs/interfaces/ICD.md` (no TCP/UDP/IPv4/TSN content) |
| Ground station over IP | **Not implemented** (serial-only) | `tools/gsw/ff_gsw.py` (V&V tool T7, `docs/vv/tool-qualification.md`) |
| Tuxera TCP/IP, RTEMS libbsd adapters | **External / user-supplied** | plug into `FF_NetStack_t`; not part of this repository |

### 8.1 How to use it (what to call, what flags, what is user-supplied)

1. **Code against the contract, not a stack.** Use `FF_NetStack_t`
   (`src/hal/hal_net.h`): `init`, `attach_netif`, `open`, `connect`, `bind`,
   `listen`, `accept`, `send`, `recv`, `close`. Addresses are an opaque byte
   array (`const uint8_t *addr, uint16_t addr_len`); the 6-octet convention
   (IPv4[4] big-endian, then port[2] big-endian) is an adapter agreement used
   by the lwIP adapter (see `docs/architecture/network-stack.md`); the loopback
   reference backend uses a 1-byte port in `addr[0]`. `send` may be partial
   (check `*sent`).
2. **Pick a reference backend** (both are built from `src/`):
   - **Loopback** — `FF_NetStackLoopback` (`src/drivers/net/ff_net_loopback.{c,h}`),
     built with `FF_BUILD_DRIVERS=ON` (default). In-process and static; it
     exercises the contract and is **not** a TCP/IP stack.
   - **lwIP** — `FF_NetStackLwip` (`src/hal/net/lwip/ff_net_lwip.{c,h}` +
     `ff_lwip_netif.{c,h}`). Build
     with `-DFF_USE_LWIP=ON` (default **OFF**) and fetch the pinned stack first:
     `tools/scripts/fetch_third_party.sh` (lwIP release tag
     `STABLE-2_2_1_RELEASE`, **BSD-3-Clause**, fetched to `third_party/lwip`,
     **not committed**).
3. **Bind a MAC** with `FF_EthMac_t` / `FF_NetIf_t` (`src/hal/hal_eth.h`) via the
   stack's `attach_netif`; the reference GEM MAC driver is in
   `src/drivers/eth/gem/` (see `docs/engineering/driver-development.md`).
4. **Supply your own stack** (Tuxera TCP/IP, RTEMS `libbsd`, or another) by
   implementing `FF_NetStack_t` in an adapter — **no core change is required**
   (see §12).
5. **Use a link backend for PUS-C over IP/TSN.** `FF_LinkIpBackend` (SRS-REQ-060)
   carries CCSDS frames over UDP (default) or TCP (with `link_seg` framing,
   SRS-REQ-059) on top of any `FF_NetStack_t` backend (loopback for host tests,
   lwIP on target). `FF_LinkTsnBackend` (SRS-REQ-061) carries CCSDS frames over
   a GEM MAC using EtherType `0x88F5`, receiving frames via the MAC RX callback
   into a local ring buffer. Both are selectable with `FF_LinkSelect()` on
   `FF_LinkIpBackend` or `FF_LinkTsnBackend`. On-target link-level qualification
   remains **GAP-A5** (Renode GEM DMA is register-only); host unit tests
   (`test_link_ip`, `test_link_tsn`) provide round-trip evidence.

### 8.2 Using the IP and TSN link backends

**IP link backend (`FF_LinkIpBackend`).** UDP by default (datagram = one CCSDS
frame); TCP mode adds 2-byte BE length-prefix framing via `link_seg`:

```c
FF_LinkIpConfig_t cfg = FF_LinkIpDefaultConfig(&my_net_stack);
cfg.port = 8080u;
cfg.mode = FF_LINK_IPMODE_UDP;  /* or FF_LINK_IPMODE_TCP */
(void)FF_LinkIpInit(&cfg);
(void)FF_LinkSelect(&FF_LinkIpBackend);
```

**TSN link backend (`FF_LinkTsnBackend`).** Ethernet frames with EtherType
`0x88F5`; dest MAC defaults to broadcast, no VLAN tag by default:

```c
FF_LinkTsnConfig_t cfg = FF_LinkTsnDefaultConfig(&my_mac, &my_tsn);
(void)FF_LinkTsnInit(&cfg);
(void)FF_LinkSelect(&FF_LinkTsnBackend);
```

Gap context: on-target netif/PHY and stack qualification is **GAP-A5**; formal-verification
depth is **GAP-A1**; on-target code assurance / runtime evidence is **GAP-A4**
(`docs/quality/class-a-gaps.md`).

## 9. TSN (deterministic Ethernet) — status

TSN in frick-fsw is an **Ethernet-MAC-level capability** (IEEE 802.1), not a
PUS-C transport: it classifies, schedules and synchronises the link that a
TCP/IP or other network stack would later use. Milestone **M8** delivers the
full 802.1AS gPTP stack and the GEM TSN shaping/filtering completion.

| Capability | Status | Evidence / source |
| ---------- | ------ | ----------------- |
| `FF_TsnBackend_t` contract (802.1Q VLAN/PCP, 802.1Qbv, 802.1Qav idle slope, 802.1Qci stream filter, 802.1Qbb PFC, 802.1AS time-read seam) | **Implemented** | `src/hal/hal_tsn.h` (SRS-REQ-028) |
| GEM native TSN reference backend: 802.1Q (VLAN/PCP) classification, 802.1Qbv (ENST), 802.1AS time-read seam | **Implemented** | `src/drivers/eth/gem/ff_gem_tsn.{c,h}` (SRS-REQ-038); `tests/unit/test_gem_tsn.c` (22 checks) |
| 802.1Qci per-stream filtering (PSFP) via GEM screener type 1 | **Implemented** | `src/drivers/eth/gem/ff_gem_tsn.c` (SRS-REQ-066); `tests/unit/test_gem_tsn.c` |
| 802.1Qav credit-based shaping (CBS) | **Not implemented** (returns `FF_NOT_IMPLEMENTED`) | `src/drivers/eth/gem/ff_gem_tsn.c` (SRS-REQ-066; GEM register subset limitation; software fallback tracked NC-S8-01) |
| 802.1Qbb priority flow control (PFC) | **Not implemented** (returns `FF_NOT_IMPLEMENTED`) | `src/drivers/eth/gem/ff_gem_tsn.c` (SRS-REQ-066; managed by the fabric CoreTSN switch; NC-S8-02) |
| Full 802.1AS **gPTP** stack (BMCA, Sync/Follow_Up, PDelay, servo) | **Implemented (M8)** | `src/drivers/eth/gptp/ff_gptp.{c,h}` (SRS-REQ-068); `tests/unit/test_gptp.c` (58 checks); integrates with `FF_EthMac_t` + `FF_TsnBackend_t::get_time_ns` |
| Fabric **CoreTSN** path (multi-port switch / 802.1CB FRER) | **Planned (M14, fabric)** | Requires the fabric-device contract (SRS-REQ-067); NC-S8-02 |
| TSN traffic classification in the ground ICD | **Absent** | `docs/interfaces/ICD.md` (no TSN content) |

### 9.1 How to use it (what to call, what flags)

1. **Code against the contract.** Use `FF_TsnBackend_t` (`src/hal/hal_tsn.h`,
   SRS-REQ-028): `set_vlan(vid, pcp)` (802.1Q VLAN/PCP), `configure_tas(schedule,
   len)` (802.1Qbv time-aware gate list), `configure_cbs(traffic_class,
   idle_slope_bps)` (802.1Qav), `set_stream_filter(traffic_class, dest_mac, vid,
   ethertype, enable)` (802.1Qci per-stream), `enable_pfc(pfc_bitmap)`
   (802.1Qbb), and `get_time_ns(&ns)` (802.1AS time).
2. **Use the GEM native reference backend.** Instantiate `FF_TsnGem_t` and call
   `FF_TsnGemSetup(&tsn, &bus, base, speed_mbps)`
   (`src/drivers/eth/gem/ff_gem_tsn.{c,h}`, SRS-REQ-038). Qbv gate entries are
   `FF_TsnGate_t` (`queue`, `start_sec`/`start_nsec`, `on_nsec`/`off_nsec`).
3. **Know what each call does today.** `set_vlan` (802.1Q) and `configure_tas`
   (802.1Qbv ENST) and `set_stream_filter` (802.1Qci) are implemented;
   `configure_cbs` (802.1Qav) and `enable_pfc` (802.1Qbb) **return
   `FF_NOT_IMPLEMENTED`** (not exposed in the GEM register subset used);
   `get_time_ns` is an **802.1AS time-read seam** — a hardware GEM-TSU clock
   read, **not** the gPTP protocol.
4. **Use the gPTP stack.** Instantiate `FF_GptpPort_t` with
   `FF_GptpInit(&port, mac, tsn_backend)`, call `FF_GptpSetEnabled(&port, 1)`,
   then `FF_GptpTick(&port, now_ns)` each cycle. Incoming PTP frames are
   dispatched automatically via the MAC receive callback; send PDelay_Req with
   `FF_GptpSendPDelayReq(&port, t1_ns)`. Read the synchronized clock with
   `FF_GptpGetGrandmasterTime(&port, &ns)` (norm N18: on-Renode the stack is
   host-testable; on-target gPTP timing accuracy = GAP-A5).
5. **TSN is not PUS-C.** It shapes/schedules the Ethernet link; a network stack
   (for example lwIP, §8) would use that link later. It does not carry PUS-C.
6. **The gPTP stack is M8.** BMCA, Sync/Follow_Up, PDelay and servo are now
   implemented (host unit tests with mock MAC + TSU). The fabric CoreTSN /
   802.1CB FRER backend requires the fabric-device contract (SRS-REQ-067) and
   is planned for M14.

Gap context: on-target TSN behaviour (timing, scheduling, PTP servo on the real
GEM/PHY) is **GAP-A5**.

## 10. Which TCP/IP stack is included?

**Pronouncement: no TCP/IP stack is linked into the default build.** The
repository includes:

- the **`FF_NetStack_t` contract** (`src/hal/hal_net.h`, SRS-REQ-027) — always
  present, stack-independent;
- the **loopback reference stack** (`FF_NetStackLoopback`,
  `src/drivers/net/ff_net_loopback.{c,h}`, SRS-REQ-039) — built with
  `FF_BUILD_DRIVERS` (default ON); it exercises the contract but is **not** a
  TCP/IP stack;
- the **lwIP adapter** (`FF_NetStackLwip`, `src/hal/net/lwip/ff_net_lwip.{c,h}` +
  `ff_lwip_netif.{c,h}`, SRS-REQ-040) — built only with `FF_USE_LWIP=ON`
  (default **OFF**). lwIP itself is a **third-party BSD-3-Clause** stack, fetched
  pinned to the release tag **`STABLE-2_2_1_RELEASE`** (`third_party/lwip`) by
  `tools/scripts/fetch_third_party.sh`, and is **not committed** to the
  repository.

A mission that needs TCP/IP therefore either enables lwIP explicitly or supplies
its own adapter for Tuxera TCP/IP, RTEMS libbsd or another stack; no core change
is required (details in `docs/architecture/network-stack.md`). The lwIP adapter
is exercised in CI, but a compiling adapter is **not** a qualified network
stack: on-target qualification remains **GAP-A5**.

Note: the `FF_LinkIpBackend` and `FF_LinkTsnBackend` (§8.2) are **reference
`FF_Link_t` implementations**, not a TCP/IP stack — they frame CCSDS frames over
a `FF_NetStack_t` socket (IP) or a GEM MAC (TSN) and require a separate stack or
MAC driver underneath.

## 11. Integrating a driver / hardware

All hardware access goes through the **PAL** (OS primitives) and the **HAL**
(bus/device contracts, milestone M3). You implement a backend behind the
contract and register it; the middleware is not modified. The procedure,
obligations (buffer ownership, DMA + cache coherency, ISR/task split, no
dynamic allocation, bounded latency) and the partition/CMake options that let
you include only the drivers you need are in
`docs/engineering/driver-development.md` and `docs/development-plan.md`.

## 12. Integrating a third-party stack (pluggable)

Network stacks are pluggable by design: the middleware talks to a
**stack-independent contract** (`FF_NetStack_t`, M3), not to a specific stack.
To adopt Tuxera TCP/IP, lwIP, RTEMS libbsd or your own, implement the contract
in an adapter; no core change is required. The same pluggable-backend principle
applies to CAN/CANopen, TSN and storage backends (see
`docs/development-plan.md` §3). Which stack ships by default is stated in §10;
the current implemented/planned status of the TCP/IP and TSN paths is in §8 and
§9.

## 13. Build, test, verify

```bash
make configure build test    # host simulator + unit tests (ctest)
make verify-lean             # Lean formal verification (norm N1)
make trace                   # traceability convergence (norm N4)
make coverage                # coverage -> build/coverage/coverage.json
make metrics                 # regenerate the README status panel
make ci-local                # configure/build/test/Lean/trace in controlled Docker image
make smoke                   # native simulator smoke test (separate)
make docs                    # Doxygen generation (separate)
```

Register a new test in `tests/CMakeLists.txt`; add requirement links in
`docs/requirements/traceability.yaml` (norm N3/N4).

## 14. Contribution checklist (encoded in the norms)

1. English only (N2). 2. Requirements + traceability updated (N3/N4).
3. Lean model for behaviour-bearing code (N1). 4. Tests (N11).
5. Docs updated (N8), revision history (N16). 6. Full cycle green (N15).
7. Independent reviews R1/R2/R3 and documentation review R4, NCs closed
(N9/N10/N12/N14/N17). 8. Commit and push both remotes (N7).

## 15. Mathematics (libm support)

Applications that need floating-point mathematics (`sqrt/sin/cos/tan/atan2/pow/
exp/log/fabs/fmod`) use the portable facade `src/math/ff_math.h`
(`FF_MathSqrt`, `FF_MathSin`, ... plus `FF_MathDeg2Rad`, `FF_MathRad2Deg`,
`FF_MathClamp`, `FF_MathIsFinite`). The framework core uses integer arithmetic
only. Out-of-domain arguments follow the platform `libm` (`sqrt(x<0)=NaN`,
`log(x<=0)=-inf`); test results with `FF_MathIsFinite()` when a defined fallback
is required, and do not use transcendentals in hard real-time critical paths
without a WCET/stack analysis. See `docs/architecture/numerics.md` and ADR-0012.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial user & integration manual (OBJ-02, milestone M3) |
| 0.2 | 2026-10-10 | frick-fsw | M6: add mathematics (libm) support section |
| 0.3 | 2026-10-12 | frick-fsw | Document TCP/IP-over-PUS-C and TSN status (implemented / planned / external / third-party + GAP references) and which TCP/IP stack is included by default; renumber following sections |
| 0.4 | 2026-10-12 | frick-fsw | Add user "How to use" subsections §8.1 (TCP/IP: contract, `FF_NetStackLoopback`/`FF_NetStackLwip`, `FF_USE_LWIP`, fetch pin/licence, user-supplied stacks) and §9.1 (TSN: `FF_TsnBackend_t`/`FF_TsnGemSetup`, Qav `FF_NOT_IMPLEMENTED`, 802.1AS time-read seam, M8); state framework-owns-contracts vs stack-separate; add the lwIP pin `STABLE-2_2_1_RELEASE`/BSD-3-Clause and the `FF_EthMac_t`/`FF_NetIf_t` row |
| 0.5 | 2026-10-12 | frick-fsw | Add §7.1 "PUS-C transport channels": PUS-C is transport-agnostic, carried by `FF_Link_t`; only loopback and UART reference backends ship (default loopback on host / UART on RTEMS), `FF_LinkSelect()` plugs another backend, and IP/TSN/CAN/SpaceWire link backends are not implemented (planned) |
| 0.6 | 2026-10-12 | frick-fsw | Update §8/§10: the IP/TSN link backends are now the explicit M4-residual scope (SRS-REQ-059/060/061, sprint S4b), not an "absorbed" plan |
| 0.7 | 2026-10-13 | frick-fsw | M8 implementation: gPTP stack (BMCA, Sync/Follow_Up, PDelay, PI servo), 802.1Qci via GEM screener type 1; update §9 status table and §9.1 for gPTP usage; fix §1 init sequence (add BoardInit/MissionTablesInit/LinkInit/EvsSetListener/AppTmOutputInit/AppHkRegister), §4 tables (`FF_TblValidateFn`), §7.1 link backends (`link_loopback.c`/`link_uart.c`, `FF_LinkSelect` NULL guard), §8 `GAP-A1` correction, lwIP file list (`ff_lwip_netif.{c,h}` in §8.1/§10), §13 separate `coverage`+`metrics`; R4+FSW review NCs recorded for unresolved items |
| 0.8 | 2026-10-13 | frick-fsw | **S4b delivered** (SRS-REQ-059/060/061): segmentation/reassembly layer `link_seg.{c,h}` (2-byte BE length-prefix, allocation-free); IP `FF_Link_t` backend `link_ip.{c,h}` (UDP + TCP); TSN `FF_Link_t` backend `link_tsn.{c,h}` (EtherType 0x88F5, callback-driven RX ring); updated §7 (link transport list), §7.1 (four backends), §8 (status + table rows), new §8.2 (IP/TSN usage), §10 (note link backends are reference, not a TCP/IP stack); Renode SKIP (host-only); R4 documentation re-review (FRICK-REVIEW-0164) |
| 0.9 | 2026-10-13 | frick-fsw | **S4b delta re-review (N12 closure)**: all findings from FRICK-REVIEW-0165 (R1+R2, REJECTED) and FRICK-REVIEW-0166 (PA, REJECTED) resolved; test counts corrected (test_link_ip 19 funcs, test_link_tsn 14 funcs); `-Wall -Wextra -Werror` clean; R1+R2 delta (FRICK-REVIEW-0165-D1) and PA delta (FRICK-REVIEW-0166-D1) both APPROVED; commit/push unblocked |
| 0.10 | 2026-10-13 | frick-fsw | §8 status counts updated for the S4b follow-up (FRICK-REVIEW-0168 DOC-R4-030): `test_link_ip` 20 test functions (10 seg + 9 IP + 1 `test_pus_ip_roundtrip`) and `test_link_tsn` 15 test functions (incl. `test_pus_tsn_roundtrip`); header Revision aligned with the History |
| 0.11 | 2026-10-13 | frick-fsw | Bound `make ci-local` to configure/build/test/Lean/trace, list separate smoke/docs commands, and state the mutable image-input limitation (NC-EXT-35). |
