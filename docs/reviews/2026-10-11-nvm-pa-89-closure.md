# Review Report — Product Assurance closure review of PA-89-01 (eNVM NVM increment)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0090                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | PA (Senior SW Product Assurance engineer; norm N20) |
| Reference revision                | commit `eff09cf` ("NVM: use the writable eNVM data region for a real Renode write/read check (PA-89-01)"), branch `main`, pushed to `origin` and `gitea` |
| Items under review                | Closure of the lone residual **Minor PA-89-01** raised by FRICK-REVIEW-0089 (`docs/reviews/2026-10-11-nvm-pa-final-review.md`) against the SRS-REQ-055 reference eNVM NVM increment. Products: `config/platform/platform_mpfs.h`; `src/apps/app_renode_test.c` (`renode_hal_nvm_check`, `mpfs_envm_read8/write8`); `src/drivers/nvm/mpfs/ff_nvm_mpfs.{c,h}`; records `CHANGELOG.md`, `docs/vv/verification-results.md` (0.26), `README.md`, `docs/status/handover.md` (1.3); plus the traceability manifest `docs/requirements/traceability.yaml` and the tool register `docs/vv/tool-qualification.md` as adjacent records. |
| Date                              | 2026-10-11                             |
| Scope                             | Whether PA-89-01 is closed at HEAD `eff09cf` and whether the **N20 gate for SRS-REQ-055** remains satisfied: the correctness of the new eNVM base and write/read self-test, the accuracy of the code comments and the enumerated records, the honesty of the fresh Renode `hal_nvm=PASS` evidence, and a re-run of the project gates. ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025). |
| Independence declaration          | Reviewer took no part in authoring, implementing, executing or reviewing the items; fresh independent session with no prior author context. Read-only for the repository: only this report file was created; no tracked file was modified (working tree clean before and after). Gates were executed read-only (outputs under the git-ignored `build/`); the Renode probe ran in an ephemeral container on the pinned image. |
| Norms applied                     | N1–N20 (see `docs/rules.md`); focus N4, N8, N9, N10, N11, N12, N15, N16, N17, N18, N20 |

## 1. Scope

This is the norm-N20 Product Assurance **closure** review of the single residual
Minor (PA-89-01) left open by the PA final review FRICK-REVIEW-0089 of the
milestone M4/M11 reference eNVM NVM backend (SRS-REQ-055). FRICK-REVIEW-0089
judged that the justification recorded for the Renode `hal_nvm` clause was
imprecise: it asserted that "the emulated eNVM region is read-only" and that a
write self-test "faults" on the model, whereas an independent probe showed the
driver's base `0x20200000` is the `envmCfg` configuration block (writes dropped
with a warning, not a fault) and the eNVM **data** array at `0x20220000` is
writable. The required remedy was to correct the wording to the measured
behaviour, record which region the check exercises, and either archive the
failed-run console log under the pinned image digest or document the digest used.

**Included:** the disposition of PA-89-01 at HEAD `eff09cf`; the correctness of
the new base address and the `FF_NvmMpfsSelfTest` write/read path; the accuracy
of the code comments and of the enumerated records
(`CHANGELOG`/`verification-results`/`README`/`handover`); an independent probe of
the exact addresses the new check uses on the pinned Renode image; the freshness
and content of the recorded Renode run; and a re-run of the project gates.

**Excluded:** the internal code correctness owned by R1; the V&V adequacy owned
by R2; the documentation adjudication owned by R4; the Lean proofs; and
on-target/silicon behaviour, which is GAP-A5 by the increment's own scope. This
review does not re-open the dispositions already made in FRICK-REVIEW-0089 except
as they bear on PA-89-01.

**Method.** Read the HEAD diff and every in-scope artifact; independently
executed the project gates; reconciled the recorded numbers against measured
ones; inspected the repository/remote state; and independently probed the pinned
Renode emulated eNVM platform at the addresses the new self-test uses.

## 2. References

- Review under closure: `docs/reviews/2026-10-11-nvm-pa-final-review.md`
  (FRICK-REVIEW-0089, Minor PA-89-01).
