# Review Report — Renode V&V environment: BeagleV-Fire board repl (documentation, N17)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0101                      |
| Reviewer session                  | (independent agent, norm N9)           |
| Reviewer profile                  | R4 (SW documentation)                  |
| Reference revision                | 8764dd4 (Renode V&V environment: enable the BeagleV-Fire board repl) |
| Items under review                | `tools/docker/renode/frick-renode.resc` (platform switch), `docs/vv/renode-environment.md` (platform row + narrative), `docs/vv/verification-results.md` (0.28 cycle row), `CHANGELOG.md`, `docs/status/loop-audit.md` (QSPI task wording) |
| Date                              | 2026-10-11                             |
| Scope                             | Documentation consistency of the Renode board-repl change: platform referenced consistently across the changed files and the normative environment description; trace/coverage counts; header `Revision` vs `## History`; English-only (norm N2); relative links; re-run of the CI docs `required`/`uniqueness` block and `tools/scripts/trace.py` |
| Independence declaration          | Independent R4 reviewer (fresh context); no part in authoring/executing the reviewed items |
| Norms applied                     | N1–N20 (see `docs/rules.md`); focus N2, N8, N16, N17, N18 |

## 1. Scope

Reviewed the documentation set changed by commit `8764dd4`, which switches the
Renode **functional** test script from the SoC-only
`@platforms/cpus/polarfire-soc.repl` to the board repl
`@platforms/boards/beaglev-fire.repl` (which includes the SoC and adds the QSPI
NOR flash `SPI.MPFS_QSPI` + `SPI.Micron_MT25Q`, `MPFS_SystemServices`, and a
board button on GPIO0), and the four documents changed in the same commit
(`docs/vv/renode-environment.md`, `docs/vv/verification-results.md`,
`CHANGELOG.md`, `docs/status/loop-audit.md`).

Excluded from scope (other profiles): the correctness/adequacy of the emulated
platform switch, the HAL self-check behaviour, and the engineering claim that a
genuine QSPI write/erase check still needs the controller command driver
(R1/R2). The technical content was checked only for documentary consistency and
completeness of the trace.

Assumptions recorded: the environment self-test script
`tools/docker/renode/renode_env.resc` was **not** changed by this commit and
still loads `@platforms/cpus/polarfire-soc.repl` (verified), so references to
the self-test loading `polarfire-soc` remain correct; only the functional run
(`renode_vv.sh` → `frick-renode.resc`) changed platform.

Reproduction performed on the reviewed revision (all green):

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml`
  → `traceability: PASS (98 requirements, 57 srs, 27 sss, 14 gaps, 51 lean theorems checked)`.
- CI docs block (required-documents + document-id uniqueness, copied verbatim from
  `.github/workflows/ci.yml`) → `documentation set: OK (152 unique document ids)`
  (the 152 count is before this report is committed; this report introduces the
  unique id `FRICK-REVIEW-0101`, so the next run reports 153).
- Reviewer-agent persona drift block → `reviewer-agent personas: bodies in sync`.
- Relative-link scan over the five changed files → no broken links.
- English-only scan → the changed text is ASCII; the only non-ASCII in the
  touched files is the pre-existing prime in `CHANGELOG.md:241` (`F′`), not part
  of this change.

## 2. References

- Project norms: `docs/rules.md` (N1–N20)
- Traceability manifest: `docs/requirements/traceability.yaml`
- ECSS-E-ST-40C (software information/DRDs), ECSS-Q-ST-80C (product assurance/data), CCSDS 133.0-B-2
- Review template: `docs/reviews/review-template.md`
- Preceding related reviews: FRICK-REVIEW-0093/0097 (MTD docs), FRICK-REVIEW-0083/0088 (NVM docs)

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| 1 | Minor | `docs/vv/renode-environment.md` (N16) | **Per-document history not advanced although the content changed.** The platform row (`:26`) and the section 1 narrative (`:17-18`) were edited in this commit, but the header `Revision` stays `0.9` and no `## History` row was added (latest row is `0.9`, "M11: NET … exercised"). The sibling artifact `verification-results.md` was correctly bumped to `0.28` with a matching history row (`:7`, `:417`); `renode-environment.md` was not, so it contradicts its own change. | `docs/vv/renode-environment.md:7` (`Revision | 0.9`) and `:124` (last history row `0.9`); diff of `8764dd4` (no history hunk); compare `docs/vv/verification-results.md:7,417` | Add a `0.10` history row (functional platform switched to the `beaglev-fire` board repl; QSPI NOR + system services loaded; all HAL checks still PASS) and bump the header `Revision` to `0.10`. |
| 2 | Minor | `docs/status/loop-audit.md` (N16/N19) | **`Revision`/`History` not advanced although a tracked disposition changed.** The QSPI disposition row was reworded (`:147`, "board repl … now loaded; needs the QSPI controller command driver"), but the header stays `0.7` (equals the last history row) with no new row. The same defect class was raised for this file by FRICK-REVIEW-0093 finding 2 and is repeated here. | `docs/status/loop-audit.md:7` (`Revision | 0.7`), `:147` (changed row), `:160` (last history row `0.7`) | Add a `0.8` history row (QSPI environment enabled; driver pending) and bump the header `Revision`. |
| 3 | Minor | `docs/vv/renode-environment.md:73` (N8/N17; N18) | **Stale platform reference for the functional run.** Section 4 states "`renode_vv.sh` runs the pinned Renode image headless on the `polarfire-soc` platform", but `frick-renode.resc` now loads `beaglev-fire.repl`. The reader following the doc will expect the wrong repl. Note the adjacent `make renode-env` bullet (`:80`) remains **correct** because `renode_env.resc` is unchanged — the doc should distinguish the two. | `docs/vv/renode-environment.md:73-74` vs `tools/docker/renode/frick-renode.resc:15`; `tools/docker/renode/renode_env.resc:12` (unchanged) | Reword `:73` to "on the `beaglev-fire` board platform (which includes `polarfire-soc`)" and, where useful, state explicitly that the environment self-test still loads the SoC-only repl. |
| 4 | Minor | `docs/vv/renode-environment.md:39-47,89-93` (N8) | **NVM omitted from the marker example and the peripheral-bound enumeration, contradicting the changed verification row.** Section 3 shows the expected console output but omits `[renode-test] hal_nvm=PASS`, while the app prints it (`src/apps/app_renode_test.c:294`) and the new `verification-results.md:40` claims `hal_nvm=PASS`. Section 5 states "**Peripheral-bound** (5/13)" but enumerates only four classes (GPIO/SERIAL/MEM/ETH); NVM is the fifth. Partly residual (section 3/5 predate this commit), but the two changed documents now disagree. | `docs/vv/renode-environment.md:39-47` (no `hal_nvm`), `:89` (`5/13`) vs `:89-93` (four classes, marker list without `hal_nvm`); `src/apps/app_renode_test.c:225,294`; `docs/vv/verification-results.md:40` | Add `[renode-test] hal_nvm=PASS` to the section 3 example and add **NVM** (SRS-REQ-055) + `hal_nvm=PASS` to the section 5 peripheral-bound list. |
| 5 | Comment | Norm N18 description & README (N8) | **Cross-document naming of the functional platform was not reconciled.** The normative N18 description still pins the environment to the "`polarfire-soc` platform" (`docs/rules.md:291`), and other user-facing text does the same (`README.md:305`, `docs/tooling.md:70`, `docs/roadmap.md:146`, `docs/development-plan.md:189`, `docs/requirements/SRS.md:213`, `docs/quality/class-a-gaps.md:93`, `docs/vv/emulated-target.md:31,48`, `docs/status/handover.md:154`). None of these is false (the board repl includes the SoC), but after the switch the doc set is not uniform. | `docs/rules.md:291`; `README.md:305`; `docs/tooling.md:70`; `docs/roadmap.md:146` | At minimum refresh `docs/rules.md` (N18) and `README.md` to name the `beaglev-fire` board repl (with the SoC included); no derogation otherwise (Comment). |
| 6 | Comment | `docs/vv/verification-results.md:40` (N8) | **Cycle-row formatting is not uniform with its siblings.** The trace cell reads `PASS (98/57/27/14, 51 theorems, 12 CI claims)` whereas every other row uses the long form `98 reqs / 57 srs / 27 sss / 14 gap`; the ctest cell `PASS (18/18)` also omits the check-count detail used elsewhere. Values are correct (trace reproduced 98/57/27/14/51). | `docs/vv/verification-results.md:40` vs `:39` and prior rows | Align the 0.28 row to the established `98 reqs / 57 srs / 27 sss / 14 gap` form (cosmetic). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must be
  corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### Positive verification (no finding)

