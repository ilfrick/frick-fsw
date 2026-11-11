# Independent V&V Review Report — N18 Renode Re-execution on Current Tree (NC-EXT-165)

|                                   |                                        |
|----------------------------------:|:---------------------------------------|
| Document id                      | FRICK-REVIEW-VV-R2-N18-CT-20261111     |
| Reviewer profile                 | R2 (Senior SW V&V), norm N9 / N10      |
| Reference revision               | `HEAD 833a0e0` (branch `main`), tracked tree clean — no uncommitted source change |
| Items under review               | `tools/scripts/renode_vv.sh`, `tools/scripts/renode_vv_verdict.sh`, `Makefile:96-97`, `build/renode/` (current-tree N18 run evidence), `docs/planning/sprint-plan.md:284` (NC-EXT-165), `docs/vv/tool-qualification.md:40` (T17), `docs/rules.md:315-331` (N18), `docs/rules.md:93-108` (N7) |
| Date                             | 2026-11-11                             |
| Scope                            | Independent review of whether the current-tree N18 Renode functional-validation PASS satisfies NC-EXT-165; whether NC-TS-01 (ELF freshness) is addressed; whether additional gates/evidence are needed; T17/T3 tool-qualification impact; and any findings blocking commit/push or NC-EXT-165 resolution. Target/HIL, NC-PAL-01, RTPFS160ZT fidelity are explicitly EXCLUDED from credit (see VV-R2-005). |
| Independence declaration         | Reviewer had no part in authoring/executing the N18 run or the tracked tree; fresh context. This is the independent R2 verification referenced by NC-EXT-165 ("verifier: independent R2"). |
| Norms applied                    | N1-N22 (docs/rules.md), in particular N7 (commit/push gate), N9 (independent review), N10 (test-script/V&V-tool review), N11 (evidence retention), N15 (full cycle per change), N18 (Renode functional-validation gate, in force), N20 (PA) |
| ECSS references                  | ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2 (V&V tools as verification tools, qualification by review + CM + traceability) |

## 1. Evidence summary (verified on the current tree, 2026-11-11)

| Evidence item | Observed value | Location |
|---|---|---|
| Tracked-tree state | clean at `833a0e0`; no uncommitted source change; increment = N18 re-execution only | `git status --porcelain` (none) |
| Host build | exit 0, 0 warnings | `make build` |
| Host CTest | 56/56 PASS | `make test` |
| Lean | `lake build` PASS | `make verify-lean` |
| Traceability | PASS (120 req / 76 SRS / 30 SSS / 14 GAP / 68 Lean) | `make trace` |
| Format | PASS (pinned clang-format 18.1.3) | `make format-check` |
| License | OK | N13 checker / `make gsw-selfcheck` |
| Renode ELF | freshly built today `2026-11-11 18:11`, 4 133 504 bytes, byte-identical to native `frick_fsw` | `build/renode/frick_fsw_renode.elf` |
| N18 console marker | `RENODE_TEST: PASS` present (1 occurrence) | `build/renode/console.log` |
| N18 process log | clean disposal: `Machine paused.` / `Disposed.` (container exited cleanly) | `build/renode/renode-process.log` |
| N18 verdict (per context) | `[renode] PASS (RENODE_TEST: PASS; container status=exited container exit code=0)` | `make renode` → `tools/scripts/renode_vv.sh` |

Gate mechanism confirmed by inspection:
- `Makefile:96-97`: `make renode` → `bash tools/scripts/renode_vv.sh`.
- `tools/scripts/renode_vv.sh:34` ELF path `build/renode/frick_fsw_renode.elf`; `:41-44` SKIP when ELF absent, otherwise gate in force; `:72-86` bounded wait with `CONTAINER_STATUS`/`CONTAINER_EXIT` captured; `:94-101` fail-closed log capture; `:121-122` retention of console + process log to `build/renode/` (note: `status.env` with the exit code is kept ONLY in the per-run `/tmp` directory, NOT retained in-tree — see VV-R2-004).
- `tools/scripts/renode_vv_verdict.sh:85-88`: marker present in this run's captures → `exit 0` PASS.
- `docs/rules.md:315-331` (N18): gate passes when the emulated run produces `RENODE_TEST: PASS`; gate in force on the real open-source RTEMS 6.2 image; missing marker fails (exit 1).
- `docs/planning/sprint-plan.md:284` (NC-EXT-165): requires a fresh RTEMS 6.2 cross-build and prescribed `make renode` on the current tree with retained logs and source/image/ELF identity; blocks N7 commit/push until satisfied; verifier = independent R2.

## 2. Findings

