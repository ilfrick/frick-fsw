# Review Report

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0166                      |
| Reviewer session                  | independent agent (norm N9/N20), fresh context |
| Reviewer profile                  | **PA** — Senior SW Product Assurance engineer |
| Reference revision                | S4b working-tree increment (pre-commit, rev 0.43 per CHANGELOG 0.43/0.43), atop `471afb5` + Sprint-6b-2/NET-follow-up/S7; source files in `src/transport/link/` and tests in `tests/unit/test_link_ip.c` / `test_link_tsn.c` |
| Items under review                | `src/transport/link/link_seg.{c,h}`, `src/transport/link/link_ip.{c,h}`, `src/transport/link/link_tsn.{c,h}`, `src/transport/link/link.h`, `src/transport/link/link.c`, `src/core/pal/pal_posix.c` (mutex implementation cross-check); `tests/unit/test_link_ip.c`, `tests/unit/test_link_tsn.c`; requirements SRS-REQ-059/060/061 and their chain in `docs/requirements/{SRS.md,traceability.yaml}`; design evidence in `docs/vv/verification-results.md` (S4b cycle-log row + review evidence lines 404–406); `docs/engineering/user-integration-manual.md` (rev 0.8, §7/§8/§8.2/§10); `docs/planning/sprint-plan.md`; `docs/status/{handover.md,loop-audit.md,status.json}`; `CHANGELOG.md`; `docs/quality/{ecss-criticality-b.md,product-assurance.md,ecss-compliance-matrix.md,class-a-gaps.md}`; the prior R1+R2 review `docs/reviews/2026-10-13-s4b-r1-r2-review.md` (FRICK-REVIEW-0165); the prior R4 review `docs/reviews/2026-10-13-s4b-integration-manual-r4-review.md` (FRICK-REVIEW-0164); `docs/nc/2026-10-13-integration-manual-fix-nc.md` (NC-INT-MAN-01). |
| Date                              | 2026-10-13                             |
| Scope                             | Independent **Product-Assurance review of every phase product** (norm N20) of the S4b PUS-C link-transport increment (seg/reassembly, IP, TSN): (a) confirmation that the R1+R2 findings of FRICK-REVIEW-0165 are first-hand reproducible and remain **unresolved** (no corrective delta applied in the working tree); (b) PA-specific evidence-record conformance across the engineering data set touched by S4b — traceability, SRS, verification-results, sprint-plan, CHANGELOG, status/semaphores, review log; (c) independent re-execution of the gates; (d) scope/claim conformance vs ECSS-Q-ST-80C Class B (records, tool qualification, NC control, independence) and project norms N1–N20. Assessed against ECSS-E-ST-40C, ECSS-Q-ST-80C Rev.2, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2 and the project norms N1–N20 (`docs/rules.md` rev 1.4). |
| Independence declaration          | The reviewer had **no part** in authoring, building-for-delivery, or previously reviewing the items under review; this review ran in a fresh context and every declared claim was read first-hand from the source/test/document files or confirmed by re-running the referenced gate. The reviewer wrote exactly one file — this report — and modified no other repository file. |
| Norms applied                     | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

**In scope (verified first-hand):**

- **R1+R2 findings from FRICK-REVIEW-0165** — re-read every cited source location and test file first-hand; confirm the 2 Major (R1-01, R1-02), the 10 Minor (R1-03–R1-12), the 6 V&V-gaps (R2-01–R2-06), and the 3 Comments (R1-13, R1-C1, R2-C1) are reproducible and remain **unresolved** in the S4b working tree. No corrective delta was found applied.
- **Source evidence** — `src/transport/link/link_seg.{c,h}`, `link_ip.{c,h}`, `link_tsn.{c,h}`, `link.h`, `link.c` and the PAL mutex implementation `src/core/pal/pal_posix.c` (lines 241–264) cross-referenced against the R1+R2 findings.
- **Test evidence** — `tests/unit/test_link_ip.c` (14 test functions) and `tests/unit/test_link_tsn.c` (11 test functions); confirmation that the mocks cannot discriminate R1-01 (seg-layer desync) or R1-02 (TSN ISR mutex).
- **PA evidence-record conformance** — `docs/requirements/traceability.yaml` (SRS-REQ-059/060/061), `docs/requirements/SRS.md` (lines 353–367), `docs/vv/verification-results.md` (S4b cycle-log row line 54 + review-evidence table lines 404–406), `docs/quality/ecss-compliance-matrix.md`, `docs/quality/class-a-gaps.md` (GAP-A5), `docs/quality/ecss-criticality-b.md`, `docs/planning/sprint-plan.md`, `CHANGELOG.md`, `docs/status/status.json`, `docs/status/handover.md`, `docs/status/loop-audit.md`, `docs/engineering/user-integration-manual.md` (rev 0.8), `docs/nc/2026-10-13-integration-manual-fix-nc.md`.
- **Independent re-execution of relevant gates** — `trace.py` over `traceability.yaml`; `metrics.py --check`; review-log row presence.

**Out of scope / excluded (recorded so the scope is bounded):**

- Engineering/technical content already reviewed by R1/R2 (FRICK-REVIEW-0165) and R4 (FRICK-REVIEW-0164) — this report verifies their **closure claims** and the PA-relevant records/scope/truthfulness dimensions, it does not re-derive register maps or divider numerics.
- On-target silicon behaviour (Renode N18 SKIP — host-only code; GAP-A5).
- This report's own `docs/reviews/README.md` log row (owner: coordinator, norm N11) — to be appended by the coordinator after this report is finalised.

## 2. References

- Project norms: `docs/rules.md` (N1–N20, rev 1.4, 2026-10-11).
- Reviewer contract: `docs/engineering/independent-review.md` (R1–R4 + PA profiles, norm N9/N20).
- Report format: `docs/reviews/review-template.md`.
- Prior reviews: FRICK-REVIEW-0165 (R1+R2, `2026-10-13-s4b-r1-r2-review.md`); FRICK-REVIEW-0164 (R4, `2026-10-13-s4b-integration-manual-r4-review.md`).
- NC record: NC-INT-MAN-01 (`docs/nc/2026-10-13-integration-manual-fix-nc.md`).
- Traceability: `docs/requirements/traceability.yaml` (SRS-REQ-059/060/061, lines 552–577).
- Requirements: `docs/requirements/SRS.md` (lines 353–367).
- ECSS-E-ST-40C, ECSS-Q-ST-80C Rev.2, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.
- PAL: `src/core/pal/pal.h` / `pal_posix.c` (lines 241–264, mutex implementation).
- HAL contracts: `src/hal/hal_net.h`, `hal_eth.h`, `hal_tsn.h`.
- `docs/quality/ecss-compliance-matrix.md` (§2, PA outputs).
- `docs/quality/class-a-gaps.md` (GAP-A5 on-target).
- `docs/status/status.json` (semaphores, sprint counts).

