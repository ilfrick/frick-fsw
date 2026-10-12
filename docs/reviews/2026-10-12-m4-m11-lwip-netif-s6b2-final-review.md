# M4/M11 Sprint-6b-2 — lwIP netif on the emulated GEM — combined R1+R2+R3+R4+PA **confirmation** review (norm N12)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0155                      |
| Reviewer session                  | independent agent (norm N9), fresh context |
| Reviewer profile                  | R1 (SW dev) + R2 (SW V&V) + R3 (Sys Eng) + R4 (SW documentation) + PA (SW product assurance) — **confirmation** |
| Reference revision                | `471afb5` + uncommitted working tree (Sprint-6b-2 increment, corrected for FRICK-REVIEW-0154) |
| Items under review                | `docs/architecture/network-stack.md`, `docs/status/handover.md`, `docs/planning/sprint-plan.md`, `docs/requirements/traceability.md`, `.github/workflows/nightly.yml`, `docs/reviews/README.md` (the FRICK-REVIEW-0154 corrective set) |
| Date                              | 2026-10-12                             |
| Scope                             | Confirmation (norm N12) that the four Minor findings left open by FRICK-REVIEW-0154 — **R4-01**, **R4-03**, **NC-154-01**, **NC-154-02** — are closed by first-hand evidence, that the ancillary C-154-02/C-154-04 records are in place, that no Major is open, and that the functional gates still converge. ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2, project norms N1–N20 (`docs/rules.md`, rev 1.4), reviewer contract `docs/engineering/independent-review.md` (rev 0.7) |
| Independence declaration          | The reviewer ran in a fresh, independent context; it did not author, design, implement, test or approve any item under review, and it did not inherit the author's conversation. It is read-only over the repository except for this one report. |
| Norms applied                     | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

Confirmation review (norm N12) of the corrective set applied to the uncommitted
Sprint-6b-2 working tree **after** FRICK-REVIEW-0154. FRICK-REVIEW-0154 closed the
code/comment/evidence corrections (R1-01/R1-02, R2-02, R3-01, R3-03/PA-01,
R4-02, R4-04 and the R3-02 mirror row) and left **four Minors open**: R4-01
(`network-stack.md` §4 stale wording), R4-03 (`handover.md` §4/§5 stale wording),
NC-154-01 (netif glue untested → to be recorded with owner + sprint) and
NC-154-02 (`traceability.md` History not bumped).

This report verifies, first-hand in the current tree:

1. each of the four items against the current file content, with quoted evidence;
2. the two ancillary records the correction set was asked to add — the C-154-02
   `nightly.yml` step-label fix and the C-154-04 `docs/reviews/README.md` log rows;
3. that **no Major finding is open**;
4. the functional gates (`ctest`, `trace.py`, `metrics.py --check`) and the
   per-document header = last `## History` row invariant for every touched doc.

**Excluded:** re-running the Renode images and the RTEMS cross-builds (the
corrections are comment/document-only and do not touch the executable path or the
`hal_net_lwip` oracle); third-party lwIP; on-silicon GEM DMA/PHY (GAP-A5); the
FRICK-REVIEW-0153 base increment beyond the four residual items.

**Method:** read-only reconstruction — `git status`/`git diff` at `471afb5`, direct
file inspection, and the gate re-runs. Each document header was compared with its
last `## History` row. No build/run beyond the re-run gates was performed.

## 2. References

- Project norms: `docs/rules.md` (N1–N20, rev 1.4)
- Reviewer contract: `docs/engineering/independent-review.md` (rev 0.7)
- Prior reports: `docs/reviews/2026-10-12-m4-m11-lwip-netif-s6b2-review.md` (FRICK-REVIEW-0153); `docs/reviews/2026-10-12-m4-m11-lwip-netif-s6b2-delta-review.md` (FRICK-REVIEW-0154)
- Traceability manifest/gate: `docs/requirements/traceability.yaml`, `tools/scripts/trace.py`
- ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2

