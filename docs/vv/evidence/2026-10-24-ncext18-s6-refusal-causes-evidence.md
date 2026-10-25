# NC-EXT-18 (PUS S6 memory-access contract) — reproduced defect and fix evidence

Date: 2026-10-24
Tree: `/home/nicola/fsw/frick-fsw` (uncommitted working tree)
Sprint: S8-PUS-FU
Constraints retained: **N18 Renode remains FAIL (NC-EXT-198)** and was **NOT RUN**;
**NC-PAL-01 remains OPEN**. No emulator, hardware, mission parameter or wire-format
decision was made. No NC was closed. No frozen manifest was refreshed. No commit/push.

Independent review: `docs/reviews/2026-10-24-ncext18-s6-vv-review.md`
(FRICK-REVIEW-0437) — **ACCEPT / APPROVED WITH COMMENTS**, no Major findings.
Its F01/F02/F05/F06 corrections are applied in this record; the remaining findings
are registered as NC-EXT-214..219 and deferred under N21.

## 1. Scope actually addressed

NC-EXT-18 disposition reads: "define address width and memory-region ID in the
controlled interface **[authority-owned]**; implement RV64-safe range checks and
boundary tests **[FSW]**". Only the FSW half was addressed:

- the software-visible refusal causes were made distinguishable (they previously
  collapsed into a single `FF_ERROR`);
- the wrap-free bound check was documented and its adversarial boundary cases were
  added as tests, with a content oracle that actually discriminates.

**Not** addressed, still authority-owned: the field widths/layout of the
`TC[6,5]` profile, the approved memory-access authorization model, and the
PUS-C failure-code allocation that would separate the causes **on the wire**.

## 2. Defect reproduced before the fix

`test_memory_dump_bounded` was extended with assertions that the two refusal
causes differ, plus adversarial offsets (`0xFFFFFFFF`, `0xFFFFFFFE`, `0xFFFFFFFD`),
the half-open boundary pair (`size-1`/`1` accepted, `size-1`/`2` refused,
`size`/`1` refused) and region-resolution precedence.

Run against the pre-fix handler (`svc6_dump` as at `HEAD`; the pre-fix file hashes to
`49bc86db75762cc4face23012cd110fc921d1176fb39251f92dcb7842f6cc3cf`, which the
independent reviewer mechanically reproduced byte-identically by re-applying the
`HEAD` hunk to the final candidate):

```
FAIL tests/unit/test_pus.c:230: FF_PusIngest(frame, frame_len) == FF_INVALID_ARGUMENT
FAIL tests/unit/test_pus.c:263: st_unknown == FF_NOT_FOUND
FAIL tests/unit/test_pus.c:264: st_out_of_range == FF_INVALID_ARGUMENT
FAIL tests/unit/test_pus.c:266: st_unknown != st_out_of_range
FAIL tests/unit/test_pus.c:276: FF_PusIngest(frame, frame_len) == FF_NOT_FOUND
FAIL tests/unit/test_pus.c:291: FF_PusIngest(frame, frame_len) == FF_INVALID_ARGUMENT   (x3, loop)
FAIL tests/unit/test_pus.c:313: FF_PusIngest(frame, frame_len) == FF_INVALID_ARGUMENT
FAIL tests/unit/test_pus.c:324: FF_PusIngest(frame, frame_len) == FF_NOT_FOUND
tests/unit/test_pus.c checks=134 failures=10
```

`make test`: `98% tests passed, 1 tests failed out of 46` (`pus`).

This is the reproduced defect: a caller could not tell "no such region" from
"outside the region", so S6 refusals were not diagnosable in software. The prior
suite encoded the defect — both cases were asserted as `FF_ERROR`, which is why it
had never been caught (the reviewer additionally confirmed that the *old* test file
built against the *fixed* library fails 2 of 61 checks).

