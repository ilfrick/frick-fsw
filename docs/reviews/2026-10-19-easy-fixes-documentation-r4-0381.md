# Independent R4 documentation review — "easy-fixes batch" `e7e48ee..318f49b` (2026-10-19)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0381                      |
| Reviewer session                  | Independent R4 documentation review (fresh context, norm N9/N17) |
| Reviewer profile                  | R4 (SW documentation), with source-, build-config- and evidence-level cross-checks |
| Reference revision                | HEAD `7e93325dad78180be152a4fcaf2c616bd7e62e2c` on `main` (= `origin/main` = `gitea/main`); **engineering baseline under review `318f49b`** (batch range `e7e48ee..318f49b`, 27 files); post-baseline status-only commits `b6e1f61`, `7e93325` |
| Items under review                | The 2026-10-19 "easy-fixes batch" documents: `README.md`, `CHANGELOG.md`, `docs/requirements/SRS.md`, `docs/requirements/SSS.md`, `docs/quality/licensing.md`, `docs/architecture/component-gap-analysis.md`, `docs/architecture/pus-c-from-scratch-assessment.md`, `docs/engineering/user-integration-manual.md`, `docs/planning/sprint-plan.md`, `docs/vv/test-spec/ts-5-core-services.md`, `docs/vv/test-spec/ts-6-sw-robustness.md`, `docs/vv/verification-results.md`, `docs/status/handover.md`, `docs/status/loop-audit.md`, `docs/reviews/README.md`, the two carried reports (`FRICK-REVIEW-0378`, `FRICK-REVIEW-0379/0380`), `docs/vv/evidence/2026-10-1{7,9}-*` (8 files), `tests/CMakeLists.txt`, `tests/unit/test_decoder_corpus.c` |
| Date                              | 2026-10-19                             |
| Scope                             | Bounded **N17 documentation review of the changed documents** of the published easy-fixes batch (the deferred review recorded by `NC-EXT-99`): factual accuracy of the claims the batch wrote, currency/consistency of the affected controlled records, evidence-record traceability, header/history discipline, and honest review-coverage statements. Against project norms `docs/rules.md` (N1–N22) and the ECSS-E-ST-40C / ECSS-Q-ST-80C information-item context invoked by `docs/quality/`. |
| Independence declaration          | Fresh review context; no part in authoring, executing or approving the batch, its evidence, or any reviewed file. Read-only against the repository; **this report is the only file this reviewer created or modified.** No NC, gate, phase or release was closed or approved by this reviewer. |
| Norms applied                     | N1–N22 (see `docs/rules.md`), emphasis on N4, N8, N9, N11, N16, N17, N21, N22 |
| Review-ID note                    | The ledger in `docs/reviews/README.md` reaches `0380`; `0381` was free (`grep -rn FRICK-REVIEW-0381 docs/` empty before this report; no ledger row carries `0381`). Two sibling reviews of the same batch are being produced concurrently in the same worktree (`FRICK-REVIEW-0382` R2/R3 V&V, `FRICK-REVIEW-0383` PA — both untracked at the time of writing); nothing here duplicates or pre-empts them. Per the recorded `NC-EXT-99` deferral this report is **not** added to `docs/reviews/README.md` by this reviewer; indexing is the owner/FSW action when the deferred review set is registered. |

## 1. Scope

**In scope.** The 27-file batch `e7e48ee..318f49b` (engineering baseline `318f49b`),
reviewed as the documentation product of that cycle, plus the two status-only
commits `b6e1f61`/`7e93325` insofar as they affect the currency of the status
documents. The batch claims (sprint-plan history 1.72, CHANGELOG, handover 3.09,
loop-audit 10.11, verification-results 2.91): one functional test defect
correction (`tests/unit/test_decoder_corpus.c`, masked minimum-PDL sub-case),
an enforced CTest wall-clock budget on the two S8-VV-FU targets, document/test-spec
accuracy corrections, and the registration of `NC-EXT-97/98/99` with updates to
`NC-EXT-94/96`.

