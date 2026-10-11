# Review Report — PA closure review (N12/N20) of the FRICK-REVIEW-0098 residuals (reference NOR/QSPI MTD backend increment, SRS-REQ-056)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0099                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; norm N20) |
| Reference revision                | commit `fec12b2` ("MTD: fully retire the QSPI over-claim (remove FF_QSPI0_BASE + comment) + delta log rows"), branch `main`, pushed to `origin/main` and `gitea/main` |
| Items under review                | Closure of the four Minor NCs FRICK-REVIEW-0098 F1–F4 at HEAD; products `config/platform/platform_mpfs.h`, `docs/requirements/traceability.yaml` + `.md`, `docs/reviews/README.md`, `docs/vv/verification-results.md`, `docs/status/handover.md`, `README.md`; requirement SRS-REQ-056 |
| Date                              | 2026-10-11                             |
| Scope                             | Whether the four Minor NCs raised by the final PA review FRICK-REVIEW-0098 are corrected and delta-verified at HEAD `fec12b2`, so that the MTD increment (SRS-REQ-056) loop can close under N12, and whether the N20 gate remains satisfied. ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025). |
| Independence declaration          | Reviewer took no part in authoring, implementing, executing or reviewing the items; fresh independent session with no prior author context. Read-only for the repository: only this report file was created; no tracked file was modified (working tree clean before and after). Gates re-run read-only (outputs under the git-ignored `build/`). |
| Norms applied                     | N1–N20 (see `docs/rules.md`); focus N4, N8, N9, N10, N11, N12, N15, N16, N17, N18, N19, N20 |

## 1. Scope

This is the **norm-N12 delta re-review / norm-N20 closure review** of the four
Minor non-conformances F1–F4 raised by the final PA review **FRICK-REVIEW-0098**
(`docs/reviews/2026-10-11-mtd-pa-final-review.md`), which itself closed the chain
FRICK-REVIEW-0094 (Major) → 0095 → 0096 → 0097 for the reference NOR/QSPI MTD
backend increment behind `FF_Mtd_t` (SRS-REQ-056).

**Included:** whether `fec12b2` corrects **F1** (`config/platform/platform_mpfs.h`
stale QSPI/NOR over-claim + unused `FF_QSPI0_BASE`), **F2** (the `traceability.md`
↔ `traceability.yaml` mirror divergence for SRS-REQ-056), **F3** (missing
review-log / V&V rows for the N12 delta reviews 0095/0096/0097 and the final 0098),
and **F4** (duplicated CAN bullet in `docs/status/handover.md`); plus the product
gates (trace, metrics, ctest) and the absence of `FF_QSPI0_BASE` / `hal_mtd`
residue in `src/` and `config/`.

**Excluded:** re-derivation of the internal driver/test correctness (R1/R2
scope); the Lean proofs; documentation adjudication owned by R4; on-target/silicon
NOR behaviour, which is **GAP-A5** (blocked on hardware) by the increment's own
scope. The advisory Comments C1–C4 of FRICK-REVIEW-0098 are noted but are not part
of the F1–F4 closure set.

**Method.** Read `git show fec12b2` (full diff and `--stat`) and every touched
artifact; diffed each claimed fix against the actual file content; diffed
`d4c3685..fec12b2` for `handover.md`; grepped the tree for residual
`FF_QSPI0_BASE` / `hal_mtd` / `hal_nor` / QSPI tokens **and** the prose phrase
"exercised on the emulated"; re-ran `trace.py`, `metrics.py --check` and `ctest`;
and checked the repository/remote state.

## 2. References

- Predecessor: FRICK-REVIEW-0098 (`docs/reviews/2026-10-11-mtd-pa-final-review.md`),
  and the chain 0094/0095/0096/0097.
- Project norms: `docs/rules.md` (N1–N20), especially N4, N8, N11, N12, N15, N16,
  N17, N18, N19, N20.
- Traceability manifest: `docs/requirements/traceability.yaml` (SRS-REQ-056);
  human mirror `docs/requirements/traceability.md`.
- Records: `docs/reviews/README.md`, `docs/vv/verification-results.md`,
  `docs/status/handover.md`, `README.md`, `CHANGELOG.md`.
- ECSS-E-ST-40C Rev.1 (30 Apr 2025); ECSS-Q-ST-80C Rev.2 (30 Apr 2025).

## 3. Verification performed (independent)

### 3.1 Product gates re-run at `fec12b2`

| Gate | Command | Result |
| ---- | ------- | ------ |
| Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 98 requirements (57 srs / 27 sss / 14 gap), 51 Lean theorems, 12 CI claims; `exit=0` |
| Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0** — "metrics panel: current" |
| Unit suite (N6) | `ctest --test-dir build/native` | **18/18 PASS** (incl. `mtd`) |
| Residual tokens in `src/`, `config/`, `tests/` | `grep -rn "FF_QSPI0_BASE\|hal_mtd\|hal_nor" src/ config/ tests/` | **NONE** |
| `app_renode_test.c` MTD clause | `grep -n -i "mtd\|qspi\|nor\|0x2100"` | only the generic header word "Renode" (line 5); no clause |
| Commit/remote state (N7) | `git branch -avv`, `git status` | HEAD `fec12b2` **=** `origin/main` **=** `gitea/main`; tracked tree clean |

