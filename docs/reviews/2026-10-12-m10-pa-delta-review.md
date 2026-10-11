# Review Report — PA DELTA RE-REVIEW of the M10 closure increment (norm N12/N20)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0127                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; norms N20/N12) |
| Reference revision                | base commit `aee4135` + **uncommitted** M10 closure increment (working tree, 2026-10-12); HEAD `aee4135` |
| Items under review                | The closure increment for **FRICK-REVIEW-0123** (this reviewer's prior M10 PA review, **REJECTED**, Major PA-01/PA-03): `.github/workflows/nightly.yml`, `docs/vv/tool-qualification.md` (0.3.16), `docs/status/status.json`, `docs/vv/emulated-target.md` (0.3), `docs/vv/verification-plan.md` (0.1.3), `docs/vv/verification-results.md` (0.32), `docs/requirements/traceability.yaml` / `traceability.md` (SRS-REQ-043), `config/platform/platform_qemu.h`, `cmake/toolchains/rtems-riscv-generic.cmake`, `tools/scripts/qemu_vv.sh`, `tools/scripts/emulated_run.sh`, `src/bsp/mpfs/board.c`, `src/core/pal/pal_rtems.c`, `src/core/pal/pal_posix.c`, `src/transport/link/link.c`, `docs/status/loop-audit.md`, `docs/reviews/README.md`, `docs/reviews/2026-10-10-m10-emulated-r1-r2-r4-review.md` (FRICK-REVIEW-0035 follow-up table); sibling delta reports `2026-10-12-m10-r2-delta-review.md` (FRICK-REVIEW-0125) and `2026-10-12-m10-r4-delta-review.md` (FRICK-REVIEW-0126); residual-gap register (`docs/quality/class-a-gaps.md`, GAP-A1/A2/A4/A5) |
| Date                              | 2026-10-12                             |
| Scope                             | **DELTA RE-REVIEW (norm N12):** whether the fixes to the two Major findings of FRICK-REVIEW-0123 are genuinely closed with evidence, and re-audit of norms **N1–N20** for this change (process/records, tool qualification, traceability/CM of SRS-REQ-043, revision histories, claim discipline, residual-gap register). ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025) — organisation, SPAP/records, tool and tool qualification, supplier/CM, lifecycle. |
| Independence declaration          | The reviewer took no part in authoring, implementing, executing, designing or dispositioning the reviewed items; fresh independent session with **no author context** (task statement + repository only). Read-only for the repository: only this report file was created; **no tracked file was modified**. Host commands were read-only (`trace.py`, `metrics.py --check`, `ctest`, `sha256sum`, `readelf`, `strings`, `grep`); outputs land under git-ignored `build/`. **No docker, Renode or QEMU run was performed** (per task constraints). |
| Norms applied                     | N1–N20 (`docs/rules.md` 1.4); focus N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N15, N16, N17, N19, N20 |

## 1. Scope

This is the norm-N20 / norm-N12 **Product Assurance delta re-review** of the
increment that closes the two **Major** findings of **FRICK-REVIEW-0123**
(PA-01 records/closure gate; PA-03 = adopted R2-01, the T20
`ci:emulated-run` qualification over-claim), plus its Minor/Comment findings
PA-02, PA-04…PA-10, and the specifically flagged T18 toolchain-image
reproducibility caveat.

**Included:** (a) independent confirmation that PA-01 and PA-03 are genuinely
closed — not asserted; (b) re-audit of norms N1–N20 for this change; (c) the
SRS-REQ-043 traceability and CM (manifest ↔ mirror ↔ code/tooling ↔ evidence);
(d) revision-history consistency of the touched documents; (e) claim discipline
(no over-claim of emulation, silicon, qualification or certification);
(f) the residual-gap register (GAP-A1/A2/A4/A5); (g) the records gate: the
review-log rows for the current set must be committed **with** the change (N11).

**Excluded:** the internal code/design adjudication owned by R1 (script/CMake
design) and R2 (V&V method/coverage) — where the closure of those findings is
in scope, this report adopts the sibling delta conclusions
(**R2 FRICK-REVIEW-0125**, **R4 FRICK-REVIEW-0126**) rather than re-deriving
them; the Lean model (N1 satisfied by explicit `lean_note`); a Renode/QEMU/docker
re-run (not executed per the task constraints; N18 is unchanged by this
increment). This review issues **no** certification, approval, qualification,
residual-risk or release statement.

