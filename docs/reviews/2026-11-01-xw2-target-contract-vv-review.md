# Independent SW V&V review — XW-2 target contract (SRS-REQ-073/075)

| Field | Value |
| --- | --- |
| Review ID | `VV-REVIEW-XW2-20261101` |
| Date issued | 2026-11-01 |
| Date archived | 2026-11-02 |
| Reviewer profile | Senior Aeronautics and Space SW Verification & Validation (SW V&V) Engineer (R2 profile) |
| Configured subagent | `aeronautics-space-sw-vv-engineer` |
| Harness session | `ses_e67b8290bffe5Edah5mlqBsL3g` (reported by the task harness; state `completed`) |
| Model / route | `opencode/big-pickle` (Big Pickle, provider id `opencode`, $0 free route) |
| Scope | XW-2 target-contract increment: TS-7, `qemu_xw2_vv.sh`, `xw2_manifest.json`, CMake/CI wiring, T25 register entry, `verification-results.md` cycle record, `traceability.yaml` mapping, sprint-plan Rev 1.116 |
| Base HEAD | `65e4a91` |
| Verdict | **PASS WITH COMMENTS AND OPEN FINDINGS** |

> **Authority statement.** This is an AI expert-persona review, not a human
> credential, qualification, certification, or approval. It does not approve
> gates, close non-conformances, authorize release or flight readiness, or waive
> any applicable requirement. Human approval is mandatory for all decision gates.

## 1. Summary (as issued)

The XW-2 target-contract increment **successfully demonstrates** the middleware's
obligations under the abstract external-watchdog contract (SRS-REQ-073/075) on a
test-only RTEMS/QEMU image with a validated test double. The 13 cases pass, the
no-wrap control fails as expected, wrapper segregation is proven, and the
non-claims are explicitly documented.

The following items must be resolved before this evidence can be used to satisfy
criticality-B V&V gates:

1. **T25 tool qualification (N10)** — independent R2 review and a hosted CI
   execution record are required. The current Gitea blocking gate runs an
   unqualified tool.
2. **N18 functional validation** — remains FAIL; this increment does not close it.
3. **Cross-CI reproducibility** — Gitea (pre-seeded image) vs GitHub (RSB
   bootstrap) is not shown to be equivalent.
4. **Toolchain image provenance** — `frick-rtems-toolchain:local` is assembled
   stepwise; a clean build is pending (NC-EXT-40).
5. **SRS-REQ-074 / XW-G2** — the pre-reset report ≤250 ms obligation has no
   target evidence; an authority disposition is required.

## 2. Claim-by-claim assessment

| Claim | Supported? | Basis |
| --- | --- | --- |
| C1: middleware C1–C7 satisfied on the RTEMS build | YES (bounded) | 13/13 cases PASS on a test-only image with a validated test double; the no-wrap control discriminates the substitution |
| C2: SRS-REQ-073 verified at middleware level | YES (bounded) | 12 cases; host XW-1 + target XW-2 both PASS |
| C3: SRS-REQ-075 verified at middleware level | YES (bounded) | XW-T7 PASS; at-most-once device assertion lives in the model; software-guard authority interpretation open |
| C4: this PASS constitutes N18 | NO | Explicitly NOT N18; N18 remains FAIL (NC-EXT-198/NC-PAL-01) |
| C5: establishes device/reset scope/electrical independence/timing/mission timeout | NO | Explicit non-claims, TS-7 §5 and `verification-results.md` |
| C6: T25 qualified | NO | `tool-qualification.md` T25: NOT QUALIFIED |
| C7: Gitea blocking gate carries qualified evidence | NO | Gate executes an unqualified tool on a mutable pre-seeded image (NC-EXT-35/40) |
| C8: sprint-plan row 1.116 is correct | YES | Rev 1.116; the 1.114/1.116 row-repair is a genuine repair, not a renumbering |

## 3. Positive findings F-01…F-10 (as issued)

The original review issued these in a table with columns **# | Finding |
Evidence**. **No severity column and no per-finding disposition column were
issued** for F-01…F-10; they are restated here without invented columns.

