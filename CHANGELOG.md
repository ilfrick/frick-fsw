# Changelog

Detailed revision history of frick-fsw (norm N16). Document-level histories are
maintained in each file's `## History` table; this file records the project
level history per release and per significant change, with the origin of every
change. All dates UTC, project language English (norm N2).

## [Unreleased]

### CI format-gate correction (2026-10-16)

- Restored the clang-format layout of `FF_LINK_IP_REF_PEER`, `FF_LINK_TSN_REF_MAC`
  and `MOCK_MAC_ADDR` to the form produced by the CI runner's clang-format 18.x.
  They had been collapsed by a local clang-format 23 run, which made the hosted
  `static analysis` job's "Format check" step fail (exit 123).
- Pinned formatting to the runner's clang-format major: `make format` and the
  new `make format-check` refuse to run when the local clang-format major differs
  from `FF_CLANG_FORMAT_MAJOR` (default 18), with an explicit override variable.
  The previous "clang-format passed" statement in the S8-FU entry was produced
  under a newer host clang-format and did not cover the CI gate.
- Refreshed the README metrics panel (review-stream counter 116 -> 117 after
  review 0339) which had failed the hosted `build / unit tests` job's
  "Metrics panel currency (norm N8)" step.

### S8-FU protocol ingest, GSW and UART framing corrective work (2026-10-15)

- Hardened CCSDS TC/TM packet-length checks and added symmetric boundary tests;
  factored production command-ingest receive into a testable one-packet helper
  and verified the maximum configured TC through the real helper.
- Added incremental bounded GSW serial-stream parsing and deadline-limited exact
  ping-response reading, with tests for split/coalesced input and timeout bounds.
- Implemented bounded CCSDS-PDL UART stream framing in the production backend;
  host PTY tests exercise partial/coalesced frames, too-small caller capacity,
  over-limit recovery, maximum packet, timeout retention, and ambiguous
  incomplete-header head-of-line behavior. V&V 0319 accepts only this enumerated
  host scope. NC-EXT-07 remains open for authorized timeout/recovery policy and
  target runtime/timing/hardware evidence.
- Corrected the GEM TSN mock fixture size for exercised offsets; focused ASan
  evidence is bounded to that fixture scope (V&V 0318).
- Final local candidate checks: `make ci-local` 37/37 CTest plus Lean/trace;
  smoke, GSW/license, metrics, docs, clang-format, Renode emulation and RTEMS
  cross-build passed. No hosted run covers this uncommitted candidate. Renode
  is emulation, not hardware verification; NC-EXT-03 external ground E2E and
  NC-EXT-07 authority/target items remain open. NC-EXT-01/02 have bounded V&V
  evidence accepted, but remain open pending authorized NC-owner dispositions.
- R4 FRICK-REVIEW-0323 and PA FRICK-REVIEW-0324 accepted their enumerated
  documentation/status corrections only. NC-EXT-01/02 remain OPEN pending
  authorized owner disposition; the scoped reviews do not pass N17/N20 phase
  or release gates.

### S8-FU post-review verification cycle (2026-10-16)

- Re-ran `make ci-local` on the current documentation candidate: 37/37 CTest,
  Lean and trace passed; retained the full output at
  `docs/vv/evidence/2026-10-16-s8-fu-ci-local.log`. Smoke, GSW/license checks,
  metrics, Doxygen generation, all-source clang-format, JSON validation,
  `git diff --check`, RTEMS cross-build, and Renode emulation also passed.
- These local results are not hosted CI, target runtime/timing, physical UART,
  external ground-segment E2E, qualification, release, or flight evidence.
  S8-FU remains open for authorized NC/profile decisions and applicable final
  independent review/assurance gates.
- R1 review 0325 findings were addressed for disconnected-UART zero reads and
  unique host PTY paths; R1 0326 accepted only those bounded corrections. R2
  reviews 0327–0329 corrected TS-2-81 to NC-driven characterization (no SRS
  credit) and reconciled 48 covered / 32 gaps / 1 characterization. R4/PA 0330/
  0331 findings were accepted as corrected by bounded reviews 0332/0333; final
  current-document verification remains due. NC-EXT-07's malformed plausible
  PDL recovery and all authority/target gates remain open.
- Re-ran the applicable local cycle after current-document reconciliation:
  `make ci-local` 37/37, Lean/trace, smoke, GSW/license, metrics, docs,
  formatting, JSON/diff checks, RTEMS cross-build and Renode emulation passed;
  focused UART PTY reported 51/0. See verification-results 2.53 and the
  candidate manifest for retained logs and hashes. This remains local evidence
  only; sprint, authority, external E2E, target and release gates remain open.
- Owner decision (2026-10-16): physical-target tests are postponed. Added a
  prominent README disclaimer that no RTPFS160ZT runtime/timing/physical-UART
  evidence is established; the deferral does not close target-dependent NCs.
  Scoped R4/PA reviews 0336/0337 accepted the metadata/effectivity corrections,
  and the applicable local cycle passed. NC-EXT-01/02/03 and NC-EXT-07 remain
  open; no overall sprint/release approval follows.

### S4c/S4d delivery and default configuration reconciliation (2026-10-15)

- Implemented and host-tested the generic fabric-window contract, fabric CAN-FD
  backend, documented allocation-free CANopen subset, RS-485 DE/RE control, and
  PUSopen adapter seam. CAN-FD tests use host MMIO mocks only because the
  available Renode platform has no fabric CAN-FD peripheral (`NC-S4c-01`); the
  PUSopen adapter tests use an in-repository API double and do not establish
  vendor SDK compatibility. CANopen reset commands are explicitly unsupported;
  complete CiA 301/302/402 conformance is not claimed.
- Selected the open CCSDS PUS backend by default, retained the N7/ESA ASN.1
  backend as opt-in, and enabled lwIP by default (`ADR-0018`). Added pinned-lwIP
  fetch-before-configure ordering for default CMake builds and forge workflows.
- Hardened reused dependency checkout validation to fail closed on a wrong
  pinned HEAD and tracked, untracked, or ignored working-tree content. Captured
  wrong-revision/tracked/ignored/untracked/clean-retry probes; R2/PA reviewed
  the bounded behavior (FRICK-REVIEW-0298..0300). T23 is not thereby qualified.
- Final local verification cycle: default Debug, Release and ASan CTest 36/36;
  focused current tests: fabric 119 checks, CAN-FD 328, CANopen 171, serial 67,
  PUSopen adapter 33; Lean and traceability passed. Renode/QEMU and cross-build
  checks passed with bounded evidence records. Independent reviews and scoped
  corrective deltas are recorded; NC-S4c-02 and NC-EXT-82 close only within
  their stated source/behavior scopes. Target/authority, vendor SDK, T23
  qualification, candidate provenance, hosted CI and broader release gates
  remain open; no release/qualification claim is made.

### CI documentation-job fix: create the Doxygen output directory (2026-10-15)

- Fixed `make docs` failing on a fresh checkout in the GitHub and Gitea CI
  `docs` jobs. Doxygen 1.9.8 creates `OUTPUT_DIRECTORY` with a single-level
  `mkdir(2)`, so `mkdir("build/doxygen")` failed with `ENOENT` when the parent
  `build/` did not exist, producing `error: tag OUTPUT_DIRECTORY: Output
  directory 'build/doxygen' does not exist and cannot be created`. The `docs`
  target now runs `mkdir -p build/doxygen` before `doxygen Doxyfile`, so
  `make docs` works from a fresh clone as stated in `AGENTS.md` section 3. The
  condition was not visible locally because earlier builds had already created
  `build/`.
- Provenance (regression origin): commit `7cb90a6` (2026-10-10, "ci(docs):
  create doxygen output dir before generating docs") fixed the same Doxygen
  1.9.8 error by adding `mkdir -p build/doxygen && doxygen Doxyfile` inline in
  both workflow steps; `a4c436c` replaced those inline steps with `make docs`
  while the `docs` target did not create the directory, so the CI `docs` job
  regressed. This change restores directory creation in the target.
- Evidence (pristine tree with no `build/`, Doxygen 1.9.8 in `frick-vv:local`,
  matching the GitHub `docs` job): reproduction of the original failure
  (`Output directory 'build/doxygen' ... cannot be created`, Doxygen exit 1 /
  `make` exit 2) and
  post-fix `make docs` exit 0 with `build/doxygen/html/index.html` present;
  documentation-set N8/N16 id-uniqueness check (248 unique document ids);
  reviewer-persona drift check; N13 license check; `make gsw-selfcheck`
  (5 GSW + 9 metrics + 14 license Python checks). Pre-existing Doxygen warnings
  are unchanged and non-fatal.
- CI metrics-currency gate (norm N8): the two new review reports added one
  review stream, so the README status panel was regenerated
  (`tools/scripts/metrics.py --readme`); review streams 32/78 -> 32/79 and
  `tools/scripts/metrics.py --check` now reports `metrics panel: current`.
  This was surfaced by hosted CI run #82 (`build` job, step *Metrics panel
  currency*), which the earlier local sweep did not exercise.
- This corrects a functional build defect (N21). It does not close hosted-CI
  evidence, T21 tool qualification, NC-EXT-43, or any release/qualification
  gate.

### `make docs` output-existence guard (2026-10-15)