Check-count note: the pre-extension suite had 108 checks; the extended suite used for
the reproduction is 134 checks / 10 failures (corrected per review finding F01).

## 3. Minimal fix

`src/pus/pus_services.c` — `svc6_dump()`: region resolution is split from the
range check.

```diff
+    const ff_mem_region_t *region = NULL;
     for (uint16_t i = 0; i < FF_PUS_MAX_MEM_REGIONS; i++) {
-        if (!g_mem_regions[i].used || (g_mem_regions[i].id != region_id)) {
-            continue;
-        }
-        /* Provisional profile: ... */
-        if ((offset <= g_mem_regions[i].size) &&
-            ((uint32_t)size <= (g_mem_regions[i].size - offset))) {
-            const uint8_t *src = g_mem_regions[i].base + (ptrdiff_t)offset;
-            return FF_PusSendTm(FF_PUS_SERVICE_MEMORY, 6u, src, size);
-        }
+        if (g_mem_regions[i].used && (g_mem_regions[i].id == region_id)) {
+            region = &g_mem_regions[i];
+            break;
+        }
     }
-    return FF_ERROR; /* address outside any registered region */
+    if (region == NULL) {
+        return FF_NOT_FOUND;
+    }
+    if ((offset <= region->size) && ((uint32_t)size <= (region->size - offset))) {
+        const uint8_t *src = region->base + (ptrdiff_t)offset;
+        return FF_PusSendTm(FF_PUS_SERVICE_MEMORY, 6u, src, size);
+    }
+    return FF_INVALID_ARGUMENT;
```

Properties that make this the minimal correct delta:

- Refusal causes: `FF_NOT_FOUND` (unregistered identifier) vs
  `FF_INVALID_ARGUMENT` (region known, range outside) vs `FF_BAD_SIZE`
  (malformed request: payload length ≠ 7, `region id == 0`, `length == 0`,
  `length > FF_PUS_MAX_TM_FRAME` — all unchanged).
- Validation precedence, now documented in the ICD: payload length → `region id`/
  `length` range → region resolution → region bounds.
- Region resolution precedes the range check, so an unknown region is never
  reported as a range refusal (asserted).
- No wraparound: the ordered operands short-circuit before `region->size - offset`,
  and `offset + length` is never formed; both compared operands are `uint32_t`,
  so the evaluation is identical on RV32 and RV64 (the reviewer proved this at
  type level for both LP64 and `-m32` ILP32). `ptrdiff_t` is 64-bit on RV64 and
  `offset` is bounded by the region size before the pointer is formed.
- `offset <= region->size` rather than `<` is observationally identical because a
  zero length is rejected earlier; it is retained because it is the form recorded
  in ADR-0020 (reviewer confirmed a `<` mutant passes 134/0, i.e. no behavioural
  difference).
- **No wire change.** Every refusal still emits S1(1,8) with
  `FF_PUS_FAIL_EXECUTION` when the completion ack is set and no response when clear.

## 4. Test-oracle correction found by the independent review (F02)

The dump-content oracle previously compared a **zero-filled** static region against
itself, so it could not detect a handler that read the wrong offset: the reviewer's
mutant "offset ignored when forming `src`" survived 134/0.

Fixed in this increment: `test_memory_dump_bounded` now fills `g_region` with a
deterministic index-dependent pattern (`0xA5u ^ (i & 0xFFu)`) before registration.
Discrimination re-proved by mutation on the final source:

```
mutant: region->base + (ptrdiff_t)0   (offset dropped)
  FAIL tests/unit/test_pus.c:209: stream_has_memory_bytes(4u, 4u)
  FAIL tests/unit/test_pus.c:228: stream_has_memory_bytes(sizeof(g_region) - 1u, 1u)
  FAIL tests/unit/test_pus.c:311: stream_has_memory_bytes(sizeof(g_region) - 1u, 1u)
  tests/unit/test_pus.c checks=134 failures=3        -> mutant KILLED
restored source: 134 checks, 0 failures             -> PASS
```

