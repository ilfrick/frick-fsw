# M4/M11 Sprint-6b-2 — lwIP netif on the emulated GEM + RTEMS object sizing — combined R1+R2+R3+R4+PA review

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0153                      |
| Reviewer session                  | independent agent (norm N9), fresh context |
| Reviewer profile                  | R1 (SW dev) + R2 (SW V&V) + R3 (Sys Eng) + R4 (SW documentation) + PA (SW product assurance) |
| Reference revision                | `471afb5` + uncommitted working tree (Sprint-6b-2 increment) |
| Items under review                | `src/hal/net/lwip/ff_lwip_netif.{c,h}` (new), `src/apps/app_renode_test.c`, `config/rtems/rtems_config.c`, `CMakeLists.txt`, `src/hal/net/lwip/ff_net_lwip.c`, `src/hal/net/lwip/rtems/sys_arch.c`, `src/hal/net/lwip/rtems/arch/sys_arch.h`, `tools/scripts/ci_rtems_cross_build.sh`, `.github/workflows/nightly.yml`, `docs/requirements/traceability.yaml`, `docs/architecture/network-stack.md`, `docs/vv/{renode-environment,tool-qualification,verification-results}.md`, `docs/planning/sprint-plan.md`, `docs/development-plan.md`, `docs/status/{handover,loop-audit,status.json}`, `CHANGELOG.md`, `README.md` |
| Date                              | 2026-10-12                             |
| Scope                             | Verify the implemented content against the documented scope; falsify `hal_net_lwip`; review the netif glue / app check / RTEMS sizing / CI leg (R1/R2); SRS-REQ-040 manifest+mirror accuracy and counts (R3); documentation currency and NC recording (R4); product-assurance verdict and scope conformance (PA). ECSS-E-ST-40C Rev.1 (SW development/V&V), ECSS-Q-ST-80C Rev.2 (SW product assurance), project norms N1–N20 (`docs/rules.md`), reviewer contract `docs/engineering/independent-review.md` (rev 0.7) |
| Independence declaration          | The reviewer ran in a fresh, independent context; it did not author, design, implement, test or approve any item under review, and it did not inherit the author's conversation. It is read-only over the repository except for this one report. |
| Norms applied                     | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

Reviewed the **Sprint-6b-2 increment** in the uncommitted working tree at
`471afb5`:

- (A) RTEMS runtime object sizing (`config/rtems/rtems_config.c`):
  `CONFIGURE_MAXIMUM_MESSAGE_QUEUES 24`, `CONFIGURE_MAXIMUM_TASKS 24` (was 16),
  `CONFIGURE_MAXIMUM_SEMAPHORES 32` (was 16) — NC-S6-02.
- (B) New framework↔lwIP netif glue `src/hal/net/lwip/ff_lwip_netif.{c,h}`;
  the app check `renode_hal_net_lwip_check()` in `src/apps/app_renode_test.c`
  (guarded by `#if defined(FF_USE_LWIP)`), the `hal_net_lwip` term in the
  aggregate verdict, and the `ff_net_lwip.c` `attach_netif` hook-up (SRS-REQ-040).
- (C) `sys_thread_new` failure diagnostic (NC-151-05); the CI lwIP leg +
  `nm` link check (NC-151-02) in `tools/scripts/ci_rtems_cross_build.sh` and
  `.github/workflows/nightly.yml`.
- The associated documentation/records deltas.

**Excluded:** the widescale review of the unchanged S6b-1 RTEMS `sys_arch`
port (reviewed in FRICK-REVIEW-0152) — only the NC-151-05 delta was re-checked;
on-silicon GEM DMA/PHY behaviour (GAP-A5); the third-party lwIP sources.

**Method:** first-hand reconstruction — the lwIP-enabled Renode image was
rebuilt (`frick-rtems-toolchain:local`), copied into `build/renode/` and run;
the default (lwIP-off) image was rebuilt and run; all gates were re-run;
the code and documents were read against the claims.

## 2. References

