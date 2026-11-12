# DOC-R4-20261112 — Software Documentation Review (N17, R4 profile)

**Repository:** /home/nicola/fsw/frick-fsw  
**Branch:** main  
**HEAD:** c3fe024d8636c172eec42af020d85e46aa84371a (short: c3fe024)  
**Target:** Microchip RT PolarFire SoC RTPFS160ZT (RISC-V) under RTEMS 6.2  
**Criticality target:** ECSS class B  
**Reviewer profile:** Senior Aeronautics and Space Software Documentation Engineer (R4) — independent documentation review  
**Review date:** 2026-11-12  
**Change under review (uncommitted working tree):**  
`tests/unit/test_pus.c` — added `test_service1_progress_success` verifying PUS service 1 subservice 5 (TC[17,1] with `FF_PUS_ACK_PROGRESS` → TM[1,5]) end-to-end through `FF_PusIngest`. No production source files (`src/`) modified. Registered in `FF_TEST_MAIN`.  
**Gates reported as passing:** `make build` and `make test` green; `make verify-lean` green; `make trace` green; format; license.

---

## 1. Change characterisation (R4 assessment)

The delta is a single new static test function plus one `FF_TEST_MAIN` registration entry in the unit-test file `tests/unit/test_pus.c`.

Verified scope of the delta:
- No source-code change (`src/` untouched); no behaviour change to `FF_PusIngest`, `FF_PusSendVerification`, or any PUS service.
- No hardware / target / device-tree / BSP / HAL / clock / timing / FDIR change.
- No requirements, traceability matrix (`docs/requirements/traceability.yaml`), SSS/SRS, or verification artefact change; no traceability requirement ID added or removed.
- No controlled deliverable document (ICD, plans, reports, user/operator material) changed.
- Norm N2 (English): all added comments are English — **compliant** (see §4).
- Norm N8 (complete, current, deployable documentation): no separate document requires updating; the change is test-only and self-documenting, and is aligned with the existing test spec — **compliant** (see §4).
- Norm N16 (revision history): no revision-history entry required — no controlled document changed by this delta (see §4).

---

## 2. Accuracy of the new test comments against implementation (review item 4)

**Fact, High confidence — directly observed in source and controlled docs.**

`FF_PusSendVerification` (`src/pus/pus.c:316-317`) implements the stage→subservice map:

| stage | success=1 | success=0 |
|---|---|---|
| ACCEPTANCE | subservice 1 | subservice 2 |
| START | subservice 3 | subservice 4 |
| PROGRESS | subservice 5 | subservice 6 |
| COMPLETION | subservice 7 | subservice 8 |

The progress-ack call site (`src/pus/pus.c:456`) invokes `FF_PusSendVerification(&tc, FF_PUS_STAGE_PROGRESS, 1u, 0u)` when `tc.ack_flags & FF_PUS_ACK_PROGRESS` (`pus.c:417`), i.e. `success=1` → subservice 5 → TM[1,5]. The ICD names subservice 5 "progress success" (`docs/interfaces/ICD.md:105`: `| 1 | 3 / 5 | start / progress success | TM | errcode 0 |`).

Each comment claim in the new function was checked:

| Comment claim (file:line) | Verification | Verdict |
|---|---|---|
| "`test_ping_roundtrip` covers acceptance (1,1), start (1,3) and completion (1,7) but not progress (1,5)." (`test_pus.c:560-562`) | `test_ping_roundtrip` sets `ack_flags = ACCEPTANCE | START | COMPLETION` (`:200-201`) and asserts `(1,1)`, `(1,3)`, `(1,7)`, `(17,2)` (`:214-217`); it never asserts `(1,5)`. | **Accurate** |
| "TC[17,1] with the `FF_PUS_ACK_PROGRESS` flag emits TM[1,5] ... through the full TC ingestion path." (`test_pus.c:560-562`) | Path: `FF_CcsdsEncodeTc` → `FF_PusIngest` → `FF_PusSendVerification(STAGE_PROGRESS, success=1)` → subservice 5 (`pus.c:316-317,417,456`). | **Accurate** |
| "Expected TM: 1,5 progress success - 17,2 ping reply. No acceptance or completion TM since those ack flags were not requested." (`test_pus.c:589-594`) | Matches the ack-flag matrix `N22-TS1-19` (`docs/vv/test-spec/ts-1-protocol-core.md:120`): PROGRESS-only → exactly one `(1,5)`, no `(1,1)`/`(1,7)`. Test asserts `stream_has(1,5)`, `stream_has(17,2)`, `!stream_has(1,1)`, `!stream_has(1,7)`. | **Accurate** |
| "Count should be exactly 2: one (1,5) and one (17,2)." (`test_pus.c:596-598`) | `stream_count(1,5) == 1` and `stream_count(17,2) == 1`. | **Accurate** |

No comment states an implementation detail as a requirement, no placeholder is rendered as approved content, and no TBD/TBC is concealed. The header term "S1 Service 1" correctly identifies the verification/acknowledgement service that emits TM[1,5] in response to a service-17 TC — this is standard PUS-C behaviour and is not mis-stated.

---

## 3. Whether any other documentation needs updating (review items 1 and 3)

**Finding: No.** The change is test-only; no controlled document (ICD, SRS/SSS, traceability matrix, plans, reports, procedures) is affected.

