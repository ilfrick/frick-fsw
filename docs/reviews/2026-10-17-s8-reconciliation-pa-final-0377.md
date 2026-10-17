# Software Product Assurance final re-verification — S8 reconciliation candidate (FRICK-REVIEW-0377)

**Review ID:** FRICK-REVIEW-0377  
**Profile:** PA (independent, norm N20)  
**Date:** 2026-10-17  
**Phase boundary:** N20 product assurance only (not approval, not certification, not release, not flightworthiness)  
**Assessor:** aeronautics-space-sw-pa-engineer (independent)  
**Reference revision:** git commit `b8c3aeec5484476407367024f3ce25c5af2bd30e` (= b8c3aee, HEAD) + uncommitted working-tree candidate (211 differing files)

> **Authority statement.** This persona is an AI assurance assistant, not a
> licensed engineer, designated certification representative, auditor of record,
> approval authority, or holder of proprietary standards. It cannot certify
> compliance, approve a safety case, accept residual risk, close a
> nonconformance, qualify a tool, authorize release, or declare flightworthiness.
> Independent qualified humans retain those decisions.

## 1. Scope and artifacts inspected

This independent reassessment re-verifies the S8 reconciliation candidate tree
against the seven mandatory checklist items recorded for this pass: (1) manifest
identity, (2) code delta bounds, (3) evidence gates, (4) review chain &
independence, (5) NC & authority status, (6) overstating-claims audit, and
(7) provisional vs approved ICD status.

**Artifacts consulted (full paths relative to repo, all read independently):**

- `docs/vv/evidence/2026-10-17-s8-reconciliation-candidate-manifest.md` — the 211-file manifest
- `docs/vv/evidence/2026-10-17-s8-reconciliation-{ci-local,format-check,docs,gsw-selfcheck,smoke,rtems-cross-build,renode,qemu}.log` — all 8 evidence gate logs
- `build/ci/CMakeCache.txt`, `build/ci/frick_fsw` — cycle build tree artifacts
- `CMakeLists.txt` — lines 24–27 (option defaults), line 151 (N7_OBCP guard)
- `src/pus/pus.h` — lines 65–72 (signature at line 69)
- `src/core/sched/ff_sched.{c,h}` — deadline-overflow fix (the only sched behavioral change)
- `src/main/fsw_main.c` — comment-only change
- `src/pus/pus.c`, `src/pus/pus_services.c`, `src/pus/adapters/pus_adapter_n7.c` — PUS behavioral changes
- `docs/reviews/2026-10-17-s8-reconciliation-pa-0373.md` — prior PA bounded review (F-0373-01..05)
- `docs/reviews/2026-10-17-s8-reconciliation-docs-r4-rereview-0376.md` — R4 independent re-review (F-0376-01..06)
- `docs/reviews/2026-10-17-s8-vv-pus-delta-0370.md` — R2 V&V delta review
- `docs/reviews/2026-10-17-s8-requirement-grounding-r3-0371.md` — R3 requirement grounding
- `docs/reviews/2026-10-17-s8-ts5-77-oracle-r2-0374.md` — R2 TS-5-77 oracle review
- `docs/reviews/2026-10-17-s8-icd-1-3-delta-r3-0375.md` — R3 ICD delta review
- `docs/planning/sprint-plan.md` — rev 1.70 (NC register and revision history)
- `docs/status/loop-audit.md` — rev 10.7 (N11 row and NC status)
- `docs/status/status.json` — meta block, gates, semaphores
- `docs/interfaces/ICD.md` — rev 1.4 provisional profile
- `docs/adr/ADR-0020-provisional-pus-and-scheduler-policies.md` — rev 0.1
- `CHANGELOG.md` — rev (Unreleased section)

All files were read from the working tree and/or `git show b8c3aee:<file>` for
baseline comparison. No files were modified except this report and the README
disposition row.

## 2. Assessment against required questions

### (1) Manifest identity — 211 files, combined digest `c50798b5…`

**Fact (high confidence).** The candidate manifest at
`docs/vv/evidence/2026-10-17-s8-reconciliation-candidate-manifest.md` contains
exactly **211 table rows**. The git accounting is consistent:

| Source | Count |
|---|---|
| `git diff --name-only b8c3aee` (tracked, modified vs baseline) | 59 files |
| `git ls-files --others --exclude-standard` (untracked) | 150 files |
| Subtotal (source/doc files differing from b8c3aee) | 209 |
| gitignored build artifacts explicitly referenced (`build/ci/CMakeCache.txt`, `build/ci/frick_fsw`) | 2 |
| **Total** | **211** |

The combined digest was independently recomputed by parsing all 211
`<sha256>  <path>` lines (sorted by path), joining with `\n`, and hashing with
Python `hashlib`:

**Independently computed:** `c50798b5abcdaa345e62c50bc95f93aa25fbc15b426f5b6b2d93364ef8a8cbe2`  
**Manifest claims:** `c50798b5abcdaa345e62c50bc95f93aa25fbc15b426f5b6b2d93364ef8a8cbe2`  
**Match: Yes.**

