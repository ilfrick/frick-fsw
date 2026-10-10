# Review Report — Milestone M6 (mathematics library / libm integration)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0020                      |
| Reviewer session                  | (independent agent, norm N9)           |
| Reviewer profile                  | R3 (Senior Systems Engineer, N14) **and** R4 (Senior SW documentation reviewer, N17) |
| Reference revision                | base commit `17d9410` + uncommitted M6 working tree (`git status` in `/home/nicola/fsw/frick-fsw`) |
| Items under review                | `docs/requirements/SSS.md` (SSS-REQ-018), `docs/requirements/SRS.md` (SRS-REQ-033), `docs/requirements/traceability.{yaml,md}`; `docs/adr/ADR-0012-libm-math.md`; `docs/architecture/numerics.md`; `docs/quality/licensing.md`, `docs/quality/ecss-compliance-matrix.md`; `docs/development-plan.md`, `docs/roadmap.md`, `README.md`, `CHANGELOG.md`, `docs/tooling.md`, `docs/status/status.json`; `.github/workflows/ci.yml`, `.gitea/workflows/ci.yml`; `docs/vv/verification-results.md`; plus the M6 artefacts `src/math/ff_math.{c,h}`, `tests/unit/test_math.c`, `CMakeLists.txt`, `tests/CMakeLists.txt` |
| Date                              | 2026-10-10                             |
| Scope                             | Completeness/correctness of the M6 documentation and requirements delta, against ECSS-E-ST-40C, ECSS-Q-ST-80C and norms N1–N17 |
| Independence declaration          | Reviewer took no part in authoring, implementing or executing the M6 items; fresh context; every claim below is derived from the repository contents and the cited tool runs |
| Norms applied                     | N1–N17 (see `docs/rules.md`)           |

## 1. Scope

Two independent profiles were applied to the M6 (libm mathematics facade) delta:

- **R3 (Senior Systems Engineer, norm N14):** review of the system-level
  requirement `SSS-REQ-018` and its single child `SRS-REQ-033` — requirement
  quality (complete, unambiguous, verifiable, feasibility at criticality B),
  consistency with the mission/operational concept, interface and ICD fit, and
  traceability.
- **R4 (Senior SW documentation reviewer, norm N17):** completeness and
  correctness of the documentation set touched by the change — requirement
  counts, coverage figures, revision/history coherence, document-id uniqueness,
  link resolution, README metrics panel, SPDX/licence scoping, and internal
  cross-document consistency.

Included in the review: the exact working-tree delta (16 modified + 4 untracked
paths listed above). Excluded: hardware-dependent behaviour and the M4 driver
work (documented class-A gaps), and code changes outside the M6 delta.

Tools executed by the reviewer (not modified):

```text
python3 tools/scripts/trace.py docs/requirements/traceability.yaml   -> PASS (exit 0)
python3 tools/scripts/metrics.py                                     -> matches README panel
document-id uniqueness scan (all docs/**/*.md)                       -> 54 declarations, 0 duplicates
markdown relative-link scan                                          -> 45 links checked, 0 broken
revision-header vs History scan (all docs with a History table)      -> 2 mismatches (R4-1, R4-2)
```

`trace.py` output: `65 requirements, 34 srs, 18 sss, 13 gaps, 45 lean
theorems checked`; `CI evidence claims: 9`. This is consistent with every count
stated in `traceability.md`, `README.md`, the README metrics panel and the M6
row of `verification-results.md`.

## 2. References

- Project norms: `docs/rules.md` (N1–N17); reviewer profiles:
  `docs/engineering/independent-review.md`.
- Traceability manifest and gate: `docs/requirements/traceability.yaml`,
  `tools/scripts/trace.py`.
- Metrics/coverage: `tools/scripts/metrics.py`, `tools/scripts/coverage.py`,
  `build/coverage/coverage.json`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R3-1 | Minor | SSS-REQ-018 / SRS-REQ-033 | **No requirement-level numerical accuracy is stated.** The system requirement asks for a "portable numerical/mathematics interface" and the SRS asks for named functions plus out-of-domain behaviour, but neither defines an accuracy/precision expectation (e.g. inherited platform-`libm` accuracy or a ULP bound). Verification is therefore only tolerance-based with test-chosen epsilons, which cannot be traced to a required value. For an interface advertised for attitude/geometry/transforms this is a requirements-completeness gap. | `docs/requirements/SSS.md:80-83`; `docs/requirements/SRS.md:150-154`; `tests/unit/test_math.c:25-47` (eps `1e-9`…`1e-12`); `docs/architecture/numerics.md:57-63` | Add a precision requirement (e.g. "results within the accuracy of the platform `libm` for the active toolchain" or an explicit bound) or record a justified waiver; tie the test tolerance to it. |