- Project norms: `docs/rules.md` (N1–N20, rev 1.4)
- Reviewer contract: `docs/engineering/independent-review.md` (rev 0.7)
- Traceability manifest: `docs/requirements/traceability.yaml`
- ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2
- Prior reports: FRICK-REVIEW-0148 (S6a), FRICK-REVIEW-0150/0151/0152 (S6b-1)
- Reviewed artifact hashes (sha256): `ff_lwip_netif.c` `8d4018c7…`, `ff_lwip_netif.h`
  `568780af…`, `app_renode_test.c` `0fc3480a…`, `rtems_config.c` `002a8175…`,
  `ci_rtems_cross_build.sh` `47d180f0…`, `nightly.yml` `f5eee9c1…`,
  `sys_arch.c` `f26f4841…`, `CMakeLists.txt` `b31052cb…`

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R1-01 | Minor | `ff_lwip_netif.c` / SRS-REQ-040 | The netif MTU is set to the **frame** limit, not the IP MTU: `netif->mtu = (u16_t)mac->mtu` = `FF_GEM_MTU` = **1536** (a header+payload frame size). lwIP may therefore hand `linkoutput` IP packets up to 1536 B, producing ~1550 B frames that `ff_lwip_netif_linkoutput` rejects (`ERR_MEM`, len > 1536) and `gem_transmit` rejects (`FF_BAD_SIZE`) → silent TX failure / possible ARP failure at the largest sizes. Latent (no TX on target). | `ff_lwip_netif.c:112`; `ff_gem.h:27` (`FF_GEM_MTU 1536`); `ff_lwip_netif.h:27`; `ff_gem.c:149` | Set `netif->mtu` to 1500 (or `FF_LWIP_NETIF_FRAME_MAX - 14`), or define `FF_EthMac_t::mtu` as the IP payload per `hal_eth.h`. |
| R1-02 | Minor | `app_renode_test.c`, `ff_lwip_netif.h` | The check binds a netif to a MAC whose `FF_Gem_t::mac.init()` was **never called**, violating the glue's documented precondition ("The MAC must already be initialised by the caller"), while the check comment states the GEM MAC "is configured through the driver" although it configures `NCFGR`/`SA1B`/`SA1T` by direct MMIO (no `gem.mac.init`). | `ff_lwip_netif.h:31-32`; `app_renode_test.c:174-187,207-219` | Call `gem.mac.init` (blocked by NC-S6-03) or document the deliberate "uninitialised MAC + manual MMIO" deviation and correct the comment. |
| R1-03 | Comment | `ff_lwip_netif.c` | Threading-model comment imprecision: with `LWIP_TCPIP_CORE_LOCKING=1`, `linkoutput` can run in the **socket-calling** thread (holding the core lock), not only in the tcpip thread. The single static `tx_scratch` is still safe (serialised by the core lock), but the stated model is inaccurate. | `ff_lwip_netif.c:8-13,40-41,143-151`; `src/hal/net/lwip/lwipopts.h:25` | Reword to "serialised by the lwIP core lock". |
| R1-04 | Comment | `ff_lwip_netif.c` | `FF_LwipNetifIsUp`/`FF_LwipNetifGetMac` read lwIP state without the core lock. Benign for a status query, but not a lock-free guarantee. | `ff_lwip_netif.c:158-171` | Document that the query is a best-effort status (or take the lock). |
| R2-01 | Minor | V&V coverage / N10/N15 | The new glue's **`linkoutput` and RX paths are exercised by no test anywhere**: `test_lwip.c` has no netif-glue reference; the host lwIP build compiles `frick_net_lwip` but no test calls the glue; the Renode check only runs `init`/`attach`/`IsUp`/`GetMac` and never enables RE/TE, so no frame is transmitted or received. | `tests/unit/test_lwip.c` (no `ff_lwip_netif`/`attach_netif` refs); `app_renode_test.c:239-250`; host `ctest -R lwip` passes without touching the glue | Add a **host** unit test using a mock `FF_EthMac_t`: assert `linkoutput` copies the pbuf and calls `mac->transmit` (incl. >MTU rejection) and that the RX callback takes a pbuf and posts via `netif->input`. |
| R2-02 | Minor | `verification-results.md` (row 0.40) / N11 | The row claims "ASan/UBSan clean", but the ASan CI job builds with `FF_USE_LWIP=OFF`; the new `ff_lwip_netif.c` is **not** ASan/UBSan-instrumented, and no UBSan job exists. The claim is unsubstantiated for the new code. | `.github/workflows/ci.yml:52-59` (asan, no `-DFF_USE_LWIP=ON`); `docs/vv/verification-results.md:51` | Qualify the claim, or add an ASan (and UBSan) leg with `FF_USE_LWIP=ON` and record it. |
| R2-03 | Comment | `app_renode_test.c` oracle | The `hal_net_lwip` oracle is weak **by construction**: (a) `FF_LwipNetifIsUp()` is set inside `attach`, so the "up" term is near-tautological and cannot independently catch a stuck link; (b) the hwaddr assertion compares against the driver's cached `cfg.mac`, not a GEM register readback; (c) there is no negative test that `hal_net_lwip` fails when the netif is not up. | `app_renode_test.c:242-250`; `ff_lwip_netif.c:144-155,158-171`; `ff_gem.c:115-126` | Keep the check but describe its true strength; optionally add a failure-path (attach twice → `FF_ALREADY_EXISTS`) negative. |
| R2-04 | Comment | Emulator scope disclosure | The check's `NCFGR` write sets bits the emulated GEM does not implement (`FD`/`BIG`/`GBE`/`MDC`/`DBW` → "Unhandled bits"); the write reaches `mac0` but those fields are a model no-op. Unlike the TSN screener limit, this scope limit is not disclosed. | `build/renode/renode-process.log` (`mac0: Unhandled write to offset 0x4 … Unhandled bits: [1,8,10,19,21]`); `app_renode_test.c:211-219` | Add a one-line scope-limit note (comment + `renode-environment.md`), as for TSN. |
| R3-01 | Minor | `traceability.yaml` SRS-REQ-040 / N4/N8 | The `lean_note` is **self-contradictory**: it states the netif check is delivered (`hal_net_lwip`, S6b-2) and then adds "The peripheral-bound lwIP-on-GEM check (S6b-2) is deferred (NC-S6-01); the RTEMS runtime object sizing still needs `CONFIGURE_MAXIMUM_MESSAGE_QUEUES` (NC-S6-02)." NC-S6-02 is now closed and the check is delivered. | `docs/requirements/traceability.yaml` SRS-REQ-040 `lean_note` | Delete the stale tail; state the true residual (packet round-trip / GEM DMA = NC-S6-01/NC-S6-03). |
| R3-02 | Minor | `docs/requirements/traceability.md` mirror / N4/N8 | The SRS-REQ-040 mirror row was **not regenerated** for S6b-2: it still lists `hal/net/lwip/ff_net_lwip.c/h, hal/net/lwip/rtems/` and omits the new `ff_lwip_netif.{c,h}` that the manifest now carries; there is no History row after 0.34. | `docs/requirements/traceability.md:105`; manifest implementation list; History last row 0.34 | Add the new files to the mirror row and a History row. |
| R3-03 | Minor | NC tracking / N12/N19 | NC-S6-03 and the NC-S6-01 residual are recorded with an **owner but no specific sprint/milestone identifier** ("a driver sprint" / "a follow-up NET sprint"), unlike the convention used for NC-S7-01 (S7). No sprint in the plan currently schedules the GEM-DMA fix or the packet round-trip. | `docs/planning/sprint-plan.md:72,77`; `docs/development-plan.md` M11 row ("Pending: PCIE" only) | Assign sprint IDs (e.g. a new S6c / S4x) and reflect them in the M11 row. |
| R3-04 | Comment | Counts | Counts are **consistent**: manifest/README/verification-results all read 114 reqs (71 srs / 29 sss / 14 gap), 51 theorems, 12 CI claims; `trace.py` PASS. No count NC. | `trace.py` output; `README.md:61`; `docs/vv/verification-results.md:51` | None. |
| R4-01 | Minor | `docs/architecture/network-stack.md` §4 / N8 | Stale lwIP-backend bullet: still says "on-target run = S6b-2, NC-S6-02, which must also wire the RTEMS runtime object sizing … Until S6b-2 the adapter is archived but not linked" — future tense, contradicted by the new bullet in the same section and by History 0.6 (S6b-2 delivered, NC-S6-02 closed). | `docs/architecture/network-stack.md:46-52` vs `:53-58,:107` | Rewrite the bullet to the delivered state. |
| R4-02 | Minor | `docs/development-plan.md` OBJ-19 / N8/N19 | The OBJ-19 emulator-limits note still lists "NET lwIP-on-GEM (NC-S6-01 …) … exercised at **contract level**", contradicted by the M11 row in the **same file** (lwIP netif bound to the emulated GEM, `hal_net_lwip=PASS`, S6b-2). | `docs/development-plan.md:57-60` vs `:323` | Update the note (NET is now netif-bound on the GEM; only the round-trip is contract-level/host-tested). |
| R4-03 | Minor | `docs/status/handover.md` body / N8 | The handover body is stale: "Remaining M11 peripheral classes: NET(lwIP-on-GEM, NC-S6-01, sprint S6b)/PCIE/SPACEWIRE" and "NET … peripheral-bound (NC-S6-01, deferred)". Only the baseline line and History were updated. | `docs/status/handover.md:118-119,148` | Update the M11/next-step text to the S6b-2 state. |
| R4-04 | Minor | `docs/status/loop-audit.md` M11 row / N8/N19 | The M11 audit row still states "NET lwIP-on-GEM = NC-S6-01 (lwIP has no RTEMS port)" — stale: the port was delivered in S6b-1 and the netif is bound in S6b-2. | `docs/status/loop-audit.md:34` | Update the row (NET residual = packet round-trip / NC-S6-03). |
| R4-05 | Comment | `nightly.yml` / T10 / N10 | Step name "Renode lwIP-on-GEM validation (NC-S6-01/NC-S6-02)" references NC-S6-02 (now closed); the CI `nm` link check uses `grep -qw` over `nm` output (an undefined `U` symbol would also match, though a successful link precludes it). Separately, the T10 register row describes `setup_rtems.sh`, while the CI map binds T10→`ci:rtems-cross-build`; re-qualification attribution is ambiguous. | `.github/workflows/nightly.yml:78`; `docs/vv/tool-qualification.md:34,78` | Fix the step label; make the T10 script/evidence mapping unambiguous. |
| R4-06 | Comment | Reviews records / N11/N12 | The NC closures are recorded in `sprint-plan.md` §3 and this report, but the **source reports' follow-up tables** (FRICK-REVIEW-0148, FRICK-REVIEW-0151) are not amended; a review-log row for FRICK-REVIEW-0153 must be added on commit. | `docs/reviews/README.md` (no 0153 row); `docs/reviews/2026-10-12-m4-m11-tsn-s6a-review.md`, `…-lwip-rtems-s6b1-final-review.md` | Cross-reference FRICK-REVIEW-0153 in those tables; add the log row when committing (N11). |
| PA-01 | Minor | Product assurance / scope records | Same as R3-03 from the PA angle: the "open item" (`NC-S6-03`, NC-S6-01 residual) has an owner but no dated milestone, so the N19/N12 tracking is incomplete in form. The NC-S6-03 **technical claim itself is accurate and honestly recorded** (see §5). | `docs/planning/sprint-plan.md:72,77`; `CHANGELOG.md:20-27` | Assign a sprint/milestone with a target date. |

