# Independent SW PA review — XW-2 target contract (SRS-REQ-073/075)

| Field | Value |
| --- | --- |
| Review ID | `PA-REVIEW-XW2-20261102` |
| Date | 2026-11-02 |
| Reviewer profile | Senior Aeronautics and Space Software Product Assurance (PA) Engineer |
| Configured subagent | `aeronautics-space-sw-pa-engineer` |
| Harness session | `ses_e663d6753ffeUrSrkkLe6Fj7IK` (harness-reported; state `completed`) |
| Model / route | `opencode/big-pickle` (Big Pickle, provider `opencode`, $0 free route) |
| Increment under review | XW-2 target contract: TS-7, `qemu_xw2_vv.sh`, `xw2_manifest.json`, CMake/CI wiring, T25 register entry, `verification-results.md`, traceability mapping, sprint-plan Rev 1.116 |
| Base HEAD | `65e4a91` |
| Verdict | **PASS WITH COMMENTS — conditional for commit** |
| Review mode | read-only; no build, test, formatter, XW-2 runner or writer executed |

> **Authority statement.** AI expert-persona review. It is not a human
> credential, not a certification, qualification, airworthiness finding, waiver
> or release approval, and it closes no non-conformance. Human approval remains
> mandatory for every decision gate.

## 1. Basis of verdict (as issued)

The bounded technical result is supported by direct retained evidence: run
`build/xw2-run-20261101T172050Z` (log `build/gate-evidence/xw2-final.log`, exit
`xw2-final.exit` = 0) showing 13/13, `SRS-REQ-073 PASS=12` /
`SRS-REQ-075 PASS=1`, `WHOLE-RUN PASS`, the no-wrap control as `EXPECTED
FAILURE`, positive/control ELF `0972d96a…c710f251` / `b695b1ad…d6011926`, and
50/50 host CTest; the new evidence archive verifies 38/38 hashes. The non-claim
boundary is the strongest part of the increment: N18 remains FAIL everywhere, and
no device, reset-scope, independence, timing, mission-timeout or qualification
claim was found.

Three Majors require correction before commit: the evidence-of-record pointer
cites the only run produced by the pre-final runner (PA-01), the Gitea blocking
gate cannot execute as written (PA-03), and the blocking gate rests on a tool
declared NOT QUALIFIED with no recorded deviation (PA-02). None of these changes
the technical result.

## 2. Findings

