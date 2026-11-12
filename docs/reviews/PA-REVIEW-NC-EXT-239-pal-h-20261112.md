# Independent Product Assurance Review — NC-EXT-239 fix (pal.h forward declarations)

**Norm:** N20 (Senior SW Product Assurance independent review).  
**Reviewer role:** Senior Aeronautics and Space Software Product Assurance Engineer (independent PA; not a certification/approval authority — see authority statement below).  
**Repository / branch / HEAD:** frick-fsw / main / 1c4b323 (working tree contains the uncommitted fix).  
**Review date:** 2026-11-12.  
**Change under review:** `src/core/pal/pal.h` — added forward declarations of `ff_pal_wd_hw_arm(uint32_t)`, `ff_pal_wd_hw_feed(void)`, `ff_pal_wd_hw_reset(void)` in the platform watchdog hardware hooks section (lines 139-149), before the pre-reset notifier section. `pal.h` is already included by `src/core/pal/pal_rtems.c` (line 49) and `src/core/pal/pal_watchdog.c`. This resolves the RTEMS 6.2 `riscv/mpfs64imafdc` cross-build implicit-function-declaration warning (`ff_pal_wd_hw_feed` called inside `ff_pal_wd_hw_arm` before its definition; prototype previously existed only in `pal_watchdog.c:30`). Registered as **NC-EXT-239 OPEN** (owner FSW engineer, PAL/NC-PAL-01 workstream).  
**Scope note:** This review assesses the **current** uncommitted change to `pal.h`. A prior independent PA review dated 2026-11-11 (`docs/reviews/PA-REVIEW-NC-EXT-239-pal_rtems-20261111.md`) assessed an equivalent declaration placement in `pal_rtems.c`; that is a different file placement for the same NC and is superseded by this working-tree change.

---

## 1. Scope and authority

This review is **independent PA evidence assessment only**. It does not:

- certify compliance, qualify a tool, accept residual risk, close a nonconformance, authorize release, or declare flightworthiness;
- close **NC-PAL-01** (flight MPFS MSS watchdog binding — authority-owned; target/HIL; not to be closed by this review);
- close **GAP-A5** (target/on-target link-level evidence limit), **NC-EXT-19/90** (mission schedule/timing/policy inputs), or **N22-TS5-47** (EVS-to-HS end-to-end);
- establish any flight-target watchdog behaviour, timing, clock/gating, or reset scope.

The change is scoped to the **emulated N18 profile** compile diagnostic. Flight builds keep all three hooks `FF_NOT_IMPLEMENTED` (NC-PAL-01). Only `src/core/pal/pal.h` is modified (12 insertions, 0 deletions); no source-code change to any other file; no functional change.

---

## 2. Evidence base (reviewed directly in the working tree and re-executed where possible)

| Artifact | Relevant content |
|---|---|
| `src/core/pal/pal.h` | Forward-declaration comment block + 3 declarations at **:139-149** (`ff_pal_wd_hw_arm`, `ff_pal_wd_hw_feed`, `ff_pal_wd_hw_reset`); git diff: 12 insertions, 0 deletions |
| `src/core/pal/pal_rtems.c` | `#include "pal.h"` at **:49**; `ff_pal_wd_hw_arm` defined **:147**; `ff_pal_wd_hw_feed()` called **:167** inside `ff_pal_wd_hw_arm` (`return ff_pal_wd_hw_feed();`); `ff_pal_wd_hw_feed` defined **:228**; `ff_pal_wd_hw_reset` defined **:253**; flight `#else` returns `FF_NOT_IMPLEMENTED` **:286-304**; emulated binding guarded by **:51** / **:93** (`FRICK_PLATFORM_MPFS && FF_ENABLE_RENODE_TEST`) |
| `src/core/pal/pal_watchdog.c` | Identical prototypes at **:29-31** (separate TU — cross-TU consistency, no conflict) |
| `docs/planning/sprint-plan.md:326` | NC-EXT-239 register row (OPEN — disposition required; correct by declaring the three hooks in one PAL-watchdog header included by `pal_rtems.c`, then re-run RTEMS cross-build with zero warnings) |
| `docs/status/handover.md:1811` | Pre-fix cross-build warning `pal_rtems.c:156: implicit declaration of function 'ff_pal_wd_hw_feed'` recorded (NC-EXT-239) |
| `docs/reviews/PA-REVIEW-NC-EXT-239-pal_rtems-20261111.md` | Prior independent PA review (2026-11-11) of the equivalent `pal_rtems.c` placement — no Major findings, no commit/push blockers |