**Severity definition:** *Major* disqualifies the item for criticality B or defeats a requirement/safety barrier; *Minor* must be corrected or justified; *Comment* is an observation/recommendation.

No **Major** finding was identified.

## 4. Scope-conformance

| Declared scope (task / sprint-plan) | Implemented content | Verdict |
| --- | --- | --- |
| RTEMS object sizing wired (NC-S6-02) | `CONFIGURE_MAXIMUM_MESSAGE_QUEUES 24`, `TASKS 24`, `SEMAPHORES 32`, bounded and justified | **Conformant** (verified: the lwIP image runs `tcpip_init` on the emulated target) |
| Netif glue bound to the GEM via `FF_EthMac_t`; RX fed to `tcpip_input` | `ff_lwip_netif.{c,h}` implements `linkoutput`/`init`/RX callback; `ff_net_lwip.c::attach_netif` calls `FF_LwipNetifAttach` | **Conformant** |
| `hal_net_lwip` app check asserting netif-up + hwaddr + GEM MODID | `renode_hal_net_lwip_check()` does exactly this on the lwIP image; `hal_net_lwip_ok` is ANDed into the aggregate | **Conformant** |
| CI lwIP leg + `nm` link check (NC-151-02) | `ci_rtems_cross_build.sh` + `.github/workflows/nightly.yml` build `FF_USE_LWIP=ON` and grep `tcpip_init`/`FF_LwipNetifAttach` | **Conformant** (substance; see R4-05) |
| `sys_thread_new` failure diagnostic (NC-151-05) | `sys_arch.c` records failures via `frick_lwip_thread_failed()` + accessors; `LWIP_PLATFORM_DIAG` under `LWIP_DEBUG` | **Conformant** (see R2 note: the accessors are not surfaced/tested) |
| The **documented** S6b-2 success criterion `hal_net` **peripheral-bound round trip** | Replaced by `hal_net_lwip` (netif-up + MODID); no packet round trip; GEM DMA not armed | **Deviation — honestly recorded** (NC-S6-01 narrowed, NC-S6-03 opened, sprint-plan/verification-results updated). No new scope NC; tracking must be completed (R3-03/PA-01). |

