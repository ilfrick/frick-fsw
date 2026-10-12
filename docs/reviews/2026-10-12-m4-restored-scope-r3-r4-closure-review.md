# Review Report — M4 restored scope — combined R3+R4 delta-closure review (norm N12/N17/N14/N9)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                           | frick-fsw                              |
| Document id                       | FRICK-REVIEW-0141                      |
| Reviewer session                  | independent agent, norm N9 (fresh context) |
| Reviewer profile                  | **R3 + R4** — Senior Systems / avionics engineer **and** Senior SW documentation engineer (combined delta-closure) |
| Reference revision                | `ce0bff3` + **uncommitted** working tree (HEAD `ce0bff3`) |
| Items under review                | `docs/requirements/SSS.md` (0.14), `docs/requirements/SRS.md` (1.18), `docs/requirements/traceability.yaml` + `docs/requirements/traceability.md` (0.30), `docs/roadmap.md` (0.18), `docs/development-plan.md` (0.20), `docs/planning/sprint-plan.md` (0.6), `docs/quality/class-a-gaps.md` (0.8), `docs/architecture/network-stack.md` (0.4), `docs/engineering/user-integration-manual.md` (0.6), `docs/engineering/driver-development.md` (0.4), `docs/status/loop-audit.md` (0.14), `docs/status/status.json`, `docs/vv/verification-results.md` (0.35), `README.md`, `docs/reviews/README.md`; the review set `docs/reviews/2026-10-12-m4-restored-scope-{r3-review,r3-delta-review,r4-review,pa-review}.md` (FRICK-REVIEW-0137/0139/0138/0140) |
| Date                              | 2026-10-12                             |
| Scope                             | **Combined delta-closure review (norm N12)** of the R3-delta non-conformances (R3-D01..R3-D09, `FRICK-REVIEW-0139`) and the R4 non-conformances (R4-01..R4-08, `FRICK-REVIEW-0138`) of the "restored explicit M4 scope" requirements/planning increment. Verified first-hand, on the fixed working tree: (a) each listed finding against the actual files; (b) that the fixes did not regress the requirements/planning coherence or the documentation consistency; (c) re-run of the applicable gates (`trace.py` N4, `metrics.py --check` N8/T13). Assessed against ECSS-E-ST-40C Rev.1 (requirements/architecture process, data items, documentation obligations), ECSS-Q-ST-80C Rev.2 (assurance and records), ECSS-E-ST-70-41C (PUS-C) and CCSDS 133.0-B-2, under project norms N1–N20 (`docs/rules.md` 1.4). |
| Independence declaration          | The reviewer had no part in authoring or executing the items under review — it did not author the increment, the FRICK-REVIEW-0137/0138/0139/0140 reports, or any reviewed document — and ran in a fresh, independent context with only the material under review, the prior reports and the reference documents. Read-only for the repository: it wrote **exactly one** file (this report) and modified, created or deleted no other file; nothing was committed or pushed. |
| Norms applied                     | N1–N20 (see `docs/rules.md`), focus N4, N8, N9, N11, N12, N14, N15, N16, N17, N19, N20 |

## 1. Scope

This is the **combined same-profile delta re-review** required by **norm N12**
after the non-conformances of the R3 delta (`FRICK-REVIEW-0139`, R3-D01..R3-D09)
and the R4 documentation review (`FRICK-REVIEW-0138`, R4-01..R4-08) were
corrected. The PA review (`FRICK-REVIEW-0140`) raised **PA-01 (Minor, pre-commit)**
because those fixes were present in the working tree but their follow-up tables
still listed the NCs as open and no same-profile delta re-review existed. This
report closes that gap: it verifies every fix first-hand and **records the
disposition and the delta re-review** for both profiles.

**Included:** the closure of R3-D01..R3-D09 and R4-01..R4-08; the coherence of the
fixed requirements/planning/documentation state; the applicable gates; and the
records-currency of the review set.

