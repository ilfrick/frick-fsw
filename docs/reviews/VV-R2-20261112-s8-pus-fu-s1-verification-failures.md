# Independent SW V&V Review — PUS Service 1 ingestion failure paths (`test_verification_failure_via_ingest`)

| Field | Value |
|---|---|
| Review ID | VV-R2-20261112-S8-PUS-FU-S1-INGEST |
| Reviewer profile | Senior Aeronautics and Space SW V&V Engineer (independent-context R2, norms N9/N10/N22) |
| Repository | `/home/nicola/fsw/frick-fsw`, branch `main`, HEAD `e5f01d4` |
| Change under review | Uncommitted working tree: `tests/unit/test_pus.c` only (no `src/` change) — new `test_verification_failure_via_ingest` (lines 705-793), new helper `stream_has_verification` (lines 680-698), bounds guard added to the pre-existing `stream_has` (line 158), registration in `FF_TEST_MAIN` (line 1396). Companion documentation deltas: `docs/vv/test-spec/ts-1-protocol-core.md` rev 1.11, `docs/vv/tool-qualification.md` rev 1.8, `docs/vv/verification-results.md`, `docs/planning/sprint-plan.md` (NC-EXT-240 row), evidence directory `docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/`. |
| Date | 2026-11-12 |
| Governing norms | N1-N22 (`docs/rules.md`); in particular N10 (test-script/tool qualification), N11 (evidence retention), N21 (work sequencing), N22 (requirement-based nominal + out-of-bound tests with explicit expected results), N7 (commit/push authority). ECSS-E-ST-40C Rev.1 / ECSS-Q-ST-80C Rev.2 context; ECSS-E-ST-70-41C (PUS-C) service 1; CCSDS 133.0-B-2. |
| Criticality | ECSS Class B project target (no controlled mission assignment evidenced — NC-EXT-5/11 class) |
| Build / run environment | Reviewer-owned `build/vv-r2-review-20261112` (CMake + Ninja, `Debug`, `FF_USE_PUS_ASN1=ON`, host x86_64). Author evidence in `build/evidence-20261112-pus-s1` was **not** touched. |
| Scope | The new test's oracles against `src/pus/pus.c`, `src/pus/pus_types.h`, `src/transport/ccsds/ccsds.c`; determinism/isolation; independent re-execution of the mutation controls; full CTest regression; TS-1 case bookkeeping; T3 tool-qualification row accuracy. |
| Exclusions | No target/RTEMS/MPFS/Renode/QEMU/HIL execution; no claim about N18 evidence; no Lean/trace re-run (their inputs are unchanged by a test-only delta); no review of `test_pus_s11_residual.c` content beyond confirming NC-EXT-240's factual basis; no production-source review (`src/` is byte-identical to HEAD). |

> **Authority statement.** This report is an independent analysis artifact. It has no approval, qualification, acceptance, certification, or flight-readiness authority. Commit/push authorization (norm N7), safety/security acceptance, PA sign-off (norm N20) and any residual-risk decision remain with the designated FSW engineer and the human authorities. This report may recommend "do not proceed"; it may not authorize proceeding.

## 1. Change under review (verified diff)

`git diff tests/unit/test_pus.c`: +127 / −4 lines. Verified content:

| Element | Lines | Verified content |
|---|---|---|
| `stream_has` guard | 154-170 (guard at 158) | Loop condition changed from `pos < g_rx_count` to `(pos + 9u) <= g_rx_count`; no other change |
| `stream_has_verification(subservice, req_service, req_subservice, code)` | 680-698 | Walks the captured stream by CCSDS PDL stride; requires `step >= 28`; matches TM service byte 7 = 1, subservice byte 8, payload bytes 25/26/27 |
| `test_verification_failure_via_ingest` | 705-793 | Five sub-cases (1: illegal APID + ack = ACCEPTANCE; 2: illegal APID + ack = 0; 3: TC(17,9) + ack = COMPLETION; 4: TC(0,1) + ack = COMPLETION; 5: TC(17,9) + ack = 0) |
| `FF_TEST_MAIN` registration | 1396 | Registered after `test_verification_missing_subservices` |

