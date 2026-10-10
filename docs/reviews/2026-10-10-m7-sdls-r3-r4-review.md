# Review Report — Milestone M7 (CCSDS SDLS security / key management)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0022                      |
| Reviewer session                  | (independent agent, norm N9)           |
| Reviewer profile                  | R3 (Senior Systems Engineer, N14) **and** R4 (Senior SW documentation reviewer, N17) |
| Reference revision                | base commit `3096241` + uncommitted M7 working tree (`git status` in `/home/nicola/fsw/frick-fsw`) |
| Items under review                | `docs/requirements/SSS.md` (SSS-REQ-019), `docs/requirements/SRS.md` (SRS-REQ-034/-035), `docs/requirements/traceability.{yaml,md}`; `docs/adr/ADR-0013-sdls.md`; `docs/architecture/security-sdls.md`; `docs/quality/{licensing,class-a-gaps,ecss-compliance-matrix,ecss-criticality-b}.md` (GAP-A14); `docs/development-plan.md` (M7, OBJ-14); `docs/roadmap.md` (M7); `README.md`; `CHANGELOG.md`; `docs/status/status.json` (M7); `docs/vv/verification-results.md` (M7 row/notes, coverage 1192/1925); `.github/workflows/ci.yml`, `.gitea/workflows/ci.yml`; plus the M7 artefacts `src/security/ff_sdls.{c,h}`, `src/security/ff_crypto_test.{c,h}`, `tests/unit/test_sdls.c`, `verification/lean/Frick/Sdls.lean` |
| Date                              | 2026-10-10                             |
| Scope                             | Completeness/correctness of the M7 documentation and requirements delta, against ECSS-E-ST-40C, ECSS-Q-ST-80C, CCSDS 355.0-B / 352.0-B (SDLS) and norms N1–N17 |
| Independence declaration          | Reviewer took no part in authoring, implementing or executing the M7 items; fresh context; every claim below is derived from the repository contents and the cited tool runs |
| Norms applied                     | N1–N17 (see `docs/rules.md`)           |

> Report document id `FRICK-REVIEW-0022` is the next free number in the
> `FRICK-REVIEW-00nn` namespace after `0021`
> (`docs/reviews/2026-10-10-m6-math-delta-review.md`); the id-uniqueness scan
> confirms it is unused.

## 1. Scope

Two independent profiles were applied to the M7 (CCSDS SDLS security
sub-layer) delta:

- **R3 (Senior Systems Engineer, norm N14):** review of the system-level
  requirement `SSS-REQ-019` and its two children `SRS-REQ-034` and
  `SRS-REQ-035` — requirement quality (complete, unambiguous, verifiable,
  feasible at criticality B), the implemented / pluggable / class-A-gap
  boundary, consistency with the CCSDS and ECSS references, and traceability.
- **R4 (Senior SW documentation reviewer, norm N17):** completeness and
  correctness of the documentation set touched by the change — requirement
  counts, coverage figures, revision/history coherence, document-id uniqueness,
  link resolution, README metrics panel, SPDX/licence scoping, English (N2),
  and internal cross-document consistency.

Included in the review: the exact working-tree delta (21 modified + 5 untracked
paths listed above). Excluded: hardware-dependent behaviour and the M4 driver
work (documented class-A gaps), and deep code correctness (that is the R1/R2
review); only the documentation/requirement-facing consequences of the code are
assessed here.

Tools executed by the reviewer (no repository file was modified):

```text
python3 tools/scripts/trace.py docs/requirements/traceability.yaml   -> PASS (exit 0)
python3 tools/scripts/metrics.py                                     -> matches README panel byte-for-byte
document-id uniqueness scan (all docs/**/*.md)                       -> 58 declarations, 0 duplicates
markdown local-link scan                                             -> 46 relative links checked, 0 broken
in-page anchor scan (README ToC)                                     -> 14 anchors, all resolve
revision-header vs History scan (docs with a History table)          -> numeric match; 1 unrecorded change (R4-1)
check-suite/blob scan (test_sdls count, Lean log, coverage.json)     -> consistent
```

`trace.py` output: `69 requirements, 36 srs, 19 sss, 14 gaps, 51 lean
theorems checked`; `CI evidence claims: 9`. This is consistent with every count
stated in `traceability.md`, `README.md`, the README metrics panel, the M7 row
of `verification-results.md` and `status.json`.