### VV-R2-001 — Major (gate-determining; condition SATISFIED, positive)
**Item/Requirement:** NC-EXT-165 (Major gate NC); N18 gate (docs/rules.md:315-331); sprint-plan.md:284.
**Finding:** The N18 Renode functional-validation gate PASSED on the current tree: `RENODE_TEST: PASS` marker present, container status=`exited`, container exit code=0. The ELF was freshly built today (2026-11-11 18:11) and logs are retained (`build/renode/console.log`, `build/renode/renode-process.log`). This satisfies NC-EXT-165's required evidence: fresh RTEMS 6.2 cross-build + prescribed `make renode` on the current tree, with retained logs and ELF identity.
**Evidence:** `tools/scripts/renode_vv.sh:41-44,72-86,121-122`; `tools/scripts/renode_vv_verdict.sh:85-88`; `Makefile:96-97`; `build/renode/console.log` (tail: `RENODE_TEST: PASS`); `build/renode/renode-process.log` (clean disposal); `build/renode/frick_fsw_renode.elf` (mtime 2026-11-11 18:11).
**Blocks commit/push:** No — this is the evidence that removes the N7 block; host gates all green.
**Blocks NC-EXT-165 resolution:** No — this finding SUPPORTS resolution of NC-EXT-165 for this tree, subject to the Minor findings below (freshness register, documentation currency, scope boundary).
**Recommendation:** The responsible FSW/V&V owner may disposition NC-EXT-165 as resolved-for-this-tree, with this independent R2 review as the verification record, and close the NC per N12 after the currency updates in VV-R2-004 are committed.

### VV-R2-002 — Minor — NC-TS-01 ELF freshness not structurally enforced (mitigated for this run)
**Item/Requirement:** NC-TS-01 (ELF freshness weakness); `tools/scripts/renode_vv.sh`.
**Finding:** `renode_vv.sh:41-44` gates on ELF **existence** (`[[ ! -f "${ELF}" ]]`), not on freshness relative to source/build inputs. NC-TS-01 therefore remains OPEN as a register item: a stale ELF could still produce a misleading PASS in a future run. For THIS run the concern is mitigated: the ELF was freshly built (mtime 2026-11-11 18:11, after the tracked sources) and the tree is clean at HEAD 833a0e0, so the binary corresponds to the current sources.
**Evidence:** `tools/scripts/renode_vv.sh:41-44`; `docs/vv/tool-qualification.md` history entries `0.3.88`/`0.3.89` (NC-TS-01 registration; fresh build mitigates the run only; closure requires freshness enforcement).
**Blocks commit/push:** No — this run's evidence is sound (fresh ELF demonstrated by build timing).
**Blocks NC-EXT-165 resolution:** No for this tree; NC-TS-01 remains an open register item to be closed by a script delta (ELF-vs-source freshness comparison or forced `make renode-image` rebuild inside the gate) plus independent N10 review. Not a precondition for this commit.
**Recommendation:** Add freshness enforcement to `renode_vv.sh` (e.g., hash manifest over `src/`, `config/`, tests and CMake inputs compared against an ELF build manifest, or an unconditional rebuild), then route the script delta through N10 independent review before claiming NC-TS-01 closed.

### VV-R2-003 — Minor — T17 tool qualification: no T17/T3 delta in this increment; register currency only
**Item/Requirement:** T17 (Renode V&V environment, norm N18); docs/vv/tool-qualification.md:40; N10.
**Finding:** No V&V script/tool changed in this increment (`renode_vv.sh`, `renode_vv_verdict.sh`, `tools/docker/renode/`, `src/apps/app_renode_test.c` are all at HEAD 833a0e0, unchanged). Per N10 ("Any change to a recorded V&V script invalidates its qualification and forces a re-qualification review"), no re-qualification is triggered, and there is **no T3 impact**. The T17 register entry's statement "no Renode PASS is claimed for the current tree" is now factually superseded by this run; it requires currency correction (see VV-R2-004) but not re-qualification. T17 remains "reviewed + CM; independent R2 on change; not silicon"; this run is emulator evidence only.
**Evidence:** `docs/vv/tool-qualification.md:40` (T17 row); `git status` (no T17 file modifications); N10 at `docs/rules.md:138-156`.
**Blocks commit/push:** No.
**Blocks NC-EXT-165 resolution:** No.
**Recommendation:** Update the T17 row and the NC-TS-01-related history to reflect the current-tree PASS and the fresh-build mitigation; no new qualification review is required because no tool changed.

