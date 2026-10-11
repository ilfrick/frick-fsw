# Review Report — Reference NOR/QSPI MTD backend (SRS-REQ-056), documentation (N17)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0093                      |
| Reviewer session                  | (independent agent, norm N9)           |
| Reviewer profile                  | R4 (SW documentation)                  |
| Reference revision                | a3cdd68 (Sprint 1.2: reference NOR/QSPI MTD backend, SRS-REQ-056) |
| Items under review                | `docs/requirements/SRS.md`, `docs/requirements/traceability.yaml`, `docs/requirements/traceability.md`, `README.md`, `docs/development-plan.md`, `docs/status/loop-audit.md`, `docs/status/handover.md`, `CHANGELOG.md`, `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md` |
| Date                              | 2026-10-11                             |
| Scope                             | Documentation consistency of the NOR/QSPI MTD increment: single metrics panel; header `Revision` vs `History`; requirement/coverage counts (98/57/27/14); stale "QSPI pending" / "NOR MTD next" statements; cross-document consistency of the M11/M4 completion state; relative links/anchors; English-only (norm N2); re-run of the CI docs `required`/`uniqueness` block and `tools/scripts/trace.py` |
| Independence declaration          | Independent R4 reviewer (fresh context); no part in authoring/executing the reviewed items |
| Norms applied                     | N1–N20 (see `docs/rules.md`); focus N2, N3, N4, N8, N9, N16, N17 |

## 1. Scope

Reviewed the documentation set changed by the Sprint 1.2 increment that adds the
reference NOR/QSPI MTD backend (SRS-REQ-056) at HEAD `a3cdd68`. Excluded from
scope (other profiles): the driver source `src/drivers/mtd/mpfs/ff_mtd_mpfs.{c,h}`,
`tests/unit/test_mtd.c`, `src/apps/app_renode_test.c` and the platform header
(R1/R2 code review), and the product-assurance judgement (N20/PA). The technical
content of SRS-REQ-056 (correctness of the requirement statement versus the
implementation) was checked only for documentary consistency and completeness of
trace, not for engineering adequacy.

