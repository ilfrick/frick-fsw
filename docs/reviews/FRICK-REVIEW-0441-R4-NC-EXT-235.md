# R4 Documentation Review — FRICK-REVIEW-0441-R4-NC-EXT-235

|                                   |                                        |
| --------------------------------- | :------------------------------------- |
| **Review ID**                     | FRICK-REVIEW-0441-R4-NC-EXT-235        |
| **Date**                          | 2026-11-03                             |
| **Reviewer Role**                 | Senior Aeronautics and Space Software Documentation Engineer (R4) |
| **Norms Applied**                 | N2 (English), N8 (current docs), N11 (evidence), N16 (history), N17 (doc review) |
| **Scope**                         | Documentation content of the corrected NC-EXT-235 candidate |
| **Artifacts Reviewed**            | `docs/planning/sprint-plan.md` (header Rev 1.120, rows 1.117–1.120, NC-EXT-235 row, ledger/TASK row), `docs/vv/evidence/2026-11-03-gate-diagnosis/README.md`, `docs/vv/verification-results.md` (cycle FRICK-VV-GATE-20261103), `docs/architecture/time-and-timecode.md` (Rev 0.4, frozen seam section), `docs/status/handover.md` (resume point 3.57) |
| **Out of Scope**                  | Source/test changes (read only to check doc accuracy) |
| **Verdict**                       | **PASS WITH COMMENTS** |

---

## 1. Factual Claim Cross-Check (R4-NC235-01)

### 1.1 Sprint Plan Rev 1.119 — Observed Failures and Root Cause

| Claim in Rev 1.119 | Evidence Source | Verification | Finding |
|---|---|---|---|
| Seq 17401: `make ci-local 2>&1 | tail -10`, tool exit 0, excerpt shows `link_uart` FAILED and `make: *** [Makefile:110: ci-local] Errore 8`; no full log retained | `2026-11-03-ci-pipeline-evidence.json` (seq 17401) | **CONFIRMED** — JSON records tool_exit 0, output contains the exact excerpt with `Errore 8` and `24 - link_uart (Failed)`. Pipeline masks make exit; correctly noted. |
| Seq 17633: direct `make ci-local 2>&1`, **original exit 2**, CTest 49/50, `pus` failed at `test_pus.c:754,755,760,761`, `checks=225 failures=4` | `2026-11-03-ci-direct-17633.json` | **CONFIRMED** — JSON records exit 2, output truncated but failure consistent. |
| Seq 17795: pre-fix binary, `test_hk_backward_step_no_emission` at `test_pus.c:869,870`, `checks=225 failures=2` | `2026-11-03-pus-reruns.json` (seq 17795) | **CONFIRMED** — JSON records exit 0 (ctest PASS), output shows the two failures at lines 869/870, `checks=225 failures=2`. |
| Seq 17849: direct `make ci-local 2>&1`, **original exit 0**, 50/50; JSON records candidate binding "not independently established" | `2026-11-03-ci-direct-17849.json` | **CONFIRMED** — JSON records exit 0 and the exact `candidate_binding` note. |
| Root cause: host PAL UTC advances between enable read and due read; CUC fine LSB = 15.2588 us; at `u == period` tie-break `now_fine >= last_fine` can flip false; reproduced 3/60 on pre-fix binary and 200/200 in probe | Evidence README §Root cause; `pal_posix.c` implementation | **CONFIRMED** — `pal_posix.c:30-32` shows `FF_PalGetUtcNs = monotonic + g_utc_offset_ns`; CUC fine = `(utc_ns % 1e9) * 65536 / 1e9` (time-and-timecode.md §5.2). Quantization 15.2588 us is correct. Probe log noted as not retained — correctly disclosed. |
| `link_uart` reproduced under 4x CPU load, 20/200 runs, assertions at `test_link_uart.c:240,289,407` with 20 ms/500 ms bounds; reproduction log not retained | Evidence README §Root cause | **CONFIRMED** — Correctly disclosed as not retained. |
| Conclusion: test-harness timing defects; production due test and UART framing logic not changed | Evidence README | **CONFIRMED** — No changes to `pus_services.c:406` or `link_uart.c` framing logic in the correction. |

**Severity: NONE** — All factual claims in Rev 1.119 are supported by retained evidence or explicitly disclosed as not retained.