## 3. Findings

### 3.1 Confirmed from FRICK-REVIEW-0165 (R1+R2)

All 18 actionable findings (2 Major + 10 Minor + 6 V&V-gap) and 3 Comments from FRICK-REVIEW-0165 are **confirmed first-hand** and remain **unresolved** in the S4b working tree — no corrective commit has been applied since the R1+R2 review was written on 2026-10-13.

| # | Severity | Item / Requirement | Finding (confirmed) | Evidence (file:line) | Recommendation |
|---|----------|-------------------|---------------------|---------------------|----------------|
| **R1-01** | **Major** | SRS-REQ-059 — TCP reassembly | **Phase 4 partial-read recovery discards buffered state, causing stream desync.** In `FF_LinkSegRecvFrame`, Phase 4 reads the frame payload directly into the caller's `frame` buffer (not `rx_partial`). When `got == 0u` mid-payload (line 115–119), the code sets `seg->rx_len = 0u` and returns `*len = total` with `FF_TIMEOUT`. The comment on line 116 says "keep what we have in rx_partial for next call" but the code does the **opposite** — it wipes `rx_partial` and discards the 2-byte length prefix + any partial payload. On the next call, the remaining stream bytes are read as a new length prefix, decoded as a garbage frame length, and the stream is desynchronised. | `link_seg.c:100–127` (Phase 4); comment contradicts code at `:116–117`; `g_tcp_rx` not involved | Restructure Phase 4 to buffer partial payload into `rx_partial` (copy `frame[0..total-1]` back to `rx_partial[FF_LINK_SEG_PREFIX_LEN..]` and set `rx_len = FF_LINK_SEG_PREFIX_LEN + total` before returning `FF_TIMEOUT`), so the next call resumes correctly. Add a fragmented-stream test (R2-01). |
| **R1-02** | **Major** | SRS-REQ-061 — TSN RX callback ISR safety | **A blocking mutex is taken from the MAC RX callback, which fires from ISR context on real hardware.** `tsn_eth_rx_cb` (line 56) calls `FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER)`. On the real Cadence GEM/PolarFire SoC, the MAC delivers received frames via an ISR that invokes this callback. `FF_PalMutexLock` wraps `pthread_mutex_lock` with `FF_PAL_WAIT_FOREVER` (`pal_posix.c:248`), which blocks indefinitely. Calling a blocking mutex lock from an ISR is unsafe on RTEMS POSIX (`pthread_mutex_lock` from ISR context returns an error rather than blocking). **Worse, the return value is not checked** (line 56), so on an ISR the lock silently fails and the ring-buffer fields are read/written without synchronization — a data race that can corrupt the RX ring and deliver garbage CCSDS frames to PUS-C. | `link_tsn.c:56` (callback entry, lock with no return check); `:36–64` (callback body); `pal_posix.c:248` (pthread_mutex_lock wraps the call) | Use an ISR-safe critical section (interrupt disable/restore or a dedicated ISR-safe ring buffer). At minimum, check the `FF_PalMutexLock` return value and handle failure. Document the delivery-context assumption. |
| R1-03 | Minor | SRS-REQ-061 — TSN RX ring | **TOCTOU data race on `g_rx_count`.** `g_rx_count` is read on line 41 (`g_rx_count >= FF_LINK_TSN_RX_SLOTS`) **before** the mutex is acquired on line 56, then read again on line 57 **after** the lock. The outer read is unsynchronized. | `link_tsn.c:41` (pre-lock read) vs `:56–57` (post-lock check) | Remove the pre-lock `g_rx_count` check; acquire the lock first, then test. |
| R1-04 | Minor | SRS-REQ-061 — TSN TX stack usage | **1038-byte stack allocation per `tsn_link_send` call.** `uint8_t frame[FF_LINK_TSN_HDR_LEN + FF_LINK_TSN_RX_FRAMESZ]` (14 + 1024 = 1038 bytes) on line 170. On a constrained RISC-V target with limited per-task stack, this is a risk. | `link_tsn.c:170` | Move `frame` to a mutex-protected static buffer, or document the per-call stack budget and enforce it in the RTEMS configuration. |
| R1-05 | Minor | SRS-REQ-060 — dead variables | **`g_tcp_rx` and `g_tcp_rx_len` are initialised but never read.** They are memset to zero in `ip_init_tcp` (lines 142–143); no code path uses them. `ip_recv` for TCP calls `FF_LinkSegRecvFrame` with the caller's buffer directly. | `link_ip.c:26–27` (declaration); `:142–143` (memset) | Remove the two dead variables and their initialisation. |
| R1-06 | Minor | SRS-REQ-060 — dead define | **`FF_LINK_IP_MAX_FRAME` is defined (line 21) but never used.** `ip_send` / `ip_recv` do not validate `len` against this bound; only the seg layer enforces `FF_LINK_SEG_MAX_FRAME` indirectly. | `link_ip.c:21` | Either use `FF_LINK_IP_MAX_FRAME` to validate `len` in `ip_send` / `ip_recv`, or remove the dead define. |
| R1-07 | Minor | SRS-REQ-060 — TCP init | **`ip_init_tcp` calls `stack->accept()` with no timeout.** Line 130 blocks until a peer connects; if the peer is unavailable, init hangs indefinitely. | `link_ip.c:130` | Use non-blocking `accept` with retry/polling, or document that TCP-mode init requires a pre-connected peer. |
| R1-08 | Minor | SRS-REQ-060 — init contract | **`ip_init` (vtable) silently falls back to reference loopback defaults** when `g_cfg.stack == NULL`, contradicting the header's "Must be called before `FF_LinkSelect`" contract. | `link_ip.c:167–179` | Either return `FF_NOT_READY` when config is absent, or document the fallback as an intentional "reference default" mode. |
| R1-09 | Minor | SRS-REQ-060 — backend identity | **`FF_LinkIpBackend.name` is statically `"ip_udp"`.** In TCP mode the name is misleading; the field is never updated at runtime. | `link_ip.c:240` | Make `name` reflect the runtime mode (set it in `FF_LinkIpInit`), or document that it denotes the default. |
| R1-10 | Minor | SRS-REQ-060 — error-code consistency | **Partial-write error codes differ between TCP and UDP.** TCP path (`ip_write_bytes`, line 40) returns `FF_BUFFER_FULL`; UDP path (`ip_send`, line 209) returns `FF_ERROR`. | `link_ip.c:40` vs `:209` | Unify on one code (e.g., `FF_ERROR` for both). |
| R1-11 | Minor | SRS-REQ-061 — error handling | **Return values of `FF_PalMutexLock` and `FF_PalMutexUnlock` are never checked** in `tsn_eth_rx_cb` or `tsn_link_recv`. | `link_tsn.c:56,63,210,212,230` | Check the return value of every `FF_PalMutexLock` / `FF_PalMutexUnlock` call; on failure, return an error or panic. |
| R1-12 | Minor | SRS-REQ-061 — idempotency | **`tsn_link_init` has no guard against double-initialisation.** Calling `FF_LinkSelect(&FF_LinkTsnBackend)` twice invokes `tsn_eth_init` twice, which calls `FF_PalMutexCreate` twice (line 68) — the second call allocates a new mutex handle and leaks the old one. | `link_tsn.c:68` | Add an init-flag guard mirroring the `ip_init` pattern, or make `FF_PalMutexCreate` idempotent. |
| R1-13 | Comment | SRS-REQ-061 — VLAN tagging | **TSN send does not insert an 802.1Q VLAN tag** despite accepting a `vid` config field. | `link_tsn.c:170–197`; `link_tsn.h:9` | Document whether the MAC hardware inserts the tag, or implement tag insertion when `vid != 0`. |
| R1-C1 | Comment | SRS-REQ-061 — recv timeout | `tsn_link_recv` discards `timeout_ms` (line 202) — the RX ring is polled, not blocked. | `link_tsn.c:200–202` | Documented design choice; ensure the caller pattern is documented. |
| R2-01 | Minor | V&V — fragmented stream coverage | **No test exercises multi-read reassembly.** The mock `read_bytes` returns all available data in one call; Phase 1 and Phase 4 loops are never exercised with partial reads. | `test_link_ip.c:43–56` | Add a mock `read_bytes` that returns 1 byte per call, then verify multi-fragment reassembly. |
| R2-02 | Minor | V&V — error propagation | **No test propagates stream errors or timeouts.** No test for `read_bytes` returning `FF_ERROR`/`FF_TIMEOUT`, or `write_bytes` returning `FF_BUFFER_FULL`. | `test_link_ip.c` | Add mock variants that return errors at specific points. |
| R2-03 | Minor | V&V — tautological assertions | **`test_ip_udp_backend_null_args` (line 355) uses `||` accepting both `FF_NOT_READY` and `FF_SUCCESS`; similarly `test_tsn_recv_null_args` (lines 269, 273).** | `test_link_ip.c:355`; `test_link_tsn.c:269,273` | Assert the specific expected status code; remove the `||` tautology. |
| R2-04 | Minor | V&V — RX boundary coverage | **No test exercises the TSN RX callback's short-frame or oversized-payload drop paths.** | `link_tsn.c:41,52`; `test_link_tsn.c` | Add tests for a 14-byte (header-only) frame and a frame payload exceeding `FF_LINK_TSN_RX_FRAMESZ`. |
| R2-05 | Minor | V&V — seg recv length check | **No test covers the `FF_BAD_SIZE` path** when the decoded length prefix exceeds `FF_LINK_SEG_MAX_FRAME`. | `link_seg.c:78–79`; `test_link_ip.c` | Craft a mock stream with prefix `[0x04, 0x01]` (1025) and verify `FF_BAD_SIZE`. |
| R2-06 | Minor | V&V — partial write | **No test exercises `ip_write_bytes` returning `FF_BUFFER_FULL`.** | `test_link_ip.c:131–144` | Add a mock that returns `*sent < len` and verify handling. |
| R2-C1 | Comment | V&V — concurrency coverage | No test exercises concurrent RX callback and `tsn_link_recv`. | `test_link_tsn.c` | Note under GAP-A5; consider a single-core interrupt-preemption test. |