| ID | Finding | Evidence |
| --- | --- | --- |
| F-01 | All 13 required cases executed and reported PASS on the test-only image | `verification-results.md`: `COVERAGE: 13/13 required cases`, `WHOLE-RUN: PASS`, `RESULT: PASS` |
| F-02 | No-wrap control reproduced the expected watchdog-init panic and was rejected by the component verifier (non-zero exit); control shape check passed (only XW-T8 passes; XW-B1 absent) | `verification-results.md`: "no-wrap control: EXPECTED FAILURE reproduced: watchdog init panic + component verifier exit 1" |
| F-03 | Wrapper segregation proven by `nm`/`objdump`: 3 wrapper call sites / 0 real-hook call sites in the positive image; 0 wrappers / 3 real-hook call sites in the flight-shaped image | `qemu_xw2_vv.sh` L129–139; `nm-segregation.txt` |
| F-04 | The oracle is the SUT's own configured timeout (`FF_GetMissionCfg()->watchdog_timeout_ms`) as a test parameter; no mission value is asserted; 8 s remains an ESC-7 placeholder | TS-7 §1.3, §2 row "Clock", §6 "Mission value" |
| F-05 | Host counterpart (XW-1) passes: `test_extwd_model` 73 checks, `test_extwd_middleware` 49 checks, full host CTest 50/50 | `verification-results.md` XW-1 host table |
| F-06 | Upstream gates green: host CTest 50/50, `verify-lean` PASS, `trace` PASS | `verification-results.md` |
| F-07 | Non-claims explicitly documented: not a device, not N18, no independence, no timing/WCET, no reset scope, no electrical/clock independence; N18 remains FAIL (NC-EXT-198, root cause NC-PAL-01) | TS-7 §1.2/§5, `verification-results.md` authority block |
| F-08 | Sprint-plan row 1.116 corrected (no concatenation artifact) | `docs/planning/sprint-plan.md` Rev 1.116 |
| F-09 | Traceability updated: `traceability.yaml` maps SRS-REQ-073/075 → `ci:xw2-target` | `traceability.yaml` L681–706 |
| F-10 | Gaps declared in the manifest: XW-G1 (fatal arm/feed policy), XW-G2 (pre-reset report ≤250 ms — no real reset), XW-G3 (cadence/jitter/margin authority inputs), XW-G4 (no requirement for withholding feed), XW-G5 (reset scope/electrical independence require a device) | `xw2_manifest.json` gaps array |

## 4. Findings requiring attention F-11…F-20 (as issued)

The original table columns were **# | Finding | Severity | Rationale**; there was
no per-finding disposition column. Dispositions appeared only in the separate
§8 required-actions table (section 5 below).

| ID | Finding | Severity | Rationale (as issued) | Evidence cited |
| --- | --- | --- | --- | --- |
| F-11 | T25 tool is NOT QUALIFIED per N10; qualification requires an independent delta review and a hosted CI execution record; no independent review record for T25 was found in `docs/reviews/` | Major (gate) | N10 requires qualification before an evidence claim at criticality B; the register states NOT QUALIFIED | `docs/vv/tool-qualification.md` T25 entry |
| F-12 | No independent V&V review of the T25 delta recorded; first execution 2026-11-01 is the initial qualification event | Major (process) | N10 procedure steps 1–5; a change invalidates evidence until requalified | `tool-qualification.md` T25 and procedure |
| F-13 | The Gitea `xw2-target` blocking gate runs in the pre-seeded `frick-rtems-toolchain:local` image with mutable provenance (NC-EXT-35, NC-EXT-40), assembled stepwise rather than by a clean `docker build` | Major (reproducibility) | T18 note: stepwise-assembled image; clean build pending (NC-EXT-40) | `.gitea/workflows/ci.yml` job `xw2-target`; `tool-qualification.md` T18 |
| F-14 | The GitHub nightly `xw2-target` bootstraps RTEMS via RSB (`setup_rtems.sh`) each run; this does not reproduce the Gitea pre-seeded image; cross-environment consistency is undemonstrated | Medium | Two CI paths for one gate; no evidence of bit-identical or behaviour-identical images | `.github/workflows/nightly.yml` job `xw2-target` |
| F-15 | No N18 evidence — explicitly acknowledged; N18 remains FAIL (NC-EXT-198/NC-PAL-01); the increment does not advance N18 closure | Major (program) | N18 is in force per `AGENTS.md` | `verification-results.md` authority block; TS-7 §5 |
| F-16 | Test fidelity is limited to the middleware contract: no device backend, no real reset, no reset scope, no electrical/clock independence, no timing/WCET, no mission timeout | Medium (claim boundary) | TS-7 §2 row "Cannot establish"; §5 non-claims | TS-7 §2, §5 |
| F-17 | The control image build is required to pass (the script exits if the no-wrap build fails); the control is a separate CMake target, not the same binary unwrapped | Minor (process) | `qemu_xw2_vv.sh` L93–106 enforce control-build success | `qemu_xw2_vv.sh` |
| F-18 | XW-G1…XW-G5 remain open; XW-G2 (pre-reset ≤250 ms) is a direct SRS-REQ-074 obligation with no real reset; XW-G5 requires a device | Medium (requirements) | Correctly declared; authority disposition for SRS-REQ-074/075 closure required | `xw2_manifest.json` |
| F-19 | The runner prefers the Docker image if present, else the host toolchain; local and Gitea runs may use different backends; behavioural equivalence is unverified | Minor (configuration) | `qemu_xw2_vv.sh` L52–74; backend recorded but equivalence unverified | `qemu_xw2_vv.sh` |
| F-20 | QEMU exit code 124 is accepted (timeout); pass/fail relies on the verifier, not the process exit | Minor (oracle) | `verification-results.md`: `QEMU-PROCESS: exit 124 (accepted)`; verifier logic must be independently reviewed | `verification-results.md` |

