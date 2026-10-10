# Verification and Validation Results

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-RES-001                       |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-10                             |
| Baseline cycle                    | v0.1.0 initial release                 |
| Norms                             | N6, N11 (records committed every cycle)|

This log is the running verification record. Every development/verification
cycle appends a row with the evidence produced; rows reference the committed
artifacts (no external state).

## Cycle log

| Cycle / date | Build (host) | ctest | Lean | trace | Smoke | Static | RTEMS cross | Comment |
| ------------ | ------------ | ----- | ---- | ----- | ----- | ------ | ----------- | ------- |
| v0.1.0 initial, 2026-10-10 | PASS | PASS (4/4) | PASS | PASS | PASS (6 apps) | clang-tidy clean | PASS (toolchain artifacts) | Baseline release; formal verification details below |
| v0.1.0 review closure, 2026-10-10 | PASS | PASS (4/4) | PASS (`lake build` 15/15 + 3/3 checks) | PASS (42 reqs, 20 theorems) | PASS (6 apps) | clean | proxy | Norm N15 re-run after review NCs (Lean proofs, S6 test, trace gate); see `docs/reviews/2026-10-10-*.md` |

## v0.1.0 baseline notes

### Formal verification (norm N1)

Lean 4 project under `verification/lean/` proves the requirements referenced by
the traceability manifest, including:

- CRC-16/CCITT-FALSE transmission check (frames extended by their CRC yield a
  zero CRC) - `Frick.Ccsds`.
- Software Bus delivery counting and queue-depth bound - `Frick.SoftwareBus`.
- PUS verification-count and TM counter monotonicity (mod 16384) -
  `Frick.Pus`, `Frick.Requirements`.
- CCSDS length arithmetic - `Frick.Requirements`.

verification/results: `make verify-lean` (lake build) is the gate; theorem-by-
theorem discharge is mirrored in the traceability manifest `lean:` links. In the
v0.1.0 review-closure cycle the project was built in the pinned Docker V&V
image: `lake build` compiled 15/15 modules and `lake exe frick_verify_checks`
reported 3/3 runtime checks PASS.

### Unit test results (methods Tst)

| Suite | Result | Coverage note |
| ----- | ------ | ------------- |
| test_ccsds | PASS | packet headers, length fields, CRC detect/append |
| test_sb    | PASS | exact and wildcard subscriptions, FIFO, depth |
| test_tbl   | PASS | load/validate/activate, atomic activation |
| test_pus   | PASS | dispatch, S1 verification packets, S17 ping, S6 region bound |

### System demonstration (method Dem)

Native simulator boots with 6 applications (scheduler, command ingest, housekeeping,
health & safety, sample telemetry, TM output); EVS events flow to the PUS
service 5 telemetry stream on the loopback link. Ground tool `ff_gsw.py` can
inject a ping and decode the returned service 17 telemetry.

### Traceability (norms N3/N4)

`make trace` exits 0: 21 SRS, 15 SSS, 6 GAP items, every id present in the
documents, every srs -> sss parent resolved, every Lean link resolved, every
`ci:*` evidence claim declared in a workflow.

### Open non-conformances (norm N12)

None open at the v0.1.0 baseline. Class-A gaps (GAP-A1..A6) are tracked in
`docs/quality/class-a-gaps.md` and are not NCs for the class-B baseline.

## Review evidence (norm N9/N11)

| Review | Profile | Date | Conclusion | Report |
| ------ | ------- | ---- | ---------- | ------ |
| Baseline V&V result assessment | R2 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-baseline-vv-review.md |
| SSS requirements review (norm N14) | R3 (independent) | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-sss-systems-review.md |
| Norms N1-N16 conflict analysis | R1+R3 | 2026-10-10 | APPROVED WITH COMMENTS | docs/reviews/2026-10-10-rules-conflict-review.md |

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | v0.1.0 initial V&V record |
| 0.1.1 | 2026-10-10 | frick-fsw | Review-closure cycle: Lean build fixed and verified, S6 test corrected, trace gate hardened |