**Method (cheap, read-only, independently re-derived).** `git show`/`git diff`/
`git log` on the batch range and on the status-only commits; line-level reads of
every changed document and its own `## History` table; `sha256sum` recomputation
of all recorded evidence logs; `grep -c add_test tests/CMakeLists.txt` and
`ctest -N` in `build/native`, `build/ci`, `build/asan`; `sed`/`grep` source
cross-checks (`src/pus/pus.c`, `tests/CMakeLists.txt`, `CMakeLists.txt:19-26`);
two Python table re-counts of the TS-5/TS-6 case tables; `python3
tools/scripts/trace.py docs/requirements/traceability.yaml` and `python3
tools/scripts/metrics.py --check` (both read-only; re-run by this session).

**Excluded from scope.** No `make ci-local`/docker/RTEMS/Renode/QEMU/format-check
re-run and no re-execution of the mutant-m3 probe (its retained log was read
instead); no edit to any source, test or configuration file; no disposition of
`NC-EXT-97` (requirements semantics — R3/N14 authority) or of the ICD §4 row
(interface/requirements authority); no hosted-CI re-verification; no target/HIL
legs; no NC closure, N17/N20 phase pass, qualification or release approval.
The existing `build/*` trees were used only to read CTest registration
(`ctest -N`), never as pass/fail evidence.

## 2. References

- Project norms: `docs/rules.md` (N1–N22; N16 interpretation note 3 on per-document History scope)
- Review template: `docs/reviews/review-template.md`
- Traceability manifest: `docs/requirements/traceability.yaml`
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2
- Batch under review: commits `e7e48ee` → `318f49b`; status-only `b6e1f61`, `7e93325`
- Carried reviews: `FRICK-REVIEW-0378` (R4 confirmation), `FRICK-REVIEW-0379/0380` (R2 core + R3 annex)
- NC register: `docs/planning/sprint-plan.md` rev 1.72 (`NC-EXT-94/96/97/98/99`, `GAP-R3-1`)
- Evidence: `docs/vv/evidence/2026-10-19-easy-fixes-{ci-local,format-check,docs,gsw-selfcheck,smoke,asan}.log`,
  `docs/vv/evidence/2026-10-19-s8-vv-fu-mutant-m3-discrimination.log`,
  `docs/vv/evidence/2026-10-17-s8-reconciliation-candidate-manifest.md`

## 3. Findings

### 3.0 Independently re-derived claims — CONFIRMED (no finding)

