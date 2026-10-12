# Review Report

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0149                      |
| Reviewer session                  | independent agent (norm N9), fresh context |
| Reviewer profile                  | **R1+R2+R3+R4+PA (delta)** — R1 (Senior FSW / real-time / space software developer) + R2 (Senior SW V&V engineer) + R3 (Senior avionics / systems engineer, N14) + R4 (Senior SW documentation engineer, N17) + PA (Senior SW Product Assurance engineer, N20) |
| Reference revision                | `56ada82` (HEAD) + **uncommitted** Sprint-6a working-tree increment (GEM native TSN peripheral-bound, SRS-REQ-038) — delta re-review of FRICK-REVIEW-0148 |
| Items under review                | The corrective delta since FRICK-REVIEW-0148: `src/apps/app_renode_test.c` (`renode_hal_tsn_check`, `hal_tsn` aggregate term); `docs/vv/{renode-environment.md,verification-results.md,tool-qualification.md}`; `docs/requirements/{traceability.yaml,traceability.md}`; `docs/planning/sprint-plan.md`; `docs/development-plan.md`; `docs/status/{handover.md,loop-audit.md,status.json}`; `CHANGELOG.md`; `README.md`; `docs/reviews/README.md`; both `ci-renode` workflow markers |
| Date                              | 2026-10-12                             |
| Scope                             | Combined R1/R2/R3/R4/PA **delta** closure review of the FRICK-REVIEW-0148 findings (R1-01 Major; R1-02, R3-01, R4-01, R4-02, R4-03, PA-01 Minors), against ECSS-E-ST-40C (SW development/V&V), ECSS-Q-ST-80C (SW PA), the project norms N1–N20 (`docs/rules.md`), the reviewer contract `docs/engineering/independent-review.md` (rev 0.7), the HAL contract `FF_TsnBackend_t` (`src/hal/hal_tsn.h`) and the authoritative behaviour of the Renode `Network.CadenceGEM` model |
| Independence declaration          | The reviewer had no part in authoring, fixing, building-for-delivery or previously reviewing the items under review. This review ran in a fresh context; every declared claim was re-executed or read first-hand (see §9). |
| Norms applied                    | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

**In scope (verified first-hand):**

- **R1-01 (Major, FRICK-REVIEW-0148).** That the double register base is
  genuinely removed: `renode_hal_tsn_check()` now calls
  `FF_TsnGemSetup(&tsn, &bus, 0u, 1000u)`; a fresh `make renode` produces **no**
  `0x4022xxxx` "non existing peripheral" access and reports `hal_tsn=PASS`; the
  aggregate `RENODE_TEST:` verdict cannot pass with a failed TSN term; and no
  other HAL backend check double-adds a register base.
- **R1-02, R3-01, R4-01, R4-02, R4-03, PA-01 (Minors, FRICK-REVIEW-0148)** —
  each verified first-hand against the working tree and a fresh emulated run.
- **Scope conformance (standing rule).** That the S6a/S6b split + NC-S6-01 is
  honestly recorded (owner, scheduled sprint, no over-claim), that the M11
  **10/13** peripheral-bound count is substantiated, and that the docs which
  previously propagated the false claim no longer do.
- **Gate re-execution.** `ctest --test-dir build/native` (22/22),
  `ctest --test-dir build/asan`, `trace.py docs/requirements/traceability.yaml`
  (114/71/29/14), `metrics.py --check`, `clang-format --dry-run --Werror` on the
  changed C file, and `make renode` / `make qemu` (images present); plus the
  per-document header-vs-last-`## History` check (N16).

**Out of scope / excluded (recorded so the scope is bounded):**

- On-target silicon behaviour (802.1AS PTP accuracy, TSU clock quality, real
  802.1Q screener / 802.1Qbv hardware, the full gPTP / fabric CoreTSN stack,
  M8) — tracked to **GAP-A5**, not verifiable here (unchanged from 0148).
- The `ff_gem_tsn.c` register logic itself (unchanged by this increment; re-run
  only as a regression).
- Unrelated increments re-run only as aggregate regressions.

