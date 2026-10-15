# S4b follow-up documentation review (norm N17) — R4

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0168                      |
| Reviewer session                  | (independent agent, norm N9)           |
| Reviewer profile                  | R4 (Senior SW documentation engineer)  |
| Reference revision                | baseline `57974d2` + uncommitted "S4b follow-up" working tree |
| Items under review                | The S4b follow-up change (12 modified files) and the documentation set it affects: `CHANGELOG.md`, `README.md`, `docs/planning/sprint-plan.md`, `docs/quality/class-a-gaps.md`, `docs/requirements/traceability.md`, `docs/requirements/traceability.yaml`, `docs/status/loop-audit.md`, `docs/status/status.json`, `docs/vv/tool-qualification.md`, `docs/vv/verification-results.md`, `tests/unit/test_link_ip.c`, `tests/unit/test_link_tsn.c`; plus the wider documentation set against which the delivered scope must be consistent (`docs/engineering/software-description.md`, `docs/architecture/network-stack.md`, `docs/development-plan.md`, `docs/roadmap.md`, `docs/status/handover.md`, `docs/engineering/user-integration-manual.md`, `docs/interfaces/ICD.md`). |
| Date                              | 2026-10-13                             |
| Scope                             | Independent **documentation review (norm N17)** of the uncommitted "S4b follow-up" change (N19 conformance audit closing NC-S4b-01..05, plus the host `pus_ip`/`pus_tsn` PUS-C TM/TC round-trip demonstrations). Verified: (a) the implemented content matches the documented scope across the whole documentation set; (b) document currency (each header `Revision`/`Date` matches its `## History` table) and revision-history completeness (N16); (c) count accuracy (test-function/check counts, requirement/coverage counts) against first-hand measurement; (d) cross-document consistency (traceability, SRS, verification-results, sprint-plan, loop-audit, CHANGELOG, status); (e) English-only content (N2). Assessed against ECSS-E-ST-40C, ECSS-Q-ST-80C Rev.2 and the project norms N1–N20 (`docs/rules.md`). |
| Independence declaration          | Fresh independent reviewer session (R4, norm N9); the reviewer took no part in authoring or executing the S4b follow-up change and had no prior context from the authoring agent. Every claim below is read directly from the working tree at `57974d2` + uncommitted changes, or reproduced first-hand. |
| Norms applied                     | N1–N20 (see `docs/rules.md`)           |

**Point-in-time counts (audit D1).** The counts recorded in this report (e.g.
`test_link_ip` `checks=77`) are the values the reviewer measured at review time,
before the `-Wall -Wextra -Werror` corrective delta added the IP send-leg and
PUS-TM-sink assertions; the current values are `test_link_ip` 20 funcs / **79**
checks and `test_time` 99 checks. A review report is an immutable record of the
state it examined and is deliberately not retro-edited; the current counts are
carried by `docs/vv/verification-results.md` (rev 0.45+) and by the delta
re-review `FRICK-REVIEW-0168-D1`.

## 1. Scope

This review covers the documentation of the **uncommitted "S4b follow-up" change**
(working tree vs `57974d2`; 12 modified files). The change is the N19
conformance-audit closure of NC-S4b-01..05 plus two new host tests
(`test_pus_ip_roundtrip`, `test_pus_tsn_roundtrip`) providing the `Dem` method
evidence for SRS-REQ-060/061. The review checks that the change is internally
consistent **and** that the delivered SRS-REQ-059/060/061 scope is reflected
consistently across the whole documentation set (norm N8), with correct headers,
histories (N16) and counts.

**First-hand gates re-run** (host, 2026-10-13):

