# Review Report

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0148                      |
| Reviewer session                  | independent agent (norm N9), fresh context |
| Reviewer profile                  | **R1+R2+R3+R4+PA** — R1 (Senior FSW / real-time / space software developer) + R2 (Senior SW V&V engineer) + R3 (Senior avionics / systems engineer, N14) + R4 (Senior SW documentation engineer, N17) + PA (Senior SW Product Assurance engineer, N20) |
| Reference revision                | `56ada82` (HEAD) + **uncommitted** Sprint-6a working-tree increment (GEM native TSN backend exercised peripheral-bound, SRS-REQ-038) |
| Items under review                | `src/apps/app_renode_test.c` (`renode_hal_tsn_check`, `hal_tsn` aggregate term); the TSN backend under test `src/drivers/eth/gem/ff_gem_tsn.{c,h}` and its registers `ff_gem_regs.h`; `config/platform/platform_mpfs.h` (`FF_GEM0_BASE`); requirement **SRS-REQ-038** (and **SRS-REQ-040** deferred note) in `docs/requirements/{SRS.md,traceability.yaml,traceability.md}`; `docs/vv/{verification-results.md,renode-environment.md,tool-qualification.md}`; `docs/planning/sprint-plan.md` (S6 split); `docs/development-plan.md`; `docs/status/{handover,loop-audit,status.json}`; `CHANGELOG.md`; `README.md`; both `ci-renode` workflow markers |
| Date                              | 2026-10-12                             |
| Scope                             | Combined R1/R2 (code + V&V) + R3 (systems/requirements, N14) + R4 (documentation, N17) + PA (product assurance, N20) review of the Sprint-6a TSN peripheral-bound increment, against ECSS-E-ST-40C (SW development/V&V), ECSS-Q-ST-80C (SW PA), the project norms N1–N20 (`docs/rules.md`), the reviewer contract `docs/engineering/independent-review.md` (rev 0.7), the HAL contract `FF_TsnBackend_t` (`src/hal/hal_tsn.h`) and the authoritative behaviour of the Renode `Network.CadenceGEM` model |
| Independence declaration          | The reviewer had no part in authoring, building-for-delivery or previously reviewing the items under review. This review ran in a fresh context; every declared claim was re-executed or read first-hand (see §12). |
| Norms applied                    | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

**In scope (verified first-hand):**

- **Part A — R1/R2 (code + V&V).** Correctness of the new `renode_hal_tsn_check()`
  in `src/apps/app_renode_test.c`: that it really binds the `FF_TsnGem_t` backend
  to the emulated Cadence GEM at `FF_GEM0_BASE 0x20110000`, exercises the
  `FF_TsnBackend_t` contract (`init` + `set_vlan(1,3)` + `get_time_ns`) against
  the emulated TSU, asserts plausibly and does **not over-claim**; that the GEM
  TSU register access is correct; and that the aggregate `RENODE_TEST:` verdict
  cannot pass with a failed TSN check. Re-execution of `make renode` and the
  gates (`ctest` native + ASan, `trace.py`, `metrics --check`, `clang-format`).
- **Part B — R3 (systems/requirements, N14).** SRS-REQ-038 well-formedness and
  the accuracy of its newly-claimed peripheral-bound evidence (`ci:renode`); the
  SRS-REQ-040 deferred note; trace manifest / mirror / top-down / bottom-up
  agreement (counts **114/71/29/14/51**).
- **Part C — R4 (documentation, N17).** Currency and internal consistency of
  `docs/vv/{verification-results,renode-environment,tool-qualification}.md`,
  `docs/planning/sprint-plan.md`, `docs/development-plan.md`,
  `docs/status/{handover,loop-audit,status.json}`, `CHANGELOG.md`, `README.md`,
  `docs/requirements/traceability.md`, both `ci-renode` markers; per-document
  header `Revision`/`Date` vs last `## History` row (N16); the M11 **10/13**
  count consistency.
- **Part D — PA (product assurance, N20) + scope conformance.** Whether the
  S6 split into S6a (delivered) + S6b (**NC-S6-01**, scheduled) is the correct
  and honestly-recorded disposition; whether NC-S6-01 is opened with owner +
  scheduled sprint; claim discipline and the residual-gap register (GAP-A5).

**Out of scope / excluded (recorded so the scope is bounded):**

- On-target silicon behaviour: real 802.1AS PTP accuracy (servo/BMCA/path-delay),
  the GEM TSU clock quality, the real 802.1Q screener / 802.1Qbv hardware, and
  the full gPTP / fabric CoreTSN stack (**M8**) — tracked to **GAP-A5**, not
  verifiable here.
- The `ff_gem_tsn.c` register logic itself (802.1Q screener, ENST, TSU) is
  unchanged by this increment and was reviewed previously
  (`FRICK-REVIEW-*` GEM-TSN series); it is re-exercised only as a regression.
- The NET lwIP-on-GEM part (deferred, NC-S6-01) beyond confirming the deferral
  is recorded honestly.
- Unrelated increments re-run only as aggregate regressions.

**Assumptions recorded:** (a) the assigned document id `FRICK-REVIEW-0148` was
supplied by the review lead (the highest committed id observed in
`docs/reviews/` is `FRICK-REVIEW-0147`); (b) the item under review is the
uncommitted working tree atop `56ada82`; (c) the Renode `Network.CadenceGEM`
model is the authoritative oracle for what "peripheral-bound on the emulated GEM"
can mean.

## 2. References

- Project norms: `docs/rules.md` (N1–N20, rev 1.4); reviewer contract:
  `docs/engineering/independent-review.md` (rev 0.7); template:
  `docs/reviews/review-template.md`.
