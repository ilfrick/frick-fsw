# FRICK-REVIEW-0420 — S8-FDIR-01 correction round, R2/N22 review

**Profile:** R2 V&V, fresh context  
**Disposition:** Review evidence only. No edits, no testware qualification, no
gate pass, no NC closure.

> **Attribution note (added 2026-10-21, acceptance verification).** This report is the independent review of the S8-FDIR-01 increment **as of the tree state at 2026-10-21T19:30Z**. Its findings were corrected by the implementing agent in the follow-up correction round; the "corrected" entries below record that disposition and the reviewer's own verification of the correction where it was performed. This report is **not** a fresh independent review of the corrected product as a separate delta. The duplicate-emission mutation was executed on a scratch copy and was not retained as evidence at the time; it was re-executed and **retained** in `docs/vv/evidence/2026-10-21-s8-fdir-post-review-remediation.log` during the 2026-10-21 acceptance verification, and the corrected product was independently reviewed once more (see the acceptance review in `docs/reviews/`).

## Independently reproduced

| Check | Result |
|---|---|
| Native build and full CTest | 44/44 passed |
| Targeted counts | `test_pal_watchdog` 320, `test_es_hs_lifecycle` 180, `test_link_uart` 71, `test_time` 113 — all 0 failures, stable over repeated runs |
| TS-5 re-count | 87 rows, ids 1–87 unique; 48 nominal / 29 OOB / 10 mixed; 76 individual / 11 combined; 59 covered / 28 gap; SIL/PIL/HIL 83/1/3 — §3.2 matches the table; every gap row is mapped in §3 |
| Source identity | the 31-entry manifest re-verifies OK against the current tree |
| Lean evidence | the 14 Lean source hashes re-verify; the recorded `Watchdog.olean` digest matches the artefact in the tree |
| Host-timing-as-WCET audit | no instance found in TS-5, verification-results, tool-qualification, loop-audit, traceability, ADR-0021, the handover or the test comments |

## Findings and their disposition in the follow-up round

1. **Major (previous round) — the exactly-one-frame oracle did not discriminate
   a duplicated emission.** The loopback pop drains the whole transmit ring, so
   a second pop found nothing even after a duplicated emission. **Corrected:**
   the case now asserts that the popped length equals exactly one CCSDS space
   packet, derived from the primary-header packet-length field. This was
   verified by a duplicate-emission mutation on a scratch copy of the tree: the
   corrected case now fails, and it failed on the production wiring path, not
   only on the stub-writer path.
2. **Minor — the loopback post-copy deadline re-check had no case, and two
   records claimed it had one.** **Corrected:** the records now state the
   actual semantics — the re-check is a post-copy guard whose `FF_TIMEOUT`
   branch is not reachable in a host test because an in-memory copy cannot
   exhaust the budget, and a returned `FF_TIMEOUT` can mean the frame was
   already enqueued.
3. **Minor — the UART stall sub-case did not observe the partial frame.**
   **Corrected:** the case now reads what reached the master and asserts that
   fewer bytes than requested were emitted.
4. **Minor — neither descriptor-mode restore was regression-covered.**
   **Corrected:** the UART case asserts the descriptor is not left
   non-blocking, and the console case asserts the same for `stderr`.
5. **Minor — the PUS service/subservice of the pre-reset frame was not
   asserted.** **Corrected:** the wiring case now decodes the frame and asserts
   PUS service 5 / subservice 3.
6. **Minor — the TS-5 revision record did not cover the final corrections.**
   **Corrected:** TS-5 is at rev 1.12 with a history row naming the changes
   and stating that no case ID, status or count changed.
7. **Minor — the loop-audit published a superseded count in one row.**
   **Corrected.**
8. **Minor — ADR-0021 was stale on the shared-deadline API.**
   **Corrected** (ADR-0021 rev 1.2).

## Residual limitations (recorded, not defects)

- The `vsnprintf` negative-return guard is a defensive branch that no host
  case can reach; it is recorded as unexercised rather than covered.
- The busy-retry writers are CPU-bound for up to the shared budget and act on
  a descriptor shared with the telemetry and command paths; the target
  concurrency and readiness semantics remain unverified (NC-EXT-191).
- The IP and TSN backends deliver no pre-reset report (NC-EXT-191).
- The `ff_panic_console_write_until` and `uart_send_until` writers are not
  driven by a hardware readiness signal on the target; the target UART driver
  semantics remain a target/HIL item.

## Conclusion

The delta's implementation is correct as described and the corrected oracles
are discriminating. The remaining items are record currency and the recorded
target-side limitations, which are registered as NC-EXT-184..192. No host
timing is presented as target WCET, and no requirement-policy, cadence,
qualification or release decision is made or implied.

**Confidence:** high for the reproduced counts, the CTest result, the TS-5
re-count, the manifest and Lean re-verification and the mutation results.
Medium for the target-side concurrency observations, which are reasoned from
the code paths and are not host-observable.
