# PA review — PUS Service 1 ingestion-owned verification failures (S8-PUS-FU)

| Field | Value |
| --- | --- |
| Document id | FRICK-REVIEW-PA-PUS-S1-INGEST-20261112 |
| Norm | N20 (independent Software Product Assurance review), with N4, N8, N10, N11, N16, N17, N22 |
| Reviewer profile | Senior Aeronautics and Space Software Product Assurance Engineer |
| Date | 2026-11-12 |
| Repository / branch | `/home/nicola/fsw/frick-fsw`, `main` |
| Candidate baseline | `HEAD` = `e5f01d4` + uncommitted working-tree delta (4 modified files + 1 evidence directory) |
| Decision under review | Commit/push of the S1 ingestion-failure test increment |
| Review type | Independent delta review of the uncommitted change set |
| Result | **2 Major findings — commit/push NOT recommended until corrected.** 6 Minor findings and 2 observations are non-blocking. |

> **Authority statement.** This review is an independent assurance assessment. It
> does not approve release or flight, close or accept an NC, accept residual
> risk, qualify a tool, or authorize a commit. Disposition of every finding and
> the commit decision remain with the designated FSW engineer and the
> authorized project authorities.

## 1. What was reviewed

Working-tree delta against `e5f01d4`:

| File | Change | sha256 (working tree) |
| --- | --- | --- |
| `tests/unit/test_pus.c` | +`stream_has_verification` (helper), +`test_verification_failure_via_ingest`, +1 entry in `FF_TEST_MAIN` | `db8579c7db5d57ff5f3a205db08befbd8c5e0671e8cec69da8b799f593b109f2` |
| `docs/vv/test-spec/ts-1-protocol-core.md` | rev 1.10 → 1.11; new case `N22-TS1-104`; §2 range; §3.1 totals; history | (tracked, `git diff`) |
| `docs/vv/tool-qualification.md` | rev 1.7 → 1.8; T3 `test_pus.c` hash re-bound | (tracked, `git diff`) |
| `docs/vv/verification-results.md` | new cycle section (gates, mutation control, residual items) | (tracked, `git diff`) |
| `docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/` | 9 retained logs | (untracked, new) |

Declared out-of-scope and **not** closed by this change: NC-EXT-15, NC-EXT-19/90,
N22-TS5-47, GAP-A5, NC-PAL-01, NC-EXT-47. NC-EXT-211 untouched.

## 2. Method and independently re-executed evidence

I did not rely on the retained logs alone. Re-executed by me on the candidate
working tree:

| Check | Command | My result |
| --- | --- | --- |
| Unit test binary | `build/evidence-20261112-pus-s1/tests/test_pus` (after `cmake --build … --target test_pus`, target reported up to date) | `checks=315 failures=0`, exit 0 |
| Full CTest | `ctest` in `build/evidence-20261112-pus-s1` | `100% tests passed, 0 tests failed out of 57` |
| Lean (N1) | `lake build` in `verification/lean` | `Build completed successfully.`, exit 0 |
| Traceability (N4) | `make trace` | `PASS (120 requirements, 76 srs, 30 sss, 14 gaps, 68 lean theorems checked)` |
| Source immutability | `git diff HEAD -- src/`, `git status --porcelain -- src/` | empty, empty (`src/pus/pus.c` = `1eea40c06ecf399326347585995f71eec718318e535332f44edb043db3e15401`) |
| T3 hash binding | `sha256sum tests/unit/test_pus.c` | `db8579c7…109f2` — matches the re-issued row |
| TS-1 table recount | pipe-aware parser over the case table | 104 rows, no duplicate/missing IDs (1…104) |
| Mutation controls | 2 recorded + **2 additional** mutations applied to `src/pus/pus.c`, rebuilt, restored (see §5) | all four detected at the intended assertions; source restored and re-verified |

Not re-executed by me: the RTEMS cross build, Renode/N18, smoke, coverage and
the Docker CI image. The change contains no `src/` delta, so those gates cannot
have been affected by it; their prior evidence is unchanged and I make no claim
about it here.

