# Independent V&V review (R2) — S1 progress-failure emission (NC-EXT-94)

| Field | Value |
| ----- | ----- |
| Document id | FRICK-REVIEW-R2-20261114-S1-PROGRESS |
| Reviewer profile | **R2 — Senior Aeronautics and Space Software V&V Engineer** (norm N9) |
| Review type | Independent delta review of an **uncommitted working-tree** change |
| Date | 2026-11-14 |
| Baseline (committed HEAD) | `fc68b9d121bce2194c1beaaa49a7dc4cdab0552c` (verified) |
| Change under review | `FF_PusIngest` progress verification after `handler(...)`; failure → TM[1,6] code 20; success → TM[1,5] code 0; never both. Tests + TS-1 rev 1.14 (`N22-TS1-105`). |
| Verdict | **CORRECTIONS REQUIRED** — no functional defect found in the change or in the new test design; **four Major V&V-record accuracy findings** and **one Major formal-coverage finding** must be corrected or explicitly tracked before this V&V record is committed as accurate evidence (N8/N11/N16). |

## 1. Independence caveat and authority boundary

This review is provided by a **subagent session**, i.e. **contextual independence only**.
It is **not organizational independence**, not a separate reporting path, and not a
different budget/management chain. It therefore **cannot** satisfy an N9 independence
determination on its own, and it is not claimed as such.

This review **does not** approve, certify, qualify, accept, release, or declare flight
readiness; it does **not** close or disposition any NC (including NC-EXT-94, NC-EXT-244,
NC-EXT-245, NC-EXT-246, NC-EXT-43/44); it does **not** assert PUS-C compliance or
mission tailoring; it does **not** constitute target/HIL evidence. All such decisions
belong to the authorized human roles defined by the governing baseline.

The review was **read-only**: no file was modified except the creation of this report, no
build directory was cleaned, no Docker command was run, and nothing was committed or pushed.

## 2. Binding-hash verification (performed before any review activity)

`sha256sum` was executed on all nine candidate files and all nine matched the binding
manifest exactly; `git rev-parse HEAD` matched `fc68b9d…0552c`. `git status --porcelain`
showed the nine modified files plus the untracked evidence directory, consistent with the
described delta.

| File | Expected (binding) | Observed | Match |
| ---- | ------------------ | -------- | ----- |
| `src/pus/pus.c` | `195b8cf5…c22293b` | `195b8cf5754e7b4043f8b62bcc0c47d06ff2e10e67cdc2ef2ad054dd8c22293b` | yes |
| `tests/unit/test_pus_s11_residual.c` | `33b43b96…3b42a13` | `33b43b961616b3b5a91925c75982054e07bda8b9b23fd57707ae9a30f3b42a13` | yes |
| `CHANGELOG.md` | `a35e9ea4…de8a4d7` | `a35e9ea4dead23f3bc29714e62c719699fb6b1df1756ec780d1c7d670de8a4d7` | yes |
| `README.md` | `5b119888…5de9b6` | `5b119888eb176e10747459021b54bd5605fc1adf44f8d689bf018e1caa5de9b6` | yes |
| `docs/planning/sprint-plan.md` | `27a42bd3…29985d` | `27a42bd3568a605fedc6b2e8bbad74782ee9a0b5a273b4bcc97ddc577a29985d` | yes |
| `docs/status/handover.md` | `ac374ec6…67cc11` | `ac374ec65e39069cfba39b9f5db0844e0bf507e716c457d9be837e1f4767cc11` | yes |
| `docs/status/loop-audit.md` | `c36a9e64…d4b2b41` | `c36a9e645c8795ed008010fc98d1f5d5e16af83dc501922a0934f7cb8d4b2b41` | yes |
| `docs/vv/test-spec/ts-1-protocol-core.md` | `7ceaa4b5…bacce95` | `7ceaa4b5bf403fa72fb3b25cd1fa83e6e1a59f0e2cdfcf7fcd915f649bacce95` | yes |
| `docs/vv/verification-results.md` | `8ab0fab7…10450bd` | `8ab0fab7f48b5ded9ffd065bc6124e3f963200d548877ae1e7be6edc010450bd` | yes |

## 3. Method, and what this review actually executed

Read: full `git diff` of the delta, `src/pus/pus.c` `FF_PusIngest`/`FF_PusSendVerification`,
`src/transport/ccsds/ccsds.c` TM/TC encoders, `config/fsw_config.h`, the complete new test
file, `docs/requirements/traceability.yaml`/`.md`, `docs/requirements/SRS.md` (SRS-REQ-006),
`docs/interfaces/ICD.md` §4, `verification/lean/Frick/Pus.lean`,
`verification/lean/Frick/Requirements.lean`, `tools/scripts/trace.py`, all 14 retained
evidence logs, TS-1 §2/§3/§3.1 and history, and the sprint-plan/handover/loop-audit/
CHANGELOG/README deltas.