No undeclared content was found: the working-tree file set matches the
increment description.

## 5. Falsification results

**Exercise (as required).** Rebuilt the lwIP-enabled Renode image
(`cmake -S . -B build/renode-lwip -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-mpfs.cmake -DFF_PLATFORM=rtems -DFF_ENABLE_RENODE_TEST=ON -DFF_USE_LWIP=ON && cmake --build build/renode-lwip`), copied the ELF into `build/renode/frick_fsw_renode.elf`, ran `make renode`, and inspected `build/renode/console.log` and `build/renode/renode-process.log`. Also rebuilt and ran the default (lwIP-off) image.

**Results (Facts):**

- lwIP image: `hal_net=PASS`, `hal_net_lwip=PASS`, `RENODE_TEST: PASS`.
- **No false pass.** `grep -c "non existing"` = **0** and `grep -c "0x4022"` = **0** — the S6a double-base signature is **absent**. The only mac0 accesses are `offset 0x540` (the known TSN screener model no-op) and `offset 0x4` = `NCFGR` (`value 0x220502`), which **land on the mapped emulated GEM** (the S6a bug produced `0x4022xxxx` "non existing peripheral" accesses). The `NCFGR` write proves the lwIP check's MMIO reaches the GEM at the correct address.
- The check depends on **real MMIO**: the final `FF_GEM_MODID` read (at `FF_GEM0_BASE+0xFC`) must equal the Cadence identification `0x002/0x118`; an unmapped read would return 0 and the check would return `false`.
- The check depends on **real RTEMS/lwIP init**: `FF_NetStackLwip.init()` calls `tcpip_init()`, which blocks until the tcpip thread signals completion; the marker appears only after `attach_netif`, `FF_LwipNetifIsUp` and the hwaddr/MODID assertions. Hence the RTEMS object sizing (NC-S6-02) is demonstrably effective.
- `hal_net_lwip_ok` **is** ANDed into the aggregate verdict (`app_renode_test.c:578-586`), so a failing `hal_net_lwip` would make `RENODE_TEST: FAIL`; the nightly greps `hal_net_lwip=PASS`.
- Default image unchanged: `hal_net=PASS`, **no** `hal_net_lwip` line (`grep -c` = 0), `RENODE_TEST: PASS`; same benign warnings as before.

