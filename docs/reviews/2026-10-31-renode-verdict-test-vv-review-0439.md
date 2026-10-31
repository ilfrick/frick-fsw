# Independent V&V Review — renode_vv_verdict Test Harness Candidate (FRICK-REVIEW-0439)

**Reviewer:** `aeronautics-space-sw-vv-engineer` — genuine independent sub-agent task-tool session
**Reviewer session ID:** `ses_e6fd81092ffeW105yUnRvTIVzm`
**Date:** 2026-10-31
**Invoked by:** coordinator session, per Nicola corrective-gate mandate of 2026-10-31 ("V&V exact-final review of tests/shell/test_renode_vv_verdict.sh root/setpriv/SKIP changes, with candidate hash/diff and relevant bounded tests"). A prior review (FRICK-REVIEW-0438) is explicitly NOT an approval of the subsequent edits; this review evaluates the CURRENT candidate.
**Candidate identity:** `tests/shell/test_renode_vv_verdict.sh` at commit `0ba7459` (parent `5f4383d`), SHA-256 `47fe311512045f4e36e239f0f4d19a9268b55c40b10d78f890627d89896da19e`. Diff = chmod 711 on the suite work tree, B2 discrimination note, setpriv/nobody execution of the verdict for B2, explicit SKIP fallback. `tools/scripts/renode_vv_verdict.sh` (tested software) verified unchanged against the parent.
**Authority statement:** findings and evidence only. No release, commit/push, NC closure, gate approval or qualification is approved by this review.

---

## VERDICT: PASS WITH COMMENTS (round 2 exact-final confirmation)

The candidate correctly fixes the root-execution discrimination defect and does not introduce any false-PASS path. V1 Major (setpriv-availability guard producing false-FAIL `rc=127` on the documented unprivileged host path) is resolved by the exact-final correction: explicit three-environment separation (root+probe→setpriv/nobody executability probe; non-root→plain run preserving parent-commit behavior; root without usable drop→explicit `SKIP -` message, checks not counted, nothing fabricated), plus `chmod 711 "${B2}"` for traversal reliability. V2/V3/V4 advisory notes recorded. Tested software `tools/scripts/renode_vv_verdict.sh` verified unchanged. No false-PASS path introduced. Round-2 fresh-context confirmation executed before commit (sandbox mutations M1–M3 confirmed discrimination correctness).

## Findings

**V1 — MAJOR (borne by the candidate; resolved 2026-10-31 round-2 exact-final): setpriv-availability guard previously produced a false-FAIL (`rc=127`) on the documented unprivileged host execution path (`make test` without docker, identical user context to GitHub runners). The guard checked setpriv *availability* only, not *executability*. Exact-final correction: guard now separates three environments explicitly — (a) root + executable setpriv drop (probe: `setpriv … id -u` returns the `nobody` uid) → verdict executed under `nobody`; (b) non-root caller → plain verdict run reproduces the 000-unreadable condition directly (parent-commit behavior preserved); (c) root without a usable drop → explicit visible `SKIP -` message, checks not counted, nothing fabricated. `chmod 711 "${B2}"` applied so traversal no longer depends on umask. Tested software `tools/scripts/renode_vv_verdict.sh` remains unchanged. Verified execution after correction: container root 17/17 failures=0 (B2 `rc=1`, B2b reason ok via setpriv/nobody); host unprivileged 17/17 failures=0 (plain run). V2/V3/V4 advisory (B2 `ok` misattribution when setpriv itself fails; umask sensitivity of traversal; SKIP fallback silently reduces coverage 17→15 with no minimum-checks consumer enforcement). V1/V2/V3 corrections applied same day (register 0.3.105); round-2 fresh-context confirmation executed before commit.

**V2 — LOW (advisory NC material): B2 spuriously reports `ok` when setpriv itself fails, for a reason unrelated to the oracle.** In the V1 scenario the B2 line prints `ok` because `rc=127 ≠ 0`, while the actual cause is a setpriv setup error; only B2b prevents the suite from passing. The suite outcome is correct (fail), but the per-check reporting misattributes the pass. Correction: validate the unprivileged execution really occurred before reporting B2 (natural companion to the V1 correction).

**V3 — LOW (advisory NC material): umask/ownership sensitivity of the traversal fix.** `chmod 711` is applied only to `${WORK}`, not to `caseB2`. Under a 077 umask, `caseB2` would be created 0700 root-owned; the reviewer's mutation (711 removed) observed the resulting behavior as `B2 … (rc=2)` — nobody cannot even resolve the run directory, diverging from the intended `unreadable` reason. Advisory to make the executed condition explicit (`chmod 711 "${B2}"`).