- Corrective commit: `eff09cf` (HEAD).
- Project norms: `docs/rules.md` (N1–N20); entry point `AGENTS.md`.
- Traceability manifest/gate: `docs/requirements/traceability.yaml`;
  `tools/scripts/trace.py`.
- HAL contract: `src/hal/hal_mem.h` (`FF_Nvm_t`, SRS-REQ-032).
- Renode platform: `tools/docker/renode/frick-renode.resc` (loads
  `@platforms/cpus/polarfire-soc.repl` from the digest-pinned `antmicro/renode`
  image) and `tools/docker/renode/Dockerfile` (pin
  `sha256:7905c329…`).
- Records: `docs/vv/verification-results.md` (0.26), `docs/vv/tool-qualification.md`,
  `CHANGELOG.md`, `README.md`, `docs/status/handover.md` (1.3).
- ECSS-E-ST-40C Rev.1 (30 Apr 2025); ECSS-Q-ST-80C Rev.2 (30 Apr 2025) — tool
  qualification, product-assurance records, traceability matrices.

## 3. Verification performed (independent)

| # | Check | Command / artifact | Result |
| - | ----- | ------------------ | ------ |
| 1 | eNVM base = data region | `config/platform/platform_mpfs.h:45-48` | **`FF_ENVM0_BASE 0x20220000u`**; comment states "DATA region (writable) … config/control block is at 0x20200000" |
| 2 | Renode check is a write/read self-test | `src/apps/app_renode_test.c:210-238`; `ff_nvm_mpfs.c:72-84` | `renode_hal_nvm_check` now calls `FF_NvmMpfsSelfTest(&drv)`; the self-test writes `1`/`0xA5` at `FF_NVM_SCRATCH_OFF` (=1024) and reads `0xA5` back — a discriminating MMIO round trip, not a presence read |
| 3 | Scratch slot is outside the id table | `ff_nvm_mpfs.h:31-35` | `FF_NVM_SCRATCH_OFF = FF_NVM_MAX_IDS*FF_NVM_SLOT_SIZE = 32*32 = 1024`; `0x20220000+1024 = 0x20220400`, beyond the 1 KiB slot table |
| 4 | Code comments accurate | `app_renode_test.c:220-226`; `ff_nvm_mpfs.h:47-48` | No "read-only"/"faults" claim remains; comment says the DATA region at `0x20220000` is writable and the config block is not; header says the DATA region is writable and used by the Renode check |
| 5 | Records accurate | `git show eff09cf` on `CHANGELOG.md`, `docs/vv/verification-results.md`, `README.md`, `docs/status/handover.md` | All four describe an "eNVM data write/read check" (the four records the task enumerated) |
| 6 | Trace convergence (N4) | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 97 requirements (56 srs / 27 sss / 14 gap), 51 Lean theorems (exit 0) |
| 7 | Metrics-panel currency (N8) | `python3 tools/scripts/metrics.py --check` | **exit 0** — "metrics panel: current" |
| 8 | Unit suite (N6) | `ctest --test-dir build/native` | **17/17 PASS** (incl. `nvm`) |
| 9 | Repository/remote state (N7) | `git branch -avv`, `git status --short` | HEAD `eff09cf` **=** `origin/main` **=** `gitea/main`; **clean** working tree |
| 10 | Fresh Renode run (N18) | `build/renode/console.log`; ELF; `git log -1 --format=%ci` | console **18:28:40**, ELF **18:28:25**, commit **18:29:31** — contemporaneous with HEAD content; **`hal_nvm=PASS`**, **`RENODE_TEST: PASS`**; no `Unhandled write` around the clause |
| 11 | Built base address | `strings`/byte scan of `build/renode/frick_fsw_renode.elf` | ELF contains `0x20220000` (new base); the pre-change base build had it absent by construction |
| 12 | Independent eNVM probe (pinned image) | ephemeral `antmicro/renode@sha256:7905c329…` (config id `e278e663…`), `polarfire-soc.repl`, monitor `WriteByte`/`ReadByte` | `envmCfg` `0x20200000` write `0xAA` → "Unhandled write … Unhandled bits" (**dropped**); `envmData` allocates a **64 KiB** segment and **accepted writes at `0x20220000`, `0x20220400`, `0x20220401`** (the exact scratch addresses the self-test uses) — confirming the Data region is writable and the scratch offset is in range |
| 13 | Pinned-image identity (0089 detection limit) | `docker images --digests antmicro/renode`; `docker inspect …@sha256:7905c329…` | local `antmicro/renode:latest` DIGEST = `sha256:7905c329…` = the Dockerfile pin; image id `e278e663…`. The prior "local ≠ pinned" note (0089) compared a config id against a manifest digest; the probe **did** run on the pinned image |
| 14 | Adjacent record scan (N8/N11/N16) | `grep -rn "presence" docs/`; `git show --stat eff09cf` | `docs/requirements/traceability.yaml:523` `lean_note` still says "MMIO presence check"; `tool-qualification.md` and `renode-environment.md` not touched at HEAD |

