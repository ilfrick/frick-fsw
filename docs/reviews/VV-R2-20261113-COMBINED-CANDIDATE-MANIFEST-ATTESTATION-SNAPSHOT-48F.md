# R2/V&V Manifest Check Attestation — Snapshot 48f

**Date:** 2026-11-13. **Status:** historical checksum/inventory evidence only;
not current package attestation. The manifest digest was subsequently invalidated
by covered documentation/review changes.

At the inspected snapshot, active `r4/assets.sha256` SHA-256 was
`48f858ecdb6cdbc205c1011e56a4054a52d76016c1d7e7b03d66893505acb448` with **292
entries**. Independent inventory reconstruction found 293 declared paths (137
regular files under r4 plus 156 outside-list paths), with the manifest excluded.
The path set matched: 0 missing, 0 extra, 0 duplicates; no symlinks, special
files, or build paths were found. `sha256sum -c` from repository root returned
**292/292 OK, exit 0**. Stdout SHA-256 was
`655bbbbf96f21c44ad014de8f96426d195482eeac852f3130d58c5b078e311d6`.

All 18 recorded tracked identity hashes matched the inspected snapshot, and the
candidate patch matched the baseline diff at HEAD
`e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83`; patch SHA-256 was
`ad966dab09c45819f260df67877009bfcfca103e2d4aa3c13a9eae90904a4ce8`.

This is not an attestation for a later digest or final Git tree, and does not
qualify tools, accept product, close NCs, or approve release/flight.