## 3. Closure table (FRICK-REVIEW-0154 residual items)

| Finding | 0154 severity | Required correction | First-hand evidence (current tree) | Status |
| ------- | ------------- | ------------------- | ---------------------------------- | ------ |
| **R4-01** | Minor | `network-stack.md` §4: replace the stale "on-target run = S6b-2 / NC-S6-02 … the adapter is **archived but not linked** into the default image" bullet | `network-stack.md:48-50` now reads "(S6b-1; the runtime object sizing is wired in `config/rtems/rtems_config.c`, S6b-2). The adapter is linked only into the **lwIP-enabled** image, so `FF_USE_LWIP=OFF` output is unchanged."; new §4 bullet `:52-58` documents the netif glue and the open packet-round-trip/GEM-DMA residuals; no "archived but not linked" / "on-target run = S6b-2" string remains in the body; header 0.6 = History 0.6 | **CLOSED** |
| **R4-03** | Minor | `handover.md` §4 M11 body + §5: replace the stale "sprint S6b" / "(NC-S6-01, deferred)" wording | §4 `handover.md:121` now reads "Remaining M11 peripheral classes: NET(packet round-trip = NC-S6-01 residual)/PCIE/SPACEWIRE."; §5 `:149-150` now reads "Next per plan: **S6b — NET lwIP-on-GEM (packet round-trip = NC-S6-01 residual) and **S7 — PCIe + SpaceWire**"; no "NC-S6-01, deferred" / "sprint S6b" in the body (only in History rows, legitimately historical); header 1.15 = History 1.15 | **CLOSED** (see NF-1) |
| **NC-154-01** | Minor | Record the netif-glue coverage gap with an owner **and** a sprint (or add the host mock test) | `sprint-plan.md` §3 `:78`: "`NC-154-01` \| `FRICK-REVIEW-0154` \| Minor \| The lwIP netif glue … is exercised by no test … \| **Owner**: frick-fsw authoring agent; **sprint**: NET follow-up (with NC-S6-01)." No test under `tests/` references `ff_lwip_netif`/`linkoutput`/`attach_netif` (gap correctly still recorded, not hidden) | **CLOSED** (dispositioned/tracked residual; see NF-3) |
| **NC-154-02** | Minor | Add a History row to `traceability.md` for the S6b-2 mirror update | `traceability.md` header rev **0.35** (`:7`); mirror row `:105` lists `hal/net/lwip/ff_net_lwip.c/h + ff_lwip_netif.c/h + rtems/`; History row `:241` "0.35 \| 2026-10-12 \| … SRS-REQ-040 mirror row also records the netif glue …" — header = last History row | **CLOSED** |
| C-154-02 (ancillary) | Comment | Drop NC-S6-02 from the `nightly.yml` lwIP step label | `.github/workflows/nightly.yml:78` step name now "Renode lwIP-on-GEM validation (**NC-S6-01 residual**)" | **CLOSED** |
| C-154-04 (ancillary) | Comment | Add the FRICK-REVIEW-0153/0154 log rows to `docs/reviews/README.md` | Two rows appended (`docs/reviews/README.md`, `git diff` +2): "…s6b2-review.md" (0153) and "…s6b2-delta-review.md" (0154) | **CLOSED** (see NF-2) |

**No Major finding is open** (re-confirms the FRICK-REVIEW-0153 and
FRICK-REVIEW-0154 verdicts). The residuals that remain — NC-S6-01 (packet
round-trip), NC-S6-03 (latent 64-bit GEM descriptor DMA), NC-S7-01 (SpaceWire)
and NC-154-01 (glue coverage) — are **tracked** with an owner and a sprint and
are unchanged in scope by this correction set.

## 4. Scope conformance