## 2. References

- Project norms: `docs/rules.md` (N1–N17); reviewer profiles:
  `docs/engineering/independent-review.md`.
- Traceability manifest and gate: `docs/requirements/traceability.yaml`,
  `tools/scripts/trace.py`.
- Metrics/coverage: `tools/scripts/metrics.py`, `build/coverage/coverage.json`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2,
  CCSDS 355.0-B (SDLS), CCSDS 352.0-B (SDLS extended procedures / algorithms).
- Decision records: `docs/adr/ADR-0013-sdls.md`; architecture:
  `docs/architecture/security-sdls.md`.

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R3-1 | Comment | SSS-REQ-019 / SRS-REQ-034 | **The applicable-standards front matter was not extended with the SDLS standards the new requirements cite.** `SSS-REQ-019` explicitly invokes *CCSDS 355.0-B (SDLS)*, and the architecture note/ADR additionally rely on *CCSDS 352.0-B* (algorithms), but the `Applicable standards` rows of `SSS.md` and `SRS.md` still list only ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C and CCSDS 133.0-B-2. | `docs/requirements/SSS.md:9,84-88`; `docs/requirements/SRS.md:9,160-170`; `docs/adr/ADR-0013-sdls.md:9-13`; `docs/architecture/security-sdls.md:15` | Add CCSDS 355.0-B (and, where algorithm-level, CCSDS 352.0-B) to the `Applicable standards` of `SSS.md`/`SRS.md` and to the references in `ecss-criticality-b.md` §1 (see also R4-3). |
| R3-2 | Comment | SRS-REQ-035 (zeroisation) | **The zeroisation clause is verified only indirectly.** `SRS-REQ-035` requires "zeroise key material on rotation and removal", but `test_sdls.c` asserts only the registry count and a subsequent `FF_NOT_FOUND`; it never checks that the master/session key bytes are actually zeroed after `FF_SdlsSaRemove`/`FF_SdlsZeroize`. The `Anl`/`Tst` evidence for this clause is therefore weaker than the wording implies. | `docs/requirements/SRS.md:165-170`; `tests/unit/test_sdls.c:156-166` (count + `FF_NOT_FOUND` only); `src/security/ff_sdls.c:168-180,223-228` | Add a test that snapshots the SA memory and asserts the key bytes are zero after removal/zeroisation (or record an explicit analysis `lean_note`); this also strengthens the `Tst` claim in `traceability.md`. |
| R3-3 | Minor | `docs/architecture/security-sdls.md` §4 / SRS-REQ-034 | **The documented IV-uniqueness property is stated unconditionally but the code only provides it for `iv_len >= 4`.** The note claims "the IV is the SA base with a per-frame counter written big-endian, giving IV uniqueness within a session key", but `ff_sdls_build_iv()` writes the per-frame counter only `if (sa->iv_len >= 4u)`, and `FF_SdlsSetBackend()` accepts any `iv_len` from 0 to `FF_SDLS_IV_MAX` (no minimum). A backend configured with `iv_len` in {0,1,2,3} would reuse a constant IV across frames under one session key, contradicting the note (and the SDLS header = SPI+IV expectation of `SRS-REQ-034`). | `docs/architecture/security-sdls.md:53-59`; `src/security/ff_sdls.c:42-54` (`if iv_len >= 4`), `:80-93,110-117` (no lower-bound check); `docs/requirements/SRS.md:160-164` | Either enforce `iv_len >= 4` in `FF_SdlsSetBackend()` (and reject otherwise), or state the assumption explicitly in `security-sdls.md` §5 as a limitation for `FF_SdlsProtect`/the backend contract. (Code-hardening is primarily R1/R2; the documentation inaccuracy is in scope for R4.) |
| R4-1 | Minor | `docs/quality/ecss-criticality-b.md` (norm N16) | **A change was made without a revision bump or `History` entry, leaving header and history inconsistent.** The `Gaps to class A` row was edited from `GAP-A1..A13` to `GAP-A1..A14` and the file is part of the M7 delta, but `Revision` stays `0.3` and the last `History` row is still `0.3` — "ECSS compliance assessment … gaps GAP-A1..A13". Header now says A14 while the history reason says A13. Every other M7 quality doc (`class-a-gaps.md` 0.4, `ecss-compliance-matrix.md` 0.3, `licensing.md` 0.4) correctly carries an M7 `History` row. | `docs/quality/ecss-criticality-b.md:7,10` vs `:81-83`; `git diff -- docs/quality/ecss-criticality-b.md` | Bump to `0.4` and add an M7 row (e.g. "M7: SDLS security; class-A gap reference extended to GAP-A1..A14"). |
| R4-2 | Minor | `docs/adr/ADR-0013-sdls.md`; `src/security/ff_crypto_test.h` (norms N8/N17) | **Documentation overstates where the non-secure reference backend is used.** Both the ADR ("used only by the tests **and the host simulator**") and the header comment ("used only to exercise the SDLS SA/key-management logic in tests **and on the host simulator**") claim host-simulator use. The backend is compiled **only** into `test_sdls`; `ff_crypto_test.c` is not in `frick_fsw_core`, is not referenced from `src/main/` or `src/apps/`, and no code calls `FF_SdlsSetBackend()` outside the tests — so the host simulator does not use it. (`licensing.md` and `security-sdls.md` correctly describe it as "test only".) | `docs/adr/ADR-0013-sdls.md:25-26`; `src/security/ff_crypto_test.h:3-8`; `tests/CMakeLists.txt:21-24` (only `test_sdls`); `grep -rn FF_CryptoTestBackend src/ tests/` → test only; `docs/quality/licensing.md:44`; `docs/architecture/security-sdls.md:30` | Change "and the host simulator" to "test-only" in ADR-0013 and the `ff_crypto_test.h` comment, consistent with `licensing.md`/`security-sdls.md`. |
| R4-3 | Comment | `docs/quality/ecss-criticality-b.md` §1/§3 (norm N8) | **The class-B baseline document is not fully updated for M7.** §1 applicable references omit CCSDS 355.0-B/352.0-B, and the §3 "Engineering data set" table has no Security/SDLS row even though the compliance matrix gained one (`ecss-compliance-matrix.md:29`). The deliverable list is therefore less complete than its sibling matrix for the new module. | `docs/quality/ecss-criticality-b.md:13-21` (§1), `:40-59` (§3); `docs/quality/ecss-compliance-matrix.md:29` | Add a Security (SDLS) row to §3 and the SDLS standards to §1, mirroring the compliance matrix. |
| R4-4 | Comment | `docs/quality/class-a-gaps.md`, `docs/quality/licensing.md` | Both files end without a trailing newline; the M7 edit appended a new last line but kept the no-newline ending (the diff still shows `\ No newline at end of file`). Cosmetic, but previously raised for other files by an R4 review. | `tail -c1 docs/quality/class-a-gaps.md docs/quality/licensing.md`; `git diff` markers | Add a trailing newline to each. |
| R4-5 | Comment | Sibling review artifact `docs/reviews/2026-10-10-m7-sdls-r1-r2-review.md` (norms N8/N16) | **The concurrent M7 R1/R2 report uses a non-`FRICK-REVIEW-00nn` document id** (`FRICK-REVIEW-M7-R1R2`), contrary to the convention asserted in `CHANGELOG.md` and enforced by the prior R4 reviews (M2/M3 findings N-5). The id is unique, so the CI uniqueness scanner passes, but the namespace is no longer uniform. This does not affect the M7 requirement/document deltas reviewed here. | `docs/reviews/2026-10-10-m7-sdls-r1-r2-review.md:5`; `docs/rules.md` (N16); `CHANGELOG.md:91-92,126-128`; id scan (59 unique, 0 duplicates) | Renumber to `FRICK-REVIEW-0023` (next free) and log both M7 reports in `docs/reviews/README.md`; keep the numeric namespace uniform. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### Conformance verified (no finding)