All 10 evidence-log hashes (8 gate logs + CMakeCache.txt + frick_fsw) were
independently recomputed with `sha256sum` and all match the manifest's linkage
hashes. The manifest's self-referential entry
(`docs/vv/evidence/2026-10-17-s8-reconciliation-candidate-manifest.md`,
hash `e3b0c442…`, size 0) is the SHA-256 of an empty string — the standard
self-reference convention for a manifest that includes itself before it is
populated; the manifest's footer note (line 246) correctly documents this as a
working-tree artifact and disclaims signed attestation/provenance.

The manifest includes `src/obcp/ff_obcp.c` and `src/obcp/ff_obcp.h` (rows 221–222
in the manifest). On disk, `src/obcp/` contains exactly `ff_obcp.c` and
`ff_obcp.h`. In `b8c3aee`, `git ls-tree -r b8c3aee --name-only | grep obcp`
returns zero entries — the entire `src/obcp/` directory is new in the working
tree (untracked, not present at baseline).

**Conclusion (1):** Manifest identity is fully verified. This resolves PA 0373
F-0373-04 (stale 49-file/19-file manifests superseded). The manifest is a
working-tree artifact, not a signed run attestation, and generated-tool
provenance (NC-EXT-84) remains not established.

### (2) Code delta bounds

**Fact (high confidence).** `git diff --name-only b8c3aee` returns 59 tracked
modified files. Of the 14 modified `.c`/`.h` source files:

| Category | Files |
|---|---|
| Scheduler deadline-overflow fix (only sched behavioral change) | `src/core/sched/ff_sched.c`, `src/core/sched/ff_sched.h` |
| PUS service behavioral changes (NOT sched-only) | `src/pus/pus.c`, `src/pus/pus.h`, `src/pus/pus_services.c`, `src/pus/adapters/pus_adapter_n7.c` |
| Comment-only change | `src/main/fsw_main.c` |
| Test changes | `tests/unit/test_decoder_corpus.c`, `tests/unit/test_es_hs_lifecycle.c`, `tests/unit/test_pus.c`, `tests/unit/test_pusopen.c`, `tests/unit/test_r2_gap.c`, `tests/unit/test_r3_gap.c`, `tests/unit/test_sched.c` |
| Build system | `tests/CMakeLists.txt` |

**Discrepancy with checklist item 2 (verified).** A prior checklist claimed the
behavioral source delta was "just sched fix + comment-only `fsw_main.c".
This is **factually incorrect**: the delta includes four PUS behavioral source
files (`pus.c`, `pus.h`, `pus_services.c`, `pus_adapter_n7.c`) with substantive
changes (duplicate-registration rejection, `FF_PusRegisterMemoryRegion`
signature change to add `region_id`, `svc3_report` first-failure retention,
`svc6_dump` offset-based memory access, S11 stub visibility event, `svc20_report`
length validation, service-table subservice correction). Comment-only change
is confirmed only for `fsw_main.c`.

**Discrepancy with checklist item 2 (verified).** A prior checklist claimed
`FF_USE_N7_OBCP=OFF` and that `ff_obcp.c` is "NOT compiled." Both are
**factually incorrect**:

- `CMakeLists.txt` line 26: `option(FF_USE_N7_OBCP "Build and link the pinned
  N7 Space MicroPython OBCP backend" ON)` — defaults to **ON**, not OFF.
- `CMakeLists.txt` line 25: `option(FF_USE_PUS_ASN1 "Use the N7/ESA
  asn1-pusc-lib PUS-C backend by default …" ON)` — also changed from
  `OFF` at `b8c3aee` to `ON` in the working tree.
- CI-local log (line 92): `[5/193] Building C object
  CMakeFiles/frick_fsw_core.dir/src/obcp/ff_obcp.c.o` — **`ff_obcp.c` IS
  compiled** by the cycle build.
- `src/obcp/` directory is **not present at `b8c3aee`** — it is new/untracked.

`CMakeLists.txt` line 151 contains `if(FF_USE_N7_OBCP)` confirming the option
is actively used in the build guard.

**Conclusion (2):** The code delta includes PUS behavioral changes well beyond
the sched fix, and `FF_USE_N7_OBCP` defaults ON with `ff_obcp.c` compiled. The
prior checklist's item 2 claims on code-delta bounds and OBCP compilation
status are incorrect. No behavioral source change affects `fsw_main.c` beyond
a comment.

### (3) Evidence gates (8)

**Fact (high confidence).** All 8 evidence gate logs were independently read
and their SHA-256 hashes recomputed with `sha256sum` — all match the manifest
linkage hashes:

| Gate | Log hash | Verified |
|---|---|---|
| ci-local | `4cecaa75…` | ✓ match |
| format-check | `c43cc487…` | ✓ match |
| docs | `f89ee26e…` | ✓ match |
| gsw-selfcheck | `c9fbce7e…` | ✓ match |
| smoke | `64b1a189…` | ✓ match |
| rtems-cross-build | `1e1b232b…` | ✓ match |
| renode | `0a806a8d…` | ✓ match |
| qemu | `da25f66c…` | ✓ match |

Gate results:

- **ci-local:** `100% tests passed, 0 tests failed out of 43`; Lean
  `Build completed successfully` with 62 theorems; trace `PASS (117 requirements,
  73 srs, 30 sss, 14 gaps, 62 lean theorems checked)`. ✓
