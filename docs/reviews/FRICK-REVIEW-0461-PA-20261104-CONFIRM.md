# FRICK-REVIEW-0461 — Independent PA/N20 closure confirmation (2026-11-04)

| Field | Value |
| --- | --- |
| Document ID | **FRICK-REVIEW-0461** |
| Profile | **PA / N20** (Senior SW Product Assurance), independent sub-agent session |
| Reviewer session | `ses_e5a42c283ffe4XAeNLegyqUYzn` (observed in `OPENCODE_SESSION_ID`) |
| Date | 2026-11-04 |
| Repository | `/home/nicola/fsw/frick-fsw` (branch `main`) |
| Base commit | `41dcbe21435e14b5aaff06c63d9df6f0de04df54` — all reviewed content is uncommitted |
| Manifest | `docs/vv/evidence/2026-11-04-re-review/candidate-manifest.sha256` (192 entries) |
| Manifest sha256 | `7e47fed14e5a4d5530919ea53e69e775c1691c37783d8fc98345398d1f3a3125` — recomputed by me, matches the value supplied with the task |
| Manifest verification | `sha256sum -c` → **exit 0, 192 OK, 0 FAILED, 192 lines**, run first and re-run after all other checks |
| Scope | Closure confirmation of **PA-0459-01, PA-0459-02, PA-0459-O-01, PA-0459-O-02, PA-0459-O-03**; new-items review of the PA-0459 corrections (README §2.3, `gate11` ordering, manifest coverage); configuration-management and status consistency sweep |
| Out of scope | Full re-review; re-litigation of PA-0452..0458 findings; N18/target/HIL validation; human qualification; tool-qualification decisions; release/commit authority; `third_party/n7s-obcp`; `build*/`, `.opencode/`, `opencode.json`, `tests/node/`; the content of `docs/vv/evidence/2026-11-02-n22-test-evidence-mapping.md` |

**Verdict: PASS WITH COMMENTS — 0 Blocker, 0 Major, 1 Minor, 5 Observations.**

All five PA-0459 items are **RESOLVED**. The corrections are present in the manifested
bytes, the gate ordering is structurally sound, and the candidate manifest reconstructs
the build/test/licence-check inputs. One **new** document-control defect was found (TS-5
header date, Minor) and five non-functional observations are recorded. No NC is closed,
no sprint is closed, and this report is **not** release approval.

## 1. Independence declaration

- I am an independent sub-agent session (`ses_e5a42c283ffe4XAeNLegyqUYzn`), separate from
  the authoring/correction work and from every earlier reviewer (0458/0459:
  `ses_e5a4ecb0…`; 0456/0457: `ses_e5a62463…`; 0454/0455: `ses_e5a708cc…`). I did not
  author, edit or direct any part of this candidate and have no schedule, financial or
  reporting-line interest in its acceptance.
- I did not modify tracked content. **This report is the only file I created.** No commit
  or push was performed or authorised.
- I did not close, downgrade or dispose any NC, finding or risk, and I did not qualify
  any tool. The verdict is a recommendation to the human authority, who alone may accept
  it. This is a recorded session boundary plus a declaration, not a formal N9
  independence certificate.
- **Checks I re-executed myself**, on the manifested bytes, from outside the build tree:
  - `sha256sum -c candidate-manifest.sha256` → exit 0, **192 OK** (twice: before and
    after the review);
  - `build/gate11/tests/test_evs_hs_health` → `checks=110 failures=0`, exit 0;
  - `build/gate11/tests/test_frozen_utc_seam` → `checks=4325 failures=0`, exit 0;
  - `python3 tools/scripts/license_check.py` → exit 0, `... check: OK`.
- **Limits of this pass.** I did **not** rebuild from a clean configure, re-run CTest,
  format, smoke, Lean, trace, RTEMS or cppcheck. For those I audited the retained logs,
  exit-code files and artefact timestamps. Binary-to-source binding therefore rests on
  the retained build log plus the manifest, not on a fresh build of my own. This is a
  narrow closure confirmation, not a full re-verification.

## 2. Findings

### 2.1 New finding