`git status --short src/` is empty and `sha256sum src/pus/pus.c` = `1eea40c06ecf399326347585995f71eec718318e535332f44edb043db3e15401`, identical to HEAD. The claim "no production source changed" is **confirmed**.

`sha256sum tests/unit/test_pus.c` = `1e9c86e8179858b30da2c40911e289916e076a24b3698bfaeb9e5f6aae3cc264`, matching `tool-qualification.md` rev 1.8.

## 2. Oracle verification against the implementation

### 2.1 Offset arithmetic (independently derived and confirmed)

| Quantity | Derivation | Implementation evidence | Verdict |
|---|---|---|---|
| TM primary header | 6 octets | `src/transport/ccsds/ccsds.h:19` (`FF_CCSDS_HDR_LEN 6u`) | confirmed |
| PUS TM secondary header | 13 octets | `src/pus/pus_types.h:71` (`FF_PUS_TM_SEC_HDR_LEN 13u`); layout written at `src/transport/ccsds/ccsds.c:200-208` (version, service, subservice, counter, destination, coarse, fine) | confirmed |
| Verification payload start | 6 + 13 = **19** | `ccsds.c:210-211` copies `data` at `FF_CCSDS_HDR_LEN + FF_PUS_TM_SEC_HDR_LEN`; `ICD.md` §3 documents the same 13-octet TM secondary header | confirmed |
| Request ID | 6 octets (source 2 + destination 2 + sequence count 2) → offsets **19..24** | `src/pus/pus.c:332-344` (`s_sequence_count`, `payload[0..5]`, destination `0x0001`) | confirmed |
| Echoed service / subservice | offsets **25 / 26** | `pus.c:350-351` (`extended[6]`, `extended[7]`) | confirmed |
| Failure code | offset **27** = `payload[8]` | `pus.c:352` (`extended[8] = code`); 9-octet payload sent at `pus.c:353` | confirmed |
| Service/subservice header bytes | TM bytes 7 / 8 | `ccsds.c:200-202` | confirmed |

The helper's minimum-packet guard (`step >= 28`, line 690) equals 6 + 13 + 9, and remains correct with the CRC trailer enabled (30-octet packet, `FF_FRAME_WITH_CRC=1` in `config/fsw_config.h:111`), because PDL includes the trailer.

### 2.2 Sub-case oracle verification

| # | Test assertion | Implementation path | Verdict |
|---|---|---|---|
| 1 | APID `0x011`, ack = ACCEPTANCE → `FF_ERROR`; exactly one TM(1,2); echoed 17/1; code `FF_PUS_FAIL_ILLEGAL_APID` at payload[8]; Request-ID 19..22 = `01 02 00 01`; no TM(1,1); no TM(17,2) | `pus.c:420-428`: `apid != FF_APID_TC` → if `ack_accept`, send `FF_PUS_STAGE_ACCEPTANCE, success=0, FF_PUS_FAIL_ILLEGAL_APID` and `return FF_ERROR`; stage mapping `pus.c:310-311` yields subservice 2; `payload[3] = 0x01u` at `pus.c:340` | **confirmed**, discriminating (see §4) |
| 2 | APID `0x020`, ack = 0 → `FF_ERROR`, no TM | `pus.c:420-428`, `ack_accept` false → unlock and `return FF_ERROR` with no emission | **confirmed** |
| 3 | TC(17,9), ack = COMPLETION → `FF_NOT_FOUND`; exactly one TM(1,8) code 9, echoed 17/9; no TM(1,1) | `(17,9)` is not registered (`src/pus/pus_services.c:688-723`, table contains `{17,1}` only) → `pus.c:437-444`: `code = UNKNOWN_SUBSERVICE` (service ≠ 0), `ack_complete` → completion failure (subservice 8) and `FF_NOT_FOUND` | **confirmed** |
| 4 | TC(0,1), ack = COMPLETION → `FF_NOT_FOUND`; TM(1,8) code 8; explicitly not code 9 | `pus.c:438-439` selects `FF_PUS_FAIL_UNKNOWN_SERVICE` (8) when `tc.service == 0u`; service 0 is unregistered | **confirmed** |
| 5 | TC(17,9), ack = 0 → `FF_NOT_FOUND`, no TM | `pus.c:440-446`: `ack_complete` false → unlock and `return FF_NOT_FOUND` with no emission | **confirmed** |