- **format-check:** `find src config tests -name '*.[ch]' -print0 | xargs -0
  -n1 clang-format --dry-run --Werror` — no output, exit 0 = no formatting
  violations. ✓
- **docs:** Doxygen 1.9.8 build completes; `test -s
  build/doxygen/html/index.html` guard passes; no `error:`/`FAILED`/`fatal:`
  lines in 2919-line log. ✓
- **gsw-selfcheck:** 12+9+14 Python test OK; MPL-2.0 license check OK. ✓
- **smoke:** `PASS (banner + 6 applications registered)`. ✓
- **rtems-cross-build:** `[100%] Built target frick_fsw_core`; `[100%] Built
  target frick_fsw`. ✓
- **renode:** `PASS (RENODE_TEST: PASS)`. ✓
- **qemu:** `[qemu] SKIP: generic-BSP ELF absent
  (/home/nicola/fsw/frick-fsw/build/rtems-generic/frick_fsw); build with
  'make qemu-image'` — **SKIP, not PASS.** This is the documented QEMU gate
  behavior when the RTEMS generic image is absent (AGENTS.md documents this as
  a SKIP). It does not constitute a functional verification run.

**Discrepancy with checklist item 3 (verified).** If a prior checklist claimed
QEMU was "green" (PASS), that is **factually incorrect** — the QEMU gate is
SKIP (no functional execution occurred; ELF absent, exit 0 per documented SKIP
behavior).

**Conclusion (3):** 7 of 8 gates are verified green (ci-local, format-check,
docs, gsw-selfcheck, smoke, rtems-cross-build, renode). QEMU is SKIP, not PASS.
The QEMU gate is correctly documented as a host-only proxy for the RTEMS
generic image path; M10 (QEMU functional check) was previously marked done in
status.json, and the SKIP here is consistent with AGENTS.md's documented
behavior for a missing image.

### (4) Review chain & independence

**Fact (high confidence).** The following reviews are present as files in
`docs/reviews/` and were read in full:

| ID | Profile | File present | Independently verified findings |
|---|---|---|---|
| 0370 | R2 | ✓ | Gap-list/table synchronization for TS-1/TS-5/TS-6 |
| 0371 | R3 | ✓ | F-001..007 (ICD S3/S6, ADR-0020 boundary, scheduler exhaustion) |
| 0372 | R4 | ✓ | Bounded doc corrections |
| 0373 | PA | ✓ | F-0373-01..05 (closure taxonomy, TS5-77 review gap, NC-EXT-92, stale identity, provenance) |
| 0374 | R2 | ✓ | TS-6 §3.2 undercount, GAP-R3-1 wording, activation→`FF_SchedInit` coverage limit |
| 0375 | R3 | ✓ | F-0375-01..04/06/08 (ICD rev 1.4 qualifications) |
| 0376 | R4 | ✓ | F-0376-01..06 (citation errors and CHANGELOG annotation) |

**R4 0376 findings independently re-verified:**

- **F-0376-01** (`src/pus/pus.h:66` should be `:69`): `src/pus/pus.h` line 69
  reads `FF_Status_t FF_PusRegisterMemoryRegion(uint8_t region_id, const void
  *base, uint32_t size);` — **VERIFIED.** The function is at line 69, not 66.
  ✓
- **F-0376-02** (`status.json` has a `"revision"` key reading 2.90):
  `python3 -m json.tool docs/status/status.json` → valid JSON; meta keys are
  `updated`, `source`, `note`, `ci_gitea_verification`, `semaphore_semantics` —
  **no `revision` key.** The string "2.90" appears only inside the `note` text
  value, referring to `docs/vv/verification-results.md` revision (line 7).
  **VERIFIED.** ✓
