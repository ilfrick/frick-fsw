# DOC-R4-20261112-PUS-S1 — Software Documentation Review (N17, R4 profile)

**Repository:** /home/nicola/fsw/frick-fsw
**Branch:** main
**HEAD:** `e5f01d4` plus the uncommitted working-tree delta
**Target:** Microchip RT PolarFire SoC RTPFS160ZT (RISC-V) under RTEMS 6.2
**Criticality target:** ECSS class B
**Reviewer profile:** Senior Aeronautics and Space Software Documentation Engineer (R4) — independent documentation review
**Review date:** 2026-11-12
**Change under review:** S8-PUS-FU PUS Service 1 ingestion-failure test increment
**Prior reviews of the same delta:** `docs/reviews/PA-REVIEW-PUS-S1-INGEST-VERIFICATION-FAILURES-20261112.md` (N20, first pass), `docs/reviews/PA-R2-20261112-pus-s1-ingest-verification-failures.md` (N20, re-review), `docs/reviews/VV-R2-20261112-s8-pus-fu-s1-verification-failures.md` (N9/R2/N22)
**Result:** **CHANGES REQUIRED** — 4 Major (DOC-R4-001, DOC-R4-002, DOC-R4-003, DOC-R4-008) and 6 Minor. The Majors block commit/push of this delta until they are corrected or a disposition is recorded by the authorized owner.

> **Authority statement.** This report is an independent documentation analysis artifact. It has no approval, qualification, acceptance, certification, or release authority, and it does not close an NC, accept residual risk, or authorize a commit. The verdict below is a recommendation. Commit/push authority (N7) and any disposition of these findings remain with the designated FSW engineer and the authorized project/PA authorities. This review discharges only the N17 (R4) obligation for the documentation in scope; it does not discharge N9/R2, N20 or N7.

---

## 1. Scope and method

### 1.1 Files reviewed

| Path | State | Change |
| --- | --- | --- |
| `tests/unit/test_pus.c` | modified | `stream_has_verification` helper (:680–698), bounds guard on `stream_has` (:154–170), `test_verification_failure_via_ingest` (:705–793), `FF_TEST_MAIN` registration (:1396) |
| `docs/vv/test-spec/ts-1-protocol-core.md` | modified | rev 1.10 → 1.11; `N22-TS1-20`/`N22-TS1-23` oracles restated; new `gap` case `N22-TS1-104`; §2 range, §3 gap list, §3.1 totals, baseline, history |
| `docs/vv/tool-qualification.md` | modified | T3 row (:26) hash re-bound; history row 1.8 (:258) |
| `docs/vv/verification-results.md` | modified | new cycle section (:623–703) |
| `docs/planning/sprint-plan.md` | modified | one added row (:432), `NC-EXT-240` |
| `docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/` | new, untracked | 9 gate/mutation logs |
| `docs/interfaces/ICD.md` | **unchanged** | read as the cited interface authority |
| `src/pus/pus.c`, `src/pus/pus_types.h`, `src/transport/ccsds/ccsds.c`, `tests/CMakeLists.txt` | **unchanged** | read to verify every claim |

### 1.2 Out of scope

No target, RTEMS, MPFS, Renode, QEMU or hardware behaviour was reviewed or is claimed. No Lean or trace re-derivation beyond reading the retained log. No production-source design review (`src/` is byte-identical to `e5f01d4`; verified by hash). No review of `tests/unit/test_pus_s11_residual.c` content beyond confirming the factual basis of `NC-EXT-240`. ECSS-E-ST-70-41C and CCSDS 133.0-B-2 full texts were **not** accessed (paywalled); no clause-level conformance statement is made.

### 1.3 Independent reproduction

The gate claims were re-executed in a reviewer-owned build directory (`/tmp/opencode/r4-doc-build`); the author's `build/evidence-20261112-pus-s1` was not modified.

```bash
cmake -S . -B /tmp/opencode/r4-doc-build -DCMAKE_BUILD_TYPE=Debug -DFF_USE_PUS_ASN1=ON
cmake --build /tmp/opencode/r4-doc-build --target test_pus test_pus_s11_residual \
      test_link_tsn test_link_ip -j"$(nproc)"
cd /tmp/opencode/r4-doc-build
./tests/test_pus; echo $?             # checks=320 failures=0, exit 0
./tests/test_pus_s11_residual; echo $? # checks=1164 failures=10, exit 1
./tests/test_link_tsn; echo $?        # checks=149 failures=0, exit 0
./tests/test_link_ip; echo $?         # checks=93  failures=0, exit 0
```

