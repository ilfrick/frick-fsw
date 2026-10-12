# Review Report — M4 restored scope (requirements / planning increment)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                           | frick-fsw                              |
| Document id                       | FRICK-REVIEW-0137                      |
| Reviewer session                  | independent agent, norm N9 (fresh context) |
| Reviewer profile                  | R3 — Senior Systems / avionics engineer (space/aeronautics) |
| Reference revision                | `ce0bff3` + uncommitted working tree   |
| Items under review                | `docs/requirements/SSS.md` (0.14), `docs/requirements/SRS.md` (1.18), `docs/requirements/traceability.yaml` + `docs/requirements/traceability.md` (0.30), `docs/roadmap.md` (0.18), `docs/development-plan.md` (0.20), `docs/planning/sprint-plan.md` (0.6), `docs/quality/class-a-gaps.md` (0.8), `docs/architecture/network-stack.md` (0.4), `docs/engineering/user-integration-manual.md` (0.6), `docs/engineering/driver-development.md` (0.4) |
| Date                              | 2026-10-12                             |
| Scope                             | Requirements/architecture coherence, traceability (top-down + bottom-up), requirement well-formedness, and milestone/sprint scheduling of the restored M4-residual / M8 scopes; assessed against ECSS-E-ST-40C (requirements and architecture process), ECSS-E-ST-70-41C (PUS-C) and CCSDS 133.0-B-2, under project norms N1–N20 |
| Independence declaration          | The reviewer had no part in authoring or executing the items under review; it ran in a fresh context with only the material under review and the reference documents. It is read-only for the repository except for this single report. |
| Norms applied                     | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

Reviewed the **requirements/planning increment** that converts previously "absorbed"
prose scopes into explicit, scheduled deliverables:

- new **SSS-REQ-028** (PUS-C transport over IP and TSN) and **SSS-REQ-029**
  (extended fieldbus + PUS-library coverage); **SSS-REQ-015** reconciled;
- new **SRS-REQ-059 … 066** (PUS-C segmentation/reassembly; TCP/IP and TSN
  `FF_Link_t` backends; CAN-FD fabric backend; CANopen stack; RS-485 DE/RE;
  PUSopen adapter; GEM TSN Qav/Qci/Qbb);
- manifest + matrices (`traceability.yaml`, `traceability.md` 0.30, counts
  67 srs / 29 sss / 14 gap);
- `docs/roadmap.md` (M4 section, OBJ-25…OBJ-30, M8 extension),
  `docs/development-plan.md` (§2, §4, §6), `docs/planning/sprint-plan.md`
  (S4b/S4c/S4d, S13-S14);
- `docs/quality/class-a-gaps.md` GAP-A5 RS-485 DE/RE residual;
- the "absorbed" wording removals in `network-stack.md`,
  `user-integration-manual.md`, `driver-development.md`.

**In scope:** requirement well-formedness (singular / testable / parented /
method-annotated / bounded / free of over-claim); parent/child coherence;
manifest ↔ matrix ↔ requirement-text agreement; top-down and bottom-up
convergence; sprint/roadmap/development-plan scheduling coherence; accuracy of
the PUS-C / IP / TSN statements; PUSopen reconciliation; RS-485 DE residual
handling; re-run of the traceability gate.

