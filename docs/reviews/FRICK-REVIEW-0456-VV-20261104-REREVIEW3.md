# FRICK-REVIEW-0456 — Independent V&V Delta Re-review (round 3)

| Field | Value |
| --- | --- |
| Document ID | **FRICK-REVIEW-0456** (V&V) |
| Reviewer profile | Senior Aeronautics and Space Software Verification & Validation (SW V&V) Engineer — independent delta re-review (norm N9 / R2 profile) |
| Reviewer session | **`ses_e5a62463fffefAErDhQTtZQzrL`** (observed in `OPENCODE_SESSION_ID` of this reviewer process, 2026-11-04) |
| Date | 2026-11-04 |
| Candidate base | Branch `main`, commit `41dcbe21435e14b5aaff06c63d9df6f0de04df54`; all reviewed content is uncommitted working-tree content on top of that commit |
| Candidate manifest | `docs/vv/evidence/2026-11-04-re-review/candidate-manifest.sha256` |
| Manifest sha256 | `08bf50ced391e0acbe0761ed2c329de5e854f132eee48d8290edf007fce0bb6b` (recomputed; **matches**) |
| Manifest entries | **130** (`sha256sum -c` → **exit 0, 130 OK, 0 FAILED**) |
| Scope | Delta re-review of the corrections to FRICK-REVIEW-0454 (F-0454-01…05) and, for completeness, re-confirmation of FRICK-REVIEW-0452 (F-01…F-11) against the current tree; fresh N10 assessment of `test_frozen_utc_seam.c` and `test_evs_hs_health.c` at their stated hashes; evidence-package integrity (manifest, 2026-11-03 assets manifest, RTEMS symbol transcript, disclosure §2.1, gate-log correspondence) |
| Out of scope | N18/Renode/target/HIL behaviour; hardware; human qualification; security testing; PUS-C ICD verification; full clean rebuild; Lean/trace/cppcheck re-execution; any compliance, qualification, acceptance, NC closure or release decision; findings of FRICK-REVIEW-0455 (PA) except where they touch the same artifacts |
| **Verdict** | **PASS WITH COMMENTS** |

**Verdict rationale (one line):** every round-two Major is genuinely corrected and independently reproducible — the 2026-11-03 assets manifest verifies at **exit 0 / 29 OK** in the correct README→manifest→verify order, TS-5 now states **108/108** and the binary reproduces it, no live `T3 QUALIFIED` claim survives, and the RTEMS transcript now carries literal commands whose counts (1450 / 1085 / 1450) and zero seam symbols I reproduced with the RTEMS `nm` — but the corrections introduced or left **three Minor and three Advisory accuracy defects**, the most material being an unsupported "402 targets / 0 compiler warnings" build claim and an inconsistent NC-EXT-236 scope that is contradicted by the test and by `app_tm_output.c`.

---

## 1. Finding summary

| Severity | Count |
| --- | --- |
| Blocker | 0 |
| Major | 0 |
| Minor | 3 |
| Advisory | 3 |
| **Total** | **6** |

