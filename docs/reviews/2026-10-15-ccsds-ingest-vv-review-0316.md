# CCSDS boundary and command-ingest review (FRICK-REVIEW-0316)

| Field | Value |
| --- | --- |
| Profile | Independent SW V&V review |
| Date | 2026-10-15 |
| Scope | NC-EXT-01 TC/TM boundary tests; NC-EXT-02 production command-ingest receive capacity; N22-TS1-9/96 |
| Baseline | Source HEAD `13ebf92` plus uncommitted app/test changes |
| Verdict | **ACCEPTED for bounded host codec and receive-helper scope** |

## Evidence reviewed

- TC and TM tests reject minimum-minus-one PDL, PDL `0xFFFA`, PDL `0xFFFF`,
  truncation, and trailing bytes. For malformed PDL, all decoder output
  parameters (APID, secondary header, data pointer, data length) remain unchanged.
- `FF_AppCmdIngestReceiveOne` owns the production `FF_CCSDS_MAX_TC_PACKET`
  buffer, invokes `FF_LinkRecv`, and dispatches accepted input to `FF_PusIngest`;
  `FF_AppCmdIngestMain` uses this helper in its loop.
- The controlled-link host test encodes a 1037-octet maximum TC, asserts the
  receive capacity is exactly 1037, and checks all 1023 service-handler argument
  octets byte-for-byte.
- Fresh focused Debug/Ninja build and `r2_gap` CTest passed; direct execution:
  613 checks, 0 failures.

An earlier attempt using the existing `build/ci` cache failed because it referred
to stale `/workspace` paths and encountered a permission error. It is not counted
as a pass; the fresh focused build is the successful evidence.

## Disposition and limits

This supports closure of NC-EXT-01 for the enumerated decoder boundary/arithmetic
behaviors and NC-EXT-02 for the host maximum-buffer/receive-helper behavior, if
the project NC owner records that bounded scope. It is not target or physical-
hardware verification and does not exercise the infinite app loop, a real link
backend, RTEMS stack adequacy, or timing. The link backend remains responsible
for honoring its `cap` contract.