Executed (host-only, non-destructive):
- `sha256sum` ×9 and `git rev-parse HEAD`, `git diff`, `git show`;
- `ctest --test-dir build/doc-update -R '^pus_s11_residual$' -V` → **PASS, `checks=1206 failures=0`**;
- `make trace` → **PASS** (120 requirements, 76 srs, 30 sss, 14 gaps, 68 lean theorems);
- a read-only Python recount of the TS-1 case table.

Not re-executed: full 60-test CTest run, `make verify-lean`, Renode/N18, Docker. Those
conclusions rest on the retained logs, which were read and cross-checked.

## 4. Answers to the six review questions (norm N22)

### Q1 — Does `N22-TS1-105` state explicit expected results for nominal and out-of-bound cases with a discriminating oracle?

**Yes, with four completeness gaps.**

`docs/vv/test-spec/ts-1-protocol-core.md:147` states explicit, countable expected results for
three sub-cases: exactly one TM(1,6) with code octet = `FF_PUS_FAIL_EXECUTION` (20), zero
TM(1,5), unchanged TM(1,8) when the completion ack is requested; nominal: exactly one
TM(1,5), zero TM(1,6), TM(1,7); no-completion-ack: exactly one TM(1,6), zero TM(1,5), zero
TM(1,8); "never both". The oracle is discriminating: it is a **presence + count + payload-octet**
oracle, so a revert to the pre-fix behaviour (which emitted TM(1,5) unconditionally) fails
`stream_count(1u, 6u) == 1u`.

Gaps (see VV-R2-06/07): the code value is asserted through the shared symbolic constant, not
a literal; the nominal sub-case does not assert code 0 on TM(1,5); no ordering assertion
(TM(1,6) before TM(1,8)) is specified or asserted; sub-case 1 does not assert TM(1,3)==1
although all four ack flags are set.

### Q2 — Is bidirectional traceability complete (SRS-REQ-006 → TS-1 → test function → Lean)?

**No. The chain is broken at two links, and the V&V record asserts otherwise.**

| Link | Status |
| ---- | ------ |
| SRS-REQ-006 → TS-1 row | **present** (`ts-1-protocol-core.md:147`) |
| TS-1 row → test function | **present** (row names `test_s1_progress_fail_regression`) |
| test function → SRS-REQ-006 (manifest) | **ABSENT** — `traceability.yaml:85-91` lists only `tests/unit/test_pus.c` and `tests/unit/test_r3_gap.c` |
| SRS-REQ-006 → Lean | **present but does not cover progress** (see Q2/VV-R2-05) |

`docs/vv/verification-results.md:985-988` states that "SRS-REQ-006 already lists
`tests/unit/test_pus_s11_residual.c` (line 106 of traceability.yaml)". That is **false on
both counts**: `traceability.yaml:106` lies inside the **SRS-REQ-008** block, and the
SRS-REQ-006 block (lines 85-91) does not list that file at all. Finding **VV-R2-01**.

The claimed `make trace` figures (120/76/30/14/68) are **reproduced exactly** by this
review, but `trace.py:296-299` only checks that each `tests` entry is an **existing path**;
it does not bind a case or test function to a requirement. A green `make trace` therefore
**cannot** evidence requirement→test-function traceability (VV-R2-15).

### Q3 — Are the three sub-cases genuinely independent and non-tautological, and does the test read the real emitted frame?

**Yes on all three points, with caveats.**

- **Real frame, no masking mock.** `tm_sink` byte-copies the emitted packet
  (`test_pus_s11_residual.c:35-41`). `stream_code_byte` (`:63-75`) walks the concatenated
  CCSDS packets using the primary-header PDL and reads `pos+27`. The layout was verified
  against `FF_CcsdsEncodeTm`/`FF_CcsdsEncodeTc`: 6 primary + 13 PUS TM secondary header +
  9-octet S1 payload (Request-ID[6] + service + subservice + code) + 2-octet CRC
  (`FF_FRAME_WITH_CRC 1`, `config/fsw_config.h:110-111`) = 30 octets, so the code octet is
  indeed at offset 27 and the CRC at 28-29. The `pos+7`/`pos+8` service/subservice reads are
  also correct (`ccsds.c:200-202`). No mock sits between `FF_PusIngest` and the assertion.
- **Non-tautological.** The three sub-cases produce three *different* expected sets and each
  can fail independently: (1) failure + completion ack; (2) success (the discriminating
  control that separates 1,5 from 1,6); (3) failure without completion ack (proves TM(1,8)
  gating is unchanged). A defect in any one branch fails a specific sub-case.