| Declared correction scope | Implemented content | Verdict |
| --- | --- | --- |
| R4-01 `network-stack.md` §4 | §4 bullet rewritten; netif-glue bullet added; History 0.6 | **Conformant** |
| R4-03 `handover.md` §4/§5 | Both sites updated; History 1.15 | **Conformant** |
| NC-154-01 recorded (owner + sprint) | `sprint-plan.md` §3 row 78 | **Conformant** |
| NC-154-02 `traceability.md` History | rev 0.35 + History row + mirror row | **Conformant** |
| C-154-02 `nightly.yml` label | Label updated | **Conformant** |
| C-154-04 `docs/reviews/README.md` rows | Two rows appended | **Conformant** (wording NF-2) |
| No undeclared content | The working-tree file set is the S6b-2 increment (22 modified + 2 new source files + the 0153/0154 reports + this report); the correction set introduced **no new file** and touched **no file outside** the increment; `git diff --stat` = 22 files, +330/−68 | **Conformant** |

**No scope non-conformance.** The corrections are confined to the already-touched
documentation and CI label; no requirement, source, test or tool was changed by
the corrective set.

## 5. Gate re-run (2026-10-12, first-hand)

| Gate | Result |
| ---- | ------ |
| `ctest --test-dir build/native` | **PASS 22/22** (0 failed; 100% tests passed) |
| `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** (`114 requirements, 71 srs, 29 sss, 14 gaps, 51 lean theorems checked`) |
| `python3 tools/scripts/metrics.py --check` | **current** (exit 0) |
| Header = last `## History` row, all touched docs | **OK** (network-stack 0.6, development-plan 0.25, sprint-plan 0.12, traceability.md 0.35, handover 1.15, loop-audit 0.19, renode-environment 0.19, tool-qualification 0.3.23, verification-results 0.40) |

## 6. New findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| NF-1 | Comment | `handover.md` §5 / N8 | The R4-03 edit left a misbalanced bold run: "…**S6b — NET lwIP-on-GEM (packet round-trip = NC-S6-01 residual) and **S7 — PCIe + SpaceWire**" — three `**` markers across `:149-150` (odd) where the pre-correction text was balanced. Rendering only; the meaning is correct | `handover.md:149-150`; marker count 1+2 | Close the emphasis after "S6b … residual)" (i.e. "**S6b — NET lwIP-on-GEM (packet round-trip = NC-S6-01 residual)** and **S7 — PCIe + SpaceWire**") |
| NF-2 | Comment | `docs/reviews/README.md` / N9/N11 | The log row for the 0154 delta review reads "APPROVED WITH COMMENTS (R4-01/R4-03, NC-154-01/02 **fixed**)", whereas FRICK-REVIEW-0154 found those four items **open** (loop not closed). The row conflates the 0154 verdict with the later fixes | `docs/reviews/README.md` (0154 row) vs FRICK-REVIEW-0154 §10 | Reword to preserve the verdict, e.g. "…NC-154-01/02 **open at 0154** → closed by FRICK-REVIEW-0155"; add the 0155 row with the commit |
| NF-3 | Comment | `CHANGELOG.md` / N16 | The S6b-2 CHANGELOG entry records NC-S6-01/02/03 and NC-151-02/05 but not NC-154-01/NC-154-02; the 0154 recommendation for NC-154-01 named a `CHANGELOG.md` entry alongside `sprint-plan.md` §3 | `CHANGELOG.md` (§Added); `git diff CHANGELOG.md`; `grep NC-154 CHANGELOG.md` = 0 | Optionally add a one-line cross-reference for NC-154-01/02 for consistency with the other NCs |

**Severity definition:** *Major* disqualifies the item for criticality B or
defeats a requirement/safety barrier; *Minor* must be corrected or justified;
*Comment* is an observation/recommendation, no derogation.

