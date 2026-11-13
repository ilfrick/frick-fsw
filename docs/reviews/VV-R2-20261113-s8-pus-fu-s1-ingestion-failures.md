# Independent SW V&V R2 Review — S8-PUS-FU S1 Ingestion-Failure Test

| Field | Value |
|---|---|
| Review ID | VV-R2-20261113-S8-PUS-FU-S1-INGEST |
| Reviewer profile | Independent-context SW V&V R2; norms N9/N10/N22 |
| Repository | `/home/nicola/fsw/frick-fsw`, HEAD `e5f01d4` plus uncommitted candidate |
| Date | 2026-11-13 |
| Scope | `tests/unit/test_pus.c`, TS-1 rev 1.12, traceability mirror/mapping, verification-results, tool qualification, sprint plan, loop audit, prior R1/R3/R2/PA/R4 reports, and current r4 evidence package |
| Current test hash | `dc8ea3dbae907c43f5897daa799ad792165604eca8cddd119612b2b3a6d889c7` |
| Decision | Test logic is a coherent implementation-characterization test; current final evidence binding is **NOT VERIFIED** (one Major evidence-integrity finding). This is not qualification, approval, NC closure, or commit authorization. |

> This is an AI analysis artifact, not delegated approval authority. No safety, acceptance, qualification, release, or flight-readiness decision is made.

## 1. Review basis and limitations

Read the current candidate and controlled records, prior reviews, r4 README, retained r4 logs and manifest. Recounted TS-1 rows/status tokens and ran `sha256sum -c` against the current r4 manifest from repository root. I did not modify product artifacts, run the test binary, rebuild, execute Renode, or mutate production code. The earlier `VV-R2-20261112...` report reviewed an earlier test hash (`1e9c86e8…`), and is historical for that earlier candidate; it is not substituted for this review.

No external ECSS text was independently consulted. No target/HIL execution, formal test-tool qualification, or product-wide release assessment is claimed.

## 2. Positive findings

### 2.1 Expected results and characterization boundary

`test_verification_failure_via_ingest` in `tests/unit/test_pus.c` exercises five explicit inputs and checks observable outcomes:

1. Non-TC APID `0x011`, acceptance ack: characterized `FF_ERROR`, one TM(1,2) with the current symbolic illegal-APID code, echoed service/subservice and selected Request-ID octets, no TM(1,8), no acceptance-success TM, and no ping dispatch/reply.
2. Illegal APID `0x020`, ack zero: characterized `FF_ERROR` and no TM.
3. TC(17,9), completion ack: `FF_NOT_FOUND` and one TM(1,8) carrying the current unknown-subservice code.
4. TC(0,1), completion ack: `FF_NOT_FOUND`, current unknown-service code, and explicitly not the unknown-subservice code.
5. TC(17,9), ack zero: characterized `FF_NOT_FOUND` and no TM.

The current test comments and TS-1 rev 1.12 correctly distinguish controlled/requirement-derived evidence from implementation characterization. In particular, N22-TS1-23 is `gap` because the nonconfigured-APID rejection and acceptance-failure policy is not established by the cited SRS/ICD text (NC-EXT-13); N22-TS1-104 is `gap` because ack-zero observations conflict with the unresolved SRS-REQ-005 semantics (NC-EXT-04). The code value, 9-octet payload order/destination, and `FF_PusIngest` return codes are properly qualified as provisional, implementation-derived, or characterization. The R1/R3 correction re-review says the Major coverage-attribution correction is substantiated; its separate synchronization confirmation is also present. I agree with that bounded correction and do not infer a mission policy.

Assertions are explicit enough to be executable oracles for characterization. The helper checks the TM(1,x) service/subservice and selected payload octets; the case-1 packet-count/no-follow-on checks are appropriately scoped to the specified input, not a general PUS sequencing policy. A residual implementation-level limitation is that the helper does not itself validate all packet bytes/CRC or every output packet for every subcase; no broader packet-integrity claim should be made from it.

### 2.2 Test discrimination and repeatability evidence