- Contract: `src/hal/hal_tsn.h` (`FF_TsnBackend_t`, lines 17–34).
- Backend: `src/drivers/eth/gem/ff_gem_tsn.{c,h}`; registers
  `src/drivers/eth/gem/ff_gem_regs.h`; platform `config/platform/platform_mpfs.h`
  (`FF_GEM0_BASE 0x20110000`, line 37).
- Requirements: `docs/requirements/SRS.md` (SRS-REQ-038 lines 187–191;
  SRS-REQ-040 lines 197–200), `docs/requirements/traceability.yaml`,
  `docs/requirements/traceability.md` (line 103).
- Environment: `docs/vv/renode-environment.md` (rev 0.17); Renode platform
  `platforms/cpus/polarfire-soc.repl` (`mac0: Network.CadenceGEM @ sysbus 0x20110000`)
  and `platforms/boards/beaglev-fire.repl`.
- Prior reviews: `docs/reviews/2026-10-10-m4-gem-tsn-r1-r2-r4-review.md`,
  `2026-10-10-m4-gem-tsn-delta-review.md`,
  `2026-10-12-m4-m11-block-delta-r3-r4-pa-review.md` (FRICK-REVIEW-0147).
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Part A — R1/R2 (code + V&V) findings

### R1-01 — **Major** — the `hal_tsn` check never reaches the emulated GEM (double register base); `hal_tsn=PASS` is a **false pass**

**The defect.** `renode_hal_tsn_check()` passes `FF_GEM0_BASE` (0x20110000) as the
`base` argument to `FF_TsnGemSetup` (`app_renode_test.c:150`) while the injected
bus accessors already add `FF_GEM0_BASE`:

- `mpfs_gem_read32`/`mpfs_gem_write32` compute `FF_GEM0_BASE + off`
  (`app_renode_test.c:111–115, 129–133`);
- the backend adds its `base` **again**: `tsn_rd`/`tsn_wr` do
  `t->bus.read32(t->bus.ctx, t->base + off)` with `t->base = FF_GEM0_BASE`
  (`ff_gem_tsn.c:15–23, 128–136`).

Every TSN MMIO access therefore targets `FF_GEM0_BASE + FF_GEM0_BASE + off`
= **0x4022_xxxx**, which is **not a mapped peripheral** on the emulated
PolarFire SoC (the GEM is at 0x20110000; nothing is mapped at 0x40220000). The
increment's own run log proves it (fresh `make renode`,
`build/renode/renode-process.log`):

```text
[WARNING] sysbus: [u54_1: 0x10000037B8] WriteDoubleWord to non existing peripheral at 0x40220540, value 0x130.
[WARNING] sysbus: [u54_1: 0x10000037AA] ReadDoubleWord from non existing peripheral at 0x402201C0.
[WARNING] sysbus: [u54_1: 0x10000037AA] ReadDoubleWord from non existing peripheral at 0x402201D0.
[WARNING] sysbus: [u54_1: 0x10000037AA] ReadDoubleWord from non existing peripheral at 0x402201D4.
```

The chain is arithmetically exact: `0x20110000 + 0x20110000 + 0x540 = 0x40220540`
(the `set_vlan` screener write; value `0x130 = (3<<4)|(1<<8)` = PCP 3 + VLAN-enable,
matching `set_vlan(1,3)`), and `+0x1C0 / +0x1D0 / +0x1D4 = 0x402201C0 / 0x402201D0 /
0x402201D4` (the `get_time_ns` TSH/TSL/TN reads). The correct convention is
established by the sibling ETH check (`renode_hal_eth_check` reads
`FF_GEM0_BASE + FF_GEM_MODID_OFF = 0x201100FC` and passes) and by the host unit
test (`FF_TsnGemSetup(&t, &bus, 0u, 1000u)`, `test_gem_tsn.c:48`): **`base` must be
`0`** when the injected bus is already GEM0-relative.

**Consequence.** The emulated Cadence GEM / TSU is **never touched**. `init`
only checks the callbacks are non-NULL; `set_vlan` writes to a nonexistent
peripheral (dropped, returns `FF_SUCCESS`); `get_time_ns` reads zeros from a
nonexistent peripheral and returns `FF_SUCCESS`. The check therefore **cannot
fail** and provides no discrimination. Consequently `hal_tsn=PASS` is a false
pass, and everything the increment claims on the strength of it is unsubstantiated:
the "M11 peripheral-bound **10/13**" count (really 9/13 — TSN is not exercised),
the SRS-REQ-038 `ci:renode` peripheral-bound evidence, the CHANGELOG /
`verification-results.md` / `renode-environment.md` / `handover.md` /
`loop-audit.md` / `development-plan.md` / `README.md` / T17 statements, and the
`traceability.yaml` `lean_note`.

**Required correction.** Pass `base = 0` to `FF_TsnGemSetup`
(`FF_TsnGemSetup(&tsn, &bus, 0u, 1000u)`), matching `test_gem_tsn.c:48` and the
`renode_hal_eth_check` convention (or, less desirably, make the injected GEM bus
raw-absolute and adjust the ETH check accordingly). Re-run `make renode` and
confirm the TSN accesses no longer emit "non existing peripheral" warnings and
that the TSU registers are actually read. Then correct the propagating
documentation (§5, R4-02) and re-review the delta (norm N12).

> Note on the "cannot pass with a failed TSN check" requirement: the aggregate
> verdict *is* correctly wired (`hal_tsn_ok` is an AND term of the final
> `RENODE_TEST:` marker, `app_renode_test.c:473–477`). The problem is not the
> aggregate logic but that the TSN term itself is vacuous, so it can never be
> `false` on the emulator.

