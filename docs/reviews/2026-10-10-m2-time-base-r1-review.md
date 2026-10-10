# Review Report — Milestone M2 Time Base (PPS tone discipline, leap seconds, CUC framing)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0007                      |
| Reviewer session                  | independent agent (fresh context, norm N9) |
| Reviewer profile                  | R1 (SW developer, real-time / space / ECSS class A/B experience) |
| Reference revision                | working tree on top of `a69732f` (base commit); milestone M2 uncommitted delta |
| Items under review                | `src/core/time/time.h/.c`, `src/core/time/pps.h/.c`, `config/fsw_config.h` (time section), `tests/unit/test_time.c`, `tests/CMakeLists.txt`, `CMakeLists.txt`, `docs/architecture/time-and-timecode.md`, `docs/interfaces/ICD.md` (§5), `docs/engineering/driver-development.md` (§3), `verification/lean/Frick/Time.lean`, `docs/requirements/{SSS,SRS,traceability}.yaml/md` |
| Date                              | 2026-10-10                             |
| Scope                             | Correctness/safety of real-time behaviour, conformance to ECSS-E-ST-40C / ECSS-Q-ST-80C and norms N1-N16, defensive programming, portability (RTEMS 6 / MPFS RTPFS160ZT), traceability SRS-REQ-021..024 / SSS-REQ-007/016 / GAP-A7 |
| Independence declaration          | Reviewer had no part in authoring or executing the reviewed items; fresh context; findings based only on repository evidence and normative references |
| Norms applied                     | N1-N16 (see `docs/rules.md`)            |

## 1. Scope

Reviewed (code + related documentation):

- `src/core/time/time.h`, `src/core/time/time.c`: tone signal, source selection,
  leap scheduling, CUC encode/decode, tone diagnostics.
- `src/core/time/pps.h`, `src/core/time/pps.c`: backend vtable and the software
  tone task (period 1000 ms, priority 40).
- `config/fsw_config.h`: `FF_TIME_TONE_NS`, `FF_TIME_CUC_COARSE_OCTETS`,
  `FF_TIME_CUC_FINE_OCTETS`, `FF_TIME_CUC_MAX_LEAP_S`,
  `FF_TIME_SW_TONE_PERIOD_MS`, `FF_TIME_TONE_TASK_PRIORITY`.
- `tests/unit/test_time.c` and the CMake registration (test `time`, pps.c added
  to `FF_CORE_SOURCES`).
- Traceability of the items to SRS-REQ-021..024, SSS-REQ-007/016, GAP-A7, the
  Lean model `Frick.Time`, and the tests.

Verification performed by the reviewer:

- Static analysis of the code and arithmetic (framing, tone/leap arithmetic).
- Normative cross-check of the CUC P-field `0x2E` decomposition against the
  reference given in the task (unsegmented | agency-defined epoch `010` |
  coarse octets-1 = 3 | fine octets = 2) and against CCSDS 301.0-B-4 §3.2.2
  layout as documented in ICD §5 / ARCH-0006 §5.
- `cmake --build build/native` + `ctest`: **5/5 tests PASS** (incl. `time`).
- `python3 tools/scripts/trace.py`: **PASS** (48 requirements, 41 Lean theorems
  checked, all links resolved).
- An empirical probe of the test-suite determinism at the `FF_PalGetUtcNs()`
  live-clock semantics (2×2,000,000 runs, -O2 and -O0) — see finding F1.
- cFS TIME heritage reference: `nasa/cFE` `modules/time/fsw/src/cfe_time_tone.c`
  (Draco) — used for the tone-interval validation and tone-edge-phase
  conventions.

Excluded (not re-executed in this session): `make verify-lean` (`lake` not
present in this environment; recorded as PASS in `docs/vv/verification-results.md`
for the M2 cycle), RTEMS cross-build, simulator smoke. These are recorded
verification evidence claims (norm N6/N15), not independently re-run here.

## 2. References