| ID | Severity | Location | Finding | Required action |
| --- | --- | --- | --- | --- |
| **F-0456-01** | **Minor** | `docs/vv/evidence/2026-11-04-re-review/README.md:65`; `docs/vv/evidence/2026-11-04-re-review/build.log` (3 lines) | **The build row claims more than the bound artifact evidences.** README §3 states `build — exit 0 — 402 targets, 0 compiler warnings`. The manifested `build.log` is a three-line *incremental* rebuild (`[1/2] Building C object tests/…/test_evs_hs_health.c.o`, `[2/2] Linking C executable`). It cannot evidence 402 targets, and "0 compiler warnings" is evidenced for one translation unit only. `build/gate7/.ninja_log` has 599 entries, so a full build very probably occurred, but its transcript was overwritten by the post-format rebuild and its loss is **not disclosed**, unlike the format-check overwrite in §2.1. This is the same claim-vs-artifact class the review chain has been correcting. | Either retain the full-build transcript or restate the row as what `build.log` shows (post-format incremental rebuild: 1 TU + 1 link, 0 warnings) and disclose the overwrite. Do not publish "402 targets" against an artifact that contains 2 build edges. |
| **F-0456-02** | **Minor** | `docs/planning/sprint-plan.md:323`; `docs/vv/verification-results.md:292-295`; `docs/vv/test-spec/ts-5-core-services.md:177`; `tests/unit/test_evs_hs_health.c:89`; `src/apps/app_tm_output.c:14-25` | **NC-EXT-236 is defined inconsistently and its headline statement is factually wrong.** The plan and the residual list say the S5 "TM sink → link forwarding leg is **not implemented/verified**"; `app_tm_output.c:14-25` *does* implement it (`ff_to_sink` → `FF_LinkSend`, installed by `FF_AppTmOutputInit`), and `test_evs_hs_health.c:138-148` installs a sink that calls `FF_LinkSend` and `:293-295` asserts `FF_LinkLoopbackPop` returns an identical frame — so the sink→link leg *is* exercised with a test sink. Conversely the script header (`:89`) says "the listener → TM sink leg is NC-EXT-236", which contradicts the plan's narrowing ("listener → sink covered by `test_r3_gap.c:520-571`"). What is actually absent is one end-to-end test of the assembled chain using the **production** `ff_to_sink`; no host test instantiates `FF_AppTmOutputInit`. | Harmonise the NC-EXT-236 definition across plan, TS-5, residual list and test header. Correct "not implemented": the production sink exists. State the real gap: "no end-to-end test exercises `FF_AppTmOutputInit`/`ff_to_sink` in the listener→S5→TM-sink→link chain". Closure criteria must be restated on that basis. |
| **F-0456-03** | **Advisory** | `tests/unit/test_evs_hs_health.c:107-119`, `:248`, `:410`; `src/core/pal/pal_posix.c:25` | **The F-0454-05 disposition is recorded and directionally sound, but its behavioural oracle is incomplete.** Avoiding `FF_PAL_MAX_TASKS` is correct — it is a file-local `#define` in `pal_posix.c`, not public PAL contract. However `fillers < 64` also passes when task creation fails immediately (`fillers == 0`), and `FF_EsStartApp(...) == FF_ERROR` does not distinguish "slots exhausted" from "task creation broken", so the fixture does not actually demonstrate exhaustion and the test would pass for the wrong reason. | Add `fillers > 0` (or assert the last `FF_PalTaskCreate` returned `FF_NO_RESOURCE`) and amend the §2 disposition to say what the oracle does and does not discriminate. Keep the private constant out of the test. |
| **F-0456-04** | **Minor** | `docs/vv/evidence/2026-11-04-re-review/README.md:45-55`; `docs/vv/evidence/2026-11-04-re-review/format-check.log`; `format-check-exit.txt` | **The §2.1 disclosure is candid but is not N11-compliant and misdescribes its own artifact.** (a) AGENTS.md N11 requires failed-cycle reports to be retained; the failing `format-check` transcript was intentionally destroyed (disclosed, but not cured). (b) §2.1 says "Only the final PASS transcript is retained in `format-check.log`", but that file contains **only the command line** — there is no PASS indication in it; the exit status lives separately in `format-check-exit.txt` (`FORMAT_EXIT=0`). (c) "No C source or header was changed to satisfy this gate" is literally contradicted by "`clang-format -i` was applied" to `test_evs_hs_health.c`; the intended meaning is "no semantic change", and the "whitespace/layout only" claim cannot be verified because no pre-format hash was kept. The final state is nevertheless format-clean (I verified `clang-format --dry-run --Werror` → RC 0 on both scripts, clang-format 18.1.3). | Retain the failing transcript (or a hash) in future cycles; reword §2.1 to "command + exit code retained; PASS output is empty by construction" and to "no semantic change; pre-format hash not retained, so token identity is unverified". |
| **F-0456-05** | **Advisory** | `docs/vv/evidence/2026-11-04-re-review/ctest.log` (mtime 08:50:57); `build/gate7/libfrick_fsw_core.a`, `build/gate7/tests/test_evs_hs_health`, `test_frozen_utc_seam`, `test_pus` (mtime 08:52:33); `tools/scripts/run_sim.sh:20-25` | **The retained CTest transcript predates the final link.** `ctest.log` was written at 08:50:57; the core archive and several test binaries were reconfigured/relinked at **08:52:33** by the smoke step (`run_sim.sh` re-runs `cmake -S … -B build/gate7` and `cmake --build`), with no retained build log for that step. The claim is nevertheless true on the current tree: `cmake --build` reports "no work to do" and my CTest re-run gives **100% tests passed, 0 failed out of 52**. | Order the gate sequence so CTest runs after the last build (or run smoke before the CTest gate), or disclose the rebuild and retain its log. |
| **F-0456-06** | **Advisory** | `docs/vv/evidence/2026-11-04-re-review/rtems-symbols.log:44` | **The §5/§6 exit-code note is accurate but easy to misread.** `exit=1` on `grep -c` (zero matches) is the desired negative result, and `exit=2` on the missing `compile_commands.json` is disclosed. The log explains both, but a reader scanning exit codes could mistake them for gate failures. The log correctly states `RTEMS_SYMBOLS_EXIT=0` is the script status. | No action required beyond the existing note; consider prefixing negative-check exit codes with `expected` for machine readers. |