`test_pus_s11_residual` fails at lines 217, 226, 235, 245, 246, 262, 273, 486, 487, 498 (10 `FAIL` lines, exit 1) — identical to the count reported in the delta. The four `checks=` totals match `tool-qualification.md` rev 1.8 and `verification-results.md` exactly.

---

## 2. Verified claims (fact check)

Every material numeric, hash and locator claim in the changed documents was checked against the tree and the retained logs.

| # | Claim in the delta | Verification | Verdict |
| --- | --- | --- | --- |
| V1 | `test_pus.c` sha256 `1e9c86e8…cc264` | `sha256sum` = `1e9c86e8179858b30da2c40911e289916e076a24b3698bfaeb9e5f6aae3cc264` | **true** |
| V2 | `test_link_tsn.c` sha256 `c4ffe085…7132f` | `sha256sum` = `c4ffe0858cc2e08d5812cfb8445a5b081400a05e03131aa700a9d96c0677132f`, unchanged since `ea0ac69` (2026-11-11) | **true** (and it corrects the erroneous `0b2cac2a…` recorded in rev 1.7) |
| V3 | `test_link_ip.c` sha256 `8153e6e1…ba20` | matches | **true** |
| V4 | `src/pus/pus.c` byte-identical to HEAD, `1eea40c0…5401` | matches; `git status --porcelain src/` empty | **true** |
| V5 | `test_pus` executes 320 checks, 0 failures | reproduced (V1.3) | **true** |
| V6 | `test_pus_s11_residual` = 1164 checks, 10 failures | reproduced, exit 1 | **true** |
| V7 | `test_pus_s11_residual` is in `FF_TEST_TARGETS` (`tests/CMakeLists.txt:110`) but has no `add_test` | confirmed: 57 `add_test` calls in total, none for that target | **true** |
| V8 | CTest 57/57 excludes that target | `gate-ctest.log` = 57/57, no `s11_residual` entry | **true** |
| V9 | TS-1 rev 1.11 = 104 cases = 77 `covered` + 27 `gap`; 51 OOB, 48 nominal, 5 mixed, 93 individual, 11 combined, SIL 101 / PIL 1 / HIL 2 | recount over the table rows: exactly those totals, 104 unique IDs, no duplicates or missing IDs | **true** |
| V10 | §3 gap list contains the same 27 IDs as the table | set comparison: identical | **true** |
| V11 | `covered` count not inflated (77 before and after); the increment adds exactly one `gap` case | 103 → 104 rows, 26 → 27 `gap`, `covered` unchanged at 77 | **true — no coverage inflation** |
| V12 | §2 case range now `N22-TS1-1 … N22-TS1-104` | matches the table (`:90`) | **true** |
| V13 | payload octet correction `payload[4]` → `payload[8]` | TM secondary header is 13 octets (`FF_PUS_TM_SEC_HDR_LEN`), payload starts at absolute 19; `pus.c:352` writes the code at `extended[8]`; the helper reads absolute byte 27 (`g_rx[pos + 27]`) = `payload[8]`. The pre-change `payload[4]` was wrong. | **true — correction is right** |
| V14 | echoed service/subservice at TM bytes 7/8 | `FF_CcsdsEncodeTm` writes service/subservice at absolute 7/8; helper reads `pos + 7` / `pos + 8` | **true** |
| V15 | mutation 1: `FF_PUS_FAIL_ILLEGAL_APID` → `..._BAD_DATA` at `src/pus/pus.c:424` fails 1 check at `test_pus.c:731` | `mutation-check.log` records exact patch, line, before/after sha256, the failing check, and restoration; the line references match the tree | **true** |
| V16 | mutation 2: removing the `tc.service == 0u` selection at `src/pus/pus.c:439` fails 2 checks at `test_pus.c:779-780` | same | **true** |
| V17 | `N22-TS1-20` handler-not-found branch was already covered by `tests/unit/test_r3_gap.c` (ack = ACCEPTANCE\|COMPLETION, codes 8/9) | `test_r3_gap.c` drives (17,8) with codes 8/9 | **true** |
| V18 | no test in the tree referenced `FF_PUS_FAIL_ILLEGAL_APID` before the change | `grep` shows the symbol only in `pus_types.h:39`, `pus.c:424` and the new test | **true** |
| V19 | TS-1 baseline row moved to `e5f01d4` | header `:14` matches | **true** |
| V20 | T3 remains NOT FULLY QUALIFIED; no NC or sprint closed; NC-EXT-15 not closed | `tool-qualification.md:26`, `:258`; `verification-results.md:688–696` | **true — no qualification upgrade, no NC closure** |
| V21 | "Independent delta review is pending and is not claimed by this record" (TS-1 history 1.11) | VV-R2 review of 2026-11-12 exists; at the time TS-1 rev 1.11 was written the statement was true, and the statement is scoped to that record | **true at issue time**; see DOC-R4-003 for the consequence |
| V22 | `NC-EXT-240` is the next free NC identifier (sequence contiguous with NC-EXT-239) | `sprint-plan.md:37` requires exactly this | **true** |

