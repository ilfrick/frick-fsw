# S1PF-R3-20261114 — Avionics Systems Review (R3 profile)

| Field | Value |
| ----- | ----- |
| Repository | `/home/nicola/fsw/frick-fsw` |
| Committed HEAD | `fc68b9d121bce2194c1beaaa49a7dc4cdab0552c` |
| Target | Microchip RT PolarFire SoC RTPFS160ZT (RISC-V) under RTEMS 6.2 |
| Criticality target | ECSS class B |
| Reviewer profile | Senior Aeronautics and Space Avionics Systems Engineer (R3), independent review (norm N9/N14) |
| Review date | 2026-11-14 |
| Document id | S1PF-R3-20261114 (this report only; not a controlled product) |
| Change under review | Uncommitted working-tree delta: PUS-C Service 1 progress-failure emission (NC-EXT-94) |
| Review state | **CONDITIONAL — NOT APPROVED.** See verdict. |

## Candidate identity (binding hashes verified before analysis)

All nine files were hashed with `sha256sum` before any analysis and all match the
review request exactly:

| File | sha256 (verified) |
| ---- | ----------------- |
| `src/pus/pus.c` | `195b8cf5754e7b4043f8b62bcc0c47d06ff2e10e67cdc2ef2ad054dd8c22293b` |
| `tests/unit/test_pus_s11_residual.c` | `33b43b961616b3b5a91925c75982054e07bda8b9b23fd57707ae9a30f3b42a13` |
| `CHANGELOG.md` | `a35e9ea4dead23f3bc29714e62c719699fb6b1df1756ec780d1c7d670de8a4d7` |
| `README.md` | `5b119888eb176e10747459021b54bd5605fc1adf44f8d689bf018e1caa5de9b6` |
| `docs/planning/sprint-plan.md` | `27a42bd3568a605fedc6b2e8bbad74782ee9a0b5a273b4bcc97ddc577a29985d` |
| `docs/status/handover.md` | `ac374ec65e39069cfba39b9f5db0844e0bf507e716c457d9be837e1f4767cc11` |
| `docs/status/loop-audit.md` | `c36a9e645c8795ed008010fc98d1f5d5e16af83dc501922a0934f7cb8d4b2b41` |
| `docs/vv/test-spec/ts-1-protocol-core.md` | `7ceaa4b5bf403fa72fb3b25cd1fa83e6e1a59f0e2cdfcf7fcd915f649bacce95` |
| `docs/vv/verification-results.md` | `8ab0fab7f48b5ded9ffd065bc6124e3f963200d548877ae1e7be6edc010450bd` |

Committed HEAD `fc68b9d` matches the request. One untracked evidence directory is
present: `docs/vv/evidence/2026-11-14-s1-progress-failure/` (14 files).

## Independence caveat and authority limits

This review provides **contextual independence only**: a separate session with no
part in authoring the delta, but the same tooling, the same repository and the
same underlying model family. It is **not** organisational, contractual or
accreditation independence, and it does not substitute for the independent
reviews that norms N9/N17/N20 require.

I do **not** approve, and this report must not be cited as approval of: the ICD
interface contract, the PUS mission profile, requirement baselines, failure-code
allocations, coverage/qualification claims, NC closure, risk acceptance,
airworthiness, human-rating, mission acceptance, release or flight readiness.
Every disposition below is a **recommendation to the responsible authority**.

Nothing was modified, committed, pushed, deleted or containerised during this
review. Two read-only verifications were performed that are worth declaring:
the retained test binary `build/doc-update/tests/test_pus_s11_residual` was
executed (no build, no writes) and its objects predate no source file in the
delta (objects 12:22, sources 12:21:53), so the binary is built from the
candidate; and the retained evidence logs were read.

## Scope

Systems/interface level review of the delta, against the six questions put to
this review:

1. Consistency of the implemented Service 1 subservice set with ICD §4 rev 1.9
   and with PUS-C / ECSS-E-ST-70-41C service 1 semantics.
2. Whether the change is a ground-segment-visible contract change requiring
   ICD/interface-authority approval, and whether NC-EXT-94 is correctly left
   OPEN.
3. Whether the absence of TM[1,4] is documented without contradiction.
4. Unaddressed mission-level consequences (TM budget, telemetry rate,
   FDIR/ground-automation triggers on TM[1,6]).
