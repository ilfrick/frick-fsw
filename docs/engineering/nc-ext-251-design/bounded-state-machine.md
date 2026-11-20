# NC-EXT-251 Design Note: Bounded State Machine for PUS Lock-Free TC Dispatch

**Date:** 2026-11-15 (status updated 2026-11-17)
**Status:** PARTIALLY IMPLEMENTED — this note is a design target, not a
description of the current code. What was actually implemented on 2026-11-17 is
listed in §9; the per-TC reservation slots, rollback and the full lock split
described below are **not** implemented and remain a proposal.
**Baseline:** `fc68b9d121bce2194c1beaaa49a7dc4cdab0552c` (verified: `git cat-file -t` = commit) is superseded; the
implementation baseline for §9 is `c166cea36483c3262a64edd63dc4368fcbc119df`
plus the working-tree change in `src/pus/pus.c`.

---

## 1. Constraints Summary (Owner Dispositions)

| Constraint | Source |
|------------|--------|
| Preserve SRS-REQ-006 exact-one ACK per requested stage | SRS / Owner |
| Reserve bounded resources for requested S1 ACK stages **before** TC acceptance | Owner |
| Separate try-only TC dispatch serialization (not the state mutex) | Owner |
| PUS state mutex: try-only, never held across handlers/callbacks/TM sink | Owner |
| Map `FF_TIMEOUT` → `FF_BUSY`; propagate other PAL errors distinctly | Owner |
| `FF_PusSendTm` contention returns `FF_BUSY` for caller retry | Owner |
| `FF_PusSendTmDirect` remains lock-free with shared atomic modulo-16384 counter | Owner |
| Keep synchronous `void` TM sink; do not weaken SRS, change sink contract, add worker, or claim downstream delivery | Owner |
| No architecture/authority change without explicit `DECISION-FOR-NICOLA` | Process |

---

## 2. Bounded State Machine

### 2.1 States and Capacity

```
TC Dispatch State Machine
┌─────────────────────────────────────────────────────────────────────┐
│ Capacity: 1 TC dispatch at a time (serialized by dispatch lock)    │
│ Reservation slots: up to 4 S1 ACK stages per TC (accept/start/     │
│                    progress/completion)                             │
│ Lifetime: reservation exists from pre-acceptance until TC accepted  │
│           or rejected; released on acceptance or error/rollback     │
└─────────────────────────────────────────────────────────────────────┘
```

**State variables (added to PUS module, protected by new dispatch mutex):**

```c
typedef struct {
    bool         in_dispatch;           // A TC is being processed
    uint8_t      reserved_stages;       // Bitmask of 4 ACK stages reserved
    FF_Pus_TcHdr_t current_tc;          // Copy of TC being dispatched
    uint8_t      tc_buf[FF_PUS_MAX_TC_FRAME]; // TC raw bytes for re-emission
    uint16_t     tc_len;
} ff_pus_dispatch_t;
```

**Capacity limits:**
- Max 1 concurrent dispatch (try-lock on dispatch mutex)
- Max 4 reservation slots per TC (the 4 S1 stages)
- If dispatch mutex unavailable → `FF_BUSY` immediately (no queue)
- If any stage reservation fails → rollback all, return `FF_BUSY`

---

### 2.2 Lock Order

```
1. Dispatch mutex (try-only): acquired first, released after acceptance OR on any error
   ↓ protects: in_dispatch, reserved_stages, current_tc, tc_buf
2. State mutex (try-only): acquired ONLY for:
   - Reading/writing service registry (ff_pus_find)
   - Reading/writing TM counter (FF_PusNextTmCounter)
   - Reading/writing backend/tm_sink pointers
   NEVER held across: handler execution, TM sink call, FF_PusSendTm
3. Direct path: NO mutex; uses atomic counter only
```

---

### 2.3 Stage-to-ACK Mapping

| Stage Bit | SRS Stage | Verification Subservice (success) | Subservice (failure) |
|-----------|-----------|-----------------------------------|----------------------|
| Bit 0 (0x01) | Acceptance | 1 | 2 |
| Bit 1 (0x02) | Start | 3 | 4 |
| Bit 2 (0x04) | Progress | 5 | 6 |
| Bit 3 (0x08) | Completion | 7 | 8 |

**Reservation protocol:**
- At TC decode, compute `stage_mask = tc.ack_flags & 0x0F` (4 LSB)
- Attempt to reserve ALL requested stages atomically under dispatch mutex
- If any stage cannot be reserved (internal counter exhausted) → rollback, `FF_BUSY`
- Reservation released when ACK is emitted OR TC rejected

---

### 2.4 Pre-Acceptance Rollback on Rejection/Error