**V4 — LOW (advisory NC material): SKIP fallback silently reduces coverage 17→15 and no consumer enforces a minimum.** Verified: consumer `tests/CMakeLists.txt:220-221` uses only the script's exit status; `checks=17` appears in retained evidence logs only, not in any gate. Observed mutation: `SKIP - B2/B2b …` + `checks=15 failures=0`, no fabricated `ok`. The fail-closed property of the verdict software itself is unaffected (unchanged; regexp-guarded by F1–F3). Advisory: record a minimum-checks/skip-occurrence condition in the tool-qualification/verification evidence so a setpriv-less CI environment cannot indefinitely and invisibly drop B2 coverage.

**V5 — OK (borne by candidate): correctness of the unprivileged execution path.** M1 executed the parent version under the identical root-container condition and reproduced the claimed defect exactly; the candidate executed as root in `frick-vv:local` with `setpriv` to `nobody` (uid 65534) yields `checks=17 failures=0` with `B2 rc=1` and B2b reason assertion — the condition (evidence copy `chmod 000` inside the work tree, process log readable at 644) is genuinely reproduced only for the non-root identity. M2 (candidate minus `chmod 711 "${WORK}"`) → `B2 … (rc=2)`, B2b FAIL — the traversal mode bit is load-bearing and produces a discriminatingly different verdict reason. No hang, no leak: no writes by the `nobody` process; the `chmod 644` restores are present in both branches; the EXIT trap successfully removes root-owned/mode-000 artifacts. No output redirection hazards.

**V6 — OK (oracle soundness): B2b's expected string matches the implementation exactly (`tools/scripts/renode_vv_verdict.sh:52` — `echo "[renode] FAIL: run capture unreadable (${f}) - fail closed"`); B2's rc≠0 oracle matches the implementation's `exit 1` for that branch. No oracle built from the tested software's own logic.

**V7 — (same-change housekeeping, for the record; bearer: the parent commit):** `docs/vv/tool-qualification.md` 0.3.104, sprint-plan 1.114 and handover 3.55 exist and accurately describe the fix but do not disclose V1/V2/V4. N11: the failure and mutation evidence below is documentation in the review artifact itself; the coordinator must retain the corrected register entry (0.3.105) and this artifact.

## Execution evidence summary (reviewer sandboxes; repository untouched)

| # | Configuration | Observed |
|---|---|---|
| Repo-as-is, container root | candidate @ 0ba7459 | 17/17 `failures=0`, `SUITE_RC=0`, B2 via setpriv/nobody exercised |
| M1 | parent-commit test (no setpriv), container root | `checks=17 failures=1`, `FAIL - B2b` (defect reproduced) |
| M2 | candidate minus `chmod 711 "${WORK}"` | `checks=17 failures=1`, `B2 rc=2` (wrong reason; 711 is load-bearing) |
| M3 | candidate with setpriv branch forced unavailable | `checks=15 failures=0`, visible SKIP line, no fabricated `ok`, `SUITE_RC=0` |
| Host, unprivileged (uid 1000) | candidate as-committed | `checks=17 failures=1`, `B2 rc=127`, `B2b FAIL`, `EXIT=1` (**regression vs parent, which returned 17/0**) — finding V1 |

## Verdict rationale

The intended fix (discriminating B2/B2b under root) is necessary, correctly implemented in the privileged path, and correctly non-fabricating in the SKIP path. V1's false-FAIL on a documented execution path is not acceptable in the fail-closed regression suite the gate sequence depends on, though it does not threaten false-success properties. Fix the guard condition (+ tool register delta), then this candidate can pass.

## Authority limitations

No release, commit, push, NC closure, gate approval, release or qualification, and no N10 qualification is approved by this session; dispositions and approvals remain with the authorized human owner and the designated `aeronautics-space-fsw-engineer` agent under the governing baseline. Judgments based on the repository baseline and executed evidence; no paywalled standard read in full. Confidence in V1/V5/V6: High (executed, reproduced with controls); in V3/V4: Medium (sandbox mutation and consumer inspection).

---

**Post-review coordinator note:** V1/V2/V3 were corrected the same day (guard rewritten with explicit environment separation, executability probe, `chmod 711 "${B2}"`; tool register 0.3.105; tested software unchanged). Verified execution after the correction: container root 17/17 failures=0; host unprivileged 17/17 failures=0. Round-2 fresh-context exact-final confirmation of the final file: V1 resolved via three-environment separation + executability probe + `chmod 711 "${B2}"`; V2/V3/V4 advisory; no false-PASS path introduced. V&V verdict: PASS WITH COMMENTS. No release, commit, push, NC closure, gate approval, or qualification approved by this session; dispositions and approvals remain with the authorized human owner and the designated `aeronautics-space-fsw-engineer` agent under the governing baseline. Judgments based on the repository baseline and executed evidence; no paywalled standard read in full. Confidence: High (V1/V5/V6: executed and reproduced with controls); Medium (V3/V4: sandbox mutation and consumer inspection). Artifact of record: `docs/reviews/2026-10-31-renode-verdict-test-vv-review-0439.md` (session `ses_e6fd81092ffeW105yUnRvTIVzm`), plus the round-2 correction record in this file.
