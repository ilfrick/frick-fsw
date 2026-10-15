# Test Specification TS-6 — Software-only robustness (memory safety, numeric protection, concurrency/timing, fault injection, resources)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-TSPEC-TS6                     |
| Revision                         | 1.1                                    |
| Date                             | 2026-10-14                             |
| Norms                            | N10, N11, N22                          |
| Umbrella                         | `docs/vv/comprehensive-test-spec.md` (FRICK-VV-TSPEC-001), §6 |
| Work-package index               | `docs/vv/test-spec/README.md` (FRICK-VV-TSPEC-002), work package TS-6 |
| Companion                        | `docs/vv/system-verification-architecture.md` (FRICK-VV-SVA-001) |
| Scope                            | Software-only robustness: memory safety, numeric/overflow protection, PAL concurrency and timing edges, software-level fault injection and resource bounds; **no I/O and no functional precondition** |
| Baseline                         | Working tree at HEAD `57974d2` as read 2026-10-14 (uncommitted changes possible) |
| Modules under test               | `src/core/pal/pal_posix.c`, `src/core/sb/sb.c`, `src/core/evs/evs.c`, `src/core/es/es.c`, `src/core/tbl/tbl.c`, `src/transport/ccsds/ccsds.c`, `src/pus/pus.c`, `src/drivers/mem/ff_mem_static.c`, `src/math/ff_math.c`, `src/hal/hal.c` |
| Interfaces cited                 | `pal.h`, `sb.h`, `evs.h`, `es.h`, `tbl.h`, `hal_mem.h`, `math/ff_math.h`, `transport/ccsds/ccsds.h`, `core/common/ff_types.h`, `config/fsw_config.h`; `docs/architecture/software-bus.md` (FRICK-ARCH-0002), `docs/architecture/numerics.md` (FRICK-ARCH-0009) |

> **Authority statement.** This work package defines verification *cases and
> oracles* for the current baseline. It does not assign criticality, accept
> residual risk, qualify tools, close an NC, or approve release/flight
> readiness. Requirement/interface gaps are recorded (never resolved by
> inventing an oracle); disposition and closure require the authorized
> authorities.

## 1. Scope and method

### 1.1 In scope

Cases derived from the requirements in this work package's area that are present
in `docs/requirements/traceability.yaml`:

`SRS-REQ-001` (SB delivery semantics), `SRS-REQ-002` (SB FIFO ordering),
`SRS-REQ-003` (CCSDS PDL widened arithmetic and exact packet boundary),
`SRS-REQ-004` (CRC-16/CCITT-FALSE trailer and reject-on-mismatch),
`SRS-REQ-007` (PUS TM counter, mod 16384), `SRS-REQ-009` (memory-supervision
region bound), `SRS-REQ-010` (S9 time correlation), `SRS-REQ-011` (S5 event
forwarding), `SRS-REQ-013` (atomic table activation), `SRS-REQ-014`
(application lifecycle and Health & Safety detection), `SRS-REQ-015`
(deterministic memory, no dynamic allocation after init), `SRS-REQ-024` (CUC
framing forms and fine-fraction bound), `SRS-REQ-031` (GPIO interrupt contract:
ISR defers to a task), `SRS-REQ-033` (mathematics facade and out-of-domain
policy), `SRS-REQ-037` (RTEMS application integration and runtime sizing),
`SRS-REQ-044` (Renode functional-validation environment, cited only by the PIL
gap case `N22-TS6-48`), `SRS-REQ-049` (bounded, heap-free static RAM pool),
`SRS-REQ-051` (multicore/SMP, cited only by the HIL gap case `N22-TS6-32`) and
`SRS-REQ-052` (PUS-C service behaviour behind the backend contract). SSS parents
carried by the same modules: `SSS-REQ-002` (bounded SB queue), `SSS-REQ-008`
(HS heartbeat and health checks), `SSS-REQ-010` (deterministic memory),
`SSS-REQ-030` (bounded table-driven dispatch).

Out of scope (owned by other work packages): the nominal and boundary coverage
of each module surface — CCSDS/PUS/time (TS-1), link/SDLS (TS-2), the I/O
drivers and HAL seams (TS-3/TS-4) and the core services' functional behaviour
(TS-5). TS-6 owns only the cross-cutting **robustness property and its oracle**
(misuse, exhaustion, overflow, timing, fault). Where a case touches a surface
owned by a sibling package, the sibling remains the owner of the nominal
per-module behaviour; this package cites it only as a fixture.

### 1.2 Class, scope, environment and status conventions

- **Class**: `nominal` = a legal input inside the specified operating envelope
  (this includes legal minimum/maximum boundary values); `OOB` = a
  range-violating, malformed, out-of-envelope, resource-exhausted or
  prohibited-ordering input.
- **Scope**: `individual` = one module surface is under test (other modules are
  only fixtures); `combined` = the case deliberately asserts coupling across two
  or more modules/layers in one scenario (e.g. SB over the PAL queue,
  PUS counter + time + EVS, or an init-order fault across PAL/ES/SB).
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
  tool/environment is not established), so the case is recorded against
   **NC-EXT-47** and **not** executed to a fabricated oracle.
 - **Exploratory status**: `exploratory` is permitted only for a deliberately
   labeled stress hypothesis with explicit expected observations derived from
   configured structural bounds. It is reported separately and earns no
   requirement-coverage, mission-envelope, timing, target, or ground-delivery
   credit. It is distinct from `covered` and `gap`.
- **Gap discipline (N22)**: every `OOB` row (and any nominal row) whose expected
  behaviour is not defined by a requirement/architecture/interface clause is
  `gap`; the oracle cell names the open NC/TBD and, where applicable, an
  uncontrolled *observation* (explicitly not an oracle). The Gaps list (§3) maps
  every gap row to an NC-EXT-47 sub-item (FRICK-VV-TSPEC-001 §7).
- **No I/O, no functional precondition**: every case below is executable on the
  host from a fresh process with only `FF_PalInit()` and the module init calls
  as setup; no bus, device, network, file or ground-segment interaction is
  required (HIL/PIL rows are recorded only as environment-bound gaps).

### 1.3 Interface clauses used as oracle ground

