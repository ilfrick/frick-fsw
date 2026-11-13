# PA re-review — PUS Service 1 ingestion-owned verification failures (S8-PUS-FU)

| Field | Value |
| --- | --- |
| Document id | FRICK-REVIEW-PA-R2-PUS-S1-INGEST-20261112 |
| Norm | N20 (independent Software Product Assurance review), with N4, N8, N10, N11, N12, N16, N17, N21, N22 |
| Reviewer profile | Senior Aeronautics and Space Software Product Assurance Engineer |
| Date | 2026-11-12 |
| Repository / branch | `/home/nicola/fsw/frick-fsw`, `main` |
| Candidate baseline | `HEAD` = `e5f01d4` + uncommitted working-tree delta (4 modified files + 1 untracked evidence directory) |
| Decision under review | Commit/push of the S1 ingestion-failure test increment |
| Review type | Independent PA re-review of the author's corrections to the prior PA review (`FRICK-REVIEW-PA-PUS-S1-INGEST-20261112`) |
| Prior review | `docs/reviews/PA-REVIEW-PUS-S1-INGEST-VERIFICATION-FAILURES-20261112.md` (2 Major, 6 Minor, 2 Observations) |
| Result | **All 10 prior findings are closed in substance.** 6 residual Minor findings and 1 pre-existing Major finding are recorded; **none of them blocks commit/push of this delta.** |

> **Authority statement.** This review is an independent assurance assessment. It
> does not approve release or flight, close or accept an NC, accept residual
> risk, qualify a tool, or authorize a commit. Disposition of every finding and
> the commit decision remain with the designated FSW engineer and the authorized
> project authorities. This report is written under N20; it does not discharge
> the N17 (R4 documentation) or N9/R2 (independent delta) review obligations,
> and it cannot waive them.

## 1. What was re-reviewed

Working-tree delta against `e5f01d4`, unchanged in scope from the prior review
(no new files, no `src/` delta):

| File | Change | sha256 (working tree) |
| --- | --- | --- |
| `tests/unit/test_pus.c` | `stream_has_verification` helper, `test_verification_failure_via_ingest` (5 sub-cases), bounds guard on the pre-existing `stream_has`, `FF_TEST_MAIN` registration | `1e9c86e8179858b30da2c40911e289916e076a24b3698bfaeb9e5f6aae3cc264` |
| `docs/vv/test-spec/ts-1-protocol-core.md` | rev 1.10 → 1.11; `N22-TS1-104` added as `gap`; `N22-TS1-23` re-stated; §2 range, §3 gap list, §3.1 totals, baseline row, history | tracked (`git diff`) |
| `docs/vv/tool-qualification.md` | rev 1.7 → 1.8; T3 `test_pus.c` hash re-bound, `test_link_tsn.c` hash refreshed | tracked (`git diff`) |
| `docs/vv/verification-results.md` | new cycle section: scope, gap addressed, gates, mutation control, residual items, staging scope | tracked (`git diff`) |
| `docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/` | retained logs (8 present) | untracked, new |

Production source is untouched: `src/pus/pus.c` =
`1eea40c06ecf399326347585995f71eec718318e535332f44edb043db3e15401`,
`git status --porcelain -- src/` empty at the end of this re-review.
`git diff --cached` is empty (nothing is staged).

Declared out of scope and **not** closed by this change: NC-EXT-15, NC-EXT-04,
NC-EXT-14..18, NC-EXT-47, NC-EXT-19/90, N22-TS5-47, GAP-A5, NC-PAL-01,
NC-EXT-211. T3 remains **NOT FULLY QUALIFIED** for this candidate.

## 2. Method — what I re-executed myself

I did not accept the retained logs or the author's closure statements as
evidence. Every closure decision below rests on my own execution or my own
inspection of the controlled artifact.