No Blocker and no Major: every substantive product/test correction in the round-two scope behaves as claimed in every execution I performed, and all four round-two Majors (0454-01 / 455-01, 0454-02 / 455-03, 0454-03 / 455-02, 0454-04 / 455-06) are now genuinely corrected.

---

## 2. Independence declaration

- This review ran in **`ses_e5a62463fffefAErDhQTtZQzrL`**, a separate sub-agent session from the authoring/correction work, and from the earlier reviewers (`ses_e5a708cc8ffeFtRJHwjkLf6ymm` for 0454, `ses_e5a708cc5ffeEhtKqtfiTt3xPl` for 0455). The identifier is recorded in the header and in the project review index by this report's authoring step.
- I authored none of the reviewed content, ran none of the recorded gate campaigns, and modified **only this report**. I re-executed already-built artifacts and one containerised test suite, and recomputed hashes and symbol counts, to test the published claims.
- I am an AI analysis role: **no human credentials, no regulatory delegation, no test-witness authority, no independence certification, and no power to approve** qualification, acceptance, certification, anomaly closure, or release/flight readiness. Independence here is a recorded session boundary plus a declaration, not a formal N9 certificate.
- I read no paywalled standard in full. Governing-baseline statements are limited to the persona source register and project-controlled artifacts (N8/N9/N10/N11/N21/N22 as invoked by the project).

---

## 3. Evidence independently reproduced

All commands run from `/home/nicola/fsw/frick-fsw`; container runs use `frick-vv:local` or `frick-rtems-toolchain:local`.

| Check | Command (abridged) | Result |
| --- | --- | --- |
| Candidate manifest | `sha256sum -c docs/vv/evidence/2026-11-04-re-review/candidate-manifest.sha256` | **exit 0, 130 OK, 0 FAILED** |
| Manifest self-hash | `sha256sum …/candidate-manifest.sha256` | `08bf50ce…0bb6b` — **matches** |
| 2026-11-03 assets manifest | `sha256sum -c docs/vv/evidence/2026-11-03-gate-diagnosis/assets.sha256` | **exit 0, 29 OK, 0 FAILED** |
| Assets set completeness | compare manifest list vs directory | **29 listed / 29 present**, 0 unlisted, 0 missing, 0 duplicates; self-entry absent |
| Assets freeze order | `find … -newer assets.sha256` | **no listed file newer than the manifest** (README 09:07:35 < manifest 09:07:39 < verify log 09:07:44) |
| Assets verify transcript | content vs my re-run | 29 `: OK` lines, **identical** to my run |
| `test_evs_hs_health` | direct execution, 5 consecutive runs | `checks=108 failures=0` every run, exit 0 |
| `test_frozen_utc_seam` | direct execution, 3 consecutive runs | `checks=4325 failures=0` every run |
| Script hashes | `sha256sum` | `ba72fb91…cb9e9ef`, `8660eb77…c1ecc8` — match manifest and TS-5 |
| CTest (current tree) | `cmake --build` then `ctest --test-dir build/gate7` | build **"no work to do"**; **52/52 passed, 0 failed** |
| RTEMS ELF identity | `sha256sum build/rtems-gate7/frick_fsw` | `2c4934d5…a87b` — matches log; size 3133056 |
| RTEMS symbol counts | `/opt/rtems/6/bin/riscv-rtems6-nm` | `nm` **1450**, `-g` **1085**, `--defined-only` **1450**, `-u` 0 |
| RTEMS seam absence | `nm` and `nm -g` | **0** and **0** (`grep -ci timeTest` also 0) |
| RTEMS seam strings | `strings` | **0** |
| RTEMS positive controls | `nm \| grep " T <sym>$"` | all four present (`FF_PalGetUtcNs`, `FF_PalGetMonotonicNs`, `FF_TimeInit`, `FF_TimeGetCuc`) |
| `FF_TEST_HOOKS` in RTEMS tree | `grep -rl` | **0 files**; `compile_commands.json` genuinely absent, so §6's 0-match/exit-2 note is truthful |
| `FF_TEST_HOOKS` gating | `CMakeLists.txt:218-220` | defined only when `FF_BUILD_TESTS AND NOT FF_PLATFORM MATCHES "^rtems"` |
| 2026-11-03 RTEMS ELF (secondary) | same checks on `build/rtems/frick_fsw` | sha256 `45519bb6…dc00f`; seam 0; positive control 1 |
| Formatting of the two new scripts | `clang-format --dry-run --Werror` (18.1.3) | **RC 0** for both |

