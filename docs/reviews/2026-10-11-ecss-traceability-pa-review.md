# Review Report — ECSS traceability: is SRS/SDD→source-code tracing mandated, and is frick-fsw conformant?

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0067                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; N20) |
| Reference revision                | `4c9516e` (branch `main`), 2026-10-11 |
| Items under review                | Project owner question on ECSS in-source traceability; `docs/requirements/traceability.yaml`, `docs/requirements/traceability.md`, `docs/requirements/SRS.md`, `docs/requirements/SSS.md`, `docs/design/software-design.md`, `docs/engineering/coding-style.md`, `docs/quality/ecss-criticality-b.md`, `docs/quality/ecss-compliance-matrix.md`, `tools/scripts/trace.py`, and the `src/` + `config/` + `tests/` trees |
| Date                              | 2026-10-11                             |
| Scope                             | (a) Establish from the ECSS standards themselves what is **actually required** about traceability between requirements (SRS/SSS), design (SDD), code and tests, for software criticality **B**, and whether tracing requirement IDs **in the source code** is mandated, recommended, or merely one acceptable means. (b) Assess frick-fsw conformance with repository evidence. (c) If non-conformant, define the NC. |
| Independence declaration          | Reviewer took no part in authoring or implementing any reviewed item; fresh independent session with no prior context. The review is read-only for the repository: only this report file was created; no other repository file was modified. |
| Norms applied                     | N3, N4, N5, N9, N10, N12, N16, N20 (see `docs/rules.md`) |

## 1. Scope

The owner asked:

> "According to ECSS, is it required that the SRS requirements and/or the SDD
> be traced IN THE SOURCE CODE? If so, how? Is our codebase compliant with what
> ECSS prescribes? If not, an NC must be opened and resolved in the next sprint."

This report answers the question **only from the ECSS source text** for the
baseline the project targets (ECSS software criticality **B**, `docs/quality/ecss-criticality-b.md:41`),
then assesses the repository. It is deliberately strict about the distinction
between (i) a normative **shall**, (ii) a **should**/"recommended", and (iii) an
**acceptable means** that ECSS merely permits.

**Assumption / boundary.** Only the two standards named in the question are
treated as the obligation source (ECSS-E-ST-40C Rev.1 and ECSS-Q-ST-80C Rev.2).
The actual project contract, statement of work and customer-approved tailoring
matrix were **not available to this reviewer**; ECSS is applicable "to the
extent defined by the tailoring process" (E-ST-40C Scope, p.1). If a contract
clause adds a stricter requirement (e.g. mandatory in-source requirement IDs),
that clause would prevail and this report would have to be re-baselined. No such
clause is present in the repository.

## 2. References

**Standards — downloaded from the issuer and MD5-verified against the issuer's
published checksums (Fact, verified 2026-10-11):**

- ECSS-E-ST-40C Rev.1, *Software*, 30 April 2025. URL
  https://ecss.nl/standard/ecss-e-st-40c-rev-1-software-30-april-2025/ ; PDF
  `ECSS-E-ST-40C-Rev.1(30April2025).pdf`; issuer MD5
  `227F26315BFD00E18624BAD64D49F462` — **matched** the downloaded file.
- ECSS-Q-ST-80C Rev.2, *Software product assurance*, 30 April 2025. URL
  https://ecss.nl/standard/ecss-q-st-80c-rev-2-software-product-assurance-30-april-2025/ ;
  PDF `ECSS-Q-ST-80C-Rev.2(30April2025).pdf`; issuer MD5
  `1F39D5B1A1BCDA8406EAD96CCBB70B93` — **matched** the downloaded file.

**Project:**

- Project norms: `docs/rules.md` (N1–N20), especially **N3** (backwards ECSS
  documentation) and **N4** (bidirectional traceability).
- Traceability manifest: `docs/requirements/traceability.yaml`.
- Human matrix: `docs/requirements/traceability.md`.
- Requirements: `docs/requirements/SRS.md`, `docs/requirements/SSS.md`.
- Design: `docs/design/software-design.md`.
- Coding style: `docs/engineering/coding-style.md`.
- Class-B baseline: `docs/quality/ecss-criticality-b.md`; compliance matrix
  `docs/quality/ecss-compliance-matrix.md`.
- Gate: `tools/scripts/trace.py`.

## 3. What ECSS actually requires (Fact, from the issuer PDFs)

### 3.1 The mandatory traceability chain