- Project norms: `docs/rules.md` (N1-N16); enforcement summary.
- Traceability manifest: `docs/requirements/traceability.yaml`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C; ECSS-E-ST-70-41C (PUS-C, service 9);
  CCSDS 133.0-B-2; CCSDS 301.0-B-4 (CUC) — the P-field/T-field layout per the
  normative reference in the review task.
- cFS (NASA core Flight System, Draco): `cfe_time_tone.c` tone-count /
  edge-convention / tone-validity heritage.
- Project docs: `docs/architecture/time-and-timecode.md` (ARCH-0006 rev 0.2),
  `docs/interfaces/ICD.md` (§5), `docs/engineering/driver-development.md` (§3),
  `docs/vv/verification-results.md`, `docs/reviews/review-template.md`.

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| F1 | Minor | `tests/unit/test_time.c`; SRS-REQ-024 (Tst evidence); norms N6/N10 | Two assertions compare a *fine* value across two separate wall-clock reads of a live UTC PAL, so they are timing-dependent. `test_init_and_basic_cuc` asserts `fine == 0` after `FF_TimeInit()` (i.e. that < 15.26 µs elapsed between `FF_PalSetUtcNs` and `FF_TimeGetCuc`); `test_cuc_bytes` asserts `c1 == c0 && f1 == f0` where c0/f0 are read at instant A and the encode re-reads the live clock at instant B. A fine step is 15258.8 ns (`1e9/65536`); any gap above that flips the fine. Empirical probe (2,000,000 runs each): fine≠0 after init in 10/2M (-O2) and 14/2M (-O0); decode≠snapshot in 9/2M (-O2) and 5/2M (-O0); the max measured gap between the two reads is up to ~199 µs (OS preemption), i.e. >13 fine quanta. | `tests/unit/test_time.c:29` (`coarse == 0u && fine == 0u`), `:77-80`; `src/core/pal/pal_posix.c:413-421` (`FF_PalGetUtcNs = mono + offset`, live); probe results in `/tmp/opencode/frick-review/probe*`. | Make the fine assertions deterministic: keep the fixed reference buffer `{0x2E,0,0,0,5,0x40,0x00}->5s,16384` and the coarse-only cross-checks; for the round-trip, compare the decode against the bytes just encoded (single-instant, unconditionally lossless) or drive a *fixed* UTC through encode->set->decode; remove the `fine == 0` live-read assertion or snapshot UTC at a known value first. |
| F2 | Minor (borderline Major for TONE operation) | `src/core/time/time.c` vs `docs/architecture/time-and-timecode.md` §2; SRS-REQ-021/022; norm N8 | Documentation/implementation mismatch plus an undefined tone-edge phase convention. The doc states the first tone only *latches* `g_tone_utc_ns = FF_PalGetUtcNs()` and that only *further* tones add 1 s; the code adds `FF_TIME_TONE_NS` to `g_tone_utc_ns` on **every** accepted tone, including the first (`g_tone_utc_ns = FF_PalGetUtcNs() + 1 s` at the first edge). The tests corroborate the code (`coarse == 1` after the first tone). Consequences of the undocumented +1 s: if the ground correlation (PUS 9,1) is aligned to the PPS edge (the usual "at the next 1PPS the clock reads X" semantics), the TONE-mode CUC runs exactly one second ahead of the correlation with no correction path — `FF_TimeSetUtcNs` only writes the PAL offset (inert while TONE is the selected source, `time.c:53-62` reads `g_tone_utc`). cFS treats this edge convention as a *mission configuration* (`CFE_MISSION_TIME_AT_TONE_WILL_BE` vs `..._WAS`). | `docs/architecture/time-and-timecode.md:71-74` vs `src/core/time/time.c:92-95`; time.c:53-62; `tests/unit/test_time.c:131` (`coarse==1`); cfs `cfe_time_tone.c` (`#if CFE_MISSION_TIME_AT_TONE_WILL_BE NewMET.Seconds++`). | Decide and pin the edge convention in ARCH-0006 (first tone = completion of a second → +1 s, cFS `AT_TONE_WILL_BE` style) or change the code to latch without the +1 s; add a test that pins the correlation-to-epoch mapping at the first tone; document (or define) the S9,1-vs-TONE interaction, including a re-alignment path. Resolve before TONE is used operationally. |
| F3 | Minor | `src/core/time/time.c` tone state; `docs/engineering/driver-development.md` §3; profile area (1) ISR context / real-time | The tone state (`g_tone_valid`, `g_tone_epoch_ns`, `g_tone_utc_ns`, `g_tone_count`, `g_leap_seconds`) is updated by `FF_TimeToneSignal` and read by `FF_TimeGetUtcNs`/`FF_TimeGetCuc` without any protection (seqlock, IPL masking or mutex). A producer/consumer on different tasks (or an ISR, which the driver guide explicitly instructs for the board integrator) can observe torn pairs — e.g. a new epoch with an old `g_tone_utc_ns` or vice-versa — producing CUC/UTC jumps up to ±1 s. cFS solves exactly this with `CFE_TIME_StartReferenceUpdate`/`FinishReferenceUpdate` (seqlock). Additionally, the first-tone latch calls `FF_PalGetUtcNs()` (a clock read) inside the hook, contradicting the guide's own "do the minimum in the ISR" rule. | `src/core/time/time.c:20-26,51-62,87-106,131-142`; `docs/engineering/driver-development.md:107-111`; `src/core/time/pps.c:24-34` (concurrent task path already exists with the SW backend). | Add a seqlock / critical-section guard around tone updates and a consistent snapshot for readers (mirror the cFS reference-update pattern), or explicitly document the single-threaded-startup assumption as a limitation and provide an atomic `FF_TimeSnapshot()` for consumers. Address before wiring the HW PPS ISR (GAP-A7). |
| F4 | Minor | `src/core/time/pps.c` SW backend | Error path leaves inconsistent state: `ff_pps_sw_start` sets `g_sw_running = true` *before* `FF_PalTaskCreate`; if creation fails it returns the error but leaves `g_sw_running == true` and `g_sw_task_id` uninitialized (0). A subsequent `FF_PpsStart` then returns `FF_SUCCESS` (falsely reports running, `pps.c:44-45`) and `FF_PpsStop` calls `FF_PalTaskDelete(0)`, which on the POSIX backend cancels whatever task occupies slot 0 (any task may) instead of reporting an error. | `src/core/time/pps.c:42-50`; `src/core/pal/pal_posix.c:175-184` (`FF_PalTaskDelete(0)` acts on `g_tasks[0]` if `used`); `src/core/pal/pal_posix.c:126-173`. | Set `g_sw_running = true` only after a successful create (or reset it on failure), and guard `FF_PalTaskDelete` with a valid-ID check; add a unit test forcing the create-failure path. |
| F5 | Minor | `config/fsw_config.h` + `src/core/time/time.c` | The CUC octet macros are presented as tunables but are not enforced or fully implemented: `FF_TIME_CUC_LEN` derives from `FF_TIME_CUC_COARSE_OCTETS/FINE_OCTETS`, yet encode/decode hard-code the 4+2-octet layout and the P-field `0x2E` (whose CCSDS bits encode the same 4+2). Changing a macro silently yields either a length/field mismatch or a code whose P-field contradicts its own field sizes. Similarly `FF_TIME_CUC_MAX_LEAP_S` is defined and documented but never referenced — the code hard-codes the -1/0/+1 bound. | `config/fsw_config.h:68-71`; `src/core/time/time.h:70-71`; `src/core/time/time.c:115,166-172,182,188-190`; `docs/architecture/time-and-timecode.md:110,179-182`. | Add compile-time checks (`_Static_assert` coarse==4 && fine==2 && `FF_TIME_CUC_MAX_LEAP_S==1`) or implement the tailoring; otherwise remove the misleading macros. |
| F6 | Minor | `src/core/time/time.c` `FF_TimeToneSignal`; SRS-REQ-022; defensive programming | No tone-plausibility validation is performed: only strict monotonicity is enforced, so any strictly-increasing edge is counted as a full second, regardless of spacing. A spurious extra edge (GPIO bounce / PLIC retrigger on the documented HW backend, GAP-A7) adds one second to the disciplined clock with no error reporting. cFS validates the inter-tone interval against `ToneOverLimit`/`ToneUnderLimit` and marks out-of-window tones invalid instead of counting them. | `src/core/time/time.c:87-98`; cfs `cfe_time_tone.c` (`CFE_TIME_Tone1HzISR` interval validation); `docs/engineering/driver-development.md:103-113` (HW backend = GAP-A7). | Add an inter-tone interval window check (e.g. reject tones closer than a configurable fraction of `FF_TIME_TONE_NS`, count them as tone errors) at the latest when the HW PPS capture is integrated; document the SW backend's tolerance. |
| C1 | Comment | `config/fsw_config.h:74`, `src/core/time/pps.c:48`; SRS-REQ-016 | `FF_TIME_TONE_TASK_PRIORITY` has no effect on the POSIX host backend: the PAL uses the default `SCHED_OTHER` policy, where `pthread_attr_setschedparam` priority is ignored; it is effective only on RTEMS. The 2048-byte stack request is also silently dropped on Linux (return value of `pthread_attr_setstacksize` is ignored) and is unverified on the target (GAP-A4 stack/WCET analysis). | `src/core/pal/pal_posix.c:150-166,154-156`; `docs/engineering/driver-development.md:96-101`. | Document "priority/stack size effective on RTEMS, advisory on host"; verify the SW tone task's actual stack usage on the target as part of GAP-A4, and handle `setstacksize` failure explicitly. |
| C2 | Comment | SRS-REQ-021/023; `tests/unit/test_time.c` | Test coverage note: the CUC non-regression across a tone boundary with non-negative pending leap is discharged only by the Lean theorem `req_srs021_utc_non_regression`, with no unit test exercising the source-switch-plus-tone-boundary scenario; the negative-leap case (the intended skipped second) is not asserted either. `FF_PpsStop` with no backend registered (`FF_NOT_READY`) is also untested. | `docs/architecture/time-and-timecode.md:58-61,114-116`; `tests/unit/test_time.c:101-111,171-180`; traceability.yaml SRS-REQ-021 (`lean` list). | Add deterministic unit tests for leap non-regression at a tone boundary with pending +1/-1 and for the unregistered-backend stop path (no wall-clock reads). |