The independent delta review re-derived this independently: 3 failures at lines 209,
228 and 311, with the out-of-region and boundary assertions still passing (only the
content assertions move), and confirmed by exhaustive enumeration that the fill
pattern `0xA5u ^ (i & 0xFFu)` is injective over `[0,64)` for every reachable
offset/length pair (0 aliasing cases).

## 5. Documentation deltas (descriptive, no wire/profile change)

- `docs/interfaces/ICD.md` rev 1.6: the S6 row names the distinct
  software-visible statuses, restates that **none** of the refusal causes is
  distinguishable on the wire, and records the validation precedence; the
  failure-code allocation is explicitly deferred to the interface authority.
- `docs/adr/ADR-0020…` rev 0.2: records the refusal-cause distinction and the
  wrap-free bound check; provisional profile values unchanged; the ADR decision
  date is retained at 2026-10-16 with the change carried in the revision history.

## 6. Gates on the final candidate — exact results

| Gate | Result |
| --- | --- |
| `make build` | **PASS** |
| `make test` | **PASS** — 100% tests passed, 0 failed out of 46 |
| `test_pus` | **PASS** — `checks=134 failures=0` (pre-fix: 134 checks / 10 failures) |
| `make format-check` | **PASS** in `frick-vv:local` (`clang-format --dry-run --Werror`, no output) |
| `make trace` | **PASS** (120 requirements, 76 srs, 30 sss, 14 gaps, 68 Lean theorems) |
| `make verify-lean` | **PASS** (`lake build` completed successfully, in `frick-vv:local`) |
| `make gsw-selfcheck` | **PASS** (12 + 9 + 14 python checks OK; `license_check` OK; GSW ping frame self-check OK) |
| `make renode` (N18) | **FAIL — unchanged (NC-EXT-198), NOT RUN per owner directive** |
| RTEMS cross-build | **NOT RUN** in this cycle |
| RV32/RV64 execution | **NOT RUN** (bound check proven at type level only, per review) |

## 7. Candidate hashes (SHA-256, final)

```
055086117dfb0596397fbe78c860e008394685f54f06a488cf9df4bf188dcebb  src/pus/pus_services.c
ac6889745882b4c001a4913cfe2564aef8a17ff4ef7356c8b9a69eeac14f1d71  tests/unit/test_pus.c
67ae7584a4f8adb50210be0933d5ea6f10e5ac03466a3c09e67b105d452643ad  docs/interfaces/ICD.md
b7e33e1af921a81ec0604fb464f9fd4c200bf16b9741d06e615ab07d540bcb29  docs/adr/ADR-0020-provisional-pus-and-scheduler-policies.md
```

## 8. What remains open after this increment

- **On-wire cause separation** — needs the PUS-C failure-code allocation from the
  interface authority (NC-EXT-18 authority half).
- **Approved memory-access authorization model** and the field widths/layout of
  the `region_id/offset/length` profile remain provisional (ADR-0020).
- **No target, timing or qualification evidence** exists for this path; all
  evidence is host-native, and RV32/RV64 execution was not exercised.
- Review findings registered as NC-EXT-214..219 and deferred under N21.

## 9. Revision history

| Rev | Date | Author | Change |
| --- | --- | --- | --- |
| 0.1 | 2026-10-24 | frick-fsw (delegated) | Record the reproduced S6 refusal-cause indistinguishability, the minimal fix, the adversarial boundary tests and the full gate results with candidate hashes. |
| 0.2 | 2026-10-24 | frick-fsw (delegated) | Apply independent review findings: correct the pre-fix check count (F01), replace the degenerate zero-filled payload oracle with an index-dependent pattern and retain the mutation proof that it now kills the offset-dropping mutant (F02), sharpen the ICD wire-indistinguishability and precedence wording (F05), restore the ADR decision date (F06). Refresh the gate table and hashes. |
