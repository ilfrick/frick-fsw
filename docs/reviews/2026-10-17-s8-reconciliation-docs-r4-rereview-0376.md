# R4 (Documentation) Independent Re-review — S8 Reconciliation Closure (FRICK-REVIEW-0376)

| Field | Value |
| --- | --- |
| Profile | R4 (Senior SW documentation reviewer) — independent re-verification |
| Date | 2026-10-17 |
| Scope | Independent verification of the self-authored R4 re-review report at `docs/reviews/2026-10-17-s8-reconciliation-docs-r4-rereview-0376.md`, checking every count, hash, line citation, and text-attribution claim against the actual repository artifacts at `git diff b8c3aee -- <path>` (working tree) and `git show b8c3aee:<path>` (pushed baseline) |
| Reference revision | git commit `b8c3aee` + uncommitted working-tree delta |
| Conclusion | **CORRECTED APPROVED WITH COMMENTS** — the underlying documentation changes are substantively correct, but the re-review report contains factual errors in its evidence citations and its sole finding is premised on a non-existent text string. Corrections are required (F-0376-01..04) before the report stands as an accurate record; F-0376-05/06 are non-blocking. N9 independence is not established by this self-contextual re-assessment; PA 0377 independent re-verification remains required |

> **Independence declaration.** This report was produced by re-reading every
> cited artifact from `git diff b8c3aee -- <path>` and `git show b8c3aee:<path>`,
> recomputing every SHA-256 hash with `sha256sum`, re-extracting every test-spec
> case-table count with an independent Python parser, and independently verifying
> every line-number and text-attribution claim with `grep -n`. The original 0376
> report was self-authored by the change author (per its §1 Independence
> limitation note) due to the dedicated R4 agent being unavailable. This
> re-review was performed from fresh context.
>
> **What this review does NOT establish.** This is a documentation evidential
> review, not a technical correctness review of ICD §4 wire behavior (that is
> R3 0375's scope, explicitly excluded here per norm N9). It does not establish
> Phase/Release/N17/N20 approval, PUS-C compliance, or target physical-hardware
> evidence. The working tree is an uncommitted candidate against pushed baseline
> `b8c3aee`.

## 1. Method

Every claim in the source report was checked against the actual file contents:

- **Hashes**: all 5 evidence logs recomputed with `sha256sum` (Python `hashlib` used as cross-check).
- **Counts**: all test-spec case tables re-parsed with an independent Python script counting the status column (exact `covered`, `gap`, `covered (…)` variants, `host covered (…)`, `exploratory`).
- **Line citations**: every `file:line` reference verified with `git show b8c3aee:<file>` for baseline and `sed -n` for working tree.
- **Text attributions**: every quoted or paraphrased text string verified with `grep -n`.
- **Git diff hunk counts**: verified with `git diff b8c3aee -- <file> | grep -c '^@@'`.

## 2. Artifacts inspected

| Artifact | Source checked | Method |
| --- | --- | --- |
| `docs/interfaces/ICD.md` | working tree vs `b8c3aee` | `git diff b8c3aee` (4 hunks) |
| `docs/engineering/service-development.md` | working tree vs `b8c3aee` | `git diff b8c3aee` + `grep -n` vs `src/pus/pus.h` and `src/pus/pus_services.c` |
| `docs/engineering/user-integration-manual.md` | working tree vs `b8c3aee` | `git diff b8c3aee` + `grep -n` |
| `docs/vv/test-spec/ts-1-protocol-core.md` | working tree | Python parse of case table + read §3.1 coverage summary |
| `docs/vv/test-spec/ts-5-core-services.md` | working tree | Python parse of case table + read §3.2 coverage summary |
| `docs/vv/test-spec/ts-6-sw-robustness.md` | working tree | Python parse of case table + read §3.2 coverage summary |
| `docs/planning/sprint-plan.md` | working tree | `grep -n` for header/history/NC rows |
| `docs/reviews/README.md` | working tree vs `b8c3aee` | `grep -n` + `git diff` for `||` rows |
| `docs/vv/verification-results.md` | working tree vs `b8c3aee` | `grep -n` for ICD ref, revision, history |
| `docs/status/loop-audit.md` | working tree vs `b8c3aee` | `grep -n` for N11 row, §3 gap text, history |
| `docs/status/handover.md` | working tree vs `b8c3aee` | `grep -n` for ICD ref, revision |
| `docs/status/status.json` | working tree + `b8c3aee` | `python3 -m json.tool` + key inspection |
| `CHANGELOG.md` | working tree vs `b8c3aee` | `grep -n` for 0370–0376 + superseded annotation |
| `docs/vv/evidence/2026-10-17-s8-reconciliation-*.log` (×5) | working tree | `sha256sum` |

## 3. Verification results

### 3.1 ICD.md rev 1.4 — documentation consistency (all verified ✓)

- **Diff confinement**: `git diff b8c3aee -- docs/interfaces/ICD.md` = exactly **4 hunks** (confirmed via `grep -c '^@@'`). Hunks touch only: (a) header Revision/Date (1.2→1.4), (b) §4 intro sentence, (c) §4 body rows, (d) provisional note + History table (1.3 + 1.4 rows). §1, §2, §3 (Scope), §5 (Time code), §6 (Mission tailoring) are untouched. **No silent change outside §4/intro/history.**
- **S3 enable/disable rows**: read `set periodic-generation **state flag** to enabled/disabled` with inline qualifier `(state flag only; no periodic emission path exists in this release — TM[3,25] is produced only by TC[3,27])`. ✓
- **TC[3,27] row**: states `count N[1] (N >= 1)`, `payload length must be exactly 1+N octets`, `reporting the **first** error only while still serving the remaining valid ids`. ✓
- **TM[6,6] row**: documents raw region octets payload, reject/error causes (`FF_BAD_SIZE` for `region_id==0`/`length==0`, `FF_ERROR` for unknown/out-of-range), on-wire non-distinguishability. ✓
- **Service 11 stub**: present, marked `FF_NOT_IMPLEMENTED`. ✓
- **§4 intro**: states the table is a framework reference, not a selected mission profile. ✓
- **Provisional note**: attributes every row in §4 to ADR-0020; names NC-EXT-14/15/16/17/18, NC-S9-01/M15. Does not present the profile as approved. ✓

### 3.2 Engineering docs — registration signature (verified ✓, with error)

- `docs/engineering/service-development.md:101` reads `FF_PusRegisterMemoryRegion(region_id, base, size)` ✓
- `docs/engineering/user-integration-manual.md:101` reads `FF_PusRegisterMemoryRegion(region_id, base, size)` ✓
- Git diffs confirm both files changed from `FF_PusRegisterMemoryRegion(base, size)` → `FF_PusRegisterMemoryRegion(region_id, base, size)` ✓
- The signature matches the source: `src/pus/pus.h:69` reads `FF_Status_t FF_PusRegisterMemoryRegion(uint8_t region_id, const void *base, uint32_t size);` ✓
- `src/pus/pus_services.c:184` matches ✓
- **Error**: The source report cites `src/pus/pus.h:66`; the actual line is **69** (see F-0376-01).

### 3.3 Spec count reconciliation (independent re-extraction)

| Spec | Rows | §3 Gaps list | §3.x Coverage | Recompute | Verdict |
| --- | --- | --- | --- | --- | --- |
| TS-1 | 103 | §3: 26 IDs | §3.1: Total 103, Nominal 48, OOB 50, Mixed 5, covered 77, gap 26, SIL/PIL/HIL 100/1/2 | table gap=26, covered=77 (66 exact + 11 `host covered` variants) | ✓ matches (77 + 26 = 103) |
| TS-5 | 77 | §3: 21 IDs | §3.2: Total 77, covered 56, gap 21, SIL/PIL/HIL 74/1/2 | table gap=21, covered=56 | ✓ matches (56 + 21 = 77) |
| TS-6 | 58 | §3: 27 IDs + 1 exploratory | §3.2: Total 58, covered 30, gap 27, Exploratory 1, SIL/PIL/HIL 53/1/4 | table gap=27, covered=30, exploratory=1 | ✓ matches (53 + 1 + 4 = 58) |

Set-difference verification (table gap IDs vs §3 gap-list IDs): **empty for all three specs** ✓ (verified with independent Python extraction).

**Note on TS-1 §3.2 reference**: TS-1 has no §3.2 section. The coverage summary with counts 48/50/5 and SIL/PIL/HIL 100/1/2 is under **§3.1** (Coverage summary, line 274). The source report cites these as "§3.2" (see F-0376-05).

### 3.4 status.json validity and ICD-rev references

- `python3 -m json.tool docs/status/status.json` → **valid JSON** ✓
- `"updated": "2026-10-17"` ✓
- **Error**: The report claims a `"revision"` block reads 2.90. **status.json has no `revision` key** in either `b8c3aee` or the working tree. Meta block keys are: `updated`, `source`, `note`, `ci_gitea_verification`, `semaphore_semantics`. The string "2.90" appears only inside the `note` text value ("verification-results 2.90"), referring to `docs/vv/verification-results.md` revision (confirmed at line 7 of that file). See F-0376-02.
- Current-state ICD-rev references:
  - `docs/status/status.json` note → mentions "rev 1.4" in the context of R3 0375 ✓ (but not as a standalone "FRICK-ICD-001 rev 1.4" field)
  - `docs/status/handover.md:435` → `FRICK-ICD-001` rev 1.4 ✓ (exact text: "`FRICK-ICD-001` rev 1.4 now records the implemented S3 subservice codes and S6 wire profile as **provisional**")
  - `docs/status/loop-audit.md:458` → mentions "ICD rev 1.3" for R3 0371, but does **not** contain the text "(R3 0375 subsequently corrected the ICD to rev 1.4)" claimed by the report. See F-0376-03.
  - `docs/vv/verification-results.md:63` → `FRICK-ICD-001` rev 1.4 ✓
  - `docs/vv/verification-results.md:1544` → "R3 0371 published rev 1.3; R3 0375 corrected the ICD to rev 1.4" ✓ (this is where the parenthetical text actually appears)
- `docs/vv/verification-results.md` revision = **2.90** (line 7) ✓
- `docs/status/loop-audit.md` revision = **10.7** (working tree header) ✓
- `docs/status/handover.md` revision = **3.05** ✓

### 3.5 Evidence-log hashes (recomputed with sha256sum)

| File | Report claims | Recomputed | Match |
| --- | --- | --- | --- |
| `2026-10-17-s8-reconciliation-ci-local.log` | `4cecaa75…` | `4cecaa75…` | ✓ |
| `2026-10-17-s8-reconciliation-format-check.log` | `c43cc487…` | `c43cc487…` | ✓ |
| `2026-10-17-s8-reconciliation-docs.log` | `f89ee26e…` | `f89ee26e…` | ✓ |
| `2026-10-17-s8-reconciliation-gsw-selfcheck.log` | `c9fbce7e…` | `c9fbce7e…` | ✓ |
| `2026-10-17-s8-reconciliation-smoke.log` | `64b1a189…` | `64b1a189…` | ✓ |

### 3.6 Cycle-result claim vs retained log

- ci-local log (line 370): `100% tests passed, 0 tests failed out of 43` ✓
- ci-local log (line 382): `Build completed successfully` ✓
- ci-local log (line 389): `lean theorems found: 62` ✓
- ci-local log (line 424): `traceability: PASS (117 requirements, 73 srs, 30 sss, 14 gaps, 62 lean theorems checked)` ✓

### 3.7 Sprint-plan rev identity and references

- Header: `Revision | 1.70 | Date | 2026-10-17` ✓ (line 7–8)
- History rows 1.69 and 1.70 present, non-duplicated ✓ (lines 505–506)
- NC-EXT-92 row (line 228) references `TS-4 §3.1` ✓
- NC-EXT-93 row (line 229) references reviews `0372/0373/0374/0375` ✓
- S8-VV-FU row (line 98): `43/43 host ctest locally` ✓; also states "TS5 §3.2 counts 77 total/56 covered/21 gap" and "TS6 §3.2 counts 58 total/30 covered/27 gap plus 1 exploratory row" ✓
- Pointer line (line 37): `the highest existing record (currently NC-EXT-93)` ✓

### 3.8 Review log (`docs/reviews/README.md`)

- Index rows for 0370–0375 present at table top ✓ (lines 5–10)
- Each referenced report file exists ✓
- 0376 and 0377 are NOT indexed ✓ (not found anywhere in README)
- The S8-FU NC-disposition rows (IDs 0338/0339) are present in the table ✓ (lines 54–55)
- No remaining merged `||` table rows ✓ (grep for `||` in working tree returns 0)
- "Ledger notes" section present ✓ (line 470)

### 3.9 Remaining status artifacts

- `loop-audit.md` rev 10.7 (header + history) ✓
- `handover.md` rev 3.05 ✓; history row 3.05 present ✓
- `CHANGELOG.md` reconciles 0370–0376 entries ✓
- Superseded `76 total / 55 covered` statement at CHANGELOG line 223 is annotated ✓: "so TS-5 is now **77 total / 56 covered / 21 gap**"

### 3.10 Loop-audit N11 row (independent check)

- **b8c3aee** (line 62): "Current worktree retains reviews **0316..0329**…"
- **Working tree** (line 62): "Current worktree retains reviews **through 0375**…"
- The text "through 0364" appears **nowhere** in the N11 row in either version.
  - "0364" appears elsewhere in loop-audit.md (lines 60, 73, 450) in the context of PA review 0364, not as "through 0364" in the N11 row.
- See F-0376-04 regarding the false basis of the source report's F-0376-01 finding.

## 4. Findings

| ID | Severity | Location (source report) | Finding | Recommendation |
| --- | --- | --- | --- | --- |
| F-0376-01 | **Major** | §2 table (line 46) and §3.2 (line 89): `src/pus/pus.h:66` | The report cites `src/pus/pus.h:66` for `FF_PusRegisterMemoryRegion`, but the function is at **line 69**. `src/pus/pus_services.c:184` is correctly cited. The signature text and comparison are otherwise correct; only the pus.h line number is wrong. | Correct `:66` → `:69` in §2 (method column) and §3.2 (claim text). |
| F-0376-02 | **Major** | §3.4 (line 108): `"revision"` block reads 2.90; §2 table (line 56): revision "1.7" | `docs/status/status.json` has **no `revision` key** in its `meta` block — in either `b8c3aee` or the working tree. Meta keys are: `updated`, `source`, `note`, `ci_gitea_verification`, `semaphore_semantics`. The value "2.90" is the revision of `verification-results.md` (line 7), referenced only as text inside status.json's `note` string. The §2 table's claimed revision "1.7" does not exist anywhere in the file. | Remove the claim that status.json has a `"revision"` block reading 2.90. Clarify that "2.90" is verification-results.md's revision, referenced in status.json's note text. Remove or correct the "1.7" attribution in the §2 table. |
| F-0376-03 | **Major** | §3.4 (line 113–114): `loop-audit.md:458` "attributes rev 1.3 to R3 0371 with `(R3 0375 subsequently corrected the ICD to rev 1.4)`" | Line 458 (history row 10.7) does attribute "ICD rev 1.3" to R3 0371, **but it does not contain the parenthetical "(R3 0375 subsequently corrected the ICD to rev 1.4)"**. That text appears only in `verification-results.md:1544`, not in `loop-audit.md`. The line says: "R3 0371 (requirement grounding) ICD rev 1.3 for the provisional S3 subservice codes and S6 profile". | Correct the attribution: loop-audit.md:458 mentions only rev 1.3 for R3 0371. The "subsequently corrected to rev 1.4" text is in verification-results.md:1544. Either remove the false attribution or correct it to reference the actual source. |
| F-0376-04 | **Major** | §4 (line 173): Finding F-0376-01 — "The N11 row still states 'reviews through 0364'" | This finding is premised on the N11 row of `loop-audit.md` containing the text "through 0364". **The text "through 0364" does not appear in the N11 row in either b8c3aee ("reviews 0316..0329") or the working tree ("reviews through 0375").** The only mention of "0364" in loop-audit.md is in the context of PA review 0364 (lines 60, 73, 450), not as a retention-set boundary in the N11 row. The N11 row's retention set has already been updated to "through 0375", and the source report's claimed state ("through 0364") never existed. | Retract finding F-0376-01. The N11 row now correctly states "through 0375" (not "through 0364"). No documentation change is required for this item; the finding was based on a phantom text string. |
| F-0376-05 | **Comment** | §3.3 (line 96): TS-1 "§3.2 counts 48/50/5 Mixed, SIL/PIL/HIL 100/1/2" | TS-1 has **no §3.2 section**. The coverage summary containing these counts is under **§3.1** (Coverage summary, line 274). TS-5 and TS-6 do have §3.2. The counts themselves (48/50/5, 100/1/2) are correct and verified; only the section reference for TS-1 is wrong. | Change "§3.2" → "§3.1" for the TS-1 row in §3.3. |
| F-0376-06 | **Comment** | §6 (Recheck trigger, line 208) and CHANGELOG line 40 | The CHANGELOG entry for 0376 (working tree, line 40) states "0376 R4 (this re-review, APPROVED WITH COMMENTS)" — this entry is **uncommitted** (absent from `b8c3aee`) and pre-states the approval conclusion before the independent R4 re-review (this document) and PA 0377 have completed. The re-review report's §1 Independence limitation note flags the self-authorship, but the CHANGELOG entry does not carry this qualification. | Annotate the CHANGELOG 0376 entry with the same self-assessment/N9/independence limitation caveat, or defer recording "APPROVED WITH COMMENTS" until independent confirmation. |

**Summary of the three prior findings referenced as corrected**: The source report states all three findings from FRICK-REVIEW-0372 that required action have been corrected in the tree. Independent verification confirms:
- Sprint-plan header 1.69→1.70 ✓
- ICD rev references corrected to 1.4 ✓
- README `||` merged row split ✓
- NC-EXT-92 section reference corrected to §3.1 ✓

## 5. What this review does NOT establish

- **No N9 independence for the original 0376 re-assessment.** The original report was self-authored by the change author (its own §1 notes this). This independent re-verification is also produced in a self-contextual session; while it reads every artifact independently, it does not constitute the independent-context review required by norm N9. PA 0377 re-verification remains required.
- **No Phase / Release / N17 / N20 approval.** The working tree is an uncommitted candidate against pushed baseline `b8c3aee`.
- **No PUS-C or mission-profile compliance.** The ICD is an internal reference profile pending authority adoption.
- **No target or physical-hardware evidence.** The ci-local 43/43 is a host-only native run.
- **No immutable/reproducible build attestation.** The candidate manifest is a working-tree file list.

## 6. Recheck trigger

If after this delta settles: the TS-1/TS-5/TS-6 §3 gap lists or §3.1/§3.2 summaries diverge; the sprint-plan header is re-bumped; the ICD revises; the candidate manifest is regenerated (hashes change); or an independent R4/PA review finds this re-assessment insufficient for N9 — re-run an independent R4 re-review before any commit/push (N7).

This review report is a working-tree artifact (norm N11). It modifies no other file, commits nothing, and pushes nothing.

---

**Evidence digest (independently recomputed):**
- ICD.md git diff hunks: **4** (`grep -c '^@@'`)
- `src/pus/pus.h` line for `FF_PusRegisterMemoryRegion`: **69** (not 66)
- `src/pus/pus_services.c` line: **184**
- status.json meta keys: `updated`, `source`, `note`, `ci_gitea_verification`, `semaphore_semantics` (no `revision`)
- status.json `updated`: `2026-10-17`; valid JSON: yes
- loop-audit N11 b8c3aee: "reviews 0316..0329" | working tree: "reviews through 0375"
- loop-audit "through 0364": **not present in N11 row in either version**
- loop-audit:458 text: "R3 0371 (requirement grounding) ICD rev 1.3 for the provisional S3 subservice codes and S6 profile" (no "subsequently corrected" parenthetical)
- verification-results.md revision: **2.90** (line 7)
- verification-results.md:63 → `FRICK-ICD-001` rev 1.4 ✓
- verification-results.md:1544 → contains "(R3 0375 corrected the ICD to rev 1.4)" ✓
- handover.md revision: **3.05**; handover.md:435 → `FRICK-ICD-001` rev 1.4 ✓
- loop-audit.md revision: **10.7**
- sprint-plan revision: **1.70**, date 2026-10-17 ✓
- sprint-plan NC-EXT-92 → TS-4 §3.1 ✓
- sprint-plan NC-EXT-93 → references 0372/0373/0374/0375 ✓
- sprint-plan S8-VV-FU → 43/43 ✓
- sprint-plan pointer → "currently NC-EXT-93" ✓
- Reviews README: 0370–0375 indexed ✓; 0376/0377 not indexed ✓; 0 `||` rows ✓; Ledger notes present ✓
- TS-1: total 103, gap 26, covered 77, §3 gap list = 26 (set diff empty) ✓
- TS-5: total 77, gap 21, covered 56, §3 gap list = 21 (set diff empty) ✓
- TS-6: total 58, gap 27, covered 30, exploratory 1, §3 gap list = 27 (set diff empty) ✓
- CI-local log: 43/43 tests, 62 lean theorems, trace PASS (117/73/30/14/62) ✓
- Evidence log hashes: all 5 ✓ (recomputed with sha256sum)
- CHANGELOG: 0370–0376 entries present ✓; superseded "76 total / 55 covered" annotated ✓
- CHANGELOG 0376 entry: uncommitted (absent from b8c3aee)