```
TC arrives
    ↓
Try acquire dispatch mutex (FF_PalMutexTryLock)
    ├─ FAIL → FF_BUSY (no state mutation)
    ↓
Decode TC, extract ack_flags
    ↓
Reserve ALL requested stages (check internal capacity)
    ├─ FAIL → release dispatch mutex, FF_BUSY (no state mutation)
    ↓
Acquire state mutex (try) → read registry → release state mutex
    ├─ FAIL (registry lookup) → rollback reservations, FF_BUSY
    ↓
ILLEGAL APID check
    ├─ FAIL → if ack_accept: emit failure ACK (state mutex try), rollback, FF_ERROR
    ├─ FAIL → if no ack_accept: rollback, FF_ERROR
    ↓
HANDLER NOT FOUND check
    ├─ FAIL → if ack_complete: emit failure ACK (state mutex try), rollback, FF_NOT_FOUND
    ├─ FAIL → if no ack_complete: rollback, FF_NOT_FOUND
    ↓
ACCEPTANCE ACK (if requested)
    ├─ Try state mutex → FF_PusSendVerification → release state mutex
    ├─ FAIL → rollback reservations, return error code
    ↓
START ACK (if requested)
    ├─ Try state mutex → FF_PusSendVerification → release state mutex
    ├─ FAIL → rollback reservations, return error code
    ↓
TC ACCEPTED: clear in_dispatch, release dispatch mutex
    ↓
EXECUTE HANDLER (NO mutex held)
    ↓
PROGRESS ACK (if requested) → same try-lock pattern
    ↓
COMPLETION ACK (if requested) → same try-lock pattern
    ↓
All done: reservations fully consumed
```

**Key invariant:** At any error before "TC ACCEPTED", all reservations are rolled back and no ACK is emitted for that TC.

---

### 2.5 Handler-Originated TM and Sink Reentry

- Handler may call `FF_PusSendTm` (ordinary TM) → tries state mutex
  - Contention → returns `FF_BUSY` (caller retries or drops)
  - Success → increments TM counter, calls sink, releases state mutex
- Handler may call `FF_PusIngest` (nested TC) → tries dispatch mutex
  - Contention → returns `FF_BUSY` (nested ingest rejected)
  - Success → proceeds as separate dispatch
- TM sink callback (`g_tm_sink`) executes **without any PUS mutex held**
  - Sink is `void`, synchronous, unbounded
  - Emission is **not re-entrant**: `FF_PusSendTm` holds an atomic emission
    guard (`g_tm_emitting`) across frame composition and handover, so a sink
    that calls `FF_PusSendTm` (or `FF_PusSendTmDirect` from a second context
    while an emission is in flight) is refused with `FF_BUSY` and emits nothing.
    A sink that needs to react to a frame must queue the reaction and emit after
    returning
  - A sink that calls `FF_PusIngest` is *not* blocked by the emission guard, but
    the dispatch reservation is held for the whole ingest, so the nested ingest
    is refused with `FF_BUSY`
  - Sink non-termination is not bounded by PUS (documented boundary)

---

### 2.6 Blocked/Nonterminating Callback Behavior

| Scenario | Behavior |
|----------|----------|
| `FF_PusSendTm` contention (state mutex busy) | Returns `FF_BUSY`; caller decides retry/drop |
| `FF_PusIngest` nested (dispatch mutex busy) | Returns `FF_BUSY`; caller decides retry/drop |
| TM sink never returns | Not bounded; transport must provide deadline if required |
| Handler never returns | Not bounded; application must provide watchdog |

**PUS does not enforce timeouts on callbacks.** This is a documented limitation.

---

### 2.7 Backpressure and Real-Time Limits

- No queueing: `FF_BUSY` is immediate, no waiting
- Caller (e.g., link layer) must implement retry/backoff
- Max 1 TC in dispatch + 1 ordinary TM emission at a time
- `FF_PusSendTmDirect` is completely independent (atomic counter only)
- No dynamic allocation after init (SRS-REQ-015)

---

## 3. API Changes Required

### New Public Status Codes

None (reuse existing: `FF_BUSY`, `FF_TIMEOUT`, `FF_SUCCESS`, etc.)

### Modified Signatures (to expose lock failure)

```c
// Currently void; must return FF_Status_t to propagate FF_BUSY
FF_Status_t FF_PusSetTmSink(FF_Pus_TmSink_t sink);
FF_Status_t FF_PusInstallPanicReporter(FF_Pus_DirectWriter_t writer);

// Currently returns uint16_t; no error channel
// Option A: add out-param for status
// Option B: keep as-is (counter always succeeds if state mutex acquired)
// Decision: keep scalar; counter increment failure only on mutex, which is try-lock → FF_BUSY
uint16_t FF_PusNextTmCounter(void);          // unchanged
uint16_t FF_PusPeekTmCounter(void);          // unchanged
uint32_t FF_PusGetTcCount(void);             // unchanged
```

**Minimal approach:** Keep counter getters as scalar; only `FF_PusSendTm` and `FF_PusIngest` return `FF_BUSY` on contention. `FF_PusSendTmDirect` is lock-free.

---

## 4. Test Plan for RED Phase

### 4.1 First RED Test (Only This One for Now)

**Test:** `test_pus_dispatch_reservation_contention`