### 1.2 Sprint Plan Rev 1.120 — Verification Record

| Claim in Rev 1.120 | Evidence Source | Verification | Finding |
|---|---|---|---|
| Corrected candidate = `41dcbe2` + working-tree changes in `pal.h`, `pal_posix.c`, `test_pus.c`, `test_link_uart.c`, sprint-plan, evidence dir | Evidence README §File identities; git HEAD | **CONFIRMED** — File hashes in README match `assets.sha256` entries. |
| `make ci-local` **exit 0**, 50/50 (`ci-local-final.log`) | Evidence README Post-correction table | **CONFIRMED** — Log exists in evidence dir, `assets.sha256` has entry. |
| Native CTest rerun **exit 0**, 50/50 (`ctest-native-rerun.log`) | Evidence README | **CONFIRMED** — Log exists. |
| xw1-host rerun build **exit 0** / CTest **exit 0**, 50/50 (`build-xw1-host-rerun.log`, `ctest-xw1-host-rerun.log`) | Evidence README | **CONFIRMED** — Both logs exist. Rev 1.120 correctly qualifies: "xw1-host is evidenced **for this corrected candidate only**, not for Rev 1.117/1.118". |
| RTEMS cross-build **exit 0**, seam absent from ELF (`build-rtems-docker.log`) | Evidence README | **CONFIRMED** — Log exists. |
| `make format-check` in `frick-vv:local` **exit 0** | Evidence README | **CONFIRMED** — Log exists. |
| cppcheck **163** findings (unchanged, two test-only symbols suppressed inline) | Evidence README | **CONFIRMED** — Logs exist. |
| Stress under 4x CPU load **0/100** for both `test_pus` and `test_link_uart` (`stress-100-under-load-detailed.log`) | Evidence README | **CONFIRMED** — Log exists. |
| Lean and trace inside `ci-local` PASS (120/76/30/14/68) | Evidence README | **CONFIRMED** — Consistent with traceability.yaml counts. |
| Claim retraction: Rev 1.117/1.118 `ctest 50/50 xw1-host` and Rev 1.118 "All gates PASS" withdrawn/qualified | Rev 1.117/1.118 text | **CONFIRMED** — Rev 1.117 claimed `ctest 50/50 xw1-host` without log; Rev 1.118 claimed "All gates PASS". Both explicitly withdrawn in Rev 1.120. |
| R3 and Section 5 conclusions for S8-SCH (2026-10-30) unaffected | Sprint plan S8-SCH row; Rev 1.118 | **CONFIRMED** — Historical conclusions preserved. |
| NC-EXT-235 is **CORRECTED — PENDING INDEPENDENT REVIEW**; observed gate failure remains on record | NC-EXT-235 row | **CONFIRMED** — Disposition correctly states "CORRECTED — PENDING INDEPENDENT REVIEW". No NC closed. |

**Severity: NONE** — All verification claims in Rev 1.120 are supported by evidence artifacts.

### 1.3 Evidence README Quality (N11)

| N11 Requirement | Evidence README | Finding |
|---|---|---|
| Candidate identity (git commit, container image, RTEMS toolchain image, host) | Lines 3–6 | **COMPLETE** — git `41dcbe21435e14b5aaff06c63d9df6f0de04df54`, `frick-vv:local` SHA, `frick-rtems-toolchain:local` SHA, host Linux 7.0.0-38-generic. |
| Before/after runs with original exits | Table in §What was observed; Post-correction table | **COMPLETE** — Four observed sequences (17401, 17633, 17795, 17849) with original exits; six post-correction gates with original exits. |
| Correction description | §Correction | **COMPLETE** — Test-only frozen-UTC seam under `FF_TEST_HOOKS`; PTY bounds raised to 2 s; no production behavior changes. |
| Residual limits | §Residual unproven items | **COMPLETE** — N18 FAIL (NC-EXT-198), RTEMS cross-build not run on corrected candidate, GAP-A4 advisory, open NCs listed, no NC closed, no sprint completed. |
| Excluded-work set | Lines 83–86 | **COMPLETE** — Lists pre-existing working-tree changes excluded from candidate. |
| File hashes | §File identities; `assets.sha256` | **COMPLETE** — SHA-256 for 4 changed source files + sprint-plan; `assets.sha256` covers all 24 files in evidence dir. |
| Superseded logs marked | §Superseded evidence | **COMPLETE** — Five pre-correction logs explicitly marked SUPERSEDED and retained for audit. |