**Positive findings (verbatim checks that passed):**

- CUC framing (task item a): P-field `0x2E` = `0b0010 1110` → unsegmented (bit7=0),
  agency-defined epoch (bits6-4=`010`), coarse octets-1=3 → 4 octets (bits3-2=`11`),
  fine octets=2 (bits1-0=`10`); T-field = coarse[4] BE + fine[2] BE, `FF_TIME_CUC_LEN`=7;
  decode(encode(x)) lossless; `fine = frac*65536/1e9` fits 16 bits (Lean
  `req_srs024_fine_frac_bounded` also independently proven). Fixed reference
  {0x2E,0,0,0,5,0x40,0x00} → coarse 5, fine 16384 verified.
- Tone discipline (task item b): epoch strictly increases (non-monotonic edges
  rejected), one second per accepted tone, leap applied/cleared exactly at a tone
  boundary via a single pending slot (`g_leap_seconds += g_pending_leap;
  g_pending_leap = 0`) and counted only in the CUC coarse (`utc_ns/1e9 + leap`),
  never injected into the UTC base — no double-count. Lean `req_srs023_*`,
  `req_srs021_utc_non_regression` mirror this.
- PPS backend (task item c): fully static storage, no allocation, bounded delay
  (1000 ms) and bounded work; single-threaded registration (`FF_ALREADY_EXISTS`
  on double register) is safe for the framework's start-up sequence.