- The behaviour covered (progress-only acknowledgement → TM[1,5]) is already specified in the existing test spec `N22-TS1-19` (`docs/vv/test-spec/ts-1-protocol-core.md:120`), so the new test implements rather than extends documented coverage. No documentation/test divergence is introduced.
- The evidence-mapping table `docs/vv/evidence/2026-11-02-n22-test-evidence-mapping.md:70` already claims `test_pus.c` covers SRS-REQ-006 Progress (1,5) (attributed to `test_ping_roundtrip`). The new test makes the file-level claim genuinely true (see DOC-R4-004, a pre-existing attribution nuance, non-blocking).
- Aggregate CTest count records in `docs/vv/verification-results.md` are snapshot gate-run records tied to specific evidence logs, not a living per-file count that this delta must amend; the reported gates pass on the current tree.

No SRS/SSS requirement ID, trace link, ICD row, or verification artefact requires a corresponding change.

---

## 4. Norm N2 and Norm N8 (review items 2 and 3)

- **Norm N2 (English):** The added header, purpose, and inline comments are English. Abbreviations (TC, TM, S1, ack) and the symbol `FF_PUS_ACK_PROGRESS` are established in-repo code terminology. No non-English text, no vague terms ("fast", "appropriate"), no passive actors or undefined pronouns. **Compliant — no finding.**
- **Norm N8 (complete, current, deployable documentation):** The delta is self-contained and deployable as-is; no separate document needs updating; test and documented coverage (N22-TS1-19) are aligned. **Compliant — no finding.**
- **Norm N16 (revision history):** No controlled document is changed by this delta, so no per-document revision-history entry is required. **Compliant — no finding.**

---

## 5. Findings

| Finding ID | Classification | File:line | Blocks commit/push? | Finding |
|---|---|---|---|---|
| DOC-R4-001 | Minor | `tests/unit/test_pus.c:558-599,1274` | No | Norm N2 (English): all new comments (header, purpose, inline, `FF_TEST_MAIN` registration) are English; terminology is established in-repo code terminology. Compliant — no language defect. |
| DOC-R4-002 | Minor | `tests/unit/test_pus.c:560-562,589-594`; `src/pus/pus.c:316-317,417,456`; `docs/interfaces/ICD.md:105`; `docs/vv/test-spec/ts-1-protocol-core.md:120` | No | Comment accuracy verified against implementation: `test_ping_roundtrip` covers (1,1)/(1,3)/(1,7) but not (1,5); `FF_PusSendVerification` maps STAGE_PROGRESS success→subservice 5 (TM[1,5]) via the progress-ack call site; ICD and test spec name subservice 5 "progress success". All comment claims accurate; no implementation disguised as comment; no placeholder rendered as approved content. |
| DOC-R4-003 | Minor | `tests/unit/test_pus.c` (delta); `docs/vv/test-spec/ts-1-protocol-core.md:120` | No | Norm N8: test-only delta; no controlled document requires updating; new test implements already-specified coverage (N22-TS1-19 PROGRESS-only → exactly one (1,5), no (1,1)/(1,7)). Norm N16: no revision-history entry required because no controlled document changed. Compliant — no N8/N16 finding. |
| DOC-R4-004 | Minor | `docs/vv/evidence/2026-11-02-n22-test-evidence-mapping.md:70` | No (pre-existing, non-blocking) | Pre-existing attribution nuance, not introduced by this change: the SRS-REQ-006 row attributes Progress (1,5) coverage to `test_ping_roundtrip`, which does not emit (1,5) (ack flags ACCEPTANCE\|START\|COMPLETION only). The new `test_service1_progress_success` genuinely covers Progress (1,5), so the file-level claim is now satisfied; only the case attribution is imprecise. Recommend aligning attribution to `test_service1_progress_success` at the next documentation cycle. Does not block commit/push. |

**No Major findings. No finding blocks commit/push.**

---

## 6. Relationship to open NCs

- Untouched NCs (per context): NC-EXT-19/90, N22-TS5-47, GAP-A5, NC-PAL-01. None are affected by this test-only delta.
- NC-EXT-26 / NC-EXT-94 (Service 1 subservice 4/6 emission status) concern different subservice rows and are unrelated to the progress (1,5) path added here; no closure or regression is implied by this change.

---

## 7. Independent reviews already on record

None specific to this delta are on record at review time. This R4 documentation review is the N17 documentation gate for the change. Any PA (N20), V&V (N22), systems (R1/R3), or FSW (R1) reviews required by the project's sequencing remain the responsibility of the designated authorities and are not implied or closed by this review.

---

## 8. Recommendation (R4 documentation perspective)

**Commit/push: permitted** from the documentation (N17/R4) standpoint, provided:

- the project's remaining applicable gates are completed by the designated authorities (N7 sequencing; PA/N20 review where required; N1/N3/N4/N10/N11/N15/N18/N20/N21/N22 as applicable),
- DOC-R4-004 (evidence-mapping case attribution) is treated as a non-blocking recommendation for the next documentation cycle,
- no new controlled document is introduced by this change without its own N17 review.

This review does **not** waive any separate safety, security, regulatory, contractual, or release gate, does **not** certify compliance, does **not** close any NC or approve any risk, and does **not** substitute for engineering evidence. Technical content and safety-critical decisions require review and approval by qualified authorized humans.

---

## 9. Review revision history

| Version | Date | Author/role | Description |
|---|---|---|---|
| 1.0 | 2026-11-12 | Senior SW Documentation Engineer (R4) | Independent N17 documentation review of `tests/unit/test_pus.c` `test_service1_progress_success` (TC[17,1] progress → TM[1,5]) |