The delta's factual content is, with the exceptions recorded in §3, accurate: hashes, counts, offsets, line references, log filenames and mutation results all check out. The remaining findings are about **where and how** that accurate content is recorded, and about one oracle whose ground is claimed but absent.

---

## 3. Findings

Severity convention used here: **Major** = affects the truth of an assurance, coverage or evidence claim, or misstates the disposition of an NC; **Minor** = identity, bookkeeping, currency or wording. The "blocks commit" column is a separate determination.

**Relation to the N20 findings (PA-R2-001 … PA-R2-006).** This review was performed after the PA re-review and independently reached several of the same conclusions. For the review loop, the mapping is:

| R4 finding | N20 finding | Status here |
| --- | --- | --- |
| DOC-R4-001 | PA-R2-006 (Major; requested "NC registration … with owners and milestones") | Registration exists but is **mis-filed** — repeat, not closed |
| DOC-R4-002 | *(none — new)* | ICD §4 does not define the S1 payload layout the spec cites |
| DOC-R4-003 | PA-R2-003 (Minor; "staging-scope paragraph says the files 'are staged'; nothing is staged") | Wording changed to "are committed", but the paragraph now mischaracterises this cycle's own reports — partially addressed |
| DOC-R4-007 | PA-R2-001 (Minor; "novelty wording overstates the gap") | Uncorrected — repeat |
| DOC-R4-008 | PA-R2-002 (Minor; "`gate-configure.log` is cited but absent") | The remedy substituted a **later** configure's output (20:07) for the cited 19:39 gate — the retention defect is not resolved |
| DOC-R4-004 | *(none — new)* | stale register header |
| DOC-R4-005 | *(none — new)* | history-count and hash-correction wording |
| DOC-R4-006 | *(none — new)* | sprint-plan revision history |
| DOC-R4-009 | *(none — pre-existing)* | table ordering |
| DOC-R4-010 | *(none — new)* | SRS-REQ-017 trace attribution |

Findings raised twice by two independent profiles and left uncorrected should be treated as higher-risk than their individual severity suggests (N12).


### DOC-R4-001 — `NC-EXT-240` is filed in a table whose column contract it violates, under a deferral heading that contradicts its own milestone, and it is absent from the NC register (Major — blocks)

**Location:** `docs/planning/sprint-plan.md:432`

**Evidence**

- The NC register header is `docs/planning/sprint-plan.md:114`: `| NC | Source | Severity | Item | Disposition |` — five columns.
- The row was inserted into the subsection `**Postponed to the last planned sprint (M15 / S18-S25):**` (`:421`) whose header is `:423`: `| NC | Topic | Deferral consequence |` — three columns.
- The inserted row carries **five** cells, including `Major (verification registration) NC` in the position where the 3-column header has `Topic`. It renders with two orphan cells and a severity value in the topic column.
- The row's own disposition text states `Milestone: M17`. A reader of the M15 deferral list is therefore told that a Major NC will be handled in the last planned sprint, while the row itself schedules it one milestone earlier.
- `grep -rn "NC-EXT-240"` over the repository returns no occurrence inside the five-column register, so the NC is not visible where NC dispositions and milestones are tracked (and will not be found from `docs/status/handover.md`, which points at that register).

**Why it matters.** The register is the record of what is open, who owns it and when it is due. Filing a new Major NC only inside a differently-shaped deferral table misstates both its severity and its schedule. `NC-EXT-240` records a real defect (a declared test target that CTest never runs); a mis-filed record makes it more likely that the M17 obligation is missed, which is the failure mode N21/N12 exist to prevent.

**Bounded fix (mechanical).** Move the row into the five-column register under `:114` with the same five fields (`NC | Source | Severity | Item | Disposition`), keeping the existing text and the M17 milestone. If a per-milestone grouping is wanted, add a correctly-shaped subsection with its own 3-column header *and* a separate severity statement, and cross-reference it from the register. Do not change the factual content; do not close, downgrade or re-scope the NC.

### DOC-R4-002 — TS-1 grounds an octet-level S1 payload oracle in "ICD §4", which does not define that layout; the ICD's own S1 rows contradict the emitted payload (Major — blocks unless the citation is narrowed or the interface authority acts)

**Locations:** `docs/vv/test-spec/ts-1-protocol-core.md:123` (`N22-TS1-20`), `:126` (`N22-TS1-23`); `docs/interfaces/ICD.md:103–108`, `:123`

