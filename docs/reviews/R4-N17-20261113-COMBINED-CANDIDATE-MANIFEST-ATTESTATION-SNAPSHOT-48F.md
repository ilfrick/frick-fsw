# R4/N17 Inventory Check — Snapshot 48f

**Date:** 2026-11-13. **Status:** historical snapshot evidence, not current
package attestation or aggregate N17 approval.

For manifest digest `48f858ecdb6cdbc205c1011e56a4054a52d76016c1d7e7b03d66893505acb448`,
R4 independently observed 293 declared paths (137 regular files under r4 and
156 outside paths), with 292 manifest entries. The path set matched with no
missing/extra/duplicate entries, symlinks, special files, or build paths.
`sha256sum -c` returned exit 0, 292/292 OK. The three current combined-candidate
attestation paths were detached; older S1-named reports were included as history.
The candidate identity/patch matched baseline HEAD
`e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83` and 18 tracked paths.

R4 subsequently found R4-AUDIT-01: loop-audit §12.3 had not distinguished these
review-snapshot results from the latest package state. The correction was
re-reviewed as resolved, but invalidated this manifest snapshot. Do not use this
report as current candidate attestation; a fresh digest and three matching
post-manifest reports are required. No qualification, NC closure, release, or
flight approval is implied.
