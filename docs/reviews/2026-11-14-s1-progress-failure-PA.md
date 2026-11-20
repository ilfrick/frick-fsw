# PA Review — S1 progress-failure emission (NC-EXT-94), uncommitted working-tree delta

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0521                      |
| Reviewer profile                 | PA — Senior SW Product Assurance (norm N20) |
| Reviewer session                  | Sub-agent (child session of the FSW-engineer session); **contextual independence only** |
| Date                             | 2026-11-14                             |
| Reference revision                | Committed HEAD `fc68b9d121bce2194c1beaaa49a7dc4cdab0552c` + uncommitted working tree (9 modified files, 1 untracked evidence directory) |
| Items under review                | `src/pus/pus.c`, `tests/unit/test_pus_s11_residual.c`, `CHANGELOG.md`, `README.md`, `docs/planning/sprint-plan.md`, `docs/status/handover.md`, `docs/status/loop-audit.md`, `docs/vv/test-spec/ts-1-protocol-core.md`, `docs/vv/verification-results.md`, `docs/vv/evidence/2026-11-14-s1-progress-failure/` |
| Scope                            | Process conformance N1–N23, NC-register integrity, gate/evidence integrity, class-B adequacy, release-readiness boundary |
| Norms applied                    | N1–N23 (`docs/rules.md`); ECSS-Q-ST-80C Rev.2, ECSS-E-ST-40C Rev.1 (space scope only) |
| Target criticality               | ECSS class B (class-A gaps must be documented) |

## 0. Independence declaration and limits of this review

- This is a **sub-agent review: contextual independence only.** It is *not*
  organizational independence. The reviewer shares the parent session's model,
  provider and tooling, and reviewed artifacts authored in that same session
  lineage. It therefore **cannot** discharge N20's independence requirement on
  its own and must not be cited as an independent PA acceptance.
- The reviewer **may not** close NCs, accept residual risk, approve waivers,
  approve release, or authorize commit/push. Those remain with the authorized
  human/project authority.
- Read-only: no file was modified except this report; nothing was built, run,
  committed, pushed or deleted. No Docker command was executed.
- Standards were not re-fetched in this session; no clause-level claim about
  ECSS/NASA text is made beyond what the repository records.

## 1. Hash verification (binding precondition)

`sha256sum` executed on 2026-11-14 before any content was read. **All nine
candidate hashes matched the values supplied to this review; no mismatch, so the
review proceeded.**

| File | Verified sha256 |
| --- | --- |
| `src/pus/pus.c` | `195b8cf5754e7b4043f8b62bcc0c47d06ff2e10e67cdc2ef2ad054dd8c22293b` |
| `tests/unit/test_pus_s11_residual.c` | `33b43b961616b3b5a91925c75982054e07bda8b9b23fd57707ae9a30f3b42a13` |
| `CHANGELOG.md` | `a35e9ea4dead23f3bc29714e62c719699fb6b1df1756ec780d1c7d670de8a4d7` |
| `README.md` | `5b119888eb176e10747459021b54bd5605fc1adf44f8d689bf018e1caa5de9b6` |
| `docs/planning/sprint-plan.md` | `27a42bd3568a605fedc6b2e8bbad74782ee9a0b5a273b4bcc97ddc577a29985d` |
| `docs/status/handover.md` | `ac374ec65e39069cfba39b9f5db0844e0bf507e716c457d9be837e1f4767cc11` |
| `docs/status/loop-audit.md` | `c36a9e645c8795ed008010fc98d1f5d5e16af83dc501922a0934f7cb8d4b2b41` |
| `docs/vv/test-spec/ts-1-protocol-core.md` | `7ceaa4b5bf403fa72fb3b25cd1fa83e6e1a59f0e2cdfcf7fcd915f649bacce95` |
| `docs/vv/verification-results.md` | `8ab0fab7f48b5ded9ffd065bc6124e3f963200d548877ae1e7be6edc010450bd` |

Working-tree state confirmed: 9 modified files, 1 untracked directory
(`docs/vv/evidence/2026-11-14-s1-progress-failure/`), no commit, no push.
HEAD confirmed `fc68b9d121bce2194c1beaaa49a7dc4cdab0552c`
("ICD: document TM[1,6] emission support and TC[20,1] set (NC-EXT-94/95)").

All 14 files in the evidence directory were hashed; the ones cited below are
bound to these digests:

