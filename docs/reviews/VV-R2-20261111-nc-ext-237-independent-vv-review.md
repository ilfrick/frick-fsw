# Independent SW V&V Review Report — VV-R2-20261111

**Repository / branch / commit:** frick-fsw / main / `4ca8931` (fix(pus): NC-EXT-237 — FF_PusEmitEvent owns the Service 5 report header), plus uncommitted link-backend increment (NC-EXT-191) in the working tree.

**Reviewer profile:** Senior Aeronautics and Space SW Verification & Validation Engineer — independent N10/R2 review (fresh context; not the author of the fix or of the new test).

**Normative baseline:** docs/rules.md rev 2.8 (norms N1-N22, esp. N4, N6, N7, N10, N11, N12, N15, N18, N20, N22); ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2, ECSS-E-ST-70-41C (PUS-C); IEEE 1012-2024 V&V framework (terminology reconciled per project glossary).

**Evidence baseline reviewed:** `docs/vv/evidence/2026-11-11-nc-ext-237/` (gate-build, gate-ctest, gate-format, gate-lean, gate-trace, gate-license, mutation-check, clean-tree-verify, README); `docs/vv/tool-qualification.md` rev 1.6 (T3 row); `docs/vv/test-spec/ts-1-protocol-core.md` (N22-TS1-58), `ts-5-core-services.md` (N22-TS5-47); `docs/interfaces/ICD.md` rev 1.7 (§4 S5); `docs/status/handover.md` rev 3.68; `docs/planning/sprint-plan.md` rev 1.130 (NC-EXT-237, NC-EXT-191); `docs/requirements/traceability.yaml` (SRS-REQ-011, SRS-REQ-060/061/074).

**Review date:** 2026-11-11.

---

## 1. Review scope and verdict

