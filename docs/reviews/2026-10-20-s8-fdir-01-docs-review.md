# Independent R4 documentation review — sprint S8-FDIR-01 "panic / reset-recovery" (working tree, 2026-10-20)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0386                      |
| Reviewer session                  | Independent R4 documentation review (fresh context, norm N9/N17) |
| Reviewer profile                  | R4 (Senior SW documentation), with source-, build-config- and traceability-level cross-checks |
| Reference revision                | HEAD `44d2af224675cd65a66d2870eb0fc9553952799c` on `main`; increment under review is the **uncommitted working-tree delta on top of engineering baseline `318f49b`** (`git status --short`: 31 modified + 5 untracked files). No commit; the increment is the reviewed scope |
| Items under review                | `docs/adr/ADR-0021-watchdog-reset-recovery.md` (new), `docs/requirements/SRS.md`, `docs/requirements/traceability.yaml`, `docs/vv/test-spec/ts-5-core-services.md`, `docs/vv/tool-qualification.md`, `docs/vv/verification-results.md`, `docs/interfaces/ICD.md`, `docs/architecture/applications.md`, `docs/architecture/rtems-integration.md`, `docs/engineering/software-description.md`, `docs/engineering/user-integration-manual.md`, `docs/status/loop-audit.md`, `docs/status/handover.md`, `docs/planning/sprint-plan.md`, `CHANGELOG.md`, `README.md`, `docs/tooling.md`, `AGENTS.md`; plus `docs/requirements/traceability.md` and `docs/architecture/overview.md` as cross-checked (unmodified) siblings |
| Date                              | 2026-10-20                             |
| Scope                             | Bounded **N17 documentation review of the changed documents** of the S8-FDIR-01 increment: N8 factual accuracy and mutual consistency (notably the registered-application count 6 → 7), N16 revision/history discipline, honesty of the recorded state (unwired hardware binding, N18 gap, review deferral), the new ICD event-id catalogue caveat (NC-EXT-102), requirement text quality of SRS-REQ-073/074/075 and their `lean_note` honesty, internal link/reference validity, and the ADR. Against project norms `docs/rules.md` (N1–N22) and the ECSS-E-ST-40C / ECSS-Q-ST-80C information-item context invoked by `docs/quality/`. |
| Independence declaration          | Fresh review context; no part in authoring, executing or approving the increment, its evidence or any reviewed file. Read-only against the repository; **this report is the only file this reviewer created or modified.** No NC, gate, phase, approval or release was closed or approved by this reviewer. |
| Norms applied                     | N1–N22 (see `docs/rules.md`), emphasis on N4, N8, N9, N11, N16, N17, N21, N22 |
| Review-ID note                    | `grep -rn "FRICK-REVIEW-0386" docs/` is empty; the ledger in `docs/reviews/README.md` reaches `0383` and `0384`/`0385` are also unused. `0386` was free. Per the established convention (and the recorded NC-EXT-99/`F-0381-05` deferral) this reviewer does **not** add itself to `docs/reviews/README.md`; indexing is an owner/FSW action. |

## 1. Scope

**In scope.** The documentation product of the S8-FDIR-01 increment (PAL hardware
watchdog abstraction + pre-reset PUS S5 event report + single reset path). The
increment's own claimed state per `CHANGELOG.md` "S8-FDIR-01" section,
`docs/status/handover.md` §4 (rev 3.11), `docs/status/loop-audit.md` (rev 10.13),
`docs/planning/sprint-plan.md` row 1.74 and `docs/vv/verification-results.md`
rev 2.94: two new requirements-plus-one, `watchdog_timeout_s` 30 s → 8 s, a new
7th registered app, the lock-free panic report, the deliberately unwired RTEMS
binding (NC-PAL-01), the N18 gap (NC-TS-01) and the ICD event-id caveat
(NC-EXT-102).