| Evidence file | sha256 |
| --- | --- |
| `build.log` | `0d2a824875825caf0fce34153a18aee0162d74d6db6f74520f6924007d3106c6` |
| `ctest.log` | `0d29bd0e417e5a2a05dfb47cd6bb9f18859d936a7f2c18e9b162f1413b05ad64` |
| `focused-pus.log` | `cb79071561d31d11f78ccc41b1d7f517718fc2822e0ec6eb46f4ff3d4f34bb0c` |
| `format-changed-files.log` | `719c8c1be79ff25775baa23cf7229fb8216ef0d9b291316beb6309ee85b1f0d2` |
| `format-full.log` | `c2f7e17af874c8337053fc8e98c377b77173a0e6f698564cb31c7345d81e941d` |
| `gsw.log` | `546c1f03a1b479d94f0f43105e14bd94d00faeb75de2bd3f62d9bf7ea70fcdf0` |
| `smoke.log` | `349dd8000596e2d2f241eb9ed85e7147ed25c0180e5c64f96448c40c77a35a0c` |
| `smoke-safe.log` | `4c5e5a1e3196ace42ee03501775d9052bb104da04a7177f467fb9af19daba249` |
| `renode-process.log` | `91ddfbed24c01ce13680bd0f8bb1edc7390a6090afe6f969fd17979895da9782` |

## 2. Scope

Reviewed for assurance/process conformance, not code style:

1. Norm conformance N1–N23 as objectively verifiable from the working tree.
2. NC-register integrity (94/95/245/246/43/44): open state, owner, milestone,
   absence of duplication/misattribution, and preservation of historical text.
3. Gate integrity: traceability of each claimed result to a retained raw log,
   and honesty about the three failed gates.
4. Class-B adequacy and class-A gap documentation for this increment.
5. Release/commit readiness per N7 and the stated boundary.
6. Evidence integrity: hashes, directory identity, absence of fabricated or
   internally inconsistent logs.

Excluded: functional code correctness review (that is R1/R2 scope; a bounded
structural read was performed only where needed to judge the assurance claims),
test-value adequacy beyond N22, and any authority disposition.

## 3. Findings

Severity: **Blocker** = the delta must not be committed until corrected;
**Major** = non-conformity that must be corrected or formally justified before
the applicable gate; **Minor** = must be corrected or justified, not
gate-blocking on its own; **Comment** = observation, no derogation.

### PA-001 — Blocker — Gate evidence does not bind the tested binaries to the candidate

- `docs/vv/evidence/2026-11-14-s1-progress-failure/build.log` contains **zero**
  compile/link steps (all 76 lines are `Built target …`), whereas the prior
  cycle's log for the same tree contains 146 `Building C object` lines
  (`docs/vv/evidence/2026-11-13-icd-s3-s6-contract/build.log`). `ctest.log`
  likewise has no build output. The retained build log is therefore a no-op
  incremental build: it does not evidence that `src/pus/pus.c` or
  `tests/unit/test_pus_s11_residual.c` were compiled, and the claim
  "Build | exit 0; 0 compiler warnings (`-Wall -Wextra -Werror`)"
  (`docs/vv/verification-results.md:1047`, repeated at `CHANGELOG.md`, and in
  handover 3.95 / sprint 1.153) is unsupported — "0 warnings" is vacuous when
  nothing is compiled.
- **No file in the evidence directory contains the string `checks=`**, yet
  `docs/vv/verification-results.md:1050` asserts
  "`test_pus_s11_residual` | checks=1176, failures=0". `1176` is exactly the
  check count reported for the *pre-delta* revision of the same test file
  (`docs/vv/evidence/2026-11-13-icd-s3-s6-contract/pus-s11-residual.log`:
  `checks=1176 failures=0`). The delta adds 29 assertions (26 in
  `test_s1_progress_fail_regression`, 3 in the extended `test_s9_time_subservices`
  case), so the post-delta count must be ≈1205, not 1176.
- Consequence: either the number was carried over from the previous cycle's log
  (claim without evidence), or the binary executed by `ctest` predates the delta
  (then the 60/60 and 7/7 PASS results do not evidence the delta at all). The
  evidence set cannot distinguish these cases.
- Assurance impact: the N15 claim "complete cycle re-run to success" is not
  established for this candidate, and `tests/unit/test_pus_s11_residual.c` is
  the only control for the changed wire behaviour.
- Recommendation: rebuild in the isolated build directory with a retained
  compile log (`cmake --build … 2>&1 | tee`), retain the harness output line
  (`test_pus_s11_residual.c checks=N failures=0`) for the exact candidate, and
  correct the `checks=` figure to the measured value. Optionally retain a
  mutation control (see PA-011) that fails on the pre-delta `pus.c`.

### PA-002 — Blocker — N3/N4 traceability not converged, and the recorded justification is factually wrong

- `docs/vv/verification-results.md:986-989` states: "SRS-REQ-006 already lists
  `tests/unit/test_pus_s11_residual.c` (line 106 of traceability.yaml), so no
  traceability mapping change was required".
- This is false. `docs/requirements/traceability.yaml:85-90` (SRS-REQ-006)
  lists `tests: ["tests/unit/test_pus.c", "tests/unit/test_r3_gap.c"]` — the
  new test is absent. Line 106 of that file is the `tests` list of
  **SRS-REQ-008** (`docs/requirements/traceability.yaml:101-108`), which does
  contain `test_pus_s11_residual.c`; the two requirements were conflated.
- The human-readable mirror is also un-updated:
  `docs/requirements/traceability.md:79` (`SRS-REQ-006 | pus.c | test_pus.c`).
