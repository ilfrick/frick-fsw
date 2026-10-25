# Handover / Current Context (resume point)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-HANDOVER-001                     |
| Revision                         | 3.54                                   |
| Date                              | 2026-10-24                            |
| Norms                             | N8 (entry point), N16 (history)        |

This document is the **authoritative resume point** for an agent picking up the
work without prior conversation context. It summarises objectives, rules, how to
build/test/verify, milestones and the immediate next steps. It is updated on
every significant increment (norm N8/N16).

## NC-EXT-18 S6 refusal-cause addendum — 2026-10-24 (S8-PUS-FU)

**Status: bounded software-only increment ACCEPTED by FRICK-REVIEW-0437 and its delta
review; in-tree only, not committed.**

- What changed: `svc6_dump()` resolved the region and the range in one loop and returned
  a single `FF_ERROR` for both an unregistered region identifier and an out-of-region
  `offset`+`length`, so the two refusals were indistinguishable to any caller. Region
  resolution is now separated: `FF_NOT_FOUND` for an unknown identifier,
  `FF_INVALID_ARGUMENT` for an out-of-range request within a known region, and
  `FF_BAD_SIZE` unchanged for malformed requests. Files: `src/pus/pus_services.c`
  (`05508611…8dcebb`), `tests/unit/test_pus.c` (`ac688974…4f1d71`),
  `docs/interfaces/ICD.md` rev 1.6 (`67ae7584…52643ad`),
  `docs/adr/ADR-0020…` rev 0.2 (`b7e33e1a…0bcb29`).
- Reproduced before fixing: the extended test suite failed 10 of 134 checks against the
  pre-fix handler (`49bc86db…`), and the *old* suite had encoded the defect by asserting
  `FF_ERROR` for both causes.
- Test oracle strengthened after review: the region is now filled with an
  index-dependent pattern, so the content oracle discriminates; the offset-dropping
  mutant that previously survived `134/0` now fails 3 checks.
- Gates: `make build` PASS; `make test` PASS 46/46 (`test_pus` 134/0); `make
  format-check` PASS in `frick-vv:local`; `make trace` PASS (120/76/30/14/68); `make
  verify-lean` PASS; `make gsw-selfcheck` PASS. **N18 Renode FAIL (NC-EXT-198), NOT
  RUN. NC-PAL-01 OPEN.**
- Evidence: `docs/vv/evidence/2026-10-24-ncext18-s6-refusal-causes-evidence.md` rev 0.2.
  Reviews: `docs/reviews/2026-10-24-ncext18-s6-vv-review.md` (FRICK-REVIEW-0437) and
  `…-vv-delta-review.md`.
- **Still authority-owned (not done here):** separating the refusal causes **on the
  wire** requires the PUS-C failure-code allocation; the `TC[6,5]` field widths/profile
  and the memory-access authorization model remain provisional (ADR-0020). New NCs
  NC-EXT-214..219 registered; NC-EXT-18 remains OPEN.

## NC-EXT-207 S3 periodic due-test wrap/TM-flood addendum — 2026-10-25 (S8-PUS-FU)

**Status: software-separable correction IMPLEMENTED and independently reviewed; in-tree only, not committed.**

- What changed: the periodic due test was replaced by a wrap-safe signed-modular
  formulation (`time_period_due`): `u = now_coarse - last_coarse` (mod 2^32);
  `u >= 2^31` → wait (undecided band); `u > period` → due; `u < period` → not
  due; `u == period` → due iff `now_fine >= last_fine` (sub-second tie-break
  preserved). Files: `src/pus/pus_services.c` (`915b62e7…31e6de3`),
  `tests/unit/test_pus.c` (`c42b1250…041b61`).
- Reproduced before fixing: pre-fix `time_ge`/`time_add_period` produced
  premature emission + re-emit burst at epoch wrap (R-1) and forced-wrap TM flood
  from a single `period_s >= 2^32 - now` command (R-2). Extended test suite
  failed 10 of 225 checks against pre-fix source.
- Tests added: three new tests (`test_hk_coarse_wrap_due`,
  `test_hk_large_period_held_unserved`, `test_hk_backward_step_no_emission`)
  covering nominal, wrap-straddle, forced-wrap-flood, backstep, and
  undecided-band classes with exact-count oracles.
- Mutation discrimination: three critical mutants (wait-band removal, tie-break
  `>=`→`>`, `u>period`→`u>=period`) built and **killed** at expected sites.
- Gates: `make build` PASS; `make test` PASS 46/46 (`test_pus` 225/0);
  `make format-check` PASS; `make trace` PASS (120/76/30/14/68); `make
  verify-lean` PASS; `make gsw-selfcheck` PASS. **N18 Renode FAIL (NC-EXT-198),
  NOT RUN. NC-PAL-01 OPEN.**
- Evidence: `docs/vv/evidence/2026-10-25-ncext207-wrap-due-evidence.md`.
  Reviews: `docs/reviews/2026-10-25-ncext207-wrap-due-vv-review.md` (V&V
  exact-candidate), `…-pa-review.md` (PA exact-candidate) — both ACCEPT narrow
  claim.
- **Still authority-owned (not done here):** `u >= 2^31` undecided band causes
  `period_s >= 2^31` reports to hold unserved — formulation property, NOT a
  validation/limit. ESC-HK-06 must dispose max-period/undecided-band policy
  before activation; ESC-HK-03 must add held-unserved hazard; NC-EXT-208
  (period validation) untouched. No caller added (ESC-HK-02 pending).

## HK periodic-emission addendum — 2026-10-24 (S8-PUS-FU, NC-EXT-17)

**Status: bounded increment accepted narrowly by both final delta reviews; NOT committed,
NOT activated, NOT release-ready.**

- What changed: PUS service 3 housekeeping reports gained an optional periodic period
  (`period_s` in the create payload), a per-report last-emission timestamp, and a new
  `FF_PusEmitPeriodicHk()` entry point that emits every enabled report whose period has
  elapsed. Files: `src/pus/pus_services.c` (SHA-256 `49bc86db…6cc3cf`),
  `src/pus/pus.h` (`e520bb33…7345fd`), `tests/unit/test_pus.c` (`57db92de…eb5789`),
  plus payload-width alignment in `test_pus_fu.c`, `test_r2_gap.c`, `test_r3_gap.c`.
- Defect found and fixed during acceptance: `time_add_period()` double-counted the
  period (a 10 s period behaved as 20 s — a sustained 50 % HK sample-rate loss). A
  positive time-due test reproduced it; the fix removes the duplicate coarse addition.
  Evidence: `docs/vv/evidence/2026-10-24-hk-periodic-ncext17-evidence.md`.
- Gates on the final candidate: `make build` PASS; `make test` PASS 46/46
  (`test_pus` 108 checks / `test_pus_fu` 159 / `test_r2_gap` 615 / `test_r3_gap` 201,
  0 failures); `make format-check` PASS in the pinned image; `make trace` PASS
  (120/76/30/14/68); `make verify-lean` PASS; `make gsw-selfcheck` PASS. **N18 Renode
  remains FAIL (NC-EXT-198) and NC-PAL-01 remains OPEN — neither was re-run.**
- Reviews retained (5): `2026-10-24-hk-periodic-vv-review.md`,
  `…-safety-review.md`, `…-vv-delta-review.md`, `…-safety-delta-review.md`,
  `…-vv-final-delta-review.md`, `…-pa-final-delta-review.md`.
- **Do not activate.** `FF_PusEmitPeriodicHk()` has no caller (ESC-HK-02). The
  `time_ge()` comparison is not wrap-exact and the wrap is forceable with a large
  `period_s` (NC-EXT-207, activation blocker); `period_s` is unvalidated (NC-EXT-208).
  The 6-octet `TC[3,1]` payload is an authority-pending private extension against
  ICD §4's 2 octets (NC-EXT-17 / ESC-HK-01) — **no ECSS compliance is claimed**.
- New NCs registered in `docs/planning/sprint-plan.md`: NC-EXT-207..213. New
  escalations in `docs/status/escalations.md` rev 1.7: ESC-HK-01..06, all
  `AWAITING OWNER`.

## Current continuation addendum — 2026-10-21 (owner-directed delta)

- Owner decisions 1317/1318 are recorded in `docs/status/escalations.md` and
  ADR-0021. The source delta threads an absolute monotonic 250 ms deadline
  through panic console/PUS/UART paths and propagates watchdog arm/feed errors;
  this is a design target with partial host evidence, **not** an end-to-end
  verified bound or target WCET.
- Fresh-context R1/R2/R3/R4 draft reports 0409–0412 are retained in
  `docs/reviews/`; see the index. R1/R3 identify unresolved requirement/source
  mismatches on overlong panic text, exact-one report semantics, and formatting
  bound.
- Feed cadence is deliberately **not** changed: `FF_AppWdMain` still uses
  relative delay. Choosing absolute phase/margin now would silently select
  feed behavior without an established earliest valid MSS refresh time or
  approved execution/jitter margin. Public Microchip HAL header is only an
  audit lead; exact RTPFS160ZT datasheet, selected instance/configuration,
  clock/reset dependencies, RTEMS binding and target/HIL evidence remain open.
  See ADR-0021 and TS-5 `N22-TS5-82/83`.
- The earlier Renode command pulled the absent pinned image; this side effect is
  retained in prior evidence. Subsequent inspections used `--pull=never`; do
  not repeat a pull. Host checks are not target WCET evidence.

## Post-review remediation addendum — 2026-10-21 (same increment, bounded)

**Changed files (all uncommitted):**

| Area | Files | Change |
| --- | --- | --- |
| PAL host-only seam | `src/core/pal/pal.h`, `src/core/pal/pal_posix.c` | Reset-hook status injection added to the existing `FF_TEST_HOOKS` fault-injection API (host test builds only; no flight hook, no degraded mode) |
| Link backend | `src/transport/link/link_loopback.c` | **Functional defect corrected:** the loopback backend had no deadline-aware writer, so the production reporter `FF_LinkSendUntil` silently dropped the pre-reset report on the host simulator. It now implements `send_until` and honours an already-expired deadline. The target UART backend was unaffected |
| Link/console writers | `src/core/pal/pal_posix.c`, `src/transport/link/link_uart.c` | **Flight-target link defect corrected:** the owner-directed delta had introduced `poll()` into these RTEMS-compiled sources, and RTEMS 6.2 provides the `poll.h` types but no `poll()` implementation, so the flight target did not link (reproduced: the project cross-build failed with `undefined reference to 'poll'`). The bounded console and UART writers now use a non-blocking descriptor with a short back-off and a retry against the shared deadline, and restore the descriptor mode afterwards |
| Link backend | `src/transport/link/link_loopback.c` | Deadline re-checked after the copy (not only on entry); `FF_LinkLoopbackPop` validates its output pointers |
| Tests | `tests/unit/test_pal_watchdog.c`, `tests/unit/test_es_hs_lifecycle.c`, `tests/unit/test_link_uart.c`, `tests/CMakeLists.txt` | External link-time harnesses only, now guarded by the same condition as the linker option: `nanosleep` wrapper (EINTR retry, non-EINTR EIO) for the PAL wait; `write` wrapper forcing deterministic short writes on the UART slave descriptor. New cases: failed-feed expiry/count preservation, reset-hook error, saturated-stderr whole-path shared-deadline oracle (notifier observes the expired deadline and the pre-reset reset counter), production reporter wiring with an exactly-one-frame assertion, link deadline-writer semantics, UART partial-progress/stall with a bounded drain, exact 96/97/256-character panic-reason counterexample |
| V&V records | `docs/vv/test-spec/ts-5-core-services.md` (rev 1.12), `docs/vv/tool-qualification.md` (rev 0.3.94), `docs/vv/verification-results.md` (rev 3.04) | Cases `N22-TS5-84/85/86/87` added and re-counted (87 total / 48 nominal / 29 OOB / 10 mixed; 59 covered / 28 gap); §3 now maps every gap row (80, 84, 85, 86, 87 included); requirement-level gaps kept explicit |
| Status/reviews | `docs/status/loop-audit.md` (rev 10.36), `docs/status/status.json`, `docs/planning/sprint-plan.md` (rev 1.99), `docs/reviews/README.md`, `docs/reviews/2026-10-21-s8-fdir-remediation-{r1-0414,r2-0415,pa-0413}.md`, `docs/reviews/2026-10-21-s8-fdir-final-delta-{r1-0416,r2-0417,pa-0418}.md`, `docs/reviews/2026-10-21-s8-fdir-correction-{r2-0420,pa-0419}.md` | Current-state records, the NC register entries NC-EXT-184..192 for the 0409-0419 findings, and the eight fresh delta reviews retained |
| Evidence | `docs/vv/evidence/2026-10-21-s8-fdir-post-review-remediation.log`, `docs/vv/evidence/2026-10-21-s8-fdir-post-review-source.sha256` | Raw transcript of the host build/test/trace runs, the pinned formatter run (version and exit code) and the RTEMS 6.2 cross-build (configure/link exit codes, artifact, symbol check); the source manifest covers all 31 changed files in `src/`, `config/`, `tests/`, `verification/lean/` and `tools/` plus the build control files and verifies OK |

**Fresh build/test and source identity (this tree):** direct
`cmake --build build/native -j2` + `ctest --test-dir build/native -j2
--output-on-failure` → **44/44 PASS** (repeated after the final formatting
pass). Executed targeted counts: `test_pal_watchdog` 320, `test_es_hs_lifecycle`
180, `test_link_uart` 72, `test_time` 113, all zero failures. `make trace` PASS
(120 requirements, 68 theorem symbols). Pinned `frick-vv:local make
format-check` PASS (formatter identity and exit code retained in the
transcript). **RTEMS 6.2 flight-target cross-build: configure PASS, link PASS,
`frick_fsw` produced** with the cached `frick-rtems-toolchain:local` image
(`--pull=never`, read-only source mount); the artifact exports no
`poll`/`select` symbol and none of the `FF_TEST_HOOKS` test seams. Source
identity: the sha256 manifest above, covering every changed file in `src/`,
`config/`, `tests/` and `verification/lean/` plus the build control files.
`make build` still times out in dependency/configure preparation; no Lean build
was run in this cycle (the cached V&V image has the `lake` executable but **no**
`/root/.elan/toolchains`; a `lake --version` probe inside a disposable `--rm`
container attempted a toolchain download and was not repeated) — the retained
Lean evidence of 2026-10-21T09:58Z remains valid for the current tree because
its 14 Lean source hashes re-verify against the current files and no Lean
source has changed since, and it is not a Lean run of this cycle; `make smoke`
is still blocked by pre-existing ignored generated `third_party/n7s-obcp`
directories — nothing was removed, refetched or re-permissioned. N18 was not
rerun on this tree, and no target execution, target timing or HIL result is
claimed.

**Review outcomes (final, retained):**

| Review | Profile | Outcome |
| --- | --- | --- |
| FRICK-REVIEW-0409 | R1 | 2 Major: over-length policy, unbounded formatting work |
| FRICK-REVIEW-0410 | R2 | 2 Major + 3 Minor: no end-to-end timing oracle; host ≠ target WCET; narrow error/wait/time coverage |
| FRICK-REVIEW-0411 | R3 | 7 Major: exactly-one applicability, mandatory-text envelope, 250 ms not demonstrated, reset-path liveness, actual inter-feed timing, monotonic/PPS policy, missing refresh-window authority |
| FRICK-REVIEW-0412 | R4 | 5 findings: SRS date, handover watchdog-model claim, historical TS-5 totals (corrections applied) |
| FRICK-REVIEW-0413 | PA / N20 | **NOT APPROVED**: F-01 Blocker (reset-path liveness), F-02 requirement semantics, F-03 target timing, F-04 cadence, F-05 unwired binding, F-06/F-07 evidence currency, F-08 authority record, F-09 scope |
| FRICK-REVIEW-0414 | R1 (delta) | **REJECTED for closure of 0409** — both Majors still open; no production bypass introduced |
| FRICK-REVIEW-0415 | R2 (delta) | Host evidence only; target 250 ms/WCET open; UART retry now observed rather than assumed; record binding flagged |
| FRICK-REVIEW-0416 | R1 (final delta) | **REJECTED for closure of 0409/0414** — both Majors still open; 9 Minor items corrected in the follow-up round (NC-EXT-190) |
| FRICK-REVIEW-0417 | R2 (final delta) | 250 ms target and WCET still undemonstrated; 4 Major and 8 Minor record/provenance findings corrected in the follow-up round |
| FRICK-REVIEW-0418 | PA / N20 (final delta) | **NOT APPROVED** — **B-01 Blocker: the increment had introduced `poll()` into RTEMS-compiled sources, so the flight target did not link** (RTEMS 6.2 implements no `poll()`). Corrected and re-verified by a fresh RTEMS 6.2 cross-build. 5 Major record/evidence findings corrected; the authority set remains open |
| FRICK-REVIEW-0419 | PA / N20 (correction round) | **NOT APPROVED** — the product corrections independently verified; 4 Major and 8 Minor record-currency findings corrected in the follow-up round; the authority set remains open |
| FRICK-REVIEW-0420 | R2 / N22 (correction round) | The previously non-discriminating exactly-one-frame oracle is now shown to fail on a duplicate-emission mutation; descriptor-mode restore, PUS subservice and partial-frame emission are asserted; record corrections applied. Host evidence only |

**Precise external blockers (unchanged, not decided here; registered as
NC-EXT-184..188 and NC-EXT-191..192):**

1. **Report obligation and mandatory-text envelope (requirements + safety).**
   SRS-REQ-074's "exactly one" does not state whether it means attempted,
   generated or successfully written, and does not define the mandatory text,
   the maximum reason length or the over-length behavior. The implementation
   currently suppresses the notifier when formatting is incomplete (96
   characters report, 97 and 256 do not) and the reset is still requested.
2. **Bounded formatting work (design + safety).** `vsnprintf` runs before any
   deadline check and may scan an unbounded caller `%s`; a bounded output
   buffer does not bound the work. The owner-approved 250 ms target therefore
   does not yet cover all pre-reset work.
3. **Controlled watchdog configuration, refresh window and margin (systems +
   avionics).** The 8 s value stays an arbitrary placeholder; the actual
   feed-to-feed interval, the earliest permitted MSS refresh and the approved
   execution/jitter margin are unestablished, so the feeder cadence is
   deliberately unchanged and relative.
4. **Target protection and timing evidence (HIL).** The MPFS binding is
   unwired (NC-PAL-01); no target/HIL watchdog, reset or 250 ms timing evidence
   exists; the Renode models are generic and not RTPFS160ZT evidence.

**Policy options for the owner (recommendation in bold):**

- *Report obligation.* (a) One **attempted** report always, with the mandatory
  fields preserved and the reason text bounded to a defined maximum;
  (b) one report only when the content is complete, otherwise no report and a
  separate diagnostic; (c) one report with a fixed-length truncation marker.
  **Recommendation: (a) with a bounded, explicitly truncated reason and a
  defined maximum length**, because it keeps the ground-visible event
  deterministic while the requirement is still being interpreted; (b) and (c)
  both leave the ground without the reset cause.
- *Over-length handling and formatting bound.* (a) Define a maximum reason
  length and format with a bounded, length-limited formatter so the whole
  attempt stays inside one budget; (b) keep the variadic API and accept that
  the formatting work is unbounded by the deadline; (c) restrict the panic
  reason to a fixed-size, non-variadic payload.
  **Recommendation: (a)**, because it is the only option that makes the
  owner-approved single budget provable without changing the call sites.
- *Feed cadence.* (a) Keep the current relative delay and document the
  resulting inter-feed interval as unverified; (b) switch to an absolute
  monotonic schedule once the refresh window and margin are authorized;
  (c) adopt the 8 s / 2 s placeholder as a flight value.
  **Recommendation: (b) with (a) until the authority inputs exist** — (c) would
  ratify an unjustified number, and (a) alone leaves the SRS-REQ-073
  inter-feed clause unverified.

**Correction round (2026-10-21, after FRICK-REVIEW-0419/0420):** the
record-currency findings are applied — executed counts 320/180/71/113, the
flight-target Blocker attributed to FRICK-REVIEW-0418 (the PA report that
raised it), TS-5 rev 1.12 with the strengthened exactly-one oracle, ADR-0021
rev 1.2, NC-EXT-191/192 registered, the source-identity manifest extended to
the changed V&V scripts, and the cross-build transcript now recording the exact
invocation, exit codes, artifact identity and symbol checks. The corrected
exactly-one oracle was verified by a duplicate-emission mutation: it now fails
on the production wiring path, which the previous assertion did not. The duplicate-emission mutation is retained as evidence in `docs/vv/evidence/2026-10-21-s8-fdir-post-review-remediation.log`; the corrected product was independently reviewed once more in the 2026-10-21 acceptance review. The
descriptor-mode restore, the PUS service/subservice of the pre-reset frame and
the partial-frame emission of a stalled send are now asserted.