### VV-R2-004 — Minor — Evidence/documentation currency; exit-code retention gap
**Item/Requirement:** N8 (docs current), N11 (evidence retention), NC-EXT-165's "retained logs ... identity" condition.
**Finding:** (a) `verification-results.md`, `tool-qualification.md:40`, `docs/status/handover.md`, `docs/status/loop-audit.md`, `docs/status/status.json`, and the sprint-plan NC-EXT-165 row still reflect the pre-run pending/FAIL state and must be updated to record the current-tree N18 PASS and NC-EXT-165 resolution. (b) `renode_vv.sh:121-122` retains only the console and process log to `build/renode/`; `status.env` (which carries `CONTAINER_EXIT=0`, the decisive exit-code evidence) lives only in the per-run `/tmp` directory and is NOT retained in the tree. Norm N11 requires evidence retention; the exit code should be copied into `build/renode/` alongside the other logs.
**Evidence:** `tools/scripts/renode_vv.sh:54,87-91,121-122` (status.env written to RUN_DIR, not retained); `docs/vv/tool-qualification.md:40`; `docs/planning/sprint-plan.md:284`.
**Blocks commit/push:** No (documentation currency, correctable in the same commit cycle).
**Blocks NC-EXT-165 resolution:** No for the technical gate; strengthens the "retained logs" condition once done.
**Recommendation:** (1) Retain `status.env` (or at least `CONTAINER_EXIT`) into `build/renode/` in `renode_vv.sh` after the verdict step; (2) update `verification-results.md`, `tool-qualification.md`, `handover.md`, `loop-audit.md`, `status.json` and the sprint-plan NC-EXT-165 row to record the 2026-11-11 current-tree PASS and disposition NC-EXT-165 resolved-for-this-tree; keep revision history per N16.

### VV-R2-005 — Minor (scope boundary) — N18 is emulated evidence only; no target/HIL, RTPFS160ZT, or NC-PAL-01 credit
**Item/Requirement:** NC-PAL-01; GAP-A5; N22-TS5-82 (HIL gap); docs/rules.md:326-331; docs/vv/renode-environment.md:22-27.
**Finding:** The PASS establishes functional validation on the emulated Microchip PolarFire SoC (Renode `beaglev-fire` board, `polarfire-soc.repl` model) only. It does NOT establish target/HIL evidence, RTPFS160ZT register/clock/reset-scope fidelity, hardware watchdog behaviour, WCET/timing, or mission suitability; it does not close NC-PAL-01 (flight watchdog hooks remain `FF_NOT_IMPLEMENTED`, e.g. `src/core/pal/pal_rtems.c:34`, `pal.h:123`) or GAP-A5. This is consistent with `docs/status/status.json`, which lists GAP-A5/NC-PAL-01 as open but NOT blockers for commit/push.
**Evidence:** `docs/vv/renode-environment.md:22-27` (platform/model scope limits), `:177-180` (GAP-A5 residual); `docs/vv/test-spec/ts-5-core-services.md:212` (N22-TS5-82 HIL gap); `src/core/pal/pal_rtems.c:34`; `docs/planning/sprint-plan.md:284` (NC-EXT-165 is the N18-for-this-tree gate only).
**Blocks commit/push:** No — NC-PAL-01/GAP-A5 are documented as open non-blockers.
**Blocks NC-EXT-165 resolution:** No — NC-EXT-165 concerns the N18 emulated gate for this tree, which is satisfied; target/HIL remains a separate open item.
**Recommendation:** Do not over-claim. Keep NC-PAL-01 and GAP-A5 open with owner/avionics authority; pursue target/HIL (N22-TS5-82) separately.