```
Setup:
  1. Initialize PUS, set backend, set TM sink
  2. Spawn thread A: hold ordinary TM sink resource (block in sink callback)
  3. Thread B: attempt ACK-requesting TC (accept/start/progress/completion)

Expected RED behavior (before implementation):
  - TC dispatch should succeed (old code has no reservation)
  - Test asserts FF_BUSY is returned before acceptance
  - No handler invoked, no S1 ACK emitted, no TC/counter mutation from this TC
  - Thread A's ordinary TM emission still gets its counter increment
  - Both threads joined cleanly

Assertion that fails (RED):
  FF_TEST_ASSERT(rc == FF_BUSY);
```

### 4.2 Later Tests (Not Added Yet)

| Test | Scenario |
|------|----------|
| `test_pus_dispatch_max_four_stages` | Reserve all 4 stages, verify all ACKs emitted |
| `test_pus_dispatch_stage_order` | ACKs emitted in order: accept → start → progress → completion |
| `test_pus_dispatch_reservation_release` | Error before acceptance releases all slots |
| `test_pus_dispatch_exhaustion` | Second TC while first holds slots → FF_BUSY |
| `test_pus_dispatch_accepted_path_contention` | Accepted TC handler calls FF_PusSendTm → FF_BUSY or success |
| `test_pus_dispatch_sink_reentry` | Sink calls FF_PusIngest → FF_BUSY (dispatch busy) |

---

## 5. Viability Assessment

**DECISION-FOR-NICOLA CHECK:**
- All constraints can be satisfied within current architecture
- No SRS weakening required
- No sink contract change required
- No worker thread required
- Only API additions: `FF_PusSetTmSink`/`InstallPanicReporter` return status
- No production test hook needed (test uses real PAL threads + real sink)

**Verdict:** Design is viable. Proceed to RED test implementation.

---

## 6. Implementation Sketch (For Reference Only — No Edits Yet)

```c
// New module-level dispatch state
static FF_MutexId_t g_dispatch_mutex;
static bool g_dispatch_mutex_created = false;
static ff_pus_dispatch_t g_dispatch;

// In FF_PusIngest, before any state mutation:
FF_Status_t st = FF_PalMutexTryLock(g_dispatch_mutex);
if (st != FF_SUCCESS) return FF_BUSY;
g_dispatch.in_dispatch = true;
// ... reserve stages ...
// ... on acceptance: g_dispatch.in_dispatch = false; FF_PalMutexUnlock(g_dispatch_mutex);
// ... on any error: rollback reservations; g_dispatch.in_dispatch = false; FF_PalMutexUnlock(g_dispatch_mutex);

// FF_PusSendTm uses state mutex try-lock; contention → FF_BUSY
// FF_PusSendTmDirect uses atomic counter only (no mutex)
```

---

---

## 9. What was actually implemented (2026-11-17)

The delivered change is deliberately narrower than the design target above. In
`src/pus/pus.c`:

- `FF_PusSendTm` now checks the `FF_PalMutexLock` result. On `FF_TIMEOUT` it
  returns `FF_BUSY`; any other PAL error is propagated. It never unlocks a lock
  it does not hold. This is the defect that made the mutual exclusion ineffective
  for handler-emitted telemetry.
- The TM sequence counter is a single compare-exchange
  `ff_pus_next_tm_counter_atomic()` (`__atomic_load_n` + `__atomic_compare_exchange_n`
  with `next = (uint16_t)((cur + 1u) & 0x3FFFu)`) shared by both the normal path and
  the reset/panic direct path. A single masked sequence on one atomic object is what
  keeps the two paths from diverging: an earlier version used a plain read-modify-write
  under the mutex for one path and an `__atomic_fetch_add` for the other, so storage
  could exceed 16383 and two frames could share a counter value. `FF_PusSendTm` now
  *peeks* the value, composes the frame, and only then commits the advance with
  `ff_pus_tm_counter_try_advance()`, retrying the composition if another context moved
  the counter meanwhile; a frame that is never transmitted (no sink, encode failure)
  consumes no value.
- `FF_PusIngest` releases `g_pus_mutex` around `handler(...)` and re-acquires it
  afterwards. The state mutex is therefore no longer held across the handler
  callback, as this note requires. The dispatch reservation (`g_dispatch_reserved`)
  stays set across the handler, so a concurrent ingest still fails with
  `FF_BUSY`.

Not implemented from this note: the per-TC reservation slots with rollback, the
separate state/dispatch/emission lock tiering, and the try-only lock protocol.
`g_dispatch_reserved` remains a single boolean guarded by the one mutex.

Verification: N22 case `test_sink_recursive_tm_busy` in
`tests/unit/test_pus_pre_acceptance_ack_reservation.c` asserts that a
sink-recursive emission is refused with `FF_BUSY`, that the outer dispatch still
emits exactly one acceptance and one completion ACK, that no frame is emitted for
the refused call, and that a later dispatch succeeds. TDD RED to GREEN is retained
in `docs/vv/evidence/2026-11-17-m9-m11-renode-hal/tdd-nc-ext-251/` (2 failures on
the previous code; `checks=147 failures=0` on the corrected code). NC-EXT-251 is
corrected but **not closed**; closure requires the corrected-tree R1/R2/PA
reviews.