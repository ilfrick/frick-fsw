# 2026-11-12-R4-DOC-NC-EXT-239-PAL-H — R4 Software Documentation Review (NC-EXT-239, `pal.h` forward declarations)

| Field | Value |
|---|---|
| Document id | 2026-11-12-R4-DOC-NC-EXT-239-PAL-H |
| Reviewer profile | R4 (Senior SW Documentation Engineer) — independent subagent, fresh context |
| Reference revision | Branch `main`, HEAD `1c4b323`; uncommitted working-tree change to `src/core/pal/pal.h` only |
| Items under review | `src/core/pal/pal.h:139-149` — added "Platform watchdog hardware hooks" comment block (lines 139-146) and forward declarations of `ff_pal_wd_hw_arm(uint32_t)`, `ff_pal_wd_hw_feed(void)`, `ff_pal_wd_hw_reset(void)` (lines 147-149), immediately before the pre-reset notifier section; related NC/status records citing `pal_rtems.c:156` |
| Date | 2026-11-12 |
| Scope | Independent documentation review under norm N17: (1) whether any documentation needs updating for this change (API, behavior, interface, traceability), (2) norm N2 (English) in the added comment, (3) norm N8 (complete/current/deployable documentation) — no doc changes expected, (4) whether the added comment accurately describes the change (NC-EXT-239, forward declarations in `pal.h`, purpose of the declarations), (5) documentation findings that block commit/push. Excluded: technical correctness of the C change; build/CTest/Lean/trace execution and results; NC-EXT-239 disposition authority; commit/push authority (norm N7). |
| Independence declaration | Reviewer had no part in authoring or executing the change; no inheritance of author context (norm N9). |
| Norms applied | N2 (English), N4 (traceability), N8 (complete/current/deployable documentation), N11 (evidence retention), N15 (full cycle on every change), N16 (revision history), N17 (documentation review loop), N21 (bug fix vs NC disposition). ECSS references per project baseline: ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2. |

---

## 1. Change under review

`git diff HEAD -- src/core/pal/pal.h`: 1 file, 11 insertions, 0 deletions.

Added content, `src/core/pal/pal.h`:

```
139: /* ---- Platform watchdog hardware hooks --------------------------------
140:  * Platform-specific hardware binding hooks for the watchdog. Declared here
141:  * (a single header) so that every platform translation unit that implements
142:  * or calls them (pal_posix.c, pal_rtems.c) sees a prototype before first use,
143:  * avoiding implicit-declaration diagnostics (NC-EXT-239). Implemented by
144:  * exactly one platform translation unit; pal_watchdog.c declares the same
145:  * prototypes for cross-TU consistency.
146:  */
147: FF_Status_t ff_pal_wd_hw_arm(uint32_t timeout_ms);
148: FF_Status_t ff_pal_wd_hw_feed(void);
149: FF_Status_t ff_pal_wd_hw_reset(void);
```

Context verified in the working tree:

- `pal.h` is included by `pal_rtems.c:49`, `pal_watchdog.c:24`, `pal_posix.c:13` — so all three platform TUs that implement or call the hooks now see the prototype via `pal.h` before any use.
- `pal_rtems.c` additionally retains its own local forward declarations at `:111-113` (committed ancestor `0cc455f`); first use/call at `:167` (`ff_pal_wd_hw_feed()` inside `ff_pal_wd_hw_arm()`); definitions at `:147`/`:228`/`:253`; flight-target stubs returning `FF_NOT_IMPLEMENTED` at `:286-304` under the `#else` branch (NC-PAL-01).
- `pal_watchdog.c:29-31` declares the same three prototypes in a separate TU; calls at `:45`/`:65`/`:93`. Compatible duplicate declarations; build exit 0 with 0 project-source warnings.
- `pal_posix.c:673-694` provides the simulated-backend definitions under `!MPFS && !QEMU`; `pal_rtems.c` is compiled only under `FRICK_PLATFORM_MPFS || FRICK_PLATFORM_QEMU` (`:51`), emulation gated by `FF_ENABLE_RENODE_TEST` (`:93`). Complementary guards — no duplicate-definition/ODR risk.
- No API change: the public `FF_PalWatchdog*` API (`pal.h:129-137`) is unchanged; no new public symbol (lowercase `ff_*` internal names); no interface, behavior, register, timing, power, or traceability-requirement change. Traceability manifest already lists `pal.h` in the watchdog requirement implementation set (`docs/requirements/traceability.yaml:685`); trace gate unchanged (120/76/30/14/68).
- Gates reported green: build exit 0 (0 warnings), CTest, Lean `lake build`, trace PASS, format-check (pinned clang-format 18), license OK, RTEMS 6.2 cross-build exit 0 with zero implicit-declaration warnings.

