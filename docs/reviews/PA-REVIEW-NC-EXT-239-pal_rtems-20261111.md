# Independent Product Assurance Review — NC-EXT-239 fix (pal_rtems.c forward declarations)

**Norm:** N20 (Senior SW Product Assurance independent review).  
**Reviewer role:** Senior Aeronautics and Space Software Product Assurance Engineer (independent PA; not a certification/approval authority — see authority statement below).  
**Repository / branch / HEAD:** frick-fsw / main / ea0ac69 (working tree contains the uncommitted fix).  
**Review date:** 2026-11-11.  
**Change under review:** `src/core/pal/pal_rtems.c` — added forward declarations of `ff_pal_wd_hw_arm`, `ff_pal_wd_hw_feed`, `ff_pal_wd_hw_reset` to resolve an RTEMS 6.2 `riscv/mpfs64imafdc` cross-build implicit-function-declaration warning (`ff_pal_wd_hw_feed` called at line 156 pre-fix, defined at line 217 pre-fix). Registered as **NC-EXT-239 OPEN** (owner FSW, PAL/NC-PAL-01 workstream).

---

## 1. Scope and authority

This review is **independent PA evidence assessment only**. It does not:

- certify compliance, qualify a tool, accept residual risk, close a nonconformance, authorize release, or declare flightworthiness;
- close **NC-PAL-01** (flight MPFS MSS watchdog binding — authority-owned; target/HIL; not to be closed by this review);
- establish any flight-target watchdog behaviour, timing, clock/gating, or reset scope.

The change is scoped to the **emulated N18 profile** (`FRICK_PLATFORM_MPFS && FF_ENABLE_RENODE_TEST == 1`) compile diagnostic. Flight builds keep all three hooks `FF_NOT_IMPLEMENTED` (NC-PAL-01).

---

## 2. Evidence base (reviewed directly in the working tree)

| Artifact | Relevant content |
|---|---|
| `src/core/pal/pal_rtems.c` | Fix context (full file read); forward declarations at :111-113; `ff_pal_wd_hw_arm` def :147; `ff_pal_wd_hw_feed()` call :167; `ff_pal_wd_hw_feed` def :228; `ff_pal_wd_hw_reset` def :253; flight `#else` NOT IMPLEMENTED :278-306; platform guards :51 and :93 |
| `src/core/pal/pal_watchdog.c` | Identical prototypes at :29-31 (separate translation unit — no conflict) |
| `src/core/pal/pal_posix.c` | Simulated backend defs :673-703, guarded by `!MPFS && !QEMU` at :658 (never co-compiled with pal_rtems.c on RTEMS MPFS/QEMU) |
| `src/core/pal/pal.h` | Exposes only public `FF_PalWatchdog*` API; does NOT declare the internal `ff_pal_wd_hw_*` hooks (no redeclaration risk) |
| `git diff HEAD -- src/core/pal/pal_rtems.c` | 1 file, 11 insertions, 0 deletions (comment :104-110 + declarations :111-113) |
| `docs/status/handover.md` row 3.67 (2026-11-10) | Pre-fix cross-build warning recorded: `pal_rtems.c:156: implicit declaration of function 'ff_pal_wd_hw_feed'` (prototype only in pal_watchdog.c) — the exact NC-EXT-239 diagnostic |
| `docs/status/status.json`, `docs/status/escalations.md` ESC-3 | NC-PAL-01 OPEN; unwired; owner + avionics authority; compatibility decision open |
| `docs/vv/test-spec/ts-5-core-services.md` | N22-TS5-82 HIL watchdog case = gap; hardware reaction gap due to NC-PAL-01 |
| `docs/architecture/mpfs-mss-watchdog-integration-spec.md` | NC-PAL-01 closure conditions HL-1/HL-2 (target/HIL); emulated binding is test-only, not flight |

Trace counts (unchanged by this non-functional change): 120/76/30/14/68. No new Lean theorems, no generated code, no new algorithm.

---

## 3. Verification of review claims

### Claim 1 — Forward declarations correctly placed before first use of each hook: VERIFIED

- Declarations: `pal_rtems.c:111` (`ff_pal_wd_hw_arm`), `:112` (`ff_pal_wd_hw_feed`), `:113` (`ff_pal_wd_hw_reset`).
- First uses / definitions in the same translation unit:
  - `ff_pal_wd_hw_arm` defined `:147` (after the declaration at :111) ✓
  - `ff_pal_wd_hw_feed()` first called `:167` inside `ff_pal_wd_hw_arm` (`return ff_pal_wd_hw_feed();`); defined `:228` — declaration at :112 precedes both ✓
  - `ff_pal_wd_hw_reset` defined `:253` — declaration at :113 precedes it ✓

All three declarations precede every first use and every definition within the TU. The C implicit-declaration diagnostic cannot fire for these hooks once a visible prototype precedes the call.