## 4. Disposition of PA-89-01

**PA-89-01 — CLOSED.**

FRICK-REVIEW-0089 required three remedies for the imprecise justification:

1. **Correct the wording to the measured behaviour** — done. The comment and the
   header no longer assert "read-only" or "faults"; they state that the DATA
   region at `0x20220000` is writable and the config block at `0x20200000` is
   not (checks 1, 4), matching the independent probe (check 12).
2. **Record which region the check exercises** — done. The check now targets the
   writable data region and its comment names `0x20220000`; the platform header
   documents the config block at `0x20200000` (checks 1, 4).
3. **Archive the failed-run log under the pinned digest, or document the digest**
   — **moot**. The read-presence justification the failed-run log would have
   supported was removed entirely: the clause is now a *discriminating*
   write/read round trip (check 2), so no failed-run log is applicable. The
   digest concern is also resolved: the independent probe ran on the pinned
   image (`sha256:7905c329…`, check 13), removing the detection limit stated in
   FRICK-REVIEW-0089.

The remedy therefore exceeds the minimum: instead of merely re-wording a
read-presence check, the increment replaced it with a real MMIO write/read
round trip and kept the honest scope statement (on-target flash
write/endurance/retention remains GAP-A5). The fresh Renode run is
contemporaneous with HEAD and shows `hal_nvm=PASS` (check 10), and the enumerated
records describe the check accurately (check 5). PA-89-01 is **closed with no
carry-over**.

Two of the prior advisory Comments are also now addressed at HEAD:
**PA-89-05** (CHANGELOG not updated) — the commit updates `CHANGELOG.md` to the
data-region wording (check 5) → **closed**; and **PA-89-04/D1** (the header claim
about a Renode caller) remains correct.

