# FRICK-REVIEW-0460 — Independent V&V Closure Confirmation (N9/R2)

| Field | Value |
| --- | --- |
| Document ID | **FRICK-REVIEW-0460** (V&V) |
| Reviewer profile | Senior Aeronautics and Space Software Verification & Validation (SW V&V) Engineer — independent closure confirmation (norm N9 / R2 profile) |
| Reviewer session | **`ses_e5a42c284ffek26d3YdHyLsvB8`** (observed in `OPENCODE_SESSION_ID` of this reviewer process, 2026-11-04) |
| Date | 2026-11-04 |
| Candidate base commit | `41dcbe21435e14b5aaff06c63d9df6f0de04df54` (branch `main`, verified as `HEAD`); all reviewed content is uncommitted working-tree content on top of that commit |
| Candidate manifest | `docs/vv/evidence/2026-11-04-re-review/candidate-manifest.sha256` |
| Manifest entries / result | **192 entries**; `sha256sum -c` → **exit 0, 192 OK, 0 FAILED** (first action of this review) |
| Manifest sha256 | `7e47fed14e5a4d5530919ea53e69e775c1691c37783d8fc98345398d1f3a3125` (recomputed; **matches** the value in the review request) |
| Scope | Narrow closure confirmation of the two findings of FRICK-REVIEW-0458 on the current bytes: **F-0458-01** (`test_evs_hs_health.c` header accuracy/citation) and **F-0458-02** (smoke rebuilt the authoritative gate directory after CTest). Plus: regression check that the delta changed no `.c`/`.h` outside the test file, that `src/main/fsw_main_reqtest.c` is manifest-bound but uncompiled, and that no named document now claims NC/sprint closure, N18 pass, T3 qualification, hardware evidence or release approval. |
| Out of scope | Fresh full review of the candidate; N18/Renode/QEMU/HIL/target behaviour; hardware; human qualification; security testing; PUS-C ICD verification; Lean/trace/format/cppcheck/smoke/GSW re-execution; RTEMS cross-build; N10 re-assessment of test scripts; PA leg; any compliance, qualification, acceptance, NC-closure or release decision |
| **Verdict** | **PASS WITH OBSERVATIONS** |

**Verdict rationale (one line):** both FRICK-REVIEW-0458 findings are genuinely cured and independently reproducible on the current bytes — the test header now states the correct production wiring and the correct NC-EXT-236 scope, and the authoritative `build/gate11` artifacts are structurally ordered (all binaries 09:43:18, CTest log 09:43:42, test-execution log 09:43:43, smoke in a separate `build/gate11-smoke` at 09:44:43, and `cmake --build` reports no work) — while two non-functional documentation-consistency observations remain (mixed 520-571 / 523-575 citations, and smoke-log wording that attributes the separate-directory proof to the log text rather than to the artifacts).

---

## 1. Mandatory first action — manifest verification

```
$ sha256sum -c docs/vv/evidence/2026-11-04-re-review/candidate-manifest.sha256
exit = 0 ;  192 lines ": OK" ;  0 "FAILED" ;  192 manifest entries
$ sha256sum docs/vv/evidence/2026-11-04-re-review/candidate-manifest.sha256
7e47fed14e5a4d5530919ea53e69e775c1691c37783d8fc98345398d1f3a3125
```

The gate passed as required (exit 0, 192 OK), so the review proceeded. `git rev-parse HEAD` = `41dcbe21435e14b5aaff06c63d9df6f0de04df54`, branch `main`.

---

## 2. Independence declaration

- This review ran in **`ses_e5a42c284ffek26d3YdHyLsvB8`**, a separate sub-agent session from the authoring/correction work and from earlier reviewers (`ses_e5a4ecb0fffeXKbgIlYxvKkKwE` for 0458, `ses_e5a4ecb0effetawalmRd4UDzgi` for 0459).
- I authored none of the reviewed content, ran none of the recorded gate campaigns, and modified **only this report**. I recomputed hashes, inspected the retained artifacts, re-ran CTest and the test binary, and issued one `cmake --build` no-op check.
- I am an AI analysis role: **no human credentials, no regulatory delegation, no test-witness authority, no independence certification, and no power to approve** qualification, acceptance, certification, anomaly closure, or release/flight readiness. Independence here is a recorded session boundary plus this declaration, not a formal N9 certificate.
- I read no paywalled standard in full; governing-baseline statements are limited to project-controlled artifacts.

---

## 3. Evidence independently reproduced

All commands from `/home/nicola/fsw/frick-fsw`; container runs use `frick-vv:local`. The authoritative gate directory is `build/gate11`; `build/gate10` and older logs were treated as disclosed superseded history per the package README.

