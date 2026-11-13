# Systems Review Report — Independent R1/R3 Avionics Review

**Repository:** frick-fsw (branch `main`)
**Reviewer role:** Senior Aeronautics and Space Avionics Systems Engineer (independent review, R1/R3 profile)
**Review date:** 2026-11-11
**Target:** ECSS class B; Microchip RT PolarFire SoC RTPFS160ZT (RISC-V) under RTEMS 6.2
**Scope under review:**
1. ICD §4 service 5 wire format vs `FF_PusEmitEvent` implementation (NC-EXT-237 fix)
2. Caller consistency for the emitter-owned header contract (all six callers)
3. System-level impact of NC-EXT-237 on event reporting to ground (PUS service 5 TM)
4. Uncommitted link-backend `send_until` deadline-bounded send (SRS-REQ-074 pre-reset report path)
5. Cross-disciplinary consistency: GNC, safety, FDIR, operations interfaces

**Authority statement:** This review provides engineering evidence only. It does not certify, qualify, accept, waive, or approve any airworthiness, human-rating, safety, or mission-acceptance item. All safety-critical and regulatory decisions remain with authorized humans.

---

## Executive summary

The NC-EXT-237 fix (`FF_PusEmitEvent` now owns the Service 5 report header: `event id[2 BE]` + `type[1]`, callers pass auxiliary data only) is **interface-consistent with ICD §4 rev 1.7**, and **all six callers correctly pass text-only auxiliary data**. The fix is safety-positive: event identity is now emitter-controlled, enabling deterministic ground and on-board FDIR correlation. Host gates pass: build 0 warnings, CTest 55/55 (incl. `link_ip`, `link_tsn`, `es-hs-lifecycle`, `nc-ext-237`, `pus_event_identity`).

No finding is a functional blocker for host commit/push. One finding (NC-EXT-237 acceptance) remains OPEN pending Software PA disposition; two target/operations items require resolution before target deployment or operational use.

---

## Findings register

| ID | Classification | Blocks commit/push? | Finding |
|----|---------------|---------------------|---------|
| SYS-R1/R3-001 | Minor | No | ICD §4 does not document S5 subservice mapping (1/2/3) explicitly |
| SYS-R1/R3-002 | Major | No (NC open) | NC-EXT-237 fix: emitter-owned event identity closes latent defect; PA acceptance pending |
| SYS-R1/R3-003 | Minor | No | `test_r2_gap.c` uses caller payload resembling id+type header (old pattern) |
| SYS-R1/R3-004 | Minor | No (target item) | `link_ip.c`/`link_tsn.c` built for RTEMS target now depend on `FF_PalGetMonotonicNs()` |
| SYS-R1/R3-005 | Minor | No | `ip_send` argument contract tightened (NULL+non-zero now rejected) |
| SYS-R1/R3-006 | Minor | No (operations item) | Event-id catalogue not interface-authority controlled; PUS profile provisional |
| SYS-R1/R3-007 | Minor (positive) | No | All six callers correctly pass text-only auxiliary data |
| SYS-R1/R3-008 | Minor (positive) | No | Pre-reset report wiring verified end-to-end (SRS-REQ-074 support) |

---

## Detail: Findings

### SYS-R1/R3-001 — Minor — ICD §4 subservice mapping not explicit in table

**Evidence.**
- `docs/interfaces/ICD.md:115`: Service 5 row records `event id[@2, big-endian], type[1], text` and states the header is built by `FF_PusEmitEvent`. It does **not** state the subservice values (1=INFO, 2=WARNING, 3=CRITICAL/unknown).
- `src/pus/pus.c:378-386`: implementation maps `FF_EVS_INFO → 1`, `FF_EVS_WARNING → 2`, everything else (incl. `FF_EVS_ERROR`, `FF_EVS_CRITICAL`, unknown) → `3`.
- `src/pus/pus.h:82-83`: documents the mapping in the API comment.

**Assessment.** The wire format (id[2 BE] + type[1]) is documented; the subservice mapping lives only in code/header comments. This is a documentation traceability gap: a ground-station or GSW developer reading ICD §4 cannot derive the subservice from the ICD alone. Ground event decoding depends on this mapping.

**Recommendation.** Add to ICD.md §4 Service 5 row: subservice `1 = INFO, 2 = WARNING, 3 = CRITICAL (unknown severities fall back to 3)`, and reference `src/pus/pus.h` as the authoritative contract. No code change required.

**Blocker?** No. Documentation-only; does not affect committed code or tests.

---

### SYS-R1/R3-002 — Major — NC-EXT-237: emitter-owned event identity closes latent defect (PA acceptance pending)

