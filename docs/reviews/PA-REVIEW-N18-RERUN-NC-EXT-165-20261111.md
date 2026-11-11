# Independent Product Assurance Review — N18 Re-run / NC-EXT-165

**Repository / branch / HEAD:** frick-fsw / main / `833a0e0` (`NC-S4b-09: close — run_sim.sh CMake generator conflict fixed and independently reviewed`)
**Review date:** 2026-11-11
**Reviewer role:** Senior Aeronautics and Space Software Product Assurance Engineer (independent PA; N20). Not a certification/approval authority — see authority statement below.
**Change under review:** No source-code change. Increment = re-execution of norm N18 (Renode emulated-target functional validation) on the current tree, with retained evidence. Associated registered NC: **NC-EXT-165** (Major gate NC: "N18 functional validation is not satisfied for the current tree").
**Companion registered NCs in scope:** NC-PAL-01 (flight MPFS MSS watchdog binding — authority-owned), NC-TS-01 (Renode ELF-freshness gate), NC-EXT-239 (RTEMS implicit-declaration compile diagnostic), NC-EXT-40 (mutable toolchain provenance).

---

## 1. Determination (summary)

| Question | Determination |
|---|---|
| 1. N18 PASSES on current tree (exit 0, `RENODE_TEST: PASS`)? | **YES — directly verified.** `build/renode/console.log` on HEAD `833a0e0` contains `RENODE_TEST: PASS`, watchdog `WDOG0 armed` (emulated profile), all 14 HAL checks `PASS`, `sb_roundtrip=PASS`. console.log mtime 2026-11-11 22:58 (today); ELF `build/renode/frick_fsw_renode.elf` built today 18:11, sha256 `55188f5a460bf7c2a97f4540bd8a12f571da56d276394c68a1be25b6a9251f84`, identical to retained NC-EXT-191 evidence ELF. Renode process log shows the machine ran 21:57–21:58 today. |
| 2. N18 is emulated (Renode), not target/HIL; does not close NC-PAL-01 or claim target fidelity? | **YES.** `tools/scripts/renode_vv.sh:1-6` runs the middleware on the "emulated PolarFire SoC (Renode image, pinned)". `docs/vv/verification-results.md:470-474`: "emulated-target functional validation... not HIL, timing/WCET, radiation or mission evidence... does not close NC-PAL-01." Flight hooks remain `FF_NOT_IMPLEMENTED` (`src/core/pal/pal_rtems.c:278-306`). NC-PAL-01 remains OPEN and authority-owned. |
| 3. Companion host gates green (build, CTest, Lean, trace, format, license)? | **YES — documented green in `docs/vv/verification-results.md` at HEAD `833a0e0`.** NC-EXT-237 section (lines 506-511) and NC-EXT-191 section (lines 573-582): host build exit 0 no warnings; CTest 56/56 PASS; Lean `lake build` exit 0; trace PASS (120/76/30/14/68); format-check exit 0 (pinned clang-format 18.1.3); N13 license OK. RTEMS 6.2 cross-build exit 0 (NC-EXT-191 section; the pre-existing `pal_rtems.c:156` implicit-declaration warning recorded there is resolved on HEAD by NC-EXT-239 forward declarations — see PA-REVIEW-NC-EXT-239-pal_rtems-20261111.md). |
| 4. NC-EXT-165 gate requirement satisfied by current-tree N18 PASS? | **Substantively YES; formally pending closure.** All NC-EXT-165 required elements are present on the current tree: fresh RTEMS 6.2 cross-build (`make renode-image` profile, `FF_ENABLE_RENODE_TEST=ON`), prescribed `make renode` (`renode_vv.sh`), retained logs (`build/renode/console.log`, `build/renode/renode-process.log`), and source/image/ELF identity (ELF sha256 `55188f5a…` matches retained evidence; source unchanged since NC-EXT-191 except declaration-only `pal_rtems.c` change from NC-EXT-239). However the `sprint-plan.md` NC-EXT-165 register row (line 284) still states "OPEN — N18 must be re-run on this tree" and has not been updated to reflect the current-tree PASS. Formal closure requires (a) row update, (b) independent R2 verification per the NC's own terms ("verifier: independent R2"). |
| 5. PA findings that block commit/push or NC-EXT-165 resolution? | **No technical block.** No PA finding creates a new commit/push or N18 block. NC-EXT-165 substantive gate is satisfied. Formal NC closure is pending administrative steps (row update + R2 verification). NC-PAL-01 must remain OPEN and must NOT be closed by this review or by N18. |

