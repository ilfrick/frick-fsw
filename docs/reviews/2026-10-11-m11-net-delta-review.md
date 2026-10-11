# Review Report — M11 NET delta re-review (dispositions of FRICK-REVIEW-0048 / 0049 / 0050)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0051                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R2 (SW V&V, N9/N10) — **delta re-reviewer, norm N12** |
| Reference revision                | base commit `205bfc5` + uncommitted working tree (M11 NET delta dispositions, 2026-10-11) |
| Items under review                | the **dispositions** recorded by the author for the three parallel M11 NET reviews: FRICK-REVIEW-0048 (R4 docs), FRICK-REVIEW-0049 (R1 FSW), FRICK-REVIEW-0050 (R2 V&V), against the current tree — `src/apps/app_renode_test.c`; `tools/scripts/renode_vv.sh`; `.github/workflows/ci.yml` / `.gitea/workflows/ci.yml`; `docs/development-plan.md`; `docs/status/{handover,loop-audit}.md`; `docs/vv/{renode-environment,verification-results,tool-qualification}.md`; `docs/requirements/{traceability.yaml,traceability.md,SRS.md}`; `CHANGELOG.md` |
| Date                              | 2026-10-11                             |
| Scope                             | Independent **delta re-review (N12)** verifying that each claimed disposition is actually present in the current working tree and, where executable, actually holds under re-execution. Reproduced `make renode`, the native `ctest` suite, `trace.py`, the `clang-format` gate in `frick-vv:local`, a sabotage probe for the strengthened oracle and a failure-path leak probe. ECSS-E-ST-40C, ECSS-Q-ST-80C. A fresh full re-review of the NET increment beyond the dispositions is out of scope (the three underlying reports remain the records of record); on-target lwIP/GEM remains GAP-A5. |
| Independence declaration          | Reviewer took no part in authoring, implementing or executing any reviewed item; fresh independent session (spawned by a separate session). Only this report was written; no other tracked repository file was modified. Throwaway probes were built under `/tmp/opencode/netdelta` (outside the repository); the gitignored `build/` was rebuilt. The Renode functional run was re-executed independently against the pinned image `antmicro/renode@sha256:7905c329…` and the RTEMS/RISC-V ELF. |
| Norms applied                     | N1-N19 (see `docs/rules.md`)           |

## 1. Scope

Re-reviewed the eight dispositions the author recorded for the fifth M11 HAL
increment (NET exercised in the Renode run through the pluggable
`FF_NetStack_t` contract using the reference loopback stack):

1. **Scope reclassification** (R1-01 / R2-01 / R4-04): NET is now documented as a
   `FF_NetStack_t` **contract/behaviour check** (in-process loopback), *not*
   peripheral-bound; M11 is stated as **4/13 peripheral-bound** (GPIO/SERIAL/MEM/
   ETH) with "NET (lwIP-on-GEM)" pending.
2. **Oracle strengthened** (R1-02): non-zero port `0x42`, `accept` must report the
   server port, and a negative case (connect to an unlistened port must fail).
3. **Cleanup on failure** (R1-03): failure paths reset via `init()`.
4. **`renode_vv.sh` robustness** (R2-06 / R1-08): UART log copied to a
   host-writable `/tmp` path (`/tmp/frick-console.log`) and best-effort to
   `build/renode`.
5. **`ci:renode` binding** (R2-02): both workflow evidence markers now list
   SRS-REQ-036/039/044/047/048/049 and SSS-REQ-013.
6. **T17 register** (R2-03): `src/apps/app_renode_test.c` registered under T17
   plus a re-qualification history row.
7. **Doc consistency** (R4-01/R4-02/R4-03): `traceability.md` SRS-REQ-039 row
   (+`ci:renode`, revision/history), `development-plan.md` M11 row reconciled,
   `handover.md` revision/History bumped.

Each disposition was checked in the tree and, where executable, re-executed. The
result is summarised in the matrix (§3); findings that remain are in §4.

## 2. References