Symbolic binding is correct: the test asserts `FF_PUS_FAIL_ILLEGAL_APID` / `..._UNKNOWN_SERVICE` / `..._UNKNOWN_SUBSERVICE` (`src/pus/pus_types.h:39/42/43`) rather than the numeric literals, so a re-allocation of the codes cannot silently pass.

The `stream_has` guard change (line 158) is a defensive robustness fix rather than a correction of an observable defect: the previous condition could read `g_rx[pos+7]`/`[pos+8]` past the captured data if `g_rx_count` ever ended mid-packet. It cannot be triggered by the current `tm_sink` (`test_pus.c:146-151`), which only appends whole packets, so the guard protects future sink/truncation changes and aligns `stream_has` with the already-guarded `stream_count` (line 173). No legitimate TM packet is shorter than 9 octets, so no existing assertion changes meaning (confirmed by the 57/57 regression, §5).

## 3. Determinism and isolation

| Property | Finding |
|---|---|
| Threading | Single-threaded `main` (`tests/ff_test.h:33-42`); no test threads; `FF_PusIngest` is called synchronously |
| Wall clock | `FF_PusSendTm` stamps the TM header from `FF_TimeGetCuc` (`pus.c:185`), but the test asserts **no** time octets (13..18); no wall-clock dependency |
| Cross-test state | Each test calls `FF_PalInit`/`FF_EvsInit`/`FF_TimeInit`/`FF_PusInit`/`FF_PusSetBackend`; `FF_PusInit` clears `g_svcs`, `g_backend`, `g_tm_sink`, `g_tm_counter`, `g_tc_count` (`pus.c:44-51`) and `FF_PusRegisterCoreServices` clears every service registry (`pus_services.c:672-681`), so registration is not cumulative |
| Residual shared state | `s_sequence_count` (`pus.c:332`) is a function-local static that `FF_PusInit` does **not** reset and that is incremented outside `g_pus_mutex`. The test never asserts payload[4..5] (sequence count, absolute offsets 23-24), so the residual state cannot influence the result — see VV-R2-07 |
| Empirical order-independence | New test executed **alone**: `checks=30 failures=0`; new test moved to the **end** of `FF_TEST_MAIN` (after all other tests, i.e. maximum sequence-count drift): `checks=320 failures=0`; unmodified binary executed three times: `320/0` each time |
| Non-aborting harness | `FF_TEST_ASSERT` does not abort (`ff_test.h:17-24`), so all assertions in the test execute; a stale-buffer read would produce additional, not hidden, failures (see VV-R2-06) |

## 4. Mutation controls — independently re-executed

I re-ran both mutations recorded in `docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/mutation-check.log` in my own build directory, with the author's `src/` restored afterwards. Hashes and failing lines reproduce the log **exactly**.

| Control | Mutation (file:line) | Mutated sha256 | Result | Matches log |
|---|---|---|---|---|
| Author M1 | `pus.c:424` `FF_PUS_FAIL_ILLEGAL_APID` → `FF_PUS_FAIL_BAD_DATA` (string occurs once in the file) | `1df235d2…e8489f` | 1 failure at `test_pus.c:731` | yes, byte-for-byte |
| Author M2 | `pus.c:439` remove `(tc.service == 0u) ?` selection → always `FF_PUS_FAIL_UNKNOWN_SUBSERVICE` | `cd2b01fa…e8b9ed` | 2 failures at `test_pus.c:779`, `:780` | yes, byte-for-byte |
| Restored | — | `1eea40c0…e15401` | `checks=320 failures=0`; `git status --short src/` empty | yes |

