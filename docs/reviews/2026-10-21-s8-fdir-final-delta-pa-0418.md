# FRICK-REVIEW-0418 — S8-FDIR-01 final delta, N20 product assurance review

**Profile:** PA / N20, fresh independent context  
**Disposition:** **Not approved.** Not a phase, gate, qualification, safety or
release approval. No NC is closed, no gate is passed and no residual risk is
accepted by this review.

**Product:** uncommitted working tree, HEAD `44d2af224675cd65a66d2870eb0fc9553952799c`,
engineering baseline `318f49b`.

## B-01 — Blocker (target product integrity): the flight target did not link

The owner-directed delta introduced `poll()` into the RTEMS-compiled sources
(`pal_posix.c`, `link_uart.c`). RTEMS 6.2 provides the FreeBSD-derived
`poll.h` **types** but implements no `poll()` function, so the flight target
could not be linked.

Verified three ways: an exhaustive symbol scan of every archive in the pinned
toolchain image found no `poll` definition; both project objects carry an
undefined `poll` reference under the project's own target flags; and the real
project cross-build failed with `undefined reference to 'poll'` for
`pal_posix.c` and `link_uart.c`. The last successful RTEMS flight executable in
the tree predated the increment, so the tree that passed N18 on 2026-10-20 could
no longer be linked for the target.

**Disposition: corrected in the follow-up round, and re-verified.** The bounded
console and UART writers no longer use `poll()`/`select()`; they use a
non-blocking descriptor with a short back-off and a retry against the shared
deadline, and restore the descriptor mode afterwards. A fresh RTEMS 6.2
cross-build with the cached `frick-rtems-toolchain:local` image (`--pull=never`,
read-only source mount) now configures, links and produces `frick_fsw`; the
artifact exports no `poll`/`select` symbol and none of the `FF_TEST_HOOKS` test
seams. This is a build/link result only — not a target execution, timing or
watchdog result.

## M-01 — Major (evidence identity): the source-identity manifest was too narrow

The manifest presented as the source identity covered 15 of 24 changed files in
`src/`, `tests/` and `config/`, omitting the production wiring
(`src/main/fsw_main.c`), the unwired target binding (`pal_rtems.c`), the
`send_until = NULL` backends, the source of the published `test_time` count and
the mission tables. An edit to any of them would have left the manifest
verifying while the published results no longer corresponded to the tree.

**Disposition: corrected.** The manifest now covers every changed file in
`src/`, `config/`, `tests/` and `verification/lean/` plus `CMakeLists.txt` and
`Makefile`.

## M-02 — Major (evidence traceability): the pinned format-check claim had no retained transcript

Six records asserted a pinned `frick-vv:local make format-check` PASS while the
cited transcript carried no container run, no formatter-version identity and no
exit code for that leg. The claim was true — the reviewer reproduced it
independently (clang-format 18.1.3, exit 0) — but it was evidenced nowhere.
This is the NC-EXT-131 defect class recurring, and the new transcript also
dropped the per-section `exit=` discipline of the earlier gates log.

**Disposition: corrected.** The post-review transcript now carries the pinned
formatter run with its version identity and exit code, and the loop-audit,
tool-qualification, verification-results, handover, status and changelog records
cite it.

## M-03 — Major (records): the N19 loop-audit norm rows were stale

The N11, N12, N15, N20 and N22 rows still described the state before the
owner-directed delta: reviews "through FRICK-REVIEW-0408", R3 as unreviewed,
the review legs pending, and the N20 row ending at PA 0404.

**Disposition: corrected.** The rows now record the retained reviews through
0416, the new NC register entries, the current gate outcomes and the
not-closed N20 disposition.

## M-04 — Major (records): the live sprint row was stale

The normative S8-FDIR-01 sprint row still published the pre-remediation counts
(298/109), pointed at the pre-remediation transcript, and stated that the R3
and PA reports were not retained.

**Disposition: corrected.** The row now carries the current counts, cites the
current transcript and states that the R3/PA legs are reviewed but not
satisfied, with the findings registered in NC-EXT-184..190.

## M-05 — Major (N12 traceability): the 0409-0416 findings were in no NC register

The sprint plan is the NC register. Its S8-FDIR-01 entries cited 0405-0408
only, so the findings of 0409-0416 — including the safety Blocker — had no
owner, no closure condition and no visible status.

**Disposition: corrected.** NC-EXT-184..190 are registered with owner and
closure condition. This review assigns no NC identifiers and closes none.

## m-01..m-05 — Minor

The gates log carries no supersession banner of its own (m-01); the
`O_NONBLOCK` side effect on the shared UART descriptor is undocumented
(m-02, now restored by the writer and recorded); the recorded smoke blocker
describes ignored generated directories that are not present in the current
tree, so it is transient rather than standing (m-03); the loopback deadline is
honoured on entry only (m-04, corrected); the IP and TSN backends deliver no
pre-reset report (m-05, recorded as a limitation).

## Honesty checks — passed

The pinned-image pull is reported honestly and in the right place, with no
cleanup performed and `--pull=never` used afterwards. The `lake --version`
probe effect is disclosed in the same terms, and no Lean result is claimed for
this cycle. Every gate stays yellow except `ci_github`; no target WCET,
target/HIL, hardware-watchdog or flight-readiness claim appears anywhere. The
review outcomes are recorded accurately, with the NC register and the
authority items still open.

## Disposition

**NOT APPROVED.** The flight-target link blocker is corrected and re-verified;
the evidence-identity, traceability and record-currency findings are corrected.
What remains open is the authority set, which this review cannot decide or
accept: the requirement semantics of the pre-reset report and its mandatory
text envelope (NC-EXT-184), the reset-path liveness and the unbounded
formatting work (NC-EXT-185), the target timing and WCET (NC-EXT-186), the
watchdog cadence, refresh window and approved margin (NC-EXT-187), and the
unwired target watchdog protection (NC-EXT-188).

**Confidence:** high for the identity, count, TS-5 re-count and record-content
findings, and for B-01 (reproduced three independent ways, including the real
project cross-build). Low for any claim about target operation, target timing,
watchdog reset behaviour or flight suitability.

**Required human authority:** requirements/system and safety authorities for the
report obligation, mandatory-text envelope and bounded-formatting decisions;
avionics authority for the watchdog configuration, refresh window and cadence;
the authorized release authority for any later release decision.