**Evidence.**
- Pre-fix (commit `4ca8931^`): `src/pus/pus.c` executed `(void)event_id;` and forwarded `data` verbatim; event identity was carried only by bytes the caller embedded in `data`.
- Post-fix (`src/pus/pus.c:356-388`): `FF_PusEmitEvent` encodes `event id[2 BE]` at `payload[0:1]`, `type[1]` at `payload[2]`, appends `data`/`len` from `payload[3]`, bounds `len <= FF_PUS_MAX_TM_FRAME - 3` (`FF_BAD_SIZE`), rejects `data == NULL && len != 0` (`FF_INVALID_ARGUMENT`).
- `src/pus/pus.h:78-86`: API contract updated to match.
- `docs/interfaces/ICD.md:115` and `126-138`: ICD §4 rev 1.7 and event-id catalogue note updated; NC-EXT-102 (catalogue control) remains open.
- `tests/unit/test_pus_event_identity.c`: new discriminating oracle, 5 cases / 106 checks; restoring the pre-fix body fails 33 of 106 checks (`docs/vv/evidence/2026-11-11-nc-ext-237/mutation-check.log`).
- Gates: build 0 warnings, CTest 55/55 (host, ASN1 off), Lean, trace, format, license.

**System-level impact.** Before the fix, ground could not reliably correlate a received S5 TM frame to an on-board event ID unless the caller happened to embed it — and a caller could silently substitute a different identity in the auxiliary data. After the fix, the header is at fixed wire positions `[0:2] BE` + `[2]`, so ground GSW and on-board health/FDIR logic can deterministically extract event identity. This is a safety-relevant improvement for event reporting, health monitoring, and FDIR correlation.

**NC status.** `NC-EXT-237` is FIXED IN SOURCE + discriminating test added, but remains OPEN pending Software PA acceptance (`sprint-plan.md:324`). No NC closure claimed.

**Blocker?** No for host commit/push — the fix is complete and verified at host level. The NC is open by design pending PA disposition, which is a separate authority gate (N20/N21). Not a functional defect in the current tree.

---

### SYS-R1/R3-003 — Minor — `test_r2_gap.c` retains old caller-embedded-header pattern

