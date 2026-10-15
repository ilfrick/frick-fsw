# T21 fail-closed correction — R2 delta review — FRICK-REVIEW-0249

| Field | Value |
| --- | --- |
| Reviewer profile | R2 — Senior SW V&V engineer (independent) |
| Reviewer session | `ses_ec89a0201ffeSrnmMx72J7DcE6` |
| Reference | Mutable worktree; review conducted after FRICK-REVIEW-0248 correction |
| Date | 2026-10-14 |
| Scope | Unreadable managed source behavior, regression test, non-managed binary behavior, local self-check |

## Bounded verdict

**PASS for the bounded fail-closed correction; no new finding in this scope.**
This does not close NC-EXT-44, qualify T21, establish hosted execution, or
approve the overall license transition.

## Evidence

- Checker catches `UnicodeDecodeError` and `OSError` while validating managed
  source-like files and appends a validation error.
- Regression test exercises invalid UTF-8 in a managed `.c` file.
- Temporary probes: invalid-UTF-8 `src/probe.c` produces an error; a binary
  `assets/probe.bin` outside the managed SPDX scope is not treated as source.
- `make gsw-selfcheck`: PASS (GSW 5, metrics 9, license checker 14 tests; ping
  self-check completed).
- `python3 tools/scripts/license_check.py`: PASS.

One initial ad hoc probe failed due to a missing Python import path; corrected
invocation passed. No repository files were intentionally modified by the
review.

## Limits

Mutable worktree identity; no hosted CI/configuration evidence or immutable
artifact binding. T21 remains NOT FULLY QUALIFIED and NC-EXT-44 OPEN pending
complete independent R2/R4/PA and hosted-evidence criteria.