| Check | Command / method | My result |
| --- | --- | --- |
| `test_pus` binary | `build/evidence-20261112-pus-s1/tests/test_pus` | `checks=320 failures=0`, exit 0 |
| Full CTest | `ctest` in `build/evidence-20261112-pus-s1` | `100% tests passed, 0 tests failed out of 57` |
| Lean 4 (N1) | `lake build` in `verification/lean` | `Build completed successfully.`, exit 0 |
| Traceability (N4) | `make trace` | `PASS (120 requirements, 76 srs, 30 sss, 14 gaps, 68 lean theorems checked)` |
| Compiler warnings on the delta | forced recompile of `test_pus.c` | 0 warnings |
| Format gate (N10) | `clang-format --dry-run --Werror tests/unit/test_pus.c` | exit 0 (independently reproduced) |
| Source immutability | `git status --porcelain -- src/`, `sha256sum src/pus/pus.c` | empty, `1eea40c0…5401` |
| TS-1 table recount | own pipe-aware parser (`/tmp/opencode/pa_r2_recount.py`) | see §4 |
| TS-1 gap-list cross-check | own parser (`/tmp/opencode/pa_r2_gapcheck.py`) | see §4 |
| Mutation controls | both recorded mutations re-applied to `src/pus/pus.c`, rebuilt, re-tested, restored (see §5) | both reproduce; source restored and re-verified |
| Illegal-APID novelty claim | `git grep FF_PUS_FAIL_ILLEGAL_APID`, inspection of `test_pus_s11_residual.c`, `test_decoder_corpus.c` | see PA-R2-001 |

Not re-executed by me: the RTEMS cross build, Renode/N18, native smoke,
coverage, Docker CI image, doxygen. The delta contains no `src/` change, so
those gates cannot have been affected by it; their prior evidence is unchanged
and I make no claim about it here. This is a host-configuration review only.

## 3. Verification matrix for the prior findings

| Prior ID | Severity | Author's claim | My verdict | Decisive evidence I inspected |
| --- | --- | --- | --- | --- |
| PA-001 | Major | `N22-TS1-104` reclassified from `covered` to `gap`; no `covered` count inflation | **Closed** | ts-1 line 146 is a single `gap` row with observations only and cross-references `N22-TS1-22` / NC-EXT-04; §3 gap list (lines 232-234) contains it; my recount still gives 77 `covered` (unchanged from rev 1.10) |
| PA-002 | Major | Novelty rationale corrected; `test_r3_gap.c` credited for the (1,8) branch | **Closed in substance, wording imprecise** | `tests/unit/test_r3_gap.c:280-296` does drive `FF_PusIngest` for unknown service/subservice with ack = ACCEPTANCE\|COMPLETION and asserts `d[8]` codes 8/9; ts-1 line 339 credits it. Residual imprecision → PA-R2-001 |
| PA-003 | Minor | "exactly one (1,2)" now asserted, Request-ID octets asserted | **Closed** | `test_pus.c:732` `stream_count(1u, 2u) == 1u`; `g_rx[19..22]` asserted `01 02 00 01`, matching the 9-octet verification payload layout in `src/pus/pus.c:298-353` |
| PA-004 | Minor | `test_link_tsn.c` hash refreshed in the T3 row | **Closed** | `sha256sum tests/unit/test_link_tsn.c` = `c4ffe0858cc2e08d5812cfb8445a5b081400a05e03131aa700a9d96c0677132f`; matches the re-issued row; the run is 149 checks / 0 failures |
| PA-005 | Minor | `mutation-check.log` now artifact-bound | **Closed** | Log names file, exact substitution, line, pre/post sha256, and restoration; I reproduced mutation 1 byte-exactly (mutant `1df235d2…`, 1 failure at `test_pus.c:731`) and mutation 2 (2 failures at `test_pus.c:779-780`) — details in §5 |
| PA-006 | Minor | `gate-format.log` now carries its exit status | **Closed** | Log content is `clang-format --dry-run --Werror exit=0`; I re-ran the command myself, exit 0 |
| PA-007 | Minor | TS-1 baseline row moved to current HEAD | **Closed** | ts-1 line 14 records `e5f01d4` plus the uncommitted delta and the host configuration |
| PA-008 | Minor | Staging hygiene | **Closed for intent, claim inaccurate** | Staging-scope paragraph restricts the change set correctly, but `git diff --cached` is empty while the text says the files "are staged" → PA-R2-003 |
| PA-009 | Observation | Case input aligned to APID `0x011` | **Closed** | `test_pus.c:722-731` encodes APID `0x011`; the ack = 0 variant deliberately uses `0x020` and says so |
| PA-010 | Observation | Bounds guard added to `stream_has` | **Closed** | `test_pus.c:158` guard `(pos + 9u) <= g_rx_count` before the `pos+7`/`pos+8` reads |