**Evidence**

- TS-1:123 asserts "exactly one (1,8) whose **9-octet payload** carries the failure code in its last octet (**payload[8]**) … (SRS-REQ-005 + **ICD §4**)".
- TS-1:126 asserts the same for (1,2) and adds "… and the **Request-ID octets** (source `0x0102`, **provisional destination** `0x0001`) (SRS-REQ-017 TC APID + **ICD §4**)".
- `docs/interfaces/ICD.md` §4 (rev 1.7) describes the S1 payload only as: (1,1) `TC type/subtype, source id, errcode 0`; (1,2) `errcode`; (1,8) `errcode` (`:103–108`). There is no octet order, no offset, no 9-octet length, no Request-ID field, no destination identifier and no sequence count anywhere in §4; `:123` only lists the `FF_PUS_FAIL_*` code names.
- The 9-octet layout exists only in the implementation: `src/pus/pus.c:326–353` (comment "Request ID per ECSS-E-ST-70-41C §5.4.11.2.1(c) … The TM sink expects: Request ID (6) + service (1) + subservice (1) + code (1) = 9 octets") where the destination octets are annotated "provisional framework default (GEO OBC = 0x0001). **Mission authority must confirm or replace via ICD**."
- The test oracle (`test_pus.c:731`, `:779-780`, helper at `:680–698` reading payload bytes 25/26/27) is therefore derived from the same source comment as the implementation it verifies — a common-mode assumption, not an independent interface oracle. The ICD row for (1,2) says `errcode`, i.e. the controlled interface document describes a payload that this build does not emit.
- I could not verify ECSS-E-ST-70-41C §5.4.11.2.1(c) (paywalled, not accessed). **I make no statement** about whether the Request ID should contain a destination identifier; that is an interface-authority question, and the source comment itself defers it to that authority.

**Why it matters.** TS-1's own taxonomy defines `covered` as "oracle grounded in a requirement/interface clause" (`:292`), and §3.1 provides a `host covered (…)` form for rows that carry provisional qualifiers. `N22-TS1-20` and `N22-TS1-23` are marked plain `covered` while their octet-level oracle has no interface clause behind it. That overstates the evidential value of the row and, more importantly, hides a real ICD↔implementation divergence (S1 payload layout and the provisional destination identifier) that no NC currently records.

**Bounded fix (choose one).**

- **(a) Narrow the record — preferred, editorial, no authority decision needed.** Replace "ICD §4" in `:123` and `:126` with "SRS-REQ-005/017 + implementation `FF_PusSendVerification`; S1 payload octet layout is **not** yet ICD-controlled", change the status to the `host covered (S1 payload layout not ICD-controlled; destination identifier provisional)` form, and register the ICD §4 S1-layout divergence as a new NC (owner: interface authority; milestone to be set by the owner) so the divergence is visible.
- **(b) Have the interface authority define the layout** in ICD §4 — including whether the destination identifier belongs in the Request ID — and bump the ICD revision. This makes the existing citation true, but it is an authority decision and must not be made editorially inside this test increment.

Do **not** resolve this by deleting the octet assertions: they are the discriminating part of the test.

### DOC-R4-003 — The cycle's own review reports are untracked, the review index has no rows for them, and `verification-results.md` declares untracked review reports to be unrelated to this change (Major — blocks)

**Locations:** `docs/vv/verification-results.md:697–703`; `docs/reviews/README.md`; untracked `docs/reviews/PA-R2-20261112-pus-s1-ingest-verification-failures.md`, `docs/reviews/VV-R2-20261112-s8-pus-fu-s1-verification-failures.md`, and this report

**Evidence**

- `git status --porcelain` lists both reports for this increment as `??` (untracked).
- `verification-results.md:697–703` states: "Pre-existing untracked paths in the worktree (`.opencode/plugins/`, `opencode.json`, **earlier review reports**, `tests/unit/test_sched_events.c.bak`) are unrelated to this change, are not gitignored by the project, and are deliberately left untouched and unstaged." The PA-R2 re-review and the VV-R2 review of *this* delta are not "earlier … unrelated" material: they are this cycle's assurance evidence, produced after the delta and cited by it (`NC-EXT-240` is derived from PA-R2-006).
- `docs/reviews/README.md` has no row for either report and none for this report.
- The immediately preceding increment in the same series, commit `3b04cb2`, committed its four review reports **and** a README row in the same commit (`git show --stat 3b04cb2`: `PA-REVIEW-TEST-PUS-SVC20-PARAM-SET-20261112.md`, `VV-R2-20261112-pus-s20-param-set-bounded.md`, `SYS-REVIEW-…`, `2026-11-12-r4-doc-review-…`, `README.md`).
- N11 (`docs/rules.md:158–176`): all review results "shall be stored in the project repository … the complete evidence set is committed and pushed with the change". N17 (`:287–300`): the review loop "shall be closed with all findings resolved before the change is committed and pushed".