**Method.** Verified the frozen baseline by `sha256sum`; re-ran the project's
gates read-only (`trace.py`, `metrics.py --check`, `ctest` on `build/native`);
parsed the ELF header and `strings` of the QEMU image; inspected the recorded
QEMU console capture against the claims; read the full closure delta
(`git diff`, untracked files) and the sibling delta reports; and audited the
review-log/revision-history state to determine whether the N12/N11 records
obligations are met at the reviewed revision.

## 2. References

- Project norms: `docs/rules.md` (N1–N20), especially N4, N5, N9, N10, N11, N12,
  N13, N15, N16, N17, N19, N20.
- Prior PA review under closure: `docs/reviews/2026-10-12-m10-pa-review.md`
  (FRICK-REVIEW-0123, REJECTED, Major PA-01/PA-03).
- Sibling delta re-reviews (same increment): `docs/reviews/2026-10-12-m10-r2-delta-review.md`
  (FRICK-REVIEW-0125, APPROVED WITH COMMENTS, R2-01 closed, new R2-D1/R2-D2) and
  `docs/reviews/2026-10-12-m10-r4-delta-review.md` (FRICK-REVIEW-0126, APPROVED
  WITH COMMENTS, new R4-N1/R4-N2/R4-N3).
- Predecessor M10 review: `docs/reviews/2026-10-10-m10-emulated-r1-r2-r4-review.md`
  (FRICK-REVIEW-0035).
- Records: `docs/requirements/traceability.yaml` / `.md`, `docs/vv/tool-qualification.md`,
  `docs/vv/emulated-target.md`, `docs/vv/verification-plan.md`,
  `docs/vv/verification-results.md`, `docs/status/status.json`,
  `docs/status/loop-audit.md`, `docs/quality/class-a-gaps.md`,
  `CHANGELOG.md`, `README.md`, `docs/reviews/README.md`.
- ECSS-E-ST-40C Rev.1; ECSS-Q-ST-80C Rev.2.

## 3. Verification performed (independent), with results

| # | Gate / check (N) | Exact command or artifact | Result (independently reproduced) |
| - | ---------------- | ------------------------- | --------------------------------- |
| V1 | Frozen-baseline integrity (N4/N11) | `sha256sum` of the 7 frozen files in the brief | **all 7 match**: `87578828…` (traceability.yaml), `54aa5890…` (tool-qualification.md), `cf514729…` (nightly.yml), `84cb2f22…` (status.json), `797769a6…` (emulated-target.md), `caaccaaa…` / `f8cf4ac1…` (results/plan) — the closure delta is coherent on `aee4135` |
| V2 | Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 99 requirements (58 srs / 27 sss / 14 gaps), 51 Lean theorems, **12 CI evidence claims**, exit 0 |
| V3 | Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0** — `metrics panel: current` |
| V4 | Unit suite, native (N6) | `ctest --test-dir build/native` | **19/19 PASS**, 0 failed |
| V5 | QEMU image identity (N10) | `readelf -h` / `strings` of `build/rtems-generic/frick_fsw` | **RISC-V (ELF64 EXEC)**, `e_entry = 0x80000000`; `strings` show `-march=rv64imafdc_zicsr`, BSP path `…/riscv/rv64imafdc`, and the strings `qemu-virt-rtems` / `QEMU virt (generic rv64imafdc RTEMS)`; **no** `PolarFire`/`RTPFS160ZT` string — the image is the generic BSP build (PA-04 fix confirmed at the binary) |
| V6 | QEMU console evidence (N11) | `build/rtems-generic/qemu-console.log` | real boot capture: toolchain image id on line 1, banner `platform : qemu-virt-rtems`, `board: QEMU virt (generic rv64imafdc RTEMS) booting (generic RTEMS BSP)`, `frick-fsw: 6 applications registered`, per-app `started` lines and `TM -> link` frames. Both pass markers present |
| V7 | PA-03 → T20 exercised by CI (N10/N4) | `.github/workflows/nightly.yml` (read), `Makefile` (`qemu`/`qemu-image`), `tools/scripts/setup_rtems.sh` (read-only) | **nightly.yml:57-70** carries `# evidence: ci:emulated-run`, installs `qemu-system-misc`, builds the generic image (`make qemu-image`) and runs the binding check (`make qemu` → `qemu_vv.sh`) with `grep -aq "applications registered"`. `setup_rtems.sh` step 3/3 builds the `riscv/rv64imafdc` BSP the job needs. `qemu_vv.sh` exits **1** when the image is present but banner/count is missing, **0 SKIP** when absent — genuinely falsifiable |
| V8 | T20 evidence-claim wiring (N4/N10) | `grep -rn "# evidence:" .github/workflows .gitea/workflows`; `trace.py::ci_evidence` | nightly claims `ci:emulated-run`; both `ci-emulated-run` (per-commit) and `ci:emulated-run` (nightly) normalise to claim `emulated-run` (trace.py strips `ci:`/`ci-`). T20 mapping now names the nightly job (`tool-qualification.md:45,82`) |
| V9 | Records/closure gate (N9/N11/N12/N20) | `git status`, `docs/reviews/README.md`, FRICK-REVIEW-0035 §6, `docs/reviews/2026-10-12-m10-*` | Log rows for **0120/0121/0122/0123** present (README.md:120-123) and honest (`0121 -> fixed`, `0123 -> fixed`); FRICK-REVIEW-0035 NCs **R1-01…R4-09 all `closed`** with re-verification evidence; **sibling delta reports 0125/0126 exist** (untracked) closing R2-01 and R4-01..08. **Log rows for 0125/0126 (and this 0127) not yet appended** — see §4 |
| V10 | Revision-history / metadata consistency (N8/N16) | read of headers vs `## History` | **loop-audit.md header `Revision 0.10` lagging History `0.11`**; header `Date` still 2026-10-11 on loop-audit / tool-qualification / traceability.md / verification-results while their newest History rows are 2026-10-12; §4 closing sentence of loop-audit still lists **M10** among "tracked" though M10 is closed (adopts R4-N1/R4-N2/R4-N3, R2-D1) |
| V11 | Over-claim discipline (claim discipline) | read of `emulated-target.md`, `verification-results.md`, manifest `lean_note`, README | QEMU scope bounded to boot/PAL/tasking/bring-up, "no MPFS peripherals", "not silicon"; GAP-A5 **open** / partial mitigation. No `silicon-validated`/`certified`/`flight-proven` claim in the delta |
| V12 | Licensing (N13) | `grep -m1 SPDX` on the three new files | `qemu_vv.sh`, `rtems-riscv-generic.cmake`, `platform_qemu.h` all carry `SPDX-License-Identifier: AGPL-3.0-only` — OK |
| V13 | Reviewer read-only constraint | `git status --porcelain` | Only this report added; no tracked file modified by the reviewer |