**Answers to the falsification questions:**

- *Is `hal_net_lwip=PASS` produced by a real check?* **Yes** — real MMIO (MODID read + `NCFGR`/`SA1B`/`SA1T` writes reaching `mac0`) and a real `tcpip_init` on RTEMS.
- *Could the check pass without the netif being bound to the GEM?* Partially, and this is the residual weakness (R2-03): the netif's hardware address comes from the **driver's cached `cfg.mac`**, not a GEM register readback, and the GEM presence is asserted by a read **independent of the netif**. The check therefore proves "stack starts + netif registers/up + GEM present + driver config matches", **not** "the netif's TX/RX reach the GEM" (no frame traverses `linkoutput`/RX). This is honestly stated in the check comment and `renode-environment.md`, but the phrase "bound to the emulated GEM MAC" slightly over-claims.
- *Does the verdict actually fail if the netif is not up?* **Yes** (`FF_LwipNetifIsUp` gates `hal_net_lwip_ok`), but the "up" term is set inside `attach`, so it is near-tautological (R2-03).

**Conclusion of the exercise:** the new `hal_net_lwip` check is a **real pass**, not a false pass; it is materially stronger than the S6a `hal_tsn` check was (no unmapped access, depends on a genuine MODID read), but weaker than its wording in two documented places (hwaddr readback, link state).