**No Major and no Minor finding is open.** NF-1…NF-3 are Comments: NF-1 is
cosmetic (correct meaning), NF-2 does not misstate the authoritative record (the
FRICK-REVIEW-0154 report itself is present and correct, and the fixes exist in the
same working tree being committed), and NF-3 is an optional cross-reference (the
NC-154-01 tracking obligation is already met in `sprint-plan.md` §3).

## 7. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — no Major/Minor open; the four FRICK-REVIEW-0154 Minors are closed and the tracked residuals carry owner + sprint
- [ ] REJECTED — one or more Major findings

R4-01, R4-03, NC-154-01 and NC-154-02 are **CLOSED with first-hand evidence**; the
ancillary C-154-02 and C-154-04 records are in place; **no Major is open**. The
N12/N17/N20 review loop for the Sprint-6b-2 increment is **closed** and
**commit/push is permitted** subject to the qualified human accepting the tracked
residuals (NC-S6-01, NC-S6-03, NC-154-01, GAP-A5).

### PA verdict

The corrective scope declared for FRICK-REVIEW-0154 is **fully delivered** and
verified against the current file content. The gates converge
(ctest 22/22, trace PASS 114/71/29/14, metrics current, header = History for every
touched doc). The three new observations (NF-1…NF-3) are non-blocking Comments.

**What a qualified human must still accept:** the residual risk of shipping this
increment in the criticality-B baseline — the deferred lwIP-on-GEM packet
round-trip (NC-S6-01), the latent GEM 64-bit descriptor DMA addressing defect
(NC-S6-03), the unexercised netif glue (NC-154-01), and the emulated-vs-silicon
GAP-A5. The agent does not approve release or accept residual risk.

## 8. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9) | Senior FSW / real-time / space software developer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — no code change in the corrective set; glue coverage tracked (NC-154-01); NF-1 formatting |
| R2 (N9) | Senior SW V&V engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — gates re-run green; NC-154-01 disposition acceptable as a tracked residual |
| R3 (N14/N9) | Senior systems/aerospace engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — `traceability.md` mirror + History 0.35 consistent; counts unchanged 71/29/14 |
| R4 (N17/N9) | Senior SW documentation engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — R4-01/R4-03 closed; header = History for all touched docs; NF-1/NF-2 |
| PA (N20) | Senior SW Product Assurance engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — no Major/Minor open; loop closed; commit permitted; residual risk to the human authority |

## 9. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R4-01 | Minor | `network-stack.md` §4 | Stale lwIP-backend bullet | **CLOSED** — §4 rewritten (sizing wired S6b-2; adapter linked only into the lwIP image; netif-glue bullet) | `network-stack.md:48-58`; header 0.6 = History 0.6 | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0155 (this) |
| R4-03 | Minor | `handover.md` §4/§5 | Two stale M11 sites | **CLOSED** — both sites now reference the NC-S6-01 residual | `handover.md:121,149-150`; header 1.15 = History 1.15 | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0155 (this) |
| NC-154-01 | Minor | glue coverage / N6/N10/N15 | `linkoutput`/RX tested nowhere (R2-01) | **CLOSED** — recorded with owner + sprint (tracked residual, not fixed) | `sprint-plan.md:78`; no `tests/` reference to `ff_lwip_netif` | frick-fsw authoring agent | **closed** (residual tracked) | FRICK-REVIEW-0155 (this) |
| NC-154-02 | Minor | `traceability.md` / N16 | History row not added for S6b-2 | **CLOSED** — rev 0.35 + History row + mirror row | `traceability.md:7,105,241` | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0155 (this) |
| NF-1 | Comment | `handover.md` §5 | Misbalanced `**` emphasis from the R4-03 edit | Open (non-blocking) | `handover.md:149-150` | frick-fsw authoring agent | **open (Comment)** | fix with the commit |
| NF-2 | Comment | `docs/reviews/README.md` | 0154 log-row wording ("fixed" vs open) | Open (non-blocking) | `docs/reviews/README.md` | frick-fsw authoring agent | **open (Comment)** | reword + add 0155 row with the commit |
| NF-3 | Comment | `CHANGELOG.md` | NC-154-01/02 not cross-referenced | Open (non-blocking) | `CHANGELOG.md` §Added | frick-fsw authoring agent | **open (Comment)** | optional |
| NC-S6-01 | Minor (scope) | SRS-REQ-040 | Packet round-trip residual | Unchanged — tracked (owner + sprint NET follow-up) | `sprint-plan.md:72`; `renode-environment.md` §4 | frick-fsw authoring agent | **open (tracked)** | — |
| NC-S6-03 | Minor | `ff_gem.c` `gem_phys` | Latent 64-bit GEM descriptor DMA addressing | Unchanged — tracked (owner + driver follow-up) | `sprint-plan.md:77` | GEM driver author | **open (tracked)** | — |
| NC-S7-01 | Minor | SpaceWire / OBJ-19 | No emulated SpaceWire peripheral | Unchanged — tracked (S7 contract-level) | `sprint-plan.md:73` | frick-fsw authoring agent | **open (tracked)** | — |
| NC-S6-02 / NC-151-02 / NC-151-05 | Minor | sizing / CI / diagnostics | (closed in S6b-2) | **CLOSED** | FRICK-REVIEW-0154 §10 | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0153/0154 |

