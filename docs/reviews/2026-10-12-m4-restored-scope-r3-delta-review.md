# Review Report — M4 restored scope (requirements / planning increment) — R3 delta re-review (norm N12)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                           | frick-fsw                              |
| Document id                       | FRICK-REVIEW-0139                      |
| Reviewer session                  | independent agent, norm N9 (fresh context) |
| Reviewer profile                  | R3 — Senior Systems / avionics engineer (space/aeronautics) |
| Reference revision                | `ce0bff3` + uncommitted working tree (fixed revision) |
| Items under review                | `docs/requirements/SSS.md` (0.14), `docs/requirements/SRS.md` (1.18), `docs/requirements/traceability.yaml`, `docs/requirements/traceability.md` (0.30), `docs/roadmap.md` (0.18), `docs/development-plan.md` (0.20), `docs/planning/sprint-plan.md` (0.6), `docs/quality/class-a-gaps.md` (0.8), `docs/status/loop-audit.md` (0.14), `docs/status/status.json`, `docs/status/handover.md` (1.10), `README.md`, `docs/architecture/network-stack.md` (0.4), `docs/engineering/user-integration-manual.md` (0.6), `docs/engineering/driver-development.md` (0.4), `docs/glossary.md` (0.2), `docs/engineering/software-description.md` (0.3), `CHANGELOG.md`, `AGENTS.md` |
| Date                              | 2026-10-12                             |
| Scope                             | **Delta re-review (norm N12)** of the "restored explicit M4 scope" requirements/planning increment after the initial R3 review (`FRICK-REVIEW-0137`, findings R3-01..R3-12) and the R4 documentation review (`FRICK-REVIEW-0138`, findings R4-01..R4-08) were fixed. Verified: closure of each prior R3 finding first-hand, absence of regressions, requirement well-formedness and parent/child coherence of the new/changed requirements, manifest ↔ matrix ↔ requirement-text agreement, top-down and bottom-up convergence, milestone/sprint scheduling coherence, and gate re-runs. Assessed against ECSS-E-ST-40C (requirements and architecture process), ECSS-Q-ST-80C (assurance), ECSS-E-ST-70-41C (PUS-C) and CCSDS 133.0-B-2, under project norms N1–N20. |
| Independence declaration          | The reviewer had no part in authoring or executing the items under review; it ran in a fresh context (a new independent session, separate from the author's context and from the initial R3 session's working memory) with only the material under review, the prior review reports and the reference documents. It is read-only for the repository except for this single report. |
| Norms applied                     | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

This is the **delta re-review required by norm N12** after the non-conformances
raised by `FRICK-REVIEW-0137` (R3-01..R3-12) and the documentation findings
raised by `FRICK-REVIEW-0138` (R4-01..R4-08) were dispositioned. It covers:

- the **re-parenting** of SRS-REQ-062 and SRS-REQ-065 to SSS-REQ-029
  (SSS-REQ-029 now owns four SRS children: 062/063/064/065) — R3-01;
- the **PUSopen delivery-status and default-backend reconciliation** across
  SSS-REQ-015 / SRS-REQ-018 / SRS-REQ-065 (and the manifest SSS-REQ-015
  justification) — R3-02;
- the `traceability.md` **top-down matrix and intro** (SSS-REQ-015 row; planned
  set and parent list) — R3-03/R3-04;
- the **GEM 802.1Qav feasibility** reconciliation in SRS-REQ-066 — R3-05;
- the **link-selection semantics** (SSS-REQ-028 vs SRS-REQ-060/061) — R3-06;
- the **IP protocol binding** (SRS-REQ-060) — R3-07;
- the new **SRS-REQ-067** (generic fabric-device FIC/AXI contract) and
  **SRS-REQ-068** (gPTP 802.1AS + fabric CoreTSN / 802.1CB) and their
  sprint/objective scheduling — R3-08/R3-09;
- the **loop-audit N3 disposition** and the **SRS-REQ-059 TBC owner** —
  R3-10/R3-11; the **planned-item method/evidence** statement — R3-12;
- the R4-round fixes that touch the requirements/planning scope
  (`traceability.md` intro parent list; `network-stack.md` header requirements;
  `driver-development.md` link-default mapping; README/`status.json`);