## 4. Findings

### 4.1 Delta closure table — FRICK-REVIEW-0123 findings

| Prior NC | Severity | Claimed disposition | Independent re-verification | Delta status |
| -------- | -------- | ------------------- | --------------------------- | ------------ |
| **PA-01** | **Major** | Review set logged (README rows); predecessor FRICK-REVIEW-0035 NCs closed with evidence; status.json/loop-audit reconciled; M10 no longer declared done on unclosed records | **Confirmed.** `docs/reviews/README.md:120-123` now log R1/R2/R4/PA (with the honest `REJECTED … -> fixed` wording); FRICK-REVIEW-0035 §6 marks **R1-01…R4-09 all `closed`** with dispositions + evidence; `status.json` `M10: done`, `sprints.done 2`, refreshed note; `loop-audit.md` §1/§4/§5 M10 closed, §2 `T1-T20`. The PA Major itself is closed by this report; the R2 Major by FRICK-REVIEW-0125. | **CLOSED** (residual records item → PA-D3) |
| **PA-02** | Minor | T18 reproducibility caveat recorded in the tool register (clean `make rtems-toolchain` tracked) | **Confirmed documented:** `tool-qualification.md:59-66` states the image was assembled step-by-step and a clean `docker build` was not completed; row T18 now reads `QUALIFIED (see note)`. **Tracking residual:** no owner/milestone anchor (adopts R2-D2) → PA-D2 | **CLOSED with comment** |
| **PA-03** | **Major** | T20 exercised by the nightly workflow; register mapping records the correct basis | **Confirmed.** `nightly.yml:57-70` runs `make qemu-image` + `make qemu` (`qemu_vv.sh`) and guards the app-count marker; `tool-qualification.md:45,82` maps T20 to `ci:emulated-run` **naming the nightly job**; `qemu_vv.sh` is falsifiable (FAIL on missing marker, SKIP never a pass). Independently corroborated by **R2 FRICK-REVIEW-0125 (R2-01 CLOSED)**. | **CLOSED** |
| PA-04 | Minor | QEMU console banner parameterised to a generic platform | **Confirmed.** `config/platform/platform_qemu.h` (`FF_PLATFORM_NAME "qemu-virt-rtems"`, `FF_BOARD_NAME "QEMU virt …"`); `CMakeLists.txt:38-41` defines `FRICK_PLATFORM_QEMU` (no MPFS board module); `board.c` prints the generic banner; the retained console and the ELF `strings` (V5/V6) now show `qemu-virt-rtems`, no PolarFire string | **CLOSED** |
| PA-05 | Minor | status.json updated (M10 done, sprints, note) | **Confirmed.** `status.json` M10 `done`, `sprints.done 2`/`in_progress 1`, refreshed last-cycle note; README panel regenerated (`2/26`) and `metrics.py --check` current. Residual semaphore wording → PA-D4 (Comment) | **CLOSED** |
| PA-06 | Minor | Emulated environment/activity added to `verification-plan.md` (predecessor R4-08) | **Confirmed.** `verification-plan.md:103-111` (environment table) and `:140-146` (`Emulated target (Dem, M10)`) with cross-link to `emulated-target.md`; rev 0.1.3 History row | **CLOSED** |
| PA-07 | Minor | loop-audit §2/§4/§5 reconciled to M10 closed / T20 | **Confirmed.** §2 row `registry T1-T20`; §4 item 3 "QEMU … **closed** (`make qemu`, T20)"; §5 row "closed". Two metadata residuals → PA-D1 (header revision) and PA-D5 (Comment, §4 tracked list) | **CLOSED with comment** |
| PA-08 | Minor | traceability.md SRS-REQ-043 mirror regenerated | **Confirmed.** `traceability.md:105` lists the 4 implementation paths (`emulated_run.sh`, `qemu_vv.sh`, `rtems-riscv-generic.cmake`, `emulated-target.md`) matching `traceability.yaml:412`; rev 0.28 History row | **CLOSED** |
| PA-09 | Comment | Results preamble reworded (regenerable logs); trailing newline | **Confirmed (a/c).** `verification-results.md:14-17` states the console captures are regenerable (untracked) build outputs; `git diff --check` clean and every touched file ends in LF. **(b) residual:** the M10 row still omits the `12 CI claims` count carried by sibling rows → PA-D6 (Comment) | **CLOSED with comment** |
| PA-10 | Comment | QEMU invocation de-duplicated / "binding" wording qualified | **Partly confirmed.** `emulated_run.sh` now runs QEMU via the toolchain image and also greps `applications registered`; `emulated-target.md:52-58` words the check as binding with SKIP when the image is absent. The two scripts still contain separate QEMU invocations (not merged), so the single-source-of-truth aspect of PA-10 is only partially met — observation only | **CLOSED with comment** |