NC-EXT-239 registration (`docs/planning/sprint-plan.md:326`): Minor (build hygiene/declaration) NC, OPEN; corrective action: declare `ff_pal_wd_hw_arm/feed/reset` in one PAL-watchdog header included by `pal_rtems.c`, then re-run the RTEMS cross-build with zero warnings. This change implements the declaration part of that corrective action (`pal.h` is included by `pal_rtems.c:49`). NC disposition remains an FSW/PA authority action; `sprint-plan.md:835` records NC-EXT-239 as **Deferred (non-functional, N21)**.

---

## 2. Sources consulted (all in repository, accessed 2026-11-12)

- `src/core/pal/pal.h` (working tree, lines 139-149, 129-137)
- `src/core/pal/pal_rtems.c` (lines 49, 104-113, 147, 167, 228, 253, 278-304)
- `src/core/pal/pal_watchdog.c` (lines 24, 29-31, 45, 65, 93)
- `src/core/pal/pal_posix.c` (lines 13, 658, 673-694)
- `docs/planning/sprint-plan.md` (NC-EXT-239 at :326; NC-PAL-01 at :240; NC-EXT-239 deferral at :835)
- `docs/status/handover.md:166,1811`; `docs/vv/verification-results.md:417` (historical records citing pre-fix `pal_rtems.c:156`)
- `docs/requirements/traceability.yaml:685`; `docs/requirements/traceability.md:146`
- `docs/architecture/rtems-integration.md:33,145`; `docs/engineering/user-integration-manual.md:378-379`
- Corroborating independent reviews retained in the working tree: `docs/reviews/SYS-R1-R3-pal-wd-forward-declarations.md` (R1/R3, findings SYS-R1-001..006, all Minor/No block), `docs/reviews/PA-REVIEW-NC-EXT-239-pal_rtems-20261111.md` (PA/N20), `docs/reviews/2026-11-11-vv-r2-nc-ext-239-independent-vv-review.md` (R2/V&V), prior R4 review `docs/reviews/FRICK-REVIEW-0479-R4-DOC-NC-EXT-239.md` (its DOC-R4-001 stale-line finding on `pal_rtems.c` was corrected in the committed tree).

