# Combined S8-PUS-FU S1 and UART deadline-writer candidate evidence

## Current snapshot and status

This r4 directory now records the combined uncommitted candidate at baseline
`e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83`, including the S1 ingestion-failure
host tests and the UART `FF_Link_t::send_until` restore/deadline correction. It
is a working candidate record, not a release package, qualification, risk
acceptance, or flight approval. The previous 175-entry S1 inventory and its PA,
R2 and R4 attestations are historical snapshot evidence only; the prior active
manifest is preserved as `assets-historical-pre-uart-20261113.sha256`.
The active `assets.sha256` is generated from the combined candidate inventory;
its exact path set and checksum result are covered by the three excluded
post-manifest reports registered in `docs/reviews/README.md`.

The updated candidate identity is `candidate-identity.txt`; the complete tracked
delta is `candidate-working-tree.patch`. Its modified tracked files are hashed
in the identity record. `assets-outside-paths.txt` enumerates untracked review and
evidence files outside this r4 directory that are included in the package. The
unrelated `.opencode/plugins/`, `opencode.json`, and
`tests/unit/test_sched_events.c.bak` paths remain unmodified and excluded.

## Verification evidence

- Combined native build and CTest: PASS, **57/57**; focused `test_pus`: 322
  checks, 0 failures; 55 distinct compiled CTest binaries and ordered invocation
  inventory retained under `artifacts/`.
- UART focused PTY test: **104 checks, 0 failures**; source/header/test/CMake and
  test executable hashes, all focused/full logs, and raw Renode captures are in
  `docs/vv/evidence/2026-11-13-uart-restore-error/`. Its local `run-assets.sha256`
  verifies only that sub-package, not the whole candidate.
- Lean, trace (120 requirements, 76 SRS, 30 SSS, 14 gaps, 68 Lean theorems),
  GSW/license, isolated simulator smoke, Doxygen, metrics, RTEMS 6.2 cross-build,
  and Renode N18 all PASS for the latest recorded cycle.
- Changed-file pinned clang-format 18 passes. Full pinned format check remains
  blocked by three unchanged initializers in `link_tsn.c`, `link_ip.c`, and
  `test_link_tsn.c` (NC-EXT-244; deferred under N21).
- Renode is emulator-only evidence, not physical target, HIL, timing/WCET,
  qualification, or flight evidence. T3 remains NOT FULLY QUALIFIED.

The UART API subcases do not establish full SRS-REQ-074 compliance. NC-EXT-191
remains OPEN: shared-descriptor effects can occur during the temporary mode
window, and a failed restore may leave the descriptor nonblocking; RTEMS target
readiness and the target 250 ms bound remain unverified. NC-EXT-240..244 and all
authority-owned decisions remain open as recorded in the sprint plan and
verification results. No NC, sprint, release or flight decision is closed.

## Independent substantive reviews

The current bounded source/API and systems reviews R1/R3, V&V R2, documentation
R4, and PA/N20 substantive review are retained in `docs/reviews/` and indexed in
`docs/reviews/README.md`. Their scopes are explicit; they do not qualify T3 or
close NC-EXT-191. The three final inventory/check reports below are separate
post-manifest attestations, not substitutes for those substantive reviews.

## Current combined-candidate manifest protocol

Inventory rule: every regular file recursively under this r4 directory plus every
regular path in `assets-outside-paths.txt`, using repository-root-relative
paths. Reject symlinks/special files, missing/nonregular explicit paths,
duplicates, and any included path under `build/`. No submodules/gitlinks are
configured. The list itself, candidate identity/patch, this README, all retained
logs and prior manifests are included.

Only exclusions: this directory's `assets.sha256` itself and these exact
post-manifest attestation reports, pre-registered in `docs/reviews/README.md`:

- `docs/reviews/PA-N20-20261113-COMBINED-CANDIDATE-MANIFEST-ATTESTATION.md`
- `docs/reviews/VV-R2-20261113-COMBINED-CANDIDATE-MANIFEST-ATTESTATION.md`
- `docs/reviews/R4-N17-20261113-COMBINED-CANDIDATE-MANIFEST-ATTESTATION.md`

The older pre-UART `assets.sha256` and its earlier attestations are included as
historical evidence, not excluded. The `assets-final-generation.log` and
`assets-preview*` records document prior snapshots/attempts only; the current
combined freeze details are in `assets-combined-final-generation.log`.

The final manifest generator must enumerate and sort the declared inventory,
verify exact path-set agreement, generate SHA-256 entries, run
`sha256sum -c <r4>/assets.sha256` from repository root, and retain the complete
output. Any covered-byte change invalidates the manifest and requires a new
freeze and PA/R2/R4 attestation. The attestations must identify the exact same
manifest digest, inventory/exclusion review, and successful checksum result.
The final Git commit tree is intended to bind the manifest and three reports
that are excluded from it. Package-wide integrity is established only when all
three reports identify the same manifest digest and independently confirm the
inventory and full checksum result; previous S1 attestations do not apply. The
review index and these reports are the current state authority.