ECSS-E-ST-40C Rev.1 makes traceability between requirements, design, code and
tests a **normative requirement** throughout the verification process
(clause 5.8.3), and the required *evidence object* is a **traceability matrix**,
not an in-code annotation:

| ECSS-E-ST-40C Rev.1 clause (Fact) | Requirement (paraphrase of the shall-text) | Required output |
| --------------------------------- | ------------------------------------------ | --------------- |
| 5.8.3.2a.2–3 | "the traceability between system requirements and software requirements is complete"; untraced requirements justified | **Requirements traceability matrices** (5.8.3.2a eo a) |
| 5.8.3.3a.3 | "the traceability between the requirements and the software components is complete" | **Software architectural design to requirements traceability matrices** (5.8.3.3a eo a) |
| 5.8.3.4a.3 | "the traceability between the architecture and the detailed design is complete" | **Detailed design traceability matrices** (5.8.3.4a eo a, CDR) |
| **5.8.3.5a.3** | "the code is **traceable to design and requirements**, testable, correct, and in conformity to software requirements and coding standards" | **Software code traceability matrices** (5.8.3.5a eo a, CDR) |
| 5.8.3.6a.2 | "the unit tests are traceable to software requirements, design and code" | **Software unit tests traceability matrices** (5.8.3.6a eo a, CDR) |
| 5.8.3.8a | validation covers all requirements of the TS and RB | Traceability of RB→validation spec and TS→validation spec (5.8.3.8a eo a/b) |
| 5.7.3.5a | "the acceptance tests shall be traced to the requirements baseline" | Traceability of acceptance tests to RB |

**Criticality applicability (Fact).** ECSS-E-ST-40C Rev.1 **Annex R, Table R-1
"Criticality applicability"** marks these outputs **Y (applicable)** for
criticality **B**, specifically `5.8.3.3a eo a` (architectural design to
requirements), `5.8.3.4a eo a` (detailed design), `5.8.3.5a eo a` (**software
code traceability matrices**), `5.8.3.6a eo a` (unit tests), `5.8.3.2a eo a`
(requirements), and `5.8.3.8a eo a/b`. The detailed-design requirements
`5.5.2.1`–`5.5.2.10` are also **Y** for B. So for criticality B, code-to-design
and code-to-requirements traceability matrices are **required deliverables**.

**PA verification of the matrices (Fact).** ECSS-Q-ST-80C Rev.2 clause
**6.2.6.12a** states: "Traceability matrices (as defined in ECSS-E-ST-40
clause 5.8) shall be verified at each milestone" (expected output: Software
product assurance milestone report; SRR, PDR, CDR, QR, AR, ORR). Clause
**6.2.4.7a.3(e)** requires traceability matrices to be controlled documents.
Clause **6.2.7.4a.3** requires, for reuse, forward and backward traceability
between system requirements, software requirements, design and code.

### 3.2 The mandatory **form** is a matrix, not in-source tagging

**Fact / key finding.** The expected output of the code-traceability requirement
is consistently "**Software code traceability matrices**" (5.8.3.5a eo a; Annex
R Table R-1). The standard does **not** specify that these matrices live inside
the source code. It explicitly allows the trace to be realised elsewhere, by
reference:

- ECSS-E-ST-40C Rev.1 **Annex M (SVR DRD), M.2.1<4.4>a.1**: the SVR shall present
  "Traceability (**when not already included in related software design documents
  or software code**), presenting the following traceability matrices: software
  detailed design to software architectural design, **Software code to software
  detailed design**, Software unit test to requirements, design."

  This is the decisive clause for the owner's question: **"already included …
  in software code" is an expressly permitted means** of carrying the trace — it
  is not an obligation, and it is not the only permitted means. A separate
  matrix in the design/verification documents satisfies the same requirement.

- ECSS-E-ST-40C Rev.1 **Annex D (SRS DRD), D.2.1<7>a**: "The SRS **shall** report
  the traceability matrices … forward … and backward" between the upper-level
  specification and the SRS requirements; D.2.1<7>b permits this to be "separately
  provided in the DJF" with a reference.

- ECSS-E-ST-40C Rev.1 **Annex F (SDD DRD), F.2.1<6>a**: "The SDD **shall**
  provide traceability matrices … from the software requirements to component
  down to the lower identified component in the software hierarchy (forward) and
  … from the software components … up to the software requirements (backward)";
  F.2.1<6>b permits separate provision in the DJF by reference.