## 5. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| **PA-90-01** | **Minor** | `docs/requirements/traceability.yaml:523`; SRS-REQ-055 (N4/N8/N11/N16) | **The traceability manifest's `lean_note` for SRS-REQ-055 still describes the Renode evidence as an "MMIO presence check on the emulated SoC (hal_nvm)".** After `eff09cf` the clause is a data-region **write/read** round trip (checks 2, 4), so the manifest under-states the realised evidence. This is a controlled requirement record (the N4 source of truth) that now misdescribes the verified behaviour; it was not updated in the same commit (norm N8). Non-derogating: the manifest's structural mapping (implementation/test links to `ff_nvm_mpfs.c/h`, `platform_mpfs.h`, `test_nvm.c`, `ci:renode`) is correct and the `trace.py` gate passes, so the requirement is not invalidated. | `traceability.yaml:523`; `git show eff09cf -- docs/requirements/traceability.yaml` (untouched); checks 4–6 | Correct the `lean_note` to "bounded slot table; host-tested register logic; Renode eNVM data-region write/read check (`hal_nvm`)". |
| PA-90-02 | Comment | `docs/vv/tool-qualification.md` (N10/N16) | The Renode testware `src/apps/app_renode_test.c` (listed under T17) changed behaviour at HEAD (presence → data write/read), but the tool register gained no re-qualification History row; `AGENTS.md` §5.3 asks for a register update when a V&V script/tool changes. Non-derogating: T17's description already lists `hal_nvm` and the check count is unchanged, so the qualification basis (reviewed + CM) still holds. | `git show --stat eff09cf` (no `tool-qualification.md`); `tool-qualification.md:41,108` | Add a T17 History row (e.g. 0.3.11) noting the `hal_nvm` check is now a data-region write/read round trip. |
| PA-90-03 | Comment | `docs/status/handover.md` (N16) | The handover body line `:101` was re-worded ("eNVM MMIO presence check" → "eNVM data write/read check") while the document stays at Revision 1.3 with History row 1.3; FRICK-REVIEW-0088 DEL-1 established that a change to the handover content warrants a History row/bump. The counts are unchanged, so this is a bookkeeping nuance, not a stale count. | `handover.md:7,101,186`; `git show eff09cf -- docs/status/handover.md` | Either add a 1.4 History row for the wording change, or accept that the refinement falls within the 1.3 NVM increment (record the decision). |
| PA-90-04 | Comment | `docs/vv/renode-environment.md` (N10) | Carry-over of PA-89-02: the Renode environment note still mentions eNVM only in the platform list (`:25`) and does not document the `hal_nvm` check's scope/emulator dependency. Now that the check is a real write/read, a short note that it relies on the emulated `envmData` region being writable (64 KiB at `0x20220000`) would preserve the evidence-scope record. | `renode-environment.md:25`; `git show --stat eff09cf` (untouched); check 12 | Add a one-paragraph scope/dependency note. |
| PA-90-05 | Comment | `.github/workflows/ci.yml:80`, `.gitea/workflows/ci.yml:83` (N6) | Carry-over of PA-89-03: the `ci-renode` provenance parentheticals still omit `SRS-REQ-055` (they list `SRS-REQ-036…050`, `SSS-REQ-013`, `SSS-REQ-020`). The gate checks only claim-name existence, so non-blocking. | `ci.yml:80` | Add `SRS-REQ-055` to both parentheticals when the workflows are next touched. |
| PA-90-06 | Comment | `docs/vv/tool-qualification.md:95,97` (N11/N16) | Carry-over of PA-89-04: stale counts — T3 history cites `test_gpio` "21 checks" (actual 30) and `test_mem` "20 checks" (actual 23). Pre-existing and non-derogating. | register history rows | Correct when the register is next touched. |
| PA-90-07 | Comment | committed FRICK-REVIEW-0081/0082/0083 (N12) | Carry-over of PA-89-06: the parent R1/R2/R4 reports were committed as snapshots with NC rows still "open"; the real closures live in the delta reports 0086/0087/0088. Bookkeeping, not a live NC. | committed parent tables | Optionally annotate the parent tables ("closed via FRICK-REVIEW-008X"). |

**Severity definition** (from `review-template.md`): **Major** = non-conformity
that disqualifies the item for the applicable criticality, or a defect that can
defeat a requirement or a safety barrier; **Minor** = non-conformity that does
not invalidate the requirement but must be corrected or justified; **Comment** =
observation/recommendation, no derogation.

**No Major finding** is raised. The single Minor (PA-90-01) is a record-accuracy
defect adjacent to PA-89-01's subject matter; it does not re-open PA-89-01 (whose
specific remedies are discharged, §4) and does not affect the N20 gate.

## 6. Process-obligation assessment

- **N18 (Renode).** **Met.** The gate is in force; the fresh run at HEAD
  (`hal_nvm=PASS`, `RENODE_TEST: PASS`) is contemporaneous with the commit, and
  the `hal_nvm` clause is now a *discriminating* round trip folded into the
  `RENODE_TEST: PASS` conjunction, so a regression fails the gate. The check
  exercises the writable data region (independently probed, check 12).
- **N4 (trace convergence).** **Met** structurally — `trace.py` PASS
  97/56/27/14/51; SRS-REQ-055 is wired to SSS-REQ-020 with implementation + test
  evidence. The manifest `lean_note` prose is stale (PA-90-01), but the
  structural mapping is sound.
- **N8/N11/N16 (records/revision history).** **Substantially met.** The four
  enumerated records (`CHANGELOG`/`verification-results`/`README`/`handover`)
  describe the new behaviour accurately; residuals are PA-90-01 (manifest note),
  PA-90-02 (tool register row) and PA-90-03 (handover bump), all non-derogating.
