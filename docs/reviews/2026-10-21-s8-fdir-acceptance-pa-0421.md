# FRICK-REVIEW-0421 — S8-FDIR-01 handover-acceptance verification (N20)

**Profile:** PA / N20, fresh independent context  
**Disposition:** **Not approved.** Review evidence only. No NC closed or
re-opened, no gate passed, no requirement-policy or cadence decision, no
residual-risk acceptance, no release or target-WCET claim.

> **Post-review correction (added 2026-10-21, gate-verification phase).** The
> statement in item 3 that "the drain thread cannot hang the case" is
> **superseded**: the stop flag is a field of a main-thread stack object passed to
> `pthread_create`, written by the main thread and read by the drain thread with no
> mutex, C11 atomic or condition variable, so `volatile` provides no inter-thread
> synchronization. ThreadSanitizer reproduces the data race (read at
> `test_link_uart.c:97` versus write at `test_link_uart.c:304`); it lies on the
> send-failure path only, so the nominal suite run does not exercise it. The
> finding is registered OPEN/deferred for owner triage as **NC-EXT-197**, with the
> retained transcript in
> `docs/vv/evidence/2026-10-21-s8-fdir-gate-verification.log`. The 120-run
> observation of no hang remains true as an observation, not as a guarantee.

**Product:** uncommitted working tree, HEAD `44d2af224675cd65a66d2870eb0fc9553952799c`,
engineering baseline `318f49b`; 111 changed entries at review start and end.
The reviewer modified nothing in the repository; all builds and mutations ran
in a throwaway out-of-tree copy.

**Narrow scope:** the corrections made **after** FRICK-REVIEW-0419/0420, which
had therefore not been independently reviewed, plus the source/evidence
identity.

## 1. Poll-free bounded writers — verified

No `poll`/`select`/`ppoll`/`pselect` call or symbol remains in the project
sources or in the flight artifact. The retry is bounded by the shared absolute
deadline (re-checked at each loop iteration; the back-off is a 1 ms
`nanosleep`, so the overshoot is bounded by one back-off interval). The
descriptor mode is restored on every exit path — the only early return is taken
when the mode was not changed. The reset request remains unconditional and
outside the notification guard, so a failed notification cannot prevent it.
The residual (busy retry on a descriptor shared with the telemetry/command
path; unverified target driver readiness semantics) stays registered as
NC-EXT-191 and was not re-opened.

## 2. Exactly-one-frame oracle — verified discriminating on the production path

Two directed mutations in a throwaway copy:

| Mutation | Result |
|---|---|
| The production direct writer in `FF_PusSendTmDirect` invoked twice (the writer is the production `FF_LinkSendUntil`) | the case **fails** at the popped-length equality assertion and at the PUS 5/3 decode assertions — i.e. on the **production wiring path** |
| The loopback `send_until` sending twice | the case fails likewise |
| **Control:** the pre-correction oracle (decode and popped-length assertions removed, "second pop finds nothing" only) with the duplicate-emission mutation still applied | the case **passes** |

The control run is decisive: before the correction, a duplicated emission on
the production path was not detected by this case; after the correction it is.
The mutation has since been **re-executed and retained** in
`docs/vv/evidence/2026-10-21-s8-fdir-post-review-remediation.log` (acceptance
block), so the claim is re-verifiable from the tree.

Stated limitation (not a defect): the oracle covers the loopback backend; it
detects more than one packet concatenated on the ring and cannot detect a
duplicate that was dropped by the ring, or one emitted on another backend.

## 3. UART case — verified

The partial-frame observation is present and a genuine partial emission was
measured (13,824 of 60,000 bytes before the deadline). The descriptor-mode
restore is asserted and verified (`O_NONBLOCK` clear). The drain thread cannot
hang the case: the master descriptor is non-blocking, the drain loop exits on
the stop flag, and the flag is set before the join whenever the send failed;
120 consecutive runs showed no failure and no hang, and CTest bounds the case
at 5 s.

## 4. Source/evidence identity — verified

The 31-entry manifest verifies 31/31 (exit 0) and its coverage is exact in
both directions against the changed files of `src/`, `config/`, `tests/`,
`verification/lean/`, `tools/` plus `CMakeLists.txt` and `Makefile`, including
the two changed V&V scripts. The retained transcript contains the final
markers: CTest 44/44, targeted 320/180/71/113, trace PASS, `format-check`
exit 0 with clang-format 18.1.3, and the RTEMS configure/build exit 0. The
reviewer independently reproduced all of them on the current tree, including
the RTEMS cross-build, whose artifact exports no POSIX `poll`/`select` and no
`FF_TEST_HOOKS` seam.

**Lean:** the retained 2026-10-21T09:58Z evidence re-verifies against the
current sources (14/14 hashes) and the recorded `Watchdog.olean` digest
matches; it is valid for the current tree but is **not a Lean run of this
cycle**.

## 5. Attribution of FRICK-REVIEW-0419/0420

The post-fix statements in those reports are factually accurate, but their
tense could be read as reviewed content that was not reviewed at that
revision. Correct attribution wording was applied to both reports and to the
four records repeating their conclusions: those reports are the independent
reviews of the increment **as of the 19:30Z tree state**; their findings were
corrected by the implementing agent in the follow-up round; they are **not** a
fresh independent review of the corrected product, which the acceptance review
(0421) now is.

## 6. Findings