**Assumptions recorded:** (a) the assigned document id `FRICK-REVIEW-0149` was
supplied by the review lead (the highest id observed in `docs/reviews/` is
`FRICK-REVIEW-0148`, whose report is currently untracked); (b) the item under
review is the uncommitted working tree atop `56ada82`; (c) the Renode
`Network.CadenceGEM` model is the authoritative oracle for what "peripheral-bound
on the emulated GEM" can mean.

## 2. References

- Project norms: `docs/rules.md` (N1–N20, rev 1.4); reviewer contract:
  `docs/engineering/independent-review.md` (rev 0.7); template:
  `docs/reviews/review-template.md`.
- Initial review under closure: `docs/reviews/2026-10-12-m4-m11-tsn-s6a-review.md`
  (FRICK-REVIEW-0148, REJECTED).
- Contract: `src/hal/hal_tsn.h` (`FF_TsnBackend_t`, lines 17–34).
- Backend: `src/drivers/eth/gem/ff_gem_tsn.{c,h}`; registers
  `src/drivers/eth/gem/ff_gem_regs.h` (`FF_GEM_SCRT2 0x0540`,
  `FF_GEM_TSH 0x01C0`, `FF_GEM_TSL 0x01D0`, `FF_GEM_TN 0x01D4`); platform
  `config/platform/platform_mpfs.h` (`FF_GEM0_BASE 0x20110000`, line 37).
- Requirements: `docs/requirements/SRS.md` (SRS-REQ-038 lines 187–191;
  SRS-REQ-040 lines 197–200), `docs/requirements/traceability.yaml`,
  `docs/requirements/traceability.md` (line 103).
- Environment: `docs/vv/renode-environment.md` (rev 0.17); Renode platform
  `platforms/cpus/polarfire-soc.repl` (`mac0: Network.CadenceGEM @ sysbus
  0x20110000`).
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Closure table — FRICK-REVIEW-0148 findings

| Finding | Sev | Disposition verified first-hand | Evidence (first-hand, 2026-10-12) | Status |
| ------- | --- | ------------------------------- | --------------------------------- | ------ |
| **R1-01** | **Major** | **CLOSED.** `base = 0` (the injected accessors add `FF_GEM0_BASE`). Fresh `make renode` reaches the real emulated GEM/TSU: **no** `0x4022xxxx` access, **no** "non existing peripheral" warning, `hal_tsn=PASS`, `RENODE_TEST: PASS`. Aggregate AND term confirmed; no other backend double-adds a base. | `app_renode_test.c:153` (`FF_TsnGemSetup(&tsn, &bus, 0u, 1000u)`); fresh `make renode` -> `[renode] PASS`; `strings build/renode/renode-process.log \| grep -c 0x4022` = **0**, `grep -c "non existing"` = **0**; `console.log` `hal_tsn=PASS` / `RENODE_TEST: PASS`; `app_renode_test.c:476–478` (`hal_tsn_ok` AND term); setup-signature audit of all HAL backends (only TSN takes a base) | **CLOSED** |
| R1-02 | Minor | **PARTIALLY CLOSED.** The model scope limit (802.1Q screener at `0x540` not fully modelled → VLAN/PCP write is a model no-op; TSU read is the genuinely peripheral-bound part) is now disclosed in `renode-environment.md`. **Residual:** the required disclosure was **not** propagated to the code comment, `CHANGELOG.md` or `verification-results.md` (nor qualified in the `traceability.yaml` `lean_note`). Recorded as **NEW-02**. | `renode-environment.md:129–133` (`*Scope limit (TSN):* … the 802.1Q screener register used by `set_vlan` is not fully modelled … GAP-A5`); fresh run still logs `mac0: Unhandled write to offset 0x540, value 0x130`; `app_renode_test.c:135–140` comment omits the caveat; `CHANGELOG.md:12–18`; `verification-results.md:50`; `traceability.yaml:369` | **PARTIAL → NEW-02** |
| R3-01 | Minor | **CLOSED.** SRS-REQ-038 added to **both** `ci-renode` markers. | `.github/workflows/ci.yml:80`; `.gitea/workflows/ci.yml:83` (both now list `SRS-REQ-038`) | **CLOSED** |
| R4-01 | Minor | **PARTIALLY CLOSED.** The `traceability.md` mirror row now lists `test_gem_tsn.c, ci:renode`. **Residual:** the document `Revision`/`## History` was **not** bumped (still 0.32) although the row content changed — R4-01 required "+ history" and N16 requires a history entry for every change. Recorded as **NEW-01**. | `traceability.md:103` (row updated); `traceability.md:7` (`Revision 0.32`) and last history row `0.32` unchanged; precedent: rev 0.28 records a mirror-row-only change | **PARTIAL → NEW-01** |
| R4-02 | Minor | **CLOSED (substantively).** R4-02 was the propagation of the *false* claim caused by R1-01; with R1-01 fixed the "exercised peripheral-bound" claim is now true (the TSU registers are actually read). No doc asserts a now-false statement. The residual R1-02 caveat-propagation nuance is tracked under **NEW-02**. | Fresh run reaches `0x201101C0/…D4` (no warning); doc set reads 10/13 / `hal_tsn=PASS` consistently | **CLOSED** |
| R4-03 | Minor | **CLOSED.** NC-S6-01 is entered in `sprint-plan.md` §3 with an owner and the scheduled sprint. | `sprint-plan.md:72` (`NC-S6-01 \| FRICK-REVIEW-0148 (scope conformance) \| Minor \| … \| Owner: frick-fsw authoring agent; resolve in **sprint S6b**`); S6b row `:51`; history `:113` | **CLOSED** |
| PA-01 | Minor | **CLOSED (log row).** The `docs/reviews/README.md` row for FRICK-REVIEW-0148 was added. **Note:** `status.json` semaphores were not refreshed (still all-green) — to be reconciled at the corrective commit (see NEW-05, Comment). | `docs/reviews/README.md:148`; `status.json:26–41` unchanged (all gates `green`) | **CLOSED** (see NEW-05) |

