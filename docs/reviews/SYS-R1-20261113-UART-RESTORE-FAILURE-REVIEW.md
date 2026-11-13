# Independent R1/FSW Review — UART restore/deadline correction

**Date:** 2026-11-13. **Scope:** first-pass review of the uncommitted UART
`send_until()` correction, API contract and N22-TS2-84. This is not qualification,
release approval, or NC closure.

**Conclusion:** no defect found in the restore-failure branch. The code checks
restoration before processing `write()` results, stops with `FF_ERROR`, and
preserves errno across `fcntl()`. Focused CTest `link_uart` passed 1/1 in the
review snapshot.

**Findings:** R1-01 Minor—`traceability.yaml` overstates shared-descriptor
concurrency as resolved despite a failed restore leaving `O_NONBLOCK`; narrow the
claim and keep NC-EXT-191 open. R1-02 Minor—scope N22-TS2-84 as host API-error
subcase evidence, not SRS-REQ-074 compliance. R1-03 Minor—no-retry wording was
supported indirectly by absence of transmitted bytes, not a write-attempt
counter; narrow wording or count attempts.

**Limits:** no target readiness/timing or full panic-path evidence. Restore
failure may leave the descriptor mode changed. A later exact-candidate re-review
is required after the N22-TS2-85 deadline correction and documentation updates.
