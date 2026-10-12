# Review Report

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0152                      |
| Reviewer session                  | independent agent (norm N9), fresh context |
| Reviewer profile                  | **R1+R2+R3+R4+PA (confirmation)** — R1 (Senior FSW / real-time / space software developer) + R2 (Senior SW V&V engineer) + R3 (Senior avionics / systems engineer, N14) + R4 (Senior SW documentation engineer, N17) + PA (Senior SW Product Assurance engineer, N20) |
| Reference revision                | `3822f72` (HEAD) + **uncommitted** Sprint-6b-1 working tree (post-FRICK-REVIEW-0151 corrections) |
| Items under review                | The Sprint-6b-1 increment corrections for FRICK-REVIEW-0151: `docs/vv/verification-results.md` (0.39 row + History), `docs/planning/sprint-plan.md` (0.11 history + §3), `docs/vv/tool-qualification.md` (0.3.22, T10), `docs/architecture/network-stack.md` (0.5 §4), `tools/scripts/ci_rtems_cross_build.sh`, plus the whole touched doc set and the FRICK-REVIEW-0151 NCs NC-151-01…07 |
| Date                              | 2026-10-12                             |
| Scope                             | Short **combined R1+R2+R3+R4+PA confirmation review** (norm N12) of the Sprint-6b-1 increment after the FRICK-REVIEW-0151 findings were corrected: confirm NC-151-01 (Major, residual false evidence wording) is genuinely closed and that the corrected statement is accurate; record closure for NC-151-02…05 (and 06/07); re-run the gates; verify scope conformance. ECSS-E-ST-40C Rev.1 (SW development / V&V), ECSS-Q-ST-80C Rev.2 (SW product assurance), project norms N1–N20 (`docs/rules.md`), reviewer contract `docs/engineering/independent-review.md` (rev 0.7) |
| Independence declaration          | The reviewer had no part in authoring, building-for-delivery, fixing, or previously reviewing the items under review; it is a different agent/session from the FRICK-REVIEW-0150 and FRICK-REVIEW-0151 reviewers. This review ran in a fresh context; every declared claim was re-executed or read first-hand (see §5). No author context was inherited and no clarifications were taken from the author. |
| Norms applied                    | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

**In scope (verified first-hand):**

- **Closure of FRICK-REVIEW-0151 NC-151-01 (Major)** — repository-wide grep
  (excluding `docs/reviews/`) for the residual false evidence wording
  ("builds and links", "link-checked", "build/link PASS", "links with lwIP"),
  and independent confirmation that the **corrected** statement is accurate:
  the plain RTEMS image contains no lwIP symbols; a forced-extraction link
  resolves the stack; the host suite is 23/23 with `FF_USE_LWIP=ON`.
- **Record CLOSED/OPEN for NC-151-02…05 and NC-151-06/07** with first-hand
  evidence.
- **Gate re-run** — `ctest --test-dir build/native` (expected 22/22),
  `tools/scripts/trace.py` (expected PASS 114/71/29/14), `metrics.py --check`,
  and header-vs-last-`## History` for every touched document (N16).
- **Scope conformance** (standing rule) and residual/new findings.

**Explicitly excluded (recorded so the scope is bounded):**