The author's two controls only exercise the code-selection and subservice legs. Because the report claims payload-octet-level evidence, I added three reviewer probes on legs the author's log does not cover:

| Probe | Mutation (file:line) | Mutated sha256 | Result |
|---|---|---|---|
| P1 Request-ID destination octet | `pus.c:340` `payload[3] = 0x01u` → `0x02u` | `c19710e8…db8eaf` | 1 failure at `test_pus.c:737` (`g_rx[22] == 0x01u`) |
| P2 echoed subservice octet | `pus.c:351` `extended[7] = tc->subservice` → `0u` | `19f7cc03…384f464` | 3 failures at `:731`, `:765`, `:779` |
| P3 completion-ack gate on the handler-not-found branch | `pus.c:440` `if (ack_complete)` → `if (1)` | `d80c2559…fb7f718` | 1 failure at `test_pus.c:792` (`g_rx_count == 0u`) |

P1/P2 prove the Request-ID and echo assertions are discriminating. **P3 is significant for bookkeeping** (VV-R2-04): the "observation only" sub-case 5 is in fact pinned as a pass/fail check, so resolving NC-EXT-04 by making the completion failure unconditional will break this test.

`src/pus/pus.c` was restored to `1eea40c0…e15401` after every probe; `git status --short src/` is empty and the final run is `checks=320 failures=0`.

## 5. Regression (full CTest, reviewer build)

| Check | Result |
|---|---|
| `cmake -S . -B build/vv-r2-review-20261112 -G Ninja -DCMAKE_BUILD_TYPE=Debug` | exit 0 |
| `ninja` (full build, 413 targets) | exit 0, 0 warnings (project builds tests with `-Wall -Wextra -Werror`) |
| `ctest --output-on-failure` | **57/57 passed, 0 failed**, 28.37 s |
| `tests/test_pus` | `checks=320 failures=0` |
| HEAD-version `test_pus.c` linked against the same libraries (reviewer reproduction) | `checks=290 failures=0` |
| Delta | **+30 checks**, exactly the number of assertions in the new test (counted manually: 30; measured in isolation: 30) |
| `clang-format --dry-run --Werror tests/unit/test_pus.c` | exit 0 |

**Bounded statement (VV-R2-08):** "57/57" is the registered CTest set only. `test_pus_s11_residual` is declared in `FF_TEST_TARGETS` (`tests/CMakeLists.txt:110`) but has no `add_test(...)` registration; run directly it reports `checks=1164 failures=10` (reviewer reproduction). This is the pre-existing **NC-EXT-240**, correctly recorded as such in this change set; it does not affect `test_pus` and does not block this commit, but it does bound any "all declared test targets pass" claim.

## 6. TS-1 case bookkeeping — recount (not inflated)

Recounted independently from the rev 1.11 case table:

| Metric | Doc §3.1 | Reviewer recount | Verdict |
|---|---|---|---|
| Total cases | 104 | 104 rows, 104 unique IDs, 0 duplicates | consistent |
| `covered` | 77 | 74 rows with status `covered` + 3 with `covered (…)` qualifiers (`N22-TS1-9`, `-95`, `-99`) = **77** | consistent |
| `gap` | 27 | 27 | consistent |
| Out-of-bound | 51 | 51 | consistent |
| Nominal / mixed | 48 / 5 | 48 / 5 | consistent |
| Individual / combined | 93 / 11 | 93 / 11 | consistent |
| SIL / PIL / HIL | 101 / 1 / 2 | 101 / 1 / 2 | consistent |