| Check | Command (abridged) | Result |
| --- | --- | --- |
| Candidate manifest | `sha256sum -c …/candidate-manifest.sha256` | **exit 0, 192 OK, 0 FAILED** |
| Manifest self-hash | `sha256sum …/candidate-manifest.sha256` | `7e47fed1…a3125` — **matches** the request |
| Test-script hash | `sha256sum tests/unit/test_evs_hs_health.c` | `23535db30a28bc15f90ecf1679b0195edc835bfada9fa5f6b044e4c851626035` — matches manifest, TS-5, tool-qualification and README |
| Retained new-test counts | `cat gate11-new-tests.log` | `test_frozen_utc_seam` **4325/0**; `test_evs_hs_health` **110/0**; `GATE11_NEW_TESTS_EXIT=0` |
| Test binary re-execution | `/workspace/build/gate11/tests/test_evs_hs_health` | `checks=110 failures=0`, exit 0 |
| CTest on authoritative binaries | `ctest --test-dir /workspace/build/gate11` (container) | `100% tests passed, 0 tests failed out of 52`, exit 0 |
| No-op rebuild | `cmake --build /workspace/build/gate11` (container) | `[0/2] Re-checking globbed directories… ninja: no work to do.` exit 0 |
| Build transcript | `gate11-build.log` | 403 lines, **402 edges** `[1/402]`…`[402/402]`, **0** `warning:`/`error:`; `GATE11_BUILD_EXIT=0` |
| Configure / cppcheck exits | `gate11-*-exit.txt` | `GATE11_CONFIGURE_EXIT=0`, `GATE11_CPPCHECK_EXIT=0` |
| Gate ordering (mtime, ns) | `stat -c '%y'` | binaries **09:43:18.198–.481**; `gate11-ctest.log` **09:43:42.893**; `gate11-new-tests.log` **09:43:43.565** |
| Last ninja build edge | `.ninja_log` `mtime_ns` field, max over 405 entries | **09:43:19.861** (`tests/test_lwip_netif`) — before the CTest log |
| Separate smoke build | `stat build/gate11-smoke`, `gate11-smoke.log` | smoke dir binaries `frick_fsw` **09:44:43.465**, `test_evs_hs_health` **09:44:46.434**; log **09:45:00.494**; `GATE11_SMOKE_EXIT=0` |
| `build/gate11` after smoke | `stat` binaries again | unchanged at **09:43:18** — no relink |

Reviewer-caused artifact note: my no-op `cmake --build` touched `build/gate11/.ninja_log` metadata (glob restat entry) at ~09:47, but added **no compile/link edge** and left every binary mtime unchanged. It is untracked build metadata, not reviewed content.

---

## 4. Verdict and finding summary

| Severity | Count |
| --- | --- |
| Blocker | 0 |
| Major | 0 |
| Minor | 0 |
| Observation (non-functional) | 2 |
| **Total findings** | **2 observations** |

Verdict: **PASS WITH OBSERVATIONS**. Both FRICK-REVIEW-0458 findings are **RESOLVED**. No Blocker/Major/Minor remains in the two scoped items. The two observations below are documentation-consistency items that do not affect any gate result or product claim, and I classify them explicitly as non-functional.

| ID | Class | Location | Observation | Effect |
| --- | --- | --- | --- | --- |
| **O-0460-01** | Observation (non-functional) | `tests/unit/test_evs_hs_health.c:17`; `docs/vv/test-spec/ts-5-core-services.md:177`; `docs/vv/verification-results.md:313-318`; `docs/planning/sprint-plan.md:323`; `docs/status/handover.md:1599` | The same listener→sink coverage is cited as **`test_r3_gap.c:523-575`** in the test header/README §2.3 and as **`520-571`** in the other records. Both ranges contain the listener adapter and every listener→S5→sink assertion (`rx_find(5,1)`, `(5,2)`, `(5,3)` at 554/561/566); `520-571` stops mid-block and omits only the final EVS-log assertion at 573. | None. No gate result or product claim depends on the exact end line. The header's `523-575` is the accurate full-range citation. |
| **O-0460-02** | Observation (non-functional) | `docs/vv/evidence/2026-11-04-re-review/README.md` §2.2 ("`gate11-smoke.log` … records the separate directory's own build") | The log text is three lines (`[smoke] building`, the boot window, `PASS`) and does not itself name `build/gate11-smoke`. The separate-directory fact is proven by the artifacts — `build/gate11-smoke/frick_fsw` at 09:44:43, `Makefile`/`run_sim.sh` honouring `BUILD_DIR`, and untouched `build/gate11` binaries — not by the log text. | None. The claim is true and structurally substantiated; only the attribution to the log wording is slightly strong. |