- On-target lwIP behaviour (netif/PHY binding, ISR input, TCP correctness,
  timing, stack high-water): not runnable in this increment; registered as
  **GAP-A5** / **NC-S6-01**/**NC-S6-02** (unchanged).
- The lwIP third-party sources themselves (`third_party/lwip`, fetched, pinned,
  BSD-3): reviewed only as the API contract.
- `verification/lean` (`lake build`) not re-run: the increment adds no
  Lean-modelled artifact (`lean: []` + `lean_note` for SRS-REQ-040, norm N1
  interpretation note 1).
- The SSS (`docs/requirements/SSS.md`) was not modified (N14 trigger inactive).
- The FRICK-REVIEW-0150/0151 reports themselves (they are references / the
  closure vehicle, not items to edit).

## 2. References

- Project norms: `docs/rules.md` (N1–N20); reviewer contract and profiles:
  `docs/engineering/independent-review.md` (rev 0.7); `AGENTS.md`, `README.md`.
- Prior reviews: `docs/reviews/2026-10-12-m4-m11-lwip-rtems-s6b1-review.md`
  (FRICK-REVIEW-0150), `docs/reviews/2026-10-12-m4-m11-lwip-rtems-s6b1-delta-review.md`
  (FRICK-REVIEW-0151); `docs/reviews/2026-10-12-m4-m11-tsn-s6a-review.md`
  (FRICK-REVIEW-0148) and its delta (FRICK-REVIEW-0149); review log
  `docs/reviews/README.md`; template `docs/reviews/review-template.md`.
- Traceability manifest/gate: `docs/requirements/traceability.yaml`,
  `tools/scripts/trace.py` (N4); planning set `docs/development-plan.md`,
  `docs/planning/sprint-plan.md`.
- V&V tooling: `docs/vv/tool-qualification.md` (norm N10),
  `docs/vv/verification-results.md`, `tools/scripts/ci_rtems_cross_build.sh`,
  `.github/workflows/{ci,nightly}.yml`, `.gitea/workflows/ci.yml`.
- ECSS-E-ST-40C (30 Apr 2025), ECSS-Q-ST-80C (30 Apr 2025);
  ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2 (context only).
- lwIP `STABLE-2_2_1_RELEASE` (pinned); RTEMS 6.2 classic API
  (`frick-rtems-toolchain:local`, `90f81d080611`).

## 3. Findings and closure table (FRICK-REVIEW-0151 NCs)

| NC (FRICK-REVIEW-0151) | Severity | Status now (FRICK-REVIEW-0152) | First-hand evidence |
| ---------------------- | -------- | ------------------------------ | ------------------- |
| **NC-151-01** — residual false evidence wording in two History rows (R2-01/PA-01 incomplete) | **Major** | **CLOSED** | `verification-results.md:445` and `sprint-plan.md:118` are reworded to the verified fact ("RTEMS build produces the lwIP archive; forced-extraction link resolves the stack; the image does not reference the adapter yet"). Repo-wide grep (excluding `docs/reviews/`) for `link-checked` / `builds and links` / `builds+links` / `build/link PASS` / `links with lwIP` → **0 matches** (E1). Corrected statement independently substantiated: plain image **0/1390** lwIP symbols; archive defines `tcpip_init`/`lwip_socket`/all probed `sys_*`; forced-extraction link **OK**, all probed symbols resolved (115 lwIP symbols in the forced image; `.text` 124 396 → 197 520 B); `FF_USE_LWIP=ON` `.text` **cmp-identical** to `FF_USE_LWIP=OFF` (E5–E8); host **23/23** with `FF_USE_LWIP=ON` (E2). |
| **NC-151-02** — RTEMS+lwIP not gated; N10 re-qualification missing; inaccurate script comment | Minor | **PARTIAL — OPEN (residual)** | Comment corrected and the `FF_USE_LWIP=ON` cross-build block retained (`ci_rtems_cross_build.sh:34-46`, E9); T10 re-qualified (`tool-qualification.md` rev 0.3.22 + History row, E10). **Residual:** no CI job reaches the new block — the nightly job configures `-DFF_PLATFORM=rtems` **without** `-DFF_USE_LWIP=ON` and does not call the script (`.github/workflows/nightly.yml:20-40`), the per-commit job SKIPs (toolchain absent) before the block (`.github/workflows/ci.yml:263-271`; `.gitea/workflows/ci.yml:302-318`), and the host lwIP job is host-only (`ci.yml:66`); **no `-Wl,--undefined` link check** was added; the S6b-2 deferral is **not tracked** (see **NF-1**). |
| **NC-151-03** — `network-stack.md` §4/§7 incomplete | Minor | **CLOSED (substance)** | `network-stack.md` §4 now states the RTEMS object prerequisites (`CONFIGURE_MAXIMUM_MESSAGE_QUEUES`, NC-S6-02) and the "archived but **not linked** into the default image" statement (E11); History 0.5 = header 0.5. Residual: §7 (Gaps) was not cross-referenced (advisory, NF-5). |
| **NC-151-04** — "`-Wall -Wextra -Werror` clean on the RTEMS target" over-stated | Minor | **CLOSED** | `verification-results.md:51` now reads "new port files `-Wall -Wextra` clean on the RTEMS target (third-party lwIP stays non-`-Werror`, unchanged policy)" (E11). No "-Werror clean on the RTEMS target" claim remains anywhere in `docs/` (E12). |
| **NC-151-05** — `sys_thread_new` silent failure open and untracked (R1-01) | Minor | **CLOSED** | The port code is unchanged (`sys_arch.c:468-489` still returns `{RTEMS_ID_NONE}` silently), but the review allowed *fix-or-track*; the disposition is now **tracked with owner + sprint** in `sprint-plan.md` §3:75 (`NC-151-05 | FRICK-REVIEW-0151 | … Owner: frick-fsw authoring agent … S6b-2`, E13). Code fix deferred to S6b-2 as recorded. |
| **NC-151-06** — review-log reason inaccurate | Comment | **CLOSED (moot/accurate)** | `docs/reviews/README.md:149` reason reads "APPROVED WITH COMMENTS (R2-01 evidence wording corrected)"; with NC-151-01 now closed the wording **is** corrected, so the reason is substantively accurate. The suggested "(…in the delta — FRICK-REVIEW-0151)" clarification was not added (advisory). |
| **NC-151-07** — FRICK-REVIEW-0148 follow-up table not updated (R4-02 residue) | Comment → Minor | **CLOSED (substance)** | NC-S6-01 (narrowed with root cause), NC-S6-02 and NC-S7-01 are recorded with owner + sprint in `sprint-plan.md` §3:72-74 and in the FRICK-REVIEW-0151 follow-up table (E13). The `2026-10-12-m4-m11-tsn-s6a-review.md` / `…-delta-review.md` tables themselves were **not** amended (advisory; see NF-2 note). |

### New findings

| # | Severity | Profile | Item | Finding | Required action |
| - | -------- | ------- | ---- | ------- | --------------- |
| **NF-1** | **Minor** | R2/PA/N19 | `sprint-plan.md` §3 / CI / `ci_rtems_cross_build.sh` | NC-151-02's remaining part (CI gating + `-Wl,--undefined` link check) is deferred to S6b-2 but is **not tracked** with owner/milestone, although FRICK-REVIEW-0151 required "if not done now, track it to S6b-2 with owner" (N12/N19). | Complete it now, or add a `sprint-plan.md` §3 row (owner + S6b-2), consistent with NC-S6-02/NC-151-05. |
| NF-2 | Comment | R4/PA (N11) | `docs/reviews/README.md` | The review log has **no row for FRICK-REVIEW-0149** (`2026-10-12-m4-m11-tsn-s6a-delta-review.md`), which is a committed report (added in `3822f72` alongside FRICK-REVIEW-0148); norm N11 requires every review result to be logged. Pre-existing, but the log is touched this cycle. | Add the 0149 row with the 0151/0152 rows at commit. |
| NF-3 | Comment | PA (N11/N20) | `docs/reviews/README.md` | Log rows for FRICK-REVIEW-0151 and this FRICK-REVIEW-0152 are not yet present (reviewer is read-only and writes exactly one file). | Author adds both rows with this cycle's commit; refresh the status panel/semaphores. |
| NF-4 | Comment | R3/PA | `docs/status/status.json` | Semaphores remain all `green` and `deferred: []` while NC-151-02/NC-S6-01/NC-S6-02/NC-S7-01 are open (recurrence of R3-02/PA-03/PA-02). | Consider recording the deferrals (owner/milestone) so the panel does not read "nothing deferred". |
| NF-5 | Comment | R4 (N8) | `docs/architecture/network-stack.md` §7 | §7 (Gaps) was not cross-referenced to the §4 "not linked until S6b-2"/object-sizing statements as NC-151-03 literally requested. | Optional: add a pointer in §7. |

**Severity definition** (from `docs/reviews/review-template.md`): **Major** — a
non-conformity that disqualifies the item for the applicable criticality, or a
defect that can defeat a requirement or a safety barrier. **Minor** — a
non-conformity that does not invalidate the requirement but must be corrected or
justified. **Comment** — observation/recommendation, no derogation.

**No Major finding is open.** NC-151-01 (the sole Major) is closed with direct
file evidence and an independently reproduced link check. NC-151-02 remains a
**Minor** (records/tracking), and all other items are Comments.

## 4. Scope conformance (standing rule)

**Verdict: conformant — no scope-deviation NC raised.**

- The **increment** implements exactly what it declares: four new files under
  `src/hal/net/lwip/rtems/` (the `sys_arch` port, `arch/cc.h`,
  `arch/sys_arch.h`, `lwipopts.h`) and the `if(FF_USE_LWIP)` block of
  `CMakeLists.txt` selecting the RTEMS port only for `FF_PLATFORM=rtems`.
  `FF_USE_LWIP` stays `OFF` by default; the adapter is not referenced by the
  RTEMS application (0 lwIP symbols in the image; `FF_USE_LWIP=ON` `.text` is
  byte-identical to `FF_USE_LWIP=OFF`, E7); no netif/ISR work (S6b-2) is
  preceded.
- The **delta** implements exactly the mandated corrections plus nothing else:
  the two History-row rewords, the `network-stack.md` §4 content, the
  `verification-results.md` `-Wall -Wextra` wording, the
  `ci_rtems_cross_build.sh` comment + `FF_USE_LWIP=ON` block, the
  `tool-qualification.md` T10 re-qualification, the `sprint-plan.md` §3
  NC-151-05 tracking, and the `docs/reviews/README.md` log row. No scope creep.
- The only conformance defects found are **evidential/records**, not scope:
  the untracked NC-151-02 residual (NF-1) and the review-log rows (NF-2/NF-3).

## 5. Evidence ledger (all re-executed/obtained first-hand on 2026-10-12)

| # | Command / artefact | Observed result |
| - | ------------------ | --------------- |
| E1 | repo-wide grep (excl. `docs/reviews/`) for `link-checked\|builds and links\|builds\+links\|build/link PASS\|links with lwIP` | **0 matches**; the only near-hit (a "build/link is fine" cell in `sprint-plan.md:74`) is the accurate build-vs-runtime distinction |
| E2 | `cmake -S . -B build/r152-lwip -DFF_USE_LWIP=ON -DCMAKE_BUILD_TYPE=Debug && cmake --build && ctest` | build exit 0; **23/23 PASS** (incl. test 23 `lwip`) |
| E3 | `ctest --test-dir build/native` | **22/22 PASS**, 0 failures |
| E4 | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS — 114 requirements (71 srs / 29 sss / 14 gap), 51 Lean theorems checked** |
| E5 | `python3 tools/scripts/metrics.py --check` | `metrics panel: current` (exit 0) |
| E6 | fresh RTEMS cross build in `frick-rtems-toolchain:local` (`-DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-mpfs.cmake -DFF_PLATFORM=rtems -DFF_USE_LWIP=ON`) | **PASS**; `frick_lwip` = **85 objects** incl. `…/rtems/sys_arch.c.obj` |
| E7 | `riscv-rtems6-nm build/r152-rtems-lwip/frick_fsw` / `libfrick_lwip.a`; forced-extraction link (`-Wl,--undefined=FF_NetStackLwip -Wl,--undefined=tcpip_init -Wl,--undefined=sys_arch_mbox_fetch`) + `objcopy --only-section=.text` + `cmp` | plain image: **0** lwIP symbols of **1390**; archive defines `tcpip_init`/`lwip_socket`/`sys_thread_new`/`sys_arch_mbox_fetch`/`sys_mbox_trypost_fromisr`/`sys_now`/`sys_jiffies`; forced link **OK** (115 lwIP symbols; all probed resolved); `.text` 124396 → 197520 B; `FF_USE_LWIP=ON` `.text` **cmp-identical** to `FF_USE_LWIP=OFF` |
| E8 | host port identity: `find build/r152-lwip/CMakeFiles/frick_lwip.dir -name "*.o"` | host uses the upstream **unix** port (`contrib/ports/unix/port/sys_arch.c.o`, `perf.c.o`); no RTEMS-port object in the host tree |
| E9 | `tools/scripts/ci_rtems_cross_build.sh:34-46`; `.github/workflows/nightly.yml:20-40`; `.github/workflows/ci.yml:263-271`; `.gitea/workflows/ci.yml:302-318`; `ci.yml:66` | the `FF_USE_LWIP=ON` cross-build block is present and works locally; **no CI job reaches it** (nightly omits `-DFF_USE_LWIP=ON` and does not call the script; per-commit SKIPs on the absent toolchain; the host lwIP job is host-only); no link check added |
| E10 | `docs/vv/tool-qualification.md` | header 0.3.22 = last History 0.3.22; row 0.3.22 re-qualifies T10 `ci:rtems-cross-build` |
| E11 | `git diff` of `network-stack.md` / `verification-results.md` | `network-stack.md` §4 gains the RTEMS prerequisites + "archived but not linked until S6b-2" (header 0.5 = History 0.5); `verification-results.md:51` reads `-Wall -Wextra` (not `-Werror`), History 0.39 = header 0.39 |
| E12 | `grep -rn "\-Werror" docs/ README.md` | no "-Werror clean on the RTEMS target" claim remains; only the accurate "lwIP sources are not compiled with the project's `-Werror`" (`network-stack.md:52`) |
| E13 | `sprint-plan.md` §3 rows 72-75 | NC-S6-01 (narrowed), NC-S7-01, NC-S6-02 and NC-151-05 recorded with owner + sprint (NC-151-05 → S6b-2); **no NC-151-02 row** |
| E14 | header vs last `## History` row, every touched document | **MATCH** for all: `network-stack` 0.5, `development-plan` 0.24, `sprint-plan` 0.11, `traceability.md` 0.34, `handover` 1.14, `loop-audit` 0.18, `renode-environment` 0.18, `verification-results` 0.39, `tool-qualification` 0.3.22 (also `rules.md` 1.4, `independent-review.md` 0.7) |
| E15 | `docs/reviews/README.md` (committed at `3822f72` and working tree) | FRICK-REVIEW-0148 row present; **no FRICK-REVIEW-0149 row**; FRICK-REVIEW-0150 row present (working tree) |
| E16 | `docs/reviews/2026-10-12-m4-m11-tsn-s6a-review.md` / `…-delta-review.md`; `git status --porcelain` | both **unchanged** (not amended); `grep -c "NC-S6-02\|NC-S7-01"` = **0** in each |
| E17 | `docs/status/status.json` | semaphores all `green`; `deferred: []` |
| E18 | SPDX + `clang-format --dry-run --Werror` on the four port files | all four carry `SPDX-License-Identifier: AGPL-3.0-only`; clang-format clean |

## 6. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**Rationale.** The FRICK-REVIEW-0151 **Major NC-151-01 is genuinely closed**:
both false History rows are reworded, a repository-wide scan finds no residual
false evidence wording, and the corrected statement is independently accurate
(the RTEMS image contains no lwIP code — 0 of 1390 symbols — while the archive
is complete and a forced-extraction link resolves every probed symbol; the host
suite is 23/23 with `FF_USE_LWIP=ON`; the default image is byte-identical). The
`-Werror` wording (NC-151-04), the `network-stack.md` content (NC-151-03), the
N10 re-qualification of T10 and the tracking of R1-01 (NC-151-05) are corrected;
NC-151-06/07 are closed substantively. The gates reproduce green — `ctest`
**22/22**, `trace` **PASS 114/71/29/14**, `metrics --check` current, and every
touched document's header equals its last History row. No new Major was raised,
so the REJECTED verdict is cleared to **APPROVED WITH COMMENTS**.

**The loop is closed for the Major (commit no longer blocked by N12).** Commit/push
is permitted once the author folds in two trivial **records** actions in the same
commit: (1) **NF-1** — complete NC-151-02's CI-gating/link-check part or track it
to S6b-2 with an owner in `sprint-plan.md` §3; (2) **NF-2/NF-3** — add the
`docs/reviews/README.md` rows for FRICK-REVIEW-0149/0151/0152 and refresh the
status panel. These are Minors/Comments (no derogation); the pre-existing
**NC-S6-01 (narrowed)**, **NC-S6-02** and **NC-S7-01** remain open/scheduled
(S6b-2 / S7); **GAP-A5** remains for human acceptance.

## 7. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9) | Senior FSW / real-time / space software developer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — port correct/complete/allocation-free; corrected evidence statement verified; NF-1 (NC-151-02 CI/link residual) |
| R2 (N9) | Senior SW V&V engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — Major cleared: 0 lwIP symbols in the plain image, forced link resolves the stack, host 23/23, 22/22, trace PASS; NF-1 (ungated RTEMS+lwIP config) |
| R3 (N14) | Senior avionics / systems engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — manifest/mirror/counts accurate (114/71/29/14); NF-4 (panel/deferred) |
| R4 (N17) | Senior SW documentation engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — headers = History in 9/9 touched docs; wording corrected; NF-2/NF-3 (log rows), NF-5 (§7 pointer) |
| PA (N20) | Senior SW Product Assurance engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — scope conformance verified (no scope NC); NC-151-01 closed; evidence integrity restored; NF-1 (tracking) remains; on-target behaviour stays with the human authority (GAP-A5, NC-S6-02) |

## 8. Follow-up records

Non-conformances and their dispositions (norm N12: NCs are resolved by
re-applying **all** rules to the affected scope and re-reviewing the deltas until
full compliance).

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| **NC-151-01** | Major → closed | `verification-results.md:445`, `sprint-plan.md:118` | Residual false "builds+links"/"build/link PASS" wording | Both rows reworded to the verified fact | E1 (0 matches), E6–E8 | frick-fsw authoring agent | **CLOSED** | FRICK-REVIEW-0152 (this report) |
| NC-151-02 | Minor | `ci_rtems_cross_build.sh`; CI; `tool-qualification.md` | Ungated RTEMS+lwIP config; no link check; N10 re-qual missing | Comment fixed; block retained; T10 re-qualified; **CI gating + link check deferred, untracked** | E9 (ungated), E10 (T10) → **NF-1** | frick-fsw authoring agent | **OPEN (residual)** | this report |
| NC-151-03 | Minor | `network-stack.md` §4/§7 | Missing prerequisites + "not linked yet" | §4 content added; §7 not cross-referenced | E11 (header=history); NF-5 | frick-fsw authoring agent | **CLOSED** (NF-5 advisory) | this report |
| NC-151-04 | Minor | `verification-results.md:51` | `-Werror` claim not the build config | Reworded to `-Wall -Wextra` | E11, E12 | frick-fsw authoring agent | **CLOSED** | this report |
| NC-151-05 | Minor | `sys_arch.c`; plan records | R1-01 open + untracked | Disposition recorded (owner + S6b-2) | E13 (`sprint-plan.md:75`) | frick-fsw authoring agent | **CLOSED** | this report |
| NC-151-06 | Comment | `docs/reviews/README.md:149` | Log reason inaccurate | Now substantively accurate (NC-151-01 closed) | E15 | frick-fsw authoring agent | **CLOSED** (advisory) | this report |
| NC-151-07 | Comment → Minor | FRICK-REVIEW-0148 follow-up table | NC narrowing/new NCs not recorded | Recorded in §3 + FRICK-REVIEW-0151 table; 0148/0149 tables unamended | E13, E16 | frick-fsw authoring agent | **CLOSED** (advisory; NF-2) | this report |
| **NF-1** | Minor | `sprint-plan.md` §3; CI; script | NC-151-02 residual untracked | Complete or track (owner + S6b-2) | E9, E13 | frick-fsw authoring agent | **OPEN** | this report / S6b-2 |
| NF-2, NF-3 | Comment | `docs/reviews/README.md` | Missing log rows (0149; 0151/0152) | Add at commit (N11) | E15 | frick-fsw authoring agent | **OPEN (advisory)** | this report |
| NF-4, NF-5 | Comment | `status.json`; `network-stack.md` §7 | Panel/deferred; §7 pointer | Consider/record | E17, E11 | frick-fsw authoring agent | **OPEN (advisory)** | this report |
| **NC-S6-01** | Minor (scope) | SRS-REQ-040 / S6b-2 | NET lwIP-on-GEM peripheral-bound deferred | Unchanged; host-tested adapter | `sprint-plan.md` §2/§3; `traceability.yaml` note | frick-fsw authoring agent | **open (scheduled S6b-2)** | S6b-2 |
| **NC-S6-02** | Minor | `config/rtems/rtems_config.c` | No `CONFIGURE_MAXIMUM_MESSAGE_QUEUES` → runtime `tcpip_init` fails | Unchanged; correctly scoped | `sprint-plan.md` §3:74 | frick-fsw authoring agent | **open (scheduled S6b-2)** | S6b-2 |
| **NC-S7-01** | Minor (scope) | OBJ-19 / SRS-REQ-032 (SpaceWire) | No SpaceWire peripheral in the Renode model | Unchanged; contract-level check in S7 | `sprint-plan.md` §2/§3:73 | frick-fsw authoring agent | **open (scheduled S7)** | S7 |

## 9. Norm-by-norm audit (confirmation)

| N | Assessment |
| - | ---------- |
| N1 | **OK with justification** — no Lean-modelled artifact added; SRS-REQ-040 keeps `lean: []` + explicit `lean_note` (no silent waiver) |
| N2 | **OK** — all changed content is English |
| N3 | **OK** — SRS/SSS unchanged; SRS-REQ-040 text still accurate |
| N4 | **OK** — `trace.py` PASS (114/71/29/14); manifest lists the new port paths |
| N5 | **OK** — GAP-A5 kept open; no level-A claim |
| N6 | **Gap** — the RTEMS+lwIP configuration is still not exercised by any deployed CI job (NF-1) |
| N7 | **n/a (reviewer)** — nothing committed/pushed by this review; the author pushes both remotes after the records actions |
| N8 | **OK with advisories** — wording corrected; NF-2/NF-5 remain |
| N9 | **OK** — fresh, independent context over the exact revision |
| N10 | **OK (partial)** — T10 re-qualified (0.3.22); the link check the register now implies is not yet gated (NF-1) |
| N11 | **Gap** — review-log rows for 0149/0151/0152 pending (NF-2/NF-3) |
| N12 | **OK (Major cleared)** — NC-151-01 closed; NC-151-02 residual + NF-1 tracked for the same commit / S6b-2 |
| N13 | **OK** — AGPL-3.0-only SPDX on all four port files |
| N14 | **Not triggered / satisfied** — SSS unmodified |
| N15 | **Satisfied for the re-run gates** — 22/22, 23/23, trace PASS, metrics current; RTEMS+lwIP cross build PASS; Lean n/a |
| N16 | **OK** — header = last History row in 9/9 touched documents |
| N17 | **Required/partial** — documentation deltas corrected; log rows pending (NF-2/NF-3) |
| N18 | **Satisfied (unchanged)** — Renode image byte-identical (`FF_USE_LWIP` OFF by default) |
| N19 | **Gap** — NC-151-02 residual not tracked with owner/milestone (NF-1) |
| N20 | **Discharged** — PA reviewed the phase products in independent context; the Major is closed and no new Major is open |

---

## Facts, Assumptions, Inferences, Recommendations, Confidence, Open gaps

**Facts (verified first-hand at `3822f72` + working tree, 2026-10-12).**

1. **NC-151-01 is closed:** both History rows are reworded; a repo-wide grep
   (excluding `docs/reviews/`) for the false phrases returns **0** matches.
2. **The corrected statement is accurate:** the plain RTEMS image has **0**
   lwIP symbols of 1390; `libfrick_lwip.a` defines the stack entry points; the
   forced-extraction link succeeds (all probed symbols resolved; `.text`
   124396 → 197520 B); `FF_USE_LWIP=ON` `.text` is `cmp`-identical to
   `FF_USE_LWIP=OFF`.
3. Host `FF_USE_LWIP=ON` build: **23/23 PASS**; host uses the upstream unix port.
4. Default `ctest`: **22/22 PASS**; `trace.py`: **PASS 114/71/29/14** (51
   theorems); `metrics --check`: current.
5. Every touched document's header equals its last `## History` row (9/9).
6. `tool-qualification.md` (0.3.22) re-qualifies T10; the `FF_USE_LWIP=ON`
   cross-build block is present and works locally.
7. **No CI job reaches the RTEMS+lwIP block** (nightly omits `-DFF_USE_LWIP=ON`
   and does not call the script; per-commit SKIPs; the host lwIP job is
   host-only); no link check was added; the S6b-2 deferral is untracked (NF-1).
8. `network-stack.md` §4 carries the prerequisites + "not linked until S6b-2";
   §7 was not cross-referenced (NF-5).
9. `sprint-plan.md` §3:75 tracks NC-151-05 (owner + S6b-2), closing the R1-01
   tracking requirement.
10. `docs/reviews/README.md` has the 0148 and 0150 rows but **no 0149 row**;
    0151/0152 rows are pending (NF-2/NF-3).
11. No file other than this report was modified, created or deleted by the
    reviewer; nothing was committed or pushed.

**Assumptions.**

1. The pinned `third_party/lwip` tree (`STABLE-2_2_1_RELEASE`) is the stack a
   mission would use; its `sys.h`/`tcpip.c` are the contract the port satisfies.
2. `frick-rtems-toolchain:local` (`90f81d080611`) is the project's pinned RTEMS
   6.2/mpfs64imafdc toolchain; results hold for it.
3. The reviewer's out-of-band forced-extraction link is acceptable objective
   evidence for *linkability*; it is not produced by a project gate (NF-1).

**Inferences.**

1. NC-151-02 was addressed at the script/register level; the CI-reachability
   part was left for S6b-2 (allowed by FRICK-REVIEW-0151) but the required
   owner/milestone tracking was not added (NF-1).
2. The missing FRICK-REVIEW-0149 log row is a pre-existing N11 records gap from
   the S6a commit, surfaced now because the log is touched this cycle (NF-2).

**Recommendations.**

1. Fold the two records actions (NF-1 tracking, NF-2/NF-3 log rows) into the
   S6b-1 commit; no code change is required for the Major, which is closed.
2. Keep GAP-A5 and NC-S6-02 open; size `CONFIGURE_MAXIMUM_MESSAGE_QUEUES` (plus
   semaphore/task headroom) with the real netconn/socket budget before the first
   on-target `tcpip_init`; wire the RTEMS+lwIP configuration into a
   toolchain-present job and add the `-Wl,--undefined` link check in S6b-2.

**Confidence.**

- **High** for NC-151-01 closure and the accuracy of the corrected statement
  (direct file evidence + fresh RTEMS build, `nm`, forced link and `.text`
  comparison).
- **High** for the gate results and the header/History check.
- **Medium-High** for the CI-reachability analysis (NF-1): it rests on reading
  the three workflow files and the script; a runner with both a preinstalled
  toolchain and the fetched lwIP tree would change the outcome.
- Confidence is not a probability of safety.

**Open gaps** (not closed by this review).

- **NC-151-02 (Minor residual)** — CI gating + link check deferred and untracked
  (NF-1).
- **NF-2/NF-3 (Comment)** — review-log rows pending; **NF-4/NF-5** advisory.
- **NC-S6-01 (narrowed)** open (S6b-2); **NC-S6-02** open (S6b-2);
  **NC-S7-01** open (S7).
- **GAP-A5** (on-target lwIP behaviour) — no on-target run, no netif/PHY, no
  timing/stack-high-water; the stack cannot start on target yet (NC-S6-02).
  **For human acceptance.**
- **Not verifiable by the reviewer:** the author's intent behind the residual
  tracking omission; any evidence held outside the repository. Paywalled/absent
  sources relied upon: none.

---

*This report is authored to be committed and pushed with the reviewed change
(norm N11). The review was read-only with respect to repository content: the
reviewer wrote exactly one file (this report); **no other file was modified,
created or deleted**, and nothing was committed or pushed. The gate commands
regenerated only git-ignored `build/` artifacts (`build/{native,r152-lwip,r152-rtems-lwip,r152-rtems-off}`).
Independence is declared per norm N9 (fresh context; no part in authoring,
fixing, building-for-delivery or previously reviewing the items under review).*

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-12 | independent reviewer (FRICK-REVIEW-0152) | Combined R1+R2+R3+R4+PA confirmation review of the Sprint-6b-1 increment after the FRICK-REVIEW-0151 corrections at `3822f72` + working tree: **APPROVED WITH COMMENTS** — NC-151-01 (Major) CLOSED with first-hand evidence (plain image 0/1390 lwIP symbols; forced link resolves the stack; host 23/23); NC-151-03/04/05/06/07 CLOSED; NC-151-02 OPEN (residual: CI gating + link check untracked, NF-1); new NF-2..NF-5 (records/advisory); loop closed for the Major, commit permitted after the records actions |