Reviewed evidence for the NC-EXT-237 fix (`FF_PusEmitEvent(event_id, evs_type, data, now encodes `event id[2 BE]` + `type[1]` from its arguments and treats `data`/`len` as auxiliary data only; bounds `len > FF_PUS_MAX_TM_FRAME - 3 → FF_BAD_SIZE`, `data == NULL && len != 0 → FF_INVALID_ARGUMENT`) and its discriminating oracle `tests/unit/test_pus_event_identity.c` (5 cases, 106 checks).

**Overall V&V verdict for the NC-EXT-237 commit gate: the host/SIL evidence is sufficient and internally consistent; NO finding blocks commit/push of commit `4ca8931`.** All findings below are recorded (norm N11) and must be dispositioned before any target-level qualification claim, any NC closure, or any bundling of the uncommitted link-backend changes into this change (norms N15/N8/N12).

NC-EXT-237 itself remains OPEN pending acceptance by the project/Software PA (acceptance authority: Nicola; no acceptance date). Per sprint-plan.md:324 it is **not a blocker for commit or push**. N18 Renode/hardware evidence is explicitly deferred and is not a blocker for this host gate.

---

## 2. Findings register

### VV-R2-001 — T3 (CTest / unit tests) NOT FULLY QUALIFIED for this candidate — finding, not a blocker

**Classification:** Major (significance: invalidates qualified-evidence status of the whole T3 suite incl. the 56/56 result). **Blocks commit/push: No.** Blocks qualification claims and evidence reuse: Yes.

**Evidence / reference:** `docs/vv/tool-qualification.md` rev 1.6, T3 register row (lines 256-257); `docs/vv/evidence/2026-11-11-nc-ext-237/README.md` open items; sprint-plan.md NC-EXT-237 row.

`tests/unit/test_pus_event_identity.c` (sha256 `828b6a22…`, 106 checks) and `tests/unit/test_evs_hs_health.c` (sha256 `5310cc3c…`, 122 checks), together with `tests/unit/test_sched_events.c`, have **no N10 review at their current manifest hashes**. Per norm N10 (rules.md:138-156) every V&V test script is a verification tool requiring source review + inspection + configuration management at criticality B, and **any change to a recorded V&V script invalidates its qualification until a re-qualification review is performed**. The prior T3 qualification (FRICK-REVIEW-0443) covered only `test_pus.c` @ `6f68de14…` and `test_link_uart.c`; the candidate `test_pus.c` is `4ca0e2d0…` and the reviewed revision is not recoverable, so the reviewed-to-candidate delta is unverifiable (tool-qualification.md rev 1.3). Execution evidence (`gate-ctest.log` 56/56) proves execution, not qualification (tool-qualification.md:256).

**Recommendation:** Obtain an independent N10 review of `test_pus_event_identity.c` (and the other T3 scripts at their current hashes) and re-issue the T3 row before citing this test suite as qualified V&V evidence. Do not withdraw the host gate on this finding alone (per tasking: finding, not blocker).

---

### VV-R2-002 — NC-EXT-237 open pending Software PA acceptance; no NC closure claimed

**Classification:** Minor (status/administrative). **Blocks commit/push: No.**

**Evidence / reference:** sprint-plan.md:324 (NC-EXT-237 row — "OPEN pending owner acceptance … NOT a blocker for commit or push"); `docs/status/handover.md` rev 3.68; `docs/vv/evidence/2026-11-11-nc-ext-237/README.md` §Open items.

The V&V evidence is delivered, but NC-EXT-237 acceptance/closure rests with the project/Software PA. V&V does not close an NC or accept residual risk (see persona doctrine §4, §15). Norm N12/N20 closure loops (review → correction → re-review → APPROVED/APPROVED WITH COMMENTS with all NCs dispositioned) remain with the PA owner.

---

### VV-R2-003 — CTest 56/56 is host/SIL evidence only; target/RTEMS evidence needed for target claims

**Classification:** Minor (coverage limitation, tracked). **Blocks commit/push: No** for the host gate; blocks target deployment / target-level qualification claim: Yes.

**Evidence / reference:** `gate-ctest.log` (test #24 `pus_event_identity` PASS on `build2/native`); norm N18 (rules.md:315-331 — Renode gate deferred; hardware NCs not blocking); traceability.yaml lean_notes for SRS-REQ-060/061 (on-target link-level = GAP-A5).

`test_pus_event_identity.c` runs on the host simulator (native build) through the production `FF_PusBackendOpen` backend and an injected capture sink. It does **not** exercise RTEMS tasking, target memory/partition behaviour, target timer/monotonic clock, target endianness, interrupts, DMA, or multicore interference. Target-specific behaviour (WCET, caches, MMU/MPU, reset/boot, errata) requires target or justified analysis evidence (persona doctrine §3.8). N18 Renode functional validation is deferred per the tasking and must not block this host gate, but it remains open before any target/RTEMS claim.

---

### VV-R2-004 — Uncommitted link-backend changes are outside the recorded NC-EXT-237 gate evidence

**Classification:** Minor (configuration / evidence integrity). **Blocks commit/push of commit `4ca8931`: No** (separate, uncommitted changes). **Blocks bundling the link-backend changes into this commit without a full cycle re-run: Yes.**

**Evidence / reference:** `git status` — uncommitted: `src/transport/link/link.h`, `link_ip.c`, `link_tsn.c`, `tests/unit/test_link_ip.c`, `tests/unit/test_link_tsn.c`, plus docs (`CHANGELOG.md`, `README.md`, `docs/vv/test-spec/ts-2-link-security.md`, `docs/vv/verification-results.md`, `docs/status/*`, `docs/requirements/traceability.yaml`). Uncommitted evidence dir `docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/` (NC-EXT-191 backend-coverage half). `clean-tree-verify.log` verified commit `4ca8931` only (detached worktree at HEAD), not the working-tree link-backend delta.

Per norm N15 (rules.md:254-270) every change re-runs the complete applicable development+V&V cycle; per norm N8 docs must be updated in the same commit; per norm N11 evidence is retained immediately and committed with the completed compliant cycle. The NC-EXT-237 gate evidence (56/56 etc.) was captured on commit `4ca8931` which does not include the link-backend delta, so it is stale for any combined change.

**Recommendation:** Keep the link-backend increment as a separate change with its own full gate cycle (build, CTest, Lean, trace, format, license, smoke, RTEMS cross build, Renode N18) and its own evidence record (`2026-11-11-nc-ext-191-link-backends/`). Do not merge it into the NC-EXT-237 commit without re-running and re-recording the full cycle for the combined tree.

---

### VV-R2-005 — Link-backend deadline tests depend on a test-only clock seam; no free-running target-clock evidence

**Classification:** Minor (documented limitation, tracked). **Blocks commit/push: No** for the host gate; blocks target timing claim: Yes.

**Evidence / reference:** `src/core/pal/pal.h:98-99` (`FF_PalMonotonicTestSet` / `FF_PalMonotonicTestUnfreeze` test seam); `tests/unit/test_link_ip.c:837-899` (`test_ip_send_until_deadline`); `tests/unit/test_link_tsn.c:525-581` (`test_tsn_send_until_deadline`); `src/transport/link/link.h:26-42` (documented entry/post-check contract); TS-2 rev 0.10 (N22-TS2-82/83); sprint-plan.md NC-EXT-191 row.

The new deadline tests freeze the monotonic clock via the PAL test seam (`pal.h:98-99`) to make budget-expiry and overrun deterministic on the host. That seam does not exist on target; deadline behaviour under a free-running RTEMS 6.2 clock (real timers, interrupt latency, scheduler jitter) is not exercised. `ip_send_until` / `tsn_send_until` are synchronous: they enforce the deadline on entry and re-check after the transfer, reporting `FF_TIMEOUT` on overrun, but they **cannot preempt a stalled driver/stack** (documented in `link.h:36-39`). The 250 ms target single-monotonic deadline bound is GAP-A5 / open (NC-EXT-191: UART shared-descriptor concurrency, target readiness semantics). This is an honest limitation, not a defect; target timing evidence (RTEMS cross-build timing, Renode, or HIL) is required before target/flight use.

---

### VV-R2-006 — `ip_check_tx()` adds a `NULL`+non-zero rejection to the plain `ip_send()` writer (behavior change beyond the deadline feature)

**Classification:** Minor (regression surface). **Blocks commit/push: No.**

**Evidence / reference:** `src/transport/link/link_ip.c:188-250` (`ip_check_tx` shared by `ip_send` and `ip_send_until`; `ip_send` previously forwarded `NULL`+non-zero to the stack per `docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/README.md`); `tests/unit/test_link_ip.c:894-895` (asserts `FF_LinkIpBackend.send(NULL, sizeof(tx)) == FF_INVALID_ARGUMENT`).

The refactor makes the plain writer reject `data == NULL && len != 0` with `FF_INVALID_ARGUMENT` where it previously reached the stack. This is a contract/behaviour change to a shared TM/TC write path (`FF_LinkSend`) beyond the SRS-REQ-074 deadline feature. The new test covers the rejection, and existing roundtrip tests (`test_ip_udp_send_recv_roundtrip`, etc.) exercise the plain path, but regression across **all** `FF_LinkSend` callers and backends should be confirmed before target use.

---

### VV-R2-007 — Evidence-record completeness (observation)

**Classification:** Minor / observation. **Blocks commit/push: No.**

**Evidence / reference:** `docs/vv/evidence/2026-11-11-nc-ext-237/gate-format.log` (records only the clang-format command and `exit=0`; no per-file detail, no tool identity/version beyond "pinned clang-format 18"); `gate-build.log` uses Unix Makefiles/`gmake` while `clean-tree-verify.log` uses Ninja — both pass; generator divergence noted, no defect.

The format gate log is minimal but sufficient as a pass record given the pinned `frick-vv:local` image (clang-format 18.1.3 per the link-backend evidence README). No defect found. Recommend retaining per-file format output and the pinned tool identity in future gate logs for audit traceability.

---

## 3. Area-by-area assessment

### 3.1 `tests/unit/test_pus_event_identity.c` — N22 compliance, discriminating power, independence

**Norm N22 (rules.md:405-439) — SATISFIED for this test.**

| N22 element | Assessment | Reference |
|---|---|---|
| Nominal operation | Cases 1-4 + exact-budget acceptance (Case 5 last block): valid ids (0x1111/0x2222/0x0001/0x0000/0xFFFF/0x4242), all severities, header-only (`len==0`), exact `FF_PUS_MAX_TM_FRAME-3` budget, auxiliary data passed through uninterpreted. | test:101-280 |
| Applicable out-of-bound operation | Case 5: `FF_PUS_MAX_TM_FRAME-2` → `FF_BAD_SIZE`; `UINT16_MAX` → `FF_BAD_SIZE`; `NULL` data + non-zero length → `FF_INVALID_ARGUMENT`; refused before framing (`g_frame_count == 0`). | test:246-281 |
| Explicit expected results | Every `FF_TEST_ASSERT` compares against a literal expected value (service/subservice/dl/octets/tolerances). No "runs without crash" oracle. | test:103-278 |
| Traceability | Header cites ICD §4, ECSS-E-ST-70-41C service 5, NC-EXT-237; recorded against N22-TS1-58 (SRS-REQ-011) in TS-1 rev 1.10; verified by `gate-trace.log` PASS (120/76/30/14/68). | test:1-27; ts-1-protocol-core.md:168; gate-trace.log |

**Discriminating power (mutation control) — CONFIRMED.**
- Pre-fix body (event_id ignored, `(void)event_id;`): `checks=106 failures=33`, `mutation_test_exit=1`. Fails the payload-length and header-octet assertions in Cases 1-4 and the exact-budget block of Case 5. (`mutation-check.log:14-47`)
- Fixed body: `checks=106 failures=0`, `restored_test_exit=0`. (`mutation-check.log:59-60`)
- The oracle therefore discriminates the defect it documents (NC-EXT-237). This is a strong, validated control.

**Independence from implementation — ADEQUATE.**
- The test drives `FF_PusEmitEvent` through the **production** backend (`FF_PusBackendOpen`) into an injected capture sink (`evt_cap_sink`), then decodes frames with the production CCSDS parser `FF_CcsdsDecodeTm` and checks octet positions against the ICD §4 layout (`event id[2 BE] | type[1] | auxiliary data`). It does **not** re-implement the header-encoding algorithm in the oracle, so the same algorithmic defect cannot be mirrored on both sides. Case 2 (`aux = {0xAB,0xCD,0x00}`) specifically confirms the header comes from the arguments, not from id-like auxiliary octets.
- Residual: the CCSDS decoder is shared production code; a decoder defect that symmetrically masks a framing defect would not be caught. Acceptable given the mutation control demonstrates sensitivity; no separate finding raised.
- Test-author independence: no N10 review exists (see VV-R2-001). The test was authored alongside the fix; independent review required before qualification claims.

### 3.2 Gate evidence — `docs/vv/evidence/2026-11-11-nc-ext-237/`

| Gate | Result | Reference | Assessment |
|---|---|---|---|
| Build (configure + build, `BUILD_DIR=build2/native`) | exit 0, 0 warnings | gate-build.log:1-344 (exit=0 line 344); clean-tree-verify.log:14-19 (Ninja, 0 warnings) | PASS. Build log also shows ASN1SCC endianness warnings from third-party `pusclib` — third-party, not project code; not a defect. |
| CTest | 56/56 PASS, exit 0; `pus_event_identity` = test #24 PASS | gate-ctest.log:1-115 (first run) and :141-258 (final re-run after doc updates) | PASS. Labels `n22` (10 tests) and `nc-ext-237` (1 test) present. |
| Lean | `lake build` "Build completed successfully", exit 0 | gate-lean.log:1-3 | PASS (norm N1). |
| Traceability | PASS (120 reqs: 76 SRS / 30 SSS / 14 gap; 68 Lean theorems), exit 0 | gate-trace.log:1-45 | PASS (norm N4). |
| Format | `clang-format --dry-run --Werror` exit 0 (pinned 18.1.3) | gate-format.log:1-2 | PASS. Minimal log (VV-R2-007). |
| License | MPL-2.0 identity + SPDX check OK, exit 0 | gate-license.log:1-2 | PASS (norm N13). |
| Clean-tree verification | Commit `4ca8931` re-verified in detached worktree, no untracked files: configure 0, build 0 warnings, ctest 56/56 | clean-tree-verify.log:1-24 | PASS — commit is self-contained. Third-party paths symlinked from pinned fetch cache (gitignored by design). |

**Mutation control:** pre-fix fails 33/106 (exit 1); fixed passes 0/106 (exit 0). (mutation-check.log)

All six host gates are green and mutually consistent (same hashes, same counts). The gate evidence supports the NC-EXT-237 fix at the host/SIL level.

### 3.3 Sufficiency of CTest 56/56 for the NC-EXT-237 fix

**Sufficient for the host/SIL commit gate; not sufficient for target-level qualification.**

- The fix is a pure host-portable encoding/bounds change in `src/pus/pus.c:356-388`. The discriminating oracle (`test_pus_event_identity.c`, test #24) directly exercises it with nominal, boundary, and out-of-bound cases, and the mutation control proves sensitivity.
- CTest 56/56 additionally regresses the whole suite (incl. `test_evs_hs_health` #22 at 122 checks, `test_tm_output_chain` #23, `test_r3_gap` #19), so no regression in the PUS/EVS/TM path is indicated.
- **Not covered:** target/RTEMS-specific behaviour, target timing, endianness on target, memory/partition, interrupts, and Renode/hardware (N18 deferred). These are tracked (VV-R2-003, VV-R2-005) and do not block the host gate.

**Additional target/RTEMS evidence:** Needed before any target deployment or target-level qualification claim; not required to block this commit (N18 deferred per tasking). Recommend RTEMS cross-build execution of the PUS event path plus Renode timing of the monotonic-clock-dependent behaviour when the toolchain image is available.

### 3.4 Tool qualification status — T3 NOT FULLY QUALIFIED

Recorded as finding **VV-R2-001** (Major significance, non-blocking per tasking). Execution is not qualification (tool-qualification.md:256). T3 remains asserted only for the exact revisions reviewed by FRICK-REVIEW-0443; the row is re-issuable only after an N10 review of each script at its manifest sha256 (tool-qualification.md rev 1.6, lines 256-257).

### 3.5 V&V concerns with uncommitted link-backend changes

Concerns recorded as **VV-R2-004, VV-R2-005, VV-R2-006**. Summary:
- The link-backend delta (`link.h`, `link_ip.c`, `link_tsn.c`, `test_link_ip.c`, `test_link_tsn.c` + docs) is uncommitted and is **not** covered by the NC-EXT-237 gate evidence (clean-tree verified `4ca8931` only).
- It has its own uncommitted evidence record (`docs/vv/evidence/2026-11-11-nc-ext-191-link-backends/`) showing 56/56, Lean, trace, format, license/GSW, smoke, RTEMS cross-build and Renode N18 all exit 0 — but T3 is not qualified for these test scripts at the new hashes (no N10 review).
- The deadline tests use a test-only PAL clock seam (`pal.h:98-99`); no free-running target-clock evidence (VV-R2-005).
- `ip_check_tx` changes the plain `ip_send` NULL+non-zero behaviour (VV-R2-006).
- NC-EXT-191 remains open (UART shared-descriptor concurrency, target readiness semantics, 250 ms bound = GAP-A5).

---

## 4. Positive evidence (verified, no finding)

- `src/pus/pus.c:356-388` implements the contract exactly: BE `event_id[2]`, `evs_type[1]`, `data`/`len` as auxiliary data only; `FF_BAD_SIZE` when `len > FF_PUS_MAX_TM_FRAME-3`; `FF_INVALID_ARGUMENT` when `data == NULL && len != 0`; subservice mapping INFO=1 / WARNING=2 / else=3; delegates to `FF_PusSendTm`.
- `src/pus/pus.h:87` declaration unchanged; all callers (`fsw_main.c`, `fsw_main_reqtest.c`, `fsw_main_xw2.c`, `test_r3_gap.c`, `test_tm_output_chain.c`, `test_evs_hs_health.c`) updated to pass text only (per handover 3.68 / sprint-plan).
- ICD §4 (ICD.md rev 1.7) and `service-development.md` §4 (rev 0.5) document the emitter-owned header contract; wire format unchanged (ICD rev 1.7).
- Traceability gate PASS confirms N22-TS1-58 (SRS-REQ-011) is linked to the new test (TS-1 rev 1.10).
- Clean-tree verification confirms commit `4ca8931` is self-contained and reproducible from a detached worktree with no untracked files.

---

## 5. Conclusion and required actions before commit/push / qualification

**For commit `4ca8931` (NC-EXT-237 fix) alone:** no V&V finding blocks commit/push. The host gates are green, the oracle is discriminating (mutation control 33/106 → 0/106), N22 is satisfied, and traceability converges. The designated `aeronautics-space-fsw-engineer` agent may commit/push only after all applicable norms/gates pass (norm N7); this V&V review is recorded in the working tree (norm N11) but does not itself authorize commit, close NC-EXT-237, or qualify T3.

**Required dispositions (norms N10/N12/N20):**
1. Independent N10 review of `test_pus_event_identity.c` and the other current T3 scripts at their manifest hashes; re-issue tool-qualification.md T3 row (VV-R2-001).
2. Software PA acceptance of NC-EXT-237 before NC closure (VV-R2-002).
3. Target/RTEMS (and, when available, Renode N18) evidence for the event-identity path before any target-level qualification claim (VV-R2-003, VV-R2-005).
4. Keep the uncommitted link-backend increment separate; run and record its full gate cycle before any combined commit (VV-R2-004); confirm plain-writer regression for `ip_send` NULL+non-zero (VV-R2-006).
5. PA review (N20) and documentation review (N17/R4) loops for the combined change set before push.

**This review makes no safety/security/release approval and no certification claim.** Test success shows observed outcomes matched defined oracles under recorded conditions; it does not prove defect absence or operational fitness outside the evidence envelope (persona doctrine §1).

---
*End of report. Reviewer: Senior SW V&V Engineer (N10/R2 independent profile). Working-tree evidence retained per norm N11; no commit or push performed by this review.*