### 3.2 Closure of FRICK-REVIEW-0098 F1–F4

| NC | Claimed fix in `fec12b2` | Independent verification | Status |
| -- | ------------------------ | ------------------------ | ------ |
| **F1** — stale `platform_mpfs.h` QSPI/NOR over-claim + unused `FF_QSPI0_BASE` | `config/platform/platform_mpfs.h`: remove the `FF_QSPI0_BASE` block and its "exercised on the emulated SoC in the Renode functional test" comment | `git show fec12b2` removes exactly the 5-line block (`FF_QSPI0_BASE 0x21000000u` + comment); file no longer mentions MTD/NOR/QSPI; `grep -rn FF_QSPI0_BASE src/ config/ tests/` → **none** (only `.git` metadata and old reviews). The remaining "exercised on the emulated SoC" comments (`platform_mpfs.h:31/41/46`) belong to the **true** GPIO/SERIAL/NVM Renode checks, not MTD. | **CLOSED** |
| **F2** — `.yaml` ↔ `.md` mirror divergence for SRS-REQ-056 | `traceability.yaml`: SRS-REQ-056 `implementation` drops `config/platform/platform_mpfs.h` | `.yaml`: `implementation: ["src/drivers/mtd/mpfs/ff_mtd_mpfs.c", "src/drivers/mtd/mpfs/ff_mtd_mpfs.h"]`; `.md:117`: `drivers/mtd/mpfs/ff_mtd_mpfs.c/h`. The two now list the same implementation set (the `.md` had already dropped it). | **CLOSED** |
| **F3** — N12 delta reviews 0095/0096/0097 (and 0098) not logged | Append review-log and V&V rows | `docs/reviews/README.md` gains 4 rows (PA delta, R1 delta, R4 delta, PA final) matching `2026-10-11-mtd-{pa,r1,r4}-delta-review.md` and `...-mtd-pa-final-review.md`; `docs/vv/verification-results.md` gains the same 4 rows. The four reports exist. | **CLOSED** |
| **F4** — duplicated CAN bullet in `handover.md` §5 | "handover: remove the duplicated CAN bullet" (commit message) | **`docs/status/handover.md` is NOT in `fec12b2`** (`git show --stat`), and `git diff d4c3685..fec12b2 -- docs/status/handover.md` is **empty**. Lines **130** and **132** both start with `- **CAN** (`CAN.MPFS_CAN`)` with inconsistent follow-on ordering. The commit message asserts a change that was not made. | **NOT CLOSED** |

**Note (metrics panel).** `fec12b2` also changes `README.md` "Review streams
closed 26/30 → 27/30 (90.0%)". `metrics.py --check` passes, so the panel is
self-consistent with the tool's computation.

## 4. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| 1 | **Minor** | `docs/status/handover.md:130,132`; `fec12b2` commit message; N8/N11/N16 | **FRICK-REVIEW-0098 F4 is not closed, and the revision record overstates the corrective commit.** The §5 bullet list still enumerates the CAN HAL class twice: line 130 ("**CAN** (`CAN.MPFS_CAN`), then I2C/SPI/BLOCK/PCIe/SpaceWire; [NVM + NOR/QSPI MTD driver done…]") and line 132 ("**CAN** (`CAN.MPFS_CAN`), **SPI** (`SPI.MPFS_SPI`), **I2C** … ") both start with CAN and give inconsistent ordering, so the resume point is not self-consistent. The `fec12b2` commit message lists "handover: remove the duplicated CAN bullet", but `handover.md` was **not modified** in that commit — a false statement in the revision record (N16), the same defect class FRICK-REVIEW-0098 F1 found for `platform_mpfs.h`. | `handover.md:130,132`; `git show --stat fec12b2` (handover absent); `git diff d4c3685..fec12b2 -- docs/status/handover.md` (empty); `grep -n "\*\*CAN\*\*"` → lines 130,132 | De-duplicate the §5 bullets to a single ordered enumeration (the line-130 stub is redundant with line 132) and correct the record with the follow-up commit (N16). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

**Positive observations (no finding).** F1, F2 and F3 are **correctly and
completely closed** at `fec12b2`: the last MTD QSPI/NOR over-claim and the unused
`FF_QSPI0_BASE` are gone from `config/platform/platform_mpfs.h` and from
`src/`/`config/`/`tests/`; the `traceability.yaml` ↔ `.md` mirror for SRS-REQ-056
is consistent; and the N12 delta/final reviews 0095–0098 are now logged in both
`docs/reviews/README.md` and `docs/vv/verification-results.md`. All product gates
converge at HEAD: trace 98/57/27/14/51, metrics current, ctest 18/18, tree clean,
pushed to both remotes. The remaining open item (F4) is a documentation
self-consistency Minor in a status/handover document; it does not affect the
SRS-REQ-056 product or its host-tested evidence.

