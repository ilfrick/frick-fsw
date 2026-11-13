# Independent SW V&V Review — NC-EXT-239 fix (`pal_rtems.c` forward declarations)

|                                   |                                        |
| ---------------------------------:| :------------------------------------- |
| Document id                       | FRICK-REVIEW-VV-R2-NC-EXT-239-20261111 |
| Reviewer profile                  | R2 — Senior SW V&V Engineer (norm N9)  |
| Repository / branch / HEAD        | frick-fsw / main / ea0ac69             |
| Change under review               | Working-tree (uncommitted) fix to `src/core/pal/pal_rtems.c`: forward declarations of `ff_pal_wd_hw_arm`, `ff_pal_wd_hw_feed`, `ff_pal_wd_hw_reset` added at lines 111-113 (comment block lines 104-110) |
| Registered NC                     | NC-EXT-239 — Minor (build hygiene / declaration) — OPEN, owner FSW engineer (PAL/NC-PAL-01 workstream); registered at `docs/planning/sprint-plan.md:326` |
| Date                              | 2026-11-11                             |
| Independence declaration          | Reviewer had no part in authoring or executing the change; fresh context; no inheritance of author context (norm N9) |
| Norms applied                     | N1-N22 (see `docs/rules.md`); ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2 as invoked by project norms |
| Authority statement               | Evidence and recommendation only. No certification, approval, qualification, acceptance, anomaly closure, or release authority. Commit/push rests with `aeronautics-space-fsw-engineer` after all gates/norms pass (norm N7); PA (N20) and R4 (N17) reviews are separate requirements |

---

## 1. Scope

Reviewed: the NC-EXT-239 fix and its verification evidence against the five review questions:

1. Is the RTEMS cross-build (exit 0, no implicit-declaration warning) sufficient verification evidence for NC-EXT-239?
2. Are additional host tests, target tests, or gate evidence needed?
3. Does the change affect any existing test behavior?
4. Tool qualification considerations (no new test scripts added; no T3 impact).
5. Which V&V findings block commit/push?

In scope: `src/core/pal/pal_rtems.c` working-tree diff (1 file, 11 insertions, 0 deletions), the NC-EXT-239 registration (`sprint-plan.md:326`), the pre-existing diagnostic evidence (`handover.md:166-168`, `handover.md:1811`), traceability, the existing watchdog test evidence, and the independent PA and R4 reviews retained in the working tree (`PA-REVIEW-NC-EXT-239-pal_rtems-20261111.md` findings PA-001/PA-002; `FRICK-REVIEW-0479-R4-DOC-NC-EXT-239.md` finding DOC-R4-001).

Out of scope: NC-PAL-01 (flight MPFS MSS watchdog binding — authority-owned, target/HIL; not closed by this change), target/HIL timing evidence (GAP-A5), mission parameter confirmation (NC-EXT-238), Lean theorem content (no Lean sources touched), any documentation outside the changed file.

---

## 2. Change under review (verified in the working tree)

`git diff HEAD -- src/core/pal/pal_rtems.c`: 1 file, 11 insertions, 0 deletions.

Added content, `src/core/pal/pal_rtems.c`:

- `:104-110` — rationale comment referencing NC-EXT-239 and the call-before-definition ordering (line references in this comment are stale: see finding VV-R2-002).
- `:111-113` — forward declarations:

```c
FF_Status_t ff_pal_wd_hw_arm(uint32_t timeout_ms);
FF_Status_t ff_pal_wd_hw_feed(void);
FF_Status_t ff_pal_wd_hw_reset(void);
```

Defect site (pre-fix, recorded at `handover.md:166-168`): `ff_pal_wd_hw_feed()` called at `pal_rtems.c:156` inside `ff_pal_wd_hw_arm()` defined at `:147` (working tree), while `ff_pal_wd_hw_feed` was defined at `:228` with no visible prototype in the TU. The only prototype lived in `src/core/pal/pal_watchdog.c:29-31` (separate translation unit). Implicit function declaration is ill-formed in C99 and fails under `-Werror`.