| Claim in the batch | Independent re-derivation | Result |
| --- | --- | --- |
| Service-1 emitted subservices are `{1,2,3,5,7,8}` (README:179; `pus-c-from-scratch-assessment.md:39`; `NC-EXT-94`) | `src/pus/pus.c:171-186`: ACCEPTANCE→{1,2}, START→{3,4}, PROGRESS→`5u`, COMPLETION→{7,8}; call sites `:240`(→2), `:246`(→1), `:254`(→8), `:260` (`STAGE_START`, `success=1` → 3), `:263`(→5), `:269`(→7/8). No path passes `success=0` on `STAGE_START`; `6` is never mapped | **CONFIRMED.** 4 is mapped but unreachable, 6 unmapped; the README note that the ICD §4 "4 / 6" row is not answerable on the wire is accurate |
| 43 CTest registrations = 14 unconditional + 29 option-guarded; 41 without lwIP (TS-6 §3.1, rev 1.6) | `grep -c add_test tests/CMakeLists.txt` = 43; `ctest -N` reports 43 in `build/native`, `build/ci`, `build/asan`; guard branches at `tests/CMakeLists.txt:182-206` (20 `FF_BUILD_DRIVERS` names match the TS-6 list exactly; + `hal`, `math`, `sdls`, `link_uart`, `obcp_engine_connectivity`, `pus_n7_adapter_contract`, `pusopen_adapter_contract`; `lwip`/`lwip_netif` under `FF_USE_LWIP`); 14 unconditional IDs match | **CONFIRMED**, including the new `TIMEOUT 60` / `LABELS n22;s8-vv-fu` properties at `tests/CMakeLists.txt:167-168` (N22 comment at `:165-166`) |
| TS-6 §3.2 counts `58 / 15 nominal / 43 OOB / 47 individual / 11 combined / 30 covered / 27 gap / 1 exploratory` | Parsed the §2 case table: 58 rows, IDs 1..58 unique, none missing; status column 30 `covered` + 27 `gap` + 1 `exploratory`; kind column 15 nominal + 41 OOB + 1 exploratory-OOB + 1 mixed = 43 OOB-folded; 47 individual + 11 combined | **CONFIRMED** (history 1.6 correctly states "no §3.2 count changed") |
| All 7 evidence hashes recorded in `verification-results.md` rev 2.91 | `sha256sum` recomputed for asan `4e32e214…404a054`, ci-local `92c19b3e…180d4c35`, docs `c7d1f9a5…e8a6c019a`, format-check `da69e9e4…24de6c83e`, gsw-selfcheck `d1223ce3…1c03e74f1`, smoke `6e4327b5…2c2e343043`, mutant-m3 `70948df0…9085089cb` | **All 7 MATCH** the retained files |
| Mutant-m3 discrimination evidence (15/15 discriminating failures) | Retained log: `exit=1`, `checks=14941 failures=15`; every FAIL at `tests/unit/test_decoder_corpus.c:316` (verified = `FF_TEST_ASSERT(rc == FF_PARSE_ERROR);`), templates 0–8 + 0–5 = 15; `14926 + 15 = 14941` is internally consistent with the corpus report | **CONFIRMED** as a retained, self-consistent discrimination record |
| Gate honesty: `NC-EXT-99` deferral recorded, no N17/N20 pass claimed | `sprint-plan.md:236`, `verification-results.md` (cycle section + rev 2.91), `handover.md` §4/3.09, `loop-audit.md` N7/N17/N20 rows all state the batch-scoped N17/R2-R3/N20 reviews are deferred and open; loop-audit N7 correctly declines to infer historical authorization | **CONFIRMED** — no document claims a review, phase or release approval that did not occur |
| Traceability and metrics gates | `trace.py` re-run by this session: `PASS (117 requirements, 73 srs, 30 sss, 14 gaps, 62 lean theorems checked)`; `metrics.py --check`: `metrics panel: current`, exit 0, tree unchanged | **CONFIRMED** (both read-only) |
| Header/history discipline of the 14 changed documents | Header `Revision` == last `## History` row in every case: component-gap-analysis `0.5`, licensing `1.8`, SSS `0.21`, SRS `1.37`, pus-c-from-scratch-assessment `0.3`, user-integration-manual `0.12`, sprint-plan `1.72`, ts-5 `1.4`, ts-6 `1.6`, verification-results `2.91`, handover `3.09`, loop-audit `10.11` | **CONFIRMED.** `F-0378-04/-05/-06` (stale header dates) are corrected: all four documents now carry `2026-10-19` dates |
| "No requirement … change" in this cycle (`verification-results.md:31-33`) | `git diff e7e48ee..318f49b -- docs/requirements/SRS.md docs/requirements/SSS.md`: header Revision/Date + one history row each, nothing else | **CONFIRMED** — no requirement text, ID or link changed |
| Review-ID allocation | `grep -rn FRICK-REVIEW-0381 docs/` empty; no ledger row carries `0381`; ledger reaches `0380` | **CONFIRMED** — `0381` was the next free sequential ID |