| ID | Severity | File:line | Finding | Required action |
| --- | --- | --- | --- | --- |
| **PA-0461-01** | **Minor** (document control) | `docs/vv/test-spec/ts-5-core-services.md:7-8` | The header still reads **Revision 1.14 / Date 2026-10-21**, while its own history rows **1.13 and 1.14 are dated 2026-11-04** (lines 431–432). Every other controlled test spec in the tree follows the convention "header Date = latest revision date" (`ts-1` 1.9/2026-10-17, `ts-2` 0.9/2026-10-16, `ts-4` 0.8/2026-10-15, `ts-6` 1.7/2026-10-19, `ts-9` 0.4/2026-10-16; `ts-7` 1.1/2026-11-02). This is not a new convention: the previous TS-5 bump changed both fields together (commit `8c84c7f`: Revision 1.5→1.12 with Date 2026-10-19→2026-10-21). A reader or tool that dates the specification from its header will treat the N22-TS5-47 and NC-EXT-236 corrections as three weeks older than they are, and the same document is cited as evidence by `CANDIDATE.md:26`. **No claim or gate result depends on the header date**; the revision *number* is correct and the correction rows are accurate. | Set `Date` to **2026-11-04** (or the true revision date) when the next TS-5 edit is authorised, and add a line to the revision row stating that 1.13/1.14 were metadata-only for the case set. Do not back-date the history rows. |

### 2.2 Observations (non-functional)

| ID | Class | Location | Observation | Effect |
| --- | --- | --- | --- | --- |
| **O-0461-01** | Observation (evidence precision) | `docs/vv/evidence/2026-11-04-re-review/README.md:87-89` | The sentence "**every** `build/gate11` binary is dated 09:43:18" is an approximation: 28 executables are stamped 09:43:18 and **24 are stamped 09:43:19** (last link `test_lwip_netif` at 09:43:19.861); the two 09:42:53 executables are CMake compiler-probe artefacts, not product binaries. The sentence's *purpose* is unaffected: the last link (09:43:19.86) still precedes `gate11-ctest.log` (09:43:42) and `gate11-new-tests.log` (09:43:43). Separately, the bullet "`cmake --build build/gate11` … prints `ninja: no work to do`" has **no retained transcript**; the no-relink fact is proven by binary mtimes and the CTest log timestamp, not by that output. | None. The ordering conclusion is true and independently checkable; only the wording overstates precision and cites an unretained command. |
| **O-0461-02** | Observation (status record) | `docs/status/handover.md:1600-1601` (rev 3.63/3.64) | Handover 3.64 says "`build/gate11` is the authoritative directory" and 3.63 says the clean run is in `build/gate10`, but neither row states that **`build/gate10` is superseded**. README §2.2 and `verification-results.md` do say so. A reader resuming from the handover alone (the documented resume point) can still take `gate10` as current. | None; no claim depends on it. Add "gate10 superseded by gate11 (F-0458-02)" to the 3.64 row at the next authorised handover edit. |
| **O-0461-03** | Observation (review index, pre-existing) | `docs/reviews/README.md` | The index rows added for **0458/0459 are correct** (session IDs, verdicts and "manifest 174 OK" match the reports), so PA-0459-O-03 is closed. But the wider index is not a reliable register: **0437** exists (`2026-10-24-ncext18-s6-vv-review.md`) with no row; **two distinct reports share ID 0438** (`2026-10-24-ncext18-s6-vv-delta-review.md` and `2026-10-31-root-cause-audit-study-provenance-pa-review.md`) and **two share ID 0439** (`FRICK-REVIEW-0439-VV-NC-EXT-235.md` and `2026-10-31-renode-verdict-test-vv-review-0439.md`), with only one row each; **0460 is not indexed** (and 0461 will need a row). Pre-existing and not caused by this delta. | None for the candidate. The coordinator should renumber/de-duplicate 0438/0439, index 0437 and 0460, and add the 0461 row when accepting this report. |
| **O-0461-04** | Observation (carried from O-0460-01) | `tests/unit/test_evs_hs_health.c:17`; `ts-5-core-services.md:177`; `verification-results.md:313-318`; `sprint-plan.md:323`; `handover.md:1599` | The same listener→sink coverage is cited as `test_r3_gap.c:523-575` in the test header / README §2.3 and as `520-571` in the other records. Both ranges contain the listener adapter and every `rx_find(5,1/2/3)` assertion (554/561/566); `520-571` omits only the final EVS-log assertion at 573. | None. No gate or product claim depends on the end line. Still worth harmonising on the full `523-575` range. |
| **O-0461-05** | Observation (carried from O-0460-02) | `docs/vv/evidence/2026-11-04-re-review/README.md` §2.2; `gate11-smoke.log` | The smoke log text is three lines (`[smoke] building`, boot window, `PASS`) and does not itself name `build/gate11-smoke`. The separate-directory fact is proven by the artefacts — `build/gate11-smoke/frick_fsw` at 09:44:43, the untouched `build/gate11` binaries, and the `BUILD_DIR` handling — not by the log wording. | None. The claim is true and structurally substantiated. |