**Cross-TU interface check (no conflicting-prototype / ODR risk):**
- `pal_watchdog.c:29-31` declares the same three prototypes in a separate translation unit — identical prototypes across TUs are permitted; no conflict.
- `pal.h` does not declare the internal hooks (public `FF_PalWatchdog*` API only) — no redeclaration.
- `pal_posix.c:673-703` provides the simulated-backend definitions guarded by `#if !defined(FRICK_PLATFORM_MPFS) && !defined(FRICK_PLATFORM_QEMU)` (`:658`); `pal_rtems.c` is compiled only under `FRICK_PLATFORM_MPFS || FRICK_PLATFORM_QEMU` (`:51`). The guards are complementary, so the hooks are defined in exactly one platform-appropriate TU per build — no duplicate-definition/ODR risk on host, RTEMS MPFS, or QEMU.

### Claim 2 — Change is bounded (declarations only, no logic, no hardware wired): VERIFIED

- `git diff`: 11 insertions, 0 deletions. Added content = comment block `:104-110` + 3 declarations `:111-113`.
- No control-flow, data, register, clock, timeout, or mission-parameter change.
- Flight-target hooks (`:286`, `:294`, `:299`) remain `FF_NOT_IMPLEMENTED` (NC-PAL-01). Emulated binding remains guarded by `:93` (`FRICK_PLATFORM_MPFS && FF_ENABLE_RENODE_TEST == 1`).
- No new Lean theorems, no generated code; trace gate unchanged (120/76/30/14/68).

### Claim 3 — NC-PAL-01 is NOT closed by this change: VERIFIED

- Flight build `#else` branch (`:278-306`) returns `FF_NOT_IMPLEMENTED` for all three hooks and logs that the MPFS binding requires owner authorisation.
- Emulated binding is explicitly `FF_ENABLE_RENODE_TEST`-gated (`:93`) and the file header (`:31-40`) states it "establishes no target behaviour."
- `docs/status/handover.md`, `status.json`, `escalations.md` ESC-3, and `mpfs-mss-watchdog-integration-spec.md` all record NC-PAL-01 as OPEN, authority-owned, requiring target/HIL evidence and authorised human sign-off (closure conditions HL-1/HL-2).

**This PA review does not close NC-PAL-01.** NC-PAL-01 remains open (target/HIL; authority-owned).

### Claim 4 — RTEMS cross-build no longer emits the implicit-declaration warning for pal_rtems.c / ff_pal_wd_hw_*: VERIFIED at source level; gate evidence per provided log

- **Before the fix:** `docs/status/handover.md` 3.67 records the cross-build warning `pal_rtems.c:156: implicit declaration of function 'ff_pal_wd_hw_feed'` — the diagnostic is real and located exactly where the comment says (the call site pre-fix).
- **After the fix:** a visible prototype for `ff_pal_wd_hw_feed` exists at `:112`, before the call at `:167`; likewise `ff_pal_wd_hw_arm` (`:111`) and `ff_pal_wd_hw_reset` (`:113`) are declared before use. By construction (C requires a declaration before a call), the implicit-declaration diagnostic cannot fire for these hooks.
- **Local cross-build re-run:** NOT executed by PA. The `frick-rtems-toolchain:local` image is not present in this environment; building it (`make rtems-toolchain`) is a mutable, non-reproducible apt/RSB toolchain activity explicitly flagged as NC-EXT-40 and is out of scope for this PA review. Cross-build re-run evidence therefore rests on the provided gate log (Docker RTEMS 6.2 cross-build exit 0, no implicit-declaration warnings for `pal_rtems.c`/`ff_pal_wd_hw_*`). Residual uncertainty is limited to build-environment/toolchain execution, not to the source change.

---

## 4. Findings

| ID | Severity | File:line | Finding | Recommended disposition | Blocks commit/push? |
|---|---|---|---|---|---|
| **PA-001** | Minor | `src/core/pal/pal_rtems.c:104-110` | The new forward-declaration comment cites stale line numbers: "`ff_pal_wd_hw_feed()` is called from `ff_pal_wd_hw_arm()` (line 156) before its definition (line 217)." After the 11-line insertion, `ff_pal_wd_hw_arm` is defined at :147, `ff_pal_wd_hw_feed()` is called at :167, `ff_pal_wd_hw_feed` is defined at :228, and `ff_pal_wd_hw_reset` at :253. No compile/safety effect; affects audit traceability (norms N4/N8) only. | Correct the comment line references to :147 (arm definition), :167 (feed call site), :228 (feed definition), :253 (reset definition) as part of this change. Trivially correctable; log as NC for disposition if not folded in now (norm N21: non-functional findings tracked, do not stop unrelated development). | **No** |
| **PA-002** | Minor | Evidence boundary (Claim 4) | PA did not re-execute the RTEMS 6.2 cross-build locally (`frick-rtems-toolchain:local` image absent; building it is the mutable/non-reproducible NC-EXT-40 activity out of scope). Claim 4 rests on source-level resolution by construction plus the provided gate log. | Accept provided gate evidence; record the evidence boundary in the assurance ledger. Optionally re-run `make rtems-docker` when the toolchain image is available to close the execution evidence gap. | **No** |

