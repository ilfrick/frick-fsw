# Review Report

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0151                      |
| Reviewer session                  | independent agent (norm N9), fresh context |
| Reviewer profile                  | **R1+R2+R3+R4+PA (delta)** — R1 (Senior FSW / real-time / space software developer) + R2 (Senior SW V&V engineer) + R3 (Senior avionics / systems engineer, N14) + R4 (Senior SW documentation engineer, N17) + PA (Senior SW Product Assurance engineer, N20) |
| Reference revision                | `3822f72` (HEAD) + **uncommitted** Sprint-6b-1 working tree (FRICK-REVIEW-0150 delta) |
| Items under review                | The pre-commit delta of FRICK-REVIEW-0150: `docs/vv/verification-results.md` (0.39), `CHANGELOG.md`, `docs/requirements/traceability.yaml` (SRS-REQ-040 note), `docs/requirements/traceability.md` (0.34), `README.md`, `docs/status/{handover.md (1.14),loop-audit.md (0.18),status.json}`, `docs/architecture/network-stack.md` (0.5), `docs/development-plan.md` (0.24), `docs/planning/sprint-plan.md` (0.11), `docs/vv/renode-environment.md` (0.18), `tools/scripts/ci_rtems_cross_build.sh`, `docs/reviews/README.md`; the dispositions of R2-01/PA-01, R1-01, R2-02, R3-01, R4-01, R4-02, PA-02 |
| Date                              | 2026-10-12                             |
| Scope                             | Independent **delta re-review** (norm N12: re-apply all rules to the affected scope, re-review the deltas until full compliance) of the Sprint-6b-1 lwIP RTEMS `sys_arch` increment after the FRICK-REVIEW-0150 findings were corrected. Verify the mandatory evidence-wording correction (R2-01/PA-01) is genuinely closed and residual-free, record closure for R1-01/R2-02/R3-01/R4-01/R4-02/PA-02, re-run every gate, and confirm scope conformance. ECSS-E-ST-40C Rev.1 (SW development / V&V), ECSS-Q-ST-80C Rev.2 (SW product assurance), project norms N1–N20 (`docs/rules.md`), reviewer contract `docs/engineering/independent-review.md` (rev 0.7) |
| Independence declaration          | The reviewer had no part in authoring, building-for-delivery, or previously reviewing the items under review; it is a different agent/session from the FRICK-REVIEW-0150 reviewer. This review ran in a fresh context; every declared correction was re-read and every relied-upon claim was re-executed first-hand (see §7 Evidence ledger). No author context was inherited and no clarifications were taken from the author. |
| Norms applied                    | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

**In scope (verified first-hand):**

- **Closure of the FRICK-REVIEW-0150 findings** — R2-01/PA-01 (mandatory
  evidence wording), R1-01, R2-02, R3-01, R4-01, R4-02, and PA-02 — against the
  actual working-tree content.
- **Correctness of the corrected evidence statement** — that "the RTEMS build
  produces the lwIP archive (85 objects incl. `sys_arch.c`) and a forced-extraction
  link resolves `tcpip_init`/`lwip_socket`/all `sys_*`; the default image does
  not reference the adapter yet (S6b-2), so it is unchanged" is true and matches
  what the delivered build actually does.
- **Scope conformance** — that the implemented content matches the documented
  scope of the increment and of the delta (standing rule).
- **Gate re-run** — default `ctest` (expected 22/22),
  `tools/scripts/trace.py` (expected PASS 114/71/29/14),
  `tools/scripts/metrics.py --check`, header-vs-`## History` for every touched
  document, and `make renode` (image present).
- **Residual/new findings** introduced or left by the delta.

**Explicitly excluded (recorded so the scope is bounded):**

