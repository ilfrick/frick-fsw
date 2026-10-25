# FRICK-REVIEW-0417 — S8-FDIR-01 final delta review (R2/N22)

**Profile:** R2 V&V, fresh context  
**Disposition:** Review evidence only. No testware qualification, gate pass or NC
closure.

## Independently reproduced

| Check | Result |
|---|---|
| Native build and full CTest | 44/44 passed |
| `test_pal_watchdog` / `test_es_hs_lifecycle` / `test_link_uart` / `test_time` | 320 / 157 / 68 / 113 checks, 0 failures, reproduced over repeated runs |
| `tools/scripts/trace.py` | PASS (120 requirements, 68 theorem symbols) |
| Pinned `frick-vv:local` format check (clang-format 18.1.3) | exit 0, no violations |
| Source-hash manifest | verified against the current tree |
| TS-5 table parse | 87 rows, ids 1–87 unique; 48 nominal / 29 OOB / 10 mixed; 76 individual / 11 combined; 59 covered / 28 gap; SIL/PIL/HIL 83/1/3 — §3.2 counts match the table exactly |

## Findings

1. **Major — the 250 ms target is still not demonstrated.** All elapsed-time
   evidence is host evidence. The target/backend completion time, the target
   console/UART completion and the target WCET remain unestablished; TS-5
   `N22-TS5-83` stays a gap.
2. **Major — the published per-suite counts were not in the cited transcript.**
   The owner-delta record published counts that no retained log carried, which
   is the defect class FRICK-REVIEW-0415 had flagged. **Corrected in the
   follow-up round:** the counts now come from the retained post-review
   transcript.
3. **Major — the source-identity manifest covered only part of the changed
   tree**, so the published results were not bound to a reproducible source
   state. **Corrected in the follow-up round:** the manifest now covers every
   changed file in `src/`, `config/`, `tests/` and `verification/lean/` plus the
   build control files.
4. **Major — the TS-5 §3 gap mapping did not cover every gap row.** Rows 80,
   84, 85 and 86 were unmapped. **Corrected in the follow-up round:** §3 now
   maps every gap row, and the new case 87 is mapped as well.
5. **Minor — the "exactly one frame" wording exceeded the cited evidence.**
   **Corrected in the follow-up round:** the case now asserts that a second pop
   finds nothing.
6. **Minor — the whole-path oracle used a stub notifier,** so the production
   reporter's expired-deadline path was not exercised under a console stall.
   The production wiring is now covered by its own case, and the remaining
   target-path exposure is recorded as NC-EXT-185.
7. **Minor — the traceability note for SRS-REQ-074 was stale** and
   self-contradictory after the delta. **Corrected in the follow-up round.**
8. **Minor — the "no current-tree Lean result" statement was wrong.** The
   retained Lean evidence remains valid for the current tree: its 14 Lean
   source hashes re-verify against the current files and the recorded
   `Watchdog.olean` digest still matches, and no Lean source has changed
   since. **Corrected in the follow-up round:** the record now states the
   evidence as valid for the current tree while making clear that no Lean build
   was run in this cycle.
9. **Minor — the retained ctest transcript pointed at the wrong
   "authoritative" transcript,** and the post-review transcript carried no exit
   status for the formatter leg. **Corrected in the follow-up round.**
10. **Minor — the new time cases had no case identifier in any work package.**
    The tone holdover and UTC-step cases are now referenced from the TS-5
    time-scope statement, and their limits (host-only, no holdover accuracy
    claim) are stated.
11. **Minor — the panic path left the process `stderr` permanently
    non-blocking,** degrading later failure diagnostics. **Corrected in the
    follow-up round:** the console writer restores the descriptor mode.
12. **Comment — the pre-reset reporter is effective only on the UART and
    loopback backends;** the IP and TSN backends have no deadline-aware
    writer, so the report is dropped there. Recorded as a limitation.
13. **Comment — the S5 frame from the production wiring carries the degraded
    monotonic time base,** which the ICD does not yet describe. Recorded.

## Conclusion

No defect was found in the delta's implementation that would invalidate the
enumerated oracles. The findings above are confined to record currency, evidence
provenance and oracle completeness; the in-scope ones were corrected in the
follow-up round and the remainder are registered as NC-EXT-184..190.

**Confidence:** high for the reproduced counts, the CTest result and the TS-5
re-count. No host timing is reported as target WCET.
