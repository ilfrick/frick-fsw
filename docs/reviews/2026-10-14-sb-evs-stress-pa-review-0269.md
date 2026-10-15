# FRICK-REVIEW-0269 — SB/EVS structural stress product-assurance delta

| Field | Value |
|---|---|
| Date | 2026-10-14 |
| Reviewer profile | PA — Senior Software Product Assurance Engineer (independent-context review) |
| Scope | `test_sb_stress`, CTest isolation, TS-6-54, verification-results claim, and related SB/EVS analysis references |
| Baseline | Working tree at `57974d2`; uncommitted |
| Result | **APPROVED WITH COMMENTS for this test/evidence delta only** |

## Review and correction record

The independent review iterations identified and the author corrected:

1. The original stress oracle did not distinguish earlier from newest full-queue
   log entries. The test now uses wildcard subscriptions and distinct message
   IDs (`0x0B00`–`0x0B02`) and asserts that the retained suffix is burst
   `0x0B01`, then `0x0B02`, with ordered application IDs.
2. The source comment understated cumulative EVS log records. It now accounts
   for 32 busy-lock events plus 96 full-queue events, totaling 128.
3. TS-6-54 is identified as an exploratory structural stress case, separately
   defined from covered and gap statuses and explicitly receiving no mission,
   requirement-coverage, timing, target, or ground-delivery credit.
4. The full-pipe and EVS-ring cases in `sb_evs_overload_analysis.md` were
   updated from stale “Pending” labels to current local test-spec coverage,
   while expressly retaining NC-EXT-75/76 and ground-delivery limitations.
5. The initial in-process stress test failed due PAL queue slots consumed by
   preceding tests; the test was moved into its own executable/process. Cause
   and correction are recorded in the evidence note and verification log.

## Evidence reviewed

- Fresh clean Debug build in `build/stress-clean`: 114 Ninja build steps.
- Full fresh CTest: **28/28 passed**; direct `test_sb_stress`: **454 checks,
  zero failures**.
- `make ci-local`: PASS (28/28 tests, Lean build, trace 117 requirements and
  51 Lean links); `make smoke`: PASS; `make renode`: PASS; targeted format and
  diff checks: PASS.
- Containerized Doxygen generation completed with existing repository-wide
  warnings. RTEMS cross-build and hosted CI were not run.
- Selected source, library, executable, and Docker-image IDs are retained in
  `docs/vv/evidence/2026-10-14-sb-evs-structural-stress.md`; they are not a
  complete immutable build manifest.

## Bounded assurance conclusion

The test supports the configured host-side structural boundary only: 32 held
queue locks cause 32 accounted drops/reports; 32 pipes at depth 32 accept 1,024
copies; three subsequent publishes cause 96 full-queue drops; EVS accepts and
dispatches 96 notifications across both phases, counts 32 notification losses,
and retains the newest 64 of 128 log records. No event-rate, WCET, target,
ground-forwarding, or SRS-REQ-072 claim is supported.

This is not overall PA approval, safety acceptance, qualification, release
approval, or closure of NC-EXT-75/76. Authority-owned event-envelope values and
end-to-end loss semantics/reporting remain open.