---

## 2. Evidence reviewed (directly inspected)

| # | Artifact | Content relevant to this review | File:line |
|---|---|---|---|
| E1 | `build/renode/console.log` (untracked build artifact, HEAD `833a0e0`) | `watchdog: WDOG0 armed (emulated profile: timeout 10000 ms, ...)`; `WD armed: timeout 10000 ms, feed 2500 ms`; `[renode-test] hal_*=PASS` (14 HAL classes); `sb_roundtrip=PASS`; `RENODE_TEST: PASS`. mtime 2026-11-11 22:58. | — (untracked; see PA-004) |
| E2 | `build/renode/renode-process.log` | Renode machine started 21:57:47, paused 21:58:00, disposed. Confirms the emulated target executed on the current tree today. | — (untracked; see PA-004) |
| E3 | `build/renode/frick_fsw_renode.elf` | Built today 18:11 on HEAD `833a0e0`. sha256 `55188f5a460bf7c2a97f4540bd8a12f571da56d276394c68a1be25b6a9251f84` — identical to the ELF recorded in retained NC-EXT-191 evidence. | — (untracked; see PA-004) |
| E4 | `tools/scripts/renode_vv.sh` | N18 gate script: emulated PolarFire SoC (Renode, pinned image `antmicro/renode@sha256:7905c329…`); fresh per-run directory via `mktemp -d`; fail-closed capture; verdict delegated to `renode_vv_verdict.sh`; retains console + process logs. | `renode_vv.sh:1-6,33-44,46-51,124-125` |
| E5 | `tools/scripts/renode_vv_verdict.sh` | Fail-closed verdict unit (17 checks, 0 failures per `verification-results.md:10-18`); distinguishes missing/empty/unreadable capture (rc=1/2) from genuine PASS. | `verification-results.md:10-18` |
| E6 | `docs/vv/verification-results.md` (committed at HEAD `833a0e0`) | Documents N18 PASS on current tree across the 2026-11-09, 2026-11-10, and 2026-11-11 cycles; companion host gates green (build exit 0 no warnings; CTest 56/56; Lean exit 0; trace PASS 120/76/30/14/68; format exit 0; license OK; RTEMS cross-build exit 0; smoke PASS; coverage/metrics). Explicitly states N18 is emulated, not HIL, and does not close NC-PAL-01. | Lines 353-364, 401-414, 470-474, 506-511, 569-582 |
| E7 | `docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/gate-renode-n18.log` | `[renode] PASS (RENODE_TEST: PASS; container status=exited container exit code=0)`; run dir `/tmp/frick-renode-vv.Ohz0um`. | — |
| E8 | `docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/README.md` | Evidence identity manifest: source file SHA-256s, ELF sha256 `55188f5a…`, commands and results, open items (NC-EXT-191, NC-PAL-01, NC-EXT-239 untouched). | README.md:41-116 |
| E9 | `docs/planning/sprint-plan.md` | NC-EXT-165 register row (Major gate NC): requires "fresh RTEMS 6.2 cross-build and prescribed `make renode` on the current tree with retained logs and source/image/ELF identity"; states "blocks N7 commit/push until satisfied"; verifier: independent R2. Also NC-TS-01 (ELF-freshness), NC-EXT-239, NC-PAL-01 rows. | Lines 284 (NC-EXT-165), 242 (NC-TS-01), 326 (NC-EXT-239), 647 (NC-PAL-01), 831 (NC-EXT-239 status) |
| E10 | `docs/vv/tool-qualification.md` T17 row | Renode V&V environment (norm N18). Current text still states "no Renode PASS is claimed for the current tree (NC-EXT-165)" referencing only the 2026-10-20 state — stale relative to verified current-tree PASS. | Line 40 |
| E11 | `src/core/pal/pal_rtems.c` | NC-EXT-239 fix (HEAD includes commit `0cc455f`): forward declarations `ff_pal_wd_hw_arm/feed/reset` at :111-113 resolve the RTEMS cross-build implicit-declaration diagnostic. Declaration-only; no logic/hardware change. Flight hooks `FF_NOT_IMPLEMENTED` at :278-306 (NC-PAL-01). Emulated binding guarded by `:93` (`FRICK_PLATFORM_MPFS && FF_ENABLE_RENODE_TEST`). | :93, :111-113, :147, :167, :228, :253, :278-306 |
| E12 | `docs/reviews/PA-REVIEW-NC-EXT-239-pal_rtems-20261111.md` | Prior independent PA review (N20, 2026-11-11) of NC-EXT-239: resolved from PA perspective on host/SIL (emulated N18) evidence base; NC-PAL-01 remains OPEN; no PA finding blocks commit/push. | Full document |
| E13 | Git lineage | `ea0ac69` (NC-EXT-191) → `0cc455f` (NC-EXT-239, pal_rtems.c forward declarations) → `833a0e0` (HEAD, docs/reviews only). `git diff 0cc455f..HEAD -- src/ config/ tests/ tools/ verification/ CMakeLists.txt Makefile` = empty (no source change since NC-EXT-239). | `git log` / `git diff` |