## 6. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

No Major finding. The increment is functionally correct, the `hal_net_lwip`
check is a real pass, and the RTEMS object sizing is verifiably effective. The
Minor findings (documentation currency R4-01…R4-04, manifest/mirror R3-01/R3-02,
coverage R2-01/R2-02, MTU R1-01, tracking R3-03/PA-01) must be corrected and the
deltas re-reviewed (norm N12/N17/N20: review → correction → re-review, repeated
until the review passes with no open Major/Minor) **before commit/push**.

### PA verdict

The Sprint-6b-2 **implemented content matches the documented scope** with one
honestly-recorded, tracked deviation (the peripheral-bound NET round trip →
`hal_net_lwip` + NC-S6-01/NC-S6-03). The **NC-S6-03 claim is accurate**:
`gem_phys()` returns `uint32_t` (`ff_gem.c:30-36`), so it cannot express an MPFS
DDR address ≥ `0x1000000000` (the Renode image loads at `0x1000000000`); the
descriptor translation would therefore truncate the buffer addresses if RE/TE
were armed, which is consistent with (and the reason for) not enabling them.
The claim is Fact for the code and Inference for the emulator fault/loop (not
independently reproduced without modifying source); the linkage to GAP-A5 is
appropriate. The NC is honestly recorded in CHANGELOG/sprint-plan/verification-results.

**What a qualified human must still accept:** the residual risk of shipping this
increment as part of the criticality-B baseline — namely, the deferred lwIP-on-GEM
data path (NC-S6-01 residual), the latent 64-bit GEM DMA addressing defect
(NC-S6-03) until fixed, the unexercised `linkoutput`/RX glue (R2-01), and the
on-target GEM DMA/PHY GAP-A5 — together with the correction of the Minor findings.
The agent does not approve release or accept residual risk.

## 7. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9) | Senior FSW / real-time / space software developer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — glue correct/allocation-free; MTU (R1-01) + uninitialised-MAC/comment (R1-02) minors |
| R2 (N9) | Senior SW V&V engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — falsification shows a real pass; coverage (R2-01) + ASan claim (R2-02) minors |
| R3 (N14/N9) | Senior systems/aerospace engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — SRS-REQ-040 note (R3-01) + mirror (R3-02) + tracking (R3-03) |
| R4 (N17/N9) | Senior SW documentation engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — four stale doc sites (R4-01…R4-04) + records (R4-05/R4-06) |
| PA (N20) | Senior SW Product Assurance engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — scope conformant; NC-S6-03 claim accurate; minors tracked; residual risk to the human authority |

## 8. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-S6-02 | Minor | `config/rtems/rtems_config.c` | No `CONFIGURE_MAXIMUM_MESSAGE_QUEUES` → runtime `tcpip_init` fails | **CLOSED** — sizing wired (`MQUEUES 24`/`TASKS 24`/`SEMAPHORES 32`, bounded) | lwIP Renode image runs `tcpip_init` → `hal_net_lwip=PASS`/`RENODE_TEST: PASS` (reproduced) | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0153 (this) |
| NC-151-02 | Minor | `ci_rtems_cross_build.sh`; CI | `FF_USE_LWIP=ON` cross-build not reached by CI; no link check | **CLOSED** — nightly lwIP leg + `nm` link check; local `ci_rtems_cross_build.sh` reproduces both builds + link check | `ci_rtems_cross_build.sh` PASS (plain + lwIP + `nm`); `nightly.yml:57-84` | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0153 (this) |
| NC-151-05 | Minor | `sys_arch.c` | `sys_thread_new` fails silently | **CLOSED** — failure recorded (`frick_lwip_thread_failed`, accessors) + `LWIP_PLATFORM_DIAG` under `LWIP_DEBUG` (accessors not yet surfaced/tested — comment) | `sys_arch.c:71-101,500-525` | frick-fsw authoring agent | **closed** | FRICK-REVIEW-0153 (this) |
| NC-S6-01 | Minor (scope) | SRS-REQ-040 | NET lwIP-on-GEM peripheral-bound round trip not delivered | **Narrowed, OPEN (residual)** — netif now bound/up + GEM identification; packet round trip + GEM DMA remain | `hal_net_lwip=PASS`; `sprint-plan.md:72` | frick-fsw authoring agent | **open** (residual) | add sprint ID (R3-03) |
| NC-S6-03 | Minor | `ff_gem.c` `gem_phys` | Latent 64-bit GEM descriptor DMA addressing | **OPEN (new)** — claim verified accurate; needs a sprint/milestone | `ff_gem.c:30-36`; `sprint-plan.md:77`; `CHANGELOG.md:20-27` | GEM driver author | **open** | assign sprint (R3-03/PA-01) |
| R1-01..R1-02 | Minor | glue/app | MTU; uninitialised MAC + comment | open (to correct) | this report | frick-fsw authoring agent | **open** | FRICK-REVIEW-0153 delta |
| R2-01..R2-02 | Minor | V&V evidence | unexercised glue; ASan claim | open (to correct) | this report | frick-fsw authoring agent | **open** | FRICK-REVIEW-0153 delta |
| R3-01..R3-03 | Minor | manifest/mirror/tracking | note contradiction; mirror stale; sprint IDs | open (to correct) | this report | frick-fsw authoring agent | **open** | FRICK-REVIEW-0153 delta |
| R4-01..R4-04 | Minor | docs | stale network-stack/development-plan/handover/loop-audit | open (to correct) | this report | frick-fsw authoring agent | **open** | FRICK-REVIEW-0153 delta |