### 4.2 New findings in the delta (raised or adopted)

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| PA-D1 | Minor | `docs/status/loop-audit.md:7` vs `:165`; N16/N8 | **Header revision lags its own History.** The closure appended History row `0.11` (2026-10-12) but the header still reads `Revision 0.10`. (Independently reproduced; identical to **R4-N1**.) | `loop-audit.md:7` (`0.10`) vs `:165` (`0.11`, `2026-10-12`) | Bump the header `Revision` to `0.11` and refresh its `Date` (same commit). |
| PA-D2 | Minor | `docs/vv/tool-qualification.md:59-66`; N10/N19 | **The T18 caveat's "closure action" is disclosed but not tracked with an owner/milestone** (adopts **R2-D2**). N19 requires gaps to be "tracked with an owner and a milestone"; a `grep` finds the caveat only in the register. | `tool-qualification.md:65-66`; negative grep in `loop-audit.md`/`class-a-gaps.md`/`development-plan.md` | Add a tracked entry (owner + milestone) for the clean `make rtems-toolchain` → `make qemu` re-qualification. |
| PA-D3 | Minor | `docs/reviews/README.md`; N11 | **Review-log rows for the delta reports are not yet appended.** README.md has rows through `0123`; the delta reports `0125`/`0126` (and this `0127`) exist on disk but have **no log rows**. N11 requires the review results to be committed with the change; the owner must append the rows and `git add` the currently untracked reports/new files in the same commit. | `docs/reviews/README.md:120-123`; `git status --porcelain docs/reviews/` (0125/0126 untracked, unlogged) | Append log rows for 0125/0126/0127 and stage all review reports + `platform_qemu.h`, `rtems-riscv-generic.cmake`, `qemu_vv.sh` in the closure commit (N11/N7). |
| PA-D4 | Comment | `docs/status/status.json`; `README.md` panel; N8/N19 | **Semaphores contradict the "in closure" note.** `status.json` gates and the generated README panel show `product_assurance_review` and `reviews` **GREEN**, while the note says "reviews FRICK-REVIEW-0120..0123 (R1/R2/R4/PA) **in closure**". The green is honest only once this PA delta passes; at the reviewed revision the semaphore precedes the conclusion. | `status.json:5,36,39`; `README.md:81-85` | Re-assert the semaphores at commit (after the delta set passes), or word the note to make the pre-commit state unambiguous. |
| PA-D5 | Comment | `docs/status/loop-audit.md:129`; N8 | **Stale "tracked" reference.** §4's closing sentence still lists **M10** among "explicitly tracked … (M10/M14/…)" while §1/§4 item 3 record M10 **closed**. (Adopts **R4-N3**.) | `loop-audit.md:129` vs `:33,124-125,141` | Drop `M10` from the tracked list. |
| PA-D6 | Comment | `docs/vv/verification-results.md:45`; N8/N11 | **M10 cycle row omits the `12 CI claims` count** present in every sibling row (a residual of PA-09b), so the row's trace column is not drawn from the same template as its peers. | `verification-results.md:45` vs sibling rows | Add the `12 CI claims` count for uniformity. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### 4.3 Positive observations (no finding)