- **F-0376-03** (`loop-audit.md:458` contains ICD-1.4 correction text):
  `loop-audit.md` line 458 reads "R3 0371 (requirement grounding) ICD rev 1.3
  for the provisional S3 subservice codes and S6 profile" — it mentions ICD rev
  1.3 for R3 0371 but does **not** contain the parenthetical "(R3 0375
  subsequently corrected the ICD to rev 1.4)." That text appears only in
  `verification-results.md:1544` ("R3 0371 published rev 1.3; R3 0375 corrected
  the ICD to rev 1.4"). **VERIFIED.** ✓
- **F-0376-04** (N11 row states "reviews through 0364"): `loop-audit.md` line 62
  N11 row reads "Current worktree retains reviews **through 0375**" — no "through
  0364" in the N11 row in either `b8c3aee` ("reviews 0316..0329") or working
  tree. "0364" appears elsewhere in loop-audit (lines 60, 73, 450) in the
  context of PA review 0364, not as a retention boundary. **VERIFIED.** ✓
- **F-0376-05** (TS-1 references §3.2): TS-1 has no §3.2 section; the coverage
  summary with counts 48/50/5 is under §3.1 (Coverage summary, line 274 of
  `ts-1-protocol-core.md`). TS-5 and TS-6 do have §3.2. **VERIFIED.** ✓
- **F-0376-06** (CHANGELOG 0376 entry pre-states approval, lacks annotation):
  At `b8c3aee`, `git show b8c3aee:CHANGELOG.md | grep -c '0376'` returns 0 —
  the 0376 CHANGELOG entry is absent from baseline. In the working tree,
  CHANGELOG line 40 already reads "**0376 R4** — independent documentation
  re-review. **CORRECTED APPROVED WITH COMMENTS** (… the review report itself
  carries the report's own N9-independence caveat … PA **0377** pending
  independent re-verification of the corrected tree; no commit/push until it
  passes)." The current text already carries the `CORRECTED` qualifier, the N9
  independence caveat, and the explicit PA-0377 deferral. The substance of F-0376-06's
  recommendation is **already addressed** in the current working tree. However,
  the 0376 report's finding text describes the CHANGELOG as stating "0376 R4
  (this re-review, APPROVED WITH COMMENTS)" without qualification — this
  description does **not** match the current working tree CHANGELOG, which
  already includes the qualification. This is a discrepancy in the 0376 report's
  characterization of its own evidence.

**Independence (N9) caveat:** The 0376 R4 re-review report (at §1 and §5)
explicitly states that its original form was self-authored by the change author
and that this re-verification was produced from "fresh context" but is
self-contextual — it does **not** constitute the independent-context review
required by norm N9. PA 0377 (this report) is the required independent PA
re-verification. The 0376 report, PA 0373, R2 reviews 0370/0374, and R3 reviews
0371/0375 were each produced from independent review contexts and do not
self-approve.

**README index gap (verified):** `docs/reviews/README.md` lists indexed entries
for 0370–0375 (lines 5–10) but does **not** contain "0376" or "0377" anywhere
in the file (grep returns 0 matches). The 0376 report file exists and is
documented in CHANGELOG line 40, but is not indexed. The 0377 report (this
document) is also not yet indexed. This is a PA-scoped ledger gap.

**Conclusion (4):** All seven reviews (0370–0376) are present as files and were
read in full. All six R4 0376 findings are independently reproduced and
verified. The README review index is missing entries for 0376 R4 and 0377 PA.
The 0376 report's F-0376-06 characterization of the CHANGELOG does not match
the current working tree (the CHANGELOG already carries the annotation).

### (5) NC & authority status

**Fact (high confidence).** NC register status from `docs/planning/sprint-plan.md`
(rev 1.70, dated 2026-10-17), independently read and cross-checked:

| NC | Status | Verified text |
|---|---|---|
| NC-EXT-88 | CLOSED narrowly by R1 0355 | "This closes only the enumerated report-all first-failure overwrite defect — the broader S3 ID/enable/remove/provider and periodic semantics track under NC-EXT-17, and PA/R4/release gates and any PUS-C compliance claim remain separate." |
| NC-EXT-89 | CLOSED narrowly by R2 0356 | "This closes only the enumerated test-evidence/trace gaps; full-cycle/PA/R4/release gates and no PUS-C compliance claim remain separate." |
| NC-EXT-91 | CLOSED NARROWLY (19-file delta identity only) | "This does not establish complete S8/OBCP candidate identity, immutable/reproducible provenance, or source-to-binary provenance; NC-EXT-84 and other phase/release gates remain open." |
| NC-EXT-90 | OPEN | "checked non-wrapping deadline arithmetic and saturating miss diagnostics are host-tested under provisional ADR-0020 policy. Obtain Systems/FSW authority disposition and target/mission evidence before operational use" |
| NC-EXT-84 | PARTIALLY CORRECTED; OPEN | "Generator/tool version, reproducible generation and qualification implications remain unverified" |
| NC-EXT-85 | CLOSED narrowly by R4 0347 | "No general N17/phase approval." |
| NC-EXT-86 | CLOSED narrowly by R4 0344 | "No phase/release claim." |
| NC-EXT-87 | CLOSED narrowly by R4 0344 | "row identifies delivered bounded scope and separates remaining target/timing/policy evidence" |
| NC-EXT-92 | OPEN (pre-existing at b8c3aee) | "The equivalent TS-4 decision is deliberately not taken unilaterally… No target, qualification or phase claim is implied." |
| NC-EXT-93 | OPEN (bounded mitigation) | "Disambiguates rows by report filename… Disposition: rename one series in a bounded, independently reviewed cleanup." |
| NC-EXT-14..18 | OPEN | Authority-dependent (S1 Request ID, ack flags, S3/S9/S20 subservices, S6 address/region, S9 scheduling) |
| NC-EXT-19 | OPEN | "Reference application scheduling boundaries, timing policy, target runtime behavior" |
| NC-S9-01 | DEFERRED/OPEN | "No VM-enforced instruction/time quota… Production service/PUS S18 bindings and target execution evidence are not established." |
| NC-EXT-40 | OPEN | "Active T18 reproducibility claims and qualification status conflicted with mutable apt/RSB inputs… T18 row states NOT FULLY REQUALIFIED." |
| NC-EXT-47 | OPEN (postponed) | "Renode aggregate marker is not treated as coverage… requirements/integration-test mapping moves to the last sprint." |

**Finding (5a).** No NC that remains open is claimed "complete" or "approved"
without caveats. Every narrow closure (NC-EXT-88/89/91/85/86/87) explicitly
states the remaining scope and open gates. Open NCs all retain authority or
evidence requirements. NC-S9-01 is DEFERRED per owner direction with explicit
limitations. **Consistently tracked.** ✓

**Finding (5b — minor).** NC-EXT-92's row in sprint-plan cites "TS-4 §3.2" in
the NC description text but the revision history and the NC row body reference
"TS-4 §3.1 (Coverage summary)" — PA 0373 F-0373-03 identified this section-
reference typo. The current sprint-plan (rev 1.70) NC-EXT-92 row cites "TS-4 §3.1"
(line 228). This appears corrected in the working tree. ✓

**Conclusion (5):** NC & authority status is consistently tracked. No open NC
is prematurely claimed complete; narrow closures are properly bounded; all
authority/phase/release gates remain explicitly open.

### (6) Overstating-claims audit

**Fact (high confidence).** An independent grep across the three PA-relevant
artifacts for forbidden phrasings (certify, certification, flightworthy,
flightworthiness, authorize release) plus positive qualification/compliance
claims:

| Artifact | Forbidden positive phrasings found | Notes |
|---|---|---|
| `docs/reviews/2026-10-17-s8-reconciliation-pa-0373.md` | 0 | All "qualif" occurrences (8) are in negative context ("does not qualify tools", "not qualification/phase/release", "qualification implications remain unverified", "not a phase/release approval") |
| `docs/reviews/2026-10-17-s8-reconciliation-docs-r4-rereview-0376.md` | 0 | No forbidden phrasings |
| `docs/vv/evidence/2026-10-17-s8-reconciliation-candidate-manifest.md` | 0 | Only "qualif" match is the filename `docs/vv/tool-qualification.md` in the file hash table |
| `CHANGELOG.md` (working tree) | 0 | No forbidden phrasings; all entries carry "no phase/release/N20 approval" qualifiers |

The PA reports (0373, 0377) and the 0376 R4 re-review explicitly state they do
not grant phase/release approval, do not certify compliance, do not declare
flightworthiness, and do not authorize release. No overstating claims found.

**Conclusion (6):** Overstating-claims audit passes — no forbidden phrasings
in positive context. ✓

### (7) Provisional vs approved ICD status

**Fact (high confidence).** `docs/interfaces/ICD.md`:

- Header: `Revision | 1.4`, `Date | 2026-10-17`, `Profile | PUS-C (ECSS-E-ST-70-41C) over CCSDS Space Packets (CCSDS 133.0-B-2)`.
- §1 Scope (lines 11–16): "not an approved mission tailoring baseline; applicable
  profile values and acceptance rules require approval by the authorized
  mission/customer authority."
- §4 (lines 96–121): Service/subservice reference table. Line 98–99: "The list is
  a framework reference profile, **not** a selected mission PUS profile."
- §4 provisional-profile note (lines 126–134): "Every service/subservice row in
  this section is a provisional framework default recorded in
  `docs/adr/ADR-0020-provisional-pus-and-scheduler-policies.md`". Covers
  S1/S3/S5/S6/S8/S9/S11/S17/S20.
- Service 11 OBCP row (line 119): marked as stub returning `FF_NOT_IMPLEMENTED`.
- ADR-0020 attribution intact (lines 126–134; ADR-0020 §4 disclaimer, lines 60–70).

`docs/adr/ADR-0020-provisional-pus-and-scheduler-policies.md`:
- Status: "Accepted as a framework engineering reference; not a mission baseline"
- §3 Disclaimer: explicitly disclaims "system-authority approval, a mission PUS
  profile, ECSS compliance, packet interoperability, safe memory access
  authorization, scheduler schedulability, target timing, or flight
  suitability."
- §4 sources: pins ECSS-E-ST-70-41C (2016) with verified SHA-256
  `fed627dae2aa2a1e9dbfb873861002fbee40a37a3883019d6701af34852ea51d`.

**CHANGELOG verification:** At `b8c3aee`, `git show b8c3aee:CHANGELOG.md` contains
zero mentions of review IDs 0370–0377 (the entire "Unreleased" review-history
section was added in the working tree). Working-tree CHANGELOG line 216 (rev
1.3, 2026-10-17) and line 217 (rev 1.4, 2026-10-17) document the ICD revisions.

**Conclusion (7):** ICD rev 1.4 is correctly marked as **provisional, not
approved**. The provisional-profile note covers all §4 service rows
(S1/S3/S5/S6/S8/S9/S11/S17/S20). ADR-0020 is properly attributed and disclaims
all authority/approval/compliance claims. No approved ICD status is presented or
implied. ✓

## 4. Findings

### F-0377-01 (Medium, documentation/ledger — README index gap)
**Criterion/source:** N8/N16 review log completeness; `docs/reviews/README.md`
review index.
**Observed:** The README review index (lines 5–10) lists entries for reviews
0370–0375 only. It does **not** contain "0376" or "0377" anywhere in the file
(grep returns 0 matches for each). The 0376 R4 report file exists at
`docs/reviews/2026-10-17-s8-reconciliation-docs-r4-rereview-0376.md` and is
documented in CHANGELOG line 40, but is not indexed. The 0377 PA report (this
document) is also not indexed.
**Affected:** `docs/reviews/README.md` review index; N8 documentation currency.
**Gap:** Ledger inconsistency — published review files are not all discoverable
from the index.
**Risk:** Low — the files exist and are cited in the CHANGELOG, but the index is
incomplete, reducing discoverability and audit traceability.
**Owner:** FSW/documentation lead; verifier: independent R4/PA.
**Due:** Before any commit/push (N7/N8).
**Closure evidence:** Both 0376 and 0377 rows appended to the README review
index table.

### F-0377-02 (Medium, checklist/data-integrity — code delta scope)
**Criterion/source:** Configuration identity (manifest vs git diff); checklist
item 2.
**Observed:** A prior checklist claimed the behavioral source delta vs `b8c3aee`
was "just sched fix + comment-only `fsw_main.c`." This is **factually
incorrect**: `git diff --name-only b8c3aee` shows 4 PUS behavioral source files
changed (`src/pus/pus.c`, `src/pus/pus.h`, `src/pus/pus_services.c`,
`src/pus/adapters/pus_adapter_n7.c`) with substantive logic changes (duplicate
registration rejection, memory-region ID parameter, svc3 first-failure
retention, svc6 offset-based dump, svc11 stub event, svc20 length validation,
service-table subservice correction). The only comment-only change is
`fsw_main.c`.
**Affected:** Checklist item 2 accuracy; any downstream consumer relying on the
checklist's delta-scope characterization.
**Gap:** Inaccurate delta description.
**Risk:** Low for safety (the PUS changes are independently reviewed per R1
0349/0355, R2 0350/0356, and R3 0371); medium for evidence traceability if the
checklist is consumed as authoritative.
**Owner:** Checklist author / FSW CM.
**Due:** Before next cycle; correct the checklist delta characterization.
**Closure evidence:** Corrected checklist item 2 reflecting the full behavioral
delta (sched + PUS + tests) and the 211-file manifest coverage.

### F-0377-03 (Medium, configuration identity — FF_USE_N7_OBCP default)
**Criterion/source:** `CMakeLists.txt` option defaults; CI build evidence;
checklist item 2.
**Observed:** A prior checklist claimed `FF_USE_N7_OBCP=OFF` and that
`ff_obcp.c` is "NOT compiled." Both are **factually incorrect**:
- `CMakeLists.txt` line 26: `option(FF_USE_N7_OBCP "Build and link the pinned
  N7 Space MicroPython OBCP backend" ON)` — defaults **ON**.
- `CMakeLists.txt` line 25: `FF_USE_PUS_ASN1` also changed from `OFF` at
  `b8c3aee` to `ON`.
- CI-local log line 92: `[5/193] Building C object
  CMakeFiles/frick_fsw_core.dir/src/obcp/ff_obcp.c.o` — compiled.
- `src/obcp/` directory does **not** exist at `b8c3aee` (verified: `git ls-tree
  -r b8c3aee --name-only | grep obcp` returns 0) — it is a new untracked
  directory in the working tree.
**Affected:** Checklist item 2; build-configuration identity; any reviewer or
gate relying on the checklist's compilation claims.
**Gap:** Incorrect configuration identity claim.
**Risk:** Low for safety (OBCP is deferred per NC-S9-01; host connectivity
tested only); medium for evidence traceability.
**Owner:** Checklist author / FSW CM.
**Due:** Before next cycle.
**Closure evidence:** Corrected checklist reflecting `FF_USE_N7_OBCP=ON` default
and `ff_obcp.c` compilation, with `src/obcp/` noted as new vs `b8c3aee`.

### F-0377-04 (Comment, report-fidelity — 0376 F-0376-06 characterization)
**Criterion/source:** N8/N11 documentation record accuracy; R4 0376 report
F-0376-06.
**Observed:** The 0376 R4 re-review report's F-0376-06 finding states the
working-tree CHANGELOG entry at line 40 "states '0376 R4 (this re-review,
APPROVED WITH COMMENTS)' … the CHANGELOG entry does not carry this
qualification." However, the **current working tree** CHANGELOG at line 40
already reads "**0376 R4** — independent documentation re-review.
**CORRECTED APPROVED WITH COMMENTS** (… the review report itself carries the
report's own N9-independence caveat … PA **0377** pending independent
re-verification of the corrected tree; no commit/push until it passes)." The
`CORRECTED` qualifier, the N9-independence caveat, and the PA-0377 deferral are
all present. The 0376 report's description of the CHANGELOG does **not match**
the current working tree.
**Affected:** Accuracy of the 0376 report's own evidence citation.
**Gap:** The 0376 report's F-0376-06 premise (CHANGELOG lacks annotation) is
already satisfied in the current tree; no further CHANGELOG annotation is
required for F-0376-06.
**Risk:** None — the recommendation is already addressed; the 0376 report's
characterization is stale relative to the working tree.
**Owner:** R4 0376 author (for record correction).
**Due:** Next documentation update.
**Closure evidence:** Confirmation that CHANGELOG line 40 (working tree) already
carries the `CORRECTED` qualifier and N9-independence caveat (as verified above).

