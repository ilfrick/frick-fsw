# Test Specification TS-2 — Link layer (loopback / IP / TSN / UART) and CCSDS SDLS

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-TSPEC-TS2                     |
| Revision                         | 0.9                                    |
| Date                             | 2026-10-16                             |
| Norms                            | N10, N11, N22                          |
| Umbrella                         | `docs/vv/comprehensive-test-spec.md` (FRICK-VV-TSPEC-001) |
| Work-package index               | `docs/vv/test-spec/README.md` (FRICK-VV-TSPEC-002), work package TS-2 |
| Companion                        | `docs/vv/system-verification-architecture.md` (FRICK-VV-SVA-001) |
| Scope                            | Link backends (`link.c`, `link_loopback.c`, `link_uart.c`, `link_ip.c`, `link_tsn.c`), PUS-C segmentation/reassembly (`link_seg.c`, SRS-REQ-059) and the CCSDS SDLS sub-layer (`src/security/ff_sdls.c`, SRS-REQ-034/035) |
| Baseline                         | Candidate delta: source HEAD `13ebf92` plus uncommitted UART receive-framing changes/tests and prior T7 changes; see current verification-results. Earlier TS-2 cases retain their historical baselines. |
| Modules under test               | `src/transport/link/link.c`, `link_loopback.c`, `link_uart.c`, `link_seg.c`, `link_ip.c`, `link_tsn.c`; `src/security/ff_sdls.c` |
| Interfaces cited                 | `link.h`, `link_seg.h`, `link_ip.h`, `link_tsn.h`, `security/ff_sdls.h`, `core/common/ff_types.h`, `docs/interfaces/ICD.md` (FRICK-ICD-001), `docs/architecture/security-sdls.md` (FRICK-ARCH-0010), `docs/architecture/network-stack.md` (FRICK-ARCH-0012) |

> **Authority statement.** This work package defines verification *cases and
> oracles* for the current baseline. It does not assign criticality, accept
> residual risk, qualify tools, close an NC, or approve release/flight
> readiness. Requirement/interface gaps are recorded (never resolved by
> inventing an oracle); disposition and closure require the authorized
> authorities.

## 1. Scope and method

### 1.1 In scope

Cases derived from the following requirements, which are the ones in this
work package's area and present in `docs/requirements/traceability.yaml`:

`SRS-REQ-015` (deterministic memory, no dynamic allocation after init),
`SRS-REQ-016` (portability: host simulator and RTEMS target via the PAL),
`SRS-REQ-026` (Ethernet MAC + net-interface contract), `SRS-REQ-027`
(pluggable network-stack contract), `SRS-REQ-028` (TSN backend contract),
`SRS-REQ-034` (SDLS processing), `SRS-REQ-035` (SA database and key lifecycle),
`SRS-REQ-039` (reference network stack), `SRS-REQ-044` (Renode
functional-validation environment, cited only by the PIL gap cases
`N22-TS2-76`/`N22-TS2-77`), `SRS-REQ-059` (PUS-C link segmentation and
reassembly), `SRS-REQ-060` (IP-based link backend, TCP/UDP), `SRS-REQ-061` (TSN
link backend). SSS parents carried by the same modules: `SSS-REQ-014` (transport
and link decoupled; `link*` implementation set), `SSS-REQ-019` (SDLS),
`SSS-REQ-028` (PUS-C transport over IP and TSN).

N22-TS2-81 is an additional NC-driven characterization case for the current
UART reference implementation, not a requirement-derived verification case;
its timeout/recovery behavior awaits authority disposition (NC-EXT-07).

Out of scope (owned by other work packages): CCSDS/PUS/time codec (TS-1), the
I/O device drivers and HAL seams (TS-3/TS-4), core services (TS-5), the
software-only robustness programme (TS-6), and the general multicore/timing
programme (SVA-3). The `FF_NetStackLoopback` adapter is cited only as a fixture;
its own behavior is TS-3.

### 1.2 Class, scope, environment and status conventions

- **Class**: `nominal` = a legal input inside the specified operating envelope
  (this includes legal minimum/maximum boundary values); `OOB` = a
  range-violating, malformed, out-of-envelope, resource-exhausted or
  prohibited-ordering input.
- **Scope**: `individual` = one module surface is under test (other modules are
  only fixtures); `combined` = the case deliberately asserts coupling across two
  or more modules/layers in one scenario (e.g. PUS-C + link backend, seg +
  link_ip, SDLS + link).
- **Env**: `SIL` = native host unit/integration suite (`make test`, ctest);
  `PIL` = Renode emulated PolarFire SoC; `HIL` = target silicon
  (RTPFS160ZT). Per N22 and FRICK-VV-TSPEC-001 §1.3, an emulator establishes
  only what its model represents.
- **Status**: `covered` = the case is specified with an oracle grounded in a
  controlled requirement/architecture/interface clause or in the declared API
  contract of the cited header, and is executable in the named environment (the
  test may still have to be added; execution status is tracked by the umbrella
  and `docs/vv/verification-results.md`). `gap` = no controlled oracle exists
  (the requirement/interface is silent, contradictory, mission-tailored or the
  feature is not wired), so the case is recorded against **NC-EXT-47** and
  **not** executed to a fabricated oracle.
- `characterization` = a controlled requirement oracle is absent, but the case
  captures explicitly identified current implementation behavior for an open
  NC. It earns no SRS/SSS verification credit and is not an accepted mission
  interface policy; retain the owner/authority NC open.
- **Gap discipline (N22)**: every `OOB` row (and any nominal row) whose expected
  behaviour is not defined by a requirement/architecture/interface clause is
  `gap`; the oracle cell names the open NC/TBD and, where applicable, an
  uncontrolled *observation* (explicitly not an oracle). The Gaps list (§3) maps
  every gap row to an NC-EXT-47 sub-item (FRICK-VV-TSPEC-001 §7).

### 1.3 Interface clauses used as oracle ground