| Clause | Content used as oracle ground |
| --- | --- |
| `SRS.md` §2–§5 | `SRS-REQ-001` (copy into every matching + wildcard pipe), `SRS-REQ-002` (FIFO per pipe), `SRS-REQ-003` (widened PDL arithmetic, minimum PUS-secondary-header/CRC size, exact supplied packet boundary), `SRS-REQ-004` (CRC-16/CCITT-FALSE trailer, reject on mismatch), `SRS-REQ-007` (TM counter strictly increases and wraps mod 16384), `SRS-REQ-013` (activation atomic; staged image active only on explicit activate), `SRS-REQ-014` (independent tasks, ES states, HS detection), `SRS-REQ-015` (no dynamic allocation after init) |
| `SRS.md` §4/§6 | `SRS-REQ-011` (every on-board EVS event forwarded to ground), `SRS-REQ-024` (7-octet self-describing CUC lossless, rejects wrong length/P-field; 6-octet T-field accepted only at exactly 6 octets), `SRS-REQ-033` (facade over libm, documented out-of-domain behaviour, tolerance-based tests), `SRS-REQ-049` (bounded, heap-free bump allocator over a caller-provided region), `SRS-REQ-052` (in-house PUS-C behaviour behind the backend) |
| FRICK-ARCH-0002 §2 | SB **overflow policy**: publish never blocks; a full pipe drops the new message **and raises an EVS error event** |
| FRICK-ARCH-0002 §4 | All SB pools are static tables sized at compile time; publish/lock guarded by the PAL mutex |
| FRICK-ARCH-0009 §3 | Facade domain behaviour: `sqrt(x<0)=NaN`, `log(x<=0)=-inf/NaN`; callers needing a fallback test with `FF_MathIsFinite()`; `errno` shall not be relied on |
| FRICK-ARCH-0009 §4 | The facade is reentrant and thread-safe: stateless wrappers, no shared mutable state, no dynamic allocation; transcendental functions not for hard real-time use without WCET/stack analysis (GAP-A4) |
| `pal.h`, `sb.h`, `evs.h`, `tbl.h`, `hal_mem.h`, `ff_math.h`, `ccsds.h`, `ff_types.h` | declared API/contract: status codes (`FF_SUCCESS=0` and the negative error set), opaque small-integer handles with `FF_INVALID_ID`/`FF_TBL_INVALID_HANDLE`, PAL bounds (`FF_PAL_MAX_TASKS`/`MUTEXES`/`QUEUES` = 32, `FF_PAL_MAX_QUEUE_DEPTH` = 32, `FF_PAL_MAX_ITEM_SIZE` = 1024, `FF_PAL_WAIT_FOREVER`), EVS "circular on-board log", TBL `validate` optional/`NULL`, `FF_MemPool_t` `alloc`/`reset` shape, `FF_MathClamp` requires `lo <= hi` and returns NaN unclamped, `FF_SB_*` and `FF_EVS_LOG_DEPTH`/`FF_TBL_MAX_*` bounds from `config/fsw_config.h` |
| FRICK-VV-TSPEC-001 §6 | TC-MEM acceptance oracles (ASan: no report; UBSan: no report; MSan where available; boundary min-1/max+1; stack depth ≤ bound; alignment/aliasing on packed wire structs) and TC-NUM oracles (PDL widen/wrap, counters mod-16384, CUC fine bound, saturation/NaN policy) |

## 2. Test cases

Case IDs `N22-TS6-1` … `N22-TS6-54`; groups: `1–12` memory safety and bounded
static storage, `13–24` numeric protections and overflow, `25–36` concurrency,
timing and service edges, `37–48` resource bounds, fault injection and
environment-bound gaps; `49–51` cover deferred EVS behaviors, `52` records the
authority-controlled event-rate/dispatch/latency gap, `53` covers deterministic
queue-lock contention, and `54` is an exploratory configured-capacity stress
case (not a mission-envelope requirement).

