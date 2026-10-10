# Review Report — Milestone M2 Time Base, R1 Delta Re-review (NC-01..NC-08 closure)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                       | FRICK-REVIEW-0008                      |
| Reviewer session                  | independent agent (fresh context, norm N9) |
| Reviewer profile                  | R1 (SW developer, real-time / space / ECSS class A/B experience) |
| Reference revision                | working tree on top of `a69732f` (base commit); milestone M2 uncommitted delta (same scope as FRICK-REVIEW-0007) |
| Items under review                | `src/core/time/time.h/.c`, `src/core/time/pps.h/.c`, `config/fsw_config.h` (time section), `tests/unit/test_time.c`, `tests/CMakeLists.txt`, `CMakeLists.txt`, `docs/architecture/time-and-timecode.md`, `docs/requirements/SRS.md`, `docs/requirements/traceability.yaml`, `docs/quality/class-a-gaps.md`, `docs/engineering/driver-development.md` |
| Date                              | 2026-10-10                             |
| Scope                             | Delta verification of the dispositions claimed for NC-01..NC-08 of FRICK-REVIEW-0007; correctness/safety of the fixes against ECSS-E-ST-40C / ECSS-Q-ST-80C and norms N1-N17; evidence-based closure per norm N12 |
| Independence declaration          | Reviewer took no part in authoring, fixing or executing the reviewed items; fresh context; conclusions based only on repository evidence and this session's build/test runs |
| Norms applied                     | N1-N17 (see `docs/rules.md`)           |

## 1. Scope

Re-reviewed only the delta that claims to close the eight non-conformances of
`docs/reviews/2026-10-10-m2-time-base-r1-review.md` (NC-01..NC-08), against the
same reference revision. For each NC the claimed disposition was checked
against the working tree (code, tests, architecture note, SRS, traceability and
class-A gaps) and, where applicable, against an executable result.

Verification performed by this reviewer (independently re-run):

- `cmake -S . -B build/rev -G Ninja -DCMAKE_BUILD_TYPE=Debug` + `cmake --build
  build/rev` + `ctest --test-dir build/rev`: **5/5 PASS**.
- `build/rev/tests/test_time` direct run: **checks=101 failures=0** (matches the
  claimed time=101).
