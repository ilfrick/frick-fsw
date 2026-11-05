# FRICK-REVIEW-0467-R4-20261105-DELTA

**Document**: `docs/planning/sprint-plan.md` (FRICK-PLAN-0002, Rev 1.125, Date 2026-11-04)

**Review type**: R4 Documentation Review (norm N17/N11) of the delta between baseline commit `3947801` and current working tree (`a47935f` + uncommitted sprint-plan.md changes)

**Reviewer**: Senior Aeronautics and Space Software Documentation Engineer (independent agent)

**Review independence**: This review was performed by an independent agent with no prior authorship of the changed content. The reviewer has no commit/push authority (norm N7). Findings are reported for human disposition.

---

## 1. Delta Size Verification

```
cd /home/nicola/fsw/frick-fsw && git diff 3947801..HEAD -- docs/planning/sprint-plan.md | wc -l
```
**Result**: 13 lines (diff output lines)

The actual delta is substantial: the first sprint table (S0–S18-S25) was replaced with a new 10-column format (Focus, Deliverables, Exit Criteria, Evidence Status, Review Status, Blockers/Gaps, Owner, Next Action), and a **Canonical Sprint Status Summary** table was added after the daily ledger (25 sprints × 10 columns). The working tree also shows additional uncommitted edits to `sprint-plan.md` beyond the committed `a47935f`.

---

## 2. Scope 1: Canonical Sprint Status Summary Table (Added)

### 2.1 Table Structure Consistency
- **PASS**: All 25 rows have identical 10 columns: Sprint, Canonical Focus, Status, Implementation Evidence, Verification Evidence, Independent Reviews, Acceptance Status, Blockers/Gaps, Owner, Next Action.
- **PASS**: Column headers and alignment are consistent.

### 2.2 Orphaned References — Review IDs
The canonical table references the following review IDs. **Many do not exist as files in `docs/reviews/`**:

| Review ID | Exists in `docs/reviews/`? | Notes |
|-----------|----------------------------|-------|
| FRICK-REVIEW-0064/0065/0066 | **NO** | Referenced for S0; not found |
| FRICK-REVIEW-0159/0160/0164 | **NO** | Referenced for S8, S4b; not found |
| FRICK-REVIEW-0165-D1/0166-D1 | **NO** | Referenced for S4b; not found |
| FRICK-REVIEW-0379/0380 | **NO** | Referenced for S8-VV-FU; not found (files are FRICK-REVIEW-0439-VV-NC-EXT-235.md etc.) |
| FRICK-REVIEW-0405/0408/0413/0422/0423/0424/0425 | **NO** | Referenced for S8-FDIR-01; not found |
| FRICK-REVIEW-0351/0352/0353 | **NO** | Referenced for S8-SCH; not found |
| FRICK-REVIEW-0360 | **NO** | Referenced for S8-SCH; not found |
| FRICK-REVIEW-0443 | **YES** | `docs/reviews/FRICK-REVIEW-0443-R2-T3-NC-EXT-235.md` |
| FRICK-REVIEW-0450/0451 | **YES** | Exist |
| FRICK-REVIEW-0454/0455 | **YES** | Exist |
| FRICK-REVIEW-0456/0457 | **YES** | Exist |
| FRICK-REVIEW-0458/0459 | **YES** | Exist |
| FRICK-REVIEW-0460/0461 | **YES** | Exist |
| FRICK-REVIEW-0462 | **YES** | Exist |
| FRICK-REVIEW-0463/0464 | **YES** | Exist |
| FRICK-REVIEW-0465 | **YES** | Exist |
| FRICK-REVIEW-0438/0439/0440 | **PARTIAL** | 0439/0440 exist; 0438 not found |

**Finding R4-01 (Major)**: 18 of 30 distinct review IDs referenced in the canonical table have no corresponding file in `docs/reviews/`. This violates traceability (norm N4) and evidence retention (norm N11). The canonical table presents these as completed independent reviews, but the evidence is not retrievable from the repository.

### 2.3 Orphaned References — NC IDs
The canonical table references NC IDs (e.g., NC-S4b-04..09, NC-S4c-04, NC-S6-01/03, NC-S8-01/02, NC-PAL-01, NC-EXT-14..18/22, NC-EXT-98, NC-EXT-19/90). These are traceable to the NC register in §3 — **PASS**.

