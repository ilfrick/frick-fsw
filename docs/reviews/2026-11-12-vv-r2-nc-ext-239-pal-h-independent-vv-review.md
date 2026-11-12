# Independent SW V&V Review — NC-EXT-239 disposition (`pal.h` shared watchdog-hook declarations)

|                                   |                                        |
| ---------------------------------:| :------------------------------------- |
| Document id                       | FRICK-REVIEW-VV-R2-NC-EXT-239-PALH-20261112 |
| Reviewer profile                  | R2 — Senior SW V&V Engineer (norm N9/R2 persona) — independent subagent review |
| Repository / branch / HEAD        | frick-fsw / main / 1c4b323 |
| Change under review               | Working-tree (uncommitted) fix to `src/core/pal/pal.h` only: forward declarations of `ff_pal_wd_hw_arm(uint32_t)`, `ff_pal_wd_hw_feed(void)`, `ff_pal_wd_hw_reset(void)` added at lines 147-149 (hardware-watchdog section, before the pre-reset notifier at :151). `pal_rtems.c` is UNMODIFIED by this change. |
| Registered NC                     | NC-EXT-239 — Minor (build hygiene / declaration) — OPEN, owner FSW engineer (PAL / NC-PAL-01 workstream); registered at `docs/planning/sprint-plan.md:326` |
| Date                              | 2026-11-12 |
| Independence declaration          | Reviewer had no part in authoring or executing the change; fresh context; no inheritance of author context (norm N9). This review does not inherit or adopt prior review verdicts; it re-assesses the evidence independently. |
| Norms applied                     | N1-N22 (see `docs/rules.md`); ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2 as invoked by project norms |
| Authority statement               | Evidence and recommendation only. No certification, approval, qualification, acceptance, anomaly closure, or release authority. Commit/push rests with `aeronautics-space-fsw-engineer` after all gates/norms pass (norm N7); PA (N20) and R4 (N17) reviews are separate requirements. Passing tests show observed outcomes matched oracles under recorded conditions; they do not prove defect absence. |

---

## 1. Scope

In scope:
- The NC-EXT-239 disposition and its verification evidence against the five review questions:
  1. Is the RTEMS cross-build (exit 0, no implicit-declaration warning) sufficient verification evidence for NC-EXT-239?
  2. Are additional tests/gates needed (the fix is forward declarations; the RTEMS cross-build itself is the verification)?
  3. Does the change affect any existing test behavior?
  4. Tool qualification considerations (no new test scripts added; no T3 impact; no N10 re-qualification trigger).
  5. Which V&V findings block commit/push?
- Files: `src/core/pal/pal.h` (changed), `src/core/pal/pal_rtems.c` (unchanged, reference), `src/core/pal/pal_watchdog.c` (unchanged, reference), `docs/planning/sprint-plan.md:326` (NC register), retained build evidence.

Out of scope: NC-PAL-01 (flight MPFS MSS watchdog binding — authority-owned, target/HIL; not closed by this change), target/HIL timing evidence (GAP-A5), mission parameter confirmation (NC-EXT-238), Lean theorem content (no Lean sources touched), documentation outside the changed file, any fix to `pal_rtems.c` (out of scope; that file is unmodified here).

---

## 2. Change under review (verified in the working tree)

`git diff HEAD -- src/core/pal/pal.h`: 1 file, 12 insertions, 0 deletions.

Added content, `src/core/pal/pal.h`:

```c
/* ---- Platform watchdog hardware hooks --------------------------------
 * Platform-specific hardware binding hooks for the watchdog. Declared here
 * (a single header) so that every platform translation unit that implements
 * or calls them (pal_posix.c, pal_rtems.c) sees a prototype before first use,
 * avoiding implicit-declaration diagnostics (NC-EXT-239). Implemented by
 * exactly one platform translation unit; pal_watchdog.c declares the same
 * prototypes for cross-TU consistency.
 */
FF_Status_t ff_pal_wd_hw_arm(uint32_t timeout_ms);
FF_Status_t ff_pal_wd_hw_feed(void);
FF_Status_t ff_pal_wd_hw_reset(void);
```

- `pal_rtems.c:49` includes `pal.h`, so the TU now sees these prototypes via the shared header in addition to its own local forward declarations at `:111-113`.
- `pal_watchdog.c:24` includes `pal.h` and already declares the same prototypes locally at `:29-31`.
- `pal_posix.c` defines the three hooks (definitions at `:673/:685/:694`); `pal.h` now also declares them (forward declarations before definitions — compatible).

