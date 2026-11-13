# R4 Independent Documentation Review — frick-fsw (norm N17)

**Reviewer profile:** Senior Aeronautics and Space Software Documentation Engineer (R4). No engineering/safety/certification/release authority; findings are advisory. Only the `aeronautics-space-fsw-engineer` agent (or an explicitly delegated human) may commit/push, and only after all applicable gates pass.

**Repository:** /home/nicola/fsw/frick-fsw, branch `main`
**Review date:** 2026-11-11
**Scope:** All documentation touched in this cycle:
- Committed (NC-EXT-237 cycle): ICD rev 1.7, TS-1 rev 1.10 (`N22-TS1-58`), TS-5 rev 1.16 (`N22-TS5-47`), `service-development.md` rev 0.5, CHANGELOG.md, sprint-plan.md, handover.md, loop-audit.md, verification-results.md, traceability.yaml.
- Uncommitted (NC-EXT-191 cycle): ts-2-link-security.md rev 0.10, tool-qualification.md rev 1.7, verification-results.md, handover.md rev 3.69, loop-audit.md rev 10.47, sprint-plan.md rev 1.131, traceability.yaml, CHANGELOG.md, README.md; code `src/transport/link/{link.h,link_ip.c,link_tsn.c}` + `tests/unit/{test_link_ip.c,test_link_tsn.c}` reviewed only as documentation anchors.

**Gate status observed:** `make build` 0 warnings; CTest 56/56 PASS; Lean `lake build` PASS; `trace.py` PASS (120/76/30/14/68) — re-run on the tree *with* the uncommitted traceability.yaml changes; pinned clang-format 18.1.3 PASS; `make gsw-selfcheck` OK; `make smoke` PASS (7 apps); RTEMS 6.2 cross build exit 0; `make renode` PASS. Four discriminating test binaries were built and executed from sources to verify the check counts cited in docs (see §Confirmations).

---

## 1. Findings

### DOC-R4-001 — TS-5 history contains a duplicate revision number (1.14)

- **Classification:** Minor
- **File:lines:** `docs/vv/test-spec/ts-5-core-services.md:436-437`
- **Evidence:** Two history rows both carry `1.14 | 2026-11-04` with different reasons:
  - row at :436 — “NC-EXT-236 evidence delivered: `test_tm_output_chain.c` (47 checks) closes the production sink gap…”
  - row at :437 — “FRICK-REVIEW-0458 F-0458-01 correction; hash re-bound to the corrected script…”
- **Norm impact:** N16 (detailed chronological revision history) / N8 (controlled documentation must unambiguously identify its revision). Duplicate revision numbers on the same date break revision lineage and configuration identification.
- **Current state:** Document header correctly reads Revision 1.16 (:7) and the :439 entry (1.16) states the header “stale at 1.14 after the 1.15 entry” was corrected to 1.16 — verified against :7. No technical content is wrong; the defect is history metadata only.
- **Blocks commit/push:** No. Disposition as an NC and correct before the next TS-5 revision/baseline (e.g., renumber the second entry `1.14a`/`1.15` with a distinct date or merge the two 11-04 entries). Does not block this functional increment under N21 (non-functional finding).

### DOC-R4-002 — `traceability.md` readable mirror is stale vs `traceability.yaml` (single source of truth)

- **Classification:** Minor
- **File:lines:** `README.md:54` (declares “Single source of truth: docs/requirements/traceability.yaml (mirrored in traceability.md)”); `docs/requirements/traceability.md:84` (SRS-REQ-011 row); `docs/requirements/traceability.md:147` (SRS-REQ-074 row); `docs/requirements/traceability.yaml:132` and uncommitted `:692-694`.
- **Evidence:**
  - `traceability.yaml:132` (committed in NC-EXT-237 cycle) lists `tests/unit/test_pus_event_identity.c` under SRS-REQ-011; `traceability.md:84` lists `test_pus.c, test_sb.c, test_r2_gap.c, test_r3_gap.c, ci:simulator-smoke` — the new evidence link is missing from the mirror.
  - The uncommitted `traceability.yaml` change adds `tests/unit/test_link_ip.c` and `tests/unit/test_link_tsn.c` to SRS-REQ-074 (`:692-694`); `traceability.md:147` still lists only `tests/unit/test_es_hs_lifecycle.c`.
  - `traceability.py` itself converges PASS on the yaml (120 requirements) — the manifest is consistent; only the hand-maintained mirror lags.