**Artifacts I read but did not re-execute:** the 2026-11-03 `ci-local`/`xw1-host`/stress logs, `verify-lean.log`, `trace.log`, `cppcheck.log`, `smoke.log`, `gsw-selfcheck.log`, and `rtems-build.log`. Their existence, mtimes, command shapes and internal result lines were inspected; their campaigns were not re-run.

---

## 4. Per-finding closure — FRICK-REVIEW-0454

| ID | Prior severity | Status | Evidence in the current tree |
| --- | --- | --- | --- |
| **F-0454-01** | Major | **RESOLVED** | `assets.sha256` has 29 entries, no self-entry, lists every other regular file in the directory (29/29 present, no extras, no duplicates). `sha256sum -c` → **exit 0, 29 OK, 0 FAILED**. The freeze order is now correct and monotonic (README 09:07:35 → manifest 09:07:39 → verify log 09:07:44), and **no listed file is newer than the manifest**. The retained `assets-sha256-verify.log` is byte-for-byte consistent with my re-run, so it is a genuine transcript of the final state. The README correction history (§ "Correction history") is **honest**: it records both original defects, states that the first correction was rejected by 0454/0455, and states the freeze→generate→verify order explicitly. Cross-checks support it: 29 = 22 previous real entries + 7 previously unbound `*-final*` files (8 `*-final*` files exist, 7 are new); the superseded-log mtimes are 07:51–07:57 as claimed; `cppcheck.txt` has exactly 163 lines; both container image digests match the README exactly. The README still makes **no** claim the artifacts do not support, except that its body hashes are correctly scoped as "2026-11-03 state, not the current candidate" (`:113-114`). |
| **F-0454-02** | Minor | **RESOLVED** | `ts-5-core-services.md:177` now records `108/108 checks PASS`, script sha256 `ba72fb91…cb9e9ef`, and the run log; the 60-check figure is explicitly labelled the superseded predecessor. Script hash matches the file; `evs-hs-health-run.log` records `checks=108 failures=0`; I re-executed the binary 5× → `108/0` each time. The gap statement was also narrowed to sink→link (see F-0456-02 for the residual wording defect, which is separate from this count/hash correction). |
| **F-0454-03** | Minor | **RESOLVED** | No live, unannotated "T3 QUALIFIED" remains outside historical review reports. `sprint-plan.md` no longer contains the two forward-looking claims; `tool-qualification.md:26` and rev 1.3 (`:253`) state **NOT FULLY QUALIFIED**. `2026-11-04-gate-reconciliation/CANDIDATE.md` opens with a `> **SUPERSEDED — do not review as the current candidate.**` banner and every stale row carries a per-row "Superseded" annotation. Residual "QUALIFIED" occurrences are: the rev 1.2 revision-history row (inherently historical, immediately superseded by rev 1.3), the annotated handover 3.59 row (inside a `[Superseded …]` annotation), and correction narratives. I inspected each; none is a live claim. |
| **F-0454-04** | Minor | **RESOLVED** | `rtems-symbols.log` now contains literal, copyable commands with `$` prompts and exit codes. Counts reproduce exactly with the RTEMS toolchain: `nm` **1450**, `nm -g` **1085**, `nm --defined-only` **1450** (the earlier host `nm` 2323/1085 mismatch was indeed a host-vs-RTEMS `nm` artefact). Seam symbols are **0 under both `nm` and `nm -g`**, seam strings **0**, and all four positive controls are present. The §6→§7 substitution is handled honestly: the missing `compile_commands.json` is disclosed, the `grep` exit 2 is explained as file-missing (not a seam finding), and the replacement check is sound (0 files contain `FF_TEST_HOOKS`; `C_DEFINES` for `frick_fsw_core` lacks it). I confirmed the file is genuinely absent and the count is genuinely 0. |
| **F-0454-05** | Advisory | **PARTIALLY RESOLVED** | The disposition is **recorded** (`README.md:43`) and **directionally defensible**: `FF_PAL_MAX_TASKS` is a file-local `#define` at `pal_posix.c:25`, absent from `pal.h`, so binding a test oracle to `32` would couple the test to a private implementation constant — refusing that is good practice. However the disposition overstates what the behavioural oracle establishes: `fillers < 64` (`test_evs_hs_health.c:248,410`) is also satisfied by `fillers == 0`, and `FF_EsStartApp(...) == FF_ERROR` does not separate "slots exhausted" from "task creation broken". The advisory is therefore addressed in principle but not fully cured → **F-0456-03** (Advisory). |