## 5. Residual provenance/identity gaps with owners (unchanged from PA 0373)

| Gap | Description | Owner | Status |
|---|---|---|---|
| ID-G1 | Updated candidate manifest/digest for current working tree (211-file manifest produced and verified) | FSW/CM + PA | **RESOLVED** (this review) |
| ID-G2 | Correlation of current CI-local run (log/cache/executable hashes) to the 211-file manifest | PA + FSW | **RESOLVED** (all 10 hashes verified) |
| PROV-G1 | NC-EXT-84: generator/tool provenance for N7 generated tree | FSW/tooling | OPEN |
| REV-G1 | Independent review of N22-TS5-77 test/oracle (PA 0373 F-0373-02; R2 0374 assessed) | R2 | **RESOLVED** (R2 0374 assessed TS5-77) |
| NC-G1 | NC-EXT-90: time overflow domain/policy/exhaustion response | Systems + FSW + V&V | OPEN |
| NC-G2 | NC-EXT-88/89: full PA acceptance pending full cycle/identity per 0354 | PA + FSW | Narrowly closed (defect corrected/verified by R1/R2) |
| NC-G3 | NC-EXT-19: mission schedule/time policy, target timing | Systems + FSW | OPEN |
| NC-G4 | NC-EXT-91: narrowly closed for 19-file PUS/Scheduler delta only | PA + CM | CLOSED (narrow) |
| NC-G5 | NC-EXT-92: reconcile summary vs row statuses | FSW + R2/R4 | OPEN (pre-existing) |
| ID-G3 | OBCP mission-use gaps (NC-S9-01 deferred) | FSW + OBCP authority | OPEN |