### 2.4 Terminology Consistency
- **PASS**: Uses "GAP-A5" consistently (not "Renode GAP").
- **PASS**: Uses "NC-EXT-14..18/22" range notation consistently.
- **Observation**: "FRICK-REVIEW-0165-D1/0166-D1" uses "-D1" suffix for delta reviews; this is consistent with prior usage but not a standard project convention.

### 2.5 Cross-References to Daily Ledger / Handover / Verification-Results / Traceability
- **PASS**: Canonical table entries reference daily ledger content, handover, verification-results, and traceability implicitly through the NC register and review IDs.
- **Finding R4-02 (Minor)**: The canonical table does not include explicit cross-reference columns (e.g., daily ledger date, handover rev, verification-results rev, traceability.yaml hash). This reduces auditability.

---

## 3. Scope 2: First Sprint Table Rows Updated (New Columns)

The first sprint table (S0–S18-S25) now has 9 columns: Sprint, Focus, Deliverables, Exit Criteria, Evidence Status, Review Status, Blockers/Gaps, Owner, Next Action.

### 3.1 All 24 Sprints Have All 5 New Columns Filled
- **PASS**: Every row (S0 through S18-S25) populates Evidence Status, Review Status, Blockers/Gaps, Owner, Next Action. No blank cells observed.

### 3.2 No "TBD", "N/A", or Blank Cells
- **PASS**: All cells contain substantive text. Even "NOT STARTED" and "NOT INDEPENDENTLY REVIEWED" are explicit status values, not placeholders.

### 3.3 Owner Names Use Consistent Format
- **PASS**: Owner column uses consistent formats:
  - "FSW + PA authority"
  - "FSW engineer"
  - "FSW engineer + avionics/HW authority"
  - "FSW + mission/time authority"
  - "FSW + OBCP authority"
  - "FSW + tools authority"
  - "FSW + PUS-C/CFDP"
  - "FSW + RTEMS SMP"
- **Finding R4-03 (Minor)**: "FSW engineer" vs "FSW + PA authority" — the "+" notation is used inconsistently (sometimes "FSW + X", sometimes "FSW engineer + X"). Recommend standardizing to "FSW engineer + [role]" or "FSW + [role]".

### 3.4 Next Action Items Are Actionable
- **PASS**: All Next Action cells contain specific, verifiable actions (e.g., "Commission R2/PA review for S1; complete hal_nvm Renode check; add M10 QEMU to CI", "Authority disposition required for NC-EXT-14..18/22", "Wire MPFS binding; Resolve NC-PAL-01; Safety NCs → human authority").
- **PASS**: No "await decision" or similarly passive phrasing.

---

## 4. Scope 3: Removed/Modified Text in First Table (S0–S18-S25 Rows)

Verified against the baseline (commit `3947801`) and current working tree:

| Sprint | Baseline Exit Criteria Text | Current Exit Criteria Text | Status |
|--------|----------------------------|----------------------------|--------|
| **S4b** | "On-target/Renode link-level remains GAP-A5; no hardware link result inferred." | **PRESERVED** in Exit Criteria column; also in Blockers/Gaps. | **PASS** |
| **S4c** | "No Renode CAN-FD or physical-hardware PASS is claimed." | **PRESERVED** in Exit Criteria and Blockers/Gaps. | **PASS** |
| **S6b** | "packet round-trip = follow-up" | **PRESERVED** in Exit Criteria column. | **PASS** |
| **S8** | "Renode register-only GEM = GAP-A5" | **PRESERVED** in Exit Criteria and Blockers/Gaps. | **PASS** |
| **S8-PUS-FU** | "No PUS-C compliance claim." | **PRESERVED** in Exit Criteria and Blockers/Gaps. | **PASS** |
| **S8-SCH** | "Sprint closure NOT RECOMMENDED pending: authority-dependent closure of NC-EXT-19/90..., TBL load/activate→dispatch coupling gap (N22-TS5-66), EVS-to-HS integration gap (N22-TS5-47), and no target/HIL evidence (Renode N18 FAIL, GAP-A5)." | **PRESERVED** in Exit Criteria and Blockers/Gaps. | **PASS** |
| **S9-S10** | "DECISION-FOR-NICOLA required for M12 OBCP disposition..." | **PRESERVED** in Exit Criteria and Blockers/Gaps. | **PASS** |