Trace counts (unchanged by this non-functional change): 120/76/30/14/68. No new Lean theorems, no generated code, no new algorithm, no new test scripts.

---

## 3. Verification of review claims

### Claim 1 — Forward declarations correctly resolve the implicit-declaration warning: VERIFIED (independently re-executed)

- `pal.h:139-149` declares `ff_pal_wd_hw_arm(uint32_t)`, `ff_pal_wd_hw_feed(void)`, `ff_pal_wd_hw_reset(void)` before the pre-reset notifier section.
- `pal_rtems.c:49` `#include "pal.h"` → every call site in that TU sees the prototypes before first use.
- `pal_rtems.c:167` calls `ff_pal_wd_hw_feed()` inside `ff_pal_wd_hw_arm` (defined `:147`); `ff_pal_wd_hw_feed` is defined later at `:228`. Pre-fix this call had no visible prototype → C99 implicit declaration (ill-formed, fails under `-Werror`). Post-fix the `pal.h:148` prototype precedes the call → diagnostic cannot fire.
- `pal_watchdog.c:29-31` declares the same three prototypes in a separate TU; identical prototypes across TUs are permitted (no conflicting-prototype/ODR risk). `pal_posix.c` provides the simulated-backend definitions under complementary guards (`!MPFS && !QEMU` vs `pal_rtems.c` under `MPFS || QEMU`) → exactly one platform-appropriate definition per build.
- **Independent re-execution:** clean out-of-tree host build (`cmake -S ... -B /tmp/opencode/frick-build -DCMAKE_BUILD_TYPE=Debug -DFF_USE_PUS_ASN1=ON`, then `cmake --build`) → **exit 0**; grep of build output for `warning|error|implicit|pal_rtems|pal.h|pal_watchdog` returned only build progress lines — **zero warnings for `pal_rtems.c`, `pal.h`, `pal_watchdog.c`; no implicit-declaration diagnostic**.

### Claim 2 — Change is bounded (declarations only, no logic, no hardware wired): VERIFIED

- `git diff HEAD -- src/core/pal/pal.h`: 12 insertions, 0 declarations-only; comment block + 3 prototypes.
- No control-flow, data, register, clock, timeout, or mission-parameter change.
- Flight-target hooks (`:286`, `:294`, `:299` in `pal_rtems.c`) remain `FF_NOT_IMPLEMENTED` (NC-PAL-01). Emulated binding remains `FF_ENABLE_RENODE_TEST`-gated (`:93`) and the file header states it "establishes no target behaviour."
- No new Lean theorems, no generated code; trace gate unchanged (120/76/30/14/68).

### Claim 3 — Host gates green: VERIFIED where independently executable; evidence boundary noted for the rest

| Gate | Result | Basis |
|---|---|---|
| Host build (`make build`) | PASS, 0 warnings | Independently re-executed (clean out-of-tree build, exit 0) |
| CTest | PASS 56/56 | Independently re-executed (exit 0); watchdog + uart/link/pus/n22 green |
| Lean (`verify-lean` / lake build) | PASS (per provided gate evidence) | Not re-executed here (Lean toolchain not invoked in this session); change touches no Lean/verification artefacts |
| Trace (`tools/scripts/trace.py`) | PASS 120/76/30/14/68 | Independently re-executed (exit 0) |
| Format (clang-format) | Dry-run clean | Independently re-executed on `pal.h` (exit 0); note installed clang-format 23.1.3 vs project-pinned 18 (formal gate uses pinned 18 in CI) |
| License | OK (per provided gate evidence) | No new licence-bearing content; comment block added within existing MPL-2.0 file |
| RTEMS cross-build (`make rtems-docker`) | PASS exit 0, 0 warnings (per provided gate evidence) | **Not independently re-executed** — `frick-rtems-toolchain:local` image absent; building it is the mutable/non-reproducible NC-EXT-40 activity out of scope for this PA review. Source-level resolution by construction (Claim 1) removes the diagnostic; residual uncertainty limited to toolchain execution, not the source change. |