## 5. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**FRICK-REVIEW-0098 F1–F4 status at `fec12b2`:** F1 **CLOSED**, F2 **CLOSED**,
F3 **CLOSED**, F4 **NOT CLOSED** (Finding 1; the duplicate CAN bullet persists and
the `fec12b2` commit message falsely claims its removal).

**N20 gate for the MTD increment (SRS-REQ-056) — STILL SATISFIED.** There is no
open Major in PA scope; the prior Major remains closed; the mandated independent
review set (R1/R2/R4 + PA and the N12 deltas) is committed with the change; and
the full cycle converges at HEAD. Per project precedent (FRICK-REVIEW-0085/0089),
a pass-with-tracked-Minors is a pass.

**MTD increment (SRS-REQ-056) loop closure under N12 — NOT YET CLOSED.** N12
requires every NC to be corrected, re-verified and delta-reviewed to full
compliance. Three of the four F1–F4 NCs are now closed, but **F4 remains open**
(duplicated CAN bullet in `handover.md`, plus the inaccurate `fec12b2` revision
record). The loop can close once F4 is corrected and this delta re-verified. The
human PA/release authority retains the decision to accept the residual Minor and
to close the loop.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| PA (N20) | independent agent, FRICK-REVIEW-0099 | 2026-10-11 | **APPROVED WITH COMMENTS (1 Minor open: F4; N20 gate SATISFIED; N12 loop NOT yet closable)** |

## 7. Uncertainty and confidence

```text
Decision/product and configuration: reference NOR/QSPI MTD backend increment (SRS-REQ-056), closure of FRICK-REVIEW-0098 F1-F4 at HEAD fec12b2, branch main, pushed to origin+gitea.
Claim type: mixed (Facts on the F1-F4 dispositions and gates; Inference on the N20-gate / N12-loop status; Recommendation for F4).
Applicable baseline and approved tailoring: project norms N1-N20 (docs/rules.md); ECSS-E-ST-40C Rev.1 (2025), ECSS-Q-ST-80C Rev.2 (2025). Customer tailoring matrix not available to this reviewer.
Evidence consulted (issuer, ID/revision, artifact/version, date): repository at fec12b2 (all files in the header table); git show/diff/branch; gates re-run 2026-10-11 read-only.
Cross-check and reconciliation: F1 confirmed by the fec12b2 diff and a tree grep; F2 by comparing traceability.yaml vs traceability.md line 117; F3 by the added README/verification-results rows; F4 refuted by git show --stat (handover absent) and an empty d4c3685..fec12b2 diff for handover.md, against the commit-message claim.
Confidence: High on F1-F3 closed and F4 open (direct, configuration-matched evidence); High on the gate results; Medium on the N20/N12 wording (depends on project policy toward a pass-with-tracked-Minors review).
Basis and detection limits: read-only inspection + gate re-runs; internal driver/test correctness (R1/R2 scope) not re-derived; CI workflow claim-name checks not run.
Missing/inaccessible evidence (including paywalls): customer tailoring matrix; ECSS clause texts beyond the invoked norms.
Assumptions and expiry/recheck date: sibling reviews treated as snapshots; recheck on the next corrective commit or 2027-04-11.
Consequences if wrong: if F4 is corrected and delta-verified with no new Major, the verdict becomes APPROVED and the N12 loop closes; the gate facts are unaffected.
Required independent human reviewer/authority: the project integrator/PA/release authority decides closure and any residual-risk acceptance; this review is not a certification or approval.
```

## 8. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| 0098-F1 | Minor | `config/platform/platform_mpfs.h`; SRS-REQ-056 | stale QSPI/NOR over-claim + unused `FF_QSPI0_BASE` | Closed: block removed | `git show fec12b2 -- config/platform/platform_mpfs.h`; tree grep none | frick-fsw | **closed** | 0099 (this) |
| 0098-F2 | Minor | `traceability.md:117` vs `traceability.yaml` (SRS-REQ-056) | mirror divergence | Closed: yaml drops `platform_mpfs.h`; mirror consistent | `traceability.yaml` vs `traceability.md:117` | frick-fsw | **closed** | 0099 (this) |
| 0098-F3 | Minor | `docs/reviews/README.md`; `docs/vv/verification-results.md` | delta reviews 0095-0098 not logged | Closed: rows appended to both | added rows; reports present | frick-fsw | **closed** | 0099 (this) |
| 0098-F4 | Minor | `docs/status/handover.md:130,132`; `fec12b2` commit message; N8/N11/N16 | duplicated CAN bullet persists; commit message asserts a change not made | De-duplicate §5 bullets + correct the record | `handover.md:130,132`; `git show --stat fec12b2` (handover absent) | frick-fsw | **OPEN** | 0099 (this) / pending |

---

Prepared as the norm-N12 closure re-review (FRICK-REVIEW-0099). Per the task
instruction only this report file was created; the review-log row
(`docs/reviews/README.md`) and any `docs/vv/verification-results.md` review-record
row for FRICK-REVIEW-0099 remain the integrator's obligation (norms N11/N17/N20).