- The new test-spec oracle `N22-TS1-105` is explicitly bound to **SRS-REQ-006**
  (`docs/vv/test-spec/ts-1-protocol-core.md:147`), so the requirement→test link
  the delta relies on does not exist in the traceability record. `make trace`
  PASS (`trace.log`) cannot detect this because it validates the manifest's own
  references, not the test-spec→manifest link.
- Assurance impact: N3 (backwards SRS/SSS coverage) and N4 (bidirectional
  traceability converged at every commit) are non-conforming, and a controlled
  record contains a false statement used to justify omitting the N3/N4 update.
- Recommendation: add `tests/unit/test_pus_s11_residual.c` to the SRS-REQ-006
  `tests` list in both `traceability.yaml` and `traceability.md`, re-run
  `make trace`, and replace the false justification with the actual change.

### PA-003 — Major — NC-EXT-95 historical text was rewritten in place (record integrity)

- `docs/planning/sprint-plan.md:233` (NC-EXT-95 row). The **defect statement**
  was replaced by a current-state statement: previously "`FRICK-ICD-001` §4
  Service 20 row lists only the parameter report (TM) side although …", now
  "… §4 Service 20 row **now lists both** the parameter report (TM) side and the
  TC[20,1] set side (ICD rev 1.9, line 123) …". The recorded **corrective
  action** ("add the registered TC[20,1] set subservice (and its payload) to the
  ICD row") was deleted, and the disposition was rewritten to begin
  "**NO PRODUCTION CODE MISSING:**".
- NC-EXT-94, by contrast, was extended append-only with a dated
  "Implementation status appended (2026-11-14)" paragraph and retains its full
  historical rationale (`docs/planning/sprint-plan.md:232`). The inconsistent
  treatment of two sibling NCs in the same commit is itself a signal.
- Assurance impact: the register no longer preserves the objective defect
  statement and the corrective action for NC-EXT-95, so a later reader cannot
  reconstruct what was found, what was required, or what remains open. This
  conflicts with N12/N16 record-keeping expectations and with the project's own
  append-only precedent. NC-EXT-95 also remains labelled OPEN while its defect
  column now asserts conformance, leaving its scope ambiguous.
- Recommendation: restore the original defect and corrective-action text
  verbatim and append the 2026-11-14 status as a dated note (as done for
  NC-EXT-94). Do not close or re-scope NC-EXT-95 without interface-authority
  disposition.

### PA-004 — Major — TS-1 rev 1.14 history claims a §3.1 update that was not made, and the claimed split contradicts the new row's own status

- `docs/vv/test-spec/ts-1-protocol-core.md:349` (history, rev 1.14) states
  "§3.1 totals updated (105 cases = 76 `covered` + 29 `gap`)".
- `docs/vv/test-spec/ts-1-protocol-core.md:291-300` (§3.1) is **unchanged**:
  Total cases 104, nominal 48, out-of-bound 51, mixed 5, individual 93,
  combined 11, `covered` 76, `gap` 28. The diff for this file touches only the
  revision table, the §2 range statement and the new row.
- Additionally, the new row `N22-TS1-105` carries the status token
  "host covered (…)" (`:147`), which per the document's own counting convention
  (`:99-100`) counts as `covered`. The arithmetically consistent totals would be
  **105 = 77 `covered` + 28 `gap`**, and mixed = 6 (the case is
  "nominal + OOB"). The stated "76 + 29" is inconsistent with the document's own
  taxonomy.
- The wrong figure propagates to `docs/status/loop-audit.md:453` (N22 row),
  `docs/status/handover.md` (history 3.95), `docs/planning/sprint-plan.md`
  (history 1.153) and `docs/vv/verification-results.md:990-992`.
- Assurance impact: N16/N17 documentation accuracy; a controlled V&V record
  misdescribes its own content. Not a functional or coverage-claim defect, but it
  is the class of defect that previously produced NC-EXT-92 (TS-4 count
  mismatch) and must not be left to accumulate.
- Recommendation: update §3.1 (and the mixed/individual counts) to the
  arithmetically correct values for the chosen status token, or change the
  status token to `gap` with an explicit rationale; then align the four
  dependent records. This is a disposition decision for the TS-1 author +
  independent R4, not a unilateral PA edit.

### PA-005 — Major — The N19 audit's baseline commit identifier does not exist

- `docs/status/loop-audit.md:9` states the baseline commit is
  `fc68b9dbe7dac9309c1c4a3e3bcaf16fb2c320cd`. No such object exists in the
  repository (`git cat-file -t` → *could not get object info*). The actual HEAD
  is `fc68b9d121bce2194c1beaaa49a7dc4cdab0552c`. The document itself uses the
  correct short form `fc68b9d` at `:423`, so the header field is internally
  contradicted.
- Assurance impact: the N19 audit exists to fix the baseline at loop start; an
  unverifiable baseline identifier defeats configuration-status accounting and
  prevents a reviewer from binding the audit to a tree. N19 is therefore only
  partially satisfied even setting aside the (honestly recorded) sequencing
  deviation NC-EXT-245.
- Recommendation: correct the hash to the full `fc68b9d121bce…` value and record
  the correction in the revision history.

### PA-006 — Major — "Pinned clang-format 18 via Docker" claim is not supported by any retained log

- `docs/vv/verification-results.md:1053` claims
  "Format (changed files, pinned clang-format 18 via Docker `frick-vv:local`) |
  `clang-format --dry-run --Werror` PASS", repeated at `CHANGELOG.md`,
  `docs/status/loop-audit.md:451` and handover 3.95.
- `docs/vv/evidence/2026-11-14-s1-progress-failure/format-changed-files.log`
  contains exactly one line: `FORMAT_CHANGED_EXIT=0`. It records no tool
  version, no container/image identity, no command line, no file list and no
  output.
- This reproduces precisely the defect already registered as **NC-EXT-101**
  ("format/docs logs attribute PASS from absence of output … neither log records
  the exit status or an explicit PASS marker"). The prior cycle retained
  `correction-format-pinned.log` with a `PINNED_FORMAT_CHANGED_FILE_EXIT` marker
  and an explicit tool-version statement; this cycle regressed below that
  standard.
- The claim also contradicts the immediately preceding text in the same document
  at HEAD, which recorded that only host clang-format 23.1.3 is installed and
  that the pinned-18 gate is unavailable on the host — so the pinned-18 result
  depends entirely on a Docker invocation for which no log exists
  (`docker-logs.stderr` is an empty file and `status.env` records the *Renode*
  container).
- Assurance impact: N10 — a tool-identity claim (pinned 18 vs host 23) is the
  whole point of the gate, and it is unverifiable. The narrow claim "the two
  changed files are clang-format-clean under some formatter" is plausible but
  also unevidenced, since no file list is retained.
- Recommendation: retain a log containing `clang-format --version`, the exact
  command, the file list and an explicit PASS/exit marker (or the raw
  `--dry-run --Werror` output); reference NC-EXT-101 in the record.

### PA-007 — Major — N18 is not re-established for this delta and no NC covers the gap

- N18 requires functional validation of **every change** in the Renode emulated
  PolarFire SoC environment. This delta changes production wire behaviour in
  `src/pus/pus.c:455-465` (progress verification moved after `handler(...)`,
  TM[1,6] now emitted).
- The Renode artifacts in the evidence directory are from the **prior** run, not
  this candidate: `renode-process.log` timestamps run 11:26:43–11:26:56 while
  every other log in the directory is stamped 12:40; the image under test is the
  2026-11-13 ELF (`41acb185…cb20`) which predates the delta.
- The documents do say "retained from prior run" (`docs/status/handover.md:23`,
  `docs/status/loop-audit.md` N18 row, sprint 1.153). That is honest. However
  (a) the N18 norm row still reads as a PASS without stating that the ELF does
  not contain the delta, and (b) **no NC registers "N18 not re-run for this
  change"**. NC-EXT-246 covers only the Docker runner preflight/cleanup
  deviation, which is a different condition.
- Assurance impact: under N21 this is the textbook "absence of target/hardware
  evidence" case — it must be opened (or extended) with owner and milestone and
  dispositioned "open to system, non-blocking". As it stands the gap is
  invisible in the register while the norm table reads PASS.
- Recommendation: state explicitly that the Renode ELF predates the delta and
  the delta has no emulated-target evidence; register the gap as an NC (or
  extend NC-EXT-246 with a clearly separate item) with owner + milestone
  (e.g. next N18 cycle before commit). No host/hardware claim is implied either
  way.

### PA-008 — Major — Evidence-set integrity: no digest manifest, and the directory is described as committed when it is untracked

- `docs/vv/verification-results.md:1011` describes
  `docs/vv/evidence/2026-11-14-s1-progress-failure/` as a
  "repo-committed evidence directory". It is **untracked** (`git status`:
  `?? docs/vv/evidence/2026-11-14-s1-progress-failure/`) and nothing in this
  cycle is committed. The word "committed" is factually wrong at review time and
  will silently become true or false depending on the next commit.
- The directory carries **no digest manifest**, unlike prior cycles
  (`docs/vv/evidence/2026-11-13-uart-restore-error/run-assets.sha256`,
  `…/2026-11-13-icd-s3-s6-contract/assets.sha256`). There is therefore no
  tamper-evidence binding for the retained logs and no revision/history record
  for the evidence set (cf. NC-EXT-100).
- `docker-logs.stderr` is a zero-byte file presented alongside real logs; it
  conveys nothing and should either carry content or be removed from the set.
- Assurance impact: N11 (results retained) is satisfied only in the weak sense
  that files exist; N16/CM integrity for the evidence set is not established.
- Recommendation: add an `assets.sha256` (or equivalent) covering the retained
  logs, correct the "repo-committed" wording to "working-tree evidence
  directory (untracked at review time)", and keep the manifest with the set
  through commit.

### PA-009 — Minor — NC-EXT-94 and NC-EXT-95 have no explicit milestone/target

- `docs/planning/sprint-plan.md:232-233`: both rows state owner
  ("FSW + interface/requirements authority" / "FSW + interface authority") and
  an open condition ("pending interface-authority disposition") but no
  `Milestone:` field. NC-EXT-245/246 (`:332-333`) do carry owner **and**
  milestone, so the register is internally inconsistent.
- N19 requires every gap to be "closed or tracked with owner/milestone".
- Recommendation: add an explicit milestone (e.g. "M15 interface-authority
  review of ICD §4" / "before any PUS-C profile claim") to both rows.

### PA-010 — Minor — The `vm.c` SPDX finding is cross-referenced to NC-EXT-44 where the mapping is questionable

- The failure recorded in `gsw.log` is
  `tools/patches/n7s-obcp/src/modifications/py/vm.c: missing MPL-2.0 SPDX
  header`. It is attributed to NC-EXT-43 (external/generated boundaries) *and*
  cross-referenced to NC-EXT-44 ("T21 checker semantics",
  `docs/planning/sprint-plan.md:181`).
- A genuinely missing SPDX label is a licensing/provenance condition, not a
  checker-semantics defect; NC-EXT-44 concerns the checker accepting malformed
  or compound expressions. The NC-EXT-43 attribution is sound; the NC-EXT-44
  cross-reference risks diluting the finding into "the checker is at fault".
- Note also that NC-EXT-246 was allocated to this finding and then reallocated
  away (`docs/status/loop-audit.md` history 11.06→11.07,
  `docs/planning/sprint-plan.md:740`). The reallocation is transparently
  recorded and is not itself a finding; but after it, the licensing finding has
  no milestone beyond NC-EXT-43's "postponed to the last sprint".
- Recommendation: drop or justify the NC-EXT-44 cross-reference and state the
  disposition route (license-authority decision on whether the file is
  in-scope for MPL-2.0 labelling or must be excluded from the checker's managed
  set).

### PA-011 — Minor — No pre-fix/mutation control for the new regression test

- The new `test_s1_progress_fail_regression` (`tests/unit/test_pus_s11_residual.c:393-475`)
  is a correct regression control by construction: at the pre-delta revision the
  progress TM is emitted as TM[1,5] before the handler runs, so
  `stream_count(1u, 6u) == 1u` would fail. That analysis is sound but is **not
  demonstrated by retained evidence**.
- Project precedent retains such a control explicitly (NC-EXT-237: a pre-fix
  mutation control failing 33 checks; `test_pus_fu` mutation logs under
  `docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/`).
- This is exactly the control that would have detected PA-001 (a stale or
  non-rebuilt test binary).
- Recommendation: retain a mutation log showing the new assertions failing
  against the pre-delta `pus.c` and passing against the candidate. (V&V/R2
  scope; recorded here because it bears on the credibility of the gate set.)

### PA-012 — Minor — The `smoke-safe` gate does not evidence what the script's header claims

- `tools/scripts/run_sim.sh` documents that it asserts "telemetry is produced on
  the loopback link (EVS → PUS service 5)", but the script only checks the
  banner and the application count; the third claim is not implemented.
- The substitute PASS used for this cycle
  (`docs/vv/evidence/2026-11-14-s1-progress-failure/smoke-safe.log`) therefore
  evidences boot + 7 app registrations only. The script also deletes its raw
  simulator log (`rm -f "${LOG}"`), so no console output is retained.
- Cross-check performed (resolves an apparent inconsistency): the log's
  "7 applications registered" is **consistent** with the native build
  (`FF_ENABLE_RENODE_TEST` defaults OFF, `CMakeLists.txt:24`,
  `src/main/fsw_main.c:105-107`), while the Renode image registers 8
  (`uart-console.log`). The mismatch is not evidence of fabrication.
- Recommendation: either implement the telemetry assertion or correct the header
  claim, and retain the simulator console log for the smoke gate (N22 explicit
  oracle; N10 tool adequacy).

### PA-013 — Minor — Loop-audit revision-history rows are inserted above the table header separator

- `docs/status/loop-audit.md:481-483` (rows 11.05–11.07) sit **before** the
  `| --- | ---------- | --------- | ------ |` separator at `:484`, so they
  render outside the history table. The pattern pre-exists (11.02–11.04), but
  this delta extends it by three rows.
- Recommendation: move the separator/header above the newest rows (documentation
  structure; R4 scope).

### PA-014 — Minor — Class-A gap attribution for this increment is generic, not increment-specific

- Class-B adequacy for this delta rests on: host-only test evidence (SIL), no
  formal model of `FF_PusIngest` stage/subservice selection or handler-result
  ordering (the Lean theorem `req_srs006_acceptance_completion` in
  `verification/lean/Frick/Requirements.lean:58` models only the packet *count*
  from ack flags, not subservice identity), no third-party independent
  verification of the new test, and no emulated-target re-run.
- These are covered at a generic level by GAP-A1 (deep formal verification),
  GAP-A2 (full PUS-C service coverage), GAP-A5 (on-target qualification) and
  GAP-A11 (third-party V&V) in `docs/quality/class-a-gaps.md`, so N5 is
  satisfied in substance. They are **not** annotated for this increment.
- Recommendation: add one line to the increment record naming the class-A gaps
  that apply (GAP-A1/A2 for the unmodelled subservice selection, GAP-A5 for the
  missing emulated-target run, GAP-A11 for test independence). No class-A claim
  is at risk.

### PA-015 — Comment — The production change is coherent under PA scrutiny

- `src/pus/pus.c:455-465`: the handler still executes under `g_pus_mutex` (the
  lock/unlock discipline around `FF_PusSendVerification` is unchanged), so no
  new concurrency or re-entrancy hazard is introduced; exactly one progress
  verification is emitted per ingest by construction, satisfying the
  "never both 1,5 and 1,6" claim; acceptance/start/completion and the early
  returns for illegal APID and unknown handler are untouched.
- The failure code is fixed at `FF_PUS_FAIL_EXECUTION` rather than the specific
  handler return, which mirrors the existing completion path and remains under
  the provisional code allocation tracked by NC-EXT-18.
- No safety-relevant behaviour change was identified in the reviewed paths.

### PA-016 — Comment — Interaction with NC-EXT-243 (unsynchronised sequence counter)

- Moving progress emission after the handler changes the emission ordering that
  NC-EXT-243 (function-local static `s_sequence_count` incremented outside the
  mutex, `src/pus/pus.c:332`) already flags. The new assertions read the
  verification code octet (offset 27), not the sequence-count octets, so the new
  test is not order-dependent; no new defect, but NC-EXT-243 remains the correct
  tracker and should not be re-scoped to this delta.

### PA-017 — Comment — Gate-failure honesty is a genuine strength; no suppression found

- The three failed gates are declared consistently and not suppressed:
  default `make smoke` exit 2 (CMake cache bound to `/workspace`,
  `smoke.log`), `make gsw-selfcheck` exit 2 (`gsw.log`), full pinned
  `format-check` exit 123 (`format-full.log`, violations only in files outside
  the delta: `src/obcp/ff_obcp_inhouse.c`, `src/pus/pus_services.c`,
  `src/transport/link/link_ip.c`, `src/transport/link/link_tsn.c`,
  `tests/unit/test_link_tsn.c`, `tests/unit/test_obcp_abort.c`,
  `tests/unit/test_obcp_budget.c`, `tests/unit/test_obcp_inhouse.c`,
  `tests/unit/test_pus_s18.c` — the two changed files do not appear).
- Cross-check of the failed-gate exit codes against the logs found no
  inconsistency: each `*_EXIT=` marker matches the log body. No log appears
  fabricated; the defects found (PA-001, PA-006, PA-008) are defects of
  *sufficiency and binding*, not of truthfulness.
- Independent reviews (N9 R1/R3, R2/R4, N17, N20) are marked UNVERIFIED
  throughout — correctly, and no review report was fabricated.

### PA-018 — Comment — Review index not updated

- `docs/reviews/README.md` has no row for this report. This reviewer may create
  only the report file, so the index row must be added by the FSW engineer
  (documentation, R4/N17 scope). Suggested row: 2026-11-14 · S1 progress-failure
  emission (NC-EXT-94) delta · PA / N20 · **REJECTED (contextual independence
  only)** · `2026-11-14-s1-progress-failure-PA.md`.

## 4. Norm-by-norm conformance assessment

| Norm | Result | Basis |
| --- | --- | --- |
| N1 Lean on code artifacts | **Partial** | `lean.log` exit 0 but is a no-op rebuild ("Build completed successfully", no compile output); no theorem models the changed stage→subservice selection (PA-014). No new generated artifact. |
| N2 English | **Pass** | Docs, comments, commit-style text all English. |
| N3 SRS coverage | **Fail** | PA-002. |
| N4 Bidirectional traceability | **Fail** | PA-002; `trace.log` PASS is a manifest-internal check and does not detect the missing link. |
| N5 Class B / class-A gaps | **Pass in substance** | Class-A gaps documented generically (PA-014); class-B deliverable unchanged. |
| N6 Docker CI/CD | **Partial, declared** | CI not re-run for an uncommitted candidate; the one Docker-dependent claim (pinned format 18) has no log (PA-006). |
| N7 Commit/push authority | **Pass** | Nothing committed or pushed; the worktree state is described as uncommitted in handover, loop-audit, sprint-plan and CHANGELOG. Correctly withheld. |
| N8 Docs current | **Partial** | Docs updated in the same working tree, but with three accuracy defects (PA-004, PA-005, PA-008). |
| N9 Independent review | **Unverified** | Declared UNVERIFIED; this sub-agent review is contextual independence only. |
| N10 Tool qualification | **Partial** | No V&V tool changed, so no new qualification delta is required; T3 remains NOT FULLY QUALIFIED (stated); tool-identity claim unevidenced (PA-006, PA-012). |
| N11 Evidence retained | **Partial** | Logs retained, but untracked, without digest manifest, and one gate figure has no supporting log (PA-001, PA-008). |
| N12 NC resolution | **Fail (record integrity)** | NC-EXT-95 historical text rewritten (PA-003); no NC silently closed or merged. |
| N13 Licensing | **Partial, tracked** | `gsw-selfcheck` fails on an unchanged upstream-derived file; tracked under NC-EXT-43 (+44, see PA-010). No delta regression. |
| N14 SSS review | **N/A** | No SSS change. |
| N15 Full cycle re-run | **Fail** | Gate results not attributable to the candidate (PA-001); three gates failed and were declared honestly (PA-017). |
| N16 Revision history | **Partial** | Every touched document has a history entry; two entries contain factual errors (PA-004, PA-005) and one rewrites an NC (PA-003). |
| N17 Documentation review | **Unverified** | Declared UNVERIFIED; not performed. |
| N18 Renode functional validation | **Fail for this delta** | Not re-run; prior-run artifacts retained and honestly labelled, but no NC covers the gap (PA-007). |
| N19 Loop audit | **Partial** | Audit and §13 assessment exist; sequencing deviation registered as NC-EXT-245; baseline hash invalid (PA-005). |
| N20 PA review | **Not satisfied** | This review provides contextual independence only; it cannot discharge N20. |
| N21 NC triage | **Partial** | Functional fix implemented, non-functional findings registered, nothing silently closed; the N18 gap lacks an NC (PA-007); NC-EXT-95 re-scoped (PA-003). |
| N22 Test design | **Partial** | Nominal + out-of-bound cases with explicit expected results and a discriminating oracle (`N22-TS1-105`); no pre-fix/mutation control (PA-011); smoke oracle weaker than documented (PA-012). |
| N23 Proceed-if-permitted | **Pass** | The FSW engineer proceeded on a rule-authorized functional fix and recorded the decisions and residual gaps. |

## 5. NC register integrity (requested checks)

| Check | Result |
| --- | --- |
| NC-EXT-94 remains OPEN | **Yes** — append-only "Implementation status appended (2026-11-14)" with "**REMAINS OPEN** pending interface-authority disposition"; historical rationale intact (`docs/planning/sprint-plan.md:232`). |
| NC-EXT-95 remains OPEN | **Yes, but** the historical defect statement and corrective action were overwritten (PA-003). |
| NC-EXT-245 open with owner + milestone | **Yes** — owner FSW engineer, milestone "next N19 loop start", disposition "open to system, non-blocking under N21" (`:332`). |
| NC-EXT-246 open with owner + milestone | **Yes** — owner FSW engineer, milestone "before any future Docker runner invocation", blocker to future Docker use only (`:333`). |
| NC-EXT-43/44 not duplicated | **Yes** — one row each; the `vm.c` observation is appended to NC-EXT-43, no new licensing NC created. |
| NC-EXT-43/44 not misattributed | **Mostly** — NC-EXT-43 attribution sound; the NC-EXT-44 cross-reference is questionable (PA-010). |
| Historical NC text preserved | **No** for NC-EXT-95 (PA-003); **Yes** for NC-EXT-94, 43, 44. |
| Any NC silently closed, merged or mis-scoped | **None closed or merged.** NC-EXT-95 effectively re-scoped from "documentation defect" to "closure sign-off" without an authority disposition (PA-003). NC-EXT-246's reallocation away from licensing is transparently recorded in the history (11.06→11.07) and is not a silent change. |

## 6. Gate integrity (requested checks)

| Gate | Claimed | Retained evidence | PA assessment |
| --- | --- | --- | --- |
| Configure | exit 0 | none (no configure log) | **Unsupported** (folded into PA-001) |
| Build | exit 0, 0 warnings | `build.log` (no compile lines) | **Not evidenced** — PA-001 |
| CTest 60/60 | PASS | `ctest.log` (60/60, exit 0) | Result present, but **not attributable** to the candidate — PA-001 |
| Focused PUS 7/7 | PASS | `focused-pus.log` (7/7, exit 0) | Same attribution limit — PA-001 |
| `checks=1176` | PASS | **no log contains `checks=`** | **Unsupported / inconsistent** — PA-001 |
| Lean | PASS | `lean.log` (exit 0, no-op rebuild) | Plausible, weak evidence |
| Trace | PASS 120/76/30/14/68 | `trace.log` | PASS present; blind to PA-002 |
| Format (changed files, pinned 18) | PASS | `format-changed-files.log` (exit marker only) | **Not evidenced** — PA-006 |
| `make smoke` (default) | **FAILED exit 2** | `smoke.log` (CMake cache mismatch) | **Honestly declared** — PA-017 |
| `run_sim.sh` (isolated BUILD_DIR) | PASS | `smoke-safe.log` | PASS consistent with the native 7-app configuration; weaker oracle than documented — PA-012 |
| `make gsw-selfcheck` | **FAILED exit 2** | `gsw.log` | **Honestly declared**; pre-existing, unchanged file |
| `make format-check` (full, pinned) | **FAILED exit 123** | `format-full.log` | **Honestly declared**; no delta file among the violations |
| N18 Renode | PASS (prior run) | `renode-process.log`, `uart-console.log` | **Not this candidate** — PA-007 |
| Independent reviews | UNVERIFIED | — | Correctly declared; nothing fabricated |

No gate is presented as passed without a corresponding log *except* the
`checks=1176` figure (PA-001) and the pinned-format-18 claim (PA-006).

## 7. Verdict

**REJECTED — commit/push must remain withheld until PA-001 and PA-002 are
corrected**, with PA-003 through PA-008 corrected or formally justified before
the applicable gate.

- The functional change itself is sound, minimal and coherent (PA-015), and the
  delta's handling of *failed* gates and of *unobtained* independent reviews is
  candid and free of suppression (PA-017). This is materially better than the
  failure mode the project has previously recorded.
- The delta nevertheless fails the N15 gate-attribution test (PA-001) and the
  N3/N4 traceability test (PA-002), and it degrades controlled-record integrity
  in three places (PA-003 NC text, PA-004/PA-005 factual errors).
- N20 is **not** satisfied by this review: contextual independence only.

## 8. Residual risks and required actions

| # | Action | Owner | Blocking? |
| --- | --- | --- | --- |
| 1 | Rebuild the isolated build directory and retain a compile log proving the two changed files were compiled; retain the harness `checks=N failures=0` line for the exact candidate and correct the `1176` figure (≈1205 expected). Optionally retain a pre-fix mutation control. | FSW engineer (with V&V/R2) | **Yes** — PA-001 |
| 2 | Add `tests/unit/test_pus_s11_residual.c` to the SRS-REQ-006 `tests` list in `traceability.yaml` and `traceability.md`; re-run `make trace`; replace the false "line 106" justification. | FSW engineer | **Yes** — PA-002 |
| 3 | Restore the original NC-EXT-95 defect and corrective-action text verbatim and append the 2026-11-14 status as a dated note. | FSW engineer | Major — PA-003 |
| 4 | Correct TS-1 §3.1 (total 105, mixed 6, `covered`/`gap` consistent with the chosen status token) and align loop-audit/handover/sprint/verification-results. | TS-1 author + independent R4 | Major — PA-004 |
| 5 | Correct the loop-audit baseline hash to `fc68b9d121bce…`. | FSW engineer | Major — PA-005 |
| 6 | Retain a pinned-format log with tool version, command, file list and explicit PASS/exit; reference NC-EXT-101. | FSW engineer / tooling | Major — PA-006 |
| 7 | State that the Renode ELF predates the delta and register the missing N18 run for this change as an NC (or a distinct item under NC-EXT-246) with owner + milestone; disposition "open to system, non-blocking" per N21. | FSW engineer | Major — PA-007 |
| 8 | Add an `assets.sha256` for the evidence set; correct "repo-committed evidence directory" to "working-tree evidence directory (untracked)"; remove or populate the empty `docker-logs.stderr`. | FSW engineer | Major — PA-008 |
| 9 | Add explicit milestones to NC-EXT-94/95. | FSW engineer + interface authority | Minor — PA-009 |
| 10 | Justify or drop the NC-EXT-44 cross-reference for the `vm.c` SPDX finding. | FSW engineer + PA | Minor — PA-010 |
| 11 | Implement or correct the `run_sim.sh` telemetry claim; retain the simulator console log. | V&V/tooling | Minor — PA-012 |
| 12 | Add the class-A gap annotation for this increment (GAP-A1/A2/A5/A11). | FSW engineer | Minor — PA-014 |
| 13 | Add the review-index row for this report. | FSW engineer (R4/N17) | Comment — PA-018 |
| 14 | Obtain genuine N9/N17/N20 reviews with organizational independence before any commit or package attestation. | Project authority | Not satisfiable by this review |

**Residual risks that remain after the actions above** (not accepted by this
reviewer): the delta has no emulated-target and no on-target evidence; the
subservice selection and handler-ordering logic are outside the Lean model; the
regression test is host-only and single-implementation; NC-EXT-18 (provisional
code allocation), NC-EXT-243 (unsynchronised sequence counter) and NC-EXT-244
(pinned-format baseline) remain open and are unaffected by this delta.

## 9. Escalation statement

**No immediate safety or security concern requires escalation.** The reviewed
production change is a wire-behaviour correction in `FF_PusIngest` with
unchanged locking discipline and no hazard-control, boot, update, key-management
or privilege interaction. The findings in this report concern process,
traceability and evidence integrity; they are commit-blocking, not
safety-blocking. NC-EXT-246 (Docker runner preflight/cleanup) already records the
only security-adjacent condition, and it is open with owner and milestone.

This report does not approve, certify, waive, close or accept anything. It
records findings for correction and for independent human disposition.