**Handover acceptance verification (2026-10-21, FRICK-REVIEW-0421):** the
review chronology was reconciled against the file timestamps. The production
sources are unchanged since the 0419/0420 reviews, but the strengthened
exactly-one-frame oracle and the descriptor-restore/partial-frame assertions
were edited afterwards and had **not** been independently reviewed; a fresh
narrow N20 review (0421) was therefore obtained on exactly those corrections.
It confirms them: the writers are poll-free, deadline-bounded and
mode-restoring with an unconditional reset request; the exactly-one-frame
oracle is shown to fail on a **duplicate-emission mutation of the production
`FF_LinkSendUntil` path**, with a control run proving the pre-correction oracle
did not; the UART observation and restore assertions are real and the drain
thread cannot hang; and the 31-entry manifest verifies 31/31 with exact
coverage while the RTEMS flight link was independently reproduced clean of
`poll`/`select` and of any test seam. The mutation has been re-executed and
**retained** in the evidence log, so the claim is re-verifiable from the tree.
0421's one Major and six Minor record/oracle findings were corrected in the
acceptance round and registered as NC-EXT-193..196. The authority set is
unchanged and open.

**Compact gate result (2026-10-21, gate-verification phase):**

| Gate | Result | Reason / evidence |
| --- | --- | --- |
| `make build` (canonical, isolated `build/acceptance`) | **PASS** | Pinned third-party checkouts clean at their pinned commits; `frick-asn1scc:local` cached; exit 0; checkouts re-verified clean afterwards |
| `make test` (canonical, same isolated dir) | **PASS** | 44/44 CTest; exit 0 |
| `make smoke` | **PASS** | Banner + 7 applications registered; exit 0 (the earlier ignored-generated-directory blocker no longer exists) |
| `make trace` | **PASS** | 120 requirements, 68 theorem symbols |
| pinned `format-check` (`frick-vv:local`) | **PASS** | clang-format 18.1.3, exit 0 |
| RTEMS 6.2 cross-build (`frick-rtems-toolchain:local`, `--pull=never`) | **PASS** | configure/build exit 0; artifact exports no POSIX `poll`/`select` and no `FF_TEST_HOOKS` seam |
| **Lean** (`lake build`) | **PASS** | The project-pinned toolchain `leanprover/lean4:v4.10.0` (Lean 4.10.0, commit `c375e19f6b65`, Release; Lake 5.0.0-c375e19) was downloaded through elan in `frick-vv:local` into the **dedicated** cache `~/.cache/frick-fsw/elan` (ELAN_HOME; no global default change, host ownership of the new files restored). The prescribed check `cd verification/lean && lake build` (Makefile `verify-lean`) was executed in the controlled image and reported `Build completed successfully.` with exit 0 |
| **N18 Renode** (`make renode`, corrected runner) | **FAIL** | Image downloaded and digest-verified against the prescribed pin (`antmicro/renode@sha256:7905c32…`, 908 MB); current-tree ELF rebuilt (`build/renode/frick_fsw_renode.elf`, sha256 `23a4c8da…`, 4 114 616 B). The run **exits with the RTEMS fatal error before the test application can emit its marker**: the RTEMS feeder panics at start-up (`[FF-PANIC] watchdog: init failed` → `watchdog: reset requested but MPFS binding NOT WIRED - NC-PAL-01` → `*** EXIT STATUS NOT ZERO ***`, RTEMS fatal exit 5), because the owner-approved arm-error propagation treats the unwired MPFS binding's `FF_NOT_IMPLEMENTED` as a fatal arm failure. The marker is absent from this run's process log **and** UART console → **FAIL, not PASS and not SKIP**. Root cause registered as **NC-EXT-198** (owner decision). The runner's own `PASS` verdict was a **false PASS from a stale 2026-10-20 log** (its fixed `/tmp` log paths are not writable here) — registered as **NC-EXT-199**, since corrected and retested (fresh per-run log directory, fail-closed verdict unit, mutation-verified regression suite, independent review FRICK-REVIEW-0423); the corrected runner now reports the truthful verdict above |
| Hosted CI (GitHub/Gitea) | **NOT RUN** | Separate gate; `ci_github` remains green from an earlier, unrelated run and is not evidence for this tree |
| Target / HIL / QEMU | **NOT RUN** | Separate gate; no target execution, timing, watchdog or flight-readiness evidence exists |
| UART drain-stop flag | **OPEN finding** | ThreadSanitizer-confirmed C11 data race; registered **NC-EXT-197** (OPEN/deferred for owner triage); the earlier "cannot hang" wording is superseded |

**Watchdog specification phase — retained summary (2026-10-22):** paths:
`docs/architecture/mpfs-mss-watchdog-integration-spec.md` (FRICK-ARCH-0024
**rev 1.5**), `docs/architecture/n18-watchdog-integration-proposal.md`
(FRICK-ARCH-0023 rev 1.5), the retained reviews
`docs/reviews/2026-10-21-s8-fdir-embedded-safety-0424.md` and
`…-vv-0425.md` (rev 1.0), `docs/reviews/2026-10-22-s8-fdir-embedded-safety-0426.md`
and `…-vv-0427.md` (final delta), `docs/reviews/2026-10-22-s8-fdir-embedded-safety-0428.md`
and `…-vv-0429.md` (the confirmation addenda of rev 1.4), the retained evidence
`docs/vv/evidence/2026-10-22-s8-fdir-wd-spec-bsp-inventory.log`,
`…-wd-model-probe.log` and `…-wd-official-docs-retrieval.log`, the documentation manifest
`docs/vv/evidence/2026-10-22-s8-fdir-wd-spec-docs.sha256` (19 entries; regenerated
after the final text freeze) and the **unchanged** code manifest
`docs/vv/evidence/2026-10-21-s8-fdir-post-review-source.sha256` (34 entries,
re-verified 34/34, not re-derived here). Genuinely unresolved and **for the
owner**: the six grouped input registers **G-A** (controlled device
documentation, lock rule, timeout unit, design-time configuration), **G-B**
(clock source, gating, sleep/low-power timebase), **G-C** (mission hart,
instance, reset coverage, reset scope), **G-D** (mission timeout, reaction
window, cadence/margins, false-reset tolerance, `MSVP`), **G-E** (HIL plan and
thresholds, the watchdog-timeout report path) and **G-F** (documentation
basis) — with the decisions **A** (basis), **B1** (structure/inventory),
**B2** (content), **C** (instance/hart/reset coverage and the boot-lock
contingency), **D** (timing values), **E** (interrupt policy and false-reset
tolerance) and **F** (verification, the N18 gate and the emulator-leg scope),
all OPEN. **B2, D, E and F cannot be taken before the corresponding group
closes; no implementation, register write, production change, policy decision,
NC closure or gate approval has occurred.** Since rev 1.3 the **issuer's own
documents** have been retrieved (MSS TRM rev S, Power-Up/Resets UG rev M, data
sheet rev E, RT product overview rev C, the RTPFS160ZT-applicable errata rev
A, AN4903 — hashes and quoted extracts retained), and they **change the
instance and reset-scope facts**: only `WDOG0` (E51) can reset the MSS, a U54
watchdog can only interrupt the E51, the power-up default timeout is ≈10.47 s,
and the watchdog reset reason is `WDOG_RESET` (bit 5) with an **MSS**-scope
reset. The rev 1.3 instance recommendation is therefore **withdrawn** and
agenda item C is reframed. The **"PolarFire SoC Device Register Map"** and the
RT "MS Description" are **not retrievable** from this environment and remain
the outstanding controlled input M-1.

**Final read-only acceptance checkpoint (2026-10-22):** the frozen rev 1.5
bodies were verified by SHA-256 (spec `9b09605c…`, proposal `749ecafc…`, the
official-document retrieval log `54a44964…`, the model probe log `64139566…`)
and confirmed by two short independent read-only addenda —
**FRICK-REVIEW-0430** (embedded safety) and **FRICK-REVIEW-0431** (V&V), both
scoped to the 1.4→1.5 delta and both stating that they do not extend the
rev 1.4 confirmation addenda. **No design defect was found.** The arithmetic
annotation slip in the retrieval log's *labelled inference* and nine
structural/metadata observations are recorded as **OPEN/deferred**
(NC-EXT-206, and a residual added to NC-EXT-205); the frozen bodies were **not**
modified, per the owner's direction not to refactor for minor findings. The
checkpoint record is retained at
`docs/vv/evidence/2026-10-22-s8-fdir-wd-final-readonly-acceptance.log`.