### 3.1 Findings table

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| F-0381-01 | Minor | `docs/vv/verification-results.md` rev 2.91 (evidence record, N11/N16) | The ASan leg is recorded as "**43/43 PASS (166.36 s)**", but the cited retained artifact does not support the duration: `2026-10-19-easy-fixes-asan.log:157` reports `Total Test time (real) = 94.02 sec`, and `166.36` appears in no retained 2026-10-19 log. The PASS verdict itself (43/43) **is** in the log, so this is not a contradicted result but an **unsubstantiated numeric in a controlled evidence record** (most plausibly an operator wall-clock including the build; that is not stated and no artifact substantiates it). | `docs/vv/verification-results.md:77` and history row 2.91 (`:1630`); `docs/vv/evidence/2026-10-19-easy-fixes-asan.log:157` (`Total Test time (real) = 94.02 sec`); `grep -rn '166.36' docs/` → only these two records | Correct the record additively: either replace with the log-supported `94.02 s` (ctest test time) or keep the operator wall-clock with an explicit label naming what it measures and which artifact it came from. Do not overwrite the retained log. |
| F-0381-02 | Minor | `docs/vv/verification-results.md` rev 2.91 (currency/consistency, N8/N16) | The 2026-10-19 section still describes the batch as **uncommitted** after it was committed and pushed: section header "(2026-10-19; **uncommitted**)", "**Scope of this cycle.** **Working-tree-only** batch on top of the pushed baseline `e7e48ee`", "**Cycle evidence (this batch, uncommitted tree)**". The sibling status records were refreshed post-commit (`handover.md` 3.08/3.09 record `318f49b` as the engineering baseline; `loop-audit.md` 10.10/10.11 and `sprint-plan` `NC-EXT-99` state "committed and pushed"), so a reader of `verification-results.md` alone is left with the opposite impression. The "(uncommitted)" label is an established point-in-time convention in this document (e.g. the 2026-10-17 section), but unlike those, this cycle's committed state is now recorded elsewhere and unmarked here. | `docs/vv/verification-results.md:25`, `:27-28`, `:59` vs `docs/status/handover.md` (rev 3.09 §4), `docs/status/loop-audit.md` (rev 10.11, N7/N17/N20 rows), `docs/planning/sprint-plan.md:236` | Refresh the 2026-10-19 section header/scope line to name the committed baseline `318f49b` (keeping the original working-tree provenance note), or add a one-line "superseded for state by commit `318f49b`" marker, in the next documentation delta. |
| F-0381-03 | Minor | `docs/interfaces/ICD.md` (`FRICK-ICD-001`) §4 — disposition already owned by `NC-EXT-94` | The controlled ICD still overstates the Service-1 wire behaviour that the batch corrected everywhere else: the §4 lead-in says "Rows below are the services/subservices **this build answers on the wire**" and the row "`1 \| 4 / 6 \| start / progress failure \| TM \| errcode`" is unqualified, while the batch-established emitted set is `{1,2,3,5,7,8}` (neither 4 nor 6 has an emission path). The README (`:179`) now cross-references the discrepancy accurately, so the two documents disagree on the same fact unless the reader follows the NC link. The ICD is interface-authority-governed, so this is **not** a unilateral-edit recommendation. | `docs/interfaces/ICD.md:96` (lead-in), `:106` (row) — unchanged by the batch (`git diff e7e48ee..318f49b -- docs/interfaces/ICD.md` empty); `README.md:179`; `docs/planning/sprint-plan.md:231` (`NC-EXT-94`) | Carry the already-registered `NC-EXT-94` to its interface/requirements-authority (R3/N14) disposition in the next ICD delta: either annotate the "4 / 6" row as reserved-but-not-generated (with the lead-in narrowed accordingly) or implement the emission path. No new NC required. |
| F-0381-04 | Minor | `docs/requirements/SRS.md` rev 1.37 — disposition already owned by `NC-EXT-97` | The released SRS carries a live contradiction between two controlled requirements: `SRS-REQ-018` (derived) states "the reference build uses the **open CCSDS backend by default**; the N7/ESA ASN.1 backend and PUSopen(R) adapter are opt-in (SRS-REQ-050/065)", while `SRS-REQ-050` states `FF_USE_PUS_ASN1=ON` **by default**, which is the implemented default. The batch deliberately did not edit either statement (correctly, since both are controlled requirements) and registered `NC-EXT-97`; the contradiction nonetheless remains in the document at the reviewed revision. | `docs/requirements/SRS.md:500-503` (`SRS-REQ-018`) vs `:271-277` (`SRS-REQ-050`); `CMakeLists.txt:25` `option(FF_USE_PUS_ASN1 ... ON)`; SRS history row 1.37; `sprint-plan.md:234` (`NC-EXT-97`) | Disposition `NC-EXT-97` with the requirements authority (R3/N14) in the next requirements delta, including the `ADR-0019` cross-reference. No new NC required; do not edit the requirement text unilaterally. |
| F-0381-05 | Comment | `docs/reviews/README.md` — "Current status" narrative block (pre-existing, not introduced by this batch) | The prose block after the ledger still narrates project status as of ~2026-10-14/15, including "The current uncommitted S8-FU candidate includes CCSDS/ingest/GSW and UART framing deltas", which has been superseded by several committed baselines. The block is careful never to claim closure, and the ledger rows above it are current (metrics gate green), so the derogation is limited to a reader mistaking a historical narrative for current state. | `docs/reviews/README.md:388-430+` (block beginning "> **Current status:** FRICK-REVIEW-0167/0168/0169 remain…"; "The current uncommitted S8-FU candidate…"); block unchanged by the batch | In a future index housekeeping delta, either date-stamp the block explicitly as historical or retire it in favour of the ledger rows and the sprint-plan NC register, which are the maintained sources. Non-functional; `N21` applies. |
| F-0381-06 | Comment | `docs/vv/evidence/2026-10-17-s8-reconciliation-candidate-manifest.md` (refreshed by this batch) and the evidence-manifest convention generally | Evidence manifests carry no header `Revision`/`Date` field and no `## History` table. N16 interpretation note 3 exempts only ADRs explicitly; on its face the norm's "V&V documents" scope reaches `docs/vv/evidence/` manifests. This is an established convention across all 11 sibling manifests, and this manifest records its 2026-10-18 refresh inline as a dated paragraph, so no provenance is actually lost — the gap is a scope-wording mismatch between the norm and the practice. | `docs/vv/evidence/*.md` (11 manifests, none with `## History` or a Revision field, checked individually); `docs/rules.md:495-501` (N16 note 3 exempts ADRs only); manifest refresh paragraph "Refreshed 2026-10-18 to satisfy action A-PA-2026-10-18-01…" | Either extend N16 interpretation note 3 to cover point-in-time evidence manifests (records rather than living documents) or add a one-line Revision/date field to manifests that are refreshed in place. Documentation-owner decision; no immediate defect. |