- `N22-TS1-104` carries status `gap` and environment `SIL`, scope `individual`; it is **not** counted as covered. §3.1 stays at **77 covered** — the increment is not inflated.
- `N22-TS1-23` was previously marked `covered` with **no implementing test** (no test in the tree referenced `FF_PUS_FAIL_ILLEGAL_APID` before this change). The new test closes that gap; the row now names its evidence. Left unaddressed, a `covered` row without evidence would have been a Major evidence-integrity defect — the change removes it.
- `N22-TS1-20` was already covered by `tests/unit/test_r3_gap.c::test_pus_verification_failures` (lines 280-300: service 0 → code 8, subservice 0x7F → code 9, both ack = ACCEPTANCE\|COMPLETION). The new sub-cases 3/4 re-assert it at payload-octet level and correctly claim **no** additional covered row. Verified.
- The `payload[4]` → `payload[8]` correction in the `N22-TS1-20` row is factually right: payload[4] is the sequence-count high octet; the code is payload[8] (§2.1).

## 7. N10 / T3 tool qualification

| Check | Result |
|---|---|
| T3 row (rev 1.8) `test_pus.c` hash `1e9c86e8…c264` vs current bytes | match |
| T3 row "320 checks, 0 failures" | match (3 runs) |
| T3 row lists the five S8-PUS-FU additions incl. `test_verification_failure_via_ingest` | all five functions exist and are registered in `FF_TEST_MAIN` |
| T3 row re-bound `test_link_tsn.c` `c4ffe085…132f`, 149 checks | match (`sha256sum`; binary `checks=149 failures=0`) |
| T3 row `test_link_ip.c` `8153e6e1…ba20`, 93 checks | match |
| T3 row `test_evs_hs_health.c` `5310cc3c…277f`, `test_sched_events.c` `2a31472c…5ed5c`, `test_pus_event_identity.c` `828b6a22…24ac` | match |
| T3 row "No tool under `tools/` changed" | match (`git status` shows no `tools/` delta) |

**Factual accuracy of the T3 row at the current bytes: correct.** No new script or tool was added; `test_pus.c` remains a T3 script. **No unmet N10 obligation is created by this change**: the row does not claim qualification at these bytes — it states NOT FULLY QUALIFIED and that no independent review exists at them. This report is that independent R2 delta review for `test_pus.c` at `1e9c86e8…c264`; if accepted, the FSW engineer should re-issue the T3 row citing this review ID and hash, and PA (N20) should confirm. Qualification itself remains a project/PA act, not something this report grants.

## 8. Findings