- ECSS-E-ST-40C Rev.1 **Annex F, F.2.1<5.4.4>a** uses a **non-normative**
  auxiliary verb: "The purpose of a component **should** describe its trace to the
  software requirements that it implements" (NOTE: "Backward traceability depends
  upon each component description explicitly referencing the requirements that
  justify its existence"). This is a **recommended practice** for the SDD's
  *component description*, not a mandate to annotate source files.

### 3.3 Exhaustive search for any in-code annotation requirement — none found

**Fact.** A full-text scan of both issuer PDFs for the strings `in the source
code`, `within the code`, `comment`, `inline`, `annotation`, `requirement
identifier`, and `tag` returned **no clause requiring requirement/SDD IDs to be
embedded in source code**:

- The word "comment"/"commentary" appears in ECSS-Q-ST-80C **6.3.4.1a** only as
  part of the coding-standard requirement — "Coding standards (including
  security, consistent naming conventions and **adequate commentary rules**)
  shall be specified and observed" — i.e. the project must *have* a comment
  convention, not a requirement-ID convention.
- ECSS-E-ST-40C Rev.1 **Annex U (informative) "Software code verification"**
  lists 14 recommended code checks; **none** concerns requirement-ID traceability
  comments.
- The only "traceability between … code" clauses concern code-to-requirements/
  design (5.8.3.5a) and **source code to object code** (5.8.3.5e), the latter
  being a compiler/optimisation concern, unrelated to requirement tags.

**Conclusion of §3 (Fact, High confidence).** ECSS for criticality B **requires**
bidirectional traceability among SSS↔SRS↔architecture↔detailed design↔code↔tests,
evidenced by **traceability matrices** that may be held in the DJF (design or
verification documents) **or** in the code. **In-source SRS/SDD requirement
tagging is one acceptable means; it is not a requirement and not a recommended
practice in ECSS-E-ST-40C Rev.1 or ECSS-Q-ST-80C Rev.2.** Therefore the premise
"if the code does not carry requirement IDs, an NC must be opened" is **not
supported by ECSS**.

## 4. frick-fsw assessment (evidence-based)

### 4.1 Requirement-level traceability — present and gate-enforced (Fact)

- `docs/requirements/traceability.yaml` is the single source of truth with, per
  requirement, `parent_sss`, `implementation:` (a list of **source/config file
  paths**) and `tests:`/`lean:` evidence — e.g. `SRS-REQ-001` →
  `implementation: ["src/core/sb/sb.c", "src/core/sb/sb.h"]`, `tests:
  ["tests/unit/test_sb.c"]` (`traceability.yaml:40-45`).
- `tools/scripts/trace.py` enforces convergence: every non-derived requirement
  must have implementation, test/CI evidence and Lean or an explicit
  `lean_note`; **every implementation/test path must exist**; every SRS has
  exactly one SSS parent and every SSS at least one child (bidirectional
  hierarchy) — `tools/scripts/trace.py:10-19,276-284`.
- `docs/requirements/traceability.md` presents the same content as a human
  matrix (SSS→SRS at `:24`, SRS verification at `:55`, SSS verification at
  `:119`, bottom-up gate checks at `:166`).
- The thus-produced SSS↔SRS↔implementation-file↔tests/Lean chain satisfies the
  *requirements-level* matrices of §3.1 (E-ST-40C 5.8.3.2a, and the intent of
  5.8.3.8a at requirements granularity).

### 4.2 In-source requirement IDs — sparse, but this is **not** an NC (Fact)

- `src/`+`config/` contain **82** C/H files; only **19** contain any
  `SRS-REQ-*`/`SSS-REQ-*` string, and even those are informal banner comments
  (e.g. `src/hal/hal_eth.h:6`, `src/core/time/time.h:20`), with **no** uniform
  scheme and **no** design/unit identifier. **63/82 files carry none.** In
  `tests/`, 12/17 files mention requirement IDs informally.
- **Because §3 establishes that ECSS does not require in-code tags**, this
  sparseness is **not** itself a non-conformity. It is, at most, a *reduction of
  the redundant evidence* that would otherwise make the backward trace
  obvious in the code. See Recommendation R-1 in §6.

### 4.3 Design-level traceability — **not demonstrable** (Fact → NC)

This is the genuine ECSS gap, and it is a **different** issue from in-source
tagging:

- **SDD DRD Annex F <6>a requires the SDD itself to present the
  requirements→design-component traceability matrices, forward and backward.**
  `docs/design/software-design.md` does **not** contain such a matrix. Its §6
  merely delegates: "Design elements trace to SRS/SSS through
  `docs/requirements/traceability.yaml` (**implementation paths**)"
  (`software-design.md:92-95`). But `traceability.yaml` maps requirements to
  **source files**, not to design components/units, so it is **not** a
  requirement→design-component matrix.
- The SDD is 110 lines and describes an **architectural/module-level** design
  only. It references just **four** requirement IDs in total (`SRS-REQ-015`/
  `SSS-REQ-010` at `software-design.md:25`; `SSS-REQ-018`/`SRS-REQ-033` at
  `:78`). There is **no detailed design down to software units** (required by
  E-ST-40C 5.5.2.1, applicable **Y** for B).
- Consequently the SVR outputs of **Annex M M.2.1<4.4>a.1** — "software detailed
  design to software architectural design", "**software code to software
  detailed design**", "software unit test to requirements, design" — cannot be
  demonstrated: there is no detailed design to trace *to*.
- The project's own records acknowledge the substance but **misclassify it as a
  class-A-only gap**: `GAP-A9` ("SDD detailed design completeness for every
  module", `docs/requirements/traceability.md:150`; `docs/quality/ecss-compliance-matrix.md:85,98-99`).
  Per **Annex R Table R-1**, `5.5.2.x` and `5.8.3.4a eo a` are **Y for B**, so
  the design traceability obligation is on the **B** baseline, not only class A.
- `docs/quality/ecss-compliance-matrix.md:26` asserts the SDD is "present (added
  for this matrix)" and `:95-97` claims requirements, design, interfaces … are
  "all present and traced". For the *design→requirements* matrix this overstates
  the state.

### 4.4 Verdict

**Partially conformant.**
(1) ECSS does **not** require SRS/SDD IDs in source code → the codebase is
**conformant** on that point, and **no NC is raised for missing in-source
tags**. (2) Requirement-level bidirectional traceability is strong and
gate-enforced. (3) Design-level traceability (requirement→design component, and
code→detailed design) is **not demonstrable**, and the SDD does not present the
matrix that E-ST-40C Annex F <6>a requires. **One NC (Minor) is therefore
opened for the design-level traceability gap — not for in-source tagging.**

## 5. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| 1 | Comment | E-ST-40C 5.8.3.5a; M.2.1<4.4>a.1 | **In-source requirement IDs are not required by ECSS.** Code traceability is required, but as a matrix that may be held in design/verification documents *or* in code. The owner's premise is not supported. | E-ST-40C Rev.1 5.8.3.5a eo a ("Software code traceability matrices"); Annex M M.2.1<4.4>a.1 ("when not already included in related software design documents or software code"); full-text scan found no in-code-tagging clause (§3.3) | No action required on in-source tagging. Retain the current file-level requirement→implementation manifest. |
| 2 | **Minor** | E-ST-40C 5.5.2.1; 5.8.3.3a eo a; **5.8.3.4a eo a**; **Annex F F.2.1<6>a** | The SDD does not present the requirements→design-component traceability matrices (forward and backward) required by the SDD DRD, and no detailed design exists, so code→detailed-design and detailed-design→architectural-design traces cannot be demonstrated. | `docs/design/software-design.md:92-95` delegates to implementation *file* paths; only 4 requirement IDs referenced (`:25,:78`); no unit-level design; `docs/quality/ecss-compliance-matrix.md:26,95-97` overstates "design … traced"; `GAP-A9` filed as class-A though `5.5.2.x`/`5.8.3.4a` are Y for B (Annex R Table R-1) | **NC PA-67-01** (§6). Reclassify the design-trace requirement onto the B baseline, add the Annex F <6> matrices to the SDD, and record their milestone verification (Q-ST-80C 6.2.6.12a). |
| 3 | Comment | E-ST-40C Annex M M.2.1<4.4>a.1 | The requirement→test trace is present at requirement granularity, but there is no explicit *design*→*test* nor *unit*→*test* mapping. Once a detailed design exists, add the design/unit dimension to the test traceability. | `traceability.yaml` (tests keyed per requirement); `traceability.md:55,119` | Fold into NC PA-67-01 corrective action; no separate NC. |