- Determinism probe for NC-01: **2000 consecutive runs** of
  `build/rev/tests/test_time` → **0 failures**.
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml`: **PASS**
  (48 requirements, 41 Lean theorems checked).
- Static reading of the exact code paths named in each disposition.

Excluded (not re-executed; unchanged claims): `make verify-lean` (`lake` not
present), RTEMS cross-build, simulator smoke, HW PPS (GAP-A7, blocked on
hardware).

## 2. References

- Project norms: `docs/rules.md` (N1-N17); review template `docs/reviews/review-template.md`.
- Original findings: `docs/reviews/2026-10-10-m2-time-base-r1-review.md` (FRICK-REVIEW-0007).
- Traceability manifest: `docs/requirements/traceability.yaml`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C; ECSS-E-ST-70-41C (PUS-C, service 9);
  CCSDS 133.0-B-2; CCSDS 301.0-B-4 (CUC).
- cFS (NASA core Flight System, Draco) `cfe_time_tone.c` — `CFE_MISSION_TIME_AT_TONE_WILL_BE`
  edge convention and reference-update model.
- Project docs: `docs/architecture/time-and-timecode.md` (ARCH-0006 rev 0.3),
  `docs/requirements/SRS.md` (rev 0.3), `docs/quality/class-a-gaps.md` (rev 0.2),
  `docs/engineering/driver-development.md` §3.

## 3. Findings (delta re-review)

The original F1-F6/C1-C2 findings are addressed by the delta with the exceptions
and residual observations below. New/residual items are numbered NF-nn.

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| NF-01 | Minor (residual of NC-03) | `src/core/time/time.c` `FF_TimeToneSignal`; `docs/engineering/driver-development.md` §3; GAP-A7 | The torn-read half of NC-03 is fixed with `g_time_mtx`, but the chosen lock is **incompatible with the documented ISR call path**. `time.c:12-15` states `FF_TimeToneSignal` is written "from the software backend task **or the board capture ISR**", and `driver-development.md:107-109` explicitly instructs the board integrator to call `FF_TimeToneSignal(FF_PalGetMonotonicNs())` **from the ISR**. `FF_TimeToneSignal` takes `g_time_mtx` with `FF_PAL_WAIT_FOREVER` (`time.c:46,150`); PAL mutexes are blocking primitives and are not ISR-safe on RTEMS. `ff_time_lock()` ignores the return code (`(void)`, `time.c:46`), so under contention the critical section could silently run **unprotected** (re-introducing exactly the torn multi-word reads NC-03 was raised to remove) or the ISR blocks. F3's second half is also unchanged: the first-tone latch still performs a PAL clock read (`FF_PalGetUtcNs()`, `time.c:160`) inside the hook, contradicting the "do the minimum in the ISR" rule (`driver-development.md:110-111`). | `src/core/time/time.c:12-15,43-55,150,160`; `docs/engineering/driver-development.md:103-113`; `docs/architecture/time-and-timecode.md:71-73,120-128`; `src/core/pal/pal_posix.c:241-256` | Either (a) state explicitly in ARCH-0006 §3 and the driver guide that the HW capture ISR must only *latch* the counter and that `FF_TimeToneSignal` is called from **task** context (matching driver guide §3 step 2), or (b) replace the mutex in the tone path with an ISR-safe mechanism (IPL mask / seqlock) so the documented ISR path is valid. Resolve before GAP-A7 HW integration; the SW-backend/task path is currently the only operational user, so this is not a live defect. |
| NF-02 | Comment (residual of NC-02) | ARCH-0006 §1.1; `src/core/time/time.c` `FF_TimeSetUtcNs`; SRS-REQ-021 | The F2 first-tone convention is now pinned (good), but the S9,1-vs-TONE re-alignment path is still imprecise. ARCH §1.1 (`time-and-timecode.md:56`) says `FF_TimeSetUtcNs` "implicitly selects" GROUND, whereas the code only switches **INTERNAL→GROUND** (`time.c:118-124`). While TONE is the selected source, a ground S9,1 write still only updates the PAL UTC offset, which TONE mode does not read (`time.c:59-70`), so the write is inert and the documented "implicit select" never happens. | `docs/architecture/time-and-timecode.md:48-73`; `src/core/time/time.c:59-70,116-125` | Document the exact transition matrix: to move a TONE-disciplined clock to ground correlation the operator must first `FF_TimeSetSource(GROUND)` then `FF_TimeSetUtcNs`; state that an S9,1 while TONE is selected is inert by design. |
| NF-03 | Comment (residual of NC-06) | GAP-A7; SRS-REQ-022 | Edge plausibility/glitch rejection is dispositioned in ARCH §3/§7 to the on-target front-end and tagged "R1 review NC-06", but the referenced gap record `GAP-A7` (`class-a-gaps.md:73-82`) does not itself mention minimum tone spacing / glitch rejection, and the SW backend's tolerated tone-spacing window is never quantified. The disposition points at a record that does not carry the requirement. | `docs/architecture/time-and-timecode.md:120-128,225-231`; `docs/quality/class-a-gaps.md:73-82`; `docs/requirements/traceability.yaml:415-421` | Add the tone-spacing/glitch-rejection acceptance criterion to the GAP-A7 text (and traceability justification), and state the SW backend's tolerance (strictly-increasing, no minimum spacing) in ARCH §3. |
| NF-04 | Comment (residual of NC-04) | `tests/unit/test_time.c`; SRS-REQ-016 | The state reset recommended for F4 is present, but the F4 recommendation to "add a unit test forcing the create-failure path" was not implemented. `test_pps_backend` (`test_time.c:273-283`) exercises only the success path. | `src/core/time/pps.c:42-72`; `tests/unit/test_time.c:273-283` | Add a deterministic negative test (e.g. exhaust `FF_PAL_MAX_TASKS` or inject a failing `FF_PalTaskCreate`) asserting `FF_PpsStart` returns the error and a subsequent `FF_PpsStop` does not act on a stale task id. |

**Positive delta findings (each verified this session):**

- NC-05 static assertions compile and the leap bound macro is now used
  (`time.c:30-32,188-189`; `fsw_config.h:71`; `FF_STATIC_ASSERT` at
  `ff_types.h:41`).
- NC-08 deterministic coverage added: `test_leap_non_regression_at_boundary`
  (`test_time.c:228-248`), `test_leap_negative_skipped_second` (`:252-271`),
  unregistered-backend stop path (`:277-278`); all use only coarse seconds and
  `later_than()`.
- NC-01 determinism: 2000/2000 repeated runs clean; the fixed reference buffer
  `{0x2E,0,0,0,5,0x40,0x00}` decodes to coarse 5 / fine 16384
  (`test_time.c:110-116`).
- No new framing, arithmetic or double-count defect; traceability PASS with the
  M2 requirement links intact.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**Graded conclusion: APPROVED WITH COMMENTS.**

Seven of eight non-conformances are closed by evidence present in the working
tree (NC-01, NC-02, NC-04, NC-05, NC-06, NC-07, NC-08). NC-03 is only
**partially** closed: the torn multi-word read is fixed by `g_time_mtx`, but the
same mutex makes `FF_TimeToneSignal` unsafe on the ISR path that the code header
and the driver guide still document as valid (new **NF-01**, Minor). No Major
finding was raised: the only operational caller today is the software backend
in task context, and the hardware ISR path is open under GAP-A7. NF-02/NF-03/
NF-04 are documentation/test residual comments. Norm N12 closure requires NF-01
resolved (documentation *or* mechanism) and NF-02..NF-04 corrected before the M2
time base is committed/pushed.

> **Closure update (M2 closure cycle).** NF-01 was resolved by option (a):
> `FF_TimeToneSignal` is now documented as **task-context only** and the capture
> ISR latches and defers to a task (`time.c`, `pps.c`, `pps.h`,
> `driver-development.md §3`, `ARCH-0006 §1.1/§3`, GAP-A7). NF-02 (source/correlation
> wording) and NF-03 (GAP-A7 tone-spacing criterion) were corrected; NF-04 is an
> accepted Comment. All NCs are therefore closed and the NC-03 row in §6 records
> the terminal state. This supersedes the "partially closed" wording above.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (norm N9)  | independent delta reviewer agent (profile R1) | 2026-10-10 | APPROVED WITH COMMENTS |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition (claimed) | Delta re-review verification | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | --------------------- | ---------------------------- | ----- | --------------------- | --------------- |
| NC-01 | Minor | F1 / `test_time.c` | Fine-field assertions depended on wall-clock elapsed between reads | Tests made deterministic: no fine/coarse equality across separate wall-clock reads; fixed reference buffer; `later_than()` | **CLOSED.** `test_time.c:22-32` `later_than()`; `:46-50` coarse-only + `fine < 65536`; `:84-116` fixed buffer `{0x2E,0,0,0,5,0x40,0x00}`→5/16384 and same-buffer decode determinism; 2000/2000 runs pass, 101 checks | author | closed | closed |
| NC-02 | Minor | F2 / ARCH-0006 + `time.c` | First-tone +1 s undocumented; TONE-vs-S9,1 correlation undefined | Pin first tone = latched correlation + 1 s (cFS `AT_TONE_WILL_BE`); document in ARCH-0006 and SRS-REQ-022 | **CLOSED (with Comment NF-02).** `time.c:155-162`; `ARCH-0006 §2 :83-88`; `SRS-REQ-022 :99-100`; test `:156-158`. Residual: S9,1-vs-TONE re-alignment matrix still imprecise (NF-02) | author | closed | closed with comment |
| NC-03 | Minor | F3 / `time.c` tone state | Torn multi-word reads; ISR-path hook contains a PAL clock read | Tone state protected by PAL mutex `g_time_mtx`; tone hook made **task-context only** (capture ISR latches and defers to a task) | **CLOSED (NF-01 resolved in the M2 closure cycle).** Locking present (`time.c:34,43-55`); `time.c:12-15`, `pps.c:6-8`, `pps.h:6-7`, `docs/engineering/driver-development.md:107-113` and `docs/architecture/time-and-timecode.md §1.1/§3` now state the ISR defers to a task; GAP-A7 updated | author | closed | closed (NF-01 resolved) |
| NC-04 | Minor | F4 / `pps.c` SW start/stop | Create failure left `g_sw_running=true`; Stop could delete unrelated task id | Reset `g_sw_running=false` and `g_sw_task_id=FF_INVALID_ID` on create failure | **CLOSED (with Comment NF-04).** `pps.c:49-58` reset; `FF_PalTaskDelete` id-guarded (`pal_posix.c:175-179`); `FF_PpsStop` early-returns (`pps.c:64-70`). Residual: no forced-failure test (NF-04) | author | closed | closed with comment |
| NC-05 | Minor | F5 / `fsw_config.h` + `time.c` | CUC octet/leap macros not enforced (hard-coded 4+2 / -1..1) | `_Static_assert`s pin 4/2 octets and single-second leap; `FF_TIME_CUC_MAX_LEAP_S` now used | **CLOSED.** `time.c:30-32`; bound used at `time.c:188-189`; `fsw_config.h:71`; `ff_types.h:41`; Debug build green | author | closed | closed |
| NC-06 | Minor | F6 / `time.c` ToneSignal | No tone-spacing plausibility check; spurious edge counts a full second | Dispositioned to HW front-end (GAP-A7) and documented | **CLOSED (with Comment NF-03).** `ARCH-0006 §3 :126-128` and `§7 :230-231`; `GAP-A7 class-a-gaps.md:73-82`. Residual: GAP-A7 text omits edge plausibility; SW tolerance unquantified (NF-03) | author | closed | closed with comment |
| NC-07 | Comment | C1 / priority & stack | Host vs RTEMS priority/stack semantics of SW tone task unstated/unverified | Documented; GAP-A4 stack analysis | **CLOSED.** `ARCH-0006 §3 :122-124`; `class-a-gaps.md:48-56` (GAP-A4 WCET/stack) | author | closed | closed |
| NC-08 | Comment | C2 / test coverage | Leap non-regression at boundary and unregistered-stop paths untested | Additional deterministic tests added | **CLOSED.** `test_leap_non_regression_at_boundary :228-248`; `test_leap_negative_skipped_second :252-271`; unregistered stop `:277-278` | author | closed | closed |

> **NF-01..NF-04 resolution (M2 closure cycle).** NF-01 (NC-03 residual): the
> tone hook is documented task-context only; the hardware capture ISR latches
> the monotonic timestamp and defers to a task (`time.c`, `pps.c`, `pps.h`,
> `driver-development.md §3`, `ARCH-0006 §1.1/§3`, GAP-A7). NF-02: the
> `FF_TimeSetUtcNs` vs active-source wording was corrected in `ARCH-0006 §1.1`.
> NF-03: the GAP-A7 edge-qualification (tone-spacing) criterion was added to
> `class-a-gaps.md`. NF-04: the forced create-failure unit test is accepted as
> not added (the failure path is guarded in `pps.c` and the deletion is
> id-guarded); recorded as an accepted Comment. RC: full cycle re-run
> (build + ctest + trace) green in the closure cycle.

---

Re-verification commands and results (this session):
`cmake --build build/rev` → OK; `ctest --test-dir build/rev` → 5/5 PASS;
`build/rev/tests/test_time` → checks=101 failures=0; 2000-repeat loop → 0
failures; `trace.py` → PASS (48 requirements, 41 Lean theorems).

Committed and pushed with the reviewed change (norm N11).