No prior finding is left unaddressed, and no correction regressed an
independently verified property: the production source is still byte-identical
to `HEAD`, the Lean and traceability gates still pass, and the case count
moved by exactly one row (103 → 104).

## 4. Independent recount of the TS-1 case table

Own parser, run on the working-tree `docs/vv/test-spec/ts-1-protocol-core.md`
(the author's numbers are not reused):

| Metric | My count | Author's §3.1 | Match |
| --- | --- | --- | --- |
| Total rows | 104 | 104 | yes |
| Unique IDs / duplicates / missing | 104 / none / none (1…104) | — | yes |
| `covered` | 77 | 77 | yes |
| `gap` | 27 | 27 | yes |
| Nominal | 48 | 48 | yes |
| Out-of-bound | 51 | 51 | yes |
| Mixed nominal + OOB | 5 | 5 | yes |
| Individual | 93 | 93 | yes |
| Combined | 11 | 11 | yes |
| SIL / PIL / HIL | 101 / 1 / 2 | 101 / 1 / 2 | yes |

Gap-list cross-check: 27 `gap`-status rows, 27 §3 gap bullets, set difference
empty in both directions, no row both `covered` and `gap`. Row width is a
consistent 9 columns for all 104 rows, so no row is silently mis-parsed. The
`covered` total is identical to rev 1.10 (77), which is the correct outcome: the
delta added one `gap` case and re-stated one already-`covered` case.

## 5. Mutation control — independent reproduction

I re-applied both recorded mutations to `src/pus/pus.c` myself, rebuilt
`test_pus`, ran it, and restored the file.

| Mutation | Recorded mutant sha256 | My mutant sha256 | Detected at | Restored |
| --- | --- | --- | --- | --- |
| 1: `FF_PUS_FAIL_ILLEGAL_APID` → `FF_PUS_FAIL_BAD_DATA` (line 424) | `1df235d2…` | `1df235d2e200f3b8d467c6a18d12af8e49e342701946ce5f987434ec2e8489f` | `test_pus.c:731`, 1 failure | `1eea40c0…5401`, `git status -- src/` empty |
| 2: remove `(tc.service == 0u)` failure-code selection (line 439) | `cd2b01fa…` | `cd2b01fa0f…` **only with the literal ternary-line replacement** | `test_pus.c:779-780`, 2 failures | `1eea40c0…5401`, `git status -- src/` empty |

Note on mutation 2: a collapsed one-line variant of the same logical change
produces a different mutant hash (`81455adc…`). The recorded hash is therefore
reproducible only for the exact substitution documented in the log. The log
does state the exact substitution, so the record is adequate; the hash alone
would not be. This is a reproducibility observation, not a finding — no
assertion depends on the mutant hash.

Both mutations are detected at the assertions that the new case introduces,
which is the property that matters: the oracle is discriminating, not merely
satisfied.

## 6. Findings

| ID | Severity | Summary | Blocks commit/push of this delta? |
| --- | --- | --- | --- |
| PA-R2-001 | Minor | Novelty wording overstates the gap: an implementing test for the illegal-APID branch did exist | No |
| PA-R2-002 | Minor | `gate-configure.log` is cited but absent; the evidence set shrank from 9 to 8 logs | No |
| PA-R2-003 | Minor | Staging-scope paragraph says the files "are staged"; nothing is staged | No |
| PA-R2-004 | Minor | `N22-TS1-23` input cell states ack = ACCEPTANCE\|COMPLETION; the bound evidence uses ACCEPTANCE only | No |
| PA-R2-005 | Minor | `payload[4]` in `N22-TS1-20`/`N22-TS1-23` contradicts the 9-octet payload (code at `payload[8]`) | No |
| PA-R2-006 | **Major** | `test_pus_s11_residual` is built but not registered with CTest and fails 10/1164 checks | No — pre-existing, outside this delta (N21) |

### PA-R2-001 — Minor — the novelty claim is imprecise about what was previously unasserted

**Criterion.** N8 (complete, current, accurate documentation), N11 (evidence
statements must be supported by the retained record), N16 (accurate revision
history).

**Observed fact.** Two statements assert that the illegal-APID failure branch
had no implementing test:

- `docs/vv/test-spec/ts-1-protocol-core.md:339` — "`N22-TS1-23` … was marked
  `covered` although no test implemented it".
- `docs/vv/verification-results.md:630-632` — "was recorded as `covered` in TS-1
  without any implementing test; no test in the tree referenced
  `FF_PUS_FAIL_ILLEGAL_APID`".

Both statements are accurate about the *symbol* (`FF_PUS_FAIL_ILLEGAL_APID` had
no reference in the tree at `HEAD`) but inaccurate about the *behaviour*: two
pre-existing tests already exercised non-TC-APID rejection through the ingest
path and asserted its observable consequence —

- `tests/unit/test_pus_s11_residual.c:139-152`: non-TC APID `0x1234` with
  ack = ACCEPTANCE returns `FF_ERROR` and emits exactly one (1,2) packet
  (`stream_count(1, 2) == 1`).
- `tests/unit/test_decoder_corpus.c:622-628`: non-`0x010` APIDs are not
  dispatched.

What is genuinely new in this delta is therefore narrower and stronger: the
failure-code octet, the echoed service/subservice octets, the Request-ID octets,
and the explicit `!(17,2)` non-dispatch assertion. Those are the discriminating
additions, and they are real.

**Gap.** The record attributes to the delta a novelty that belongs partly to
pre-existing tests. A reader could conclude that the illegal-APID branch was
completely unverified before, which is not the case.

**Consequence / risk.** Low. The evidence is stronger than the text claims, not
weaker; no coverage or oracle is overstated. The risk is a future reviewer
re-doing the same analysis and losing confidence in the record's precision.

**Classification.** Minor, non-functional (wording accuracy of a verification
record). Under N21 it is an NC for later disposition and does not stop
unrelated development. A precise correction would name the pre-existing tests
and restrict the novelty claim to the payload-level assertions.

**Contributing cause (PA self-correction).** My prior review's PA-002
disposition recommended wording built around "the illegal-APID branch was not
executed". That prescription was itself inaccurate: the branch *was* executed
by `test_pus_s11_residual`; what was missing was an assertion on the failure
code. The author implemented my recommendation faithfully. I record this
explicitly so the finding is not read as an independent author error.

**Owner / due.** Designated FSW engineer; next documentation touch of ts-1 or
`verification-results.md`. No re-review of this delta is required for it.

### PA-R2-002 — Minor — a cited evidence artifact is absent from the evidence set

**Criterion.** N10 (V&V tools and evidence are reviewable), N11 (evidence
retained and referenced must exist), N12 (findings resolved by re-applying the
rules).

**Observed fact.** `docs/vv/verification-results.md:653` cites
`docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/gate-configure.log`
for "Configure (`FF_USE_PUS_ASN1=ON`) | exit 0". The directory contains 8 files
(`gate-build`, `gate-ctest`, `gate-format`, `gate-lean`, `gate-license`,
`gate-test-pus`, `gate-trace`, `mutation-check`); `gate-configure.log` is not
among them. My prior review's §1 table recorded 9 retained logs, so the file was
either never captured or was removed between the two reviews; neither is
acceptable practice for a cited artifact.

**Corroborating, not substituting, evidence.** The configure step demonstrably
ran: `build/evidence-20261112-pus-s1/CMakeCache.txt` is timestamped
2026-11-12T19:39:51, before the earliest retained log (19:51:48), and the build
directory contains the configured `FF_USE_PUS_ASN1=ON` artifacts. So this is an
evidence-retention defect, not a fabrication of a gate result.

**Gap.** A cited gate artifact does not exist in the cited location; the
retained evidence set is not self-consistent with the results record.

**Consequence / risk.** Low-to-moderate for reviewability: a future reader
cannot re-check the configure configuration from the evidence set alone and
must fall back to build artifacts that may be cleaned. The gate result itself is
not in doubt.

**Classification.** Minor, non-functional. **Owner / due.** Designated FSW
engineer; either restore the log or amend the citation at the next touch of the
results record.

### PA-R2-003 — Minor — the staging-scope statement is not true of the repository state

**Criterion.** N8, N11 (records must describe the actual state), N16.

**Observed fact.** `docs/vv/verification-results.md:687-693` states "Only
`tests/unit/test_pus.c`, the three updated documents, and this evidence
directory are staged." `git diff --cached --stat` is empty: nothing is staged.
The paragraph does correctly identify the intended change set and correctly
lists the pre-existing untracked paths that must stay out.

**Gap.** The record asserts a repository state that does not hold. The intent
(commit exactly these paths, leave the rest untouched) is sound; the tense is
wrong.

**Consequence / risk.** Low. The risk is that a reader trusts "staged" as proof
that the commit set was verified, when in fact the staging decision is still
ahead of the committer.

**Classification.** Minor, non-functional. **Owner / due.** Designated FSW
engineer, at commit time — a one-word correction is sufficient, and the
commit itself is where the statement becomes verifiable.

### PA-R2-004 — Minor — `N22-TS1-23` input cell and bound evidence disagree on ack flags

**Criterion.** N4 (bidirectional traceability: the case, the test, and the
result must describe the same configuration), N22 (expected results against
explicit inputs).

**Observed fact.** `docs/vv/test-spec/ts-1-protocol-core.md:126` specifies the
input as "ack = ACCEPTANCE|COMPLETION". The bound evidence
(`test_pus.c:722-731`) sets `tc.ack_flags = FF_PUS_ACK_ACCEPTANCE` only. The
same row discloses this in-line: "Evidence: … with ack = ACCEPTANCE only".

**Gap.** A traceability mismatch between the case input cell and the executed
input. The behaviour is equivalent for the asserted oracle — an illegal APID is
rejected at the acceptance stage, so the completion flag cannot add a second
failure packet, and the test asserts `stream_count(1u, 2u) == 1u` — but the
trace is not literal.

**Consequence / risk.** Low. The disclosure in-row prevents a false claim; the
residual risk is a future reader assuming the completion-stage path was also
exercised for the illegal-APID branch.

**Classification.** Minor, non-functional. **Owner / due.** Designated FSW
engineer; align the input cell with the executed input at the next ts-1 touch,
or extend the test to the specified ack set.

### PA-R2-005 — Minor — `payload[4]` is the wrong octet for the failure code

**Criterion.** N4, N8 (interface/record accuracy against the implemented wire
layout).

**Observed fact.** `N22-TS1-20` and `N22-TS1-23` state the oracle as "payload[4]
= `FF_PUS_FAIL_…`". `FF_PusSendVerification` (`src/pus/pus.c:298-353`) emits a
9-octet payload: Request ID source (0-1), destination (2-3), sequence count
(4-5), service (6), subservice (7), code (8). `payload[4]` is the high byte of
the sequence count; the failure code is at `payload[8]`. The bound evidence
asserts octets 6/7/8, i.e. the correct layout.

**Gap.** A specification cell misnames the octet that carries the asserted
value.

**Consequence / risk.** Low for this delta (the test asserts the right octets),
moderate if the cell is used to derive a future oracle: a test written from
`payload[4]` would be non-deterministic against the monotonic sequence count.

**Classification.** Minor, non-functional, pre-existing text now made visible by
adjacent evidence. **Owner / due.** Designated FSW engineer; correct both rows at
the next ts-1 touch. This is exactly the class of item N21 says must be logged
rather than turned into a wording-perfection loop.

### PA-R2-006 — Major — an unregistered, failing test is cited as evidence elsewhere

**Criterion.** N22 (tests and evidence must be trustworthy and applicable), N11
(evidence integrity), N12 (findings resolved to compliance), N15 (full cycle).

**Observed fact.**

1. `tests/CMakeLists.txt:99` builds `test_pus_s11_residual` and line 110 lists
   it in `FF_TEST_TARGETS`, but there is no `add_test` for it. Consequently the
   "57/57 PASS" CTest result does not include it, and neither does the T3
   inventory in `docs/vv/tool-qualification.md`.
2. I executed the binary directly: `checks=1164 failures=10`, exit 1, with
   failures at `test_pus_s11_residual.c:486-487` (expects `FF_PusIngest` success
   and one (6,6) report) and `:498` (expects `FF_NOT_FOUND`). The expectations
   are stale with respect to the current 7-octet S6 profile and the S3 payload
   validation.
3. The sprint plan cites its 1164 checks as evidence.

**Gap.** A test target that is part of the declared inventory is neither
executed by the registered gate nor passing, and its check count is quoted as
evidence. This is a pre-existing condition: the delta neither created nor
touched that file, and `src/` is unchanged, so the failure predates `e5f01d4`.

**Consequence / risk.** Moderate and genuinely assurance-relevant: the
registered gate result ("57/57") does not cover every declared test target, so
"all tests pass" is true only for the registered subset. Anyone quoting 1164
checks from the sprint plan is quoting a failing suite.

**Classification.** **Major severity**, but **non-blocking for this delta**
under N21: it is a pre-existing non-functional/test-inventory defect, unrelated
to the S1 ingestion change, and N21 explicitly forbids stopping unrelated
development for it. It must be registered as an NC with an owner and a
milestone, and either the `add_test` registration plus repair, or the removal
from `FF_TEST_TARGETS`/T3/sprint evidence, must follow. It must not be silently
inherited.

**Owner / due.** Designated FSW engineer to register the NC; disposition
(repair or retire) at the next test-inventory milestone. **This finding does not
block commit/push of the current delta, and PA does not close or accept it.**

## 7. Observations (non-findings, no action required for this delta)

1. **`N22-TS1-20` row does not cite its implementing test.** The row's evidence
   column is silent; only the rev 1.11 history line names
   `tests/unit/test_r3_gap.c`. Traceability would be stronger with the citation
   in the row itself.
2. **"Exactly one (1,8)" is asserted for sub-case 3 but not for sub-case 4.**
   Sub-case 4 (unknown service 0) asserts the code-8 packet and the absence of
   the code-9 variant, not the packet count. Adequate but asymmetric with
   sub-case 3.
3. **"Three revisions stale"** in the results record counts three committed
   revisions plus this delta; the phrase is defensible but could be read either
   way.
4. **`gate-build.log` does not evidence compilation of the delta.**
   It is an up-to-date second build: line 24 reads `Built target test_pus` with
   no compile line, so the "0 compiler warnings" claim in the results table is
   not evidenced by that log. I forced a recompile of `test_pus.c` myself: 0
   warnings, so the claim is true; only its evidence trail is thin.
5. **Sub-cases 2 and 5 are characterisation locks, not oracles.** They are
   correctly recorded as observations under `N22-TS1-104` and are expected to
   change when NC-EXT-04 is dispositioned. This is the right treatment.
6. **The mutation-2 mutant hash is sensitive to substitution shape** (§5). The
   log documents the exact substitution, which is what makes the record usable.

## 8. Positive confirmations

- The two Major findings of the prior review are genuinely fixed, not
  re-labelled: the `gap` classification is now internally consistent with the
  §3 gap list and with NC-EXT-04, and the `test_r3_gap.c` evidence is credited.
- The delta adds **no** `covered` count while adding a case. Coverage accounting
  is honest.
- The new case is discriminating: two independent mutations of the production
  source, each detected at the intended assertion, with byte-level restoration
  proof.
- The production source is byte-identical to `HEAD`; the change is test-and-
  documentation only, so no functional regression surface is introduced.
- Lean (`lake build`), traceability (`make trace`), format, license and the
  registered CTest suite all pass under my own execution.
- The record refuses closure of NC-EXT-15, NC-EXT-04, NC-EXT-14..18 and
  NC-EXT-47, and refuses to claim T3 qualification. That restraint is correct
  and is the behaviour N20 is meant to protect.
- The `stream_has` bounds guard (PA-010) is a real hardening of a pre-existing
  helper and does not weaken any existing assertion.

## 9. Does anything block commit/push?

**No finding in this re-review blocks commit/push of this delta.**

Reasoning, stated as an assurance position and not as a commit authorization:

- PA-R2-001 … PA-R2-005 are Minor, non-functional documentation/evidence
  accuracy items. N21 explicitly forbids expanding a functional change into a
  wording-perfection loop; they are to be logged as NCs and dispositioned at the
  next documentation touch.
- PA-R2-006 is Major in severity but pre-existing, outside this delta, and
  unrelated to the S1 ingestion change. Under N21 it is an NC, not a stop
  condition for this change. It must be registered, with an owner and
  milestone, before it is forgotten.
- No functional bug, no `src/` delta, no safety or security impact, no
  unreproducible build, no untraceable artifact and no unapproved waiver is
  present in this change.

**Gates that remain outstanding and that PA cannot discharge or waive:**

1. **N17 / R4** documentation review of the delta (ts-1 rev 1.11,
   `tool-qualification.md` rev 1.8, the new `verification-results.md` section).
2. **N9 / R2** independent V&V delta review of the new case.
3. **Index entry**: `docs/reviews/README.md` needs a row for this report and for
   the prior PA review. PA is restricted in this task to creating this single
   file and has not edited the index; the designated FSW engineer must add both
   entries.
4. **NC registration** for PA-R2-001 … PA-R2-006 in the project NC log, with
   owners and milestones (PA-R2-006 with a milestone, not just an owner).
5. **N7** — only the designated FSW engineer commits, and only after all
   applicable gates and the full sprint scope are complete.

## 10. Uncertainty and confidence

```text
Decision/product and configuration: commit/push of the S8-PUS-FU S1 ingestion-failure
  test increment; HEAD e5f01d4 + working-tree delta; host configuration only.
Claim type: Findings are Facts from direct inspection and my own execution; the
  non-blocking conclusion is an Inference from N21 and the delta's scope.
Applicable baseline and approved tailoring: project norms N1-N22 (AGENTS.md,
  docs/rules.md); ECSS-E-ST-40C / Q-ST-80C invoked at project level; no mission
  tailoring reviewed here.
Evidence consulted (issuer, ID/revision, artifact/version, date):
  ts-1 rev 1.11; tool-qualification rev 1.8; verification-results S8-PUS-FU
  section (2026-11-12); evidence dir 2026-11-12-s8-pus-fu-s1-verification-failures
  (8 logs); tests/unit/test_pus.c 1e9c86e8…; tests/unit/test_pus_s11_residual.c;
  tests/unit/test_decoder_corpus.c; tests/unit/test_r3_gap.c; tests/CMakeLists.txt;
  src/pus/pus.c 1eea40c0…; own recount/gapcheck/mutation tooling in /tmp/opencode.
Cross-check and reconciliation: author's counts, hashes and gate results were all
  reproduced independently; the only discrepancy is the absent gate-configure.log
  (PA-R2-002), corroborated by CMakeCache timestamp 19:39:51.
Confidence: High for the closure of PA-001..PA-010 and for the findings as stated;
  Medium for the non-blocking conclusion on PA-R2-006, because it depends on the
  project's N21 interpretation and on the NC being registered rather than dropped.
Basis and detection limits: I inspected the working tree, not a frozen candidate;
  I did not run RTEMS, Renode/N18, smoke, coverage or Docker CI. The delta has no
  src/ change, so those gates are out of this change's blast radius, but I make no
  claim about their current status.
Missing/inaccessible evidence: gate-configure.log (absent); RTCA/ECSS licensed
  clause texts were not consulted for this review (not needed for a test-record
  delta); no N17/R2 review reports exist yet for this delta.
Assumptions and expiry/recheck date: the working-tree delta does not change before
  commit (recheck at commit time); recheck 2026-11-19 or on any further edit.
Consequences if wrong: if PA-R2-006 is treated as closed by deferral, the project
  will carry a failing declared test target with a quoted check count as evidence,
  which is a genuine traceability defect at the next audit.
Required independent human reviewer/authority: designated FSW engineer (commit and
  NC registration), R4 documentation reviewer (N17), R2 V&V reviewer (N9), and the
  project/Software PA authority for NC disposition. PA does not decide any of these.
```

## 11. Review history

| Rev | Date | Author | Change |
| --- | --- | --- | --- |
| 1.0 | 2026-11-12 | aeronautics-space-sw-pa-engineer | Independent PA re-review of the author's corrections to `FRICK-REVIEW-PA-PUS-S1-INGEST-20261112`. All 10 prior findings closed in substance; 6 residual findings recorded (5 Minor, 1 pre-existing Major); explicit position that no finding blocks commit/push of this delta; N17/R2/N7 gates and the `docs/reviews/README.md` index entry remain outstanding. Includes a PA self-correction on the PA-002 disposition wording. |