| R3-2 | Minor | SRS-REQ-033 | **Reentrancy/thread-safety and error-signalling contract are not documented.** The facade may set `errno` and returns `NaN`/`±inf`; the policy covers finiteness checks and real-time/WCET limits (GAP-A4) but is silent on whether the functions are reentrant/ISR-safe and on the `errno` semantics expected by callers. A user-facing numerical interface on a flight platform should state this contract. | `src/math/ff_math.h:31-51`; `src/math/ff_math.c:77-81`; `docs/architecture/numerics.md:34-49`; `docs/adr/ADR-0012-libm-math.md:32-37` | Add one sentence to `numerics.md` (and, if normative, to SRS-REQ-033) stating the reentrancy/ISR suitability and the `errno`/`NaN` propagation contract, or reference the platform `libm` contract. |
| R3-3 | Comment | SRS-REQ-033 | The elementary-function set is enumerated as a closed list (`sqrt/sin/cos/tan/atan2/pow/exp/log/fabs/fmod`). Any later addition (e.g. `asin/acos/hyperbolic`) requires a requirement change; there is no extension clause. | `docs/requirements/SRS.md:150-154`; `src/math/ff_math.h:34-43` | Consider wording "shall expose at least …" with a documented extension policy, or record that the list is intentionally fixed. |
| R3-4 | Comment | SSS-REQ-018 ↔ SSS-REQ-010 | The determinism boundary is handled well: `SSS-REQ-018` requires the *implications* to be documented while `SSS-REQ-010` keeps the core allocation-free, and `numerics.md` confines floating point to applications. No action; recorded as a positive conformance observation. | `docs/requirements/SSS.md:55-56,80-83`; `docs/architecture/numerics.md:19-26,38-49`; `docs/adr/ADR-0012-libm-math.md:22` | None. |
| R4-1 | Minor | `docs/development-plan.md` (norm N16) | **Revision header does not match its own History table.** Front matter is `Revision 0.1` while the History table now has rows through `0.3`; the M6 change added the `0.3` row ("Add M6 … objective OBJ-13") but did not bump the header (the `0.2` row was already stale). | `docs/development-plan.md:7` (`0.1`) vs `docs/development-plan.md:190-193` (rows `0.1`,`0.2`,`0.3`); `git diff -- docs/development-plan.md` | Bump the header to `0.3`. |
| R4-2 | Minor | `docs/vv/verification-results.md` (norm N16) | **Revision header does not match its own History table.** Front matter is `Revision 0.3` while the History table now has rows through `0.5`; the M6 change added the `0.5` row but did not bump the header (`0.4` was already stale). | `docs/vv/verification-results.md:7` (`0.3`) vs `docs/vv/verification-results.md:241-246` (rows `0.1`…`0.5`); `git diff -- docs/vv/verification-results.md` | Bump the header to `0.5`. |
| R4-3 | Minor | `README.md` vs N13 / `docs/quality/licensing.md` (norm N13/N8) | **SPDX statement is incoherent with the scoping established by N13 and re-affirmed by this M6 change.** N13 scopes the SPDX header to "source, configuration and build files" and states engineering documentation is covered by `LICENSE`/`NOTICE`; `licensing.md` §1 (M6) now says the same. `README.md` still claims "Every authored file carries the copyright header and `SPDX-License-Identifier: AGPL-3.0-only`", which is false for the 62/67 `docs/**/*.md` files that carry no SPDX header. | `README.md:324`; `docs/rules.md:188-192`; `docs/quality/licensing.md:21-28`; scan: 5 of 67 `docs/**/*.md` files contain `SPDX-License-Identifier` | Align the README sentence with the N13 scope (e.g. "every authored source/config/build file"), consistent with `CHANGELOG.md:80-83`. |
| R4-4 | Comment | `docs/development-plan.md` §4, `docs/roadmap.md` | **Milestone ordering is non-monotonic.** The M6 section is placed *before* M5 in both documents, while §6 of the development plan and `status.json` order M5 before M6. Readability/consistency only. | `docs/development-plan.md:144-155` (M6 then M5); `docs/roadmap.md:84-102` (M6 then M5); `docs/development-plan.md:178-185`; `docs/status/status.json:8-15` | Move the M6 section after M5 (or renumber), so all lists agree. |
| R4-5 | Comment | M6-touched docs | Five files finished without a trailing newline (`licensing.md`, `traceability.md`, `traceability.yaml`, `tooling.md`, `verification-results.md`). The same class was previously raised by an R4 review for `mapping-cfs.md`; it is cosmetic but easy to fix. | `tail -c1` on each file; `git diff` "`\ No newline at end of file`" markers | Add a trailing newline to each. |
| R4-6 | Comment | `docs/adr/ADR-0012-libm-math.md` (norm N2) | Inconsistent spelling of the borrowed word: the ADR uses "façade" while the code/header/numerics note use "facade". English is otherwise coherent throughout the delta. | `docs/adr/ADR-0012-libm-math.md:28` ("façade") vs `src/math/ff_math.h:2,13`; `docs/architecture/numerics.md` | Standardise on "facade". |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### Conformance verified (no finding)

These items were checked and found complete and correct; they are recorded so
the review is reproducible.