- `make build` / `make test` — PASS; `ctest` **27/27**.
- `build/native/tests/test_link_ip` → `checks=77 failures=0` (20 test functions).
- `build/native/tests/test_link_tsn` → `checks=135 failures=0` (15 test functions).
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` — PASS
  (114 reqs / 71 srs / 29 sss / 14 gap / 51 theorems).
- `make coverage` — 5618/7877 lines, 682/702 functions (matches the README panel).

**Items verified as correct (no finding):**

- `tests/unit/test_link_ip.c` and `test_link_tsn.c` add `test_pus_ip_roundtrip`
  (line 501) and `test_pus_tsn_roundtrip` (line 449); both carry SPDX headers;
  counts 20/77 and 15/135 are exact.
- `docs/requirements/traceability.yaml` SRS-REQ-059 `Verfied`→`Verified` fixed;
  SRS-REQ-059/060/061 `lean_note` records the `Dem` evidence; `traceability.md`
  rev 0.38 mirror row now lists `test_link_ip.c` only for SRS-REQ-059 (NC-S4b-04).
- `docs/quality/class-a-gaps.md` rev 0.11 GAP-A5 enumerates the IP/TSN on-target
  residual (NC-S4b-01), consistent with `traceability.yaml`/`verification-results.md`.
- `docs/vv/verification-results.md` rev 0.43: S4b row counts corrected
  (19 funcs/58 checks, 14 funcs/122 checks), the new S4b-follow-up row added
  (20/77, 15/135), and the "Remaining (M4)" list reconciled (NC-S4b-03/05).
- `docs/status/loop-audit.md` rev 0.25 §1 M4 row reconciled with the delivered
  S3/S4/S5/S7 + S4b state (NC-S4b-05).
- `docs/planning/sprint-plan.md` header 0.17 = History 0.17; rev 0.13→0.15→0.16→0.17.
- `CHANGELOG.md` follow-up entry is accurate; `status.json` is valid JSON and its
  `note` is accurate.
- Header/History currency is **correct** for the changed docs: `verification-results.md`
  0.43=0.43, `traceability.md` 0.38=0.38, `loop-audit.md` 0.25=0.25, `sprint-plan.md`
  0.17=0.17, `class-a-gaps.md` 0.11=0.11, `tool-qualification.md` 0.3.25=0.3.25,
  `SRS.md` 1.23=1.23.

**Excluded from this review (other profiles / not available):** source-code
algorithmic correctness and V&V methods (R1/R2); PA records and evidence
retention (N20, PA); Lean proof validity; the Renode/Lean tool environments
(not available locally — no Renode/Lean execution is claimed); on-target
behaviour (GAP-A5).

## 2. References

- Project norms: `docs/rules.md` (N1–N20), in particular N8 (current
  documentation), N16 (revision history), N17 (documentation review), N2 (English).
- `docs/reviews/review-template.md`, `docs/engineering/independent-review.md`.
- `docs/requirements/traceability.yaml`, `docs/requirements/traceability.md`,
  `docs/requirements/SRS.md`, `docs/requirements/SSS.md`.
- Prior related reviews: `docs/reviews/2026-10-13-s4b-integration-manual-r4-review.md`
  (FRICK-REVIEW-0164), `2026-10-13-s4b-r1-r2-review.md` (FRICK-REVIEW-0165) +
  delta, `2026-10-13-s4b-pa-review.md` (FRICK-REVIEW-0166) + delta.
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| 1 | Minor | `docs/engineering/software-description.md` (FRICK-ENG-0007, rev 0.5) — SRS-REQ-059/060/061 | The DDJF still declares PUS-C over IP/TSN **not implemented**: §"not a TCP/IP stack" states "**PUS-C over IP is not implemented end-to-end**"; §3.7 states "**PUS-C over IP is planned, not implemented**: no `FF_Link_t` TCP/IP/TSN backend, no segmentation/reassembly"; §3.9 lists `FF_Link_t` as "**Implemented (loopback + UART only)**" with "TCP/IP/SpaceWire/radio planned". This contradicts the delivered S4b link backends and seg/reassembly layer. | `software-description.md:85`; `:230-232`; `:271` (rev 0.5, dated 2026-10-12 — not touched by the change). | Update the DDJF (bump rev + History): mark seg/reassembly, IP and TSN link backends implemented; align with `user-integration-manual.md` §7/§8/§9. |
| 2 | Minor | `docs/engineering/software-description.md` §3.9 — SRS-REQ-031/032 (PCIe/SpaceWire) | The interface-contract table lists `FF_Pcie_t` and SpaceWire as "**Interface-only** (planned)", but S7 delivered PCIe peripheral-bound (`hal_pcie=PASS`) and SpaceWire contract-level (`hal_spacewire=CONTRACT`). | `software-description.md:281-282`; `docs/vv/verification-results.md:457` (M11/S7 row); `docs/status/loop-audit.md` M4 row. | Update the status column to reflect the delivered S7 peripheral-bound/contract-level checks. |
| 3 | Minor | `docs/architecture/network-stack.md` (FRICK-ARCH-0012, rev 0.6) §6 | §6 "Status — PUS-C over IP and TSN" states "**PUS-C over IP is not implemented end-to-end in this revision**" and that the TCP/IP and TSN `FF_Link_t` backends and the PUS-C-over-IP segmentation/reassembly "**are planned**"; it also says the ICD has no allocation yet. Contradicts the delivered S4b scope. | `network-stack.md:69-73` (rev 0.6, dated 2026-10-12). | Rewrite §6 to state the backends/segmentation are implemented, cite `test_link_ip`/`test_link_tsn` as evidence, and record the open on-target residual (GAP-A5); bump rev + History. |
| 4 | Minor | `README.md` "Project scope" matrix + "Planned extensions" | The scope matrix lists Transport "Implemented now" as "link (loopback/UART)" (omits the delivered IP/TSN link backends); the Drivers row still lists "**PCIe, SpaceWire, storage** (M4/M11)" under **Planned** although S5/S7 delivered the MSS block device, PCIe and SpaceWire; "Planned extensions" still lists "**TCP/IP + TSN transport over PUS-C (M4)**". | `README.md:136`, `:138`, `:164-166`. | Update the matrix (Transport + Drivers) and the "Planned extensions" paragraph to the delivered S4b/S5/S7 state. |
| 5 | Minor | `docs/development-plan.md` (FRICK-PLAN-0001, rev 0.25) §6 — S5/S7 | The objective-to-milestone table is stale: the M4 row ends "**eMMC/storage/PCIe/SpaceWire pending**"; the M11 row says "in progress (**10/13** peripheral-bound) … **Pending: PCIE**". Both contradict the delivered S5 block device and S7 PCIe peripheral-bound (`hal_pcie=PASS`, M11 11/13) + SpaceWire contract-level. | `development-plan.md:317`, `:324` (rev 0.25, dated 2026-10-12). | Reconcile §6 with the same delivered S3/S4/S5/S7 state used for NC-S4b-05; bump rev + History. |
| 6 | Minor | `docs/status/handover.md` (FRICK-HANDOVER-001) — N16 | The header `Revision` is `1.20` but the `## History` table's last row is rev `1.21`; and no History row records the S4b follow-up. | `handover.md:7` ("Revision 1.20") vs `:259` (`\| 1.21 …`). | Set the header to the History-last revision and add an S4b-follow-up History row in the same change. |
| 7 | Minor | `docs/status/handover.md` line 130 — N8/N2 | The resume paragraph still reports stale counts "test_link_ip **19** test functions, test_link_tsn **14** test functions" (now 20/15 after the follow-up) and contains **literal `\n` escape sequences** instead of line breaks (the text was inserted with escaped newlines). | `handover.md:130` (quoted in full above, including `…\n  reassembly layer…`). | Correct the counts to 20/15 and replace the literal `\n` with a proper bullet list; this is a render/usability defect. |
| 8 | Minor | `docs/engineering/user-integration-manual.md` (FRICK-ENG-0005) §8 + header — N8/N16 | §8 status table still reports "(**19 test functions: 10 seg + 9 IP**)" and "(**14 test functions**)", not updated after the follow-up added `test_pus_ip_roundtrip` (`test_link_ip` → 20 funcs) and `test_pus_tsn_roundtrip` (`test_link_tsn` → 15 funcs); the header `Revision` is `0.8` while the `## History` last row is `0.9`. | `user-integration-manual.md:203-204`; header `:7` ("Revision 0.8") vs History `:419` (`\| 0.9 …`). | Update the §8 counts to 20/15 and align the header with the History; the manual references the changed test files, so N8 requires it in the same change. |
| 9 | Comment | `docs/status/status.json` gates + README semaphores | `documentation_review`, `product_assurance_review` and `reviews` are `"green"` while the S4b follow-up's own N17 (R4, this review) and N20 (PA) reviews are still open; only the `note` was updated. The README semaphore panel mirrors this. | `status.json:35-39`; `README.md` Semaphores table. | Hold the review semaphores at the pre-review state (or mark them pending) until the follow-up's review loop closes, then set green. |
| 10 | Comment | `docs/planning/sprint-plan.md` (FRICK-PLAN-0002) History — N16 | The History table jumps from rev `0.13` to rev `0.15`; rev `0.14` has no row (pre-existing gap). The follow-up bumped the header 0.15→0.17 and added rev 0.17 but did not restore the missing 0.14 entry. | `sprint-plan.md:129` (`\| 0.13 …`) → `:130` (`\| 0.15 …`). | Add the missing 0.14 row (or renumber) so the revision sequence is continuous per N16. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

