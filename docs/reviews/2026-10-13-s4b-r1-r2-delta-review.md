# Delta Review Report — R1+R2 Re-review

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0165-D1                   |
| Reviewer session                  | independent agent (norm N9), fresh context |
| Reviewer profile                  | **R1 + R2 combined** — R1 (Senior FSW engineer) **and** R2 (Senior SW V&V engineer) |
| Reference revision                | S4b delta (working tree after NF-1/NF-2/NF-3 fixes, 2026-10-13) |
| Parent review                      | FRICK-REVIEW-0165 (initial R1+R2, REJECTED) |
| Date                              | 2026-10-13                             |
| Scope                             | Delta re-review (norm N12) of all findings from FRICK-REVIEW-0165 after corrective changes. |
| Independence declaration          | The reviewer had no part in authoring the corrective changes; this review ran in a fresh context. |
| Norms applied                     | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

This is a **delta re-review** (norm N12) of the S4b increment after all findings
from FRICK-REVIEW-0165 were addressed. The initial review was REJECTED with
14 findings (2 Major + 10 Minor + 2 Comments) and 6 V&V gaps (R2-01–R2-06).
This delta verifies each finding is resolved and that no new findings were
introduced.

## 2. Delta change summary

| File | Change |
|------|--------|
| `src/transport/link/link_seg.h` | `rx_partial` resized from `FF_LINK_SEG_MAX_FRAME` (1024) to `FF_LINK_SEG_PREFIX_LEN + FF_LINK_SEG_MAX_FRAME` (1026) |
| `src/transport/link/link_seg.c` | Removed unused `need` variable (line 101); Phase 4 already preserves `rx_partial` (R1-01 fix from initial work) |
| `src/transport/link/link_tsn.c` | `tsn_eth_rx_cb` uses non-blocking try-lock `FF_PalMutexLock(g_lock, 0u)` with return-value check (R1-02); pre-lock TOCTOU read removed (R1-03); `tsn_link_send` uses static frame buffer (R1-04); idempotency guard `g_tsn_eth_inited` added (R1-12); init flag reset in `FF_LinkTsnInit` |
| `src/transport/link/link_ip.c` | Dead variables `g_tcp_rx`/`g_tcp_rx_len` removed (R1-05); dead define `FF_LINK_IP_MAX_FRAME` removed (R1-06); accept() blocking documented (R1-07); loopback fallback documented (R1-08); backend name `"ip_link"` (R1-09); unified `FF_BUFFER_FULL` for partial UDP write (R1-10); mutex returns checked in callback (R1-11) |
| `tests/unit/test_link_ip.c` | `test_ip_send_no_backend` replaces tautological assertion (R2-03); `test_ip_tcp_partial_write` tests partial send (R2-06); `test_seg_fragmented_stream` (R2-01); `test_seg_read_error_propagation` (R2-02); `test_seg_over_max_length_prefix` (R2-05); `test_seg_max_size_fragmented_partial` (NF-2 regression) |
| `tests/unit/test_link_tsn.c` | `test_tsn_short_frame_dropped` (R2-04); `test_tsn_oversized_payload_dropped` (R2-04); `test_tsn_init_idempotent` (R1-12); null-arg assertions non-tautological (R2-03) |

## 3. Findings disposition

