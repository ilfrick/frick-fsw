# Independent R4/N17 Delta Review — UART records

**Date:** 2026-11-13. This is an independent documentation delta review, not
package attestation, N17 approval, or release approval.

**Confirmed:** status.json now describes current 57/57 CTest, 104 focused UART
checks, full-format NC-EXT-244, T3 NOT FULLY QUALIFIED, and pending exact-candidate
reviews/package binding; README metrics date is current. Traceability mirror rev
0.56 includes cases 84–86 and bounds them as API-only. Tool register rev 2.0
records the current hashes and 104 checks without upgrading qualification.
Loop-audit header/history agree and identify the actual HEAD and historical S1
manifest state. Sprint erratum applies to revs 1.135 and 1.136 and preserves
shared-descriptor residuals. Handover points to traceability rev 0.56 and no
longer lists its correction as pending. TS-2 totals reconcile: 86 cases, 44
nominal, 40 OOB, 2 mixed; 48 covered, 5 host covered, 32 gap, 1 characterization.
Local UART run-assets checksums pass for the declared files.

**Remaining:** the r4 package-wide manifest still binds a historical snapshot;
its old attestations are not current for this UART-inclusive candidate. This
finding must be closed by a fresh complete inventory/manifest and independent
attestation, not by the local run-assets list.