No Major finding is raised in this review. The one Major known to the change
(NC-S4b-02) is closed **within** the change (host `pus_ip`/`pus_tsn` round-trip
tests added; on-target residual recorded as GAP-A5).

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**APPROVED WITH COMMENTS.** The S4b follow-up change is internally consistent and
its measured evidence is reproducible (ctest 27/27; `test_link_ip` checks=77,
`test_link_tsn` checks=135; trace PASS 114/71/29/14/51; coverage 5618/7877 lines,
682/702 functions). The NC-S4b-01..05 closures and the `Dem` evidence for
SRS-REQ-060/061 are correct and correctly recorded in the changed documents.

However, the change was scoped to a subset of the affected documentation set, so
**eight Minor non-conformities (DOC-R4-023..030) and two Comments
(DOC-R4-031..032)** remain open: several documents **not touched** by the change
still describe the delivered S4b/S7 scope as "planned/not implemented"
(`software-description.md`, `network-stack.md`, `README.md`,
`development-plan.md`), and three documents have header/History or count currency
defects (`handover.md`, `user-integration-manual.md`, `sprint-plan.md`). These do
not invalidate the delivered requirements but must be corrected to close the N17
documentation loop (N8/N16). Re-review of the corrected documents is required
before commit/push.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R4 (N9/N17)   | Senior SW documentation engineer (independent context) | 2026-10-13 | APPROVED WITH COMMENTS (DOC-R4-023..030 Minor; DOC-R4-031..032 Comment — tracked to closure) |

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| DOC-R4-023 | Minor | `docs/engineering/software-description.md` (FRICK-ENG-0007, rev 0.5) | PUS-C over IP/TSN still described as "not implemented / planned"; `FF_Link_t` "loopback + UART only" (contradicts SRS-REQ-059/060/061 delivery) | Open — update the DDJF, bump rev + History | pending | frick-fsw authoring agent | **closed** (authoring correction; see the closure record below) | pending delta re-review |
| DOC-R4-024 | Minor | `docs/engineering/software-description.md` §3.9 | PCIe/SpaceWire listed "Interface-only (planned)" (contradicts S7 delivery) | Open — update the status column | pending | frick-fsw authoring agent | **closed** (authoring correction; see the closure record below) | pending delta re-review |
| DOC-R4-025 | Minor | `docs/architecture/network-stack.md` (FRICK-ARCH-0012, rev 0.6) §6 | "PUS-C over IP is not implemented end-to-end"; backends "planned" (contradicts S4b) | Open — rewrite §6, bump rev + History | pending | frick-fsw authoring agent | **closed** (authoring correction; see the closure record below) | pending delta re-review |
| DOC-R4-026 | Minor | `README.md` "Project scope" matrix + "Planned extensions" | Transport row omits IP/TSN; Drivers row lists PCIe/SpaceWire/storage as Planned; "TCP/IP + TSN transport over PUS-C (M4)" still planned | Open — update the matrix + planned-extensions paragraph | pending | frick-fsw authoring agent | **closed** (authoring correction; see the closure record below) | pending delta re-review |
| DOC-R4-027 | Minor | `docs/development-plan.md` (FRICK-PLAN-0001, rev 0.25) §6 | M4 row "eMMC/storage/PCIe/SpaceWire pending"; M11 row "10/13 … Pending: PCIE" (contradicts S5/S7) | Open — reconcile §6, bump rev + History | pending | frick-fsw authoring agent | **closed** (authoring correction; see the closure record below) | pending delta re-review |
| DOC-R4-028 | Minor | `docs/status/handover.md` (FRICK-HANDOVER-001) | Header `Revision 1.20` vs History last `1.21`; no S4b-follow-up History row (N16) | Open — align header + add History row | pending | frick-fsw authoring agent | **closed** (authoring correction; see the closure record below) | pending delta re-review |
| DOC-R4-029 | Minor | `docs/status/handover.md` line 130 (N8/N2) | Stale counts (19/14 vs 20/15) and literal `\n` escape sequences | Open — correct counts, render as a proper list | pending | frick-fsw authoring agent | **closed** (authoring correction; see the closure record below) | pending delta re-review |
| DOC-R4-030 | Minor | `docs/engineering/user-integration-manual.md` (FRICK-ENG-0005) §8 + header | §8 counts "(19: 10 seg + 9 IP)" / "(14)" stale (now 20/15); header `0.8` vs History `0.9` | Open — update counts + align header | pending | frick-fsw authoring agent | **closed** (authoring correction; see the closure record below) | pending delta re-review |
| DOC-R4-031 | Comment | `docs/status/status.json` gates + README semaphores | `documentation_review`/`product_assurance_review`/`reviews` green while the follow-up N17/N20 reviews are open | Open — reflect the pending review state until closure | pending | frick-fsw authoring agent | **closed** (authoring correction; see the closure record below) | pending delta re-review |
| DOC-R4-032 | Comment | `docs/planning/sprint-plan.md` (FRICK-PLAN-0002) History | Missing rev 0.14 row (0.13 → 0.15) (N16) | Open — add the 0.14 row or renumber | pending | frick-fsw authoring agent | **closed** (authoring correction; see the closure record below) | pending delta re-review |