## 3. Verification of the change against the review questions

### 3.1 Does the test genuinely exercise the ingestion-owned S1 failure branches?

**Yes.** Traced against `src/pus/pus.c`:

| Test sub-case | Test lines | Implementation branch | Discriminating? |
| --- | --- | --- | --- |
| (a) TC on TM APID `0x020`, ack = ACCEPTANCE | `test_pus.c:717-731` | `pus.c:420-429` (`apid != FF_APID_TC`, `ack_accept` → `FF_PusSendVerification(ACCEPTANCE, 0, FF_PUS_FAIL_ILLEGAL_APID)`, return `FF_ERROR`) | Yes (mutation 1) |
| (b) same, ack = 0 | `:733-740` | `pus.c:427-428` (unlock, `FF_ERROR`, no TM) | Yes (mutation 3) |
| (c) TC (17,9), ack = COMPLETION | `:742-758` | `pus.c:436-447` (`ff_pus_find` → NULL, `ack_complete` → TM (1,8) with `code`, return `FF_NOT_FOUND`) | Yes (mutation 2, partially) |
| (d) TC (0,1), ack = COMPLETION | `:760-770` | `pus.c:438-439` (`tc.service == 0u` → `FF_PUS_FAIL_UNKNOWN_SERVICE`) | Yes (mutation 2) |
| (e) TC (17,9), ack = 0 | `:772-781` | `pus.c:445-446` (`FF_NOT_FOUND`, no TM) | Yes (mutation 4) |

Supporting details verified, not assumed:

- Constants: `FF_PUS_FAIL_ILLEGAL_APID` = 1, `UNKNOWN_SERVICE` = 8,
  `UNKNOWN_SUBSERVICE` = 9 (`src/pus/pus_types.h:39-44`); `FF_APID_TC` = `0x010`,
  `FF_APID_TM` = `0x020` (`config/fsw_config.h:92-93`). The spec's code values
  (1, 8, 9) are correct.
- Packet offsets in `stream_has_verification` (`test_pus.c:680-698`) are correct:
  6-octet CCSDS primary header + 13-octet PUS TM secondary header ⇒ service at
  offset 7, subservice at 8; payload at 19 ⇒ Request ID 19..24, echoed service
  25, echoed subservice 26, code 27. The loop guard `(pos + 28u) <= g_rx_count`
  prevents reading past the capture, and the zero-progress guard prevents an
  infinite loop. This helper is *safer* than the pre-existing `stream_has`
  (see PA-010).
- The test resets `g_rx_count` before every ingest, so no assertion can be
  satisfied by residue from a previous sub-case.
- No production source was touched; the branches exercised are the shipped ones.

### 3.2 Is there an `src/` change, and does the record match code and logs?

**No `src/` change** — verified twice (§2). The record is otherwise materially
accurate: gate results match my re-execution, `0 compiler warnings` is correct
(`grep -c "warning:" gate-build.log` = 0; the only "warning" match is the
`n7s-obcp/flat/warning.c.o` file name), the T3 status stays **NOT FULLY
QUALIFIED**, and the documents explicitly refuse to close NC-EXT-15 /
NC-EXT-14..18 / NC-EXT-47. `docs/status/loop-audit.md:198` still records
NC-EXT-15 as OPEN, consistent with the change text.

Two statements do **not** match the code base — see PA-001 and PA-002. Both are
about the *classification and justification* of the new case, not about the
correctness of the test or the logs.

### 3.3 Are the TS-1 counts internally consistent?

**Yes, arithmetically** (recounted from the table, pipe-aware):

| Metric | Spec §3.1 | My count | Verdict |
| --- | --- | --- | --- |
| Total cases | 104 | 104 (IDs 1…104, no gaps, no duplicates) | OK |
| Nominal / OOB / mixed | 48 / 51 / 5 | 48 / 51 / 5 (=104) | OK |
| Individual / combined | 93 / 11 | 93 / 11 (=104) | OK |
| `covered` / `gap` | 78 / 26 | 78 / 26 (=104) | OK |
| SIL / PIL / HIL | 101 / 1 / 2 | 101 / 1 / 2 (=104) | OK |
| §3 gap list entries | — | 26 (20 + 3 + 1 + 2) | OK, matches 26 gap rows |