---

## 3. Findings

### PA-001 — Major — NC-EXT-165 register row not current; substantive gate satisfied, formal closure pending independent R2 verification

**File:line:** `docs/planning/sprint-plan.md:284` (NC-EXT-165 row)

**Finding:** NC-EXT-165 is registered as a "Major gate NC" whose stated exit condition is "fresh RTEMS 6.2 cross-build and prescribed `make renode` on the current tree with retained logs and source/image/ELF identity" and which "blocks N7 commit/push until satisfied." Direct evidence on HEAD `833a0e0` confirms every exit element: fresh RTEMS cross-build (ELF built today 18:11, sha256 `55188f5a…`); prescribed `make renode` executed (`renode_vv.sh`, fail-closed, container exit 0); retained logs (`build/renode/console.log` with `RENODE_TEST: PASS` + 14 HAL checks PASS + emulated WDOG0 armed; `build/renode/renode-process.log` showing the machine ran 21:57–21:58 today); ELF identity matches retained NC-EXT-191 evidence; source functionally unchanged since NC-EXT-191 (NC-EXT-239 `pal_rtems.c` change is declaration-only, does not alter the binary). The NC-EXT-165 row nevertheless still reads "OPEN — N18 must be re-run on this tree" and has not been updated to record the current-tree PASS. Formal closure additionally requires independent R2 verification (the NC's own terms: "verifier: independent R2").

**Recommended disposition:**
1. Update the `sprint-plan.md` NC-EXT-165 row to record the current-tree N18 PASS evidence (E1-E3, E7, ELF sha256 `55188f5a…`, tree HEAD `833a0e0`, run date 2026-11-11) and change status from "OPEN" to reflect satisfaction, pending R2 disposition.
2. Obtain independent R2 verification of the N18 re-run evidence (per NC-EXT-165 terms) before formal NC closure.
3. Record NC-PAL-01 as still OPEN in any closure record (see PA-006). NC-EXT-165 closure must not imply or subsume NC-PAL-01.

**Blocks commit/push?** No new technical block — the substantive N18 gate evidence satisfies NC-EXT-165's exit condition. The remaining items (row update + R2 verification) are administrative closure steps.
**Blocks NC-EXT-165 resolution?** Formally yes, until the row is updated and R2 verification is recorded. Substantively no — the required N18 evidence is present.

---

### PA-002 — Minor — `tool-qualification.md` T17 row stale relative to current-tree N18 PASS

**File:line:** `docs/vv/tool-qualification.md:40`

**Finding:** The T17 (Renode V&V environment, norm N18) row states: "no Renode PASS is claimed for the current tree (NC-EXT-165, FRICK-REVIEW-0406 F-11)" and scopes the PASS to the 2026-10-20 tree state only. This contradicts `docs/vv/verification-results.md` (which documents current-tree N18 PASS on 2026-11-09/10/11) and the directly verified `build/renode/console.log` (`RENODE_TEST: PASS` on HEAD `833a0e0`). This is the same class of documentation-currency issue previously registered as NC-EXT-176 (marked "corrected" but the T17 text still contains the stale "no Renode PASS is claimed for the current tree" language).

**Recommended disposition:** Update T17 to reflect the current-tree N18 PASS (evidence E1-E7, verification-results.md sections dated 2026-11-09/10/11). Retain the NC-TS-01 ELF-freshness caveat and NC-EXT-40 mutable-toolchain-provenance caveat as separate open tooling items. This is documentation currency (norms N8/N11/N16), non-functional.

**Blocks commit/push?** No.
**Blocks NC-EXT-165 resolution?** No.

---

### PA-003 — Minor — Evidence-integrity: NC-EXT-191 evidence README source-identity hashes do not match committed blobs for `link_ip.c` / `link_tsn.c`

**File:line:** `docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/README.md:113-114`

**Finding:** The retained NC-EXT-191 evidence identity manifest records `link_ip.c` sha256 `c62f99751b77e8dd2258c4c032d3b0278fd4a5c12c854616753381c064461324` and `link_tsn.c` sha256 `d019bb9dd21d353806ca2ec2d8324cd4c3563ed48c2d3d2f05356ebd7fea56e0`. Direct inspection shows the committed blobs at both NC-EXT-191 (`ea0ac69`) and HEAD `833a0e0` are `link_ip.c` `9db045905dca12e88e74b8aa2f33653bfa5bba8655f9c8d46d1e0714345b635c` and `link_tsn.c` `9f1b60048d769ee574ca2a97c751fdeec5062c7607c5b44d3b03692e218f3b39`. Only `link.h` (`c06d2fadb2e1051961caf0df02b3dd5540639ffd84ebde6eb413c2aaaa699bda`) matches. The N18 verdict itself depends on the ELF binary (sha256 `55188f5a…`, which matches between evidence and current build), not on these source hashes, so the N18 PASS conclusion is unaffected; however source-artifact identity traceability (norms N4/N11) for the retained evidence package is broken for two of three files.

**Recommended disposition:** Correct the `link_ip.c` and `link_tsn.c` SHA-256 entries in the evidence README to match the committed blobs (`9db0459…` / `9f1b600…`), re-run the identity verification, and record the correction. Retain the original hashes as superseded history if an audit trail is desired.

**Blocks commit/push?** No.
**Blocks NC-EXT-165 resolution?** No.

---

### PA-004 — Minor — Today's N18 console.log / process.log / ELF are untracked build artifacts, not committed to a controlled evidence directory

**File:line:** `build/renode/console.log`, `build/renode/renode-process.log`, `build/renode/frick_fsw_renode.elf` (all confirmed gitignored/untracked via `git check-ignore`)

**Finding:** The direct N18 PASS evidence on HEAD `833a0e0` (console.log showing `RENODE_TEST: PASS`, process log, ELF sha256 `55188f5a…`) resides in `build/renode/`, which is gitignored and untracked. It is therefore not configuration-controlled repository evidence under norm N11 (records retention / evidence integrity). The committed N18 evidence package (`docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/`) was captured on tree `ea0ac69`; although the ELF is bit-identical on HEAD (declaration-only `pal_rtems.c` change), the today's console/process logs are not archived in a committed evidence directory with run date, tree identity, ELF hash, and container exit code.

**Recommended disposition:** Archive `build/renode/console.log` and `build/renode/renode-process.log` (and a record of the ELF sha256 `55188f5a…`, tree HEAD `833a0e0`, run date 2026-11-11, container status=exited exit code=0) into a committed N18 evidence directory following the NC-EXT-191 pattern (README.md + gate log + assets manifest). Reference it from `verification-results.md` and the NC-EXT-165 row.

**Blocks commit/push?** No.
**Blocks NC-EXT-165 resolution?** No (the evidence exists and is directly verifiable; this is an evidence-retention hygiene finding).

---

### PA-005 — Minor (observation) — NC-TS-01 residual: Renode gate still keys on ELF existence, not freshness

**File:line:** `tools/scripts/renode_vv.sh:41-44`; `docs/planning/sprint-plan.md:242` (NC-TS-01 row)

**Finding:** `renode_vv.sh` gates the N18 run on the *existence* of `build/renode/frick_fsw_renode.elf`, not on its freshness relative to the source tree (NC-TS-01, registered Minor tooling/evidence-integrity NC, open). The current run was mitigated by a fresh build (ELF built today immediately before the Renode run), so this run is not stale. However the gate implementation itself is unchanged: a stale pre-change ELF would still satisfy the gate. This is non-functional (N21) and does not affect the validity of today's fresh-build PASS.

**Recommended disposition:** Retain NC-TS-01 as open. Recommended closure: make the Renode gate compare the ELF hash/mtime against the source tree (or rebuild the image as part of the gate) and fail/record staleness rather than passing. Owner: tooling/V&V (T17/N10); verifier: independent R2/R4.

**Blocks commit/push?** No.
**Blocks NC-EXT-165 resolution?** No (today's run used a fresh build; NC-TS-01 is a separate tooling item).

---

### PA-006 — Confirmation (not a new NC) — NC-PAL-01 is NOT closed by N18; remains authority-open

**File:line:** `src/core/pal/pal_rtems.c:93,278-306`; `tools/scripts/renode_vv.sh:1-6`; `docs/vv/verification-results.md:470-474`; `docs/reviews/PA-REVIEW-NC-EXT-239-pal_rtems-20261111.md:16-18,65-71`

**Finding:** N18 is emulated-target functional validation on Renode (not target/HIL). The flight-target watchdog hooks remain `FF_NOT_IMPLEMENTED` (`pal_rtems.c:278-306`); the MPFS MSS hardware watchdog binding is unwired. NC-PAL-01 (flight MPFS MSS watchdog binding — Major FDIR completeness, target/HIL closure with authorized-human sign-off) remains OPEN and authority-owned. N18 establishes no target behaviour, reset scope, timing/WCET, or flight watchdog readiness. This review must not close NC-PAL-01, and nothing in the N18 re-run does so.

**Recommended disposition:** Record NC-PAL-01 as OPEN in any NC-EXT-165 closure record. Require target/HIL evidence and authorized human sign-off (closure conditions HL-1/HL-2 per `docs/architecture/mpfs-mss-watchdog-integration-spec.md`) before NC-PAL-01 disposition.

**Blocks commit/push?** No (N18 does not depend on NC-PAL-01).
**Blocks NC-EXT-165 resolution?** No — but NC-PAL-01 must not be conflated with or closed by NC-EXT-165.

---

### PA-007 — Confirmation (not a new NC) — NC-EXT-239 resolved from PA perspective on emulated N18 evidence base

**File:line:** `src/core/pal/pal_rtems.c:104-113`; `docs/reviews/PA-REVIEW-NC-EXT-239-pal_rtems-20261111.md`

**Finding:** NC-EXT-239 (RTEMS cross-build implicit-declaration warning for `ff_pal_wd_hw_*` in `pal_rtems.c:156`) is resolved by the forward declarations committed in `0cc455f` (included in HEAD `833a0e0`). The fix is declaration-only (11 insertions, 0 deletions), no logic/hardware change, flight hooks unaffected. Prior independent PA review (PA-REVIEW-NC-EXT-239-pal_rtems-20261111.md) concluded: resolved from PA perspective on host/SIL (emulated N18) evidence base; NC-PAL-01 remains open; no PA finding blocks commit/push.

**Recommended disposition:** No action required from this review. Note that NC-EXT-239's sprint-plan status row (line 831, "Deferred (non-functional, N21)") appears stale relative to the committed fix; update as part of documentation currency housekeeping.

**Blocks commit/push?** No.
**Blocks NC-EXT-165 resolution?** No.

---

## 4. Companion host gates — verification summary (documented at HEAD `833a0e0`)

| Gate | Result | Evidence location | Notes |
|---|---|---|---|
| Host build (all targets) | PASS, exit 0, no warnings | `verification-results.md:506` (NC-EXT-237), `:573` (NC-EXT-191) | `BUILD_DIR=build2/native make build` |
| CTest | PASS, 56/56, exit 0 | `verification-results.md:507`, `:574` | NC-EXT-237 and NC-EXT-191 cycles |
| Lean 4 (`lake build`) | PASS, exit 0 | `verification-results.md:508`, `:575` | `verification/lean` |
| Traceability (`trace.py`) | PASS, 120/76/30/14/68 | `verification-results.md:509`, `:576` | `docs/requirements/traceability.yaml` |
| Format-check | PASS, exit 0, pinned clang-format 18.1.3 | `verification-results.md:510`, `:577` | `--dry-run --Werror` over `src config tests` |
| N13 license | OK | `verification-results.md:511`, GSW/license log | `license_check.py` + `make gsw-selfcheck` |
| Smoke (native simulator) | PASS, 7 applications | `verification-results.md:579` | `[smoke] PASS (banner + 7 applications registered)` |
| Coverage / metrics | PASS, exit 0 | `verification-results.md:580` | 15328/32948 statements (46.5%, indicator only) |
| RTEMS 6.2 cross-build (N18 profile) | PASS, exit 0 | `verification-results.md:363`, `:413` | `frick-rtems-toolchain:local`, `FF_ENABLE_RENODE_TEST=ON`; ELF produced; on HEAD the NC-EXT-239 `pal_rtems.c:156` implicit-declaration warning is resolved by forward declarations |
| **N18 Renode** | **PASS**, `RENODE_TEST: PASS`, container status=exited exit code=0 | E1 `build/renode/console.log` (directly verified), E7 `gate-renode-n18.log`, `verification-results.md:364`, `:414` | Emulated PolarFire SoC, not HIL/target |

All companion gates are green on the current tree. Source has not functionally changed since the NC-EXT-191 / NC-EXT-237 evidence was produced (only the declaration-only `pal_rtems.c` change from NC-EXT-239 and docs/reviews in HEAD).

---

## 5. NC-EXT-165 resolution determination

**Substantive gate requirement: SATISFIED.** All elements required by the NC-EXT-165 row (`docs/planning/sprint-plan.md:284`) are present on HEAD `833a0e0`:
- Fresh RTEMS 6.2 cross-build (`make renode-image` profile, `FF_ENABLE_RENODE_TEST=ON`) — ELF built today on HEAD, sha256 `55188f5a…`.
- Prescribed `make renode` (`tools/scripts/renode_vv.sh`) — executed today, fail-closed, container status=exited exit code=0.
- Retained logs — `build/renode/console.log` (`RENODE_TEST: PASS`, 14 HAL checks PASS, emulated WDOG0 armed) and `build/renode/renode-process.log` (machine ran 21:57–21:58 today).
- Source/image/ELF identity — ELF sha256 matches retained NC-EXT-191 evidence; source unchanged since NC-EXT-191 except declaration-only NC-EXT-239 `pal_rtems.c` change (no binary effect).

**Formal NC closure: PENDING.** Two administrative steps remain:
1. Update the `sprint-plan.md` NC-EXT-165 row (line 284) to record the current-tree N18 PASS and change status from "OPEN — N18 must be re-run on this tree."
2. Obtain independent R2 verification (the NC's own terms: "verifier: independent R2") before formal closure.

**Commit/push impact:** No PA finding creates a new technical block to N7 commit/push. The N18 gate evidence satisfies NC-EXT-165's exit condition. NC-PAL-01 remains OPEN and must not be closed (PA-006). NC-TS-01 (PA-005) and NC-EXT-40 remain open tooling items but do not gate N18.

---

## 6. Uncertainty and confidence report

| Field | Content |
|---|---|
| Decision/product and configuration | N18 re-run / NC-EXT-165 on frick-fsw HEAD `833a0e0`, emulated PolarFire SoC (Renode), RTEMS 6.2 `riscv/mpfs64imafdc`, `FF_ENABLE_RENODE_TEST=ON` |
| Claim type | Fact (direct evidence E1-E3, E7), Inference (source unchanged since NC-EXT-191 except declaration-only change), Recommendation (PA-001..005 dispositions) |
| Applicable baseline and approved tailoring | Project norms N1-N22 (`docs/rules.md`); N20 independent PA review; ECSS-Q-ST-80C Rev.2 / ECSS-E-ST-40C Rev.1 as invoked by project norms. N18 gate per `AGENTS.md` and `sprint-plan.md` NC-EXT-165 row. |
| Evidence consulted | E1 `build/renode/console.log`; E2 `build/renode/renode-process.log`; E3 `build/renode/frick_fsw_renode.elf` (sha256 `55188f5a…`); E4-E5 `renode_vv.sh` / `renode_vv_verdict.sh`; E6 `verification-results.md` at HEAD `833a0e0`; E7 `gate-renode-n18.log`; E8 `docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/README.md`; E9 `sprint-plan.md:284`; E10 `tool-qualification.md:40`; E11 `pal_rtems.c:93,111-113,278-306`; E12 prior PA review of NC-EXT-239; E13 git lineage/diff |
| Cross-check and reconciliation | ELF sha256 `55188f5a…` identical between current build and retained NC-EXT-191 evidence; `git diff 0cc455f..HEAD` over source = empty (only docs/reviews changed); `pal_rtems.c` forward declarations are declaration-only (no binary effect); NC-PAL-01 confirmed open across `pal_rtems.c`, `handover.md`, `status.json`, `escalations.md` ESC-3 |
| Confidence | **High** for N18 PASS on current tree (direct console.log/process.log/ELF evidence, no conflict). **High-with-bounded-note** for formal NC-EXT-165 closure (evidence complete; closure pending administrative row update + independent R2 verification). |
| Missing/inaccessible evidence | None for N18 PASS. Formal R2 verification of NC-EXT-165 not yet performed (outside this review). `frick-rtems-toolchain:local` cross-build re-run on HEAD not independently re-executed by PA in this session (relies on documented exit 0 in verification-results.md and the bit-identical ELF). |
| Assumptions and expiry | Assumes `build/renode/` artifacts were produced from HEAD `833a0e0` source (confirmed: clean git tree, gitignored build dir, ELF built today 18:11 after last commit). Re-check if toolchain/image or source changes. |
| Consequences if wrong | If the console.log were not from HEAD `833a0e0`, the N18 PASS claim would be invalid; mitigated by ELF hash matching retained evidence and clean git tree. No flight-safety impact (N18 is emulated only; NC-PAL-01 open). |
| Required independent human reviewer/authority | FSW engineer (commit authority, norm N7); independent R2 verifier (NC-EXT-165 terms); authorized human authority for NC-PAL-01 disposition / flight watchdog binding (target/HIL sign-off). |

---

## 7. Authority statement

This report is an independent product assurance assessment only. It does not certify compliance, qualify a tool, accept residual risk, close a nonconformance, authorize release, or declare flightworthiness. It does **not** close NC-PAL-01 (authority-owned, target/HIL) or NC-EXT-165 (formal closure requires row update + independent R2 verification). It does not establish target/HIL fidelity, timing/WCET, radiation, or mission suitability. Independent qualified humans retain all certification, safety, release, and risk-acceptance decisions.

---

**PA review outcome:** N18 re-run PASSES on current tree HEAD `833a0e0` (emulated Renode, `RENODE_TEST: PASS`, container exit 0). Companion host gates green. NC-EXT-165 substantive gate requirement satisfied; formal closure pending sprint-plan row update + independent R2 verification. NC-PAL-01 remains OPEN and must not be closed. No PA finding blocks commit/push on technical grounds.

**Findings register:** PA-001 (Major), PA-002 (Minor), PA-003 (Minor), PA-004 (Minor), PA-005 (Minor observation), PA-006 (confirmation), PA-007 (confirmation).
