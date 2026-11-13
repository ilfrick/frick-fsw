# Independent PA/N20 Substantive Review — UART deadline-writer correction

**Date:** 2026-11-13. **Scope:** current UART implementation/API/test and related
records in the combined S1 candidate at baseline `e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83`.
This is not package-manifest attestation, risk acceptance, release/flight
approval, or NC closure.

**Scoped substantive outcome:** no new functional defect or traceability/oracle
overclaim found. The prior restore-result, injection, deadline post-check,
expired zero-length, evidence-count/hash and scope-attribution findings are
substantively corrected. The reviewer independently ran the retained UART test:
104 checks, 0 failures. Local run-assets checksums verify.

The reviewed cases are narrowly bounded host API evidence: N22-TS2-84 tests
EAGAIN + restore failure/no retry and successful-restore errno preservation;
N22-TS2-85 tests final-write deadline overrun while confirming bytes were
emitted; N22-TS2-86 tests expired zero-length rejection with no write. They do
not establish full SRS-REQ-074 compliance, target timing/readiness, WCET or HIL.
T3 remains **NOT FULLY QUALIFIED**.

**Residuals:** NC-EXT-191 remains OPEN because a concurrent writer may observe
temporary nonblocking mode and failed restoration may leave the shared
descriptor nonblocking; target readiness and the 250 ms bound remain
unverified. S1 residuals including NC-EXT-240..244 remain open as recorded.
The package-wide r4 manifest is historical/stale for the combined candidate;
fresh complete inventory and independent verification are a separate pending
CM/evidence-binding task. Prior r4 attestations do not bind this candidate.