Context guards:
- `pal_rtems.c:51` — TU compiled only for `FRICK_PLATFORM_MPFS || FRICK_PLATFORM_QEMU`.
- `pal_rtems.c:93` — emulated binding active only for `FRICK_PLATFORM_MPFS && FF_ENABLE_RENODE_TEST == 1` (N18 test profile).
- `pal_rtems.c:278-306` — flight `#else` branch: all three hooks return `FF_NOT_IMPLEMENTED` with an NC-PAL-01 log message.
- `pal_posix.c:658,673-703` — simulated backend definitions guarded by `!MPFS && !QEMU` (host simulator only; never co-compiled with `pal_rtems.c`).

No control-flow, data, register, clock, timeout, or mission-parameter change; no new Lean theorems; no generated code.

---

## 3. References

- Project norms: `docs/rules.md` (N1-N22), in particular N4 (traceability), N6/N10 (V&V tools), N11 (evidence retention), N15 (full cycle on every change), N16 (revision history), N17 (documentation review), N18 (Renode functional validation), N20 (PA review), N21 (sequencing), N22 (nominal/out-of-bound oracles)
- Traceability manifest: `docs/requirements/traceability.yaml` (pal_rtems.c traced via SRS-REQ-015/016/037/073 etc.; no requirement text changed by this fix)
- NC register: `docs/planning/sprint-plan.md:326` (NC-EXT-239), `:240` (NC-PAL-01)
- Standards matrix: ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025), ECSS-E-ST-10-02C Rev.1 — project baseline per norms
- Corroborating independent reviews in the working tree: PA review (PA-001 Minor comment lines; PA-002 evidence boundary), R4 review (DOC-R4-001 Minor stale comment line references); both conclude **no finding blocks commit/push**

---

## 4. Evidence executed in this review