**Excluded (other profiles' remits / already dispositioned):** R3-01..R3-12
(closed by `FRICK-REVIEW-0139` §7.1 and re-confirmed by `FRICK-REVIEW-0140` §4.1);
the PA findings PA-01..PA-06 except where they are the residuals of the R3-D/R4
findings reviewed here; the internal technical correctness of code/build/tests
(**none changed by this increment** — `git status` shows only documentation/
requirements files); the open ECSS NC `PA-67-01` (SDD design matrices, tracked to
S0/S1, outside this increment); on-target silicon behaviour (GAP-A5).

**Assumption recorded:** the increment is an **uncommitted** working tree on
`ce0bff3`, so no commit hash identifies the reviewed state; evidence is tied to
the file content at the time of review. The increment is documentation/
requirements-only, so it triggers no code/V&V/Lean re-run (N1/N15 apply to the
touched scope); the applicable gates were nevertheless re-executed.

## 2. References

- Project norms: `docs/rules.md` (N1–N20, rev 1.4); reviewer contract and NC loop:
  `docs/engineering/independent-review.md` (rev 0.7, norm N9/N12/N14/N17/N20);
  template `docs/reviews/review-template.md`.
- Prior reviews: R3 `docs/reviews/2026-10-12-m4-restored-scope-r3-review.md`
  (FRICK-REVIEW-0137); R3-delta
  `docs/reviews/2026-10-12-m4-restored-scope-r3-delta-review.md`
  (FRICK-REVIEW-0139); R4 `docs/reviews/2026-10-12-m4-restored-scope-r4-review.md`
  (FRICK-REVIEW-0138); PA `docs/reviews/2026-10-12-m4-restored-scope-pa-review.md`
  (FRICK-REVIEW-0140).
- Traceability manifest: `docs/requirements/traceability.yaml`; matrices:
  `docs/requirements/traceability.md`; gates: `tools/scripts/trace.py` (N4),
  `tools/scripts/metrics.py` (V&V tool T13).
- ECSS-E-ST-40C Rev.1 (requirements/architecture process, decomposition,
  traceability, documentation/data items); ECSS-Q-ST-80C Rev.2 (assurance,
  records, independent verification); ECSS-E-ST-70-41C (PUS-C); CCSDS 133.0-B-2
  (Space Packet).
- `docs/reviews/review-template.md`; `docs/reviews/README.md`.

**Gate re-runs (evidence, reproduced first-hand on the fixed working tree).**

1. `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` →
   `traceability: PASS  (112 requirements, 69 srs, 29 sss, 14 gaps, 51 lean
   theorems checked)`, **exit 0** (norm N4). Bottom-up summary:
   `SSS-REQ-015 <- 2 srs children`, `SSS-REQ-028 <- 3 srs children`,
   `SSS-REQ-029 <- 4 srs children`.
2. `python3 tools/scripts/metrics.py --check` → `metrics panel: current`,
   **exit 0** (norm N8 / T13).

## 3. Verification performed (independent), with results

| # | Check (norm) | Artifact / command | Result (independently reproduced 2026-10-12) |
| - | ------------ | ------------------ | -------------------------------------------- |
| V1 | Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 112 requirements (69 srs / 29 sss / 14 gaps), 51 Lean theorems, exit 0 |
| V2 | Metrics-panel currency (N8/T13) | `python3 tools/scripts/metrics.py --check` | **exit 0** — `metrics panel: current` |
| V3 | SSS-REQ-015 decomposition (N4) | `SSS.md` §3; `traceability.yaml`; gate bottom-up | SSS-REQ-015 text names only the open + N7/ESA ASN.1 backends and cross-refers SSS-REQ-029; `SSS-REQ-015 <- 2 srs children` (018/050) |
| V4 | Manifest wording reconciliation (N4/N8) | `traceability.yaml` SRS-REQ-018 `justification`, SSS-REQ-028 `lean_note`, SRS-REQ-066 `lean_note` | All corrected (N7/ESA default; integrator + run-time `FF_LinkSelect`; CBS TBC `owner: M8`); no live "integration time"/"open default" outside review reports and the historical ADR-0003 title |
| V5 | Metrics/sprints reconciliation (N8/N19) | `status.json`; `README.md:51`; `sprint-plan.md` §2 | `sprints {total:29, done:3, in_progress:1}`; panel `3/29 (10.3%)`; consistent with S1/S2/S3 delivered, S4 in progress |
| V6 | Documentation id-citations (N4) | `development-plan.md`, `roadmap.md`, `network-stack.md`, `traceability.md` intro | SSS-REQ-028/029 and SRS-REQ-067/068 cited in the M4/M8 bullets; `network-stack.md` header lists SRS-REQ-059/060/061; `traceability.md` intro parent list = SSS-REQ-017/020/028/029 |
| V7 | Records/loop currency (N11/N16/N19) | `loop-audit.md:9`; `verification-results.md` last row; `user-integration-manual.md` History 0.5 | baseline `ce0bff3 (Sprint-3) + restored-M4-scope increment`; cycle row 0.35 present (112/69/29/14/51); History 0.5 no longer claims a DDJF cross-reference |
| V8 | Fix regressions (N4/N8/N12) | full-file re-read of every touched requirement/planning/doc fragment | no regression introduced by the fixes; the increment remains documentation/requirements-only |

## 4. Closure table (R3-D01..D09 and R4-01..R4-08)

Dispositions are based on **first-hand inspection of the fixed working tree**
(not on the author's summary). "Record" notes whether the closure is now recorded
(N12). Severity is as originally raised.

### 4.1 R3 delta (FRICK-REVIEW-0139) — R3-D01..R3-D09

| # | Sev | Finding (item) | Disposition | First-hand evidence | Record | Status |
| - | --- | -------------- | ----------- | ------------------- | ------ | ------ |
| R3-D01 | Minor | SSS-REQ-015 named the PUSopen adapter but decomposed it only under SSS-REQ-029 | Fixed: SSS-REQ-015 text restricted to the open + N7/ESA ASN.1 backends and cross-references SSS-REQ-029 for the adapter (one-parent-per-SRS manifest convention) | `SSS.md` §3 SSS-REQ-015 ("…(the PUSopen(R) adapter is covered by SSS-REQ-029)"); gate `SSS-REQ-015 <- 2 srs children` (018/050), `SSS-REQ-029 <- 4 srs children` (062/063/064/065) | FRICK-REVIEW-0141 (this report) | **CLOSED** |
| R3-D02 | Minor | `traceability.yaml` SRS-REQ-018 `justification` still asserted the "open default" | Fixed: justification now "…the N7/ESA ASN.1 backend is the default (SRS-REQ-050), the open backend is an alternative, and the PUSopen(R) adapter is optional/planned (SRS-REQ-065)" | `traceability.yaml:190` SRS-REQ-018 `justification`; consistent with `SSS.md` 015, `SRS.md` 050/018, ADR-0016 | FRICK-REVIEW-0141 | **CLOSED** |
| R3-D03 | Minor | `traceability.yaml` SSS-REQ-028 `lean_note` still said "link selectable at integration time"; integrator-select semantics undefined | Fixed: note now "…the active link is selectable by the integrator (and switchable at run time via `FF_LinkSelect`)" — one semantics consistent with SSS-REQ-028, SRS-REQ-060/061 and the manual §7.1 | `traceability.yaml:856` SSS-REQ-028 `lean_note`; `grep -rn "integration time" docs/` → only review reports | FRICK-REVIEW-0141 | **CLOSED** |
| R3-D04 | Minor | Stale README metrics panel; `metrics.py --check` exit 1; `sprints.done=4` untraceable to the plan | Fixed: panel regenerated; `sprints.done` reconciled to 3 (S1/S2/S3) | V2 `metrics panel: current` (exit 0); `status.json` `sprints {total:29, done:3, in_progress:1}`; `README.md:51` `3/29 (10.3%)` | FRICK-REVIEW-0141 | **CLOSED** |
| R3-D05 | Comment | "TCP/IP" shorthand in SSS-REQ-028 / SRS-REQ-060 title vs the body's UDP reference | Fixed at the normative system level: SSS-REQ-028 now "IP-based links (TCP or UDP per the mission ICD)" (the transport is a stated mission input). Residual: the SRS-REQ-060 **title** still says "TCP/IP PUS-C link backend" (label only) — see NC-141-03 | `SSS.md` SSS-REQ-028; `SRS.md` SRS-REQ-060 body ("TCP or UDP as selected by the mission ICD; the reference uses UDP datagrams") vs title | FRICK-REVIEW-0141 (residual → NC-141-03) | **CLOSED** (residual Comment) |
| R3-D06 | Comment | SRS-REQ-066 feasibility TBC without owner/milestone | Fixed: owner added in both places | `SRS.md` SRS-REQ-066 ("…= TBC (owner: M8)"); `traceability.yaml` SRS-REQ-066 `lean_note` ("…= TBC (owner: M8)") | FRICK-REVIEW-0141 | **CLOSED** |
| R3-D07 | Comment | Manifest `implementation` links for SSS-REQ-028/029 and SRS-REQ-067/068 not id-verifiable | Fixed: the M4/M8 bullets of both documents now cite the ids | `development-plan.md:157-162` (SSS-REQ-028/029, SRS-REQ-067), `:192` (SRS-REQ-068); `roadmap.md:79-84` (SSS-REQ-028/029, SRS-REQ-067), `:131` (SRS-REQ-068) | FRICK-REVIEW-0141 | **CLOSED** |
| R3-D08 | Comment | `loop-audit.md` `Baseline commit` was the previous loop's revision | Fixed: re-baselined to the audited revision | `loop-audit.md:9` `ce0bff3 (Sprint-3) + restored-M4-scope increment (audited revision)`; History 0.14 records it | FRICK-REVIEW-0141 | **CLOSED** |
| R3-D09 | Comment | No `verification-results.md` cycle row for this increment | Fixed: cycle row appended | `verification-results.md` rev 0.35 row: `112 reqs / 69 srs / 29 sss / 14 gap, 51 theorems`, trace + metrics green | FRICK-REVIEW-0141 | **CLOSED** |

### 4.2 R4 (FRICK-REVIEW-0138) — R4-01..R4-08

| # | Sev | Finding (item) | Disposition | First-hand evidence | Record | Status |
| - | --- | -------------- | ----------- | ------------------- | ------ | ------ |
| R4-01 | Minor | `traceability.md` intro parent list wrongly included SSS-REQ-015 | Fixed: list corrected to the actual parents | `traceability.md` intro "…(SRS-REQ-059..068 with SSS parents SSS-REQ-017/020/028/029)"; manifest `parent_sss` (059/060/061→028, 062-065→029, 066/068→020, 067→017) | FRICK-REVIEW-0141 | **CLOSED** |
| R4-02 | Minor | SRS-REQ-018 + manifest justification stale "open backend" default | Fixed: SRS-REQ-018 text and manifest justification now state the N7/ESA ASN.1 default with the open backend as the fallback (same fix as R3-D02) | `SRS.md` SRS-REQ-018; `traceability.yaml:190` | FRICK-REVIEW-0141 | **CLOSED** |
| R4-03 | Minor | `driver-development.md` §1 default-link mapping omitted QEMU | Fixed: §1 now "(FRICK_PLATFORM_MPFS **or** FRICK_PLATFORM_QEMU → UART, otherwise → loopback)" | `driver-development.md` §1; consistent with `link.c:15` and the manual §7.1 | FRICK-REVIEW-0141 | **CLOSED** |
| R4-04 | Comment | README Highlights listed 802.1AS without the "time-read seam" qualifier | Fixed: qualifier present in both occurrences | `README.md:119` ("802.1Q/Qbv + an 802.1AS time-read seam"); `README.md:181` (driver table) | FRICK-REVIEW-0141 | **CLOSED** |
| R4-05 | Comment | Sprint denominator (26) vs the added S4b/S4c/S4d rows | Fixed: denominator aligned to 29 | `status.json` `sprints.total=29`; `README.md:51` `3/29`; V2 current | FRICK-REVIEW-0141 | **CLOSED** |
| R4-06 | Comment | `user-integration-manual.md` History 0.5 claimed a DDJF cross-reference not present in §7.1 | Fixed: History 0.5 reworded; no false DDJF claim in §7.1 | `user-integration-manual.md` History 0.5 | FRICK-REVIEW-0141 | **CLOSED** |
| R4-07 | Comment | FRICK-REVIEW-0137 record state vs the committed tree; R3 NCs open | Fixed: the R3 delta report records the fixed revision and closes R3-01..R3-12 | `FRICK-REVIEW-0139` header ("`ce0bff3` + uncommitted working tree (fixed revision)") and §7.1 (R3-01..12 closed) | FRICK-REVIEW-0141 | **CLOSED** |
| R4-08 | Comment | `network-stack.md` header "Requirements" omitted SRS-REQ-059/060/061 | Fixed: header now lists them | `network-stack.md` header `Requirements: SRS-REQ-027, SRS-REQ-039, SRS-REQ-040, SRS-REQ-059, SRS-REQ-060, SRS-REQ-061` | FRICK-REVIEW-0141 | **CLOSED** |

**Net closure:** **all 17 findings** (R3-D01..R3-D09, R4-01..R4-08) are
**CLOSED** on first-hand evidence; the fixes are present in the working tree, no
fix regressed the requirements/planning/documentation coherence, and the
applicable gates pass. Three **Comment**-level records residuals remain
(NC-141-01..NC-141-03, §5) and are advisory.

**Positive confirmations (verified, no finding).**

- `trace.py` PASS with the expected counts (112/69/29/14/51, exit 0) and
  `metrics.py --check` current (exit 0).
- SSS-REQ-029 now parents all four components named in its text (062/063/064/065);
  SSS-REQ-015 has exactly two children (018/050) and no longer names the adapter.
- The N7/ESA ASN.1 default is stated consistently across `SSS.md` SSS-REQ-015,
  `SRS.md` SRS-REQ-018/050, the manifest justifications and ADR-0016.
- Link-selection semantics are now single-worded ("selectable by the integrator …
  switchable at run time via `FF_LinkSelect`") across the SSS, the manifest note,
  the SRS children and the user manual §7.1.
- The TBCs carry owners (SRS-REQ-059 `owner: M4-residual`; SRS-REQ-066
  `owner: M8` in both the SRS text and the manifest note) — no silent TBC.
- No over-claim: only loopback + UART link backends ship; the IP/TSN/CAN/SpaceWire
  PUS-C link backends remain explicitly "not implemented (planned)"; 802.1Qav
  returns `FF_NOT_IMPLEMENTED`; 802.1AS is a "time-read seam" (gPTP = M8).

## 5. New findings (delta)

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| NC-141-01 | Comment | NC close-out records (N12/N11) | **The two source follow-up tables still record the NCs as open.** `FRICK-REVIEW-0139` §7.2 lists R3-D01..R3-D09 as "open / To be dispositioned" and `FRICK-REVIEW-0138` §6 lists R4-01..R4-08 as "open". The disposition and the same-profile delta re-review are now recorded **in this report** (FRICK-REVIEW-0141), which satisfies N12; but the source tables and the log are not updated. This is the residual of PA-01 (records currency, not a product defect). | `2026-10-12-m4-restored-scope-r3-delta-review.md` §7.2; `…-r4-review.md` §6; §4 of this report | Update the two follow-up tables to reference FRICK-REVIEW-0141 (or mark them superseded) in the same commit (N11/N12). |
| NC-141-02 | Comment | `docs/reviews/README.md` log rows (N11/N16) | **The review log is not reconciled with the reports.** Row 138 (R4) still reads "APPROVED WITH COMMENTS (R4-01..03 closed)" although R4-01..R4-08 are now closed (and were listed open in the report); row 140 (PA) still reads "(pending)" although `FRICK-REVIEW-0140` exists (APPROVED WITH COMMENTS); and there is no row for `FRICK-REVIEW-0141` yet. This is the residual of PA-02/PA-03. | `docs/reviews/README.md:138,140`; `FRICK-REVIEW-0138` §6; `FRICK-REVIEW-0140` header | Reconcile the rows at commit and add the closure-review row (N11). |
| NC-141-03 | Comment | `SRS.md` SRS-REQ-060 title (N8) | **Residual "TCP/IP" shorthand in the SRS-REQ-060 title** ("TCP/IP PUS-C link backend") while its body binds the transport to the mission ICD ("TCP or UDP … reference UDP") and SSS-REQ-028 now says "IP-based links (TCP or UDP per the mission ICD)". The body is testable; the title label could re-introduce the ambiguity. Residual of R3-D05/PA-06. | `SRS.md` SRS-REQ-060 title vs body; `SSS.md` SSS-REQ-028 | Harmonise the title (e.g. "IP (UDP reference) PUS-C link backend") or add a parenthetical pointer to the mission-ICD input (N8). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
  *(None identified.)*
- **Minor**: non-conformity that does not invalidate the requirement but must be
  corrected or justified. *(None open — all prior Minors closed.)*
- **Comment**: observation, recommendation, no derogation. *(NC-141-01..03.)*

## 6. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — all prior Minors closed; three Comment-level records residuals tracked
- [ ] REJECTED — one or more Major findings

**Conclusion: APPROVED WITH COMMENTS.** No Major finding. **All 17 prior
findings (R3-D01..R3-D09, R4-01..R4-08) are CLOSED** on first-hand evidence and
recorded here per norm N12; the delta re-review for both profiles (R3 and R4) is
satisfied by this report, and no fix introduced a regression. The applicable
gates are green (`trace.py` PASS 112/69/29/14/51; `metrics.py --check` current).
The residual items are **Comment-level records/wording** observations
(NC-141-01..03): the source follow-up tables and the review log are not yet
reconciled, and the SRS-REQ-060 title keeps the "TCP/IP" shorthand. Per norm N12
the R3-D/R4 non-conformances are closed; per norm N7 the increment may proceed to
commit/push once the records reconciliation (NC-141-01/02) and the PA-01
close-out are recorded in the same commit. No certification, airworthiness,
human-rating or qualification claim is made or implied by this review.

## 7. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R3 + R4 (N9/N12/N14/N17) | Senior Systems / avionics engineer **and** Senior SW documentation engineer (independent agent, norm N9) | 2026-10-12 | **APPROVED WITH COMMENTS** — R3-D01..D09 and R4-01..R4-08 CLOSED; three Comment-level records residuals (NC-141-01..03); no Major |

## 8. Follow-up records

(norm N12: NCs are resolved by re-applying all rules to the affected scope and
re-reviewing the deltas until full compliance.)

### 8.1 R3 delta (FRICK-REVIEW-0139) — closure record

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | ------ | --------------- |
| R3-D01 | Minor | SSS-REQ-015 | Adapter named but decomposed under SSS-REQ-029 | Fixed: text restricted + cross-reference | `SSS.md` 015; gate `SSS-REQ-015 <- 2` | frick-fsw | **closed** | FRICK-REVIEW-0141 |
| R3-D02 | Minor | `traceability.yaml` SRS-REQ-018 | Justification "open default" | Fixed: N7/ESA default + open alternative + planned PUSopen | `traceability.yaml:190` | frick-fsw | **closed** | FRICK-REVIEW-0141 |
| R3-D03 | Minor | `traceability.yaml` SSS-REQ-028 | Note "integration time"; semantics undefined | Fixed: integrator + run-time `FF_LinkSelect` | `traceability.yaml:856` | frick-fsw | **closed** | FRICK-REVIEW-0141 |
| R3-D04 | Minor | README panel vs `status.json` | Stale panel; `metrics.py --check` exit 1; `done=4` untraceable | Fixed: panel regenerated; `done=3` | V2; `status.json`; `README.md:51` | frick-fsw | **closed** | FRICK-REVIEW-0141 |
| R3-D05 | Comment | SSS-REQ-028 / SRS-REQ-060 title | "TCP/IP" shorthand vs UDP body | Fixed in the SSS text (mission-ICD binding); title residual → NC-141-03 | `SSS.md` 028 | frick-fsw | **closed** (residual → NC-141-03) | FRICK-REVIEW-0141 |
| R3-D06 | Comment | SRS-REQ-066 | CBS TBC without owner | Fixed: `owner: M8` (SRS text + manifest note) | `SRS.md` 066; `traceability.yaml` 066 | frick-fsw | **closed** | FRICK-REVIEW-0141 |
| R3-D07 | Comment | Manifest links SSS-REQ-028/029, SRS-REQ-067/068 | Ids not cited in development-plan/roadmap | Fixed: ids cited in both | `development-plan.md`; `roadmap.md` | frick-fsw | **closed** | FRICK-REVIEW-0141 |
| R3-D08 | Comment | `loop-audit.md` header | Baseline commit stale | Fixed: re-baselined | `loop-audit.md:9` | frick-fsw | **closed** | FRICK-REVIEW-0141 |
| R3-D09 | Comment | `verification-results.md` | No cycle row | Fixed: row 0.35 appended | `verification-results.md` 0.35 | frick-fsw | **closed** | FRICK-REVIEW-0141 |

### 8.2 R4 (FRICK-REVIEW-0138) — closure record

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | ------ | --------------- |
| R4-01 | Minor | `traceability.md` intro | Parent list wrongly included SSS-REQ-015 | Fixed: SSS-REQ-017/020/028/029 | `traceability.md` intro | frick-fsw | **closed** | FRICK-REVIEW-0141 |
| R4-02 | Minor | SRS-REQ-018 + manifest justification | Stale "open backend" default | Fixed: N7/ESA default + open fallback | `SRS.md` 018; `traceability.yaml:190` | frick-fsw | **closed** | FRICK-REVIEW-0141 |
| R4-03 | Minor | `driver-development.md` §1 | QEMU → UART omitted | Fixed: "MPFS or QEMU → UART" | `driver-development.md` §1 | frick-fsw | **closed** | FRICK-REVIEW-0141 |
| R4-04 | Comment | README Highlights | 802.1AS unqualified | Fixed: "802.1AS time-read seam" | `README.md:119,181` | frick-fsw | **closed** | FRICK-REVIEW-0141 |
| R4-05 | Comment | `status.json` / README sprints | Denominator 26 vs S4b/S4c/S4d | Fixed: total 29 | `status.json`; V2 | frick-fsw | **closed** | FRICK-REVIEW-0141 |
| R4-06 | Comment | `user-integration-manual.md` History 0.5 | Claimed a DDJF cross-reference not present | Fixed: reworded | History 0.5 | frick-fsw | **closed** | FRICK-REVIEW-0141 |
| R4-07 | Comment | FRICK-REVIEW-0137 record | Record state vs tree; NCs open | Fixed: R3-delta records the fixed revision; R3-01..12 closed | `FRICK-REVIEW-0139` header/§7.1 | frick-fsw | **closed** | FRICK-REVIEW-0141 |
| R4-08 | Comment | `network-stack.md` header | Requirements field omitted SRS-REQ-059/060/061 | Fixed: added | `network-stack.md` header | frick-fsw | **closed** | FRICK-REVIEW-0141 |

### 8.3 New NCs (this closure review)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | ------ | --------------- |
| NC-141-01 | Comment | 0139 §7.2 / 0138 §6 follow-up tables (N12/N11) | Source tables still list the NCs open | Update to reference FRICK-REVIEW-0141 | §4 of this report | frick-fsw | open (advisory) | — |
| NC-141-02 | Comment | `docs/reviews/README.md` rows 138/140 (N11/N16) | Log not reconciled; no row for 0141 | Reconcile at commit | `README.md:138,140` | frick-fsw | open (advisory) | — |
| NC-141-03 | Comment | `SRS.md` SRS-REQ-060 title (N8) | "TCP/IP" shorthand vs the mission-ICD body | Harmonise the title | `SRS.md` 060 | frick-fsw | open (advisory) | — |

## 9. Facts / Assumptions / Inferences / Recommendations / Confidence / Open gaps

**Facts (independently reproduced with evidence):**

- `trace.py` PASS: 112 requirements (69 srs / 29 sss / 14 gaps), 51 Lean theorems,
  exit 0; `metrics.py --check` → `metrics panel: current`, exit 0.
- SSS-REQ-015 now names only the open + N7/ESA ASN.1 backends and cross-refers
  SSS-REQ-029; the gate shows `SSS-REQ-015 <- 2` and `SSS-REQ-029 <- 4` children.
- The manifest SRS-REQ-018 `justification` states the N7/ESA ASN.1 default + open
  alternative + planned PUSopen; the SSS-REQ-028 `lean_note` states "selectable by
  the integrator (and switchable at run time via `FF_LinkSelect`)"; the
  SRS-REQ-066 `lean_note` carries `owner: M8`. No live "integration time" or "open
  default" remains outside review reports / the historical ADR-0003 title.
- `status.json` `sprints {total:29, done:3, in_progress:1}` and `README.md:51`
  `3/29 (10.3%)` are consistent; the metrics gate passes.
- `development-plan.md` and `roadmap.md` now cite SSS-REQ-028/029 and
  SRS-REQ-067/068; `network-stack.md` header lists SRS-REQ-059/060/061;
  `traceability.md` intro lists SSS-REQ-017/020/028/029.
- `loop-audit.md:9` is re-baselined; `verification-results.md` has the 0.35 cycle
  row (112/69/29/14/51); `user-integration-manual.md` History 0.5 no longer claims
  a DDJF cross-reference.
- `FRICK-REVIEW-0139` §7.2 and `FRICK-REVIEW-0138` §6 still list their NCs as
  open; `docs/reviews/README.md` rows 138/140 are not reconciled; `SRS.md`
  SRS-REQ-060 title still says "TCP/IP".

**Assumptions:**

- The material under review is exactly `ce0bff3` + the uncommitted working tree
  (confirmed with `git status`); the increment is documentation/requirements-only.
- "M4-residual" is a re-plan of the original M4 scope scheduled after sprint S4;
  the new SSS/SRS items are planned deliverables, so `ci:docs` is the expected
  evidence until delivery (the R3-12 convention).

**Inferences:**

- The fixes are local wording/decomposition/records corrections of an otherwise
  complete reconciliation; they do not invalidate any requirement or the delivered
  baseline and are correctable without rework.
- The remaining items (NC-141-01..03) are records-currency and label-level
  observations, not product defects; they do not block the technical closure of
  the NCs recorded here.

**Recommendations:**

- Reconcile the two source follow-up tables (NC-141-01) and the review-log rows
  (NC-141-02) in the same commit (N11/N12).
- Harmonise the SRS-REQ-060 title (NC-141-03) for consistency with the body.
- Prefer manifest-derived generation of `traceability.md` §1 and the planned-set
  statement to prevent matrix/manifest divergence recurring.
- Log this report (FRICK-REVIEW-0141) and update the `docs/reviews/README.md`
  rows at commit (N11).

**Confidence:** **High** for the closure facts and the gate reproductions (each
reproduced directly from the files and the two gates); **High** for the R3-D/R4
dispositions (direct text comparisons); **Medium** for the records/wording
comments (NC-141-01..03), which depend on review-log and title conventions.

**Open gaps (not closed by this review):**

- The source follow-up tables / review log reconciliation (NC-141-01/02) — an
  administrative close-out to be done at commit.
- The SRS-REQ-060 title shorthand (NC-141-03).
- The residual TBCs by design: GAP-A5 (RS-485 DE/RE on-target), the GEM CBS
  feasibility TBC (owner M8) and the PUS-C-over-IP mission-profile TBC (owner
  M4-residual).
- The open ECSS NC `PA-67-01` (SDD design matrices) — outside this increment.

---

**Independence declaration (norm N9/N12).** I performed this combined R3+R4
delta-closure review in an independent, fresh context with no part in authoring or
executing the items under review (I did not author the increment, the
FRICK-REVIEW-0137/0138/0139/0140 reports, or any reviewed document), based only
on the evidence in the repository at `ce0bff3` + the uncommitted working tree and
the reference documents. I am read-only for the repository and, per norm N11 and
the task, I wrote **exactly one** file — this report — and modified, created or
deleted no other file; I did not commit or push. (Read-only command output was
written outside the repository under `/tmp/opencode/`.)

Committed and pushed with the reviewed change (norm N11).