| # | Severity | Item / requirement | Finding | Evidence | Recommendation |
|---|---|---|---|---|---|
| VV-R2-01 | **Minor** | N22-TS1-23 oracle provenance | The row's expected result asserts `FF_PusIngest` returns `FF_ERROR`, but no controlled source states that return code: SRS-REQ-005 defines the failure *report*, SRS-REQ-017 defines the APIDs, `ICD.md` §4 defines the S1 rows and code set, and `src/pus/pus.h:56` declares `FF_PusIngest` with no return-code contract. The return-code leg is characterization of the current implementation, while the wire-level legs (TM(1,2), code, no dispatch) are requirement/ICD-grounded. | `docs/vv/test-spec/ts-1-protocol-core.md` (row 23); `src/pus/pus.h:56`; `src/pus/pus.c:420-428` | Document the `FF_PusIngest` return-code contract in `pus.h`/`ICD.md` (API contract), or label the return-code leg as characterization in the TS-1 cell. Non-blocking. |
| VV-R2-02 | **Minor** | Failure-code allocation vs NC-EXT-18 | `ICD.md` §4 records that the failure-code allocation ("unknown service/subservice, illegal APID, execution failure") is an interface-authority item and that "the PUS-C failure-code allocation … is not made here (NC-EXT-18)". TS-1 rows 20/23/104 assert concrete codes (1, 8, 9) at payload[8] without an NC-EXT-18 qualifier. The test itself is correct (symbolic constants). | `docs/interfaces/ICD.md:123-124` and §4 S6 row; TS-1 rows 20/23/104; `src/pus/pus_types.h:39-45` | Add the provisional/NC-EXT-18 qualifier to the failure-code expectations in TS-1 rows 20/21/22/23/36/104 so the oracle's authority basis is explicit. Non-blocking. |
| VV-R2-03 | **Minor** | Verification payload layout vs ICD | The ICD documents the 13-octet TM secondary header (§3) — hence payload start at absolute 19 — but does not document the 9-octet S1 verification payload order (6-octet Request ID including destination and sequence count; service/subservice/code at payload[6..8]). The helper's internal offsets are therefore implementation-derived (`src/pus/pus.c:332-353`), with an ECSS-E-ST-70-41C §5.4.11.2.1(c) citation present only in a source comment; the TS-1 rows cite "ICD §4", which states only field *content* in prose. | `docs/interfaces/ICD.md` §3/§4 (S1 rows); `src/pus/pus.c:330-353`; `tests/unit/test_pus.c:676-679` | Record the S1 verification payload layout in `ICD.md` §4 (interface authority; NC-EXT-15 open) and/or cite ECSS-E-ST-70-41C §5.4.11.2.1(c) in the TS-1 oracle column. Non-blocking — the change already corrected a real error (payload[4] → payload[8]). |
| VV-R2-04 | **Minor** | TS-1 rev 1.11 row `N22-TS1-104` / §3 gap list wording | The row and gap list state "No oracle is asserted here", but `tests/unit/test_pus.c:748-749` and `:791-792` assert `FF_ERROR`/`FF_NOT_FOUND` and `g_rx_count == 0u` as pass/fail checks. Probe P3 (§4) proves these assertions lock the ack-gated behaviour that conflicts with SRS-REQ-005. When NC-EXT-04 is resolved, the test fails and the row must be re-dispositioned. | TS-1 rows 22/104 and §3 list; `tests/unit/test_pus.c:741-749`, `:782-792`; §4 probe P3 | Reword to "no requirement-derived oracle; the script pins the current behaviour as a characterization check", and add an `NC-EXT-04` comment at both assertion sites so the future fixer updates script and row together. Non-blocking. |
| VV-R2-05 | **Comment** | "Exactly one TM(1,2)" | Sub-case 1 asserts `stream_count(1u,2u) == 1u` plus the absence of (1,1) and (17,2), but not the total packet count; an implementation that additionally emitted a TM(1,8) after the acceptance failure would still pass. The TS-1 row does not require "no (1,8)", so the oracle matches the controlled row. | `tests/unit/test_pus.c:732-739`; TS-1 row 23 | Consider adding `stream_count(1u,8u) == 0u` (or a total-packet assertion) to lock "an acceptance failure is not followed by a completion failure" (PUS-C 5.4.11). |
| VV-R2-06 | **Comment** | Robustness of direct octet reads | `g_rx[19..22]` (lines 734-737) are read without a length precondition. Because `FF_TEST_ASSERT` is non-aborting, a failure of the preceding helper assertion still evaluates these reads against stale/zero buffer content (in-bounds in the 1024-octet `g_rx`, so no undefined behaviour) and would add misleading secondary failures. | `tests/unit/test_pus.c:731-737`; `tests/ff_test.h:17-24`; `g_rx[8*128]` at `test_pus.c:21` | Assert `g_rx_count >= 28u` before indexing, or fold the Request-ID check into `stream_has_verification`. |
| VV-R2-07 | **Comment (pre-existing, out of scope)** | `FF_PusSendVerification` Request-ID sequence counter | `s_sequence_count` (`src/pus/pus.c:332`) is a function-local static incremented **outside** `g_pus_mutex` (every caller unlocks before calling; e.g. `pus.c:421-424`) and is **not** reset by `FF_PusInit`. (a) In a multi-task configuration the increment is unsynchronized (data race on a value placed on the wire); (b) any future assertion on payload[4..5] would be order-dependent. The new test correctly avoids those octets. | `src/pus/pus.c:298-353`, `:421-424`; `tests/unit/test_pus.c:733-737` | Raise an NC under N21 for the concurrency aspect (production change required); keep the test's avoidance of payload[4..5] documented. Not introduced by this change. |
| VV-R2-08 | **Comment** | Evidence scope of "CTest 57/57" | NC-EXT-240's factual basis is confirmed: `test_pus_s11_residual` is declared in `FF_TEST_TARGETS` (`tests/CMakeLists.txt:110`) but never registered with `add_test`; direct invocation yields `checks=1164 failures=10`, and the failures are stale oracles (S6 dump requested on subservice 5 while the registered handler is `{6,2}` at `src/pus/pus_services.c:702`). Therefore 57/57 covers registered tests only. | §5; `tests/CMakeLists.txt:99,110,211-245`; `src/pus/pus_services.c:702` | No action in this commit (NC-EXT-240 is already recorded, owner FSW, milestone M17). It blocks any claim that the CTest count represents every declared test target. |

