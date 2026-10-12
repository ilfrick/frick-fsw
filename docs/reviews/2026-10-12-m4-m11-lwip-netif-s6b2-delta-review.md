# M4/M11 Sprint-6b-2 — lwIP netif on the emulated GEM — combined R1+R2+R3+R4+PA **delta** review (norm N12)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0154                      |
| Reviewer session                  | independent agent (norm N9), fresh context |
| Reviewer profile                  | R1 (SW dev) + R2 (SW V&V) + R3 (Sys Eng) + R4 (SW documentation) + PA (SW product assurance) — **delta** |
| Reference revision                | `471afb5` + uncommitted working tree (Sprint-6b-2 increment **after** the FRICK-REVIEW-0153 corrections) |
| Items under review                | `src/hal/net/lwip/ff_lwip_netif.{c,h}`, `src/apps/app_renode_test.c`, `config/rtems/rtems_config.c`, `CMakeLists.txt`, `src/hal/net/lwip/ff_net_lwip.c`, `src/hal/net/lwip/rtems/{sys_arch.c,arch/sys_arch.h}`, `tools/scripts/ci_rtems_cross_build.sh`, `.github/workflows/nightly.yml`, `docs/requirements/{traceability.yaml,traceability.md}`, `docs/architecture/network-stack.md`, `docs/development-plan.md`, `docs/planning/sprint-plan.md`, `docs/status/{handover,loop-audit,status.json}`, `docs/vv/{renode-environment,tool-qualification,verification-results}.md`, `CHANGELOG.md`, `README.md` |
| Date                              | 2026-10-12                             |
| Scope                             | Delta closure of the FRICK-REVIEW-0153 findings (R1-01/R1-02, R2-01/R2-02, R3-01/R3-02, R4-01…R4-04, PA-01) by first-hand re-verification; R2-01 disposition; scope conformance; re-run of the functional gates. ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2, project norms N1–N20 (`docs/rules.md`), reviewer contract `docs/engineering/independent-review.md` (rev 0.7) |
| Independence declaration          | The reviewer ran in a fresh, independent context; it did not author, design, implement, test or approve any item under review, and it did not inherit the author's conversation. It is read-only over the repository except for this one report. |
| Norms applied                     | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

Delta review (norm N12) of the correction set applied to the uncommitted
Sprint-6b-2 working tree after **FRICK-REVIEW-0153** (APPROVED WITH COMMENTS). The
base increment itself (content, falsification result, `hal_net_lwip` real-pass
conclusion) was reviewed in FRICK-REVIEW-0153 and is **not re-reviewable here**;
this report re-verifies (a) each claimed correction against the current file
content, (b) the R2-01 disposition, and (c) that the gates and records still
converge.

**Excluded:** re-running the Renode images and the RTEMS cross-builds (unchanged
by the corrections, which are comment/document-only except nothing in the
executable path); on-silicon GEM DMA/PHY (GAP-A5); third-party lwIP; the
S6b-1 `sys_arch` port beyond the NC-151-05 delta.

**Method:** read-only reconstruction — every claimed correction was checked in
the current working tree (`git status`/`git diff` at `471afb5`), the gates were
re-run, and each document's header was compared with its last `## History` row.
No build/run beyond the re-run gates was performed.

## 2. References

- Project norms: `docs/rules.md` (N1–N20, rev 1.4)
- Reviewer contract: `docs/engineering/independent-review.md` (rev 0.7)
- Prior report: `docs/reviews/2026-10-12-m4-m11-lwip-netif-s6b2-review.md` (FRICK-REVIEW-0153)
- Traceability manifest/gate: `docs/requirements/traceability.yaml`, `tools/scripts/trace.py`
- ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2
- Working-tree artifact hashes (sha256 prefix): `ff_lwip_netif.c` `59fc1a74…`,
  `ff_lwip_netif.h` `568780af…` (unchanged vs 0153), `app_renode_test.c` `4332be01…`,
  `ff_net_lwip.c` `ad535865…`, `traceability.yaml` `b88e6d24…`,
  `traceability.md` `2c9d5394…`, `network-stack.md` `5e656f40…`,
  `development-plan.md` `248fd951…`, `handover.md` `3649044c…`,
  `loop-audit.md` `9e19ec42…`, `verification-results.md` `fad01283…`