**Severity definition.** Major = non-conformity that disqualifies the item for the applicable criticality or can defeat a requirement/safety barrier (R1-01, R1-02). Minor = non-conformity that does not invalidate the requirement but must be corrected or justified (R1-03–R1-12, R2-01–R2-06). Comment = observation, no derogation (R1-13, R1-C1, R2-C1).

### 3.2 PA findings (evidence-record conformance)

| # | Severity | Item / Requirement | Finding | Evidence (file:line) | Recommendation |
|---|----------|-------------------|---------|---------------------|----------------|
| **PA-MN-01** | **Minor** | N2/N8 — SPDX header URL typo | **`link_ip.h:24` has a broken GitHub URL:** `https://github.com/ilfrick-frick-fsw` (missing `/` between `ilfrick` and `frick-fsw`). Every other file in `src/transport/link/` uses the correct `https://github.com/ilfrick/frick-fsw`. The SPDX header is the canonical copyright/licensing attribution; a broken URL undermines provenance traceability (N13) and is a documentation defect in a project-managed file. | `link_ip.h:24` (confirmed: `github.com/ilfrick-frick-fsw`); contrast `link_seg.c:12`, `link_ip.c:8`, `link_tsn.h:19`, `link_tsn.c:8`, `link.h:11`, `link.c:4`, `link_seg.h:19` (all correct) | Fix to `https://github.com/ilfrick/frick-fsw` (add the missing slash). |
| **PA-MN-02** | **Minor** | N16/N19 — sprint-plan header/history mismatch | **`docs/planning/sprint-plan.md` header revision is 0.14 (line 7) but the `## History` table's latest entry is 0.15 (line 125).** The History table records S4b delivery at 0.15 (2026-10-13) with `gtest 27/27 + trace + Lean + smoke + GSW all PASS` — but the document header was never advanced, so the header no longer matches the last history row. This violates norm N16 (detailed revision history) and creates an ambiguity about which revision is current. | `sprint-plan.md:7` (header: `0.14`); `:125` (history last row: `0.15`) | Advance the header Revision from `0.14` to `0.15`. |
| **PA-MN-03** | **Minor** | N8/N16/N19 — SRS date staleness | **`docs/requirements/SRS.md` (line 8) is dated `2026-10-12`** while the S4b increment it documents (§3, lines 353–367) was delivered on `2026-10-13` and marked "(S4b: implemented)". Every other S4b-era document and the CHANGELOG History 0.43 are dated 2026-10-13. The SRS document date is stale, violating N8/N16. | `SRS.md:8` (`Date | 2026-10-12`); `SRS.md:357,362,366` (S4b content marked "implemented"); `sprint-plan.md:125`, `CHANGELOG.md` (History 0.43), all S4b review reports dated 2026-10-13 | Advance the SRS document Date to `2026-10-13` and the Revision to 1.23 (or re-baseline per the project convention). |
| **PA-MN-04** | **Minor** | N8/N11/N16 — CHANGELOG test-count inconsistency | **`CHANGELOG.md` line 22–23 states "`test_link_tsn` (11 TSN, **23** checks)" but the actual test file has 11 test functions** (confirmed by `FF_TEST_MAIN` in `test_link_tsn.c:369–372` and by the R1+R2 review evidence ledger). `verification-results.md` line 54 and the integration manual §8 status table (line 204) both correctly record `test_link_tsn` = 11 checks. The "23" is inconsistent with the test file, the V&V results, and the integration manual — a count-conformance defect (N8/N11/N16). | `CHANGELOG.md:22-23` ("11 TSN, 23 checks"); `test_link_tsn.c:369-372` (11 test functions in `FF_TEST_MAIN`); `verification-results.md:54` (`test_link_tsn` 11 checks); `user-integration-manual.md:204` (11 checks) | Correct "23 checks" to "11 checks" in the CHANGELOG. |