**Method (read-only, independently re-derived).** `git status --short`,
`git diff` on every changed file; `sed`/`grep` line reads of each modified
document and its own `## History` table; source cross-checks of
`src/main/fsw_main.c` (8 `FF_EsRegisterApp` call sites, 7 unconditionally
registered), `src/apps/app.h`, `src/apps/app_wd.c`, `src/core/pal/pal_watchdog.c`,
`src/core/pal/pal_posix.c` (`FF_PalPanic` order), `src/core/pal/pal_rtems.c`
(unwired hooks), `src/pus/pus.c` (`FF_PusSendTmDirect`/`FF_PusEmitPanicEvent`,
lock-free path), `config/fsw_config.h` (`FF_EVS_PANIC_RESET 0x0104`),
`verification/lean/Frick/Watchdog.lean` (6 theorems), `tests/CMakeLists.txt`
(44 `add_test`, `test_pal_watchdog` registration) and `tests/unit/*`;
`python3 tools/scripts/trace.py docs/requirements/traceability.yaml` and
`python3 tools/scripts/metrics.py --check` (both re-run by this session,
read-only); repo-wide app-count greps.

**Excluded from scope.** No `make build`/`ci-local`/docker/RTEMS/Renode/QEMU
re-run and no unit-test re-execution (the retained-count claims are read, not
reproduced); no edit to any reviewed file; no disposition of NC-PAL-01,
NC-EXT-102 or NC-TS-01 (owner/authority items); no requirement/ICD semantics
change; no hosted-CI re-verification; no NC closure or N17/N20 phase pass.

## 2. References

- Project norms: `docs/rules.md` (N1–N22; N16 interpretation note 3 on per-document History scope)
- Review template: `docs/reviews/review-template.md`
- Traceability manifest (single source of truth): `docs/requirements/traceability.yaml`
- Requirement precedents: `FRICK-REVIEW-0381` (R4), M3 V&V R2 review `NC-1` (Major, stale `traceability.md`), `NC-EXT-77`
- NC register: `docs/planning/sprint-plan.md` rev 1.74 (`NC-PAL-01`, `NC-EXT-102`, `NC-TS-01`)
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2; ECSS-E-ST-70-11 5.7.5.2 (reset-recovery independence, report-before-reset), as cited by the ADR

## 3. Findings

### 3.0 Independently re-derived claims — CONFIRMED (no finding)