These items were checked and found complete and correct; they are recorded so
the review is reproducible.

- **Traceability gate:** `python3 tools/scripts/trace.py
  docs/requirements/traceability.yaml` → `PASS`, exit 0, with
  `69 requirements (srs=36, sss=19, gap=14)`, `51 lean theorems`, `9 CI
  evidence claims`. The bottom-up hierarchy shows `SSS-REQ-019 <- 2 srs
  children`.
- **SSS-REQ-019 decomposition:** `SSS-REQ-019` → `SRS-REQ-034` + `SRS-REQ-035`
  is present in the manifest (`parent_sss: SSS-REQ-019`,
  `traceability.yaml:326-342`), in the top-down matrix
  (`traceability.md:44`), and in the §2/§3 verification tables
  (`traceability.md:84-85,114`). Requirement ids are unique in `SSS.md`/
  `SRS.md` and the gate validates both directions.
- **Counts (69/36/19/14/51/9) are mutually consistent** across
  `traceability.md:18-20`, `README.md:72,244`, the README metrics panel,
  `verification-results.md:26,175,230`, `status.json` and `trace.py`.
- **README metrics panel vs `metrics.py`:** byte-for-byte identical when
  `python3 tools/scripts/metrics.py` is run; semaphores (`documentation_review`
  etc.) match `docs/status/status.json`.