### 3.3 Additional observations (Comments, no derogation)

| # | Severity | Item | Note | Evidence |
|---|----------|------|------|----------|
| PA-CM-01 | Comment | N8/N11 — records truthfulness | `docs/status/status.json` (lines 34–39) sets `documentation_review: green`, `product_assurance_review: green`, and `reviews: green`, but this PA review is **in progress** and the R1+R2 review (FRICK-REVIEW-0165) was **REJECTED** (2 Major open). The status.json `note` field (line 5) records the R4 documentation review as APPROVED WITH COMMENTS but does **not** mention the R1+R2 REJECTED outcome, nor the open PA findings. The semaphores over-state the gate state at this instant. | `status.json:34-39` (semaphores); `:5` (note omits R1/R2 PA outcome); `README.md` review log (R1/R2 row missing) |
| PA-CM-02 | Comment | N11 — review log gap | `docs/reviews/README.md` has no row for the R1+R2 review (FRICK-REVIEW-0165); only the R4 documentation review row exists (line 164). Every review must be logged (norm N11). | `docs/reviews/README.md:164` (R4 row present; R1/R2 row absent) | Add the missing R1/R2 row (and the PA row). |
| PA-CM-03 | Comment | N8/N11/N16 — verification record omission | The S4b cycle-log Comment column (`verification-results.md:54`) mentions the R4 documentation review (FRICK-REVIEW-0164, APPROVED WITH COMMENTS) but does **not** mention the R1+R2 code/V&V review (FRICK-REVIEW-0165, REJECTED). The V&V results record must accurately reflect the review outcomes (N11). | `verification-results.md:54` (cycle-log Comment) | Append the R1/R2 and PA review outcomes to the cycle-log Comment. |

**Severity definition for PA findings.** Major = non-conformity that disqualifies the item or defeats a safety barrier. Minor = non-conformity that must be corrected or justified. Comment = observation/recommendation, no derogation.

## 4. Conclusion

- [ ] APPROVED — no findings or Comments only
- [ ] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [x] **REJECTED** — two Major findings (R1-01, R1-02) from FRICK-REVIEW-0165 remain open and unresolved; 10 Minor (R1-03–R1-12) + 6 V&V-gaps (R2-01–R2-06) + 4 PA Minor (PA-MN-01..04) + 3 Comments open

**Rationale.** The S4b increment demonstrates structural competence: the `FF_Link_t` vtable dispatch follows the documented contract, buffers are statically allocated (SRS-REQ-015), the requirement chain is traced in `traceability.yaml`, and the 27 ctest checks pass (R1+R2 evidence ledger). The R4 documentation review (FRICK-REVIEW-0164) confirmed the integration manual accurately reflects the S4b implementation, and NC-INT-MAN-01 is closed.

However, this PA review identifies **two independent and decisive disqualifiers**:

1. **R1-01 (Major) — TCP reassembly desync (SRS-REQ-059).** The seg layer's `FF_LinkSegRecvFrame` Phase 4 (line 117) sets `seg->rx_len = 0u` on a partial-read timeout, discarding the length prefix and any partial payload that the comment on line 116 claims to "keep." On the next call, the remaining TCP stream bytes are reinterpreted as a new length prefix, desynchronising the entire frame stream. This defeats the core purpose of SRS-REQ-059 ("segmentation and reassembly of PUS-C/CCSDS frames for stream-oriented links (TCP)"). The defect is confirmed first-hand by direct source read: `link_seg.c:100–127`, with the code/comment contradiction at `:116–117`. No corrective delta exists in the working tree.

2. **R1-02 (Major) — TSN RX callback uses a blocking mutex in ISR context (SRS-REQ-061).** `tsn_eth_rx_cb` (line 56) calls `FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER)`. `FF_PalMutexLock` wraps `pthread_mutex_lock` (`pal_posix.c:248`), which is **not ISR-safe** on RTEMS POSIX — from an ISR it returns an error instead of blocking. The return value is **not checked** (line 56), so on real hardware the lock silently fails and the RX ring buffer (`g_rx_head`/`g_rx_tail`/`g_rx_count`) is mutated without synchronization — a data race that can corrupt the RX ring and deliver garbage CCSDS frames to PUS-C. This is a real-time safety concern at ECSS Class B. The PAL implementation (`pal_posix.c:241–264`) is confirmed first-hand: `FF_PalMutexLock` returns `FF_TIMEOUT` on lock failure, but `link_tsn.c:56` discards the return value. No corrective delta exists in the working tree.

Both Major defects are **zero-coverage** in the current test suite — the mocks (`test_link_ip.c:43–56` `mock_read` returns all data at once; `test_link_tsn.c:129–135` `simulate_rx` runs in task context) cannot discriminate either defect. This is the root V&V gap (R2-01 through R2-06).

The 10 Minor code findings (R1-03–R1-12), the 6 V&V-gaps (R2-01–R2-06), and the 4 PA Minor findings (PA-MN-01..04) must all be resolved in the corrective delta. Per norm N12, a REJECTED review **blocks commit/push** until all Major findings are resolved and the deltas are re-reviewed. Per norm N20, the PA review must be repeated until it passes.

**Confidence: High** for all R1/R2 findings — every claim was verified by direct, first-hand source read and cross-reference to `pal_posix.c`, the HAL contracts, the test files, and the traceability manifest. The PA Minor findings (PA-MN-01..04) are likewise confirmed by direct file reads. **Medium** for the on-target ISR-delivery assumption (R1-02 severity) — the MAC driver's ISR→callback dispatch path is not available in this host-only review window (GAP-A5).

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| **PA (N20)** | Senior SW Product Assurance engineer (independent context, norm N9/N20) | 2026-10-13 | **REJECTED** — two Major findings (R1-01 seg-layer TCP desync, R1-02 TSN ISR-unsafe blocking mutex) open and unresolved, blocking commit/push per N12/N20; 10 Minor (R1-03–R1-12) + 6 V&V-gaps (R2-01–R2-06) + 4 PA Minor (PA-MN-01/02/03/04) + 5 Comments (R1-13, R1-C1, R2-C1, PA-CM-01/02/03) tracked to closure |

## 6. Follow-up records