**Adjacent 0455 findings touched by the shared artifacts (spot-check only, not a full PA closure):** PA-0455-01 resolved (same 29/29 evidence); PA-0455-02 resolved (no live claim); PA-0455-03 resolved for the count/hash, residual scope wording = F-0456-02; PA-0455-04 resolved (0449 erratum appended, `:153` withdraws SRS-REQ-074 SUPPORTED); PA-0455-05 materially addressed (untracked `.py`/`.sh`/`.resc` inputs are now manifested); PA-0455-06 resolved (commands + counts); PA-0455-07 resolved (owner **Nicola**, residual list `:292-298`); PA-0455-08 resolved (handover 3.59 annotation now says **4325**); PA-O-03 wording corrected (`gsw-selfcheck` 12+9+14 = 35).

---

## 5. Per-finding closure — FRICK-REVIEW-0452 F-01…F-11 (completeness)

| 0452 ID | Status | Basis in the current tree |
| --- | --- | --- |
| F-01 (T3 overstated) | **RESOLVED** | `tool-qualification.md` rev 1.3 records NOT FULLY QUALIFIED for this candidate and binds qualification to `test_pus.c`@`6f68de14…` and `test_link_uart.c` only; candidate `test_pus.c` is `4ca0e2d0…`; the two new scripts have no N10 review. Consistent with `verification-results.md` and handover 3.61/3.62. |
| F-02 (PA-NC235-07 overstated) | **RESOLVED** | `sprint-plan.md:322`, `handover.md:1596-1598`, `verification-results.md` record PA-NC235-07 **PARTIALLY RESOLVED**; the 0451 erratum §14 item 1 explicitly withdraws the "all 7 resolved" summary and states the finding row governs. |
| F-03 (assets manifest) | **RESOLVED** | See F-0454-01: exit 0, 29/29, complete set, correct order, genuine transcript. |
| F-04 (e2e test overclaimed chain) | **RESOLVED** | `tests/unit/test_evs_hs_e2e.c` is absent; `evs_hs_health` is registered (`tests/CMakeLists.txt:231`, label `n22`); the header states what is not covered (NC-EXT-236) and the script does not claim the assembled chain. |
| F-05 (event-id identity overclaim) | **RESOLVED** | NC-EXT-237 registered (`sprint-plan.md:324`); `src/pus/pus.c` still has `(void)event_id;` and the test now asserts payload transparency, explicitly disclaiming event-id semantics. |
| F-06 (wrong requirement link) | **RESOLVED** | `traceability.yaml:695` (SRS-REQ-074) lists only `test_es_hs_lifecycle.c` and `test_link_uart.c`; `:159` (SRS-REQ-014) and `:765` (SSS-REQ-008) retain `test_evs_hs_health.c`, defensible for HS detection of a FAILED app. |
| F-07 (tautological frozen-seam block) | **RESOLVED** | `test_frozen_cuc_advance` (`test_frozen_utc_seam.c:95-118`) calls production `FF_TimeGetCuc` at 1000 s, 1010 s and 1010.25 s and asserts `fine == 16384`; no arithmetic-only block remains. |
| F-08 (handover figures) | **RESOLVED** | Handover 3.59 is annotated as superseded with **4325** checks and **52/52** CTest; rev 3.61/3.62 restate the corrected position. |
| F-09 (local duplicate constant) | **RESOLVED** | `test_evs_hs_health.c:30` includes `fsw_config.h` and uses `FF_EVS_APP_ERROR`/`FF_EVS_CRITICAL`; no local redefinition. |
| F-10 (unbounded `strstr` on raw payload) | **RESOLVED** | `decode_tm` rejects `payload_len >= payload_text_size` before copying and NUL-terminates (`:163-167`); `cap_hs_reports` runs `strstr` only over `FF_EvsEvent_t.text` (`char[96]`, `FF_EVS_TEXT_LEN`), written by `vsnprintf` in `evs.c:52`. No `strstr` on raw payload bytes. |
| F-11 (seam scope wording) | **RESOLVED** | `docs/architecture/time-and-timecode.md` rev 0.5 (`:267`) states the seam is compile-time guarded and that the default host simulator also carries dormant symbols. |

**0452 closure: 11 RESOLVED / 0 PARTIAL / 0 NOT RESOLVED.**