### R1-02 — **Minor** — the model does not implement the 802.1Q screener, so "VLAN/PCP" cannot be claimed peripheral-bound on this model

Even with the base corrected, the Renode `Network.CadenceGEM` model does **not**
implement the 802.1Q screener type-2 register at offset `0x540`: a first-hand
probe (`sysbus WriteDoubleWord 0x20110540 …`) logs
`mac0: Unhandled write to offset 0x540`, and a read logs
`mac0: Unhandled read from offset 0x540`. So `set_vlan` is a **no-op in the
model** (the driver returns `FF_SUCCESS` regardless of the write being dropped).
By contrast the 802.1AS TSU registers are implemented by the model: reads at
`0x201101C0 / 0x201101D0 / 0x201101D4` produce **no** "Unhandled read" warning.

Therefore the claim "init + VLAN/PCP + 802.1AS TSU time" exercised
peripheral-bound is only accurate for the **TSU time read** on this model; the
VLAN/PCP programming is a register write the model discards. The code comment
(`app_renode_test.c:135–140`), `renode-environment.md:95–107`, `CHANGELOG.md:12–18`
and `verification-results.md:50` should disclose this model scope limit (as the
sibling SPI/BLOCK checks already disclose "no registered slave" / "no SD card"),
or the check should assert on an observable TSU effect (e.g. that the read value
is plausible/non-degenerate) rather than only `FF_SUCCESS`.

## 4. Part B — R3 (systems / requirements, N14) findings

- **B-SRS — SRS-REQ-038 wording: compliant (no finding on the requirement text).**
  `docs/requirements/SRS.md:187–191` states the backend, 802.1Q VLAN/PCP,
  802.1Qbv ENST, 802.1AS time from the GEM TSU, 802.1Qav as documented
  not-exposed, `method: Tst | Inp`. The wording is singular, testable,
  parented to SSS-REQ-020 (`derived: false`) and does **not** itself over-claim
  peripheral-bound (that claim lives only in the evidence/manifest). The
  requirement text was **not modified** by this increment (correct: only the
  evidence changed).
- **B-EVIDENCE — SRS-REQ-038 peripheral-bound evidence: NOT substantiated (Major
  → R1-01).** The manifest now lists `ci:renode` for SRS-REQ-038
  (`traceability.yaml:367`) and the `lean_note` asserts "the `FF_TsnBackend_t`
  seam is exercised peripheral-bound on the emulated Cadence GEM". This is not
  true (R1-01). The evidence claim must be removed/qualified until the check is
  corrected.
- **B-TRACE — counts converged (no finding).** `trace.py` PASS with **114
  requirements (71 srs / 29 sss / 14 gaps), 51 Lean theorems, 12 CI claims**;
  `traceability.md` (rev 0.32) and the README panel agree. Top-down: SRS-REQ-038
  → SSS-REQ-020 (`traceability.md:50`); bottom-up: `traceability.md:103`.
  **However** the mirror row `traceability.md:103` still shows only
  `test_gem_tsn.c` and does not list the new `ci:renode` → **R4-01**.
- **B-040 — SRS-REQ-040 deferred note: accurate (no finding).** The manifest
  `lean_note` for SRS-REQ-040 (`traceability.yaml:387`) correctly records that
  the peripheral-bound lwIP-on-GEM check is deferred (NC-S6-01, sprint S6b) and
  that the adapter remains host-tested; the SRS-REQ-040 text is unchanged and
  does not over-claim. The stated reason (lwIP RTEMS port + Renode network path)
  is plausible and is a legitimate deferral.

## 5. Part C — R4 (documentation, N17) findings

**Header/history consistency (N16) — all touched documents compliant:** for every
touched document with a `## History` table the header `Revision`/`Date` equals
the last history row and rows are in ascending order — `development-plan.md`
0.23, `sprint-plan.md` 0.9, `handover.md` 1.13, `loop-audit.md` 0.17,
`verification-results.md` 0.38, `renode-environment.md` 0.17,
`tool-qualification.md` 0.3.21. `README.md`/`CHANGELOG.md` are not
`## History`-bearing (CHANGELOG carries the project-level `[Unreleased]` entry).

- **R4-01 — Minor — `docs/requirements/traceability.md` mirror not regenerated
  (N4/N8).** Line **103** still reads
  `| SRS-REQ-038 | drivers/eth/gem/ff_gem_tsn.c/h | test_gem_tsn.c | - | … |`,
  i.e. it does **not** list the new `ci:renode` evidence that the manifest
  (`traceability.yaml:367`) now carries. Sibling rows that do have `ci:renode`
  (e.g. SRS-REQ-036 `:101`, SRS-REQ-039 `:104`) show the expected form. Fix:
  update the row to `test_gem_tsn.c, ci:renode` and bump the document history.
  (This is the recurring "traceability.md mirror" NC class.)
- **R4-02 — Minor — the "exercised peripheral-bound" claim propagates through the
  doc set (N8).** `CHANGELOG.md:12–18`, `verification-results.md:50`,
  `renode-environment.md:95–107,169`, `handover.md:110–119,148,218`,
  `loop-audit.md:34,172`, `development-plan.md:317,356`, `README.md:418–421`,
  `tool-qualification.md:41` (T17) and the SRS-REQ-038 `lean_note` all state the
  TSN seam is "exercised peripheral-bound (init + VLAN/PCP + 802.1AS TSU time)".
  This is inaccurate (R1-01); every occurrence must be corrected in the same
  cycle as the code fix. **Manifestation of R1-01.**