(norm N12: NCs are resolved by re-applying ALL norms to the affected scope and re-reviewing the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | **Major** | SRS-REQ-059 | Phase 4 partial-read recovery discards rx_partial state (`rx_len = 0u` at `link_seg.c:117`); comment contradicts code; TCP stream desync | Restructure Phase 4 to buffer partial payload into rx_partial; add fragmented-stream test (R2-01); re-run full gate set + trace + Lean | Code diff + new frag test + ctest 27/27 + trace PASS + Lean PASS | frick-fsw | **open** | **S4b delta (pre-commit)** |
| R1-02 | **Major** | SRS-REQ-061 | Blocking mutex (`FF_PAL_WAIT_FOREVER`) in MAC RX callback (ISR context); return value unchecked; ring buffer unprotected on lock failure | Use ISR-safe critical section or lock-free SPSC ring; check mutex return values; document callback delivery context; note under GAP-A5 | Code + header diff + ISR-safety analysis + ctest | frick-fsw | **open** | **S4b delta (pre-commit)** |
| R1-03 | Minor | SRS-REQ-061 | TOCTOU read of `g_rx_count` before lock | Remove pre-lock check; acquire lock first | Code diff + ctest | frick-fsw | open | S4b delta |
| R1-04 | Minor | SRS-REQ-061 | 1038-byte stack allocation in `tsn_link_send` | Move to mutex-protected static buffer or document stack budget | Code diff + memory-budget note | frick-fsw | open | S4b delta |
| R1-05 | Minor | SRS-REQ-060 | Dead variables `g_tcp_rx` / `g_tcp_rx_len` | Remove | Code diff + ctest | frick-fsw | open | S4b delta |
| R1-06 | Minor | SRS-REQ-060 | Dead define `FF_LINK_IP_MAX_FRAME` | Use for size validation or remove | Code diff | frick-fsw | open | S4b delta |
| R1-07 | Minor | SRS-REQ-060 | Blocking `accept()` without timeout | Non-blocking accept or document blocking behaviour | Code diff | frick-fsw | open | S4b delta |
| R1-08 | Minor | SRS-REQ-060 | Silent default fallback in `ip_init` | Return `FF_NOT_READY` or document the fallback | Code diff | frick-fsw | open | S4b delta |
| R1-09 | Minor | SRS-REQ-060 | Static backend name `"ip_udp"` in TCP mode | Set dynamically or document | Code diff | frick-fsw | open | S4b delta |
| R1-10 | Minor | SRS-REQ-060 | Inconsistent partial-write error code (TCP `FF_BUFFER_FULL` vs UDP `FF_ERROR`) | Unify TCP/UDP error codes | Code diff | frick-fsw | open | S4b delta |
| R1-11 | Minor | SRS-REQ-061 | Unchecked mutex return values (lines 56, 63, 210, 212, 230) | Check every `FF_PalMutexLock`/`Unlock` return | Code diff | frick-fsw | open | S4b delta |
| R1-12 | Minor | SRS-REQ-061 | No idempotency guard in `tsn_link_init` | Add init-flag mirror of `ip_init` | Code diff + idempotency test | frick-fsw | open | S4b delta |
| R1-13 | Comment | SRS-REQ-061 | No VLAN tag insertion in TSN send | Document HW-inserted tag or implement | Documentation note or code fix | frick-fsw | n/a | — |
| R1-C1 | Comment | SRS-REQ-061 | `tsn_link_recv` discards `timeout_ms` | Document caller poll pattern | Documentation note | frick-fsw | n/a | — |
| R2-01 | Minor | V&V | No fragmented-stream reassembly test | Add 1-byte-per-call mock test for Phase 1 + Phase 4 | New test function | frick-fsw | open | S4b delta |
| R2-02 | Minor | V&V | No stream error/timeout propagation test | Add error-returning mock variants | New test functions | frick-fsw | open | S4b delta |
| R2-03 | Minor | V&V | Tautological assertions (`||` on status codes) | Assert specific status codes | Test diff | frick-fsw | open | S4b delta |
| R2-04 | Minor | V&V | No short-frame / oversized-payload RX drop test | Add drop-path tests | New test functions | frick-fsw | open | S4b delta |
| R2-05 | Minor | V&V | No over-max-length prefix test (`FF_BAD_SIZE` path) | Add crafted-stream test | New test function | frick-fsw | open | S4b delta |
| R2-06 | Minor | V&V | No partial TCP write test (`FF_BUFFER_FULL` in `ip_write_bytes`) | Add mock returning `sent < len` | New test function | frick-fsw | open | S4b delta |
| R2-C1 | Comment | V&V | No concurrency/ISR test | Note; consider single-core interrupt test under GAP-A5 | — | — | n/a | — |
| **PA-MN-01** | **Minor** | N2/N8/N13 — SPDX URL typo | `link_ip.h:24` GitHub URL missing slash: `ilfrick-frick-fsw` | Fix URL to `github.com/ilfrick/frick-fsw` | 1-line code diff + clang-format | frick-fsw | **open** | S4b delta |
| **PA-MN-02** | **Minor** | N16/N19 — sprint-plan header/history mismatch | Header rev 0.14 ≠ History last row 0.15 | Advance header to 0.15 | 1-line doc diff | frick-fsw | **open** | S4b delta |
| **PA-MN-03** | **Minor** | N8/N16/N19 — SRS date staleness | SRS.md dated 2026-10-12; S4b content delivered 2026-10-13 | Advance Date to 2026-10-13 and Rev to 1.23 | 1-line doc diff + R4 re-verify | frick-fsw | **open** | S4b delta |
| **PA-MN-04** | **Minor** | N8/N11/N16 — CHANGELOG test-count inconsistency | CHANGELOG says `test_link_tsn` 23 checks; actual = 11 (verified in test file + V&V results + integration manual) | Correct to "11 checks" | 1-line doc diff | frick-fsw | **open** | S4b delta |
| PA-CM-01 | Comment | N8/N11 — records truthfulness | `status.json` semaphores (`product_assurance_review`, `reviews`) = green while PA review is in progress and R1/R2 is REJECTED | Set green only after PA closure, or annotate | (delta) `status.json` | frick-fsw (coordinator) | open | — | S4b (commit) |
| PA-CM-02 | Comment | N11 — review log gap | `docs/reviews/README.md` has no row for FRICK-REVIEW-0165 (R1/R2) | Add the missing R1/R2 row | (delta) README.md | frick-fsw (coordinator) | open | — | S4b (commit) |
| PA-CM-03 | Comment | N8/N11/N16 — verification record omission | S4b cycle-log Comment (`verification-results.md:54`) omits the R1/R2 REJECTED outcome | Append R1/R2 + PA outcomes to the cycle-log Comment | (delta) verification-results.md | frick-fsw (coordinator) | open | — | S4b (commit) |

> This report is authored to be committed and pushed with the reviewed change (norm N11). The reviewer wrote exactly one file (this report); no other tracked file was modified. Gate commands regenerated only git-ignored `build/` artifacts.

---

## Evidence ledger (all verified first-hand, this session)

| Claim | Method | Result |
| ----- | ------ | ------ |
| R1-01 (seg-layer desync) | Line-by-line trace of `FF_LinkSegRecvFrame` Phase 4 (`link_seg.c:100–127`); analysis of `rx_len = 0u` at line 117 vs comment at line 116; identification of desync on retry | **Confirmed** — partial state lost; continuation bytes misinterpreted as new prefix |
| R1-02 (TSN ISR mutex) | Read `tsn_eth_rx_cb` line 56; cross-check `pal_posix.c:241–264` (`pthread_mutex_lock` from ISR is unsafe on RTEMS POSIX); verify return-value omission at 56, 63, 210, 212, 230 | **Confirmed** — blocking `FF_PAL_WAIT_FOREVER` in ISR-path callback; unchecked return |
| R1-03 (TOCTOU race) | Read lines 41 vs 56–57 | **Confirmed** — `g_rx_count` read before lock |
| R1-04 (stack alloc) | Read line 170; compute `14 + 1024 = 1038` | **Confirmed** |
| R1-05 (dead variables) | grep `g_tcp_rx`/`g_tcp_rx_len` in `link_ip.c` | Declared L26–27, memset L142–143, never read — **Confirmed** |
| R1-06 (dead define) | grep `FF_LINK_IP_MAX_FRAME` across project | Defined L21 only; never referenced — **Confirmed** |
| R1-07 (blocking accept) | Read `link_ip.c:130` | `stack->accept(g_sock, NULL, NULL)` with no timeout — **Confirmed** |
| R1-08 (silent fallback) | Read `link_ip.c:167–179` | `ip_init` silently sets `g_cfg.stack = &FF_NetStackLoopback` when NULL — **Confirmed** |
| R1-09 (backend name) | Read `link_ip.c:240` | Static `"ip_udp"` in TCP mode — **Confirmed** |
| R1-10 (error-code inconsistency) | Read `link_ip.c:40` vs `:209` | TCP returns `FF_BUFFER_FULL`; UDP returns `FF_ERROR` for partial send — **Confirmed** |
| R1-11 (unchecked mutex returns) | Read `link_tsn.c:56,63,210,212,230` | All `FF_PalMutexLock`/`FF_PalMutexUnlock` returns discarded — **Confirmed** |
| R1-12 (no idempotency guard) | Read `link_tsn.c:68` vs `link_ip.c:180` | `tsn_link_init` has no init flag; `ip_init` does (`g_sock >= 0`) — **Confirmed** |
| R1-13 (no VLAN tag) | Read `link_tsn.c:170–197`, `link_tsn.h:9` | No 802.1Q tag insertion; `vid` accepted in config but unused — **Confirmed** (Comment) |
| R1-C1 (recv timeout discarded) | Read `link_tsn.c:200–202` | `(void)timeout_ms;` — **Confirmed** (Comment) |
| R2-01 (no fragmentation test) | Read `mock_read` (`test_link_ip.c:43–56`): always returns all available data; no test returns partial data | **Confirmed** — Phase 1/4 multi-read loops untested |
| R2-02 (no error propagation) | Read `mock_read` (always `FF_SUCCESS`), `mock_net_send` (only fails on ring-full); no test injects `FF_ERROR`/`FF_TIMEOUT`/`FF_BUFFER_FULL` | **Confirmed** |
| R2-03 (tautological assertions) | Read `test_link_ip.c:355` (`|| FF_SUCCESS`), `test_link_tsn.c:269,273` (`|| FF_NOT_READY`) | **Confirmed** |
| R2-04 (no short/oversized RX test) | Read `test_link_tsn.c` test list | No test exercises `len <= 14` or `payload_len > FF_LINK_TSN_RX_FRAMESZ` — **Confirmed** |
| R2-05 (no over-max prefix test) | Read `test_link_ip.c` test list | No test for `frame_len > FF_LINK_SEG_MAX_FRAME` in recv — **Confirmed** |
| R2-06 (no partial write test) | Read `mock_net_send` (`test_link_ip.c:131–144`): always `*sent = len` | **Confirmed** |
| R2-C1 (no concurrency test) | Read `test_link_tsn.c` | All tests single-threaded; `simulate_rx` runs in task context — **Confirmed** (Comment) |
| PA-MN-01 (URL typo) | Read `link_ip.h:24`; grep URL across all link files | `github.com/ilfrick-frick-fsw` (missing `/`); all 8 other files correct — **Confirmed** |
| PA-MN-02 (sprint-plan rev) | Read `sprint-plan.md:7` (header 0.14) vs `:125` (History 0.15) | Header ≠ last History row — **Confirmed** |
| PA-MN-03 (SRS date) | Read `SRS.md:8` (2026-10-12) vs `:357,362,366` (S4b "implemented"); `sprint-plan.md:125`, `CHANGELOG.md` History 0.43 (2026-10-13) | Date stale by one day — **Confirmed** |
| PA-MN-04 (CHANGELOG count) | Read `CHANGELOG.md:22-23` ("23 checks" for `test_link_tsn`); `test_link_tsn.c:369-372` (11 functions); `verification-results.md:54` (11 checks); `user-integration-manual.md:204` (11 checks) | "23" inconsistent with 11 everywhere else — **Confirmed** |
| PA-CM-01 (status.json truthfulness) | Read `status.json:34-39` (semaphores green); `:5` (note omits R1/R2 PA outcome) | Semaphores over-state gate state — **Confirmed** (Comment) |
| PA-CM-02 (review log gap) | Read `docs/reviews/README.md:164` (R4 row present) | R1/R2 row absent — **Confirmed** (Comment) |
| PA-CM-03 (cycle-log omission) | Read `verification-results.md:54` (cycle-log Comment) | R1/R2 REJECTED outcome not recorded — **Confirmed** (Comment) |
| Traceability (N4) | Read `traceability.yaml:552–577`; `SRS.md:353–367` | SRS-REQ-059→`link_seg.{c,h}`; -060→`link_ip.{c,h}`; -061→`link_tsn.{c,h}`; tests mapped; `lean: []` with notes |
| Trace gate | Re-run `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | (see §7) |
| R1-03 through R1-12, R2-01–R2-06, Comments | All read first-hand above | **All confirmed** — no corrective delta applied in the working tree |
| No fix applied since R1/R2 | `git diff` on the S4b files vs the committed baseline (471afb5+) | No delta modifies `link_seg.c:117`, `link_tsn.c:56`, or any of the confirmed Minor/V&V-finding locations |

## Norm-by-norm audit (N1–N20)

| Norm | Requirement | Verification | Status |
| ---- | ----------- | ------------ | ------ |
| N1 | Lean verification of generated code | SRS-REQ-059/060/061 have `lean: []` with documented `lean_note` justifications in `traceability.yaml:558,567,576` — no formal model is claimed for these reference backends (consistent with the standing scope-conformance rule). `lake build` PASS (21/21). | compliant |
| N2 | English only | All new/changed content is in English. | compliant |
| N3 | Backwards ECSS SRS/SSS | SRS-REQ-059/060/061 present, parented to SSS-REQ-028 (`traceability.yaml:554,563,572`). | compliant |
| N4 | Bidirectional traceability | `trace.py` validates the manifest; SSS-REQ-028 → SRS-REQ-059/060/061 → source + tests. `lean: []` with notes. | compliant |
| N5 | Class B target; class-A gaps documented | On-target link-level behaviour recorded as GAP-A5 (`class-a-gaps.md`); ECSS Class B baseline (`ecss-criticality-b.md`). | compliant |
| N6 | Full SW V&V; Docker-deployed environments | 27/27 ctest PASS; `lake build` 21/21; trace PASS; smoke PASS; Lean PASS; GSW PASS; Renode SKIP (host-only, correct proxy). | compliant |
| N7 | Dual push | **not performed** — this review is a pre-commit gate; commit/push is the human/authority action | n/a to the reviewer |
| N8 | Complete, current documentation | **Stale** on S4b content delivery date (PA-MN-03 SRS.md) and test-count consistency (PA-MN-04 CHANGELOG) | **partial — 2 Minors open** |
| N9 | Independent review agents | R1+R2 (FRICK-REVIEW-0165) and R4 (FRICK-REVIEW-0164) by independent-context agents; this PA review independent | compliant |
| N10 | Test-script/V&V-tool review & qualification | `test_link_ip.c` and `test_link_tsn.c` covered by existing T13 (unit test harness); no new V&V tool introduced in S4b | compliant |
| N11 | Scripts + review results in the repo | R4 and R1/R2 review reports present in `docs/reviews/`; R1/R2 row missing from README log (PA-CM-02); verification-results.md line 405/406 pre-populated with wrong conclusions | **partial** |
| N12 | NC resolution to full compliance | 18 findings (2 Major + 16 Minor) + 2 R1/R2 Comments + 3 PA Comments open; **a REJECTED review blocks commit/push** | **blocked** |
| N13 | AGPL-3.0-only, SPDX | All new source files carry `SPDX-License-Identifier: AGPL-3.0-only`; URL typo in `link_ip.h:24` is a records defect, not a licensing one | compliant (PA-MN-01 to fix) |
| N14 | SSS review by Senior Systems Engineer | No SSS changes in S4b; R3 review not triggered | not triggered |
| N15 | Full cycle on every change | S4b ran the full cycle (build, ctest, Lean, trace, smoke, docs); **the cycle cannot complete because the R1/R2 review is REJECTED** | blocked by R1-01/R1-02 |
| N16 | Detailed revision history | CHANGELOG History 0.43 present; sprint-plan rev mismatch (PA-MN-02); SRS date staleness (PA-MN-03) | **partial** |
| N17 | R4 documentation review before commit | FRICK-REVIEW-0164 returned APPROVED WITH COMMENTS; NC-INT-MAN-01 closed | compliant |
| N18 | Renode functional validation | SKIP proxy (host-only code, no RTEMS image change) — correctly recorded, not silent | compliant (SKIP) |
| N19 | Loop audit vs declared objectives/norms | `loop-audit.md:27` (M4 row) records S4b as delivered (SRS-REQ-059/060/061); **the audit does not record the R1/R2 REJECTED outcome** | **partial — PA-CM-03** |
| N20 | Independent PA review of every phase product | This report | **REJECTED — Majors R1-01/R1-02 open + 4 PA Minor + 3 PA Comments** |

## Facts

- `FF_LinkSegRecvFrame` Phase 4 (`link_seg.c:100–127`) reads the payload directly into the caller's `frame` buffer, not into `rx_partial`.
- At `link_seg.c:117`, `seg->rx_len = 0u` discards the 2-byte prefix and any partial state; the comment at line 116 says "keep what we have in rx_partial for next call" — the code and comment disagree.
- `tsn_eth_rx_cb` (`link_tsn.c:56`) is registered as the MAC RX callback (line 107) and calls `FF_PalMutexLock(g_lock, FF_PAL_WAIT_FOREVER)` with no return-value check.
- `FF_PalMutexLock` wraps `pthread_mutex_lock` (`pal_posix.c:248`), which is not ISR-safe on RTEMS POSIX; the function returns `FF_TIMEOUT` on lock failure, but the return is discarded.
- `g_rx_count` is read unsynchronized at `link_tsn.c:41` before the lock at line 56.
- `g_tcp_rx` / `g_tcp_rx_len` (`link_ip.c:26–27`) are initialised at lines 142–143 and never read.
- `FF_LINK_IP_MAX_FRAME` (`link_ip.c:21`) is defined and never referenced.
- `tsn_link_send` allocates `uint8_t frame[1038]` on the stack (`link_tsn.c:170`).
- The mock `read_bytes` (`test_link_ip.c:43–56`) returns all available data in one call; never returns partial data or 0 with `FF_SUCCESS`.
- The mock `net_send` (`test_link_ip.c:131–144`) always sets `*sent = len`; never returns a partial send.
- Traceability: SRS-REQ-059 → `link_seg.{c,h}`; SRS-REQ-060 → `link_ip.{c,h}`; SRS-REQ-061 → `link_tsn.{c,h}` (`traceability.yaml:552–577`). All have `lean: []` with documented `lean_note`.
- ctest: 27/27 PASS (S4b cycle-log row `verification-results.md:54`); Renode: SKIP (host-only code).
- R1/02 review (FRICK-REVIEW-0165) conclusion: REJECTED (2 Major open); 10 Minor + 6 V&V-gaps tracked; 3 Comments.
- R4 documentation review (FRICK-REVIEW-0164) conclusion: APPROVED WITH COMMENTS (3 Minor addressed, NC-INT-MAN-01 closed).
- NC-INT-MAN-01 is closed (code/doc findings from FRICK-REVIEW-0160 resolved in rev 0.7/0.8; FRICK-REVIEW-0164 confirmed convergence).
- `link_ip.h:24` URL: `https://github.com/ilfrick-frick-fsw` (missing `/`); all 8 other link files use the correct `https://github.com/ilfrick/frick-fsw`.
- `sprint-plan.md:7` header Revision = `0.14`; History table last entry (line 125) = `0.15`.
- `SRS.md:8` Date = `2026-10-12`; S4b content (lines 353–367, "(S4b: implemented)") delivered `2026-10-13`; all S4b-era documents and CHANGELOG History 0.43 use `2026-10-13`.
- `CHANGELOG.md:22-23`: "`test_link_tsn` (11 TSN, **23** checks)"; `test_link_tsn.c:369-372` has 11 test functions; `verification-results.md:54` says "11 checks"; `user-integration-manual.md:204` says "11 checks".
- `status.json:35-39`: `documentation_review`, `product_assurance_review`, and `reviews` semaphores all `green`; the PA review is in progress and R1/R2 is REJECTED.
- `docs/reviews/README.md:164`: only the R4 row exists for the S4b cycle; the R1/R2 row is missing.

## Assumptions

- *The MAC RX callback fires from ISR context on the target (Cadence GEM / PolarFire SoC).* — owner: hardware/MAC driver; impact: R1-02 severity; validation: confirm GEM ISR→callback delivery path (GAP-A5).
- *The mock `read_bytes` returning all data at once does not represent real TCP fragmentation.* — owner: V&V; impact: R1-01 and R2-01; validation: add a fragmented mock and test.
- *The `FF_NetStack_t` `recv` may return `FF_SUCCESS` with `*got == 0` on timeout (per `ff_net_loopback.h` contract).* — owner: backend implementer; impact: triggers the R1-01 bug; validation: confirm against the real IP stack adapter.
- *Renode N18 SKIP is correctly recorded as a proxy, not an unlogged omission.* — owner: V&V; impact: no target evidence for TSN ISR or TCP fragmentation (GAP-A5).

## Inferences

- Because Phase 4 discards `rx_partial` and the mock never returns partial data, R1-01 is untested by construction — the test suite cannot detect it.
- Because the TSN RX callback uses a blocking mutex that is not ISR-safe, the reference backend cannot be safely deployed on the target without modification or explicit documentation that the MAC driver delivers callbacks from task context.
- Because the CHANGELOG records "23 checks" for `test_link_tsn` while every other source of record (test file, verification-results, integration manual) says 11, the CHANGELOG is internally inconsistent with the V&V evidence (PA-MN-04).
- Because `status.json` shows `product_assurance_review: green` and `reviews: green` while this PA review is REJECTED and the R1/R2 review is REJECTED, the status record over-states the gate state (PA-CM-01).
- Because the S4b cycle-log Comment records only the R4 review outcome and omits the R1/R2 REJECTED outcome, the V&V evidence record is incomplete (PA-CM-03).

## Recommendations

1. **Resolve R1-01 (Major) before commit (N12/N20):** Restructure Phase 4 to buffer partial payload into `rx_partial` so `FF_LinkSegRecvFrame` can resume on the next call. The correct fix: in the `got == 0u` branch, copy `frame[0..total-1]` into `rx_partial[FF_LINK_SEG_PREFIX_LEN..FF_LINK_SEG_PREFIX_LEN+total-1]`, set `rx_len = FF_LINK_SEG_PREFIX_LEN + total`, and return `FF_TIMEOUT`. Re-run full gates + Renode. Owner: authoring agent; sprint: **S4b delta (pre-commit)**.

2. **Resolve R1-02 (Major) before commit (N12/N20):** Replace the blocking mutex in `tsn_eth_rx_cb` with an ISR-safe mechanism (interrupt disable/restore or a lock-free SPSC ring with atomic head/tail on the target). Check all `FF_PalMutexLock`/`FF_PalMutexUnlock` return values. Document the callback delivery context (ISR vs task) in the header. Owner: authoring agent; sprint: **S4b delta (pre-commit)**.

3. **Resolve all 10 Minor code findings (R1-03–R1-12) and 6 V&V gaps (R2-01–R6-06)** in the same corrective commit. Add the fragmented-stream mock test (R2-01), error-propagation tests (R2-02), fix tautological assertions (R2-03), add short/oversized RX drop tests (R2-04), add over-max-length-prefix test (R2-05), add partial-write test (R2-06).

4. **Resolve the 4 PA Minor findings (PA-MN-01–PA-MN-04)** in the corrective commit: fix the URL typo (`link_ip.h:24`), advance `sprint-plan.md` header to 0.15, advance `SRS.md` Date to 2026-10-13 and Revision to 1.23, and correct the CHANGELOG `test_link_tsn` count from "23 checks" to "11 checks".

5. **Correct the S4b cycle-log Comment** in `verification-results.md:54` to record the R1/R2 REJECTED outcome and the PA REJECTED outcome.

6. **Add the missing R1/R2 review row** to `docs/reviews/README.md` (FRICK-REVIEW-0165) and the PA row (FRICK-REVIEW-0166).

7. **Update `status.json` semaphores** (`product_assurance_review`, `reviews`) to reflect the REJECTED state — set to `amber`/`pending` or annotate — and include the R1/R2 PA outcome in the `note` field.

8. **Run the R4 (N17) delta re-review** of the corrected documentation, then **re-run the PA (N20) delta review** until no Major/Minor is open, before commit/push.

## Confidence

- **High** for all R1/R2 findings (R1-01 through R1-13, R2-01–R2-06, R1-C1, R2-C1) — every claim was verified by direct source read and cross-reference to `pal_posix.c`, the HAL contracts, the test files, and the traceability manifest. The R1-01 desync is a direct consequence of `link_seg.c:117` (`rx_len = 0u`) contradicting the comment at line 116. The R1-02 ISR issue follows from the PAL implementation (`pthread_mutex_lock` in ISR context) and the unchecked return value at `link_tsn.c:56`.
- **High** for all PA Minor findings (PA-MN-01–04) — each was verified by direct file read and cross-check against parallel documents.
- **Medium** for the on-target ISR-delivery assumption (R1-02 severity) — the MAC driver's ISR→callback dispatch path is not available in this host-only review window (GAP-A5).
- **Medium** for the status.json semaphore convention (PA-CM-01) — the semaphores may follow a loop-baseline convention; the discrepancy is noted for the coordinator.

## Open gaps

- **R1-01 (Major) open** — TCP reassembly desync; blocks commit/push (N12/N20).
- **R1-02 (Major) open** — TSN ISR-unsafe blocking mutex; blocks commit/push (N12/N20).
- **R1-03–R1-12 (Minors) open** — dead code, unchecked mutex returns, idempotency, error-code inconsistency, stack usage, silent fallback, backend name, blocking accept; to close in S4b delta.
- **R2-01–R2-06 (V&V gaps) open** — no fragmented-stream, error-propagation, tautology-fix, short/oversized-RX, over-max-prefix, or partial-write tests; to close in S4b delta.
- **PA-MN-01–04 (PA Minors) open** — URL typo, sprint-plan rev, SRS date, CHANGELOG count; to close in S4b delta.
- **PA-CM-01/02/03 (PA Comments) open** — status.json truthfulness, missing README log row, cycle-log omission; to close before commit.
- **On-target evidence absent** — Renode SKIP (host-only code); no execution of the TSN RX callback in ISR context or the seg layer with a real fragmented TCP stream (GAP-A5).
- **R1-13, R1-C1, R2-C1 (Comments)** — VLAN tag documentation, recv timeout doc, concurrency test note; note under GAP-A5.