## 5. Required actions (as issued, heading "§8 Required Actions (for Human Decision Authorities)")

The original table columns were **Action | Owner | Gate**. It was **not**
titled "before commit/push" and carried **no blocking yes/no column**.

| Action | Owner | Gate |
| --- | --- | --- |
| Conduct an independent R2 V&V review of T25 (source, procedure, CI execution) and update `tool-qualification.md` | V&V engineer (R2 profile) | N10 qualification |
| Execute T25 in both Gitea and GitHub CI with clean, reproducible toolchain images; record evidence | CI/CD engineer | N6/N10 |
| Resolve N18 (Renode `RENODE_TEST: PASS` on PolarFire SoC) — root cause NC-PAL-01 | FSW engineer + PA | N18 / norm N18 |
| Obtain an authority disposition on the SRS-REQ-074 pre-reset report deadline (250 ms) and the SRS-REQ-075 software at-most-once enforcement | System/FSW authority | Requirements baseline |
| Document QEMU `virt` model credibility for this specific use (per NASA-STD-7009B) | V&V engineer | Simulation credibility |
| Verify cross-environment behavioural equivalence of the Gitea vs GitHub XW-2 runs | V&V engineer | N6 reproducibility |

## 6. Artifacts and hashes relied on (as recorded by the reviewer)

Opened and read: `AGENTS.md`, `docs/vv/test-spec/ts-7-xw2-target-contract.md`,
`docs/vv/verification-plan.md`, `docs/vv/verification-results.md`,
`docs/planning/sprint-plan.md`, `docs/vv/tool-qualification.md`,
`docs/requirements/traceability.yaml`, `.gitea/workflows/ci.yml`,
`.github/workflows/nightly.yml`, `tools/scripts/qemu_xw2_vv.sh`,
`tools/scripts/xw2_manifest.json`, and repository status/discovery.

Hashes relied on (as reported at issue time, not recomputed by the reviewer):
positive ELF `0972d96a0e4f549ae3c956af2edb4c98c94ea03c657ef0a2c5a60fe3c710f251`;
no-wrap control ELF `b695b1ad4c92647f28017c1e765f456a9dcae0b6bfec1d6cad7e3223d6011926`.

Log locator relied on (as reported at issue time): run directory
`build/xw2-run-20261101T170112Z` with per-requirement aggregation
`SRS-REQ-073 PASS=12 / SRS-REQ-075 PASS=1 / COVERAGE 13/13 / QEMU-PROCESS exit
124 (accepted) / WHOLE-RUN PASS / RESULT PASS`, base HEAD `65e4a91`.

**Record-consistency note (added at archival, not by the reviewer).** The review
was performed against an earlier retained run (`…T170112Z`). The final retained
gate evidence for the same tree is `build/gate-evidence/xw2-final.log` /
`xw2-final.exit` (rc=0) from run `build/xw2-run-20261101T172050Z`, which reports
the same 13/13 and the same positive/control ELF hashes. The identity-bearing
outcome is therefore unchanged; the record pointers are reconciled in the
verification-results and sprint-plan corrections that follow this review.

## 7. Non-claims required by this review

1. **Not N18.** The increment is not N18 evidence; N18 remains FAIL (NC-EXT-198, root cause NC-PAL-01).
2. **Not a device.** The three PAL hooks are substituted at link time; no device/backend behaviour is established.
3. **No reset scope.** Reset scope (mission function vs hart) is not established.
4. **No electrical/clock independence.** C3 independence is modelled only.
5. **No timing/WCET.** No WCET, jitter, interrupt latency or on-target cadence is asserted; XW-B1 asserts counts, not deadlines.
6. **No mission timeout.** The 8 s table value remains an ESC-7 placeholder; expectations derive from the configured value.
7. **No HIL / physical target.**
8. **T25 is NOT QUALIFIED** per N10.
9. **Not SRS-REQ-074 evidence on target** (XW-G2 has no real reset on this image).
10. **SRS-REQ-075 software at-most-once enforcement** remains an open authority interpretation; enforcement in this model is by the device.

## 8. Archival provenance

- The review was executed as a configured subagent task in session
  `ses_e67b8290bffe5Edah5mlqBsL3g` (harness-reported), model `opencode/big-pickle`.
- The reviewer's issued report was returned in chat and **not** written to disk at
  issue time. On 2026-11-02 the same session was asked to restate the issued
  content verbatim without re-deriving it; the restatement is the source of
  sections 1–7. The restatement explicitly declined to invent severity or
  disposition columns that were not issued, and that fidelity note is preserved
  here.
- No hash, log line or session identifier in this file was invented; hashes were
  independently recomputed on 2026-11-02 for the archived evidence set
  (`docs/vv/evidence/2026-11-01-xw2-target-final/assets.sha256`).