- **PA-01/PA-03 genuinely closed, not asserted.** The records gate is now
  satisfied by a truthful NC ledger (FRICK-REVIEW-0035 §6) and logged review
  rows, and the T20 qualification basis is now explicitly tied to a real CI
  workflow that can fail — independently reproduced (V2, V7, V8) and
  corroborated by the R2 delta (FRICK-REVIEW-0125 closes R2-01).
- **Product evidence is sound and honest.** The generic `rv64imafdc` image
  (ELF entry `0x80000000`, `-march=rv64imafdc_zicsr`, no MPFS string) boots on
  `qemu-system-riscv64 -M virt`, the console shows the self-identifying generic
  banner and six registered applications, and the toolchain image id is recorded
  for provenance (V5/V6). GAP-A5 is correctly kept open as a partial mitigation.
- **Traceability converged and mirrored.** `trace.py` PASS (99/58/27/14, 51
  theorems, 12 CI claims); SRS-REQ-043 ↔ SSS-REQ-013 parenting with a
  substantive `lean_note`; mirror row matches the manifest (V2, V8, PA-08).
- **CM/licensing intact.** Both new code/build files carry
  `SPDX-License-Identifier: AGPL-3.0-only`; `git diff --check` is clean and every
  touched file ends in LF (V12, PA-09).

## 5. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — **both Major findings (PA-01, PA-03) closed
  with evidence; no Major remains.** Open **Minors** PA-D1, PA-D2, PA-D3 and
  Comments PA-D4…PA-D6 tracked to closure.
- [ ] REJECTED — one or more Major findings

Both Majors of FRICK-REVIEW-0123 are **genuinely closed**:

- **PA-01 (records/closure gate)** — the review set is logged (README.md:120-123,
  honest `REJECTED … -> fixed` wording), the predecessor FRICK-REVIEW-0035 NCs
  **R1-01…R4-09 are closed with dispositions and re-verification evidence**, and
  `status.json`/`loop-audit.md` are reconciled to M10 done. Independently
  reproduced (V9).
- **PA-03 (= R2-01)** — **T20 is now exercised by the nightly workflow**
  (`.github/workflows/nightly.yml:57-70` builds the generic image and runs the
  binding `qemu_vv.sh`, which FAILs on a missing app-count marker), and the tool
  register's evidence-claims mapping names that nightly job
  (`tool-qualification.md:45,82`). Independently reproduced (V7, V8) and
  corroborated by the R2 delta re-review FRICK-REVIEW-0125, which **closes
  R2-01**.

At the **product level** the increment is sound: the QEMU run is a real,
falsifiable, provenance-recorded generic-BSP boot; the platform branch is a
genuine generic platform with no MPFS board module; the local gates pass
(trace 99/58/27/14, ctest 19/19, metrics current); and claim discipline is
preserved (emulation-only, GAP-A5 open). The remaining items are
**records/metadata**, not product defects: the loop-audit header revision
(PA-D1), the T18 caveat's owner/milestone anchor (PA-D2), the review-log rows
for the delta set plus staging of the new files (PA-D3), and three Comments
(PA-D4…PA-D6). None defeats a requirement or a safety barrier; all are correctable
in the closure commit.

**Verdict: APPROVED WITH COMMENTS.**

### 5.1 What a qualified human must still accept (this assistant cannot)