Defect site (historical, recorded in retained evidence): `ff_pal_wd_hw_feed()` was called inside `ff_pal_wd_hw_arm()` with no visible prototype in the TU at the time NC-EXT-239 was registered (2026-11-10). The retained `docs/vv/evidence/2026-11-10-n22-ts5-66/rtems-n18-build.log:7-11` records:

```
/workspace/src/core/pal/pal_rtems.c: In function 'ff_pal_wd_hw_arm':
/workspace/src/core/pal/pal_rtems.c:156:16: warning: implicit declaration of function 'ff_pal_wd_hw_feed' ... [-Wimplicit-function-declaration]
  156 |         return ff_pal_wd_hw_feed();
```

Implicit function declaration is ill-formed in C99 and fails under `-Werror`; NC-EXT-239 is registered OPEN pending disposition.

Context guards (`pal_rtems.c`):
- `:51` — TU compiled only for `FRICK_PLATFORM_MPFS || FRICK_PLATFORM_QEMU`.
- Emulated binding active only for `FRICK_PLATFORM_MPFS && FF_ENABLE_RENODE_TEST == 1` (N18 test profile).
- `:278-306` — flight `#else` branch: all three hooks return `FF_NOT_IMPLEMENTED` (NC-PAL-01).

No control-flow, data, register, clock, timeout, or mission-parameter change; no new Lean theorems; no generated code; no new test scripts.

---

## 3. Evidence executed in this review