---

## 6. N10 assessment — `tests/unit/test_frozen_utc_seam.c`

**Script sha256:** `8660eb7755fcd34e3671b83cbbb1801f9501908327aac4f677b31540d5c1ecc8` (matches manifest).
**Execution:** 3/3 runs, `checks=4325 failures=0`, exit 0.

| N10 attribute | Assessment |
| --- | --- |
| Requirement linkage | Test-support verification for the NC-EXT-235 deterministic-time correction; it is not itself a requirement oracle and does not claim to be. Appropriate. |
| Oracle independence | Asserts the seam contract (`freeze(t)` ⇒ `FF_PalGetUtcNs() == t`) and, for the CUC claim, uses the **production** `FF_TimeGetCuc` (`:101,106,113`) rather than re-implementing the conversion. The F-07 tautology is genuinely gone. |
| Nominal coverage | Basic freeze/unfreeze; 50 cycles; whole-second and sub-second CUC advance; freeze + `FF_TimeSetUtcNs` interaction; monotonic-clock independence. |
| Out-of-bound coverage | 1000 rapid freeze/unfreeze cycles and 50-cycle monotonicity. **Limitation (not a finding):** no boundary values (freeze at 0, near `UINT64_MAX`), no re-freeze without unfreeze, no CUC fine-field wrap at a second boundary (e.g. 1010.999999 s). For a test-support seam with a narrow contract this is acceptable; it would not be sufficient as a requirement-level N22 oracle. |
| Determinism | 3/3 identical. No sleeps and no wall-clock oracle: `FF_TimeInit` sets UTC to 0 (`time.c:92`), so `after` is elapsed-since-init, not host uptime. |
| Bounded buffer handling | Not applicable — no payload parsing and no string functions on external data. |
| Tautology | None material. `after != frozen_value` and the monotonicity checks compare distinct observable values; the `after < 4000 s` bound is weak but genuine. |
| Stated scope | Accurate: compile-time-guarded `FF_TEST_HOOKS` seam, absent from RTEMS/flight; independently confirmed (0 seam symbols/strings in the RTEMS ELF; 0 `FF_TEST_HOOKS` files in the RTEMS tree). |
| **Verdict** | **PASS** (no finding; out-of-bound limitation recorded above). |

---

## 7. N10 assessment — `tests/unit/test_evs_hs_health.c`

**Script sha256:** `ba72fb913f26899ba1e53a99c9e0d97525c1924561433302f1a336eedcb9e9ef` (matches manifest and TS-5).
**Execution:** 5/5 runs, `checks=108 failures=0`, exit 0.

| N10 attribute | Assessment |
| --- | --- |
| Requirement linkage | SRS-REQ-014 / SSS-REQ-008 (HS detection of a FAILED app) and PUS-C S5 emission plumbing. The header correctly disclaims listener→sink→link as a chain and `event_id` semantics. |
| Oracle independence | Events are captured at the EVS listener (production observation point); TM frames are decoded by the CC SDS decoder; severity→subservice is checked against PUS S5 subservices 1/2/3 (`:305-344`); payload transparency is checked by `memcmp` (`:340`). No re-implementation of the encoder in the oracle. |
| Nominal coverage | INFO/WARNING/CRITICAL/unknown mapping; two app ids through the loopback link; HS detection across two monitoring passes. |
| Out-of-bound coverage (N22) | Oversized payload (`FF_PUS_MAX_TM_FRAME - 2`), `UINT16_MAX` length, `NULL` data with non-zero length, `NULL` data with zero length (`:350-372`). `FF_PusEmitEvent` rejects oversize **before** reading `data` (`pus.c:337-339`) and `FF_PusSendTm` rejects `NULL`+non-zero (`pus.c:176-179`), so the 8-byte buffer is not over-read. The `NULL`+non-zero case asserts only `!= FF_SUCCESS` rather than the exact `FF_INVALID_ARGUMENT`; explicit but less precise than the other cases. |
| Determinism | 5/5 identical; fixed 20 ms settle delays; no free-running-time oracle in assertions. |
| Bounded buffer handling | `decode_tm` checks `payload_len >= payload_text_size` before copy and NUL-terminates; `cap_hs_reports` uses `snprintf` into a 32-byte needle and `strstr` only over the NUL-terminated `FF_EvsEvent_t.text` (`char[96]`, written by `vsnprintf`). No unbounded read. |
| Tautology | The event-id bytes checked in `verify_hs_failure_tm` are the **payload** bytes, not the ignored `event_id` argument — explicitly disclosed, so a scoped payload check, not a tautology. One weak precondition: the PAL-slot fixture (`fillers < 64`) does not discriminate exhaustion from a broken create path → **F-0456-03**. |
| Stated scope | The script's own scope is accurate and conservative. However its NC-EXT-236 label (`:89`, "listener → TM sink leg") conflicts with the plan/TS-5 narrowing ("sink → link only") and with the fact that its sink does exercise `FF_LinkSend` → loopback → **F-0456-02**. |
| **Verdict** | **PASS WITH COMMENTS** (F-0456-02 scope-label inconsistency; F-0456-03 weak exhaustion precondition). |