**Severity definition** (from `review-template.md`): **Major** = non-conformity
that disqualifies the item for the applicable criticality, or a defect that can
defeat a requirement or a safety barrier; **Minor** = non-conformity that does
not invalidate the requirement but must be corrected or justified; **Comment** =
observation/recommendation, no derogation.

Finding 2 is **Minor** because the requirement→code and requirement→test traces
still exist, so no requirement is orphaned and no safety barrier is defeated;
it must nevertheless be corrected to close the B-baseline design traceability
obligation. **Escalation condition:** if the project claims CDR/qualification
readiness for this baseline, the absent detailed-design traceability matrices
(5.8.3.4a eo a) become a **Major** gate blocker.

## 6. Non-conformance PA-67-01

| Field | Content |
| ----- | ------- |
| **NC id** | PA-67-01 |
| **Severity** | Minor |
| **Standard reference** | ECSS-E-ST-40C Rev.1 (30 Apr 2025) clause 5.5.2.1 and 5.8.3.3a/5.8.3.4a eo a; **Annex F (SDD DRD) F.2.1<6>a**; Annex R Table R-1 (Y for criticality B); ECSS-Q-ST-80C Rev.2 (30 Apr 2025) 6.2.6.12a (traceability matrices verified at each milestone) |
| **Finding** | The software design does not present the bidirectional requirements→design-component traceability matrices that the SDD DRD requires, and no detailed design down to software units exists; as a result the code→detailed-design and detailed-design→architectural-design traces required by E-ST-40C 5.8.3.4a/M.2.1<4.4>a.1 are not demonstrable. `traceability.yaml` traces requirements to **source files**, not to design components/units, so it does not discharge the SDD DRD obligation. |
| **Evidence (file:line)** | `docs/design/software-design.md:92-95` (§6 delegates to implementation *file* paths); `docs/design/software-design.md:25,78` (only 4 requirement IDs referenced in 110 lines); absence of any detailed-design/unit section; `docs/quality/ecss-compliance-matrix.md:26` ("SDD … architectural + detailed design") and `:95-97` ("design … all present and traced") overstate the state; `docs/requirements/traceability.yaml:44-53` (implementation = file paths); `docs/requirements/traceability.md:150` (GAP-A9 filed as class-A). |
| **Root cause** | The design artifacts stop at the architectural/module level and traceability is implemented only as requirement→source-file (a convenient generator-friendly form), so the design dimension of the E-ST-40C traceability chain was never materialised; GAP-A9 was scoped to class A. |
| **Corrective action** | 1. Add to the SDD the **requirements→design-component** forward and backward matrices per Annex F <6>a, keyed to the SDD's named components (`software-design.md:36-49`), not to source files. 2. Add an explicit **design-component ↔ source-file/unit** mapping so **code→detailed-design** is demonstrable; where the project deliberately treats "design component = software unit" at this scale, state that simplification and its justification (subject to customer tailoring). 3. Record the milestone **verification of the matrices** per ECSS-Q-ST-80C 6.2.6.12a. 4. Correct `docs/quality/ecss-compliance-matrix.md:26,95-97` to state design traceability truthfully, and either re-scope GAP-A9 onto the **B** baseline or justify its class-A scoping against Annex R Table R-1. |
| **Owner** | frick-fsw (design/documentation work package); PA to verify closure |
| **Sprint to resolve** | **Sprint 0/1** (immediate) |
| **Closure / re-verification** | Independent N12/N20 re-review: confirm the SDD contains the Annex F <6> matrices, `trace.py` (or an extension) can reconcile them with `traceability.yaml`, the compliance matrix is truthful, and Q-ST-80C 6.2.6.12a milestone records exist. |

**Recommendation R-1 (Comment, not part of the NC).** Although ECSS does not
require it, adding a **lightweight, uniform** requirement-ID reference in the
header comment of each authored source/unit (and, once it exists, the design-unit
ID) would make the backward trace evident to a code reader without a tool, and
would strengthen the `5.8.3.5a` evidence. If adopted, define it as a project
rule so `trace.py` can enforce it; do **not** present it as an ECSS mandate.

## 7. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS — 1 Minor (PA-67-01) tracked to closure**
- [ ] REJECTED — one or more Major findings