1. **Emulation-only M10 evidence.** SRS-REQ-043 is verified by a boot/registration
   check on an emulated generic RISC-V (`virt`) machine plus the Renode MPFS run.
   There is no real silicon, no MPFS-peripheral verification on the QEMU leg, no
   timing/WCET and no radiation/electrical evidence: the emulated result is a
   **partial mitigation** of GAP-A5 (on-target qualification, blocked on
   hardware). A human must accept that residual for criticality B.
2. **Toolchain-image reproducibility.** A qualified human must accept either a
   completed clean `docker build` of the T18 image or the recorded tracked
   limitation (PA-D2: the limitation is disclosed but its re-qualification action
   still needs an owner/milestone).
3. **T20 qualification basis.** A human must accept the corrected T20 basis: a
   **weekly** nightly exercise of the binding QEMU check (no per-commit run and no
   recorded nightly pass yet); the local run evidence is on file
   (`build/rtems-generic/qemu-console.log`, image id recorded). The independent
   R2 delta (FRICK-REVIEW-0125) is the corroborating V&V adjudication.
4. **Residual class-A gaps.** GAP-A5 (on-target) and GAP-A1 (deep formal
   refinement), GAP-A2 (full PUS-C coverage), GAP-A4 (certified checker/WCET)
   remain open; the M10 closure claims no credit beyond emulated execution.
5. **Commit/release authority and records.** This PA review is an assurance input
   only. A human must ensure, before commit/push (N7), that the open Minors
   (PA-D1/D2/D3, plus sibling R2-D1/R2-D2 and R4-N1/N2/N3) are corrected, the
   review-log rows for 0125/0126/0127 are appended, and the currently untracked
   reports/new files are staged with the change (N11). The decision to commit and
   push to both remotes remains with the project's authorized human/authority.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| PA (N20/N12)  | independent reviewer agent (FRICK-REVIEW-0127) | 2026-10-12 | **APPROVED WITH COMMENTS** — Majors PA-01 and PA-03 closed with evidence; open Minors PA-D1/D2/D3 and Comments PA-D4…PA-D6 tracked to closure |