No new functional defect was observed in the exact bytes. I did not manufacture a finding
to offset these observations, and I did not suppress a defect I believe is real.

## 3. Per-item closure of PA-0459

| Item (from FRICK-REVIEW-0459) | Status | Independent evidence |
| --- | --- | --- |
| **PA-0459-01** — `CANDIDATE.md:27` cited `tool-qualification.md` rev 1.3 against a manifested rev 1.4 | **RESOLVED** | `CANDIDATE.md:27` now says "rev 1.5, **T3 NOT FULLY QUALIFIED** for this candidate (status unchanged from rev 1.3; rev 1.4/1.5 only re-bind the script hashes)". The manifested header is `tool-qualification.md:7-8` = Revision **1.5**, dated 2026-11-04, and row T3 (`:26`) still says NOT FULLY QUALIFIED for the candidate. The 1.5 history row (`:255`) re-binds `test_evs_hs_health.c` to `23535db3…6035` and records 110 checks. The identity table no longer contradicts the bundle. |
| **PA-0459-02** — unbound `src/main/fsw_main_reqtest.c` in the N13/cppcheck input sets; "cannot affect any gate result" too strong | **RESOLVED** | `candidate-manifest.sha256:39` binds `src/main/fsw_main_reqtest.c` = `21cfaa0e…1277`, which equals the working-tree hash. The rationale at `CANDIDATE.md:90-99` is now precise: bound as a manifest-only input, not compiled (`#error` guard at `fsw_main_reqtest.c:44-45`; no build-system reference), but enumerated by `license_check.py` (`MANAGED_ROOTS` includes `src`) and scanned by `cppcheck_scan.sh` (`PATHS=(/workspace/src /workspace/config)`), "so that base commit + manifest reconstructs those gate inputs; it cannot change a build, CTest or execution result because it is never linked". I re-ran `license_check.py` → exit 0. The overstated sentence is gone. |
| **PA-0459-O-01** — live "52/52, Rev 1.121" label | **RESOLVED** | `sprint-plan.md:101` now reads "the current candidate gate is 52/52 **as of Rev 1.121** and is **still 52/52 at Rev 1.125**". The claim matches `gate11-ctest.log` (52/52, exit 0) and the plan header (Rev 1.125). |
| **PA-0459-O-02** — rev 1.3 pointers did not state the status is unchanged | **RESOLVED** | `verification-results.md:279` now says "(`docs/vv/tool-qualification.md` rev 1.3, the revision of record; **unchanged at rev 1.5** except for re-bound script hashes)"; `sprint-plan.md:322-323` now says "see `tool-qualification.md` rev 1.3, **unchanged at rev 1.5**". Retaining 1.3 as the revision of the T3 determination is accurate: rev 1.5 changes only the hash binding, not the status. |
| **PA-0459-O-03** — review index rows missing for 0458/0459 | **RESOLVED** | `docs/reviews/README.md:5` (0458) and `:6` (0459) exist with the correct session IDs (`ses_e5a4ecb0fffeXKbgIlYxvKkKwE`, `ses_e5a4ecb0effetawalmRd4UDzgi`), verdicts (PASS WITH COMMENTS, 0 Blocker/0 Major) and "manifest 174 OK". Both match the reports they index. (Wider index gaps are pre-existing → O-0461-03.) |

**New-items review of the PA-0459 corrections.** The corrections themselves introduce no
new contradiction:

- README §2.3 rows for PA-0459-01/-02/-O-01..-03 match what is actually in the tree, and
  §2.2 correctly assigns authority to `build/gate11` and marks `gate7`–`gate10` superseded.
- `verification-results.md` keeps rev 1.3 as the T3 revision of record and adds the
  "unchanged at 1.5" annotation; `sprint-plan.md` and `handover.md` do the same. No live
  "T3 QUALIFIED" claim remains in the bundle, the plan, the handover or the result record.
- The `gate10`-vs-`gate11` distinction is stated in README §2.2, `verification-results.md`
  and `CANDIDATE.md`; only the handover omits it (O-0461-02).
- Manifest coverage was re-tested end-to-end, not read from the README claim (see §4).

## 4. Gate-ordering and manifest verification (item 3)

### 4.1 Ordering of the authoritative gate