**Why it matters.** As written, the staging-scope paragraph would justify committing the delta without the reviews that make it a compliant cycle, and the review index would silently lose this increment. The statement is also factually wrong about the provenance of the two named reports, which is exactly the kind of provenance error that previous reviews in this repository have treated as blocking.

**Bounded fix.** (1) `git add` this cycle's review reports (`PA-R2-…`, `VV-R2-…`, and this R4 report after the re-review loop closes). (2) Add one README row per report. (3) Rewrite the staging-scope sentence to distinguish three groups explicitly: reports belonging to this cycle (committed with the delta); genuinely earlier untracked reports (state which, and why they are out of scope); and non-evidence worktree items (`.opencode/plugins/`, `opencode.json`, `tests/unit/test_sched_events.c.bak`) that are not committed.

### DOC-R4-004 — `tool-qualification.md` header still reads rev 1.7 / 2026-11-11 while the file contains history row 1.8 (Minor)

**Location:** `docs/vv/tool-qualification.md:8` (Revision `1.7`), `:9` (Date `2026-11-11`), history row 1.8 at `:258`

**Evidence.** The document now carries an 1.8 history row dated 2026-11-12 and `verification-results.md:692–693` cites "`tool-qualification.md` rev 1.8", but the header block was not updated. TS-1's header *was* bumped in the same change (1.10 → 1.11), so the treatment is inconsistent within one delta. The same defect class was previously recorded by PA as a Minor in FRICK-REVIEW-0459.

**Fix.** Set `Revision` to `1.8` and `Date` to `2026-11-12`. Does not block on its own; fix in the same commit.

### DOC-R4-005 — Rev 1.8's history text understates the hash staleness and does not record that rev 1.7's `test_link_tsn.c` hash matched no committed revision (Minor)

**Location:** `docs/vv/tool-qualification.md:258`

**Evidence.**

- "The previously recorded candidate hash `cb9e7bb2…` was **three revisions stale**": `cb9e7bb2` is `test_pus.c` at `3b04cb2`; the file then changed in `29cba32` (`fa061e2e`), `c3fe024` (`bcfb5b99`), `e5f01d4` (`776a7863`) **and** in the uncommitted working tree (`1e9c86e8`). Relative to the candidate the register must describe, that is **four** later revisions (three intervening commits). The count is arguable, the wording is not: "three revisions stale" is at best ambiguous in a register whose purpose is exact hash binding.
- Part (b): rev 1.7 recorded `test_link_tsn.c` as `0b2cac2a…e382`; the file has been `c4ffe085…` since commit `ea0ac69` (2026-11-11) and no commit contains `0b2cac2a`. Rev 1.8 re-binds the hash correctly but describes this only as "likewise re-bound … its NC-EXT-191 revision", without stating that the previously recorded value was erroneous. N16 asks for a detailed, accurate revision history; a correction that does not say what was wrong leaves the next reader unable to tell whether a revision changed or the record did.

**Fix.** Say "three intervening commits (four later revisions of the file, including the working-tree delta)" and add one clause to the `test_link_tsn.c` sentence: "rev 1.7's recorded `0b2cac2a…` matches no committed revision; the file has been `c4ffe085…` since `ea0ac69`."

### DOC-R4-006 — `sprint-plan.md` modified without bumping the header revision or adding a History row; no CHANGELOG entry (Minor)

**Locations:** `docs/planning/sprint-plan.md:6` (Revision `1.137`), `:7` (Date `2026-11-12`), History section at `:471`