The arithmetic is consistent **given** the `covered` classification of
`N22-TS1-104`; that classification is what PA-001 disputes.

### 3.4 Is the T3 hash rebind factually correct, and is the status unchanged?

**Yes on both counts.** `sha256sum tests/unit/test_pus.c` =
`db8579c7db5d57ff5f3a205db08befbd8c5e0671e8cec69da8b799f593b109f2`, exactly the
value written into `tool-qualification.md:26` and history rev 1.8
(`tool-qualification.md:258`). The row still reads **NOT FULLY QUALIFIED**, and
rev 1.8 states explicitly that "execution is not qualification" and that "No N10
review exists at these bytes". The claim that the previous `cb9e7bb2…` candidate
hash was three revisions stale is correct (S20, S8, S9 and S1-progress case
additions). The `315` check count in the row matches my execution.

One inherited defect in the same row: PA-004.

### 3.5 Are the mutation controls real and discriminating?

**Yes.** I reproduced both recorded mutations and added two of my own. Each
mutation was applied to `src/pus/pus.c`, rebuilt, executed, and then restored
with `git checkout`; the restoration was hash-verified after each experiment
(`1eea40c0…5401`), and `git status --porcelain -- src/` is empty.

| # | Mutation | Result | Recorded? |
| --- | --- | --- | --- |
| 1 | `pus.c:424` `FF_PUS_FAIL_ILLEGAL_APID` → `FF_PUS_FAIL_BAD_DATA` | `checks=315 failures=1` at `test_pus.c:729` | Yes, identical |
| 2 | `pus.c:438-439` service-0 code selection removed | `checks=315 failures=2` at `:769`, `:770` | Yes, identical |
| 3 | `pus.c:421` illegal-APID `if (ack_accept)` → `if (1)` | `checks=315 failures=1` at `:740` | No (my addition) |
| 4 | `pus.c:440` handler-not-found `if (ack_complete)` → `if (1)` | `checks=315 failures=1` at `:781` | No (my addition) |

Mutations 3 and 4 confirm that the **ack-gating** assertions — the part of the
change the spec sells as "ack-gated emission" — are genuinely discriminating,
not just the code values. The controls are real. Their *record* is weak: PA-005.

### 3.6 Do any findings block commit/push?

PA-001 and PA-002 do (both are false statements in a controlled V&V record that
this change set introduces or re-issues, both correctable in the same commit).
The remaining findings do not block, subject to PA-008 (commit hygiene).

## 4. Findings

| ID | Severity | Subject | Blocks commit/push |
| --- | --- | --- | --- |
| PA-001 | **Major** | `N22-TS1-104` is classified `covered` although sub-cases (b)/(e) assert behaviour that the same spec classifies `gap` under NC-EXT-04 | **Yes** |
| PA-002 | **Major** | Novelty rationale is factually wrong: the (1,8) ingestion branch was already executed via `FF_PusIngest` in `test_r3_gap.c` | **Yes** |
| PA-003 | Minor | Spec claims "exactly one (1,2)" and a verified Request ID that the test does not assert | No |
| PA-004 | Minor | Re-issued T3 row still carries a stale `test_link_tsn.c` hash | No |
| PA-005 | Minor | `mutation-check.log` is not artifact-bound (no mutated file, patch, or hashes) | No |
| PA-006 | Minor | `gate-format.log` is 0 bytes; "exit 0" is not evidenced by content | No |
| PA-007 | Minor | TS-1 `Baseline` row still names `b8c3aee` (94 commits behind `e5f01d4`) | No |
| PA-008 | Minor | Untracked non-change artifacts and an unrelated `src/` stash in the working tree | Only if staged |
| PA-009 | Observation | `N22-TS1-23` specifies APID `0x011`; the implemented case uses `0x020` | No |
| PA-010 | Observation | Pre-existing `stream_has` reads `pos+7`/`pos+8` without a bounds guard | No |