- **Traceability gate:** `trace.py` exits 0 — `65 requirements, 34 srs,
  18 sss, 13 gaps, 45 lean theorems`, `9 CI evidence claims`; hierarchy shows
  `SSS-REQ-018 <- 1 srs children`.
- **SSS-REQ-018 decomposition:** `SSS-REQ-018` → `SRS-REQ-033` present in the
  manifest (`parent_sss: SSS-REQ-018`), the bottom-up matrix
  (`traceability.md:43`), §2/§3 tables and the manifests' `lean_note`.
- **Counts:** `README.md` prose (34/18/13/45), README metrics panel
  (65 = srs 34, sss 18, gap 13), `traceability.md:18-20`,
  `verification-results.md:25` and the M6 notes are mutually consistent.
- **README metrics panel vs `metrics.py`:** byte-for-byte identical when
  `python3 tools/scripts/metrics.py` is run.
- **Coverage:** `build/coverage/coverage.json` = `lines 932/1575`,
  `funcs 141/184`; identical to the README panel and to
  `verification-results.md:204`.
- **Document ids:** 54 `Document id` declarations across `docs/**/*.md`,
  **no duplicates**; the CI uniqueness scanner in both workflows would pass.
- **Links:** 45 relative markdown links checked, none broken; `numerics.md` is
  linked from `README.md:275` and required by **both** CI workflows
  (`.github/workflows/ci.yml:138`, `.gitea/workflows/ci.yml:179`; the two
  required-document lists are identical, 27 entries each).
- **SPDX enforcement:** the `docs` CI licence job (AGPL-3.0 text + no
  `Apache-2.0` SPDX headers) is unchanged and consistent with N13.
- **Language (N2):** only intentional non-ASCII characters (em dash, middle
  dot, arrows, `Eärendil`/`façade` accents, section sign, emoji); content is
  English.
- **Licence analysis:** the new `libm` row in `licensing.md:43` is compatible
  with the AGPL-3.0-only project conclusion and with ADR-0012; no new
  third-party component is distributed.

## 4. Conclusion

### R3 — Senior Systems Engineer (N14)

- [ ] APPROVED — no findings or Comments only
- [x] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

`SSS-REQ-018` and its child `SRS-REQ-033` are complete, unambiguous, verifiable
and correctly traced; the determinism and licensing implications are coherent
and the feasibility concerns are not hidden (WCET/stack → GAP-A4, FP context →
GAP-A5). Two Minor findings (R3-1 accuracy requirement, R3-2 reentrancy/errno
contract) require correction or an explicit justification; no Major finding.

### R4 — Senior SW documentation reviewer (N17)

- [ ] APPROVED — no findings or Comments only
- [x] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

All quantitative claims in the M6 delta are derivable and mutually consistent
(requirements, theorems, GAPs, CI claims, coverage, review gate), the trace
gate passes, document ids are unique and links resolve. Three Minor findings
(R4-1/R4-2 stale revision headers vs History, R4-3 over-broad README SPDX
claim) must be corrected; the remaining items are Comments. No Major finding.
The review loop (norm N12/N17) is not closed until the Minor findings are
dispositioned and the delta is re-reviewed.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R3 (N9/N14)   | independent agent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS (R3-1, R3-2 Minor; R3-3, R3-4 Comment) |
| R4 (N9/N17)   | independent agent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS (R4-1..R4-3 Minor; R4-4..R4-6 Comment) |

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R3-1 | Minor | SSS-REQ-018 / SRS-REQ-033 | no stated numerical accuracy | (to be dispositioned by author) | | frick-fsw | open | required |
| R3-2 | Minor | SRS-REQ-033 / numerics.md | reentrancy/errno contract undocumented | (to be dispositioned by author) | | frick-fsw | open | required |
| R3-3 | Comment | SRS-REQ-033 | closed function list | (to be dispositioned by author) | | frick-fsw | open | not required |
| R3-4 | Comment | SSS-REQ-018 / SSS-REQ-010 | determinism boundary (positive) | none | as-reviewed | frick-fsw | closed | not required |
| R4-1 | Minor | docs/development-plan.md | header 0.1 vs History 0.3 (N16) | (to be dispositioned by author) | | frick-fsw | open | required |
| R4-2 | Minor | docs/vv/verification-results.md | header 0.3 vs History 0.5 (N16) | (to be dispositioned by author) | | frick-fsw | open | required |
| R4-3 | Minor | README.md vs N13/licensing.md | over-broad SPDX claim | (to be dispositioned by author) | | frick-fsw | open | required |
| R4-4 | Comment | development-plan.md, roadmap.md | non-monotonic M6/M5 ordering | (to be dispositioned by author) | | frick-fsw | open | not required |
| R4-5 | Comment | 5 M6 docs | missing trailing newline | (to be dispositioned by author) | | frick-fsw | open | not required |
| R4-6 | Comment | ADR-0012 | facade/façade spelling | (to be dispositioned by author) | | frick-fsw | open | not required |

---

Committed and pushed with the reviewed change (norm N11).