**Watchdog specification phase (2026-10-22, owner path A — ESC-9/ESC-10):**
the controlled **MPFS/MSS watchdog integration specification** is produced at
`docs/architecture/mpfs-mss-watchdog-integration-spec.md` (FRICK-ARCH-0024
**rev 1.3**) and independently reviewed (**FRICK-REVIEW-0424**/**0425** for rev 1.0,
**FRICK-REVIEW-0426** embedded safety and **FRICK-REVIEW-0427** V&V for the final
delta); all four reports are retained, indexed, and their findings are itemised
against the corrections in the specification's §10. It contains the reference
inventory (public files pinned by upstream commit `5a890a5b…` and SHA-256, not
retained in the tree per the N13 boundary; the installed RTEMS BSP inventory
transcript is retained as evidence and confirms **no MSS register map and no
MSS watchdog driver** in the BSP), the verified facts with their source class,
the recommendations, the **grouped owner input register G-A..G-F**, the corrected
emulator/host/target case sets with a negative control, the target/HIL legs
including the two **NC-PAL-01 closure conditions**, and the **owner decision
agenda A, B1/B2, C, D, E, F — all OPEN**. The owner's rejection of operating without a watchdog
(ESC-10) is applied: the proposal's "capability absent ⇒ non-fatal" variant is
**withdrawn**, and the alternative is keeping the N18 gate blocked and FAIL.
Documentation only — **no binding, no register write, no production change, no
policy decision, no NC or gate closed**; N18 remains FAIL and Lean remains PASS
per the retained evidence, neither re-run.

**Runner correction and integration proposal (2026-10-21):** NC-EXT-199 is
**corrected and retested** — the N18 runner now writes into a fresh per-run
directory, the verdict is taken only from that directory, a missing/unreadable/
empty capture fails closed, the container status/exit code and the bounded-wait
outcome are kept and reported, and a marker-absent run is distinguished as a
target startup fatal exit or a timeout. The regression suite uses only genuine
recorded runs (stale PASS copy + marker-less run → FAIL; invalid captures →
FAIL; the recorded 2026-10-20 success → PASS; the recorded current-tree
startup-fatal run → FAIL) and is mutation-verified; independent review
FRICK-REVIEW-0423 confirmed it. The **actual N18 run on the current tree is
still FAIL** — the target panics at start-up because the owner-approved
arm-error propagation treats the unwired MPFS binding as fatal (NC-EXT-198).
A **bounded integration proposal** (two options, recommendation, evidence,
required owner decisions and explicit limits) is recorded in
`docs/architecture/n18-watchdog-integration-proposal.md` (FRICK-ARCH-0023);
**nothing was implemented**, no policy was decided, and the report-envelope,
250 ms, refresh-window and margin questions stay open.

**Renode/Lean phase (2026-10-21, owner-authorized Telegram 1397):** the
project-pinned Lean toolchain was installed into a dedicated cache and the
prescribed Lean check now **PASSES** on the current tree. The digest-pinned
Renode image was downloaded and its digest verified against the prescribed pin,
the current-tree Renode ELF was rebuilt, and the N18 functional validation
**FAILS**: the target panics at start-up because the owner-approved arm-error
propagation treats the unwired MPFS binding as a fatal arm failure, so the
`RENODE_TEST: PASS` marker is absent from this run's process log and UART
console. That is recorded as **FAIL** (not PASS, not SKIP) and registered as
**NC-EXT-198** for owner decision; the runner's own PASS verdict was a false
PASS from a stale log, registered as **NC-EXT-199**. No watchdog policy,
cadence, binding or requirement wording was changed.

**NC-EXT-197 correction phase (2026-10-21, owner-authorized):** the UART
drain-stop flag is now protected by a POSIX `pthread_mutex_t` in the same stack
object — the project requires C99 (`CMAKE_C_STANDARD 99`, required), so C11
atomics are not used — with the mutex initialized before `pthread_create` and
destroyed after `pthread_join`. The stack-object lifetime, the bounded drain
shutdown and the forced send-failure semantics are unchanged, and no production
source, coverage, expected result or sanitizer setting was touched (the UART
check count went 72 → 74, two assertions added). The **identical** forced-failure
ThreadSanitizer reproducer now reports **zero** races (before: 1), and the
before/after pair, the exact pre-fix source (`sha256 b6227e69…`, retained
verbatim as `docs/vv/evidence/2026-10-21-s8-fdir-uart-drain-stop-prefix.c`) and
the post-fix source (`sha256 e5c85c4a…`) are recorded in
`docs/vv/evidence/2026-10-21-s8-fdir-gate-verification.log` and in the refreshed
31-entry source manifest. Independent review **FRICK-REVIEW-0422** confirms the
race is resolved and that no new defect was introduced; its four residual Low
items are registered for owner triage. **NC-EXT-197 stays OPEN** pending the
owner's disposition; no gate approval is claimed.

**Gate-verification phase (2026-10-21):** the canonical gates were reconciled
against the approved sequence and executed with the existing pinned tools and
isolated output directories; the compact per-gate result is recorded in
`docs/vv/evidence/2026-10-21-s8-fdir-gate-verification.log` and in
`docs/status/loop-audit.md`. The previously recorded `make build` / `make smoke`
blocker no longer exists: the pinned third-party checkouts are clean at their
pinned commits and the `frick-asn1scc:local` image is cached, so `make build`
and `make smoke` now pass. The drain thread's stop flag was shown by
ThreadSanitizer to be a genuine C11 data race (`volatile` is not a
synchronization primitive); it is registered **OPEN/deferred** as NC-EXT-197 and
the earlier "the drain thread cannot hang" wording is superseded.

**Resume point:** the in-scope functional and record defects that the
independent reviews found in this increment are corrected (the flight-target
link defect among them) and the results are re-verified; what remains is
outside the increment's authority. Complete the authorized input for items 1-3
above, then re-run the applicable cycle (Lean with a toolchain, N18 on a
freshly linked ELF, smoke after the third-party checkout is clean, target/HIL)
and re-review the delta. Until then: no NC closure, no gate pass, no
commit/push (N7), and no target WCET, hardware-protection or flight-readiness
claim. The MPFS hardware watchdog binding stays unwired and the 8 s timeout
stays an arbitrary placeholder.


## 0. How to resume in a new session

Start a fresh session in the repository root and use a kickoff prompt like:

> "Read `AGENTS.md`, `docs/rules.md` and `docs/status/handover.md`; then continue
> the frick-fsw roadmap from the handover's §5 next steps, respecting norms N1-N22
> (full cycle + independent review + trace + Renode N18). Do not commit/push
> unless you are `aeronautics-space-fsw-engineer` after all gates and full sprint
> scope pass, or the user explicitly delegated that authority."

Everything needed is in the repo (no external state): objectives/roadmap
(`docs/development-plan.md`, `docs/roadmap.md`), rules (`docs/rules.md`), status
and this handover. Verify the tree is clean (`git status`) and the gates are green
before starting a new increment.

## 1. What the project is

A **cFS-inspired flight-software middleware** (not an application): PUS-C TM/TC on
CCSDS Space Packets, POSIX host simulator + Microchip RT PolarFire SoC
RTPFS160ZT (RISC-V) under **open-source RTEMS 6.2**, target ECSS criticality B.
Users write their own applications; `src/apps/*` are reference/demo apps.

Remotes: `origin` (GitHub: ilfrick/frick-fsw) and `gitea`
(housefz.com/git/ilfrick/frick-fsw). Branch: **main only**. Under N7, only
`aeronautics-space-fsw-engineer` commits/pushes after all gates/norms and full
sprint scope pass; exceptions require explicit user delegation. Credentials in
`/home/nicola/.netrc` (+ `gh` helper).

## 2. Non-derogable rules

`docs/rules.md` (N1-N22). Key gates before every commit/push:

1. `make build` + `make test` green (native).
2. `bash tools/scripts/trace.py docs/requirements/traceability.yaml` PASS.
3. Lean: `lake build` in `verification/lean` (host lacks lake; run in the V&V
   image or skip with note).
4. `clang-format --dry-run --Werror` over `src config tests`.
5. **N18**: functional validation in **Renode** must pass — `make renode`
   (blocking when `build/renode/frick_fsw_renode.elf` exists; per-commit SKIP
   proxy otherwise; nightly does the real run).
6. N9/N12: independent review (R1/R2/R3/R4/PA as applicable) + NC closure; reports
   under `docs/reviews/` and logged in `docs/reviews/README.md`.
7. N8/N16: update all affected docs + revision histories.
8. N17: documentation review (R4).
9. **N20**: independent **Senior SW PA** review of every phase product; findings
   fixed and reviews repeated until pass (`docs/engineering/reviewer-agents/`).

## 3. How to build / test / verify (exact commands)

```bash
# Host build + unit tests (default N7 backend) + trace + format
make pus-asn1
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Debug -DFF_USE_PUS_ASN1=ON && cmake --build build/native
ctest --test-dir build/native --output-on-failure
python3 tools/scripts/trace.py docs/requirements/traceability.yaml   # PASS 117/73/30/14/62
docker run --rm -v "$PWD:/workspace" -w /workspace frick-vv:local \
  bash -lc 'find src config tests -name "*.[ch]" -print0 | xargs -0 -n1 clang-format --dry-run --Werror'
```

The canonical cycle is `make ci-local` in `frick-vv:local` (configure/build/CTest/
Lean/trace; test count varies with the candidate). For the AddressSanitizer leg and coverage/metrics regeneration, run
inside the container (`build/asan`, `make coverage`, `make metrics`) so the
panel and the `ci-local` cache agree; the README panel is green only when
`tools/scripts/metrics.py --check` passes after `make coverage` in the image.

# Renode V&V environment self-test (fast)
make renode-env

# Open RTEMS 6.2 toolchain + RMTS image + Renode functional run
make rtems-toolchain            # build open RTEMS toolchain image (long, ~75 min)
make rtems-docker               # cross compile inside the toolchain image
#   (or, if the toolchain image lacks cmake, install at runtime in the container)
make renode-image               # build build/renode/frick_fsw_renode.elf (FF_ENABLE_RENODE_TEST=ON)
make renode                     # -> "[renode] PASS (RENODE_TEST: PASS)"
```

Toolchain image `frick-rtems-toolchain:local` (open RTEMS 6.2, RSB tag 6.2, BSP
`riscv/mpfs64imafdc`); VM image `frick-vv:local` (cmake/ninja/gcc/lean/…); Renode
image `antmicro/renode@sha256:7905c3…`. Container runs as root → after mounted
edits run `chown -R 1000:1000` (no sudo on host). Host lacks cmake-format/lake on
occasion; prefer Docker.

## 4. Current state (engineering baseline `318f49b`; status-only follow-ups committed)

- **Working-tree increment `S8-FDIR-01` (uncommitted, 2026-10-20):** the first
  **real code change since `318f49b`** — panic/reset-recovery. It addresses the
  outstanding `NC-EXT-98`/`F-0379-03` reset-path work flagged in the previous
  cycle by adding the PAL panic seam. Contents: the PAL hardware-watchdog
  abstraction (`src/core/pal/pal_watchdog.c` + `pal.h` contract; simulated host
  backend in `pal_posix.c`; **deliberately unwired** RTEMS binding in
  `pal_rtems.c` → NC-PAL-01); the reworked `FF_PalPanic` (log → lock-free PUS S5
  pre-reset report → single `FF_PalWatchdogReset` path → test seam → `abort()`);
  the dedicated lowest-priority feeder app `src/apps/app_wd.c`; the now-live
  `watchdog_timeout_s` (30 s → 8 s, owner-authorised, fed every 2 s); new event
  `FF_EVS_PANIC_RESET` (`0x0104`); SRS-REQ-073/074/075 + Lean
  `Frick/Watchdog.lean` (6 theorems); `test_pal_watchdog.c` (298 executed checks) + 5
  reset-path cases in `test_es_hs_lifecycle.c` (109 executed checks); ADR-0021. Gates:
  build/test **44/44**, smoke (7 apps), Lean, trace all PASS. **Not sprint
   complete and not safety-approved:** the MPFS hardware binding is unwired
   (NC-PAL-01); the pinned Renode BeagleV-Fire platform imports the PolarFire SoC
   description and instantiates five generic MPFS watchdog models, but this is
   not evidence of RTPFS160ZT fidelity, RTEMS binding, timing or reset behavior
   (GAP-A5 / TS-5 `N22-TS5-82`); the new
  ground-visible event id is not yet in the ICD event catalogue (NC-EXT-102).
  **Independent reviews (2026-10-21):** R2 V&V `FRICK-REVIEW-0405` and its
  bounded N12 re-review `FRICK-REVIEW-0407` are retained in `docs/reviews/`; the
  R4 documentation review is `FRICK-REVIEW-0408`. R2's Blocker was a **real
  defect** — `Frick/Watchdog.lean` was not in the build graph and did not
  compile, so the six SRS-REQ-073/074/075 theorems had never been elaborated;
  it is corrected (`Frick/Requirements.lean` imports the module; two theorems
  re-expressed with core Lean 4.10 `List` operations) and registered as
  NC-EXT-120. The review findings are NC-EXT-120..183 and remain **OPEN**; the
  R3 (requirements/system) and PA (N20) report legs are **not yet retained**
  (NC-EXT-141), so they are recorded as pending, not satisfied. The N17 R4
  documentation review `FRICK-REVIEW-0386` (2026-10-20) is **REJECTED**
  (1 Major, 4 Minor, 4 Comments); findings are registered as NC-EXT-103..111.
  The Major traceability-mirror correction is in the working tree and `make trace`
   passes, but R4 delta re-review is pending; other findings remain open. Nothing
   is committed until the full
   sprint scope and all applicable gates close (N7).
   **Later host-gate reconciliation (2026-10-20):** the ordered host sweep is
   2 PASS / 5 FAIL; `make trace` and GSW/license pass, while smoke/metrics stop on
   Git ownership, host Lean/format tools are absent, and host Doxygen cleanup is
   denied. The separate pinned T24 clang-format dry-run passes in `frick-vv:local`.
   Evidence and open/deferred environmental NC-EXT-112 are recorded in sprint-plan
   rev 1.77 and verification-results rev 2.96; NC-EXT-35 carries the Lean image
    observation. N15 remains incomplete; no gate or NC is closed.
   **R4 delta review record:** `FRICK-REVIEW-0387` found four Minor evidence/status
   findings. They are tracked under existing NC-EXT-28/35/112 and new NC-EXT-113;
   all remain OPEN/deferred. The review report is
   `docs/reviews/2026-10-20-s8-fdir-host-gates-r4-delta-0387.md`. The report does
    not correct the §5 resume wording, close NCs/gates, or grant approval; at
    that review point N15 and the independent R2/R3/PA, Renode N18, target and HIL
    gates remained open. The current N18 follow-up below records the later bounded
    Renode PASS.
    **N18 status for this tree (2026-10-21):** the 2026-10-20 cross-build and
   Renode PASS describe the 2026-10-20 tree state; after the 2026-10-21
   corrections to `src/apps/app_wd.c` and `tests/unit/test_pal_watchdog.c` they
   have **not** been re-run, so N18 is **pending for the current tree**
   (NC-EXT-165) and no PASS is claimed for it.
   **N18 follow-up (2026-10-20 — SUPERSEDED for the current tree by NC-EXT-165):** the existing local RTEMS 6.2 toolchain image
    was used for a fresh cross-build in a new temporary build tree; `make renode`
    returned PASS on the newly linked ELF, with source/artifact identities and
    full logs retained in `docs/vv/evidence/2026-10-20-n18-*`. This is emulator
    evidence only. NC-TS-01 remains OPEN because `renode_vv.sh` still gates on ELF
    existence rather than freshness; NC-EXT-40 provenance limits, N15, independent
     reviews, hardware watchdog and target/HIL gates remain open. See verification
        results rev 3.00 and loop-audit rev 10.29. Independent R2 review
      FRICK-REVIEW-0390 is APPROVED WITH COMMENTS (two Minor findings): the
      console and process UART captures disagree (NC-EXT-115 OPEN), and the SB
      receive-success check lacks expected-message-field comparison (mapped to
      existing NC-EXT-47, OPEN). The process log contains the WD-enabled startup
      and PASS marker; the inconsistent console capture is not attributed to the
      same run. N18 prescribed execution remains PASS, but gate acceptance remains
      open pending applicable review and evidence disposition. No repeat run was
      made. The N18 R2 report is indexed in `docs/reviews/README.md`.
      The bounded R4/PA review of the resulting documentation delta (FRICK-REVIEW-
      0391/0392) found two Minor currentness issues in the sprint plan: the NC-TS-01
      result pointer was stale and the sequence note ended at 113. Corrections are
      recorded in sprint-plan rev 1.83; NC-EXT-116/117 remain OPEN pending bounded
       R4/PA confirmation. R4 delta re-review FRICK-REVIEW-0393 confirmed both
       fixes but found NC-EXT-116/117 dispositions still instructing the owner to
       make already-applied corrections. The wording is corrected in sprint-plan
       rev 1.84 and tracked as NC-EXT-118 OPEN pending bounded R4/PA confirmation;
        all three NCs remain OPEN. R4 FRICK-REVIEW-0394 then identified stale
        active N18 pointers in the sprint-plan S8-FDIR-01 row and this handover
         (§4/§5); at that point they pointed to verification-results rev 3.00
         and loop-audit rev 10.22 in sprint-plan rev 1.85 / handover rev 3.27.
         Later R4/PA reviews corrected the current handover pointers to
         verification-results rev 3.00 and loop-audit rev 10.27 in handover rev
         3.31. PA FRICK-REVIEW-0402 identified the earlier attribution as
         inaccurate; this historical record now distinguishes the point-in-time
         references from the later correction (NC-EXT-119 OPEN).
         These current-pointer issues are mapped to
         NC-EXT-116 OPEN pending bounded R4/PA confirmation. R4 FRICK-REVIEW-0395
         then found the §5 current N18 pointer still at rev 2.99; it now points to
         rev 3.00 in this rev 3.28. All current-pointer observations map to
        NC-EXT-116 OPEN pending bounded R4/PA confirmation. R4/PA reviews
        FRICK-REVIEW-0396/0397 then found the NC-EXT-116 disposition still
        referenced handover rev 3.27; it now correctly references rev 3.28 in
         sprint-plan rev 1.87. Bounded R4/PA reviews FRICK-REVIEW-0398/0399
         confirmed that correction. NC-EXT-116 remains OPEN pending authorized
          NC-owner disposition. R4/PA FRICK-REVIEW-0400/0401 identified the
           stale audit and verification-results pointers in this current-status
           section and §5; the active pointers now use audit rev 10.29 and results
           rev 3.00. PA FRICK-REVIEW-0402 found a separate historical-attribution
           discrepancy, corrected above and registered as NC-EXT-119 OPEN pending
           bounded R4/PA confirmation. FRICK-REVIEW-0403/0404 confirmed the
           bounded NC-EXT-119 history correction; NC-EXT-119 remains OPEN pending
           authorized NC-owner disposition. The documentation
         review cycle is not complete.
     PA review also identified an unsupported on-target PCIe enumeration/DMA claim
     in `docs/vv/renode-environment.md` §5 that conflicts with the current absence
     of physical RTPFS160ZT evidence. It is tracked OPEN as NC-EXT-114; no target
     evidence is inferred and it is unrelated to the bounded N18 PASS.

- **Engineering baseline:** the latest **engineering** baseline is `318f49b`
  (2026-10-19 easy-fixes batch), on this chain: `b8c3aee` → `1fdbb83` (S8 reconciliation batch:
  S8-SCH + S8-VV-FU + S8-PUS-FU + selected N7 OBCP integration) → `2111968`
  (documentation alignment + NC-EXT-94/95/96) → `5be0e8c` (metrics refresh) →
  `e7e48ee` (bounded PA manifest review) → `318f49b` (easy-fixes batch), pushed to
  `origin` and `gitea`. S8-SCH and S8-VV-FU are delivered on the pushed history.
  **Baseline convention:** a *status-only* commit that changes no code, test,
  requirement or retained-evidence content does **not** advance the engineering
  baseline — e.g. `b6e1f61` (post-commit status refresh) sits on top of `318f49b`
  without advancing it. This keeps the recorded baseline stable across bookkeeping
  commits instead of chasing each commit's own hash.
  The easy-fixes batch cycle evidence is produced on the pre-commit working tree,
  so it must not be conflated with prior cycle evidence. Candidate verification and
  generated-input identity are recorded in `docs/vv/verification-results.md`, and
  the batch delta identity in the addendum of
  `docs/vv/evidence/2026-10-17-s8-reconciliation-candidate-manifest.md`.
- **2026-10-19 easy-fixes batch (committed as `318f49b`, pushed to both remotes):**
  corrected the masked `N22-TS6-57` minimum-PDL sub-case in `test_decoder_corpus.c`
  (a real test defect: the old vector was rejected by an earlier guard, so the
  minimum-length rule was never exercised — the discriminating mutant now kills
  it), added template/iteration context to the corpus anomaly report, added an
  enforced CTest wall-clock budget (`TIMEOUT 60`) to the two S8-VV-FU targets, and
  aligned the TS-5/TS-6 N22 rows with what the oracles actually assert. It also
  registered NC-EXT-97 (requirements-authority contradiction), NC-EXT-98
  (residual S8-VV-FU test evidence) and **NC-EXT-99** (this batch's own N17/N20 and
  V&V-confirmation reviews deferred by explicit owner disposition to the next
  cycle), corrected the Service-1 subservice claim (emitted set `{1,2,3,5,7,8}`:
  subservice 4 is mapped but unreachable and 6 is unmapped — NC-EXT-94 widened), and
  resolved a review-ID collision by renumbering the S8-VV-FU V&V delta review to
  `FRICK-REVIEW-0379/0380`.
  **Resume point (post-publication reviews available):** the batch-scoped reviews
   deferred under NC-EXT-99 were performed in cycle 2026-10-19 at HEAD 7e93325 -
   FRICK-REVIEW-0381 (R4), 0382 (R2/R3) and 0383 (PA), all APPROVED WITH COMMENTS
   with 0 Major findings (reports under `docs/reviews/2026-10-19-*.md`; reconciliation
   in `docs/vv/verification-results.md` rev 2.92). The bounded corrections they
   implied were applied to this tree and are marked "delta re-review pending" (N12);
   authority-owned items were disposition-registered (ICD-vs-README divergence
   appended to NC-EXT-94; the SRS-REQ-018/050 contradiction stays under NC-EXT-97).
   NC-EXT-99 therefore stays OPEN pending N12 confirmation, and NC-EXT-100/101 were
   registered for the remaining process-level items. The outstanding work below is
   otherwise unchanged in kind: close NC-EXT-98 / F-0379-03 (the reset path) by
   adding the PAL panic seam/stub that `tests/` currently lacks, then re-run the
   0370..0373 delta re-reviews and the 0381/0382/0383 delta re-confirmations.
- **S8-SCH delivered:** `ff_sched` table-driven dispatcher (`ff_sched.{c,h}`,
  `app_scheduler.c`) with validation, atomic double-buffer activation, absolute
  per-entry release targets, bounded per-poll work, missed-slot/heartbeat/
  publish-error stats; `test_sched` 57 checks. NC-EXT-19 functionally corrected;
  remains OPEN pending independent R4/PA review.
- **S8-VV-FU implemented/committed:** `test_es_hs_lifecycle.c` (ES/HS lifecycle +
  FAILED-application detection host integration with an EVS-listener oracle) and
  `test_decoder_corpus.c` (bounded random-byte/length-boundary/single-bit-
  mutation/cross-type/round-trip/`FF_PusIngest` corpus with requirement-grounded
  oracles). GAP-R3-1 and NC-EXT-25 materially closed and now independently
  reviewed (`FRICK-REVIEW-0379/0380`, APPROVED WITH COMMENTS, no Major findings);
  both **remain OPEN** — the residual is the review's `F-0379-03` reset-path case
  (NC-EXT-98), and `NC-EXT-25`'s external/reference-oracle independence is not
  discharged. Closure is an owner disposition (§3.2 is tracking only).
- **Batch reviews 0341–0343 completed; delta closure is in progress.** They
  covered S8-SCH + S8-VV-FU + S8-PUS-FU + selected N7 OBCP integration. Remaining
  controlled-profile/interface work and focused delta reviews mean the sprint is
  not complete. The pinned N7 engine is integrated into the
  default core build with host callback-connectivity evidence, but production
  mission service/PUS S18 binding and M12 completion are not claimed. NC-S9-01
  remains deferred/open. See `docs/architecture/obcp-integration-assessment.md`.
 - **Review batch 0338–0340:** R4 found a stale current-cycle count; PA identified
   candidate-identity and generated-source provenance gaps; V&V found missing
   OBCP READY and post-final-assertion oracles. The test/trace and candidate
   product-set identity findings were addressed in subsequent bounded reviews.
   NC-EXT-84 remains open for generated-source/tool provenance.
 - **Full S8 batch reviews 0341–0347:** R4 identified status.json, current-cycle,
  and S8-SCH plan-status reconciliation NCs; PA reaffirmed NC-EXT-83/84; V&V
  found missing S20 zero/exact-capacity oracles and an overstatement that the
  decoder corpus CRC check was independent. The S20 cases and CRC wording are
  corrected in the working tree. Post-0343 CI-local and applicable auxiliary
  verification legs have passed; see the candidate manifest and evidence logs.
   PA 0345 accepts product-set identity for the manifest's 23 enumerated files
   and local results only (NC-EXT-83 narrow closure). PA 0346 verifies the 221-file
   generated tree identity but not tool provenance; NC-EXT-84 remains open.
   R4 0347 closes NC-EXT-85 only for current-status reconciliation. No phase or
   release approval follows.
 - **S8-PUS-FU R1/R2 findings 0349/0350:** R1 found S3 report-all could replace
  an earlier failure with a later `FF_NOT_FOUND`; code now retains the first
  failure and TS1-102 exercises both error orders. R2 identified missing exact
  capacity+1, HK encoded-length, duplicate-retention, and SRS-REQ-050 trace
  evidence; bounded test/spec/trace corrections are in the working tree. The
   focused `pus_fu` test passes after retained compile/oracle failures. R1/R2
   delta re-reviews confirm the bounded fixes; PA considers its delta acceptable
   conditionally on configuration-matched full-cycle evidence. Current CI-local
   and auxiliary legs pass; PA 0361 verified listed-file identity only. NC-EXT-
   88/89 remain OPEN pending formal dispositions. R4 0357–0359 and 0362–0363
   reviewed the scoped TS1/status/audit/manifest corrections; no aggregate N17
   approval follows. NC-EXT-14/15/16/18 still need controlled inputs.
 - **S8-SCH review delta:** R2 scheduler review found mismatched missed-slot
  inputs and insufficient rejected-image/output oracles. Tests/specs now align
  on a 1000 ms period, assert exact validation statuses, dispatched old-image
  first-entry output and missed-slot message fields. Focused `sched` passed 1/1;
  R2 re-review 0353 accepts these bounded oracles after narrowing the claim to
  the tested entry and fixing duplicate TS6 history numbering. R1 identified
  unchecked `uint64_t` release-target overflow; NC-EXT-90 records the open time
  domain/exhaustion policy. NC-EXT-19 remains OPEN for independent review,
  mission inputs, target timing, and authority decisions.
- **Latest local cycle legs:** final `make ci-local` passed 43/43 + Lean/trace
  after correcting old S3 test vectors and formatting (verification-results
  2.89; latest post-loop-audit log hash retained there). Smoke, GSW/license, pinned format and
  format-check passed. Containerized docs generation, Renode PASS and RTEMS 6.2
  cross-build passed; Doxygen emitted unresolved-reference/list/entity warnings
  while output generation succeeded. PA independently verified the supplemental
  19-file PUS/Scheduler delta identity and cited log/cache/executable hashes;
  NC-EXT-91 is closed narrowly for that delta only. Broader candidate identity,
  provenance and physical-target evidence remain open, and no hosted CI run covers
  that batch commit `1fdbb83` itself (later chain commits do have hosted runs — see
  below).
- **Hosted GitHub CI:** run `39218060325` passed all eight jobs (documentation
  set, Lean, simulator smoke, traceability gate, RTEMS cross build, host
  Debug+Release build/unit tests, static analysis, pipeline summary) for the
  engineering baseline `318f49b`, and run `39219583424` passed all eight jobs for
  the status-only follow-up `b6e1f61`. The earlier recorded run `38793643786`
  passed for `13ebf92`. No hosted CI run has head `1fdbb83` itself (the pushed
  default-N7 S8 batch), though later chain commits are covered.
- **Known limits:** NC-EXT-01/02/03/04/06/07/13/19/25/47/90 and all target/authority/
  phase/safety/security/release gates remain OPEN. Renode is emulation; RTEMS
  cross-build is not target runtime evidence; no physical RTPFS160ZT evidence is
  established (owner-postponed, README-disclosed). No commit/push of the next
  batch increments until the batch review passes and gates clear.
- **Specific sprint inputs still missing:** S8-PUS-FU cannot finish NC-EXT-14/15/16
  (PUS request/verification/subservice oracle) or NC-EXT-18 (S6 address width and
  region-ID contract) by guessing; controlled PUS profile/SRS/ICD and system
  authority dispositions are needed before those implementation decisions.
  Implemented bounded NC corrections remain open pending independent review.
  For OBCP, NC-S9-01 is deferred and is not a stop on unrelated work, but missing
  production bindings and an accepted execution envelope block M12/SRS-REQ-045
  closure and mission use. Batch reviews and product-assurance approval are also
  required before commit under N7/N9/N20.

- **Historical snapshot (2026-10-14):** M1..M7, M8, M10 delivered; M9 in progress; M11(11/13) in progress; M12-M17 planned; S4b delivered. Current M4 S4c/S4d implementation status is given above and in the roadmap.
- **N18 in force**: open RTEMS 6.2 toolchain built and validated; real cross
  build OK; **Renode functional run of the RTEMS image PASS**
  (`RENODE_TEST: PASS`). Boot: U54_1 at entry `0x1000000000`, console `mmuart1`.
- **M10 done (emulated-target verification, OBJ-18):** Renode real run PASS plus a
  **QEMU run** of the generic `riscv/rv64imafdc` RTEMS image
  (`make qemu-image` + `make qemu`; tool T20 `tools/scripts/qemu_vv.sh`; console
  `build/rtems-generic/qemu-console.log`) — banner + applications registered on
  `qemu-system-riscv64 -M virt` (recorded then as 6; **now expected to be 7** after
  the S8-FDIR-01 WD feeder app, so the historical 6-app QEMU/Renode logs are stale
  and must be rebuilt before the count claim is re-made — NC-TS-01). The toolchain
  image (`tools/docker/rtems`, T18) now also builds the `rv64imafdc` BSP and carries
  QEMU.
- **PUS backend decision:** ADR-0019 records the owner clarification that
  `FF_USE_PUS_ASN1=ON` is the default. ADR-0018 remains applicable to lwIP only.
  Current N7 integration uses generated common type-width definitions and the
  N7/ESA CRC wrapper, but delegates packet framing and PUS field coding to the
  reference codec; full service-codec mapping is not claimed.
- **OBCP integration (S9, working tree):** pinned `n7space/n7s-obcp` engine is
   now built/linked by default behind `src/obcp/ff_obcp.{h,c}`; `make flat`
   generates ignored embedded sources at configure time. The connectivity test
   exercises the engine with test callback doubles across datapool, EVS, output,
   step-control, time and packet modules. It does not establish production
   service bindings, PUS S18, target portability, resource bounds or mission
   suitability. Abort is cooperative at script-defined begin-step callbacks;
   arbitrary Python loops can run indefinitely, and the pinned engine's execute
   return does not propagate interpreter errors. NC-S9-01 is DEFERRED/OPEN;
   SRS-REQ-045 is not claimed complete and mission procedures must not be enabled
   from this evidence. Assessment: `docs/architecture/obcp-integration-assessment.md`.
   Default `make ci-local` passes 43/43 CTest + Lean + trace; `make rtems-docker`
   cross-build, rebuilt-image Renode run (`RENODE_TEST: PASS`) and pinned-container
   format-check, docs generation, smoke and GSW self-check pass. Renode does not
   execute an OBCP script. Evidence and limits are recorded in
   `docs/vv/verification-results.md`.
   S11-S12/M16 in-house replacement-engine development is deferred per owner
   direction; keep SRS-REQ-054 as a separately governed open fallback requirement.
- **M11 in progress (Renode HAL coverage, OBJ-19):** **11/13 peripheral-bound**
  classes done — **GPIO** (`ff_gpio_mpfs`, SRS-REQ-047), **SERIAL**
  (`ff_serial_mpfs`, SRS-REQ-048), **MEM** (`ff_mem_static`, SRS-REQ-049),
  **ETH** (Cadence GEM **identification** via the read-only Module ID,
  SRS-REQ-036), **NVM** (`ff_nvm_mpfs`, SRS-REQ-055; eNVM data write/read check),
  **CAN** (`ff_can_mpfs`, SRS-REQ-057; MMIO register round trip), **I2C**
  (`ff_i2c_mpfs`, SRS-REQ-058; MMIO register round trip), **SPI**
  (`ff_spi_mpfs`, SRS-REQ-069; MMIO register round trip) and **BLOCK**
  (`ff_block_mpfs`, SRS-REQ-070; MMIO register round trip), **TSN** (GEM
  native TSN backend on the emulated Cadence GEM, SRS-REQ-038; init + VLAN/PCP +
  802.1AS TSU time) and **PCIe** (MPFS PCIe root complex, SRS-REQ-031/032; ECAM
  config-space round-trip on the emulated Cadence GEM, `hal_pcie=PASS`). Additionally
  a **NET `FF_NetStack_t` contract check** through the reference loopback stack
  (SRS-REQ-039; in-process — **not** peripheral-bound).
  GPIO/SERIAL/MEM/NVM/CAN/I2C/SPI/BLOCK go through their HAL backends; the ETH check is an MMIO
  identification (full GEM driver/DMA path host-tested in `test_gem` + the new `test_lwip_netif` mock-MAC loopback for the TX/RX data path). Renode
  checks: `hal_gpio`/`hal_serial`/`hal_mem`/`hal_eth`/`hal_net`/`hal_nvm`/`hal_can`/`hal_i2c`/`hal_spi`/`hal_block`/`hal_tsn`/`hal_pcie=PASS`; on the **lwIP-enabled image** also `hal_net_lwip=PASS` (lwIP netif bound to the emulated GEM; full GEM DMA round-trip = GAP-A5).
  M11 peripheral classes: NET(packet round-trip = NC-S6-01 residual, mock-MAC host test) and SpaceWire (contract-level, NC-S7-01).
- **Project codename "Eärendil"**: third-party-name (Tolkien) non-affiliation
  disclaimer in `NOTICE`, `README.md`, `docs/quality/licensing.md §6`.
- **M4-residual MSS drivers delivered so far**: GEM Ethernet (+TSN), MSS GPIO,
  MSS MMUART, the static RAM pool/DMA helper, the eNVM NVM backend, the NOR/QSPI
  MTD backend, the MSS CAN, MSS I2C, MSS SPI and MSS MMC/SD (BLOCK) drivers, plus
  PCIe and the SpaceWire contract-level backend (S7). **Restored M4 scope** (S4b/S4c/S4d):
  PUS-C over IP/TSN (SRS-REQ-059/060/061), CAN-FD/CANopen/RS-485 DE
  (SRS-REQ-062/063/064), PUSopen(R) adapter (SRS-REQ-065), fabric contract
  (SRS-REQ-067); M8 gPTP/CoreTSN (SRS-REQ-068) + GEM TSN Qav/Qci/Qbb
  (SRS-REQ-066). **S4b delivered** (see below).
- **S4b delivered (PUS-C over IP/TSN, SRS-REQ-059/060/061):**
  - segmentation/reassembly layer `link_seg.c/h` (2-byte BE length-prefix,
    allocation-free);
  - IP `FF_Link_t` backend `link_ip.c/h` (UDP default + TCP with seg) over
    `FF_NetStack_t`;
  - TSN `FF_Link_t` backend `link_tsn.c/h` (EtherType 0x88F5, callback-driven RX
    ring) over `FF_EthMac_t + FF_TsnBackend_t`.
  - ctest 27/27 green (test_link_ip **20** test functions, test_link_tsn **15**
    test functions, incl. the `test_pus_ip_roundtrip` / `test_pus_tsn_roundtrip`
    PUS-C TM/TC round-trip demonstrations); Lean PASS; smoke + GSW PASS; Renode
    SKIP (host-only code).
  - `-Wall -Wextra -Werror` is enforced on every test target
    (`tests/CMakeLists.txt`, `FF_TEST_TARGETS`) and all `tests/unit/*.c` compile
    clean under it (N9/M9 quality gate). The host IP test drives a mock
    `FF_NetStack_t`; the reference `FF_NetStackLoopback` is a socket pair (it
    models a peer link, not a single-link self-loop), so the UDP/TCP behaviour is
    established at the `FF_NetStack_t` seam. `link.h` declares the new backends.
    Scope: on-target link-level = GAP-A5 (no Renode image change).
  - Delta re-review: R1+R2 (FRICK-REVIEW-0165-D1) + PA (FRICK-REVIEW-0166-D1)
    both APPROVED; all findings from initial REJECTED reviews resolved (2 Major,
    10 Minor, 6 V&V gaps, 4 PA Minor, 3 Comments + NF-1/NF-2/NF-3 closed).
  - **M8** gPTP (802.1AS) and Qci are delivered (S8); fabric CoreTSN is deferred to
    **M14** (SRS-REQ-068). The repository owner authorized MPL-2.0 for original
    project content; S8-FU addresses remaining corrective work. The owner's
    general instruction to resume with the next sprint as planned is interpreted
    here as selecting S8-FU ahead of competing driver-first sequencing; it did
     not explicitly name S8-FU. R4 review 0311 accepted this bounded
     interpretation; NC-EXT-30/65 are closed for this planning wording issue
     only. The later M12/M16/M13/M9/M14/M15 sequence is recorded in
     `docs/planning/sprint-plan.md` rev 1.27, subject to applicable gates.
- Requirements: **117 (73 srs / 30 sss / 14 gap)**; current `make trace`
  run found/checked 62 Lean theorems.
- Recent reviews: FRICK-REVIEW-0040 (real RTEMS+Renode), 0041 (M12/M13 plan),
  0042 (GPIO), 0043 (SERIAL), 0044 (MEM), 0045 (PUS-C ASN.1), 0046 (README/docs),
  0101..0105 (NVM/MTD + board repl), **0106..0119** (M4/M11 MSS CAN: R1/R2/R4
  initial + delta + closure reviews and the PA freeze sign-off), **0120..0128**
  (M10 QEMU closure: R1/R2/R4/PA initial, delta and final closure),
  **0129..0136** (Sprint-3 M4/M11 MSS I2C: R1/R2/R3/R4/PA initial + R1/R2 delta +
  PA delta; PA converted REJECTED -> APPROVED WITH COMMENTS, loop closed) and
  **FRICK-REVIEW-0148..0161** (S6a GEM TSN, S6b-1 lwIP RTEMS port, S6b-2 lwIP
  netif on GEM, S7 PCIe + SpaceWire: all review/closure sets completed and closed),
  **FRICK-REVIEW-0162** (NET follow-up: NC-S6-03 co-implemented + NC-154-01
  closed — APPROVED WITH COMMENTS; all 8 Minor findings corrected) and
  **FRICK-REVIEW-0163** (NET follow-up delta re-review — APPROVED; all
  corrections verified) — all closed (see `docs/reviews/README.md`).
- **FRICK-REVIEW-0159** (FSW cross-check of `docs/engineering/user-integration-manual.md`
  rev 0.6 — REJECTED) and **FRICK-REVIEW-0160** (R4 documentation review of the
  same manual — REJECTED): both reviewers found 1 Blocker + 7 Major + 9 Minor
  findings (34 NCs). The corrective manual review FRICK-REVIEW-0164 is recorded
  as completed in the review log; it is not a pending next step. NCs logged in
  `docs/nc/2026-10-13-integration-manual-fix-nc.md`. **FRICK-REVIEW-0165** (S4b
  R1+R2, initial — REJECTED, 2 Major + 10 Minor + 6 V&V gaps) and
  **FRICK-REVIEW-0166** (S4b PA, initial — REJECTED, same 2 Major + 10 Minor +
  6 V&V gaps + 4 PA Minor) both closed: delta re-reviews **FRICK-REVIEW-0165-D1**
  (APPROVED) and **FRICK-REVIEW-0166-D1** (APPROVED) resolve all findings including
  3 new findings (NF-1/NF-2/NF-3) raised and closed in the delta.

## 5. Historical next-step ledger and current resume actions

> Items 1–15 below preserve prior handover decisions and snapshots; they are
> historical, not current instructions. For present priorities, use item 16 and
> the live state in §4 plus `docs/planning/sprint-plan.md`.

1. The current S4c/S4d/defaults code is committed and pushed; GitHub CI is green
   for `13ebf92`. Gitea Actions status remains unverified, and no release,
   qualification, or flight-readiness conclusion follows from the GitHub run.
2. Keep NC-S4c-01/03/04/05 open as applicable. In particular, the CANopen guard
   has host-test evidence, but needs independent review and integration evidence;
   Renode does not model the CAN-FD fabric peripheral, and no physical-hardware
   result is claimed. PUSopen remains user-supplied; vendor SDK compatibility is
   not established.
3. The owner's instruction to resume with the next sprint as planned is
   interpreted here as selecting **S8-FU**, the first sprint in the proposed
   corrective sequence; the instruction did not explicitly name that sprint.
   Independent R4 review 0311 accepted this bounded interpretation and closed
   NC-EXT-30/65 for this planning comment only; no broader gate is closed.
 4. T23/NC-EXT-82 closure remains narrowly scoped; broader provenance, licensing,
    independent-review, safety/security, authority, and release gates remain
    open. This is not authorization for release, flight, or operational action.
5. S8-FU factors the production receive iteration into
   `FF_AppCmdIngestReceiveOne`, called by `FF_AppCmdIngestMain`; N22-TS1-96 drives
   a maximum-size TC through that exact helper using a controlled link and checks
    the receive capacity and dispatched bytes. V&V review 0316 accepted this
    bounded host evidence; NC-EXT-01/02 remain OPEN pending authorized NC-owner
   recording of that bounded disposition.
   NC-EXT-03 remains open for external ground-segment E2E; no target/hardware
    claim follows from the local Renode run.
6. **Historical 2026-10-15 UART status, superseded by items 8–9:** the first
   host PTY TS2-81 run reported 46 checks and V&V 0319 accepted its enumerated
   PDL behavior. The later R1 correction adds disconnect handling and unique
   test paths (51 checks); R1/R2 and current NC-EXT-07 limits are recorded in
   items 8–9 and `docs/vv/verification-results.md`.
7. R4 review 0323 accepted the bounded TS-2/NC-status documentation corrections;
   PA review 0324 accepted its bounded findings 1/3 corrections. Neither is
   general N17/N20 or phase approval. The candidate source/build/artifact
   identity and retained CTest logs are in
    `docs/vv/evidence/2026-10-16-s8-fu-r1-candidate-manifest.md`; the Oct-15
    manifest is retained as the earlier source snapshot. NC-EXT-29 remains
   open because this is not complete reproducibility/provenance evidence.
8. Independent R1 review 0325 identified a zero-read busy-spin on terminal
   disconnect, ambiguous malformed in-range PDL recovery that can consume a
   following valid TC, and a shared PTY test path. R1 delta 0326 accepted the
   fail-fast zero-read behavior and unique temporary PTY path for bounded host
   scope. The malformed-candidate recovery remains authority-blocked under
   NC-EXT-07; no inner-boundary heuristic was introduced. The user confirmed
   that no approved authority decisions are available and selected to keep
   S8-FU blocked; no policy is inferred. See the sprint register and review
   reports 0325/0326.
9. Independent V&V reviews 0327–0329 accepted the disconnected-PTY host oracle,
   required TS-2 case 81 to be characterization-only (not SRS-REQ-016 coverage),
   and reconciled TS-2/SVA-1 as 48 covered, 32 gaps, 1 characterization. The
   updated SVA-2 interface summary, ICD and test/tool records reflect this. The
   user chose to keep authority-blocked; no controlled recovery/profile/NC-owner
   decisions were supplied. The current code candidate digest and logs are in
   `docs/vv/evidence/2026-10-16-s8-fu-r1-candidate-manifest.md`. NC-EXT-07,
   NC-EXT-01/02 and broader S8-FU remain open; there is no commit/push eligibility.
10. R4 review 0330 and PA review 0331 raised metadata/effectivity/ledger findings.
    Bounded re-reviews 0332 and 0333 accepted their corrections, but explicitly
    require a final full applicable cycle against the current documentation.
    This is not broad N17 or N20 phase approval. Run and retain that cycle, then
    reconcile its result in verification/status records without changing the
    user-directed authority-blocked state.
11. The applicable local cycle was rerun after those corrections: CI-local 37/37,
    Lean/trace, smoke, GSW/license, metrics, docs, formatting, JSON/diff, RTEMS
    cross-build, Renode and focused UART PTY (51/0) passed. Retained logs and
    hashes are referenced by verification-results 2.53 and the current
    candidate manifest. This does not close S8-FU authority/external/target
    items or authorize commit/push.
12. On 2026-10-16 the user decided to postpone physical-target tests and require
    a homepage disclaimer. README now states that no physical RTPFS160ZT runtime,
    timing, or physical UART evidence is established and distinguishes cross-build
    from Renode emulation. This is a deferral, not an NC closure or general
    verification waiver. NC-EXT-07 remains open for recovery policy; NC-EXT-01/02
    owner dispositions, NC-EXT-03 external E2E/profile, and other sprint/release
    gates remain unresolved. Scoped R4/PA reviews 0336/0337 accepted only the
    metadata/effectivity corrections. The applicable local cycle then passed;
    logs/hashes are recorded in verification-results 2.58. This is not target
    evidence or sprint completion. No commit/push is currently eligible.

13. **Historical S8-VV-FU completion snapshot:** baseline is now HEAD
    `b8c3aee` (pushed to `origin` and `gitea`): it adds S8-SCH `ff_sched` and the
    S8-VV-FU ES/HS lifecycle host integration harness + deterministic bounded
    CCSDS/PUS decoder corpus. Local gates are green on `f0e1083` (40/40 CTest
    default + 40/40 ASan, `make trace` PASS 117/62, `make format-check` +
    `git diff --check` clean, `status.json` valid, coverage/README regenerated
    10238/18419 statements). Evidence is under
    `docs/vv/evidence/2026-10-16-s8-vv-fu-{ci-local,asan,trace}.log` and in
    verification-results.md (S8-VV-FU section). GAP-R3-1 and NC-EXT-25 are
    materially closed but remain OPEN pending independent R2/R3 review (owner
    disposition §3.2 is tracking only); NC-EXT-19 functionally corrected, stays
    OPEN pending R4/PA. At that historical snapshot S8-PUS-FU was next. Current
    active status and review/cycle results are in §§4–5 and
    `docs/planning/sprint-plan.md`. No phase/safety/security/release approval is
    inferred.

14. **Historical candidate snapshot (2026-10-16, pre-0343; superseded for current
    status by §4 above and §5 next steps):** uncommitted changes added
    duplicate-registration rejection, provider-output-length checks, visible
    S11-stub status, N7-adapter TC CRC/PDL guards, and HK provider-failure
    propagation. Tests `test_pus_fu`/`test_pus_n7_adapter` and TS-1 cases 97–101
    were added. At that snapshot, `make ci-local` and ASan passed 42/42;
    format-check, smoke, GSW/license checks and docs generation passed.
    Coverage/README refreshed to 10388/18736 statements and 1145/1400 functions.
    A then-current Renode result was from a build configured before the
    default-N7 transition and is retained only as historical evidence. Later
    post-0343 Renode evidence is in the current candidate manifest and remains
    emulation only, not physical hardware or an OBCP procedure oracle. The current
    host adapter test links pinned generated
    common ASN1SCC files and the real N7/ESA CRC wrapper, not a controlled CRC
    double. No mission SDK/profile compatibility is claimed.
    This remains an incomplete S8-PUS-FU increment: C-04..C-06, C-09, C-15/C-16
    and related interface items remain. PUSopen C-12 has controlled-stub APID
    validation evidence but NC-EXT-21 remains OPEN pending independent review and
    commercial SDK compatibility is unverified. NC-EXT-17 has bounded host
    evidence for provider-failure propagation but remains OPEN for broader S3
    mapping and independent review. NC-EXT-20/22/23/24 remain OPEN pending
     independent review/disposition. No commit/push is eligible
     until the sprint scope, applicable gates and batch R4+PA+V&V review pass.

15. **Historical resume actions (2026-10-17; superseded by item 16):**
     - Preserve verified local-cycle evidence. R4 0357–0369 covered earlier
       deltas and do not establish aggregate N17 or phase approval. The four
       reconciliation reviews 0370 (R2 V&V delta), 0371 (R3 requirement
       grounding), 0372 (R4 documentation) and 0373 (PA) were processed on
       2026-10-17: their bounded findings are corrected (TS-1/TS-5/TS-6 §3 gap
       lists and histories, oracle-independence wording, sprint-plan counts and
       references, review-log ledger), `FRICK-ICD-001` rev 1.4 now records the       implemented S3 subservice codes and S6 wire profile as **provisional**
       (ADR-0020), and NC-EXT-93 records the review-log ID collision. Two
       bounded follow-ups are queued: an independent review of the `N22-TS5-77`
       scheduler oracle (PA F-0373-02) and a PA re-verification of the refreshed
       candidate manifest (PA F-0373-04). PA granted no approval; the narrow
       R1/R2 closures are not PA full-cycle acceptance.
     - ADR-0020 and the candidate now record bounded public-reference defaults:
       ECSS-referenced S3 subservice mapping and one-shot list request, a
       logical region-ID/byte-offset S6 profile, and checked scheduler deadline
       exhaustion. These are framework implementation choices only, not
       mission baselines. Obtain controlled PUS profile/SRS/ICD inputs or
       authorized dispositions for NC-EXT-14/15/16/18; keep these NCs open and
       do not enable mission procedures from the provisional choices.
    - Keep NC-EXT-19/90 open pending approved schedule/time-domain/exhaustion
      policy, Table Services operational integration evidence, and applicable
      target timing/fault-response evidence. No schedulability or target runtime
      claim follows from host tests, cross-build, or Renode.
    - Continue only bounded selected-N7 OBCP integration. NC-S9-01, production
      service/PUS S18 bindings, and accepted script execution/resource envelope
      block mission use and SRS-REQ-045 closure. Do not implement a replacement
      engine or claim SRS-REQ-054 complete.
     - Keep NC-EXT-84 provenance, hosted CI, physical-target, full sprint,
       independent-review and release gates open. Do not commit or push.
     - Reconciliation cycle evidence: `make ci-local` 43/43 + Lean (62 theorems)
       + trace PASS, pinned `format-check`, containerized `docs`, `gsw-selfcheck`
       and `smoke` all green on 2026-10-17; logs retained under
       `docs/vv/evidence/2026-10-17-s8-reconciliation-*.log` and the refreshed
       candidate manifest `2026-10-17-s8-reconciliation-candidate-manifest.md`
       supersedes the stale 49/19-file inventories (verification-results 2.90,
       loop-audit 10.7, sprint-plan 1.69). Renode/RTEMS/QEMU legs must be
       re-recorded for this tree before any claim cites them.

16. **Current resume actions (2026-10-20):**
    - The repository is on `main` at HEAD `44d2af2`, a status-only follow-up
      over engineering baseline `318f49b`. The working tree contains the
      uncommitted S8-FDIR-01 increment; preserve attribution and do not commit
      or push until N7 gates and the full sprint scope pass.
    - Reconcile the independent R4 documentation review
      `FRICK-REVIEW-0386` (N17): it was performed on 2026-10-20 and **REJECTED**
      with 1 Major, 4 Minor and 4 Comments. F-0386-01 identifies the stale
      human-readable `docs/requirements/traceability.md` mirror for
      SRS-REQ-073/074/075. This Major correction is now present in the working
      tree (`traceability.md` rev 0.51; `make trace` PASS), but NC-EXT-103 stays
      OPEN until R4 delta re-review under N12. Bounded corrections for
      NC-EXT-104..107 and NC-EXT-110 are now in the working tree; delta review
      remains pending. NC-EXT-108/109/111 remain open for clarity or authority
      disposition. Do not infer approval from the review's confirmed claims or
      earlier cycle.
    - R2, R3 and PA independent reviews for S8-FDIR-01 remain pending.
      Requirements-authority questions and the ICD event catalogue are not to be
      guessed or unilaterally resolved.
    - **Historical N18 state at the host-gate sweep:** true **SKIP/gap** because
      no current RTEMS toolchain image was available then; the stale ELF had been
      removed after producing a misleading PASS (NC-TS-01). **Current follow-up:**
      a fresh RTEMS cross-build and prescribed `make renode` PASS are recorded in
       verification-results rev 3.00. This is emulator evidence only; the RTEMS
      watchdog binding is deliberately unwired (NC-PAL-01), and target/HIL
      evidence and authorized-human sign-off remain open.
     - **Current host-gate reconciliation:** the ordered host run is **2 PASS / 5
       FAIL**, not a green local cycle. `make trace` and `make gsw-selfcheck` passed;
       `make smoke`, `make verify-lean`, `make format-check`, `make metrics` and
       `make docs` failed for the recorded environment/tooling reasons. In
       particular, a generated metrics panel or successful metrics/status check
       must not be read as a passing `make metrics` gate. The separate T24
       container format dry-run passed, but its run identity remains qualified
       under OPEN NC-EXT-113. Exact results and retained evidence are in
       `docs/vv/verification-results.md` §S8-FDIR-01 (rev 3.00) and
       `docs/vv/evidence/2026-10-20-s8-fdir-host-gates.log`. R4 review
       `FRICK-REVIEW-0387` recorded four Minor findings; NC-EXT-28/35/112/113
       remain OPEN/deferred. N15 remains incomplete. The earlier 43/44 CTest and
       manual sweep results are historical and do not replace this later host run.
       The later bounded N18 emulator run passed as recorded in §4 and verification
       results rev 3.00; it does not close target/HIL or watchdog evidence. R2/R3/PA,
       target/HIL and NC-EXT-108/109/111 authority dispositions remain open.

### Historical 2026-10-14 status narrative (superseded for current actions by §4/§5)

> Execution view: `docs/planning/sprint-plan.md` (FRICK-PLAN-0002). S1 is complete
> (NVM + NOR/QSPI MTD + MSS CAN done, **M10 closed via the QEMU run**); S2 (MSS CAN)
> done; **S3 (MSS I2C) delivered** (`hal_i2c=PASS`, SRS-REQ-058), **S4 (MSS SPI)
> delivered** (`hal_spi=PASS`, SRS-REQ-069), **S5 (MSS block device) delivered**
> (`hal_block=PASS`, SRS-REQ-070), **S6a (GEM TSN peripheral-bound) delivered**
> (`hal_tsn=PASS`, M11 10/13, ctest 22/22), **S6b-2 (lwIP netif on the GEM;
> `hal_net_lwip=PASS`)** delivered; **S7 (PCIe + SpaceWire) delivered**
> (`hal_pcie=PASS`, `hal_spacewire=CONTRACT`, M11 11/13) and the **NET follow-up
> sprint** done (NC-S6-03 co-implemented: GEM 64-bit `gem_phys`/`gem_wr_qbqp`/
> `TBQP_HI`/`RBQP_HI`/`DMACFG.ADDR64`; NC-154-01 closed: `test_lwip_netif` 26/26
> host mock-MAC loopback; on-Renode GEM DMA still GAP-A5). **S8 (M8) delivered**:
> gPTP stack (`test_gptp` 58 checks host) + 802.1Qci via GEM screener type 1 (`test_gem_tsn`
> 22 checks); 802.1Qav CBS + 802.1Qbb PFC documented `FF_NOT_IMPLEMENTED` (NC-S8-01/NC-S8-02).
> **S4b implementation delivered** (SRS-REQ-059/060/061); the historical delivery
> evidence remains in verification-results.md. Follow-up review reports
> FRICK-REVIEW-0167/0168/0169 are historical APPROVED WITH COMMENTS reports; their
> delta reviews remain pending. FRICK-REVIEW-0221..0228 are NOT APPROVED for
> their reviewed scopes; 0229 is bounded R2 review, 0230 scoped R4 approval, and
> 0231 scoped PA consistency only, and 0232 scoped handover R4 pass. Transition
> audits 0233..0234 and follow-ups 0235..0259 have open findings; R2 0242,
> 0245 and 0249 are bounded results only, not T21 qualification. R4 0243/0244 and
> 0246 is NOT APPROVED; 0247 verifies the DDJF correction only. PA 0248 found a
> fail-open unreadable-file case; R2 0249 bounded review passed the author fix.
> R4 0250/0252/0254 and PA 0251/0253/0255 found active-document scope/status
> inconsistencies; 0252..0257 were NOT APPROVED for their reviewed scopes;
> bounded R4/PA follow-ups 0258/0259 found no new status/history finding but
> close no NC or overall gate. R4 0260 and PA 0261 are bounded N21/N22 reviews;
> the duplicate report ID was resolved by renumbering PA to 0261. NC-EXT-48..68
> are recorded in the sprint plan. R4 0262 reviewed the N22 document set and
> recorded NC-EXT-69..74; no overall N17/PA gate is closed. PA 0263 found the
> SB overflow EVS event was missing; the initial approach that changed all
> `FF_EvsSend` delivery was rejected by R1 follow-up FRICK-REVIEW-0264. The
> current tree restores synchronous ordinary sends and uses explicit
> `FF_EvsSendDeferred` only for SB overflow, dispatched through the bounded
> TM-output task queue. Tests cover normal synchronous send, overflow delivery,
> callback re-entry/dispatch bounds, and saturation accounting. Listener-driven
> publisher waiting on the SB or EVS mutex was removed by try-locks; strict
> approved queue-lock correction now uses `FF_PalQueueTryPut`: queue-mutex busy
> skips only that pipe copy, increments dropped accounting, continues fan-out,
> requests a busy-specific deferred event and returns `FF_TIMEOUT`; full queue
> remains distinct. The synchronized test confirms return while the lock is held
> and another matched pipe receives its copy. Whole-publish WCET/freedom remains
> unproven. R1 FRICK-REVIEW-0264 also found saturation can lose
> SRS-REQ-011 ground forwarding. R3 FRICK-REVIEW-0265 did not approve the first
> requirements draft for baselining; event/loss/measurement boundaries were
> corrected in the working tree. FRICK-REVIEW-0266 approves continued
> requirements development with comments only, not baseline/N14 closure.
> SSS-REQ-006 and SRS-REQ-072 allocate `Bmax`, `Rmax`, `Dmin`, `Lmax`, service
> interval, link profile and ground-receipt boundary, all authority-owned TBDs.
> NC-EXT-77 records the corrected trace-count comment. NC-EXT-75/76 remain open
> pending profile authority, loss-counter/reporting implementation, full-cycle,
> and independent evidence. T21 remains NOT FULLY QUALIFIED; no overall
> N17/PA approval or NC closure is implied.
> These do not close NCs or qualify T13; NC-EXT-34..81 remain open. The owner
> authorized MPL-2.0 for original project-authored code and materials; NC-EXT-43 remains open only
> for external/generated and combined-distribution compatibility. No additional
> license authorization is pending from the owner.
> NC-S4b-01..09, NC-EXT-01..81
> and GAP-R3-1 remain open pending independent review or authority disposition.
> Sprint precedence is unresolved: the corrective sequence is proposed, but it
> conflicts with “drivers first”; do not start a competing sprint until the plan
> and human owner resolve ordering/approval.

**Historical resumption snapshot (SB/EVS configured structural stress; point-in-time text, superseded by current §4/§5; do not use its hosted-CI/NC statuses as current):**

 1. Continue using `docs/planning/sprint-plan.md` rev 1.10. Under N21,
     correct functional bugs and implement missing features committed for the
     current or previous sprints; record other findings as NCs and continue
     unrelated development. The owner has
     authorized MPL-2.0 for original project-authored code and materials; no further authorization
     is needed for that scope. Obtain R4/PA re-review of the current scope/status
     corrections; complete NC-EXT-43 external/generated compatibility disposition;
     obtain remaining independent R4/PA and hosted/configuration/immutable evidence
     before any applicable release gate (bounded R2 0249 is complete only for its scope).
        The narrowly scoped `FF_EvsSendDeferred` SB overflow correction restores
       normal synchronous-send behavior following PA FRICK-REVIEW-0263 and R1
       FRICK-REVIEW-0264. The initial event-envelope requirement draft was
       NOT APPROVED by FRICK-REVIEW-0265; FRICK-REVIEW-0266 approves continued
       development only, not baselining. NC-EXT-75/76 remain open pending
       authorized rate/service/latency/link values, loss-report implementation,
        applicable verification and independent delta reviews. SB queue-mutex
        busy/partial-fan-out behavior is implemented and host-tested; full queues
        retain the distinct overflow path. SRS-REQ-072 end-to-end loss counter,
        reporting and approved event profile remain unimplemented/open, and
        SSS/SRS authority-owned TBDs are unchanged. Keep all NCs open until their
       criteria are met.
     A host-only exploratory structural stress test now holds all 32 subscriber
     queue locks for one publish (32 drops, `FF_TIMEOUT`), fills 32 pipes × depth
     32 (1024 queued copies), then creates three 32-event overflow bursts:
     96 more SB drops, 96 accepted EVS notifications cumulatively, 32 counted
     EVS notification losses and 128 total log entries (newest 64 retained).
     Dispatch invokes 96 callbacks across the two phases. This is configuration-bound
     saturation observation, not an event-rate/WCET/ground-delivery guarantee.
     `make ci-local` (28/28, Lean, trace), `make smoke`, `make renode` and targeted
     clang-format/diff checks pass after this addition. RTEMS cross-build and
     hosted CI were not run. The initial test placement failed due PAL queue-slot
     exhaustion across the pre-existing in-process test suite; separate-process
     isolation resolved that test-harness limitation.
     **Commit/push remains blocked**: applicable independent R1/R3/R4/PA reviews,
     N14 baseline authority, open NC-EXT-75/76, end-to-end loss semantics and
     implementation, full sprint scope, and remaining configuration/hosted/RTEMS
     evidence are not complete. Passing this stress test does not close those gates.
      Scoped R4 FRICK-REVIEW-0268 and PA FRICK-REVIEW-0269 have since reviewed the
      stress-test documentation/evidence with comments only; they are not overall
      N17/N20 approval. Fresh clean Debug build and 28/28 CTest passed; selected
      artifact identities and limits are in `docs/vv/evidence/2026-10-14-sb-evs-structural-stress.md`.
      The authorized user then directed a reproducible framework-only SB/EVS
      parameter characterization without inventing mission inputs. A retained
      sweep covers fan-out/depth 1–32 and overflow-burst counts from zero through
      first EVS notification saturation (9,984 profiles), plus dispatch limits
      1–64 with excess pending for limits 1–63. It passed 1,697,329 assertions;
      see `docs/architecture/sb-evs-characterization.md` and
      `docs/vv/evidence/2026-10-14-sb-evs-structural-sweep.md` + `.csv`.
      R3/R2/R4/PA scoped reports FRICK-REVIEW-0270..0273 are recorded. This is
      native-host structural characterization only, not mission performance,
      WCET, target timing, link availability or ground receipt. The first
      malformed CSV was overwritten and is not retained separately. T22 remains
      pending formal tool-owner N10 acceptance; NC-EXT-75/76, mission values,
      detection/counter/reporting semantics, N14 mission baseline, hosted CI,
      and broader sprint/release gates remain open. A fresh RTEMS 6 cross-build
      (`build/rtems-sweep-final`, ELF64 RISC-V) and real Renode functional check
       (`RENODE_TEST: PASS`) had passed at the time; hosted CI on that then-current
       uncommitted tree was unrun. R2 source-identity delta FRICK-REVIEW-0274 confirms the
      final script hash and byte-identical retained CSV.
 2. Then continue from the open NC register in the same plan. Keep
        NC-S4b-01..09, NC-EXT-01..81 and GAP-R3-1 open until independent delta reviews or
   authorized requirements/profile dispositions are recorded. Initial
   FRICK-REVIEW-0167/0168/0169 remain APPROVED WITH COMMENTS; no D1 reports exist.
  3. The external assessment's PDL wrap finding has a widened decoder fix and
   boundary tests; an independent reviewer found the TM decoder tests needed
   parity with TC and a separate GSW stream-offset parsing defect. Both were
   corrected; rerun full tests and independent delta reviews.
  4. Evidence now includes native build/test, trace, GSW vectors, smoke, RTEMS
    Docker cross-build and a real Renode `RENODE_TEST: PASS`. ASan/UBSan and the Docker-image `make ci-local` have passed after the final C/Python
   test changes; `make ci-local` supplies the Lean `lake build` evidence. The host
   `make verify-lean` attempt failed with `lake: not found`; distinguish that local
    limitation from the observed Docker-container PASS. Forge CI is still unrun on this tree.
  5. FRICK-REVIEW-0173 independently verified the finite-buffer GSW resynchronization
    case but did not close T7 overall; it opened NC-EXT-13 for the CCSDS primary
    header profile/acceptance decision. Its severity is provisional pending the
     controlled criterion and rationale tracked by NC-EXT-31. At this historical
     snapshot NC-EXT-02 lacked maximum-size application-ingest evidence; current
     status is in §4 and `docs/planning/sprint-plan.md`. NC-EXT-03 lacks external
     ground-segment E2E.
  6. Status gates for documentation review, PA review, review closure, loop audit
     and V&V records are intentionally yellow while this loop is open. Do not
     commit/push until N15/N17/N20 and required independent re-reviews pass.
     Although the Docker-image `make ci-local`, smoke/GSW, format, metrics and Renode
     commands passed, their status semaphores remain yellow pending NC-EXT-29's
     complete immutable configuration/tool/artifact/log identity. Host
      `make check` is blocked at `verify-lean` because `lake` is absent; Docker
      `lake build` passed in the observed local image.
   7. The proposed sprint sequence is S8-FU closure → S8-VV-FU (GAP-R3-1,
   NC-EXT-25) → S8-PUS-FU (NC-EXT-14..18,
   20..21, 23..24 where requirements/profile permit autonomous correction) →
     S8-SCH (NC-EXT-19/M17) → M12. The current plan interprets the owner's
     general resumption instruction as selecting S8-FU first; bounded R4 review
     0311 accepted this wording, closing NC-EXT-30/65 only for this plan comment.
    NC-EXT-22 (PUS S11 stub) is assigned to M15;
    it is not closed by the application scheduler. Existing GAP-A1..A14 remain
     open in `class-a-gaps.md`; mission-specific licensing/adoption and
      SAVOIR/OSRA applicability require authority/legal decisions before any
      conformance/adoption claim; NC-EXT-43 tracks external/generated component
      boundaries and combined-distribution compatibility, not pending owner
      authorization for original project content.
      GAP-A1..A14
     closure work is mapped in sprint-plan §3.1; those records remain OPEN.
  8. N14 is OPEN: the current SSS-REQ-030 change lacks an independent R3 review
    (NC-EXT-33). Latest author corrections address the ICD TC/TM diagram and time offsets,
    the PUS backend signature, a stale ICD source-comment path, and review-report
    publication instructions (NC-EXT-26/27/32). All remain OPEN pending
    independent re-review; do not infer closure from these edits.

- **S8/M8 gPTP + Qci completed** (SRS-REQ-068/066). gPTP stack: BMCA,
   Sync/Follow_Up, PDelay, PI servo (`test_gptp` 58/58 host) — integrates with GEM TSU
   `get_time_ns` + `FF_EthMac_t`; on-Renode GEM TSU register accessible (host-tested
   mock-MAC frame exchange; full GEM DMA = GAP-A5). Qci: screener type 1 entry per
   traffic-class; `test_gem_tsn` 22/22. Qav CBS + Qbb PFC = `FF_NOT_IMPLEMENTED`
    (NC-S8-01/NC-S8-02). Integration-manual R1/R4 reviews (FRICK-REVIEW-0159/0160)
     corrected; its R4 delta review is recorded as FRICK-REVIEW-0164. The
     broader S8-FU R4 gate remains open; licensing/status review 0252 was
      NOT APPROVED and was followed by 0254/0255 and 0256/0257, all NOT
      APPROVED. Their active-status corrections are recorded; independent
      re-review remains open.
2. **M4 residual** restored M4 scope (S4b/S4c/S4d): PUS-C over IP/TSN
   (SRS-REQ-059/060/061), CAN-FD/CANopen/RS-485 DE (SRS-REQ-062/063/064),
   PUSopen adapter (SRS-REQ-065), fabric contract (SRS-REQ-067).
3. **M12** OBCP engine; **M13** mission services; **M14** multicore/SMP;
   **M15** in-house PUS-C + CFDP + VFS; **M16** in-house OBCP engine;
   **M17** RTEMS-backed application scheduler.
4. Keep N18 blocking (nightly real run) and close GAP-A4 (certified MISRA checker)
   when available.

## 6. Key facts / gotchas

- Renode MPFS platform peripheral base addresses: uarts `mmuart0..4`
  (0x20000000, 0x20100000, …); console = **mmuart1**; gpio`0/1/2`
  (0x20120000/…); CAN 0x2010C000/0xD000; SPI 0x20108000/0x9000; I2C
  0x2010A000/0xB000; GEM 0x20110000/0x12000; SD 0x20008000; eNVM cfg
  0x20200000 / data 0x20220000.
- **MSS GPIO register map** (validated): per-pin `GPIO_CFG[pin]` at `0x00+pin*4`
  (bit0 output enable, bit1 input enable, bit3 int enable, bits5-7 int type);
  `GPIO_IN=0x84`, `GPIO_OUT=0x88`. GPIO0 has **14** pins.
- **MSS MMUART register map** (validated): the Renode `polarfire-soc.repl`
  instantiates the UARTs as `UART.NS16550` with **`wideRegisters: true`**, so the
  MPFS MMUART uses **4-byte register spacing** on hardware *and* in Renode (the
  model divides `offset` by 4 when `offset % 4 == 0`, so wide offsets
  `0x00/0x04/0x08/0x0C/0x10/0x14/0x18/0x1C` alias the compact registers). A serial
  driver must use the wide offsets (hardware-correct) — compact offsets also
  "work" in Renode by aliasing but are wrong on silicon.
- RTEMS image entry point is **0x1000000000** (HSS high DDR alias); set
  `u54_1 PC 0x1000000000`, halt `e51`/`u54_2..4`.
- `make renode` passes only when `build/renode/frick_fsw_renode.elf` exists
  (git-ignored); otherwise it SKIPs (proxy). Rebuild it after C changes.
- `trace.py` requires every non-derived requirement to have `tests`/CI evidence
  and existing implementation paths; planned reqs use `ci:docs`.
- Independent reviews tend to find **real defects**; run them before every
  commit and close NCs (N12).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial handover/resume document (N18 in force; M11 GPIO done) |
| 0.2 | 2026-10-10 | frick-fsw | M11 serial deferred/reverted; codename "Eärendil" disclaimer added |
| 0.3 | 2026-10-10 | frick-fsw | M11 serial re-implemented with hardware-correct 4-byte MMUART offsets (Renode wideRegisters); key fact recorded |
| 0.4 | 2026-10-10 | frick-fsw | M11 GPIO + SERIAL done (SRS-REQ-047/048); counts 86/49/23/14 |
| 0.5 | 2026-10-11 | frick-fsw | Add §0 "How to resume in a new session" (kickoff prompt) |
| 0.6 | 2026-10-11 | frick-fsw | M11 GPIO + SERIAL + MEM done (SRS-REQ-047/048/049); counts 87/50/23/14 |
| 0.7 | 2026-10-11 | frick-fsw | PUS-C ASN.1 default backend (SRS-REQ-050, ADR-0016); M14/M15 added; counts 94/54/26/14 |
| 0.8 | 2026-10-11 | frick-fsw | M11 ETH HAL class exercised in Renode (`hal_eth=PASS`) |
| 0.9 | 2026-10-11 | frick-fsw | M11 NET `FF_NetStack_t` contract check exercised in Renode (`hal_net=PASS`) |
| 1.0 | 2026-10-11 | frick-fsw | Norm N20 (Senior SW PA review of every phase product); counts refreshed 94/54/26/14/51; PA gate added |
| 1.1 | 2026-10-11 | frick-fsw | N12 loop closed: independent R1/R2/R4 reviews of the N20 set and FRICK-REVIEW-0032 to closure; PA review PASS (FRICK-REVIEW-0063) |
| 1.2 | 2026-10-11 | frick-fsw | Milestone M16 added; counts 96/55/27/14; sprint plan published; next steps point to Sprint 0/1 |
| 1.3 | 2026-10-11 | frick-fsw | Sprint 1.1: reference eNVM NVM backend (SRS-REQ-055), M11 5/13, counts 97/56/27/14 |
| 1.4 | 2026-10-11 | frick-fsw | Sprint 1.2: reference NOR/QSPI MTD backend (SRS-REQ-056); counts 98/57/27/14 |
| 1.5 | 2026-10-11 | frick-fsw | Sprint 1.3: reference MSS CAN backend (SRS-REQ-057); M11 6/13 (`hal_can=PASS`); counts 99/58/27/14 |
| 1.6 | 2026-10-11 | frick-fsw | CAN review set closed (FRICK-REVIEW-0106..0119: R1/R2/R4 + PA initial/delta/closure + freeze); accept-all mask, clock 80 MHz, test 129 checks |
| 1.7 | 2026-10-11 | frick-fsw | M10 done: QEMU generic-BSP (riscv/rv64imafdc) emulated-target run (`make qemu`, T20); toolchain image gains the rv64imafdc BSP + QEMU |
| 1.8 | 2026-10-12 | frick-fsw | Sprint 3: reference MSS I2C backend (SRS-REQ-058); M11 7/13 peripheral-bound (`hal_i2c=PASS`); counts 100/59/27/14/51; ctest 20/20 (test_i2c 144 checks); next S4 (SPI) |
| 1.9 | 2026-10-12 | frick-fsw | Sprint-3 review set closed (FRICK-REVIEW-0129..0136: R1/R2/R3/R4/PA initial + deltas; PA REJECTED -> APPROVED WITH COMMENTS); user-facing TCP/IP + TSN sections and the expanded framework description (FRICK-ENG-0007) added; T14 reproducible via `make misra-docker`; next S4 (SPI) |
| 1.10 | 2026-10-12 | frick-fsw | Restored explicit M4 scope (N19 audit): PUS-C over IP/TSN (SRS-REQ-059/060/061), fabric contract (SRS-REQ-067), CAN-FD/CANopen/RS-485 DE (SRS-REQ-062/063/064), PUSopen adapter (SRS-REQ-065); M8 gPTP/CoreTSN (SRS-REQ-068) + GEM TSN Qav/Qci/Qbb (SRS-REQ-066); counts 112/69/29/14/51; DDJF acronym registered (FRICK-ENG-0007 0.3); sprint S4b/S4c/S4d — FRICK-REVIEW-0137 |
| 1.11 | 2026-10-12 | frick-fsw | Sprint 4: reference MSS SPI backend (SRS-REQ-069); M11 8/13 peripheral-bound (`hal_spi=PASS`); counts 113/70/29/14/51; ctest 21/21 (test_spi 126 checks); next S5 (BLOCK eMMC/SD) |
| 1.12 | 2026-10-12 | frick-fsw | Sprint 5: reference MSS block-device backend (SRS-REQ-070); M11 9/13 peripheral-bound (`hal_block=PASS`); counts 114/71/29/14/51; ctest 22/22 (test_block 85 checks); next S6 (NET/TSN peripheral-bound) |
| 1.13 | 2026-10-12 | frick-fsw | Sprint 6a: GEM native TSN backend exercised peripheral-bound (`hal_tsn=PASS`, M11 10/13; SRS-REQ-038); NC-S6-01 opened (NET lwIP-on-GEM peripheral-bound deferred to sprint S6b); next S6b/S7 |
| 1.14 | 2026-10-12 | frick-fsw | Sprint 6b-1: in-repo lwIP RTEMS `sys_arch` port (SRS-REQ-040) — host 23/23 with `FF_USE_LWIP=ON`, RTEMS build produces the lwIP archive (forced-extraction link resolves the stack; the image does not reference the adapter yet); NC-S6-02 opened (RTEMS runtime object sizing → S6b-2); NC-S7-01 (SpaceWire no model → S7 contract-level) |
| 1.15 | 2026-10-12 | frick-fsw | Sprint 6b-2: RTEMS object sizing wired (NC-S6-02 closed) + lwIP netif bound to the emulated GEM (`hal_net_lwip=PASS` on the lwIP image); NC-151-02/05 closed; **NC-S6-03** opened (latent GEM 64-bit descriptor DMA); NC-S6-01 narrowed (packet round-trip residual) |
| 1.16 | 2026-10-12 | frick-fsw | Sprint-6b-2 review close-out (FRICK-REVIEW-0153/0154/0155): netif MTU fixed (1500), glue-test gap tracked (NC-154-01), traceability History bumped (NC-154-02), stale doc sites + nightly label corrected |
| 1.17 | 2026-10-12 | frick-fsw | Sprint 7 delivered + pushed (`64961ac`): PCIe (`hal_pcie=PASS`) + SpaceWire contract-level (`hal_spacewire=CONTRACT`, M11 11/13); NET follow-up sprint: NC-S6-03 co-implemented (GEM 64-bit descriptor addressing: `gem_phys` returns `uint64_t`, `gem_wr_qbqp`, `TBQP_HI`/`RBQP_HI`, `DMACFG.ADDR64`; per-descriptor truncation explicit; full 3-word descriptor format deferred to M14); NC-154-01 closed (`test_lwip_netif` 26/26 host, mock-MAC loopback); `test_gem` extended for 64-bit register writes; trace/lean/format/redline all green; combined R1+R2+R3+R4+PA review APPROVED WITH COMMENTS (FRICK-REVIEW-0162, 8 Minor findings, all corrected); delta re-review APPROVED (FRICK-REVIEW-0163, all corrections verified); commit + push to both remotes completed |
| 1.18 | 2026-10-12 | frick-fsw | Handover updated to record FRICK-REVIEW-0162 (NET follow-up confirmation, APPROVED WITH COMMENTS) and FRICK-REVIEW-0163 (delta re-review, APPROVED) in the recent-reviews list; all findings closed; next sprint S8 = M8 gPTP + CoreTSN |
| 1.19 | 2026-10-13 | frick-fsw | **S8/M8 delivered:** gPTP stack (BMCA, Sync/Follow_Up, PDelay, PI servo) in `src/drivers/eth/gptp/` (58 checks host); 802.1Qci via GEM screener type 1 in `ff_gem_tsn.c` (test_gem_tsn 22 checks); 802.1Qav CBS + 802.1Qbb PFC documented `FF_NOT_IMPLEMENTED` (NC-S8-01/NC-S8-02); integration-manual R1/R4 reviews (FRICK-REVIEW-0159/0160) corrected; traceability + SRS updated for SRS-REQ-066/068 |
| 1.20 | 2026-10-13 | frick-fsw | **S4b delivered:** PUS-C over IP/TSN (SRS-REQ-059/060/061) — seg/reassembly layer (`link_seg.c/h`, 2-byte BE length-prefix, allocation-free), IP backend (`link_ip.c/h`, UDP default + TCP with seg) over `FF_NetStack_t`, TSN backend (`link_tsn.c/h`, EtherType 0x88F5, callback-driven RX ring) over `FF_EthMac_t + FF_TsnBackend_t`; `link.h` declares new backends; ctest 27/27 + trace + Lean + smoke + GSW all PASS; Renode SKIP (host-only code). Traceability + SSS-REQ-028 updated from planned to implemented. |
| 1.21 | 2026-10-13 | frick-fsw | **S4b delta re-review (N12 closure):** initial R1+R2 (FRICK-REVIEW-0165) and PA (FRICK-REVIEW-0166) reviews REJECTED (2 Major + 10 Minor + 6 V&V gaps + 4 PA Minor); all findings fixed; 3 new findings raised and closed (NF-1 unused variable, NF-2 Major buffer overflow in `rx_partial` copy-back, NF-3 doc omission); R1+R2 delta (FRICK-REVIEW-0165-D1) APPROVED; PA delta (FRICK-REVIEW-0166-D1) APPROVED; `-Wall -Wextra -Werror` clean; max-size fragmented regression test added; status.json/status/handover/loop-audit/verification-results/CHANGELOG/reviews all updated |
| 1.22 | 2026-10-13 | frick-fsw | S4b host PUS-C round-trip demonstrations added and documentation corrections applied; NC-S4b-01..05 remain open pending independent delta-review evidence (initial FRICK-REVIEW-0167/0168/0169 are APPROVED WITH COMMENTS). |
| 1.23 | 2026-10-13 | frick-fsw | README/network/handover scope/status corrections applied for NC-S4b-06..08; independent delta review remains pending. |
| 1.24 | 2026-10-13 | frick-fsw | Current handover reset to uncommitted baseline `57974d2` + S8-FU changes; record NC-S4b/NC-EXT open status, latest review follow-ups, corrected GSW stream-offset parser and pending gates; document RTEMS Docker cross-build/Renode PASS and local Lean unavailability. |
| 1.25 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0173 partial re-review and NC-EXT-13; clarify maximum-ingest and external-GSW evidence gaps; align the current S8-FU open status with sprint-plan rev 0.26 and verification-results rev 0.48. |
| 1.26 | 2026-10-13 | frick-fsw | Align resume point to sprint-plan rev 0.28; track NC-EXT-14..24 and sequence S8-PUS-FU/S8-SCH, while preserving unresolved authority and assurance gaps as OPEN |
| 1.27 | 2026-10-13 | frick-fsw | Align handover with sprint-plan rev 0.29; map existing GAP-A1..A14 closure work and authority/resource requests |
| 1.28 | 2026-10-13 | frick-fsw | Align handover with sprint-plan rev 0.30; include GAP-R3-1 and NC-EXT-25 in S8-VV-FU before PUS/scheduler work |
| 1.29 | 2026-10-13 | frick-fsw | Align handover with sprint-plan rev 0.32; include NC-EXT-26/27 and maintain open independent-review gates |
| 1.30 | 2026-10-13 | frick-fsw | Record latest R4 REJECTED/PA FAIL verdicts and NC-EXT-28..31; synchronize current summary, M17, counts and unresolved owner sequencing |
| 1.31 | 2026-10-13 | frick-fsw | Correct stale integration-manual review statement; reflect separate R4 gate verdict and keep all closure reviews open |
| 1.32 | 2026-10-13 | frick-fsw | Record follow-up review IDs and qualify NC-EXT-13 as an open authority policy issue pending controlled acceptance and severity basis |
| 1.33 | 2026-10-13 | frick-fsw | Identify FRICK-REVIEW-0174 as the latest R4 verdict; update sprint-plan revision reference and include planned M17 in the milestone summary |
| 1.34 | 2026-10-13 | frick-fsw | Record latest R4/PA reports 0176/0177 and maintain explicit blocked status |
| 1.35 | 2026-10-13 | frick-fsw | Reconcile M9 status with status.json and distinguish pending evidence configuration from test command outcomes |
| 1.36 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0178/0179 and NC-EXT-32; retain current release gates open |
| 1.37 | 2026-10-13 | frick-fsw | Explain passing command results versus yellow verification semaphores and record the host lake limitation |
| 1.38 | 2026-10-13 | frick-fsw | Align sprint-plan revision reference and record author corrections for NC-EXT-26/27 without claiming closure |
| 1.39 | 2026-10-13 | frick-fsw | Record latest failed review verdicts and NC-EXT-26/27 re-review status |
| 1.40 | 2026-10-13 | frick-fsw | Capture author corrections and pending re-review scope for NC-EXT-26/27/32 |
| 1.41 | 2026-10-13 | frick-fsw | Synchronize latest recorded review IDs/verdicts and sprint-plan rev 0.38 |
| 1.42 | 2026-10-13 | frick-fsw | Align handover with sprint-plan rev 0.39 and expanded N11/NC-EXT-32 clarification |
| 1.43 | 2026-10-13 | frick-fsw | Align resume pointer with sprint-plan rev 0.39 and preserve pending owner precedence decision |
| 1.44 | 2026-10-13 | frick-fsw | Align live sprint pointer and proposed sequence wording with sprint-plan rev 0.41; no owner priority inferred |
| 1.45 | 2026-10-13 | frick-fsw | Correct live sprint pointer after rev 0.41 and make unresolved precedence wording explicit |
| 1.46 | 2026-10-13 | frick-fsw | Track new N14/R3 review gap and label proposed sprint order as pending owner decision |
| 1.47 | 2026-10-13 | frick-fsw | Align resume point with sprint-plan rev 0.42 and explicitly record N14/NC-EXT-33 as open |
| 1.48 | 2026-10-13 | frick-fsw | Correct stale live sprint-plan pointer to rev 0.42 after R4 delta review |
| 1.49 | 2026-10-13 | frick-fsw | Align live pointer with sprint-plan rev 0.43 and record ICD authority scope remains under NC-EXT-13 |
| 1.50 | 2026-10-13 | frick-fsw | Align live sprint pointer with sprint-plan rev 0.44; record FRICK-REVIEW-0180 corrections pending review |
| 1.51 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0180/0181 as latest verdicts and the corrective re-review scope |
| 1.52 | 2026-10-13 | frick-fsw | Correct current sprint-plan pointer to rev 0.44 and align PA gate wording with N12 |
| 1.53 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0182/0183 and align live sprint-plan pointer with rev 0.45 |
| 1.54 | 2026-10-13 | frick-fsw | Align current resume pointer with sprint-plan rev 0.45 after review follow-up |
| 1.55 | 2026-10-13 | frick-fsw | Align with sprint-plan rev 0.46 and record FRICK-REVIEW-0184/0185 plus NC-EXT-34 |
| 1.56 | 2026-10-13 | frick-fsw | Correct handover NC range and live sprint-plan pointer to rev 0.46 |
| 1.57 | 2026-10-13 | frick-fsw | Align sprint-plan pointer to rev 0.47 after NC-EXT-34 source correction |
| 1.58 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0186/0187 and align current pointer with sprint-plan rev 0.48 |
| 1.59 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0188/0189 scoped results and align plan pointer to rev 0.49 |
| 1.60 | 2026-10-13 | frick-fsw | Align to sprint-plan rev 0.50 and include T1 requalification NC-EXT-35/36 in resumption record |
| 1.61 | 2026-10-13 | frick-fsw | Align pointer with sprint-plan rev 0.51 and record FRICK-REVIEW-0190 T1 findings |
| 1.62 | 2026-10-13 | frick-fsw | Align to sprint-plan rev 0.52 and update latest T1/NC-EXT-34 status wording |
| 1.63 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0191..0193 and align pointer to sprint-plan rev 0.53 |
| 1.64 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0194 and align pointer to sprint-plan rev 0.54 |
| 1.65 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0195..0197 scoped results; retain NC-EXT-34..37 open |
| 1.66 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0198/0199 and pending re-review of active T1 claim corrections |
| 1.67 | 2026-10-13 | frick-fsw | Reconcile handover evidence wording with observed, mutable Docker-image limitation under NC-EXT-35 |
| 1.68 | 2026-10-13 | frick-fsw | Reconcile active Docker/T1 reproducibility language across handover; NC-EXT-35 remains open |
| 1.69 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0200/0201 and pending independent review of active T1 wording corrections |
| 1.70 | 2026-10-13 | frick-fsw | Bound `make ci-local` description to actual stages and retain separate smoke/docs commands |
| 1.71 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0202/0203 and NC-EXT-38; preserve open review/hosted gates |
| 1.72 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0204/0205 and align handover to sprint-plan rev 0.56; NCs remain open |
| 1.73 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0206/0207; retain qualification, evidence, and scope findings open |
| 1.74 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0208/0209; align plan pointer to rev 0.58; retain all NCs open |
| 1.75 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0210/0211; point to sprint-plan rev 0.59; retain NC-EXT-38/41 open |
| 1.76 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0212/0213, align NC range to 41 and retain guide-history findings open |
| 1.77 | 2026-10-13 | frick-fsw | Update plan pointer to rev 0.60 and current failed review references; NCs remain open |
| 1.78 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0214/0215 and point to sprint-plan rev 0.62; retain open findings |
| 1.79 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0216/0217 and point to sprint-plan rev 0.63; retain all NCs open |
| 1.80 | 2026-10-13 | frick-fsw | Record ledger correction and point to sprint-plan rev 0.64; NC-EXT-41 remains open |
| 1.81 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0218/0219 and point to sprint-plan rev 0.66; no NC closed |
| 1.82 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0221..0223; update plan pointer/range to rev 0.68 and NC-EXT-42 |
| 1.83 | 2026-10-13 | frick-fsw | Refresh current-review summary and NC-EXT-42 status after T13/metric corrections |
| 1.84 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0224..0226 and update plan pointer to rev 0.70; retain all findings open |
| 1.85 | 2026-10-13 | frick-fsw | Point to sprint-plan rev 0.71 and preserve latest independent review status |
| 1.86 | 2026-10-13 | frick-fsw | Correct duplicate review-status sentence, include T13 qualification limit, and point to sprint-plan rev 0.72 |
| 1.87 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0227 and point to sprint-plan rev 0.73; retain all NCs open |
| 1.88 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0228 and preserve T13 review/qualification caveats |
| 1.89 | 2026-10-13 | frick-fsw | Align active plan pointers to sprint-plan rev 0.74 after R4 finding |
| 1.90 | 2026-10-13 | frick-fsw | Record scoped reviews 0229..0231 and update active plan pointer to rev 0.75 |
| 1.91 | 2026-10-13 | frick-fsw | Align handover review-status summary with scoped reports 0229..0231; retain T13/NC caveats |
| 1.92 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0232 and point to sprint-plan rev 0.76; no NC closed |
| 1.93 | 2026-10-13 | frick-fsw | Prioritize candidate MPL-2.0 transition and record NC-EXT-43 gates; point to plan rev 0.77 |
| 1.94 | 2026-10-13 | frick-fsw | Record transition reviews 0235..0237, NC-EXT-44, and point to plan rev 0.78; no NC closed |
| 1.95 | 2026-10-13 | frick-fsw | Record T21 reviews 0238..0242; update current plan pointer to rev 0.82; no NC closed |
| 1.96 | 2026-10-13 | frick-fsw | Record explicit candidate-only corrections from R4/PA findings; point to sprint plan rev 0.84; NC-EXT-43/44 remain open |
| 1.97 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0244 findings, point to sprint plan rev 0.85, and retain NC-EXT-43/44/T21 gates open |
| 1.98 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0245 bounded SVG result; point to sprint plan rev 0.86 and retain all release gates |
| 1.99 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0246 DDJF range finding, point to plan rev 0.87, and retain NC-EXT-43/44/45 open |
| 2.00 | 2026-10-13 | frick-fsw | Record DDJF T1..T21 author correction, point to plan rev 0.88, and retain NC-EXT-45 pending independent review |
| 2.01 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0247 bounded DDJF pass; retain NC-EXT-45 pending PA and all other gates |
| 2.02 | 2026-10-14 | frick-fsw | Record owner authorization scope, PA-0248 unreadable-file finding and local correction; update S8-FU resume point to plan rev 0.89 |
| 2.03 | 2026-10-14 | frick-fsw | Record bounded R2 0249 result and R4/PA scope findings 0250/0251; correct active scope and update review status |
| 2.04 | 2026-10-14 | frick-fsw | Record scope/status re-review findings 0252/0253; clarify current resumption metadata, NC-EXT-43/45 scope, and pending PA review |
| 2.05 | 2026-10-14 | frick-fsw | Record follow-up review findings 0254/0255; clarify broader S8-FU gate versus latest scoped outcome; update T21 status history |
| 2.06 | 2026-10-14 | frick-fsw | Correct active review summary after 0254/0255, distinguish historical broad gate from scoped latest review, and record re-review status |
| 2.07 | 2026-10-14 | frick-fsw | Correct sprint-plan pointers, clarify current review delta, and extend NC-EXT-45 PA source trail |
| 2.08 | 2026-10-14 | frick-fsw | Record status follow-ups 0256/0257 and latest re-review state; keep all gates open |
| 2.09 | 2026-10-14 | frick-fsw | Record bounded no-new-finding status reviews 0258/0259; retain NCs and qualification gates open |
| 2.10 | 2026-10-14 | frick-fsw | Align active plan pointers to rev 0.95 and preserve bounded 0258/0259 limits |
| 2.11 | 2026-10-14 | frick-fsw | Record N21 development sequencing rule, NC-EXT-46 deferral, and sprint-plan rev 0.96 pointer |
| 2.12 | 2026-10-14 | frick-fsw | Record N22 test rule, NC-EXT-47, and sprint-plan rev 0.97 pointer |
| 2.13 | 2026-10-14 | frick-fsw | Record N21 wording propagation, N7 designated-agent commit/push authority, FRICK-REVIEW-0260/0261 reconciliation, NC-EXT-48..68, and N22 work-package state; point to sprint-plan rev 1.00 |
| 2.15 | 2026-10-14 | frick-fsw | Record FRICK-REVIEW-0263 and NC-EXT-75 for the SB overflow EVS functional correction; update active NC range and preserve delta-review/gate status |
| 2.16 | 2026-10-14 | frick-fsw | Update resumption pointer to sprint-plan rev 1.02 and record the SB functional correction as implemented but open pending full-cycle/independent verification |
| 2.17 | 2026-10-14 | frick-fsw | Record R1 follow-up correction: defer EVS listener callbacks to bounded TM-output dispatch, document explicit queue overflow accounting, and keep NC-EXT-75 open pending full-cycle/independent delta review |
| 2.18 | 2026-10-14 | frick-fsw | Restore synchronous ordinary EVS sends and scope deferred API to SB overflow; record exact test evidence and leave strict mutex nonblocking interpretation and NC-EXT-75 open |
| 2.19 | 2026-10-14 | frick-fsw | Record FRICK-REVIEW-0264 and NC-EXT-76; update resumption pointer to sprint-plan rev 1.04 and preserve the unresolved no-block/ground-forwarding conflict |
| 2.20 | 2026-10-14 | frick-fsw | Record initial R3 review FRICK-REVIEW-0265, corrected requirement boundaries, current SRS-REQ-072 trace count and sprint-plan rev 1.07; retain authority/profile and review gates open |
| 2.21 | 2026-10-14 | frick-fsw | Record R3 delta FRICK-REVIEW-0266 (continued development only), NC-EXT-77 count correction, NC-EXT-76 remaining loss semantics, and sprint-plan rev 1.09 |
| 2.22 | 2026-10-14 | frick-fsw | Record user-approved PAL queue try-put behavior, partial fan-out, SRS-REQ-001 update, current verification evidence/limitations and sprint-plan rev 1.10; retain NC-EXT-75/76 and SRS-REQ-072 profile/loss-reporting work open |
| 2.23 | 2026-10-14 | frick-fsw | Record isolated configured-limit SB/EVS saturation stress results, initial test-harness resource collision and resolution; preserve commit/push blockers and open event-envelope authority/loss-reporting gates |
| 2.24 | 2026-10-14 | frick-fsw | Record fresh clean structural-stress evidence and scoped R4/PA reviews; retain N14, NC-EXT-75/76, full-sprint and broader release gates as open |
| 2.25 | 2026-10-14 | frick-fsw | Record 9,984-profile SB/EVS host structural characterization, retained CSV/evidence identities, bounded R3/R2/R4/PA reports and final CI-local cycle; retain mission authority inputs, NC-EXT-75/76, T22 N10 acceptance and release gates as open |
| 2.26 | 2026-10-15 | frick-fsw | Record the committed/pushed state (`a4c436c`, `03773f7`, `620fff8`), green GitHub Actions run #83, and the `make docs` output-existence guard delta with scoped R4 FRICK-REVIEW-0285 / PA FRICK-REVIEW-0286 reviews (both APPROVED WITH COMMENTS; R4 NC-01/NC-02 verified closed by bounded delta re-review FRICK-REVIEW-0287); retain hosted-evidence (T21), NC-EXT-43/75/76/78..81 and release gates open |
| 2.27 | 2026-10-15 | frick-fsw | Record the committed guard delta (`77cd548`) and its green hosted GitHub Actions run #84 (all 8 jobs); Gitea run log unevidenced (NC-EXT-36); retain hosted-CI qualification evidence (T21), NC-EXT-43/75/76/78..81 and release gates open |
| 2.28 | 2026-10-15 | frick-fsw | Record hosted run #85 for the record-only commit `03366f1` (all 8 jobs) and the loop-audit N15 record-only convention that terminates the record → run → record regress; Gitea run log unevidenced (NC-EXT-36); retain hosted-CI qualification evidence (T21), NC-EXT-43/75/76/78..81 and release gates open |
| 2.29 | 2026-10-15 | frick-fsw | Resume point moved to HEAD 6ce6bd7 plus uncommitted S4c/S4d/default work; record focused and fresh-default host results, NC-S4c-01 limitation, ADR-0018 defaults and outstanding full-cycle/independent/hosted gates |
| 2.30 | 2026-10-15 | frick-fsw | Record completed local Debug/Release/ASan, Lean/trace, docs/format/smoke/GSW, RTEMS/Renode/QEMU cycle and update next steps to independent reviews and hosted CI; no commit/push or release approval claimed |
| 2.31 | 2026-10-15 | frick-fsw | Record 169-check CANopen test oracle completion, final count and clarified RS-485 release-call hold semantics; review and hosted gates remain pending |
| 2.32 | 2026-10-15 | frick-fsw | Complete exact response-frame assertions for absent-index CANopen download and update the final test count to 171; reviews and hosted CI remain open |
| 2.33 | 2026-10-15 | frick-fsw | Record final 171-check full verification cycle and completed bounded independent review set; retain open authority, dependency evidence, provenance, hosted-CI, target and vendor gates |
| 2.34 | 2026-10-15 | frick-fsw | Record scoped closures for NC-S4c-02 and NC-EXT-82, final T23 behavior review, and update next actions to remaining PA/R4, authority and hosted gates |
| 2.35 | 2026-10-15 | frick-fsw | Record R2 tracked-state follow-up 0299, index 0298 as point-in-time, and retain PA/R4/hosted/authority gates |
| 2.36 | 2026-10-15 | frick-fsw | Record PA tracked-state review 0300 and remaining R4 scope; retain T23 qualification, hosted, target/vendor, authority and provenance gates |
| 2.37 | 2026-10-15 | frick-fsw | Record bounded R4 status/evidence review 0301 and preserve remaining overall documentation, hosted, target/vendor and authority gates |
| 2.38 | 2026-10-15 | frick-fsw | Record final bounded R4/PA evidence reviews 0302/0303; keep phase/release and hosted/provenance/target/authority gates open |
| 2.39 | 2026-10-15 | frick-fsw | Record latest R4/PA timing chronology follow-ups 0304/0305 and keep all broader gates open |
| 2.40 | 2026-10-15 | frick-fsw | Record R4/PA timing text follow-ups 0306/0307 and retain open phase/release, hosted, provenance and authority gates |
| 2.41 | 2026-10-15 | frick-fsw | Reconcile current baseline to pushed HEAD 13ebf92; record green GitHub run 38793643786, prior CI formatting/metrics failures and correction, Gitea API verification limit, CANopen guard/test delta, and unresolved sprint precedence; retain NC-S4c-04 and broader gates open |
| 2.42 | 2026-10-15 | frick-fsw | Clarify committed baseline versus current uncommitted documentation delta, state that prior GitHub CI does not cover it, and avoid asserting historical N7 pre-push compliance without supporting records |
| 2.43 | 2026-10-15 | frick-fsw | Record initial scoped R4/PA reviews 0308/0309, their corrected findings and pending delta re-reviews; retain N7 and hosted-CI limits |
| 2.44 | 2026-10-15 | frick-fsw | Record bounded final re-review 0310 of the 0308/0309 wording corrections; retain uncommitted-delta, historical N7, Gitea, and broader review limits |
| 2.45 | 2026-10-15 | frick-fsw | Record the owner's sequencing direction: resume with S8-FU; keep NC-EXT-30 open pending independent R4 review of the plan change |
| 2.46 | 2026-10-15 | frick-fsw | Clarify S8-FU as this plan's interpretation of the owner's general instruction, not a verbatim owner selection; preserve pending NC-EXT-30 review |
| 2.47 | 2026-10-15 | frick-fsw | Reconcile current and prior resume passages to sprint-plan rev 1.26 and distinguish owner instruction from the plan's S8-FU interpretation |
| 2.48 | 2026-10-15 | frick-fsw | Record R4 review 0311 closure of NC-EXT-30/65 for bounded sequencing wording and align current sprint pointer to S8-FU |
| 2.49 | 2026-10-15 | frick-fsw | Record N22-TS1-96 as partial host evidence for NC-EXT-02, retain receive-loop limitation, and label older handover passage as point-in-time history |
| 2.50 | 2026-10-15 | frick-fsw | Record the production receive-helper test, V&V 0316 accepted bounded evidence, and remaining NC-owner/target/NC-EXT-03 limits |
| 2.51 | 2026-10-15 | frick-fsw | Record UART PDL framing, host PTY V&V 0319, current local verification results and NC-EXT-07 authority/target limits |
| 2.52 | 2026-10-15 | frick-fsw | Clarify NC-EXT-01/02 remain open until authorized owner dispositions are recorded; record current PA/R4 findings |
| 2.53 | 2026-10-15 | frick-fsw | Link S8-FU candidate manifest and retained logs; record R4/PA corrective delta and remaining review/authority blockers |
| 2.54 | 2026-10-15 | frick-fsw | Record R4 0323 and PA 0324 bounded acceptance; preserve NC-owner and broader N17/N20 gates |
| 2.55 | 2026-10-15 | frick-fsw | Reconcile exact NC-EXT-01/02 evidence wording and record current R4/PA bounded re-review plus candidate manifest pointer |
| 2.56 | 2026-10-16 | frick-fsw | Record fresh post-review CI-local, smoke, tools, docs, RTEMS cross-build and Renode results; S8-FU authority gates remain open |
| 2.57 | 2026-10-16 | frick-fsw | Record independent R1 UART findings, bounded corrections, unresolved malformed-PDL recovery and user's authority-blocked direction |
| 2.58 | 2026-10-16 | frick-fsw | Record V&V 0327–0329 case-mapping/count correction, SVA-2 update, current manifest and authority-blocked sprint status |
| 2.59 | 2026-10-16 | frick-fsw | Mark the 46-check UART summary historical, point to the current 51-check candidate manifest and reconcile active baseline claims |
| 2.60 | 2026-10-16 | frick-fsw | Record bounded R4/PA finding re-review acceptance and identify final applicable verification cycle as next action |
| 2.61 | 2026-10-16 | frick-fsw | Record final applicable local verification cycle and preserve authority/external/target blockers |
| 2.62 | 2026-10-16 | frick-fsw | Record physical-target test postponement, homepage disclaimer and remaining independent blockers |
| 2.63 | 2026-10-16 | frick-fsw | Record scoped R4/PA findings on target-deferral documentation; sprint header/effectivity corrected and latest cycle/re-review pending |
| 2.64 | 2026-10-16 | frick-fsw | Record bounded R4/PA re-review 0336/0337; final applicable verification and non-target sprint gates remain open |
| 2.65 | 2026-10-16 | frick-fsw | Record post-deferral local verification cycle and keep all authority/external/release gates open |
| 2.68 | 2026-10-16 | frick-fsw | Record in-progress S8-PUS-FU N7 adapter CRC/PDL correction, 42/42 host and ASan cycle, and controlled-SDK/emulation evidence limits |
| 2.69 | 2026-10-16 | frick-fsw | Record S3 provider-failure propagation, TS1-101, updated coverage and 42/42 cycle; retain S8-PUS-FU and independent-review gates open |
| 2.70 | 2026-10-16 | frick-fsw | Extend S3 TC oracle to provider and telemetry-encoding failures, refresh current evidence and coverage; no NC or sprint closure claimed |
| 2.71 | 2026-10-16 | frick-fsw | Record ADR-0019 default-N7 decision, actual generated-source host test, stale-cache cycle correction and current Renode/ASan evidence status; sprint remains incomplete |
| 2.72 | 2026-10-16 | frick-fsw | Record bounded R4/PA/V&V reviews 0338–0340, status reconciliation, and applied test/trace fixes pending delta review and fresh evidence cycle |
| 2.73 | 2026-10-16 | frick-fsw | Record full-batch reviews 0341–0343, S20 boundary test/CRC-oracle corrections, and the pending refreshed CI, candidate manifest and focused delta reviews |
| 2.74 | 2026-10-16 | frick-fsw | Record completed post-0343 local/ASan/auxiliary cycle, corrected current-cycle status and candidate-manifest identity pending PA acceptance |
| 2.75 | 2026-10-16 | frick-fsw | Reconcile R4 delta findings on review status and cycle counts; record bounded PA acceptance of enumerated product-set identity without phase/release approval |
| 2.76 | 2026-10-16 | frick-fsw | Record PA 0345 narrow closure of NC-EXT-83, leave NC-EXT-84 open, and update the active batch review/cycle status |
| 2.77 | 2026-10-16 | frick-fsw | Record exact generated-tree hash-list verification, reconcile NC-EXT-83/84 status and queue final R4 currentness delta |
| 2.78 | 2026-10-16 | frick-fsw | Mark pre-0343 42/42 PUS candidate narrative as historical and reconcile it with the current 43/43 post-0343 cycle and Renode evidence |
| 2.79 | 2026-10-16 | frick-fsw | Record PA 0346 generated-tree identity verification and R4 0347 narrow NC-EXT-85 closure; retain generator, authority, target and release gates |
| 2.80 | 2026-10-16 | frick-fsw | Record final R4 0347 NC-EXT-85 narrow disposition and update current status references; no overall N17 or phase approval |
| 2.81 | 2026-10-16 | frick-fsw | Record R1/R2 findings 0349/0350, S3 report-all first-error fix and new PUS test/trace deltas; full verification and delta reviews remain pending |
| 2.82 | 2026-10-16 | frick-fsw | Record focused PUS pass and retained compile/oracle attempts; update current candidate identity and full-cycle gates as pending |
| 2.83 | 2026-10-16 | frick-fsw | Record S8-PUS R1/R2/PA delta outcomes, S8-SCH R2 oracle corrections/re-review and NC-EXT-90 time-range residual; current cycle and R4 remain pending |
| 2.84 | 2026-10-16 | frick-fsw | Record R4 0357–0359 bounded documentation corrections and current CI-local state; auxiliary evidence and latest audit review remain pending |
| 2.85 | 2026-10-16 | frick-fsw | Record current candidate ASan/smoke/GSW/format/docs/Renode passes, host-tool failures and pending RTEMS cross-build/identity review |
| 2.86 | 2026-10-16 | frick-fsw | Record RTEMS 6.2 cross-build pass and 49-file candidate identity aid; preserve pending independent identity review, current CI rerun and target/hosted gates |
| 2.87 | 2026-10-16 | frick-fsw | Record PA 0361 bounded identity verification and latest CI-local identity-cycle pass; keep provenance, full review, authority, hosted and target gates open |
| 2.88 | 2026-10-16 | frick-fsw | Record R4 0362/0363 re-review of current audit/manifest evidence status and clarify current cycle and NC closure state |
| 2.89 | 2026-10-16 | frick-fsw | Record PA 0364 bounded final run-linkage review and current R4 gate for verification-results 2.82/status updates |
| 2.90 | 2026-10-16 | frick-fsw | Correct latest CI-local hash pointer per R4 0365; retain current bounded review/phase gates |
| 2.91 | 2026-10-16 | frick-fsw | Record R4 0366 confirmation of corrected final CI-local hash pointer; no aggregate N17/phase approval |
| 2.92 | 2026-10-16 | frick-fsw | Refresh immediate next steps after current reviews and local verification; record authority/target/OBCP blockers and prohibit commit/push pending gates |
| 2.93 | 2026-10-16 | frick-fsw | Record latest post-0366 CI-local log pointer and update verification-results revision 2.83 reference |
| 2.94 | 2026-10-16 | frick-fsw | Mark stale items 1–12 in §5 as historical/superseded per R4 0367 and direct resume to current item 15 |
| 2.95 | 2026-10-16 | frick-fsw | Record ADR-0020 provisional public-reference implementation boundaries and stale-vector failure correction; fresh full CI-local remains pending |
| 2.96 | 2026-10-16 | frick-fsw | Record corrected 43/43 CI-local/Lean/trace pass; clarify auxiliary cycle and candidate identity review remain pending |
| 2.97 | 2026-10-16 | frick-fsw | Reconcile R4 current-action and sequence-note findings; retain non-aggregate review and phase limits |
| 2.98 | 2026-10-16 | frick-fsw | Correct local-cycle leg effectivity and report current host/pinned formatting limitations, NC-EXT-91 and remaining auxiliary reruns |
| 2.99 | 2026-10-16 | frick-fsw | Record successful docs/Renode/RTEMS legs and corrected format recipe; retain pending recursive format/CI retry and exact-candidate identity gap |
| 3.00 | 2026-10-16 | frick-fsw | Record successful pinned format/CI retry, auxiliary gates, and PA narrow closure of NC-EXT-91; preserve broader identity, mission and release limits |
| 3.01 | 2026-10-16 | frick-fsw | Update current verification-results reference after PA narrow identity disposition; preserve broader candidate and release gates |
| 3.02 | 2026-10-16 | frick-fsw | Fix stale results revision pointer identified by scoped R4 review |
| 3.03 | 2026-10-16 | frick-fsw | Point current-cycle summary to post-reference CI-local rerun and verification-results 2.88 |
| 3.04 | 2026-10-16 | frick-fsw | Point current-cycle summary to post-loop-audit CI-local rerun and verification-results 2.89 |
| 3.05 | 2026-10-17 | frick-fsw | Process the four reconciliation reviews (0370 R2, 0371 R3, 0372 R4, 0373 PA), record ICD rev 1.4 provisional S3/S6 profile, NC-EXT-93, the corrected TS-1/TS-5/TS-6 §3 lists/histories and the sprint-plan 1.69 counts, and point the current-cycle summary to the 2026-10-17 reconciliation logs, the refreshed candidate manifest and verification-results 2.90 |
| 3.06 | 2026-10-18 | frick-fsw | Documentation-vs-implementation audit (N21 non-functional only): refresh the §4 committed-baseline fact to the pushed S8 batch `1fdbb83` and correct the flagged §4 hosted-CI wording; see `docs/reviews/2026-10-18-docs-implementation-alignment.md`. No NC closed and no gate, phase or release approval implied |
| 3.07 | 2026-10-19 | frick-fsw | Easy-fixes batch (N21): refresh the §4 committed baseline to the pushed `e7e48ee` chain and record the 2026-10-19 batch (corrected masked `N22-TS6-57` minimum-PDL sub-case, corpus anomaly context, CTest time budget, TS-5/TS-6 oracle wording, Service-1 emitted set `{1,2,3,5,7,8}` with NC-EXT-94 widened) plus the completed independent R2/R3 delta review (`FRICK-REVIEW-0379/0380`) and the R4 `NC-EXT-96` confirmation; register NC-EXT-97 (requirements authority), NC-EXT-98 (residual S8-VV-FU test evidence) and NC-EXT-99 (this batch's own N17/N20 and V&V-confirmation reviews deferred by explicit owner disposition to the next cycle). `GAP-R3-1` and `NC-EXT-25` remain OPEN; no NC closed and no gate, phase, qualification or release approval implied |
| 3.08 | 2026-10-19 | frick-fsw | Post-commit baseline refresh: repoint §4 from the pre-commit `e7e48ee` baseline to the pushed `318f49b` (easy-fixes batch committed and pushed to `origin` and `gitea`), identify the batch commit explicitly, and record hosted GitHub CI run `39218060325` (all eight jobs green for `318f49b`); no NC closed and no gate, phase or release approval implied |
| 3.09 | 2026-10-19 | frick-fsw | Declare the engineering-baseline convention (status-only commits do not advance it), rename the §4 record to the engineering baseline `318f49b` with `b6e1f61` as a status-only follow-up, and record hosted CI run `39219583424` (all eight jobs green for `b6e1f61`) alongside run `39218060325`; no NC closed and no gate, phase or release approval implied |
| 3.10 | 2026-10-19 | frick-fsw | Post-publication review integration (N12): the deferred batch-scoped N17/N20/N12 reviews were performed at HEAD 7e93325 (FRICK-REVIEW-0381 R4, 0382 R2/R3, 0383 PA; all APPROVED WITH COMMENTS, 0 Major); the bounded corrections were applied and each is marked delta-re-review pending (ASan duration, uncommitted label, TS-6 3.1 lwIP/ASan bullet, TS-5 oracle caveat, VP 9 S7/S8 rationale, manifest binding-commit, reviews/README status dated historical); NC-EXT-94 extended with the README-vs-ICD divergence and NC-EXT-99 updated to record the three performed legs; NC-EXT-100/101 registered. No NC closed and no gate, phase, qualification or release approval implied |
| 3.11 | 2026-10-20 | frick-fsw | Record the S8-FDIR-01 working-tree increment (panic/reset-recovery) as the current resume point: it addresses the outstanding NC-EXT-98/F-0379-03 reset-path work by adding the PAL panic seam and hardware-watchdog abstraction, the pre-reset PUS S5 report and the single reset path; registers NC-PAL-01 (unwired MPFS binding) and NC-EXT-102 (event-id catalogue); notes 7 reference apps (WD feeder) and the open gates/reviews. No NC closed and no gate, phase, qualification or release approval implied |
| 3.12 | 2026-10-20 | frick-fsw | Reconcile §5 with the live S8-FDIR-01 state: supersede the 2026-10-17 current-resume instructions, record HEAD `44d2af2`/engineering baseline `318f49b`, the R4 N17 review `FRICK-REVIEW-0386` REJECTED (1 Major, 4 Minor, 4 Comments), its still-open findings and the pending R2/R3/PA and N18/target gates; no finding or NC is closed and no approval inferred |
| 3.13 | 2026-10-20 | frick-fsw | Process the review-0386 state update: register findings as NC-EXT-103..111 in sprint-plan rev 1.75; correct the Major human traceability mirror (rev 0.51; `make trace` PASS), index FRICK-REVIEW-0386 in the review log, and record R4 delta re-review, R2/R3/PA, N18 and target/HIL as still open. No finding/NC closed and no approval inferred |
| 3.14 | 2026-10-20 | frick-fsw | Record bounded corrections for NC-EXT-104..107 and NC-EXT-110 across requirement/software-description histories, application-count docs and status.json; delta re-review and full local cycle remain pending, no NC closed or approval inferred |
| 3.15 | 2026-10-20 | frick-fsw | Record the actual partial N15 checks (trace/metrics/JSON/diff/GSW pass; 43/44 CTest plus a manual 9,984-profile sweep pass), the missing CTest Python path, Docker socket denial and restricted generated ASN.1 directory; controlled full-cycle rerun and all independent review/target gates remain open |
| 3.16 | 2026-10-20 | frick-fsw | Record R4 delta-review result and correction of M1 v0.1.0 app-count claim in development-plan.md; bounded recheck pending; all NCs, N15 and approval gates remain open |
| 3.17 | 2026-10-20 | frick-fsw | Record bounded recheck: historical M1 count correction confirmed, but exact flag spelling was corrected and stale CMake/app_renode_test comments still claim six current default apps; source-comment reconciliation must wait until OpenCode is paused; other gates remain open |
| 3.18 | 2026-10-20 | frick-fsw | Record source-comment-only correction after owner confirmed OpenCode idle: current seven default apps and optional FF_ENABLE_RENODE_TEST registration; diff check clean, no behavior or approvals changed |
| 3.19 | 2026-10-20 | frick-fsw | Refresh the S8-FDIR-01 resume point from the ordered host-gates log and T24 container dry-run; record NC-EXT-112 and the new NC-EXT-35 Lean-image evidence, with N15/reviews/target gates still open and no NC or gate closure |
| 3.20 | 2026-10-20 | frick-fsw | Index bounded independent R4 review FRICK-REVIEW-0387 and its four OPEN/deferred Minor findings under NC-EXT-28/35/112/113; do not apply the reviewed corrections or imply closure/approval |
| 3.21 | 2026-10-20 | frick-fsw | Record fresh current-tree RTEMS cross-build and prescribed N18 Renode PASS with evidence links; keep NC-TS-01, tool-provenance, N15, independent review, hardware and target/HIL gates OPEN |
| 3.22 | 2026-10-20 | frick-fsw | Correct the §5 resume summary per bounded R4 finding F-0387-01: reconcile the later ordered host run (2 PASS/5 FAIL), distinguish `make metrics` FAIL from metrics-panel/status checks, qualify the separate T24 PASS under open NC-EXT-113, and retain open N15/review/target gates; no NC or gate closed and no approval inferred |
| 3.23 | 2026-10-20 | frick-fsw | Record bounded N18 PASS and PA-identified unsupported PCIe target claim as NC-EXT-114 OPEN; retain emulator/physical-target boundary and all open approval gates |
| 3.24 | 2026-10-20 | frick-fsw | Triage independent R2 FRICK-REVIEW-0390: record NC-EXT-115 OPEN for inconsistent N18 UART captures, map SB oracle limitation to existing NC-EXT-47, update verification-results pointer to rev 3.00 and loop-audit to rev 10.20; keep N18 acceptance and all NC/release/target gates open |
| 3.25 | 2026-10-20 | frick-fsw | Record bounded R4/PA reviews FRICK-REVIEW-0391/0392 and their Minor currentness findings NC-EXT-116/117; note corrections in sprint-plan rev 1.83 with delta confirmation pending, and keep N18 acceptance and N15 gates open |
| 3.26 | 2026-10-20 | frick-fsw | Record R4 delta re-review FRICK-REVIEW-0393: confirm NC-EXT-116/117 currentness fixes, register NC-EXT-118 OPEN for stale correction instructions, and update sprint-plan pointer to rev 1.84; NCs, N18 acceptance and N15 remain open |
| 3.27 | 2026-10-20 | frick-fsw | Record R4 FRICK-REVIEW-0394 stale-pointer finding mapped to NC-EXT-116 OPEN; correct current N18 pointers to verification-results rev 3.00 and loop-audit rev 10.22 in handover and sprint-plan rev 1.85, with bounded R4/PA confirmation pending |
| 3.28 | 2026-10-20 | frick-fsw | Record R4 FRICK-REVIEW-0395 partial confirmation and correction of remaining §5 N18 pointer to verification-results rev 3.00; NC-EXT-116 remains OPEN pending bounded R4/PA confirmation |
| 3.29 | 2026-10-20 | frick-fsw | Triage R4/PA FRICK-REVIEW-0396/0397 and correct NC-EXT-116's handover revision reference to rev 3.28 in sprint-plan rev 1.87; NC-EXT-116 and N18/N15 gates remain open |
| 3.30 | 2026-10-20 | frick-fsw | Record bounded R4/PA confirmations FRICK-REVIEW-0398/0399 of NC-EXT-116's revision-reference correction; NC-EXT-116 remains OPEN pending authorized NC-owner disposition, with all gates open |
| 3.31 | 2026-10-20 | frick-fsw | Triage R4/PA FRICK-REVIEW-0400/0401 stale N18 pointers; update handover references to verification-results rev 3.00 and loop-audit rev 10.27, keeping NC-EXT-116 and all gates open pending confirmation |
| 3.32 | 2026-10-20 | frick-fsw | Record PA FRICK-REVIEW-0402 chronology finding as NC-EXT-119 OPEN; restore point-in-time loop-audit rev 10.22 for the rev 1.85/rev 3.27 state and distinguish the later current pointer rev 10.27; keep N18/N15 and all other gates open |
| 3.33 | 2026-10-20 | frick-fsw | Update current loop-audit pointer to rev 10.28 following audit refresh; record NC-EXT-119 pending R4/PA confirmation and retain N18/N15/open-gate boundaries |
| 3.34 | 2026-10-20 | frick-fsw | Record bounded R4/PA confirmations FRICK-REVIEW-0403/0404 of NC-EXT-119 history correction; retain NC-EXT-119 OPEN pending authorized owner disposition and all gates open |
| 3.35 | 2026-10-20 | frick-fsw | Refresh the active handover loop-audit pointer to rev 10.29; retain historical 10.22/10.27 references as point-in-time records and keep all NC/gate statuses open |
| 3.36 | 2026-10-21 | frick-fsw | Record the 2026-10-21 independent-review cycle for S8-FDIR-01: R2 `FRICK-REVIEW-0405` and re-review `0407`, R4 `FRICK-REVIEW-0408`, review findings NC-EXT-120..183 all OPEN, the R3/PA report publication gap (NC-EXT-141), the corrected Lean module (NC-EXT-120), the executed check counts with their basis, and N18 as **pending for the current tree** (NC-EXT-165); nothing committed, all gates open |
| 3.37 | 2026-10-21 | frick-fsw | Record owner-directed PAL/UART/deadline delta, fresh R1/R2/R3/R4 draft reviews 0409–0412, explicitly pending PA/N20 review, corrected Renode model inventory and public vendor-reference audit lead, exact verification outcomes and unresolved reporter/text/formatting and watchdog cadence/configuration blockers; no gate or NC closure |
| 3.38 | 2026-10-21 | frick-fsw | Final handover for this bounded increment: report review and test outcomes, external requirement/refresh-window blockers, uncommitted state and pending PA/N20/N18/Lean/smoke gates; no commit, NC closure, safety approval or release authorization |
| 3.39 | 2026-10-21 | frick-fsw | Post-review remediation handover for the same owner-approved increment: changed-file list, host build/test results and source identity (full CTest 44/44; targeted 320/157/68/113; trace and pinned format PASS), the retained R1 0414 / R2 0415 / N20 PA 0413 (not approved) outcomes, the four precise external blockers and two-to-three policy options with a recommendation for each. Loopback deadline-aware writer defect corrected; no commit, NC closure, gate pass, safety approval or release authorization |
| 3.40 | 2026-10-21 | frick-fsw | Final remediation handover for the same increment: the flight-target link defect found by PA FRICK-REVIEW-0418 (the increment's `poll()` usage, unsupported by RTEMS 6.2) corrected and re-verified by a fresh RTEMS 6.2 cross-build; the in-scope oracle and record corrections applied; the retained reviews 0416/0417/0418 and the NC register entries NC-EXT-184..190 added; host CTest 44/44 with targeted 320/172/68/113, trace and pinned format PASS, and the source-identity manifest extended to every changed file. The authority set remains open; no commit, NC closure, gate pass, safety approval or release authorization |
| 3.41 | 2026-10-21 | frick-fsw | Correction-round handover: the record-currency findings of FRICK-REVIEW-0419/0420 are applied (executed counts, Blocker attribution, TS-5 rev 1.12 with the strengthened exactly-one oracle, ADR-0021 rev 1.2, NC register NC-EXT-191/192, the 31-entry source manifest including the changed V&V scripts, and the cross-build transcript with the exact invocation and symbol checks). Host CTest 44/44 with targeted 320/180/71/113 and the RTEMS 6.2 cross-build linking cleanly. The authority set remains open; no commit, NC closure, gate pass, safety approval or release authorization |
| 3.42 | 2026-10-21 | frick-fsw | Handover acceptance verification: review chronology reconciled against the file timestamps, the post-0419/0420 corrections independently reviewed by a fresh narrow N20 review (FRICK-REVIEW-0421) and confirmed, the duplicate-emission mutation re-executed and retained as evidence, the exact RTEMS symbol-check expressions and the container invocation recorded, and the 0421 Major/Minor record findings corrected and registered as NC-EXT-193..196. Host CTest 44/44 with targeted 320/180/72/113, trace PASS, pinned format PASS, RTEMS link clean, manifest 31/31. The authority set remains open; no commit, NC closure, gate pass, safety approval or release authorization |
| 3.43 | 2026-10-21 | frick-fsw | Gate-verification phase: the canonical `make build` and `make smoke` gates executed and PASS with the existing pinned tools and an isolated build directory (the earlier third-party blocker no longer exists), the current-tree Renode ELF rebuilt for the N18 prerequisite, the Lean evidence re-verified source-identical and labelled this-cycle NOT RUN, and the UART drain-stop flag shown by ThreadSanitizer to be a C11 data race (registered OPEN/deferred as NC-EXT-197). N18 remains NOT RUN: the digest-pinned Renode image is absent from the local store and the prescribed runner would pull it. No commit, NC closure, gate pass, safety approval or release authorization |
| 3.44 | 2026-10-21 | frick-fsw | NC-EXT-197 correction phase: the UART drain-stop flag is now protected by a POSIX pthread mutex (the project requires C99, so C11 atomics are not used) with the mutex initialized before `pthread_create` and destroyed after `pthread_join`; the identical forced-failure ThreadSanitizer reproducer now reports zero races (before: 1), the exact pre-fix source is retained verbatim (sha256 `b6227e69…`) beside the post-fix source (`e5c85c4a…`), and independent review FRICK-REVIEW-0422 confirms the fix with no production, coverage, expected-result or sanitizer change. NC-EXT-197 remains OPEN for owner disposition; no commit, NC closure, gate pass, safety approval or release authorization |
| 3.45 | 2026-10-21 | frick-fsw | Renode/Lean phase after the owner authorization (Telegram 1397): the project-pinned Lean toolchain (`leanprover/lean4:v4.10.0`, Lean 4.10.0 commit c375e19f6b65) was installed into the dedicated cache `~/.cache/frick-fsw/elan` and the prescribed `lake build` now **PASSES** on the current tree; the digest-pinned Renode image was downloaded and digest-verified, the current-tree Renode ELF was rebuilt (sha256 `23a4c8da…`), and the N18 functional validation **FAILS** — the target panics at start-up (`watchdog: init failed`, RTEMS fatal exit 5) because the owner-approved arm-error propagation treats the unwired MPFS binding as fatal, so the `RENODE_TEST: PASS` marker is absent from this run's process log and UART console. The runner's own PASS verdict was a false PASS from a stale 2026-10-20 log (its `/tmp` log paths are not writable here). Registered as NC-EXT-198 (owner decision) and NC-EXT-199 (runner evidence integrity). No policy, cadence, binding or requirement change; no commit, NC closure, gate pass, safety approval or release authorization |
| 3.46 | 2026-10-21 | frick-fsw | NC-EXT-199 corrected and retested: the N18 runner uses a fresh per-run log directory and a fail-closed verdict unit (missing/unreadable/empty capture and failed directory/container/capture steps → FAIL), keeps the container status/exit code and the bounded-wait outcome visible, and distinguishes a target startup fatal exit from a timeout; the regression suite uses only genuine recorded runs, is registered with CTest and is mutation-verified (17 checks, 0 failures), and independent review FRICK-REVIEW-0423 confirmed the correction and its negative controls. The actual N18 run on the current tree remains **FAIL** (the target panics at start-up — NC-EXT-198), and the bounded integration proposal with two options is recorded in `docs/architecture/n18-watchdog-integration-proposal.md`. NC-EXT-198/199 remain OPEN; no commit, NC closure, gate pass, safety approval or release authorization |
| 3.47 | 2026-10-22 | frick-fsw | Watchdog specification phase (owner path A, ESC-9/ESC-10): FRICK-ARCH-0024 rev 1.1 produced with the reference inventory, the retained BSP inventory transcript, the verified facts/recommendations/missing-inputs separation, the corrected emulator and host test sets, the target/HIL legs including the NC-PAL-01 closure conditions, and the owner decision table D-01..D-15 (all OPEN); independently reviewed by FRICK-REVIEW-0424 (embedded safety) and FRICK-REVIEW-0425 (V&V) with their findings applied; the proposal's non-fatal variant withdrawn per the owner's rejection. Documentation only: no binding, register write, production change, policy decision, NC closure, gate pass, commit or push |
| 3.48 | 2026-10-22 | frick-fsw | Final-delta acceptance correction: the retained pinned-image model probe was re-attributed (0x08 = STATUS, 0x04 = CONTROL, `TIME` 0x0c unprobed — the TIME-width claim withdrawn, the key-sensitivity claim downgraded to "not established by the retained probe"), the case-placement reasons were made probe-accurate, the 250 ms restatement was re-attached to the report-path duration (new §5.4 item RP-1), the read-back ordering / `MSVP` wording / bound value / independence wording / current-vs-history wording were reconciled with the cited sources, the owner agenda was consolidated to A, B1/B2, C, D, E, F, and the §10 finding→correction itemisation was reinstated. Independent final-delta reviews FRICK-REVIEW-0426 (embedded safety) and 0427 (V&V) retained and indexed; the documentation identity manifest regenerated after the text freeze; the 34-entry code manifest verified, not re-derived. No implementation, register write, production change, policy decision, NC closure, gate approval, commit or push |
| 3.49 | 2026-10-22 | frick-fsw | Acceptance round 2: the NC-1 negative-control oracle was corrected to detect on the positive case HS-P2 (healthy control stated), HS-P5 was defined against the mock model's ordered write observer, and the §5.4 heading, the evidence index's proposal revision and a dangling §7.5 pointer were corrected. Official Microchip documents were retrieved from the issuer's public download path (MSS TRM rev S, Power-Up/Resets UG rev M, data sheet rev E, RT product overview rev C, the RTPFS160ZT-applicable errata rev A, AN4903) with URLs, HTTP status and SHA-256 retained; the new facts F-18..F-24 supersede the rev 1.3 instance recommendation (only WDOG0/E51 can reset the MSS; a U54 watchdog only interrupts the E51), add the ≈10.47 s power-up default timeout, the debug-halt/WFI behaviour, the 26-bit load field, the MSS reset scope and the WDOG_RESET reason bit, and record the absence of a published watchdog erratum. R-1 withdrawn; agenda C/D/E/F reframed; G-A narrowed; HL-8/HL-9 added. No parameter adopted, no implementation, register write, production change, NC closure, gate approval, commit or push |
| 3.50 | 2026-10-22 | frick-fsw | Acceptance round 2 corrections and the two **confirmation addenda** of rev 1.4 (FRICK-REVIEW-0428 embedded safety, FRICK-REVIEW-0429 V&V, both verifying the reviewed body identity by SHA-256 and retained/indexed): the corrupted §1 status bullet repaired; the review-coverage attribution corrected (rev 1.2 reviews and rev 1.4 confirmations distinguished); the companion proposal aligned to rev 1.5 (the superseded instance selection withdrawn — only `WDOG0`/E51 can reset the MSS); the host mock's key-checked reload semantics and observables declared as a precondition of the host suite; the HS-P5 oracle relaxed to a relative-order requirement with its stimulus's delivery mechanism stated; the §5.1 emulator row split (sequence vs order) and the three host cells without a host case marked; `RESET_SR`/`DEVRST_N` added to the retained retrieval record; the §5.4 leg↔quantity mapping added; the HS-P1/HS-P2 inputs pinned; the target-only heading scoped; the §2.3/F-21/F-24 wordings and the labelled inference's assumptions corrected; the editorial defects corrected (§2.4 order, history order, §9 duplicate row, stray table row, the stale manifest entry count). The documentation identity manifest is regenerated after the text freeze; the 34-entry code manifest is verified, not re-derived. New NC-EXT-205 records the three host cases still to be written. No parameter adopted, no implementation, register write, production change, NC closure, gate approval, commit or push |
| 3.51 | 2026-10-22 | frick-fsw | Final **read-only acceptance checkpoint** of the frozen rev 1.5 bodies: the three frozen artefacts were verified by SHA-256 and confirmed by two short independent addenda (FRICK-REVIEW-0430 embedded safety, FRICK-REVIEW-0431 V&V), each scoped to the 1.4→1.5 delta and each stating that it does not extend the rev 1.4 addenda; no design defect was found. The arithmetic annotation slip in the retrieval log's labelled inference (16 777 200 = 0xFFFFF0, not 0xFFF3D8) and the structural/metadata observations are recorded as OPEN/deferred (NC-EXT-206; a wrong-key-class residual added to NC-EXT-205) and the **frozen bodies were not modified**. The checkpoint record is retained. No production, source, register, probe, build, model, configuration or mission change; no parameter, clock, timeout, instance or reset policy adopted; no NC or gate closed; no commit or push |\n
| 3.52 | 2026-10-24 | frick-fsw | Add the S8-PUS-FU HK periodic-emission addendum (NC-EXT-17): bounded increment, reproduced-and-fixed `time_add_period` double-count defect, final candidate hashes, gate results with N18 still FAIL, five retained review reports, and the do-not-activate conditions (NC-EXT-207/208, ESC-HK-01/02/06) |
| 3.53 | 2026-10-24 | frick-fsw | Add the NC-EXT-18 S6 refusal-cause addendum: reproduced defect, minimal resolution/range split, strengthened content oracle with mutation proof, final hashes, gate results with N18 still FAIL, and the authority-owned residuals (wire failure codes, TC[6,5] profile, authorization model) |
| 3.54 | 2026-10-25 | frick-fsw | Add the NC-EXT-207 S3 periodic due-test wrap/TM-flood addendum: software-separable correction implemented (`time_period_due` signed-modular formulation), reproduced defect (10 failures), three new tests with exact oracles + mutation discrimination, all gates PASS (build/test/trace/Lean/format/GSW), N18 FAIL preserved, authority half (ESC-HK-06/03, NC-EXT-208) remains OPEN, no commit/push |