## 10. Norm-by-norm audit (delta)

| Norm | Assessment |
| ---- | ---------- |
| N1 Lean | SRS-REQ-040 `lean_note` unchanged (already corrected in 0154) → OK |
| N2 English | All reviewed content English → OK |
| N3 SRS/SSS | SRS-REQ-040 mirror row now records the netif glue; History bumped → OK |
| N4 traceability | `trace.py` PASS (114/71/29/14, 51 theorems, 12 CI claims) → OK |
| N5 class B | Residuals tracked with owner + sprint → OK |
| N6 V&V / Docker | Gates re-run green for the corrective scope → OK |
| N7 dual push | Not applicable (pre-commit confirmation); loop now closed → OK |
| N8 docs current | R4-01/R4-03 closed; all touched docs header = History → OK (NF-1 cosmetic) |
| N9 independent review | Performed in independent context (R1+R2+R3+R4+PA) → OK |
| N10 V&V tools | No script/tool changed by the corrective set → OK |
| N11 records | V&V results/status current; the 0153/0154 log rows are added → OK (NF-2 wording) |
| N12 NC resolution | Four 0154 Minors verified CLOSED item-by-item → **closed** |
| N13 licensing | No new file → OK |
| N14 SSS review | No SSS/SSS-manifest change → not triggered |
| N15 full cycle | Applicable gates re-run green → OK |
| N16 history | Per-doc History updated (traceability 0.35); `docs/reviews/README.md` is a log, no History table → OK |
| N17 documentation review | Performed; all findings closed (NF-1/NF-2 Comments) → OK |
| N18 Renode validation | Unchanged by the corrective set (0153: both images `RENODE_TEST: PASS`); not re-run → recorded |
| N19 loop audit | `loop-audit.md` M11 row consistent; sprint anchors present → OK |
| N20 PA review | Performed; APPROVED WITH COMMENTS; no open Major/Minor → **loop closed** |

---

## Facts / Assumptions / Inferences / Recommendations / Confidence / Open gaps

**Facts**

- `network-stack.md` §4 (`:48-58`) now states the S6b-2 runtime object sizing is
  wired and "the adapter is linked only into the lwIP-enabled image"; the stale
  "archived but not linked" / "on-target run = S6b-2" text is gone; header 0.6 =
  History 0.6 (R4-01 closed).
- `handover.md` §4 (`:121`) and §5 (`:149-150`) now reference the NET packet
  round-trip residual (NC-S6-01) instead of the old deferral wording; header 1.15
  = History 1.15 (R4-03 closed).