---

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Blocks commit/push? |
|---|----------|--------------------|---------|----------|---------------------|
| DOC-R4-001 | Minor | `src/core/pal/pal.h:139-146` (comment on forward declarations; norm N8 accuracy) | The comment is substantively correct but contains two imprecisions: (a) "Implemented by exactly one platform translation unit" is overstated repository-wide — the hooks are implemented by **two** platform translation units (`pal_posix.c:673-694` simulated backend and `pal_rtems.c:147-253` emulated N18 binding, plus flight stubs `:286-304`); they are never co-compiled because of complementary guards, i.e. exactly one **per build configuration**, not exactly one in the repository. (b) The parenthetical "(pal_posix.c, pal_rtems.c)" omits `pal_watchdog.c`, which is the primary caller (`pal_watchdog.c:45/:65/:93`) and is protected by the `pal.h` include at `:24` just like the others. The claims "sees a prototype before first use" and "pal_watchdog.c declares the same prototypes for cross-TU consistency" are accurate. No compile/safety effect (build exit 0, 0 warnings; RTEMS cross-build exit 0; compatible duplicate declarations). Affects header-comment accuracy under norm N8 only. | `pal.h:141-144`; `pal_posix.c:13/658/673-694`; `pal_rtems.c:49/51/93/111-113/147-304`; `pal_watchdog.c:24/29-31/45/65/93`; build and cross-build gate results | **No**. Recommended pre-commit wording polish to close the N17 documentation-review loop cleanly; does not disqualify the item. Non-functional, N21-trackable if deferred. |
| DOC-R4-002 | Minor | `docs/planning/sprint-plan.md:326` (NC-EXT-239 register); `docs/status/handover.md:166,1811`; `docs/vv/verification-results.md:417` | These records still register NC-EXT-239 as OPEN and cite `pal_rtems.c:156`/`:217`, which are the pre-fix line numbers of the prior commit cycle (the `:111-113` declarations inserted in `0cc455f` shifted the call to `:167` and the `ff_pal_wd_hw_feed` definition to `:228`; the `pal_rtems.c` inline comment was re-based accordingly). They are NC-register/status entries and historical evidence, not product documentation of this change. If this commit is intended to close NC-EXT-239, register disposition and citation correction are FSW/PA authority actions, not a documentation update inherent to the declaration fix. This review neither closes NC-EXT-239 nor states it as closed; NC-PAL-01 remains OPEN regardless (flight hooks `FF_NOT_IMPLEMENTED`, `pal_rtems.c:278-304`). | NC register and status records cited | **No**. Disposition belongs to the FSW engineer/PA owner; no documentation finding blocks commit/push. |

---

## 4. Conclusions on the five review questions

**Q1 — Does any documentation need updating for this change (API, behavior, interface, traceability)?**
**No.** The change adds internal forward declarations only. The public PAL watchdog API (`pal.h:129-137`) is unchanged; no new public symbol; no ICD, command/telemetry, event, parameter, limit, or procedure impact. Traceability is unaffected — `traceability.yaml:685` already lists `pal.h` in the watchdog requirement implementation set, and the trace gate is unchanged (120/76/30/14/68). Hand-maintained documents describing the watchdog seam remain accurate (`docs/architecture/rtems-integration.md:33` describes the PAL platform-hook seam; `docs/engineering/user-integration-manual.md:378-379` describes implementing the hooks for a SoC watchdog — still correct, since the declarations do not alter what must be implemented). Test-contract documents (`docs/vv/test-spec/ts-7-xw2-target-contract.md:41,80`; `docs/vv/external-watchdog-test-contract.md`) describe link-time substitution of the hooks and are unaffected. Norm N8 is satisfied for the change scope; no SRS/SDD/ICD/traceability.yaml/user-manual edit is required.

**Q2 — Norm N2 (English) in the added comment?**
**Yes.** The added comment block at `pal.h:139-146` is written in English, is grammatically complete, and contains no non-English terms. Norm N2 satisfied.

**Q3 — Norm N8 (complete, current, deployable documentation)?**
**Satisfied for the change scope; no documentation changes are needed.** `pal.h` is parsed by Doxygen for the generated API reference, so the new section flows into generated output on regeneration (`make docs` gate passed) — no manual document edit is required. The only N8-relevant defect is internal to the change itself: DOC-R4-001 (comment imprecision on "exactly one platform translation unit" and the omitted caller TU). Rewording it keeps N8 satisfied; no separate document needs updating.