## 6. Evidence-log hash summary (all independently recomputed)

| File | SHA-256 (recomputed) | Match |
|---|---|---|
| `docs/vv/evidence/2026-10-17-s8-reconciliation-ci-local.log` | `4cecaa75…` | ✓ |
| `docs/vv/evidence/2026-10-17-s8-reconciliation-format-check.log` | `c43cc487…` | ✓ |
| `docs/vv/evidence/2026-10-17-s8-reconciliation-docs.log` | `f89ee26e…` | ✓ |
| `docs/vv/evidence/2026-10-17-s8-reconciliation-gsw-selfcheck.log` | `c9fbce7e…` | ✓ |
| `docs/vv/evidence/2026-10-17-s8-reconciliation-smoke.log` | `64b1a189…` | ✓ |
| `docs/vv/evidence/2026-10-17-s8-reconciliation-qemu.log` | `da25f66c…` | ✓ (SKIP content) |
| `docs/vv/evidence/2026-10-17-s8-reconciliation-rtems-cross-build.log` | `1e1b232b…` | ✓ |
| `docs/vv/evidence/2026-10-17-s8-reconciliation-renode.log` | `0a806a8d…` | ✓ |
| `build/ci/CMakeCache.txt` | `7f865d42…` | ✓ |
| `build/ci/frick_fsw` | `7ee78ef3…` | ✓ |
| Manifest combined digest | `c50798b5…` | ✓ |