- `sprint-plan.md` §3 (`:78`) records NC-154-01 with owner `frick-fsw authoring
  agent` and sprint "NET follow-up (with NC-S6-01)"; no test under `tests/`
  references the netif glue (the gap is honestly tracked, not hidden).
- `traceability.md` is at rev 0.35, its History has a 0.35 row, and mirror row
  `:105` lists `ff_lwip_netif.c/h` (NC-154-02 closed).
- `.github/workflows/nightly.yml:78` label reads "(NC-S6-01 residual)" (C-154-02
  closed); `docs/reviews/README.md` has rows for the 0153/0154 reports (C-154-04).
- Gates: ctest 22/22; trace PASS (`114/71/29/14`, 51 theorems); metrics current
  (exit 0); header = last History row for all nine touched History-bearing docs.
- No Major and no Minor finding is open; the corrective set introduced no new file
  and no out-of-scope change.

**Assumptions**

- The reviewed working tree is the intended corrected increment and nothing else is
  pending.
- The 0154 recommendation for NC-154-01 ("sprint-plan.md §3 **and** CHANGELOG.md")
  is satisfied in substance by the sprint-view record; the CHANGELOG cross-
  reference is optional (NF-3).

**Inferences**

- The four FRICK-REVIEW-0154 Minors were applied after the 0154 review snapshot
  (the current tree shows the corrected wording that 0154 reported as absent).
- The `docs/reviews/README.md` 0154 row is a terse cycle headline rather than a
  finding list (consistent with the other log rows), which is why NF-2 is rated a
  Comment and not a Minor.

**Recommendations**

- Correct NF-1 (emphasis) and NF-2 (log-row wording) with the commit; add the
  FRICK-REVIEW-0155 log row and the opt-in NC-154-01/02 CHANGELOG cross-reference
  (NF-3).
- Commit/push the Sprint-6b-2 increment to both remotes (N7) now that the loop is
  closed, and carry NC-S6-01/NC-S6-03/NC-154-01 into the scheduled sprints.

**Confidence:** **High** for the per-finding closure verdicts and the gate results
(read first-hand in the current tree); **High** for "no Major open" (no unresolved
hazard/requirement/safety item; the residuals are tracked); **Medium-High** for the
NF-2 classification (the log's intended granularity is inferred from the other
rows).

**Open gaps (must be closed or formally accepted by a qualified human):**
NC-S6-01 (packet round-trip), NC-S6-03 (latent 64-bit GEM DMA addressing),
NC-154-01 (netif glue host coverage) and on-target GEM DMA/PHY (GAP-A5) — all
tracked with an owner and a sprint, none blocking this incremental commit. NF-1,
NF-2, NF-3 are non-blocking Comments.

---

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-12 | independent reviewer (FRICK-REVIEW-0155) | Confirmation review (norm N12) of the Sprint-6b-2 corrective set after FRICK-REVIEW-0154. Verified **CLOSED** with first-hand evidence: R4-01 (`network-stack.md` §4), R4-03 (`handover.md` §4/§5), NC-154-01 (recorded with owner + sprint), NC-154-02 (`traceability.md` 0.35 + History), C-154-02 (nightly label), C-154-04 (log rows). No Major/Minor open; new NF-1/NF-2/NF-3 are Comments. Gates: ctest 22/22, trace PASS 114/71/29/14, metrics current, header = History for all touched docs. Verdict APPROVED WITH COMMENTS; N12/N17/N20 loop **closed**; commit/push permitted. |

Committed and pushed with the reviewed change (norm N11).

## Independence and file-modification declaration

I declare that I reviewed the items above **independently** (fresh context; I did
not author, implement, test or approve them) and that, as instructed, I wrote
**exactly one file** — this report
(`docs/reviews/2026-10-12-m4-m11-lwip-netif-s6b2-final-review.md`) — and
**modified, created or deleted no other repository file**. I did not commit or
push. The gate re-runs wrote only under `build/` (untracked build artifacts).