| ID | Severity | Location | Finding | Disposition |
|---|---|---|---|---|
| F-01 | Major (records) | `docs/planning/sprint-plan.md:99` | The live S8-FDIR-01 sprint row published superseded executed counts (172/68) while the final block of the evidence log it cites publishes 180/71. | **Corrected in the acceptance round**; registered as NC-EXT-193 (the NC owner decides any re-opening). |
| F-02 | Minor (records/evidence) | evidence log; 0419 M-04/m-08 | The transcript carried summary annotations instead of the exact symbol-check expressions, and the acceptance block lacked the executed results. | **Corrected**: the invocation, toolchain identity, exact `nm` expressions and their empty results are now retained. NC-EXT-194. |
| F-03 | Minor (records) | `docs/adr/ADR-0021-watchdog-reset-recovery.md:62` | The ADR named the plain `FF_LinkSend` as the direct writer while the wiring installs `FF_LinkSendUntil`. | **Corrected** (writer of record stated). NC-EXT-195. |
| F-04 | Minor (evidence) | four records | The duplicate-emission mutation was published as a project fact with no retained artefact. | **Corrected**: the mutation transcript is now retained. NC-EXT-194. |
| F-05 | Minor (oracle) | `tests/unit/test_link_uart.c:335` | The stall sub-case asserted only "fewer bytes than requested", which also passes when nothing is emitted. | **Corrected**: the partial emission is now asserted explicitly. NC-EXT-195. |
| F-06 | Minor (build configuration) | `tests/unit/test_link_uart.c` | The descriptor-restore assertion was compiled unconditionally although the descriptor was resolved only under the link-time-wrapper guard. | **Corrected**: the guard now matches. NC-EXT-195. |
| F-07 | Minor (robustness) | `tests/unit/test_link_uart.c` | The drain thread's stop flag was read and written across threads without synchronization. | **Corrected** (`volatile`); the target-side exposure remains under NC-EXT-191. NC-EXT-195. |
| F-08 | Minor (records) | `docs/planning/sprint-plan.md:309` | The NC-EXT-190 disposition described the weaker, non-discriminating form of the oracle. | **Corrected.** |
| F-09 | Observation | `build/rtems/frick_fsw` | The post-correction flight artifact is still not retained; only its size and hash are. The build is not bit-reproducible (NC-EXT-40). | Recorded as NC-EXT-196; retaining a multi-megabyte artifact is a configuration-management decision for the owner. |
| F-10 | Observation | manifest vs report timestamps | The manifest carries a later timestamp than the reports citing it, inverting the audit order. | Recorded as NC-EXT-196. |
| F-11 | Observation | evidence log | The RTEMS results are summary lines, not raw unedited output; the file is an assembled transcript. | Recorded as NC-EXT-194. |

## 7. Explicitly UNRUN / unproven in this cycle (no claim made)

- **Lean build** — not run (the cached `frick-vv:local` has
  `/root/.elan/bin/lake` but no `/root/.elan/toolchains`); the retained Lean
  evidence of 2026-10-21T09:58Z re-verifies against the current sources and is
  valid for the current tree, but it is not a run of this cycle.
- **`make smoke`** — not run (pre-existing ignored generated
  `third_party/n7s-obcp` directories; nothing removed, refetched or
  re-permissioned).
- **N18 Renode functional validation** — not rerun for this tree
  (NC-EXT-165 remains OPEN).
- **Target execution / HIL / QEMU** — not run; no target WCET, target timing,
  hardware-watchdog, ground-receipt or flight-readiness claim is made.
- **Hosted CI** — not run; `ci_github` remains green from an earlier, unrelated
  run and is not evidence for this tree.
- **Requirement policy** (exactly-one semantics, mandatory-text envelope,
  formatter strategy) and **hardware cadence** remain owner decisions; nothing
  in this review decides them.

## 8. Disposition

**NOT APPROVED.** The four technical corrections in scope are real and correct
as described: the writers are poll-free, deadline-bounded and mode-restoring
with an unconditional reset request; the strengthened exactly-one-frame oracle
is demonstrably discriminating **on the production `FF_LinkSendUntil` path**,
proven in both directions and now retained as evidence; the UART observation
and restore assertions are real and the drain thread cannot hang; and the
source/evidence identity verifies exactly with the RTEMS flight link
independently reproduced clean of `poll`/`select` and of any test seam.

The one Major and six Minor record/oracle/configuration findings were corrected
in this round and are registered as NC-EXT-193..196. The authority set is
unchanged and open: NC-EXT-184 (report obligation and mandatory-text
envelope), NC-EXT-185 (reset-path liveness and the unbounded formatting work),
NC-EXT-186 (250 ms target and WCET), NC-EXT-187 (watchdog cadence, refresh
window and approved margin) and NC-EXT-188 / NC-PAL-01 (unwired target
watchdog).

**Confidence:** high for the writers, the oracle discrimination, the UART case
and the source/evidence identity, each reproduced directly against the tree.
Medium for the attribution judgement and the minor severity calls. Low for any
statement about target operation, target timing, watchdog reset behaviour or
flight suitability — no target evidence exists.

**Required independent human authority:** requirements/system and safety
authorities for the report obligation, mandatory-text envelope and
bounded-formatting questions; avionics authority for the watchdog
configuration, refresh window and cadence and the unwired binding; the NC owner
for the NC-EXT-192/193 dispositions; the authorized release authority for any
later release decision.