- Hardened the `docs` target against a silent generation regression,
  dispositioning Comment 5 of the scoped PA delta review `FRICK-REVIEW-0284` (as
  the separate, scoped follow-up change that review requested). After
  `doxygen Doxyfile` the target asserts
  `test -s build/doxygen/html/index.html`, so a Doxygen run that exits 0 without
  emitting HTML fails `make docs` with an explicit message. The primary guard is
  in the `docs` target, so host, GitHub and Gitea invocations are all covered;
  the two workflow `docs` jobs additionally run `doxygen --version` and assert
  the same artifact, so the CI job also fails closed at the job level (defence in
  depth: the same existence check, downstream of the target's own).
- Sync obligation: `build/doxygen/html/index.html` now appears in three assertion
  sites and derives from `Doxyfile` (`OUTPUT_DIRECTORY` and
  `GENERATE_HTML = YES`), so a change to those settings must update all three
  together. The duplication fails closed (a mismatch aborts the `docs` job), so a
  missed update is loud rather than silent.
- Evidence (`frick-vv:local`, Doxygen 1.9.8): pristine tree (no `build/`)
  `make docs` exit 0 with `build/doxygen/html/index.html` (57,394 B); a tree with
  `GENERATE_HTML = NO` reproduced the silent failure mode (Doxygen exit 0,
  `warning: No output formats selected!`, no HTML) and the guard then failed
  `make docs` with exit 2 and the explicit message; removing `index.html` from a
  successful tree also fired the guard. `test -s` is POSIX (`/bin/sh`) and the
  preceding `rm -rf build/doxygen` prevents a stale artifact from satisfying it.
- The guard is existence-only (it checks neither HTML content nor the Doxygen
  warning set). It does not close hosted-CI evidence for this delta, T21,
  NC-EXT-43, or any release/qualification gate.
- Reviewed by scoped R4 `FRICK-REVIEW-0285` and PA `FRICK-REVIEW-0286` (both
  APPROVED WITH COMMENTS); the R4 currency actions NC-01/NC-02 were corrected
  in-delta and verified closed by the bounded R4 delta re-review
  `FRICK-REVIEW-0287`.

### SB/EVS requirements trace and audit correction (2026-10-14)

- Clarified SRS-REQ-072 that dispatch-rate and `Lmax` performance guarantees
  apply within the approved arrival/link performance envelopes, while
  exactly-once loss accounting applies independently of envelope compliance.
- Reconciled machine-readable and human-readable trace entries for
  SSS-REQ-006/SRS-REQ-011/072, including implementation/test references and the
  limitation that simulator/host characterization is not end-to-end receipt
  evidence. Mission values and post-acceptance loss-counter/reporting semantics
  remain TBD (NC-EXT-75/76).
- Re-audited the current working tree for N19: 234 tracked modified paths and
  120 untracked paths form a broad, mixed historical delta; no commit allowlist
  or complete sprint scope is established. No aggregate commit/push is approved.
- Registered the full T22 sweep runner in CTest; `make ci-local` now executes
  the 9,984-profile matrix and dispatch bounds (30/30 tests, sweep 18.70 s).
  The local CI-exercise gap and scoped R2/R4/PA reviews are closed; hosted CI
  and immutable evidence remain open, so no formal tool-qualification claim is made.
- Resolved the Doxygen cleanup permission issue by running the existing
  `make docs` target inside `frick-vv:local`, where the container can remove its
  root-owned prior output; generation completed with existing warnings.
- No new reviews were started in this correction pass. The broader R1/R2/R4/PA
  attempts were halted at user direction; their incomplete attempts are not
  approvals or findings.

### SB/EVS configured structural parameter characterization (2026-10-14)

- Added a standalone parameterized host oracle and runner sweeping all fan-out
  and queue-depth values 1–32 with zero overflow through the first configured
  EVS notification saturation boundary (9,984 profiles); dispatch bounds 1–64
  are checked with excess pending for bounds 1–63.
- Retained the full result matrix and build/source identities in
  `docs/vv/evidence/2026-10-14-sb-evs-structural-sweep.md` and `.csv`; 1,697,329
  assertion checks passed. The initial CSV formatter underflow defect was
  corrected; that malformed CSV was overwritten and is not retained separately.
- Published the user-facing structural characterization and explicitly kept
  mission arrival/service/link values and loss-detection/counter/reporting
  semantics TBD. This does not establish requirements compliance, mission
  performance, WCET, target timing or ground receipt; NC-EXT-75/76 remain open.
- Recorded scoped R3/R2/R4/PA reviews FRICK-REVIEW-0270..0273. T22 formal
  tool-owner N10 acceptance, hosted CI, RTEMS/target evidence and broader release
  gates remain open; no commit/push or NC closure is claimed.

### SB/EVS configured structural stress boundary (2026-10-14)

- Added an isolated host stress executable that holds all 32 subscriber queue
  locks for a publish, fills all 32 configured SB pipes at depth 32, then
  generates three 32-event overflow bursts while EVS dispatch is withheld. The
  test confirms the 32 busy-lock drops and deferred reports, 1024 queued SB
  copies, 96 full-queue drops/log entries, EVS's 64-entry dispatch saturation
  with 32 counted notification losses, and exact dispatch of 96 accepted
  notifications across the two phases. The test uses distinct wildcard-routed
  message IDs per overflow burst and checks both message-ID and application-ID
  ordering in the retained log suffix to prove newest-record retention.
- The case characterizes storage saturation only; it does not establish mission
  event-rate, WCET, ground-receipt, or SRS-REQ-072 guarantees. Numeric mission
  profiles and end-to-end loss reporting remain authority-owned TBDs; NC-EXT-75
  and NC-EXT-76 remain open.
- The first in-suite version failed because previous tests had consumed fixed PAL
  queue slots across service reinitialization. The stress case was moved to a
  standalone test process; full `make ci-local` then passed.
- Added fresh clean-build evidence, selected artifact hashes, and scoped R4/PA
  review records (FRICK-REVIEW-0268/0269). Reconciled TS-5/TS-6/SVA case totals,
  exploratory-status semantics, sanitizer wording, and stale local-coverage
  labels. These bounded reviews do not close NC-EXT-75/76 or broader gates.

### SB subscriber queue-mutex contention behavior (2026-10-14)

- Added portable `FF_PalQueueTryPut`: the internal queue mutex is acquired once;
  `FF_TIMEOUT` means mutex busy and `FF_BUFFER_FULL` remains the separate full
  queue result. `FF_SbPublish` skips only the busy pipe copy, increments dropped
  accounting, continues bounded fan-out, requests a deferred EVS error with a
  busy-specific message, and returns `FF_TIMEOUT`. A full pipe retains the
  existing `FF_SUCCESS` publish result when no queue lock was busy.
- Added deterministic held-queue-mutex coverage with two matched pipes, exact
  status/statistics/copy/event assertions and proof the call returns before the
  held mutex is released. Existing full-queue regression continues to exercise
  the distinct overflow event/status path.
- Updated SRS-REQ-001, architecture, TS-5/TS-6, traceability, and sprint status.
  SRS-REQ-072 end-to-end loss counter/reporting and approved profile remain
  unimplemented/open; authority-owned TBDs are unchanged. NC-EXT-75/76 remain
  open.

### Event-arrival and forwarding envelope requirements (2026-10-14)

- Refined SSS-REQ-006 and SRS-REQ-011, allocated SRS-REQ-072, and updated
  traceability and N22 cases to define EVS API arrival, acceptance/queueing,
  PUS-S5 link handoff, ground receipt, `A(Δ) ≤ Bmax + Rmax·Δ`, minimum dispatch
  service `Dmin`, maximum latency `Lmax`, and exactly-once loss accounting.
- `Bmax`, `Rmax`, `Dmin`, `Lmax`, service interval, burst/link profile,
  ground-receipt boundary and loss-counter semantics remain Systems/Operations
  TBDs. Queue=64, batch=8 and nominal 1000 ms delay are candidate settings only.
- FRICK-REVIEW-0265 did not approve the first requirements draft for baseline;
  FRICK-REVIEW-0266 approves continued development with comments only, not N14
  closure. NC-EXT-76 remains open pending authority values, loss-counter/report
  implementation and verification. NC-EXT-77 records a corrected trace-count
  comment; no NC closure is inferred.

### Bounded SB publish lock failure (2026-10-14)

- Added portable `FF_PalMutexTryLock` using POSIX `pthread_mutex_trylock` in the
  shared host/RTEMS POSIX PAL implementation. `FF_SbPublish` now returns
  `FF_TIMEOUT` before any SB statistics or pipe fan-out changes when the SB
  mutex is busy.
- SB overflow's explicit deferred EVS API also uses a non-waiting EVS mutex
  attempt. On contention it returns `FF_TIMEOUT` without modifying the event log,
  deferred queue, or notification statistics. Ordinary `FF_EvsSend` remains
  synchronous; listener callbacks remain outside locks and dispatch remains
  nonrecursive and bounded.
- Deterministic lock-hold tests verify exact status, fan-out, counters, EVS log,
  and notification statistics. SB success reports SB fan-out/accounting only;
  it does not claim successful EVS enqueue or ground forwarding.
- **NC-EXT-75/76 remain OPEN.** Queue saturation or EVS-lock contention can lose
  SB overflow ground notification, and the secondary EVS status is not propagated
  by SB. SRS-REQ-011's requirement owner must provide maximum event rate, maximum
  burst, minimum dispatch service, and maximum forwarding latency plus an
  authority-owned bounded overload policy. Mission values remain TBD; the later
  requirement delta is recorded above and is not yet a baselined profile.

### Software Bus full-pipe EVS correction (2026-10-14)

- Corrected `FF_SbPublish`: each full matching subscriber pipe still drops the
  new copy and increments `dropped`, and now raises `FF_EVS_SB_ERR` at
  `FF_EVS_ERROR` after releasing the SB mutex, using the explicit
  `FF_EvsSendDeferred` API. Ordinary `FF_EvsSend` retains synchronous listener
  delivery. The fixed-size pending-notification ring is dispatched by the
  TM-output task, at most eight callbacks per loop iteration; SB-overflow
  callbacks that republish to a full pipe enqueue later work without recursively
  nesting callbacks.
- Initialized EVS before SB in the flight startup sequence so the SB overflow
  path has a ready event log/listener service.
- On notification-ring saturation, EVS retains the event in its circular log,
  drops the listener notification, increments the EVS dropped-notification
  counter, and returns `FF_NO_RESOURCE` rather than claiming enqueue success.
- Added synchronous ordinary-send, full-pipe deferred-delivery,
  re-entry/non-recursive-dispatch and notification-saturation tests, including
  integrated SB overflow under EVS queue saturation; updated FRICK-ARCH-0002
  and TS-5/TS-6 oracles. Existing
  traceability already maps the SB and EVS tests to SRS-REQ-011. NC-EXT-75 and
  NC-EXT-47 remain open; no NC closure or qualification is claimed.
- R1 delta FRICK-REVIEW-0264 confirms listener work is deferred for SB overflow
  while ordinary EVS sends remain synchronous, but does not approve strict
  nonblocking behavior: PAL mutexes still wait forever (NC-EXT-75), and
  deferred-queue saturation can lose the SRS-REQ-011 ground forward (NC-EXT-76).
  Both functional issues remain open; no requirement is weakened by this record.

### Commit/push authority and N21/NC review-record reconciliation (2026-10-14)

- Recorded the user-directed N7 restriction: only `aeronautics-space-fsw-engineer`
  may commit/push after verifying all applicable gates/norms and full sprint
  scope; any exception requires explicit user delegation. Propagated to
  `docs/rules.md`, `AGENTS.md`, `CONTRIBUTING.md`, and `README.md`.
- Propagated the N21 missing-current/previous-sprint-feature wording to the
  rules enforcement summary, sprint sequencing rule, and README focus summary.
- Reconciled duplicate report IDs: R4 remains FRICK-REVIEW-0260; PA report is
  FRICK-REVIEW-0261. Added both to `docs/reviews/README.md`.
- Registered R4 follow-up NC-EXT-48..67 and PA report trace-count inconsistency
  NC-EXT-68 in the sprint register; mapped overlapping PA findings to existing
  NCs without claiming closure. New comprehensive test work packages remain
  uncommitted/unpushed pending applicable gates.
- Retained scoped R4 test-document review FRICK-REVIEW-0262 (APPROVED WITH
  COMMENTS); registered its six non-functional findings as NC-EXT-69..74.

### Comprehensive test specification and system verification architecture (2026-10-14)

- Added `docs/vv/comprehensive-test-spec.md` (FRICK-VV-TSPEC-001): the N22
  controlled test specification. Covers all I/Os individually and combined,
  nominal and out-of-bound conditions, all implemented features per the
  requirements/project goals, and software-only robustness tests (memory safety,
  overflow/underflow protection, defensive deserialization). Gaps mapped to
  NC-EXT-47.
- Added `docs/vv/system-verification-architecture.md` (FRICK-VV-SVA-001):
  system-level V-model levels, ICD-driven interface coverage, FDIR strategy,
  timing/determinism, environmental, security and tool-qualification
  architecture; authority-owned items recorded (NC-EXT-11/31, GAP-A4/A5/A6).
- Clarified N21: functional issues **and lack of required features planned for
  the current or previous sprints** are corrected in scope; other findings are
  deferred as open NCs without stopping development.

### Development governance: functional correction and test oracles (2026-10-14)

- Added N22: requirement-based and integration tests cover nominal and
  applicable out-of-bound operation and compare observed results with explicit
  expected results. Defined integration testing and clarified that Renode is
  an emulator (a possible test environment), not a test type; an aggregate
  marker alone is not requirement coverage.
- Logged NC-EXT-47 to assess existing test evidence against N22. The finding is
  open and does not stop unrelated development under N21.

### Project license transition (NC-EXT-43; status as of 2026-10-14)

- The repository owner authorized MPL-2.0 for original project-authored code
  and materials created for frick-fsw. Current N13/NOTICE/ADR records reflect
  that project decision; no further owner authorization is pending for this
  scope. This is not independent legal or provenance advice.
- Third-party, generated, mission-supplied, commercial, and other externally
  controlled content is excluded. NC-EXT-43 remains open for component
  boundaries and combined-distribution compatibility; distribution-dependent
  disposition and independent review remain required.
- T21 has a bounded R2 pass for the unreadable-managed-file correction
  (FRICK-REVIEW-0249), but remains NOT FULLY QUALIFIED. FRICK-REVIEW-0250..0257
  contain NOT APPROVED documentation/status findings; author corrections are
  pending independent re-review. No NC is closed and no release is approved.
- Historical AGPL-3.0-only releases and prior recipients' rights are unchanged;
  earlier transition audits remain point-in-time records and are not rewritten.
- Bounded current-status re-reviews FRICK-REVIEW-0258/0259 found no new finding
  in their respective scope; they do not close NCs, qualify T21, or approve a
  release.

### T13 metrics and status follow-up (NC-EXT-41/42)

- Clarified that review-stream counts are heuristic indicators, not NC-closure
  evidence; rejected contradictory negative, pending, open, and scoped verdicts.
- Added focused verdict-classification tests and regenerated the README panel;
  T13 remains not fully requalified pending independent review.
- Corrected design-traceability metric wording to describe an SDD-text proxy;
  NC-EXT-42 remains open pending independent disposition.
- Recorded FRICK-REVIEW-0218..0226 and retained all NCs and assurance gates open.
- Added explicit negative-suffix regression cases after FRICK-REVIEW-0227;
  the review-count panel remains a heuristic indicator, not closure evidence.
- Recorded FRICK-REVIEW-0228 NOT APPROVED; corrected review-summary and
  T13-specific test-count descriptions without closing NC-EXT-41.
- Clarified in generated metrics text that project-selected coverage targets
  do not establish an approved mission criticality assignment.
- Recorded scoped FRICK-REVIEW-0229..0232 outcomes; these do not qualify T13,
  close NCs, or approve the overall documentation/assurance phase.

### S8-FU documentation and review corrections (NC-EXT-35/38/39/40)

- Qualified T1/T18 reproducibility wording against mutable tool inputs and
  stepwise RTEMS image provenance; T1/T18 qualification remains open.
- Corrected `ci-local`/`check` scope descriptions across project and contributor
  documentation; listed separate applicable verification gates.
- Reconciled T7/T18 qualification status wording and current review/status
  records. No NC was closed; hosted CI and clean T18 rebuild/retest remain open.
- Clarified `ci-local` scope in CONTRIBUTING, ADR-0007 and driver checklist
  after FRICK-REVIEW-0208/0209; NC-EXT-38 remains open pending independent review.

### Independent-review follow-up corrections

- Corrected the ICD PDL diagram to show optional CRC bytes included in the
  packet-data length and identified sequence count as currently encoded zero
  pending the controlled profile decision (NC-EXT-06/26).
- Reconciled sprint-plan, handover and README wording: corrective order remains
  proposed, and the conflict with the earlier drivers-first directive awaits a
  human owner decision (NC-EXT-30; no precedence inferred).
- Clarified N14 report publication and rejected-review closure criteria to
  require all applicable review findings and gates, not merely Major findings
  (NC-EXT-32). All affected NCs remain open pending independent review.
- The R4/PA delta review identified a missing current independent R3 review for
  SSS-REQ-030; N14 is now recorded OPEN and NC-EXT-33 tracks the required review.
- Clarified the ICD as an internal reference profile pending mission-authority
  approval, corrected CRC generation/validation wording, and aligned PA/N11
  closure and evidence-record guidance; NC-EXT-13/32 remain open for authority
  disposition and independent verification.
- Added Graphviz to the controlled V&V image after Doxygen emitted missing graph-map
  errors. T1 requalification remains pending independent R2 review.
- Corrected the V&V Dockerfile header to disclose that mutable base/package
  inputs do not establish reproducibility; NC-EXT-35 remains open.

### Assessment follow-up planning

- The independent 2026-10-13 assessment findings C-04..C-16 that were not
  already represented in the NC register are now tracked as NC-EXT-14..24.
  Planned autonomous PUS corrections are grouped in S8-PUS-FU after S8-FU;
  the scheduler defect is tracked separately as NC-EXT-19 in S8-SCH/M17, and
  the PUS S11 stub remains NC-EXT-22 in M15. None of these NCs is closed by
  adding the plan. Existing GAP-A1..A14 remain open and are mapped to their
  work packages; authority, organizational-independence, target, and legal
  decisions are explicitly retained as open dependencies.
- Existing `GAP-R3-1` (ES/HS application tests) and the bounded decoder
  fuzz/oracle gap (`NC-EXT-25`) are proposed for S8-VV-FU. The proposed sequence
  is S8-FU → S8-VV-FU → S8-PUS-FU → S8-SCH/M17 → M12, but conflicts with the
  earlier drivers-first directive; owner precedence is unresolved. New NCs
  remain OPEN.
- Follow-up independent reviews FRICK-REVIEW-0174/0175 rejected/failed the
  mutable S8-FU delta. NC-EXT-28..31 track the status/evidence/sprint-order and
  severity-basis findings; NC-EXT-13 remains an authority-owned open policy
  issue with provisional severity. Review closure and dual-remote push remain
  blocked pending the full gate cycle and fresh passing independent reviews.
- Fresh delta reviews FRICK-REVIEW-0176/0177 also rejected/failed. Their
  findings are retained in the open NC register; the handover and audit identify
  the latest verdicts, and no closure, commit/push or release is authorized.
- Further reviews FRICK-REVIEW-0178/0179 rejected/failed; NC-EXT-32 records the
  review-record publication wording correction. Verification gates are
  conservatively yellow pending complete immutable configuration and output
  identity, even where the recorded test command passed.

### Added

- **S4b — PUS-C over IP/TSN (restored M4 scope; SRS-REQ-059/060/061).**
  Implemented bounded, allocation-free segmentation/reassembly for stream-oriented
  transports (`src/transport/link/link_seg.{c,h}`, SRS-REQ-059) with a 2-byte BE
  length-prefix framing; a reference TCP/IP `FF_Link_t` backend over
  `FF_NetStack_t` (`src/transport/link/link_ip.{c,h}`, SRS-REQ-060, UDP default
  with datagram framing, TCP with the seg layer) and a reference TSN-scheduled
  `FF_Link_t` backend over `FF_EthMac_t` + `FF_TsnBackend_t`
  (`src/transport/link/link_tsn.{c,h}`, SRS-REQ-061, Ethernet framing with
  EtherType 0x88F5, callback-driven RX ring). Reference defaults: UDP port 8080,
  peer 127.0.0.1; no VLAN tag, broadcast MAC. All backends selectable via
  `FF_LinkSelect`. Host unit tests: `test_link_ip` (10 seg + 9 IP = 19 test
  functions) and `test_link_tsn` (14 test functions), using a mock byte
  stream, mock net stack, and mock MAC + mock TSN backend. Build 27/27 green;
  trace PASS; Lean PASS; smoke + GSW PASS; `-Wall -Wextra -Werror` clean on
  authored files (N9/M9 quality gate). Updated `link.h` to declare the new backends; updated `traceability.yaml`
  and `SSS-REQ-028` from planned to implemented.
  Scope: no Renode image change (backends are host-testable only; on-target
  link-level behaviour = GAP-A5).

  **S4b delta re-review (NF-1/NF-2/NF-3 closure):** restructured
  `FF_LinkSegRecvFrame` Phase 4 — `rx_partial` now preserved on partial-read
  timeout (R1-01 fix, code comment corrected to match); `rx_partial` buffer
  resized from `FF_LINK_SEG_MAX_FRAME` (1024) to `FF_LINK_SEG_PREFIX_LEN +
  FF_LINK_SEG_MAX_FRAME` (1026) to eliminate a 1-byte OOB write on max-size
  frame reassembly (NF-2, Major); removed unused variable in `link_seg.c`
  (NF-1, enabling `-Wall -Wextra -Werror`); added
  `test_seg_max_size_fragmented_partial` max-size regression test; TSN RX
  callback now uses a non-blocking try-lock `FF_PalMutexLock(g_lock, 0u)` with
  return-value check (R1-02 fix); idempotency guard added to `tsn_link_init`
  (R1-12 fix); backend name corrected to `"ip_link"` (R1-09 fix); dead
  variables/define removed from `link_ip.c` (R1-05/R1-06 fix); all test
  assertions made non-tautological (R2-03 fix); fragmented-stream, error-
  propagation, partial-write, short/oversized RX, and over-max-prefix tests
  added (R2-01–R2-06 fix). R1+R2 delta re-review (FRICK-REVIEW-0165):
  **APPROVED**. PA delta re-review (FRICK-REVIEW-0166): **APPROVED**.

- **S4b follow-up — NC-S4b-01..05 corrective work + host `pus_ip`/`pus_tsn`
  demonstration (SRS-REQ-060/061, method `Tst | Dem`).** The S4b conformance audit
  (norm N19) found five deviations. Corrective changes and host evidence are
  present, but independent delta closure remains pending:
  **NC-S4b-02 (Major)** — the S4b exit criterion "`pus_ip` / `pus_tsn` link
  round-trip PASS" had no supporting check, the declared S4b "Renode check"
  deliverable was not delivered (SKIP proxy is not a check), and the `Dem` method
  of SRS-REQ-060/061 was unmet. Resolution: added `test_pus_ip_roundtrip` (a
  PUS-C S17 ping TC is received over the IP `FF_Link_t` backend, ingested by PUS,
  and the TM reply is pushed back over the same link against the mock
  `FF_NetStack_t`; the reference `FF_NetStackLoopback` is a socket pair, so it
  models the peer across two sockets, not a single-socket self-loop) and
  `test_pus_tsn_roundtrip` (the same over the TSN backend, with the
  mock MAC looped back) — `tests/unit/test_link_ip.c` (20 test funcs / 79 checks)
  and `tests/unit/test_link_tsn.c` (15 test funcs / 135 checks). The on-target
  (Renode) IP/TSN link-level round-trip remains **GAP-A5** (register-only GEM,
  NC-S6-01 precedent). **NC-S4b-01** — GAP-A5 now enumerates the IP/TSN
  link-level residual (`docs/quality/class-a-gaps.md` rev 0.11). **NC-S4b-03** —
  corrected the stale S4b suite/check counts in `verification-results.md`, the
  `Verfied` typo in `traceability.yaml`, and the stale sprint-plan/loop-audit
  header revisions. **NC-S4b-04** — corrected the `traceability.md` SRS-REQ-059
  mirror row (removed `test_link_tsn.c`, which tests SRS-REQ-061). **NC-S4b-05** —
  reconciled the stale M4 status lists (this audit's §1 M4 row and
  `verification-results.md` "Remaining (M4)") with the delivered S3/S4/S5/S7
  state. The README metrics panel was regenerated (`make metrics`; gate
  `tools/scripts/metrics.py --check`) so it folds in the new test-script lines.
  Gate status: build + ctest 27/27 + trace (114/71/29/14/51) + smoke + GSW +
  clang-format all PASS. The `-Wall -Wextra -Werror` gate is now **enforced on
  the test targets** (`tests/CMakeLists.txt`) and every `tests/unit/*.c` compiles
  clean under it; `make verify-lean` and `make renode` were not run on the
  authoring host (no Lean toolchain; host-only change, no RTEMS image impact) and
  are carried by CI (norm N18 proxy).

- **S4b follow-up review corrections applied; closure pending.** Independent
  reports `FRICK-REVIEW-0167` (R1+R2), `0168` (R4) and `0169` (PA) are
  APPROVED WITH COMMENTS. Their reports require delta re-review, which has not
  yet been recorded; findings remain open. Authoring corrections included IP
  send-leg assertions, sink reset, mock-MAC invariant documentation, accurate
  mock-vs-reference wording, evidence provenance, tool re-qualification notes,
  histories and delivered-scope corrections. The `-Wall -Wextra -Werror` gate
  was enforced on test targets and four pre-existing test defects were fixed.

- **S4b follow-up, 2nd pass — NC-S4b-06..08 corrections applied; closure pending.**
  The second-pass audit (`docs/status/loop-audit.md`) found three deviations
  between implementation and documented scope; corrections are present, but
  independent delta closure evidence is not yet in the repository.
  **NC-S4b-06** — `README.md` "Low-level drivers": the table now lists the
  delivered IP/TSN PUS-C link backends, PCIe and SpaceWire plus a PPS row; the
  closing paragraph no longer classifies delivered drivers as "M3/M4
  deliverables" (and no longer implies a `PPS` `hal_*.h` contract) but names the
  genuinely open items (NC-S7-01, NC-S6-01/NC-S6-03, the on-SoC eMMC/SD checks;
  GAP-A7); the architecture-diagram link node now reads "loopback / UART / IP /
  TSN". **NC-S4b-07** — the gPTP / fabric-CoreTSN milestone statements in
  `network-stack.md` §6, `software-description.md` §2.1/§3.7/§3.9,
  `development-plan.md` §M8/§6 and `handover.md` contradicted the authoritative
  SRS-REQ-068; all four now record the 802.1AS gPTP stack as implemented (M8,
  `src/drivers/eth/gptp/`) and the fabric CoreTSN path as deferred to M14, with
  the M8 plan row marked "delivered (partial)" (802.1Qav/Qbb =
  `FF_NOT_IMPLEMENTED`, NC-S8-01/NC-S8-02) and OBJ-16 recorded as "M8 (gPTP) /
  M14 (fabric CoreTSN)". **NC-S4b-08** — `handover.md` still carried the
  reference-loopback claim already corrected elsewhere under R2-F03, an
  understated `-Wall -Wextra -Werror` statement and an M4-residual driver list
  omitting the delivered MSS SPI and MSS MMC/SD (BLOCK) backends; all three are
  corrected. NC-S4b-06..08 remain open until independent R1/R2/R4/PA delta
  reviews are completed and recorded.

- **S8-FU assessment follow-up.** Cross-check of the external assessment opened
  NC-EXT-01..13 and NC-S4b-09. Widened CCSDS decoder length validation, the
  maximum TC-ingest buffer, ground-tool packet layout, GSW finite-buffer
  resynchronization, malformed-input/golden-vector tests, and sanitizer mock
  correction are implemented. Profile/requirements decisions, UART framing,
  maximum-size application-ingest evidence, external ground-segment
  interoperability, and review closure remain open. FRICK-REVIEW-0170 was a
  partial pass, FRICK-REVIEW-0171 rejected, and FRICK-REVIEW-0172 failed;
  FRICK-REVIEW-0173 re-verified the GSW resynchronization case. Current local
  evidence includes pinned `make ci-local` (including Lean), 27/27 native and
  sanitizer tests, trace, smoke, GSW vectors, RTEMS Docker cross-build and real
  Renode functional PASS. Forge CI has not run on the uncommitted tree; all
  review/assurance NCs remain open except where the NC register records an
  independently verified bounded closure.

- **M11/S6b-2 - lwIP netif bound to the GEM + RTEMS object sizing (SRS-REQ-040).**
  `config/rtems/rtems_config.c` now sets `CONFIGURE_MAXIMUM_TASKS 24` /
  `CONFIGURE_MAXIMUM_SEMAPHORES 32` / `CONFIGURE_MAXIMUM_MESSAGE_QUEUES 24`
  (bounded, justified) so lwIP can start on the target (closes NC-S6-02). New
  `src/hal/net/lwip/ff_lwip_netif.{c,h}` binds an lwIP `netif` to the framework
  Ethernet MAC: on the **lwIP-enabled** Renode image the check configures the
  emulated GEM and asserts netif-up + MAC identification (`hal_net_lwip=PASS`;
  M11 progress on NC-S6-01). The `sys_thread_new` failure diagnostic
  (NC-151-05) and the CI lwIP leg + `nm` link check (NC-151-02) are closed.
- **NC-S6-03 (open).** The GEM driver's descriptor address translation
  (`gem_phys`) returns `uint32_t`, so it cannot express an MPFS DDR address
  ≥ `0x1000000000`; arming the emulated GEM DMA makes the descriptor walk
  fault/loop, so the S6b-2 netif check configures the MAC without RE/TE. Latent
  GEM defect; owner = GEM driver author; fix in a driver sprint (related GAP-A5).
- **NC-S6-01 (narrowed, open).** NET lwIP-on-GEM is now netif-bound on the
  emulated GEM; the full TX/RX packet round-trip remains open (follow-up NET
  sprint). **NC-154-01 (open)**: the netif glue has no host test (NET follow-up).
  **NC-154-02 (closed)**: the `traceability.md` History row was added.

- **M8 — gPTP (802.1AS) + GEM TSN 802.1Qci completion (SRS-REQ-068/SRS-REQ-066).**
  Implemented a full 802.1AS gPTP stack (`src/drivers/eth/gptp/ff_gptp.{c,h}`):
  BMCA (clock-class/priority/identity comparison), Sync/Follow_Up time-offset with
  a PI servo, PDelay link-delay measurement, and a port state machine
  (master/slave/uncertain/disabled). The stack integrates with the GEM TSU time-read
  seam via `FF_TsnBackend_t::get_time_ns` and sends/receives PTP frames through
  `FF_EthMac_t`. Host unit tests `test_gptp.c` (6 functions, 58 checks) use a mock
  MAC + mock TSU. The GEM TSN backend now implements 802.1Qci per-stream filtering
  via screener type 1 entries (`set_stream_filter`, `enable=1` programs
  EtherType+VID match for a traffic-class queue; `enable=0` clears it); 802.1Qav CBS
  and 802.1Qbb PFC return `FF_NOT_IMPLEMENTED` (GEM register-subset limitations,
  tracked NC-S8-01/NC-S8-02). `test_gem_tsn.c` extended to 22 checks. Renode
  check: GEM TSU `get_time_ns` register-accessible (`hal_tsn=PASS`); full gPTP
  frame exchange is host-tested (mock MAC) — the register-only `Network.CadenceGEM`
  model (no DMA descriptor engine, TSTART unhandled) is GAP-A5. On-target gPTP
  timing accuracy = GAP-A5.

- **M4/M11 - in-repo lwIP RTEMS `sys_arch` port (SRS-REQ-040, S6b-1).** The
  pinned lwIP ships only unix/win32/freertos ports, so `FF_USE_LWIP=ON` could not
  build for the RTEMS target. `src/hal/net/lwip/rtems/` now provides the lwIP 2.x
  sys API over the RTEMS classic API (semaphores, priority-inheriting mutexes,
  mailboxes as `rtems_message_queue`, threads as `rtems_task`, time from the RTEMS
  clock) plus `arch/cc.h`/`arch/sys_arch.h`/`lwipopts.h`. `CMakeLists.txt` selects
  the RTEMS port only for `FF_PLATFORM=rtems`; the host build keeps the lwIP unix
  port (`FF_USE_LWIP` still OFF by default). Verified: host **23/23** with
  `FF_USE_LWIP=ON`; the RTEMS build produces the lwIP archive (85 objects incl.
  `sys_arch.c`) and a forced-extraction link resolves `tcpip_init`/`lwip_socket`/
  all `sys_*`; the default image does not reference the adapter yet (S6b-2), so it
  is byte-identical (no regression, default **22/22**).
- **NC-S6-02 (open).** The RTEMS runtime object sizing is not yet wired
  (`config/rtems/rtems_config.c` has no `CONFIGURE_MAXIMUM_MESSAGE_QUEUES`), so a
  runtime `tcpip_init` would fail; to be fixed in **S6b-2**.
- **NC-S7-01 (open).** The Renode PolarFire SoC model has **no SpaceWire
  peripheral**, so OBJ-19's "every class peripheral-bound" cannot be met for
  SpaceWire; to be resolved in **S7** with a contract-level check (GAP-A5).

- **M11/S6a - GEM native TSN peripheral-bound (SRS-REQ-038).** The GEM native
  TSN backend (`FF_TsnGem_t`, 802.1Q VLAN/PCP + 802.1Qbv + 802.1AS TSU time) is
  now bound to the emulated Cadence GEM in the Renode functional test and
  exercised peripheral-bound (`hal_tsn=PASS`): `init`, `set_vlan` and
  `get_time_ns` are driven against the emulated MAC TSU; the emulated GEM
  implements the TSU/Timer1588 registers read by `get_time_ns`, while the 802.1Q
  screener register used by `set_vlan` is not fully modelled (the VLAN/PCP write
  is a model no-op; its effect is verified on target, GAP-A5). 802.1Qav returns
  `FF_NOT_IMPLEMENTED` (documented) and the full gPTP/CoreTSN stack is M8. M11
  peripheral-bound HAL coverage 9/13 -> 10/13.
- **NC-S6-01 (open).** The **NET lwIP-on-GEM peripheral-bound** check (SRS-REQ-040)
  is deferred: it needs the lwIP RTEMS port built into the emulated image plus a
  Renode network path. The lwIP adapter remains host-tested
  (`tests/unit/test_lwip.c`). Resolution planned in **sprint S6b** (see
  `docs/planning/sprint-plan.md`); scope deviation recorded per the scope-
  conformance rule.

- **M4/M11 - reference MSS block-device backend (SRS-REQ-070).**
  `src/drivers/block/mpfs/ff_block_mpfs.{c,h}` implements the HAL block-device
  contract `FF_BlockDev_t` over the Microchip PolarFire SoC MSS MMC/SD controller
  (SDHCI-style) per the S5 sprint: 32-bit injected MMIO bus (`FF_BlockBus_t`),
  polled single/multi-block buffer-mode transfers (CMD17/18 read, CMD24/25 write)
  through the data-buffer port, block-size/count and argument programming, bounded
  status poll (`FF_MMC_POLL_MAX`), and timeout/error/invalid-argument/not-ready
  error codes. Host unit test `test_block` (behavioural 32-bit mock; 85 checks:
  read/write, argument faults, status timeout/error, block programming) and a
  Renode MMIO register round-trip check (`hal_block=PASS` on the emulated SoC).
  M11 peripheral-bound HAL coverage 8/13 -> 9/13.
  Scope: polled buffer mode only (no DMA/ADMA, command queuing, SDIO or PHY
  tuning); the emulated board has no SD card (register round-trips only, the
  transfer path is host-tested); on-target SD clock/bus-width and card
  initialisation = GAP-A5.

- **M4/M11 - reference MSS SPI backend (SRS-REQ-069).** `src/drivers/spi/mpfs/ff_spi_mpfs.{c,h}`
  implements the HAL SPI contract `FF_Spi_t` over the Microchip PolarFire SoC MSS
  SPI (CoreSPI) per the S4 sprint: 32-bit injected MMIO bus (`FF_SpiBus_t`),
  polled master (`init(clock_hz, mode, cs)`, `transfer`, `cs_assert`,
  `cs_release`), 8-bit CLKRATE clock-divider selection from a reference 100 MHz APB
  clock, bounded status poll (`FF_SPI_POLL_MAX`), and
  timeout/invalid-argument/not-ready error codes. Host unit test `test_spi`
  (behavioural 32-bit mock; 126 checks: transfer, CS control, fault and
  clock-divider cases) and a Renode MMIO register round-trip check (`hal_spi=PASS`
  on the emulated SoC). M11 peripheral-bound HAL coverage 7/13 -> 8/13.
  Scope: polled master only (8-bit frames; no DMA/interrupt/enhanced-framing);
  the emulated `SPI.MPFS_SPI` has no registered slave (register round-trips only,
  the transfer path is host-tested); the CLKRATE divider is a reference
  derivation, on-target APB/SCL timing = GAP-A5.

- **Restored explicit M4 scope (N19 audit).** Scopes that existed only as prose
  "absorbed" notes are now explicit, traceable deliverables: **PUS-C over IP and
  TSN** — bounded segmentation/reassembly (SRS-REQ-059) and reference `FF_Link_t`
  backends for TCP/IP (SRS-REQ-060) and TSN (SRS-REQ-061); **extended fieldbus** —
  the generic fabric-device contract (SRS-REQ-067), CAN-FD via the fabric contract
  (SRS-REQ-062), CANopen (CiA 301/302/402) (SRS-REQ-063) and RS-485 half-duplex
  DE/RE (SRS-REQ-064); and a documented **PUSopen(R) adapter** (SRS-REQ-065,
  library user-supplied). M8 gains the GEM TSN shaping/filtering completion
  (802.1Qav/Qci/Qbb, SRS-REQ-066) and the explicit gPTP + fabric CoreTSN
  requirement (SRS-REQ-068). New SSS-REQ-028/029; new objectives OBJ-25..OBJ-30;
  new sprint items S4b/S4c/S4d; counts 69 srs / 29 sss. The on-board file transfer
  set remains planned in M15.

- **DDJF acronym registration + PUS-C transport-channel documentation
  (FRICK-ENG-0007 0.3, FRICK-ENG-0005 0.5, FRICK-GLOS-001 0.2).** Registered the
  project acronym **DDJF = Design Description and Justification File**: retitled
  `docs/engineering/software-description.md` (FRICK-ENG-0007) as the
  **Design Description and Justification File (DDJF)**, replaced the former
  "no repository acronym defined" naming note with the explicit definition, and
  added the **justification dimension** as §3.10 (the rationale for each major
  design element, cross-referenced to the ADRs ADR-0001…0014/0016; ADR-0015
  unused). Added the acronym to `docs/glossary.md` and named the DDJF in the
  README documentation index and `AGENTS.md` (same path). Added a user-facing
  **§7.1 "PUS-C transport channels"** to `docs/engineering/user-integration-manual.md`:
  PUS-C is transport-agnostic and carried by `FF_Link_t`; only loopback
  (`FF_LinkLoopbackBackend`) and UART (`FF_LinkUartBackend`) reference backends
  ship (default loopback on the host, UART on RTEMS), `FF_LinkSelect()` plugs
  another backend, and TCP/IP/TSN/CAN/SpaceWire PUS-C link backends are **not
  implemented** (planned) — the HAL contracts for those devices are *device*
  contracts, not PUS-C link channels. Documentation only, no capability
  over-claim.
- **User-facing TCP/IP and TSN documentation + expanded framework description
  (FRICK-ENG-0005 0.4, FRICK-ENG-0007 0.2, FRICK-ARCH-0012 0.3).** Added user
  "How to use" subsections to `docs/engineering/user-integration-manual.md`:
  §8.1 (TCP/IP — `FF_NetStack_t` contract; `FF_NetStackLoopback`/`FF_NetStackLwip`
  backends; `FF_USE_LWIP` default OFF; lwIP pinned to `STABLE-2_2_1_RELEASE`
  BSD-3-Clause; Tuxera/RTEMS `libbsd` user-supplied; PUS-C over IP **not**
  implemented end-to-end) and §9.1 (TSN — `FF_TsnBackend_t`/`FF_TsnGemSetup`;
  802.1Q/Qbv implemented; 802.1Qav `FF_NOT_IMPLEMENTED`; 802.1AS time-read seam,
  **not** gPTP; M8 plan). Expanded the comprehensive framework description
  `docs/engineering/software-description.md` (§2.1 "what frick-fsw is not", §3.8
  BSP and start-up sequence, §3.9 interface-contract inventory with
  implemented/interface-only/planned/user-supplied status, §5 platform build/target
  matrix). `docs/architecture/network-stack.md` now states the exact lwIP pin and
  licence and cross-references the manuals (§8.1/§9.1); `AGENTS.md` names the
  software-description path explicitly. Documentation only, no capability
  over-claim.
- **M4/M11 - reference MSS I2C backend (SRS-REQ-058).** `src/drivers/i2c/mpfs/ff_i2c_mpfs.{c,h}`
  implements the HAL I2C contract `FF_I2c_t` over the Microchip PolarFire SoC MSS
  I2C (CoreI2C) per the S3 sprint: byte-injected MMIO bus (`FF_I2cBus_t`),
  canonical-address API (7-bit address passed to `write`/`read`/`write_read`),
  divider-code selection from a reference 100 MHz APB clock (exposed
  `ff_i2c_div_table`; 100 kHz/400 kHz/1 MHz selections tested), polled master
  sequencing driven by the hardware SI-clear discipline (read STATUS -> act ->
  clear SI; START/STA-ADDR/SEND-DATA/begin-read/RECEIVE/STOP), bounded SI wait
  (`FF_I2C_POLL_MAX`), and NACK/timeout/invalid-argument/not-ready error codes.
  Host unit test `test_i2c` (behavioural SI-clear-driven byte mock with a
  register-based slave; 144 checks: write, read incl. cap=1, write-read, fault,
  bus-hook validation and divider cases incl. the 1 MHz code-5 mapping) and a
  Renode MMIO register round-trip check (`hal_i2c=PASS` on
  the emulated SoC). M11 peripheral-bound HAL coverage 6/13 -> 7/13.
  Scope: polled master only (no slave/SMBus/clock-stretch/interrupt path);
  the emulated controller does not model CTRL CR0/CR1/CR2 (asserts ENS1 + IDLE
  STATUS 0xF8 and round-trips ADDR/SLAVE1_ADDR only) — SCL divider is a reference
  derivation, on-target APB/SCL timing = GAP-A5.
- **Engineering documentation set.** `docs/engineering/software-description.md`
  (FRICK-ENG-0007; the "what this framework is and does" entry document, expanded
  to 0.2 and kept current per release), plus status updates in
  `docs/architecture/network-stack.md` (0.3), `docs/engineering/user-integration-manual.md`
  (0.4) and `docs/engineering/driver-development.md` (0.3) recording the
  TCP/IP-over-PUS-C and TSN implementation status (implemented / planned /
  external) with dedicated user-facing sections and no capability over-claim.
- **M10 - QEMU emulated-target run (SRS-REQ-043, OBJ-18).** A **generic**
  `riscv/rv64imafdc` RTEMS image (platform `qemu-virt-rtems`, `FF_PLATFORM=rtems-generic`,
  no PolarFire SoC board module; `config/platform/platform_qemu.h`) is built
  (`cmake/toolchains/rtems-riscv-generic.cmake`, `make qemu-image`/`qemu-image-docker`)
  and run on `qemu-system-riscv64 -M virt` (`make qemu`, V&V tool **T20**
  `tools/scripts/qemu_vv.sh`, which asserts the app count and records the toolchain
  image id): boot banner + **6 applications registered**, console retained at
  `build/rtems-generic/qemu-console.log`. Complements the Renode MPFS run (norm
  N18); the nightly workflow runs the QEMU check. The open-source RTEMS toolchain
  image (`tools/docker/rtems`, T18) now builds the `rv64imafdc` BSP and carries
  QEMU + cmake/ninja. M10 closed.
- **M4/M11 - reference MSS CAN backend (SRS-REQ-057).** `src/drivers/can/mpfs/ff_can_mpfs.{c,h}`
  implements the HAL CAN contract `FF_CanBackend_t` (classic CAN 2.0 A/B) over the
  Microchip PolarFire SoC MSS CAN: run/bit-timing set-up, round-robin TX buffers,
  accept-all RX buffer with poll dispatch, reference bit timing. Host unit test
  `test_can` (mock MMIO, 129 checks) and a Renode MMIO register round-trip check
  (`hal_can=PASS` on the emulated SoC). CAN-FD is rejected (fabric device).
  M11 peripheral-bound HAL coverage 5/13 -> 6/13.
- **Renode V&V environment: board repl (BeagleV-Fire).** The functional test now
  loads `@platforms/boards/beaglev-fire.repl` (which includes the SoC platform)
  instead of the SoC-only repl, adding the **QSPI NOR flash** (`SPI.MPFS_QSPI` +
  `SPI.Micron_MT25Q`) and `MPFS_SystemServices` models. All existing HAL checks
  still pass (`RENODE_TEST: PASS`). A genuine QSPI write/erase check still needs
  the QSPI controller command driver (M11 task).
- **M4/M11 - reference NOR/QSPI MTD backend (SRS-REQ-056).** `src/drivers/mtd/mpfs/ff_mtd_mpfs.{c,h}`
  implements the HAL memory contract `FF_Mtd_t` (byte read/write, sector-aligned
  erase) over the MSS QSPI NOR; host unit test `test_mtd` (mock, 4123 checks).
  The on-SoC check needs the QSPI controller command driver (the board repl with
  `MPFS_QSPI` + `Micron_MT25Q` is now loaded) (M11 follow-up).
- **M4/M11 - reference eNVM NVM backend (SRS-REQ-055).** `src/drivers/nvm/mpfs/ff_nvm_mpfs.{c,h}`
  implements the HAL memory contract `FF_Nvm_t` (key/value by id) over a bounded
  eNVM slot table (32 ids x 32-byte slots); host unit test `test_nvm` (mock MMIO,
  36 checks) and a Renode eNVM **data-region write/read** check (`hal_nvm=PASS` on the emulated
  SoC; the data region at 0x20220000 is writable).
  M11 peripheral-bound HAL coverage 4/13 -> 5/13.

### Fixed

- **MSS I2C review NCs (FRICK-REVIEW-0129..0136; SRS-REQ-058).** `test_i2c` extended
  from 135 to 144 checks: a real 1 MHz divider-code-5 case (R1-03/R2-01/R3-02/PA-04),
  the minimum-SCL boundary at 104166/104167 Hz (R1-01/R3-04) and a NULL-bus-hook
  rejection case (R1-C2). The header now documents the minimum achievable SCL
  (`FF_I2C_MIN_SCL_HZ`), 7-bit-only addressing, the fire-and-forget STOP, the
  non-re-entrant binding and the self-test side effects (R1-02/C3/C4/C8), and the
  register map is labelled a **reference mapping** (R3-01); the host mock is
  described as behavioural/phase-level, not an independent oracle (R1-C1/R2-03/R3-07);
  comment typos fixed (R1-C7). Records reconciled: `test_i2c` = 144 checks, the
  divider rates corrected, the four document headers bumped to their History rows
  and the loop-audit baseline refreshed (PA-02/PA-03/R4-01/R4-04); `SRS-REQ-058` was
  added to the `ci-renode` evidence markers (R2-02); a reproducible `make misra-docker`
  T14 path was added (PA-05/R1-C9); the ICD board-level scope note and the 802.1AS
  "time-read seam" wording were clarified (R3-06/R4-06); `software-description.md`
  was registered in the README index and `AGENTS.md` (PA-06); the README "Current
  focus" was refreshed (R4-03); the undefined token `RR-058` was expanded (PA-07);
  and the GAP-A5 I2C on-target residual (per-rate SCL accuracy, STOP completion,
  bus recovery) was recorded.
- **M10 QEMU review NCs (FRICK-REVIEW-0120..0123; also closes the predecessor
  M10 review FRICK-REVIEW-0035 NCs).** The QEMU image is now a genuinely generic
  RTEMS build (no PolarFire SoC board module / MPFS banner, per `platform_qemu.h`);
  tool **T20** asserts the `6 applications registered` count and records the
  toolchain image id; the `ci:emulated-run` evidence mapping is corrected and the
  QEMU check is exercised by the **nightly** workflow (`.github/workflows/nightly.yml`);
  the T18 cleanliness caveat is recorded; the emulated-target doc's per-core ISA and
  QSPI/MT25Q attribution are corrected and the driver scope is explicit; status.json,
  loop-audit, traceability mirror, verification-plan, README and trailing newlines fixed.
- **MSS CAN RX accept-all mask (FRICK-REVIEW-0107 R2-01 / 0109 PA-09 Major,
  SRS-REQ-057).** The RX acceptance mask was written to the AMR register in the
  raw identifier domain instead of the register's left-aligned identifier field
  (bits 3-31), so extended identifiers >= 0x04000000 were silently rejected; the
  reference accept-all filter (`FF_CAN_ACCEPT_ALL`) is now left-aligned into the
  identifier field with the RTR/IDE mask bits set. `test_can` gained a regression
  that discriminates the fix (9 failures at the old constant) and boundary /
  negative cases; `FF_CanMpfsPoll` re-reads the RX control word before
  acknowledging so a callback reconfiguration is not clobbered; the raw AMR write
  (including the RTR/IDE mask bits the emulator does not model) is asserted.
- **MSS CAN reference clock (FRICK-REVIEW-0110 R1-D-01 Minor, SRS-REQ-057).** The
  reference bit-timing clock was corrected from an assumed 125 MHz to **80 MHz**
  (the MSS CAN clock is one of 8..80 MHz from the MSS PLL per the PolarFire SoC
  MSS TRM); at 80 MHz the standard rates 125/250/500 kbit/s and 1 Mbit/s are
  exact (prescaler 40/20/10/5). On-target confirmation remains GAP-A5.

- **Table Services validation bypass (FRICK-REVIEW-0077 Major, SRS-REQ-013).**
  `FF_TblLoad` left a previously-staged image valid when a new load failed
  validation, so `FF_TblActivate` could commit a rejected image. It now clears
  `staged` on a validator rejection; a regression test reproduces and guards it.
- **Test bug fixed (ASan):** `test_r2_gap` cast a TC secondary header to a TM
  header pointer in the CCSDS TM test, causing a stack-buffer-overflow READ under
  AddressSanitizer; corrected to use a real `FF_Pus_TmHdr_t`.
- **`FF_PusInit` idempotence.** `FF_PusRegisterCoreServices` now resets the
  provider/function/HK/mem registries so repeated init is clean (fixes the
  order-dependence of the new PUS tests, FRICK-REVIEW-0077 R1-02).


- **Test-coverage gap closure (R2 + R3 task).** Two new host suites close the
  functions/branches that were unexecuted and the requirement-level flows that
  were `ci:docs`-only: `tests/unit/test_r2_gap.c` (487 checks: EVS/ES lifecycle,
  SB wildcard/saturation/drop, TBL revert/dump, Time MET/UTC, HAL invalid-class,
  CCSDS TC/decode errors, PUS core counters + S3/S8/S9/S20/S11, Math NaN/inf, PAL
  lifecycle) and `tests/unit/test_r3_gap.c` (198 checks: TC->dispatch->TM, S1
  verification codes, EVS->S5, TBL activate/revert, S9 set/report, PUS malformed
  TC). ctest 14 -> 16 suites. Reports FRICK-REVIEW-0075 (R2) / 0076 (R3).
- **Completion-metrics panel (README).** `tools/scripts/metrics.py` (V&V tool
  T13) now computes and renders, **above the semaphores**, a completion panel:
  sprint completion (`docs/planning/sprint-plan.md`), SSS/SRS requirements
  delivered, design traceability (SDD), verification coverage, review closure and
  code coverage (statement/function), with a note explaining each metric and the
  ECSS-class-B context. It is refreshed every cycle (norms N8/N11/N19). The metric
  definitions were reviewed for approval by the Senior SW PA (norms N20/N12):
  FRICK-REVIEW-0068/0069/0071 (PA, APPROVED WITH COMMENTS) and the T13 source
  review FRICK-REVIEW-0070 (R2, N10); a CI currency gate (`metrics.py --check`)
  keeps the panel current.
- **Milestone M16 — In-house OBCP engine development.** Added a milestone
  (objective OBJ-24; SSS-REQ-027 / SRS-REQ-054) to develop, within the project, an
  ECSS-compliant OBCP engine behind `FF_Obcp_t` (bounded resources, isolation,
  execution-time budget, abort, reporting, PUS S18 state management) as the
  fallback/hardened engine to the integrated third-party engine (M12). Execution
  order: after M12, before M13. Origin: user directive.
- **N12 PA-review closure loop (norms N12/N20).** Independent Senior reviews
  (R1/R2/R4) of the N20 governance change and of `FRICK-REVIEW-0032` were run,
  their findings fixed and re-reviewed to closure: the reference loopback now
  rejects a duplicate `bind` across sockets; `test_netloop` gains boundary,
  negative, exhaustion and full-ring partial-send cases (29 -> 328 checks); the
  network-stack contract states the timeout/partial-transfer and `bind` semantics;
  SRS-REQ-039 wording corrected; review log, loop-audit, development plan, V&V
  records and the tool register reconciled (FRICK-REVIEW-0054..0062). PA/documentation
  semaphores are driven by the review verdict.
- **Norm N20 — independent Senior SW product-assurance review of every phase
  product.** New non-derogable norm: every phase product is independently
  reviewed also by the **Senior SW Product Assurance engineer (PA)**, and
  findings are corrected and the reviews repeated until they pass
  (`docs/engineering/reviewer-agents/aeronautics-space-sw-pa-engineer.md`).
  The five Senior reviewer personas are now **versioned in this repository**
  under `docs/engineering/reviewer-agents/` (R1 FSW, R2 V&V, R3 Avionics
  Systems, R4 Documentation, PA), superseding the previous inline R1-R4
  profile blocks, and exposed as OpenCode subagents (project `.opencode/agents/`).
  Origin: user directive; ADR-0010 revised; norm N20 (see `docs/rules.md`).
- **Milestone M2 — Time base: PPS tone discipline and CUC framing.** Hardware-
  disciplined time base with time-source selection (`FF_TimeSetSource`, internal
  CPU / external 1 Hz tone / ground correlation), a software PPS backend task on
  the host (`src/core/time/pps.c`, `FF_PpsRegister/Start/Stop`) and an
  RTEMS/MPFS GPIO/PLIC capture hook for the board integrator (GAP-A7). Tone
  discipline (`FF_TimeToneSignal`): strictly increasing epoch, one-second CUC
  jump per valid tone, UTC latched at the edge. Leap seconds scheduled with
  `FF_TimeScheduleLeap` (single ±1 s) and applied/cleared exactly at the next
  tone boundary. CUC framing per CCSDS 301.0-B-4 completed: 7-byte P+T code
  (P-field `0x2E`, 4 coarse + 2 fine octets) with lossless
  encode/decode (`FF_TimeCucEncode/Decode`) and rejection of malformed frames.
  New requirements SSS-REQ-016, SRS-REQ-021..024; Lean model `Frick.Time`
  (11 theorems) + 10 requirement wrappers; unit tests `test_time`
  (ctest `time`, 5/5 suites, 174 checks total: ccsds 15, sb 20, tbl 9, pus 29,
  time 101). Docs: `time-and-timecode.md` 0.3, ICD 0.3, class-A gaps GAP-A7,
  roadmap M2 done. M2 review closure (norm N12): all R1/R2/R3 non-conformances
  resolved and delta-re-reviewed; code line coverage 845/1446 (58.4%),
  functions 114/156; Lean clean-rebuild transcript committed
  (`verification/lean/lake-build.log`).
- **Development plan** `docs/development-plan.md` (FRICK-PLAN-0001): the product
  boundary (frick-fsw is a middleware), objectives OBJ-01..OBJ-12, the
  resource/bus interface catalogue (Ethernet+pluggable TCP/IP/TSN, CAN/
  CANopen/CAN-FD, RS-422/485, SPI, I2C, PCIe, GPIO, SpaceWire ECSS, memory),
  and the milestone plan M1..M5.
- **User & Integration Manual** `docs/engineering/user-integration-manual.md`
  (FRICK-ENG-0005): how to integrate applications, drivers and pluggable stacks
  on the middleware.
- **Milestone M3 — Hardware abstraction interfaces.** A bounded, allocation-free
  HAL registry (`src/hal/hal.{c,h}`) and stack-independent contracts per resource
  (Ethernet MAC + net interface, pluggable TCP/IP stack, TSN, CAN/CANopen/CAN-FD,
  serial RS-422/485, SPI, I2C, PCIe, GPIO, SpaceWire + RMAP, RAM/DMA/MTD/NVM/
  block). Requirements SSS-REQ-017 / SRS-REQ-025..032; Lean model `Frick.Hal`;
  unit test `test_hal`; spec `docs/architecture/hardware-abstraction.md`;
  `FF_BUILD_HAL` partitioning option; layer-focused architecture diagrams.
- **ECSS design and assurance documentation.** Software Design Document
  (`docs/design/software-design.md`), configuration management plan
  (`docs/quality/configuration-management.md`), product/quality assurance plan
  (`docs/quality/product-assurance.md`), validation plan
  (`docs/vv/validation-plan.md`), preliminary safety/FDIR analysis
  (`docs/quality/safety-and-fdir.md`) and the ECSS compliance matrix
  (`docs/quality/ecss-compliance-matrix.md`); gaps to class A extended to
  GAP-A8..A13 (SDP formalisation, SDD depth, FMEA, third-party IVV, OPD,
  maintenance).
- **Milestone M6 — Mathematics library integration (libm).** Portable
  mathematics facade `src/math/ff_math.{c,h}` over the platform `libm`
  (`FF_USE_LIBM`, `FRICK_USE_LIBM=1`; glibc host / newlib RTEMS), linked with
  `-lm`; unit test `test_math` (20 checks); requirements SSS-REQ-018 /
  SRS-REQ-033; decision ADR-0012 and policy `docs/architecture/numerics.md`;
  licence analysis updated.
- **Milestone M7 — CCSDS SDLS (Space Data Link Security).** Middleware-side
  security sub-layer `src/security/ff_sdls.{c,h}`: SDLS processing (SDLS header
  + MAC; plain/auth/enc/auth-enc services), a bounded SPI-indexed
  security-association database, and key management (session-key derivation,
  rotation on frame/time thresholds with strictly increasing epoch, and
  zeroisation). The cryptographic primitive is a **pluggable backend**
  (`FF_CryptoBackend_t`); a non-secure reference backend (`ff_crypto_test.c`,
  test-only) exercises the logic. Requirements SSS-REQ-019 / SRS-REQ-034/-035;
  Lean `Frick.Sdls`; unit test `test_sdls` (69 checks, ASan-clean);
  ADR-0013 and `docs/architecture/security-sdls.md`; algorithm qualification /
  HSM / ground key management is GAP-A14.
- **Milestone M4 — reference drivers and third-party integration (in progress).**
  Reference **Ethernet driver** for the Cadence GEM of the PolarFire SoC MSS
  (`src/drivers/eth/gem/`), implementing the `FF_EthMac_t` HAL contract with
  static, allocation-free descriptor/buffer handling and injected MMIO + HAL DMA
  translation (host-testable; `test_gem`, 23 checks). Requirements SSS-REQ-020 /
  SRS-REQ-036; `FF_BUILD_DRIVERS` partition option; ADR-0014. Third-party
  integration (fetched pinned, not committed): **lwIP** (BSD-3) for
  `FF_NetStack_t` and **N7/ESA `asn1-pusc-lib`** (GPL-3.0) for
  `FF_Pus_Backend_t`; `tools/scripts/fetch_third_party.sh`. TSN uses the GEM's
  native block (802.1Q/Qbv/Qav/802.1AS).
- **M4 — RTEMS application integration.** A bounded RTEMS 6.2 configuration
  table (`config/rtems/rtems_config.c`: console/clock + POSIX objects +
  `CONFIGURE_POSIX_INIT_THREAD_TABLE`) and the `POSIX_Init` entry point
  (`pal_rtems.c`) running the shared bring-up; the RTEMS executable is built
  from the same sources. Requirement SRS-REQ-037; guide
  `docs/architecture/rtems-integration.md`; toolchain flags aligned to the
  `riscv/mpfs64imafdc` BSP.
- **M4 — GEM native TSN backend.** A reference `FF_TsnBackend_t` over the
  Cadence GEM native TSN block (`src/drivers/eth/gem/ff_gem_tsn.{c,h}`): 802.1Q
  VLAN/PCP classification (screener type 2), 802.1Qbv time-aware scheduling
  (ENST gate control lists) and 802.1AS time from the GEM TSU; 802.1Qav CBS is
  documented as not exposed. Requirement SRS-REQ-038; unit test `test_gem_tsn`
  (16 checks).
- **M4 — Reference network stack (loopback).** `src/drivers/net/ff_net_loopback.{c,h}`
  implements the pluggable network-stack contract (`FF_NetStack_t`) as a bounded,
  static, in-process loopback, exercising the contract in the unit tests and the
  emulated-target (Renode) functional check; third-party stacks (lwIP, pinned) plug
  in through the same vtable.
  Requirement SRS-REQ-039; unit test `test_netloop` (328 checks).
- **M4 — lwIP network-stack adapter.** `src/hal/net/lwip/ff_net_lwip.{c,h}`
  implements the `FF_NetStack_t` contract over the lwIP socket API (third-party
  BSD-3 stack, fetched pinned; `contrib` is part of the lwIP tree). Built with
  `FF_USE_LWIP=ON` (CMake target `frick_lwip` + adapter); a host data-path test
  (`test_lwip`, 19 checks) does a real TCP round trip over the lwIP loopback
  interface. Requirement SRS-REQ-040; a CI job fetches lwIP, builds and runs it.
- **Milestone M9 — code quality: MISRA C:2012 and coding style.** Declared the
  **frick-fsw C style** (`.clang-format` normative, cFS-inspired, MISRA-friendly;
  `docs/engineering/coding-style.md`), applied and enforced codebase-wide by the
  format gate; and a **MISRA C:2012** process (`docs/quality/misra-c.md`) with an
  open-source scan (`tools/scripts/misra.sh`, cppcheck addon, advisory → blocking)
  wired into CI (evidence `ci:misra`) and a documented deviation process.
  Requirements SSS-REQ-021 / SRS-REQ-041/042; certified MISRA checker + formal
  report are GAP-A4.
- **Milestone M10 — emulated-target verification.** Run the RTEMS target binary
  on an emulated Microchip PolarFire SoC (**Renode** `polarfire-soc`: 5 CPUs,
  UART, GEM, CAN, SPI, I2C, GPIO, PCIe, SD/eMMC, QSPI flash, eNVM, timers) and/or
  **QEMU** (`riscv/rv64imafdc`), verifying boot/PAL/drivers without hardware
  (partial GAP-A5 mitigation). `docs/vv/emulated-target.md`,
  `tools/scripts/emulated_run.sh`; requirement SRS-REQ-043; tools T15/T16.
- **Norm N18 — Renode functional validation.** A Renode V&V environment
  (pinned Antmicro image, `polarfire-soc` platform) runs the middleware on an
  emulated PolarFire SoC; `src/apps/app_renode_test.c` (CMake option
  `FF_ENABLE_RENODE_TEST=ON`; default build stays at six apps) prints the
  machine-checkable `RENODE_TEST: PASS` marker; `make renode-env` self-tests the
  environment (Renode runs + platform loads), `make renode-image` builds the
  RTEMS image and `make renode` runs the functional validation
  (`tools/scripts/renode_vv.sh`, CI evidence `ci:renode`). Requirement
  SRS-REQ-044; `docs/vv/renode-environment.md`; tool T17. Functional gate is an
  advisory proxy until the RTEMS image is available in the V&V image, then
  blocking (the proxy SKIPs when the image is absent and fails when it is present
  but the marker is missing). Reviews: FRICK-REVIEW-0036 (REJECTED, F-01 Major) +
  delta (all NCs closed).
- **Open-source RTEMS 6.2 baseline.** The RTOS baseline is the **open-source
  RTEMS 6.2** release from the RTEMS project (`gitlab.rtems.org`, source-builder
  ref `6.2`, BSP `riscv/mpfs64imafdc`); no vendor-specific or commercial
  baseline/pre-built toolchain is used. `tools/scripts/setup_rtems.sh` corrected
  (riscv tools bset + `6/rtems-kernel.bset --with-rtems-bsp=riscv/mpfs64imafdc`);
  new reproducible containerised toolchain `tools/docker/rtems/Dockerfile`
  (`make rtems-toolchain` / `make rtems-docker`, tool T18); the nightly workflow
  builds the open RTEMS Renode image and runs `make renode` (norm N18). ADR-0002,
  SSS-REQ-001 and all affected docs aligned.
- **Real open RTEMS toolchain + Renode functional validation (N18 in force).**
  The open-source RTEMS 6.2 toolchain (RSB tag `6.2`, BSP `riscv/mpfs64imafdc`,
  `tools/docker/rtems`) is built and validated; the real cross build succeeds
  (plain and Renode images) and the middleware runs on the Renode `polarfire-soc`
  platform with `make renode` -> `RENODE_TEST: PASS` (boot U54_1 at entry
  `0x1000000000`, console `mmuart1`). Defects found and fixed by the real build:
  `pal_rtems.c` included an unused, non-existent RTEMS header
  (`rtems/posix/time.h`) and other unused includes; `rtems_config.c` lacked the
  `POSIX_Init` declaration before `<rtems/confdefs.h>`; the CMake toolchain file
  now passes the BSP include/lib paths and `-mcmodel=medany` (multilib select).
  Norm N18 is now **in force** (per-commit SKIP proxy, nightly real run).
- **Milestone M12 — OBCP engine (uPython/MicroPython).** Execute On-Board Control
  Procedures written in uPython/MicroPython behind an `FF_Obcp_t` contract (OBCP
  modules bound to SB/EVS/Time/PUS), resource/safety bounded. First integration
  candidate **`n7space/n7s-obcp`** (MicroPython-based OBCP engine, MIT,
  ESA-funded); fallback: an ECSS-compliant engine. Objective OBJ-20; SSS-REQ-022 /
  SRS-REQ-045.
- **Milestone M13 — Reference mission services.** Optional PUS-based reference
  services: an **equipment manager** (device command/monitoring/reconfiguration,
  redundancy) and **fault management** (limit checking, stored command sequences,
  memory/file management). Objective OBJ-21; SSS-REQ-023 / SRS-REQ-046.
- **Component gap analysis.** `docs/architecture/component-gap-analysis.md`
  positions frick-fsw against NASA cFS (missing: LC/SC/MM/MD/CS/FM/DS/CF/SBN/ECI,
  equipment manager, several PUS services) and other FSW (F′, TASTE, N7 MBEP,
  OPS-SAT NMF).
- **Milestone M11 — Renode HAL coverage.** Extend the Renode functional test
  (norm N18) so that **every HAL resource class** is bound to its emulated
  peripheral and exercised on the `polarfire-soc` platform (ETH, NET/TCP-IP, TSN,
  CAN, SERIAL/RS-422/485, SPI, I2C, PCIE, GPIO/IRQ, SPACEWIRE, MEM, BLOCK, NVM),
  each with its backend behind the `hal_*.h` contract (M4-residual MSS
  peripherals), a binding in `frick-renode.resc` and a checkable clause in the
  marker. Objective OBJ-19; `docs/development-plan.md`,
  `docs/roadmap.md`. **Started (GPIO):** MSS GPIO driver
  `src/drivers/gpio/mpfs/ff_gpio_mpfs.c` (SRS-REQ-047) + host unit test
  (`test_gpio`, mock MMIO) + a real MMIO round trip in the Renode run
  (`hal_gpio=PASS`).
- **Reference MSS GPIO driver (M4/M11).** `FF_Gpio_t` backend for the Microchip
  PolarFire SoC MSS GPIO (bus-injected MMIO, allocation-free); host test
  `tests/unit/test_gpio.c` and emulated-MMIO check on the Renode `polarfire-soc`
  platform; requirement SRS-REQ-047.
- **Project codename / third-party name.** Non-affiliation disclaimer for the
  codename "**Eärendil**" (a J. R. R. Tolkien name) added in `NOTICE`, `README.md`
  and `docs/quality/licensing.md §6`: a single name is not copyright-protected
  (copyright vs trademark); the project is not affiliated with the Tolkien Estate
  or Middle-earth Enterprises, includes no Tolkien text, and does not use the name
  as a trademark; licensing contacts and a "not legal advice" note. Product name
  remains **frick-fsw**.
- **M11: reference MSS MMUART serial driver.** `FF_Serial_t` backend for the
  Microchip PolarFire SoC MSS MMUART (NS16550 **wide registers**, 4-byte spaced:
  THR 0x00, LCR 0x0C, LSR 0x14, SCR 0x1C; RS-422/RS-485 via transceiver),
  bus-injected MMIO, allocation-free; host unit test `tests/unit/test_serial.c`
  and an NS16550 scratch-register MMIO check on the emulated MMUART0 in the
  Renode run (`hal_serial=PASS`). The first attempt with compact offsets was
  reviewed (FRICK-REVIEW-0043) and reworked to hardware-correct wide offsets (the
  Renode platform sets `wideRegisters: true`). Requirement SRS-REQ-048.
- **M11: reference static RAM pool + DMA helper.** `FF_MemPool_t` (bounded,
  heap-free bump allocator over a caller-provided region, usually DDR) and a
  reference `FF_Dma_t` (identity address translation; cache-maintenance seam) in
  `src/drivers/mem/ff_mem_static.c`; host unit test `test_mem` and a RAM-pool + DMA
  check over the in-image RAM (emulated DDR) in the Renode run (`hal_mem=PASS`;
  the DMA cache hooks are a seam, real cache ops = GAP-A5). Requirement
  SRS-REQ-049.
- **PUS-C ASN.1 (N7/ESA) default — toolchain enabled.** Reproducible ASN1SCC
  compiler image (`tools/docker/asn1scc/Dockerfile`, pinned esa/asn1scc) and code
  generation from the N7/ESA `asn1-pusc-lib` ASN.1/ACN model
  (`tools/scripts/gen_pus_asn1.sh`, `make pus-asn1`, tool T19); generation
  validated (`BasicTypes.h`, `asn1crt.c/h`, module codecs). Remaining: consume the
  generated code in CMake, implement `FF_PusBackendN7` and switch the default
  (SRS-REQ-050). Tracked by the N19 audit (`docs/status/loop-audit.md`).
  _(Superseded by the "default backend" entry below: the generated code is now
  consumed in CMake and `FF_PusBackendN7` is the default, ADR-0016.)_
- **M15 (new) — In-house fundamental PUS-C service set** (from the PUS-C
  assessment `docs/architecture/pus-c-from-scratch-assessment.md`, FRICK-ARCH-0015):
  implement in MISRA-clean in-house C behind `FF_Pus_Backend_t` the fundamental
  PUS-C service behaviour (S1/3/5/6/8/9/11/12/17/18-surface/19/20) on the
  `asn1-pusc-lib` default data model (+ bounded datapool); est. ~34 PM; hybrid
  recommendation. Objective OBJ-23; SSS-REQ-025 / SRS-REQ-052. Placed after M14.
  **M15 also includes CFDP (CCSDS 727.0-B) and the virtual filestore (VFS) it
  operates on** (pluggable RAM/flash/eMMC/SSD) with PUS-C S23 on the VFS
  (SSS-REQ-026 / SRS-REQ-053).
- **PUS-C ASN.1 (N7/ESA) default backend (SRS-REQ-050, ADR-0016).**
  `FF_PusBackendN7` is the **default** PUS backend: the PUS-C field
  vocabulary/widths come from the ASN1SCC-generated `asn1-pusc-lib` types and the
  packet error control uses the N7/ESA ECSS Annex B.1 CRC; build-time generation
  (`make pus-asn1`, `tools/docker/asn1scc/`, T19; nothing generated/third-party
  committed). Falls back to the open backend (with a warning) when the generated
  code is absent; `-DFF_USE_PUS_ASN1=OFF` forces the open backend. Verified on
  host (14/14) and on the emulated SoC (`make renode` PASS).
- **M11: ETH HAL class exercised in the Renode run.** The functional test now
  identifies the emulated Cadence GEM via its read-only Module ID register
  (`hal_eth=PASS`), in addition to GPIO/SERIAL/MEM (SRS-REQ-036; the full GEM
  DMA/link path stays host-tested + GAP-A5).
- **M11: NET `FF_NetStack_t` contract check in the Renode run.** The functional
  test performs a socket round trip through the pluggable network-stack contract
  using the reference loopback stack (`hal_net=PASS`); it is a contract/behaviour
  check (in-process), **not** a binding to an emulated peripheral (so M11 stays
  4/13 peripheral-bound: GPIO/SERIAL/MEM/ETH). SRS-REQ-039.
- **Norm N19 — development-loop gap audit.** New non-derogable rule: at the start
  of every loop, compare declared objectives + adherence to N1-N19 with the actual
  implementation; gaps closed or tracked. First audit `docs/status/loop-audit.md`.
- **Roadmap: milestone M14 added** — multicore/SMP on the 4×U54 (after all other
  milestones), objective OBJ-22 (SSS-REQ-024 / SRS-REQ-051).
- **Norm N17 — documentation review (profile R4).** New non-derogable norm:
  before every commit/push the documentation set is reviewed for completeness
  and correctness by an independent **Senior SW documentation reviewer** (profile
  R4, `docs/engineering/independent-review.md`), and the review loop
  (review → correction → re-review, norm N12) is closed before commit/push. The
  gate is reported in the README status panel (`documentation_review` in
  `docs/status/status.json`). First run: `docs/reviews/2026-10-10-doc-set-r4-review.md`.
- Gitea CI/CD deployed and operational (norm N6): a self-hosted
  `act_runner` executes `.gitea/workflows/ci.yml`. The runner maps
  `ubuntu-24.04` to the pinned V&V image
  (`ubuntu-24.04:docker://frick-vv:local`) because the minimal
  `gitea/act_runner` Alpine image has no toolchain.
- Project mark for codename "Eärendil": a white stylized ship with a sparkling
  star on a midnight-blue field, in `assets/` (SVG source plus PNG), embedded in
  `README.md`; see `assets/README.md`.
- Mermaid diagrams, compatible with GitHub and Gitea: component architecture
  (`docs/architecture/overview.md`, `README.md`), TC/TM sequence
  (`docs/architecture/pus-layer.md`), and the V-model plus per-change cycle
  (`docs/vv/verification-plan.md`, `README.md`).
- **Copyright header, "Copyright (C) 2026 Ilfrick"** with links to the GitHub
  and Gitea project pages inserted in all 70 authored files (C, headers, Lua/Lean,
  scripts, workflows, Doxyfile, Makefile, CMake and SVGs); consolidated in
  `NOTICE`, `README.md` and `docs/quality/licensing.md` (norm N13).
- **README status panel**: coverage metrics and review/V&V traffic lights
  (red/yellow/green) generated by `tools/scripts/metrics.py` from the
  requirements manifest and `docs/status/status.json`; code line coverage
  collected by `tools/scripts/coverage.py` (`make coverage`, gcov).
- **README project index** (table of contents), documentation index with links,
  PUS-C services catalogue and low-level driver inventory.
- Engineering guides for humans and agents: `docs/engineering/driver-development.md`
  and `docs/engineering/service-development.md`.
- Development roadmap `docs/roadmap.md` (rev 0.3) and development plan
  `docs/development-plan.md`: M1 baseline (done), M2 PPS + CUC (done), M3 HAL
  interfaces + integration manuals, M4 MPFS BSP + 1 Gb/s Ethernet (absorbs
  virtual filestore, PUS service 23, CFDP, and TCP/IP + TSN over PUS-C), M5
  documentation and status program.
- `docs/architecture/time-and-timecode.md`: how MET/UTC/CUC and the PPS tone
  discipline work (PPS delivered in M2).
- **NET follow-up: GEM 64-bit descriptor addressing (NC-S6-03, NC-154-01).**
  `gem_phys` now returns `uint64_t` (was `uint32_t`) so it can express MPFS DDR
  addresses ≥ `0x1000000000`; `gem_wr_qbqp` writes the 64-bit descriptor-ring base via
  `TBQP_HI`/`RBQP_HI` (0x00C8/0x00CC) and `DMACFG.ADDR64` (bit 30) is asserted; the
  per-descriptor address truncation is now explicit `(uint32_t)gem_phys(...)` with
  the full 3-word 64-bit descriptor format deferred to M14. On-Renode GEM DMA
  round-trip remains blocked by the register-only `Network.CadenceGEM` model (no
  descriptor engine; TSTART unhandled = GAP-A5); the TX/RX data path is evidence-
  backed by the new host `test_lwip_netif` (3 tests, 19 checks; mock-MAC loopback,
  attach/double-attach/isup/getmac/arg-faults; 26/26 with `FF_USE_LWIP=ON`, ASan/UBSan
  clean) + the extended `test_gem` (asserts 64-bit register writes). Closes NC-S6-03
  (co-implementation) and NC-154-01 (host netif glue test).

### Changed

- Project codename changed from **"Vela"** to **"Eärendil"** (README banner,
  `config/fsw_config.h` `FF_CODENAME`, simulator boot banner).
- V&V image (`tools/docker/Dockerfile`) gains `nodejs`/`npm` so Gitea
  `actions/checkout` can run inside the pinned image.

### Fixed

- Documentation consistency (first norm-N17 review loop,
  `docs/reviews/2026-10-10-doc-set-r4-review.md`): README requirement counts
  aligned to the machine-generated panel (25/16/7/41) and README status panel
  regenerated to include the `documentation review` gate; revision headers
  aligned with the history tables (SRS, SSS, traceability.md, roadmap,
  driver-development, licensing, tool-qualification, verification-results,
  tooling, formal-verification); `FRICK-ARCH-0006` id collision fixed
  (`mapping-cfs.md` -> `FRICK-ARCH-0007`); review ids normalised to
  `FRICK-REVIEW-00nn`; `docs/reviews/README.md` log completed (M2 R1 + R4 rows);
  `## History` tables added to `tooling.md` and `glossary.md`;
  `formal-verification.md` M2 scope added; duplicate README `## Repository
  layout` heading removed.
- CI `simulator smoke` (GitHub): the banner check grepped for `=` which the
  banner never contained; with `bash -e` the step failed. Now checks for
  `frick-fsw` and reports a clear error.
- CI `static analysis`: the `clang-format --dry-run --Werror` gate failed on
  files that had never been auto-formatted; all C sources are now formatted with
  clang-format 18 (the runner version), so the gate passes.
- Gitea CI checkout: `act_runner` derives the job server URL without the Gitea
  `/git` sub-path, so `actions/checkout` failed with `repository not found`. The
  Gitea workflow now clones the public mirror explicitly with `git`; the GitHub
  workflow is unchanged.
- CI `documentation set`: the Doxygen step failed inside the pinned V&V image
  (doxygen 1.9.8) because `build/doxygen` did not exist; both workflows now
  create it first.

## [0.1.0] - 2026-10-10 - baseline release

**Origin:** rule set establishment (N1-N16) and initial development; review
closure (norm N12/N15) applied before the first public push.

**Origin note:** the review-closure cycle below re-ran the complete
development/verification cycle (norm N15) and is covered by
`docs/reviews/2026-10-10-baseline-vv-review.md` (R2) and
`docs/reviews/2026-10-10-sss-systems-review.md` (R3).


- Formal verification (norm N1) now actually builds: `lake build` compiles all
  15 modules and `lake exe frick_verify_checks` passes 3/3 in the Docker V&V
  image; requirement theorems are linked in the traceability manifest.
- Independent reviews under `docs/reviews/`: baseline V&V (R2), SSS systems
  review (R3, norm N14) and the norms N1-N16 conflict analysis.
- Ground support tool `tools/gsw/ff_gsw.py` (qualified as T7).

- Core framework (cFS-inspired, AGPL-3.0):
  - Portable Abstraction Layer (PAL) with POSIX (host) and RTEMS 6 backends.
  - Software Bus (publish/subscribe, wildcard subscriptions, bounded pipes).
  - Event Services, Executive Services, Table Services, Time Services.
  - CCSDS Space Packet codec + CRC-16/CCITT-FALSE trailer.
  - Link layer with loopback (host) and UART (target) transports.
  - PUS-C layer with open backend default and documented PUSopen adapter.
  - Services implemented: 1 (verification), 3 (HK), 5 (events), 6 (memory),
    8 (function), 9 (time), 17 (test), 20 (parameter).
  - Board support for the Microchip RT PolarFire SoC RTPFS160ZT (FCVG784).
  - Reference applications: CMD_INGEST, TM_OUTPUT, SCHEDULER, HK, HS, SAMPLE.
- Tests: `tests/unit/{test_ccsds,test_sb,test_tbl,test_pus}.c` (ctest; 4 suites).
- Formal verification (norm N1): Lean 4 project `verification/lean/` with
  requirement theorems (CRC, Software Bus, PUS counters, CCSDS length bounds)
  and `Main.lean` runtime checks.
- Build & tooling: CMake + Ninja + presets, RTEMS `riscv/mpfs64imafdc`
  toolchain file, `Makefile`, clang-format/tidy, Doxygen, Docker V&V image,
  act_runner deployment for Gitea, ground station `ff_gsw.py`.
- CI/CD: `.github/workflows/ci.yml` + `nightly.yml` (RTEMS), `.gitea/workflows/ci.yml`.
- Requirements & quality data (norms N3/N4/N5/N8):
  - `docs/requirements/{SSS,SRS,traceability}.{md,yaml}` (SRS=21, SSS=15, GAP=6).
  - Traceability gate `tools/scripts/trace.py` (convergence on every change).
  - `docs/rules.md` (N1-N16), `docs/tooling.md`, `docs/architecture/`,
    `docs/adr/` (ADR-0001..0011), `docs/interfaces/ICD.md`, `docs/glossary.md`,
    `docs/roadmap.md`, `docs/verification/formal-verification.md`.
  - `docs/quality/ecss-criticality-b.md`, `class-a-gaps.md` (GAP-A1..A6),
    `licensing.md`.
  - V&V: `docs/vv/{verification-plan,verification-results,tool-qualification}.md`.
  - Reviews: `docs/engineering/independent-review.md` (profiles R1, R2, R3),
    `docs/reviews/` template and log.
- Licensing: AGPL-3.0-only (`LICENSE`), SPDX headers repo-wide (norm N13).

### Changed

- (first release; no prior history)

### Fixed

- Lean proofs: added missing `import Frick.Common` in `Frick.SoftwareBus`,
  replaced the Mathlib-only `norm_num` with core tactics, made `TmCounter` an
  `abbrev` and unfolded `def` constants before `omega`; `lake build` now passes.
- PUS service 6 memory dump: corrected the S6 unit test to write the address and
  size in network (big-endian) order per the ICD, and made `svc6_dump` bound the
  requested range with overflow-safe 32-bit address arithmetic while reading
  from the registered region base (host-safe and target-accurate).
- Traceability gate: `THEOREM_RE` now uses `re.MULTILINE`; the CI evidence
  parser accepts both `ci-<name>` and bare `# evidence:` names and multiple
  claims per line; `ID_RE` accepts the `GAP-A<n>` identifiers.

### Security

- See `SECURITY.md`.

### Norms compliance status

All norms N1-N16 applied; gates PASS: host build, ctest, `lake build`,
`trace.py`, simulator smoke (6 apps), static analysis, RTEMS cross build proxy
(toolchain install via `tools/scripts/setup_rtems.sh`). Class-A gaps (GAP-A1..A6)
documented and tracked.