Positive confirmations (no finding): requirement-derived wire oracles with explicit expected results (N22 satisfied for nominal/out-of-bound); symbolic failure-code constants instead of literals; the `stream_has` bounds fix removes a latent false-positive path; the TS-1 recount is arithmetically consistent with no duplicate IDs; the T3 row hashes are accurate at the current bytes; the author's mutation log is reproducible byte-for-byte.

## 9. Uncertainty and limitations

- **Facts** above are backed by the cited file:line, by reviewer-executed builds/runs in `build/vv-r2-review-20261112`, and by the hashes quoted in §1, §4 and §7.
- **Assumptions**: the captured stream is always a concatenation of whole packets produced by `tm_sink` (`test_pus.c:146-151`), which is what makes the helper's stride parsing and its `step >= 28` guard sound; the test asserts on the *open* backend (`FF_PusBackendOpen`) only, not the N7/ASN.1 backend.
- **Model/measurement uncertainty**: none applicable (no simulation, no measurement); the only "uncertainty" is oracle provenance (VV-R2-01..03).
- **Untested state space**: host-only evidence; no target timing, compiler, endianness, RTEMS, Renode or HIL behaviour is claimed, and none is needed for this backend-independent path. Multi-task concurrency of `FF_PusIngest`/`FF_PusSendVerification` is not exercised (VV-R2-07).
- **Conflicting evidence**: none found; the author's mutation log, the TS-1 recount, the T3 hashes and the CTest result all reproduce.
- **Confidence**: **High** for the oracle/implementation conformance, determinism and discrimination conclusions (direct code reading plus independent mutation execution); **Medium-High** for the bookkeeping conclusions (documentary, recount-based, no authority interpretation).

## 10. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — four Minor findings (VV-R2-01..04) and four Comments (VV-R2-05..08); no Major finding
- [ ] REJECTED

**Blocking assessment:** **No finding blocks commit/push from the SW V&V (R2) perspective.** The change does not alter production code, the oracles match the implementation and the TS-1 rows, the test is deterministic and order-independent, its assertions are discriminating (independently mutated), the full registered CTest suite passes, and the case bookkeeping is not inflated. The four Minor findings are documentation/oracle-provenance items that must be tracked to closure (N12) but do not invalidate the evidence. Commit/push remains subject to N7 authority, the other N9 profiles and PA (N20).

**Recommended disposition before the next related increment:** (1) record the S1 verification payload layout and the `FF_PusIngest` return-code contract in `ICD.md`/`pus.h` (VV-R2-01, VV-R2-03), (2) add the NC-EXT-18 qualifier to the failure-code rows (VV-R2-02), (3) reword the `gap` rows to "no requirement-derived oracle" and comment the NC-EXT-04 assertion sites (VV-R2-04), (4) raise the NC for the unsynchronized Request-ID sequence counter (VV-R2-07).

## 11. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
|---|---|---|---|
| R2 (SW V&V, N9/N10) | Senior Aeronautics and Space SW V&V Engineer (independent context) | 2026-11-12 | APPROVED WITH COMMENTS (VV-R2-01..04 Minor, VV-R2-05..08 Comment); not an approval or qualification |
| PA (N20) | pending | — | — |
| FSW engineer (N7 commit authority) | pending | — | — |