| Artefact | Timestamp (2026-11-04) | Verified content / exit |
| --- | --- | --- |
| `gate11-configure.log` | 09:43:07 | exit 0 (`GATE11_CONFIGURE_EXIT=0`) |
| `gate11-build.log` | 09:43:19 | 403 lines, `[402/402]`, **0** `warning:` lines, exit 0 |
| last executable link | **09:43:19.861** (`tests/test_lwip_netif`) | 28 executables at 09:43:18, 24 at 09:43:19 |
| `gate11-ctest.log` | **09:43:42** | `100% tests passed, 0 tests failed out of 52`, exit 0 |
| `gate11-new-tests.log` | **09:43:43** | `test_frozen_utc_seam` **4325/0**, `test_evs_hs_health` **110/0**, exit 0 |
| `gate11-format-check.log` | 09:43:47 | exit 0 (command-only log, PASS is empty output) |
| `gate11-cppcheck.log` | 09:44:07 | Cppcheck 2.13.0, **163** findings, exit 0 (advisory, GAP-A4) |
| `build/gate11-smoke/frick_fsw` | 09:44:43 | separate build directory |
| `gate11-smoke.log` | **09:45:00** | `[smoke] PASS (banner + 7 applications registered)`, exit 0 |

The central ordering claim is **confirmed**: every CTest/test-execution log post-dates the
last link in `build/gate11`, and smoke was built in `build/gate11-smoke`, so nothing
reconfigured or relinked the authoritative directory after its CTest. The precision caveat
is O-0461-01.

I independently re-executed the two gate11 test binaries: `110/0` and `4325/0`, exit 0,
matching the retained log and the recorded script hash
`23535db30a28bc15f90ecf1679b0195edc835bfada9fa5f6b044e4c851626035`.

**Disclosure.** `build/gate11` is not in the manifest and contains post-gate byproducts
written by later confirmation runs — `.ninja_log` (09:47:21), `Testing/Temporary/LastTest.log`
and `sb-evs-structural-sweep.csv` (09:49:03). The **binaries were not relinked** (newest
executable 09:43:19.861) and the retained gate logs are separate copies in the evidence
directory. The mutation does not affect manifest verification or any gate result.

### 4.2 Manifest coverage

- 18/18 modified tracked files are bound, including all nine modified docs, both modified
  PAL sources, `tests/CMakeLists.txt` and the six modified test sources.
- All untracked files inside the licence/cppcheck roots that are gate inputs are bound:
  the eight `tests/python/*.py`, the nine `tools/scripts/*` (including the `.mjs` helper),
  the two Renode `.resc` files, the two new unit tests and `src/main/fsw_main_reqtest.c`.
- The only unbound managed-suffix files are `build2/` and `build3/` outputs (not managed
  roots, regenerable) and three files under `docs/` (`cppcheck_scan.sh` in a historical
  evidence directory and two `.resc` files), which no executed gate scans. This is
  disclosed and consistent.
- `candidate-manifest.sha256` and `manifest-verify.log` are the only files in the package
  absent from the manifest, by design (a manifest cannot contain its own hash).
- The package README and CANDIDATE are themselves bound (`:111-112`), so the review basis
  is inside the identity envelope.

**Conclusion:** base commit + manifest reconstructs the build/test/licence-check
configuration that produced the recorded gate results. This closes the PA-0459-02 concern.

## 5. Status and consistency sweep

| Fact | Where checked | Result |
| --- | --- | --- |
| 52/52 CTest, exit 0 | `gate11-ctest.log`; README §3; `CANDIDATE.md`; `sprint-plan.md:101` | Agrees |
| 4325 / 110 checks, 0 failures | `gate11-new-tests.log`; re-executed both binaries | Agrees |
| Script hash `23535db3…6035` | `sha256sum tests/unit/test_evs_hs_health.c`; manifest; `ts-5:177`; `tool-qualification.md:255` | Agrees |
| 402/402 build, 0 warnings | `gate11-build.log`; exit file | Agrees |
| cppcheck 163, advisory | `gate11-cppcheck.txt` (163 lines); exit file | Agrees |
| Smoke PASS, separate dir | `gate11-smoke.log`; `build/gate11-smoke/frick_fsw` | Agrees |
| `build/gate11` authoritative, `gate10` superseded | README §2.2; `verification-results.md`; `CANDIDATE.md` | Agrees (handover omission → O-0461-02) |
| T3 **NOT FULLY QUALIFIED** | `tool-qualification.md:26,255`; `CANDIDATE.md:27`; `sprint-plan.md:322`; `handover.md:1598-1600`; `verification-results.md:279`; README §5 | Agrees; no live QUALIFIED claim |
| N18 **FAIL** | `verification-results.md:301`; README §5; `handover.md:38,70,115` | Agrees |
| NC-EXT-19 / 236 / 237 open | `sprint-plan.md:323-324`; `verification-results.md:305-314`; `ts-5:177` | Agrees; 236/237 have owner role + acceptance authority (Nicola), no acceptance date |
| No NC closed, no sprint closed | `CANDIDATE.md`; README §5; `verification-results.md:316` | Agrees; no closure claim found |
| Review index 0458/0459 | `docs/reviews/README.md:5-6` | Correct (wider gaps → O-0461-03) |
| Test-spec header metadata | `ts-5:7-8` vs rows `:431-432` | **Inconsistent → PA-0461-01** |