5. Verbatim preservation of the NC-EXT-94 record; state of NC-EXT-95.
6. NC-EXT-43/44/245/246 mapping consistency.

Out of scope (other profiles): code-level defect hunting, MISRA/style, test
oracle quality at assertion level, documentation prose review (R4), PA
disposition (N20), V&V method adequacy (R2).

## Change as implemented (verified by inspection)

`src/pus/pus.c:455-465` — `rc = handler(&tc, data, data_len)` now precedes the
progress verification; `FF_PusSendVerification(&tc, FF_PUS_STAGE_PROGRESS,
(rc == FF_SUCCESS) ? 1u : 0u, (rc == FF_SUCCESS) ? 0u : FF_PUS_FAIL_EXECUTION)`.
`FF_PusSendVerification` (`pus.c:300-330`) maps `PROGRESS → {5,6}`,
`FF_PUS_FAIL_EXECUTION = 20u` (`src/pus/pus_types.h:46`). The mapping, the
mutex release/re-acquire pattern and the completion block are unchanged.

Emitted Service 1 set, re-derived from the code after the delta:

| Subservice | Emission condition | Emitted? |
| --- | --- | --- |
| 1 | `ack_accept` set, APID accepted | yes |
| 2 | illegal APID and `ack_accept` set | yes |
| 3 | `ack_start` set, handler found | yes |
| 4 | `STAGE_START` with `success = 0` — **no call site** | **no** |
| 5 | `ack_progress` set, handler returns `FF_SUCCESS` | yes |
| 6 | `ack_progress` set, handler returns non-success | yes (new) |
| 7 | `ack_complete` set, handler returns `FF_SUCCESS` | yes |
| 8 | `ack_complete` set, handler non-success **or** handler not found | yes |

## Findings

Severity convention used here: **Major** = a defect or authority item that must
be corrected or formally dispositioned before this delta is committed/accepted
under N7/N11/N12/N15; **Minor** = bounded documentation or record defect to
correct in the same cycle; **Observation** = recorded for the owning authority,
no action required for this delta.