**Excluded (out of this increment's scope, not assessed here):** the DDJF
acronym registration and the PUS-C transport-channel documentation in
`docs/engineering/software-description.md` (0.3) / `docs/glossary.md` (0.2)
(documentation review is profile R4, norm N17); code, build, test scripts and
V&V evidence (none changed by this increment); the open ECSS NC PA-67-01 (SDD
design matrices). The increment is requirements/planning-only; no executable
artifact changed, so no Lean/V&V re-run was required by it (N1/N15 apply to the
touched documentation scope).

## 2. References

- Project norms: `docs/rules.md` (N1–N20); reviewer contract:
  `docs/engineering/independent-review.md` (norm N9/N14).
- Traceability manifest: `docs/requirements/traceability.yaml`; matrices:
  `docs/requirements/traceability.md`; gate: `tools/scripts/trace.py`.
- ECSS-E-ST-40C (requirements/architecture process, requirements decomposition
  and traceability); ECSS-Q-ST-80C (assurance); ECSS-E-ST-70-41C (PUS-C);
  CCSDS 133.0-B-2 (Space Packet protocol).
- `docs/reviews/review-template.md`; `docs/reviews/README.md`.

**Gate re-run (evidence).** `python3 tools/scripts/trace.py docs/requirements/traceability.yaml`
→ `traceability: PASS (110 requirements, 67 srs, 29 sss, 14 gaps, 51 lean
theorems checked)`, exit 0. This confirms the manifest is internally convergent
and consistent with the SRS/SSS text (norm N4).

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R3-01 | Minor | SSS-REQ-029 | **Non-singular system requirement with an under-decomposed scope.** SSS-REQ-029 text names four components (CAN-FD, CANopen, RS-485 DE/RE, PUSopen adapter) but its SRS children cover only CANopen (SRS-REQ-063) and RS-485 (SRS-REQ-064). CAN-FD (SRS-REQ-062) is parented to SSS-REQ-020 and the PUSopen adapter (SRS-REQ-065) to SSS-REQ-015, so two of the four components named in the SSS text are not decomposed under it. | `SSS.md` §2 (SSS-REQ-029); `traceability.yaml` (062→020, 063→029, 064→029, 065→015); `traceability.md` §1 row SSS-REQ-029. | Split SSS-REQ-029 into singular requirements, or restrict its text to the components it owns and cross-reference SSS-REQ-020/SSS-REQ-015 for CAN-FD and the PUSopen adapter. (ECSS-E-ST-40C singular/verifiable requirements; N4.) |
| R3-02 | Minor | SSS-REQ-015 / SRS-REQ-018 / SRS-REQ-065 | **PUSopen adapter delivery status is contradictory, and SSS-REQ-015 still says "open PUS backend (default)".** The edited SSS-REQ-015 states "the adapter glue is in-repo and documented"; the new SRS-REQ-065 is tracked as **planned** (`ci:docs` only); SRS-REQ-018 (implemented) already lists `src/pus/adapters/pus_adapter_pusopen.c` as its implementation. Separately, SSS-REQ-015's "open PUS backend (default)" conflicts with SRS-REQ-050 / ADR-0016 (the N7/ESA ASN.1 backend is the default). | `SSS.md` §3 (SSS-REQ-015); `SRS.md` §5 (SRS-REQ-050, SRS-REQ-065); `traceability.yaml` SRS-REQ-018/050/065 + SSS-REQ-015 justification; `src/pus/adapters/pus_adapter_pusopen.c` (compile-guarded integration point, no test). | State one delivery status for the adapter across SSS-REQ-015 / SRS-REQ-018 / SRS-REQ-065 (e.g. "glue present and documented; adapter qualification/test is the M4-residual item"), and correct the SSS-REQ-015 default-backend wording to the N7/ESA default. |
| R3-03 | Minor | `traceability.md` §1 vs manifest | **Top-down matrix omits SRS-REQ-050 under SSS-REQ-015.** The manifest (and the gate bottom-up summary) show SSS-REQ-015 with three children (018, 050, 065); the matrix row lists only 018 and 065, although the row was edited in this increment. The document states it "mirrors its current content". | `traceability.md` §1 (SSS-REQ-015 row); `traceability.yaml` SRS-REQ-050 `parent_sss: SSS-REQ-015`; gate output `SSS-REQ-015 <- 3 srs children`. | Regenerate/align the matrix row to include SRS-REQ-050 (and re-check the whole top-down table against the manifest). |
| R3-04 | Minor | `traceability.md` header note | **Stale "planned" statement.** The intro says "Five of the requirements (SRS-REQ-045/046/051/053/054 …) are planned (M12..M16)"; after this increment eight further requirements (SRS-REQ-059…066) are also planned (M4-residual / M8) with `ci:docs` evidence. | `traceability.md` lines 18–24; manifest SRS-REQ-059…066 (`ci:docs`, planned). | Update the planned-set statement (or make it manifest-derived) so it reflects the current planned set and milestones. |
| R3-05 | Minor | SRS-REQ-066 vs SRS-REQ-038 | **GEM 802.1Qav feasibility is not reconciled.** SRS-REQ-038 (implemented) states credit-based shaping (802.1Qav) "is documented as not exposed in the GEM register subset"; the GEM TSN backend returns `FF_NOT_IMPLEMENTED` for `configure_cbs`. The new SRS-REQ-066 requires "extend the GEM native TSN backend with 802.1Qav (credit-based shaping)". As written, 066 may be infeasible on the GEM endpoint unless the register basis is stated. | `SRS.md` §5 SRS-REQ-038 / SRS-REQ-066; `src/drivers/eth/gem/ff_gem_tsn.h` (header comment) and `ff_gem_tsn.c:102`; `docs/architecture/hardware-abstraction.md` (Qav CBS not exposed). | Reconcile 066 with 038: state the register/device documentation basis for CBS (extended register subset) or route Qav to the fabric CoreTSN path and document it; avoid an unstated over-claim. |
| R3-06 | Minor | SSS-REQ-028 vs SRS-REQ-060/061 | **Link-selection semantics disagree.** SSS-REQ-028 says the active link is "selectable at integration time"; SRS-REQ-060/061 say "selectable with `FF_LinkSelect`", and the existing link API selects at run time. | `SSS.md` SSS-REQ-028; `SRS.md` SRS-REQ-060/061; `docs/engineering/user-integration-manual.md` §7.1 (`FF_LinkSelect` run-time). | Align on one semantics (integration-time vs run-time) across the SSS, the SRS children and the user manual. |
| R3-07 | Minor | SSS-REQ-028 / SRS-REQ-060 | **"TCP/IP" vs IP/UDP ambiguity.** SSS-REQ-028 says "IP-based links (TCP/IP)"; SRS-REQ-060 is titled "TCP/IP … backend" but its body says "carries PUS-C TM/TC over IP … (e.g. UDP datagrams)". The bound transport is not fixed, which weakens testability. | `SSS.md` SSS-REQ-028; `SRS.md` SRS-REQ-060. | Fix the terminology (e.g. "IP/UDP" per the CCSDS IP-over-CCSDS context, CCSDS 702.1-B) or state explicitly that the protocol is a mission ICD input. |
| R3-08 | Minor | SRS-REQ-062 (and 066 fabric path) | **Enabling "generic fabric-device (FIC/AXI) contract" is prose-only.** SRS-REQ-062 reaches CAN-FD "through the generic fabric (FIC/AXI) device contract", but no such contract exists under `src/hal/` and no requirement/manifest entry tracks it; it is only described in `docs/development-plan.md` / `docs/roadmap.md`. This is the same class of "declared intent not explicitly tracked" that this increment set out to remove (N19). | `SRS.md` SRS-REQ-062; `src/hal/` listing (no fabric contract); `docs/development-plan.md` §3/§4 (generic fabric-device path, OBJ-07/OBJ-10); `traceability.yaml` (no fabric requirement). | Make the generic fabric-device contract an explicit deliverable (own requirement or a tracked objective with owner/milestone) before S4c/S4b depend on it. |
| R3-09 | Minor | M8 / `sprint-plan.md` S13-S14 | **M8 gPTP / fabric CoreTSN / 802.1CB scope has no SRS requirement, and the sprint cites the wrong requirement.** The S13-S14 row cites "(SRS-REQ-037/038)" for "802.1AS gPTP + fabric CoreTSN / 802.1CB"; SRS-REQ-037 is RTEMS application integration (unrelated) and SRS-REQ-038 covers only the GEM native TSN backend. The full gPTP stack and the fabric CoreTSN / 802.1CB path (OBJ-16) remain objective-only, unlike the sibling Qav/Qci/Qbb which now has SRS-REQ-066. | `docs/planning/sprint-plan.md` S13-S14; `SRS.md` SRS-REQ-037/038; `docs/development-plan.md` OBJ-16 / M8. | Add explicit SRS requirement(s) for the M8 gPTP/fabric CoreTSN/802.1CB scope (consistency with SRS-REQ-066), and correct the S13-S14 citation. |
| R3-10 | Minor | `docs/status/loop-audit.md` | **Stale N19 disposition.** The N3 row disposition reads "planned reqs documented-only → tracked: M12-M16"; the planned set now also includes the M4-residual / M8 requirements (059…066). | `docs/status/loop-audit.md` §2 (N3 row, rev 0.14). | Update the N3 disposition to include the M4-residual/M8 planned requirements (owner/milestone per N19). |
| R3-11 | Comment | SRS-REQ-059 | **Open `TBC` on the encapsulation profile.** SRS-REQ-059 correctly leaves the IP/TSN encapsulation profile as a mission input (`TBC`) rather than inventing a parameter (good, per N19); the requirement is therefore not fully bounded/verifiable until the profile is fixed. | `SRS.md` SRS-REQ-059; manifest `lean_note` for SRS-REQ-059. | Ensure the TBC carries an owner and a decision milestone in the plan/loop-audit so it does not become a silent gap (N19). |
| R3-12 | Comment | SRS-REQ-059…066 | **Method vs evidence divergence for planned items.** The new requirements declare `method: Tst | Dem` (or `Tst`), while the manifest gives `ci:docs` only. This is consistent with the established planned-item convention (045/046/…), so no action now; the divergence must close when the items are delivered. | `SRS.md` §5; `traceability.yaml` SRS-REQ-059…066. | Keep the planned items' method/evidence divergence visible and close it at delivery (N4/N11). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must be
  corrected or justified.
- **Comment**: observation, recommendation, no derogation.

**Positive confirmations (facts verified, no finding):**

- The traceability gate passes: 110 requirements (67 srs / 29 sss / 14 gaps),
  51 Lean theorems (norm N4). The manifest parents match the task statement
  (SSS-REQ-028→059/060/061; SSS-REQ-029→063/064; SSS-REQ-015→065; SSS-REQ-020→062/066).
- The PUS-C / IP / TSN statements were **not** over-claimed: `network-stack.md`
  §6, `user-integration-manual.md` §8/§10 and `driver-development.md` §2 now say
  "planned (explicit M4-residual scope, SRS-REQ-059/060/061)" and explicitly
  state the backends are **not implemented**; only loopback and UART ship today.
- The **RS-485 DE/RE residual is handled correctly**: SRS-REQ-064 is planned,
  the contract hook (`set_rs485`, SRS-REQ-030) exists, the MSS backend records
  the request without driving the line, and the on-target residual is recorded
  in GAP-A5 (`class-a-gaps.md` 0.8) — consistent with the manifest `lean_note`.
- The file-transfer set (VFS / PUS-C S23 / CFDP) remains explicitly planned in
  M15 (roadmap M4 note, development-plan M15, SRS-REQ-053), not silently dropped.
- The restored items are scheduled after S4 as S4b/S4c/S4d and the M8 row gains
  Qav/Qci/Qbb; roadmap / development-plan / sprint-plan are mutually consistent
  on the M4-residual and M8 placements (modulo R3-08/R3-09).

## 4. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**Conclusion: APPROVED WITH COMMENTS.** No Major finding. Ten Minor findings
(R3-01…R3-10) and two Comments (R3-11, R3-12) are raised and must be resolved
per norm N12 (disposition, evidence, delta re-review by this profile) before the
increment is committed/pushed. The requirement set is convergent under the
traceability gate and the restored scopes are now explicitly tracked; the Minors
concern internal coherence (parenting/decomposition, matrix mirror, delivery
status, transport semantics, enabling-contract and M8 requirement coverage)
rather than correctness of a delivered capability. No certification or design
approval is claimed by this review.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R3 (N9/N14)   | Senior Systems / avionics engineer (independent agent) | 2026-10-12 | APPROVED WITH COMMENTS |

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing the
deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | ------ | --------------- |
| R3-01 | Minor | SSS-REQ-029 | Non-singular / under-decomposed scope | To be dispositioned | — | frick-fsw | open | — |
| R3-02 | Minor | SSS-REQ-015 / SRS-REQ-018/065 | PUSopen status contradiction; stale default-backend wording | To be dispositioned | — | frick-fsw | open | — |
| R3-03 | Minor | traceability.md §1 | Matrix omits SRS-REQ-050 under SSS-REQ-015 | To be dispositioned | — | frick-fsw | open | — |
| R3-04 | Minor | traceability.md header | Stale "five planned" statement | To be dispositioned | — | frick-fsw | open | — |
| R3-05 | Minor | SRS-REQ-066 / 038 | GEM 802.1Qav feasibility not reconciled | To be dispositioned | — | frick-fsw | open | — |
| R3-06 | Minor | SSS-REQ-028 / SRS-REQ-060/061 | Integration-time vs run-time link selection | To be dispositioned | — | frick-fsw | open | — |
| R3-07 | Minor | SSS-REQ-028 / SRS-REQ-060 | TCP/IP vs IP/UDP ambiguity | To be dispositioned | — | frick-fsw | open | — |
| R3-08 | Minor | SRS-REQ-062 (+066) | Generic fabric-device contract prose-only | To be dispositioned | — | frick-fsw | open | — |
| R3-09 | Minor | M8 / sprint S13-S14 | gPTP/fabric CoreTSN/802.1CB has no SRS req; wrong citation | To be dispositioned | — | frick-fsw | open | — |
| R3-10 | Minor | loop-audit N3 row | Stale "tracked: M12-M16" disposition | To be dispositioned | — | frick-fsw | open | — |
| R3-11 | Comment | SRS-REQ-059 | Open TBC (encapsulation profile) | Track owner/milestone | — | frick-fsw | open | — |
| R3-12 | Comment | SRS-REQ-059…066 | Method vs evidence divergence (planned items) | Close at delivery | — | frick-fsw | open | — |

## 7. Facts / Assumptions / Inferences / Recommendations / Confidence / Open gaps

**Facts (verified with evidence):**

- The traceability gate passes: 110 requirements (67 srs / 29 sss / 14 gaps),
  51 Lean theorems, exit 0 (norm N4).
- Manifest parents match the intended decomposition (SSS-REQ-028→059/060/061;
  SSS-REQ-029→063/064; SSS-REQ-015→018/050/065; SSS-REQ-020→…/062/066).
- SSS-REQ-029's text names four components; its manifest children are two.
- `traceability.md` §1 lists two children under SSS-REQ-015; the manifest and the
  gate show three.
- `src/pus/adapters/pus_adapter_pusopen.c` exists (compile-guarded integration
  point, no dedicated test).
- The MSS serial backend records `set_rs485`/`de_assert_us` without driving a
  control line; GAP-A5 records the residual.
- No generic fabric-device contract exists under `src/hal/`.
- The GEM TSN backend returns `FF_NOT_IMPLEMENTED` for `configure_cbs`
  (802.1Qav).

**Assumptions:**

- The working tree under review is exactly `ce0bff3` + the listed modifications
  (confirmed via `git status`); the increment is requirements/planning-only.
- "M4-residual" is a re-plan of the original M4 scope, scheduled after sprint S4
  per `sprint-plan.md`; no mission parameters are invented (SSS-REQ-059 profile
  is correctly left `TBC`).

**Inferences:**

- The Minor coherence issues are consistent with a rapid re-plan of
  previously-absorbed scopes; they are local and correctable without rework of
  the delivered baseline.
- Because the increment adds only planned (documented-only) requirements, no
  code/V&V/Lean regression is possible from it; the applicable review scope is
  requirements/planning/documentation coherence.

**Recommendations:**

- Resolve R3-01…R3-10 with recorded dispositions and a delta re-review by R3
  (N12) before commit/push; R3-11/R3-12 tracked to closure.
- Prefer manifest-derived generation of `traceability.md` §1 to prevent
  matrix/manifest divergence recurring (R3-03/R3-04).
- Make the enabling generic fabric-device contract and the M8 gPTP/fabric
  CoreTSN scope explicit, so the "no silent gap" intent of N19 is met uniformly
  (R3-08/R3-09).

**Confidence:** **High** for the facts and the traceability findings (directly
reproduced from the files and the gate); **Medium-High** for the
scheduling/feasibility findings (R3-05, R3-08, R3-09), which depend on
device/register documentation and milestone intent not fully specified in the
reviewed material.

**Open gaps (not closed by this review):**

- The exact GEM TSN register/device basis for 802.1Qav/Qci/Qbb (R3-05) —
  requires the device documentation / driver design.
- The definition and ownership of the generic fabric-device (FIC/AXI) contract
  (R3-08).
- The requirement decomposition of the M8 gPTP / fabric CoreTSN / 802.1CB scope
  (R3-09).
- The mission-input IP/TSN encapsulation profile for SRS-REQ-059 (R3-11).

---

**Independence declaration (norm N9).** I performed this review in an
independent, fresh context with no part in authoring or executing the items
under review, based only on the evidence in the repository at `ce0bff3` + the
uncommitted working tree and the reference documents. I am read-only for the
repository and, per norm N11 and the task, I wrote **exactly one** file — this
report — and modified, created or deleted no other file; I did not commit or
push.

Committed and pushed with the reviewed change (norm N11).