No new functional defect was observed in the exact bytes. I did not manufacture a finding to offset these observations, and I did not suppress any defect I believe is real.

---

## 5. Per-finding closure — FRICK-REVIEW-0458

| ID | Prior severity | Status | Evidence in the current tree |
| --- | --- | --- | --- |
| **F-0458-01** — test header falsely said the listener does not forward | Minor | **RESOLVED** | **(a) False claim removed.** `tests/unit/test_evs_hs_health.c:13-19` now reads: "What this test does NOT verify: the ASSEMBLED listener -> PUS service 5 -> production TM sink -> link chain. The listener does forward (`src/main/fsw_main.c` installs `ff_event_listener`, which calls `FF_PusEmitEvent`)…". The string "the listener does not currently forward" is gone. **(b) Agrees with product:** `src/main/fsw_main.c:33` defines `ff_event_listener`, `:51` calls `FF_PusEmitEvent`, `:95` installs it via `FF_EvsSetListener(ff_event_listener)`, and `:99` calls `FF_AppTmOutputInit()`. **(c) Citation verified in the actual bytes:** `test_r3_gap.c:523-542` defines `r3_evs_to_pus` (builds the S5 payload, calls `FF_PusEmitEvent` at 541); `:544-575` defines `test_pus_event_s5_propagation`, which installs the listener at 550 and consumes the sink output with `rx_find(5,1)`, `(5,2)`, `(5,3)` at 554/561/566 plus severity-byte assertions; the suite configures `tm_sink` via `FF_PusSetTmSink` at `:156`. **The cited range really contains the listener→S5→sink assertions.** **(d) NC-EXT-236 correctly described** at `:17-19` and again at `:94-96` as the absent ASSEMBLED-chain oracle through the production `FF_AppTmOutputInit`/`ff_to_sink` pair; that matches `src/apps/app_tm_output.c:16` (`ff_to_sink`) and `:25` (`FF_PusSetTmSink(ff_to_sink)` inside `FF_AppTmOutputInit`). **Hash/checks:** file sha256 `23535db3…6035` matches the recorded value and manifest line; `gate11-new-tests.log` records `checks=110 failures=0`; my own execution reproduced 110/0, exit 0. **Delta is non-semantic to the extent observable:** no `.c`/`.h` other than this file is newer than the 0458 report; comparing the gate10 and gate11 test binaries, `.text`/`.rodata` **sizes are identical (40678/2369)**, per-function sizes are identical, and the string-set difference is limited to the build-directory path (plus one 3-byte artifact) — no assertion text was added or removed and the check count is unchanged. Comment-only cannot be proven from binaries alone because `__LINE__` immediates shifted with the header, which is stated rather than glossed. | 
| **F-0458-02** — smoke rebuilt the authoritative directory after CTest | Minor | **RESOLVED** | **(a) Ordering is structural, not prose:** `build/gate11/frick_fsw` **09:43:18.198**, `tests/test_evs_hs_health` **09:43:18.481**, `tests/test_frozen_utc_seam` **09:43:18.380**; `gate11-ctest.log` **09:43:42.893**; `gate11-new-tests.log` **09:43:43.565** — both logs post-date the final link by ~24 s. The last ninja build edge is **09:43:19.861**, before CTest. **(b)** `cmake --build /workspace/build/gate11` in `frick-vv:local` prints `ninja: no work to do` (exit 0). **(c)** Smoke built a **separate** directory: `build/gate11-smoke/` exists with its own `frick_fsw` at **09:44:43.465** and `tests/test_evs_hs_health` at **09:44:46.434**, log **09:45:00.494**, `GATE11_SMOKE_EXIT=0`; `Makefile` (`BUILD_DIR ?= build/native`) and `run_sim.sh` (`BUILD_DIR="${BUILD_DIR:-…}"`, `cmake -S/-B` + `cmake --build` on that dir) show the mechanism, and the `build/gate11` binaries remained at 09:43:18 — **not relinked**. **(d)** `docs/vv/evidence/2026-11-04-re-review/README.md` §2.2 marks `build/gate10` **superseded** for exactly this reason: "F-0458-02: the smoke step re-ran `cmake -S/-B` and `cmake --build` on this directory ~30 s **after** the retained CTest, so its CTest log does not describe the final linked binaries". **Independent re-execution:** CTest on `build/gate11` → **52/52 passed, 0 failed**, exit 0. | 

**Closure of FRICK-REVIEW-0458: 2 RESOLVED, 0 PARTIALLY RESOLVED, 0 NOT RESOLVED.**

---

## 6. Regression check — did the delta change anything it should not have?