**Evidence.** `git diff docs/planning/sprint-plan.md` is a single insertion at `:432`; the header revision is unchanged and no History row was added. N16 requires a detailed revision history for every change. (Note for fairness: the three recent test-only commits `3b04cb2`, `29cba32`, `c3fe024` also skipped `CHANGELOG.md`, so the CHANGELOG omission is consistent with current practice, but the sprint-plan History omission is not — this document's own History table is maintained per revision.)

**Fix.** Bump to `1.138` and add a History row describing the `NC-EXT-240` registration (after DOC-R4-001 is applied).

### DOC-R4-007 — The novelty claim "no test implemented it" is imprecise; an existing test already asserted the (1,2) branch (Minor)

**Locations:** `docs/vv/test-spec/ts-1-protocol-core.md:339` ("was marked `covered` although **no test implemented it**"); `docs/vv/verification-results.md:630–632` ("was recorded as `covered` in TS-1 **without any implementing test**")

**Evidence.**

- The narrow claim in the same sentences is **true**: no file in the tree referenced `FF_PUS_FAIL_ILLEGAL_APID` before this change (V18), so no test asserted the *failure code*.
- The broad claim is **false**: `tests/unit/test_pus_s11_residual.c:139–152` already drove a non-TC APID (`0x1234`) with `ack = ACCEPTANCE` through `FF_PusIngest` and asserted `FF_ERROR` plus exactly one `(1,2)`; and `tests/unit/test_decoder_corpus.c:620–628` (a **registered** CTest case) already exercised non-TC-APID rejection (`FF_ERROR`, no dispatch). What is genuinely new is the code-octet assertion, the Request-ID octets, and the non-dispatch assertion at payload level.
- This is the same imprecision already raised by the N20 re-review as **PA-R2-001** (Minor), which remains uncorrected in the current text.

**Fix.** Replace "without any implementing test" / "no test implemented it" with the precise statement: "no test asserted the failure code or the payload layout; `test_pus_s11_residual.c` (unregistered) and `test_decoder_corpus.c` already exercised non-TC-APID rejection at packet level." No oracle or count changes.

### DOC-R4-008 — The retained evidence does not substantiate two gate-table claims and cannot be bound to the tested artifacts (Major — blocks unless qualified)

**Locations:** `docs/vv/verification-results.md:649–664` (gate table); `docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/`

**Evidence**

- **Chronology.** `gate-build.log` 19:51, `gate-ctest.log` 19:52, `gate-test-pus.log` 19:52, but `gate-configure.log` **20:07**. In the same build directory, `tests/test_pus` was relinked at **19:58** and `tests/test_pus_s11_residual` built at **20:00**. The log presented as the *first* gate was therefore written about 15 minutes after the build and the CTest run it is supposed to precede, so it is not the configure that produced the tested tree. The cause is traceable: the N20 re-review recorded that this log was **absent** from the evidence set (PA-R2-002) and asked for the log to be restored or the citation amended; what was restored is a **later re-run's** output, not the 19:39 configure evidenced by `CMakeCache.txt`. Restoring a log is not the same as retaining the original gate evidence, and substituting a later run silently changes what the row asserts.
- **Unqualified "0 compiler warnings".** `gate-configure.log` records no exit status and contains six ASN1SCC `warning:` lines (`pusclib/common/BasicTypes.acn`, endianness properties with no effect), while the table states "Build | exit 0, 0 compiler warnings" without a scope qualifier.
- **Filtered build log.** `gate-build.log` contains only `Built target` and `Linking` lines and **zero** compiler invocation lines (`grep -ci warning` = 0). It is filtered or incremental output and cannot substantiate "0 compiler warnings". Mitigating fact (verified): `-Werror` is applied to `frick_fsw_core` (`CMakeLists.txt:316`) and to every `FF_TEST_TARGETS` entry (`tests/CMakeLists.txt:208`), so a warning in project code fails the build; the claim is credible, but the retained log does not evidence it.
- **No artifact binding.** The directory contains no `assets.sha256` manifest and no `README.md`, unlike the precedent evidence sets (`docs/vv/evidence/2026-11-03-gate-diagnosis/assets.sha256`, `docs/vv/evidence/2026-11-11-nc-ext-237/README.md`). Nothing in the directory hashes the logs against the source set or the binaries, so the 19:52 CTest log cannot be bound to the 19:58/20:00 binaries.
- **Positive.** `mutation-check.log` is well formed (file, exact patch, line, before/after sha256, failing check lines, restoration by sha256, trailing empty `git status` for `src/pus/pus.c`). Its `Date: 2026-11-12T18:52:41Z` is consistent with the 19:52 local mtime under CET. No finding on the mutation control itself.

**Why it matters.** The gate table is the N11 record of this cycle. Two of its rows state more than the retained logs can show, and no log is hash-bound to the artifacts it is said to describe. This is the same defect class as FRICK-REVIEW-0454 F-01 (evidence not bound to the candidate).

**Bounded fix (either).**

- Re-run configure → build → CTest once in the evidence directory, capture **unfiltered** build output and explicit exit statuses for every gate, and add a `README.md` or `assets.sha256` binding each log to the source sha256 set and the test binaries; **or**
- If the original 19:39 configure output no longer exists, say so in the results record and cite the 20:07 log explicitly as a **post-hoc re-run** (not as the gate that produced the tested tree), qualify the "0 compiler warnings" row as described below, and add the log-to-artifact binding.
- Qualify the two rows in the table: "no warnings from project targets; `-Werror` enforced on `frick_fsw_core` and all `FF_TEST_TARGETS`; ASN1SCC emits 6 endianness warnings during code generation (third-party generator, not project sources)" and state the log-to-artifact binding limitation explicitly.

### DOC-R4-009 — New case row continues a pre-existing out-of-order block in the TS-1 table (Minor, pre-existing)

**Location:** `docs/vv/test-spec/ts-1-protocol-core.md:146`

**Evidence.** `N22-TS1-104` is inserted between `N22-TS1-103` and `N22-TS1-34`. Rows 95–103 already sit out of numerical order ahead of `N22-TS1-34` (a pre-existing defect, not introduced here); the new row follows the local pattern. Every ID is unique and the §2 range statement is correct, so this is presentation only.

**Fix.** None required for this delta. Record it as NC material for the next TS-1 structural pass (renumber or reorder the 95–104 block) rather than expanding this change.

### DOC-R4-010 — `traceability.yaml` SRS-REQ-017 registration does not reflect the new evidence (Minor)

**Location:** `docs/requirements/traceability.yaml:181–187`

**Evidence.** SRS-REQ-017 lists `implementation: ["config/fsw_config.h", "src/transport/ccsds/ccsds.c"]` and `tests: ["tests/unit/test_ccsds.c"]`. The delta binds new SRS-REQ-017 evidence (illegal TC APID rejected before dispatch) to `tests/unit/test_pus.c`, and that behaviour is implemented in `src/pus/pus.c:420–428`. Neither path appears under SRS-REQ-017. The immediately preceding increment in the same series, `3b04cb2`, updated the `SRS-REQ-005` `lean_note` for exactly this reason, at the V&V reviewer's request. `make trace` still passes (it validates ID existence and coverage counts, not per-file attribution), so this is a currency/attribution gap rather than a broken gate.

**Fix.** Add `src/pus/pus.c` to SRS-REQ-017 `implementation` and `tests/unit/test_pus.c` to `tests` (no count changes; re-run `make trace` to confirm), or record why the APID acceptance rule is attributed only to SRS-REQ-005.

### Findings summary

| ID | Severity | Location | Blocks commit/push? |
| --- | --- | --- | --- |
| DOC-R4-001 | Major | `sprint-plan.md:432` | **Yes** |
| DOC-R4-002 | Major | `ts-1-protocol-core.md:123,126`; `ICD.md:103–108` | **Yes**, unless citation narrowed + divergence registered (or ICD updated by authority) |
| DOC-R4-003 | Major | `verification-results.md:697–703`; `reviews/README.md` | **Yes** |
| DOC-R4-004 | Minor | `tool-qualification.md:8–9` | No — fix in same commit |
| DOC-R4-005 | Minor | `tool-qualification.md:258` | No |
| DOC-R4-006 | Minor | `sprint-plan.md:6–7,471` | No |
| DOC-R4-007 | Minor | `ts-1-protocol-core.md:339`; `verification-results.md:630–632` | No |
| DOC-R4-008 | Major | `verification-results.md:649–664`; evidence dir | **Yes**, unless the two rows are qualified and a log-to-artifact binding is added |
| DOC-R4-009 | Minor (pre-existing) | `ts-1-protocol-core.md:146` | No |
| DOC-R4-010 | Minor | `traceability.yaml:181–187` | No |

---

## 4. English, terminology and consistency

- **N2 (English):** all added and changed text in the five modified documents is English and idiomatic. No defect.
- **Terminology:** TS-1 uses `(1,2)` notation consistently with the rest of the specification; the test source comments use `TM[1,2]`. That is a source-comment style difference, not a documentation defect, and it does not affect meaning.
- **`NC-EXT-240` text quality:** the row is unusually long for a register cell but its content is precise and correctly distinguishes "stale-oracle and registration defects, not a production regression", names the ADR-0020 mapping, gives both disposition options and states explicitly that the NC is not closed, passed or waived. The content is good; only its placement and shape are wrong (DOC-R4-001).
- **Consistency across documents:** `NC-EXT-240` is described consistently in `verification-results.md:678–687` and `sprint-plan.md:432` (same line reference `tests/CMakeLists.txt:110`, same `checks=1164 failures=10`, same M17 milestone). `tool-qualification.md` rev 1.8 and `verification-results.md` agree on all four hashes and all four check counts. Two cross-document inconsistencies remain: the stale `tool-qualification.md` header (DOC-R4-004) and the TS-1 ↔ ICD §4 S1-payload divergence (DOC-R4-002).
- **Unit and offset statements:** the `payload[8]` / absolute-octet-27 / 9-octet-payload statements are arithmetically consistent across TS-1, `verification-results.md`, the helper and the implementation (V13).
- **No invented requirement or interface clause:** the delta adds no SRS/SSS requirement and no ICD clause, and says so in both TS-1 history 1.11 and `verification-results.md:688–696`. That is correct — but it is also why DOC-R4-002 exists: the octet oracle cites a clause the ICD does not contain.

---

## 5. What the delta does **not** claim (verified)

These are the claims most likely to be inflated in an increment of this kind; each was checked and is correctly bounded:

- **No tool-qualification upgrade.** T3 remains **NOT FULLY QUALIFIED**; `tool-qualification.md:26` and `:258` both say so, and rev 1.8 explicitly states "Execution is not qualification."
- **No NC closure.** `verification-results.md:688–696` keeps NC-EXT-15 open and lists NC-EXT-04, NC-EXT-14..18 and NC-EXT-47 as untouched; `tool-qualification.md:258` ends "No NC or sprint closed." `NC-EXT-240` is opened, not closed.
- **No coverage inflation.** `covered` is unchanged at 77; the increment adds exactly one `gap` case (`N22-TS1-104`) and states that explicitly in three places (TS-1:146, TS-1 history 1.11, `verification-results.md:645–647`).
- **No target, Renode or hardware claim.** The scope paragraph and the gate table are host-only, and `verification-results.md:662–664` says so.
- **No production-source change.** Verified by hash (V4).
- **The `57/57` figure is correctly bounded** in the `NC-EXT-240` row ("the CTest run (57/57) never executes it"), which is the right disclosure even though the underlying defect exists.

---

## 6. Verdict

**CHANGES REQUIRED.**

The technical content of this delta is sound and well evidenced in substance: the payload-octet correction is right, the mutation control discriminates, the counts and hashes are accurate, the ack = 0 conflict is correctly recorded as a gap rather than as coverage, and nothing is claimed that the evidence does not support in kind. The documentation defects are in **record placement, oracle grounding and evidence binding**, and all four Majors are fixable in one bounded pass:

1. **DOC-R4-001** — move `NC-EXT-240` into the five-column NC register with the M17 milestone.
2. **DOC-R4-002** — stop citing "ICD §4" for an S1 payload layout the ICD does not define; narrow the citation to the requirement plus the implementation, mark the rows with the `host covered (…)` provisional form, and register the ICD↔implementation S1-layout divergence as an NC for the interface authority (or have the authority define the layout).
3. **DOC-R4-003** — commit this cycle's review reports (including this one after re-review) and add their `docs/reviews/README.md` rows; correct the staging-scope sentence.
4. **DOC-R4-008** — substantiate or qualify the "0 compiler warnings" and Configure rows, and bind the retained logs to the source hashes and test binaries.

The six Minors do not block on their own and should be corrected in the same commit because they are one-line edits (DOC-R4-004, DOC-R4-006, DOC-R4-007, DOC-R4-010) or one-clause edits (DOC-R4-005); DOC-R4-009 is pre-existing and should be recorded as NC material rather than expanded here.

**Commit/push blocking statement.** Under N17 ("a REJECTED documentation review blocks commit/push", `docs/rules.md:287–300`) this review **does not clear** the documentation gate for this delta. DOC-R4-001, DOC-R4-002, DOC-R4-003 and DOC-R4-008 must be corrected, or a disposition must be recorded by the authorized owner (interface authority for DOC-R4-002; Software PA/FSW for the others), and the corrections must be re-reviewed before the delta is committed and pushed. This report recommends only; it does not authorize, approve or block a release, and it does not close any NC.

---

## 7. Re-review procedure

Send back to this reviewer: the corrected `sprint-plan.md` row, the corrected TS-1 rows and status tokens, the new NC registration for the ICD divergence (if option (a) of DOC-R4-002 is taken), the `git status`/`git diff --stat` showing the review reports staged, the README rows, and the corrected/qualified gate table with the evidence manifest. A delta re-review will verify only the corrections and their knock-on effects (TS-1 §3.1 totals, §3 gap list, §2 range, sprint-plan History, `make trace`), not re-open the verified content of §2 above.

---

## History

| Rev | Date | Author | Reason |
| --- | --- | --- | --- |
| 1.0 | 2026-11-12 | frick-fsw (R4 profile, independent context) | Initial N17 documentation review of the S8-PUS-FU S1 ingestion-failure test increment against `e5f01d4` plus the uncommitted delta: 22 claims verified by independent reproduction, 4 Major and 6 Minor findings, CHANGES REQUIRED, commit/push documentation gate not cleared. No file other than this report was created or modified; `build/evidence-20261112-pus-s1` was not touched. |