## 7. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing the
deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| PA-01 | Major (prior) | review set / predecessor NCs (N9/N11/N12/N15/N20) | Set unlogged, open Major, predecessor NCs unclosed, M10 declared done | **closed**: rows 0120-0123 logged; FRICK-REVIEW-0035 R1-01…R4-09 closed with evidence; status.json/loop-audit reconciled; R2 Major closed by FRICK-REVIEW-0125; PA Major by this report | `docs/reviews/README.md:120-123`; `2026-10-10-m10-emulated-r1-r2-r4-review.md:159-172`; `status.json`; `loop-audit.md`; V9 | frick-fsw | **closed** | FRICK-REVIEW-0127 |
| PA-02 | Minor (prior) | T18 reproducibility caveat (N6/N10/N8/N19) | Clean build not completed; caveat undisclosed | **closed with comment**: caveat recorded (`tool-qualification.md:59-66`); tracking anchor missing (PA-D2) | `docs/vv/tool-qualification.md` | frick-fsw | **closed (see PA-D2)** | FRICK-REVIEW-0127 |
| PA-03 | Major (prior) | T20 mapping/qualification (N10/N4/N11) | `ci:emulated-run`→T20 asserted but no CI job ran `qemu_vv.sh` | **closed**: nightly workflow exercises the binding check; mapping names the nightly job; check is falsifiable | `nightly.yml:57-70`; `qemu_vv.sh:29,56`; `tool-qualification.md:45,82`; V7/V8; FRICK-REVIEW-0125 | frick-fsw | **closed** | FRICK-REVIEW-0127 |
| PA-04 | Minor (prior) | QEMU console banner (N8) | MPFS/flight banner for a generic-BSP run | **closed**: generic `qemu-virt-rtems` platform/banner; no MPFS string in the image | `platform_qemu.h`; `board.c`; console log; ELF strings (V5/V6) | frick-fsw | **closed** | FRICK-REVIEW-0127 |
| PA-05 | Minor (prior) | `status.json` currency (N8/N16/N19) | M10 "in progress"; stale note/sprints | **closed**: M10 done; sprints 2/1; note refreshed; panel current | `status.json`; `metrics.py --check` | frick-fsw | **closed** | FRICK-REVIEW-0127 |
| PA-06 | Minor (prior) | `verification-plan.md` (N8/N15) | Emulated path absent from the plan | **closed**: environment + activity rows added | `verification-plan.md:103-111,140-146` | frick-fsw | **closed** | FRICK-REVIEW-0127 |
| PA-07 | Minor (prior) | `loop-audit.md` internal consistency (N8/N19) | §2 "T1-T19", §4/§5 QEMU pending/tracked | **closed with comment**: §2/§4/§5 reconciled; residual header revision (PA-D1) and listed M10 (PA-D5) | `loop-audit.md` | frick-fsw | **closed (see PA-D1/PA-D5)** | FRICK-REVIEW-0127 |
| PA-08 | Minor (prior) | `traceability.md` mirror (N4/N8) | SRS-REQ-043 row stale | **closed**: mirror row lists the 4 paths; rev 0.28 | `traceability.md:105,216` | frick-fsw | **closed** | FRICK-REVIEW-0127 |
| PA-09 | Comment (prior) | results preamble/row (N8/N11) | "no external state" vs untracked console; missing count; newline | **closed with comment**: preamble reworded; LF clean; M10 row count still missing (PA-D6) | `verification-results.md:14-17`; `git diff --check` | frick-fsw | **closed (see PA-D6)** | FRICK-REVIEW-0127 |
| PA-10 | Comment (prior) | `emulated_run.sh` vs `qemu_vv.sh` (N10) | Duplicated invocation; "binding" wording | **closed with comment**: invocation aligned + app-count grepped; scripts not fully merged (observation) | `emulated_run.sh`; `emulated-target.md:52-58` | frick-fsw | **closed (observation)** | FRICK-REVIEW-0127 |
| PA-D1 | Minor (new) | `loop-audit.md` header vs History (N16/N8) | Header `Revision 0.10` lags History `0.11` | open (= R4-N1) | `loop-audit.md:7` vs `:165` | frick-fsw | open | — |
| PA-D2 | Minor (new) | T18 note / N19 | Caveat disclosed but closure action not tracked (no owner/milestone) | open (= R2-D2) | `tool-qualification.md:59-66` | frick-fsw | open | — |
| PA-D3 | Minor (new) | `docs/reviews/README.md` (N11) | Log rows for 0125/0126/0127 not appended; reports/new files untracked | open | `README.md:120-123`; `git status` | frick-fsw | open | — |
| PA-D4 | Comment (new) | status/README semaphores (N8/N19) | GREEN semaphores while review set "in closure" | open | `status.json:5,36,39`; `README.md:81-85` | frick-fsw | open | — |
| PA-D5 | Comment (new) | `loop-audit.md:129` (N8) | §4 tracked-list still names M10 though closed | open (= R4-N3) | `loop-audit.md:129` | frick-fsw | open | — |
| PA-D6 | Comment (new) | `verification-results.md:45` (N8/N11) | M10 row omits the `12 CI claims` count | open | `verification-results.md:45` | frick-fsw | open | — |

> Note (N11/N20): this report is read-only output; the owner must append its row
> (and the R2/R4 delta rows 0125/0126) to `docs/reviews/README.md` and stage the
> currently untracked reports and new files in the same commit. This report does
> not modify any tracked file.

## 8. Norm-by-norm audit (N1–N20) for this delta re-review