| ID | Severity | Location | Blocks? | Finding |
| --- | --- | --- | --- | --- |
| F-01 | **Major** | `src/pus/pus.c:455-465`; `docs/interfaces/ICD.md:103-109`; `CHANGELOG.md:60-66`; `docs/vv/test-spec/ts-1-protocol-core.md:147` | Yes | **Undocumented, untested, ground-visible telemetry ordering change.** Moving the progress report after `handler(...)` changes the on-wire order for every acknowledged TC whose handler emits telemetry. `TC[17,1]` is the clearest case: `svc17_ping` (`src/pus/pus_services.c:628-631`) sends TM[17,2] from inside the handler. Before: `1,1 → 1,3 → 1,5 → 17,2 → 1,7`. After: `1,1 → 1,3 → 17,2 → 1,5 → 1,7`. The response TM now precedes the progress verification, inverting the conventional verification-then-response sequence and re-ordering the CCSDS TM counter for a ground segment that correlates verification to response. No document states this; every delta record says only "acceptance/start/completion behaviour unchanged", which is true but silent on the ordering. No test asserts inter-packet ordering (`stream_count`/`stream_code_byte` in `tests/unit/test_pus_s11_residual.c:41-80` count only). **Recommendation:** state the ordering explicitly in ICD §4 (or in the S1 note) and add an ordering case to TS-1 for a handler that emits a response TM; if the previous ordering was intended, emit the progress report before the handler and take the failure verdict afterwards, or accept the inversion as an authority decision. |
| F-02 | **Major** | `docs/interfaces/ICD.md:107-109`; `src/pus/pus.c:457-472`; `tests/unit/test_pus_s11_residual.c:410-418` | Yes | **ICD does not state whether a handler failure yields TM[1,6] alone or TM[1,6] + TM[1,8]; the implementation emits both, with identical code 20.** With `ack_progress` and `ack_complete` both set, one failed TC now produces two failure verifications carrying the same `FF_PUS_FAIL_EXECUTION`. Consequences at system level: (a) ground automation that counts S1 failure TMs will double-count a single failed TC; (b) the two messages are informationally identical for a synchronous handler, so 1,6 adds no discriminating content over 1,8; (c) for handlers that partially succeed — e.g. `TC[3,4]` processes a list and returns the first error while still serving the remaining ids (`ICD.md:114`) — TM[1,6] "progress failure" is emitted for a request that partially executed, which is semantically misleading; (d) TM budget grows by one TM per failed acknowledged TC. The new test **pins** the co-occurrence as an oracle without any controlled statement authorising it. **Recommendation:** interface authority must decide and record (i) whether 1,6 and 1,8 may co-occur, (ii) whether a partially-successful handler may be reported as a progress failure, and (iii) which single code each message carries. Until then the test encodes an implementation choice, not a contract. |
| F-03 | **Major** | `docs/interfaces/ICD.md:7` vs `docs/interfaces/ICD.md:255` | Yes | **The controlled interface document misidentifies its own revision.** The header table declares `Revision 1.8 / Date 2026-11-13`; the revision history contains `1.9 / 2026-11-14` (NC-EXT-94/95). Five records in this delta cite "ICD rev 1.9" (`CHANGELOG.md:58`, `docs/vv/verification-results.md:1029`, `docs/status/handover.md:53`, `docs/planning/sprint-plan.md:739`, `docs/status/loop-audit.md:449`). Because this delta's entire justification is "ICD §4 rev 1.9 describes this emission", the contract basis is identified by a revision number the document itself denies. N8/N16 defect in an authority-facing artefact. **Recommendation:** advance the ICD header to 1.9/2026-11-14 in the same cycle, and add a header/history consistency check to the documentation gate so this class cannot recur. |
| F-04 | **Major** | `docs/architecture/pus-c-from-scratch-assessment.md:7` vs `:284` | Yes | **Same defect in the second cited baseline.** Header declares `Revision 0.3 / Date 2026-10-19`; history contains `0.4 / 2026-11-14`. `CHANGELOG.md:59-60` and `docs/vv/verification-results.md:1030` cite "rev 0.4". The S1 row at `:39` (`partial (1,2,3,5,6,7,8)`) is correct and consistent with the implementation; only the identification is wrong. **Recommendation:** advance to 0.4. |
| F-05 | **Major** | `docs/vv/verification-results.md:1050`; `docs/status/loop-audit.md:795` | Yes | **Reported gate figure is stale and contradicted by the retained artefact.** Both records state `test_pus_s11_residual` = "checks=1176, failures=0" for this cycle. The retained binary built from this candidate reports **`checks=1206 failures=0`** (executed read-only during this review; objects 12:22, sources 12:21:53). 1176 is verifiably the *pre-delta* figure (`docs/vv/evidence/2026-11-13-icd-s3-s6-contract/pus-s11-residual.log`). The delta adds 28 static assertions (122 → 150 `FF_TEST_ASSERT` in the file), so an unchanged total is impossible. N11 evidence-accuracy defect: the number was carried over rather than re-derived. **Recommendation:** correct both records to 1206/0 and retain the harness output line in the evidence directory so the figure is re-derivable from the retained artefact. |
| F-06 | **Major** | `docs/vv/test-spec/ts-1-protocol-core.md:90, 147, 291-301, 349`; `docs/status/loop-audit.md:458`; `docs/vv/verification-results.md:993` | Yes | **TS-1 coverage ledger is internally inconsistent and §3.1 was not updated.** §2 extends the range to `N22-TS1-105`, row 147 adds `N22-TS1-105` with status token `host covered (…)`, and three records state "105 cases = 76 covered + 29 gap". §3.1 still tabulates Total 104 / `covered` 76 / `gap` 28 / mixed 5 / individual 93 / SIL 101-1-2. By the document's own counting convention (§3.1, "a status of the form `host covered (…)` is counted as `covered`"), the correct row is **105 total / 77 covered / 28 gap / mixed 6 / individual 94 / SIL 102-1-2**. As written, the revision record, the loop-audit N22 row and the results record all assert a split that contradicts the case row they describe, and the summary table contradicts all of them. This is the same class as NC-EXT-92 (TS-4) and is an objective artefact defect, so under the project NC-triage rule it blocks the cycle rather than being deferred. **Recommendation:** update §3.1 and correct the "76 covered + 29 gap" statements to "77 covered + 28 gap" (or reclassify the row as `gap` and state why), in all four places. |
| F-07 | **Major** | `docs/vv/evidence/2026-11-14-s1-progress-failure/renode-process.log`; `docs/status/loop-audit.md:454`; `docs/planning/sprint-plan.md:333` | Yes (gate) | **N18 is not satisfied for this delta and cannot currently be satisfied.** N18 is in force and requires functional validation of *every change* in the Renode environment. The retained Renode run is timestamped 11:26 on 2026-11-14 — **before** the delta's source edit at 12:21:53 — and loads the pre-existing 520464-byte ELF; no RTEMS rebuild exists in the evidence directory. The records honestly say "PASS retained from prior run", but that PASS cannot cover the new TM[1,6] behaviour. Independently, NC-EXT-246 declares Docker runner use blocked "until a preflight/cleanup procedure is approved", which is the mechanism N18 needs. The delta therefore cannot meet the N7 commit gate as it stands. **Recommendation:** obtain the approved host-safe preflight/cleanup procedure (NC-EXT-246) and a fresh RTEMS image + Renode run against this delta, **or** obtain an explicit owner waiver recorded against N18 for this change. Neither is a decision I may take. |
| F-08 | **Major** | `docs/requirements/SRS.md:65-67`; `src/pus/pus.c:437-450`; `docs/vv/test-spec/ts-1-protocol-core.md:147` | Yes | **SRS-REQ-006 is not satisfied on the no-handler path, and this delta's "covered" claim for SRS-REQ-006 does not disclose it.** SRS-REQ-006 requires that "every acknowledged stage (acceptance/start/progress/completion) of an accepted telecommand shall produce exactly one service 1 telemetry packet". For an accepted TC whose service/subservice is not registered, the handler lookup precedes the start report: with `ack_start` and/or `ack_progress` set, **zero** packets are produced for those acknowledged stages (only TM[1,8] if `ack_complete` is set, and complete silence if only `ack_progress` is set). Pre-existing, not introduced here, but this delta is the S1 emission change and marks a new SRS-REQ-006 case `covered` without noting the residual non-conformance. Note also that the Lean model `req_srs006_acceptance_completion` (`verification/lean/Frick/Requirements.lean:58`) only covers acceptance+completion, so no Lean result contradicts or supports the progress-stage claim. **Recommendation:** register the gap (extend NC-EXT-04 or open a new NC), state it in the `N22-TS1-105` status cell, and do not let the `covered` token imply full SRS-REQ-006 conformance. |
| F-09 | Minor | `docs/planning/sprint-plan.md:233` | No | **NC-EXT-95's historical finding text was overwritten, not preserved.** The description cell was rewritten from "`FRICK-ICD-001` §4 Service 20 row lists only the parameter report (TM) side although …" to "… now lists both the parameter report (TM) side and the TC[20,1] set side (ICD rev 1.9, line 123) …", so the original non-conformance statement no longer exists in the register (first divergence at character 161 of the row). The NC-EXT-94 row in the same commit does the right thing: its text is byte-identical through character 1839 and only the status cell is appended to. NC-EXT-95 remains correctly **OPEN** and its new content is factually accurate (verified: `ICD.md:123-124`, `pus_services.c:635`, `:834`, `tests/unit/test_pus.c::test_param_set_bounded`), but the record of what was found is lost from the register. **Recommendation:** restore the original description text and move the new material into the status cell, matching the NC-EXT-94 treatment. |
| F-10 | Minor | `README.md:184` | No | **README cites an ICD row label that no longer exists.** The edited sentence still says the `FRICK-ICD-001` §4 row is `"4 / 6 start / progress failure"`; ICD rev 1.9 split that row into `1  | 4 | start failure (reserved, not emitted)` (`ICD.md:106`) and `1  | 6 | progress failure` (`ICD.md:107`). The sentence was touched by this delta and left pointing at a superseded label. **Recommendation:** refer to the two current rows. |
| F-11 | Minor | `docs/interfaces/ICD.md:96` vs `:106` | No | **ICD §4 lead-in still contradicts its own table.** The lead-in states "Rows below are the services/subservices this build **answers on the wire**", while row 106 is explicitly "reserved, not emitted". This is precisely the defect raised earlier as F-0381-03 / F-0382-04 against the same line, and ICD rev 1.9 did not fix it. A ground-segment reader takes the lead-in as the definition of the contract. **Recommendation:** reword the lead-in to "services/subservices this build answers, or reserves for answer" (or similar), or move reserved rows out of the answered table. |
| F-12 | Minor | `docs/interfaces/ICD.md:103-109` | No | **The S1 rows omit the ack-flag gating that the S6 row documents.** TM[1,6] is emitted only when the progress-ack flag is set in the TC; TM[1,8] only when the completion-ack flag is set. The S6 row already spells out "when the completion ack flag is set" (`ICD.md:118`), so the S1 rows are now less precise than the document's own convention, and the new 1,6 row is the first S1 row where a reader could reasonably assume unconditional emission. **Recommendation:** add an "emitted when …" qualifier to the S1 rows. |
| F-13 | Minor | `docs/interfaces/ICD.md:107`; `src/pus/pus.c:462-463` | No | **Handler return code is collapsed to `FF_PUS_FAIL_EXECUTION` (20) with no cause information, and the ICD does not say so.** `FF_BAD_SIZE` (2), `FF_NOT_FOUND` (3), `FF_ERROR` (1) and every other handler result reach the wire as 20 on both TM[1,6] and TM[1,8], while `FF_PUS_FAIL_ILLEGAL_LENGTH` (2) and `FF_PUS_FAIL_BAD_DATA` (12) exist unused (`src/pus/pus_types.h:40-46`). The S6 row documents exactly this non-distinguishability as an explicit authority item (NC-EXT-18); the S1 rows do not, so a ground segment may design cause-specific logic that can never work. **Recommendation:** state the S1 non-distinguishability in §4 and cross-reference the code-allocation item (NC-EXT-14/15/17/18) rather than leaving it implicit. |
| F-14 | Minor | `docs/vv/verification-results.md:1002-1005`; `docs/vv/evidence/2026-11-14-s1-progress-failure/format-full.log` | No | **Format-gate narrative does not match its own retained log.** The record says the full pinned check fails on "three *unchanged* initializer files" and reports "`make format-check` → exit 123". The retained log shows **9 files / 39 violations** (`ff_obcp_inhouse.c` 8, `test_pus_s18.c` 11, `test_obcp_budget.c` 5, `test_obcp_inhouse.c` 3, `test_link_tsn.c` 3, `link_tsn.c` 3, `link_ip.c` 3, `test_obcp_abort.c` 2, `pus_services.c` 1) and records `FORMAT_FULL_EXIT=2` (123 is the `xargs`/`clang-format` status, not `make`'s). The log also records no formatter version, so the "pinned clang-format 18" attribution is unevidenced, while the changed-file PASS log contains only `FORMAT_CHANGED_EXIT=0`. **Recommendation:** correct the count/exit-code wording, record the formatter version, and either extend NC-EXT-244's scope to the nine files or state that the list is version-dependent. |
| F-15 | Minor | `docs/vv/test-spec/ts-1-protocol-core.md:147` | No | **The `N22-TS1-105` status cell lacks the provisional qualifiers its siblings carry.** The oracle asserts subservice 6 (grounded in ICD §4, which ADR-0020 declares a provisional framework default) and code value 20 (a provisional allocation). Comparable rows carry the caveat explicitly — `N22-TS1-36` says "code 20 (`FF_PUS_FAIL_EXECUTION`; provisional allocation, NC-EXT-18)". Row 147 says only "host covered (NC-EXT-94 implementation; ICD §4 S1 subservice 6; …)". **Recommendation:** add the ADR-0020/NC-EXT-18 qualifiers so the coverage claim is not read as an approved-contract oracle. |
| F-16 | Minor | `docs/interfaces/ICD.md:255` | No | **ICD rev 1.9's history entry overstates the wire change.** It says "Wire behaviour changed: TM[1,6] is now emitted …; TC[20,1] is now accepted on the wire". `TC[20,1]` was already accepted before `fc68b9d` (`svc20_set` registered at `pus_services.c:834` in `fc68b9d^`), so no S20 wire change occurred; and TM[1,6] was not emitted at `fc68b9d` either — the code change is the uncommitted delta. The committed tree `fc68b9d` therefore carried a contract revision whose history asserted behaviour its own build did not implement, and the assertion becomes true only when this delta lands. **Recommendation:** state "documented ahead of implementation; implemented in the same release" and, going forward, land interface-contract revisions and the code change in one commit. |
| F-17 | Minor | `docs/vv/verification-results.md:1038, 1046` | No | **Two bullets promise "Full log retained below"; no logs follow** — the section ends after the evidence table, and the sentence at `:1036` ends in a colon immediately before a new heading. The logs do exist in the evidence directory, so this is a presentation defect, not missing evidence. **Recommendation:** inline or link them. |
| O-01 | Observation | `tests/unit/test_pus.c:603-661` | No | No test asserts that the **ingest path** never emits TM[1,4]. `test_verification_missing_subservices` asserts the opposite at API level — that `FF_PusSendVerification(STAGE_START, success=0)` succeeds and emits subservice 4 — and `FF_PusSendVerification` is exported (`src/pus/pus.h:76`) with no production caller outside `pus.c`. So "reserved, not emitted" is true for this build's ingest path only; a future caller of the public API could put TM[1,4] on the wire. The ICD wording at `:106` would then be wrong. Recording for the interface authority; a negative ingest-path assertion would make the claim testable. |
| O-02 | Observation | `tools/gsw/ff_gsw.py:68-100` | No | The ground segment has no Service 1 semantics at all: `decode_tm` returns a generic `(service, subservice, data)` tuple with no allow-list, no subservice naming, no verification/response correlation and no FDIR/event logic. So TM[1,6] cannot break the GSW, but equally there is **no documented operator response, no FDIR trigger and no alarm mapping** for a progress-failure TM. Ground-segment readiness for the new message is unaddressed by design, not by defect. |
| O-03 | Observation | — | No | No telemetry budget or TM-rate model exists that the extra failure verification could be charged against, so the "one extra TM per failed acknowledged TC" cost (F-02) cannot be quantified against an approved allocation. Recording as a mission-level input the interface authority should request. |
| O-04 | Observation | `docs/planning/sprint-plan.md:180, 232-233, 332-333`; `docs/status/loop-audit.md:449, 454-455, 483`; `docs/status/handover.md:25, 69-70`; `docs/vv/verification-results.md:1006-1007`; `CHANGELOG.md:85-86` | No | **NC mapping verified consistent.** NC-EXT-94: historical text byte-identical to HEAD through character 1839/1840, status appended only — confirmed by string comparison against `git show HEAD:docs/planning/sprint-plan.md`. NC-EXT-43: appended only, verbatim prefix preserved. NC-EXT-44: unchanged. NC-EXT-245/246: free at HEAD, one register entry each, no duplicate IDs in the register (the repeated `NC-EXT-03/06/07/29/43/47` tokens are a separate disposition-policy summary table, not duplicate NCs). The superseded meaning of NC-EXT-246 (licensing) survives only in the explicitly superseded loop-audit history row 11.06, which is correct practice. No misattribution found. |

## Answers to the six review questions

**1. Is the emitted Service 1 set consistent with ICD §4 and with PUS-C service 1 semantics?**

The *set* is consistent. Implemented `{1,2,3,5,6,7,8}` with `4` unemitted matches
ICD `:103-109` (rows 1, 2, 3/5, 4 reserved, 6, 7, 8), `README.md:184`
("1,2,3,5,6,7,8 … start failure (4) … no command path emits it") and
`docs/architecture/pus-c-from-scratch-assessment.md:39`
("partial (1,2,3,5,6,7,8)"). The subservice numbering matches
ECSS-E-ST-70-41C service 1 (1/2 acceptance, 3/4 start, 5/6 progress,
7/8 completion). **No stale `{1,2,3,5,7,8}` claim survives** outside historical
records (verified by repository-wide search).

Consistency of the *set* is not consistency of the *contract*. Four defects
remain at the row/lead-in level: the lead-in still calls every row "answered on
the wire" while one row is reserved (F-11); the rows omit ack-flag gating
(F-12); the failure-code contract is undocumented and lossy (F-13); and the
1,6/1,8 relationship is undefined (F-02). A PUS-C "progress" report is
defined as an intermediate report during execution; with a synchronous handler
it is emitted immediately before completion, so 1,6 carries no information that
1,8 does not, and for partially-successful handlers it mislabels a partial
execution as a progress failure.

**2. Ground-visible contract change needing approval? Is NC-EXT-94 correctly OPEN?**

Yes, on both counts, with a caveat. TM[1,6] is a **new packet type on the
wire** for a TC failure that previously produced only TM[1,5] + TM[1,8]. That
is a ground-segment-visible contract change; it also re-orders telemetry
(F-01) and duplicates failure signalling (F-02). It requires
interface-authority disposition and, on the ground side, an agreed operator
response (O-02).

NC-EXT-94 is correctly **left OPEN** and correctly **not closed by
implementation or by tests**: `sprint-plan.md:232` preserves the original
finding verbatim, appends implementation status, and states "REMAINS OPEN
pending interface-authority disposition … no PUS-C compliance claim; NC not
closed by implementation/tests". `CHANGELOG.md`, `handover.md:53-56`,
`verification-results.md:1028-1032` and `ts-1:147` all repeat that boundary.
That is the correct treatment. The caveat is that the delta nonetheless
*implements* one of the two dispositions the NC offered (implement **or**
annotate as reserved) while the authority item that the implementation creates
(1,6 vs 1,8 semantics, ordering, code mapping) is not itself registered as an
open item — it is only implicit in NC-EXT-94's continued OPEN state.
**Recommendation:** the interface authority should be asked to disposition
F-01/F-02/F-11/F-12/F-13 explicitly; continuing to cite NC-EXT-94 is not
sufficient once the code path exists.

**3. Is TM[1,4] absence documented without contradiction?**

Substantially yes, with two precision gaps. `ICD.md:106` says "start failure
(reserved, not emitted)"; `README.md:184` says the map "also defines start
failure (4), but no command path emits it"; the assessment doc lists the
implemented set without 4; `CHANGELOG.md:63-64` says "no start-failure
behaviour invented"; `sprint-plan.md:232` re-derives "TM[1,4] has no emission
path". No contradiction among these.

Two gaps: the ICD lead-in contradicts the row it heads (F-11), and the
"not emitted" claim is true only of the ingest path — the exported
`FF_PusSendVerification` can produce TM[1,4] and `tests/unit/test_pus.c:614-620`
asserts that it does (O-01). No test asserts the negative for the ingest path,
so the claim is undocumented-behaviour-verified rather than test-verified.

**4. Unaddressed mission-level consequences?**

Yes, three, all recorded above: **telemetry ordering** inversion relative to
handler-emitted response TM (F-01, Major); **telemetry volume and double-counting**
— one additional TM per failed acknowledged TC, two failure TMs with identical
content for one failure, and no TM budget to charge it against (F-02, O-03);
and **FDIR / ground automation** — no trigger, alarm mapping, operator response
or GSW semantics exists for TM[1,6], and the GSW is a generic decoder (O-02).
The "progress failure" label is also semantically weak for handlers that
partially succeed (F-02c). None of these is addressed anywhere in the delta's
documentation, which discusses only "exactly one 1,6, never both 1,5 and 1,6".

**5. NC-EXT-94 verbatim? NC-EXT-95 state?**

NC-EXT-94: **verified verbatim** — programmatic comparison against
`git show HEAD:docs/planning/sprint-plan.md` shows the row identical through
character 1839 of 1840 (the final character being the row delimiter), with the
new text appended inside the status cell only.

NC-EXT-95: **remains OPEN** and its new content is factually accurate, but it is
**not untouched in substance**: the description cell was rewritten, discarding
the original finding statement (F-09). This is inconsistent with the
NC-EXT-94 treatment in the same commit and weakens the NC register as a record.

**6. NC-EXT-43/44/245/246 mapping?**

Consistent across `sprint-plan.md`, `loop-audit.md`, `handover.md`,
`verification-results.md` and `CHANGELOG.md`: NC-EXT-246 = N18 Docker runner
preflight/cleanup deviation; the `vm.c` SPDX finding = dated observation under
the existing NC-EXT-43, cross-referenced to NC-EXT-44; NC-EXT-245 = N19 audit
sequencing deviation. No duplicate NC ID, no misattribution, numbers free at
HEAD, and the superseded licensing meaning of NC-EXT-246 survives only in the
explicitly superseded history row 11.06. No finding.

## Verdict

**CONDITIONAL — NOT APPROVED at this revision.**

Positives, stated plainly: the functional defect NC-EXT-94 described is
genuinely corrected. The code change is small, readable and correctly scoped;
`TM[1,6]` is now emitted exactly once on handler failure and never together
with `TM[1,5]`; `TM[1,4]` is still not emitted; acceptance, start, completion
and both early-return paths are untouched; the emitted set now matches the ICD
§4 S1 rows, `README`, and the assessment document; NC-EXT-94 is preserved
verbatim and correctly left OPEN; NC-EXT-43/44/245/246 are cleanly mapped; and
the new test is discriminating (it would fail against the pre-delta code, and
`stream_code_byte` cannot confuse absence with a pass).

Not approvable as it stands, for eight Major items:

- two are **interface-contract defects introduced or exposed by this change**
  and require authority disposition, not a code fix: the telemetry ordering
  inversion (F-01) and the undefined TM[1,6]/TM[1,8] relationship with
  duplicated failure signalling (F-02);
- two are **identification defects in the very baselines this change cites**:
  the ICD declares revision 1.8 while five records cite 1.9 (F-03), and the
  assessment document declares 0.3 while two records cite 0.4 (F-04);
- two are **objective evidence/ledger defects** that the project's own
  NC-triage rule makes blocking: a reported check count of 1176 where the
  retained binary reports 1206 (F-05), and a TS-1 coverage ledger that
  contradicts itself and was not updated (F-06);
- one is a **requirement-conformance gap left undisclosed** while a new
  SRS-REQ-006 case is marked `covered` (F-08);
- one is a **gate**: N18 is in force, the retained Renode PASS predates the
  change by 55 minutes and runs the previous ELF, and NC-EXT-246 currently
  blocks the mechanism needed to fix that (F-07).

F-05, F-06, F-08 and F-09 are objective defects in artefacts and are therefore
not deferrable to an NC under the project's triage rule; F-01 to F-04 and F-07
need an owner/authority action that I am not authorised to take. The remaining
Minor and Observation items are bounded and can be corrected in the same cycle
or recorded.

Recommended sequence for the responsible engineer/authority: correct F-05, F-06,
F-09, F-10, F-14, F-15, F-16, F-17 (bounded edits); advance the ICD and
assessment headers (F-03, F-04); reword the ICD §4 lead-in and S1 rows (F-11,
F-12, F-13); register the SRS-REQ-006 gap (F-08); obtain interface-authority
disposition on F-01/F-02 and record the operator response (O-02); then resolve
F-07 with an approved preflight procedure plus a fresh RTEMS image and Renode
run, or an explicit owner waiver — and only then re-run the full applicable
cycle under N15 and re-submit for independent R3/R4/PA review.

## Residual risks

1. **Ground-segment mis-correlation (F-01).** A response TM that now precedes
   its progress verification may be associated with the wrong request by a
   correlator that assumes verification-first ordering. Not detectable by any
   current test.
2. **Double-counted failures (F-02).** Two failure verifications with identical
   content per failed acknowledged TC; any counter, alarm or FDIR rule keyed on
   "service 1 failure TM" will over-count. Effect on operator workload and on
   any future autonomy/FDIR logic is unquantified.
3. **Misleading progress-failure for partial success (F-02c).** `TC[3,4]`
   partially serving a list while reporting a progress failure could cause a
   ground operator to repeat a request that partly executed.
4. **Undetectable cause on the wire (F-13).** All handler errors map to code
   20; a ground segment cannot distinguish a malformed payload from an
   execution failure, and the ICD does not say so.
5. **Evidence-chain integrity (F-05, F-06, F-14).** Three separate gate figures
   in this delta are contradicted by the delta's own retained artefacts
   (check count, coverage split, format-failure enumeration). Until corrected,
   the cycle's evidence cannot be re-derived from what was retained.
6. **Baseline identification (F-03, F-04).** Two controlled documents
   misidentify their own revision while five records cite the later revision.
   Any future audit that resolves "ICD rev 1.9" by header will find rev 1.8.
7. **N18 coverage (F-07).** No functional validation of this change exists in
   the emulated target environment; the behaviour is host-verified only, and
   the mechanism to close that gap is currently blocked by an open NC.
8. **Requirements conformance (F-08).** SRS-REQ-006 "exactly one packet per
   acknowledged stage" is violated on the no-handler path; the new `covered`
   token does not disclose it.

## Statement of limits

This review is an independent technical assessment produced by an AI assistant
under the stated persona. It is not an approval of any baseline, interface
contract, requirement, coverage claim, gate, NC closure, risk acceptance or
readiness state, and it must not be represented as one. It carries contextual
independence only, not the organisational or accreditation independence that
norms N9/N17/N20 require. All dispositions above are recommendations to the
authorised human/role holders (interface/requirements authority, FSW engineer,
project owner, PA, R4, R2). No file other than this report was created,
modified or deleted during the review.