### VV-R2-006 — Minor (historical context) — NC-EXT-198 / NC-EXT-199 do not invalidate this run
**Item/Requirement:** NC-EXT-198 (watchdog init panic / false PASS root cause), NC-EXT-199 (runner evidence integrity); `renode_vv.sh` (NC-EXT-199 fix).
**Finding:** NC-EXT-198/NC-EXT-199 were registered against the 2026-10-21 tree state, where N18 FAILED (startup panic, marker absent) and the runner reported a false PASS from a stale `/tmp` log. The corrected runner (`renode_vv.sh:10-15,47-51,94-101`: fresh per-run directory, fail-closed capture, verdict only from this run's captures) and the current truthful PASS (`RENODE_TEST: PASS`, exit 0) mean neither NC invalidates this run. Both remain open register items but require no new action on this increment.
**Evidence:** `tools/scripts/renode_vv.sh:10-15,47-51,94-101`; `docs/status/handover.md:496` (historical NC-EXT-198/199 narrative); `docs/vv/tool-qualification.md` history `0.3.104`/`0.3.105` (runner correction and independent confirmation FRICK-REVIEW-0423/0439).
**Blocks commit/push:** No.
**Blocks NC-EXT-165 resolution:** No.

## 3. Answers to the review questions

**Q1 — Is the N18 PASS (exit 0, RENODE_TEST: PASS) on the current tree sufficient verification evidence for NC-EXT-165?**
Yes, for the N18-for-this-tree gate. The run used a freshly built ELF, produced the `RENODE_TEST: PASS` marker, exited with code 0, and retained console + process logs. NC-EXT-165's required evidence (fresh RTEMS 6.2 cross-build + prescribed `make renode` on the current tree, retained logs, ELF identity) is met. It is NOT sufficient to claim target/HIL, RTPFS160ZT fidelity, or to close NC-PAL-01/GAP-A5 (VV-R2-005).

**Q2 — Is the ELF freshness concern (NC-TS-01) addressed (freshly built ELF)?**
Mitigated for this run: the ELF was freshly built today (2026-11-11 18:11) on a clean tree at HEAD 833a0e0, so the binary matches the current sources. NOT structurally closed: `renode_vv.sh:41-44` still keys on ELF existence, not freshness. NC-TS-01 remains open as a register item pending a script delta + independent N10 review (VV-R2-002). It does not block this commit or this NC-EXT-165 resolution.

**Q3 — Are any additional gates or evidence needed?**
No new gate is required. Non-blocking evidence/housekeeping items: (a) retain `status.env`/exit code in-tree (`build/renode/`, currently only in per-run `/tmp` — VV-R2-004b); (b) update documentation currency (`verification-results.md`, `tool-qualification.md:40`, handover, loop-audit, status.json, sprint-plan NC-EXT-165 row — VV-R2-004a); (c) enforce ELF freshness in the gate and close NC-TS-01 via N10 review (VV-R2-002, follow-up, non-blocking); (d) independent R2 confirmation — satisfied by this report (NC-EXT-165 names "verifier: independent R2").

**Q4 — Tool qualification considerations (no new test scripts, no T3 impact)?**
Confirmed: no V&V script/tool changed in this increment, so N10 re-qualification is not triggered and there is no T3 impact. T17's existing qualification ("reviewed + CM; independent R2 on change; not silicon") stands; only register currency needs correcting (VV-R2-003). T25/XW-2 is unrelated to N18 and carries no N18 credit (tool-qualification.md:49; Makefile xw2 comment).

**Q5 — Any V&V findings that block commit/push or NC-EXT-165 resolution?**
None. VV-R2-001 removes the N18 block; VV-R2-002..006 are Minor/documentation/scope findings that do not block commit/push and do not prevent NC-EXT-165 disposition as resolved-for-this-tree. Host gates (build 0 warnings, CTest 56/56, Lean, trace 120/76/30/14/68, format, license) all pass, satisfying the N7/N15 preconditions.

## 4. Conclusion

| Verdict | Basis |
|---|---|
| **N18 gate on current tree: PASS** | `RENODE_TEST: PASS`, container exit 0, fresh ELF, retained logs |
| **NC-EXT-165: resolvable as resolved-for-this-tree** | Required fresh cross-build + prescribed `make renode` + retained evidence + ELF identity all evidenced; independent R2 verification = this report |
| **Commit/push (N7): not blocked by V&V** | All host gates green; no V&V finding classified as blocking |
| **Open, non-blocking items** | NC-TS-01 (script freshness enforcement + N10 review), NC-PAL-01/GAP-A5 (target/HIL, not N18 credit), documentation currency (VV-R2-004) |

**Recommendation:** Proceed with committing/pushing the evidence/currency updates (no source change), disposition NC-EXT-165 resolved-for-this-tree under N12 with this review as the independent R2 record, and track NC-TS-01, NC-PAL-01 and GAP-A5 to their own owners/milestones without using them to withhold this commit.

---

## 5. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
|---|---|---|---|
| R2 (SW V&V, N9/N10 independent) | Senior SW V&V Engineer (this subagent) | 2026-11-11 | PASS — N18 current-tree evidence sufficient for NC-EXT-165; no blocking findings |

## 6. Follow-up records (NC register linkage)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
|----|----------|--------------------|---------|-------------|--------------------------|-------|--------|-----------------|
| NC-EXT-165 | Major gate | N18 on current tree (sprint-plan.md:284) | PASS evidenced (VV-R2-001) | Resolve resolved-for-this-tree; commit currency updates | This report; build/renode/ console + process log + ELF | FSW/V&V | CLOSED-for-this-tree (pending owner disposition + VV-R2-004 currency commit) | This R2 review |
| NC-TS-01 | Minor | ELF freshness (tool-qualification.md 0.3.88/0.3.89) | Mitigated this run; not structurally enforced (VV-R2-002) | Freshness-enforcement script delta + N10 review | Gate script delta + independent R2 | FSW/V&V | OPEN | Required (N10) |
| NC-PAL-01 | Major | Flight watchdog binding unwired | Out of N18 credit scope (VV-R2-005) | Owner/avionics authority; target/HIL | HIL evidence (N22-TS5-82) | Owner + avionics | OPEN | — |
| GAP-A5 | — | Target/HIL fidelity | Out of N18 credit scope (VV-R2-005) | Separate work package | Target/HIL evidence | FSW/avionics | OPEN | — |