| Clause | Content used as oracle ground |
| --- | --- |
| FRICK-ICD-001 §2 | in/out-of-band transport: UART byte stream (target `/dev/console`, 115200 baud); loopback on the host simulator; max TC/TM frame 1024 |
| FRICK-ARCH-0010 §3 | SA indexed by SPI selects the service (PLAIN/AUTH/ENC/AUTH_ENC); a session key is derived from the master key via the backend KDF bound to `(SPI, epoch)`; rotation zeroises the old key, increments the epoch (strictly increasing), derives a new key and resets usage/time; zeroisation on remove/zeroize; the SA table is bounded (`FF_SDLS_MAX_SA`) with no dynamic memory |
| FRICK-ARCH-0010 §4 | on-the-wire layout `[SPI:2 BE][IV:iv_len][payload][MAC:mac_len]`; the IV is the SA base with a per-frame big-endian counter (uniqueness within a session key); the backend must provide `iv_len >= 4` (enforced by `FF_SdlsSetBackend`); the payload is encrypted for ENC/AUTH_ENC and passed through for AUTH; a MAC is appended for AUTH/AUTH_ENC over `SPI` + `IV` + `payload`; validation uses a constant-time comparison and rejects tampered units |
| FRICK-ARCH-0010 §5/§7 | anti-replay/sequence-number semantics and Space-Packet→secured-frame mapping are mission-tailored (not implemented); algorithm/HSM qualification, IV-uniqueness analysis and ground key management are GAP-A14 |
| FRICK-ARCH-0012 §6 | PUS-C over IP/TSN implemented host-verified: TCP uses the 2-byte BE length-prefix seg framing, UDP uses the datagram boundary, TSN uses EtherType 0x88F5; the ICD TCP/UDP/IPv4/EtherType allocation is mission input (absent) and on-target = GAP-A5 |
| `link.h`, `link_seg.h`, `link_ip.h`, `link_tsn.h` | declared API/byte-stream contract: opaque frame byte-stream (send/recv), reference backends, seg prefix width/max-frame bounds, IP UDP/TCP modes and reference defaults (UDP, port 8080, 127.0.0.1), TSN EtherType 0x88F5 / 14-byte header / 60-byte pad / 16-slot RX ring / `FF_LINK_TSN_RX_FRAMESZ` |
| `security/ff_sdls.h`, `ff_types.h` | declared SDLS API and status codes (`FF_SUCCESS`, `FF_ERROR`, `FF_INVALID_ARGUMENT`, `FF_NO_RESOURCE`, `FF_TIMEOUT`, `FF_NOT_FOUND`, `FF_BAD_SIZE`, `FF_BUFFER_FULL`, `FF_PARSE_ERROR`, `FF_ALREADY_EXISTS`, `FF_NOT_READY`) and bounds (`FF_SDLS_MAX_SA`, `FF_SDLS_KEY_MAX`, `FF_SDLS_MAC_MAX`, `FF_SDLS_IV_MAX`, `FF_LINK_SEG_MAX_FRAME`) |

## 2. Test cases

Case IDs `N22-TS2-1` … `N22-TS2-81`; groups: `1–8` link core and loopback,
`9–16` IP backend (UDP/TCP), `17–29` segmentation/reassembly, `30–37` TSN
backend, `38–41` UART backend, `42–69` SDLS (backend, SA/key lifecycle,
protect/unprotect), `70–80` combined scenarios and environment-bound gaps, and
`81` UART packet-framing integration.