**Q4 — Does the added comment accurately describe the change (NC-EXT-239, forward declarations in `pal.h`, per-declaration purpose)?**
**Substantively yes, with one imprecision.**
- References NC-EXT-239: accurate — the register's corrective action is declaring the hooks in one PAL-watchdog header included by `pal_rtems.c`; `pal.h` is included at `pal_rtems.c:49` before first use at `:167`.
- Forward declarations in `pal.h`: accurate — lines 147-149, prototypes match the definitions in `pal_watchdog.c`, `pal_rtems.c`, `pal_posix.c` (build exit 0, 0 warnings).
- Purpose ("sees a prototype before first use, avoiding implicit-declaration diagnostics"): accurate for all three including TUs (`pal.h` is included before any use/definition in each).
- Imprecision: "Implemented by exactly one platform translation unit" — accurate per build configuration (complementary guards), overstated repository-wide (see DOC-R4-001). The comment describes the section collectively; it does not distinguish per-declaration purpose (arm vs feed vs reset semantics), which is acceptable because those semantics are carried by the `pal_watchdog.c` header comment (`pal_watchdog.c:5-18`) and the definitions themselves; internal hooks do not require per-declaration documentation in the header.

**Q5 — Are there documentation findings that block commit/push?**
**No.** No Major findings. DOC-R4-001 (Minor) is recommended wording polish; DOC-R4-002 (Minor) is an NC-register/status record owned by FSW/PA authority, not documentation inherent to this change. Neither blocks commit/push on technical, safety, security, traceability, or regulatory grounds.

---

## 5. Recommendation

1. **Before commit (recommended, not blocking):** reword `pal.h:141-144` to remove the overstatement, e.g.:

   ```
   * Platform-specific hardware binding hooks for the watchdog. Declared here
   * (a single shared header) so that every platform translation unit that
   * implements or calls them (pal_posix.c:13, pal_watchdog.c:24,
   * pal_rtems.c:49) sees a prototype before first use, avoiding
   * implicit-declaration diagnostics (NC-EXT-239). Implemented by exactly one
   * platform translation unit per build configuration (pal_posix.c simulated
   * backend or pal_rtems.c emulated binding; complementary guards prevent
   * co-compilation). pal_watchdog.c declares the same prototypes for
   * cross-TU consistency.
   ```
   
   If not folded in now, log DOC-R4-001 as a Minor NC for disposition at the next touch of `pal.h` (norm N21: non-functional findings are tracked, they do not stop unrelated development).

2. **Do not update separate documents** (SRS/SDD/ICD/traceability/user manual) for this change; none is required, and an unsolicited edit would introduce unrelated churn.

3. **NC-EXT-239:** if this commit is intended to close NC-EXT-239, handle register disposition and the `pal_rtems.c:156` citation correction as a separate FSW/PA action (`sprint-plan.md:326` owner); do not state the NC as closed in commit/release text without owner disposition. NC-PAL-01 remains OPEN and must not be represented as closed (flight hooks `FF_NOT_IMPLEMENTED`).

4. **Retention:** retain this report in `docs/reviews/` per norms N11/N17; it is evidence and recommendation only. Commit/push authority remains with the `aeronautics-space-fsw-engineer` agent after all applicable gates/norms and the full sprint scope pass (norms N7/N15); the N20 PA and this N17 R4 loop must close before push.

---

## 6. Verdict

| Element | Verdict |
|---|---|
| Documentation correctness (change scope) | Acceptable pending optional DOC-R4-001 reword |
| Norm N2 (English) | PASS |
| Norm N8 (complete/current documentation) | PASS for change scope (pending optional DOC-R4-001) |
| Traceability (N4) | PASS — unchanged, `traceability.yaml:685` already covers `pal.h` |
| Revision history (N16) | N/A — `pal.h` is source; no documentation revision-history files changed |
| Blocks commit/push | **No** |

**Overall: APPROVED WITH COMMENTS — two Minor findings (DOC-R4-001, DOC-R4-002), neither blocks commit/push.**

---

*Authority statement: This report is an independent documentation review under norm N17 (R4 profile). It is evidence and recommendation only. It does not approve the build, close NC-EXT-239 or NC-PAL-01, authorize commit/push, or waive any separate release/safety/security/regulatory gate. Commit/push remains with the `aeronautics-space-fsw-engineer` agent after all applicable gates/norms pass (norms N7/N15); independent qualified humans retain certification, approval, and NC-disposition decisions.*