**Assumption recorded:** the severity→subservice convention (INFO→1, WARNING→2, CRITICAL/unknown→3) was checked against `pus.c` and the test's expectations, **not** against a controlled PUS-C ICD. If the mission ICD differs, the oracle would need re-derivation.

---

## 8. Assessment of the round-three corrections as corrections

### 8.1 The 2026-11-03 package (F-0454-01)

This is now a properly built evidence package: the mutable README is frozen **before** the manifest, the manifest binds every other file exactly once, verification is run afterwards with its output retained, and the whole sequence is documented as the required order. The correction history is unusually honest: it names both original defects, states that the first correction was itself rejected, and does not erase the rejected state from the narrative. The only imprecision I found is "seven `*-final*` logs" against eight files matching `*-final*`; the arithmetic supports the statement (29 current real entries − 22 previous real entries = 7 newly bound, and 7 is the number that were missing), so I do not treat it as a defect.

### 8.2 The RTEMS transcript (F-0454-04)

The §6→§7 substitution is the honest way to handle a command that could not run: the failure is retained in place, its cause is identified (no `CMAKE_EXPORT_COMPILE_COMMANDS`), the exit code is correctly interpreted (2 = missing file, not 0 seam matches), and an equivalent check is substituted and labelled. The `total_symbols` mystery is resolved: the old figure was a *host* `nm` count applied to a RISC-V ELF; the RTEMS `nm` gives 1450, which I reproduced, together with `-g` 1085 and `--defined-only` 1450.

### 8.3 The §2.1 formatting disclosure

Adequate in candour, inadequate in process and wording (F-0456-04). It correctly refuses to present a first-pass success and explains what was overwritten. But N11 requires retention of failed-cycle reports, and the artifact it points to ("PASS transcript") is a one-line command with no PASS content; the exit code is separate and the pre-format hash was not kept, so "whitespace/layout only" is asserted, not demonstrated. I independently confirmed the final state is format-clean, so the substantive risk is low.

### 8.4 Does the manifest bind a genuine final-state transcript?

Yes for the identity chain: `candidate-manifest.sha256` (09:11:53) is newer than every listed file except `manifest-verify.log` (09:11:53.81), which is deliberately excluded, and my `sha256sum -c` reproduces 130/130. For the two transcripts discussed above, the answer is qualified: `assets-sha256-verify.log` is genuine and exact; `format-check.log` is a genuine command record but not a "PASS transcript"; `ctest.log` is a genuine 52/52 run but predates the final relink (F-0456-05).

---

## 9. What this review did NOT verify (explicit limits)

1. **No clean-room reconfigure/full rebuild from scratch.** I re-used `build/gate7`; `cmake --build` reported "no work to do", and I re-ran CTest (52/52). I did not re-execute the 402-target build, format-check across the whole tree, Lean, trace, cppcheck, smoke, gsw-selfcheck, or the RTEMS cross-build.
2. **No N18 / Renode / QEMU / HIL / target silicon / hardware.** Functional validation on the emulated PolarFire SoC remains **FAIL** (NC-EXT-198 / NC-PAL-01) and is not exercised. No timing, WCET, watchdog-window, reset-scope, radiation, power or multicore-interference claim is made.
3. **No security testing.** No fuzzing, penetration test, threat-model review or malicious-input campaign.
4. **No controlled PUS-C ICD check.** The S5 mapping and event-report content were checked against the implementation and the test's expectations only.
5. **No pre-format hash of `test_evs_hs_health.c`.** "Whitespace/layout only" is therefore unverified, though the final file is clang-format clean and its checks reproduce.
6. **No independent verification of the 2026-11-03 campaign contents.** I confirmed existence, mtimes, command shapes and result lines for the 2026-11-03 `ci-local`/`xw1-host`/stress logs, but did not re-run those campaigns.
7. **No full reconstruction proof.** The manifest binds 130 files; the dirty tree contains other modified/untracked paths (e.g. `build2/`, `build3/`, `.opencode/`, `opencode.json`, `tests/node/`) that were not reviewed.
8. **No tool qualification and no N10 review of other scripts.** T3 remains NOT FULLY QUALIFIED for this candidate; `test_pus.c`, `test_link_uart.c`, `test_es_hs_lifecycle.c` and all other scripts were not reviewed here.
9. **No standards-compliance audit.** No claim is made about DO-178C/ECSS/NASA objective satisfaction, MC/DC, independence credit or certification basis.
10. **No authority disposition.** All NC statuses are as recorded by the project; this review neither endorses nor closes them. I did not review the full PA leg (0455) or the R3/R4 legs.