The current `r4/mutation-check.log` binds its baseline source hash (`1eea40c0…`), test hash (`dc8ea3db…`), mutations, mutated hashes and restoration. It reports that changing the illegal-APID code selection fails the exact code assertion, removing the service-zero selection fails the unknown-service-vs-unknown-subservice assertions, and restored source yields 322 checks / 0 failures. These are relevant mutation controls for the stated claims. Earlier R2 probes on an earlier test revision additionally showed sensitivity to destination Request-ID octet, echoed subservice, and the ack-gate; because that report's test hash differs, I treat those as corroborating historical controls, not re-performance on this exact revision.

The r4 focused execution log binds the candidate executable and reports 322/0. The ordered r4 configure/build/CTest/focused-test logs identify the current test hash and show 57/57 registered tests passing. The separate required `make build` and `make test` logs use isolated `BUILD_DIR=build/evidence-20261112-pus-s1-make` and report exit 0. The default `build/native` path attempts failed CMake source-path validation due to its prior `/workspace` cache; those attempts are disclosed, retained, and not represented as product build/test failures. The supplied evidence says the pre-existing cache was preserved. I did not independently inspect ownership or reproduce the default-path failure.

### 2.3 Case counts, traceability, and bounded coverage

I counted 104 unique TS-1 case IDs. The status tokens give 76 covered/host-covered rows and 28 gaps; the row classes give 48 nominal, 51 OOB, and 5 mixed; scope gives 93 individual and 11 combined; environments give 101 SIL, 1 PIL, and 2 HIL. Thus rev 1.12's stated totals reconcile. N22-TS1-23 and N22-TS1-104 are gaps, not covered rows. The traceability YAML and human-readable traceability mirror now qualify the SRS-REQ-017 mapping as APID-value tailoring plus implementation characterization, with policy open under NC-EXT-13.

The CTest result is explicitly limited to 57 registered tests. `test_pus_s11_residual` is compiled as a test target but not registered with CTest and reportedly fails 10/1164 when directly invoked; NC-EXT-240 is identified and open. This is an accurate scope disclosure, not evidence that every declared executable passes. Do not summarize the result as “all test targets pass.”

### 2.4 Renode stale-ELF correction

The r4 package carries a successful Docker RTEMS image-build log and fresh-run identity. The recorded ELF hash is `e36aee30f381df32d9b085b11d616e35082a9650263e3b8614ed8f4004843a41`, size 4,133,912 bytes, mtime `2026-11-13 00:42:40 +0100`; `renode-run/identity.txt` repeats the identity and records toolchain/Renode image digests. The fresh Renode log identifies the rerun, and status/process/UART logs show container exit 0 and the PASS marker. This is distinct from the earlier stale ELF `55188f5a…`, which current results correctly label invalid historical evidence. I find adequate evidence that the current stated N18 PASS came from a newly built ELF, not the stale image. It remains emulator-only integration evidence; no physical target, RTEMS-on-hardware, HIL, timing, or RTPFS160ZT-fidelity claim follows.

## 3. Finding

### VV-R2-20261113-01 — Major — current r4 manifest no longer verifies the current candidate package

**References:** `docs/vv/evidence/2026-11-12-s8-pus-fu-s1-verification-failures/r4/assets.sha256`; r4 `README.md` §§ “Integrity binding”; `docs/vv/verification-results.md` § “Executed evidence” and “Review evidence and residuals”; `docs/status/loop-audit.md` N11/N20.

**Observed evidence:** I ran the manifest command recorded in the README from repository root. The manifest currently has 45 entries, but `sha256sum -c` reports four mismatches:

- `docs/reviews/PA-R3-20261113-s8-pus-s1-ingestion-failure-review.md`
- `docs/reviews/README.md`
- `docs/status/loop-audit.md`
- `docs/vv/verification-results.md`

The manifest also does not list `r4/gate-make-build-isolated.log` or `r4/gate-make-test-isolated.log`, although the current README/results rely on these logs for the required Make-target evidence. Thus the current r4 manifest does not bind all current documents/reports and all newly reported gate evidence, contrary to the broad “all retained log files” / “current” binding claims. PA-R3's report itself records the correct sequencing issue: its earlier 45/45 verification applied to a prior package; the report update and later Make-target evidence require rebinding and a new check. The loop audit N11 also says final rebinding/verification remains pending, but verification-results and README wording still describe the manifest as generated/verified without making this subsequent failure equally prominent.