- **Norm impact:** N4 (single source of truth / bidirectional traceability), N8 (currency).
- **Blocks commit/push:** No for this commit; must be resynchronized before traceability.md is used as release/audit evidence, since a reader of the mirror would miss the NC-EXT-237 and NC-EXT-191 evidence links.

### DOC-R4-003 — Undefined status token “mapped” on N22-TS5-47; relation to §3 gap list unstated

- **Classification:** Minor
- **File:lines:** `docs/vv/test-spec/ts-5-core-services.md:177` (status column `**mapped**`), `:237` (§3 gap list still maps N22-TS5-47 as a gap row), `:387` (§3.2 coverage summary), `:1.2` status-convention text.
- **Evidence:** The N22-TS5-47 row ends with status `**mapped**`, while §3 (line 237) lists “N22-TS5-47 — scheduler heartbeat / late-missed-slot / dispatch-failure (partial: …)” under the gap mapping. §1.2 defines `covered` / `OOB` / class / env conventions; the visible convention text does not define a `mapped` status. §3.2 arithmetic holds (60 covered + 27 gap = 87 total, per :438-:439) only if `mapped` is folded into one of those buckets — not stated.
- **Norm impact:** N8 (unambiguous status), N4 (traceability status must be interpretable).
- **Blocks commit/push:** No. Define `mapped` in §1.2 or align the row status with §3 membership (covered/gap/characterization) at next revision.

### DOC-R4-004 — N22-TS5-47 evidence cell carries out-of-scope S5/NC-EXT-236 material (evidence scope bleed)

- **Classification:** Minor (editorial/attribution)
- **File:line:** `docs/vv/test-spec/ts-5-core-services.md:177`
- **Evidence:** N22-TS5-47’s case title/requirements are SRS-REQ-071 / SSS-REQ-030 (scheduler heartbeat, late/missed-slot, dispatch-failure), yet the evidence cell is dominated by PUS Service 5 emission plumbing (`test_evs_hs_health` 122/122 checks, NC-EXT-236 production sink chain) that is owned by SRS-REQ-011 and recorded against `N22-TS1-58` in TS-1 (`ts-1-protocol-core.md:168`) and cross-referenced by TS-5 `N22-TS5-63` (`ts-5-core-services.md:193`). The row’s own scheduler content (counters `release/missed/late/publish_errors`, events `0x0601–0x0605`) is correctly recorded. Not contradictory, but the evidence cell risks misattributing verification evidence across work packages.
- **Blocks commit/push:** No. Editorially trim the S5 material to a cross-reference or move it to a dedicated note; keep N22-TS5-47’s evidence cell focused on scheduler/HS evidence.

### DOC-R4-005 — Untracked evidence directories and stray backup file in the working tree

- **Classification:** Minor (housekeeping / data integrity)
- **File:lines:** `git status ??`: `docs/vv/evidence/2026-11-11-nc-ext-237/`, `docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/`; `tests/unit/test_sched_events.c.bak`.
- **Evidence:** The evidence logs cited in handover.md (:NC-EXT-191 / NC-EXT-237 sections), loop-audit.md (:10), verification-results.md, TS-2 (:0.10 entry) and TS-5 (:177, :439) live in untracked directories (`docs/vv/evidence/2026-11-11-nc-ext-…`) and will not be part of the commit. `tests/unit/test_sched_events.c.bak` is an untracked backup in the test sources (not compiled by the `*.c` glob; CTest 56/56 unaffected).
- **Norm impact:** N11 (evidence retention), N8 (deployable/current CM state).
- **Blocks commit/push:** No. Add evidence directories to CM/manifest or exclude them explicitly; remove or exclude `.bak`. Not a blocker for this increment.