**Evidence.**
- `tests/unit/test_r2_gap.c:832`: `const uint8_t payload[2] = {0x01u, 0x02u};`
- `tests/unit/test_r2_gap.c:850-853`: calls `FF_PusEmitEvent(0x0101u, FF_EVS_INFO, payload, 2)`, etc., with a 2-byte payload that coincidentally resembles an event-id+type header.
- `tests/unit/test_r2_gap.c:854-856`: asserts only `pus_stream_has(5u, 1u/2u/3u)` — subservice presence, not payload content or event-id bytes.
- `tests/CMakeLists.txt:19-23,237-238`: `test_r2_gap` is compiled and registered as CTest `r2_gap` (Test #17); it passed in the host run (55/55).

**Assessment.** After the NC-EXT-237 fix, `FF_PusEmitEvent` prepends its own `event id[2 BE] | type[1]` header, then appends the caller's `0x01 0x02` as auxiliary data. The wire payload is now `header + 0x01 0x02`, not `0x01 0x02` alone. The test still passes because it checks only subservice, but the test now exercises the **old caller-embedded-header pattern** with auxiliary data that happens to look like a header. This is misleading for future maintainers and does not exercise the emitter-owned identity contract. It also does not verify the actual event-id bytes on the wire (unlike `test_pus_event_identity.c`).

**Recommendation.** Update `test_r2_gap.c:832-856` to use clearly auxiliary data (e.g. free-text `{0x41,0x42}`) and/or add assertions on the wire payload bytes (`d[0:2]` = event id BE, `d[2]` = type). Align with the emitter-owned contract documented at `src/pus/pus.h:78-86`.

**Blocker?** No. The test passes; this is test-quality/maintainability, not a functional defect.

---

### SYS-R1/R3-004 — Minor — Target/RTEMS porting: `link_ip.c`/`link_tsn.c` now depend on `FF_PalGetMonotonicNs()`

**Evidence.**
- `CMakeLists.txt:139-140`: `link_ip.c` and `link_tsn.c` are in the **main source list**, built for both host and RTEMS target.
- `src/transport/link/link_ip.c:15`: now `#include "core/pal/pal.h"`; `link_ip.c:236,243`: calls `FF_PalGetMonotonicNs()` in `ip_send_until`.
- `src/transport/link/link_tsn.c:13`: now `#include "core/pal/pal.h"`; `link_tsn.c:245,252`: calls `FF_PalGetMonotonicNs()` in `tsn_send_until`.
- `src/transport/link/link_loopback.c:8-9,57,64`: same pattern already exists (committed precedent).
- `src/main/fsw_main.c:89`: installs `FF_LinkSendUntil` as the PUS panic reporter (pre-reset report path, SRS-REQ-074).
- `src/core/pal/pal_posix.c:589`: `FF_PalGetMonotonicNs()` host implementation (POSIX `clock_gettime`).

**Assessment.** The deadline-bounded `send_until` is correctly implemented for synchronous backends: entry check (no transmit if already expired) + post-transfer re-check (overrun reported as `FF_TIMEOUT`, never falsely as in-budget success). The documentation in `src/transport/link/link.h:23-42` honestly states the limitation: "An implementation whose underlying transmit call is synchronous cannot preempt a stalled driver."

The avionics concern is target-side: on the PolarFire SoC RTEMS 6.2 BSP, `FF_PalGetMonotonicNs()` must remain monotonic and advancing during panic/watchdog reset. If the monotonic clock stops, freezes, or resets during panic entry, the 250 ms absolute deadline bound in SRS-REQ-074 could be violated (expired-on-entry → `FF_TIMEOUT`, no report transmitted; or overrun misreported). This is a target-porting verification item, not a host code defect.

**Recommendation.** Before RTEMS target deployment: (1) confirm `FF_PalGetMonotonicNs()` implementation for the RTEMS/MPFS BSP is monotonic across watchdog reset; (2) add a target build/verification gate for `link_ip.c`/`link_tsn.c` (currently host-only evidence); (3) document the clock-assumption in the SRS-REQ-074 evidence (`docs/requirements/traceability.md` already notes "backend worst-case time is not established").

**Blocker?** No for host commit/push. Must be resolved before target deployment / SRS-REQ-074 closure.

---

### SYS-R1/R3-005 — Minor — `ip_send` argument contract tightened

**Evidence.**
- `src/transport/link/link_ip.c:185-198`: new shared `ip_check_tx()` rejects `(g_sock < 0) || (g_cfg.stack == NULL)` → `FF_NOT_READY`, and `(data == NULL) && (len != 0)` → `FF_INVALID_ARGUMENT`.
- `src/transport/link/link_ip.c:200-215`: `ip_send()` now calls `ip_check_tx()` first.
- Pre-change `ip_send` checked only `g_sock < 0` and `g_cfg.stack == NULL`; no NULL/len argument validation.

**Assessment.** The plain `send` writer now rejects `NULL` data with non-zero `len`, matching the `send_until` contract. This is a tightening, consistent with the shared contract documented in `link.h`. No known caller passes `NULL+non-zero` to `ip_send` (verified by grep), but callers should be reviewed if any external or future code uses the plain `send` writer with unchecked arguments.

**Blocker?** No. Contract tightening in the same direction as `send_until`; no caller impact found.

---

### SYS-R1/R3-006 — Minor — Operations interface: event-id catalogue uncontrolled; PUS profile provisional

**Evidence.**
- `docs/interfaces/ICD.md:126-138`: "Event-id catalogue note" — ICD §4 documents the S5 format but **not** a controlled event-id catalogue. Framework emits at least `FF_EVS_PANIC_RESET` (`0x0104`), `0x0502`, `FF_EVS_APP_ERROR`; full set of ground-visible ids is "not yet under interface-authority control" (NC-EXT-102). "Ground teams must not treat the ids emitted by this release as a controlled catalogue until the interface/requirements authority allocates them."
- `docs/interfaces/ICD.md:140-153`: "Provisional profile note" — every service/subservice row, incl. S5, is a provisional framework default, not an approved mission PUS profile.

**Assessment.** Ground event correlation depends on S5 event IDs that are not yet controlled. The wire format is fixed, but the semantics (which ID means what) are TBD. This is an operations interface gap that must be closed before operational use. It does not affect the correctness of the committed avionics code.

**Recommendation.** Track NC-EXT-102 to interface-authority allocation of event IDs; obtain mission PUS profile acceptance before operational use. Not a commit blocker.

**Blocker?** No. Operations/tracking item, not a code defect.

---

### SYS-R1/R3-007 — Minor (positive) — All six callers correctly pass text-only auxiliary data

**Evidence.**
- `src/main/fsw_main.c:40-42`: flight `ff_event_listener` passes `(const uint8_t *)evt->text, text_len` only; comment: "only the auxiliary data (the bounded reason text) is passed here."
- `src/main/fsw_main_reqtest.c:55-56`: same pattern.
- `tests/target/fsw_main_xw2.c:49-50`: same pattern (XW-2 test-only image).
- `tests/unit/test_r3_gap.c:532-534`: same pattern; comment: "the listener passes the reason text as auxiliary data only."
- `tests/unit/test_tm_output_chain.c:42-43`: same pattern (production listener copy).
- `tests/unit/test_evs_hs_health.c:307,351`: direct calls with text payload only.

**Assessment.** No caller prepends an event-id or type octet. The emitter-owned header contract is uniformly reflected. The wire format produced is `event id[2 BE] | type[1] | auxiliary data`, matching ICD §4 rev 1.7.

**Blocker?** No. Positive confirmation.

---

### SYS-R1/R3-008 — Minor (positive) — Pre-reset report wiring verified (SRS-REQ-074 support)

**Evidence.**
- `tests/unit/test_es_hs_lifecycle.c:558-631`: `test_panic_production_link_wiring_delivers_one_frame` installs `FF_LinkSendUntil` as panic reporter (exactly as `fsw_main.c:89`), triggers panic, and verifies exactly one CCSDS space packet on the link loopback with PUS service 5, subservice 3 (CRITICAL), event id `0x0104` (`FF_EVS_PANIC_RESET`), and reason text present.
- `src/pus/pus.c:217-219` (referenced in FRICK-REVIEW-0467): panic path writes 2-octet id + `FF_EVS_CRITICAL` type + bounded reason text.
- SRS-REQ-074 (`docs/requirements/SRS.md:434-444`): exactly one pre-reset PUS S5 report with panic/reset event id, CRITICAL severity, bounded lock-free path, 250 ms absolute deadline, no truncation of mandatory fields.

**Assessment.** The production wiring for the pre-reset report path is verified at host level: one frame, correct service/subservice/event-id, text present. This supports SRS-REQ-074. The deadline boundedness is exercised by `FF_LinkSendUntil` deadline tests (`test_link_ip.c:837-890`, `test_link_tsn.c:525-581`, `test_es_hs_lifecycle.c:633-654`).

**Blocker?** No. Positive confirmation; deadline worst-case time remains "not established" per `docs/requirements/traceability.md` (target item, see SYS-R1/R3-004).

---

## Cross-disciplinary consistency (GNC / safety / FDIR / operations)

| Discipline | Impact of NC-EXT-237 + send_until | Assessment |
|---|---|---|
| **GNC** | None. PUS service 5 is telemetry-only; GNC functions do not consume S5 events. | No impact. |
| **Safety** | Positive: emitter-owned event identity enables deterministic ground/on-board event correlation; panic event id `0x0104` is now reliably encoded in the pre-reset report. | Safety-positive; no new hazard introduced. |
| **FDIR** | Positive: health/FDIR logic can map received S5 frames to event IDs at fixed wire positions. Pre-reset report uses `FF_LinkSendUntil` (lock-free, independent of EVS/SB/PUS/queue/time locks). | Supports FDIR; deadline worst-case time not yet established on target (SYS-R1/R3-004). |
| **Operations** | Gap: event-id catalogue not interface-authority controlled (NC-EXT-102); PUS profile provisional. Ground teams must not treat IDs as controlled catalogue. | Must close before operational use (SYS-R1/R3-006). |

No GNC interface is affected. No safety requirement is weakened. FDIR is strengthened by the emitter-owned identity. Operations interface requires the catalogue/profile items to be accepted by authority.

---

## Verification evidence reviewed

- Host build (ASN1 off, fresh `/tmp/opencode/build-native`): 0 warnings, all targets built including `link_ip`, `link_tsn`, `test_pus_event_identity`, `test_evs_hs_health`, `test_r3_gap`, `test_tm_output_chain`, `test_es_hs_lifecycle`.
- CTest 55/55 passed (incl. `link_ip`, `link_tsn`, `es-hs-lifecycle`, `nc-ext-237`, `pus_event_identity`, `r2_gap`, `r3_gap`, `tm_output_chain`).
- `src/pus/pus.c:356-388` implementation vs `docs/interfaces/ICD.md:115` (rev 1.7) and `src/pus/pus.h:78-86` — consistent.
- All six callers verified text-only (file:line references in SYS-R1/R3-007).
- `test_pus_event_identity.c` mutation control: pre-fix body fails 33/106 checks; fixed body passes.
- Uncommitted `send_until` changes compile and pass `link_ip`/`link_tsn` tests; `es-hs-lifecycle` panic-wiring test passes.

---

## Determination

**No finding blocks host commit/push.** The committed NC-EXT-237 fix is complete, verified, and interface-consistent. The uncommitted `send_until` changes compile and pass host tests.

**Open items requiring authority disposition before target deployment or operational use:**
1. `NC-EXT-237` PA acceptance (SYS-R1/R3-002) — Software PA authority.
2. RTEMS target monotonic-clock behavior during panic for `FF_PalGetMonotonicNs()` (SYS-R1/R3-004) — target porting verification.
3. Event-id catalogue allocation and mission PUS profile acceptance (SYS-R1/R3-006) — interface/requirements authority.
4. `test_r2_gap.c` test-quality alignment (SYS-R1/R3-003) — FSW engineer, non-blocking.

This review does not close any NC, waive any gate, or approve any release. All safety-critical and regulatory decisions remain with authorized humans.