- **N10 (tool register).** **Met with an advisory gap** (PA-90-02); T3/T17 still
  list `test_nvm` (36 checks) and `hal_nvm`; the caller/header claim is correct.
- **N15 (full cycle).** **Met.** All execution gates were re-run green:
  `trace` 97/56/27/14/51, `metrics --check` exit 0, `ctest` 17/17, and the fresh
  Renode run PASS.
- **N17 (documentation review).** **Met** (unchanged from FRICK-REVIEW-0089; the
  R4 delta has no outstanding Minor).
- **N20 (PA review).** **Met.** The lone residual Minor (PA-89-01) is closed; one
  new Minor (PA-90-01) and advisory Comments are tracked.

## 7. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — PA-89-01 is closed; one new Minor and advisory Comments remain tracked
- [ ] REJECTED — one or more Major findings

**PA-89-01 is CLOSED.** Independently verified at HEAD `eff09cf`:

- `FF_ENVM0_BASE` is the writable eNVM **data** region `0x20220000`, and the
  config/control block `0x20200000` is documented as such (checks 1, 4);
- the `hal_nvm` clause is now a real driver **write/read** self-test on the
  scratch region (`0x20220400`/`0x20220401`), outside the slot table (checks 2,
  3), and the code no longer contains the imprecise "read-only / faults" claim;
- the enumerated records (`CHANGELOG`, `verification-results`, `README`,
  `handover`) describe the check accurately (check 5);
- the fresh Renode run at HEAD shows `hal_nvm=PASS` / `RENODE_TEST: PASS`, and an
  independent probe of the **pinned** image confirms the data region accepts
  writes at the self-test's exact addresses while the config block drops them
  (checks 10–13);
- all automated gates converge: `trace` 97/56/27/14/51, `metrics --check` exit 0,
  `ctest` 17/17; HEAD is pushed to both remotes with a clean tree (checks 6–9).

Against that, one **new Minor (PA-90-01)** — the stale `lean_note` for
SRS-REQ-055 in the traceability manifest — and six advisory Comments remain, none
of which invalidates SRS-REQ-055 or defeats a safety barrier considered in scope.
The verdict is therefore **APPROVED WITH COMMENTS**.

**N20 gate for the NVM increment (SRS-REQ-055) — REMAINS SATISFIED.** There is
**no Major** in PA scope, the prior Major (FRICK-REVIEW-0084 F1) is closed, the
lone residual Minor (PA-89-01) is now closed, and the full cycle converges at
HEAD. The residual Minor (PA-90-01) and the advisory Comments are tracked for
closure and do not invalidate the increment; under the project's precedent
(FRICK-REVIEW-0085/0089) a pass-with-tracked-Minors is a pass. The human
release/PA authority retains the decision to accept the residual Minor.

## 8. Confidence