## 9. Norm-by-norm audit

| Norm | Assessment |
| ---- | ---------- |
| N1 Lean | SRS-REQ-040 keeps an explicit `lean_note` (not Lean-checkable); note content is stale/contradictory (R3-01) → **gap** |
| N2 English | All reviewed content English → OK |
| N3 SRS/SSS | SRS-REQ-040 present and mirrored; mirror not regenerated (R3-02) → gap |
| N4 traceability | `trace.py` PASS (114/71/29/14, 51 theorems, 12 CI claims); manifest SRS-REQ-040 note + mirror inconsistencies (R3-01/R3-02) → gap |
| N5 class B | Unchanged; NC-S6-03 (latent GEM defect) correctly recorded → OK |
| N6 V&V / Docker | CI legs + Docker toolchain reproduced the builds and the Renode run → OK |
| N7 dual push | Not applicable to this pre-commit review; loop not yet closed (Minors open) |
| N8 docs current | Four stale doc sites (R4-01…R4-04) + manifest/mirror (R3-01/R3-02) → gap |
| N9 independent review | Performed in independent context by the R1+R2+R3+R4+PA profiles → OK |
| N10 V&V tools | T17/T10 registers touched and History bumped; T10 attribution ambiguous (R4-05) → gap (minor) |
| N11 records | V&V results/CHANGELOG/status updated; review-log row pending (R4-06) → gap (minor) |
| N12 NC resolution | NC-S6-02/NC-151-02/NC-151-05 closure evidence confirmed; NC-S6-01 narrowed; NC-S6-03 opened; Minors to close → in progress |
| N13 licensing | New file carries `SPDX-License-Identifier: AGPL-3.0-only`; lwIP BSD-3 unchanged → OK |
| N14 SSS review | No SSS/SSS-manifest change → not triggered |
| N15 full cycle | build/test/lean/trace/static-analysis/renode re-run here for the applicable scope → OK (ASan claim R2-02) |
| N16 history | All touched doc headers equal their last History row (verified for the 8 changed docs) → OK |
| N17 documentation review | Performed; stale docs found (R4) → gap |
| N18 Renode validation | Default image `RENODE_TEST: PASS`; lwIP image `RENODE_TEST: PASS` incl. `hal_net_lwip=PASS` → OK |
| N19 loop audit | Objective/rule audit present; M11 row stale and NC milestones unnamed (R3-03/R4-04) → gap |
| N20 PA review | Performed; verdict APPROVED WITH COMMENTS with Minor findings to close before commit → in progress |

---

## Facts / Assumptions / Inferences / Recommendations / Confidence / Open gaps

**Facts**