- **Coverage:** `build/coverage/coverage.json` = `lines 1192/1925`,
  `funcs 172/214`; identical to `README.md:76`, the panel, and
  `verification-results.md:230` (`61.9%`). The M7 ctest claim `8/8; 278 checks`
  is consistent: `15+20+9+29+101+31+20+53 = 278`, and `sdls 53` follows from
  the 46 `FF_TEST_ASSERT` statements plus the 8-iteration capacity loop
  (`46 + 7 = 53`), matching the harness execution model (cf. `hal 31` from 24
  source asserts).
- **Lean evidence:** `verification/lean/lake-build.log` shows
  `[3/21] Built Frick.Sdls` and `Build completed successfully` (21/21), and
  `Frick.Requirements.lean` wraps the three `Frick.Sdls` theorems as
  `req_srs035_sa_bound` / `req_srs035_epoch_monotonic` /
  `req_srs035_uses_reset`.
- **Document ids:** 58 `Document id` declarations across `docs/**/*.md`
  (excluding this report), **no duplicates**; the CI uniqueness scanner (`.github/workflows/ci.yml:149-166`,
  `.gitea/workflows/ci.yml`) would pass, and the new `FRICK-ARCH-0010`
  (`security-sdls.md`) does not collide. ADR-0013 carries `Status`/`Date`
  instead of a `Document id`, which is the documented ADR convention
  (`docs/rules.md:333-339`).
- **Links:** 46 relative markdown links resolve, 0 broken; the 14 README
  table-of-contents anchors all resolve to their headings; `security-sdls.md`
  is linked from `README.md:276`.
- **CI docs required list:** `docs/architecture/security-sdls.md` is present in
  **both** workflows (`.github/workflows/ci.yml:139`,
  `.gitea/workflows/ci.yml:180`); the two required lists remain identical.
- **Implemented / pluggable / GAP-A14 distinction:** correctly and explicitly
  separated in `security-sdls.md` §2 ("implemented, tested (+Lean)" vs
  "**pluggable** (mission/HW)" vs "**GAP-A14**"), in ADR-0013 §Decision/
  Consequences, and in the `class-a-gaps.md`/compliance-matrix/`traceability`
  GAP-A14 justifications. The non-secure reference backend's flight exclusion
  ("MUST NOT be used in flight") is stated in `ff_crypto_test.h` and the
  architecture note.
- **Requirements quality (R3):** `SRS-REQ-034` (SDLS processing with a
  configurable service through a pluggable backend) and `SRS-REQ-035` (bounded
  SPI-indexed SA DB + key lifecycle) are complete, unambiguous, verifiable and
  decompose the system requirement without loss; the "no cipher in the
  middleware" boundary is consistent with ADR-0013 and the licensing analysis.
- **SPDX/licence scoping (N13):** all five new/changed source, header, test and
  Lean files carry the AGPL-3.0-only SPDX header (`ff_sdls.c/h`,
  `ff_crypto_test.c/h`, `test_sdls.c`, `Sdls.lean`); the new documentation
  carries no SPDX header, consistent with the N13 rev 1.1 scoping
  (docs covered by `LICENSE`/`NOTICE`). `licensing.md:44` adds the SDLS backend
  row coherently (mission-supplied, not distributed; reference backend AGPL,
  test-only).
- **English (N2):** the delta is in English; only intentional non-ASCII
  characters (em dash, arrows, middle dot, `Eärendil`).