- Traceability / norms: `make trace` PASS (48 reqs, 41 theorem links); Lean
  `Frick/Time.lean` present and linked; docs updated in the same delta
  (ARCH-0006 0.2, ICD 0.2, SRS 0.2, control files); English only (N2); no
  dynamic allocation in the reviewed code (N10/SRS-REQ-015).
- Tests were built and executed: **5/5 PASS** including `test_time`; pps.c
  registered in `FF_CORE_SOURCES` (`CMakeLists.txt:47`) and `test_time` in
  `tests/CMakeLists.txt:5,15`.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

The M2 time base is functionally sound on the reviewed criteria: the CUC
framing is correct and lossless, the tone/leap arithmetic meets the stated
discipline rules and is mirrored by Lean, the PPS backend is allocation-free
and bounded, and tests/traceability are registered and pass locally. Six Minor
findings and two Comments require closure (norm N12): two of them — the live-wall-clock
non-determinism of some fine-field assertions (F1) and the first-tone +1 s
edge-convention/doc mismatch with its TONE-vs-ground-correlation 1 s offset
(F2) — should be resolved before the TONE source is used operationally; F3-F6
must be corrected or justified as documented. No framing error, no double-counted
leap and no requirement-defeating defect were found.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (norm N9)  | independent reviewer agent (profile R1) | 2026-10-10 | APPROVED WITH COMMENTS |
| R1 (norm N9) delta | independent reviewer agent (fresh) | 2026-10-10 | APPROVED WITH COMMENTS (all NCs closed; NF-01 resolved) |