- Project norms: `docs/rules.md` (N1-N19, in particular N4/N8/N10/N11/N12/N15/N16/N17/N18/N19); template: `docs/reviews/review-template.md`
- Reviews under disposition: FRICK-REVIEW-0048 (R4, `2026-10-11-m11-net-r4-review.md`), FRICK-REVIEW-0049 (R1, `2026-10-11-m11-net-r1-review.md`), FRICK-REVIEW-0050 (R2, `2026-10-11-m11-net-r2-review.md`)
- Predecessor: FRICK-REVIEW-0032 (M4 increment 4, reference loopback; **REJECTED**, R1-02/R1-03 open, no delta report recorded)
- Contract under test: `src/hal/hal_net.h` (SRS-REQ-027); requirement under change: SRS-REQ-039
- N18 evidence chain: `tools/scripts/renode_vv.sh`, `tools/docker/renode/frick-renode.resc`, `.github/workflows/ci.yml` / `.gitea/workflows/ci.yml`, `docs/vv/tool-qualification.md`
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2

## 3. Disposition verification matrix

| # | Claimed disposition | Where verified | Verdict | Evidence |
| - | ------------------- | -------------- | ------- | -------- |
| 1 | Scope reclassification to a non-peripheral `FF_NetStack_t` contract check; M11 = 4/13 peripheral-bound | `development-plan.md:294` ("in progress (4/13 peripheral-bound) … Plus a **NET `FF_NetStack_t` contract check** … in-process, **not** peripheral-bound; Pending: NET(lwIP-on-GEM)/…"), `loop-audit.md:34`, `handover.md:95-103`, `renode-environment.md:86-95`, `CHANGELOG.md` NET entry, `traceability.yaml:377` `lean_note`, `verification-results.md:36` | **Substantially closed** (residual D-01) | Read in tree; `development-plan.md` pending list = 9 incl. NET(lwIP-on-GEM) → 4+9=13; `loop-audit.md` "8 classes pending"; `renode-environment.md` adds "in-process — **not** peripheral-bound" |
| 2 | Oracle strengthened: port `0x42`, `accept` reports server port, negative unlistened-port case | `src/apps/app_renode_test.c:135-183` | **Closed** (empirically) | Sabotage probe: sabotaged `netloop_bind` → **NEW=FAIL, OLD=PASS**; sabotaged `netloop_accept` peer write → **NEW=FAIL, OLD=PASS**; real loopback **NEW=PASS** |
| 3 | Failure paths reset via `init()` | `src/apps/app_renode_test.c:129-133,156-177` | **Closed** (functionally; note D-05) | Forced-accept-failure probe → check returns FAIL and **4/4** sockets available afterwards (no retention) |
| 4 | `renode_vv.sh`: console to host-writable `/tmp` + best-effort `build/renode` | `tools/scripts/renode_vv.sh:25,46-52` | **Closed** | `make renode` → `[renode] PASS`; with `build/renode` made read-only (`chmod 555`) and `console.log` removed, `make renode` **still PASS** (marker reached `LOG` via `/tmp/frick-console.log`) |
| 5 | `ci:renode` marker lists SRS-REQ-036/039/044/047/048/049 + SSS-REQ-013 (both workflows) | `.github/workflows/ci.yml:76`, `.gitea/workflows/ci.yml:79` | **Partially closed** (residual D-03) | Both markers updated identically; **SRS-REQ-050** (also `ci:renode` in the manifest, `traceability.yaml:476`) and **SSS-REQ-020** (parent of SRS-REQ-039) are absent |
| 6 | T17 register: `app_renode_test.c` + re-qualification history row | `tool-qualification.md:41,95` | **Partially closed** (residual D-02) | T17 location list now includes `src/apps/app_renode_test.c`; History `0.3.1` added. **Evidence-claims mapping table (`:57-68`) still has no `ci:renode` → T17 row**; N10 gate wording (`rules.md:143-144`) unchanged |
| 7 | Doc consistency: `traceability.md` row/rev/history; `development-plan` M11; `handover` rev/History | `traceability.md:7,98,195-196`; `development-plan.md:294`; `handover.md:7,170` | **Closed** (with D-01 note) | `traceability.md` SRS-REQ-039 → `test_netloop.c, ci:renode`, rev **0.22**, History 0.22; `development-plan` M11 reconciled; `handover` rev **0.9** + History 0.9 |

