# Independent R4/N17 Review — Loop-Audit Manifest Status Finding

**Date:** 2026-11-13. **Disposition:** changes required for the current-status
summary; the checksum and inventory pass for digest
`48f858ecdb6cdbc205c1011e56a4054a52d76016c1d7e7b03d66893505acb448` was
independently observed, but the reserved attestation report files still held
stale `108e…` content at review time. This is not a current package attestation.

**Finding R4-AUDIT-01 (Moderate):** `docs/status/loop-audit.md` §12.3 said R2
confirmed only local run-asset integrity and R4 confirmed documentation while
noting the historical manifest, without identifying those statements as earlier
review-snapshot results. At review time the active manifest check was 292/292,
but current reports were not yet bound to its digest; the text therefore
conflated completed prior review outcomes with the current package state.

**Required correction:** make §12.3 point to the active manifest plus the three
reports whose digests match it; explicitly treat earlier review and manifest
digests as historical. Do not claim package integrity from any stale report.

**Scope limits:** no aggregate N17 approval, qualification, release/flight
approval, or NC closure. The final regenerated manifest must be independently
re-attested.