| Gate | Command / method | Result | Evidence | Independently re-executed? |
|------|------------------|--------|----------|---------------------------|
| Host build (native) | `cmake -S . -B build/host-vv ... ; cmake --build build/host-vv` | **exit 0; 0 warnings/errors**; `pal_rtems.c.o` and `pal_watchdog.c.o` compile clean | this session (`build/host-vv`) | **YES** |
| CTest unit tests | `ctest --test-dir build/host-vv` | **56/56 PASS**, 0 failed | this session | **YES** |
| Traceability (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** (120 requirements, 76 SRS, 30 SSS, 14 gaps, 68 Lean theorems) | this session | **YES** |
| RTEMS 6.2 cross-build | `make rtems-docker` / `frick-rtems-toolchain:local` | Reported **exit 0, zero warnings** (implicit declaration warning eliminated) | User-reported context | **NO** — image `frick-rtems-toolchain:local` absent here; building it is the NC-EXT-40 mutable activity not executable in this session |
| Format-check | pinned clang-format 18.1.3 (`frick-vv:local`) | Reported PASS | User-reported context | **NO** — pinned container absent; local clang-format 23 rejected by Makefile pin; change is 11 declaration lines matching existing style |
| Lean (N1) | `lake build` in `verification/lean` | Reported PASS (unaffected: no Lean sources/theorems touched) | User-reported context | **NO** — pinned toolchain unavailable locally; no Lean sources touched |
| License (N13) | `license_check.py` / `make gsw-selfcheck` | Reported OK; change is a comment inside an existing MPL-2.0 file | User-reported context | Not re-run (no license-relevant material touched) |

Corroborating prior independent reviews retained in the working tree (2026-11-11): PA review and R4 review of the earlier `pal_rtems.c` local-declaration version concluded no Major findings and neither blocked commit/push. Those findings are referenced where still applicable but not re-adopted wholesale for this `pal.h` change.

---

## 4. Answers to the review questions

### Q1 — Is the RTEMS cross-build (exit 0, no implicit-declaration warning) sufficient verification evidence for NC-EXT-239?

It is **sufficient for the registered defect** and **necessary as disposition evidence**, but not sufficient standing alone for the change.

- The registered defect is a compile diagnostic (implicit declaration of `ff_pal_wd_hw_feed`, ill-formed in C99, `-Werror` failure mode). The fix's entire observable effect is the removal of that diagnostic, so a cross-build of the exact TU, configuration (`FRICK_PLATFORM_MPFS && FF_ENABLE_RENODE_TEST`) and toolchain that produced the warning, exiting 0 with zero implicit-declaration warnings for `pal_rtems.c`/`ff_pal_wd_hw_*`, directly evidences defect resolution. By construction (visible prototype before first use), the diagnostic cannot fire for these hooks once the build passes clean.
- Per norm N15 (full cycle on every change) and N6/N10, the cross-build is one gate of the applicable cycle, not the whole of it: host build (0 warnings), CTest (56/56), traceability, Lean, format-check, license, and the cross-build must all pass. Host-executable portions are independently green in this review (section 3).
- Caveat (evidence integrity): the clean post-fix cross-build log has **not** been retained in `docs/vv/evidence/` and could not be re-executed here (image absent). The retained `2026-11-10` log shows the warning present pre-fix. Until a clean post-fix log is produced and recorded, NC-EXT-239 is not evidenced-closed and the N15 gate set is incomplete (see VV-R2-001).

### Q2 — Are additional host tests, target tests, or gate evidence needed?

- **No new host tests:** the change is declarations only with identical signatures to the existing definitions/declarations (`pal_watchdog.c:29-31`, `pal_rtems.c:111-113` and `:147/:228/:253`). On the host `pal_rtems.c` compiles to an empty translation unit (guards at `:51`), so host tests cannot observe the change and none can add discriminating evidence. Existing watchdog evidence remains valid: `test_pal_watchdog.c`, reset-path cases in `test_es_hs_lifecycle.c`, N18 emulated-binding PASS history, and XW-1/XW-2 contract tests (which substitute the three hooks at link time via the validated double and do not link `pal_rtems.c`).
- **No target tests:** the flight hooks remain `FF_NOT_IMPLEMENTED` (NC-PAL-01); the generic QEMU image (M10/T20) has no MPFS peripherals and does not exercise the MPFS binding; no target/HIL watchdog evidence exists and this change adds none. A target test would establish nothing beyond the already-scoped emulated N18 integration evidence.
- **Gate evidence still needed:** post-fix RTEMS cross-build showing exit 0 with **zero** implicit-declaration warnings for `pal_rtems.c`/`ff_pal_wd_hw_*` (VV-R2-001); format-check and license re-check once the pinned containers are available; Lean per the pinned toolchain where runnable (unaffected by the change).

### Q3 — Does the change affect any existing test behavior?

**No.** Independently verified:
- Zero deletions; insertions are a comment and three forward declarations.
- Signatures identical to `pal_watchdog.c:29-31` and the `pal_rtems.c` definitions at `:147/:228/:253`; duplicate compatible declarations across translation units are standard C and create no conflict (confirmed by zero-warning native build of `pal.h`, `pal_rtems.c`, `pal_watchdog.c`, and `pal_posix.c`).
- CTest **56/56 PASS** with the fix in the working tree (host simulator; simulated backend via `pal_posix.c`).
- Trace counts unchanged (120/76/30/14/68); no Lean theorems affected; `verification/lean/Frick/Watchdog.lean` models `pal_watchdog.c`/`pal_posix.c`, not `pal_rtems.c` MMIO (target hardware excluded per `Watchdog.lean:16-18`, NC-PAL-01).
- XW-2/XW-1 middleware-contract evidence (`docs/vv/verification-results.md`, XW-2 section) is unaffected: the hooks are substituted by the validated double.

### Q4 — Tool qualification considerations?

- No new test scripts, harnesses, or V&V tools added → **no T3 impact** and **no N10 re-qualification trigger** (norm N10 invalidates qualification on tool/script change; there is none).
- Pre-existing qualification status is unchanged by this change and is **not** a finding introduced here: T3 (unit tests) is not fully qualified for the current candidate (`docs/vv/tool-qualification.md` rev row T3); T18 (open RTEMS 6.2 toolchain image) is not fully re-qualified (NC-EXT-40). The cross-build evidence remains usable as build/gate evidence, but the tool-qualification register records these as pre-existing conditions owned elsewhere.
- The RTEMS cross-build itself is executed with T10/T18 toolchain material; no new tool is introduced by NC-EXT-239.

### Q5 — V&V findings and whether each blocks commit/push

| ID | Severity | File:line | Finding | Blocks commit/push? |
|----|----------|-----------|---------|---------------------|
| **VV-R2-001** | **Minor** | Evidence gate; `docs/planning/sprint-plan.md:326`; retained `docs/vv/evidence/2026-11-10-n22-ts5-66/rtems-n18-build.log` | NC-EXT-239 disposition evidence not yet produced/verified post-fix: RTEMS 6.2 cross-build (exit 0, zero implicit-declaration warnings for `pal_rtems.c` / `ff_pal_wd_hw_*`) has not been retained as a clean post-fix log in `docs/vv/evidence/` and could not be re-executed in this session (`frick-rtems-toolchain:local` image absent). Until green and recorded, NC-EXT-239 is not evidenced-closed and the N7/N15 gate set is incomplete. | **YES — until the clean post-fix cross-build evidence is produced and recorded** |
| **VV-R2-002** | Minor | `src/core/pal/pal.h:147-149`; `src/core/pal/pal_rtems.c:111-113,167`; retained `.../rtems-n18-build.log:7-11` | Evidence-integrity / redundancy observation: the `pal.h` forward declarations are **redundant for eliminating the `pal_rtems.c` implicit-declaration warning**, because `pal_rtems.c` already contains local forward declarations at `:111-113` (committed before this change), visible before the call at `:167`. The retained 2026-11-10 build log confirms the warning fired at `:156` when `pal_rtems.c` lacked local declarations. NC-EXT-239's premise ("prototype exists only in `pal_watchdog.c:29-31`") was already inaccurate pre-change (`pal_rtems.c:111-113` also had prototypes). The `pal.h` change is not the sole cause of the clean build; it is correct but not the unique corrective cause. Not a defect. | No |
| **VV-R2-003** | Comment | `src/core/pal/pal.h:147-149`; NC-EXT-239 `sprint-plan.md:326` | Corrective-action alignment: the `pal.h` change now correctly implements NC-EXT-239's prescribed corrective action ("declaring `ff_pal_wd_hw_arm/feed/reset` in **one PAL-watchdog header** included by `pal_rtems.c`"), addressing the prior review's deviation observation (local TU declarations instead of a shared header). Duplicate compatible declarations across `pal.h:147-149`, `pal_rtems.c:111-113`, `pal_watchdog.c:29-31` are valid C (zero-warning native build confirms no conflict). | No |
| **VV-R2-004** | Comment | `src/core/pal/pal_rtems.c:51,93,278-306`; `pal_posix.c:658,673-703`; CTest 56/56 | No additional host/target tests needed or useful (declarations only; host TU empty; flight hooks `FF_NOT_IMPLEMENTED`; QEMU generic has no MPFS peripherals). Existing test evidence unchanged and valid. Independently confirmed CTest 56/56 PASS and trace PASS. | No |
| **VV-R2-005** | Comment | `docs/vv/tool-qualification.md` rows T3/T18; norm N10 | No new scripts/tools → no T3 impact, no N10 re-qualification trigger. Pre-existing T3/T18 qualification conditions unchanged; not introduced by NC-EXT-239. | No |
| **VV-R2-006** | Minor | `docs/planning/sprint-plan.md:326`; `docs/vv/verification-results.md:416-420` | Editorial line-reference staleness: NC-EXT-239 and verification-results.md reference `pal_rtems.c:156/217`; current tree has the call at `:167`, definitions at `:147/:228/:253` (shifted by the prior local-declaration insertion). Update NC/evidence references for audit accuracy. | No |
| **VV-R2-007** | Comment | `docs/planning/sprint-plan.md:240` (NC-PAL-01); `:326` (NC-EXT-239); `pal_rtems.c:286-306` | Pre-existing open NCs not caused by and not closed by this change: NC-PAL-01 (Major, flight MPFS watchdog binding unwired; target/HIL; authority-owned) and related open qualification conditions. Flight `#else` branch (`:286-304`) still returns `FF_NOT_IMPLEMENTED`. Must not be represented as closed by NC-EXT-239 evidence. N21: open non-functional NCs do not stop unrelated development; separate safety/release gates remain. | No |
| **VV-R2-008** | Comment | Working tree | Untracked items present (`tests/unit/test_sched_events.c.bak`, NC-EXT-239 review artifacts, `.opencode/plugins/`, `opencode.json`); out of scope for NC-EXT-239 — exclude from the commit unless intentionally included. | No |

---

## 5. NC resolution assessment (V&V perspective)

| NC | Status after this change | V&V basis |
|----|--------------------------|-----------|
| **NC-EXT-239** (compile diagnostic, emulated N18 profile) | **Resolvable; disposition evidence pending retention** | Bounded declarations-only fix; declarations correctly placed before first use (`pal.h:147-149` before any call; `pal_rtems.c:111-113` before `:147/:167/:228/:253`); identical signatures to `pal_watchdog.c:29-31`; host gates green (build exit 0 with zero warnings; CTest 56/56; trace PASS). Conditional on VV-R2-001: post-fix RTEMS cross-build exit 0 with zero implicit-declaration warnings, recorded as evidence. Resolution covers the compile diagnostic on the emulated N18 profile only — it does **not** establish flight watchdog behaviour. |
| **NC-PAL-01** (flight MPFS MSS watchdog binding) | **REMAINS OPEN** | Flight `#else` branch (`pal_rtems.c:286-304`) returns `FF_NOT_IMPLEMENTED`; binding unwired; target/HIL and authorised human sign-off required. Not closed by this review or this change. |

---

## 6. Conclusion

**V&V review outcome: APPROVED WITH COMMENTS.**

The change is small, bounded, and verification-ready: declarations only, consistent interfaces, no behavior change. Host gates independently verified green (build exit 0 with zero project-source warnings; CTest 56/56; traceability PASS 120/76/30/14/68). No new test scripts or tools → no T3 impact, no N10 re-qualification trigger.

No Major findings. Findings VV-R2-002 through VV-R2-008 are Minor/Comment and do not indicate a defect in the change.

**Commit/push blocking condition:** **VV-R2-001** — the post-fix RTEMS 6.2 cross-build evidence (exit 0, no implicit-declaration warnings for `pal_rtems.c` / `ff_pal_wd_hw_*`) must be produced and recorded in `docs/vv/evidence/` before NC-EXT-239 can be treated as evidenced-closed and before the N7/N15 gate set is complete. Until then, **commit/push is not supported by complete gate evidence**. This is an evidence-retention gap, not a defect in the change.

**Remaining pre-commit items (normative, not V&V defects):**
1. Produce and record the clean RTEMS cross-build log (VV-R2-001); re-check format-check and license with the pinned containers when available.
2. Update the stale `pal_rtems.c:156/217` line references in NC-EXT-239 (`sprint-plan.md:326`) and `verification-results.md:416-420` to current `:147/:167/:228/:253` (VV-R2-006) — editorial, before commit or as a tracked Minor NC under N21.
3. Record the NC-EXT-239 disposition (corrective action now implemented via shared `pal.h` header, VV-R2-003) and update the NC register/status if NC-EXT-239 is marked closed — FSW/PA authority action; not a documentation change inherent to the fix.
4. Formal commit/push authority: `aeronautics-space-fsw-engineer` only, after all gates/norms and full sprint scope pass (norm N7); N20 PA and N17 R4 loops must close.

NC-PAL-01, NC-EXT-238, and the T3/T18 qualification conditions remain open and unaffected by this change; none is closed by this evidence.

---

## 7. Follow-up records (norms N11/N12)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
|----|----------|--------------------|---------|-------------|--------------------------|-------|--------|-----------------|
| VV-R2-001 | Minor | NC-EXT-239 disposition evidence | Post-fix RTEMS cross-build not retained / not re-executable here | Produce clean cross-build log: exit 0, zero implicit-declaration warnings for `pal_rtems.c` / `ff_pal_wd_hw_*` | `make rtems` / `frick-rtems-toolchain:local` build log retained in `docs/vv/evidence/` | FSW engineer | Open (blocks commit/push evidence) | R2 delta review of the gate log |
| VV-R2-002 | Minor | Evidence integrity / redundancy | `pal.h` declarations redundant for `pal_rtems.c` warning (local decls at `:111-113` pre-exist) | Record observation; no source change required | This report §4 Q1 / VV-R2-002 | FSW engineer | Closed by record | n/a |
| VV-R2-003 | Comment | Corrective-action alignment | `pal.h` shared header now matches NC prescription | Record rationale (equivalent objective; no conflict) | This report VV-R2-003 | FSW engineer | Closed by record | n/a |
| VV-R2-006 | Minor | Stale line references | NC/`verification-results.md` cite `:156/:217`; current `:147/:167/:228/:253` | Update references before commit or as tracked Minor NC | Updated `git diff` + format-check | FSW engineer | Open (recommended before commit) | R4 N17 loop |

*Report retained in the working tree at `docs/reviews/2026-11-12-vv-r2-nc-ext-239-pal-h-independent-vv-review.md` per norm N11; to be committed and pushed with the reviewed change only after all applicable gates pass (norms N7/N11/N12/N15).*
