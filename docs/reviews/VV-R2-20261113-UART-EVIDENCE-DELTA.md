# Independent R2/V&V Delta Review — UART evidence corrections

**Date:** 2026-11-13. **Disposition:** Local UART evidence corrections confirmed;
candidate-wide integrity remains open. This is not N10 tool qualification or
release approval.

From repository root, `sha256sum -c
docs/vv/evidence/2026-11-13-uart-restore-error/run-assets.sha256` passed for all
declared run assets. The source/header/test/CMake and executable identities match
the README; the retained test reports 104 checks, zero failures; full CTest is
57/57. `tool-qualification.md` rev 2.0 accurately binds the current test and
CMake hashes, records 104 checks, and keeps T3 **NOT FULLY QUALIFIED**.
`verification-results.md` and traceability mirror rev 0.56 record the current
result and scope. TS-2 rev 0.13 cases 84–86 cover the stated EAGAIN restore,
errno preservation, final-write deadline overrun, and expired zero-length
subcases with bounded host oracles.

**Remaining:** the older r4 `assets.sha256` is still a historical/stale
package-wide manifest and its attestation does not bind the UART-inclusive
candidate. Local run-assets integrity is not candidate integrity. No target
readiness/timing, full SRS-REQ-074 compliance, qualification or release claim.