- **Source footprint.** `find src tests config -name '*.[ch]' -newermt "2026-11-04 09:38:17"` (the mtime of FRICK-REVIEW-0458) returns exactly one path: `tests/unit/test_evs_hs_health.c` (09:41:43.612). No other C source or header changed after the 0458 review. This is consistent with the disclosed comment-only change.
- **Manifest binding of the manifest-only input.** `src/main/fsw_main_reqtest.c` is bound at the manifest (`sha256 21cfaa0efe8c500959f9d73437b428bda128461c72442c21c01168fd662b1277`). It is **not** referenced by any `CMakeLists.txt`/`*.cmake` and is absent from `build/gate11/build.ninja`; it is guarded by `#if !FF_ENABLE_TARGET_REQ_TEST` / `#error` at `:44-45`. The manifest is selective (3 `src/` entries: `pal.h`, `pal_posix.c`, `fsw_main_reqtest.c`); because the other sources are unmodified, base commit + manifest still reconstructs the `src/**` inputs used by the licence and cppcheck gates, so the PA-0459-02 statement checked here is not overstated.
- **Document status claims (named set).** No live claim of NC closure, sprint closure, N18 pass, T3 qualification, hardware evidence or release approval was found:
  - `docs/planning/sprint-plan.md` **Rev 1.125** (latest row) ends "No NC closed, no sprint closed, no commit/push"; NC-EXT-236's task row (`:323`) remains "**OPEN** — owner role: FSW engineer; acceptance authority: Nicola… No closure claimed."
  - `docs/status/handover.md` **rev 3.64** (latest row) ends "No NC closed, no sprint closed, no commit/push"; the current-state section records N18 **FAIL** (NC-EXT-198/NC-PAL-01).
  - `docs/vv/tool-qualification.md` **rev 1.5** states "T3 status unchanged (**NOT FULLY QUALIFIED**)" and "No NC closed"; the older rev 1.2 "T3 status updated to QUALIFIED" is retained as disclosed superseded history under rev 1.3's withdrawal (N11), not as a live claim.
  - `docs/vv/test-spec/ts-5-core-services.md` **Rev 1.14** records `test_evs_hs_health` at **110/110**, hash `23535db3…6035`, `gate11-new-tests.log`, and closes its revision row with "No NC closed, no tool qualified"; NC-EXT-236 is stated as an open gap, not as closed.
  - `docs/vv/verification-results.md` lists NC-EXT-236/237 as open with owner/acceptance authority and no date, states "No NC closed, no sprint completed", and bounds its verdict to "the configurations tested".
  - `docs/reviews/README.md` rows for 0458 and 0459 are **PASS WITH COMMENTS** with no closure/approval claim; there is no 0460 row yet (this report is not self-indexed).
- **Pre-existing narrow closures of other NCs** (e.g. NC-EXT-88/89/91 in an older S8-PUS-FU task row) are historical records outside this delta and are not claims about the NCs under review. I treat them as out of scope and do not certify them.

---

## 7. What this review did NOT verify (explicit limits)

1. **No full re-review** of the candidate; only the two 0458 findings and the stated regression/doc checks.
2. **No clean reconfigure or full rebuild.** I used the retained `build/gate11` artifacts, confirmed `ninja: no work to do`, and re-ran CTest and one test binary. I did not regenerate the 402-edge build, format, Lean, trace, cppcheck or smoke.
3. **No N18 / Renode / QEMU / HIL / hardware / timing / WCET / multicore-interference verification.** N18 remains FAIL per the documents; no hardware evidence is asserted here.
4. **No security testing**, no PUS-C ICD conformance check, no N10 re-qualification of any script, no review of the PA leg (0459) or of reports other than those needed for the two findings.
5. **Pre-fix source bytes not retained.** I could not diff the 0458-era `test_evs_hs_health.c` byte-for-byte; the comment-only claim is supported by mtimes, manifest binding, identical check counts, and identical binary section/function sizes, but not by a retained prior copy.
6. **No verification of the truth of historical claims** made in superseded rows or older evidence packages; they were read only to check that they are labelled superseded.

---

## 8. Explicit closure and authority statements

- This review **closes no NC**. NC-EXT-235, NC-EXT-236, NC-EXT-237, NC-EXT-19, NC-EXT-198 and NC-PAL-01 remain open.
- This review **closes no sprint** and declares no sprint or milestone complete.
- This review **does not record an N18 pass**; N18 remains FAIL.
- This review **does not qualify any tool or test script** (T3 remains NOT FULLY QUALIFIED at these bytes).
- This review **provides no hardware, target, timing, safety, security or human-review evidence**.
- This review **approves no release, acceptance, certification or flight readiness**, and waives no gate.
- This review **authorizes no commit and no push**, and I made none.

Human approval remains mandatory for anomaly closure, waiver/deviation acceptance, qualification/acceptance conclusions and release decisions. This document is analysis input, not an approval.