- the **gate re-run** (`tools/scripts/trace.py`) and the requirements-related
  documentation consistency gate (`tools/scripts/metrics.py --check`).

**Excluded (not assessed here):** the technical correctness of the DDJF
documentation rewrite and of the user-facing TCP/IP/TSN sections (profile R4,
norm N17 — `FRICK-REVIEW-0138`); code, build, tests and V&V evidence (none
changed by this increment); the open ECSS NC `PA-67-01` (SDD design matrices,
tracked to S0/S1). The increment is requirements/planning-only; no executable
artifact changed, so no Lean/V&V re-run is required by it (N1/N15 apply to the
touched documentation/requirements scope).

## 2. References

- Project norms: `docs/rules.md` (N1–N20); reviewer contract and NC loop:
  `docs/engineering/independent-review.md` (norm N9/N12/N14).
- Prior reviews: `docs/reviews/2026-10-12-m4-restored-scope-r3-review.md`
  (`FRICK-REVIEW-0137`), `docs/reviews/2026-10-12-m4-restored-scope-r4-review.md`
  (`FRICK-REVIEW-0138`).
- Traceability manifest: `docs/requirements/traceability.yaml`; matrices:
  `docs/requirements/traceability.md`; gates: `tools/scripts/trace.py`,
  `tools/scripts/metrics.py` (V&V tool T13).
- ECSS-E-ST-40C (requirements/architecture process, decomposition,
  traceability); ECSS-Q-ST-80C (assurance); ECSS-E-ST-70-41C (PUS-C);
  CCSDS 133.0-B-2 (Space Packet); CCSDS 702.1-B (IP over CCSDS, context only).
- `docs/reviews/review-template.md`; `docs/reviews/README.md`.

**Gate re-runs (evidence, reproduced first-hand on the fixed working tree).**

1. `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` →
   `traceability: PASS  (112 requirements, 69 srs, 29 sss, 14 gaps, 51 lean
   theorems checked)`, exit 0 — as expected by the task. The bottom-up summary
   shows `SSS-REQ-029 <- 4 srs children` and `SSS-REQ-015 <- 2 srs children`
   (norm N4).
2. `python3 tools/scripts/metrics.py --check` → **`error: README metrics panel
   is stale - run 'make metrics'`, exit 1** (see R3-D04; the only differing row
   is `Sprints completed`).

## 3. Prior-finding closure (R3-01..R3-12)