### DOC-R4-006 — TS-5 header “Baseline” field not refreshed for the 2026-11-11 deltas

- **Classification:** Minor (observation)
- **File:line:** `docs/vv/test-spec/ts-5-core-services.md:14`
- **Evidence:** Baseline reads “Working tree 2026-10-16 (S8-SCH dispatcher increment); previous baseline HEAD 57974d2 read 2026-10-14”, while the latest document revisions are 2026-11-11 (NC-EXT-237 rev 1.16; NC-EXT-191 rev 1.16 same doc). TS-1’s header (:14) by contrast states “Current candidate source HEAD `b8c3aee` plus uncommitted PUS/S8-SCH/OBCP source and test deltas”. Inconsistent companion-header metadata.
- **Blocks commit/push:** No. Refresh the Baseline field at the next TS-5 revision.

---

## 2. Confirmations (no findings)

| # | Item | Checked at | Result |
|---|------|------------|--------|
| C-1 | ICD §4 Service 5 row vs `FF_PusEmitEvent` implementation | `docs/interfaces/ICD.md:115`; `src/pus/pus.c:356-388` | **Consistent.** Both specify the S5 report header `event id[2, big-endian] \| type[1]` built by `FF_PusEmitEvent` from `event_id`/`evs_type`; `data`/`len` are auxiliary data only; callers must not prepend id/type; subservice INFO=1/WARNING=2/else=3; bounds `FF_PUS_MAX_TM_FRAME − 3` (`FF_BAD_SIZE`) and `data==NULL && len!=0` (`FF_INVALID_ARGUMENT`). NC-EXT-237 states “no wire behavior changed”, which the code confirms. |
| C-2 | `service-development.md` §4 event contract | `docs/engineering/service-development.md:78-88`, rev 0.5 | **Consistent** with ICD §4 and `pus.c` (emitter-owned header, auxiliary-data-only callers, severity→subservice 1/2/3, length/NULL bounds, NC-EXT-102 catalogue item, NC-EXT-237 oracle reference). |
| C-3 | Check counts cited in docs vs executed binaries | Built from sources in `/tmp/opencode/build-native/tests/` and executed: `test_pus_event_identity.c` → `checks=106 failures=0`; `test_evs_hs_health.c` → `checks=122 failures=0`; `test_link_ip.c` → `checks=93 failures=0`; `test_link_tsn.c` → `checks=149 failures=0` | **All counts verified accurate** (106/122/93/149 match TS-1:168, TS-5:177, TS-2:0.10, CHANGELOG, sprint-plan). All four binaries exit 0. |
| C-4 | TS-1 `N22-TS1-58` vs TS-5 `N22-TS5-47` cross-citation | `ts-1-protocol-core.md:168`, `:332` (rev 1.10); `ts-5-core-services.md:177`, `:439` (rev 1.16) | **Consistent.** TS-1 restates the emitter-owned header contract and owns the identity oracle (`test_pus_event_identity.c`, 5 cases/106 checks, 5 test functions confirmed). TS-5 correctly states `test_evs_hs_health` asserts payload transparency only and does not verify event-identity semantics. No current document states `event_id` is ignored; the `(void)event_id;` description appears only as pre-fix history (the construct is absent from current `src/pus/pus.c`). |
| C-5 | traceability.yaml entries vs SRS/SSS ids and test files | `traceability.yaml:129-132` (SRS-REQ-011 → SSS-REQ-006, tests include `test_pus_event_identity.c`); `:672-674` (SRS-REQ-071 → SSS-REQ-030); `:685-694` (SRS-REQ-074 → SSS-REQ-008; uncommitted tests list adds `test_link_ip.c`/`test_link_tsn.c`); `:932` (SSS-REQ-030 exists, 1 SRS child) | **Valid.** `trace.py` convergence PASS (120 requirements: 76 SRS / 30 SSS / 14 gaps; 68 Lean theorems) on the tree including uncommitted yaml changes. TS-2 rev 0.10 case table and metric sheet internally consistent (83 = 44 nominal + 37 OOB + 2 mixed; 74 individual + 9 combined; 50 covered + 32 gap + 1 characterization; SIL/PIL/HIL = 78/2/3). |
| C-6 | Revision history completeness & chronology | ICD `:215-233` (0.1→1.7 chronological); TS-1 `:310-332` (0.1→1.10; `0.8a` is an intentional disambiguation of the duplicated `0.3` label); TS-5 `:413-439` (0.1→1.16, except DOC-R4-001); service-development.md `:190-198` (0.1→0.5); handover.md (3.58→3.69); loop-audit.md (:0.1→10.47); sprint-plan.md (:1.128→1.131); CHANGELOG.md (Unreleased sections for NC-EXT-237 + NC-EXT-191) | **Complete and chronological** except DOC-R4-001. |
| C-7 | Cross-document currency (handover/loop-audit/sprint-plan/verification-results/CHANGELOG) | handover.md rev 3.69; loop-audit.md rev 10.47; sprint-plan.md rev 1.131; verification-results.md NC-EXT-191 section; CHANGELOG Unreleased | **Consistent.** All four record the NC-EXT-191 increment with matching evidence (CTest 56/56, trace 120/76/30/14/68, Renode ELF sha256 `55188f5a…`), TS-2 rev 0.10, tool-qualification rev 1.7 (T3 still NOT FULLY QUALIFIED), NC-EXT-191/NC-EXT-237 both OPEN, no NC closed. |
| C-8 | README status panel | `README.md:54-95` | **Current.** “last updated 2026-11-11”; coverage 15328/32948 (46.5%) and functions 1749/2593 (67.5%) match `verification-results.md` metrics; gates legend defined. |
| C-9 | Norm N2 (English) | All reviewed docs | **Pass.** All documentation text is English; no non-English content found. |