---

## 10. Uncertainty and confidence

- **Facts (evidence-backed):** manifest 130/130 exit 0 and self-hash match; assets 29/29 exit 0 with correct freeze order and exact transcript; no listed asset newer than its manifest; `test_evs_hs_health` 108/0 (5 runs); `test_frozen_utc_seam` 4325/0 (3 runs); CTest 52/52 on the current tree; RTEMS ELF hash `2c4934d5…a87b`, `nm` counts 1450/1085/1450, 0 seam symbols under `nm` and `nm -g`, 0 seam strings, 4 positive controls; `FF_TEST_HOOKS` in 0 RTEMS build files; both new scripts clang-format clean; `build.log` is 3 lines; the core archive and test binaries were relinked at 08:52:33 after `ctest.log` (08:50:57); `app_tm_output.c` implements the sink→link forwarding; no test instantiates `FF_AppTmOutputInit`.
- **Assumptions:** base commit + manifest is intended to represent the tested configuration; the PUS-C S5 severity convention is the intended contract; `FF_EVS_TEXT_LEN`-bounded text is the intended event-text envelope.
- **Inferences:** the full 402-target build almost certainly happened (`.ninja_log` has 599 edges), but its transcript was overwritten, so "402 targets / 0 warnings" is an inference not supported by the bound artifact. The 08:52:33 relink was caused by the smoke step's `cmake -S/-B` + build, inferred from mtimes and `run_sim.sh:20-25`, not witnessed.
- **Alternative explanations:** the `ctest.log`/binary ordering could reflect an identical relink from unchanged sources (most likely, given `ninja: no work to do` now); it would only matter if the reconfigure changed compile definitions, which the unchanged `CMakeCache` flags and the reproducing 52/52 argue against.
- **Confidence:** **High** on all closure statuses (F-0454-01…04, F-0452 F-01…F-11) and on the reproduced counts; **High** on F-0456-01/02 (direct artifact/log/source inspection); **Medium** on F-0456-04's "whitespace only" assessment (no pre-format hash); **Low** on any operational-fitness inference, which is outside this review.

---

## 11. Authority and closure statement

This report is an independent V&V assessment. It **closes no nonconformance**, **closes no sprint**, **approves no release**, **certifies no software**, **qualifies no tool**, **accepts no residual risk**, and **does not authorize any operation, test, commit, or push**. It records findings and recommendations only. Every closure, waiver, deviation, acceptance and release decision requires qualified human authority and the approvals defined by the governing baseline.

The reviewer modified no tracked content other than this report. No commit or push was performed.

**Verdict: PASS WITH COMMENTS.**

- Findings: **0 Blocker, 0 Major, 3 Minor, 3 Advisory (6 total)**.
- Closure of FRICK-REVIEW-0454: **4 RESOLVED, 1 PARTIALLY RESOLVED** (F-0454-05).
- Closure of FRICK-REVIEW-0452: **11 RESOLVED, 0 NOT RESOLVED**.
- N10: `test_frozen_utc_seam.c` **PASS**; `test_evs_hs_health.c` **PASS WITH COMMENTS**.

---

### Document control

| Item | Value |
| --- | --- |
| Report ID | FRICK-REVIEW-0456 |
| Supersedes | none (round-three delta re-review of FRICK-REVIEW-0454; re-confirms FRICK-REVIEW-0452) |
| Related | FRICK-REVIEW-0452 (V&V), FRICK-REVIEW-0453 (PA), FRICK-REVIEW-0454 (V&V), FRICK-REVIEW-0455 (PA), FRICK-REVIEW-0457 (PA sibling, if produced) |
| Prepared | 2026-11-04 |
| Reviewer | Aeronautics/space SW V&V persona (AI), session `ses_e5a62463fffefAErDhQTtZQzrL` |
| Approval | **Not approved.** No human/authority signature. |