- **Discriminating against the pre-fix code (analytic mutation control).** Before this delta,
  `FF_PusIngest` called `FF_PusSendVerification(..., STAGE_PROGRESS, 1u, 0u)` unconditionally,
  so `stream_count(1u,6u)` was 0 and `stream_count(1u,5u)` was 1: sub-cases 1 and 3 would
  fail. **This control was reasoned from the diff, not executed** — no retained pre-fix
  RED log exists (see VV-R2-07).
- Caveats: all three sub-cases exercise a **single** handler error class (`FF_BAD_SIZE` from
  S9,1); the implementation maps *any* non-`FF_SUCCESS` to code 20, so a second error class
  (e.g. `FF_NOT_FOUND`/`FF_INVALID_ARGUMENT`) would strengthen the oracle. The code value is
  compared against the same macro the SUT uses (VV-R2-06).

### Q4 — Do the retained logs support the documented results?

**Partly. Two documented claims are contradicted by the evidence.**

| Documented claim | Retained evidence | Assessment |
| ---------------- | ----------------- | ---------- |
| CTest 60/60 PASS | `ctest.log` = 60/60, `CTEST_EXIT=0` | **supported** |
| Focused PUS 7/7 PASS | `focused-pus.log` = 7/7, `FOCUSED_EXIT=0` | **supported** (the doc's file list is incomplete: the `-R "pus\|pusopen\|decoder"` selection also ran `pus_fu`, `pus_n7_adapter_contract`, `pusopen_adapter_contract`) |
| `test_pus_s11_residual` **checks=1176, failures=0** (`verification-results.md:1050`; repeated at `loop-audit.md:795`) | No retained log contains a check count. **Re-executed: `checks=1206 failures=0`** | **contradicted** — VV-R2-02 |
| Full pinned format-check fails on "three *unchanged* initializer files" (`verification-results.md:1003`; `loop-audit.md:361,400`; `handover.md:67`) | `format-full.log` shows violations in **9 files / 44 diagnostics**, including `src/pus/pus_services.c:753` and 8 hits in `src/obcp/ff_obcp_inhouse.c` | **contradicted** — VV-R2-04 |
| Build "0 compiler warnings" (`verification-results.md:1047`) | `build.log` is an incremental no-op ("Built target …" only, no compile lines) | **not evidenced** — VV-R2-09 |
| Changed-file format PASS | `format-changed-files.log` contains only `FORMAT_CHANGED_EXIT=0` (no command, no files, no output) | **not evidenced** — VV-R2-08 |
| `make smoke` FAILED (stale `build/native` cache) | `smoke.log` shows the exact CMake cache/source mismatch, `SMOKE_EXIT=2` | **supported** |
| Safe smoke PASS | `smoke-safe.log` "[smoke] PASS (banner + 7 applications registered)" | **supported** |
| `make gsw-selfcheck` FAILED, pre-existing `vm.c` SPDX | `gsw.log` shows exactly that failure; `git diff HEAD` on that file is empty | **supported** |
| `make verify-lean` PASS | `lean.log` = "Build completed successfully", `LEAN_EXIT=0` | **supported** (not re-run by this review) |
| `make trace` PASS (120/76/30/14/68) | `trace.log`; **reproduced by this review** | **supported** |
| N18 `RENODE_TEST: PASS`, container exit 0 | `uart-console.log` ends `RENODE_TEST: PASS`; `status.env` `CONTAINER_EXIT=0`; `renode-process.log` has no verdict line of its own | **supported as an emulator run, but it does not exercise the changed path** — VV-R2-10 |

### Q5 — Are the limitations correctly declared?

**Mostly yes; one is materially understated.**

- Default `make smoke` failure on the stale `build/native` cache: **correctly declared**, with
  both outcomes retained separately and no destructive workaround claimed. Good practice.
- `make gsw-selfcheck` failure: **correctly declared** and correctly attributed to the existing
  NC-EXT-43/44 scope rather than silently waived or re-labelled as new. The licence
  classification was not guessed and the file was not edited. Good practice.
- Full format-check failure: **declared but mischaracterized** as "three unchanged initializer
  files"; the retained log shows nine files, including a PUS source file
  (`src/pus/pus_services.c:753`) that is adjacent to the changed subsystem. NC-EXT-244's scope
  is therefore understated (VV-R2-04).
- N18 Docker preflight gap (NC-EXT-246) and N19 sequencing gap (NC-EXT-245): **declared**
  honestly, including that the pre-edit worktree state is unknown.
- N9/N17/N20 reviews "UNVERIFIED": **declared** honestly; no review was fabricated.
- N18 coverage of *this* change: **not declared**. The retained emulated run exercises HAL/SB
  only (TS-1 `N22-TS1-92` is itself a `gap`), so N18 functional validation of the S1
  progress-failure path is **not established** (VV-R2-10).

### Q6 — Is any requirement coverage claimed that the evidence does not support? Is NC-EXT-94 correctly left OPEN?

**Yes to over-claim; yes, NC-EXT-94 is correctly OPEN.**

Over-claims:
1. `N22-TS1-105` is marked **`host covered`** with oracle grounding "SRS-REQ-006; ICD §4 S1
   subservice 6". SRS-REQ-006 (`SRS.md:65-67`) only constrains *"exactly one service 1 packet
   per acknowledged stage"*; it does not constrain success/failure mapping or the failure code.
   The discriminating content of the oracle is ICD §4 (valid: `ICD.md:106` now documents
   subservice 6) plus a **provisional failure-code allocation (NC-EXT-18)**, which the row does
   not qualify (VV-R2-06).
2. The traceability statement at `verification-results.md:985-988` is factually wrong (VV-R2-01).
3. `loop-audit.md:438` records N1 as "**N/A** — no Lean requirement theorems changed". The
   changed behaviour's core invariant ("exactly one of 1,5/1,6; never both") is **not
   formalized**, and the Lean model `verificationCount` (`verification/lean/Frick/Pus.lean:34-37`)
   **omits the progress stage entirely**, so it under-counts relative to the implementation
   for any progress-acked TC. N1/N3 coverage for this change is therefore not "N/A" but
   "not covered" (VV-R2-05).
4. N18 is recorded as "PASS retained from prior run" without stating that the retained run does
   not exercise the changed path (VV-R2-10).

NC-EXT-94 is **correctly left OPEN**: `sprint-plan.md:232` appends implementation status while
retaining "REMAINS OPEN pending interface-authority disposition", and `ts-1:147` repeats that
no NC is closed and no PUS-C compliance is claimed. No closure, certification, qualification or
release claim was found anywhere in the delta. That is the correct posture.

## 5. Findings

Severity scale: **Critical** (invalidates the change) / **Major** (must be corrected or
formally tracked before the V&V record is committed as accurate) / **Minor** (correct or
track) / **Observation** (no action required, recorded for the authority).

| ID | Severity | Location | Summary |
| -- | -------- | -------- | ------- |
| VV-R2-01 | **Major** | `docs/vv/verification-results.md:985-988` vs `docs/requirements/traceability.yaml:85-91` | False traceability statement; SRS-REQ-006 does not list the new test; requirement→test link missing (N3/N4) |
| VV-R2-02 | **Major** | `docs/vv/verification-results.md:1050`; `docs/status/loop-audit.md:795` | Documented `checks=1176`; executed result is `checks=1206 failures=0` |
| VV-R2-03 | **Major** | `docs/vv/test-spec/ts-1-protocol-core.md:299-300,349`; `verification-results.md:992-994`; `handover.md:1899`; `loop-audit.md:458,802` | §3.1 coverage summary not updated and coverage arithmetic stated incorrectly |
| VV-R2-04 | **Major** | `docs/vv/verification-results.md:1003`; `loop-audit.md:361,400`; `handover.md:67` vs `docs/vv/evidence/2026-11-14-s1-progress-failure/format-full.log` | Format-check limitation understated ("three files" vs nine files, incl. a PUS source file) |
| VV-R2-05 | **Major** | `verification/lean/Frick/Pus.lean:34-37`; `loop-audit.md:438`; `traceability.yaml:91` | Progress stage unmodelled; Lean count model diverges from implementation; N1 recorded as "N/A" |
| VV-R2-06 | Minor | `tests/unit/test_pus_s11_residual.c:376,427,429,471`; `ts-1:147` | Failure code asserted through the SUT's own macro, not literal `20`; NC-EXT-18 provisional allocation not qualified in the row |
| VV-R2-07 | Minor | `tests/unit/test_pus_s11_residual.c:415-449,455-475`; `ts-1:147` | Oracle completeness: no code-0 assertion on TM(1,5); no ordering assertion; no TM(1,3) count in sub-case 1; single error class; no retained pre-fix RED log |
| VV-R2-08 | Minor | `docs/vv/evidence/…/format-changed-files.log`; `verification-results.md:1052` | Changed-file format PASS rests on an exit marker with no command, file list, or output (N11) |
| VV-R2-09 | Minor | `docs/vv/evidence/…/build.log`; `verification-results.md:1047` | Retained build log is an incremental no-op; cannot evidence "0 compiler warnings" |
| VV-R2-10 | Minor | `loop-audit.md:454`; `handover.md:25`; `uart-console.log` | N18 row reads "PASS retained" without stating the run does not exercise the changed PUS path |
| VV-R2-11 | Minor | `docs/interfaces/ICD.md:2-4` vs `:255`; `verification-results.md:1016`; `CHANGELOG.md:59` | ICD header still revision 1.8 while history and records cite "rev 1.9" (pre-existing at HEAD, not introduced) |
| VV-R2-12 | Observation | `src/pus/pus.c:455-472` | Progress ack now duplicates the completion ack (same result/code, back-to-back); interface-authority note for NC-EXT-94 |
| VV-R2-13 | Observation | `src/pus/pus.c:460-465` | Unlock/lock window between handler result and progress emission (pre-existing pattern, not introduced) |
| VV-R2-14 | Observation | HEAD `fc68b9d` (`ICD.md` rev 1.9, assessment rev 0.4) | HEAD already documents TM(1,6) emission without an implementation; this delta is what makes HEAD's own documentation true |
| VV-R2-15 | Observation | `tools/scripts/trace.py:296-299` | `make trace` checks test *paths* only; a green trace cannot evidence case/test-function binding (N4 tooling limit) |

### VV-R2-01 (Major) — False traceability statement; requirement→test link absent

`docs/vv/verification-results.md:985-988`:

> "SRS-REQ-006 already lists `tests/unit/test_pus_s11_residual.c` (line 106 of
> traceability.yaml), so no traceability mapping change was required …"