- **R4-03 — Minor — NC-S6-01 not entered in `sprint-plan.md` §3 and no owner
  recorded (N8/N12/N16).** `sprint-plan.md` §5 states "any new NC is added to
  §3", but §3 (`:64–71`) still lists only the Sprint-0 backlog (PA-67-01,
  M16-R3/R4/PA). NC-S6-01 appears only in the S6b row (`:51`) and the history
  row 0.9 (`:112`). No owner is recorded anywhere for NC-S6-01 (the
  `review-template` follow-up table requires one). Fix: add NC-S6-01 to §3 with
  owner (frick-fsw / V&V coordinator) and scheduled sprint S6b.
- **R4-04 — Comment — `README.md` "Current focus" ordering.** `README.md:418–421`
  says NC-S6-01 is deferred to Sprint 6b, then "Next per plan: **Sprint 7**
  (PCIe + SpaceWire, closes M11 13/13)". The plan's order is S6a → **S6b** → S7,
  so the next sprint is S6b, not S7; and S7 closes M11 13/13 only if S6b
  (NET lwIP-on-GEM) is done first. Recommend "Next: S6b, then S7 …". Advisory.
- **R4-05 — Comment — `loop-audit.md` N18 row example is stale.**
  `loop-audit.md:64` still cites "`make renode` PASS, incl. `hal_i2c=PASS` on
  the Sprint-3 image" as the N18 example. Not introduced by this increment, but
  it should name the current increment's marker (or be made generic). Advisory.

## 6. Part D — PA (product assurance, N20) and scope conformance

### 6.1 Scope conformance (standing rule)

**Documented scope of S6a.** `sprint-plan.md:50` defines S6a as "bind the GEM
native TSN backend to the emulated Cadence GEM and exercise the
`FF_TsnBackend_t` seam (SRS-REQ-038) — **delivered**" with exit criterion
`hal_tsn=PASS`. `README.md:418`, `CHANGELOG.md:12–18` and the status documents
repeat "exercised peripheral-bound (init + VLAN/PCP + 802.1AS TSU time)".

**Implemented content vs documented scope — DEVIATION (over-claim).** The
implementation does **not** achieve the binding (R1-01): the emulated GEM is
never accessed. The implemented content therefore does **not** match the
documented scope, and the "delivered / 10/13 peripheral-bound" statements are
over-claims. This is recorded as **NC R1-01 (Major)** — a scope/claim deviation,
not merely a code bug, because the increment's stated deliverable is the
peripheral-bound exercise.

**The S6 split and NC-S6-01.** Sprint 6's stated deliverable was "NET
lwIP-on-GEM peripheral-bound + TSN peripheral-bound". Delivering only part and
splitting into S6a + S6b with a scheduled NC is an acceptable, **honestly
recorded** disposition for the **NET** part: NC-S6-01 is recorded in
`sprint-plan.md` (S6b row + history), `CHANGELOG.md:19–24`, `status.json` note,
`handover.md`, `loop-audit.md`, `verification-results.md:50` and `README.md`,
with a scheduled sprint (S6b) and a stated reason. **Defects of the recording:**
no owner and not in §3 (**R4-03**). More importantly, NC-S6-01 as written implies
only the NET part is outstanding and the TSN part is done — which is false
(R1-01); a **new** Major NC (R1-01) must be opened and closed for the TSN part,
and the S6a "delivered" status must be withdrawn until then.

**Scope NC: R1-01 (Major)** — the TSN peripheral-bound scope is claimed but not
delivered. NC-S6-01 itself (NET deferral) is a correctly-recorded, scheduled
scope deviation (owner/§3 gaps = R4-03).

### 6.2 PA findings

- **Records / evidence (N11).** The increment adds the S6a row to
  `verification-results.md` (rev 0.38), the T17 update to `tool-qualification.md`
  (rev 0.3.21), the CHANGELOG entry and the history rows to every touched
  document. Records are configuration-identified and dated — **but** the S6a
  verification row and the T17 evidence claim rest on the false `hal_tsn=PASS`
  (R1-01) and must be corrected. The `traceability.md` mirror is stale (R4-01).
- **Claim discipline.** The SRS-REQ-038 text, the SRS-REQ-040 note and the
  802.1Qav `FF_NOT_IMPLEMENTED` / M8 gPTP statements are consistent and free of
  over-claim. The **peripheral-bound** claim is the exception (R1-01/R4-02) and
  is the central defect of this increment.
- **Residual-gap register (N5/N19).** GAP-A5 already tracks the TSN on-target
  timing/PTP residual; no new GAP-A* entry is needed for this increment. The
  `ci-renode` marker list is not machine-gated by `trace.py` (only the claim name
  is), which is why the missing SRS-REQ-038 entry (**R3-01**) passes the gate.
- **PA-01 — Minor — review log and semaphores (N11/N20).**
  `docs/reviews/README.md` has **no** row for FRICK-REVIEW-0148 (last row =
  Sprint-5 block closure), and `status.json` shows all gates GREEN
  (`reviews`, `documentation_review`, `product_assurance_review`) while the S6a
  loop is **open and REJECTED**. Add the log row(s) and refresh the semaphores at
  the corrective commit.
- **PA-02 — Comment — `status.json.deferred` is empty** while NC-S6-01 is a
  deferred deliverable. Consider recording the S6b deferral there (or rely on
  the sprint plan / loop-audit, which do track it). Advisory.
- **PA-03 — Comment — no independent oracle** for the TSN register semantics: the
  Renode model is the oracle and (per R1-02) does not even implement the
  screener. The TSU read is model-verified only; silicon remains GAP-A5.

### 6.3 What a qualified human must still accept (PA statement)

This agent cannot certify, approve, waive, accept residual risk or authorise
release. The following require independent qualified human acceptance:

1. **GAP-A5 TSN timing/PTP residual** — on-target 802.1AS accuracy (servo, BMCA,
   path-delay, the GEM TSU clock quality), the real 802.1Q screener / 802.1Qbv
   hardware behaviour, and the full gPTP / fabric CoreTSN stack (**M8**) are
   **not** verified by any evidence here. Even a corrected check validates only
   emulated register access, not silicon timing.
2. **The corrected TSN evidence** — the `base = 0` fix and the re-run
   `make renode` (with the TSU actually read and the VLAN/PCP model limit
   disclosed) must be reviewed and accepted before any "peripheral-bound"
   claim is made (R1-01/R1-02).
3. **Closure of the open findings** (R1-01 Major; R1-02, R3-01, R4-01, R4-02,
   R4-03, PA-01 Minors) and the doc corrections before commit/push (N12/N7).

## 7. Findings summary

| # | Severity | Profile | Item | Finding | Evidence | Required correction |
| - | -------- | ------- | ---- | ------- | -------- | ------------------- |
| **R1-01** | **Major** | R1/R2/R3 | `app_renode_test.c` `renode_hal_tsn_check`; SRS-REQ-038 | **Double register base**: `FF_TsnGemSetup(…, FF_GEM0_BASE, …)` while the injected bus already adds `FF_GEM0_BASE`; all TSN MMIO targets `0x4022xxxx` (nonexistent) → `hal_tsn=PASS` is a false pass, the emulated GEM/TSU is never exercised, the check cannot fail | `app_renode_test.c:111–115,129–133,150`; `ff_gem_tsn.c:15–23,128–136`; `renode-process.log` warnings at `0x40220540/0x402201C0/0x402201D0/0x402201D4`; `test_gem_tsn.c:48` | Pass `base = 0`; re-run `make renode` (no "non existing peripheral" warnings; TSU read); correct all propagating docs; re-review (N12) |
| R1-02 | Minor | R1/R4 | `ff_gem_tsn.c` `set_vlan`; docs | Renode `Network.CadenceGEM` does not implement the 802.1Q screener (`0x540`): "Unhandled write to offset 0x540" → VLAN/PCP is a no-op in the model; only the 802.1AS TSU read is genuinely peripheral-bound | probe `sysbus WriteDoubleWord 0x20110540` / `ReadDoubleWord 0x20110540`; TSU reads at `0x201101C0/…D4` produce no warning | Disclose the model limit in the comment + `renode-environment.md`/CHANGELOG/`verification-results.md`; or assert on an observable TSU effect |
| R3-01 | Minor | R3 | `ci-renode` markers | Both workflow markers omit SRS-REQ-038 (they list 036/039/044/047/048/049/050/055/057/058/069/070), while the manifest claims `ci:renode` for SRS-REQ-038; `trace.py` only validates the claim name | `.github/workflows/ci.yml:80`; `.gitea/workflows/ci.yml:83`; `traceability.yaml:367` | Add SRS-REQ-038 to both markers |
| R4-01 | Minor | R4 | `traceability.md:103` | Mirror row for SRS-REQ-038 still lists only `test_gem_tsn.c` (missing `ci:renode`) | `traceability.md:103` vs `:101/:104` | Update the row (`test_gem_tsn.c, ci:renode`) + history |
| R4-02 | Minor | R4 | doc set | "exercised peripheral-bound (init + VLAN/PCP + 802.1AS TSU time)" propagated inaccurately across 9 docs | `CHANGELOG.md:12–18`; `verification-results.md:50`; `renode-environment.md:95–107,169`; `handover.md:110–119,148,218`; `loop-audit.md:34,172`; `development-plan.md:317,356`; `README.md:418–421`; `tool-qualification.md:41`; `traceability.yaml` lean_note | Correct all occurrences with the code fix (manifestation of R1-01) |
| R4-03 | Minor | R4/PA | `sprint-plan.md` §3; NC-S6-01 | NC-S6-01 not entered in §3 (plan §5 requires it) and has no recorded owner | `sprint-plan.md:51,64–71,112` | Add NC-S6-01 to §3 with owner + sprint S6b |
| PA-01 | Minor | PA | review log / semaphores | No `docs/reviews/README.md` row for FRICK-REVIEW-0148; `status.json` all-GREEN while the S6a loop is open/REJECTED | `docs/reviews/README.md`; `status.json:26–41` | Add log row(s) + refresh semaphores at the corrective commit |
| R4-04 | Comment | R4 | `README.md:418–421` | "Next per plan: Sprint 7" omits S6b (the plan's next sprint); "S7 … closes M11 13/13" presumes S6b done first | `README.md:418–421`; `sprint-plan.md:50–52` | Reorder to "S6b, then S7" (advisory) |
| R4-05 | Comment | R4/N19 | `loop-audit.md:64` | N18 row example still cites "`hal_i2c=PASS` on the Sprint-3 image" | `loop-audit.md:64` | Make generic / current (advisory) |
| PA-02 | Comment | PA | `status.json.deferred` | Empty while NC-S6-01 is deferred | `status.json:48` | Consider recording the S6b deferral (advisory) |
| PA-03 | Comment | PA/R2 | TSN oracle | No independent oracle for TSN register semantics; the model omits the screener | §3 R1-02 | Track as a residual; silicon = GAP-A5 |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
  *(R1-01 — the TSN peripheral-bound verification evidence is false.)*
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified. *(R1-02, R3-01, R4-01, R4-02, R4-03, PA-01.)*
- **Comment**: observation/recommendation, no derogation. *(R4-04, R4-05,
  PA-02, PA-03.)*

## 8. Conclusion

- [ ] APPROVED — no findings or Comments only
- [ ] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [x] **REJECTED** — one or more Major findings

**Rationale.** The Sprint-6a increment's stated deliverable is a **peripheral-bound**
exercise of the GEM native TSN backend on the emulated Cadence GEM. The
implementation does not achieve it: a double register base sends every TSN MMIO
access to `0x4022xxxx`, an unmapped address, so `hal_tsn=PASS` is a **false
pass** and the emulated GEM/TSU is never exercised (**R1-01, Major**). The
aggregate `RENODE_TEST:` logic is correctly wired (a failed TSN term would fail
the marker), but the TSN term is vacuous and can never be false on the emulator.
All the gates the task requested re-run reproduce green — `ctest` **22/22**
native and ASan/UBSan, `trace.py` **PASS 114/71/29/14** (51 theorems, 12 CI
claims), `metrics --check` **current**, `clang-format` clean, and `make renode`
→ `RENODE_TEST: PASS` incl. `hal_tsn=PASS` — but the green Renode marker is
precisely the false evidence: the run log shows the TSN accesses landing on a
nonexistent peripheral. Because a Major finding is open, the increment is
**REJECTED** (norm N12/N7: commit/push is blocked until the Major is resolved and
the delta re-reviewed). The **NET** deferral disposition (**NC-S6-01**) is, by
contrast, honestly recorded with a scheduled sprint (S6b); its only defects are
the missing owner/§3 entry (R4-03) and the fact that it does not cover the TSN
defect. R3 (N14) finds the SRS-REQ-038/040 **wording** accurate but the
peripheral-bound **evidence** unsubstantiated; R4 (N17) finds the doc set
consistent in structure (all N16 headers match) but propagating the false claim
and with a stale `traceability.md` mirror; PA (N20) records the residual
on-target TSN timing/PTP risk (GAP-A5) for human acceptance.

## 9. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | Senior FSW / real-time / space software developer (independent context) | 2026-10-12 | **REJECTED** — Major R1-01 (double base → vacuous check); R1-02 |
| R2 (N9)       | Senior SW V&V engineer (independent context) | 2026-10-12 | **REJECTED** — the `ci:renode` evidence for SRS-REQ-038 is not substantiated; the check has no discriminating power (R1-01) |
| R3 (N14)      | Senior avionics / systems engineer (independent context) | 2026-10-12 | **REJECTED** — SRS-REQ-038/040 wording OK, but the peripheral-bound evidence is unsubstantiated (R1-01); R3-01, R4-01 |
| R4 (N17)      | Senior SW documentation engineer (independent context) | 2026-10-12 | **REJECTED** — the false claim propagates through 9 docs (R4-02); R4-01, R4-03; R4-04/R4-05 comments |
| PA (N20)      | Senior SW Product Assurance engineer (independent context) | 2026-10-12 | **REJECTED** — Major R1-01 (claim discipline / evidence integrity); NC-S6-01 correctly recorded but incomplete; PA-01; residual risk for human acceptance |

## 10. Follow-up records

(norm N12: NCs are resolved by re-applying all norms to the affected scope and
re-reviewing the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review | Scheduled sprint |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- | ---------------- |
| **R1-01** | **Major** | `app_renode_test.c` `renode_hal_tsn_check`; SRS-REQ-038 | double base → all TSN MMIO at `0x4022xxxx`; `hal_tsn=PASS` false | Pass `base = 0`; re-run `make renode`; confirm TSU read + no "non existing peripheral" warnings; correct docs | (delta) `app_renode_test.c` diff + fresh `renode-process.log` (no 0x4022 warnings) | frick-fsw | **open** | pending | **S6a delta (pre-commit)** |
| R1-02 | Minor | `ff_gem_tsn.c` `set_vlan`; docs | model does not implement the screener (`0x540`) → VLAN/PCP is a no-op | Disclose the model limit; or assert an observable TSU effect | (delta) `renode-environment.md`/comment diff + probe log | frick-fsw | **open** | pending | **S6a delta (pre-commit)** |
| R3-01 | Minor | `ci-renode` markers | SRS-REQ-038 missing from both `# evidence: ci-renode` lists | Add SRS-REQ-038 to both markers | (delta) workflow diff | frick-fsw | **open** | pending | **S6a delta (pre-commit)** |
| R4-01 | Minor | `traceability.md:103` | mirror row missing `ci:renode` | Update row + history | (delta) `traceability.md` diff | frick-fsw | **open** | pending | **S6a delta (pre-commit)** |
| R4-02 | Minor | doc set (9 docs) | false "exercised peripheral-bound" claim propagated | Correct all occurrences with the code fix | (delta) doc diffs | frick-fsw | **open** | pending | **S6a delta (pre-commit)** |
| R4-03 | Minor | `sprint-plan.md` §3; NC-S6-01 | NC-S6-01 not in §3; no owner | Add NC-S6-01 to §3 with owner + sprint S6b | (delta) `sprint-plan.md` diff | frick-fsw | **open** | pending | **S6a delta (pre-commit)** |
| PA-01 | Minor | `docs/reviews/README.md`; `status.json` | no log row for FRICK-REVIEW-0148; semaphores stale | Add log row(s); refresh semaphores | (delta) `README.md`/`status.json` diff | frick-fsw | **open** | pending | **S6a commit (same cycle)** |
| **NC-S6-01** | Minor (scope) | SRS-REQ-040 / sprint S6b | NET lwIP-on-GEM peripheral-bound deferred | Correctly recorded (S6b row + history + status/handover/loop-audit/CHANGELOG/README) — needs owner + §3 entry (R4-03); **does not cover the TSN defect (R1-01)** | `sprint-plan.md:51,112`; `CHANGELOG.md:19–24`; `status.json` note; `handover.md:119,148`; `loop-audit.md:34`; `verification-results.md:50` | frick-fsw (to be assigned) | **open (scheduled S6b)** | n/a | **S6b** |
| R4-04, R4-05, PA-02, PA-03 | Comment | — | Advisory (§5/§6) | Accepted / optional | — | frick-fsw | n/a | — | — |

## 11. Evidence ledger (all re-executed/obtained first-hand on 2026-10-12)

| Claim | Method | Result |
| ----- | ------ | ------ |
| Native ctest | `ctest --test-dir build/native` | **22/22 pass** |
| ASan/UBSan ctest | `cmake -S . -B build/asan -fsanitize=address` + `ctest` | **22/22 pass** |
| Traceability | `trace.py docs/requirements/traceability.yaml` | **PASS (114 requirements, 71 srs, 29 sss, 14 gaps, 51 lean theorems, 12 CI claims)** |
| Metrics panel | `metrics.py --check` | `metrics panel: current` |
| Formatting | `clang-format --dry-run --Werror src/apps/app_renode_test.c` | clean (exit 0) |
| Renode functional run (N18) | `make renode` | **PASS** (`RENODE_TEST: PASS`; `hal_tsn=PASS`) |
| TSN MMIO target (R1-01) | grep `build/renode/renode-process.log` | `ReadDoubleWord from non existing peripheral at 0x402201C0/0x402201D0/0x402201D4`; `WriteDoubleWord … 0x40220540, value 0x130` |
| Address arithmetic | `0x20110000 + 0x20110000 + off` | `0x40220540` (SCRT2), `0x402201C0/…D0/…D4` (TSU); `0x130 = (3<<4)|(1<<8)` = `set_vlan(1,3)` |
| GEM model scope (R1-02) | Renode probe `ReadDoubleWord/WriteDoubleWord` at `0x201101C0/…D0/…D4/0x20110540` | TSU registers implemented (no warning); `mac0: Unhandled write/read to/from offset 0x540` (screener absent) |
| Sibling convention | `renode_hal_eth_check` reads `0x201100FC`; `test_gem_tsn.c:48` uses `base=0` | bus is GEM0-relative → `base` must be 0 |
| Aggregate wiring | `app_renode_test.c:473–477` | `hal_tsn_ok` is an AND term (correct; but the term is vacuous) |
| ci-renode markers | both workflows, line 80/83 | SRS-REQ-038 **absent** |
| traceability mirror | `traceability.md:103` | SRS-REQ-038 row lacks `ci:renode` |
| N16 header/history | 7 touched docs | header `Revision`/`Date` = last history row |
| Review-log id | max committed id in `docs/reviews/` | `FRICK-REVIEW-0147` → `FRICK-REVIEW-0148` is next |

> Note: the gate commands regenerate build artifacts under `build/` (git-ignored)
> only; **no tracked source or documentation file was modified** by this review.
> The reviewer wrote **exactly one file** (this report); nothing was committed or
> pushed.

## 12. Norm-by-norm audit

| Norm | Status | Evidence / note |
| ---- | ------ | --------------- |
| N1 Lean | **pass** | `lake build` unaffected; SRS-REQ-038/040 carry explicit `lean_note` (no silent waiver). |
| N2 English | **pass** | All new/changed content is English. |
| N3 backwards SRS/SSS | **pass** | SRS-REQ-038/040 present and parented; text unchanged by this increment. |
| N4 traceability | **partial** | `trace.py` PASS, but the human mirror `traceability.md:103` is stale (R4-01). |
| N5 class-B/A gaps | **pass** | GAP-A5 TSN residual tracked; GAP-A1..A14 unchanged. |
| N6 V&V + deployed env | **partial** | Gate green, but the SRS-REQ-038 `ci:renode` evidence is not substantiated (R1-01). |
| N7 dual push | **n/a (pre-commit)** | Change is uncommitted; commit/push blocked until R1-01 is resolved (N12). |
| N8 docs current | **fail** | R4-02 (false claim in 9 docs), R4-01 (stale mirror), R4-03 (§3/owner). |
| N9 independent review | **pass** | This report (fresh context, five profiles). |
| N10 test scripts/tools | **partial** | T17 updated, but its evidence claim rests on the false `hal_tsn=PASS`; the check is not discriminating (R1-01). |
| N11 records in repo | **partial** | Records present; PA-01 (log row) open; S6a verification row rests on false evidence. |
| N12 NC closure | **in progress** | R1-01 (Major) + six Minors open, tracked with owner + sprint; deltas to be re-reviewed. |
| N13 licensing | **pass** | No new source files; SPDX unaffected. |
| N14 SSS review | **partial** | R3 finds requirement wording OK but the peripheral-bound evidence unsubstantiated (R1-01); R3-01. |
| N15 full cycle | **pass (this cycle)** | Build/tests/trace/metrics/format/Renode reproduced green (the Renode green is the false evidence). |
| N16 revision history | **pass** | All touched docs' header = last history row; CHANGELOG S6a entry present. |
| N17 documentation review | **fail** | R4 finds the false claim propagated and the mirror stale. |
| N18 Renode functional | **fail (substantively)** | `make renode` marker PASS, but the TSN check targets a nonexistent peripheral (R1-01); the marker does not evidence the claimed exercise. |
| N19 loop audit | **partial** | `loop-audit.md` refreshed but its M11/M4 rows over-claim TSN "done"; N18 example stale (R4-05). |
| N20 PA review | **fail** | PA performed; Major R1-01 open; PA-01; residual risk flagged for human acceptance. |

---

## Facts, Assumptions, Inferences, Recommendations, Confidence, Open gaps

**Facts** (verified first-hand, this session)

- The Sprint-6a working tree passes `FF_TsnGemSetup(&tsn, &bus, FF_GEM0_BASE, 1000u)` while the injected `mpfs_gem_read32/write32` already add `FF_GEM0_BASE`; the backend adds `base` again (`ff_gem_tsn.c:15–23,128–136`).
- A fresh `make renode` logs `ReadDoubleWord from non existing peripheral at 0x402201C0/0x402201D0/0x402201D4` and `WriteDoubleWord to non existing peripheral at 0x40220540, value 0x130` — i.e. all TSN MMIO targets the doubled address `0x4022xxxx`.
- `0x20110000 + 0x20110000 + 0x540 = 0x40220540` and `+0x1C0/…D0/…D4 = 0x402201C0/…D0/…D4`; `0x130 = (3<<4)|(1<<8)` corresponds to `set_vlan(1,3)`.
- The Renode `Network.CadenceGEM` model implements the 802.1AS TSU registers (no warning at `0x201101C0/…D0/…D4`) but **not** the 802.1Q screener (`Unhandled write to offset 0x540`).
- The sibling `renode_hal_eth_check` reads `0x201100FC` and passes; `test_gem_tsn.c:48` uses `base=0` — so the bus is GEM0-relative and `base` must be `0`.
- `hal_tsn_ok` **is** an AND term of the `RENODE_TEST:` marker (`app_renode_test.c:473–477`).
- Both `# evidence: ci-renode` markers omit SRS-REQ-038; `traceability.md:103` omits `ci:renode`.
- Gates reproduce green: `ctest` 22/22 native and ASan, `trace.py` PASS 114/71/29/14 (51 theorems), `metrics --check` current, `clang-format` clean, `make renode` → `RENODE_TEST: PASS` incl. `hal_tsn=PASS`.
- All seven touched `## History`-bearing docs have header = last history row (N16).

**Assumptions** (owner, impact, validation)

- *Document id `FRICK-REVIEW-0148`* — owner: review lead; impact: report identity only.
- *The uncommitted working-tree delta is the item under review atop `56ada82`.*
- *The Renode `Network.CadenceGEM` model is the authoritative oracle for "peripheral-bound on the emulated GEM"* — validation: the model is the only emulator present; silicon = GAP-A5.
- *`base = 0` is the intended convention* — validation: `test_gem_tsn.c:48` + `renode_hal_eth_check`; confirm with the author.

**Inferences**

- Because the TSN accesses land on a nonexistent peripheral, the check can never return `false` on the emulator, so `hal_tsn=PASS` carries no information and the "M11 10/13 peripheral-bound" claim is really 9/13.
- Because `set_vlan`'s write is discarded by the model (screener absent) even at the correct base, the "VLAN/PCP exercised peripheral-bound" claim cannot be satisfied on this model without an observable assertion or a model that implements the screener.
- Because the manifest gained `ci:renode` for SRS-REQ-038 but neither the workflow markers nor `traceability.md` were updated, the evidence-to-requirement mapping is inconsistent (trace.py gates only the claim name).

**Recommendations** (decision owner, prerequisites, trade-offs)

1. **Resolve R1-01 first**: pass `base = 0` to `FF_TsnGemSetup`; re-run `make renode` and confirm the TSN accesses no longer emit "non existing peripheral" warnings and that the TSU registers are read; consider asserting on an observable TSU property so the check has discriminating power.
2. **R1-02**: disclose the model's missing screener (comment + `renode-environment.md` + CHANGELOG + `verification-results.md`), or narrow the claim to the 802.1AS TSU read.
3. **R3-01 / R4-01**: add SRS-REQ-038 to both `ci-renode` markers and update `traceability.md:103`.
4. **R4-02**: correct every propagating "exercised peripheral-bound" statement in the same cycle as the code fix.
5. **R4-03 / PA-01**: add NC-S6-01 to `sprint-plan.md` §3 with an owner + sprint S6b; add the `docs/reviews/README.md` row for FRICK-REVIEW-0148 and refresh the semaphores.
6. Keep the GAP-A5 on-target TSN timing/PTP residual (and the M8 gPTP/CoreTSN stack) tracked; obtain a datasheet/model-independent oracle for the TSN register semantics before any flight claim.
7. Re-review the deltas (same profiles, fresh context) before commit/push (N12/N7).

**Confidence:** **High** for R1-01 (direct code read + the increment's own run-log warnings + exact address arithmetic, all mutually confirming). **High** for R1-02 (first-hand Renode probe of the model). **High** for R3-01/R4-01/R4-02/R4-03 (direct source reads). **Medium** for the significance of R4-04/R4-05/PA-02/PA-03 (wording/advisory, no functional impact). Confidence is not a probability of safety.

**Open gaps**

- **R1-01 (Major)** open — blocks commit/push until resolved and the delta re-reviewed (N12).
- **R1-02, R3-01, R4-01, R4-02, R4-03, PA-01 (Minor)** open — must be closed before commit/push.
- **NC-S6-01 (scope)** open and scheduled (S6b); needs an owner and a §3 entry.
- **No independent oracle** for the TSN register semantics; the Renode model (which omits the screener) is the only emulator.
- **On-target evidence absent** — 802.1AS PTP accuracy, TSU clock quality, real screener/Qbv hardware, gPTP/CoreTSN stack (M8) → **GAP-A5**; the residual risk is for **human acceptance**.

---

*This report is authored to be committed and pushed with the reviewed change
(norm N11). The review was read-only with respect to repository content: the
reviewer wrote exactly one file (this report); no other tracked file was
modified, created or deleted, and nothing was committed or pushed. The gate
commands regenerate only git-ignored `build/` artifacts. Independence is
declared per norm N9 (fresh context; no part in authoring or previously
reviewing the items).*