- **Status/revision coherence:** M7 is `in progress` consistently in
  `development-plan.md` (M7 + OBJ-14), `roadmap.md`, `status.json`; the
  `History` tables of `SSS.md` (0.6), `SRS.md` (0.6), `traceability.md` (0.5),
  `class-a-gaps.md` (0.4), `licensing.md` (0.4), `ecss-compliance-matrix.md`
  (0.3), `development-plan.md` (0.4), `roadmap.md` (0.6),
  `verification-results.md` (0.6) and `security-sdls.md` (0.1) all match their
  revision headers and describe the M7 change (`ecss-criticality-b.md` is the
  sole exception → R4-1).

## 4. Conclusion

### R3 — Senior Systems Engineer (N14)

- [ ] APPROVED — no findings or Comments only
- [x] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

`SSS-REQ-019` is complete, unambiguous and verifiable, and it decomposes
cleanly to `SRS-REQ-034`/`SRS-REQ-035`; the middleware / pluggable-backend /
GAP-A14 boundary is correctly and explicitly documented. The class-B feasibility
is credible (host-testable protocol logic, deterministic reference backend,
crypto qualification deferred to GAP-A14). One Minor finding (R3-3, IV-uniqueness
wording not bounded by `iv_len >= 4`) must be corrected or justified; R3-1 and
R3-2 are Comments. No Major finding.

### R4 — Senior SW documentation reviewer (N17)

- [ ] APPROVED — no findings or Comments only
- [x] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

All quantitative claims in the M7 delta are derivable and mutually consistent
(requirements 69/36/19/14, 51 theorems, 9 CI claims, coverage 1192/1925 /
172/214), the trace gate passes, document ids are unique, links resolve, the
README panel matches `metrics.py`, and SPDX scoping is coherent with N13. Two
Minor findings must be corrected before this change is committed (norm N12/N17):
R4-1 (unrecorded `ecss-criticality-b.md` revision/history for M7) and R4-2
(ADR/header comment wrongly claim host-simulator use of the reference backend);
R4-3, R4-4 and R4-5 are Comments. No Major finding. This review is therefore
**APPROVED WITH COMMENTS pending the closure of R4-1 and R4-2**.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R3 (N9/N14)   | independent agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS (1 Minor + 2 Comments open) |
| R4 (N9/N17)   | independent agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS (2 Minor + 3 Comments open) |

## 6. Follow-up records

Norm N12/N17: each finding below is tracked to closure; the change must not be
committed/pushed until the two Minor findings are corrected (or explicitly
justified) and the delta is re-reviewed from fresh context.

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R3-3 | Minor | `security-sdls.md` §4 / SRS-REQ-034 | IV-uniqueness claim unconditional; code provides it only for `iv_len >= 4` | (to dispose) | to be provided | frick-fsw | **open** | required |
| R4-1 | Minor | `ecss-criticality-b.md` (N16) | GAP reference moved to A14 with no revision bump / `History` row | (to dispose) | to be provided | frick-fsw | **open** | required |
| R4-2 | Minor | `ADR-0013-sdls.md`, `ff_crypto_test.h` (N8/N17) | Reference backend wrongly claimed "used by the host simulator" (test-only) | (to dispose) | to be provided | frick-fsw | **open** | required |
| R3-1 | Comment | SSS-REQ-019 / SRS-REQ-034 | SDLS standards absent from `Applicable standards` front matter | (to dispose) | to be provided | frick-fsw | **open** | optional |
| R3-2 | Comment | SRS-REQ-035 | Zeroisation clause verified only indirectly | (to dispose) | to be provided | frick-fsw | **open** | optional |
| R4-3 | Comment | `ecss-criticality-b.md` §1/§3 (N8) | Baseline doc not extended with SDLS references/deliverable row | (to dispose) | to be provided | frick-fsw | **open** | optional |
| R4-4 | Comment | `class-a-gaps.md`, `licensing.md` | Missing trailing newline | (to dispose) | to be provided | frick-fsw | **open** | optional |
| R4-5 | Comment | M7 R1/R2 review artifact (N8/N16) | Sibling report id is non-`FRICK-REVIEW-00nn` | (to dispose) | to be provided | frick-fsw | **open** | optional |

---

Committed and pushed with the reviewed change (norm N11).