Status is based on first-hand inspection of the fixed working tree (not on the
author's summary). "Regression?" records whether the fix introduced a new defect.

| # | Item | Status | Evidence seen | Regression? |
| - | ---- | ------ | ------------- | ----------- |
| R3-01 | SSS-REQ-029 under-decomposed | **CLOSED** | `traceability.yaml` re-parents SRS-REQ-062 and SRS-REQ-065 to `SSS-REQ-029`; gate bottom-up `SSS-REQ-029 <- 4 srs children` (062/063/064/065) matching the four components named in the SSS-REQ-029 text; each component now has a singular, verifiable SRS child. | No (but see R3-D01 on SSS-REQ-015). |
| R3-02 | PUSopen status contradiction / stale default backend | **CLOSED** (text scope) | SSS-REQ-015 now reads "the open PUS backend, the N7/ESA ASN.1 default backend, and a documented adapter for the commercial PUSopen(R) library (planned; the commercial library itself is user-supplied)"; SRS-REQ-018 now reads "the N7/ESA ASN.1 backend by default (SRS-REQ-050); the open backend is an alternative; PUSopen(R) is an optional drop-in adapter (planned, SRS-REQ-065)"; SRS-REQ-065 is "planned" in the manifest (`ci:docs`). The three documents now state one status. | Yes (small): the manifest **`justification` of SRS-REQ-018 still says "open default"** — see R3-D02. |
| R3-03 | `traceability.md` §1 omits SRS-REQ-050 | **CLOSED** | §1 SSS-REQ-015 row = "SRS-REQ-018, SRS-REQ-050"; a programmatic comparison of all 29 §1 rows against the manifest `parent_sss` values found **0 mismatches** (the whole top-down table is now manifest-consistent). | No. |
| R3-04 | Stale "five planned" statement | **CLOSED** | `traceability.md` intro now: "The planned requirements are documented-only with `ci:docs` … : M12..M16 (SRS-REQ-045/046/051/053/054 with SSS parents SSS-REQ-022/023/024/026/027) and the restored M4-residual/M8 scope (SRS-REQ-059..068 with SSS parents SSS-REQ-017/020/028/029)". The parent set matches the manifest (059/060/061→028, 062-065→029, 066/068→020, 067→017). | No. |
| R3-05 | SRS-REQ-066 Qav feasibility unreconciled | **CLOSED** | SRS-REQ-066 now ends "feasibility on the GEM register subset used by the reference backend (which does not currently expose CBS) = TBC"; the manifest `lean_note` for 066 repeats the TBC. The over-claim against SRS-REQ-038 ("credit-based shaping (802.1Qav) is documented as not exposed in the GEM register subset") is removed. | Yes (small): the new TBC carries no owner/decision milestone — see R3-D06. |
| R3-06 | "at integration time" vs run-time link select | **CLOSED** (contradiction removed) | SSS-REQ-028 now reads "the active link selectable by the integrator" (the explicit "at integration time" claim is gone); SRS-REQ-060/061 keep `FF_LinkSelect` and the user manual §7.1 documents run-time `FF_LinkSelect`. | Yes (small): the manifest `lean_note` of SSS-REQ-028 still says "link selectable at integration time" — see R3-D03. |
| R3-07 | "TCP/IP" vs IP/UDP ambiguity | **CLOSED** (requirement bound) | SRS-REQ-060 now: "an IP-based link (TCP or UDP as selected by the mission ICD; the reference uses UDP datagrams)". The transport is now a stated mission-ICD input with a fixed reference, so the requirement is testable. | Yes (cosmetic): "TCP/IP" remains in the SSS-REQ-028 text and in the SRS-REQ-060 title — see R3-D05. |
| R3-08 | Generic fabric-device contract prose-only | **CLOSED** | New **SRS-REQ-067** "Generic fabric-device contract" (FIC/AXI), `parent_sss: SSS-REQ-017`, `method: Inp`, manifest entry present, `traceability.md` §1 SSS-REQ-017 row includes 067, sprint **S4c** delivers it. | No (see R3-D07 on the implementation links). |
| R3-09 | M8 gPTP/CoreTSN has no SRS requirement; wrong citation | **CLOSED** | New **SRS-REQ-068** "gPTP (802.1AS) and fabric CoreTSN completion", `parent_sss: SSS-REQ-020`, manifest entry present; sprint S13-S14 now cites "(SRS-REQ-038, SRS-REQ-068) + GEM TSN 802.1Qav/Qci/Qbb (SRS-REQ-066)" — the wrong SRS-REQ-037 citation is gone. | No (see R3-D07). |
| R3-10 | Loop-audit N3 disposition stale | **CLOSED** | `loop-audit.md` N3 row: "compliant (112 reqs) … tracked: M4-residual/M8/M12-M16"; the M4 row also records the restored scope. | No (a related baseline-currency nit → R3-D08). |
| R3-11 | SRS-REQ-059 TBC owner | **CLOSED** | SRS-REQ-059 now: "profile = mission input, TBC; owner: M4-residual" (owner + milestone label present; the manifest `lean_note` records the TBC). | No (but see R3-D06 for the analogous SRS-REQ-066 TBC). |
| R3-12 | Method vs `ci:docs` divergence | **CLOSED** (visibility) | `traceability.md` intro states the planned items are "documented-only with `ci:docs` as evidence until their implementation lands"; the per-requirement rows carry the same note. The divergence stays visible for closure at delivery (N4/N11). | No. |

**Net closure:** all 12 prior R3 findings are **CLOSED**; four small residuals of
the same classes remain and are raised as new findings below. The R4-round fixes
verified in this scope also hold: `traceability.md` intro parent list
(SSS-REQ-017/020/028/029), `network-stack.md` header requirements
(SRS-REQ-059/060/061), `driver-development.md` default-link mapping (MPFS **or**
QEMU → UART), README 802.1AS "time-read seam" qualifier, and the
user-manual history wording.

## 4. New findings (delta)

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R3-D01 | Minor | SSS-REQ-015 vs SSS-REQ-029 | **Residual of the R3-01 class: SSS-REQ-015 still names the PUSopen adapter, but the adapter is decomposed only under SSS-REQ-029.** SSS-REQ-015 text names three components ("the open PUS backend, the N7/ESA ASN.1 default backend, and a documented adapter for the commercial PUSopen(R) library"), while its only children are SRS-REQ-018 (open backend) and SRS-REQ-050 (ASN.1 default); SRS-REQ-065 (PUSopen adapter) is parented to SSS-REQ-029. The re-parenting that closed R3-01 therefore moved the "component named in the SSS text but not decomposed under it" defect from SSS-REQ-029 to SSS-REQ-015. | `SSS.md` §3 SSS-REQ-015; `traceability.yaml` SRS-REQ-065 `parent_sss: SSS-REQ-029`; gate output `SSS-REQ-015 <- 2 srs children`. | Restrict SSS-REQ-015's text to the open backend and the N7/ESA ASN.1 default and cross-reference SSS-REQ-029 for the PUSopen adapter (the manifest supports one parent per SRS, so cross-referencing is the correct fix), or accept the adapter under SSS-REQ-015 and remove it from SSS-REQ-029 (N4). |
| R3-D02 | Minor | `traceability.yaml` SRS-REQ-018 `justification` | **Residual of the R3-02 class: the manifest justification still asserts the "open default".** `justification: "ADR-0003: abstraction layer with open default and optional PUSopen adapter."` contradicts the reconciled SSS-REQ-015, SRS-REQ-018 text, SRS-REQ-050 and ADR-0016 (N7/ESA ASN.1 is the default). The R4 review raised the same item (`FRICK-REVIEW-0138` R4-02, open) and the fix touched only the SRS text. | `traceability.yaml` SRS-REQ-018 (`justification`, line 190); `SSS.md` SSS-REQ-015; `SRS.md` SRS-REQ-050; `docs/adr/ADR-0016-pus-c-asn1-default.md`. | Correct the SRS-REQ-018 manifest justification to "N7/ESA ASN.1 default (SRS-REQ-050, ADR-0016); open backend as fallback; PUSopen adapter optional" (N4/N8). |
| R3-D03 | Minor | `traceability.yaml` SSS-REQ-028 `lean_note` | **Residual of the R3-06 class: the manifest note still says "link selectable at integration time"**, contradicting the edited SSS-REQ-028 text ("selectable by the integrator") and SRS-REQ-060/061 (`FF_LinkSelect`, run-time). In addition, "selectable by the integrator" is not defined (integration-time configuration vs run-time API call), so the selection semantics are still stated in three places with three wordings. | `traceability.yaml` SSS-REQ-028 `lean_note` (line 856, the only remaining "integration time" occurrence in `docs/`); `SSS.md` SSS-REQ-028; `SRS.md` SRS-REQ-060/061; `user-integration-manual.md` §7.1 (`FF_LinkSelect` run time). | Fix the manifest note and state the semantics once, e.g. "the default backend is platform-selected at build time; the integrator selects the active backend at run time via `FF_LinkSelect`", and use the same wording in the SSS, the SRS children and the manual (N4/N8). |
| R3-D04 | Minor | `README.md` metrics panel vs `docs/status/status.json` | **Regression introduced by the R4-05 fix: the metrics panel is stale and its gate fails.** `status.json` `sprints` was changed to `{"total": 29, "done": 4}` but the README panel was not regenerated: the panel row reads `Sprints completed 3/26 (11.5%)` while the generator renders `4/29 (13.8%)` (it is the **only** differing row; the 29 other rows match). `python3 tools/scripts/metrics.py --check` (V&V tool T13, documentation-consistency gate) exits 1 with "README metrics panel is stale". This currently **blocks commit/push** under N7/N15 until `make metrics` is run. Separately, the increment from 3 to 4 completed sprints is not traceable in the plan: `sprint-plan.md` marks only S1/S2/S3 delivered, S0's exit criterion (PA-67-01) is still open per `loop-audit.md`, and S4 is not delivered. | `README.md` (Sprints completed row); `docs/status/status.json` `sprints`; `tools/scripts/metrics.py` (`--check` output, exit 1); `docs/planning/sprint-plan.md` §2/§3; `docs/status/loop-audit.md` (PA-67-01 "tracked … S0/S1"). | Run `make metrics` (regenerate the panel) and reconcile `sprints.done` with the plan (S0..S3 vs S1..S3), then re-run `metrics.py --check` until it passes (N8/N10/N11/N19). |
| R3-D05 | Comment | SSS-REQ-028 / SRS-REQ-060 title | **Residual shorthand of the R3-07 class:** the SRS-REQ-060 **title** ("TCP/IP PUS-C link backend") and the SSS-REQ-028 text ("IP-based links (TCP/IP)") still say TCP/IP, while the requirement body now binds the transport to "TCP or UDP as selected by the mission ICD; the reference uses UDP datagrams". The body is correct and testable; the shorthand could re-introduce the ambiguity. | `SRS.md` §5 SRS-REQ-060; `SSS.md` SSS-REQ-028. | Harmonise the wording (e.g. "IP (UDP reference)" in the title and "IP-based links" in the SSS), or add a parenthetical pointer to the mission ICD input (N8). |
| R3-D06 | Comment | SRS-REQ-066 | **The new feasibility TBC carries no owner or decision milestone** (the R3-11 class, now applied to 066): "feasibility on the GEM register subset … = TBC" has no `owner:` field (SRS-REQ-059 now has one) and the manifest note has none either. Per N19 no TBC should be silent. | `SRS.md` SRS-REQ-066; `traceability.yaml` SRS-REQ-066 `lean_note`. | Add the owner/milestone to the 066 TBC (e.g. "TBC; owner: M8") and keep it in `loop-audit.md` dispositions (N19). |
| R3-D07 | Comment | Manifest `implementation` links for SSS-REQ-028/029, SRS-REQ-067/068 | The manifest lists `docs/development-plan.md` and `docs/roadmap.md` as implementation evidence for these requirements, but **those documents do not reference the requirement ids** (only `docs/planning/sprint-plan.md` does: SSS-REQ-028/029 = 0 occurrences in both; SRS-REQ-067/068 = 0). The linked documents do describe the scope (roadmap M4 block, development-plan §2/§4 and M8), so the link is substantively right but not verifiable by the id. The same pattern pre-exists for SRS-REQ-053 / SSS-REQ-026 (planned items), so this is a weak evidence link rather than a new divergence. | `traceability.yaml` implementation lists for SSS-REQ-028/029 and SRS-REQ-067/068; `traceability.md` §2/§3 rows; id-occurrence counts in `development-plan.md` / `roadmap.md` (0) vs `sprint-plan.md` (1). | Cite the requirement ids in the development-plan/roadmap sections (or narrow the implementation lists to the documents that reference them), consistently for all planned items (N4). |
| R3-D08 | Comment | `docs/status/loop-audit.md` header | Rev 0.14 audits the restored-M4 increment, but the `Baseline commit` field still reads "db8261f + Sprint-3 MSS I2C increment (audited revision)" (i.e. the previous loop's revision), while the reviewed revision is `ce0bff3` + working tree. History 0.2 records "re-baselined at …" as the established practice. | `loop-audit.md` header (rev 0.14) and `## History` (0.14). | Re-baseline the audit header to the revision actually audited in this loop (N16/N19). |
| R3-D09 | Comment | `docs/vv/verification-results.md` | No cycle row was appended for this increment, so the last recorded trace evidence (`100 reqs / 59 srs / 27 sss / 14 gap`) no longer matches the manifest and the reproduced gate (`112/69/29/14`). Comparable docs/requirements-only increments (e.g. "Renode board repl enabled") did append a row. | `verification-results.md` last row (M4/M11 MSS I2C, 100/59/27/14/51); current gate output (112/69/29/14/51); `docs/rules.md` N11/N15. | Append a cycle row for this increment (build/test/Lean unchanged; trace 112/69/29/14/51) so the running record stays current (N11/N15); ownership R2/PA. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must be
  corrected or justified.
- **Comment**: observation, recommendation, no derogation.

**Positive confirmations (verified, no finding).**

- The traceability gate passes with the expected counts: **112 requirements
  (69 srs / 29 sss / 14 gaps), 51 Lean theorems, exit 0** (norm N4).
- The whole top-down matrix (§1, 29 rows) now matches the manifest `parent_sss`
  values exactly (programmatic comparison, 0 mismatches) — R3-03 closed without
  residue.
- The re-parenting of SRS-REQ-062/065 is coherent with the SSS-REQ-029 text (all
  four named components have a child) and with the new SRS-REQ-067 contract they
  build on.
- The new SRS-REQ-067/068 are scheduled where the scope belongs (S4c; S13-S14)
  and the wrong SRS-REQ-037 citation is removed; roadmap / development-plan /
  sprint-plan agree on the M4-residual and M8 placements.
- The RS-485 DE/RE residual remains correctly handled: SRS-REQ-064 planned, the
  contract hook exists (SRS-REQ-030), the MSS backend records the request without
  driving the line, and the on-target residual is recorded in GAP-A5
  (`class-a-gaps.md` 0.8) consistently with the manifest `lean_note`.
- No over-claim was introduced: `network-stack.md` §6, the user manual §7.1/§8/§10
  and `driver-development.md` §2 still state explicitly that only loopback and
  UART link backends ship and that the IP/TSN/CAN/SpaceWire link backends are
  **not implemented** (planned).
- The file-transfer set (VFS / PUS-C S23 / CFDP) remains explicitly planned in
  M15, not silently dropped.

## 5. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**Conclusion: APPROVED WITH COMMENTS.** No Major finding. All twelve prior R3
findings (R3-01..R3-12) are **CLOSED** on first-hand evidence, and no prior
finding regressed. The delta raises **four new Minor findings** (R3-D01..R3-D04)
— four small residuals of the same classes already dispositioned (SSS/SRS text
vs decomposition, manifest justification/wording, and the metrics panel) — and
five Comments (R3-D05..R3-D09). Per norm N12 the new NCs must be dispositioned
and this profile must re-review the deltas; per norm N7 the increment must not be
committed/pushed while the `metrics.py --check` gate fails (R3-D04). No
certification, airworthiness, human-rating or qualification claim is made or
implied by this review.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R3 (N9/N14)   | Senior Systems / avionics engineer (independent agent, norm N9) | 2026-10-12 | APPROVED WITH COMMENTS |

## 7. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing the
deltas until full compliance.)

### 7.1 Prior round (FRICK-REVIEW-0137) — closure record

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | ------ | --------------- |
| R3-01 | Minor | SSS-REQ-029 | Under-decomposed scope | Fixed: 062/065 re-parented to SSS-REQ-029 | `traceability.yaml`; gate `SSS-REQ-029 <- 4 srs children` | frick-fsw | **closed** | FRICK-REVIEW-0139 (this report) |
| R3-02 | Minor | SSS-REQ-015 / SRS-REQ-018/065 | PUSopen status; stale default backend | Fixed in SSS/SRS texts | `SSS.md` 0.14; `SRS.md` 1.18; manifest 065 | frick-fsw | **closed** (residual → R3-D02) | FRICK-REVIEW-0139 |
| R3-03 | Minor | `traceability.md` §1 | Matrix omitted SRS-REQ-050 | Fixed; whole §1 table verified against the manifest | §1 row; programmatic check 0/29 mismatches | frick-fsw | **closed** | FRICK-REVIEW-0139 |
| R3-04 | Minor | `traceability.md` intro | Stale "five planned" | Fixed: both planned sets listed with parents | intro lines 19-24 | frick-fsw | **closed** | FRICK-REVIEW-0139 |
| R3-05 | Minor | SRS-REQ-066 / 038 | Qav feasibility unreconciled | Fixed: feasibility = TBC on the GEM register subset | `SRS.md` 066; manifest `lean_note` | frick-fsw | **closed** (residual → R3-D06) | FRICK-REVIEW-0139 |
| R3-06 | Minor | SSS-REQ-028 / SRS-REQ-060/061 | Integration-time vs run-time selection | Fixed in the SSS text ("by the integrator") | `SSS.md` 028 | frick-fsw | **closed** (residual → R3-D03) | FRICK-REVIEW-0139 |
| R3-07 | Minor | SSS-REQ-028 / SRS-REQ-060 | TCP/IP vs IP/UDP ambiguity | Fixed in the SRS body (mission-ICD input; reference UDP) | `SRS.md` 060 | frick-fsw | **closed** (residual → R3-D05) | FRICK-REVIEW-0139 |
| R3-08 | Minor | SRS-REQ-062 (+066) | Fabric contract prose-only | Fixed: new SRS-REQ-067 (parent SSS-REQ-017, S4c) | manifest 067; §1 row; sprint S4c | frick-fsw | **closed** (residual → R3-D07) | FRICK-REVIEW-0139 |
| R3-09 | Minor | M8 / sprint S13-S14 | gPTP/CoreTSN no SRS req; wrong citation | Fixed: new SRS-REQ-068; citation corrected | manifest 068; sprint S13-S14 | frick-fsw | **closed** (residual → R3-D07) | FRICK-REVIEW-0139 |
| R3-10 | Minor | `loop-audit.md` N3 row | Stale disposition | Fixed: "tracked: M4-residual/M8/M12-M16" | `loop-audit.md` N3 row | frick-fsw | **closed** (nit → R3-D08) | FRICK-REVIEW-0139 |
| R3-11 | Comment | SRS-REQ-059 | Open TBC without owner | Fixed: "owner: M4-residual" | `SRS.md` 059 | frick-fsw | **closed** (residual → R3-D06) | FRICK-REVIEW-0139 |
| R3-12 | Comment | SRS-REQ-059…068 | Method vs `ci:docs` divergence | Documented in `traceability.md`; close at delivery | intro lines 19-24 | frick-fsw | **closed** (visibility) | FRICK-REVIEW-0139 |

### 7.2 New NCs (this delta)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | ------ | --------------- |
| R3-D01 | Minor | SSS-REQ-015 vs SSS-REQ-029 | PUSopen adapter named in SSS-REQ-015 but decomposed only under SSS-REQ-029 | To be dispositioned (restrict text + cross-reference, or move the child) | — | frick-fsw | open | — |
| R3-D02 | Minor | `traceability.yaml` SRS-REQ-018 | Justification still says "open default" (contradicts SSS-REQ-015 / SRS-REQ-050 / ADR-0016) | To be dispositioned | — | frick-fsw | open | — |
| R3-D03 | Minor | `traceability.yaml` SSS-REQ-028 | `lean_note` still "link selectable at integration time"; integrator-select semantics undefined | To be dispositioned | — | frick-fsw | open | — |
| R3-D04 | Minor | README panel vs `status.json` | Stale metrics panel; `metrics.py --check` exit 1 (blocks commit/push); `sprints.done=4` not traceable to the plan | To be dispositioned (`make metrics` + reconcile `done`) | — | frick-fsw | open | — |
| R3-D05 | Comment | SSS-REQ-028 / SRS-REQ-060 title | "TCP/IP" shorthand vs the body's UDP reference | Recommended | — | frick-fsw | open | — |
| R3-D06 | Comment | SRS-REQ-066 | Feasibility TBC without owner/milestone | Recommended | — | frick-fsw | open | — |
| R3-D07 | Comment | SSS-REQ-028/029, SRS-REQ-067/068 manifest links | `development-plan.md`/`roadmap.md` listed but do not cite the ids | Recommended | — | frick-fsw | open | — |
| R3-D08 | Comment | `loop-audit.md` header | `Baseline commit` still the previous loop's revision | Recommended | — | frick-fsw | open | — |
| R3-D09 | Comment | `docs/vv/verification-results.md` | No cycle row for this increment; recorded trace no longer matches the manifest | Recommended (R2/PA) | — | frick-fsw | open | — |

## 8. Facts / Assumptions / Inferences / Recommendations / Confidence / Open gaps

**Facts (verified with evidence):**

- `trace.py` passes on the fixed tree: 112 requirements (69 srs / 29 sss /
  14 gaps), 51 Lean theorems, exit 0.
- `metrics.py --check` fails: exit 1, "README metrics panel is stale"; the only
  differing row is `Sprints completed` (README `3/26` vs generated `4/29`).
- SSS-REQ-029 has four SRS children (062/063/064/065); SSS-REQ-015 has two
  (018/050) and its text still names the PUSopen adapter.
- The manifest `justification` of SRS-REQ-018 still reads "open default".
- The manifest `lean_note` of SSS-REQ-028 still reads "link selectable at
  integration time"; SSS-REQ-028 now reads "selectable by the integrator";
  SRS-REQ-060/061 and the user manual use run-time `FF_LinkSelect`.
- SRS-REQ-060 now binds the transport to the mission ICD ("TCP or UDP … reference
  UDP"); its title still says "TCP/IP".
- SRS-REQ-066 states the CBS feasibility as TBC (no owner); SRS-REQ-059 carries
  "owner: M4-residual".
- All 29 rows of `traceability.md` §1 match the manifest `parent_sss` values.
- The manifest lists `docs/development-plan.md` and `docs/roadmap.md` as
  implementation evidence for SSS-REQ-028/029 and SRS-REQ-067/068, but those
  documents contain no occurrence of those ids.
- The `loop-audit.md` rev-0.14 `Baseline commit` still names the previous loop's
  revision; `verification-results.md` has no row for this increment.

**Assumptions:**

- The material under review is exactly `ce0bff3` + the uncommitted working tree
  (confirmed with `git status`); the increment is requirements/planning/documentation-only.
- "M4-residual" is a re-plan of the original M4 scope scheduled after sprint S4;
  the new SSS/SRS items are planned deliverables, so `ci:docs` is the expected
  evidence until delivery (the R3-12 convention).

**Inferences:**

- The residuals R3-D01..R3-D03 are local wording/decomposition leftovers of an
  otherwise complete reconciliation; they do not invalidate any requirement or
  the delivered baseline, and are correctable without rework.
- R3-D04 is a fix-induced inconsistency (the R4-05 disposition edited
  `status.json` without regenerating the panel); it is mechanical but currently
  blocks commit/push, so it must be corrected in the same increment.
- Because the increment adds only planned (documented-only) requirements, no
  code/V&V/Lean regression is possible from it; the applicable review scope
  remains requirements/planning/documentation coherence.

**Recommendations:**

- Correct R3-D01..R3-D04 with recorded dispositions and re-run this profile's
  delta review (N12) before commit/push; track R3-D05..R3-D09 to closure.
- Prefer manifest-derived generation of `traceability.md` §1 and of the planned-set
  statement (it already removed one class of divergence; deriving it prevents
  recurrence).
- Apply the "TBC carries an owner and a decision milestone" rule uniformly to all
  new TBCs (059 and 066) and record them in `loop-audit.md` dispositions (N19).
- Log this review and the R3/R4 reports in `docs/reviews/README.md` in the same
  commit (N11/N17), and append the cycle row to `verification-results.md` (N11).

**Confidence:** **High** for the closure/verification facts and for R3-D01..R3-D04
(each reproduced directly from the files and the two gates). **Medium** for the
`sprints.done` question inside R3-D04 (the plan does not define the denominator/
numerator convention explicitly; the panel/`status.json` divergence itself is a
fact). **Medium-High** for the scheduling/feasibility comments (R3-D05..R3-D07),
which depend on documentation wording and milestone intent.

**Open gaps (not closed by this review):**

- The exact GEM TSN register/device basis for 802.1Qav/Qci/Qbb (SRS-REQ-066) —
  now correctly a TBC, but without an owner (R3-D06).
- The definition and ownership of the generic fabric-device (FIC/AXI) contract
  (SRS-REQ-067) — requirement exists; the document-level evidence links are weak
  (R3-D07).
- The mission-input IP/TSN encapsulation profile for SRS-REQ-059 (TBC, owner
  M4-residual) — open by design.
- The open ECSS NC `PA-67-01` (SDD design matrices) — outside this increment's
  scope, tracked to S0/S1.

---

**Independence declaration (norm N9/N12).** I performed this delta re-review in
an independent, fresh context with no part in authoring or executing the items
under review (I did not author the initial R3 report's findings, the fixes, or any
reviewed document), based only on the evidence in the repository at `ce0bff3` +
the uncommitted working tree and the reference documents. I am read-only for the
repository and, per norm N11 and the task, I wrote **exactly one** file — this
report — and modified, created or deleted no other file in the repository; I did
not commit or push. (Read-only command output was written outside the repository
under `/tmp/opencode/`.)

Committed and pushed with the reviewed change (norm N11).