**No Major findings.** No finding questions the correctness or boundedness of the fix.

---

## 5. NC resolution assessment (PA perspective)

| NC | Status after this change | PA basis |
|---|---|---|
| **NC-EXT-239** (compile diagnostic, emulated N18 profile) | **Resolved from a PA perspective on the host/SIL (emulated N18) evidence base.** | Bounded declarations-only fix; declarations correctly placed before first use (Claim 1); no logic/hardware change (Claim 2); implicit-declaration diagnostic removed by construction (Claim 4); gates green per provided log. Resolution is limited to the compile diagnostic on the emulated N18 profile — it does **not** establish flight watchdog behaviour. |
| **NC-PAL-01** (flight MPFS MSS watchdog binding) | **REMAINS OPEN.** Not closed. | Flight hooks return `FF_NOT_IMPLEMENTED` (:286-304); binding unwired; target/HIL evidence and authorised human sign-off required (closure conditions HL-1/HL-2). Authority-owned. **This review must not close NC-PAL-01.** |

---

## 6. PA concerns with committing / conditions for commit

From a PA perspective, **there are no findings that block commit/push** of this change. The fix is small, bounded, correct at source level, and does not alter any flight behaviour or close any safety-relevant NC.

**Conditions / recommendations before commit/push:**

1. **Do not close NC-PAL-01.** The commit message, release notes, and any gate record must describe NC-EXT-239 as resolved only for the emulated N18 compile diagnostic, and must record NC-PAL-01 as still open (target/HIL; authority-owned). Do not imply flight watchdog readiness.
2. **Fold PA-001 into this change** (recommended): correct the stale comment line references (`:156`→`:167` call site / `:147` arm def; `:217`→`:228` feed def; add `:253` reset def) so the committed comment is auditable (norms N4/N8). If not corrected now, log PA-001 as a Minor NC to be dispositioned at the next touch of `pal_rtems.c`.
3. **Record PA-002** in the evidence ledger (cross-build re-run not executed by PA; relies on provided gate log).
4. **No NC is closed by this increment** (consistent with `docs/status/handover.md` row 3.67 precedent: "No NC closed, no commit/push" until gates and reviews complete).

**Commit authority:** Per repository norm N7, only the `aeronautics-space-fsw-engineer` agent may commit/push (or with explicit user delegation). This PA review is advisory; the FSW engineer owns the commit decision and all applicable gates (build, test, Lean, trace, format, license) must pass before push.

---

## 7. Uncertainty and confidence report

| Field | Content |
|---|---|
| Decision/product and configuration | NC-EXT-239 fix; `src/core/pal/pal_rtems.c` working tree, RTEMS 6.2 `riscv/mpfs64imafdc` / emulated N18 profile (`FF_ENABLE_RENODE_TEST=1`) |
| Claim type | Facts (source inspection, git diff, docs) + Inference (diagnostic resolved by construction) |
| Applicable baseline and approved tailoring | ECSS-Q-ST-80C Rev.2 / ECSS-E-ST-40C Rev.1 as invoked by project norms (N1-N22); N20 independent PA review |
| Evidence consulted | `pal_rtems.c` (full), `pal_watchdog.c:29-31`, `pal_posix.c:658-704`, `pal.h`, `git diff`, `handover.md` 3.67, `status.json`, `escalations.md` ESC-3, `ts-5-core-services.md`, `mpfs-mss-watchdog-integration-spec.md`; provided gate log for cross-build |
| Cross-check and reconciliation | Complementary platform guards confirmed (no duplicate definition); stale comment line references identified (PA-001); NC-PAL-01 confirmed open across multiple docs |
| Confidence | **High** for Claims 1–3 (direct source inspection, no conflict). **High-with-bounded-evidence-note** for Claim 4 (source-level resolution by construction; cross-build re-run evidence rests on provided gate log, PA-002). |
| Missing/inaccessible evidence | `frick-rtems-toolchain:local` image not present locally; post-fix cross-build output not independently re-run by PA (see PA-002). |
| Assumptions and expiry | Assumes the provided gate log accurately reflects the current working tree; re-check cross-build if toolchain/image changes. |
| Consequences if wrong | If the declaration were misplaced, the RTEMS cross-build would still warn (caught at build gate); no flight-safety impact because flight hooks remain `FF_NOT_IMPLEMENTED` (NC-PAL-01). |
| Required independent human reviewer/authority | FSW engineer (commit); authorised human authority for any NC-PAL-01 disposition / flight watchdog binding (target/HIL sign-off). |

---

## Authority statement

This report is an independent product assurance assessment, not a certification, approval, qualification, release authorization, or flightworthiness declaration. It does not close NC-PAL-01, accept residual risk, or authorize release. Independent qualified humans retain those decisions.

**PA review outcome:** NC-EXT-239 resolved on the host/SIL (emulated N18) evidence base; NC-PAL-01 remains OPEN. No PA finding blocks commit/push, subject to conditions in Section 6.