### PA-001 — Major — `covered` classification contradicts the same spec's gap list

**Criterion:** N22 (test cases are compared against explicit expected results
that are *grounded in a requirement or interface clause*); TS-1 §1.2 status
convention (`gap` = no controlled oracle exists because the requirement is
silent, contradictory, or not delivered); N4 (traceability semantics must agree);
NC-EXT-04 (SRS-REQ-005 ack/verification conflict, authority-owned, OPEN).

**Observed fact.** `docs/vv/test-spec/ts-1-protocol-core.md:146`
(`N22-TS1-104`) is status **`host covered`** and its expected-result cell
asserts, among others:

> "(b) `FF_ERROR` and no TM at all; … (e) `FF_NOT_FOUND` and no TM."

The identical scenario is recorded as a **gap** 24 lines above, at
`ts-1-protocol-core.md:125` (`N22-TS1-22`, inputs "TC (service 0x00, sub 0x00)
and TC (service 17, sub 0x7F), ack = 0"):

> "**gap** — SRS-REQ-005 states a telecommand with an unknown service/subservice
> 'shall produce a PUS service 1 completion failure'; the implementation reports
> a failure only when the corresponding ack flag is set, so the requirement and
> PUS-C ack semantics conflict. Open NC-EXT-04 (NC-EXT-47 §7.3). Uncontrolled
> observation only: no S1 packet. **No oracle is asserted here.**"

and again in the §3 gap list at `ts-1-protocol-core.md:226-228`. SRS-REQ-005
(`docs/requirements/SRS.md:61-64`) is unconditional: "…shall not be executed and
shall produce a PUS service 1 completion failure with the appropriate error
code", with no ack qualifier.

**Gap.** The new case asserts, as an oracle, exactly the behaviour the project
has already decided has no controlled oracle and which is the subject of an open
authority-owned conflict (NC-EXT-04). Sub-case (b) (illegal APID, no ack) rests
on the same contested ground: whether an APID-violating TC is a "malformed
frame" under SRS-REQ-005 is not defined anywhere in the baseline.

**Consequence.** (i) The `covered` metric published at
`ts-1-protocol-core.md:289` becomes 78 on the strength of a case whose (b)/(e)
assertions are uncontrolled observations; the honest figure for the grounded
assertions is 77 `covered` + 1 case with a gap component. (ii) A reader — or a
future NC-EXT-15 / NC-EXT-04 disposition — would conclude that the ack-gated
failure reporting is now requirement-grounded, which the project has explicitly
declined to decide. (iii) It sets a precedent that a test case may settle a
requirement conflict without authority, contrary to the document's own authority
statement (`ts-1-protocol-core.md:19-23`).

**Disposition.** Required before commit. Minimal correction (keeps the counts at
104/78/26 and is a few words): in the `N22-TS1-104` expected-result cell, move
(b) and (e) out of the oracle and label them as uncontrolled observations of the
NC-EXT-04 conflict ("no oracle asserted; NC-EXT-04 / NC-EXT-47 §7.3"), keeping
only (a), (c), (d) as the grounded oracle; and cite NC-EXT-04 in the cell.
Cleaner alternative: split the case into a `covered` row for (a)/(c)/(d) and a
new `gap` row for (b)/(e), then recount (105 cases = 78 `covered` + 27 `gap`;
52 OOB; 94 individual; SIL 102) and add the new gap row to §3.

**Not PA's decision:** whether SRS-REQ-005 or PUS-C ack semantics governs. That
remains NC-EXT-04's authority.

### PA-002 — Major — the "previously unasserted" rationale is factually wrong

**Criterion:** N16 (detailed, accurate revision history); N8 (documentation
current and correct); N11 (evidence record must describe what was actually
done); N4 (traceability semantics, not just presence).

**Observed fact.** Two controlled records claim the ingestion branch was not
previously exercised:

- `ts-1-protocol-core.md:336` (history 1.11): "The stage-to-subservice mapping
  was previously exercised only through direct `FF_PusSendVerification` calls,
  so the ingestion-chain branches were unasserted."
- `docs/vv/verification-results.md:629-634`: "the failure branches that
  `FF_PusIngest` itself owns were unasserted: illegal APID …, unknown service …
  versus unknown subservice … on TM (1,8), and the ack-gated emission of each."

The handler-not-found branch **is** already exercised through `FF_PusIngest`, at
`tests/unit/test_r3_gap.c:280-296` (`test_pus_verification_failures`):

- `:281-287` — TC (0,0), ack = ACCEPTANCE|COMPLETION → `FF_PusIngest` returns
  `FF_NOT_FOUND`, TM (1,1) and TM (1,8) present, `d[8] == FF_PUS_FAIL_UNKNOWN_SERVICE`;
- `:291-296` — TC (17,0x7F), same acks → `FF_NOT_FOUND`, TM (1,8),
  `d[8] == FF_PUS_FAIL_UNKNOWN_SUBSERVICE`.

That is the same branch, the same codes, and the same ack gating as the new
sub-cases (c)/(d), and `tests/unit/test_r3_gap.c` is the SRS-REQ-005
implementation cited in `docs/requirements/traceability.md:78`. The existing
spec row `N22-TS1-20` (`ts-1-protocol-core.md:123`) describes exactly this and is
already `covered`.

**Gap.** The change record overstates the size of the gap it closes. What is
genuinely new is narrower: the **illegal-APID** branch (TM (1,2), code 1, no
dispatch) — which no test executed before, even though `N22-TS1-23` already
described it as `covered` under the §1.2 convention — plus the ack = 0 variants.

**Consequence.** (i) A false statement enters the revision history of a
controlled V&V document (N16) and the cycle record (N11). (ii) It could be used
to argue progress on NC-EXT-15 beyond what the evidence supports. (iii) It
conceals that `N22-TS1-104` is a *second, independent* oracle for an already
covered branch — which is a legitimate and even desirable thing to record, but
must be stated as such.

**Disposition.** Required before commit. Replace the rationale with the accurate
one: "the illegal-APID branch (TM (1,2)) and the ack = 0 variants were not
executed; the handler-not-found branch was already executed through
`FF_PusIngest` by `tests/unit/test_r3_gap.c::test_pus_verification_failures`
(`N22-TS1-20`), and this case adds an independent second oracle for it." Cite
`test_r3_gap.c` in the `N22-TS1-104` row and/or in `N22-TS1-20`.

### PA-003 — Minor — expected-result cell over-claims what is asserted

**Criterion:** N22 (the test compares against the explicit expected result); N8.

**Observed fact.** `ts-1-protocol-core.md:146` states "(a) … **exactly one**
TM (1,2) whose 9-octet payload is **Request ID [6]** + service 17 + subservice 1
+ code 1". The test asserts presence (`test_pus.c:729`), absence of (1,1) and
(17,2) (`:730-731`), but **not** a count for (1,2) and **not** the Request ID
octets at offsets 19..24 (the helper only compares offsets 25/26/27). Likewise
(d) states "TM (1,8) with code 8" without a count, while (c) does assert
`stream_count(1u, 8u) == 1u` (`:758`).

**Consequence.** A duplicate (1,2) emission, or a corrupted Request ID echo,
would not be detected, although the record says it is covered. Low risk (the
emitter is single-shot by construction) but the record and the test disagree.

**Disposition.** Non-blocking. Add `stream_count(1u, 2u) == 1u` in sub-case (a)
and `stream_count(1u, 8u) == 1u` in (d), or reword the cell to "a TM (1,2)
whose service/subservice/code are …". Note this while fixing PA-001/PA-002, since
the same cell is being edited.

### PA-004 — Minor — stale `test_link_tsn.c` hash in the re-issued T3 row

**Criterion:** N10 (tool/test-script qualification is bound to exact revisions);
the row's own statement that it is "narrowed, hash-bound".

**Observed fact.** `tool-qualification.md:26` states `tests/unit/test_link_tsn.c`
is sha256 `0b2cac2ac9dbc36291adfa71faf710a8df821455fa73f0784c9e24baebe4e382`.
The working-tree file is `c4ffe0858cc2e08d5812cfb8445a5b081400a05e03131aa700a9d96c0677132f`
(last touched by `ea0ac69`, the NC-EXT-191 commit). The other hashes in the row
that I checked are current: `test_frozen_utc_seam.c` `8660eb77…`,
`test_evs_hs_health.c` `5310cc3c…`, `test_link_ip.c` `8153e6e1…`.

**Consequence.** The row already does not describe the candidate for that one
script, so "hash-bound" is only partially true. Not introduced by this change
(recorded in rev 1.7), but the row is re-issued at rev 1.8 and the defect is
carried forward.

**Disposition.** Non-blocking. Re-bind or annotate the `test_link_tsn.c` hash in
the next T3 revision (or in this one, if the row is being touched anyway). The
T3 status must not change: no N10 review exists at the current bytes.

### PA-005 — Minor — mutation evidence is not artifact-bound

**Criterion:** N11 (evidence retained and traceable); PA evidence-integrity
practice (evidence must identify the artifact and revision it was produced from).

**Observed fact.** `docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/mutation-check.log`
records only the mutation descriptions and the resulting failures. It does not
record: the mutated file path, the exact patch, the sha256 of `src/pus/pus.c`
before and after the mutation, or a hash-verified restoration. The
`=== restored: ===` line is a test result, not a proof of restoration.

**Consequence.** A reader cannot independently reconstruct the mutation, and
cannot prove the gate logs (produced 19:40) were not taken from a mutated tree
(mutation run 19:41). I verified both points by re-execution, so the underlying
control is sound — this is an evidence-quality defect, not a validity defect.

**Disposition.** Non-blocking. For future cycles record, per mutation: file,
unified diff, pre/post sha256, build id, and a post-restoration `git status`
line. My independent reproduction (§3.5) may be cited in the cycle record if
useful, with this report as the source.

### PA-006 — Minor — the format gate log does not evidence its own exit status

**Criterion:** N11 evidence sufficiency.

**Observed fact.** `gate-format.log` is 0 bytes. The gate table in
`verification-results.md:654` claims "exit 0". An empty file is consistent with
a silent success but does not record the command or the exit status.

**Consequence.** The format gate is the weakest item in an otherwise
well-evidenced package. Low impact: `test_pus.c` compiles warning-free in my
rebuild, and `clang-format` failures are visible in CI.

**Disposition.** Non-blocking. Recommend `set -x`/`echo $?` capture for
zero-output gates.

### PA-007 — Minor — TS-1 baseline row is 94 commits stale

**Criterion:** N8 (current documentation); N16.

**Observed fact.** `ts-1-protocol-core.md:14` states: "Baseline — Current
candidate source HEAD `b8c3aee` plus uncommitted PUS/S8-SCH/OBCP source and test
deltas". `b8c3aee` is an ancestor of `HEAD` with **94 commits** between them
(`git log --oneline b8c3aee..HEAD | wc -l`). The row was last set at rev 1.6
(2026-10-16) and was not refreshed at rev 1.11, although rev 1.11 registers a
case asserted against the current candidate.

**Consequence.** A controlled V&V document misidentifies the candidate it
describes. The *evidence package* binds the real candidate correctly, so the
impact is documentary; but the row says "current".

**Disposition.** Non-blocking. Refresh the row to `e5f01d4` + the declared
uncommitted delta, or mark it as not refreshed and point to the evidence
package for candidate identity.

### PA-008 — Minor — commit hygiene: untracked non-change artifacts and an unrelated stash

**Observed fact.** `git status --porcelain` shows untracked entries that are not
part of this change: `tests/unit/test_sched_events.c.bak` (**not** gitignored,
so `git add -A` would commit it), `opencode.json`, `.opencode/plugins/`, plus a
number of prior-cycle review reports. `git stash list` also shows
`stash@{0}: WIP: GAP-A4 src changes for next cycle`.

**Consequence.** A commit that stages the working tree wholesale would add a
`.bak` file and agent tooling configuration, and would break the "no `src/`
change" statement that this whole cycle rests on if the stash were popped.

**Disposition.** Non-blocking, **but blocks the commit if staged**. Stage
explicitly: the four modified files, the evidence directory, and the review
reports belonging to this cycle. Do not pop `stash@{0}` into this candidate.

### PA-009 — Observation — `N22-TS1-23` input is not the implemented input

`N22-TS1-23` (`ts-1-protocol-core.md:126`) specifies APID `0x011`; the
implemented case uses `0x020` (`test_pus.c:725`). Same branch, different value;
the `covered` status of `N22-TS1-23` is now supported by a case at a different
input. Worth a one-line note in the 1.11 history. No action required for
correctness: `0x020` is the stronger, more realistic case (a TC on the TM APID).

### PA-010 — Observation — pre-existing unguarded reads in `stream_has`

`stream_has` (`test_pus.c:154-166`) loops on `pos < g_rx_count` but then reads
`pos+7`/`pos+8`, so a capture shorter than 9 octets would read past the
populated region (within the 1 KiB array, so not a crash, but not a valid
assertion either). Pre-existing, not touched by this change; the new helper does
guard correctly. Consider hardening in a later cycle (N21: not a blocker).

## 5. Positive confirmations

Recorded because an assurance review that only lists defects is not a review:

1. **No production change.** `src/` is byte-identical to `HEAD`; the risk of
   this increment to flight behaviour is nil.
2. **The test is real and discriminating.** Four independent mutations — two of
   them mine — each fail exactly the intended assertion and nothing else. The
   ack-gating assertions are discriminating, not decorative.
3. **The test is well-engineered.** Per-sub-case capture reset, bounds-guarded
   packet walk, zero-progress guard, and negative assertions (`!stream_has(1,1)`,
   `!stream_has(17,2)`, `!stream_has_verification(…UNKNOWN_SUBSERVICE)`) that
   separate the service-0 code from the unknown-subservice code.
4. **The hash rebind is correct and the qualification status is honestly not
   upgraded.** T3 remains NOT FULLY QUALIFIED, "execution is not qualification"
   is stated, and no N10 review is claimed at these bytes.
5. **No NC closure or release claim.** NC-EXT-15, NC-EXT-14..18, NC-EXT-47,
   GAP-A5 and NC-PAL-01 are all left open and explicitly disclaimed; this is
   consistent with `docs/status/loop-audit.md:198`.
6. **Gate evidence matches my independent re-execution** for build, unit test,
   CTest, Lean and trace.
7. **Counts are internally consistent** and the recount is reproducible from the
   table alone.

## 6. Disposition summary and recommended path to commit

**Do not commit/push this change set as it stands.** Two Major findings must be
corrected in the same commit:

1. PA-001 — remove (b)/(e) from the `N22-TS1-104` oracle and label them as
   uncontrolled observations under NC-EXT-04 (or split the case and recount).
2. PA-002 — replace the "previously unasserted" rationale with the accurate
   scope, citing `tests/unit/test_r3_gap.c:280-296` / `N22-TS1-20`.

Recommended to fold in while the same cells are edited (non-blocking): PA-003,
PA-004, PA-007. Recommended for the commit step: PA-008 (stage explicitly, do
not pop the stash).

Outstanding gates that are **not** satisfied by this cycle and that the change
correctly does not claim: N17 (R4 documentation review), N9/R2 independent V&V
delta review, N20 (this report — which is one PA input, not a closure). The
documents state "independent delta review is pending", which is accurate.

## 7. Uncertainty and confidence

```text
Decision/product and configuration:
  Commit/push of the S1 ingestion-failure test increment; candidate e5f01d4 +
  working-tree delta (test_pus.c db8579c7…109f2; ts-1 rev 1.11;
  tool-qualification rev 1.8; verification-results cycle section; evidence dir
  2026-11-12-s8-pus-fu-s1-verification-failures).
Claim type: Fact (re-executed results, hashes, counts), Inference (severity and
  blocking effect), Recommendation (corrections), Open issue (NC-EXT-04
  disposition).
Applicable baseline and approved tailoring:
  Project norms N1-N22 (AGENTS.md, docs/rules.md); ECSS-Q-ST-80C Rev.2 /
  E-ST-40C Rev.1 invoked by the project; ECSS class B target; PUS-C
  ECSS-E-ST-70-41C with the ADR-0020 provisional profile disclaimers.
Evidence consulted (issuer, ID/revision, artifact/version, date):
  src/pus/pus.c 1eea40c0…5401 (HEAD); src/pus/pus_types.h:39-44;
  config/fsw_config.h:92-93; tests/unit/test_pus.c db8579c7…109f2;
  tests/unit/test_r3_gap.c (HEAD); docs/vv/test-spec/ts-1-protocol-core.md
  rev 1.11; docs/vv/tool-qualification.md rev 1.8; docs/vv/verification-results.md
  §S8-PUS-FU 2026-11-12; docs/requirements/SRS.md:61-64;
  docs/requirements/traceability.md:78; docs/status/loop-audit.md:198;
  evidence dir logs (9 files, 2026-11-12 19:39-19:41); my own re-execution
  2026-11-12 (test_pus 315/0, CTest 57/57, lake build, make trace, 4 mutations).
Cross-check and reconciliation:
  Retained logs vs my re-execution: agree (57/57, 315/0, trace 120/76/30/14/68,
  0 compiler warnings). Recorded mutation results vs my reproduction: agree
  exactly (1 and 2 failures at the same lines). Spec counts vs my table recount:
  agree. Spec novelty rationale vs code base: DO NOT agree (PA-002). Spec
  covered/gap classification vs its own gap list: DO NOT agree (PA-001).
Confidence: High for PA-001, PA-002, PA-003, PA-004, PA-008 (direct, primary,
  re-executed or mechanically recounted evidence); Medium for PA-005, PA-006
  (evidence-sufficiency judgements); High that the test and gates are sound.
Basis and detection limits:
  Static reading of one changed test file and one implementation function;
  four mutations in one function; recount of one table. No dynamic coverage
  instrumentation was run, so "the branch is taken" is established by mutation
  sensitivity, not by coverage measurement. RTEMS, Renode/N18, smoke, coverage
  and Docker CI were not re-executed; they are not affected by a test-only
  change but no claim is made about them.
Missing/inaccessible evidence:
  No N10/R2 review at the candidate test bytes (T3 remains NOT FULLY
  QUALIFIED); no R4 review of ts-1 rev 1.11; no `test_link_tsn.c` review at the
  current bytes. ECSS-Q-ST-80C Rev.2 and E-ST-40C Rev.1 clause-level text was
  not re-read for this delta (the applicable obligations are project-internal
  norms N4/N8/N10/N11/N16/N22 and the SRS).
Assumptions and expiry/recheck date:
  Assumes the four modified files plus the evidence directory are the whole
  intended commit scope (per the task statement). Recheck on any further edit to
  ts-1, tool-qualification, verification-results or test_pus.c, and at the next
  T3 re-issue.
Consequences if wrong:
  If PA-001 is waived without an authority decision, the V&V record would claim
  requirement-grounded coverage of behaviour that NC-EXT-04 says is unresolved,
  and the `covered` metric would be overstated by one case. If PA-002 is
  published, the project's own history would misstate what was previously
  verified, weakening NC-EXT-15 disposition. Neither affects the correctness of
  the delivered software, which is unchanged.
Required independent human reviewer/authority:
  Designated FSW engineer (commit decision); Software PA for this report's
  disposition; the NC-EXT-04 owner for the SRS-REQ-005 ack-semantics question;
  R4 for N17 and R2 for the N9 delta review.
```

## 8. Review history

| Rev | Date | Author | Reason |
| --- | --- | --- | --- |
| 1.0 | 2026-11-12 | aeronautics-space-sw-pa-engineer | Initial independent N20 review of the S1 ingestion-failure test increment: branch traceability against `FF_PusIngest`, `src/` immutability, TS-1 count recount, T3 hash verification, independent mutation reproduction. 2 Major (blocking), 6 Minor, 2 observations. |