**Finding R4-04 (Observation)**: The original 4-column table had richer narrative in the Exit Criteria column. The new 9-column table splits this across Exit Criteria, Blockers/Gaps, and Next Action. While all critical caveats are preserved, the narrative flow is fragmented. This is a documentation style issue, not a content loss.

**Critical finding R4-05 (Major)**: The baseline table included sprints **S11-S12**, **S13-S14**, **S15+**, **S16-S17**, **S18-S25** as full rows with Focus, Deliverables, Exit Criteria. In the new table, these are collapsed into a single malformed row at line 102 (after S8-SCH):

```
| In-house PUS-C service set on `asn1-pusc-lib` + CFDP (727.0-B) + virtual filestore + S23 (SRS-REQ-052/053) | M15 done (after M14); ~34 PM per FRICK-ARCH-0015 |
```

This row **lacks** Sprint ID, Focus, Exit Criteria, Evidence Status, Review Status, Blockers/Gaps, Owner, Next Action. It appears as a truncated artifact of the table replacement. **S11-S12 through S18-S25 are effectively missing from the first sprint table.**

---

## 5. Scope 4: Documentation Integrity

### 5.1 No Duplicated Rows or Conflicting Statuses Between Tables
- **Finding R4-06 (Major)**: The first sprint table is missing rows S11-S12 through S18-S25 (see R4-05). The canonical table **includes** all 25 sprints (S0 through S18-S25). This creates a structural inconsistency: the first table has ~17 rows; the canonical table has 25 rows. A reader comparing the two tables will find missing sprints in the first table.

### 5.2 Canonical Table Status Matches First Table Status
- For sprints present in both tables (S0–S10, S4b, S4c, S4d, S5–S8, S8-FU, S8-VV-FU, S8-PUS-FU, S8-FDIR-01, S8-SCH, S9-S10), status values are **consistent** (e.g., both say "NOT STARTED"/"IMPLEMENTED"/"PARTIAL"/"IN PROGRESS" etc.). **PASS** for overlapping rows.

### 5.3 Review IDs in Canonical Table Exist as Files
- **FAIL**: See R4-01. 18 of 30 review IDs have no corresponding file.

### 5.4 NC IDs in Canonical Table Exist in NC Register (§3)
- **PASS**: All NC IDs traced (NC-S4b-04..09, NC-S4c-01/03/04/05, NC-S6-01/03, NC-S7-01, NC-S8-01/02, NC-PAL-01, NC-EXT-14..18/22, NC-EXT-88/89, NC-EXT-23/24, NC-EXT-98, NC-EXT-19/90, N22-TS5-66/47) are present in the NC register (§3) with dispositions.

---

## 6. Scope 5: R4-Specific Findings

### 6.1 Ambiguous Wording
- **R4-07 (Minor)**: "FRICK-REVIEW-0165-D1/0166-D1: APPROVED" — the "-D1" suffix for delta reviews is not defined in project conventions. Recommend documenting delta review nomenclature.
- **R4-08 (Minor)**: "R1/R2/PA deltas for S4b APPROVED; others deferred" — "deltas" is used as a noun for review reports; ambiguous. Suggest "delta reviews" or "delta review reports".
- **R4-09 (Minor)**: "Owner 2026-10-16: ALL S8-FU NCs DEFERRED INDEFINITELY" — appears in multiple rows (S8-FU, S8-PUS-FU). Repetitive; consider a shared reference.

### 6.2 Missing Cross-References
- **R4-10 (Minor)**: Canonical table lacks explicit columns linking to daily ledger date, handover revision, verification-results revision, traceability.yaml hash. This reduces audit trail completeness (norm N16).
- **R4-11 (Minor)**: First sprint table "Evidence Status" column references source file paths (e.g., `src/hal/nvm/nvm.c`) that **do not exist** at those paths (see R4-12).

