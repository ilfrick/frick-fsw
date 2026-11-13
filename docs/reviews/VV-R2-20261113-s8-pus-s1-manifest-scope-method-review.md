# R2 Review — S8-PUS-FU S1 final-manifest scope method

| Field | Value |
| --- | --- |
| Document ID | FRICK-REVIEW-VV-R2-S8-PUS-S1-MANIFEST-SCOPE-20261113 |
| Reviewer profile | R2 / SW V&V |
| Reference | Current working tree, 2026-11-13; pre-final-manifest snapshot |
| Items under review | `r4/README.md`; `docs/reviews/README.md`; `docs/vv/verification-results.md`; `docs/status/loop-audit.md` |
| Scope | Suitability and clarity of the stated manifest inventory/exclusions, candidate-patch and review inclusion, and separation of substantive review from hash-only attestations |
| Independence | Fresh subagent context for this focused read-only review. This is an AI review record, not human independence authority or approval. |
| Norms | N8, N9, N10, N11, N12, N17, N20, N22 |

## 1. Scope and method

Read the four specified documents and inspected the current `r4/assets.sha256`
and package file listing only to establish the prior manifest state and whether
the candidate patch is retained. This is a **scope-method suitability review**;
it does not regenerate or verify the final manifest, perform `sha256sum -c`,
review the S1 test oracle, qualify testware, or grant gate/NC/release approval.

## 2. Assessment

The documented final scope is understandable and appropriately bounded:

- all regular files recursively under `r4/`, plus per-path entries for the
  explicitly enumerated outside files in the final manifest;
- exactly four exclusions: `assets.sha256` itself and the three specifically
  named PA/N20, V&V/R2, and R4/N17 post-manifest attestation reports;
- `candidate-working-tree.patch` is retained under `r4/` and therefore falls
  inside the recursive package scope;
- the substantive R4 report is distinguished from the later R4 hash attestation
  and is explicitly intended to be manifest-bound;
- the reports/index/audit/results consistently describe attestations as digest,
  inventory/exclusion, and checksum records—not as substantive N17/N20 review,
  tool qualification, anomaly disposition, or NC closure.

The boundaries also preserve the required distinctions: a checksum attestation
does not establish N22 requirements-based test adequacy, N10 tool qualification,
or closure of any NC. The review index pre-registers all three attestation paths
and separately identifies the substantive R4 review.

## 3. Finding

| # | Severity | Item | Finding | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| C-01 | Comment | `r4/README.md` §Integrity binding; current `assets.sha256` | The final scope prose correctly excludes transient `build/` paths and says exact artifacts are retained under `r4/`, but the currently present, explicitly stale 45-entry manifest still contains `build/evidence-20261112-pus-s1-r4/...` and `build/renode/...` entries. This does not invalidate the documented *intended* final method because the records label that manifest stale, but it is not evidence that the final inventory has been generated or conforms to the new scope. | `r4/README.md` lines 28–30; `docs/vv/verification-results.md` lines 796–810; current `r4/assets.sha256` lines 2–4, 16 | When the final freeze occurs, regenerate from the declared scope, ensure no transient `build/` path remains unless specifically re-authorized, and have the independent attestations report the resulting exact inventory and checksum outcome. Include this report and the resulting index update in the manifest. |
| C-02 | Comment | `docs/status/loop-audit.md` N8/N17; `docs/reviews/README.md` row 25 | The distinction between the intended substantive R4 review and the hash-only R4 attestation is clear, but current wording says the substantive report “is included” in the frozen/package manifest while its pre-indexed report file is not present in the working tree and the old manifest remains stale. The intent to include it is clear; actual inclusion is not yet established. | Loop-audit lines 617, 626; review index line 25; file lookup for `2026-11-13-r4-pus-s1-final-substantive-documentation-review.md` returned no file; stale manifest status is stated in `docs/vv/verification-results.md` lines 796–810. | Until the substantive report exists and final manifest verification establishes its inclusion, use prospective wording (“shall be included” / “intended for inclusion”). After the report and final manifest are frozen and checked, state actual inclusion with the manifest evidence reference. |

## 4. Conclusion

**APPROVED WITH COMMENTS — scope method only.** The four-document wording is
adequate to distinguish recursive package contents, explicit outside inventory,
the four exclusions, retained candidate patch, substantive review inclusion,
and hash-only attestations. C-01 records that the old manifest is still stale;
C-02 records that substantive-report inclusion is prospective, not established
on this snapshot. This report is **not** a final checksum pass, does not resolve
the previously open evidence-integrity Major, and does not close any NC or
satisfy N10/N22.

## 5. Status limits

- **Fact:** the current on-disk `assets.sha256` is the prior 45-entry manifest;
  its listed `build/` paths do not match the newly documented final-scope rule.
- **Not verified:** final manifest generation, completeness of final outside
  paths, exact exclusion check, digest identity, and `sha256sum -c` result.
- **Required next evidence:** final freeze followed by the three separately
  scoped post-manifest attestations. Any covered-file change requires a new
  freeze/rebind cycle.
- No N10 tool qualification, N22 test adequacy, N17/N20 approval, NC closure,
  qualification, acceptance, or release decision is implied.
