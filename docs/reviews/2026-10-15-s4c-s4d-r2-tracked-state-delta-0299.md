# FRICK-REVIEW-0299 — T23 tracked-dirty checkout-state follow-up

| Field | Value |
| --- | --- |
| Profile | R2 — Senior software V&V engineer (independent context) |
| Candidate | Mutable uncommitted worktree based on `6ce6bd7` |
| Scope | Supplemental tracked-file probe following FRICK-REVIEW-0298 |
| Verdict | **PASS for the enumerated tracked-dirty lwIP behavior only** |
| Date | 2026-10-15 |

## Result

The current `verify_checkout()` checks the expected pinned `HEAD`, then rejects
any output from
`git status --porcelain --untracked-files=all --ignored=matching`. A retained
tracked-file probe shows exit 1 with ` M README`; the clean retry shows exit 0.
The reviewer independently repeated a temporary tracked-file edit/restore
probe, confirmed the pre-probe file hash was restored, and verified a clean
fetch and checkout afterward.

This adds the tracked-dirty case to NC-EXT-82's **enumerated reused-checkout
behavior** closure. FRICK-REVIEW-0298 remains a point-in-time report: tracked
modification was code-inspected but had not yet been separately probed at that
review. This follow-up and probe do not qualify T23, exercise the optional
ASN.1 checkout, establish hosted workflow evidence, or close provenance,
licensing, target, or other assurance gates.

**Facts:** Source, retained logs, and the independent temporary probe agree for
lwIP. **Assumption:** no persistent changes were made during the reviewer probe;
the reviewer reported restoring the original hash. **Confidence:** high for the
tested local behavior, limited to this checkout and helper. No release or phase
approval is implied.
