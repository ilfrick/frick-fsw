# Milestone M3 - R3 (systems engineering) and R4 (documentation) delta re-review

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0018                      |
| Reviewer session                  | independent subagent, fresh context (norm N9) |
| Reviewer profile                  | **R3** (Senior Systems Engineer, N14) **and** **R4** (Senior SW documentation reviewer, N17) |
| Reference revision                | base commit `3d3511c` (branch `main`) + uncommitted M3 working tree |
| Items under review                | Delta disposition of `FRICK-REVIEW-0015` (R3-M3-01..14, `docs/reviews/2026-10-10-m3-sss-r3-review.md`) and `FRICK-REVIEW-0016` (N-1..N-8, C-1..C-7, `docs/reviews/2026-10-10-m3-doc-set-r4-review.md`) against the current working tree |
| Date                              | 2026-10-10                             |
| Scope                             | Verify that each recorded NC disposition is actually present and correct in the working tree: ECSS compliance matrix §4/§5, theorem counts, `class-a-gaps.md` header/closure policy, `formal-verification.md`, M3 review ids, `development-plan.md` catalogue, README/overview diagrams, SSS-REQ-017, `tooling.md`/`roadmap.md`/`licensing.md` histories, `reviews/README.md`. Re-run the trace gate, the document-id uniqueness scan and a Markdown link scan. |
| Independence declaration          | The reviewer took no part in authoring, fixing or previously reviewing any item; fresh context. Every claim is based on the repository contents and commands reproduced in this session. No file other than this report was modified. |
| Norms applied                     | N1-N17 (see `docs/rules.md`), with emphasis on N3, N4, N5, N8, N11, N12, N14, N16, N17 |

## 1. Scope

This is the **delta re-review** that norm N12/N14/N17 requires before the M3
non-conformances may be closed. It re-checks, against the current working tree on
base commit `3d3511c`, the dispositions recorded in the two predecessor reports:

- R3 report `FRICK-REVIEW-0015`: R3-M3-01..R3-M3-14 (7 Minor, 7 Comment).
- R4 report `FRICK-REVIEW-0016`: N-1..N-8 (8 Minor) and C-1..C-7 (7 Comment).

The reviewer holds **both** profiles: R3 (Senior Systems Engineer) covers the
SSS/SRS, compliance-matrix and feasibility aspects; R4 (Senior SW documentation
reviewer) covers completeness, currency, ids, histories and links.

Excluded: correctness of the M3 C code and Lean proofs (R1/R2 scope), M4 driver
implementations, on-target/HWIL qualification (GAP-A5). `lake` is not installed
in this environment, so `make verify-lean` was not re-executed; the committed
transcript `verification/lean/lake-build.log` (19/19 incl. `Frick.Hal`) was
inspected. The `build`/`ctest` gates are covered by the concurrent R1/R2 delta
report and were not re-run here.

> **Review-window note (independent context).** The R1/R2 delta report
> `docs/reviews/2026-10-10-m3-r1-r2-delta-review.md` (FRICK-REVIEW-0017) appeared
> during this review (file mtime 15:56:12). Its existence and its residual
> findings are incorporated below; all machine-checked facts were re-read after
> it appeared, and the pre-existing review log was not modified by this reviewer.