`docs/requirements/traceability.yaml:85-91` (SRS-REQ-006) lists
`["tests/unit/test_pus.c", "tests/unit/test_r3_gap.c"]` and **not** the new file. Line 106 is
inside the SRS-REQ-008 block, which does list it. The statement is therefore wrong in both
its requirement attribution and its factual claim, and it was used as the justification for
not updating the manifest.

Impact: the new requirement→test link exists only in prose (TS-1 row 147); the machine-checked
manifest does not carry it, and the record asserts the opposite. `make trace` cannot detect
this (VV-R2-15). This is an N3/N4 traceability defect **and** an N11 evidence-integrity defect.

Recommendation: add `tests/unit/test_pus_s11_residual.c` to the SRS-REQ-006 `tests` list
(and, if the project tracks case-level binding, the case id `N22-TS1-105`), re-run
`make trace`, and correct the paragraph. If the omission is instead judged acceptable, say so
explicitly and remove the false justification. Authority: FSW engineer; independent
confirmation: R2/R3.

### VV-R2-02 (Major) — Documented check count contradicts the executed result

`verification-results.md:1050` documents `checks=1176, failures=0` for
`test_pus_s11_residual`, and `loop-audit.md:795` repeats `checks=1176/0`. No retained log
contains any check count. This review executed the registered CTest target on the same
`build/doc-update` tree (binary mtime 12:44:07, i.e. built after the reviewed source mtime
12:21:53, and it contains `test_s1_progress_fail_regression`):