## 7. Explicit conclusion

**Summary of verification against the seven checklist items:**

1. **Manifest identity — VERIFIED.** 211 rows; combined digest
   `c50798b5abcdaa345e62c50bc95f93aa25fbc15b426f5b6b2d93364ef8a8cbe2` matches
   independent computation; all 10 embedded log/artifact hashes independently
   recomputed with `sha256sum` and match. Resolves PA 0373 F-0373-04.
2. **Code delta bounds — VERIFIED WITH DISCREPANCIES.** 14 modified `.c`/`.h`
   files include PUS behavioral changes, not just sched fix + comment-only
   `fsw_main.c`. Checklist's item 2 scope claim is incorrect (F-0377-02).
3. **Evidence gates (8) — VERIFIED WITH CAVEAT.** 7 of 8 gates green (ci-local
   43/43, format-check clean, docs pass, gsw-selfcheck pass, smoke pass,
   rtems-cross-build pass, renode pass). QEMU is SKIP (ELF absent), not PASS.
   Checklist's "green" claim for QEMU is incorrect if QEMU was counted as PASS.
4. **Review chain & independence — VERIFIED.** All seven reviews (0370–0376)
   present as files and read in full; all R4 0376 findings independently
   reproduced. README index missing 0376/0377 entries (F-0377-01). F-0376-06
   characterization of CHANGELOG does not match working tree (F-0377-04).
5. **NC & authority status — VERIFIED.** All open NCs properly tracked with
   caveats; narrow closures (88/89/91/85/86/87) properly bounded; no premature
   closure claims; authority-owned items remain OPEN.
6. **Overstating claims — VERIFIED PASS.** No forbidden phrasings
   (certify/certification/flightworthy/flightworthiness/authorize release) in
   positive context across PA 0373, R4 0376, or manifest.
7. **ICD provisional status — VERIFIED.** ICD rev 1.4 correctly marked
   provisional; ADR-0020 attribution intact; all §4 rows covered by
   provisional note; no approved ICD claim.

**PA 0373 follow-up resolution:**
- F-0373-04 (stale manifests) — **RESOLVED** by the 211-file manifest verified
  in this review.