| Claim in the increment | Independent re-derivation | Result |
| --- | --- | --- |
| Registered-application count is now **7** | `src/main/fsw_main.c:103-115`: 8 `FF_EsRegisterApp` call sites, one (`RENODE_TEST`) guarded by `FF_ENABLE_RENODE_TEST`; the other 7 (CMD_INGEST, TM_OUTPUT, SCHEDULER, HK, HS, **WD**, SAMPLE) are unconditional | **CONFIRMED** — WD is the 7th, priority field `70`, stack `8 KiB`, registered unconditionally |
| `FF_EVS_PANIC_RESET` is `0x0104`, CRITICAL, emitted as PUS S5/C3 | `config/fsw_config.h:51` `#define FF_EVS_PANIC_RESET 0x0104u`; `src/pus/pus.c:217-219` writes the 2-octet id then `FF_EVS_CRITICAL` as the type octet, then the bounded reason text; subservice `3u` matches the existing `FF_PusEmitEvent` CRITICAL mapping (`pus.c:280-287`). The ordinary EVS→PUS S5 path (`fsw_main.c:43-52`) builds the identical `[id@2][type][text]` layout, so the ICD §4 row description and the new ICD note are accurate for both paths | **CONFIRMED** |
| `FF_PalPanic` order = log → report → reset → test seam → abort | `src/core/pal/pal_posix.c` `FF_PalPanic`: `fprintf`+`fflush` → `g_panic_notifier(msg)` → `FF_PalWatchdogReset()` → `#ifdef FF_TEST_HOOKS g_panic_handler` → `abort()`. Notifier and reset are unconditional; failure ignored | **CONFIRMED** (matches the ADR/CHANGELOG/TS-5 wording) |
| The pre-reset report path is lock-free by construction | `FF_PusSendTmDirect` takes no EVS/SB/PUS/queue/time mutex, allocates nothing (static `g_direct_frame`), and uses `FF_PalGetMonotonicNs()` for a degraded (non-UTC) time base; `FF_PusNextTmCounter()` is a plain static counter with no lock (`pus.c:107-112`) | **CONFIRMED** as an inspection-level property |
| RTEMS binding is deliberately **unwired** | `src/core/pal/pal_rtems.c` `ff_pal_wd_hw_arm/_feed/_reset` return `FF_NOT_IMPLEMENTED` with a log; `pal_posix.c` provides a *simulated* backend that performs no hardware action | **CONFIRMED** — honestly disclosed in ADR-0021, `rtems-integration.md` §2.1, `user-integration-manual.md` §11.1, `software-description.md`, the sprint plan, loop-audit and handover |
| Lean `Frick/Watchdog.lean` has **6** theorems: 3× `req_srs073_*`, 1× `req_srs074_report_precedes_reset`, 2× `req_srs075_*` | `grep theorem verification/lean/Frick/Watchdog.lean` → exactly those 6; the manifest links 3/1/2 respectively | **CONFIRMED** |
| Traceability gate converges | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` → `PASS (120 requirements, 76 srs, 30 sss, 14 gaps, 68 lean theorems checked)`; `metrics.py --check` → `metrics panel: current`, exit 0 | **CONFIRMED** (both read-only) |
| README metrics panel regenerated (120/76/30/14, 68 Lean, 72/76 delivered) | `metrics.py --check` current; counts match the trace gate | **CONFIRMED** |
| New test registration: `test_pal_watchdog` + `pal_watchdog` CTest, 44 total | `tests/CMakeLists.txt`: `add_executable(test_pal_watchdog …)`, `add_test(NAME pal_watchdog …)`, `set_tests_properties(pal_watchdog TIMEOUT 60 LABELS "n22;fdir;watchdog")`; `grep -c add_test` = 44 | **CONFIRMED** |
| Header/history discipline of the bumped documents | Header `Revision` == last `## History` rev in every bumped case: applications `0.5`, rtems-integration `0.3`, user-integration-manual `0.13`, ICD `1.5`, sprint-plan `1.74`, handover `3.11`, loop-audit `10.13`, ts-5 `1.6`, tool-qualification `0.3.88` (ordered 0.3.86→0.3.87→0.3.88, no duplicate), verification-results `2.94` | **CONFIRMED** for the bumped set (see F-0386-01/-02 for the two that were **not** bumped) |
| Honesty of the recorded state | loop-audit N18 row records the increment's N18 validation as a **gap, not a PASS**; `status.json` `renode_functional_validation: "yellow"` and README panel yellow; `documentation_review`/`product_assurance_review` yellow (pre-review); CHANGELOG/handover explicitly say "not safety-approved", "no hardware-protection claim", "independent reviews pending" | **CONFIRMED** — no overclaim found in the primary status records |
| ICD event-id caveat (NC-EXT-102) | `docs/interfaces/ICD.md` §4 note states the S5 *format* is documented but no controlled event-id *catalogue* exists, names `FF_EVS_PANIC_RESET` (`0x0104`, CRITICAL) as currently emitted, and forbids treating the ids as a controlled catalogue until the interface authority allocates them | **CONFIRMED** as accurate and correctly caveated (see F-0386-06 for a placement nuance) |