```
13: …/tests/unit/test_pus_s11_residual.c checks=1206 failures=0
1/1 Test #13: pus_s11_residual ... Passed
```

`1206 − 1176 = 30`, which matches the number of assertions added by this delta: the documented
figure is the **stale pre-delta** count carried over from the previous increment's narrative.
The PASS conclusion is unaffected; the quantitative claim is wrong and unsupported.

Recommendation: replace with the executed value and cite a retained log that contains it
(e.g. `ctest -R pus_s11_residual -V` output added to the evidence directory). Authority: FSW
engineer (evidence owner).

### VV-R2-03 (Major) — TS-1 coverage summary stale; coverage arithmetic wrong

Recounted from the case table (105 rows, 105 unique ids):

| Metric | TS-1 §3.1 (`ts-1:295-302`) | Actual table | Delta |
| ------ | -------------------------- | ------------ | ----- |
| Total cases | 104 | **105** | +1 |
| Nominal | 48 | 48 | — |
| Out-of-bound | 51 | 51 | — |
| Mixed nominal + OOB | 5 | **6** | +1 |
| Individual | 93 | **94** | +1 |
| Combined | 11 | 11 | — |
| `covered` | 76 | **77** | +1 |
| `gap` | 28 | 28 | — |
| SIL/PIL/HIL | 101/1/2 | **102**/1/2 | +1 |

The new row `N22-TS1-105` is a **`host covered`, `nominal + OOB`, `individual`, `SIL`** case,
so every affected counter moves by one. In addition, the revision-history line
(`ts-1:349`) and `verification-results.md:992-994`, `handover.md:1899`,
`loop-audit.md:458,802` all state **"105 cases = 76 covered + 29 gap"**, which is
arithmetically inconsistent with both the previous baseline (104 = 76 + 28) and with adding a
`covered` case: adding a covered case can only yield **77 + 28**. The claim "§3.1 totals
updated" in the rev-1.14 history row is **not true** — §3.1 was not edited in this delta.

Impact: the coverage baseline used for the N22/N5 claim is misreported in five records, and the
error is internally inconsistent, so a reader cannot reconcile it. Severity Major because
coverage arithmetic is a controlled V&V metric, not prose.