- F-0373-02 (TS5-77 lacks independent review) — **RESOLVED** by R2 0374, which
  assessed the `test_time_exhaustion_does_not_wrap` oracle, the
  activation→`FF_SchedInit` coverage limit, and the GAP-R3-1 closure-status
  wording. Verified from R2 0374 report content.
- F-0373-01 (closure taxonomy ambiguity) — **DOCUMENTED.** The narrow-closure
  vs PA-acceptance taxonomy is now explicitly recorded in CHANGELOG line 41
  (NC-EXT-93 entry) and status.json note. No further action required for this
  review.
- F-0373-03 (NC-EXT-92 section reference typo) — **RESOLVED** in sprint-plan
  rev 1.70 (line 228 cites "TS-4 §3.1").
- F-0373-05 (provenance tooling) — **UNCLOSED** — NC-EXT-84 remains
  PARTIALLY CORRECTED/OPEN.

**Overall PA determination:** The reconciliation candidate is internally
consistent and the evidence set is intact (211-file manifest verified, all
evidence-log and build-artifact hashes match, 7/8 gates green with QEMU SKIP
documented). The behavioral code delta includes PUS corrections beyond the
scheduler fix, and `FF_USE_N7_OBCP=ON` with `ff_obcp.c` compiled and
`src/obcp/` new vs `b8c3aee`. The checklist's item 2 claims are corrected
here. All seven reviews (0370–0376) are present and their findings
independently reproduced. The README review index is missing 0376/0377 entries.
Three PA-scoped findings (F-0377-01 through F-0377-03) and one report-fidelity
comment (F-0377-04) are recorded.

### No N20 phase/release approval granted

**Explicit statement:** This is an independent Software Product Assurance
assessment (profile PA, norm N20). It does **not** constitute N20 phase/release
approval, does **not** certify compliance, does **not** approve safety
case/residual risk, does **not** close nonconformances (NC-EXT-84/90/19/92/93,
NC-S9-01 remain OPEN; NC-EXT-88/89/91 are narrow closures only), does **not**
qualify tools (NC-EXT-84, T23, NC-EXT-40 remain open), does **not** authorize
release, and does **not** declare flightworthiness. The 211-file manifest is a
working-tree artifact, not a signed run attestation or release manifest. No
immutable/reproducible build or source-to-binary provenance is established. All
decisions requiring authorized human approval remain with the designated
independent qualified human authorities.

## Confidence block (per §14)

```text
Decision/product/config: Uncommitted S8 reconciliation candidate at working tree
  state (post b8c3aee, 211 differing files), bounded scope as defined.
Claim type: Fact | Inference | Recommendation | Open issue
Applicable baseline/tailoring: ECSS-Q-ST-80C/ECSS-E-ST-40C context per project
  docs; ECSS-E-ST-70-41C (2016) pinned for PUS-C reference; exact contract/mission
  tailoring not re-validated beyond inspection of referenced artifacts.
Evidence consulted: PA 0373, R4 0376, R2 0370/0374, R3 0371/0375; 211-file
  manifest (independently hash-verified); all 8 evidence gate logs (hashes
  independently recomputed with sha256sum); BUILD/ci/CMakeCache.txt and
  build/ci/frick_fsw (hashes independently recomputed); CMakeLists.txt;
  src/pus/pus.h:69; b8c3aee:CHANGELOG.md; sprint-plan 1.70; loop-audit 10.7;
  status.json; ICD.md 1.4; ADR-0020.
Cross-check/reconciliation: Manifest 211 rows + combined digest; all 10
  log/artifact hashes match; git diff 59 tracked + 150 untracked = 209 source
  files + 2 build artifacts = 211; src/obcp/ absent from b8c3aee; PUS behavioral
  delta confirmed present.
Confidence: High (manifest identity, evidence gate hashes, NC status, ICD
  provisional status, overstating-claims audit). Medium-High (code delta scope,
  checklist discrepancy corrections — verified against git diff and CI log).
Basis/detection limits: Direct inspection of primary artifacts, hash
  recomputation for all 211 manifest entries and 10 linkage hashes, git diff
  enumeration, CI log parsing. Did not re-execute the CI cycle as part of this
  assessment.
Missing/inaccessible evidence: Signed run attestation; reproducible/deterministic
  build evidence; complete generated-source/tool provenance (NC-EXT-84);
  physical-target runtime evidence (QEMU SKIP, Renode as host proxy only).
Assumptions/expiry: Assumes evidence logs and working-tree files are as
  inspected at this date. Recheck before any gate closure relying on this delta
  or the checklist's item 2.
Consequences if wrong: Misleading checklist claims could propagate incorrect
  delta-scope or compilation-characterization evidence to later gate reviews;
  low immediate safety impact (PUS changes independently reviewed; OBCP deferred
  per NC-S9-01).
Required human reviewer/authority: Designated PA authority; R1/R2/R3/R4 leads;
  Systems authority (NC-EXT-90/19); CM/tooling leads (NC-EXT-84/40); OBCP
  authority (NC-S9-01); release authority (commit/push gate).
```

**End of report FRICK-REVIEW-0377.**