**Severity totals: 0 Major, 4 Minor, 2 Comment.**

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**Bounded conclusion.** For the documentation product of the easy-fixes batch
`e7e48ee..318f49b` as reviewed at HEAD `7e93325`: the batch's substantive factual
claims were independently re-derived and **confirmed** (Service-1 emitted set,
CTest registration arithmetic, TS-6 case counts, all 7 evidence hashes, the
mutant-m3 discrimination record, trace and metrics gates, header/history
discipline, and the honesty of the recorded review-coverage deferral). No Major
finding was identified. Four Minor and two Comment findings are tracked to
closure in §6.

**Explicit limits of this conclusion (read before citing).**

1. This is a **batch-scoped R4 documentation review**, not an N17 phase pass.
   It does not close `NC-EXT-99`: that NC also requires the R2/R3 V&V
   confirmation of the corrected test artifacts and the N20 PA review of the
   batch, neither of which is satisfied by this report.
2. It closes no NC (`NC-EXT-94/96/97/98/99`, `GAP-R3-1`, `NC-EXT-25` remain OPEN
   as recorded), approves no requirement, ICD row, baseline, deviation, risk or
   release, and implies no qualification, target or hosted-CI evidence.
3. Evidence verification was **inspection-level**: hashes were recomputed and
   retained logs were read, but the builds, sanitizer runs, format check, smoke,
   Doxygen and the mutant probe were **not re-executed** by this reviewer. The
   pass/fail results recorded for those gates rest on the retained logs and on
   the FSW engineer's cycle.
4. `NC-EXT-97` (requirement semantics) and the ICD §4 disposition are
   authority-owned and were not edited by this reviewer.

**Confidence: Medium-High** for the confirmed items (direct re-derivation from
source, build configuration, evidence files and independent tool re-runs; no
material conflict found). **Recheck trigger:** any change to `src/pus/pus.c`,
`tests/CMakeLists.txt`/`tests/unit/test_decoder_corpus.c`, the evidence logs, or
a new revision of `verification-results.md`/`ICD.md`/`SRS.md`.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R4 (N9) | Independent documentation-review session (fresh context, `aeronautics-space-sw-documentation-engineer` profile) | 2026-10-19 | APPROVED WITH COMMENTS (batch-scoped; 0 Major, 4 Minor, 2 Comment; not an N17 phase pass; no NC/gate/approval closed) |

## 6. Follow-up records