Recommendation: update §3.1 to 105/48/51/6/94/11/77/28/102-1-2, correct all five "76 + 29"
occurrences to "77 + 28", and make the history row describe what was actually edited.

### VV-R2-04 (Major) — Format-check limitation materially understated

The records state the full pinned-format gate fails on "three *unchanged* initializer files"
(`verification-results.md:1003`, `loop-audit.md:361,400`, `handover.md:67`). The retained
`format-full.log` shows violations in **nine** files: `src/obcp/ff_obcp_inhouse.c` (8
diagnostics), **`src/pus/pus_services.c:753`**, `src/transport/link/link_tsn.c:18`,
`src/transport/link/link_ip.c:19`, `tests/unit/test_obcp_abort.c`, `tests/unit/test_link_tsn.c:24`,
`tests/unit/test_obcp_budget.c`, `tests/unit/test_obcp_inhouse.c`, `tests/unit/test_pus_s18.c`
(10 diagnostics). Exit codes: `make` reports `Error 123` and the wrapper marker is
`FORMAT_FULL_EXIT=2`; the record cites only "exit 123".

Impact: NC-EXT-244's scope is understated by a factor of three in file count, and the omission
includes a **PUS source file in the changed subsystem** (`pus_services.c`), which a reviewer of
this delta would specifically want to know. The gate is correctly *not* claimed as passed; the
characterization of *why* it fails is wrong.

Recommendation: restate the failure list accurately (or reference the retained log), and state
both exit-code layers. If any of the nine files is in fact *changed* in the working tree,
that changes the assessment materially and must be checked; per `git status` none is, but the
record should say so against the full list, not a three-file sample.

### VV-R2-05 (Major) — Progress stage is not formalized; N1 recorded as "N/A"

`verification/lean/Frick/Pus.lean:34-37`:

```lean
def verificationCount (a : AckFlags) : Nat :=
  (if a.acceptance then 1 else 0) +
  (if a.start then 1 else 0) +
  (if a.completion then 1 else 0)
```

`AckFlags.progress` exists (`:24`) but is **never counted**, and no theorem mentions the
progress stage. `req_srs006_acceptance_completion` and `ack_acceptance_is_counted`
(`Requirements.lean:58`, `Pus.lean:43,53`) therefore prove a *weaker* statement than
SRS-REQ-006, which explicitly enumerates **progress** among the acknowledged stages
(`SRS.md:65-67`). For a TC with acceptance+progress+completion acks the implementation emits
three packets while the model reports two.

This is a **pre-existing** modelling gap, not introduced by the delta. It becomes material
here because (a) the delta changes the progress path, and (b) the delta's central invariant —
"exactly one of TM(1,5)/TM(1,6), never both" — is precisely a state-machine property that the
Lean model could carry and currently does not. `loop-audit.md:438` nonetheless records N1 as
"**N/A** — no Lean requirement theorems changed; `make verify-lean` PASS". A green
`lake build` proves the existing theorems still elaborate; it does not establish N1/N3
coverage of the changed behaviour.

Recommendation: either (i) extend `verificationCount` with the progress stage and add a
theorem for "exactly one progress report per progress-acked TC, and its result equals the
handler result", which would also discharge N3 for SRS-REQ-006's progress clause; or (ii)
record the gap explicitly as an NC with owner/milestone and change the N1 row from "N/A" to
"not covered — tracked". Do not leave "N/A" as the disposition. Authority: FSW engineer +
formal-methods owner; disposition of the residual: Software PA.

### VV-R2-06 (Minor) — Failure-code oracle is coupled to the SUT constant

`test_pus_s11_residual.c:376,427,429,471` assert
`stream_code_byte(...) == FF_PUS_FAIL_EXECUTION`. The implementation
(`src/pus/pus.c:463`) uses the same macro, so a change of the wire value (e.g. 20 → 21) would
change both sides and the test would still pass. TS-1 row 147 documents the value as "(20)",
so the *specification* pins the octet but the *test* does not. Additionally the row does not
carry the NC-EXT-18 qualifier ("provisional failure-code allocation") that sibling rows
(e.g. `N22-TS1-36`) do, even though the value is not authority-allocated.

Recommendation: assert the literal `20u` (with a comment referencing NC-EXT-18) in at least
one sub-case, and add the NC-EXT-18 qualifier to the row's status cell.

### VV-R2-07 (Minor) — Oracle completeness and no retained pre-fix RED

Gaps: (i) the nominal sub-case (`:437-446`) asserts TM(1,5) presence/count and TM(1,7), but
**not** that the TM(1,5) code octet is 0, although the specification and the record both claim
"code 0"; (ii) neither TS-1 nor the test asserts the **ordering** TM(1,6) before TM(1,8) —
only counts, so an implementation that emitted completion before progress would pass;
(iii) sub-case 1 sets all four ack flags but does not assert TM(1,3)==1 (sub-case 3 does);
(iv) only one handler error class (`FF_BAD_SIZE`) is exercised; (v) there is no retained log
showing the new assertions failing against the pre-fix implementation, so the discrimination
claimed in §4/Q3 is analytic, not evidenced.