## 4. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| D-01 | Minor | `docs/status/handover.md:99`; `docs/vv/renode-environment.md:86-90` (cross-ref R4-04/R1-01/R1-07) | **The peripheral-vs-contract distinction is still not applied consistently in the documents the delta touched.** `handover.md` (the resume point) now states "**GPIO/SERIAL/MEM/NET go through their HAL backends**" — grouping the in-process loopback with the peripheral-backed classes, the exact over-claim R1-01/R4-04 required be removed. `renode-environment.md` §5 still opens "the functional run also exercises HAL backends **on the emulated peripherals**:" and then lists NET (mitigated only by the added "(in-process — not peripheral-bound)" qualifier mid-sentence). The delta therefore closed R4-04 in name but left/perpetuated the framing in two documents. | `handover.md:95-103`; `renode-environment.md:86-95`; `ff_net_loopback.h:1-11`; `app_renode_test.c:135-183` (no MMIO/netif) | Reword `handover.md` to "GPIO/SERIAL/MEM (and ETH identification) are exercised through their HAL backends; NET is a `FF_NetStack_t` contract check via the in-process loopback (no emulated peripheral)" and split the `renode-environment.md` topic sentence so NET is not listed under "on the emulated peripherals". |
| D-02 | Minor | `docs/vv/tool-qualification.md:57-68`; `docs/rules.md:143-144`; N10 (cross-ref R2-03) | **R2-03 is only partially closed.** The testware registration (b) and the re-qualification history row (c) are done, but (a) the **Evidence-claims mapping table does not map `ci:renode` to any qualification ID** (no T15/T17 row), and (d) `rules.md` N10 still asserts *"the gate `make trace` checks that every script referenced as evidence exists and is listed in the qualification register"*, which `trace.py` does **not** implement (`check_path()` returns `True` for any `ci:` value and the register is never parsed). The documented gate does not exist and the reviewed `ci:renode` claim therefore still rests on unmapped/unenforced qualification. | `tool-qualification.md:57-68` (rows T1/T5/T6/T7/T3/T8/T10/T11 only); `rules.md:143-144`; `trace.py` `ci_evidence()`/`check_path()` | Add a `ci:renode → T17` (and `ci:renode`/T15) row to the mapping table; either implement the register check in `trace.py`/CI or correct the N10 text to match the implementation. |
| D-03 | Comment | `.github/workflows/ci.yml:76`; `.gitea/workflows/ci.yml:79`; `docs/requirements/traceability.yaml:376,476` | **The `ci:renode` marker is still not a complete binding.** `SRS-REQ-050` (`traceability.yaml:476`) also declares `ci:renode`, and `SSS-REQ-020` is the parent of SRS-REQ-039, but neither is listed; `trace.py` cannot detect the omission (it does not validate the parenthetical requirement list — R2-02). The added SRS-REQ-039 binding (the specific R2-02 complaint) is present and correct. | `traceability.yaml` `ci:renode` consumers: 036/039/044/047/048/049/**050**; marker lists 036/039/044/047/048/049; `SSS-REQ-020` parent (`traceability.yaml:373`) | Add SRS-REQ-050 and SSS-REQ-020 to both markers, or accept in writing that the parenthetical is indicative only. |
| D-04 | Comment | `tools/scripts/renode_vv.sh:49-52` (cross-ref R1-08/R2-06) | **The false *FAIL* is fixed, but the capture remains silently best-effort, introducing a (lower-probability) false *PASS* path.** `docker cp … "${CONSOLE}" … \|\| true` still hides a copy failure, and `${CONSOLE}` (`/tmp/frick-console.log`) is neither removed nor truncated before the copy. If a later run's `docker cp` fails while a previous run left a passing `/tmp/frick-console.log`, the stale content is appended to the fresh `LOG` and the gate can print `PASS` without a fresh marker. (Verified the primary case: a read-only `build/renode` no longer causes a false FAIL.) | `renode_vv.sh:46-55`; read-only-`build/renode` reproduction → PASS; stale-destination reasoning from the script | `rm -f "${CONSOLE}"` before the `docker cp`, and/or check the `docker cp` exit status and fail/report distinctly. Changing T17 re-qualifies it under N10. |
| D-05 | Comment | `src/apps/app_renode_test.c:129-133,179-181` (cross-ref R1-03) | **The R1-03 disposition resets the table via `init()` instead of closing descriptors and checking `close` status.** It achieves the objective (no descriptor retention, proven) and is acceptable for a single-use reference stack, but the failure path and the happy path both still discard `close()` returns with `(void)`, so a lifecycle failure is invisible. | forced-accept-failure probe → 4/4 slots free; `app_renode_test.c:179-181` (`(void)s->close(...)`) | Acceptable as-is for the reference stack; if the stack gains other callers, close-and-check. |
| D-06 | Comment | Multiple (cross-ref R1-04/R1-05/R1-06/R1-07, R2-04, R4-05..R4-12) | **Pre-existing findings outside this delta's disposition scope remain open** and are listed in the follow-up table (§7). Notably: R1-07 — FRICK-REVIEW-0032 is still `REJECTED` in `docs/reviews/README.md` with no delta report, and its R1-02 (relay/`pending` orphan) / R1-03 (duplicate `bind`) items are unfixed; and R4-05 — "pluggable TCP/IP" terminology persists in `handover.md:98`, `renode-environment.md:120`, `verification-results.md:352`, `SRS.md:194`. These are the residual items the delta did not claim. | §7; `docs/reviews/README.md:33`; `renode_vv.sh`; grep of the four documents | Close or explicitly track; record the FRICK-REVIEW-0032 delta re-review (N12). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

## 5. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**APPROVED WITH COMMENTS.** No Major finding. The delta's substantive
dispositions are correct and independently reproducible: the scope
reclassification is internally consistent (4/13 peripheral-bound, NET a contract
check), the **oracle is genuinely strengthened** (a sabotaged `bind`/`accept` now
fails the check where the old port-0 oracle passed), the failure path no longer
retains descriptors, the `renode_vv.sh` false-FAIL on a root-owned `build/renode`
is eliminated, the `ci:renode` marker now binds SRS-REQ-039, the Renode testware
is registered under T17 with a re-qualification row, and the R4-01/R4-02/R4-03
documentation-consistency items are closed. All the recorded evidence reproduces:
`make renode` → `RENODE_TEST: PASS` with `hal_net=PASS`; native `ctest` 14/14;
`trace.py` PASS (94/54/26/14/51); `clang-format --dry-run --Werror` clean in
`frick-vv:local`.

The delta is **not** fully closed as claimed, however: three of the seven
dispositions are only partially realised (D-01 handover/renode-environment
framing; D-02 R2-03 mapping + N10 gate wording; D-03 marker completeness), and a
low-probability false-PASS path remains in the console capture (D-04). These are
Minor/Comment issues to be corrected or justified and tracked to closure under
N12; together with the pre-existing open findings (R1-04..R1-07, R2-04, R4-05..
R4-12) they must be dispositioned before the increment is closed.

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R2 (SW V&V delta re-review, N9/N12) | independent agent (FRICK-REVIEW-0051) | 2026-10-11 | APPROVED WITH COMMENTS (D-01/D-02 Minor; D-03..D-06 Comment/Open residuals) |

## 7. Follow-up records

Status of every finding of the three parallel reviews after this delta **as
observed in the current working tree** (norm N12; disposition owner is the
frick-fsw author). "Delta re-review" = this report.

| NC | Sev | Disposition claim | Status now | Evidence / residual |
| -- | --- | ----------------- | ---------- | ------------------- |
| R1-01 | Minor | Docs reworded; contract check not peripheral-bound | **CLOSED** (residual D-01) | `development-plan.md:294`, `loop-audit.md:34`, `CHANGELOG.md`, `renode-environment.md`, `traceability.yaml:377` |
| R1-02 | Minor | Non-zero port + peer assert + negative case | **CLOSED** | sabotage probe NEW=FAIL / OLD=PASS; `app_renode_test.c:138-162,169` |
| R1-03 | Minor | Failure paths reset via `init()` | **CLOSED** (note D-05) | forced-failure probe → 4/4 slots free |
| R1-04 | Comment | (not claimed) | **OPEN** | single-bool marker unchanged |
| R1-05 | Comment | (not claimed; peer now asserted) | **PARTIAL** | `hal_net.h:36` `peer_len` semantics still undefined; peer asserted in check |
| R1-06 | Comment | (not claimed) | **OPEN** | `recv` `timeout_ms` limitation not stated |
| R1-07 | Comment | (not claimed) | **OPEN** | FRICK-REVIEW-0032 still `REJECTED`, no delta report; R1-02/R1-03 loopback items unfixed |
| R1-08 | Comment | `/tmp` console + best-effort `build/renode` | **CLOSED** | `make renode` PASS with read-only `build/renode`; note D-04 |
| R2-01 | Minor | NET reclassified; M11 = 4/13 peripheral-bound; conflict with R4-02 resolved | **CLOSED** | `development-plan.md:294`; reconciled with R4-02 |
| R2-02 | Minor | Marker lists SRS-REQ-039 + others | **PARTIAL** (D-03) | SRS-REQ-050 / SSS-REQ-020 omitted; `trace.py` does not enforce |
| R2-03 | Minor | Testware registered under T17 + history row | **PARTIAL** (D-02) | mapping table lacks `ci:renode`; N10 gate wording unimplemented |
| R2-04 | Minor | Claim scoped; negative case added | **PARTIAL** | still implementation-coupled (peer = same loopback); subset of `test_netloop.c` |
| R2-05 | Comment | (positive) | **CLOSED** | conjunction intact (`app_renode_test.c:266-269`) |
| R2-06 | Comment | Console copy robustness | **CLOSED** | note D-04 |
| R2-07 | Comment | "end-to-end" wording replaced | **CLOSED** | `traceability.yaml:377` reworded; `network-stack.md:32` not updated (optional) |
| R4-01 | Minor | Mirror row + `ci:renode` + rev 0.22 + History | **CLOSED** | `traceability.md:7,98,195-196` |
| R4-02 | Minor | `development-plan.md` M11 reconciled | **CLOSED** | `:294` = 4/13 peripheral-bound |
| R4-03 | Minor | `handover.md` rev 0.9 + History | **CLOSED** | `:7,170` |
| R4-04 | Minor | `renode-environment.md` reworded | **PARTIAL** (D-01) | qualifier added; topic sentence + `handover.md:99` still group NET as peripheral-backed |
| R4-05 | Minor | (not claimed) | **OPEN** | "pluggable TCP/IP" in `handover.md:98`, `renode-environment.md:120`, `verification-results.md:352`, `SRS.md:194` |
| R4-06 | Comment | (not claimed) | **OPEN** | `SRS.md:192-196` still "host simulator" + "TCP/IP" |
| R4-07 | Comment | (not claimed) | **OPEN** | `loop-audit.md` rev 0.1, baseline `e8daab3` |
| R4-08 | Comment | (not claimed) | **OPEN** | `status.json:19` `"M11": "planned"` |
| R4-09 | Comment | (not claimed) | **OPEN** | header Dates 2026-10-10 at rev 0.9/0.22 |
| R4-10 | Comment | (not claimed) | **OPEN** | `app_renode_test.c:8` still GPIO-only |
| R4-11 | Comment | (not claimed) | **OPEN** | trailing newline still missing in `traceability.yaml`, `traceability.md`, `verification-results.md`, `tool-qualification.md` |
| R4-12 | Comment | (author to append rows at commit) | **OPEN (pending)** | `docs/reviews/README.md` tail still ends at M11 ETH; FRICK-REVIEW-0048/0049/0050 rows and this report not yet logged |

### Empirical evidence (reproduced by this reviewer)

**Target run (the reviewed claim)**

- `make renode` → `[renode] PASS (RENODE_TEST: PASS)` (exit 0); `/tmp/frick-console.log` and `/tmp/frick-renode.log` contain `hal_gpio=PASS`, `hal_serial=PASS`, `hal_mem=PASS`, `hal_eth=PASS`, **`hal_net=PASS`**, `sb_roundtrip=PASS`, `RENODE_TEST: PASS` (pinned `antmicro/renode@sha256:7905c329…`, emulated PolarFire SoC).
- **False-FAIL robustness (D-04 context):** with `build/renode` set read-only (`chmod 555`) and `console.log` deleted, `make renode` **still** prints `[renode] PASS` → the root-owned-`build/renode` false FAIL is fixed.

**Host gates**

| Command | Result |
| ------- | ------ |
| `ctest --test-dir build/delta-review` (fresh Ninja Debug) | **14/14 PASS**; `test_netloop` `checks=29 failures=0` |
| `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 94 requirements (54 srs / 26 sss / 14 gaps), 51 lean theorems, 12 CI evidence claims |
| `docker run … frick-vv:local … clang-format --dry-run --Werror` over `src config tests` | **clean** (exit 0) |

**Independent probes (outside the repository, `/tmp/opencode/netdelta`)**

- **Oracle strengthening (R1-02):** replicas of the current check and of the base
  port-0 check executed against the real loopback and two sabotaged copies:
  `ok: NEW=PASS OLD=PASS`, `sab_bind: NEW=FAIL OLD=PASS`, `sab_acc: NEW=FAIL OLD=PASS`.
  The strengthened oracle detects the exact `bind`/peer regressions the old oracle
  could not.
- **Failure-path cleanup (R1-03):** with `netloop_accept` forced to fail, the check
  returns FAIL and **4/4** sockets are available immediately afterwards (no
  retention).

### Positive evidence

- The reclassification is applied coherently in `development-plan.md`,
  `loop-audit.md`, `CHANGELOG.md`, `traceability.yaml` and (with the D-01 caveat)
  `renode-environment.md`; counts are internally consistent (4 + 9 = 13).
- `CHANGELOG.md` NET entry and `verification-results.md:36` are honestly scoped
  (contract check, in-process, not an emulated-peripheral binding; no lwIP/GEM
  claim).
- The `hal_net_ok` conjunction into the `RENODE_TEST` marker is intact, so a NET
  failure yields `RENODE_TEST: FAIL` (`app_renode_test.c:266-269`).
- `traceability.yaml:377` `lean_note` no longer says "end-to-end" (R2-07); the
  `ci:renode` addition for SRS-REQ-039 is real (the Renode run executes the
  referenced implementation).

### Uncertainty and confidence

```
Product/configuration and decision supported: frick-fsw M11 NET delta (base 205bfc5 + working tree) — disposition re-review
Information item/revision/status: FRICK-REVIEW-0051 (delta re-review of FRICK-REVIEW-0048/0049/0050)
Claim type: Fact (reproduced runs, quoted code/docs) + Inference (partial-closure judgement) + Recommendation
Applicable baseline/tailoring: repo norms N1-N19 (docs/rules.md); ECSS-E-ST-40C, ECSS-Q-ST-80C
Sources and access level: full repository text (public, read directly); Renode run, ctest, trace.py, clang-format and the sabotage/leak probes re-executed
Independent cross-check and conflict reconciliation: code vs unit test vs app check vs gate vs CI marker vs qualification register vs objective, compared directly; R2-01/R4-02 conflict reconciled by the reclassification
Confidence: High (reproduction/verification of dispositions); Medium (D-04 false-PASS is a reasoned script-analysis risk, not reproduced with a stale destination; the intended OBJ-19 acceptance wording remains the authority's call)
Assumptions, TBD/TBC, detection limits: on-target lwIP/GEM data path not exercisable without hardware/firmware (GAP-A5); the review re-executes the emulated run, not silicon
Consequences if wrong: a milestone/evidence claim could be recorded as closed while minor traceability/qualification and documentation-honesty gaps remain
Required technical owner/reviewer/approver: frick-fsw author (D-01..D-05 and the open residuals), with R4/R1 delta re-review for the documentation and code items; FRICK-REVIEW-0032 closure by the authority
Recheck trigger/date: on the next delta re-review after the fixes (norm N12)
```

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-11 | independent reviewer agent (R2, FRICK-REVIEW-0051) | Initial N12 delta re-review of the M11 NET dispositions |

---

Committed and pushed with the reviewed change once all findings are closed and the
deltas re-reviewed (norms N11/N12). The author must append the FRICK-REVIEW-0048,
0049, 0050 and 0051 rows to `docs/reviews/README.md` in the same commit (this
review was scoped to write this report only).

---

## 8. Post-review closure addendum (author)

- **D-01 fixed:** `handover.md` and `renode-environment.md` separate the
  **peripheral-bound** classes (GPIO/SERIAL/MEM/ETH; M11 4/13) from the **NET
  `FF_NetStack_t` contract check** (loopback, not peripheral-bound).
- **D-02 fixed:** T17 covers `src/apps/app_renode_test.c`; an evidence-claims
  mapping row added (`ci:renode` -> T17); `rules.md` N10 wording corrected (the
  gate checks evidence-path existence; the register is maintained manually).
- **D-03 fixed:** the `ci:renode` marker also binds SRS-REQ-050 and SSS-REQ-020.
- **D-04 fixed:** `renode_vv.sh` truncates the host console copy before `docker cp`.
- **D-05 (Comment):** `close()` status discarded (benign; `init()` resets on the
  failure path; `test_netloop.c` covers close semantics).
- **D-06 (Comment, tracked):** pre-existing FRICK-REVIEW-0032 loopback items
  (relay/`pending` orphan, duplicate `bind`) remain open for a follow-up M11/M4
  loopback-hardening increment; not part of the NET class check.