- Platform reference updated consistently in `frick-renode.resc:3,15` and in
  `renode-environment.md:26`; the board contents (QSPI NOR, system services, board
  button) match the commit message and the verification row.
- Counts consistent between the changed files: trace reproduced
  `98/57/27/14, 51 theorems`; the `0.28` row cites the same; `CHANGELOG.md` and
  `loop-audit.md` carry no contradictory counts for this increment.
- `verification-results.md` header `Revision` `0.28` = latest `## History` row
  `0.28` (`:7`, `:417`).
- CI docs `required`/`uniqueness` block: OK (152 unique ids, no duplicates);
  reviewer-agent persona drift: in sync; no broken relative links; English-only
  (N2) satisfied.
- `renode-environment.md` section 2 row and section 1 narrative agree with the
  actual `.resc` load string.

## 4. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**This review (R4, N17): APPROVED WITH COMMENTS.** No Major finding. Findings 1–4
are Minor documentation non-conformities (N16 revision/history and N8 stale/
incomplete platform and NVM content) to be corrected in a follow-up commit and
delta-re-reviewed per N12; findings 5–6 are Comments with no derogation.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 / R2 / R3 / R4 (N9) / PA (N20) | R4 (Senior SW documentation engineer) | 2026-10-11 | APPROVED WITH COMMENTS |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R4-101-01 | Minor | `docs/vv/renode-environment.md:7,124`; N16 | Revision/History not advanced for the board-repl content change | pending |  | frick-fsw | open | pending |
| R4-101-02 | Minor | `docs/status/loop-audit.md:7,147,160`; N16/N19 | Revision/History not advanced for the reworded QSPI row | pending |  | frick-fsw | open | pending |
| R4-101-03 | Minor | `docs/vv/renode-environment.md:73`; N8/N17/N18 | Functional run still described as loading `polarfire-soc` | pending |  | frick-fsw | open | pending |
| R4-101-04 | Minor | `docs/vv/renode-environment.md:39-47,89-93`; N8 | `hal_nvm`/NVM missing from marker example and peripheral-bound list (`5/13` enumerates four) | pending |  | frick-fsw | open | pending |
| R4-101-05 | Comment | `docs/rules.md:291`; `README.md:305`; N8 | Cross-document `polarfire-soc` naming not reconciled | pending |  | frick-fsw | open | pending |
| R4-101-06 | Comment | `docs/vv/verification-results.md:40`; N8 | Trace-count cell format differs from sibling rows | pending |  | frick-fsw | open | pending |

---

Committed and pushed with the reviewed change (norm N11).