> Delta re-review: `docs/reviews/2026-10-10-m2-time-base-r1-delta-review.md` —
> APPROVED WITH COMMENTS (all NCs closed; NC-03's residual ISR-safety comment
> NF-01 resolved by making the tone hook task-context-only in code and docs).

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-01 | Minor | F1 / test_time.c | Fine-field assertions depend on wall-clock elapsed (fine quantum 15.26 µs) | Deterministic tests: fixed reference buffer, anchored round-trip, `later_than()` fresh edges; no cross-read fine/coarse equality | `time` checks=101; 2000 repeats 0 failures (delta R1) | frick-fsw | closed | APPROVED (`2026-10-10-m2-time-base-r1-delta-review.md`) |
| NC-02 | Minor | F2 / ARCH + time.c | First-tone +1 s undocumented | Pinned "latched correlation + 1 s" (cFS AT_TONE_WILL_BE) in ARCH §2 and SRS-REQ-022; test `coarse==1` | ARCH §2; SRS-REQ-022; `test_time.c` | frick-fsw | closed | APPROVED (comment NF-02) |
| NC-03 | Minor | F3 / time.c tone state | Torn reads; ISR clock read | PAL mutex `g_time_mtx`; tone hook documented **task-context only**, capture ISR defers to a task (delta NF-01) | `time.c:34,43-55,76-81`; driver guide §3; ARCH §1.1/§3 | frick-fsw | closed | APPROVED (NF-01 resolved) |
| NC-04 | Minor | F4 / pps.c SW start/stop | Create-failure inconsistent state | `pps.c` resets `g_sw_running=false` and `g_sw_task_id=FF_INVALID_ID` on failure; delete id-guarded | `pps.c:49-58`; `pal_posix.c` | frick-fsw | closed | APPROVED (comment NF-04: forced-failure test accepted as not-added) |
| NC-05 | Minor | F5 / fsw_config.h + time.c | CUC macro enforcement | `_Static_assert` 4/2 octets + single-second leap; `FF_TIME_CUC_MAX_LEAP_S` used | `time.c:30-32,188-189` | frick-fsw | closed | APPROVED |
| NC-06 | Minor | F6 / time.c ToneSignal | No tone-spacing check | Dispositioned to HW front-end; GAP-A7 states the edge-qualification criterion | ARCH §3; `class-a-gaps.md` GAP-A7 | frick-fsw | closed | APPROVED (comment NF-03) |
| NC-07 | Comment | C1 / priority & stack | Host vs RTEMS semantics unstated | Documented host-advisory / RTEMS-effective; GAP-A4 | ARCH §3 | frick-fsw | closed | APPROVED |
| NC-08 | Comment | C2 / test coverage | Boundary / unregistered-stop untested | Added deterministic tests (non-regression, skipped second, unregistered stop) | `test_time.c:228-278` | frick-fsw | closed | APPROVED |

---

Committed and pushed with the reviewed change (norm N11).