**Comment-level findings from 0148 (advisory, not required):** R4-04 (`README.md`
"Next per plan: Sprint 7" omits S6b) and R4-05 (`loop-audit.md:64` N18 example
stale) remain unchanged — recorded as **NEW-03**/**NEW-04** (Comments). PA-02
(`status.json.deferred` empty) and PA-03 (no independent TSN oracle) remain
accepted/tracked.

## 4. Scope conformance (standing rule)

**Documented scope.** `sprint-plan.md:50` defines **S6a** as "bind the GEM native
TSN backend to the emulated Cadence GEM and exercise the `FF_TsnBackend_t` seam
(SRS-REQ-038) — **delivered**", exit criterion `hal_tsn=PASS`; `:51` defines
**S6b** as the NET lwIP-on-GEM peripheral-bound check (SRS-REQ-040), "**planned
(NC-S6-01)**". `renode-environment.md:95–107` claims "Peripheral-bound (10/13) …
+ **TSN**".

**Implemented content vs documented scope — CONFORMANT (no scope NC).** The
implementation now genuinely binds the `FF_TsnGem_t` backend to the emulated
Cadence GEM and reads the TSU (verified: no `0x4022xxxx`; `hal_tsn=PASS`). The
M11 **10/13** peripheral-bound count is substantiated: the 10 classes are
GPIO, SERIAL, MEM, ETH, NVM, CAN, I2C, SPI, BLOCK, TSN; the 3 pending are
NET(lwIP-on-GEM = NC-S6-01), PCIE, SPACEWIRE. No over-claim remains on the TSN
scope.

**S6a/S6b split + NC-S6-01 — honestly recorded.** NC-S6-01 is recorded with an
owner (`frick-fsw authoring agent`) and a scheduled sprint (**S6b**) in
`sprint-plan.md` §3 (`:72`), the S6b row (`:51`) and the history (`:113`); it is
echoed consistently in `CHANGELOG.md:19–24`, `verification-results.md:50`,
`handover.md:119,148`, `loop-audit.md:34,172`, `development-plan.md:317,356`,
`README.md:419–420` and the `traceability.yaml` SRS-REQ-040 `lean_note`. The
NET deferral reason (lwIP RTEMS port + Renode network path) is legitimate. **The
S6a "delivered" status is now correct** because the TSN check is no longer
vacuous.

**Docs that previously propagated the false claim** (`CHANGELOG.md`,
`verification-results.md`, `renode-environment.md`, `handover.md`,
`loop-audit.md`, `development-plan.md`, `README.md`, `tool-qualification.md`,
`traceability.yaml`) were re-checked: they now state the (true) fixed behaviour.
The only residual documentation defects are the history bump (NEW-01) and the
R1-02 caveat propagation (NEW-02).

## 5. New findings

| # | Severity | Profile | Item | Finding | Evidence | Recommendation |
| - | -------- | ------- | ---- | ------- | -------- | -------------- |
| **NEW-01** | **Minor** | R4/PA | `docs/requirements/traceability.md` | The SRS-REQ-038 mirror row was changed (added `ci:renode`, reworded) but the document `Revision`/`## History` was **not** bumped (still **0.32**); R4-01 explicitly required "+ history" and N16 requires a history entry for every change (precedent: rev 0.28 is a mirror-row-only change that did bump). | `traceability.md:103` (changed) vs `:7` and last history row `0.32` (unchanged); `git diff docs/requirements/traceability.md` shows only the row change | Add a history row (e.g. `0.33 … M11/S6a: SRS-REQ-038 mirror row adds ci:renode`) and set `Revision` to match. |
| **NEW-02** | **Minor** | R1/R4 | doc set / code comment | The R1-02 model-limit disclosure is present **only** in `renode-environment.md`; the code comment (`app_renode_test.c:135–140`) and `CHANGELOG.md:12–18` / `verification-results.md:50` / `traceability.yaml:369` still present "init + VLAN/PCP + 802.1AS TSU time … peripheral-bound" without the screener-no-op caveat. R1-02 required the disclosure in "the comment + `renode-environment.md`/CHANGELOG/`verification-results.md`". | `renode-environment.md:129–133` (disclosed); `app_renode_test.c:135–140` (no caveat); `CHANGELOG.md:15–18`; `verification-results.md:50`; `traceability.yaml:369`; fresh run `mac0: Unhandled write to offset 0x540` | Add one sentence to the code comment (and optionally CHANGELOG/`verification-results`/`lean_note`) noting the screener is not modelled → the VLAN/PCP write is a model no-op (target = GAP-A5). |
| NEW-03 | Comment | R4/N8 | `README.md:420–421` | Still "Next per plan: **Sprint 7** (PCIe + SpaceWire, closes M11 13/13)", omitting the plan's next sprint **S6b**; inconsistent with `handover.md:147`/`sprint-plan.md:51`. (Residual of 0148 R4-04.) | `README.md:420–421` vs `handover.md:147`; `sprint-plan.md:50–52` | Reorder to "Next: **S6b**, then S7". Advisory. |
| NEW-04 | Comment | R4/N19 | `loop-audit.md:64` | N18 row example still cites "`make renode` PASS, incl. `hal_i2c=PASS` on the Sprint-3 image" (stale). (Residual of 0148 R4-05.) | `loop-audit.md:64` | Make generic / name the current increment's marker. Advisory. |
| NEW-05 | Comment | PA/N11 | `status.json` | Semaphores are all `green` (`reviews`, `documentation_review`, `product_assurance_review`) while the FRICK-REVIEW-0148 loop was REJECTED and this delta review is in progress. Conventionally refreshed at cycle close; to be confirmed consistent at the corrective commit. | `status.json:26–41` (unchanged by the delta); `docs/reviews/README.md:148` | Reconcile the semaphores/`note` at the corrective commit so they reflect the closed S6a review. Advisory. |

**Severity definition** (per `review-template.md`)

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
  *(None open.)*
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified. *(NEW-01, NEW-02.)*
- **Comment**: observation/recommendation, no derogation. *(NEW-03, NEW-04,
  NEW-05.)*

## 6. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**Rationale.** The FRICK-REVIEW-0148 **Major R1-01 is genuinely closed**: the
`base = 0` fix was verified first-hand and a fresh `make renode` now reaches the
real emulated Cadence GEM/TSU — **no** `0x4022xxxx` access and **no** "non
existing peripheral" warning, `hal_tsn=PASS`, `RENODE_TEST: PASS`; the aggregate
verdict correctly ANDs the TSN term, and no other backend double-adds a base.
R3-01, R4-03 and PA-01 are closed; R4-02 is substantively closed (the underlying
false claim is gone). The **S6a/S6b split + NC-S6-01** is honestly recorded
(owner, sprint S6b, no over-claim) and the M11 **10/13** count is substantiated.
Two **new Minor** findings remain: **NEW-01** (`traceability.md` history not
bumped, N16/N8) and **NEW-02** (R1-02 caveat not propagated beyond
`renode-environment.md`). Both are documentation-completeness defects with
trivial fixes; neither invalidates a requirement nor re-opens the Major. Because
no Major is open, the conclusion is **APPROVED WITH COMMENTS** (N12); commit/push
is permitted once the two new Minors are corrected in the same cycle. The
loop is **closed for the Major** and requires one further small doc correction +
confirmation for the two Minors.

## 7. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | Senior FSW / real-time / space software developer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — R1-01 (Major) closed (base=0; fresh `make renode` clean, `hal_tsn=PASS`); R1-02 partial → NEW-02 |
| R2 (N9)       | Senior SW V&V engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — the SRS-REQ-038 `ci:renode` evidence is now substantiated (TSU actually read); R1-02 disclosure partial → NEW-02 |
| R3 (N14)      | Senior avionics / systems engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — SRS-REQ-038/040 wording OK; evidence now substantiated; R3-01 closed; NEW-01 (traceability mirror history) |
| R4 (N17)      | Senior SW documentation engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — R4-01 row fixed but history not bumped (NEW-01); R4-02 closed; R4-03 closed; NEW-02/NEW-03/NEW-04 |
| PA (N20)      | Senior SW Product Assurance engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — Major closed; PA-01 log row added (NEW-05 semaphore note); NC-S6-01 honestly recorded; residual GAP-A5 TSN timing/PTP for human acceptance |

## 8. Follow-up records

(norm N12: NCs are resolved by re-applying all norms to the affected scope and
re-reviewing the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review | Scheduled sprint |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- | ---------------- |
| **R1-01** | **Major** | `app_renode_test.c` `renode_hal_tsn_check`; SRS-REQ-038 | double base → all TSN MMIO at `0x4022xxxx`; false pass | Pass `base = 0`; re-run `make renode` | Fresh `make renode` PASS; `grep 0x4022` = 0; `hal_tsn=PASS`; `app_renode_test.c:153` | frick-fsw | **CLOSED** | done (this report) | S6a delta |
| R1-02 | Minor | `ff_gem_tsn.c` `set_vlan`; docs | model omits the screener (`0x540`) → VLAN/PCP is a no-op | Disclose the model limit (comment + docs) | `renode-environment.md:129–133` (done); comment/CHANGELOG/`verification-results`/`lean_note` **pending** | frick-fsw | **PARTIAL → NEW-02** | pending | S6a delta |
| R3-01 | Minor | `ci-renode` markers | SRS-REQ-038 missing from both markers | Add SRS-REQ-038 to both | `.github/workflows/ci.yml:80`; `.gitea/workflows/ci.yml:83` | frick-fsw | **CLOSED** | done | S6a delta |
| R4-01 | Minor | `traceability.md:103` | mirror row missing `ci:renode` | Update row **+ history** | row updated (`:103`); history **not** bumped → NEW-01 | frick-fsw | **PARTIAL → NEW-01** | pending | S6a delta |
| R4-02 | Minor | doc set | false "exercised peripheral-bound" claim propagated | Correct with the code fix | underlying claim now true (R1-01 closed); caveat propagation → NEW-02 | frick-fsw | **CLOSED** | done | S6a delta |
| R4-03 | Minor | `sprint-plan.md` §3; NC-S6-01 | NC-S6-01 not in §3; no owner | Add with owner + sprint S6b | `sprint-plan.md:72` | frick-fsw | **CLOSED** | done | S6a delta |
| PA-01 | Minor | `docs/reviews/README.md`; `status.json` | no log row for 0148; semaphores stale | Add log row; refresh semaphores | `docs/reviews/README.md:148` (row added); semaphores → NEW-05 | frick-fsw | **CLOSED** (see NEW-05) | done | S6a commit |
| **NC-S6-01** | Minor (scope) | SRS-REQ-040 / sprint S6b | NET lwIP-on-GEM peripheral-bound deferred | Honestly recorded; owner + §3 present | `sprint-plan.md:51,72,113`; `CHANGELOG.md:19–24`; status/handover/loop-audit/`verification-results`/README; `traceability.yaml` SRS-REQ-040 note | frick-fsw (frick-fsw authoring agent) | **open (scheduled S6b)** | n/a | **S6b** |
| **NEW-01** | **Minor** | `traceability.md` | history/`Revision` not bumped for the row change (N16/N8) | Add a history row + bump `Revision` | (delta) `traceability.md` diff | frick-fsw | **open** | pending | S6a delta (pre-commit) |
| **NEW-02** | **Minor** | code comment / `CHANGELOG` / `verification-results` / `lean_note` | R1-02 caveat not propagated | Add the screener caveat | (delta) doc diffs | frick-fsw | **open** | pending | S6a delta (pre-commit) |
| NEW-03, NEW-04, NEW-05 | Comment | — | Advisory (§5) | Accepted / optional | — | frick-fsw | n/a | — | — |

## 9. Evidence ledger (all re-executed/obtained first-hand on 2026-10-12)

| Claim | Method | Result |
| ----- | ------ | ------ |
| Native ctest | `ctest --test-dir build/native` | **22/22 pass** |
| ASan/UBSan ctest | `ctest --test-dir build/asan` | **22/22 pass** |
| Traceability | `trace.py docs/requirements/traceability.yaml` | **PASS (114 requirements, 71 srs, 29 sss, 14 gaps, 51 lean theorems, 12 CI claims)** |
| Metrics panel | `metrics.py --check` | `metrics panel: current` |
| Formatting | `clang-format --dry-run --Werror src/apps/app_renode_test.c` | clean (exit 0) |
| Renode functional run (N18) | `make renode` | **PASS** (`RENODE_TEST: PASS`; `hal_tsn=PASS`) |
| R1-01 no double base | `strings build/renode/renode-process.log \| grep -c 0x4022`; `grep -c "non existing"` | **0** and **0** |
| TSU reached / screener no-op | `strings build/renode/renode-process.log \| grep -i mac0` | `mac0: Unhandled write to offset 0x540, value 0x130` (screener not modelled; TSU reads emit no warning) |
| QEMU emulated target | `make qemu` | **PASS** (banner + 6 applications registered) |
| TSN setup base | `app_renode_test.c:153` | `FF_TsnGemSetup(&tsn, &bus, 0u, 1000u)` |
| Aggregate wiring | `app_renode_test.c:476–478` | `hal_tsn_ok` is an AND term (cannot pass on a failed TSN check) |
| No other base double-add | setup-signature audit of all HAL backends | only `FF_TsnGemSetup` takes a `base`; all others take no base (GPIO's 3rd arg = pin count) |
| ci-renode markers | both workflows | SRS-REQ-038 **present** (`.github:80`, `.gitea:83`) |
| traceability mirror | `traceability.md:103` | row lists `test_gem_tsn.c, ci:renode`; **history not bumped** (`Revision 0.32`) |
| NC-S6-01 recording | `sprint-plan.md` §3 | present with owner + sprint S6b (`:72`) |
| Review log | `docs/reviews/README.md:148` | FRICK-REVIEW-0148 row present |
| N16 header/history | 7 touched `## History` docs | header = last history row for all; `traceability.md` changed without a bump (NEW-01) |
| M11 count | 10 peripheral-bound classes | GPIO/SERIAL/MEM/ETH/NVM/CAN/I2C/SPI/BLOCK/TSN = **10/13**; pending NET(lwIP-on-GEM)/PCIE/SPACEWIRE |

> Note: the gate commands regenerate build artifacts under `build/` (git-ignored)
> only; **no tracked source or documentation file was modified** by this review.
> The reviewer wrote **exactly one file** (this report); nothing was committed or
> pushed.

## 10. Norm-by-norm audit

| Norm | Status | Evidence / note |
| ---- | ------ | --------------- |
| N1 Lean | **pass** | `lake build` unaffected; SRS-REQ-038/040 carry explicit `lean_note` (no silent waiver). |
| N2 English | **pass** | All changed content is English. |
| N3 backwards SRS/SSS | **pass** | SRS-REQ-038/040 present and parented; text unchanged by this increment. |
| N4 traceability | **partial** | `trace.py` PASS and the mirror row now carries `ci:renode`; the `traceability.md` history was not bumped (**NEW-01**). |
| N5 class-B/A gaps | **pass** | GAP-A5 TSN residual tracked; no new GAP-A* needed. |
| N6 V&V + deployed env | **pass** | `ci:renode` evidence for SRS-REQ-038 is now substantiated (TSU actually read). |
| N7 dual push | **n/a (pre-commit)** | Change uncommitted; commit/push permitted once NEW-01/NEW-02 are corrected (N12). |
| N8 docs current | **partial** | R4-02 resolved, R4-01 row fixed, R4-03 closed; **NEW-01** (history), **NEW-02** (caveat propagation). |
| N9 independent review | **pass** | This report (fresh context, five profiles). |
| N10 test scripts/tools | **partial** | T17 gains `hal_tsn`; the check is now discriminating (reaches the peripheral) — disclosure nuance = NEW-02. |
| N11 records in repo | **partial** | 0148 log row present (PA-01); semaphores to reconcile at commit (**NEW-05**). |
| N12 NC closure | **in progress** | R1-01 Major + R3-01/R4-02/R4-03/PA-01 closed; R1-02/R4-01 partial → NEW-01/NEW-02 open, tracked with owner + sprint. |
| N13 licensing | **pass** | No new source files; SPDX unaffected. |
| N14 SSS review | **partial** | R3 finds wording OK and evidence now substantiated; R3-01 closed; NEW-01. |
| N15 full cycle | **pass (this cycle)** | Build/tests/trace/metrics/format/Renode/QEMU reproduced green. |
| N16 revision history | **fail** | `traceability.md` changed without a history/`Revision` bump (**NEW-01**). |
| N17 documentation review | **partial** | R4 finds the doc set largely correct; NEW-01/NEW-02/NEW-03/NEW-04. |
| N18 Renode functional | **pass** | `make renode` -> `RENODE_TEST: PASS`; the TSN check now reaches the emulated GEM/TSU (no `0x4022`). |
| N19 loop audit | **partial** | `loop-audit.md` refreshed and M11 10/13 tracked; N18 example stale (**NEW-04**). |
| N20 PA review | **partial** | PA performed; Major closed; PA-01 closed; NEW-05 semaphore note; residual risk flagged for human acceptance. |

---

## Facts, Assumptions, Inferences, Recommendations, Confidence, Open gaps

**Facts** (verified first-hand, this session)

- `renode_hal_tsn_check()` calls `FF_TsnGemSetup(&tsn, &bus, 0u, 1000u)` (`app_renode_test.c:153`), with the injected accessors already adding `FF_GEM0_BASE` (`:111–115,129–133`).
- A fresh `make renode` PASSes with **no** `0x4022xxxx` access and **no** "non existing peripheral" warning; `hal_tsn=PASS` and `RENODE_TEST: PASS` are present.
- The fresh run still logs `mac0: Unhandled write to offset 0x540, value 0x130` — the 802.1Q screener write is a model no-op (R1-02 basis); the TSU reads emit no warning.
- `hal_tsn_ok` is an AND term of the `RENODE_TEST:` marker (`app_renode_test.c:476–478`); the marker cannot pass with a failed TSN check.
- Only `FF_TsnGemSetup` takes a `base`; all other HAL backend setups take no base (GPIO's third arg is a pin count) — no other backend double-adds a base.
- SRS-REQ-038 is present in **both** `ci-renode` markers (`.github/workflows/ci.yml:80`, `.gitea/workflows/ci.yml:83`).
- `traceability.md:103` lists `test_gem_tsn.c, ci:renode`, but the document `Revision`/history is still **0.32** (no history row added).
- NC-S6-01 is in `sprint-plan.md` §3 with owner `frick-fsw authoring agent` and sprint **S6b** (`:72`); the `docs/reviews/README.md` row for FRICK-REVIEW-0148 is present (`:148`).
- Gates reproduce green: `ctest` **22/22** native and ASan, `trace.py` **PASS 114/71/29/14** (51 theorems, 12 CI claims), `metrics --check` current, `clang-format` clean, `make renode` PASS, `make qemu` PASS.
- All seven touched `## History`-bearing docs have header = last history row; `traceability.md` was changed without a bump.

**Assumptions** (owner, impact, validation)

- *Document id `FRICK-REVIEW-0149`* — owner: review lead; impact: report identity only.
- *The uncommitted working-tree delta is the item under review atop `56ada82`.*
- *The Renode `Network.CadenceGEM` model is the authoritative oracle for "peripheral-bound on the emulated GEM"* — validation: the model is the only emulator present; silicon = GAP-A5.

**Inferences**

- Because the TSN check now reaches the real emulated GEM/TSU, the "M11 10/13 peripheral-bound" and SRS-REQ-038 `ci:renode` claims are substantiated (R1-01 resolved).
- Because R4-01 explicitly required "+ history" and the mirror row changed without a bump, the `traceability.md` history is non-conformant with N16 (NEW-01).
- Because R1-02 required the model-limit disclosure in the comment + docs and only `renode-environment.md` carries it, the residual claims ("VLAN/PCP … peripheral-bound") remain unqualified (NEW-02).

**Recommendations** (decision owner, prerequisites, trade-offs)

1. **NEW-01**: add a `traceability.md` history row and set `Revision` to match (e.g. 0.33).
2. **NEW-02**: add one sentence to the `app_renode_test.c` comment (and optionally `CHANGELOG`/`verification-results`/`lean_note`) noting the screener is not modelled → the VLAN/PCP write is a model no-op (target = GAP-A5).
3. **NEW-03/NEW-04/NEW-05** (advisory): fix the README "Next per plan" ordering, refresh the `loop-audit.md` N18 example, and reconcile the `status.json` semaphores at the corrective commit.
4. Keep the GAP-A5 on-target TSN timing/PTP residual (and the M8 gPTP/CoreTSN stack) tracked; obtain a datasheet/model-independent oracle before any flight claim.
5. Append the FRICK-REVIEW-0149 row to `docs/reviews/README.md` and commit/push the whole set (norm N11/N7) once NEW-01/NEW-02 are corrected.

**Confidence:** **High** for R1-01 closure (direct code read + fresh `make renode` with zero `0x4022`/zero "non existing" + exact aggregate wiring). **High** for R3-01/R4-03/PA-01 (direct source reads). **High** for NEW-01 (direct diff vs history). **Medium-High** for NEW-02 (the authoritative disclosure exists in `renode-environment.md`; the residual is unqualified wording in shorter claims). Confidence is not a probability of safety.

**Open gaps**

- **NEW-01 (Minor)** open — `traceability.md` history/`Revision` not bumped.
- **NEW-02 (Minor)** open — R1-02 caveat not propagated beyond `renode-environment.md`.
- **NEW-03/NEW-04/NEW-05 (Comment)** open — advisory doc/status items.
- **NC-S6-01 (scope)** open and scheduled (**S6b**); owner recorded.
- **No independent oracle** for the TSN register semantics; the Renode model (which omits the screener) is the only emulator.
- **On-target evidence absent** — 802.1AS PTP accuracy, TSU clock quality, real screener/Qbv hardware, gPTP/CoreTSN stack (M8) → **GAP-A5**; residual risk is for **human acceptance**.

---

*This report is authored to be committed and pushed with the reviewed change
(norm N11). The review was read-only with respect to repository content: the
reviewer wrote exactly one file (this report); no other tracked file was
modified, created or deleted, and nothing was committed or pushed. The gate
commands regenerate only git-ignored `build/` artifacts. Independence is
declared per norm N9 (fresh context; no part in authoring, fixing or previously
reviewing the items).*