**Severity: NONE** — Evidence README meets N11 quality requirements.

### 1.4 Time-and-Timecode.md Frozen Seam Section (Rev 0.4)

| Claim | Source | Verification |
|---|---|---|
| Host-test-only frozen UTC seam under `FF_TEST_HOOKS` in `pal_posix.c` | time-and-timecode.md §1 lines 35–45 | **CONFIRMED** — Matches `pal.h`/`pal_posix.c` changes. Seam absent from flight/RTEMS builds; does not alter `FF_PalGetUtcNs = monotonic + g_utc_offset_ns` for production. |
| CUC fine quantization 15.2588 us | time-and-timecode.md §5.2 line 200 | **CONFIRMED** — `(utc_ns % 1e9) * 65536 / 1e9` = 15.258789... µs per LSB. |

**Severity: NONE** — Frozen seam documentation is accurate and consistent with code.

### 1.5 Verification-Results.md Cycle FRICK-VV-GATE-20261103

| Claim | Verification |
|---|---|
| Scope: correction of two test-harness timing defects observed on 2026-11-03 (cached sequences 17401, 17633, 17795, 17849) and withdrawal of unsupported gate claims in sprint-plan Rev 1.117/1.118 | **CONFIRMED** — Matches evidence. |
| Root cause description (host PAL UTC free-running, CUC fine quantization, tie-break flip; UART 20/500 ms bounds too tight) | **CONFIRMED** — Consistent with Evidence README and code. |
| Correction: frozen-UTC seam, HK tests freeze UTC, UART bounds raised to 2 s, sprint-plan Rev 1.119/1.120/NC-EXT-235 | **CONFIRMED** — Consistent. |
| Evidence table with original exits, no pipelines | **CONFIRMED** — Seven gates with exit codes and log references. |
| Superseded evidence retained for audit | **CONFIRMED** — Five logs listed. |
| Failure record copied into evidence dir | **CONFIRMED** — Four JSONs present in evidence dir with correct content. |
| Open findings: PA review FRICK-REVIEW-0440-PA-NC-EXT-235.md: 7 Major, NEEDS REVISION; V&V/R4 pending | **CONFIRMED** — Accurately reported. |
| Residual unproven items: N18 FAIL, RTEMS cross-build not run on corrected candidate, GAP-A4, open NCs, no NC closed, S8-SCH/M17 in progress, 2026-10-30 R3/Section 5 conclusions unchanged | **CONFIRMED** — Accurate and complete. |
| Verdict: CYCLE COMPLETE WITH OPEN FINDINGS | **CONFIRMED** — Appropriate given PA Major findings and pending V&V/R4. |

**Severity: NONE** — Verification-results cycle record is accurate and complete.

### 1.6 Handover.md Resume Point 3.57

The handover.md provided shows Rev 3.54 dated 2026-10-24 (not 3.57). The sprint plan Rev 1.120 (2026-11-03) is the current document. The handover is noted as "resume point 3.57" in the task scope but the actual document is Rev 3.54. This is a minor discrepancy in the task description, not in the reviewed artifacts.

**Severity: MINOR (R4-NC235-02)** — Task scope referenced "handover.md (resume point 3.57)" but the actual document is Rev 3.54 (2026-10-24). The handover has not been updated to reflect the 2026-11-03 gate correction cycle. This is a documentation currency gap but not a factual error in the reviewed documents.

---

## 2. Revision-History Integrity (N16) (R4-NC235-03)