Reproduction performed on the reviewed revision (all green):

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml`
  → `traceability: PASS (98 requirements, 57 srs, 27 sss, 14 gaps, 51 lean theorems checked)`.
- CI docs block (required-documents + document-id uniqueness, copied verbatim from
  `.github/workflows/ci.yml`) → `documentation set: OK (142 unique document ids)`.
- Reviewer-agent persona drift block → `reviewer-agent personas: bodies in sync`.
- `python3 tools/scripts/metrics.py --check` → `metrics panel: current`.
- Relative-link scan over the changed files → no broken links.
- English-only scan → compliant (only typographic non-ASCII: em dash, `§`, `×`,
  ellipsis, arrows, and the codename "Eärendil").

## 2. References

- Project norms: `docs/rules.md` (N1–N20)
- Traceability manifest: `docs/requirements/traceability.yaml`
- ECSS-E-ST-40C (software information/DRDs, incl. SRS + SDD Annex F),
  ECSS-Q-ST-80C (product assurance/data), ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2
- Review template: `docs/reviews/review-template.md`

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| 1 | Minor | `docs/vv/tool-qualification.md` (N16) | Header `Revision` and last `History` row disagree: header is `0.3.10` while the history records `0.3.11` for this increment. The T3 row gain (`test_mtd.c`) and T17 row gain (`hal_mtd`) were made but the header was not advanced. | `docs/vv/tool-qualification.md:7` (`Revision | 0.3.10`) vs `:109` (`| 0.3.11 | ... T3 gains tests/unit/test_mtd.c ...`) | Set the header `Revision` to `0.3.11` (or renumber the history entry) so header and History agree. |
| 2 | Minor | `docs/status/loop-audit.md` (N16/N19) | The file was edited in this commit (M4 objective row and N3 rule count) but its `Revision`/`History` were not advanced: header is `0.6`, last history row is `0.6` (the NVM loop). Per N19 the audit is re-baselined each loop; this increment is a new loop. | `docs/status/loop-audit.md:7` (`Revision | 0.6`); content changed at `:27` (M4 row) and `:49` (N3 `compliant (98 reqs)`); last history row `:158` (`0.6`) | Add a `0.7` history row (MTD done, counts 98/57/27/14) and bump the header Revision. |
| 3 | Minor | `docs/development-plan.md` (N16) + M11 state (N8) | The M4 row was updated to "QSPI/NOR MTD done" but the plan's Revision/History were not advanced (header `0.15` = last history row). Separately, the M11 row still reads `in progress (5/13 peripheral-bound)` and its "done" list omits MTD, contradicting the M4 row in the same document. | `docs/development-plan.md:7` (`Revision | 0.15`) & `:333` (last history `0.15`); `:295` ("QSPI/NOR MTD done") vs `:302` ("5/13 peripheral-bound … done: GPIO/SERIAL/MEM/ETH/NVM") | Add a History row for the M4/M11 progress update; reconcile the M11 row (`6/13`, add NOR MTD to the done list / to `hal_mtd=PASS`). |
| 4 | Minor | `docs/status/handover.md` §4 (N8/N16) | The resume-point "Current state" is internally inconsistent with its own §5 and History: §4 still reports **5/13** peripheral-bound classes and its state list and Renode-check list (`hal_gpio/serial/mem/eth/net/nvm`) omit MTD/`hal_mtd`, while History row 1.4 and §5 already claim the NOR/QSPI MTD backend done. Additionally the command block header still says "unit tests (**14/14**)" (now 18/18) and §5 line 126 still lists "NOR MTD" as an upcoming class. | `docs/status/handover.md:97` ("**5/13** peripheral-bound"); `:106` (Renode checks without `hal_mtd`); `:63` ("(14/14)"); `:126` ("NVM done; next classes: NOR MTD, CAN, ..."); vs `:130` ("[NVM + NOR/QSPI MTD done]") and `:187` (History 1.4) | Update §4 to 6/13 incl. NOR MTD + `hal_mtd=PASS`; fix "(14/14)"→"(18/18)"; drop "NOR MTD" from the next-class list at `:126`. |
| 5 | Minor | `docs/vv/verification-results.md` "Remaining (M4)" (N8) | The outstanding-driver list still names **QSPI** as not delivered, contradicting the SRS-REQ-056 delivery recorded in the same document's cycle row. Stale after this increment. | `docs/vv/verification-results.md:277` ("CAN/SPI/I2C/**QSPI**/MMC/eMMC/storage/PCIe/SpaceWire MSS drivers (M4 residual)") vs `:39` (MTD row, PASS) | Remove `QSPI` from the "Remaining (M4)" list (keep CAN/SPI/I2C/MMC/eMMC/storage/PCIe/SpaceWire). |
| 6 | Minor | `README.md` project-scope table + focus (N8) | The "Project scope" table still lists `QSPI/NOR` in the **Planned** column and omits NVM and NOR MTD from the **Implemented now** column, contradicting the new drivers row (`:186`) added in this same commit. The current-focus sentence also still says "NVM done; NOR/CAN". | `README.md:137` ("Implemented now … GEM … GPIO; MSS MMUART; RAM pool/DMA" ; Planned "… I2C, **QSPI/NOR**, …") vs `:186` (NOR driver implemented); `:408` ("NVM done; NOR/CAN + M10 QEMU") | Add NVM + NOR MTD to the implemented column (or at least NOR MTD), remove `QSPI/NOR` from Planned, and refresh the focus sentence to "NVM + NOR done; CAN + M10 QEMU". |
| 7 | Minor | Design traceability / SDD (N3/N8; E40C design DRD Annex F) | SRS-REQ-056 is delivered and traced to SSS-REQ-020 but is **not** allocated in the SDD requirements→design matrix, so the README "Design traceability" indicator regressed from 100% to **56/57 (98.2%)** in this commit. This is within the already-tracked SDD-matrix NC (PA-67-01, S0/S1), but this increment introduced a fresh instance that the panel transparently reports. | `README.md:54` ("Design traceability | 56/57 (98.2%)"); `docs/design/software-design.md:113` lists through `SRS-REQ-055` only; `docs/status/loop-audit.md:146` (PA-67-01 tracked) | Add SRS-REQ-056 to the SDD design matrix (or record explicitly that it is deferred to the PA-67-01 S0/S1 close-out) so design traceability returns to 57/57. |
| 8 | Comment | `docs/vv/renode-environment.md` (N8; not changed by this commit) | The HAL-coverage line still reads "(4/13)" peripheral-bound, i.e. it was not advanced for the NVM increment (5/13) and not for MTD (6/13). Cross-document stale count. | `docs/vv/renode-environment.md:88` ("**Peripheral-bound** (4/13)"); compare `docs/development-plan.md:302` (5/13) | Refresh to 6/13 and include NVM + NOR MTD (with `hal_mtd=PASS`). |
| 9 | Comment | `docs/requirements/traceability.md` prose (pre-existing) | The prose says "Six of the requirements (SRS-REQ-045/046/051/052/053/054 …) are planned … documented-only, with `ci:docs` as evidence", but SRS-REQ-052 already carries `tests/unit/test_r3_gap.c` + a non-doc implementation, so it counts as delivered (hence 52/57 ⇒ 5 documented-only, not 6). The count text `SRS = 57` was updated in this commit; the "Six" sentence was not. | `docs/requirements/traceability.md:20-23`; `traceability.yaml` SRS-REQ-052 (`implementation` includes `src/pus/pus.h`, `tests` includes `tests/unit/test_r3_gap.c`) | Reword to five documented-only, or state that SRS-REQ-052 is partially delivered. |
| 10 | Comment | `CHANGELOG.md` structure (pre-existing) | The `[Unreleased]` section contains two `### Added` sub-sections; the new MTD bullet was added to the first, which is fine, but the duplicate heading structure is retained. | `CHANGELOG.md:10` and `:32` (`### Added`) | Consolidate into a single `### Added` for the release (housekeeping; no technical impact). |
| 11 | Comment | `docs/reviews/README.md` review-log row | Per the review-log procedure the new review should be appended to the log. Not done here (this task was scoped to create the report only). | `docs/reviews/README.md:1-92` (log has no row for FRICK-REVIEW-0093) | Append the FRICK-REVIEW-0093 row when the review is recorded (norm N11). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