| ID | Severity | Finding | Disposition required |
| --- | --- | --- | --- |
| PA-01 | **Major** | Evidence of record cites `build/xw2-run-20261101T170112Z`, the only one of the three runs without `backend.txt`/`toolchain-id.txt`; the committed runner writes them. `INDEX.md` already declares `xw2-final.log` (run `172050`) authoritative. Content and ELF hashes are identical across runs, so the technical claim is unaffected. | Repoint `verification-results.md`, `sprint-plan.md` and the T25 register to `172050` / `xw2-final.log` / `xw2-final.exit` (rc 0); after commit cite the commit hash, not only `65e4a91`. |
| PA-02 | **Major** | The blocking gate consumes a tool that `tool-qualification.md` declares NOT QUALIFIED, with no risk-acceptance or compensating-control record. N10 requires qualification before tool output is used as V&V evidence; the gate's own run cannot be its qualification record (circular). "Blocking" also depends on a Gitea required-status rule not in the tree. | Record a bounded deviation (gate consumes exit status only, not acceptance evidence; named owner/date), or complete the N10 review before relying on the gate; name the required-status rule. |
| PA-03 | **Major** | Gitea `xw2-target` cannot execute: `make pus-asn1` unconditionally invokes `docker`, and the job container installs no docker CLI and mounts no socket. The identical failure is retained in `ci-local.log` (`docker: command not found` → exit 127). The job dies before `make xw2`; `pipeline` never runs. | Run `pus-asn1` on the host before the container step, pre-seed the generated output, use a docker-capable runner, or build the XW-2 image without the ASN.1 backend; then execute the job in Gitea and retain the run. |
| PA-04 | Minor | `nm` segregation counts are printed, never asserted (`\|\| true`); a future 2/1 image would still PASS. Current `nm-segregation.txt` is correct (3/0 and 0/3), so the claim is true but "proven" overstates an observation. | Assert the four counts and fail on mismatch, or change "proven" to "observed". Any script edit invalidates the `172050` evidence and requires a fresh run. |
| PA-05 | Minor | Declared gaps `XW-G1..XW-G5` are absent from TS-7 and unenforced: the verifier only rejects *undeclared* gap lines, never requires declared gaps to be reported. | Add an `XW-G1..G5` table (requirement, reason, authority) to TS-7 §5 and state that no console line is expected, or remove the unused array. |
| PA-06 | Minor | `target_req_verify.py` validates case-ID + PASS token only; the `detail` field is echoed for FAIL/SKIP but any PASS detail counts. TS-7 §3's "Expected result (oracle)" column can be read as console-verified. | State the enforcement split in TS-7 §6, or check a canonical detail string per case. |
| PA-07 | Minor | The XW-2 record is a second H1 inside an unrelated results file (`verification-results.md:1` is the `renode_vv_verdict.sh` record, `:46` is XW-2) with no document ID/revision, while `tool-qualification.md:226` says the file is "marked SUPERSEDED (G6)" — it is not. | Give XW-2 its own CM-identified results record (or a revisioned section with ID/date/owner) and reconcile the SUPERSEDED statement. |
| PA-08 | Minor | `checks=73/49` is corroborated only by the transient `build/xw1-host/Testing/Temporary/LastTest.log`; the archive's `host-ctest.log` shows pass lines only. `format-apply.log` / `format-check2.log` contain command text only and no exit file exists, yet `INDEX.md` asserts exit 0. | Cite/archive `LastTest.log`; capture format exit statuses explicitly or mark them "not captured". |
| PA-09 | Minor | sprint-plan 1.116 misstates scope ("XW-1/XW-2/XW-3"), says "Independent Reviews: None performed" although the V&V review of this increment exists, and the row repair did not reorder 1.113/1.115/1.114/1.116. | Correct scope to XW-1/XW-2; record the V&V review; reorder rows; keep `Sprint closures today: 0`. |
| PA-10 | Observation | QEMU exit 124 is accepted, so a console that prints all 13 lines then hangs still passes (a partial console fails closed). B1 waits 3000 ms and asserts counts only; no boot-time margin is retained. | Record the observed margin in the run directory; make the timeout a parameter. |
| PA-11 | Observation | XW-T7 at-most-once is enforced by the test double, not by software. Honestly disclosed; if the authority later requires software enforcement, XW-T7 will not detect its absence. | None for this increment; carry as the recorded authority item. |
| PA-12 | Observation | The toolchain image is stepwise/mutable (NC-EXT-35/40); Gitea and GitHub provision the toolchain differently and never compare backends; the GitHub per-commit workflow has no XW-2 job, so "blocking per-commit" holds only on Gitea; "nightly" is weekly. | Record image provenance and a clean-build reference; state the cadence and authoritative CI, or add a GitHub per-commit job. |
| PA-13 | Minor | The evidence archive and this review are untracked; `build2/`, `build3/`, `.opencode/`, `opencode.json`, `tests/node/` are **not** gitignored, so `git add -A` would sweep them in. | Include the archive (recommended, N11) and the V&V/PA reviews, or amend INDEX/docs to name the CM-retained evidence; stage explicit paths only. |

## 3. Commit eligibility (as issued)

**Not commit-eligible as-is.** Committable after PA-01 (two locator edits),
PA-03 (workflow prerequisite), PA-09 (sprint-plan scope/review/order), PA-02
(bounded deviation or completed N10 review) and PA-13 (staging decision).
PA-04/05/06/07/08 may follow, but **any edit to `qemu_xw2_vv.sh`, the manifest or
the test binary invalidates the `172050` evidence** and requires a fresh run; the
old evidence must then be marked superseded, not silently retained.