| Gate | Command / method | Result | Evidence |
|------|------------------|--------|----------|
| Host build | `make BUILD_DIR=build/host-review build` (fresh dir; `build/native` is root-owned container cache) | exit 0; **0 project-source warnings** | `/tmp/opencode/build-review.log`; 13 "warning" hits are all third-party (ASN.1 `.acn` diagnostics, n7s-obcp `warning.c` filename) |
| CTest unit tests | `ctest --test-dir build/host-review` | **56/56 PASS**, 0 failed | ctest output, 2026-11-11 |
| Traceability (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** (120 requirements, 76 SRS, 30 SSS, 14 gaps, 68 Lean theorems) | `/tmp/opencode/trace-review.log` |
| RTEMS 6.2 cross-build | `make rtems` / `frick-rtems-toolchain:local` | **NOT EXECUTABLE** — image `frick-rtems-toolchain:local` not present in this environment; building it is the mutable NC-EXT-40 activity | Required gate evidence for NC-EXT-239 (see VV-R2-001) |
| Format-check | pinned clang-format 18.1.3 (`frick-vv:local`) | **NOT EXECUTABLE** — pinned container not present locally; local clang-format 23.1.3 is rejected by the Makefile pin | Required gate evidence (low risk: 11 declaration lines matching existing style) |
| Lean (N1) | `lake build` in `verification/lean` | **NOT EXECUTABLE** here — pinned toolchain leanprover/lean4:v4.10.0 vs local elan 4.35.0; **no Lean sources or theorems touched** by the change, so prior Lean PASS is unaffected (N1 interpretation note, `docs/rules.md:475-483`) | Not a change regression |
| License (N13) | `license_check.py` / `make gsw-selfcheck` | Not re-run; change is a comment inside an existing MPL-2.0 file — no license-relevant material touched | Prior cycle OK stands |

Corroborating PA/R4 reviews (in tree, 2026-11-11): PA verified declaration placement (:111 arm def :147, :112 precedes feed call :167, :113 precedes reset def :253), complementary platform guards (no duplicate-definition/ODR risk), flight hooks unchanged (`FF_NOT_IMPLEMENTED`), NC-PAL-01 not closed; R4 confirmed English (N2), no API/behavior/interface/traceability change (N8), only the stale comment line numbers (DOC-R4-001).

---

## 5. Answers to the review questions

**Q1 — Is the RTEMS cross-build (exit 0, no implicit-declaration warning) sufficient verification evidence for NC-EXT-239?**

It is **sufficient for the registered defect** and **necessary as disposition evidence**, but not sufficient standing alone for the change:

- The registered defect is a compile diagnostic (`pal_rtems.c:156` implicit declaration of `ff_pal_wd_hw_feed`, ill-formed in C99, `-Werror` failure mode). The fix's entire observable effect is the removal of that diagnostic, so a cross-build of the exact TU, configuration (`FRICK_PLATFORM_MPFS && FF_ENABLE_RENODE_TEST`) and toolchain that produced the warning, exiting 0 with zero implicit-declaration warnings for `pal_rtems.c` / `ff_pal_wd_hw_*`, directly evidences defect resolution. By construction (visible prototype before first use at :167), the diagnostic cannot fire for these hooks once the build passes clean.
- Per norm N15 (full cycle on every change) and N6/N10, the cross-build is one gate of the applicable cycle, not the whole of it: host build (0 warnings), CTest (56/56), traceability, Lean, format-check, license, and the cross-build must all pass. Host-executable portions are green in this review (see section 4).

**Q2 — Are additional host tests, target tests, or gate evidence needed?**

- **No new host tests:** the change is declarations only with identical signatures to the existing definitions/declarations (`pal_watchdog.c:29-31`, `pal_rtems.c:147-304`). On the host `pal_rtems.c` compiles to an empty translation unit (guards at :51), so host tests cannot observe the change and none can add discriminating evidence. Existing watchdog evidence remains valid: `test_pal_watchdog.c` (320 checks), reset-path cases in `test_es_hs_lifecycle.c`, N18 emulated-binding PASS history, and XW-1/XW-2 contract tests, which substitute the three hooks at link time via the validated double (`tests/stubs/ff_test_extwd_{model,pal}.c`) and do not link `pal_rtems.c`.
- **No target tests:** the flight hooks remain `FF_NOT_IMPLEMENTED` (NC-PAL-01); the generic QEMU image (M10/T20) has no MPFS peripherals and does not exercise the MPFS binding; no target/HIL watchdog evidence exists and this change adds none. A target test would establish nothing beyond what is already scoped (emulated N18 integration evidence only).
- **Gate evidence still needed:** post-fix RTEMS cross-build showing exit 0 with **zero** implicit-declaration warnings for `pal_rtems.c`/`ff_pal_wd_hw_*` (VV-R2-001); format-check and license re-check once the pinned containers are available; Lean per the pinned toolchain where runnable (unaffected by the change).

**Q3 — Does the change affect any existing test behavior?**

**No.** Verified:
- Zero deletions; 11 insertions are a comment and three forward declarations.
- Signatures identical to `pal_watchdog.c:29-31` and the `pal_rtems.c` definitions at :147/:228/:253; duplicate compatible declarations across translation units are standard C and create no conflict (corroborated by PA cross-TU check).
- CTest **56/56 PASS** with the fix in the working tree (host simulator; simulated backend via `pal_posix.c`).
- Trace counts unchanged (120/76/30/14/68); no Lean theorems affected; `verification/lean/Frick/Watchdog.lean` models `pal_watchdog.c`/`pal_posix.c`, not `pal_rtems.c` MMIO (target hardware itself excluded per `Watchdog.lean:16-18`, NC-PAL-01).
- XW-2/XW-1 middleware-contract evidence (`docs/vv/verification-results.md`, XW-2 section) is unaffected: the hooks are substituted by the validated double.

**Q4 — Tool qualification considerations?**

- No new test scripts, harnesses, or V&V tools added → **no T3 impact** and no N10 re-qualification trigger (norm N10 invalidates qualification on tool/script change; there is none).
- Pre-existing qualification status is unchanged by this change and is **not** a finding introduced here: T3 (unit tests) is NOT FULLY QUALIFIED for the current candidate (`tool-qualification.md` rev 1.7 row T3); T18 (open RTEMS 6.2 toolchain image) is NOT FULLY REQUALIFIED (NC-EXT-40). The cross-build evidence remains usable as build/gate evidence, but the tool qualification register records these as pre-existing conditions owned elsewhere.
- The RTEMS cross-build itself is executed with T10/T18 toolchain material; no new tool is introduced by NC-EXT-239.

**Q5 — V&V findings and whether each blocks commit/push**

| ID | Severity | File:line | Finding | Blocks commit/push? |
|----|----------|-----------|---------|---------------------|
| VV-R2-001 | **Minor** | Evidence gate; `docs/planning/sprint-plan.md:326` | NC-EXT-239 disposition evidence not yet produced post-fix: RTEMS 6.2 cross-build (exit 0, zero implicit-declaration warnings for `pal_rtems.c` / `ff_pal_wd_hw_*`) has not been re-executed in this review environment (image absent). Until green, NC-EXT-239 is not evidenced-closed and the N7/N15 gate set is incomplete. | **YES — until the clean cross-build evidence is produced and recorded** |
| VV-R2-002 | Minor | `src/core/pal/pal_rtems.c:105-107` | The new forward-declaration comment cites pre-insertion line numbers: call at "line 156", definition at "line 217". Working tree: `ff_pal_wd_hw_arm` defined :147, `ff_pal_wd_hw_feed()` called :167, `ff_pal_wd_hw_feed` defined :228, `ff_pal_wd_hw_reset` defined :253. Corroborated by PA-001 and R4 DOC-R4-001. No compile/safety effect; N4/N8 audit-traceability only. Recommended: correct the comment before commit (or log as a Minor NC for later disposition under N21). | No |
| VV-R2-003 | Comment | `docs/planning/sprint-plan.md:326`; `src/core/pal/pal_rtems.c:111-113` | Registered corrective action prescribes declarations "in one PAL-watchdog header included by `pal_rtems.c`"; the fix uses local TU forward declarations instead. Equivalent verification objective (visible prototype before first use; identical signatures to `pal_watchdog.c:29-31`; no cross-TU conflict in C). Record the disposition deviation rationale; no header churn, minimal delta. | No |
| VV-R2-004 | Comment | Section 4; `docs/rules.md` N15 | Scope note: cross-build evidence is sufficient for the registered compile-diagnostic defect, not standing alone for the change. Remaining cycle evidence per section 4 (format-check, license, Lean-unaffected) must complete before push. | No (prerequisite framing; VV-R2-001 carries the blocking evidence gap) |
| VV-R2-005 | Comment | `src/core/pal/pal_rtems.c:51,93,278-306`; `pal_posix.c:658` | No additional host/target tests needed or useful (declarations only; host TU empty; flight hooks `FF_NOT_IMPLEMENTED`; QEMU generic has no MPFS peripherals). Existing test evidence unchanged and valid. | No |
| VV-R2-006 | Comment | `docs/vv/tool-qualification.md` rows T3/T18 | No new scripts/tools → no T3 impact, no N10 re-qualification trigger. Pre-existing T3/T18 qualification conditions unchanged; not introduced by NC-EXT-239. | No |
| VV-R2-007 | Comment | `docs/planning/sprint-plan.md:240` (NC-PAL-01); `:326` (NC-EXT-239) | Pre-existing open NCs not caused by and not closed by this change: NC-PAL-01 (Major, flight MPFS watchdog binding unwired; target/HIL; authority-owned), NC-EXT-186/188, qualification conditions, N18 target-fidelity limits. Must not be represented as closed by NC-EXT-239 evidence. N21: open non-functional NCs do not stop unrelated development; separate safety/release gates remain. | No |
| VV-R2-008 | Comment | Working tree | Untracked items present (`tests/unit/test_sched_events.c.bak`, NC-EXT-239 PA/R4 review artifacts, `.opencode/plugins/`, `opencode.json`); out of scope for NC-EXT-239 — exclude from the commit unless intentionally included. | No |

---

## 6. NC resolution assessment (V&V perspective)

| NC | Status after this change | V&V basis |
|----|--------------------------|-----------|
| **NC-EXT-239** (compile diagnostic, emulated N18 profile) | **Resolvable on the host/SIL emulated-N18 evidence base** | Bounded declarations-only fix; declarations correctly placed before first use (:111-113 before :147/:167/:228/:253); identical signatures to `pal_watchdog.c:29-31`; host gates green (build exit 0, 0 warnings; CTest 56/56; trace PASS). Conditional on VV-R2-001: post-fix RTEMS cross-build exit 0 with zero implicit-declaration warnings, recorded as evidence. Resolution covers the compile diagnostic on the emulated N18 profile only — it does **not** establish flight watchdog behaviour. |
| **NC-PAL-01** (flight MPFS MSS watchdog binding) | **REMAINS OPEN** | Flight `#else` branch (:286-304) returns `FF_NOT_IMPLEMENTED`; binding unwired; target/HIL and authorised human sign-off required (closure conditions HL-1/HL-2 in `docs/architecture/mpfs-mss-watchdog-integration-spec.md`). Not closed by this review or this change. |

---

## 7. Conclusion

**V&V review outcome: APPROVED WITH COMMENTS.** The change is small, bounded, and verification-ready: declarations only, consistent interfaces, no behavior change, host gates green (build exit 0 with zero project-source warnings; CTest 56/56; traceability PASS 120/76/30/14/68), corroborated by independent PA and R4 reviews (no Major findings; neither blocks commit/push).

No Major findings. Findings VV-R2-002 through VV-R2-008 are Minor/Comment and do not technically disqualify the item; VV-R2-002 (stale comment line numbers) should be folded into the change to close the N17/N8 documentation loop cleanly.

**Commit/push blocking condition:** VV-R2-001 — the post-fix RTEMS 6.2 cross-build evidence (exit 0, no implicit-declaration warnings for `pal_rtems.c` / `ff_pal_wd_hw_*`) must be produced and recorded before NC-EXT-239 can be treated as evidenced-closed and before the N7/N15 gate set is complete. Until then, **commit/push is not supported by complete gate evidence**.

**Remaining pre-commit items (normative, not V&V defects):**
1. Produce and record the clean RTEMS cross-build log (VV-R2-001); format-check and license re-check with the pinned containers when available.
2. Correct the stale comment line references at `src/core/pal/pal_rtems.c:105-107` (VV-R2-002 / PA-001 / DOC-R4-001) before commit, or disposition as a tracked Minor NC under N21.
3. Record the NC-EXT-239 disposition deviation rationale (VV-R2-003) and update the NC register/status if NC-EXT-239 is marked closed (FSW/PA authority action; not a documentation change inherent to the fix per R4).
4. Formal commit/push authority: `aeronautics-space-fsw-engineer` only, after all gates/norms and full sprint scope pass (norm N7); N20 PA and N17 R4 loops must close.

NC-PAL-01, NC-EXT-186/188, and the T3/T18 qualification conditions remain open and unaffected by this change; none is closed by this evidence.

---

## 8. Follow-up records (norms N11/N12)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
|----|----------|--------------------|---------|-------------|--------------------------|-------|--------|-----------------|
| VV-R2-001 | Minor | NC-EXT-239 disposition evidence | Post-fix RTEMS cross-build not yet re-executed (image absent in this environment) | Produce clean cross-build log: exit 0, zero implicit-declaration warnings for `pal_rtems.c` / `ff_pal_wd_hw_*` | `make rtems` / `frick-rtems-toolchain:local` build log retained in `docs/vv/evidence/` | FSW engineer | Open (blocks commit/push evidence) | R2 delta review of the gate log |
| VV-R2-002 | Minor | `src/core/pal/pal_rtems.c:105-107` | Stale line references in forward-declaration comment | Correct comment to :147/:167/:228/:253 or log as tracked Minor NC | Updated `git diff` + format-check | FSW engineer | Open (recommended before commit) | R4 N17 loop |
| VV-R2-003 | Comment | NC-EXT-239 registration (`sprint-plan.md:326`) | Local TU declarations vs registered "one PAL-watchdog header" corrective action | Record deviation rationale (equivalent objective) | This report §5/VV-R2-003 | FSW engineer | Closed by record | n/a |

*Report retained in the working tree at `docs/reviews/2026-11-11-vv-r2-nc-ext-239-independent-vv-review.md` per norm N11; to be committed and pushed with the reviewed change only after all applicable gates pass (norms N7/N11/N12/N15).*