No Major finding. The core documentation of the increment is consistent and the
mandatory gates pass on the reviewed revision: `trace.py` PASS (98/57/27/14),
the CI docs required/uniqueness block OK (142 unique document ids), a single
metrics panel (`README.md:39..94`, byte-identical to `metrics.py` stdout,
`--check` current), and the requirement/coverage counts (98/57/27/14; delivered
52/57; tests 84/98; Lean 20/98) are mutually consistent. The findings are
documentary non-conformities (header/History drift, stale M11/M4 status
sentences, one missing SDD design allocation) that must be corrected to
compliance (N12) before the increment is closed.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 / R2 / R3 / R4 (N9) / PA (N20) | R4 (Senior SW documentation engineer) | 2026-10-11 | APPROVED WITH COMMENTS |

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R4-93-01 | Minor | `tool-qualification.md` (N16) | Header `0.3.10` vs History `0.3.11` | open | — | frick-fsw | open | — |
| R4-93-02 | Minor | `loop-audit.md` (N16/N19) | Content changed, Revision/History not advanced (0.6) | open | — | frick-fsw | open | — |
| R4-93-03 | Minor | `development-plan.md` (N16/N8) | M4 row changed without Revision/History; M11 row stale (5/13) | open | — | frick-fsw | open | — |
| R4-93-04 | Minor | `handover.md` §4 (N8/N16) | §4 stale (5/13, no `hal_mtd`, "(14/14)", "NOR MTD" next) | open | — | frick-fsw | open | — |
| R4-93-05 | Minor | `verification-results.md` (N8) | "Remaining (M4)" still lists QSPI | open | — | frick-fsw | open | — |
| R4-93-06 | Minor | `README.md` (N8) | Scope table stale (QSPI/NOR planned); focus sentence stale | open | — | frick-fsw | open | — |
| R4-93-07 | Minor | SDD design matrix (N3) | SRS-REQ-056 not allocated; design traceability 56/57 | open | — | frick-fsw | open | — |
| R4-93-08 | Comment | `renode-environment.md` (N8) | HAL coverage "(4/13)" stale | open | — | frick-fsw | open | — |
| R4-93-09 | Comment | `traceability.md` prose | "Six … documented-only" vs 52/57 (SRS-REQ-052 partially delivered) | open | — | frick-fsw | open | — |
| R4-93-10 | Comment | `CHANGELOG.md` | Duplicate `### Added` in `[Unreleased]` | open | — | frick-fsw | open | — |
| R4-93-11 | Comment | `docs/reviews/README.md` | Review-log row for FRICK-REVIEW-0093 not yet appended | open | — | frick-fsw | open | — |

---

Committed and pushed with the reviewed change (norm N11).