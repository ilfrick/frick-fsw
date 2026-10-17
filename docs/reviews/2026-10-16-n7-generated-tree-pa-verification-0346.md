# Bounded PA verification 0346 — generated OBCP flat-source tree

**Date:** 2026-10-16  
**Scope:** NC-EXT-84 observed generated-tree identity only.  
**Conclusion:** The per-file list and aggregate hash match the current observed tree; generator/tool provenance remains OPEN. No N20 phase or release approval.

## Evidence and result

`docs/vv/evidence/2026-10-16-n7-generated-flat-file-hashes.txt` contains 221
unique paths. `build/ci/n7s-obcp/flat` contains the same 221 unique paths. A fresh
GNU `sha256sum` listing generated with the command in the candidate manifest was
byte-for-byte equal to the retained list. Both have SHA-256
`dd9666ad5cb09a9ffd71e7f3d17a86122f1dcc0211a53de9264d2ab9d9a8e822`.

**Disposition:** observed generated-tree identity is VERIFIED for this current
tree/list. This does not establish immutable historical identity, reproducible
generation, generator/tool version provenance or tool qualification. NC-EXT-84
remains OPEN for those residuals. Confidence is high for the current filesystem
identity and limited to the files observed at review time.