### 1.1 Checks reproduced in this session

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` ->
  **PASS**, exit 0: `63 requirements, 33 srs, 17 sss, 13 gaps, 45 lean theorems
  checked`, `9 CI evidence claims`. SSS-REQ-017 has exactly 8 SRS children.
- Document-id uniqueness scan (the exact CI algorithm over `docs/**/*.md`,
  placeholder `FRICK-REVIEW-XXXX` excluded) -> **52 unique ids, 0 duplicates**
  (including this report = 0018 and the concurrent R1/R2 delta = 0017; see the
  review-window note below).
- Recursive relative-link scan over every Markdown file -> **44 non-anchor
  relative links, 0 broken**.
- Theorem-count sweep -> `45` in `README.md:244`, `docs/requirements/traceability.md:18,141`,
  `docs/vv/verification-results.md:24,173`, `docs/vv/tool-qualification.md:73`,
  `docs/architecture/hardware-abstraction.md` §7. No stale `43` remains.
- `python3 tools/scripts/metrics.py` output is byte-identical to the README
  `METRICS` block (coverage 892/1514, functions 124/166; 63/33/17/13;
  50/63 tests; 18/63 Lean).
- Direct reading of `docs/quality/{ecss-compliance-matrix,class-a-gaps}.md`,
  `docs/verification/formal-verification.md`, `docs/development-plan.md`,
  `docs/roadmap.md`, `docs/tooling.md`, `docs/quality/licensing.md`, `README.md`,
  `docs/architecture/{overview,hardware-abstraction}.md`,
  `docs/requirements/{SSS,SRS,traceability}.{md,yaml}`, `src/hal/hal.h`,
  `src/hal/hal_spacewire.h`, `docs/reviews/README.md` and the M3 review reports.

## 2. References

- Project norms: `docs/rules.md` (N1-N17); profiles in
  `docs/engineering/independent-review.md` (R3 §N14, R4 §N17).
- Predecessor reviews: `docs/reviews/2026-10-10-m3-sss-r3-review.md`
  (FRICK-REVIEW-0015), `docs/reviews/2026-10-10-m3-doc-set-r4-review.md`
  (FRICK-REVIEW-0016); siblings `...-m3-hal-r1-review.md` (0013),
  `...-m3-vv-r2-review.md` (0014).
- Items: `docs/quality/ecss-compliance-matrix.md`, `docs/quality/class-a-gaps.md`,
  `docs/verification/formal-verification.md`, `docs/development-plan.md`,
  `docs/roadmap.md`, `docs/tooling.md`, `docs/quality/licensing.md`,
  `docs/requirements/{SSS,SRS,traceability}.{md,yaml}`, `README.md`,
  `docs/architecture/{overview,hardware-abstraction}.md`, `src/hal/*.h`,
  `docs/reviews/README.md`.
- Gate/records: `tools/scripts/trace.py`, `tools/scripts/metrics.py`,
  `.github/workflows/ci.yml`, `docs/vv/verification-results.md`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-10C, ECSS-E-ST-50-12/51/52/53.

## 3. Findings (delta)

The fixes are substantially present and correct: **14 of the 15** predecessor
Minor findings are closed (R3-M3-01..07; R4 N-1..N-5, N-7, N-8) and only one
stays open (N-6); most Comments are closed too. The residual items below remain.
Each is graded per the template severity definition.

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| D-1 | Comment | `docs/development-plan.md` §3 vs `docs/architecture/hardware-abstraction.md` §4 / N8 (R3-M3-08) | **Not closed.** The development plan still advertises that the HAL design documents "specificities ... half-duplex/DE, packet-vs-stream framing", but `hardware-abstraction.md` §4 still lists only the original 7 obligations (buffer ownership, DMA/cache, ISR/task, no dynamic allocation, bounded latency, endianness, error model). Neither the plan text nor the architecture list was changed. | `docs/development-plan.md:110-112`; `docs/architecture/hardware-abstraction.md:64-83` (items 1-7); `grep -rn "packet-vs-stream" docs/` -> only the development-plan line; no "half-duplex" in the architecture doc §4. | Add the two obligations to `hardware-abstraction.md` §4 (DE and packet-vs-stream framing), or drop them from the development-plan list. |
| D-2 | Minor | Review closure records / N11, N12, N17 (R4 N-2) | **Closed with residual.** The dispositions and re-verification evidence are recorded, the M3 rows were added to `docs/reviews/README.md`, and the delta re-reviews now all exist: R1/R2 = `FRICK-REVIEW-0017`, R3/R4 = this report (`FRICK-REVIEW-0018`). Residual: (a) the R2 predecessor follow-up table still marks every NC "delta re-review = pending" and should cite `0017`; (b) `status.json`/README still report `reviews`, `vv_records` and `documentation_review` green while the R1/R2 delta raised **two new open Minors** (its D-1: the V&V records say `hal 30`/`204 checks` but the build gives `hal 31`/`205`; its D-2: several R1 Comment dispositions are recorded "corrected" but are only partial). The panel state is not yet honest for the cycle. | `docs/reviews/2026-10-10-m3-r1-r2-delta-review.md` (D-1:51, D-2:52); `docs/reviews/2026-10-10-m3-vv-r2-review.md:118-127`; `docs/status/status.json:24-26`; `docs/reviews/README.md` (M3 rows present). | Fix the R1/R2-delta Minors (D-1 counts, D-2 dispositions), update the R2 "delta re-review" cells to `0017`, then set the panel gates honestly. |
| D-3 | Minor | `docs/quality/class-a-gaps.md` / N8, N4 (R4 N-6) | **Partially closed.** The closure policy no longer asserts that owners/work packages/acceptance criteria are "set in the roadmap"; it now says WPs are "mapped in `ecss-compliance-matrix.md` and scheduled in `docs/roadmap.md`". But the undefined id **`WP-FV-A`** is still used for GAP-A1, and neither the compliance matrix nor the roadmap carries a per-gap owner / WP id / acceptance criterion (the roadmap only mentions GAP-A7 incidentally). The cross-reference remains soft. | `docs/quality/class-a-gaps.md:19` (`WP-FV-A`), `:138-144` (closure policy); `docs/quality/ecss-compliance-matrix.md:73-87` (natures, no WP ids/owners/criteria); `grep -rn "WP-FV-A" docs/` -> only `class-a-gaps.md:19`. | Define `WP-FV-A` (or remove it) and either add an owner/WP/criterion mapping or point to where they are actually defined. |
| D-4 | Comment | New Markdown docs / N13 (R4 C-2) | **Not closed (advisory).** No new M3 Markdown document carries an `SPDX-License-Identifier` header, and `docs/rules.md:188-189` still states "All authored files carry the SPDX header ... new files must add it" (README repeats the claim). Neither alternative of the C-2 recommendation (scope the N13 wording, or add the headers) was applied. New code files are compliant. Pre-existing/systemic; does not block M3. | SPDX scan: none of `docs/design/software-design.md`, `docs/architecture/hardware-abstraction.md`, `docs/quality/{configuration-management,product-assurance,safety-and-fdir,ecss-compliance-matrix}.md`, `docs/vv/validation-plan.md`, `docs/engineering/user-integration-manual.md`, `docs/development-plan.md`; `docs/rules.md:188-189`. | Scope the N13 wording to authored code + `LICENSE`/`NOTICE`, or add the headers; keep the CI licence check authoritative. |
| D-5 | Comment | SRS-REQ-031 / SRS-REQ-032 granularity / ECSS-E-ST-40C (R3-M3-09) | **Not formally dispositioned.** The two clauses still bundle heterogeneous interfaces (SPI/I2C/PCIe/GPIO; SpaceWire + memory/storage); the disposition records "corrected (fix applied)" but no change was made. The original finding was explicitly advisory ("not blocking; ... in a later revision"), so the correct closure is "accepted/deferred", not "corrected". | `docs/requirements/SRS.md:140-146`; `grep` shows the bundled text unchanged. | Mark the NC "accepted (documented; defer to a later SRS revision)" or split the clauses. Non-blocking. |

Residual observations (no separate NC; minor, consistent with the fixes but worth
recording):

- **R3-M3-02 residual:** §4 was re-titled "Residual gaps and **dispositions**
  (class A / programme)", but the table's third column is still headed
  **"Why not closable here"** while its own rows (GAP-A8/A9) say the items *are*
  closable. The title/content contradiction noted in the finding is thus not
  fully removed. (`ecss-compliance-matrix.md:73-87`.)
- **R3-M3-03 residual:** the SDP is now correctly classified "partial" (line 22),
  resolving the main point; the SDD is still "present (added for this matrix)"
  (line 26) while GAP-A9 lists per-function detail as incomplete, now reconciled
  by the qualified §5 text. Acceptable, but a per-output class-B scope note would
  be stronger. (`ecss-compliance-matrix.md:22,26,89-101`.)
- **R3-M3-10 / C-7 residual:** SSS-REQ-017 is now correctly reworded
  ("hardware abstraction layer (HAL)"; "OS primitives are abstracted separately
  by the PAL") - closed. Residual: the new document `hardware-abstraction.md` is
  still titled "Hardware / OS Abstraction Layer (HAL)" (line 1), while its §1
  correctly states that OS primitives are the PAL's, not the HAL's. The title is
  a legacy label; the body draws the boundary correctly.
- **R3-M3-11 / C-7 residual:** the `Inp` inspection evidence is now recorded in
  `verification-results.md` ("Inspection evidence (methods Inp)"), closing the
  core concern. Residual: `hardware-abstraction.md` is still not linked from the
  `SSS-REQ-017` / `SRS-REQ-026..032` implementation lists in `traceability.yaml`
  (headers only), as the finding recommended.
- **R3-M3-12 residual:** `hal_spacewire.h` now cites ECSS-E-ST-50-53 in its banner
  (lines 5-6), addressing the finding at the contract boundary; the packet
  transfer is realised through `send_pid` (50-51) rather than a distinct vtable
  member, which the banner now makes visible. Closed at Comment level.
- **R3-M3-13 residual:** SRS-REQ-025 now states the bound as "the configured
  capacity" with the macro `FF_HAL_MAX_BACKENDS` as an informative parenthetical;
  the normative property no longer depends on the macro name. Closed.
- **Tiny:** the `ecss-compliance-matrix.md` History row (0.1) still says "gaps
  GAP-A8..A13 identified" although §4 now also documents A1/A2.

## 4. Conclusion

### 4.1 Profile R3 (Senior Systems Engineer, N14)

- [x] **APPROVED - no findings or Comments only** (no open Minor; 2 Comments remain)
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**Graded conclusion (R3): APPROVED.** No Major and no open Minor finding. All 7
Minor findings (R3-M3-01..07) are **closed** with evidence; of the 7 Comments
(R3-M3-08..14), 5 are closed and **2 remain open** (D-1 / R3-M3-08, D-5 /
R3-M3-09). Because only Comments remain open, the template's "APPROVED - no
findings or Comments only" applies; the two open Comments carry recommended
corrections (below) rather than a derogation.

All synthetic-system aspects converge: the trace gate is PASS (63/33/17/13,
45 theorems, 9 CI claims), SSS-REQ-017 has 8 coherent SRS children, the
compliance matrix now shows the full A1..A13 set and qualifies the class-B
claim, the theorem count and `Frick.Hal` attribution are correct, and the M3
catalogue no longer advertises the undelivered `FF_Clock_t`. The open Comments
(D-1 / R3-M3-08 and D-5 / R3-M3-09) should be corrected or re-dispositioned
(R3-M3-09 as accepted/deferred), but they do not block class-B closure.

### 4.2 Profile R4 (Senior SW documentation reviewer, N17)

- [ ] APPROVED - no findings or Comments only
- [x] **APPROVED WITH COMMENTS** - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**Graded conclusion (R4): APPROVED WITH COMMENTS.** No Major finding. Of the 8
Minor findings, **7 are closed** (N-1, N-2, N-3, N-4, N-5, N-7, N-8) and **one
remains open (D-3 / N-6, undefined `WP-FV-A` / soft closure cross-reference)**;
N-2 is closed with the residual recorded under D-2. Of the 7 Comments, **6 are
closed** (C-1, C-3, C-4, C-5, C-6, C-7) and **one (D-4 / C-2) remains open**
(systemic, advisory). The documentation set is complete, indexed, link-clean
(44/0), id-unique (52/0), metrics-current (README panel == `metrics.py`) and
internally coherent; review ids are normalised (0013/0014/0015/0016; the R1/R2
delta = 0017; this report = 0018). The only open Minor is D-3; D-2 and D-4 are
residual/advisory.

### 4.3 Cross-cutting note (both profiles)

The concurrent R1/R2 delta report (`FRICK-REVIEW-0017`) re-reviews the R2 base
review, which had been **REJECTED (2 Major)**; it confirms both Major NCs resolved
(R2 APPROVED WITH COMMENTS) and R1 APPROVED WITH COMMENTS. No Major finding
remains in any M3 review, so **milestone M3 is not blocked by a Major**. Two
caveats remain before the records are final: the R2 **predecessor** follow-up
table still marks every NC "delta re-review = pending" (it should cite `0017`),
and the R1/R2 delta's own residual Minors (its D-1 stale V&V check counts, its D-2
overstated Comment dispositions) are open while the status panel is all-green.
These are documentation/record defects, not requirement defects.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R3 (N9/N14)   | independent subagent (fresh context) | 2026-10-10 | APPROVED (0 Major, 0 Minor open; 2 Comments open: R3-M3-08, R3-M3-09) |
| R4 (N9/N17)   | independent subagent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS (1 Minor open: N-6; residual N-2; 1 Comment open: C-2) |

## 6. Follow-up records

Each predecessor NC with its verified state in the working tree. "delta re-review"
is this report.

### 6.1 R3 (from FRICK-REVIEW-0015)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R3-M3-01 | Minor | `ecss-compliance-matrix.md` §4 / N5, N8 | Class-A summary omits GAP-A1/A2 | corrected | §4 now lists GAP-A1 and GAP-A2 (lines 75-76); 13 rows A1..A13 | frick-fsw | **closed** | verified (this report) |
| R3-M3-02 | Minor | `ecss-compliance-matrix.md` §4/§5 / N8 | §4 title contradicts rows/§5 | corrected | §4 retitled "Residual gaps and dispositions"; §5 identifies residual WPs; residual column header "Why not closable here" still overstates (observation) | frick-fsw | **closed** (residual comment) | verified (this report) |
| R3-M3-03 | Minor | Compliance matrix / SSS-REQ-012, N5 | Class-B outputs misclassified; §5 "complete" unsupported | corrected | SDP now "partial"; §5 says "complete for a framework deliverable" and lists residuals (lines 91-101) | frick-fsw | **closed** | verified (this report) |
| R3-M3-04 | Minor | theorem count / N8, N16 | Count 43 vs gate 45 | corrected | 45 in README:244, traceability.md:18,141, verification-results.md:24,173, tool-qualification.md:73; gate reports 45 | frick-fsw | **closed** | verified (this report) |
| R3-M3-05 | Minor | `class-a-gaps.md` / N16 | Header 0.2 vs history 0.3 | corrected | header now `Revision 0.3` (line 7); History ends 0.3 | frick-fsw | **closed** | verified (this report) |
| R3-M3-06 | Minor | `development-plan.md` §3 / SSS-REQ-017 | `FF_Clock_t` advertised, not delivered | corrected | Time row now "PAL clock (`FF_PalGet*`) + BSP watchdog"; no `FF_Clock_t` anywhere; gate PASS | frick-fsw | **closed** | verified (this report) |
| R3-M3-07 | Minor | `src/hal/hal.h` / N8 | Banner references non-existent `hal_i2c.h` | corrected | banner now lists "hal_spi.h [SPI + I2C]" (lines 6-7); enum comment "(hal_spi.h)" (line 29) | frick-fsw | **closed** | verified (this report) |
| R3-M3-08 | Comment | `development-plan.md` vs `hardware-abstraction.md` §4 / N8 | Two integrator obligations claimed but absent | claimed corrected; **not applied** | dev-plan still lists "half-duplex/DE, packet-vs-stream framing" (line 112); architecture §4 still 7 items | frick-fsw | **open** (D-1) | not closed |
| R3-M3-09 | Comment | SRS-REQ-031/032 / granularity | Bundled heterogeneous clauses | claimed corrected; **not applied** (was advisory "later revision") | SRS.md:140-146 unchanged | frick-fsw | **open / re-disposition as accepted-deferred** (D-5) | not closed (advisory) |
| R3-M3-10 | Comment | SSS-REQ-017 / terminology | "hardware/OS" vs hardware-only HAL | corrected | SSS-REQ-017 now "hardware abstraction layer (HAL)" + "(OS primitives are abstracted separately by the PAL)"; residual: `hardware-abstraction.md` title still "Hardware / OS" | frick-fsw | **closed** (residual comment) | verified (this report) |
| R3-M3-11 | Comment | SSS-REQ-017, SRS-REQ-026..032 / N3, N4 | Inspection evidence asserted, not recorded | corrected | `verification-results.md` §"Inspection evidence (methods Inp)" records header inspection + `test_contract_presence`; residual: doc not in impl list | frick-fsw | **closed** (residual comment) | verified (this report) |
| R3-M3-12 | Comment | SRS-REQ-032 / ECSS-E-ST-50-53 | 50-53 not visible at contract boundary | corrected | `hal_spacewire.h:5-6` now cites ECSS-E-ST-50-53; realised via `send_pid` | frick-fsw | **closed** | verified (this report) |
| R3-M3-13 | Comment | SRS-REQ-025 | Macro named in normative text | corrected | SRS-REQ-025 states "configured capacity" (macro now parenthetical) | frick-fsw | **closed** | verified (this report) |
| R3-M3-14 | Comment | R1/R2 follow-up records / N11, N12 | NCs recorded open, no disposition | corrected | R1 table closed with evidence; R2 table dispositioned/closed (delta re-reviews still "pending") | frick-fsw | **closed** | verified (this report) |

### 6.2 R4 (from FRICK-REVIEW-0016)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| N-1 | Minor | Theorem count / N8, N11, N16 | Docs say 43; `Frick.Hal` mis-attribution | corrected | 45 everywhere; `hardware-abstraction.md` §7 attributes model to `Frick.Hal` and `req_srs025_*` wrappers to `Frick.Requirements`; gate 45 | frick-fsw | **closed** | verified (this report) |
| N-2 | Minor | Review closure / N11, N12, N17 | NCs open; no delta re-review; log lacks M3 rows; panel green | corrected | M3 rows added to `reviews/README.md`; dispositions recorded; delta reviews exist (R1/R2 = 0017, R3/R4 = 0018). Residuals: R2 table still "delta re-review = pending"; panel green while R1/R2-delta Minors open (D-2) | frick-fsw | **closed** (residual D-2) | verified (this report) |
| N-3 | Minor | `class-a-gaps.md` / N16 | Header 0.2 vs History 0.3 | corrected | header `Revision 0.3` (line 7) | frick-fsw | **closed** | verified (this report) |
| N-4 | Minor | `formal-verification.md` / N8 | Not updated for `Frick.Hal` | corrected | Rev 0.3; layout adds `Frick/Hal.lean` (line 27); §5 scope mentions M3 HAL registry bound; History 0.3 | frick-fsw | **closed** | verified (this report) |
| N-5 | Minor | M3 review ids / N8, N16 | Non-`FRICK-REVIEW-00nn` ids | corrected | 0013 (R1), 0014 (R2), 0015 (R3), 0016 (R4); id scan 52 unique, 0 duplicates | frick-fsw | **closed** | verified (this report) |
| N-6 | Minor | `class-a-gaps.md` / N8, N4 | Closure policy mis-references roadmap; `WP-FV-A` undefined | partial | closure policy reworded away from "set in the roadmap"; **`WP-FV-A` still undefined**, no per-gap owner/WP/criterion mapping | frick-fsw | **open** (D-3) | not closed |
| N-7 | Minor | `development-plan.md` §3 / N8 | `FF_Clock_t` advertised, not delivered | corrected | Time row -> "PAL clock + BSP watchdog"; gate PASS | frick-fsw | **closed** | verified (this report) |
| N-8 | Minor | `ecss-compliance-matrix.md` §4 / N5, N8 | Residual set omits A1/A2; title contradicts content | corrected | §4 includes A1/A2 and is retitled (D-1 in R3) | frick-fsw | **closed** | verified (this report) |
| C-1 | Comment | `ecss-compliance-matrix.md` / N5 | Class-B SDP/SDD marked present; §5 "complete" | corrected | SDP "partial"; §5 qualified to "complete for a framework deliverable" + residuals | frick-fsw | **closed** | verified (this report) |
| C-2 | Comment | New docs / N13 | No SPDX header (systemic) | not applied | no new doc carries SPDX; `rules.md:188-189` unchanged | frick-fsw | **open** (D-4, advisory) | not closed |
| C-3 | Comment | `tooling.md` / N8 | CI docs-job change not reflected | corrected | §4 stage 7 lists required-document list + document-id uniqueness; Rev 0.3; History row added | frick-fsw | **closed** | verified (this report) |
| C-4 | Comment | plan/roadmap / N16 | No History row for M3 status edit | corrected | `development-plan.md` History 0.2; `roadmap.md` History 0.4 (both record M3 "in progress") | frick-fsw | **closed** | verified (this report) |
| C-5 | Comment | README/overview diagrams / N8 | B1 coloured as interface | corrected | `classDef m4` added; B1 uses `:::m4`; legends updated in both README (line 181) and overview (lines 82-84) | frick-fsw | **closed** | verified (this report) |
| C-6 | Comment | `licensing.md` / N16 | History descending | corrected | History now ascending 0.1 -> 0.2 | frick-fsw | **closed** | verified (this report) |
| C-7 | Comment | SSS-REQ-017 / N3, N4 | "hardware/OS" wording; unrecorded inspection evidence | corrected | SSS-REQ-017 reworded; Inp evidence recorded in `verification-results.md`; residuals per R3-M3-10/11 | frick-fsw | **closed** (residual comment) | verified (this report) |

---

Report document id `FRICK-REVIEW-0018` was chosen after the concurrent R1/R2
delta report took `0017`; the document-id uniqueness scan over `docs/**/*.md`
confirms no collision (52 unique, 0 duplicates). Per norm N12 the remaining open
items (R3-M3-08 and R3-M3-09, both Comment; N-6, Minor; C-2, Comment), plus the
R1/R2 delta's residual Minors and the panel-state caveat, must be closed or
justified and this delta re-reviewed before commit/push. This reviewer modified
no file other than this report.