- On-target lwIP behaviour (netif/PHY binding, ISR input, TCP correctness,
  timing, stack high-water): not runnable in this increment; registered as
  **GAP-A5** / **NC-S6-01**/**NC-S6-02** (unchanged by the delta).
- The lwIP third-party sources themselves (`third_party/lwip`, fetched, pinned,
  BSD-3): reviewed only as the API contract.
- `verification/lean` (`lake build`) was not re-run: the delta adds no
  Lean-modelled artifact and touches no Lean source (`lean: []` + `lean_note`
  for SRS-REQ-040, norm N1 interpretation note 1).
- The SSS (`docs/requirements/SSS.md`) was not modified (N14 trigger inactive);
  R3 scope here is the SRS/manifest/plan side only.
- The `FRICK-REVIEW-0150` report itself (the report is the reference, not an item
  to be edited; its follow-up table is the closure vehicle read by this review).

## 2. References

- Project norms: `docs/rules.md` (N1–N20); reviewer contract and profiles:
  `docs/engineering/independent-review.md` (rev 0.7); `AGENTS.md`, `README.md`.
- Prior review: `docs/reviews/2026-10-12-m4-m11-lwip-rtems-s6b1-review.md`
  (FRICK-REVIEW-0150); review log `docs/reviews/README.md`; template
  `docs/reviews/review-template.md`.
- Traceability manifest/gate: `docs/requirements/traceability.yaml`,
  `tools/scripts/trace.py` (N4); planning set `docs/development-plan.md`,
  `docs/planning/sprint-plan.md`.
- V&V tooling: `docs/vv/tool-qualification.md` (norm N10),
  `docs/vv/verification-results.md`, `tools/scripts/ci_rtems_cross_build.sh`,
  `.github/workflows/ci.yml`, `.github/workflows/nightly.yml`,
  `.gitea/workflows/ci.yml`.
- ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025);
  ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2 (context only).
- lwIP `STABLE-2_2_1_RELEASE` (pinned) `src/include/lwip/sys.h`,
  `src/api/tcpip.c`; RTEMS 6.2 classic API (issuer semantics, as verified in
  FRICK-REVIEW-0150 §12).

## 3. Closure table (FRICK-REVIEW-0150 findings)

| Finding (FRICK-REVIEW-0150) | Severity | Delta status | First-hand evidence |
| --------------------------- | -------- | ------------ | ------------------- |
| **R2-01 / PA-01** — false "link-checked"/"builds and links with lwIP" evidence | Minor (escalates to **Major** if the wording is left uncorrected) | **NOT CLOSED — OPEN** | Body wording is corrected (accurate: §4.1, E1–E5), **but the false wording survives in two History rows**: `docs/vv/verification-results.md:445` ("RTEMS image builds+links with lwIP") and `docs/planning/sprint-plan.md:117` ("RTEMS image build/link PASS"). Both are part of the mandated documents (§4.2, E6). |
| **R1-01** (NC-150-02) — `sys_thread_new()` silent failure | Minor | **OPEN** (unchanged; no disposition recorded) | `src/hal/net/lwip/rtems/sys_arch.c:468-472,480-489` still return `{RTEMS_ID_NONE}` silently; no owner/milestone in `development-plan.md`/`sprint-plan.md`/`class-a-gaps.md` (E7, §5.2). |
| **R2-02** (NC-150-03) — RTEMS+lwIP not exercised by any gate/CI/tool | Minor | **OPEN (partial)** | `tools/scripts/ci_rtems_cross_build.sh:34-45` adds the `-DFF_USE_LWIP=ON` cross build and it **works** (E8); **but** (a) no link check was added, (b) N10 re-qualification was not recorded (`docs/vv/tool-qualification.md` unchanged, header 0.3.21 = last History, E9), and (c) **no CI job actually reaches the new block** (per-commit job SKIPs on the absent toolchain; the nightly job does not call the script and does not pass `-DFF_USE_LWIP=ON` — E10), so the script comment "exercised on every CI run" is inaccurate. |
| **R3-01** (NC-150-04) — `development-plan.md` M11/OBJ-19 not updated | Minor | **CLOSED** | `docs/development-plan.md` 0.24: OBJ-19 emulator-limits note (NET = NC-S6-01/NC-S6-02, SpaceWire = NC-S7-01), M11 row updated, History 0.24 = header 0.24 (E11). |
| **R4-01** (NC-150-05) — `network-stack.md` stale | Minor | **OPEN (partial)** | `docs/architecture/network-stack.md` 0.5 §4 now documents per-platform port selection (unix host / in-repo RTEMS `sys_arch`), header = History 0.5 (E12); **missing** the required RTEMS object prerequisites (`CONFIGURE_MAXIMUM_MESSAGE_QUEUES`, NC-S6-02) and the "compiled/archived but not linked until S6b-2" statement; §7 (Gaps) unchanged. |
| **R4-02** (NC-150-06) — NC-S7-01 origin mis-attributed | Minor | **OPEN (partial)** | Origin corrected in `docs/planning/sprint-plan.md:73` ("Sprint-6 recon (scope conformance)") (E13); **the second required part is not done** — the FRICK-REVIEW-0148 follow-up table was not updated to record the NC-S6-01 narrowing and NC-S6-02/NC-S7-01 (`docs/reviews/2026-10-12-m4-m11-tsn-s6a-review.md` untouched, E13). |
| **PA-02** — review-log row / panel missing | Comment | **CLOSED** | `docs/reviews/README.md:149` records FRICK-REVIEW-0150; semaphores in `status.json` remain `green` (E14). *Note:* the row's reason text "R2-01 evidence wording corrected" is now inaccurate — see NC-151-06. |
| R1-02…R1-06, R2-03, R2-04, R3-02, R4-03, PA-03…PA-05 (Comments) | Comment | **advisory / unchanged** | Not in the correction mandate. **R2-04 is moot**: `make renode` was re-run on the working tree and passed (E15). |

## 4. Mandatory blocker — R2-01/PA-01 (NC-150-01) is not genuinely closed

### 4.1 The corrected statement is accurate (verified)

The corrected wording — "the RTEMS build produces the lwIP archive (85 objects
incl. `sys_arch.c`) and a **forced-extraction link resolves**
`tcpip_init`/`lwip_socket`/all `sys_*`; the default image does not reference the
adapter yet (S6b-2), so it is unchanged" — is **true**. A fresh RTEMS cross build
(`-DCMAKE_TOOLCHAIN_FILE=…/rtems-riscv-mpfs.cmake -DFF_PLATFORM=rtems
-DFF_USE_LWIP=ON`) produces a RISC-V ELF, `frick_lwip` with **85 objects**
(incl. `…/rtems/sys_arch.c.obj`); `libfrick_lwip.a` defines `tcpip_init`,
`lwip_socket`, `sys_arch_mbox_fetch`, `sys_mbox_trypost_fromisr`, `sys_now`,
`sys_jiffies`, `sys_thread_new`; the plain image has **0** `lwip/tcpip/sys_arch`
symbols; the forced-extraction link succeeds and resolves all probed symbols;
and the `FF_USE_LWIP=ON` image's `.text` is `cmp`-identical to the
`FF_USE_LWIP=OFF` image's `.text` (E1–E5). The statement is therefore not merely
re-worded — it is substantiated.

### 4.2 Residual false evidence wording (the blocker)

The mandated correction was to propagate the corrected reading to **all** the
documents named in FRICK-REVIEW-0150 R2-01, not only their body rows. Two
**History rows** still carry the exact wording the finding declared false:

- `docs/vv/verification-results.md:445` — History rev 0.39: "…, RTEMS image
  **builds+links with lwIP**; NC-S6-02 opened …". This is the same document and
  revision named in R2-01, and it **contradicts the corrected body row of the
  same revision** (`:51`, which states the image "does **not** reference the
  adapter yet … so it is unchanged").
- `docs/planning/sprint-plan.md:117` — History rev 0.11: "… (host 23/23 +
  **RTEMS image build/link PASS**)". Same over-claim ("build/link PASS" of an
  image that does not link lwIP).

There is no other residual: a repository-wide search for "link-checked" finds the
phrase only inside the FRICK-REVIEW-0150 report (where it is the finding text),
and the seven mandated documents' bodies are correctly worded (E6).

**Assessment.** The *substantive* deliverable and evidence are correct and the
bodies are honest; the defect is a **two-cell textual residue**. However, the
correction was **mandatory**, its incompleteness leaves a false statement inside
the V&V evidence record itself, and the FRICK-REVIEW-0150 clause pre-committed
that the finding "becomes **Major** if the increment is presented as
target-verified with that wording unchanged". The wording is unchanged in the two
History rows, so **per the letter of the profile contract the finding escalates
to Major** and the pre-commit blocker is **not** cleared → the review is
**REJECTED** and commit/push is blocked (N12/N7) until the two rows are reworded.

### 4.3 One further evidence-wording defect (NC-151-04)

The new S6b-1 row (`docs/vv/verification-results.md:51`) states "new port files
`-Wall -Wextra -Werror` clean on the RTEMS target". The RTEMS build applies only
`-Wall -Wextra` to `sys_arch.c` (`CMakeLists.txt:189-191`); `-Werror` is applied
only to `frick_fsw_core` and only when `FF_ENABLE_WERROR=ON` (`CMakeLists.txt:234-235`,
default OFF). The substantive fact (0 compiler warnings) is independently
confirmed, but the literal "-Werror clean on the RTEMS target" claim is not what
the build does → Minor (evidence wording), same class as the R2-01 lineage.

## 5. New findings

### NC-151-01 — **Major** — residual false evidence wording (R2-01/PA-01 incomplete)

See §4.2. Two History rows (`docs/vv/verification-results.md:445`,
`docs/planning/sprint-plan.md:117`) still state the false "builds+links with
lwIP"/"build/link PASS" claim for an image that contains no lwIP code.
**Required correction (trivial):** reword both History rows to the verified
fact ("RTEMS build produces the lwIP archive + a forced-extraction link
resolves the stack; the default image does not reference the adapter yet").
**Owner:** frick-fsw authoring agent. **Sprint:** S6b-1 pre-commit delta.
**Blocks:** commit/push (N12).

### NC-151-02 — **Minor** — R2-02 partial: the RTEMS+lwIP configuration is still not gated, and N10 re-qualification is missing

Three residual defects in the R2-02 correction:

1. **Not actually exercised in CI.** The per-commit `rtems-cross-build` job
   (`.github/workflows/ci.yml:263-271`, `.gitea/workflows/ci.yml:302-318`) only
   checks out and runs `ci_rtems_cross_build.sh`, which exits 0 with a SKIP
   *before* the new lwIP block whenever the toolchain is absent
   (`ci_rtems_cross_build.sh:20-25`); the job does not fetch `third_party/lwip`.
   The nightly job (`nightly.yml:20-48`) installs the toolchain but configures
   `-DFF_PLATFORM=rtems` **without** `-DFF_USE_LWIP=ON` and does not call the
   script. No CI job reaches the new block, so the script comment "exercised on
   every CI run" (`ci_rtems_cross_build.sh:35-36`) is inaccurate.
2. **No link check.** R2-02 asks to add "the configuration (+ link check)"; the
   "forced-extraction link resolves …" claim therefore remains ungated.
3. **N10 re-qualification missing.** The changed script is the evidence producer
   for `ci:rtems-cross-build` (T10); per N10 "any change to a recorded V&V script
   invalidates its qualification and forces a re-qualification review".
   `docs/vv/tool-qualification.md` is unchanged (0.3.21, no new History row)
   (E9).

**Required correction:** wire `-DFF_USE_LWIP=ON` (+ the `-Wl,--undefined` link
check) into a job that has both the toolchain and the lwIP tree (or fetch the
tree in the job), correct the script comment, and re-qualify `ci:rtems-cross-build`
(T10) in the register. **Owner:** frick-fsw authoring agent. **Sprint:** S6b-1
delta / S6b-2 at the latest (PA-04).

### NC-151-03 — **Minor** — R4-01 partial: the design note still omits the required content

`docs/architecture/network-stack.md` 0.5 §4 gained the per-platform port
selection, but R4-01 also required the **RTEMS object prerequisites**
(`CONFIGURE_MAXIMUM_MESSAGE_QUEUES`, the substance of NC-S6-02) and the
**"compiled/archived but not linked until S6b-2"** statement; §7 (Gaps) was not
touched. **Required correction:** add those two facts to §4 and reference them in
§7 with the History row. **Owner:** frick-fsw authoring agent. **Sprint:** S6b-1
delta.

### NC-151-04 — **Minor** — `-Werror clean on the RTEMS target` is not what the build does

See §4.3. **Required correction:** state "-Wall -Wextra, 0 warnings on the RTEMS
target" (or enable `FF_ENABLE_WERROR` for the port and keep the claim).
**Owner:** frick-fsw authoring agent.

### NC-151-05 — **Minor** — R1-01 remains open and is not tracked with owner/milestone (N12/N19)

`sys_thread_new()` still returns an invalid handle silently on pool/task-create
failure (`sys_arch.c:468-472,480-489`). If it is intentionally deferred, N19
requires the residual to be tracked with an owner and a milestone in
`development-plan.md`/`class-a-gaps.md`; a repository search finds no
`sys_thread_new`/`NC-150-02` tracking entry outside the review reports (E7).
**Required correction:** fix it in the delta (preferred — bounded, low-risk per
FRICK-REVIEW-0150 R1-01) **or** record the disposition with owner + sprint
(S6b-2) in the plan set. **Owner:** frick-fsw authoring agent.

### NC-151-06 — **Comment** — the review-log reason text is now inaccurate

`docs/reviews/README.md:149` records FRICK-REVIEW-0150 as "APPROVED WITH
COMMENTS (R2-01 evidence wording corrected)". Given §4.2 the wording is not fully
corrected; reword after NC-151-01 (e.g. "(R2-01 wording corrected in the delta —
FRICK-REVIEW-0151)").

### NC-151-07 — **Comment** — FRICK-REVIEW-0148 follow-up table still not updated (R4-02 residue)

The NC-S6-01 narrowing and NC-S6-02/NC-S7-01 are recorded in
`sprint-plan.md` §3 but not in any review report's follow-up table
(`docs/reviews/2026-10-12-m4-m11-tsn-s6a-review.md` untouched). R4-02 required
this; fold it into the closure of NC-150-06.

### R4-02/R2-03 re-statements (unchanged advisories)

`status.json.deferred` remains `[]` while three NCs are deferred (R3-02/PA-03),
`loop-audit.md` §2 rules audit was again not re-performed for this loop (R4-03),
and the `ASan/UBSan clean` cell is carried over (the sanitizer builds exclude the
RTEMS-only port; R2-03). All advisory, no derogation, unchanged from
FRICK-REVIEW-0150 — recorded so no gap is left silent (N19).

## 6. Scope conformance (standing rule)

**Verdict: conformant — no scope-deviation NC raised.**

- The **increment** implements exactly what it declares: four new files under
  `src/hal/net/lwip/rtems/` (the `sys_arch` port, `arch/cc.h`,
  `arch/sys_arch.h`, `lwipopts.h`) and the `if(FF_USE_LWIP)` block of
  `CMakeLists.txt` selecting the RTEMS port only for `FF_PLATFORM=rtems`
  (`CMakeLists.txt:171-205`). `FF_USE_LWIP` stays `OFF` by default; the adapter
  is not referenced by the RTEMS application (0 lwIP symbols in the image, E4);
  no netif/ISR work (S6b-2) is smuggled in.
- The **delta** implements exactly the mandated corrections plus nothing else:
  the evidence-wording edits, the `ci_rtems_cross_build.sh` lwIP cross build,
  the `development-plan.md`/`network-stack.md`/`sprint-plan.md` updates, the
  NC-S7-01 origin fix, and the `docs/reviews/README.md` log row. The
  `tools/scripts/ci_rtems_cross_build.sh` change is a mandated R2-02 correction,
  not scope creep.
- The only conformance defects found are **evidential/records**, not scope:
  residual evidence wording (NC-151-01/04), incomplete R2-02/R4-01/R4-02
  corrections (NC-151-02/03/07), and an untracked open Minor (NC-151-05).

## 7. Gate re-run and evidence ledger (all first-hand, 2026-10-12)

| # | Command / artefact | Observed result |
| - | ------------------ | --------------- |
| E1 | `git rev-parse HEAD`, `git status --porcelain` | HEAD `3822f721c2f4588dab1ded10e0e21103bd50aa8d`; 15 modified tracked files + untracked `docs/reviews/2026-10-12-m4-m11-lwip-rtems-s6b1-review.md` and `src/hal/net/lwip/rtems/` — exactly the declared delta set |
| E2 | fresh RTEMS+lwIP cross build in the pinned container (`frick-rtems-toolchain:local`): `cmake -S . -B build/probe-s6b1-rtems -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-mpfs.cmake -DFF_PLATFORM=rtems -DFF_USE_LWIP=ON && cmake --build` | **PASS**; RISC-V ELF; **85** objects in `frick_lwip` incl. `…/rtems/sys_arch.c.obj`; no toolchain warning on a fresh configure |
| E3 | `riscv-rtems6-nm build/probe-s6b1-rtems/libfrick_lwip.a` | defines `T tcpip_init`, `T lwip_socket`, `T sys_thread_new`, `T sys_arch_mbox_fetch`, `T sys_mbox_trypost_fromisr`, `T sys_now`, `T sys_jiffies` |
| E4 | `riscv-rtems6-nm build/probe-s6b1-rtems/frick_fsw` | **0** matches for `lwip|tcpip|sys_arch` (1390 symbols) — the image contains no lwIP code |
| E5 | forced-extraction link (`-Wl,--undefined=FF_NetStackLwip -Wl,--undefined=tcpip_init -Wl,--undefined=sys_arch_mbox_fetch`) + `objcopy --only-section=.text` + `cmp` | link **OK**, all probed symbols resolved (`lwip_rtems_rand`, `lwip_socket`, `sys_arch_mbox_fetch`, `sys_jiffies`, `sys_mbox_trypost_fromisr`, `sys_now`, `sys_thread_new`, `tcpip_init`); `.text` 124396 → 197520 B; **`FF_USE_LWIP=ON` `.text` == `FF_USE_LWIP=OFF` `.text`** (image unchanged) |
| E6 | repository search `link-checked` / `builds+links` / `build/link` / `links with lwIP` | only two residual matches: `docs/vv/verification-results.md:445` and `docs/planning/sprint-plan.md:117` (plus the FRICK-REVIEW-0150 finding text) |
| E7 | `grep -rn "sys_thread_new\|R1-01\|NC-150" docs/{status,planning,quality,development-plan.md}` | no tracking entry for the S6b-1 `sys_thread_new` residual outside the review reports |
| E8 | `docker run … frick-rtems-toolchain:local bash tools/scripts/ci_rtems_cross_build.sh` | RTEMS cross build **PASS**; then "cross build with `FF_USE_LWIP=ON`: **PASS**" (the new block works) |
| E9 | `docs/vv/tool-qualification.md` | header 0.3.21 = last History 0.3.21; T10 row (`setup_rtems.sh` — `ci:rtems-cross-build`) unchanged; **no new History row** for the changed cross-build script |
| E10 | `.github/workflows/ci.yml:263-271`, `.gitea/workflows/ci.yml:302-318`, `.github/workflows/nightly.yml:20-48` | per-commit job runs the script (toolchain absent → SKIP before the lwIP block, no lwIP fetch); nightly configures `-DFF_PLATFORM=rtems` **without** `-DFF_USE_LWIP=ON` and does not call the script → **no CI job reaches the new block** |
| E11 | `docs/development-plan.md` | header 0.24 = History 0.24; OBJ-19 emulator-limits note + M11 row updated for S6b-1/NC-S6-01/NC-S6-02/NC-S7-01 |
| E12 | `docs/architecture/network-stack.md` | header 0.5 = History 0.5; §4 port selection added; §7 unchanged; no `CONFIGURE_MAXIMUM_MESSAGE_QUEUES`/`not linked` statement |
| E13 | `docs/planning/sprint-plan.md` §3; `git status` of `docs/reviews/2026-10-12-m4-m11-tsn-s6a-review.md` | NC-S7-01 origin = "Sprint-6 recon (scope conformance)"; the 0148 report is **not** modified |
| E14 | `docs/reviews/README.md:149`; `docs/status/status.json` | FRICK-REVIEW-0150 row present; semaphores `green`; `deferred: []` |
| E15 | `make renode` | **PASS** (`RENODE_TEST: PASS`; `hal_tsn=PASS`, `sb_roundtrip=PASS`, …) on the working tree |
| E16 | `ctest --test-dir build/native` | **22/22 PASS**, 0 failures |
| E17 | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 114 requirements (71 srs / 29 sss / 14 gap), 51 Lean theorems checked |
| E18 | `python3 tools/scripts/metrics.py --check` | "metrics panel: current" (exit 0) |
| E19 | header vs last `## History` of every touched document | `network-stack` 0.5/0.5, `development-plan` 0.24/0.24, `sprint-plan` 0.11/0.11, `traceability.md` 0.34/0.34, `handover` 1.14/1.14, `loop-audit` 0.18/0.18, `renode-environment` 0.18/0.18, `verification-results` 0.39/0.39 — all **MATCH** |
| E20 | `clang-format --dry-run --Werror` on the four port files; SPDX header scan | clean; all four files carry `SPDX-License-Identifier: AGPL-3.0-only` |

## 8. Findings summary (delta)

| # | Severity | Profile | Item | Finding | Required action |
| - | -------- | ------- | ---- | ------- | --------------- |
| **NC-151-01** (= R2-01/PA-01 incomplete) | **Major** | R2 + PA | `docs/vv/verification-results.md:445`; `docs/planning/sprint-plan.md:117` | The mandated evidence-wording correction left the false claim "RTEMS image builds+links with lwIP" / "build/link PASS" in two History rows of an image that links no lwIP code (0 symbols) | Reword both History rows to the verified fact; then the blocker clears |
| NC-151-02 (= R2-02 partial) | Minor | R2 | `tools/scripts/ci_rtems_cross_build.sh`; CI workflows; `tool-qualification.md` | The RTEMS+lwIP configuration is still not reached by any CI job (script comment "exercised on every CI run" is inaccurate); no link check added; N10 re-qualification not recorded | Wire it into a job with toolchain + lwIP tree (or fetch the tree); fix the comment; re-qualify T10 (N10) |
| NC-151-03 (= R4-01 partial) | Minor | R4 | `docs/architecture/network-stack.md` §4/§7 | Missing the RTEMS object prerequisites (NC-S6-02) and the "compiled/archived but not linked until S6b-2" statement | Add both to §4 and §7 |
| NC-151-04 | Minor | R2 | `docs/vv/verification-results.md:51` | "`-Wall -Wextra -Werror` clean on the RTEMS target" is not what the build does (only `-Wall -Wextra`; `-Werror` is core-only and off by default) | Reword (0 warnings) or enable the flag |
| NC-151-05 (= R1-01) | Minor | R1 | `src/hal/net/lwip/rtems/sys_arch.c:468-489`; plan records | `sys_thread_new()` still fails silently, and the deferred Minor is not tracked with owner/milestone (N12/N19) | Fix loudly, or record the disposition (owner + S6b-2) in the plan set |
| NC-151-06 | Comment | R4 | `docs/reviews/README.md:149` | Log reason says "R2-01 evidence wording corrected" — inaccurate per NC-151-01 | Reword after NC-151-01 |
| NC-151-07 (= R4-02 partial) | Comment → Minor | R4 | FRICK-REVIEW-0148 follow-up table | NC-S6-01 narrowing + NC-S6-02/NC-S7-01 not recorded in any review follow-up table | Record in the 0148 delta table (N11/N12) |
| R3-02/PA-03, R4-03, R2-03 | Comment | R3/R4/R2 | `status.json.deferred`; `loop-audit.md` §2; sanitizer cell | Unchanged advisories from FRICK-REVIEW-0150 | Consider/record; no derogation |

**Severity definition** (from `docs/reviews/review-template.md`): **Major** — a
non-conformity that disqualifies the item for the applicable criticality, or a
defect that can defeat a requirement or a safety barrier. **Minor** — a
non-conformity that does not invalidate the requirement but must be corrected or
justified. **Comment** — observation/recommendation, no derogation.

NC-151-01 is graded **Major** by the pre-committed escalation clause of
FRICK-REVIEW-0150 (R2-01/PA-01: "becomes Major if the increment is presented as
target-verified with that wording unchanged"): the wording is unchanged in the
record. Substantively the increment's deliverable and evidence are verified and
no functional check is a false pass; the remedy is a two-cell textual edit.

## 9. Conclusion

- [ ] APPROVED — no findings or Comments only
- [ ] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [x] **REJECTED** — one or more Major findings

**Rationale.** The Sprint-6b-1 increment itself is substantively correct and the
corrected evidence statement is accurate: a fresh RTEMS cross build with
`FF_USE_LWIP=ON` produces the 85-object lwIP archive (incl. `sys_arch.c`), the
forced-extraction link resolves `tcpip_init`/`lwip_socket`/all `sys_*`, the plain
image links no lwIP code and is byte-identical to the `FF_USE_LWIP=OFF` image;
the host/default gates are green (22/22), `trace` is PASS (114/71/29/14),
`metrics --check` is current, `make renode` passes, the touched documents are
history-consistent, and R3-01/PA-02 are properly closed. The review is
nevertheless **REJECTED** because the **mandatory pre-commit correction
(R2-01/PA-01) is incomplete**: two History rows still assert the false
"builds+links with lwIP"/"build/link PASS" claim, so the evidence record remains
internally contradictory and the pre-committed escalation clause makes this a
Major that blocks commit/push (N12/N7). R1-01, R2-02, R4-01 and R4-02 are
partially or wholly unresolved.

**Conditions to convert REJECTED → APPROVED WITH COMMENTS (S6b-1 pre-commit
delta, N12):**

1. **NC-151-01** — reword the two History rows (`verification-results.md:445`,
   `sprint-plan.md:117`). This is the sole pre-commit blocker.
2. NC-151-03/04 — complete the `network-stack.md` §4/§7 update and correct the
   `-Werror` wording.
3. NC-151-02 — wire the RTEMS+lwIP configuration into a CI job that has the
   toolchain and the lwIP tree (or fetch the tree), fix the script comment, and
   re-qualify T10 (N10). If not done now, track it to S6b-2 with owner.
4. NC-151-05 — fix `sys_thread_new()` loudly (preferred) or record the
   disposition with owner + sprint in the plan set (N12/N19).
5. NC-151-07 / NC-151-06 — record the NC narrowing/new NCs and correct the log
   reason text.
6. Re-review the deltas in a fresh context before commit/push (N12/N17/N20).

**The loop is not closed:** NC-151-01 is open (Major, blocks); NC-151-02/03/04/05
are open Minors; NC-151-07/06 are open; the pre-existing **NC-S6-01 (narrowed)**,
**NC-S6-02** and **NC-S7-01** remain open/scheduled (S6b-2 / S7); **GAP-A5**
remains for human acceptance.

## 10. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9) | Senior FSW / real-time / space software developer (independent context) | 2026-10-12 | **REJECTED** — R1-01 (NC-151-05) still open and untracked; port code otherwise unchanged and correct |
| R2 (N9) | Senior SW V&V engineer (independent context) | 2026-10-12 | **REJECTED** — NC-151-01 (mandatory evidence wording incomplete: Major), NC-151-02 (R2-02 partial, N10), NC-151-04; gates re-run green (22/22, trace PASS, metrics current, renode PASS); corrected statement independently substantiated |
| R3 (N14) | Senior avionics / systems engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — R3-01 CLOSED (development-plan.md 0.24); manifest/mirror/counts accurate (114/71/29/14); R3-02 advisory |
| R4 (N17) | Senior SW documentation engineer (independent context) | 2026-10-12 | **REJECTED** — R4-01 partial (NC-151-03), R4-02 partial (NC-151-07), residual wording (NC-151-01), log reason (NC-151-06); headers = History in 8/8 touched documents |
| PA (N20) | Senior SW Product Assurance engineer (independent context) | 2026-10-12 | **REJECTED** — scope conformance verified (no scope NC) and PA-02 CLOSED, but the mandatory evidence correction (PA-01) is incomplete (Major) and R2-02/R1-01 remain open; on-target behaviour stays with the human authority (GAP-A5, NC-S6-02) |

## 11. Follow-up records

Non-conformances and their dispositions (norm N12: NCs are resolved by
re-applying **all** rules to the affected scope and re-reviewing the deltas until
full compliance).

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| **NC-150-01** (R2-01/PA-01) | Minor → **Major** | `docs/vv/verification-results.md:445`, `docs/planning/sprint-plan.md:117` | False "builds+links with lwIP" wording survives in two History rows after a mandatory correction (image has 0 lwIP symbols) | Reword both rows to the verified fact | This report §4.2/E6; corrected bodies §4.1/E1–E5 | frick-fsw authoring agent | **OPEN (Major — blocks commit/push)** | **S6b-1 delta** |
| NC-150-02 (R1-01) | Minor | `src/hal/net/lwip/rtems/sys_arch.c:468-489` | `sys_thread_new()` silent failure; not tracked with owner/milestone | Fail loudly, or record disposition (owner + sprint) | This report §5 (NC-151-05), E7 | frick-fsw authoring agent | **OPEN** | **S6b-1 delta / S6b-2** |
| NC-150-03 (R2-02) | Minor | `tools/scripts/ci_rtems_cross_build.sh`; CI; `tool-qualification.md` | Config added and working locally, but ungated in CI; no link check; N10 re-qualification missing | Wire into a job with toolchain + lwIP tree; fix comment; re-qualify T10 | E8, E9, E10 | frick-fsw authoring agent | **OPEN (partial)** | **S6b-1 delta / S6b-2** |
| NC-150-04 (R3-01) | Minor | `docs/development-plan.md` (M11/OBJ-19 + history) | Plan not updated | Updated row + history 0.24 | E11 (header=history) | frick-fsw authoring agent | **CLOSED** | FRICK-REVIEW-0151 (this report) |
| NC-150-05 (R4-01) | Minor | `docs/architecture/network-stack.md` §4/§7 | Design note incomplete | Port selection added; prerequisites + "not linked yet" + §7 missing | E12 | frick-fsw authoring agent | **OPEN (partial)** | **S6b-1 delta** |
| NC-150-06 (R4-02) | Minor | `sprint-plan.md` §3; FRICK-REVIEW-0148 follow-up table | Origin mis-attributed; NCs not in a review table | Origin corrected; 0148 table still not updated | E13 | frick-fsw authoring agent | **OPEN (partial)** | **S6b-1 delta** |
| NC-150-07 (Comments) | Comment | see FRICK-REVIEW-0150 §8 | Advisories | Unchanged | §5 (R2-03/R3-02/R4-03 re-stated) | frick-fsw authoring agent | **open (advisory)** | S6b-2 |
| NC-151-01 | **Major** | `verification-results.md:445`, `sprint-plan.md:117` | Residual false evidence wording | Reword two History rows | §4.2/E6 | frick-fsw authoring agent | **OPEN (blocks)** | S6b-1 delta |
| NC-151-02 | Minor | CI/`ci_rtems_cross_build.sh`/`tool-qualification.md` | RTEMS+lwIP not gated; N10 | See above | E8-E10 | frick-fsw authoring agent | **OPEN** | S6b-1 delta / S6b-2 |
| NC-151-03 | Minor | `network-stack.md` §4/§7 | Missing prerequisites + "not linked yet" | Add both | E12 | frick-fsw authoring agent | **OPEN** | S6b-1 delta |
| NC-151-04 | Minor | `verification-results.md:51` | `-Werror` claim not the build config | Reword or enable `FF_ENABLE_WERROR` | §4.3 | frick-fsw authoring agent | **OPEN** | S6b-1 delta |
| NC-151-05 | Minor | `sys_arch.c`; plan records | R1-01 open + untracked | Fix or track | E7 | frick-fsw authoring agent | **OPEN** | S6b-1 delta / S6b-2 |
| NC-151-06 | Comment | `docs/reviews/README.md:149` | Log reason inaccurate | Reword after NC-151-01 | E14 | frick-fsw authoring agent | **OPEN** | S6b-1 delta |
| NC-151-07 | Minor | FRICK-REVIEW-0148 follow-up table | NC narrowing/new NCs not recorded | Record (N11/N12) | E13 | frick-fsw authoring agent | **OPEN** | S6b-1 delta |
| **NC-S6-01** | Minor (scope) | SRS-REQ-040 / S6b | NET lwIP-on-GEM peripheral-bound deferred | Unchanged from FRICK-REVIEW-0150; host-tested adapter | `sprint-plan.md` §2/§3; `traceability.yaml` note | frick-fsw authoring agent | **open (scheduled S6b/S6b-2)** | S6b-2 |
| **NC-S6-02** | Minor | `config/rtems/rtems_config.c`; runtime `tcpip_init` | No `CONFIGURE_MAXIMUM_MESSAGE_QUEUES` → runtime `tcpip_init` fails | Unchanged, correctly scoped | FRICK-REVIEW-0150 §7.2; `sprint-plan.md` §3 | frick-fsw authoring agent | **open (scheduled S6b-2)** | S6b-2 |
| **NC-S7-01** | Minor (scope) | OBJ-19 / SRS-REQ-032 (SpaceWire) | No SpaceWire peripheral in the Renode model | Unchanged; origin now correctly attributed | `sprint-plan.md` §2/§3 | frick-fsw authoring agent | **open (scheduled S7)** | S7 |

## 12. Norm-by-norm audit (delta)

| N | Assessment for this delta |
| - | ------------------------- |
| N1 | **OK with justification** — no Lean-modelled artifact added; SRS-REQ-040 keeps `lean: []` + explicit `lean_note` (no silent waiver); no Lean source touched |
| N2 | **OK** — all new/changed content is English |
| N3 | **OK** — SRS/SSS unchanged; SRS-REQ-040 text still accurate |
| N4 | **OK** — `trace.py` PASS (114/71/29/14); manifest lists the new implementation paths (all exist) |
| N5 | **OK** — GAP-A5 kept open; no level-A claim |
| N6 | **Gap** — the RTEMS+lwIP configuration is still not exercised by any deployed CI job (NC-151-02) |
| N7 | **Not applicable to the reviewer** — nothing committed/pushed by this review; the author must push both remotes after the Major is resolved |
| N8 | **Gap** — `network-stack.md` §4/§7 incomplete (NC-151-03); residual record wording (NC-151-01/04) |
| N9 | **OK** — this delta review ran in a fresh, independent context over the exact revision |
| N10 | **Gap** — `ci_rtems_cross_build.sh` changed but its qualification was not re-recorded in `tool-qualification.md` (NC-151-02) |
| N11 | **OK** — the FRICK-REVIEW-0150 row is in `docs/reviews/README.md` (PA-02 closed); this report is written to `docs/reviews/` for commit with the change |
| N12 | **Open** — NC-150-01 escalated to Major; NC-150-02/03/05/06 and NC-151-01…07 open; block commit/push until resolved and re-reviewed |
| N13 | **OK** — AGPL-3.0-only SPDX on all four port files; licence aggregate already analysed (PA-05 advisory unchanged) |
| N14 | **Not triggered / satisfied** — SSS not modified; SRS/manifest/plan side reviewed (R3-01 CLOSED) |
| N15 | **Satisfied for the re-run gates** — default `ctest` 22/22, `trace` PASS, `metrics --check` current, `make renode` PASS, RTEMS+lwIP cross build PASS; Lean not applicable |
| N16 | **OK** — header = last History row in 8/8 touched documents with a History table; CHANGELOG entry present; the two offending History rows are accurate-*wording* defects (NC-151-01), not table-shape defects |
| N17 | **Required** — the documentation deltas must be re-reviewed by R4 in a fresh context after NC-151-01/03/06/07 are corrected |
| N18 | **Satisfied** — `make renode` → `RENODE_TEST: PASS` re-executed on the working tree (E15) |
| N19 | **Gap** — R1-01 open without owner/milestone tracking (NC-151-05); `loop-audit.md` §2 rules audit not re-performed (R4-03) |
| N20 | **Discharged (with a Major)** — PA reviewed the phase products in independent context; scope conformance verified and NC dispositions checked; the loop remains open until the Major is corrected and re-reviewed |

## Facts, Assumptions, Inferences, Recommendations, Confidence, Open gaps

**Facts (verified first-hand at `3822f72` + working tree, 2026-10-12).**

1. The delta is exactly the declared set: 15 modified tracked files + the
   untracked FRICK-REVIEW-0150 report + the untracked `src/hal/net/lwip/rtems/`
   port (unchanged from FRICK-REVIEW-0150).
2. The **corrected evidence statement is accurate**: a fresh RTEMS+lwIP cross
   build produces `frick_lwip` with **85** objects (incl. `sys_arch.c.obj`) and a
   RISC-V ELF; the archive defines `tcpip_init`/`lwip_socket`/all probed `sys_*`;
   the plain image has **0** lwIP symbols; the forced-extraction link resolves
   all probed symbols; the `FF_USE_LWIP=ON` `.text` is `cmp`-identical to the
   `FF_USE_LWIP=OFF` `.text`.
3. **The mandatory correction is incomplete**: `verification-results.md:445`
   ("RTEMS image builds+links with lwIP") and `sprint-plan.md:117` ("RTEMS image
   build/link PASS") still carry the false wording (NC-151-01).
4. `docs/reviews/README.md:149` records FRICK-REVIEW-0150 (PA-02 closed).
5. `docs/development-plan.md` 0.24 closes R3-01 (M11/OBJ-19 + history),
   header = history.
6. `docs/architecture/network-stack.md` 0.5 adds the port selection but omits the
   RTEMS object prerequisites and the "not linked yet" statement; §7 unchanged.
7. `sprint-plan.md` §3 corrects NC-S7-01's origin; the FRICK-REVIEW-0148 report
   is unmodified.
8. `tools/scripts/ci_rtems_cross_build.sh` adds a working `-DFF_USE_LWIP=ON`
   cross build, but no CI job reaches it and no link check is added;
   `tool-qualification.md` is unchanged.
9. `sys_thread_new()` still fails silently (`sys_arch.c:468-489`), and the
   residual is tracked nowhere outside the review reports.
10. Gates: default `ctest` **22/22 PASS**; `trace` **PASS** (114/71/29/14, 51
    theorems); `metrics.py --check` current; `make renode` **PASS**;
    `clang-format` clean; 8/8 touched-document header = History.
11. No file other than this report was modified, created or deleted by the
    reviewer; nothing was committed or pushed.

**Assumptions.**

1. The pinned `third_party/lwip` tree (`STABLE-2_2_1_RELEASE`) is the stack a
   mission would use; its `sys.h`/`tcpip.c` are the contract the port must satisfy.
2. `frick-rtems-toolchain:local` (`90f81d080611`) is the project's pinned RTEMS
   6.2/mpfs64imafdc toolchain; results hold for it.
3. `make renode`'s PASS on the working tree is valid evidence for the N18 gate
   for this artefact (the emulated run was performed by the reviewer).
4. The author's statements are treated as claims, not evidence; every relied-upon
   one was re-verified.

**Inferences.**

1. The residual History-row wording is most plausibly an oversight (the author
   edited the body rows), not a deliberate re-assertion — but its effect is the
   same false evidence in a mandated document, so the pre-committed escalation
   applies.
2. The R2-02 correction was aimed at the script, not at CI reachability; the
   configuration is therefore "added but not gated" (the PA-04 concern of
   FRICK-REVIEW-0150 remains materially open).
3. R4-01/R4-02 were closed at the "headline" level (port selection, origin) but
   the linked sub-requirements (prerequisites + "not linked yet"; 0148 table)
   were missed.

**Recommendations.**

1. **Close NC-151-01 first** — reword the two History rows; this alone clears the
   pre-commit blocker.
2. Complete NC-151-03/04/06/07 and fix NC-151-02 (wire the RTEMS+lwIP build into
   a job with toolchain + lwIP tree and re-qualify T10); if deferred, record owner
   + sprint (N12/N19).
3. Fix NC-151-05 (fail loudly in `sys_thread_new()`) in this delta — it is the
   bounded fix already scoped by FRICK-REVIEW-0150 R1-01 — or track it.
4. Keep GAP-A5 and NC-S6-02 open; size `CONFIGURE_MAXIMUM_MESSAGE_QUEUES`
   (plus semaphore/task headroom) with the real netconn/socket budget before the
   first on-target `tcpip_init`.
5. Re-review the deltas in a fresh context (R1/R2/R4/PA) before commit/push
   (N12/N17/N20).

**Confidence.**

- **High** for every Fact above (each was re-executed or read first-hand on
  2026-10-12), including the accuracy of the corrected statement and the residual
  wording.
- **High** for NC-151-01's status (direct file evidence) and the **Medium**
  significance call to escalate it to Major (rests on FRICK-REVIEW-0150's own
  pre-committed escalation clause).
- **Medium** for the CI-reachability analysis (NC-151-02): it rests on reading the
  three workflow files and the script; a runner with a preinstalled toolchain and
  a pre-present lwIP tree would change the outcome.
- Confidence is not a probability of safety.

**Open gaps** (not closed by this review).

- **NC-151-01 (Major)** — blocks commit/push until the two History rows are
  reworded and re-reviewed.
- **NC-151-02/03/04/05/06/07** — open Minors/Comments.
- **NC-S6-01 (narrowed)** open, scheduled S6b/S6b-2; **NC-S6-02** open, scheduled
  S6b-2; **NC-S7-01** open, scheduled S7.
- **GAP-A5** (on-target behaviour) — no on-target lwIP run, no netif/PHY, no
  timing/stack-high-water evidence; the stack cannot start on target yet
  (NC-S6-02). **For human acceptance.**
- **Not verifiable by the reviewer:** the author's intent behind the undetected
  History-row edits; any evidence the author holds outside the repository.
  Paywalled/absent sources relied upon: none.

---

*This report is authored to be committed and pushed with the reviewed change
(norm N11). The review was read-only with respect to repository content: the
reviewer wrote exactly one file (this report); **no other file was modified,
created or deleted**, and nothing was committed or pushed. The gate commands
regenerated only git-ignored `build/` artifacts
(`build/{native,rtems,rtems-lwip,probe-s6b1-rtems,renode}`); independence is
declared per norm N9 (fresh context; no part in authoring, building-for-delivery
or previously reviewing the items under review).*

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-12 | independent reviewer (FRICK-REVIEW-0151) | Combined R1+R2+R3+R4+PA delta re-review of the FRICK-REVIEW-0150 corrections at `3822f72` + working tree: REJECTED — NC-151-01 Major (residual false evidence wording; R2-01/PA-01 not genuinely closed); R3-01/PA-02 CLOSED; R1-01/R2-02/R4-01/R4-02 open (partial) |