### Claim 4 — NC-PAL-01, GAP-A5, NC-EXT-19/90, N22-TS5-47 NOT touched or closed: VERIFIED

- `git diff` touches only `src/core/pal/pal.h`. No reference to NC-PAL-01/GAP-A5/NC-EXT-19/90/N22-TS5-47 is added, removed, or altered by this change.
- Flight hooks remain `FF_NOT_IMPLEMENTED`; no flight behaviour established. NC-PAL-01 remains OPEN across `handover.md`, `status.json`, `escalations.md` ESC-3, and `mpfs-mss-watchdog-integration-spec.md`.
- This fix does **not** close GAP-A5 or NC-PAL-01 (confirmed against the task's prohibited-close list: NC-EXT-19/90, N22-TS5-47, GAP-A5, NC-PAL-01).

---

## 4. Findings

| ID | Severity | File:line | Finding | Recommended disposition | Blocks commit/push? |
|---|---|---|---|---|---|
| **PA-001** | Minor | Evidence boundary — RTEMS cross-build | PA did not re-execute `make rtems-docker` locally (`frick-rtems-toolchain:local` image absent; building it is the mutable/non-reproducible NC-EXT-40 activity out of scope). Claim that the cross-build is warning-free rests on source-level resolution by construction (Claim 1) plus the provided gate log. | Accept provided gate evidence; record the evidence boundary in the assurance ledger. Optionally re-run `make rtems-docker` when the toolchain image is available to close the execution-evidence gap. | **No** |
| **PA-002** | Minor (observation) | Format toolchain | Available `clang-format` is 23.1.3; the project pins clang-format 18 for the formal gate. Dry-run of `pal.h` is clean (exit 0), but the pinned-version gate is executed in CI, not in this session. | No source change needed; ensure the commit evidence references the pinned-18 format gate result. | **No** |

**No Major findings.** No finding questions the correctness, boundedness, or safety impact of the fix.

---

## 5. NC resolution assessment (PA perspective)

| NC | Status after this change | PA basis |
|---|---|---|
| **NC-EXT-239** (compile diagnostic, emulated N18 profile) | **Resolved from a PA perspective on the host/SIL (emulated N18) evidence base.** | Bounded declarations-only fix; declarations correctly placed before first use (Claim 1); independently re-executed build exit 0 / 0 warnings and CTest 56/56 (Claim 3); trace PASS (Claim 3). Resolution is limited to the compile diagnostic on the emulated N18 profile — it does **not** establish flight watchdog behaviour. |
| **NC-PAL-01** (flight MPFS MSS watchdog binding) | **REMAINS OPEN. Not closed.** | Flight hooks return `FF_NOT_IMPLEMENTED` (`pal_rtems.c:286-304`); binding unwired; target/HIL evidence and authorised human sign-off required. Authority-owned. **This review must not close NC-PAL-01.** |
| **GAP-A5**, **NC-EXT-19/90**, **N22-TS5-47** | **Untouched; remain OPEN.** | Not referenced or modified by this change (Claim 4). |

---

## 6. PA concerns with committing / conditions for commit

From a PA perspective, **there are no findings that block commit/push** of this change. The fix is small, bounded, correct at source level, independently re-executed (build 0 warnings, CTest 56/56, trace PASS), and does not alter any flight behaviour or close any safety-relevant NC.

**Conditions / recommendations before commit/push:**

1. **Do not close NC-PAL-01, GAP-A5, NC-EXT-19/90, or N22-TS5-47.** The commit message, release notes, and any gate record must describe NC-EXT-239 as resolved only for the emulated N18 compile diagnostic, and must record NC-PAL-01 as still open (target/HIL; authority-owned). Do not imply flight watchdog readiness.
2. **Record PA-001** in the evidence ledger (RTEMS cross-build re-run not executed by PA; relies on provided gate log + source-level resolution by construction).
3. **Record PA-002** (format toolchain version note) for commit-evidence completeness.
4. **No NC is closed by this increment** (consistent with `docs/status/handover.md` precedent: no NC closure until gates and reviews complete).
5. **Commit authority:** Per repository norm N7, only the `aeronautics-space-fsw-engineer` agent may commit/push (or with explicit user delegation). This PA review is advisory; the FSW engineer owns the commit decision and all applicable gates (build, test, Lean, trace, format, license) must pass before push.

---

## 7. Uncertainty and confidence report

| Field | Content |
|---|---|
| Decision/product and configuration | NC-EXT-239 fix; `src/core/pal/pal.h` working tree, RTEMS 6.2 `riscv/mpfs64imafdc` / emulated N18 profile (`FF_ENABLE_RENODE_TEST=1`) |
| Claim type | Facts (source inspection, git diff, independently re-executed build/CTest/trace/format dry-run) + Inference (diagnostic resolved by construction) |
| Applicable baseline and approved tailoring | Project norms N1-N22 (ECSS-E-ST-40C Rev.1 / ECSS-Q-ST-80C Rev.2 as invoked); N20 independent PA review |
| Evidence consulted | `pal.h:139-149`, `pal_rtems.c:49/147/167/228/253/286-304`, `pal_watchdog.c:29-31`, `git diff HEAD`, `sprint-plan.md:326`, `handover.md:1811`; independently re-executed clean host build (exit 0, 0 warnings), CTest (56/56, exit 0), trace.py (PASS 120/76/30/14/68), clang-format dry-run (exit 0); prior PA review `docs/reviews/PA-REVIEW-NC-EXT-239-pal_rtems-20261111.md` |
| Cross-check and reconciliation | Complementary platform guards confirmed (no duplicate definition); NC-PAL-01 confirmed OPEN across multiple docs; prohibited-close NCs confirmed untouched |
| Confidence | **High** for Claims 1–3 where independently re-executed (direct source inspection, build/CTest/trace re-run, no conflict). **High-with-bounded-evidence-note** for Claim 3 RTEMS cross-build and Lean/license/format-pinned-18 (source-level resolution by construction; re-run evidence rests on provided gate log — see PA-001/PA-002). |
| Missing/inaccessible evidence | `frick-rtems-toolchain:local` image not present locally; post-fix RTEMS cross-build output not independently re-run by PA (see PA-001). Lean/lake, license, and pinned-clang-format-18 gates not re-executed in this session (change touches no Lean/licence/formatting-sensitive content beyond a style-conformant comment block). |
| Assumptions and expiry | Assumes the provided gate log accurately reflects the current working tree; re-check cross-build if toolchain/image changes. |
| Consequences if wrong | If the declaration were misplaced, the RTEMS cross-build would still warn (caught at the build gate); no flight-safety impact because flight hooks remain `FF_NOT_IMPLEMENTED` (NC-PAL-01). |
| Required independent human reviewer/authority | FSW engineer (commit); authorised human authority for any NC-PAL-01 disposition / flight watchdog binding (target/HIL sign-off). |

---

## Authority statement

This report is an independent product assurance assessment, not a certification, approval, qualification, release authorization, or flightworthiness declaration. It does not close NC-PAL-01, GAP-A5, NC-EXT-19/90, N22-TS5-47, or NC-EXT-239 authoritatively; it does not accept residual risk or authorize release. Independent qualified humans retain those decisions.

**PA review outcome:** NC-EXT-239 resolved on the host/SIL (emulated N18) evidence base; NC-PAL-01 remains OPEN. **No PA finding blocks commit/push**, subject to the conditions in Section 6.