**Impact:** The configure/build/test and fresh-Renode logs remain meaningful retained execution records, and this finding does not imply those executions failed or used the wrong artifact. However, the claimed integrity closure for the *current evidence set* is not established. It weakens N11 reproducibility/configuration traceability and prevents a positive assurance conclusion based on the package as presently bound.

**Required disposition:** Regenerate the manifest only after finalizing the included reports/index and controlled result/audit text; include both successful isolated Make logs and all applicable current artifacts; rerun `sha256sum -c` from repository root and retain the exact command/result; then reconcile README, verification-results, loop-audit, and PA re-review status to that verified manifest. Any subsequent change to a bound file requires rebind and re-verification. This finding is open and blocks an overall positive evidence-integrity/commit-readiness recommendation; it does not block the narrower technical conclusion about the test oracle.

## 4. Other gate/status dispositions

- **N10/T3:** `tool-qualification.md` correctly states NOT FULLY QUALIFIED and records the current `test_pus.c` hash. This review does not grant qualification. The test-only change does not modify a tool under `tools/`; T3 qualification/PA disposition remains separate.
- **Pinned format:** full pinned clang-format 18 fails on the three identified unchanged initializer locations. The changed `test_pus.c` passes pinned format individually. NC-EXT-244 is disclosed, open, and deferred under the project's N21 non-functional finding process. The full-repository format gate is a failure, not a pass; the record does not hide or misstate it. Any authority requiring the full pinned-format gate to pass remains applicable.
- **CTest registration:** 57/57 is registered tests only. NC-EXT-240 remains open; no closure or waiver is inferred.
- **Other open items:** NC-EXT-241 (S1 layout authority), NC-EXT-242 (TS-1 ordering), NC-EXT-243 (sequence counter synchronization), NC-EXT-244 (format discrepancy), NC-EXT-04/13/18 and NC-EXT-47 remain open as reported. No NC closure is recommended here.
- **Sprint plan:** the plan's S8-PUS-FU row describes the prior bounded implementation milestone as complete while the current verification-results/loop-audit describe this new S1 test/evidence increment as an in-progress candidate. Those statements can coexist if the scopes remain explicit; do not use the historical implementation-complete row to imply that this current review/evidence cycle or the larger sprint is closed.

## 5. Conclusion

**Test-level assessment:** The current S1 test has explicit expected observations, properly labels unresolved behavior as characterization, and uses useful mutation controls. The test-spec gap classifications and 104 / 76 / 28 totals are consistent. The CTest and Make-target evidence is clearly scope-bounded, and the updated fresh-ELF Renode evidence adequately addresses stale-image attribution for the N18 run.

**Current evidence package:** **NOT READY for an overall positive evidence-integrity recommendation** until VV-R2-20261113-01 is corrected and independently rechecked. R4 and V&V correction re-reviews and applicable PA confirmation also remain separate project gates as identified in the current records. No product defect is identified by this bounded review; no T3 qualification, NC closure, sprint completion, commit/push authorization, release approval, safety acceptance, or flight-readiness conclusion is made.

| Facts | Evidence | Confidence |
|---|---|---|
| Candidate source hash, test content, current status/counts, manifest outcomes and listed log contents | Current working tree and cited r4 files | High for directly inspected facts |
| Test discrimination | Current mutation log; historical R2 probes clearly marked as earlier revision | Medium-high; no current-hash mutation re-execution performed by this reviewer |
| Fresh N18 ELF use | Build log, ELF identity, fresh-run status/process/UART records | High that the recorded pass used the new ELF; no independent Renode execution |
| Overall adequacy for qualification/release | Not assessed | Not claimed |

## Review record

| Rev | Date | Change |
|---|---|---|
| 1.0 | 2026-11-13 | Fresh R2 review of current candidate and r4 package; test assessed as characterization evidence; Major current-manifest mismatch identified; no approval or closure. |