## 6. Residual open items

1. **PA-0461-01 (Minor)** — TS-5 header Date 2026-10-21 vs revision rows dated 2026-11-04.
2. **O-0461-01..05** — evidence-text precision, handover `gate10` annotation, review-index
   gaps/collisions, mixed `test_r3_gap.c` line ranges, smoke-log wording. None functional.
3. **Carried, unchanged:** T3 is **NOT FULLY QUALIFIED**; NC-EXT-19, NC-EXT-236,
   NC-EXT-237, NC-EXT-198/199, NC-PAL-01, NC-EXT-207/208/211 remain **open**; **N18 is
   FAIL**; the evidence-mapping document (`docs/vv/evidence/2026-11-02-n22-test-evidence-mapping.md`)
   is still an unreviewed content change that must be reviewed or reverted before any
   commit; hardware and human reviews remain deferred under NCs.
4. **Coordinator actions:** index 0460 and 0461; fix PA-0461-01 at the next authorised
   TS-5 edit; de-duplicate review IDs 0438/0439 and index 0437.

## 7. Explicit statements and confidence

**No NC is closed** by this candidate or by this review. **No sprint is closed.** This
report is **not release approval**, **not** a flightworthiness or safety statement, **not**
a tool qualification, **not** certification and **not** an independence certification. It
does not authorise a commit or push. T3 remains NOT FULLY QUALIFIED and N18 remains FAIL.

```text
Decision/product and configuration: frick-fsw working tree on base 41dcbe2,
  manifest 7e47fed1…3a3125 (192 entries, verified exit 0, 192 OK)
Claim type: Fact (verification results) + Recommendation (closure verdict)
Applicable baseline and approved tailoring: project review norms N10/N11/N20/N22 as
  recorded in the candidate bundle; ECSS Q-ST-80C/E-ST-40C context only — no
  certification or qualification claim
Evidence consulted (issuer, ID/revision, artifact/version, date):
  bound candidate manifest and evidence package 2026-11-04-re-review; gate11 logs and
  exit files; tool-qualification.md rev 1.5; sprint-plan.md Rev 1.125; handover.md
  rev 3.64; verification-results.md GATE-20261104; TS-5 rev 1.14; reviews index;
  direct re-execution of both new test binaries and license_check.py (2026-11-04)
Cross-check and reconciliation: manifest hash recomputed → identical; binary hashes
  match the manifest; 110/4325 reproduced; 52/52 CTest agrees with the log; gate
  ordering reconciled from mtimes; all five PA-0459 items traced to live text
Confidence: High for the five closures and the gate ordering; Medium for "no other
  defect exists" (narrow scope, no fresh full rebuild, retained-log dependence)
Basis and detection limits: direct artefact inspection and hash verification; no
  clean rebuild/CTest/format/smoke/Lean/trace/RTEMS/cppcheck re-run in this pass;
  sampling was targeted, not exhaustive
Missing/inaccessible evidence: fresh rebuild transcript for gate11 (by design of
  this narrow pass); no access to the authored change history beyond git objects;
  nothing material known to be withheld
Assumptions and expiry/recheck date: the working tree is unchanged since the
  manifest was generated; recheck before any commit, and immediately if any
  manifested file or build/gate11 binary is touched
Consequences if wrong: a closure reported here could be based on a superseded
  artefact; mitigated by the manifest hash match and direct binary re-execution
Required independent human reviewer/authority: Nicola (project/Software PA) for
  acceptance of this report and for any NC/sprint/release disposition
```

*End of FRICK-REVIEW-0461. This report was written by an AI assurance session; it is a
recommendation to the human authority, not an approval.*