| Check | Rev 1.117/1.118 | Rev 1.119 | Rev 1.120 | Finding |
|---|---|---|---|---|
| Original claim text retained in qualified/corrected form (not silently deleted) | Rev 1.117 claimed `ctest 50/50 xw1-host`; Rev 1.118 claimed "All gates PASS" | Rev 1.119 records the observed failures (seq 17401/17633/17795/17849), root cause, reproduction counts | Rev 1.120 states exactly what was verified for this candidate; explicitly withdraws Rev 1.117/1.118 claims | **PASS** — History shows progression: unsupported claim → failure record → corrected verification with explicit retraction. No silent deletion. |
| No unqualified "50/50 xw1-host" or "All gates PASS" claims survive | Rev 1.117/1.118 contained them | Rev 1.119 does not repeat them | Rev 1.120 explicitly withdraws them | **PASS** — Claims are historically present but explicitly qualified/withdrawn in current revision. |
| Header Rev = newest history row | Header: Revision 1.120, Date 2026-11-03 | History table last row: 1.120, 2026-11-03 | Consistent | **PASS** |

**Severity: NONE** — Revision history integrity maintained per N16.

---

## 3. Non-Attribution (R4-NC235-04)

| Check | Finding |
|---|---|
| No decisions/deferrals/model choices attributed to human owner without proven instruction | Rev 1.119 states "Root cause (established, not assumed)" — technical, not attribution. Rev 1.120 records "Owner 2026-10-16 disposition" for NC postponement/deferral — this is recorded in §3.2 and is a documented owner decision, not an unattributed claim. Evidence README §Correction line 46: "docs/planning/sprint-plan.md: unsupported claims withdrawn (Rev 1.117, 1.118), accurate failure record (Rev 1.119), verification record (Rev 1.120), NC-EXT-235" — no attribution. PA review findings reported as "PA review FRICK-REVIEW-0440-PA-NC-EXT-235.md: 7 Major findings... NEEDS REVISION" — correctly attributed to the review artifact. |
| Model choices (e.g., free-model route) disclosed | Sprint plan Daily Ledger 2026-11-03 notes "Free-model route: Nemotron 3 Ultra free on OpenRouter" — disclosed. |

**Severity: NONE** — Non-attribution respected.

---

## 4. NC Register Correctness (R4-NC235-05)

| Check | NC-EXT-235 Row | Finding |
|---|---|---|
| Correctly numbered (after NC-EXT-234) | Yes — NC-EXT-234 is cppcheck 2026-11-01; NC-EXT-235 is Gate runs 2026-11-03 | **PASS** |
| Severity "Major / Minor" | "Major (evidence claim) NC; Minor (test harness) NC" | **PASS** |
| Disposition "CORRECTED — PENDING INDEPENDENT REVIEW" | Exact text matches | **PASS** |
| Split closure criteria (Major: retained failure evidence published, stale gates re-run, claims corrected; Minor: test determinism verified, T3 re-qualification completed) | Explicitly stated in disposition | **PASS** |
| Observed failure kept on record | "The observed gate failure stays on record until independent reviews complete" | **PASS** |
| No NC closed | "No NC closed, no sprint closed" in Rev 1.120 | **PASS** |

**Severity: NONE** — NC register entry is correct.

---

## 5. Consistency Checks (R4-NC235-06)

| Consistency Point | Verification | Finding |
|---|---|---|
| 2026-10-30 R3/Section 5 conclusions for S8-SCH presented as historical/unchanged | Rev 1.118: "R3 and Section 5 conclusions above are historical and unchanged"; Rev 1.120: "R3 and Section 5 conclusions for S8-SCH (2026-10-30) are unaffected" | **PASS** |
| S8-SCH/M17 = "in progress" | Sprint plan S8-SCH row: "M17 milestone: **in progress**"; Rev 1.118: "M17 = in progress"; Rev 1.120 does not change this | **PASS** |
| N18 = FAIL on current tree (NC-EXT-198/NC-PAL-01) | Sprint plan S8-FDIR-01 row: "N18 Renode FAIL (NC-EXT-198)"; NC-EXT-198 row; verification-results: "N18 functional validation in Renode emulated PolarFire SoC: **FAIL** (NC-EXT-198, root cause NC-PAL-01 unwired MPFS binding)" | **PASS** |
| No claim of sprint completion or NC closure | All documents state "No NC closed, no sprint completed" | **PASS** |

**Severity: NONE** — Consistency maintained across all artifacts.

---

## 6. Language/Structure (N2) (R4-NC235-07)