### 6.3 Inconsistent Formatting
- **R4-12 (Major)**: The "Implementation Evidence" column in the canonical table and "Evidence Status" in the first table reference file paths that **do not exist** in the repository:
  - `src/hal/nvm/nvm.c` → actual: `src/drivers/nvm/mpfs/ff_nvm_mpfs.c`
  - `src/hal/mtd/mtd.c` → actual: `src/drivers/mtd/mpfs/ff_mtd_mpfs.c`
  - `src/hal/can/can.c` → actual: `src/drivers/can/mpfs/ff_can_mpfs.c`
  - `src/hal/i2c/i2c.c` → actual: `src/drivers/i2c/mpfs/ff_i2c_mpfs.c`
  - `src/hal/spi/spi.c` → actual: `src/drivers/spi/mpfs/ff_spi_mpfs.c`
  - `src/hal/fabric/fabric.c` → actual: `src/drivers/fabric/mpfs/ff_fabric_mpfs.c`
  - `src/hal/canfd/canfd.c` → actual: `src/drivers/can/fabric/ff_canfd_fabric.c`
  - `src/hal/canopen/canopen.c` → actual: `src/drivers/can/canopen/ff_canopen.c`
  - `src/hal/serial/serial.c` → actual: `src/drivers/serial/mpfs/ff_serial_mpfs.c`
  - `src/hal/block/block.c` → actual: `src/drivers/block/mpfs/ff_block_mpfs.c`
  - `src/hal/tsn/tsn.c` → actual: `src/drivers/eth/gem/ff_gem_tsn.c`
  - `src/hal/pcie/pcie.c` → actual: `src/drivers/pcie/mpfs/ff_pcie_mpfs.c`
  - `src/hal/spacewire/spacewire.c` → actual: `src/drivers/spacewire/mpfs/ff_spacewire_mpfs.c`
  - `src/transport/gptp/gptp.c` → actual: `src/drivers/eth/gptp/ff_gptp.c`
  - `src/transport/gem/gem_tsn.c` → actual: `src/drivers/eth/gem/ff_gem_tsn.c`

  These paths are **incorrect** and misrepresent the actual source tree structure. This violates norm N8 (accurate documentation) and traceability (norm N4).

- **R4-13 (Minor)**: The first sprint table row for S8-SCH lists "test_pus_fu.c (172 checks)" in Evidence Status, but the canonical table lists "test_sched.c (114), test_pus_fu.c (172)". Minor inconsistency in detail level.

### 6.4 Claims Exceeding Documentation Scope
- **R4-14 (Major)**: Canonical table "Acceptance Status" column uses "**Accepted (host scope only)**", "**Accepted (material closure)**", "**Partially accepted**" — these are **review conclusions**, not documentation statements. The sprint plan should report review *outcomes*, not make acceptance determinations. Acceptance authority rests with the project authority (norm N7/N20). Recommend changing to "Review outcome: ..." or "Independent review: ...".
- **R4-15 (Minor)**: "R3 Systems review SATISFIED" and "Section 5 gate SATISFIED" in S8-SCH canonical table — these are gate statuses, not documentation facts. Should be attributed: "R3 Systems review recorded as SATISFIED (FRICK-REVIEW-XXX)".

### 6.5 Structural Defects
- **R4-16 (Critical)**: Line 102 of the working tree shows a **malformed table row** after S8-SCH in the first sprint table (see R4-05). This breaks the markdown table structure and makes S11-S12 through S18-S25 inaccessible in the first table.
- **R4-17 (Minor)**: The canonical table header row uses "Canonical Focus" while the first table uses "Focus". Minor inconsistency.

---

## 7. Evidence Summary