## 12. Follow-up records

| NC | Severity | Item / requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
|---|---|---|---|---|---|---|---|---|
| VV-R2-01 | Minor | TS-1 `N22-TS1-23` oracle provenance | `FF_PusIngest` return-code expectation not stated in any controlled source | Document contract in `pus.h`/`ICD.md` or mark as characterization | — | FSW / interface authority | open | pending |
| VV-R2-02 | Minor | Failure-code allocation (NC-EXT-18) | Concrete code values asserted without the NC-EXT-18 qualifier | Add provisional qualifier to TS-1 rows | — | FSW / interface authority | open | pending |
| VV-R2-03 | Minor | ICD §4 S1 payload layout | 9-octet verification payload order not documented in the ICD | Record layout in ICD §4 and/or cite ECSS-E-ST-70-41C §5.4.11.2.1(c) in TS-1 | — | Interface authority / FSW | open | pending |
| VV-R2-04 | Minor | TS-1 rows 22/104 wording | "No oracle is asserted" contradicts the assertions at `test_pus.c:748-749,791-792` | Reword + comment the NC-EXT-04 sites | — | FSW (doc owner) | open | pending |
| VV-R2-05 | Comment | Sub-case 1 completeness | Total packet count / absence of TM(1,8) not asserted | Optional strengthening | — | FSW | open (advisory) | — |
| VV-R2-06 | Comment | Octet-read robustness | `g_rx[19..22]` read without length precondition | Optional guard | — | FSW | open (advisory) | — |
| VV-R2-07 | Comment | `s_sequence_count` concurrency | Unsynchronized static, not reset by `FF_PusInit` (pre-existing `src/`) | Raise NC under N21 | — | FSW | open | — |
| VV-R2-08 | Comment | CTest evidence scope | `test_pus_s11_residual` declared but unregistered (NC-EXT-240) | Already recorded; no action here | §5 reproduction | FSW (NC-EXT-240) | open | — |

## 13. Evidence index (reviewer-executed)

| ID | Command / artifact | Result |
|---|---|---|
| E-1 | `cmake -S . -B build/vv-r2-review-20261112 -G Ninja -DCMAKE_BUILD_TYPE=Debug` | exit 0 |
| E-2 | `ninja -C build/vv-r2-review-20261112` | exit 0, 0 warnings |
| E-3 | `ctest --output-on-failure` in `build/vv-r2-review-20261112` | 57/57 passed, 0 failed |
| E-4 | `./build/vv-r2-review-20261112/tests/test_pus` (×3) | `checks=320 failures=0` |
| E-5 | HEAD `tests/unit/test_pus.c` (`git show HEAD:…`) compiled against the same libraries | `checks=290 failures=0` (+30) |
| E-6 | New test alone / moved last (reviewer copies under `/tmp/opencode/iso-tests/`) | `30/0` and `320/0` |
| E-7 | Author mutations M1/M2 re-executed | `1df235d2…` → 1 failure @731; `cd2b01fa…` → 2 failures @779-780 |
| E-8 | Reviewer probes P1/P2/P3 | `c19710e8…` → @737; `19f7cc03…` → @731/@765/@779; `d80c2559…` → @792 |
| E-9 | `sha256sum src/pus/pus.c` after restore + `git status --short src/` | `1eea40c0…e15401`; clean |
| E-10 | `sha256sum tests/unit/test_pus.c` | `1e9c86e8…c264` (matches T3 rev 1.8) |
| E-11 | TS-1 case-table recount (script over `docs/vv/test-spec/ts-1-protocol-core.md`) | 104 rows, 77 covered, 27 gap, 51 OOB, 93 individual, 101 SIL |
| E-12 | `./build/…/tests/test_pus_s11_residual` | `checks=1164 failures=10` (NC-EXT-240 basis) |
| E-13 | `clang-format --dry-run --Werror tests/unit/test_pus.c` | exit 0 |