**Answer to the owner, in one paragraph.** No — ECSS does **not** require the
SRS or SDD to be traced *inside the source code*. For criticality B, ECSS
**does** require bidirectional traceability SSS↔SRS↔architecture↔detailed
design↔code↔tests, evidenced by **traceability matrices** (E-ST-40C 5.8.3.5a,
5.8.3.4a, 5.8.3.3a, 5.8.3.2a, 5.8.3.6a; Annex R Table R-1), and it explicitly
permits those matrices to sit in the design/verification documents **or** in the
code (Annex M M.2.1<4.4>a.1). In-source requirement IDs are one **acceptable
means**, not a requirement and not a recommended practice, so the codebase is
**conformant** on that specific point — no NC for missing in-source tags.
However, the codebase is only **partially conformant** overall: it has strong,
gate-enforced requirement-level traceability, but the **design-level**
traceability matrices that the SDD DRD (Annex F <6>) requires do not exist and
there is no detailed design, so the code→design trace required for B cannot be
demonstrated. That is a genuine, separate non-conformity: **NC PA-67-01
(Minor)**, to be resolved in **Sprint 0/1**.

## 8. Confidence and evidence limitations

```text
Decision/product and configuration:
  frick-fsw @ main 4c9516e; ECSS criticality B target.
Claim type: mixed (Fact for ECSS clause text; Inference for conformance verdict)
Applicable baseline and approved tailoring:
  ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025); project
  targets criticality B (docs/quality/ecss-criticality-b.md). A formal
  customer-approved tailoring/compliance matrix was NOT available to this reviewer.
Evidence consulted (issuer, ID/revision, artifact/version, date):
  E-ST-40C Rev.1 PDF (MD5 227F26315BFD00E18624BAD64D49F462, matched) clauses
  5.5.2.1, 5.7.3.5a, 5.8.3.2a, 5.8.3.3a, 5.8.3.4a, 5.8.3.5a, 5.8.3.6a, 5.8.3.8a,
  Annex D <7>, Annex F <5.4.4>/<6>, Annex M <4.4>, Annex R Table R-1, Annex U.
  Q-ST-80C Rev.2 PDF (MD5 1F39D5B1A1BCDA8406EAD96CCBB70B93, matched) clauses
  6.2.4.7, 6.2.6.12, 6.2.7.4, 6.3.4.1.
  Repo files read in full or by grep as cited (line numbers in §5/§6).
Cross-check and reconciliation:
  The two issuer PDFs are mutually consistent; the project's own rules N3/N4 match
  ECSS intent. No conflict between the standards and the project on the in-source
  question.
Confidence:
  High for the ECSS clause facts and for "in-source tagging is NOT required"
  (exhaustive full-text scan of both standards; checksums verified).
  Medium-high for the conformance verdict (clause text and repo evidence are
  direct, but the customer tailoring matrix was unavailable).
Basis and detection limits:
  Full normative text read; the "no in-code-tag clause" conclusion is based on a
  complete keyword scan, which is strong but not a formal proof of absence.
Missing/inaccessible evidence (including paywalls):
  Customer contract / approved tailoring matrix; system hazard analysis (not
  relevant to traceability). No paywall issues for these two ECSS documents.
Assumptions and expiry/recheck date:
  Assumes no contract clause imposes stricter in-source tagging; assumes the
  criticality-B target is authoritative. Recheck on next baseline change or
  2027-04-11.
Consequences if wrong:
  If a contract mandates in-source tags, finding 1 would invert and an additional
  NC would be required. If detailed-design matrices are contractually waived, PA-67-01
  would downgrade to a Comment.
Required independent human reviewer/authority:
  The project's authorized engineering/PA/release authority; this agent cannot
  approve, accept residual risk, or close the NC.
```

## 9. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9) | — | | |
| R2 (N9) | — | | |
| R3 (N9) | — | | |
| R4 (N9) | — | | |
| **PA (N20)** | **independent agent, FRICK-REVIEW-0067** | **2026-10-11** | **APPROVED WITH COMMENTS (1 Minor: PA-67-01)** |

## 10. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| PA-67-01 | Minor | E-ST-40C 5.5.2.1, 5.8.3.3a/5.8.3.4a eo a, Annex F <6>a; Annex R Table R-1; Q-ST-80C 6.2.6.12a | SDD lacks the bidirectional requirements→design-component traceability matrices; no detailed design, so code→detailed-design trace is not demonstrable | Open — corrective action §6, resolve in **Sprint 0/1** | pending (independent N12/N20 re-review) | frick-fsw / PA verify | **open** | pending |

---

Committed and pushed with the reviewed change (norm N11).