| Case | Requirement | Case | Class | Inputs (boundaries) | Expected result (oracle) | Env | Scope | Status |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| N22-TS2-1 | SRS-REQ-016, SSS-REQ-014 | `link.c` default link selection (host loopback vs target UART) | nominal | fresh `FF_LinkInit()`; host simulator build and MPFS/QEMU build | `FF_SUCCESS`; `FF_LinkGet()` == `&FF_LinkLoopbackBackend` on the host simulator and `&FF_LinkUartBackend` on the MPFS/QEMU platforms (ICD §2 "loopback on host simulator", `link.c`) | SIL | individual | covered |
| N22-TS2-2 | SRS-REQ-060, SRS-REQ-061, SSS-REQ-014 | `link.c` `FF_LinkSelect` of each reference backend | nominal | `FF_LinkInit()`; `FF_LinkSelect(&FF_LinkLoopbackBackend)`, then `&FF_LinkIpBackend`, then `&FF_LinkTsnBackend` (each after its required init) | each returns the selected backend's `init()` status; `FF_LinkGet()` returns exactly the selected pointer; subsequent `FF_LinkSend`/`FF_LinkRecv` dispatch to that backend (SRS-REQ-060/061 "selectable with `FF_LinkSelect`"; SSS-REQ-014) | SIL | individual | covered |
| N22-TS2-3 | SSS-REQ-014 | `link.c` `FF_LinkSelect(NULL)` | OOB | `FF_LinkSelect(NULL)` while a backend is selected | **gap** — no requirement/interface clause defines the NULL-link contract. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: `FF_INVALID_ARGUMENT`, previously selected link unchanged. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-4 | SSS-REQ-014 | `link.c` `FF_LinkSend`/`FF_LinkRecv` before any selection | OOB | call `FF_LinkSend`/`FF_LinkRecv` without `FF_LinkInit`/`FF_LinkSelect` | **gap** — no clause defines pre-selection dispatch. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: `FF_NOT_READY`, no bytes transferred. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-5 | SSS-REQ-014, SRS-REQ-016 | `link_loopback.c` byte round trip | nominal | host `FF_LinkInit()`; `FF_LinkSend({01…05})` then `FF_LinkLoopbackPop`; `FF_LinkLoopbackPush({AA,BB,CC})` then `FF_LinkRecv` | `FF_SUCCESS`; the popped bytes equal the sent bytes in order; the received bytes equal the pushed bytes in order (`link.h` byte-stream contract; ICD §2) | SIL | individual | covered |
| N22-TS2-6 | SSS-REQ-014 | `link_loopback.c` empty-ring receive | OOB | `FF_LinkRecv` with an empty RX ring; `FF_LinkLoopbackPop` with an empty TX ring | **gap** — no clause defines the empty-ring return contract. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: `FF_TIMEOUT`, `*len = 0`. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-7 | SSS-REQ-014 | `link_loopback.c` ring-full and zero-length boundaries | OOB | push/send until the 8192-octet ring is full then push one more octet; push 0 octets | **gap** — no clause defines the ring-full/zero-length contract. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: `FF_BUFFER_FULL` on overflow, `FF_SUCCESS` for zero length. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-8 | SSS-REQ-014 | `link_loopback.c` maximum-frame (1024-octet) round trip | nominal | push/send a 1024-octet frame (`FF_PUS_MAX_TM_FRAME`) and read it back | `FF_SUCCESS`; all 1024 octets retrieved byte-identical (ICD §2 max frame 1024; `link.h`) | SIL | individual | covered |
| N22-TS2-9 | SRS-REQ-060, SRS-REQ-027 | `link_ip.c` reference default configuration | nominal | `FF_LinkIpDefaultConfig(stack)` | `stack` stored; mode = `FF_LINK_IPMODE_UDP`; port = `FF_LINK_IP_DEFAULT_PORT` (8080); peer = 127.0.0.1 (`link_ip.h` documented reference profile; SRS-REQ-060 UDP reference) | SIL | individual | covered |
| N22-TS2-10 | SRS-REQ-060, SRS-REQ-039 | `link_ip.c` UDP send/recv, one datagram = one frame | nominal | `FF_LinkIpInit` UDP config over a mock/loopback `FF_NetStack_t`; `FF_LinkSelect(&FF_LinkIpBackend)`; send a frame (incl. the 1024-octet max frame); inject a datagram; recv | `FF_SUCCESS`; the datagram boundary delimits the frame (no seg prefix); the received frame is byte-identical to the sent frame (SRS-REQ-060 UDP datagram; FRICK-ARCH-0012 §6) | SIL | individual | covered |
| N22-TS2-11 | SRS-REQ-060 | `link_ip.c` UDP receive with no datagram | OOB | UDP `FF_LinkRecv` when the stack returns zero bytes | **gap** — no clause defines the UDP empty-receive contract. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: `FF_TIMEOUT`, `*len` untouched. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-12 | SRS-REQ-060 | `link_ip.c` partial send (UDP and TCP) | OOB | the stack `send` accepts fewer bytes than requested (both modes) | **gap** — no clause defines the partial-write contract. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: `FF_BUFFER_FULL`. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-13 | SRS-REQ-060 | `link_ip.c` invalid configuration | OOB | `FF_LinkIpInit(NULL)`; `FF_LinkIpDefaultConfig(NULL)` then `FF_LinkIpInit(&cfg)` | **gap** — no clause defines the invalid-config contract. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: `FF_INVALID_ARGUMENT`. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-14 | SRS-REQ-060, SRS-REQ-039 | `link_ip.c` selection before `FF_LinkIpInit` (reference fallback) | OOB | `FF_LinkSelect(&FF_LinkIpBackend)` with no prior `FF_LinkIpInit` | **gap** — `link_ip.h`/ICD do not define the no-config fallback (the implementation binds `FF_NetStackLoopback` in UDP mode; production requires `FF_LinkIpInit`). Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: the reference loopback stack is bound. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-15 | SRS-REQ-060, SRS-REQ-059, SRS-REQ-027 | `link_ip.c` TCP mode frames through the seg layer | nominal | `FF_LinkIpInit` TCP config; send a frame; recv it | `FF_SUCCESS`; on the wire the frame is preceded by the 2-byte BE length prefix; the receive leg reassembles and returns the frame byte-identical (SRS-REQ-060 TCP via seg; SRS-REQ-059; FRICK-ARCH-0012 §6) | SIL | individual | covered |
| N22-TS2-16 | SRS-REQ-060 | `link_ip.c` open/bind/listen/accept failure propagation | OOB | mock stack `open` returns -1; mock `bind`/`listen`/`accept` return failure | **gap** — no clause defines the return codes for open/bind/listen/accept failure. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_NO_RESOURCE` (open), propagated stack status (bind/listen), `FF_ERROR` (accept). No oracle is asserted here | SIL | individual | gap |
| N22-TS2-17 | SRS-REQ-059 | `link_seg.c` basic send/recv round trip | nominal | `FF_LinkSegInit`; `FF_LinkSegSendFrame` a 5-octet frame; `FF_LinkSegRecvFrame` | send = `FF_SUCCESS`; recv = `FF_SUCCESS` with `len` 5 and byte-identical payload (prefix stripped) (SRS-REQ-059 length-prefix profile; `link_seg.h`) | SIL | individual | covered |
| N22-TS2-18 | SRS-REQ-059 | `link_seg.c` length-prefix encoding (big-endian) | nominal | send a frame of length 0x0102 | first two wire octets = 0x01 0x02 (big-endian) then the payload; `FF_LINK_SEG_PREFIX_LEN` = 2 (SRS-REQ-059 "2-byte big-endian length-prefix") | SIL | individual | covered |
| N22-TS2-19 | SRS-REQ-059 | `link_seg.c` maximum-frame send/recv | nominal | send/recv `FF_LINK_SEG_MAX_FRAME` (1024) octets | `FF_SUCCESS`; prefix = 0x04 0x00; payload byte-identical (`link_seg.h` bound; SRS-REQ-059) | SIL | individual | covered |
| N22-TS2-20 | SRS-REQ-059 | `link_seg.c` send over the maximum (max+1) | OOB | `FF_LinkSegSendFrame(frame, 1025)` | `FF_BAD_SIZE`; no prefix or payload octets written (SRS-REQ-059 bounded framing; `FF_LINK_SEG_MAX_FRAME`) | SIL | individual | covered |
| N22-TS2-21 | SRS-REQ-059 | `link_seg.c` received length prefix over the maximum | OOB | prefix = 0xFFFF (65535 > `FF_LINK_SEG_MAX_FRAME`) | `FF_BAD_SIZE` (SRS-REQ-059 bounded reassembly; the declared frame must be ≤ max) | SIL | individual | covered |
| N22-TS2-22 | SRS-REQ-059 | `link_seg.c` receive buffer too small | OOB | valid prefix 0x0003 with `cap` = 2 | `FF_BUFFER_FULL` (SRS-REQ-059 bounded; the caller buffer cannot hold the declared frame) | SIL | individual | covered |
| N22-TS2-23 | SRS-REQ-059 | `link_seg.c` reassembly across stream reads | nominal | a frame delivered in small (~3-octet) chunks | `FF_SUCCESS`; the frame is reassembled byte-identical (SRS-REQ-059 "reassembly"; `link_seg.h` `rx_partial`) | SIL | individual | covered |
| N22-TS2-24 | SRS-REQ-059 | `link_seg.c` partial-frame timeout preserves state | nominal | prefix + (max−1) payload octets, then the stream returns 0 octets mid-frame | `FF_TIMEOUT`; `rx_len` = prefix + received octets with no `rx_partial` overflow; a later call resumes the frame (`link_seg.h` documented partial reassembly; SRS-REQ-059 bounded) | SIL | individual | covered |
| N22-TS2-25 | SRS-REQ-059 | `link_seg.c` two consecutive frames in one stream | nominal | send frame A then frame B; recv twice | both frames recovered in order, byte-identical (SRS-REQ-059 framing delimiter) | SIL | individual | covered |
| N22-TS2-26 | SRS-REQ-059 | `link_seg.c` zero-length frame (prefix 0x0000) | OOB | prefix = 0x0000 | **gap** — SRS-REQ-059 does not define whether a zero-length frame is legal and no clause fixes the expected result. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_SUCCESS` with `*len = 0`. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-27 | SRS-REQ-059 | `link_seg.c` stream error and de-synchronisation recovery | OOB | `read_bytes` returns `FF_ERROR` mid-frame; then a fresh valid prefixed frame is offered on the same `seg` | **gap** — SRS-REQ-059 defines framing but not error/resync recovery; after an over-max prefix the implementation returns `FF_BAD_SIZE` without consuming the prefix, permanently desynchronising the stream. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_ERROR` propagates; the bad prefix stays buffered. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-28 | SRS-REQ-059 | `link_seg.c` send-leg partial write | OOB | `write_bytes` succeeds for the prefix, then fails for the payload | **gap** — no clause defines send-leg partial-write/resynchronisation with the prefix already emitted. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: the failure status is propagated after the prefix is on the wire. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-29 | SRS-REQ-015 | `link_seg.c` bounded, allocation-free storage | nominal | inspect `rx_partial[]`; run the seg suite under ASan/LSan | no heap allocation after init; all storage static/stack (SRS-REQ-015; `link_seg.h` "no dynamic allocation") | SIL | individual | covered |
| N22-TS2-30 | SRS-REQ-061, SRS-REQ-028, SRS-REQ-026 | `link_tsn.c` default config and initialisation | nominal | `FF_LinkTsnDefaultConfig(mac, tsn)`; `FF_LinkTsnInit(&cfg)`; `FF_LinkSelect(&FF_LinkTsnBackend)` | vid = 0, pcp = 0, dest = broadcast ff:ff:ff:ff:ff:ff; init = `FF_SUCCESS`; the `FF_EthMac_t` contract is exercised (`init`, `get_mac_address`, `set_rx_callback`, `set_link_up`) and the `FF_TsnBackend_t` `set_vlan` seam when present (SRS-REQ-061/028/026; `link_tsn.h` defaults) | SIL | individual | covered |
| N22-TS2-31 | SRS-REQ-061 | `link_tsn.c` Ethernet frame construction (EtherType 0x88F5) | nominal | send a 1-octet payload; capture the MAC transmit buffer | frame[0..5] = dest MAC, frame[6..11] = source MAC (from the MAC driver), frame[12..13] = 0x88 0xF5, frame[14..] = payload; transmitted length ≥ 60 (SRS-REQ-061 EtherType 0x88F5; `link_tsn.h` layout; FRICK-ARCH-0012 §6) | SIL | individual | covered |
| N22-TS2-32 | SRS-REQ-061 | `link_tsn.c` minimum-frame padding boundary | nominal | payload sizes producing header+payload < 60 and ≥ 60 | when header+payload < 60 the frame is zero-padded to exactly 60 octets; when ≥ 60 no padding is added (`link_tsn.h` "TX zero-pads … to the 60-byte minimum") | SIL | individual | covered |
| N22-TS2-33 | SRS-REQ-061 | `link_tsn.c` payload size boundary | OOB | send payload = `FF_LINK_TSN_RX_FRAMESZ` (1024) then 1025; send payload length 0 | 1024 → accepted; 1025 → `FF_INVALID_ARGUMENT`; 0 → `FF_INVALID_ARGUMENT` (`link_tsn.c` bound; SRS-REQ-061 reference profile) | SIL | individual | covered |
| N22-TS2-34 | SRS-REQ-061 | `link_tsn.c` RX ring saturation (16 slots) | OOB | deliver 17 valid RX frames via the MAC callback | the first 16 are retrievable via `FF_LinkRecv`; the 17th is dropped (`FF_TIMEOUT` on its receive) (`FF_LINK_TSN_RX_SLOTS` = 16) | SIL | individual | covered |
| N22-TS2-35 | SRS-REQ-061 | `link_tsn.c` RX frame filtering (short / wrong EtherType / oversized) | OOB | deliver a < 14-octet frame; a frame with EtherType ≠ 0x88F5; a frame with payload > `FF_LINK_TSN_RX_FRAMESZ` | each frame is dropped; the ring stays empty (`FF_TIMEOUT` on receive) (SRS-REQ-061 profile; `link_tsn.h` EtherType + drop semantics) | SIL | individual | covered |
| N22-TS2-36 | SRS-REQ-061 | `link_tsn.c` invalid configuration | OOB | `FF_LinkTsnInit(NULL)`; a config with `mac` = NULL | **gap** — no clause defines the invalid-config contract. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: `FF_INVALID_ARGUMENT`. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-37 | SRS-REQ-061 | `link_tsn.c` receive into a too-small caller buffer | OOB | a queued 4-octet frame retrieved with `cap` = 2 | **gap** — no clause defines the small-buffer contract for TSN RX (the implementation drops the frame). Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_BUFFER_FULL`, frame discarded. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-38 | SRS-REQ-016 | `link_uart.c` device open and 115200 baud configuration | nominal | `FF_LinkSelect(&FF_LinkUartBackend)` with `FF_LINK_UART_DEVICE` openable | init = `FF_SUCCESS`; the device is opened and put in raw mode with `B115200` input and output speeds (ICD §2 "UART byte stream, 115200 baud"; `link_uart.c`) | SIL | individual | covered |
| N22-TS2-39 | SRS-REQ-016 | `link_uart.c` open failure | OOB | `FF_LINK_UART_DEVICE` cannot be opened | **gap** — no clause defines the UART open-failure return or the subsequent send/recv state. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_ERROR` from init and `FF_NOT_READY` from send/recv. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-40 | SRS-REQ-016 | `link_uart.c` UART configuration and byte transmit | nominal | paired pseudo-terminal; backend init and transmit one packet | init = `FF_SUCCESS`; transmitted bytes arrive in order at the peer (ICD §2 UART device and rate) | SIL | individual | covered |
| N22-TS2-41 | SRS-REQ-016 | `link_uart.c` short write and EAGAIN timeout | OOB | `write` returns fewer octets than requested; `read` returns with no data (`EAGAIN`) | **gap** — no clause defines the partial-write or EAGAIN contract. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: `FF_ERROR` (short write), `FF_TIMEOUT` (EAGAIN). No oracle is asserted here | SIL | individual | gap |
| N22-TS2-42 | SRS-REQ-034, SSS-REQ-019 | `ff_sdls.c` pluggable cryptographic backend registration | nominal | `FF_SdlsInit()`; `FF_SdlsSetBackend(&FF_CryptoTestBackend)`; `FF_SdlsGetBackend()` | init = `FF_SUCCESS`; the backend is registered and returned by `FF_SdlsGetBackend`; its `init()` is invoked (SRS-REQ-034 "pluggable cryptographic backend"; `ff_sdls.h`) | SIL | individual | covered |
| N22-TS2-43 | SRS-REQ-034 | `ff_sdls.c` backend IV-length enforcement (`iv_len >= 4`) | OOB | `FF_SdlsSetBackend` with `iv_len` = 3 and `iv_len` = 0 | `FF_INVALID_ARGUMENT`; backend not registered (FRICK-ARCH-0010 §4 "the backend must provide `iv_len >= 4`, enforced by `FF_SdlsSetBackend`") | SIL | individual | covered |
| N22-TS2-44 | SRS-REQ-034 | `ff_sdls.c` backend key/MAC length and function-pointer validation | OOB | `key_len` = 0 or > `FF_SDLS_KEY_MAX`; `mac_len` = 0 or > `FF_SDLS_MAC_MAX`; a NULL `init`/`encrypt`/`mac`/`key_derive`; a NULL backend | **gap** — `ff_sdls.h` declares the fields but no requirement/architecture clause defines the rejection contract for out-of-range key/MAC lengths or missing operations (only `iv_len >= 4` is controlled). Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_INVALID_ARGUMENT`. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-45 | SRS-REQ-035, SSS-REQ-019 | `ff_sdls.c` SA add/get/activate/remove lifecycle | nominal | add an AUTH_ENC SA; `FF_SdlsSaGet`; `FF_SdlsSaActivate(spi,1)`; `FF_SdlsSaRemove`; get the removed SPI | add = `FF_SUCCESS`; before activation `active` = 0; activate sets `active` = 1; remove decrements `FF_SdlsSaCount` and the removed SPI → `FF_NOT_FOUND` (SRS-REQ-035 SA lifecycle) | SIL | individual | covered |
| N22-TS2-46 | SRS-REQ-035 | `ff_sdls.c` duplicate SPI rejection | OOB | add the same SPI twice | `FF_ALREADY_EXISTS`; count unchanged (SRS-REQ-035 "SPI-indexed" database — unique index) | SIL | individual | covered |
| N22-TS2-47 | SRS-REQ-035 | `ff_sdls.c` bounded capacity (`FF_SDLS_MAX_SA` = 8, plus one) | OOB | add `FF_SDLS_MAX_SA` SAs, then a 9th | the first 8 → `FF_SUCCESS` with `FF_SdlsSaCount` = 8; the 9th → `FF_ERROR` and count unchanged (SRS-REQ-035 "bounded … database"; `FF_SDLS_MAX_SA`) | SIL | individual | covered |
| N22-TS2-48 | SRS-REQ-035 | `ff_sdls.c` session-key derivation bound to `(SPI, epoch)` | nominal | add two SAs with the same master key but different SPIs; get each session key | the derived session keys differ by SPI; `session_key_len` = backend `key_len` (16) (FRICK-ARCH-0010 §3 key derivation) | SIL | individual | covered |
| N22-TS2-49 | SRS-REQ-035 | `ff_sdls.c` key rotation: epoch, usage and key change | nominal | `FF_SdlsRotateKey(spi, now_ns)` on an AUTH_ENC SA | `FF_SUCCESS`; epoch = previous + 1 (strictly increasing); `tx_count` = 0; `iv_counter` = 0; `activation_ns` = `now_ns`; the session key differs from the previous key (SRS-REQ-035 rotation; FRICK-ARCH-0010 §3; Lean `req_srs035_epoch_monotonic`, `req_srs035_uses_reset`) | SIL | individual | covered |
| N22-TS2-50 | SRS-REQ-035 | `ff_sdls.c` rotation of a PLAIN SA | OOB | `FF_SdlsRotateKey` on an SA with service = PLAIN | **gap** — SRS-REQ-035 addresses rotation of derived session keys; the PLAIN (no-key) case is not defined. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_INVALID_ARGUMENT`. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-51 | SRS-REQ-035 | `ff_sdls.c` policy tick rotates over-threshold SAs | nominal | an active SA with `max_tx` = 1 after one protect, then `FF_SdlsTick(now)`; separately an SA whose `lifetime_ns` has elapsed | tick rotates the over-threshold active SA(s): epoch increases and `tx_count` resets; inactive/PLAIN SAs are skipped (SRS-REQ-035 "rotation on frame and time thresholds") | SIL | individual | covered |
| N22-TS2-52 | SRS-REQ-035 | `ff_sdls.c` tick when rotation fails | OOB | an active SA whose backend `key_derive` fails at rotation time | **gap** — no clause defines the tick's failure propagation contract. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_SdlsTick` returns `FF_ERROR` and the prior session key is retained. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-53 | SRS-REQ-035 | `ff_sdls.c` SA removal zeroises key material and frees the slot | nominal | remove a middle SA, then add a new SA | the removed SPI → `FF_NOT_FOUND`; the count decrements; the freed slot is reusable; key material is wiped (SRS-REQ-035 "zeroise key material … on removal") | SIL | individual | covered |
| N22-TS2-54 | SRS-REQ-035 | `ff_sdls.c` global zeroisation | nominal | `FF_SdlsZeroize()` after adding SAs | `FF_SUCCESS`; `FF_SdlsSaCount` = 0; every prior SPI → `FF_NOT_FOUND` (SRS-REQ-035 zeroisation) | SIL | individual | covered |
| N22-TS2-55 | SRS-REQ-015 | `ff_sdls.c` bounded, allocation-free SA state | nominal | inspect `g_sa[FF_SDLS_MAX_SA]`; run the SDLS suite under ASan/LSan | no heap allocation after init; the SA table is static and bounded by `FF_SDLS_MAX_SA` (SRS-REQ-015; `ff_sdls.h` "holds no dynamic memory") | SIL | individual | covered |
| N22-TS2-56 | SRS-REQ-035 | `ff_sdls.c` SA add for a non-PLAIN service before a backend is set | OOB | `FF_SdlsSaAdd(&sa)` with service = AUTH and no `FF_SdlsSetBackend` | **gap** — no clause defines the pre-backend SA-add contract. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_NOT_READY`. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-57 | SRS-REQ-035 | `ff_sdls.c` SA add with master-key/IV length mismatch | OOB | service = AUTH_ENC with `master_key_len` ≠ backend `key_len`, or `iv_len` ≠ backend `iv_len` | **gap** — FRICK-ARCH-0010 §4 fixes `iv_len >= 4` but no clause states the SA↔backend length-equality rule or its rejection. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_INVALID_ARGUMENT`. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-58 | SRS-REQ-034 | `ff_sdls.c` AUTH_ENC protect wire layout | nominal | AUTH_ENC SA (iv_len 12, mac_len 16); protect a 20-octet payload | `FF_SUCCESS`; `out_len` = 2 + 12 + 20 + 16 = 50; out[0..1] = SPI big-endian; out[2..13] = IV (base with the per-frame counter big-endian in the last 4 octets); the payload region is encrypted; the MAC over `SPI` + `IV` + `payload` is appended (SRS-REQ-034; FRICK-ARCH-0010 §4) | SIL | individual | covered |
| N22-TS2-59 | SRS-REQ-034 | `ff_sdls.c` AUTH_ENC protect/unprotect round trip | nominal | protect then unprotect a fixed payload | protect = `FF_SUCCESS`; unprotect = `FF_SUCCESS`; `out_len` = original length; `out` byte-identical to the payload (SRS-REQ-034) | SIL | individual | covered |
| N22-TS2-60 | SRS-REQ-034 | `ff_sdls.c` per-service wire lengths | nominal | protect a fixed payload with PLAIN, AUTH, ENC and AUTH_ENC | PLAIN → `in_len` + 2; AUTH → `in_len` + 2 + iv_len + mac_len; ENC → `in_len` + 2 + iv_len; AUTH_ENC → `in_len` + 2 + iv_len + mac_len (SRS-REQ-034 configurable service; FRICK-ARCH-0010 §4) | SIL | individual | covered |
| N22-TS2-61 | SRS-REQ-034 | `ff_sdls.c` PLAIN bypass passthrough | nominal | PLAIN SA (`master_key_len` = 0, `iv_len` = 0); protect then unprotect | protect → out[0..1] = SPI big-endian and the payload passed through unmodified, `out_len` = `in_len` + 2; unprotect restores the payload (SRS-REQ-034 PLAIN bypass) | SIL | individual | covered |
| N22-TS2-62 | SRS-REQ-034 | `ff_sdls.c` tampered-unit rejection (constant-time MAC compare) | OOB | flip one octet of the encrypted payload, of the IV, and of the MAC of a valid AUTH_ENC unit; unprotect each | `FF_PARSE_ERROR` for every tampered variant; the untampered unit → `FF_SUCCESS` (SRS-REQ-034 validation; FRICK-ARCH-0010 §4 "constant-time comparison and rejects tampered units") | SIL | individual | covered |
| N22-TS2-63 | SRS-REQ-034 | `ff_sdls.c` unprotect unknown SPI / under-length unit | OOB | unprotect a unit whose SPI is not in the database; `in_len` = 0, 1; `in_len` < 2 + iv_len + mac_len | **gap** — SRS-REQ-034 defines SDLS-header/MAC validation but no clause fixes the return codes for an unknown SPI or an under-length unit. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_NOT_FOUND`, `FF_INVALID_ARGUMENT`, `FF_BAD_SIZE`. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-64 | SRS-REQ-034 | `ff_sdls.c` protect on an inactive SA / unknown SPI | OOB | `FF_SdlsProtect` with `active` = 0; then with an SPI not in the database | **gap** — no clause defines the protect state/SPI preconditions. Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: `FF_NOT_READY` (inactive), `FF_NOT_FOUND` (unknown SPI). No oracle is asserted here | SIL | individual | gap |
| N22-TS2-65 | SRS-REQ-034 | `ff_sdls.c` protect output-capacity boundary | OOB | AUTH_ENC with `out_cap` = exact total, then `out_cap` = total − 1; PLAIN with `out_cap` = `in_len` + 1 | exact → `FF_SUCCESS`; total − 1 → `FF_BAD_SIZE`; PLAIN `in_len` + 1 → `FF_BAD_SIZE` (SRS-REQ-034 bounded unit; `ff_sdls.h` `out_cap` contract) | SIL | individual | covered |
| N22-TS2-66 | SRS-REQ-034 | `ff_sdls.c` protect input length that overflows the 16-bit total | OOB | protect `in_len` = 0xFFF0 with a normal backend | **gap** — no requirement clause defines the over-length rejection contract for the 16-bit unit length. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: `FF_BAD_SIZE`, nothing written. No oracle is asserted here | SIL | individual | gap |
| N22-TS2-67 | SRS-REQ-034 | `ff_sdls.c` IV uniqueness across consecutive protects | nominal | protect three frames on one SA; inspect the IV field of each | the per-frame counter increments; the last 4 IV octets strictly increase and are unique; the IV base octets are unchanged (FRICK-ARCH-0010 §4 "per-frame counter … giving IV uniqueness") | SIL | individual | covered |
| N22-TS2-68 | SRS-REQ-034 | `ff_sdls.c` MAC covered range (`SPI` + `IV` + `payload`) | nominal | unprotect a unit whose MAC was computed over the correct range, then over a shifted range | the correctly-computed MAC passes (`FF_SUCCESS`); the shifted-range MAC fails (`FF_PARSE_ERROR`) (FRICK-ARCH-0010 §4 "computed over `SPI` + `IV` + `payload`") | SIL | individual | covered |
| N22-TS2-69 | SRS-REQ-035 | `ff_sdls.c` anti-replay / sequence-number handling | OOB | replay a previously accepted protected unit | **gap** — FRICK-ARCH-0010 §5 states anti-replay/sequence-number semantics are mission-tailored and the middleware secures a generic data unit; no controlled oracle exists (the IV/counter uniqueness analysis is GAP-A14). Open TBD (NC-EXT-47 §7.3). Uncontrolled observation only: the unit is re-accepted (no replay window). No oracle is asserted here | SIL | individual | gap |
| N22-TS2-70 | SRS-REQ-060, SRS-REQ-027, SSS-REQ-028 | PUS-C TM/TC over the IP link end to end | nominal | mock/loopback `FF_NetStack_t`; `FF_LinkIpInit` (UDP); `FF_LinkSelect(&FF_LinkIpBackend)`; S17 ping TC in via the link; `FF_PusIngest`; TM out via the link | the TC decodes; the S17 reply and the S1 lifecycle TMs are handed by `FF_LinkSend` to the stack and round-trip byte-identical (SRS-REQ-060 `Dem`; existing `test_pus_ip_roundtrip`) | SIL | combined | covered |
| N22-TS2-71 | SRS-REQ-061, SRS-REQ-026 | PUS-C TM/TC over the TSN link end to end | nominal | mock MAC looped back + mock TSN; `FF_LinkTsnInit`; S17 ping TC in; reply out | the TM returns over the link with the PUS header service/subservice preserved at packet octets 7/8 (the RX length includes zero padding) (SRS-REQ-061 `Dem`; `link_tsn.h` length semantics; existing `test_pus_tsn_roundtrip`) | SIL | combined | covered |
| N22-TS2-72 | SRS-REQ-059, SRS-REQ-060, SSS-REQ-028 | TCP-segmented PUS frames end to end | nominal | TCP mode; send a PUS TM frame spanning multiple read chunks and recv it | the seg prefix frames the TM; reassembly returns the TM byte-identical and `FF_PusIngest` accepts it (SRS-REQ-059 + SRS-REQ-060) | SIL | combined | covered |
| N22-TS2-73 | SRS-REQ-034, SRS-REQ-059, SRS-REQ-060 | SDLS-protected PUS frame carried over the link | nominal | protect a CCSDS/PUS frame with an AUTH_ENC SA, carry the protected unit over the IP link, unprotect on receipt | **gap** — SDLS is not wired into any link backend, `fsw_main.c` or application (only `tests/unit/test_sdls.c` calls `FF_Sdls*`); there is no controlled secure-link requirement or integrated path, so no oracle exists (NC-EXT-47 §7.1 for the combined path, §7.3 for the missing contract; key/HSM = GAP-A14). No oracle is asserted here | SIL | combined | gap |
| N22-TS2-74 | SRS-REQ-027, SRS-REQ-039, SRS-REQ-060 | IP link backend over two pluggable stacks | nominal | select the IP backend over `FF_NetStackLoopback` and over the mock `FF_NetStack_t`; UDP round trip on each | the same PUS frame round-trips over both stacks without a change to the backend (SRS-REQ-027 pluggable stack; SRS-REQ-039 reference stack; SRS-REQ-060) | SIL | combined | covered |
| N22-TS2-75 | SRS-REQ-060, SRS-REQ-061 | Link backend ↔ ground support tool end to end | nominal | host simulator link backend ↔ `tools/gsw/ff_gsw.py` exchange of PUS-C TM/TC | **gap** — `tests/python/test_ff_gsw.py` now covers fixed vectors and link-independent fragmented/coalesced serial stream decoding, but no per-requirement host link↔GSW integration or independent ground-segment exchange is available (NC-EXT-03; NC-EXT-47 §7.1/§7.6). No oracle is asserted here | SIL | combined | gap |
| N22-TS2-76 | SRS-REQ-044, SRS-REQ-034, SRS-REQ-035 | SDLS functional check on the emulated PolarFire SoC (Renode) | nominal | Renode run of `app_renode_test` exercising SDLS protect/unprotect | **gap** — no Renode check exercises the SDLS sub-layer; the `RENODE_TEST: PASS` aggregate marker is not a per-requirement oracle (NC-EXT-47 §7.6). No oracle is asserted here | PIL | individual | gap |
| N22-TS2-77 | SRS-REQ-044, SRS-REQ-060, SRS-REQ-061 | Link backend functional check on the emulated PolarFire SoC (Renode) | nominal | Renode run of `app_renode_test` driving an `FF_Link_t` backend (IP/TSN/UART) round trip | **gap** — the Renode test application exercises the `hal_*` seams (`hal_eth`, `hal_net`, `hal_tsn`, …) but never selects nor drives `FF_LinkIpBackend`/`FF_LinkTsnBackend`/`FF_LinkUartBackend`; `RENODE_TEST: PASS` is not a per-requirement oracle (NC-EXT-47 §7.6). No oracle is asserted here | PIL | combined | gap |
| N22-TS2-78 | SRS-REQ-060, SRS-REQ-061 | On-target link-level PUS-C over UART/IP/TSN (HIL) | nominal | target RTPFS160ZT; PUS-C TM/TC over the flight UART and mission IP/TSN link | **gap** — on-silicon framing, TCP stream segmentation, UDP/TSN timing and electrical integrity require flight hardware; GAP-A5 / NC-EXT-31 (NC-EXT-47 §7.5). No oracle is asserted here | HIL | combined | gap |
| N22-TS2-79 | SRS-REQ-016 | On-target UART byte stream at 115200 baud (HIL) | nominal | target `/dev/console` UART at 115200 8N1 | **gap** — on-target UART timing and line integrity at the configured baud require flight hardware; GAP-A5 (NC-EXT-47 §7.5). No oracle is asserted here | HIL | individual | gap |
| N22-TS2-80 | SRS-REQ-035 | Real cryptographic backend, HSM and ground key management (HIL) | nominal | certified CCSDS 352.0-B algorithm + hardware keystore; IV-uniqueness and key-distribution procedures | **gap** — GAP-A14: algorithm/HSM qualification, protected zeroisation, IV/counter uniqueness analysis and ground key management are mission/HW-supplied (FRICK-ARCH-0010 §7; NC-EXT-47 §7.5). No oracle is asserted here | HIL | combined | gap |
| N22-TS2-81 | NC-EXT-07 characterization; no approved SRS oracle | `link_uart.c` packet framing over split/coalesced UART stream | OOB | PTY sends partial TC, receive timeout, remainder + second packet; too-small `cap`; over-limit PDL then valid packet; maximum frame; plausible maximum-length incomplete PDL followed by a valid packet and enough suffix to complete the declared candidate; close PTY master and read with a longer timeout | split frame is retained on timeout; complete frames return one per call byte-identically; too-small `cap` returns `FF_BAD_SIZE` without consuming the frame; over-limit candidate resynchronizes one octet; maximum 1037-octet frame is intact. For plausible incomplete candidate, return `FF_TIMEOUT`, emit no inner packet, then return the retained entire declared-length candidate only after its remaining bytes arrive (no sync delimiter permits guessing); a zero-byte read substantially before its configured VTIME chunk returns `FF_ERROR` on disconnect rather than retrying to deadline. This exposes the open malformed-candidate/head-of-line recovery policy (ICD §2/§3) | SIL | individual | characterization of current host reference behavior only; no requirement closure, timeout/recovery authority or target evidence |

## 3. Gaps (mapping to NC-EXT-47 sub-items)

All gaps are recorded, not resolved (N21). Each row below maps a `gap` case to
the NC-EXT-47 sub-item in FRICK-VV-TSPEC-001 §7 that it belongs to, and names
the specific open NC/TBD where one exists. Gap rows assert **no** oracle.

**NC-EXT-47 §7.1 — combined-I/O scenarios not systematically defined.**

- `N22-TS2-73` — SDLS + link + PUS combined secure path: SDLS is not wired into
  any link backend/application, so no controlled combined oracle exists
  (TBD; NC-EXT-47 §7.1/§7.3; GAP-A14).
- `N22-TS2-75` — link backend ↔ GSW end-to-end: no automated per-requirement
  oracle (the GSW serial session is manual) (TBD; NC-EXT-47 §7.1/§7.6).

**NC-EXT-47 §7.2 — software-only robustness oracles not specified as a
controlled, requirement-mapped set.**

- `N22-TS2-3` — `FF_LinkSelect(NULL)` (TBD; NC-EXT-47 §7.2).
- `N22-TS2-4` — `FF_LinkSend`/`FF_LinkRecv` before selection (TBD;
  NC-EXT-47 §7.2).
- `N22-TS2-6` — loopback empty-ring receive (TBD; NC-EXT-47 §7.2).
- `N22-TS2-7` — loopback ring-full / zero-length boundary (TBD;
  NC-EXT-47 §7.2).
- `N22-TS2-11` — IP UDP empty receive (TBD; NC-EXT-47 §7.2).
- `N22-TS2-12` — IP partial send `FF_BUFFER_FULL` (TBD; NC-EXT-47 §7.2).
- `N22-TS2-13` — `FF_LinkIpInit` NULL config/stack (TBD; NC-EXT-47 §7.2).
- `N22-TS2-36` — `FF_LinkTsnInit` NULL config/MAC (TBD; NC-EXT-47 §7.2).
- `N22-TS2-41` — UART short write / EAGAIN timeout (TBD; NC-EXT-47 §7.2).
- `N22-TS2-66` — SDLS protect input length overflowing the 16-bit total (TBD;
  NC-EXT-47 §7.2).

**NC-EXT-47 §7.3 — out-of-bound oracles not defined by a controlled clause
(unresolved/absent contract).**

- `N22-TS2-14` — IP backend selected before `FF_LinkIpInit` (reference
  fallback) (TBD; NC-EXT-47 §7.3).
- `N22-TS2-16` — IP open/bind/listen/accept failure return codes (TBD;
  NC-EXT-47 §7.3).
- `N22-TS2-26` — seg zero-length frame (prefix 0x0000) (TBD;
  NC-EXT-47 §7.3).
- `N22-TS2-27` — seg stream-error / de-synchronisation recovery (TBD;
  NC-EXT-47 §7.3).
- `N22-TS2-28` — seg send-leg partial write after the prefix (TBD;
  NC-EXT-47 §7.3).
- `N22-TS2-37` — TSN receive into a too-small buffer (TBD; NC-EXT-47 §7.3).
- `N22-TS2-39` — UART open failure (TBD; NC-EXT-47 §7.3).
- `N22-TS2-44` — SDLS backend key/MAC length and function-pointer validation
  (TBD; NC-EXT-47 §7.3).
- `N22-TS2-50` — SDLS rotation of a PLAIN SA (TBD; NC-EXT-47 §7.3).
- `N22-TS2-52` — SDLS policy tick when rotation fails (TBD; NC-EXT-47 §7.3).
- `N22-TS2-56` — SDLS SA add before a backend is set (TBD; NC-EXT-47 §7.3).
- `N22-TS2-57` — SDLS SA add with a master-key/IV length mismatch (TBD;
  NC-EXT-47 §7.3).
- `N22-TS2-63` — SDLS unprotect unknown SPI / under-length unit (TBD;
  NC-EXT-47 §7.3).
- `N22-TS2-64` — SDLS protect on an inactive SA / unknown SPI (TBD;
  NC-EXT-47 §7.3).
- `N22-TS2-69` — SDLS anti-replay/sequence-number handling: mission-tailored,
  not implemented (FRICK-ARCH-0010 §5; TBD; NC-EXT-47 §7.3; GAP-A14).

**NC-EXT-47 §7.5 — target (HIL) evidence remains GAP-A5 / NC-EXT-31.**

- `N22-TS2-78` — on-target link-level PUS-C over UART/IP/TSN (GAP-A5 /
  NC-EXT-31).
- `N22-TS2-79` — on-target UART byte stream at 115200 baud (GAP-A5).
- `N22-TS2-80` — real cryptographic backend / HSM / ground key management
  (GAP-A14).

**NC-EXT-47 §7.6 — emulated-target aggregate marker is not a per-requirement
oracle.**

- `N22-TS2-76` — SDLS is not exercised by the Renode test application; the
  `RENODE_TEST: PASS` marker does not evidence SRS-REQ-034/035 (TBD;
  NC-EXT-47 §7.6).
- `N22-TS2-77` — no `FF_Link_t` backend round trip is driven by the Renode test
  application; the aggregate marker does not evidence SRS-REQ-060/061 (TBD;
  NC-EXT-47 §7.6).

### 3.1 Coverage summary

| Metric | Count |
| --- | --- |
| Total cases | 81 |
| Nominal | 44 |
| Out-of-bound | 37 |
| Individual | 72 |
| Combined | 9 |
| `covered` (oracle grounded in a requirement/architecture/interface clause) | 48 |
| `gap` (no controlled oracle; recorded under NC-EXT-47) | 32 |
| `characterization` (NC-driven; no requirement closure) | 1 |
| SIL / PIL / HIL | 76 / 2 / 3 |

Requirement coverage in this scope: `SRS-REQ-015/016/026/027/028/034/035/039/
044/059/060/061` and `SSS-REQ-014/019/028` all have at least one case; the link
selection and dispatch API, all four reference backends, the segmentation and
reassembly layer, the UART stream framing, and the SDLS backend/SA/key-lifecycle
and protect/unprotect surfaces have nominal and (where a controlled contract
exists) out-of-bound cases. Automated evidence includes
`tests/unit/test_link_uart.c` (host PTY), `test_link_ip.c`, `test_link_tsn.c`,
`test_netloop.c` and `test_sdls.c`. The loopback backend still has no dedicated
unit-test target; its cases are specified but not automated.

## History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-14 | frick-fsw | Initial TS-2 work package per N22 and the FRICK-VV-TSPEC-002 chunk plan: nominal + out-of-bound case table with explicit oracles for the link core/selection, loopback, IP (UDP/TCP via seg), TSN and UART backends, the PUS-C segmentation/reassembly layer (SRS-REQ-059) and the CCSDS SDLS sub-layer (SRS-REQ-034/035); gaps mapped to NC-EXT-47 sub-items with the open NC/TBD named |
| 0.2 | 2026-10-15 | frick-fsw | Clarify that new GSW stream-fragmentation tests are link-independent and do not close the host link↔GSW or external ground-segment E2E gap |
| 0.3 | 2026-10-15 | frick-fsw | Add host-PTY evidence for production UART split/coalesced PDL framing, timeout retention, undersized caller capacity and max packet; update case counts |
| 0.4 | 2026-10-15 | frick-fsw | Add explicit plausible-incomplete-header retention/release oracle and distinguish test evidence from authority-owned recovery policy |
| 0.5 | 2026-10-15 | frick-fsw | Add SRS-REQ-017 to the scope list cited by UART framing case N22-TS2-81 |
| 0.6 | 2026-10-15 | frick-fsw | Remove SRS-REQ-017 mapping because UART PDL framing does not validate the APID/mission-identifier profile |
| 0.7 | 2026-10-16 | frick-fsw | Add explicit disconnected-terminal zero-read outcome to N22-TS2-81; preserve malformed plausible-candidate recovery as open |
| 0.8 | 2026-10-16 | frick-fsw | Remove unsupported SRS-REQ-016 verification mapping from host UART characterization case per independent V&V review |
| 0.9 | 2026-10-16 | frick-fsw | Define characterization-only status for N22-TS2-81 and reconcile coverage counts after independent V&V review |