---

## 3. Norm N8 assessment (complete, current, deployable documentation for humans and agents)

The documentation set satisfies N8 for this cycle: AGENTS.md entry point and quickstart are consistent with the repository; every changed document carries revision/date/author and a history section; the traceability manifest converges; generated/derived data (checksums, script sha256, gate logs) are cited; review authority statements are present in TS-1/TS-5 headers. Residual N8 items are DOC-R4-002 (mirror resync), DOC-R4-005 (evidence under CM), DOC-R4-006 (TS-5 baseline field). None of these makes the current tree undeployable; all must be closed before the documentation baseline is released.

---

## 4. Block/push decision

**No finding blocks the current commit/push.** All findings are Minor/housekeeping and, per N21, non-functional findings do not stop unrelated development; they are recorded as NCs for later review/disposition and must be closed before the documentation baseline/release is authorized by the appropriate authority (Software PA for NC-EXT-237 acceptance; interface/requirements authority for ICD/profile items). The technical core of the cycle — the NC-EXT-237 fix (emitter-owned S5 header) with its discriminating oracle, and the NC-EXT-191 backend-coverage half (IP/TSN deadline writers) — is documented consistently with the code and evidenced by green gates.

**Required before baseline/release:**
1. Fix TS-5 duplicate revision `1.14` (DOC-R4-001).
2. Resynchronize `traceability.md` with `traceability.yaml` (DOC-R4-002).
3. Define or align the `mapped` status on N22-TS5-47 (DOC-R4-003); trim N22-TS5-47 evidence-cell scope bleed (DOC-R4-004).
4. Bring untracked evidence under CM and remove/exclude `tests/unit/test_sched_events.c.bak` (DOC-R4-005).
5. Refresh TS-5 header Baseline field (DOC-R4-006).
