# FRICK-REVIEW-0419 — S8-FDIR-01 correction round, N20 product assurance review

**Profile:** PA / N20, fresh independent context  
**Disposition:** **Not approved.** Not a phase, gate, qualification, safety or
release approval. No NC is closed, no gate is passed and no residual risk is
accepted by this review.

**Product:** uncommitted working tree, HEAD `44d2af224675cd65a66d2870eb0fc9553952799c`,
engineering baseline `318f49b`.

> **Attribution note (added 2026-10-21, acceptance verification).** This report is the independent review of the S8-FDIR-01 increment **as of the tree state at 2026-10-21T19:30Z**. Its findings were corrected by the implementing agent in the follow-up correction round; the "corrected" entries below record that disposition and the reviewer's own verification of the correction where it was performed. This report is **not** a fresh independent review of the corrected product as a separate delta. The duplicate-emission mutation was executed on a scratch copy and was not retained as evidence at the time; it was re-executed and **retained** in `docs/vv/evidence/2026-10-21-s8-fdir-post-review-remediation.log` during the 2026-10-21 acceptance verification, and the corrected product was independently reviewed once more (see the acceptance review in `docs/reviews/`).

## Independently reproduced

| Check | Result |
|---|---|
| Flight-target link | RTEMS 6.2 cross-build from the current tree with the cached `frick-rtems-toolchain:local` image (`--pull=never`): configure and build both exit 0, artifact produced, **no POSIX `poll`/`select` symbol**, **no `FF_TEST_HOOKS` seam** exported |
| Host evidence | full CTest 44/44; targeted counts reproduced exactly; `make trace` PASS; pinned `format-check` PASS (clang-format 18.1.3, exit 0) |
| Source manifest | every entry verifies OK against the current tree |
| Lean evidence | the 14 Lean source hashes re-verify and the recorded `Watchdog.olean` digest matches the artefact in the tree; no record claims a Lean run for this cycle |
| Gate/review honesty | every gate yellow except `ci_github`; no target WCET, target/HIL, hardware-watchdog or flight-readiness claim in any record; the pinned-image pull and the `lake --version` probe effect are disclosed with no cleanup performed |

## Findings

### M-01 — Major (records): the CHANGELOG published a superseded executed count and a superseded writer description

The delta's own CHANGELOG section still published the pre-remediation
`test_es_hs_lifecycle` count and described the console writer as
"poll-driven" — the implementation the increment had removed because RTEMS 6.2
implements no `poll()`. **Corrected in the follow-up round:** the count and the
description now match the current tree and the current implementation.

### M-02 — Major (records): the loop-audit current-state row published a superseded count

The milestone row carried the pre-remediation figure while the N22 row of the
same file carried the current one. **Corrected in the follow-up round.**

### M-03 — Major (records): the flight-target link Blocker was mis-attributed

The N20 norm row, the loop-audit history row and the NC register attributed the
Blocker to FRICK-REVIEW-0416, which is the R1 report and contains no Blocker;
the Blocker is FRICK-REVIEW-0418's B-01. **Corrected in the follow-up round**,
and the NC register now resolves to the PA report.

### M-04 — Major (evidence identity): the post-correction flight artifact was not retained, and the cross-build evidence was a summary

The tree still held the 2026-10-17 pre-delta binary, and the retained
cross-build evidence carried summary annotations rather than the invocation,
the toolchain identity and the raw symbol output. **Corrected in the follow-up
round:** the transcript now records the exact invocation, the configure and
build exit codes, the artifact size and SHA-256, and the exact symbol-check
expressions with their (empty) results.

### m-01..m-08 — Minor

The superseded CTest transcript still named a superseded transcript as
authoritative for the current tree (m-01); the shared UART descriptor mode
change races with the normal telemetry path and the IP/TSN backends deliver no
pre-reset report (m-02, registered as NC-EXT-191); the "smoke is blocked"
status describes a transient configure artefact rather than a standing
condition (m-03); the handover cited a TS-5 revision that did not exist
(m-04); the loop-audit N11 row understated the retained review set (m-05); three
of this review's own predecessor's minor findings were not in the NC register
(m-06, registered as NC-EXT-192); the source manifest excluded the changed V&V
tool scripts (m-07, now included); and the "no `poll`/`select` symbol" claim
was not reproducible from the transcript (m-08, now recorded with the exact
symbol-check expression and its empty result).

### Observation

ADR-0021 rev 1.1 still stated that the APIs do not support the shared
deadline. **Corrected in the follow-up round** (ADR-0021 rev 1.2), which also
records the writer design and the removal of the `poll()`-based wait.

## Disposition

**NOT APPROVED.** The product corrections are verified: the flight target
links, the artifact exports no POSIX `poll`/`select` and no test seam, the host
evidence reproduces exactly, and the in-scope oracle corrections are real and
now discriminating — the exactly-one-frame oracle was independently shown
(duplicate-emission mutation) to fail on the production wiring path, and the
console/UART descriptor-mode restore and the PUS service/subservice of the
pre-reset frame are now asserted.

The records-side findings were corrected in the follow-up round. What remains
open is the authority set, which this review cannot decide: the report
obligation and mandatory-text envelope (NC-EXT-184), the reset-path liveness
and the unbounded formatting work (NC-EXT-185), the 250 ms target and WCET
(NC-EXT-186), the watchdog cadence, refresh window and approved margin
(NC-EXT-187), and the unwired target watchdog protection (NC-EXT-188 /
NC-PAL-01).

**Confidence:** high for the link/artifact, host counts, manifest completeness
and record findings, each reproduced directly against the tree. Low for any
claim about target operation, target timing, watchdog reset behaviour or flight
suitability.

**Required human authority:** requirements/system and safety authorities for the
report obligation, mandatory-text envelope and bounded-formatting decisions;
avionics authority for the watchdog configuration, refresh window and cadence;
the authorized release authority for any later release decision.