```text
Decision/product and configuration: frick-fsw @ eff09cf; PA closure review of the
  lone residual Minor PA-89-01 (FRICK-REVIEW-0089) for the SRS-REQ-055 reference
  eNVM NVM backend + Renode hal_nvm clause; ECSS criticality B target.
Claim type: mixed (Fact for source/record contents, gate results, repository state
  and the Renode probe; Inference for the PA-89-01 closure judgement and the
  PA-90-01 severity).
Applicable baseline and approved tailoring: project norms N1-N20; ECSS-E-ST-40C Rev.1
  (30 Apr 2025); ECSS-Q-ST-80C Rev.2 (30 Apr 2025). Customer tailoring matrix not
  available to this reviewer (unchanged from the PA-67/0084/0085/0089 assessments).
Evidence consulted: HEAD diff and message; platform_mpfs.h; app_renode_test.c;
  ff_nvm_mpfs.{c,h}; CHANGELOG.md; verification-results.md (0.26); README.md;
  handover.md (1.3); traceability.yaml; tool-qualification.md; renode-environment.md;
  live gates (trace.py, metrics.py --check, ctest); git refs/remotes/status;
  build/renode/console.log + ELF; an ephemeral Renode probe of
  config/platform/polarfire-soc.repl on the pinned antmicro/renode image.
Cross-check and reconciliation: code comment vs probe (data region writable, config
  block drops writes); trace 97/56/27/14/51 == recorded; ctest 17/17; console 18:28
  vs ELF 18:28 vs commit 18:29; local HEAD == origin/gitea; Dockerfile pin
  7905c329 == local image manifest digest (resolving the 0089 detection limit).
Confidence: High that PA-89-01 is closed and the N20 gate remains satisfied; High
  for the source/record contents and the Renode probe facts; Medium for the
  PA-90-01 severity (a record-accuracy Minor vs an advisory Comment).
Basis and detection limits: full diff and all in-scope records read; gates executed;
  the probe used the pinned image and the exact self-test addresses. The probe was a
  monitor-level write/read of the emulated peripheral, not the full firmware run; the
  firmware-run evidence is the recorded console.log, contemporaneous with HEAD.
Missing/inaccessible evidence: customer tailoring matrix; the RTEMS cross/on-target
  results (re-used as recorded, not re-run); the Renode image provenance beyond the
  locally verified digest.
Assumptions and expiry/recheck date: assumes the task instruction to modify only this
  report is authoritative. Recheck on the next corrective commit (PA-90-01) or
  2027-04-11.
Consequences if wrong: if PA-90-01 is judged a Comment rather than a Minor, the
  verdict becomes APPROVED; the PA-89-01 closure and the N20-gate statement are
  unaffected. Authorised approver: project engineering/PA/release authority; this
  agent cannot approve, close NCs, waive findings, or accept residual risk.
```

## 9. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| **PA (N20)** | **independent agent, FRICK-REVIEW-0090** | **2026-10-11** | **APPROVED WITH COMMENTS — PA-89-01 CLOSED (1 Minor PA-90-01 + advisory Comments; N20 gate for SRS-REQ-055 remains SATISFIED)** |

## 10. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| **PA-89-01** | **Minor** | app_renode_test.c; commit msg; traceability region; N8/N10/N11/N18 | read-only/faults justification inexact; region was `envmCfg` not storage; no archived failed-run log | **corrected** — base moved to the data region; check is a real write/read self-test; wording corrected in code + enumerated records; failed-log remedy moot | checks 1–5, 10–13 | frick-fsw | **closed** | FRICK-REVIEW-0090 |
| PA-89-05 | Comment | CHANGELOG.md | not updated at HEAD (0089) | addressed — CHANGELOG now records the data-region write/read check | check 5 | frick-fsw | **closed** | FRICK-REVIEW-0090 |
| **PA-90-01** | **Minor** | traceability.yaml:523 (SRS-REQ-055) | `lean_note` still says "MMIO presence check" | correct the note to a data-region write/read check | manifest re-read | frick-fsw | **open** | pending |
| PA-90-02 | Comment | tool-qualification.md (N10) | no re-qualification row for the changed T17 testware | add a T17 History row | register re-read | frick-fsw | **open (advisory)** | pending |
| PA-90-03 | Comment | handover.md (N16) | content re-worded without a History bump | add 1.4 row or record the decision | handover re-read | frick-fsw | **open (advisory)** | pending |
| PA-90-04 (= PA-89-02) | Comment | renode-environment.md | presence/write-read scope + emulator dependency not documented | add a scope paragraph | doc re-read | frick-fsw | **open (advisory)** | pending |
| PA-90-05 (= PA-89-03) | Comment | CI provenance | `SRS-REQ-055` omitted | add to both workflows | `ci.yml` re-read | frick-fsw | **open (advisory)** | pending |
| PA-90-06 (= PA-89-04) | Comment | tool register counts | gpio/mem stale | correct when next touched | register re-read | frick-fsw | **open (advisory)** | pending |
| PA-90-07 (= PA-89-06) | Comment | parent R1/R2/R4 reports | NC tables left all-open | annotate closures | parent report re-read | frick-fsw | **open (advisory)** | pending |

---

This report is the norm-N20 closure review of PA-89-01 at `eff09cf`. It modifies
no repository file other than itself. The `docs/reviews/README.md` review-log row
and the `docs/vv/verification-results.md` cycle-log row for FRICK-REVIEW-0090 are
to be added by the integrator (norm N11), and the file is to be committed and
pushed with the change (norm N7).