- Rebuilt and ran the lwIP-enabled Renode image: `hal_net=PASS`, `hal_net_lwip=PASS`, `RENODE_TEST: PASS`; `grep -c "non existing"` = 0, `grep -c "0x4022"` = 0; `mac0` `NCFGR` (offset 0x4) write reached the mapped GEM.
- Default image: `hal_net=PASS`, no `hal_net_lwip` line, `RENODE_TEST: PASS` (unchanged).
- Gates: default `ctest` 22/22; host `FF_USE_LWIP=ON` `ctest` 23/23; `trace.py` PASS (114/71/29/14, 51 theorems, 12 CI claims); `metrics.py --check` current; `clang-format --dry-run --Werror` clean (incl. the new files); RTEMS cross-builds with and without lwIP PASS (+ `nm` link check).
- `ff_lwip_netif.c` is bounded/static, allocation-free (lwIP pbuf pool only), takes `LOCK_TCPIP_CORE` around netif registration, and ANDs into the verdict.
- NC-S6-02/NC-151-02/NC-151-05 closure evidence present; NC-S6-01 narrowed; NC-S6-03 opened (claim accurate).
- All eight changed document headers equal their last `## History` row.

**Assumptions**

- The reviewed working tree is the intended increment and no other change is pending.
- The pinned lwIP (`STABLE-2_2_1_RELEASE`) and the Renode image are the intended V&V inputs (unchanged by this increment).
- "MPFS DDR ≥ 0x1000000000" is the intended on-target buffer region (the Renode image loads at that address).

**Inferences**

- The `hal_net_lwip` check is a **real pass** (not the S6a false-pass pattern): its MMIO lands on the mapped GEM and it depends on a genuine MODID read.
- The netif hwaddr assertion does not prove the GEM station-address registers were programmed (it reads the driver's cached config); the EMU link state is synthetic.
- The GEM DMA fault/loop under RE/TE (NC-S6-03) was not independently reproduced (would require arming the DMA); the 32-bit truncation is the sound root cause.

**Recommendations**

- Correct R1-01 (IP MTU), R1-02 (comment/precondition), R2-01 (host glue test), R2-02 (ASan claim), R3-01/R3-02 (manifest note + mirror), R3-03/PA-01 (sprint IDs), R4-01…R4-04 (stale docs); then delta re-review (N12/N17/N20) before commit/push.
- Disclose the NCFGR model no-op (R2-04) consistent with the TSN scope-limit disclosure.

**Confidence:** **High** for the falsification result and the functional gates (first-hand reproduction); **Medium-High** for the documentation-currency findings (read directly; some stem from the increment's partial doc updates); **Medium** for the NC-S6-03 emulator-fault inference (root cause Fact, fault not reproduced).

**Open gaps (must be closed or formally accepted by a qualified human):** deferred lwIP-on-GEM packet round trip and GEM DMA (NC-S6-01 residual / NC-S6-03); unexercised `linkoutput`/RX glue (R2-01); stale docs/manifest (R3/R4); open Minors block the N20/N17 pass until corrected and re-reviewed; on-target GEM DMA/PHY (GAP-A5).

---

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-12 | independent reviewer (FRICK-REVIEW-0153) | Combined R1+R2+R3+R4+PA review of the Sprint-6b-2 increment at `471afb5` + uncommitted working tree: **APPROVED WITH COMMENTS**. Falsification: `hal_net_lwip=PASS` is a **real pass** (no unmapped/double-base access; depends on a genuine GEM MODID read + `tcpip_init`), not the S6a false pass. NC-S6-02/NC-151-02/NC-151-05 **closed** (evidence reproduced); NC-S6-01 narrowed (open residual); NC-S6-03 **opened** (claim accurate, needs a sprint ID). 12 Minor findings + Comments (R1-01/02, R2-01/02, R3-01/02/03, R4-01…04, PA-01) to correct and re-review before commit/push. |

Committed and pushed with the reviewed change (norm N11).

## Independence and file-modification declaration

I declare that I reviewed the items above **independently** (fresh context; I did
not author/implement/test/approve them) and that, as instructed, I wrote
**exactly one file** — this report
(`docs/reviews/2026-10-12-m4-m11-lwip-netif-s6b2-review.md`) — and **modified,
created or deleted no other repository file**. I did not commit or push. The
builds/runs were executed under `build/` and `/tmp` (untracked build artifacts)
only, as required to perform the falsification exercise.