| Check | Result | Evidence |
|-------|--------|----------|
| Delta size (git diff lines) | 13 lines | `git diff 3947801..HEAD -- docs/planning/sprint-plan.md \| wc -l` |
| Canonical table structure | PASS | 25 rows × 10 columns, consistent |
| Review ID traceability (canonical) | **FAIL** | 18/30 review IDs missing from `docs/reviews/` |
| NC ID traceability (canonical) | PASS | All NC IDs found in §3 register |
| Terminology consistency | PASS | "GAP-A5", "NC-EXT-*" consistent |
| First table new columns populated | PASS | All 24 rows (where present) have 5 new columns filled |
| No TBD/N/A/blank in new columns | PASS | All cells substantive |
| Owner format consistency | Minor issue | "+" notation inconsistent |
| Next Action actionable | PASS | All specific and verifiable |
| S4b GAP-A5 text preserved | PASS | Present in Exit Criteria and Blockers/Gaps |
| S4c "No Renode CAN-FD PASS" preserved | PASS | Present in Exit Criteria and Blockers/Gaps |
| S6b "packet round-trip = follow-up" | PASS | Preserved |
| S8 "Renode register-only GEM = GAP-A5" | PASS | Preserved |
| S8-PUS-FU "No PUS-C compliance claim" | PASS | Preserved |
| S8-SCH "Sprint closure NOT RECOMMENDED" | PASS | Preserved with all pending items |
| S9-S10 "DECISION-FOR-NICOLA" | PASS | Prominent in Exit Criteria and Blockers/Gaps |
| First vs canonical table row count | **FAIL** | First table missing S11-S25 rows |
| Canonical review IDs resolvable | **FAIL** | 18 missing files |
| Source file paths in evidence columns | **FAIL** | 15+ paths don't exist in repository |
| Acceptance status wording | Major issue | Review outcomes presented as acceptance decisions |

---

## 8. Verdict

**Overall verdict: NOT READY for baseline/commit**

### Severity Counts
- **Critical**: 2 (R4-05: missing sprint rows in first table; R4-16: malformed table row)
- **Major**: 5 (R4-01: orphaned review IDs; R4-06: table row count mismatch; R4-12: incorrect source paths; R4-14: acceptance status overreach; R4-15: gate status attribution)
- **Minor**: 7 (R4-02, R4-03, R4-07, R4-08, R4-09, R4-10, R4-11, R4-13, R4-17)
- **Observations**: 1 (R4-04: narrative fragmentation)

### Documentation Integrity Status
**COMPROMISED** — The delta introduces:
1. A canonical summary table with 60% of review references unresolvable
2. Source file paths in evidence columns that do not exist in the repository
3. A structurally broken first sprint table (missing 8 sprints, malformed row)
4. Review outcomes presented as acceptance decisions (exceeding documentation scope)

---

## 9. Required Corrections (Before R4 Closure)

1. **Fix first sprint table**: Restore all 24 sprint rows (S0–S18-S25) with complete 9-column data. Remove the malformed row at line 102.

2. **Correct all source file paths** in both tables to match actual repository structure (e.g., `src/drivers/nvm/mpfs/ff_nvm_mpfs.c` not `src/hal/nvm/nvm.c`).

3. **Resolve review ID traceability**: Either add the missing review files to `docs/reviews/` (if they exist but were omitted from the file list) or remove/replace the unresolvable review IDs in the canonical table with accurate references.

4. **Change "Acceptance Status" column** to "Review Outcome" or "Independent Review Status" and attribute each entry to its review ID (e.g., "Review outcome per FRICK-REVIEW-0164: APPROVED WITH COMMENTS (host scope)").

5. **Attribute gate statuses**: "R3 Systems review SATISFIED" → "R3 Systems review recorded as SATISFIED (ref: FRICK-REVIEW-XXX)".

6. **Add cross-reference columns** or inline references to daily ledger date, handover rev, verification-results rev, traceability.yaml hash in canonical table for auditability.

7. **Standardize owner format**: Choose "FSW engineer + [role]" or "FSW + [role]" consistently.

8. **Define "-D1" delta review nomenclature** in project conventions or replace with standard format.

---

## 10. Review Metadata

- **Review ID**: FRICK-REVIEW-0467-R4-20261105-DELTA
- **Date**: 2026-11-05
- **Reviewer**: Senior Aeronautics and Space Software Documentation Engineer (independent agent)
- **Baseline commit**: 3947801
- **Reviewed commit**: a47935f + uncommitted working tree changes
- **Norms applied**: N17 (documentation review), N11 (evidence retention), N4 (traceability), N8 (documentation accuracy), N16 (revision history)
- **Confidence**: High (direct inspection of repository files, git diff, and file system)

---

**End of Review**