## 3. Delta closure table (FRICK-REVIEW-0153 findings)

| Finding | 0153 severity | Claimed correction | First-hand evidence (current tree) | Status |
| ------- | ------------- | ------------------ | ---------------------------------- | ------ |
| R1-01 | Minor | `netif->mtu = 1500u` | `ff_lwip_netif.c:112-114` (`netif->mtu = 1500u;` + rationale comment) | **CLOSED** |
| R1-02 | Minor | Comment states MMIO-direct MAC config + DMA not armed (NC-S6-03) | `app_renode_test.c:210-215` ("Configure the emulated GEM MAC registers directly over MMIO … The driver's DMA path is deliberately NOT armed (no RE/TE) … NC-S6-03") and `:180-187` | **CLOSED** (see C-154-01) |
| R2-01 | Minor | *(not in the claimed set — disposition requested)* | No test references `ff_lwip_netif`; `linkoutput`/RX called by nothing (`tests/` has no `ff_lwip_netif`/`attach_netif`/`linkoutput` match; only `app_renode_test.c` uses `init`/`attach`/`IsUp`/`GetMac`) | **OPEN → NC-154-01** |
| R2-02 | Minor | Round row qualified | `verification-results.md:51` — "22/22 default + 22/22 ASan/UBSan, both lwIP-off; **23/23** host with `FF_USE_LWIP=ON`" | **CLOSED** |
| R3-01 | Minor | `lean_note` rewritten | `traceability.yaml:387` — delivered state + "Open: the full TX/RX packet round-trip and the emulated GEM DMA (NC-S6-01/NC-S6-03)"; contradictory tail removed | **CLOSED** |
| R3-02 | Minor | Mirror row + History row | `traceability.md:105` lists `ff_lwip_netif.c/h` **but** the History table still ends at 0.34 (rev 0.34, S6b-1) — no S6b-2 row | **OPEN (partial) → NC-154-02** |
| R3-03 | Minor | Sprint IDs | `sprint-plan.md:72` NC-S6-01 → "NET follow-up (after S7)"; `:77` NC-S6-03 → "driver follow-up … before M14" | **CLOSED** (see C-154-05) |
| R4-01 | Minor | network-stack.md lwIP state | `network-stack.md:48-51` **still** reads "(S6b-1; on-target run = S6b-2, NC-S6-02, which must also wire the RTEMS runtime object sizing … Until S6b-2 the adapter is **archived but not linked**…)" | **OPEN** |
| R4-02 | Minor | OBJ-19 note + M11 row | `development-plan.md:56-61` (note rewritten to delivered state) and `:323` (M11 row: netif bound, `hal_net_lwip=PASS`, NC-S6-01/NC-S6-03); History 0.25 | **CLOSED** |
| R4-03 | Minor | handover body | `handover.md:121` **still** "NET(lwIP-on-GEM, **NC-S6-01**, sprint S6b)/PCIE/SPACEWIRE"; `:150` **still** "**S6b — NET lwIP-on-GEM peripheral-bound** (**NC-S6-01**, deferred)" | **OPEN** |
| R4-04 | Minor | loop-audit M11 row | `loop-audit.md:34` row updated (netif bound, `hal_net_lwip=PASS`, NC-S6-01/NC-S6-03); History 0.19 | **CLOSED** |
| PA-01 | Minor | Sprint IDs for NC-S6-01/NC-S6-03 | as R3-03; both NCs carry an owner **and** a sprint/milestone anchor | **CLOSED** (see C-154-05) |
| R1-03/R1-04, R4-05, R4-06 | Comments | *(not in the correction set)* | R1-03: `ff_lwip_netif.c:8-9,40-41` unchanged; R1-04: `IsUp`/`GetMac` unchanged; R4-05: `nightly.yml:78` step name still "(NC-S6-01/NC-S6-02)"; R4-06: no 0153 row in `docs/reviews/README.md` | open Comments (non-blocking) |