| Case | Requirement | Case | Class | Inputs (boundaries) | Expected result (oracle) | Env | Scope | Status |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| N22-TS6-1 | SRS-REQ-049, SSS-REQ-010 | `ff_mem_static` bump allocator bounds and 8-byte alignment | nominal | `FF_MemStaticPoolInit(&p,"p",buf,256)`; `alloc(1)`, `alloc(9)`, then fill to capacity | every returned pointer is 8-byte aligned (`FF_MEM_ALIGN`) and lies inside `[base, base+size)`; `pool.used` equals the aligned end offset; no heap allocation and no write outside the region (SRS-REQ-049 "bump allocator over a caller-provided region"; SSS-REQ-010; `hal_mem.h` `alloc`/`reset`) | SIL | individual | covered |
| N22-TS6-2 | SRS-REQ-049 | static RAM pool exhaustion and `reset()` reuse | OOB | `alloc(size)` then `alloc(1)` with the pool full; `reset()`; `alloc(size)` again | the over-capacity `alloc` returns `NULL` and leaves `used`/`cur` unchanged (no out-of-region write); after `reset()` the whole region is reusable with `used` = 0 and the first pointer equal to `base` (SRS-REQ-049 bounded, heap-free) | SIL | individual | covered |
| N22-TS6-3 | SRS-REQ-049 | exact-fit boundary accounting (remaining and remaining+1) | OOB | request exactly the aligned remaining space, then one byte more | the exact-fit request succeeds with `used` = `size`; the `+1` request returns `NULL` with `used` unchanged; alignment padding never overruns the limit (SRS-REQ-049 bounded) | SIL | individual | covered |
| N22-TS6-4 | SRS-REQ-049, SRS-REQ-015 | pool `reset()` while an allocation is still live (stale pointer) | OOB | allocate, keep the pointer, `reset()`, then write through the stale pointer | **gap** — no clause defines the allocator lifetime/ownership contract (reset versus outstanding pointer, aliasing, reuse guarantee); use-after-reset is undefined. Open TBD (NC-EXT-47 §7.2; class-A memory/runtime analysis GAP-A4). Uncontrolled observation only: `reset()` returns void and the stale pointer aliases the first post-reset allocation. No oracle is asserted here | SIL | individual | gap |
| N22-TS6-5 | SRS-REQ-015, SSS-REQ-010 | no dynamic allocation after init (ASan/LSan over the registered ctest set) | nominal | run every registered ctest target under the address/leak sanitizer; inspect `src/` for `malloc`/`calloc`/`realloc`/`free` | zero LeakSanitizer/ASan reports and no post-init heap allocation in `src/` (the PAL allocates only its static slots; `pthread_create` is RTOS-owned) (SRS-REQ-015 "shall not allocate dynamic memory after initialisation"; SSS-REQ-010) | SIL | individual | covered |
| N22-TS6-6 | SRS-REQ-015 | UBSan leg over the registered ctest set (shifts, signed overflow, alignment, misaligned load) | nominal | build the same suites with `-fsanitize=undefined -fno-sanitize-recover=all` and run ctest | **gap** — the acceptance oracle is stated (FRICK-VV-TSPEC-001 §6.1 "Undefined behavior … no UBSan report") but no committed leg configures UBSan: `.github/workflows/ci.yml` and `.gitea/workflows/ci.yml` set `-fsanitize=address` only, and the S8-FU evidence row's "ASan+UBSan 27/27" is not reproducible from the committed configuration. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only. No oracle is asserted here | SIL | individual | gap |
| N22-TS6-7 | SRS-REQ-011 | EVS circular log wrap at `FF_EVS_LOG_DEPTH` and total-count monotonicity | nominal | send `FF_EVS_LOG_DEPTH` + 1 events; `FF_EvsGetLog(out, FF_EVS_LOG_DEPTH)`; `FF_EvsGetTotalCount` | the log holds the most recent 64 events in time order (the oldest is overwritten on the 65th), `FF_EvsGetLog` returns at most `max_events` events in order and never reads before the start, and `FF_EvsGetTotalCount` = 65 (not reset by wrap) (`evs.h` "circular on-board log"; SRS-REQ-011) | SIL | individual | covered |
| N22-TS6-8 | SRS-REQ-011 | `FF_EvsGetLog` NULL/zero-capacity and use before `FF_EvsInit` | OOB | `FF_EvsGetLog(NULL, n)`; `FF_EvsGetLog(out, 0)`; `FF_EvsGetTotalCount()` before `FF_EvsInit()` | **gap** — `evs.h` declares the functions but no clause fixes the NULL/zero-capacity or pre-init contract (nor the loss semantics of a bounded 64-deep ring that cannot hold every event). Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: `0u` returned. No oracle is asserted here | SIL | individual | gap |
| N22-TS6-9 | SRS-REQ-011 | EVS message text truncation and empty format | OOB | `FF_EvsSend` with a message longer than `FF_EVS_TEXT_LEN`; an empty format string | **gap** — the "circular on-board log" contract does not define text truncation or empty-format semantics (`vsnprintf` truncation is an implementation choice). Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: NUL-terminated truncation. No oracle is asserted here | SIL | individual | gap |
| N22-TS6-10 | SRS-REQ-037, SRS-REQ-015 | static object demand versus the RTEMS `CONFIGURE_*` runtime budget | OOB | account the worst case the PAL/SB/ES can create (32 tasks + 32 mutexes + 32 queues plus per-app pipes) against `CONFIGURE_MAXIMUM_TASKS`/`SEMAPHORES`/`MESSAGE_QUEUES`; run on target | **gap** — the RTEMS configuration budget and the PAL/SB capacity are separate uncontrolled constants and no clause maps the worst-case static object demand to `CONFIGURE_*`; stack depth and WCET are GAP-A4. Open TBD (NC-EXT-47 §7.5). No oracle is asserted here | HIL | individual | gap |
| N22-TS6-11 | SRS-REQ-013 | TBL load/validate/activate lifecycle and staging isolation | nominal | register a table with a validator rejecting values > X; load a legal image; `FF_TblGet` before and after `FF_TblActivate` | `FF_TblLoad` stages and validates without modifying the active image; `FF_TblGet` before activation returns the original image; `FF_TblActivate` copies the staged image atomically and `FF_TblGet` then returns it (SRS-REQ-013 "staged image becomes active only on explicit activate"; `tbl.h` lifecycle) | SIL | individual | covered |
| N22-TS6-12 | SRS-REQ-013 | TBL wrong-length load, rejecting validator and `NULL` validator | OOB | `FF_TblLoad` with `len != size`; a validator returning `FF_ERROR`; a table registered with `validate = NULL` | wrong length = `FF_BAD_SIZE`; a rejecting validator = `FF_ERROR` and the staged image is invalidated so a later `FF_TblActivate` = `FF_NOT_READY`; a `NULL` validator accepts the image (`tbl.h` "`validate` optional, may be NULL") (SRS-REQ-013) | SIL | individual | covered |
| N22-TS6-13 | SRS-REQ-003 | CCSDS PDL widened arithmetic and exact packet boundary (legal frame) | nominal | encode TC and TM with the maximum in-range data length; decode each | decode = `FF_SUCCESS`; `data_len` equals the payload length; the decoder accepted the frame only because `FF_CCSDS_HDR_LEN + declared_data_len == len` exactly (SRS-REQ-003 widened arithmetic and exact boundary) | SIL | individual | covered |
| N22-TS6-14 | SRS-REQ-003 | PDL underflow and minimum secondary-header/CRC size rejection | OOB | raw TC/TM with PDL = 0 and small values so `declared_data_len < sec_len + CRC`; a buffer shorter than `FF_CCSDS_HDR_LEN`; PDL exactly at the minimum | every variant = `FF_PARSE_ERROR`; no read past the supplied buffer (ASan-clean); `apid`/`hdr`/`data`/`data_len` outputs are not written (SRS-REQ-003 "reject a packet if that field cannot contain its configured PUS secondary header and CRC trailer") | SIL | individual | covered |
| N22-TS6-15 | SRS-REQ-003 | PDL 16-bit wrap and trailing/coalesced bytes | OOB | PDL = `0xFFFF` with a short buffer; a valid frame with one trailing byte appended; a frame with the CRC trailer missing | `0xFFFF` does not wrap to a small accepted length; the trailing byte is rejected (`declared + 6 != len`); the missing-CRC frame is rejected; each = `FF_PARSE_ERROR` with no OOB access (SRS-REQ-003 widened arithmetic and exact boundary) | SIL | individual | covered |
| N22-TS6-16 | SRS-REQ-007 | PUS TM message-type counter wrap modulo 16384 | nominal | `FF_PusNextTmCounter` at `0x3FFE`, `0x3FFF`, `0x4000`; `FF_PusPeekTmCounter` | the counter strictly increases by one and the value after `0x3FFF` is `0x0000`; the encoded TM PKT-CTR field stays within 14 bits and does not corrupt the header (`& 0x3FFF`); Lean `req_srs007_counter_wrap_bounded` (SRS-REQ-007) | SIL | individual | covered |
| N22-TS6-17 | SRS-REQ-007 | `FF_PusGetTcCount` 32-bit wrap | OOB | drive the accepted-TC counter to `0xFFFFFFFF` and observe the next increment | **gap** — SRS-REQ-007 bounds only the TM counter (mod 16384); it is silent on the TC-count width and wrap, and `FF_PusGetTcCount` reads a plain `uint32` global without serialization (also NC-EXT-05). Open TBD (NC-EXT-47 §7.2/§7.3). Uncontrolled observation only: unsigned wrap to 0. No oracle is asserted here | SIL | individual | gap |
| N22-TS6-18 | SRS-REQ-024 | CUC fine-fraction bound, lossless round trip and exact 6-octet T-field | nominal | `FF_TimeCucEncode/Decode` at fine = 0, fine = maximum, coarse = maximum; the PUS-C 6-octet T-field at exactly 6 octets | fine fits 16 bits and round-trips exactly (lossless); the 7-octet code rejects wrong length and wrong P-field; the 6-octet T-field is accepted only at exactly 6 octets (SRS-REQ-024; Lean `req_srs024_fine_frac_bounded`) | SIL | individual | covered |
| N22-TS6-19 | SRS-REQ-024, SRS-REQ-010 | CUC coarse-second boundary, pre-epoch UTC offset and `uint64` time wrap | OOB | `FF_TimeCucDecode` with coarse = `0xFFFFFFFF`; `FF_PalSetUtcNs` below the current monotonic base (negative offset); `FF_PalGetUtcNs` near the `uint64` boundary | **gap** — SRS-REQ-024 fixes the field widths but no clause defines coarse rollover across the 32-bit second field, a negative UTC offset, or `FF_PalGetUtcNs` wrapping (`(int64_t)utc - (int64_t)now` in `pal_posix.c`). Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: two's-complement wrap. No oracle is asserted here | SIL | individual | gap |
| N22-TS6-20 | SRS-REQ-033 | math facade out-of-domain behaviour, finiteness and clamp semantics | nominal | `FF_MathSqrt(-1)`, `FF_MathLog(0)`, `FF_MathIsFinite(NaN)`, `FF_MathIsFinite(+inf)`, `FF_MathIsFinite(1.0)`, `FF_MathClamp(NaN,0,1)`, `FF_MathClamp(3.0,0,10)`, `FF_MathClamp(-1.0,0,10)` | `sqrt(-1)` = NaN, `log(0)` = -inf, `FF_MathIsFinite` = 0/0/1, `FF_MathClamp(NaN,…)` returns NaN unclamped ("a NaN x is not clamped"), and in-range clamps return `lo`/`x`/`hi`; no `errno` is required (SRS-REQ-033; FRICK-ARCH-0009 §3; `ff_math.h`) | SIL | individual | covered |
| N22-TS6-21 | SRS-REQ-033 | `FF_MathClamp` with an inverted range (`lo > hi`) | OOB | `FF_MathClamp(5, 10, 0)` and `FF_MathClamp(3, 10, 0)` | **gap** — `ff_math.h` states `lo <= hi` as a *precondition* and the numerics note is silent, so an inverted range has no defined result (the precondition is violated). Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: the first failing comparison wins (returns `lo` for `x < lo`, else `hi`). No oracle is asserted here | SIL | individual | gap |
| N22-TS6-22 | SRS-REQ-052, SRS-REQ-009 | PUS service provider-reported output lengths trusted without a bound recheck | OOB | register an HK/parameter/memory-dump provider that reports `*len` larger than the destination (over-max, zero, unchanged-output) | **gap** — NC-EXT-24: provider-reported lengths are trusted; no clause defines the rejection of an over-max or unchanged provider output. Open NC-EXT-24 (NC-EXT-47 §7.3). Uncontrolled observation only: the reported length is copied/encoded. No oracle is asserted here | SIL | individual | gap |
| N22-TS6-23 | SRS-REQ-004, SRS-REQ-003 | `FF_CcsdsCrc16` zero-length, NULL buffer and over-`uint16` length | OOB | `FF_CcsdsCrc16(NULL, 0)`; `FF_CcsdsCrc16(data, 0)`; a CRC length that cannot be represented in the `uint16` parameter | **gap** — SRS-REQ-004 fixes the polynomial/trailer and reject-on-mismatch but no clause defines the zero-length or NULL-buffer contract, or a truncated/unrepresentable length. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: the all-ones init value is returned for length 0. No oracle is asserted here | SIL | individual | gap |
| N22-TS6-24 | SRS-REQ-004 | CRC trailer coverage and reject on single-bit flips | nominal | decode a valid frame, then decode copies with one flipped bit in the primary header, the secondary header, the payload and the CRC itself | the unmodified frame = `FF_SUCCESS`; every single-bit flip = `FF_CRC_ERROR` (the CRC covers the whole frame up to the trailer) (SRS-REQ-004; Lean `req_srs004_crc_appended_zero`) | SIL | individual | covered |
| N22-TS6-25 | SRS-REQ-015, SSS-REQ-010 | PAL mutex delete followed by lock/unlock through a stale handle | OOB | `FF_PalMutexCreate`; `FF_PalMutexDelete(id)`; `FF_PalMutexLock(id,…)`/`FF_PalMutexUnlock(id)`; separately a second delete of the same id | **gap** — no clause defines the use-after-delete contract for a PAL handle, nor whether a delete of an already-deleted/never-created handle is idempotent. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: `FF_NOT_FOUND` for the deleted id (the slot is also reusable by a later create). No oracle is asserted here | SIL | individual | gap |
| N22-TS6-26 | SRS-REQ-001, SRS-REQ-002, SSS-REQ-002 | SB FIFO ordering and copy semantics across the SB→PAL queue handoff | nominal | two tasks publish/receive on one pipe through the PAL mutex; publish N messages in order and drain them | the received message order equals the publish order (FIFO), every message is copied (publisher buffer mutation after publish does not change the queued copy) and every matching/wildcard subscriber receives exactly one copy (SRS-REQ-001/002; SSS-REQ-002; FRICK-ARCH-0002 §2/§4) | SIL | combined | covered |
| N22-TS6-27 | SRS-REQ-001 | SB no-match and wildcard routing boundaries | OOB | publish a message id with no subscription; a pipe subscribed only to `FF_SB_WILDCARD`; a subscription for an id that is never published | no-match publish = `FF_SUCCESS` with zero deliveries (no queue touched); the wildcard pipe receives every message; stats account `published`/`delivered` consistently (SRS-REQ-001; Lean `req_srs001_no_match_no_delivery`) | SIL | individual | covered |
| N22-TS6-28 | SRS-REQ-007, SRS-REQ-011, SRS-REQ-024 | concurrent TM generation touching the PUS counter, time and EVS | OOB | two or more contexts call `FF_PusSendTm`/`FF_PusNextTmCounter` and time/EVS reads concurrently while TM packets are encoded | **gap** — NC-EXT-05: the PUS TM counter, TC count and TM sink are unlocked shared globals and no serialization contract exists, so duplicate or lost counters cannot be ruled out; no controlled oracle for the concurrent case. Open NC-EXT-05 (NC-EXT-47 §7.3). Uncontrolled observation only: torn/uncoordinated counter reads are possible. No oracle is asserted here | SIL | combined | gap |
| N22-TS6-29 | SRS-REQ-015, SSS-REQ-010 | PAL mutex misuse (unlock by a non-owner, double unlock, relock) | OOB | lock in one task and unlock in another; unlock twice; lock a mutex already held non-recursively in the same task | **gap** — `pal.h` does not state the ownership/non-recursive contract or the expected status for misuse, and `FF_PalMutexUnlock` maps the pthread result to `FF_ERROR` without a controlled definition. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: `FF_ERROR` or an RTOS-defined result. No oracle is asserted here | SIL | individual | gap |
| N22-TS6-30 | SRS-REQ-015, SSS-REQ-010 | PAL queue delete with a blocked waiter, then put after delete | OOB | a task blocks in `FF_PalQueueGet` on a non-empty wait; another deletes the queue; a later `FF_PalQueuePut` uses the stale id | **gap** — no clause defines the delete-with-waiter behaviour (waiter release, use-after-delete, or reuse of the destroyed slot's `pthread_cond_t`/`pthread_mutex_t`). Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: the waiter remains blocked and the id is reported `FF_NOT_FOUND`. No oracle is asserted here | SIL | individual | gap |
| N22-TS6-31 | SRS-REQ-014, SSS-REQ-008 | Health & Safety heartbeat cadence and missed-kick escalation on target | OOB | stop the HS heartbeat (hang/delete the task) and observe the external watchdog reaction within the configured window | **gap** — HS emits a 5000 ms heartbeat and the watchdog is external (`FF_PalPanic`/RTEMS monitor); no clause defines the missed-kick escalation latency, the external watchdog window, or the recovery sequence on the target. Open TBD (NC-EXT-47 §7.5). No oracle is asserted here | HIL | individual | gap |
| N22-TS6-32 | SRS-REQ-051, SRS-REQ-001 | multicore interference and cache coherence on the shared Software Bus | OOB | run the SB publish/receive load on more than one U54 core under RTEMS SMP with shared-memory SB state | **gap** — SRS-REQ-051 (multicore/SMP) is a planned milestone (M14) with no controlled interference/coherence oracle; the shared SB lock and the cache-coherency obligations are the RISC-V/RTEMS BSP's responsibility (GAP-A5). Open TBD (NC-EXT-47 §7.5). No oracle is asserted here | HIL | combined | gap |
| N22-TS6-33 | SRS-REQ-071, SSS-REQ-030 | reference scheduler starvation with mixed periods | OOB | run the table-driven dispatcher with 1000/5000/10000 ms entries and a single shared last-run timestamp | **gap** — NC-EXT-19: the reference dispatcher advances one shared `g_last_run`, so entries with longer periods can starve shorter entries; no controlled dispatch-order/lateness oracle exists and the numeric rates/tolerances are mission-controlled. Open NC-EXT-19 (NC-EXT-47 §7.3). No oracle is asserted here | SIL | individual | gap |
| N22-TS6-34 | SRS-REQ-031 | ISR-context safety of core services (calling SB/EVS/TBL/PAL from an ISR) | OOB | invoke `FF_EvsSend`, `FF_SbPublish`, `FF_TblGet` and a PAL mutex lock from an interrupt context | **gap** — SRS-REQ-031 requires only that a GPIO ISR defers work to a task; no clause states which core-service calls are ISR-safe or their locking behaviour, and `FF_EvsSend`/`FF_SbPublish` take a mutex that cannot be held from an ISR. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: a deadlock/`FF_ERROR`. No oracle is asserted here | SIL | individual | gap |
| N22-TS6-35 | SRS-REQ-003, SRS-REQ-004 | bounded, reproducible decoder fuzz/property corpus beyond selected vectors | OOB | feed a seeded, resource-bounded corpus of mutated TC/TM byte streams to `FF_CcsdsDecodeTc/Tm` and `FF_PusIngest` | **gap** — NC-EXT-25: no deterministic bounded fuzz/property corpus or independent oracle exists; only selected boundary vectors are tested. Open NC-EXT-25 (NC-EXT-47 §7.3). Uncontrolled observation only: no crash (ASan); acceptance is not a correctness oracle. No oracle is asserted here | SIL | individual | gap |
| N22-TS6-36 | SRS-REQ-014 | ES invalid state, registry saturation and start-before-register | OOB | `FF_EsStartApp` for a never-registered id; `FF_EsGetState` for an invalid id; register `FF_ES_MAX_APPS` apps then one more | **gap** — `es.h` declares the state enum and functions but no clause fixes the invalid-id/saturation return contract or the state transition on a failed start. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: `FF_NOT_FOUND`/`FF_ERROR` with the count pinned at `FF_ES_MAX_APPS`. No oracle is asserted here | SIL | individual | gap |
| N22-TS6-37 | SRS-REQ-014, SRS-REQ-015 | init-order fault injection: service create failure mid-boot | OOB | inject a PAL failure (queue create fails, or the SB mutex create fails) while `FF_SbInit`/`FF_EsRegisterApp` run, then continue the remaining init sequence | **gap** — no clause defines the rollback/consistency contract when a service fails to initialise part-way (e.g. `FF_EsRegisterApp` creates a pipe after claiming the app slot, then returns `FF_ERROR` without releasing it). Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: an app slot may stay `used`. No oracle is asserted here | SIL | combined | gap |
| N22-TS6-38 | SRS-REQ-015, SSS-REQ-010 | PAL queue timeout/deadline semantics | OOB | `FF_PalQueuePut`/`FF_PalQueueGet` with `timeout_ms` = 0, 1, `FF_PAL_WAIT_FOREVER`, and an already-expired deadline on a full/empty queue | **gap** — no clause defines the exact timeout/deadline contract (return on expiry, `dropped` accounting, spurious wakeup tolerance, or the `ETIMEDOUT`-only mapping). Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: `FF_BUFFER_FULL`/`FF_TIMEOUT` with `dropped` incremented. No oracle is asserted here | SIL | individual | gap |
| N22-TS6-39 | SRS-REQ-015 | stale PAL handle reuse after delete and recreate | OOB | create a task/mutex/queue, delete it, create a new one (which reuses the index), then use the old handle value | **gap** — handles are bare small-integer indices with no generation/validity tag; no clause defines that a recycled index is distinguishable, so a stale handle silently addresses the new object. Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: the old handle value addresses the new slot. No oracle is asserted here | SIL | individual | gap |
| N22-TS6-40 | SRS-REQ-033 | math facade reentrancy and thread-safety | nominal | call the facade concurrently from several tasks on overlapping inputs and verify results against the single-thread reference | results are identical to the single-thread reference for the same inputs (no shared mutable state, no allocation, no data race under ASan/TSan-style inspection) (FRICK-ARCH-0009 §4 "reentrant and thread-safe") | SIL | individual | covered |
| N22-TS6-41 | SRS-REQ-014 | ES application-failure path (state to `FF_APP_FAILED`, HS detection) | OOB | make an application task return/abort and observe the ES state and HS detection/restart | **gap** — SRS-REQ-014 requires HS to detect failed applications, but no clause defines the detection latency, the state transition timing or the restart/rollback behaviour, and there is no dedicated HS unit test (the traceability note defers this). Open TBD (NC-EXT-47 §7.2). Uncontrolled observation only: state remains `FF_APP_RUNNING`. No oracle is asserted here | SIL | individual | gap |
| N22-TS6-42 | SRS-REQ-014, SSS-REQ-008 | HS/ES recovery latency and fault-detection campaign | OOB | inject an application hang/abort and a PAL task-create failure; measure detection and recovery | **gap** — the recovery sequence, detection latency and restart budget are mission-controlled inputs with no controlled oracle, and the fault-injection campaign is GAP-A4. Open TBD (NC-EXT-47 §7.3). No oracle is asserted here | SIL | individual | gap |
| N22-TS6-43 | SRS-REQ-001, SSS-REQ-002 | SB subscription slot saturation (`FF_SB_MAX_SUBSCRIPTIONS` = 96, plus one) | OOB | create a pipe and add `FF_SB_MAX_SUBSCRIPTIONS` subscriptions, then a 97th | the store is bounded and not overrun (ASan-clean) and the 96 existing subscriptions are unchanged; the 97th is not created (the implementation returns `FF_NO_RESOURCE`; the exact code is not fixed by a clause) (SRS-REQ-001; SSS-REQ-002 bounded queue; FRICK-ARCH-0002 §4 static pools) | SIL | individual | covered |
| N22-TS6-44 | SRS-REQ-001, SRS-REQ-011 | SB publish to a full pipe: drop without synchronous listener work plus deferred EVS error delivery | OOB | fill a subscriber pipe and publish one more message with a listener configured | publish returns `FF_SUCCESS` before listener invocation; new copy is dropped, SB `dropped` increments once, and EVS log records `FF_EVS_SB_ERR`/`FF_EVS_ERROR` for that subscriber/message. `FF_EvsDispatchPending(1)` later invokes the configured listener with that event; ordinary `FF_EvsSend` remains synchronous (FRICK-ARCH-0002 §2; SRS-REQ-011) | SIL | individual | covered |
| N22-TS6-49 | SRS-REQ-001, SRS-REQ-011 | EVS listener republishes to a full SB pipe without recursive callback growth | OOB | fill a subscribed pipe, cause an overflow event, and have its listener publish again to the still-full pipe and re-enter the dispatcher | first dispatch invokes exactly one callback; nested `FF_EvsDispatchPending` returns zero, callback nesting depth remains 1, and the callback-generated SB overflow is delivered on a later top-level dispatch; no notification is lost and EVS `queued`/`dispatched`/`dropped` counters match 2/2/0 | SIL | combined | covered |
| N22-TS6-50 | SRS-REQ-011 | EVS deferred-notification queue saturation has explicit loss accounting | OOB | enqueue `FF_EVS_LOG_DEPTH` notifications with `FF_EvsSendDeferred` and no dispatch, then call it once more | first `FF_EVS_LOG_DEPTH` deferred sends return `FF_SUCCESS`; overflow send returns `FF_NO_RESOURCE`, remains in EVS log, increments `FF_EvsNotifyStats_t.dropped` once and invokes no listener inline; dispatcher later calls listener exactly `FF_EVS_LOG_DEPTH` times, with queued/dispatched/dropped counts equal to depth/depth/1. Ordinary `FF_EvsSend` is separately synchronous and is not an input to this queue-saturation oracle | SIL | individual | covered |
| N22-TS6-51 | SRS-REQ-001, SRS-REQ-011, SRS-REQ-072 | SB full-pipe event when the EVS deferred queue is saturated | OOB | fill a one-entry SB pipe; enqueue `FF_EVS_LOG_DEPTH` deferred notifications; publish again to cause SB overflow | `FF_SbPublish` returns `FF_SUCCESS` without invoking a listener; SB overflow event is retained as the newest EVS log event with the subscriber id; EVS dropped-notification count increments once, and only the previously accepted `FF_EVS_LOG_DEPTH` callbacks are later dispatched. This is a local loss/accounting oracle only; it is **not** ground-forwarding evidence or proof of SRS-REQ-072 loss-counter/report compliance (FRICK-ARCH-0002 §2; `FF_EvsSendDeferred`) | SIL | combined | covered |
| N22-TS6-52 | SRS-REQ-072, SSS-REQ-006 | Event-rate envelope, burst, dispatch and latency boundary | OOB | generate valid EVS API submissions at, below and above the authority-approved envelope; include maximum burst at worst phase relative to dispatcher, maximum callback load, and approved link outage/recovery | **gap/TBD** — compare arrivals, event acceptance, PUS frame handoff and ground receipt against approved `Bmax`, `Rmax`, `Dmin`, service interval, `Lmax` and outage assumptions; every pre-acceptance, queue or post-acceptance loss must increment the independent counter once and be reported. No test can pass until authorized values/profile and a loss oracle are baselined (NC-EXT-76); HIL/target ground-receipt fidelity remains TBD | SIL | combined | gap |
| N22-TS6-53 | SRS-REQ-001 | PAL distinguishes a busy queue mutex from a full queue for SB fan-out | OOB | directly exercise `FF_PalQueueTryPut` with a held mutex and a full depth-one queue; then integrate two matched SB pipes with the first queue mutex held | PAL returns `FF_TIMEOUT` for a busy mutex and `FF_BUFFER_FULL` for full storage; SB busy case returns `FF_TIMEOUT`, increments dropped once, emits deferred busy-specific event and permits another subscriber's copy; existing full-queue SB case preserves `FF_SUCCESS`, increments dropped once and emits full-specific event. The held-lock cases return before unlock; contended/full pipe gets no new copy | SIL | combined | covered |
| N22-TS6-54 | SRS-REQ-001, SRS-REQ-011, SRS-REQ-072 | Configured structural stress boundary: maximum SB pipe fan-out, simultaneous queue-lock contention, full queues, and deferred EVS saturation | OOB (exploratory stress hypothesis; not an approved mission profile) | isolated stress executable creates `FF_SB_MAX_PIPES` (32) wildcard-matched pipes, holds all 32 queue locks during one publish, fills each queue to depth `FF_SB_QUEUE_DEPTH_MAX` (32), then issues three full-queue publishes with distinct message IDs and dispatch withheld | Exact local oracle: all-lock contention returns `FF_TIMEOUT`, accounts 32 drops and defers/dispatches 32 busy events after unlock; then 32 publishes deliver 1024 copies; three full-queue publishes account 96 more SB drops/events. EVS accepts 96 notifications cumulatively across both phases, dispatches 96, counts 32 notification losses at saturation, and retains the newest 64 of 128 log records. The retained log oracle checks ordered app IDs and distinguishes the final two overflow bursts by message ID (`0x0B01`, then `0x0B02`). This demonstrates configured structural/queue boundaries only; it does not establish an arrival-rate/WCET/ground-delivery guarantee or satisfy SRS-REQ-072 (NC-EXT-76) | SIL | combined | exploratory (host structural stress only) |
| N22-TS6-45 | SRS-REQ-015, SSS-REQ-010 | bounded idempotent init and shutdown/re-init | nominal | `FF_PalInit()` twice; `FF_PalShutdown()` then `FF_PalInit()`; re-init SB/EVS/TBL and re-create objects | re-init is idempotent and bounded (no heap growth, static slot tables reset); after shutdown/re-init the services are usable again and no stale state is observed (SRS-REQ-015; FRICK-VV-TSPEC-001 §6.3 "bounded init, idempotent re-init") | SIL | individual | covered |
| N22-TS6-46 | SRS-REQ-037, SRS-REQ-015 | on-target fault injection and stack/WCET resource measurement | OOB | on the RTPFS160ZT: inject clock/read-reset/bus faults, measure worst-case stack depth per task and WCET of the core services | **gap** — stack-depth and WCET analysis and the failure-mode/fault-injection campaign are GAP-A4; on-target fault injection needs hardware (GAP-A5). Open TBD (NC-EXT-47 §7.5). No oracle is asserted here | HIL | combined | gap |
| N22-TS6-47 | SRS-REQ-015 | MSan uninitialized-read leg over the registered ctest set | nominal | build the same suites with MemorySanitizer and run ctest | **gap** — the umbrella oracle (FRICK-VV-TSPEC-001 §6.1 "Uninitialized read … MSan (where available) … no report") cannot be evidenced: MSan is not established in the toolchain/CI and `-fsanitize=memory` is not configured anywhere. Open TBD (NC-EXT-47 §7.2). No oracle is asserted here | SIL | individual | gap |
| N22-TS6-48 | SRS-REQ-044 | software-only robustness observable on the emulated PolarFire SoC (Renode) | nominal | Renode run of `app_renode_test` exercising the software-only robustness checks | **gap** — the `RENODE_TEST: PASS` aggregate marker is not a per-requirement oracle and no per-check software-only robustness observable is emitted; the PIL leg does not evidence SRS-REQ-015/003/007 robustness properties. Open TBD (NC-EXT-47 §7.6). No oracle is asserted here | PIL | individual | gap |

## 3. Gaps (mapping to NC-EXT-47 sub-items)

All gaps are recorded, not resolved (N21). Each row below maps a `gap` case to
the NC-EXT-47 sub-item in FRICK-VV-TSPEC-001 §7 that it belongs to, and names
the specific open NC/TBD where one exists. Gap rows assert **no** oracle.

**NC-EXT-47 §7.2 — software-only robustness oracles not specified as a
controlled, requirement-mapped set.**

- `N22-TS6-4` — pool `reset()` with a live allocation (stale pointer)
  (TBD; NC-EXT-47 §7.2; GAP-A4).
- `N22-TS6-6` — UBSan leg not configured in the committed CI/preset
  (TBD; NC-EXT-47 §7.2).
- `N22-TS6-8` — `FF_EvsGetLog` NULL/zero-capacity and pre-init (TBD;
  NC-EXT-47 §7.2).
- `N22-TS6-9` — EVS text truncation and empty format (TBD;
  NC-EXT-47 §7.2).
- `N22-TS6-17` — `FF_PusGetTcCount` 32-bit wrap (TBD; NC-EXT-47 §7.2/§7.3;
  related NC-EXT-05).
- `N22-TS6-19` — CUC coarse boundary / negative UTC offset / `uint64` time
  wrap (TBD; NC-EXT-47 §7.2).
- `N22-TS6-21` — `FF_MathClamp` inverted range (`lo > hi`) (TBD;
  NC-EXT-47 §7.2).
- `N22-TS6-23` — `FF_CcsdsCrc16` zero-length/NULL/over-`uint16` length (TBD;
  NC-EXT-47 §7.2).
- `N22-TS6-25` — PAL mutex use-after-delete (TBD; NC-EXT-47 §7.2).
- `N22-TS6-29` — PAL mutex misuse (non-owner unlock, double unlock, relock)
  (TBD; NC-EXT-47 §7.2).
- `N22-TS6-30` — PAL queue delete with a blocked waiter, then put (TBD;
  NC-EXT-47 §7.2).
- `N22-TS6-34` — ISR-context safety of core services (TBD; NC-EXT-47 §7.2).
- `N22-TS6-36` — ES invalid id, registry saturation and start-before-register
  (TBD; NC-EXT-47 §7.2).
- `N22-TS6-37` — init-order fault injection / partial-init consistency (TBD;
  NC-EXT-47 §7.2).
- `N22-TS6-38` — PAL queue timeout/deadline semantics (TBD; NC-EXT-47 §7.2).
- `N22-TS6-39` — stale PAL handle reuse after delete/recreate (TBD;
  NC-EXT-47 §7.2).
- `N22-TS6-41` — ES application-failure path / HS detection (TBD;
  NC-EXT-47 §7.2).
- `N22-TS6-47` — MSan uninitialized-read leg unavailable (TBD;
  NC-EXT-47 §7.2).

**NC-EXT-47 §7.3 — out-of-bound oracles not defined by a controlled clause
(unresolved/absent contract).**

- `N22-TS6-22` — PUS provider-reported output lengths (NC-EXT-24).
- `N22-TS6-28` — concurrent TM generation on unlocked PUS globals (NC-EXT-05).
- `N22-TS6-33` — reference scheduler starvation with mixed periods
  (NC-EXT-19).
- `N22-TS6-35` — bounded, reproducible decoder fuzz/property corpus
  (NC-EXT-25).
- `N22-TS6-42` — HS/ES recovery latency and fault-detection campaign (TBD;
  NC-EXT-47 §7.3; GAP-A4).

**NC-EXT-47 §7.5 — target (HIL) evidence remains GAP-A4 / GAP-A5.**

- `N22-TS6-10` — static object demand versus the RTEMS `CONFIGURE_*` budget and
  stack/WCET (GAP-A4; TBD; NC-EXT-47 §7.5).
- `N22-TS6-31` — HS heartbeat cadence / external-watchdog escalation on target
  (GAP-A5; TBD; NC-EXT-47 §7.5).
- `N22-TS6-32` — multicore interference and cache coherence on the shared SB
  (GAP-A5; SRS-REQ-051 M14; NC-EXT-47 §7.5).
- `N22-TS6-46` — on-target fault injection and stack/WCET measurement (GAP-A4 /
  GAP-A5; NC-EXT-47 §7.5).

**NC-EXT-47 §7.6 — emulated-target aggregate marker is not a per-requirement
oracle.**

- `N22-TS6-48` — `RENODE_TEST: PASS` does not evidence the software-only
  robustness properties; no per-check PIL observable is emitted (TBD;
  NC-EXT-47 §7.6).

### 3.1 Current automated evidence (as-is) versus specified-not-yet-run

The cases above are **specified**; this section records the current automation
and this increment's execution, so that `covered` is not read as `passing`:

- **Registered default suites**: 27 ctest targets from `tests/CMakeLists.txt`
  (`ccsds, sb, tbl, pus, time, r2_gap, r3_gap, hal, math, sdls` plus the
  `FF_BUILD_DRIVERS` set and `link_ip`, `link_tsn`); 29 with
  `-DFF_USE_LWIP=ON` (`lwip`, `lwip_netif`). In this increment, `test_sb` passed
  with 48 checks including the full-pipe EVS log oracle (`make test`, 27/27).
- **Sanitizer leg**: both `.github/workflows/ci.yml` and
  `.gitea/workflows/ci.yml` build `build/asan` with
  `-fsanitize=address -fno-omit-frame-pointer` and run `ctest`. This is the
  as-committed evidence for `N22-TS6-5`; LeakSanitizer is on by default on
  Linux with ASan.
- **Not automated**: there is no committed UBSan leg (`N22-TS6-6` is a gap) and
  no MSan leg (`N22-TS6-47` is a gap). The S8-FU entry in
  `docs/vv/verification-results.md` states "ASan+UBSan 27/27"; that UBSan
  configuration is not present in the committed workflows/presets, so
  `N22-TS6-6` is recorded as a gap rather than credited.
- **Unsanitized build paths**: the two lwIP targets are behind
  `FF_USE_LWIP=ON` and are not built by the default (sanitizer) configuration,
  so `test_lwip`/`test_lwip_netif` are outside the current ASan evidence.
- Coverage under this baseline is qualified accordingly: `covered` rows
  (`N22-TS6-1/2/3/5/7/11/12/13/14/15/16/18/20/24/26/27/40/43/44/45`) have a
  grounded oracle and an executable environment, but the runner must confirm
  which are already automated before any statement of "verified".

### 3.2 Coverage summary

| Metric | Count |
| --- | --- |
| Total cases | 54 |
| Nominal | 15 |
| Out-of-bound | 39 |
| Individual | 44 |
| Combined | 10 |
| `covered` (oracle grounded in a requirement/architecture/interface clause) | 24 |
| `gap` (no controlled oracle; recorded under NC-EXT-47) | 29 |
| Exploratory structural stress (not mission-envelope evidence) | 1 |
| SIL / PIL / HIL (primary environment assignments; HIL/target TS6-52 remains gap) | 49 / 1 / 4 |
| Unique cases assigned a primary environment | 54 |

Requirement coverage in this scope: `SRS-REQ-001/002/003/004/007/009/010/011/
013/014/015/024/031/033/037/044/049/051/052` and `SSS-REQ-002/008/010/030` all
have at least one case. Every non-environment group has at least one nominal and
one out-of-bound case. The memory-safety group is anchored on the ASan/LSan leg
(`N22-TS6-5`) and the bounded static pool (`N22-TS6-1/2/3`); the numeric group is
anchored on the CCSDS PDL widened arithmetic (`N22-TS6-13/14/15`), the TM
counter (`N22-TS6-16`) and the CUC fine bound (`N22-TS6-18`); UBSan, MSan, the
PAL misuse/handle contracts, the PUS concurrency and count wrap, and the ES/HS
recovery paths remain recorded gaps, not resolved by a fabricated oracle.

### 3.3 Observation (not an NC opened by this package)

FRICK-ARCH-0002 §2 defines the SB overflow policy as "a full pipe drops the new
message **and raises an EVS error event**". `N22-TS6-44`, `-49` and `-50` cover
the log/enqueue, deferred listener dispatch, bounded re-entry and notification
queue saturation behaviors. `tests/unit/test_sb.c` verifies that full-pipe
publish returns before callback dispatch; an explicit dispatcher call later
delivers the event; callback-triggered republish does not nest; and saturation
returns `FF_NO_RESOURCE` while retaining the event in the EVS log and counting
the undelivered listener notification. This corrects the initial synchronous
listener implementation identified by FRICK-REVIEW-0263. NC-EXT-75 remains open
pending independent review and all applicable gates; broader NC-EXT-47 remains
open.

The isolated `test_sb_stress` profile additionally holds all 32 subscriber
queue mutexes, verifies bounded busy-lock handling, fills 32 queues to depth 32,
and creates three full-queue bursts while deferred dispatch is withheld. Its
log oracle checks the ordered application-id suffix as well as the retained
record count, proving that the circular log retains the newest 64 entries for
this injected sequence. This is a host structural stress observation only; it
does not establish a time-based arrival envelope, WCET, or ground forwarding.

## History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-14 | frick-fsw | Initial TS-6 work package per N22 and the FRICK-VV-TSPEC-002 chunk plan: nominal + out-of-bound case table with explicit oracles for software-only robustness (memory safety and bounded static storage, numeric/overflow protection, PAL concurrency and timing edges, software-level fault injection and resource bounds); gaps mapped to NC-EXT-47 sub-items with the open NC/TBD named (NC-EXT-05/19/24/25, GAP-A4/A5) |
| 0.2 | 2026-10-14 | frick-fsw | Correct N22-TS6-44 implementation observation and specify the EVS log regression oracle for SB overflow; broader NC-EXT-47 remains open |
| 0.3 | 2026-10-14 | frick-fsw | Add deferred EVS delivery, listener-reentry bound and notification-queue saturation cases after R1 finding FRICK-REVIEW-0263; NC-EXT-75 remains open |
| 0.4 | 2026-10-14 | frick-fsw | Restrict deferred EVS behavior to `FF_EvsSendDeferred`/SB overflow and state ordinary-send synchronization and queue saturation oracle |
| 0.5 | 2026-10-14 | frick-fsw | Add integrated SB-overflow behavior oracle when the EVS deferred queue is saturated; make retained log event and loss statistics explicit |
| 0.6 | 2026-10-14 | frick-fsw | Add explicit queue-mutex-busy versus queue-full status and partial-fan-out robustness oracle plus SRS-REQ-072 envelope case; retain profile and loss-reporting TBDs |
| 0.7 | 2026-10-14 | frick-fsw | Add isolated configured-maximum SB/EVS saturation stress case; explicitly bound its claim to structural limits, not mission-rate, timing, or ground-delivery assurance |
| 0.8 | 2026-10-14 | frick-fsw | Reconcile coverage counts and revision history; record scoped PA/R4 review findings and initial corrections |
| 0.9 | 2026-10-14 | frick-fsw | Distinguish overflow bursts in the retained-log oracle; reconcile all environment assignments and update the case-group index per independent delta findings |
| 1.0 | 2026-10-14 | frick-fsw | Set the primary environment token to SIL and explicitly retain HIL/target ground-receipt fidelity as a gap; align the V-model package totals |
| 1.1 | 2026-10-14 | frick-fsw | Define exploratory status as a separate non-coverage category and reconcile sanitizer evidence wording with committed configuration |