Recommendation: add `stream_code_byte(1u,5u) == 0u` to the nominal sub-case; add an ordered
stream assertion for (1,6)→(1,8); consider a second error class; optionally retain a pre-fix
RED log (the project has precedent: `docs/vv/evidence/2026-10-19-s8-vv-fu-mutant-m3-discrimination.log`).

### VV-R2-08 (Minor) — Changed-file format evidence is an exit marker only

`format-changed-files.log` contains exactly one line, `FORMAT_CHANGED_EXIT=0`: no command
line, no file list, no tool output. The record (`verification-results.md:1052`) claims a
pinned clang-format 18 `--dry-run --Werror` PASS "via `frick-vv:local`", which cannot be
verified from the artifact. This is the same evidence-sufficiency pattern already registered
as NC-EXT-101; it recurs here.

Recommendation: retain the command and the (empty) diagnostic output, or accept the marker as
a known limitation under NC-EXT-101 and say so in the record.

### VV-R2-09 (Minor) — Build log cannot support the "0 warnings" claim

`build.log` contains only `Built target …` lines: it is an incremental no-op, so it contains
no compilation of `src/pus/pus.c` and therefore no evidence of warning-free compilation under
`-Wall -Wextra -Werror`. Its timestamp (12:40) also precedes the final test-binary link
(12:44). The claim may well be true, but the retained artifact does not show it.

Recommendation: retain the compile lines for the changed translation unit (e.g. a clean
recompile of `pus.c` and the test), or state "incremental build, no recompilation observed".

### VV-R2-10 (Minor) — N18 coverage of this change is not established and not declared as such

`loop-audit.md:454` and `handover.md:25` record "N18 Renode functional result PASS retained
from prior run". The retained console (`uart-console.log`) shows the `renode-test` application
exercising HAL classes and an SB round-trip — it does **not** drive CCSDS/PUS, and TS-1
`N22-TS1-92` already records that as a `gap`. N18 ("functional validation of every change")
is therefore **not established for the changed PUS path**, and the row's wording invites the
opposite inference.

Recommendation: state explicitly that the retained emulated run does not exercise S1 progress
emission and that N18 coverage for this delta is not established (tracked with the existing
`N22-TS1-92` gap), alongside the NC-EXT-246 preflight deviation.

### VV-R2-11 (Minor, pre-existing) — ICD header revision lags its history

`docs/interfaces/ICD.md` header says `Revision 1.8 / 2026-11-13`, while the history table has a
`1.9 | 2026-11-14` row (`:255`) and `verification-results.md:1016`, `CHANGELOG.md:59` and
`handover.md` cite "ICD rev 1.9 §4". The delta's grounding citation therefore points at a
revision number that the document header does not carry. Not introduced by this delta
(committed at `fc68b9d`), but it affects the delta's evidence chain (N8/N16).

### Observations (no action required; recorded for the authority)

- **VV-R2-12 — progress/completion duplication.** With both `ack_progress` and `ack_complete`
  set, the ingestion path now emits TM(1,5) or TM(1,6) immediately followed by TM(1,7) or
  TM(1,8) carrying the *same* result and code. In PUS-C terms a progress report is intended to
  describe execution *during* execution; for a synchronous handler there is now no
  intermediate progress content, so the progress report is a duplicate of the completion
  report. No requirement or ICD clause in this baseline constrains the ordering or the
  non-duplication, and the ICD §4 row does document subservice 6 as emitted, so this is **not**
  a non-conformance. It is, however, an interface-semantics question the interface authority
  should disposition inside NC-EXT-94 (e.g. whether progress should carry the handler result
  at all, or whether subservice 6 should be emitted only by long-running handlers).
- **VV-R2-13 — unlock/lock window.** `src/pus/pus.c:460-465` unlocks the PUS mutex to send the
  progress ack and re-locks afterwards, between the handler result and the completion ack.
  A concurrent `FF_PusIngest` on another thread can interleave its own verification stream in
  that window. This is the pre-existing pattern used for acceptance/start/completion, so it is
  not introduced here, and no concurrency requirement is claimed; recorded as a residual
  robustness limit.
- **VV-R2-14 — documentation ahead of code at HEAD.** HEAD `fc68b9d` already commits
  `ICD.md` rev 1.9 (subservice 6 emitted) and
  `docs/architecture/pus-c-from-scratch-assessment.md` rev 0.4 (emitted set
  `partial (1,2,3,5,6,7,8)`) describing behaviour that did **not** exist at HEAD. The delta is
  what makes HEAD's own documentation true. If this delta were abandoned, HEAD would be
  non-conformant. Conversely, the delta cannot be committed with a record that misstates its
  own evidence (VV-R2-01/02/03/04).