(norm N12: NCs are resolved by re-applying ALL rules to the affected scope and
re-reviewing the deltas until full compliance; N21: the Minor/Comment items below
are non-functional and are tracked for later disposition rather than blocking
unrelated development.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC candidate (to be registered by owner; next free `NC-EXT` id) | Minor | `docs/vv/verification-results.md` rev 2.91 | F-0381-01: ASan duration `166.36 s` not supported by the cited retained log (`94.02 s`); PASS verdict itself confirmed | Correct the record additively (log-supported value, or labelled operator wall-clock with its source); do not overwrite retained logs | Updated `verification-results.md` delta + R4 confirmation that the figure traces to an artifact | FSW / verification-results custodian | OPEN (raised by this review) | R4 delta check |
| NC candidate (to be registered by owner; next free `NC-EXT` id) | Minor | `docs/vv/verification-results.md` rev 2.91 | F-0381-02: 2026-10-19 section still says "uncommitted"/"working-tree-only" after the batch was committed as `318f49b` and pushed; sibling status docs were refreshed | Refresh or mark the section header/scope line as superseded for state, keeping the working-tree provenance note | Updated `verification-results.md` delta + cross-check against `handover.md`/`loop-audit.md` | FSW / documentation | OPEN (raised by this review) | R4 delta check |
| `NC-EXT-94` (existing; no new NC raised) | Minor interface/spec | `docs/interfaces/ICD.md` §4 | F-0381-03: §4 lead-in and the "4 / 6" row still state subservices answered on the wire that have no emission path (`{1,2,3,5,7,8}` emitted) | Carry the existing registration to interface/requirements-authority (R3/N14) disposition in the next ICD delta; README cross-note is already accurate | ICD delta + independent R3/R4 delta review | FSW + interface/requirements authority | OPEN (already registered) | R3/R4 ICD delta review |
| `NC-EXT-97` (existing; no new NC raised) | Minor requirements | `docs/requirements/SRS.md` rev 1.37 | F-0381-04: `SRS-REQ-018` vs `SRS-REQ-050` backend-default contradiction remains in the released SRS | Requirements-authority (R3/N14) disposition with `ADR-0019` cross-reference; no unilateral edit | SRS delta + independent R3 review | Requirements authority (R3) with FSW | OPEN (already registered) | R3 delta review |
| NC candidate (optional; Comment-level) | Comment | `docs/reviews/README.md` | F-0381-05: "Current status" narrative block is stale (2026-10-14/15 vintage, "current uncommitted S8-FU candidate") | Date-stamp as historical or retire in favour of the ledger + sprint-plan register, in an index housekeeping delta | Updated index block + R4 check | FSW / documentation | OPEN (raised by this review) | R4 delta check |
| Documentation-scope note (no NC) | Comment | `docs/rules.md` N16 note 3 / `docs/vv/evidence/*.md` | F-0381-06: evidence manifests have no History/Revision table and are not exempted by the norm's wording | Extend interpretation note 3 to point-in-time evidence manifests, or add a Revision/date field to refreshed manifests | `docs/rules.md` or manifest delta + R4 check | Documentation owner (with PA) | OPEN (observation) | R4 delta check |
| `NC-EXT-99` (existing) | Process/review-coverage | Easy-fixes batch 2026-10-19 | Deferred batch-scoped reviews: N17 R4, R2/R3 V&V confirmation, N20 PA | **This report satisfies the R4/N17 documentation leg only.** The R2/R3 V&V confirmation (`FRICK-REVIEW-0382`, concurrent) and the PA review (`FRICK-REVIEW-0383`, concurrent) remain to be produced and registered; findings processed under N12; closure is an owner disposition | This report + the two sibling reports when published and indexed | FSW (request/coordinate); verifiers = respective independent profiles | OPEN — not closed by this report | Owner disposition after all three reports are registered |

---

*This report modifies no file other than itself. Per the recorded `NC-EXT-99`
deferral it is not added to `docs/reviews/README.md` by this reviewer; indexing
and the NC dispositions above are owner/FSW actions (norm N7/N12). It is
committed and pushed with the accompanying change set under norm N11 by the
authorized agent, not by this reviewer.*