| Norm | Assessment | How established |
| ---- | ---------- | --------------- |
| N1 (Lean) | **N/A / satisfied** | No Lean artifact touched; SRS-REQ-043 keeps an explicit `lean_note` (trace PASS, 51 theorems) |
| N2 (English) | **Pass** | All new/touched content read — English only |
| N3 (SRS/SSS backwards) | **Pass** | SRS-REQ-043 present; parent SSS-REQ-013 linked; manifest ↔ mirror consistent |
| N4 (bidirectional traceability) | **Pass** | `trace.py` PASS 99/58/27/14, 12 CI claims; mirror row regenerated (PA-08); T20 claim wired to a real workflow (V7/V8) |
| N5 (class gaps) | **Pass** | GAP-A5 open/partial mitigation; GAP-A1/A2/A4 open and consistent; no credit over-claimed |
| N6 (V&V/deploy) | **Pass with caveat** | QEMU run real; Docker deployability of T18 not clean-built (PA-D2) |
| N7 (dual push) | **Pending** | Increment uncommitted (expected — pre-commit review); new files not yet staged (PA-D3) |
| N8 (docs current) | **Gap (Minor)** | PA-D1 (header revision), PA-D2 (tracking), PA-D4 (semaphores), PA-D5 (stale list), PA-D6 (count); most docs updated |
| N9 (independent review) | **Pass (closing)** | R1/R2/R4 reports + delta reports exist; R2-01 closed by FRICK-REVIEW-0125; PA Major closed here; rows to be logged (PA-D3) |
| N10 (tool qualification) | **Pass** | T20 exercised by the nightly; T18 `QUALIFIED (see note)`; scripts under CM (SPDX); T18 caveat tracking residual (PA-D2) |
| N11 (scripts/records in repo) | **Gap (Minor)** | New files untracked pre-commit and delta log rows pending (PA-D3) — owner action at commit |
| N12 (NC resolution) | **Pass** | Prior Majors PA-01/PA-03 and R2-01 closed with re-verification; Minors/Comments tracked; review conclusion for the deltas APPROVED WITH COMMENTS (no Major open) |
| N13 (licensing) | **Pass** | SPDX `AGPL-3.0-only` on all three new files (V12) |
| N14 (SSS systems review) | **N/A** | SSS-REQ-013 text unchanged → no R3 trigger |
| N15 (full cycle) | **Pass** | Local gates green; applicable reviews produced and Majors closed; findings tracked to closure |
| N16 (revision history) | **Partial** | History rows added; loop-audit header revision + header dates stale (PA-D1 and sibling R2-D1/R4-N2) |
| N17 (documentation review) | **Pass (closing)** | R4 delta FRICK-REVIEW-0126 closes R4-01..R4-08; new R4-N1/N2/N3 tracked; rows to be logged (PA-D3) |
| N18 (Renode) | **Pass (unchanged)** | Increment does not touch the Renode path; prior-cycle Renode PASS stands (not re-run here per constraints) |
| N19 (loop audit) | **Partial** | loop-audit M10 closed; header revision (PA-D1), tracked-list (PA-D5), T18 tracking (PA-D2), status semaphores (PA-D4) |
| N20 (PA review) | **Pass (this report)** | This delta re-review (FRICK-REVIEW-0127) closes the PA gate; PA-D1…PA-D6 tracked to closure |

## 9. Evidence consulted

- Norms: `docs/rules.md` (1.4); template `docs/reviews/review-template.md`.
- Frozen baseline: `sha256sum` of the 7 frozen files (all match, V1).
- Increment delta: `git diff` (27 files) + untracked `config/platform/platform_qemu.h`,
  `cmake/toolchains/rtems-riscv-generic.cmake`, `tools/scripts/qemu_vv.sh`.
- Requirements: `docs/requirements/SRS.md` (SRS-REQ-043), `SSS.md` (SSS-REQ-013),
  `traceability.yaml`, `traceability.md` (mirror).
- V&V/records: `docs/vv/emulated-target.md` (0.3), `docs/vv/tool-qualification.md`
  (0.3.16), `docs/vv/verification-plan.md` (0.1.3), `docs/vv/verification-results.md`
  (0.32), `docs/quality/class-a-gaps.md` (GAP-A1/A2/A4/A5), `build/rtems-generic/qemu-console.log`,
  `build/rtems-generic/frick_fsw` (ELF header + strings).
- Process/status: `CHANGELOG.md`, `README.md`, `AGENTS.md`, `docs/development-plan.md`
  (0.18), `docs/planning/sprint-plan.md` (0.3), `docs/status/handover.md` (1.7),
  `docs/status/loop-audit.md` (0.11), `docs/status/status.json`.
- Reviews: `docs/reviews/README.md`; `docs/reviews/2026-10-12-m10-pa-review.md`
  (FRICK-REVIEW-0123, full); `docs/reviews/2026-10-12-m10-r2-delta-review.md`
  (FRICK-REVIEW-0125); `docs/reviews/2026-10-12-m10-r4-delta-review.md`
  (FRICK-REVIEW-0126); `docs/reviews/2026-10-10-m10-emulated-r1-r2-r4-review.md`
  (FRICK-REVIEW-0035 §6).
- CI: `.github/workflows/nightly.yml`, `.github/workflows/ci.yml`,
  `.gitea/workflows/ci.yml`; `Makefile` (`qemu`/`qemu-image`);
  `tools/scripts/setup_rtems.sh`, `tools/scripts/qemu_vv.sh`, `tools/scripts/emulated_run.sh`.
- Commands re-executed (read-only): `trace.py`, `metrics.py --check`,
  `ctest --test-dir build/native`, `sha256sum`, `readelf`, `strings`, `grep`,
  `git status`/`git diff --check`.

---

Committed and pushed with the reviewed change (norm N11) — PA Majors PA-01/PA-03
closed; open Minors PA-D1/D2/D3 and Comments PA-D4…PA-D6 tracked to closure.