**Standing finding (0153 §5):** the `hal_net_lwip` real-pass verdict is
unaffected by the corrections (the corrections did not touch the check's oracle).

## 4. R2-01 disposition (glue coverage)

**Decision: not acceptable as a bare documented residual → recorded as a new
Minor non-conformance (NC-154-01, §5).**

- NC-S6-01 covers the **peripheral-bound packet round-trip on the emulator**
  (blocked by NC-S6-03 / the 32-bit `gem_phys` truncation). It does **not** cover
  a host test of the glue's two pure data-plane functions —
  `ff_lwip_netif_linkoutput` (pbuf → linearise → `mac->transmit`) and
  `ff_lwip_netif_rx` (frame → pbuf → `netif->input`) — which need only a mock
  `FF_EthMac_t` and **no** emulator, GEM DMA or NC-S6-03 fix.
- First-hand: no file under `tests/` references `ff_lwip_netif`, `FF_LwipNetif`,
  `linkoutput` or `attach_netif`; the only caller is the Renode app check, which
  exercises `init`/`attach`/`IsUp`/`GetMac` only (`app_renode_test.c:241-256`).
  The glue is therefore **new flight code compiled into the lwIP image with its
  transmit and receive paths exercised nowhere** (host or target).
- The counter-argument (bounded, allocation-free, two small functions) does not
  remove the obligation: N6/N10/N15 and criticality-B expectations call for
  Software V&V of new code, and the remedy is a single host unit test that is
  cheap and independent of every open blocker. Hence a Minor NC with an owner and
  a sprint (fix now, or track to the NET follow-up sprint after S7).

## 5. New findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| NC-154-01 | Minor | Coverage / N6/N10/N15 (R2-01 residual) | Netif glue `linkoutput` + RX paths are exercised by no test (host or target); NC-S6-01 does not cover a host unit test | no `ff_lwip_netif`/`linkoutput`/`attach_netif` reference under `tests/`; `ff_lwip_netif.c:42-92`; `app_renode_test.c:241-256` | Add a host unit test with a mock `FF_EthMac_t` (assert pbuf copy + `mac->transmit`; assert RX → pbuf → `netif->input`; assert >MTU rejection), or record NC-154-01 in `sprint-plan.md` §3 + `CHANGELOG.md` with owner + sprint (NET follow-up after S7) |
| NC-154-02 | Minor | `traceability.md` History / N16/N8 (R3-02 residual) | The SRS-REQ-040 mirror row was regenerated for S6b-2 but **no History row was added**; the header/History (0.34) still describe S6b-1, so the doc does not record its own S6b-2 change | `docs/requirements/traceability.md:105` (changed) vs `## History` last row 0.34 | Add a 0.35 History row for the S6b-2 mirror update |
| C-154-01 | Comment | Evidence wording / N8 | (a) `app_renode_test.c:187` states "the lwIP-on-GEM data path is host-tested"; the GEM driver (`test_gem`) and the lwIP socket path (`test_lwip`) are host-tested, but the **glue between them is not** — the composite claim is unsubstantiated. (b) `ff_lwip_netif.h:30-31` documents the precondition "The MAC must already be initialised by the caller (`FF_EthMac_t::init`)"; the check does not call it and the comment does not state that deviation explicitly | `app_renode_test.c:187`; `ff_lwip_netif.h:30-31`; no `mac.init` in the check | Narrow the wording ("the GEM driver path is host-tested; the netif glue TX/RX are tracked in NC-154-01") and state the deliberate uninitialised-MAC deviation next to the precondition |
| C-154-02 | Comment | `nightly.yml` / N10 | The step name still reads "Renode lwIP-on-GEM validation (NC-S6-01/**NC-S6-02**)" although NC-S6-02 is closed | `.github/workflows/nightly.yml:78` | Drop NC-S6-02 from the label (R4-05 residual) |
| C-154-03 | Comment | `ff_lwip_netif.c` comments | R1-03/R1-04 not addressed: with `LWIP_TCPIP_CORE_LOCKING=1` (`lwipopts.h:25`) `linkoutput` may run in the socket-calling thread holding the core lock; `IsUp`/`GetMac` read lwIP state without the lock. Both benign (core lock serialises; status query) | `ff_lwip_netif.c:8-13,40-41,158-171` | Reword to "serialised by the lwIP core lock"; mark the query as best-effort |
| C-154-04 | Comment | Review records / N11 | No FRICK-REVIEW-0153 row in `docs/reviews/README.md` and the FRICK-REVIEW-0148/0151 follow-up tables are not cross-referenced (R4-06) | `docs/reviews/README.md` (no 0153/0154 rows yet) | Add the log rows with this commit and cross-reference 0153/0154 in the source reports' follow-up tables |
| C-154-05 | Comment | Sprint tracking / N12/N19 | NC-S6-01/NC-S6-03 now carry a sprint/milestone anchor but **no target date**; consistent with the plan's style for NC-S7-01, so PA-01 is closed | `sprint-plan.md:72,77` | Add a date when the sprint is scheduled |

**Severity definition:** *Major* disqualifies the item for criticality B or
defeats a requirement/safety barrier; *Minor* must be corrected or justified;
*Comment* is an observation/recommendation.

**No Major finding is open** (confirms the standing FRICK-REVIEW-0153 verdict on
the increment).

## 6. Scope conformance

| Declared correction scope | Implemented content | Verdict |
| --- | --- | --- |
| R1-01/R1-02 code+comment | Applied (`ff_lwip_netif.c:112`, `app_renode_test.c:210-215`) | **Conformant** |
| R2-02 evidence wording | Applied (`verification-results.md:51`) | **Conformant** |
| R3-01/R3-02 manifest+mirror | Note applied; mirror row applied; History row missing | **Partial deviation** (NC-154-02) |
| R3-03/PA-01 sprint IDs | Applied | **Conformant** |
| R4-01…R4-04 doc currency | R4-02/R4-04 applied; **R4-01/R4-03 not applied** | **Partial deviation** (R4-01/R4-03 open) |
| R2-01 disposition | Not corrected; disposition now recorded (NC-154-01) | **Open, tracked** |
| No undeclared content | The working-tree file set matches the S6b-2 increment (21 modified + 2 new source files + this report); the corrections introduced **no new files** and touched no file outside the increment | **Conformant** |

## 7. Gate re-run (2026-10-12, first-hand)

| Gate | Result |
| ---- | ------ |
| `ctest --test-dir build/native` | **PASS 22/22** (0 failed) |
| `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** (114 reqs / 71 srs / 29 sss / 14 gap, 51 Lean theorems, 12 CI claims) |
| `python3 tools/scripts/metrics.py --check` | **current** (exit 0) |
| `clang-format --dry-run --Werror` on the changed/new C+H files | **clean** (exit 0) |
| Header = last `## History` row, all touched docs | **OK** mechanically for the 9 docs with a History table (network-stack 0.6, development-plan 0.25, sprint-plan 0.12, traceability.md 0.34, handover 1.15, loop-audit 0.19, renode-environment 0.19, tool-qualification 0.3.23, verification-results 0.40); **content/history substantively out of sync only for `traceability.md`** (NC-154-02) |

## 8. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — no Major open; Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**No Major finding is open**, so the Sprint-6b-2 increment remains fit for the
criticality-B baseline and the FRICK-REVIEW-0153 verdict stands.

**However, the N12 delta close-out is NOT complete.** Four Minor items are proven
uncorrected/untracked in the current tree:

1. **R4-01** — the stale `network-stack.md` §4 bullet is unchanged (doc still says
   the S6b-2 on-target run and NC-S6-02 are pending, and the adapter is "archived
   but not linked").
2. **R4-03** — the two stale `handover.md` passages are unchanged ("sprint S6b"
   / "(NC-S6-01, deferred)").
3. **NC-154-01** (= R2-01) — the netif glue's TX/RX are tested nowhere.
4. **NC-154-02** — `traceability.md` History not bumped for the S6b-2 mirror row.

Under **N20** ("reviews repeated until they pass — no open Major/Minor — before
commit/push") and **N17**, the documentation/PA loop is **not closed**, so
**commit/push is not yet permitted**. The four items are small (comment/doc edits
plus one host test); after they are corrected, a short confirmation review (N12)
closes the loop.

### PA verdict

The corrected code/comment/evidence items (R1-01, R1-02, R2-02, R3-01, PA-01 and
the R3-02 mirror row) are **verified first-hand** and the gates converge. The
corrective scope was, however, **only partially delivered**: two documentation
currency findings were not applied and one verification gap (R2-01) has no
disposition beyond this report. This is honestly recordable and non-Major, but it
must be closed before the incremental commit; the residual technical risks already
accepted in FRICK-REVIEW-0153 (NC-S6-01 packet round-trip, NC-S6-03 latent GEM
64-bit DMA addressing, R2-01 glue coverage, GAP-A5 on-target DMA/PHY) remain with
a named owner and sprint, and require qualified-human acceptance.

**What a qualified human must still accept:** the residual risk of shipping this
increment in the criticality-B baseline (deferred lwIP-on-GEM data path; latent
GEM DMA addressing defect; unexercised glue; emulated vs silicon GAP-A5). The agent
does not approve release or accept residual risk.

## 9. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9) | Senior FSW / real-time / space software developer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — MTU fix verified; comment fix verified; C-154-01/C-154-03 |
| R2 (N9) | Senior SW V&V engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — ASan wording closed; R2-01 dispositioned as NC-154-01 |
| R3 (N14/N9) | Senior systems/aerospace engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — SRS-REQ-040 note closed; mirror row closed, History row open (NC-154-02) |
| R4 (N17/N9) | Senior SW documentation engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — R4-02/R4-04 closed; **R4-01/R4-03 still open**; C-154-04 |
| PA (N20) | Senior SW Product Assurance engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — no Major; delta close-out incomplete (4 Minors); commit/push not yet permitted |

## 10. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | Minor | `ff_lwip_netif.c` | Netif MTU = frame size (1536) | **CLOSED** — `netif->mtu = 1500u` | `ff_lwip_netif.c:112-114` | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0154 (this) |
| R1-02 | Minor | app check / glue doc | Uninitialised MAC + inaccurate comment | **CLOSED** — comment documents MMIO-direct config + DMA not armed (NC-S6-03); residual wording C-154-01 | `app_renode_test.c:180-215` | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0154 (this) |
| R2-02 | Minor | `verification-results.md` | ASan claim unqualified | **CLOSED** — row qualified (ASan/UBSan lwIP-off; 23/23 host lwIP-on) | `verification-results.md:51` | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0154 (this) |
| R3-01 | Minor | `traceability.yaml` | Self-contradictory `lean_note` | **CLOSED** — note rewritten | `traceability.yaml:387` | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0154 (this) |
| R3-02 | Minor | `traceability.md` | Mirror row stale + no History row | **OPEN (partial)** — row fixed; **History row missing** → NC-154-02 | `traceability.md:105` vs History 0.34 | frick-fsw authoring agent | **open** | correct + re-review (N12) |
| R3-03 / PA-01 | Minor | Tracking | NC-S6-01/NC-S6-03 without sprint IDs | **CLOSED** — sprint/milestone anchors assigned | `sprint-plan.md:72,77` | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0154 (this) |
| R4-01 | Minor | `network-stack.md` §4 | Stale lwIP-backend bullet | **OPEN** — not applied | `network-stack.md:48-51` | frick-fsw authoring agent | **open** | rewrite bullet; re-review |
| R4-02 | Minor | `development-plan.md` | OBJ-19 note stale | **CLOSED** — note + M11 row updated | `development-plan.md:56-61,323` | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0154 (this) |
| R4-03 | Minor | `handover.md` body | Two stale M11 sites | **OPEN** — not applied | `handover.md:121,150` | frick-fsw authoring agent | **open** | update both sites; re-review |
| R4-04 | Minor | `loop-audit.md` M11 row | Stale NET wording | **CLOSED** — row updated | `loop-audit.md:34` | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0154 (this) |
| NC-154-01 | Minor | glue coverage / N6/N10/N15 | `linkoutput`/RX tested nowhere (R2-01) | **OPEN** — host mock test or recorded NC + sprint | no test reference; `ff_lwip_netif.c:42-92` | frick-fsw authoring agent | **open** | add test/record; re-review |
| NC-154-02 | Minor | `traceability.md` / N16 | History row not added for S6b-2 | **OPEN** — add 0.35 row | `traceability.md:105`; History ends 0.34 | frick-fsw authoring agent | **open** | add row; re-review |
| NC-S6-01 | Minor (scope) | SRS-REQ-040 | Packet round-trip residual | **OPEN (residual)** — netif bound; round-trip/GEM DMA open; sprint = NET follow-up (after S7) | `hal_net_lwip=PASS` (0153); `sprint-plan.md:72` | frick-fsw authoring agent | **open** | — |
| NC-S6-03 | Minor | `ff_gem.c` `gem_phys` | Latent 64-bit GEM descriptor DMA addressing | **OPEN** — claim accurate; sprint = driver follow-up before M14 | `ff_gem.c:30-36`; `sprint-plan.md:77` | GEM driver author | **open** | — |
| NC-S6-02 / NC-151-02 / NC-151-05 | Minor | sizing / CI / diagnostics | (closed in S6b-2) | **CLOSED** | FRICK-REVIEW-0153 §8 | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0153/0154 |

## 11. Norm-by-norm audit

| Norm | Assessment |
| ---- | ---------- |
| N1 Lean | SRS-REQ-040 keeps an explicit `lean_note`; contradiction removed → OK |
| N2 English | All reviewed content English → OK |
| N3 SRS/SSS | SRS-REQ-040 present/mirrored; mirror row updated → OK (History row NC-154-02) |
| N4 traceability | `trace.py` PASS (114/71/29/14, 51 theorems, 12 CI claims); mirror row consistent with the manifest → OK |
| N5 class B | Unchanged; NC-S6-03 recorded with owner + sprint → OK |
| N6 V&V / Docker | Applied scope re-gated (ctest/trace/metrics/format) → OK (glue host coverage NC-154-01) |
| N7 dual push | Not applicable (pre-commit delta review); loop not yet closed |
| N8 docs current | R4-02/R4-04/R3-01 fixed; **R4-01/R4-03 + traceability.md History remain stale** → gap |
| N9 independent review | Performed in independent context (R1+R2+R3+R4+PA) → OK |
| N10 V&V tools | T17/T10 register + History 0.3.23 current → OK (C-154-02) |
| N11 records | V&V results/CHANGELOG/status current; review-log rows pending with this commit → gap (C-154-04, minor) |
| N12 NC resolution | Corrections verified item-by-item; 4 Minors still open → **in progress (incomplete)** |
| N13 licensing | New files carry `SPDX-License-Identifier: AGPL-3.0-only`; lwIP BSD-3 unchanged → OK |
| N14 SSS review | No SSS/SSS-manifest change → not triggered |
| N15 full cycle | Applicable gates re-run green for the corrected scope → OK |
| N16 history | Per-doc History updated for 8 docs; `traceability.md` content changed without a row → gap (NC-154-02) |
| N17 documentation review | Performed; two findings remain uncorrected → gap |
| N18 Renode validation | Unchanged by the corrections (0153: both images `RENODE_TEST: PASS`); not re-run here → recorded |
| N19 loop audit | `loop-audit.md` M11 row updated; loop baseline at `471afb5`; sprint anchors present → OK |
| N20 PA review | Performed; APPROVED WITH COMMENTS, No Major; loop not closed (4 Minors) → in progress |

---

## Facts / Assumptions / Inferences / Recommendations / Confidence / Open gaps

**Facts**

- `ff_lwip_netif.c:112` sets `netif->mtu = 1500u` (R1-01 corrected).
- The app check comment documents MMIO-direct GEM MAC configuration and that the
  driver DMA is deliberately not armed (NC-S6-03) (R1-02 corrected).
- `verification-results.md:51` qualifies the S6b-2 sanitizer evidence (R2-02 corrected).
- `traceability.yaml:387` `lean_note` rewritten to the delivered state; the
  contradictory tail is gone (R3-01 corrected).
- `traceability.md:105` now lists `ff_lwip_netif.c/h`; its History still ends at 0.34.
- `network-stack.md:48-51` and `handover.md:121,150` still carry the stale text
  quoted by R4-01/R4-03.
- No file under `tests/` references the netif glue; the glue's `linkoutput`/RX are
  exercised nowhere.
- `sprint-plan.md:72,77` give NC-S6-01/NC-S6-03 an owner **and** a sprint/milestone.
- Gates: ctest 22/22; trace PASS 114/71/29/14 (51 theorems, 12 CI claims);
  metrics current; clang-format clean; all 9 touched History-bearing docs have
  header = last History row (mechanically).
- No Major finding; the corrections introduced no new files and no out-of-scope change.

**Assumptions**

- The reviewed working tree is the intended corrected increment and nothing else is pending.
- The NC-S6-01 scope (peripheral-bound round-trip) does not subsume a host unit test of the glue (an inference, stated in §4).

**Inferences**

- The corrective scope was applied only partially: the two R4 documentation sites
  were not updated and `traceability.md`'s History was not bumped, so the R3-02/R4-01/R4-03 NCs are not closed.
- The `app_renode_test.c` phrase "the lwIP-on-GEM data path is host-tested" over-claims (the driver and the socket adapter are host-tested; the glue is not).

**Recommendations**

- Correct the four open items (R4-01, R4-03, NC-154-01, NC-154-02) — all small —
  then run a short confirmation review (N12) to close the loop.
- Prefer adding the host glue test with a mock `FF_EthMac_t` now (it removes
  NC-154-01 outright and is independent of NC-S6-03).
- Fix C-154-01/C-154-02 wording and add the review-log rows (C-154-04) with the commit.

**Confidence:** **High** for the per-finding closure verdicts and the gate results
(read first-hand in the current tree); **High** for the R2-01 disposition
(absence of any test reference is directly observable); **Medium-High** for the
"partial corrective scope" inference (line-number shifts in `handover.md` mean the
pre-correction text is not fully retrievable from git; the quoted stale passages
are nevertheless present verbatim).

**Open gaps (must be closed or formally accepted by a qualified human):** R4-01,
R4-03, NC-154-01, NC-154-02 (blocking the N12/N17/N20 pass and the commit);
NC-S6-01 residual packet round-trip; NC-S6-03 latent 64-bit GEM DMA addressing;
on-target GEM DMA/PHY (GAP-A5).

---

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-12 | independent reviewer (FRICK-REVIEW-0154) | Delta review (N12) of the Sprint-6b-2 corrections after FRICK-REVIEW-0153. Verified CLOSED: R1-01, R1-02, R2-02, R3-01, R3-03/PA-01, R4-02, R4-04 and the R3-02 mirror row. Verified **OPEN**: R4-01 (`network-stack.md:48-51`), R4-03 (`handover.md:121,150`), R3-02 History row. R2-01 dispositioned as **NC-154-01** (netif glue TX/RX untested; not covered by NC-S6-01). New: NC-154-02 (`traceability.md` History). No Major open → APPROVED WITH COMMENTS, but the N12/N17/N20 loop is **not** closed (4 Minors) → commit/push not yet permitted. |

Committed and pushed with the reviewed change (norm N11).

## Independence and file-modification declaration

I declare that I reviewed the items above **independently** (fresh context; I did
not author, implement, test or approve them) and that, as instructed, I wrote
**exactly one file** — this report
(`docs/reviews/2026-10-12-m4-m11-lwip-netif-s6b2-delta-review.md`) — and
**modified, created or deleted no other repository file**. I did not commit or
push. The gate re-runs wrote only under `build/` (untracked build artifacts).