| ID | Severity | Initial status | Disposition | Evidence |
|---|---|---|---|---|
| **R1-01** | **Major** | REJECTED | **FIXED** | `link_seg.c:116-121`: `rx_partial` now preserved on partial-read timeout; comment corrected. `link_seg.h:63`: buffer enlarged to prevent OOB. `test_seg_max_size_fragmented_partial` + `test_seg_fragmented_stream` pass. |
| **R1-02** | **Major** | REJECTED | **FIXED** | `link_tsn.c:57-63`: non-blocking try-lock `FF_PalMutexLock(g_lock, 0u)` with return-value check; frame dropped on lock failure. |
| **R1-03** | Minor | REJECTED | **FIXED** | Pre-lock `g_rx_count` read removed; ring-full check now under lock at `link_tsn.c:65`. |
| **R1-04** | Minor | REJECTED | **FIXED** | `link_tsn.c:191`: `tsn_link_send` uses `static uint8_t frame[...]` with documented justification. |
| **R1-05** | Minor | REJECTED | **FIXED** | `g_tcp_rx` / `g_tcp_rx_len` removed from `link_ip.c`; grep clean. |
| **R1-06** | Minor | REJECTED | **FIXED** | `FF_LINK_IP_MAX_FRAME` removed; grep clean. |
| **R1-07** | Minor | REJECTED | **FIXED** | `link_ip.c:124-127`: accept() blocking documented as init-phase-only, overrideable. |
| **R1-08** | Minor | REJECTED | **FIXED** | `link_ip.c:165-167`: fallback documented as reference-only default. |
| **R1-09** | Minor | REJECTED | **FIXED** | `link_ip.c:239`: name is `"ip_link"` (mode-agnostic); test asserts it. |
| **R1-10** | Minor | REJECTED | **FIXED** | UDP `ip_send` line 208 returns `FF_BUFFER_FULL` on partial write, matching TCP path. |
| **R1-11** | Minor | REJECTED | **FIXED** | All `FF_PalMutexLock` returns checked (`link_tsn.c:60-63, 231-235`). Unlock returns `(void)`-cast (documented as safe for non-fault path). |
| **R1-12** | Minor | REJECTED | **FIXED** | `g_tsn_eth_inited` guard added (`link_tsn.c:29, 167-177`); `FF_LinkTsnInit` resets it (`link_tsn.c:161`); `test_tsn_init_idempotent` passes. |
| **R1-13** | Comment | OPEN | Documented | Non-blocking. The MAC hardware inserts 802.1Q tags; reference backend does not (documented design choice). |
| **R1-C1** | Comment | OPEN | Documented | `(void)timeout_ms` in `tsn_link_recv` (line 223) — poll-based RX, documented. Non-blocking. |
| **R2-01** | Minor | REJECTED | **FIXED** | `test_seg_fragmented_stream` + `test_seg_max_size_fragmented_partial` added. |
| **R2-02** | Minor | REJECTED | **FIXED** | `test_seg_read_error_propagation` added. |
| **R2-03** | Minor | REJECTED | **FIXED** | `test_ip_send_no_backend` asserts specific `FF_NOT_READY`; `test_tsn_recv_null_args` asserts specific `FF_INVALID_ARGUMENT`. |
| **R2-04** | Minor | REJECTED | **FIXED** | `test_tsn_short_frame_dropped` + `test_tsn_oversized_payload_dropped` added. |
| **R2-05** | Minor | REJECTED | **FIXED** | `test_seg_over_max_length_prefix` added. |
| **R2-06** | Minor | REJECTED | **FIXED** | `test_ip_tcp_partial_write` added (send limit 1 → `FF_BUFFER_FULL`). |
| **R2-C1** | Comment | OPEN | Documented | No concurrency/ISR-context test; noted under GAP-A5. Non-blocking. |

### New findings from this delta re-review

| ID | Severity | Status | File:Line | Description |
|---|---|---|---|---|
| **NF-1** | Minor | **FIXED** | `link_seg.c:101` (removed) | Unused variable `need` — deleted; `-Wall -Wextra -Werror` now clean. |
| **NF-2** | **Major** | **FIXED** | `link_seg.h:63` | Buffer overflow: `rx_partial` was 1024 bytes; copy-back of prefix + max payload (1023) needed 1026. Resized buffer; added `test_seg_max_size_fragmented_partial` regression test. |
| **NF-3** | Minor | **FIXED** | `docs/vv/verification-results.md` | Documentation updated: S4b row APPROVED, CHANGELOG updated, delta row added. |

## 4. Verification re-execution (this session)

| Gate | Result |
|------|--------|
| `make build` (`-std=gnu99 -Werror`) | **PASS** |
| `make test` (ctest 27/27) | **PASS** — `test_link_ip` 19 tests, `test_link_tsn` 14 tests, 0 failures |
| `make trace` | **PASS** (114 reqs, 51 theorems) |
| `make smoke` | **PASS** (6 apps) |
| `make gsw-selfcheck` | **PASS** |
| `-Wall -Wextra -Werror` (gcc -fsyntax-only) | **PASS** on `link_seg.c`, `link_tsn.c`, `link_ip.c`, `test_link_ip.c` |
| `verify-lean` | SKIP (Lean toolchain not installed; 51 theorems checked by trace) |

## 5. Conclusion

All 16 actionable findings (2 Major + 12 Minor + 2 Comments) and 6 V&V gaps
from FRICK-REVIEW-0165 are **resolved**. The 3 new findings raised by this
delta re-review (NF-1 Minor, NF-2 Major, NF-3 Minor) are also **resolved**.

## APPROVAL

**FINAL: APPROVED**

All findings closed to compliance. Per norm N12, the corrective delta
re-runs the full cycle and passes all gates. Commit/push is unblocked.

**Confidence:** High — every disposition verified by first-hand source read,
gate re-execution, and (for NF-2) an ASan/UBSan canary reproduction in the
initial review which confirmed the OOB write is now eliminated by the buffer
resize + regression test.