- **VV-R2-15 — trace tooling limit.** `tools/scripts/trace.py:296-299` validates that `tests`
  entries are existing paths. It cannot detect a missing case-level binding, which is why
  `make trace` PASS coexists with VV-R2-01. If N4 is to be machine-enforced at case level, the
  manifest schema needs case/test-function identifiers.

## 6. What is sound in this change (explicit positives)

- The functional change is minimal, correctly placed, and internally consistent:
  `rc` is assigned **before** both the progress and completion emissions
  (`src/pus/pus.c:455,460-463,468-470`), the existing stage→subservice map already covers
  `PROGRESS→{5,6}` (`:316-318`), and early-return paths (illegal APID, unknown handler) are
  untouched.
- The new test reads the **real** emitted frame at the correct octet (verified against the
  encoder), uses a byte-copying sink, re-initializes the stack per test, and would fail against
  the pre-fix implementation.
- The three sub-cases are non-tautological and collectively cover the failure branch, the
  success control, and completion-ack gating.
- Limitations that *are* declared (stale `build/native` smoke cache, `gsw-selfcheck` licence
  finding, NC-EXT-244/245/246, unverified N9/N17/N20 reviews) match their retained logs, and
  the safe-smoke PASS is retained separately from the failed default smoke — exactly the
  distinction N11 requires.
- NC-EXT-94 is left **OPEN** with its authority owner named; no NC closure, certification,
  qualification, release or flight-readiness claim appears anywhere in the delta.
- `README.md:184` and `ICD.md:106` are now consistent with the implemented S1 wire set
  (subservice 6 emitted; subservice 4 still has no emission path), which removes the
  documentation-vs-implementation divergence that motivated NC-EXT-94.

## 7. Verdict

**CORRECTIONS REQUIRED (V&V-record accuracy and formal coverage), no blocking functional
defect.**

- The production change and the new tests are acceptable as they stand: no functional defect,
  no oracle masking, no gate bypass, no NC closure, no over-broad assurance claim.
- Before this V&V record is committed as accurate evidence, the following must be corrected
  (or, where a correction is not possible in this cycle, explicitly tracked with owner and
  milestone and the record must stop asserting the unverified fact):
  **VV-R2-01** (false traceability statement + missing SRS-REQ-006 test link),
  **VV-R2-02** (check count 1176 → 1206),
  **VV-R2-03** (TS-1 §3.1 and the "76 + 29" arithmetic in five records),
  **VV-R2-04** (format-check failure scope),
  **VV-R2-05** (N1 recorded as "N/A" while the progress invariant is unformalized).
- VV-R2-06 through VV-R2-11 should be dispositioned as NCs or corrected in the same cycle.
- Per N21 these are non-functional findings and do not by themselves block unrelated
  development; they **do** block this specific record from being published as accurate
  evidence (N8/N11/N16).

This verdict is **not** an approval, qualification, acceptance, NC disposition, or release
recommendation, and it carries no authority beyond the evidence presented.

## 8. Residual risks and limits of this review

1. **Host SIL only.** No target, HIL, or timing evidence exists for this change; the host
   assertions say nothing about RTPFS160ZT execution, WCET, or bus behaviour.
2. **No independent failure-code oracle.** Code 20 is provisional (NC-EXT-18); the test pins
   it through the SUT's own macro, so a coordinated change would not be detected.
3. **No formal coverage of the changed invariant.** The "exactly one of 1,5/1,6, never both"
   property is tested, not proved; the Lean model omits progress entirely.
4. **No concurrency evidence.** The unlock/lock window around the progress emission is not
   exercised by any concurrent-ingestion test.
5. **Single error class and single service.** Only `FF_BAD_SIZE` from TC[9,1] is exercised;
   behaviour for other handlers/services is inferred from the binary `rc == FF_SUCCESS` test,
   not measured.
6. **Progress/completion semantic duplication** is unvalidated by the interface authority;
   a ground segment may treat TM(1,6) as a distinct progress event when it is in fact a
   restatement of the completion result.
7. **This review did not re-execute** the full 60-test suite, Lean, Renode, Docker, or the
   format checks; those conclusions are evidence-based, not reproduced, except
   `pus_s11_residual` (reproduced, 1206/0) and `make trace` (reproduced, PASS).
8. **Contextual independence only.** No organizational independence, separate reporting path,
   or management separation exists for this review; an N9 independence determination and any
   N17/N20 disposition remain with the authorized human roles.
9. **No claim of completeness.** Passing tests and coverage counts show that observed outcomes
   matched defined oracles under the recorded conditions. They do not establish absence of
   defects, target fitness, or mission suitability outside the recorded envelope.