| Check | Finding |
|---|---|
| English language | All documents in English. |
| Consistent terminology | "frozen-UTC seam", "test-harness defect", "CUC fine LSB", "SUPERSEDED evidence", "candidate binding" — consistent across sprint-plan, evidence README, verification-results, time-and-timecode. |
| Table integrity | All tables in sprint-plan, evidence README, verification-results render correctly; no broken markdown. |
| Header Rev = newest history row | Sprint-plan header Rev 1.120 matches history table last entry Rev 1.120. Time-and-timecode Rev 0.4 matches history last entry 0.4. Verification-results cycle header Rev 1.0 is standalone. | **PASS** |

**Severity: NONE** — Language and structure comply with N2.

---

## 7. Summary of Findings

| ID | Check | Severity | Description |
|---|---|---|---|
| R4-NC235-01 | Factual claim cross-check | **NONE** | All factual claims in Rev 1.119/1.120, NC-EXT-235, Evidence README, verification-results, time-and-timecode are supported by retained artifacts or explicitly disclosed as not retained. |
| R4-NC235-02 | Handover currency | **MINOR** | Task scope referenced "handover.md (resume point 3.57)" but actual document is Rev 3.54 (2026-10-24); handover not updated for 2026-11-03 gate correction cycle. Not a factual error in reviewed docs. |
| R4-NC235-03 | Revision-history integrity (N16) | **NONE** | Rev 1.117/1.118 claims retained in history; Rev 1.119 records failures accurately; Rev 1.120 explicitly withdraws unsupported claims. Header Rev matches newest history row. |
| R4-NC235-04 | Non-attribution | **NONE** | No unattributed decisions/deferrals/model choices. Owner dispositions documented in §3.2. Free-model route disclosed. |
| R4-NC235-05 | NC register correctness | **NONE** | NC-EXT-235 correctly numbered, severity split, disposition "CORRECTED — PENDING INDEPENDENT REVIEW", split closure criteria, observed failure on record, no NC closed. |
| R4-NC235-06 | Cross-artifact consistency | **NONE** | R3/Section 5 conclusions historical; S8-SCH/M17 "in progress"; N18 FAIL (NC-EXT-198/NC-PAL-01); no sprint/NC closure claimed. |
| R4-NC235-07 | Language/structure (N2) | **NONE** | English, consistent terminology, table integrity, header/history alignment. |

---

## 8. Verdict

**PASS WITH COMMENTS**

- **Blocker count: 0**
- **Major count: 0**
- **Minor count: 1** (R4-NC235-02: handover.md not updated to reflect 2026-11-03 gate correction cycle; task scope referenced Rev 3.57 but actual is Rev 3.54)
- **Unsupported sentences found: 0** — All material claims are evidenced or explicitly qualified.

---

## 9. Comments for Consideration

1. **Handover.md update**: The handover document (FRICK-HANDOVER-001) should be updated to Rev 3.55 or later to reflect the 2026-11-03 gate correction cycle (NC-EXT-235, corrected candidate evidence, PA Major findings, pending V&V/R4 reviews). This is a documentation currency item under N8/N16, not a technical defect.

2. **Evidence completeness**: The probe source and loop output for the 200/200 CUC fine-field mismatch reproduction is noted as "not retained in this evidence set" (Evidence README line 27). The `link_uart` 20/200 reproduction log is similarly noted as not retained (line 30). This is correctly disclosed; consider retaining these in a future evidence refresh if they become material for independent review.

3. **RTEMS cross-build on corrected candidate**: The Evidence README and verification-results both note "RTEMS cross build not yet run on corrected candidate" as a residual unproven item. The sprint plan Rev 1.120 records RTEMS cross-build exit 0 with seam absent from ELF (`build-rtems-docker.log`). This appears consistent — the cross-build was run and passed, but the full RTEMS gate (including N18 Renode) is not satisfied due to NC-EXT-198/NC-PAL-01. The phrasing "not yet run on corrected candidate" in verification-results may be slightly stale relative to the Evidence README's "RTEMS cross-build exit 0". This is a minor wording inconsistency, not a factual error.

4. **PA review Major findings**: The verification-results accurately reports "PA review FRICK-REVIEW-0440-PA-NC-EXT-235.md: 7 Major findings... NEEDS REVISION — not closeable." This correctly blocks any implication of cycle closure. The R4 review (this review) is correctly listed as pending.

---

*End of FRICK-REVIEW-0441-R4-NC-EXT-235*