### 3.1 Findings table

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| F-0386-01 | **Major** | `docs/requirements/traceability.md` (`FRICK-TRACE-001`, N4/N8) | The human-readable traceability matrix was **not updated** although the manifest it explicitly mirrors changed by three requirements. It still reads `Counts (current): SRS = 73 … Lean theorems = 62` and contains **no** `SRS-REQ-073/074/075` rows in any section, while `traceability.yaml`/the gate now report `76 srs … 68 lean theorems`. The document's own header states it "mirrors its *current* content. Whenever the manifest changes … this document is regenerated/reviewed in the same commit (norms N8/N11)." This is the same defect class the project previously rated **Major** (M3 V&V R2 `NC-1`) and is tracked for earlier occurrences by `NC-EXT-77`. | `docs/requirements/traceability.md:5-8` (Rev 0.50, Date 2026-10-16), `:16-18` (`Counts (current): SRS = 73 … Lean = 62`), `:143-145` (rows stop at `SRS-REQ-072`); absent from `git status --short`; `trace.py` gate output `76 srs … 68 lean theorems`; manifest diff adds `SRS-REQ-073/074/075` | Regenerate the matrix from the manifest (counts, §1 SSS→SRS parent `SSS-REQ-008`, §2/§3 rows, history row) in the same change, then re-run `trace.py`. Interface-free; owner FSW; independent R4 delta re-review. |
| F-0386-02 | **Minor** | `docs/requirements/SRS.md` (N16) and `docs/engineering/software-description.md` (N16) | Both documents were **modified by this increment but carry no revision bump and no new `## History` row**. `SRS.md` gains three controlled requirements (`SRS-REQ-073/074/075`) yet its header remains `Revision 1.37 / Date 2026-10-19` with history ending at `1.37`; `software-description.md` gains/edits normative prose (seven apps; the not-a-mission-application reset-recovery caveat) yet remains `Revision 0.18 / Date 2026-10-13` with history ending at `0.18`. N16 interpretation note 3 exempts only ADRs; both are explicitly in-scope substantive documents. | `git diff docs/requirements/SRS.md` (only the 073/074/075 block; no header/history change); `docs/requirements/SRS.md:7-8` and history tail `1.37`; `git diff docs/engineering/software-description.md` (line-74/84 edits only); `software-description.md:7-8` and history tail `0.18`; `docs/rules.md:495-501` | Add the revision bump + a dated `## History` row to each in the next delta; they are not self-describing against `318f49b` otherwise. |
| F-0386-03 | **Minor** | `docs/engineering/software-description.md` line 394 (N8 consistency) | The **same document** edited by this increment is now internally inconsistent on the app count: line 74 was changed to "The **seven** applications…" and line 84 to "the **seven** `src/apps/*`", but the `make`-shortcuts table still says "boot the native simulator (**six** reference apps)". A reader of the changed document gets two different counts. | `docs/engineering/software-description.md:74`, `:84` (changed to seven) vs `:394` (`make run` … "six reference apps", unchanged); `src/main/fsw_main.c:103-115` (7 unconditional) | Update line 394 to "seven reference apps" in the same delta. |
| F-0386-04 | **Minor** | Residual "6 apps"/"six" references in sibling controlled docs (N8) | The count change 6 → 7 was applied only to the increment's touched docs (README, AGENTS, tooling, applications, software-description L74/84, user-integration-manual). Several other controlled docs still assert **6**, now factually stale: `docs/architecture/overview.md:111` ("Six applications are registered"); `docs/development-plan.md:120` ("six reference applications") and `:301` ("boots six reference applications"); `docs/vv/verification-plan.md:175` ("6 apps registered") and `:208` (S2 criterion "6 apps registered"); `docs/vv/renode-environment.md:76` ("the default build keeps **six** applications" — now **wrong**: the default native build is 7 because WD is unconditional); `docs/vv/test-spec/sva-1-levels-vmodel.md:64`/`:144`/`:272` ("6 apps"); `docs/vv/test-spec/ts-5-core-services.md:102` ("**six** reference applications" in the FRICK-ARCH-0005 reference row of the very document this increment bumped to rev 1.6). | Grep results as listed; `src/main/fsw_main.c:112` (WD unconditional); `tools/scripts/run_sim.sh` now asserts `7 applications registered` | Sweep the residual count references to 7 (or, where a criterion must stay pinned to a build variant, state the variant and the expected number) in the next documentation delta. `renode-environment.md:76` is the highest-impact (states a build fact). |
| F-0386-05 | **Minor** | `docs/architecture/rtems-integration.md` (`FRICK-ARCH-0011`), `docs/engineering/user-integration-manual.md` (`FRICK-ENG-0005`), `docs/interfaces/ICD.md` (`FRICK-ICD-001`) (N16) | The revision/date were bumped inconsistently: the **header `Date` was not advanced** to match the new history row in three documents — rtems-integration header `2026-10-13` vs history `0.3 @ 2026-10-20`; user-integration-manual header `2026-10-19` vs history `0.13 @ 2026-10-20`; ICD header `2026-10-17` vs history `1.5 @ 2026-10-20`. This is the same stale-header-date class corrected for the easy-fixes batch in `F-0378-04/-05/-06`. | `docs/architecture/rtems-integration.md:8` (Date `2026-10-13`) vs its history row `0.3`; `docs/engineering/user-integration-manual.md:8` (Date `2026-10-19`) vs row `0.13`; `docs/interfaces/ICD.md:7` (Date `2026-10-17`) vs row `1.5` | Advance each header `Date` to `2026-10-20`. |
| F-0386-06 | Comment | `docs/interfaces/ICD.md` §4 event-id note (NC-EXT-102) | The note is factually correct and well caveated. Two clarity refinements: (a) it precedes and is separate from the "Provisional profile note" which the section says sits "at the end of this section", so §4 now has two trailing authority-pending blocks whose relative scope is not stated; (b) the note says the framework emits "at least `FF_EVS_PANIC_RESET` … and the heartbeat/FAILED-application events referenced in the app sections", but the app sections are in `docs/architecture/applications.md`, not in this ICD — the cross-reference is implicit. Neither affects correctness. | `docs/interfaces/ICD.md:126-136` (new note), `:138+` (provisional note), `:18` (S5 row); no app/event sections in `docs/interfaces/ICD.md` | Optional: merge/mutually qualify the two §4 authority notes and make the app-event cross-reference explicit. |
| F-0386-07 | Comment | `docs/requirements/SRS.md` SRS-REQ-073/074/075 (requirement text quality) | The three requirements are grounded and largely testable, but each is **compound** (multiple `shall` clauses under one id), which weakens singular-intent/testability and makes per-clause verification ambiguous: 073 mixes arm + feed-period + hardware-independent reset + no-self-reliance + initialisation order; 074 mixes "exactly one" + identifier + severity + lock-freedom + no-allocation + "bounded worst-case time" + failure-independence; 075 mixes shared-mechanism + at-most-once + reject-on-truncated/unknown. In particular 074's "within a bounded worst-case time" is a verification obligation that the increment's own `lean_note` states is **not established** ("the worst-case time bound … is NOT established"). No ID/semantics change is proposed here (requirements authority owns that). | `docs/requirements/SRS.md` (S8-FDIR-01 block); `docs/requirements/traceability.yaml` SRS-REQ-074 `lean_note`; ADR-0021 §"Pre-reset event report" ("worst-case emission time is **not** analysed") | Requirements-authority (R3/N14) consideration: split into singular requirements and either bound 074's timing as an explicit TBD or drop the unverifiable timing clause until a per-backend budget exists. Non-functional under N21. |
| F-0386-08 | Comment | `docs/status/status.json` (N17 scope) | `status.json` is in the N17 documentation scope and is the source of the README metrics panel, but it was **not refreshed** by this increment: its only `note` still describes the baseline as the S8-SCH/S8-VV-FU batch committed as `1fdbb83`, superseded by `318f49b` and then by this working-tree delta. The generated panel (`metrics.py --check`) is internally consistent, so this is narrative staleness, not a metrics/panel divergence — and it is largely pre-existing drift, not introduced here. The increment's behaviour change (new app) makes the refreshed status note more relevant. | `docs/status/status.json:5` (note cites `1fdbb83`); unchanged by the increment (`git status --short`); `docs/status/loop-audit.md`/`handover.md` name `318f49b` | Refresh the `status.json` note to the current baseline in the next status delta; non-functional. |
| F-0386-09 | Comment | App priority claim in `docs/architecture/applications.md:20`, `docs/architecture/rtems-integration.md:49`, `docs/engineering/user-integration-manual.md:385` | The docs repeatedly call `FF_AppWdMain` the "**lowest-priority**" app "so it is starved if higher-priority work hangs". In `fsw_main.c` WD is registered with `priority = 70`, which is the **numerically largest** value in the app table (others 25–60); POSIX `sched_priority` is passed through directly (`pal_posix.c:158`), and on `SCHED_FIFO`/`SCHED_RR` a larger number is a *higher* priority. **No priority-ordering convention (direction of the numeric value) is documented anywhere in the repo**, so the "lowest priority" claim rests on an undocumented convention and could be inverted from the code alone. This is not asserted as a code defect (the intended behaviour may well hold if the project's convention is smaller-value=higher-priority, as RTEMS classic-API style), but the claim is not currently self-evidencing. | `src/main/fsw_main.c:103-115` (WD `70`, max of the set); `src/core/pal/pal.h:37-38` (priority passed to `FF_PalTaskCreate`); `src/core/pal/pal_posix.c:158` (`sp.sched_priority = (int)priority`); no priority-direction statement in `pal.h`/`es.h`/architecture docs | Document the priority convention (one sentence in `pal.h`/`es.h` and/or `docs/architecture/applications.md`) so the "lowest-priority feeder" design rationale is verifiable; confirm the direction with the FSW owner. Non-functional. |

**Severity totals: 1 Major, 4 Minor, 4 Comment.**

### 3.2 Notes on claims not independently reproducible in this review

- The focused check counts (`test_pal_watchdog.c` 32 checks; `test_es_hs_lifecycle.c`
  94 checks) are **runtime** counts. Static inspection confirms 32 `FF_TEST_ASSERT`
  invocations in `test_pal_watchdog.c` and 73 in `test_es_hs_lifecycle.c`; the
  harness's reported per-assert check count can exceed the static call count, so
  the "94" figure is accepted as author-reported and is **not** independently
  reproduced here (no test execution was performed). This is a confidence limit,
  not a finding.
- The "44/44 CTest", "build PASS", "clang-format PASS", "Doxygen PASS" and the
  two self-corrected defects recorded in `verification-results.md` rev 2.94 rest
  on the author's cycle; this reviewer did not re-run the pipeline.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [x] REJECTED - one or more Major findings

**Bounded conclusion.** For the documentation product of the S8-FDIR-01
working-tree increment on baseline `318f49b` as reviewed at HEAD `44d2af2`: the
increment's substantive factual claims were independently re-derived and
**confirmed** (7 registered apps, `FF_EVS_PANIC_RESET 0x0104`/CRITICAL/S5-C3,
the `FF_PalPanic` order, the lock-free construction, the unwired RTEMS binding,
the 6 Lean theorems, the trace and metrics gates, the 44 CTest registrations,
the ICD event-id caveat, and — importantly — the **honesty** of the recorded
state: N18 recorded as a gap not a PASS, `renode`/review semaphores yellow, no
safety/qualification claim). One **Major** finding (F-0386-01) prevents approval:
the human-readable `traceability.md` mirror was not regenerated although the
manifest gained three requirements — a defect the project has previously rated
Major. Four Minor and four Comment findings are tracked in §6. Because N12/N17
close a loop only when the applicable findings are dispositioned and the delta
re-review passes, this report is **REJECTED** for the reviewed revision.

**Explicit limits of this conclusion (read before citing).**

1. This is an **increment-scoped R4 documentation review** (N17 leg), not an N17
   phase pass. The increment's independent R2/R3/PA reviews remain outstanding;
   this report satisfies none of them.
2. It closes no NC: `NC-PAL-01`, `NC-EXT-102`, `NC-TS-01`, `NC-EXT-77`,
   `NC-EXT-94/97/98/99/100/101` and all other register items remain OPEN as
   recorded. It approves no requirement, ICD row, baseline, deviation, risk or
   release, and implies no qualification, target or hosted-CI evidence.
3. Evidence verification was **inspection-level**: sources, headers/history
   tables and the manifest were read and greps re-run; the builds, tests,
   sanitizer, format, Doxygen, Renode and QEMU legs were **not re-executed**.
4. F-0386-01/-02/-03/-04/-05 are documentation currency/consistency defects
   (non-functional under N21) except that F-0386-01 is a Major gate-class
   non-conformity by the project's own precedent for the traceability mirror.
   F-0386-07 (requirement semantics) is requirements-authority-owned and was not
   edited by this reviewer.

**Confidence: Medium-High** for the confirmed items (direct re-derivation from
source, build configuration, manifest and the trace/metrics gates; no material
conflict found). **Recheck trigger:** any change to `src/main/fsw_main.c`,
`src/apps/app_wd.c`, `src/core/pal/*`, `src/pus/pus.c`, the traceability
manifest/`traceability.md`, the ICD, the SRS, or a new revision of
`verification-results.md`/`tool-qualification.md`.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R4 (N9) | Independent documentation-review session (fresh context, `aeronautics-space-sw-documentation-engineer` profile) | 2026-10-20 | REJECTED (increment-scoped; 1 Major, 4 Minor, 4 Comment; not an N17 phase pass; no NC/gate/approval closed) |

## 6. Follow-up records

(norm N12: NCs are resolved by re-applying ALL rules to the affected scope and
re-reviewing the deltas until full compliance; N21: the Minor/Comment items below
are non-functional and are tracked for later disposition rather than blocking
unrelated development, **except** that a REJECTED review blocks commit/push
(norm N7/N12) until the Major and the applicable Minor findings are resolved and
the delta re-review passes.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC candidate (to be registered by owner; next free `NC-EXT` id) | **Major** | `docs/requirements/traceability.md` (`FRICK-TRACE-001`), N4/N8 | F-0386-01: human-readable traceability mirror not regenerated for SRS-REQ-073/074/075; counts 73/62 stale vs 76/68; rows missing | Regenerate §1–§4 + counts + history from the manifest in the same change | Updated `traceability.md` + `trace.py` PASS + R4 delta check | FSW / requirements custodian | OPEN (raised by this review) | R4 delta re-review (required before commit) |
| NC candidate (next free `NC-EXT` id) | Minor | `docs/requirements/SRS.md`, `docs/engineering/software-description.md`, N16 | F-0386-02: both modified with no revision bump and no history row | Add revision bump + dated history row per document | Updated headers/history + R4 delta check | FSW / documentation | OPEN | R4 delta check |
| NC candidate (next free `NC-EXT` id) | Minor | `docs/engineering/software-description.md:394`, N8 | F-0386-03: internal "six" vs "seven" contradiction in the changed document | Correct line 394 to seven | Updated file + R4 delta check | FSW / documentation | OPEN | R4 delta check |
| NC candidate (next free `NC-EXT` id) | Minor | `docs/architecture/overview.md`, `docs/development-plan.md`, `docs/vv/verification-plan.md`, `docs/vv/renode-environment.md`, `docs/vv/test-spec/sva-1-levels-vmodel.md`, `docs/vv/test-spec/ts-5-core-services.md`, N8 | F-0386-04: residual "6 apps"/"six" references; `renode-environment.md:76` states a now-false build fact | Sweep the residual count references to 7 (or state the pinned build variant) | Grep evidence + R4 delta check | FSW / documentation | OPEN | R4 delta check |
| NC candidate (next free `NC-EXT` id) | Minor | `docs/architecture/rtems-integration.md`, `docs/engineering/user-integration-manual.md`, `docs/interfaces/ICD.md`, N16 | F-0386-05: header `Date` not advanced to match the new history row in three documents | Set each header Date to 2026-10-20 | Updated headers + R4 delta check | FSW / documentation | OPEN | R4 delta check |
| NC candidate (optional; Comment-level) | Comment | `docs/interfaces/ICD.md` §4 | F-0386-06: two trailing authority notes in §4; implicit app-event cross-reference | Optional clarity refinement | Updated ICD (interface authority) + R4 check | Interface authority with FSW | OPEN (observation) | R4 delta check |
| NC candidate (optional; Comment-level) | Comment | `docs/requirements/SRS.md` SRS-REQ-073/074/075 | F-0386-07: compound requirements (singular intent/testability); 074's timing clause not established per its own `lean_note` | Requirements-authority (R3/N14) decision to split/bound | SRS delta + independent R3 review | Requirements authority (R3) with FSW | OPEN (observation) | R3 delta review |
| NC candidate (optional; Comment-level) | Comment | `docs/status/status.json` | F-0386-08: status `note` stale (`1fdbb83` baseline); panel itself consistent | Refresh the status note in the next status delta | Updated `status.json` + R4 check | FSW / documentation | OPEN (observation) | R4 delta check |
| NC candidate (optional; Comment-level) | Comment | `docs/architecture/applications.md`, `rtems-integration.md`, `user-integration-manual.md` | F-0386-09: "lowest-priority" claim rests on an undocumented priority-ordering convention | Document the convention; confirm direction with FSW | Doc/pal.h delta + R4 check | FSW owner | OPEN (observation) | R4 delta check |

---

*This report modifies no file other than itself. Per the established convention
it is not added to `docs/reviews/README.md` by this reviewer; indexing and the NC
dispositions above are owner/FSW actions (norm N7/N12). It is committed and
pushed with the accompanying change set under norm N11 by the authorized agent,
not by this reviewer.*