Excluded from the increment: `build2/`, `build3/`, `.opencode/plugins/`,
`opencode.json`, `tests/python/*`, `tests/node/`,
`src/main/fsw_main_reqtest.c`, `docs/architecture/obcp-pluggable-interface-proposal.md`,
`docs/engineering/hermes-runtime-configuration.md`, `tools/scripts/xw3_*`,
`tools/docker/renode/frick-xw2-*.resc`,
`tools/scripts/target_req_manifest.json`, and pre-existing `docs/reviews/*` /
`docs/vv/evidence/*` other than the XW-2 archive.

## 4. Non-establishment confirmation (as issued)

N18 (remains **FAIL**, NC-EXT-198 / NC-PAL-01); any device, silicon, HIL or
flight-representative backend; a real reset, reset scope, or electrical/clock
independence; timing, WCET, jitter, interrupt latency or on-target cadence
(XW-B1 asserts counts only); mission timeout (8 s remains an ESC-7 placeholder);
T25 qualification under N10 (no hosted CI execution record exists);
certification/airworthiness (DO-178C, ECSS-Q-ST-80C, NASA NPR 7150.2/8739.8 are
context only); safety-case, security, residual-risk, waiver or release/flight
approval; SRS-REQ-074 target evidence (XW-G2); closure of any NC or sprint;
equivalence of the Gitea and GitHub toolchains/runs.

## 5. Confidence (as issued)

High for the technical result and PA-01; Medium for CI executability (static
inspection, not executed); Medium for cross-environment equivalence (no
evidence). Missing/inaccessible: Gitea job execution log; branch-protection
required-status configuration; clean-build provenance for
`frick-rtems-toolchain:local`; `LastTest.log` not archived; format exit statuses
not captured. Recheck after any script/manifest/test edit.

## 6. Disposition in this increment

| Finding | Status after this increment |
| --- | --- |
| PA-01 | **Corrected** — pointers repointed to the new authoritative run (see §7). |
| PA-02 | **Recorded deviation, approval outstanding** — bounded to exit-status gating; named owner is the software PA authority, which must approve it. Not self-approved. |
| PA-03 | **Corrected** — the Gitea job builds with `FF_USE_PUS_ASN1=OFF` and no longer runs `make pus-asn1`; the XW-2 image uses no PUS codec. |
| PA-04 | **Corrected** — the four `nm` counts are asserted, and "proven" is replaced by "asserted/observed" where the check is a count. |
| PA-05 | **Corrected** — `XW-G1..G5` table added to TS-7 §5. |
| PA-06 | **Corrected** — enforcement split stated in TS-7 §6. |
| PA-07 | **Corrected** — XW-2 results section carries a document ID/revision/date/owner; the SUPERSEDED statement is reconciled. |
| PA-08 | **Corrected** — format exit statuses captured; `LastTest.log` archived. |
| PA-09 | **Corrected** — scope, review record and row order fixed; sprint closures remain 0. |
| PA-10 | **Corrected** — QEMU timeout parameterised and the observed margin recorded in the run directory. |
| PA-11 | **Carried** — authority item, unchanged. |
| PA-12 | **Documented** — cadence and authoritative CI stated; image provenance remains NC-EXT-35/40. |
| PA-13 | **Corrected** — explicit path staging; archive and both reviews included. |

## 7. Evidence-of-record supersession (post-review)

PA-04 requires editing `qemu_xw2_vv.sh` (count assertions) and PA-03 requires
rebuilding the image without the ASN.1 backend. Both invalidate run
`20261101T172050Z`. A fresh run was therefore executed in the corrected
configuration and is the evidence of record; `172050` and its predecessors are
retained as superseded history in
`docs/vv/evidence/2026-11-01-xw2-target-final/`.

## 8. Archival provenance

- Executed as a configured subagent task in session
  `ses_e663d6753ffeUrSrkkLe6Fj7IK` (harness-reported), model
  `opencode/big-pickle`, on the exact working-tree content whose 28 SHA-256
  values were pinned in the task prompt.
- The reviewer's report was returned in chat and is transcribed here without
  adding findings, severities or approvals that were not issued.
- The reviewer verified the 38-file archive with `sha256sum -c` (38/38 OK) and
  independently recomputed all 28 supplied hashes.