### 6.1 Closure record (authoring agent, 2026-10-13)

| NC | Corrective action | Re-verification |
| -- | ----------------- | --------------- |
| DOC-R4-023 | `software-description.md` — the "PUS-C over IP not implemented / planned" statements and the `FF_Link_t` "loopback + UART only" status updated to the delivered state (seg + IP + TSN backends, host round-trip evidence); rev + History advanced | doc diff + delivered-artifact cross-check |
| DOC-R4-024 | `software-description.md` §3.9 — PCIe = peripheral-bound PASS, SpaceWire = contract-level (both S7) | doc diff vs S7 verification records |
| DOC-R4-025 | `network-stack.md` §6 rewritten to the delivered state with the GAP-A5 residual; rev + History advanced | doc diff |
| DOC-R4-026 | `README.md` scope matrix + "Planned extensions" reconciled; the generated metrics panel was left untouched and regenerated only by `make metrics` | `tools/scripts/metrics.py --check` = current |
| DOC-R4-027 | `development-plan.md` §6 M4/M11 rows reconciled (S5 block device, S7 PCIe, M11 11/13 peripheral-bound) | doc diff |
| DOC-R4-028 | `handover.md` header aligned to the History last revision + an S4b-follow-up History row added | doc diff |
| DOC-R4-029 | `handover.md` resume paragraph: counts corrected (20/15) and the literal `\n` escapes rendered as a Markdown list | doc diff |
| DOC-R4-030 | `user-integration-manual.md` §8 rows corrected (20 / 15 test functions) and the header aligned with History | doc diff + `test_link_ip`/`test_link_tsn` run |
| DOC-R4-031 | `status.json` — new `meta.semaphore_semantics` documents the gate meaning (non-runnable gates keep the last REAL run; review gates set green only after the loop closes) | `status.json` parses; metrics gate green |
| DOC-R4-032 | `sprint-plan.md` — the `0.14` row (S8/M8 delivered) **restored**: commit `e65fc1b` had overwritten it instead of appending `0.15` | comparison against `git show 135f0eb:docs/planning/sprint-plan.md` |

---

Committed and pushed with the reviewed change (norm N11). The authoring agent
must also append the FRICK-REVIEW-0168 row to the log in
`docs/reviews/README.md` (this R4 report is report-only and does not edit that
file).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-13 | independent R4 agent | Initial S4b-follow-up documentation review (FRICK-REVIEW-0168): APPROVED WITH COMMENTS; DOC-R4-023..030 (Minor) + DOC-R4-031..032 (Comment) opened |
