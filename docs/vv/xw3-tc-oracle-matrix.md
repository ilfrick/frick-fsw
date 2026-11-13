# XW-3 — external TC/TM requirement-to-oracle matrix

|                        |                                                                 |
| ---------------------- | --------------------------------------------------------------- |
| Document id            | FRICK-VV-XW3-001                                                |
| Revision               | 0.3                                                             |
| Date                   | 2026-10-26                                                      |
| Scope                  | Increment XW-3: real external telecommand on the QEMU serial input of the accepted XW-2 **test-only** image, unchanged middleware, captured telemetry verified by an independent oracle |
| Controlling clauses    | `docs/interfaces/ICD.md` §3 (packet/field layout) and §4 (implemented services); `docs/requirements/SRS.md` SRS-REQ-004/005/006/007/012/017 |
| Implementation         | `tools/scripts/qemu_xw3_vv.sh`, `tools/scripts/xw3_verdict.py`, `tools/scripts/xw3_manifest.json` |
| Negative controls      | `tools/scripts/xw3_negative_controls.py`                        |

Reproduction: `bash tools/scripts/qemu_xw3_vv.sh` writes the golden frame on
the QEMU serial input and verifies the captured TM; `python3
tools/scripts/xw3_negative_controls.py --run-dir <run>` runs the 15 controls.

## 1. What this matrix is

It states, clause by clause, **what the XW-3 oracle asserts** and **what it
deliberately does not assert**. Everything asserted is derived from the
controlled interface and the SRS; nothing is derived from the captured bytes or
from the flight C implementation. A behaviour the ICD does not fix is an
**observation**, not an assertion, and is listed as a gap.

The matrix is deliberately minimal. It is not a PUS-C profile compliance claim,
not a mission-profile claim, and not an all-service claim: it covers the single
registered service pair used as the external stimulus (S17.1 → S17.2) and the
Service 1 acknowledgement set that the framework produces for it.

## 2. Wire contract used by the oracle (ICD §3)

| Item | Value used by the oracle | ICD §3 basis |
| ---- | ------------------------ | ------------ |
| TC primary header | 6 octets; version 0, type 1, secondary-header flag 1, APID `0x010` | §3 diagram |
| PDL | encoded length minus one; total packet = 6 + PDL + 1 | §3 |
| TC secondary header | 5 octets: version/ack flags, service, subservice, source id (2 octets, big-endian) | §3 table |
| TC user data | octets after the 5-octet secondary header, before the 2-octet CRC | §3 |
| CRC | CRC-16/CCITT-FALSE (poly `0x1021`, init `0xFFFF`, no reflection) over the bytes before the trailer, compared with the transmitted trailer (not a zero-residue check) | §3 |
| TM primary header | 6 octets; version 0, type 0, secondary-header flag 1, APID `0x020` | §3 |
| TM secondary header | 13 octets: version, service, subservice, 14-bit counter (big-endian), destination id (2), coarse time (4), fine time (2) | §3 table |
| TM user data | octets after the 13-octet secondary header, before the CRC | §3 |

The oracle implements all of the above itself (`tools/scripts/xw3_verdict.py`).
It does not import or link any part of the flight codec, and it does not read
the C sources to decide what to expect.

## 3. Asserted behaviours

Stimulus for every asserted case: the golden telecommand
`18 10 C0 00 00 0A 2B 11 01 01 02 01 02 03 04 F7 2B`
(APID `0x010`, service 17, subservice 1, source id `0x0102`, user data
`01 02 03 04`), written to QEMU's serial input. It crosses the serial device
and is read by the unchanged `link_uart` backend from `/dev/console`, extracted
into a CCSDS frame and dispatched by the unchanged `CMD_INGEST` and PUS path.

| ID | Clause | Behaviour asserted | Oracle check | Where |
| -- | ------ | ------------------ | ------------ | ----- |
| M-1 | SRS-REQ-012 | A connection-test telecommand is answered with the same data on service 17 subservice 2 | exactly one `S17.2` packet exists and its payload equals the **injected TC user data** octet for octet | `S17.2-COUNT`, `S17.2-PAYLOAD` |
| M-2 | SRS-REQ-006 | Every acknowledged stage of an accepted telecommand produces exactly one Service 1 packet | exactly one `S1.1`, one `S1.3` and one `S1.7`; no other Service 1 subservice (the stimulus enables acceptance, start and completion acknowledgement) | `S1.1-COUNT`, `S1.3-COUNT`, `S1.7-COUNT`, `S1-SET` |
| M-3 | ICD §3/§4 transport provenance (interface observation; no SRS assertion) | The capture comes from the unchanged command-ingest application on the UART backend, not from an in-image loopback | the console carries the ingest application's own backend announcement (`CI app listening on link 'uart'`) | `LINK-UART` |
| M-4 | SRS-REQ-004 | Every TM frame carries a CRC-16/CCITT-FALSE trailer and a frame whose CRC does not match is not accepted | every accepted packet's CRC verifies under the oracle's own implementation; a TM-looking packet with a bad CRC fails the run | `TM-CRC` |
| M-5 | SRS-REQ-017 / ICD §3 | APID `0x020`, CCSDS version 0, type 0, secondary-header flag 1, self-consistent PDL (total = 6 + PDL + 1) | header fields and length are checked at fixed offsets for every accepted packet | `TM-HEADER` |
| M-6 | SRS-REQ-007 | The TM counter strictly increases and wraps modulo 16384 | every consecutive step is a positive modular difference bounded by half the modulus (a decreasing counter is rejected; a legitimate wrap is accepted) | `TM-COUNTER` |
| M-7 | SRS-REQ-004/005 | A frame with a corrupted CRC is not executed | injected frame's CRC is verified **invalid** by the oracle (stimulus self-check); no `S17.2`; no `S1.1`/`S1.3`/`S1.7`; the capture still carries CRC-valid TM so the serial path was live | `TC-CRC-INVALID`, `NO-S17.2`, `NO-S1.1`, `NO-S1.3`, `NO-S1.7`, `CAPTURE-LIVE` |
| M-8 | SRS-REQ-004 | The accepted stimulus itself carries a valid CRC | the injected golden frame's CRC verifies under the oracle's own implementation, so a response set is only attributed to a frame the controlled interface accepts | `TC-CRC-VALID` |

The **differential control** that ties the responses to the external stimulus is
`XW3-A1` versus `XW3-R1`: the same image, the same run length, the same injection
instant, only the CRC trailer of the injected frame differs. `XW3-A1` produces
the response set; `XW3-R1` produces none while its capture is still live. A
test-internal call path that bypassed the serial input could not depend on the
frames' CRC trailer.

`M-2` asserts the response **set shape** for a valid accepted command (exactly
one Service 1 packet per acknowledged stage, SRS-REQ-006); it does not assert
what `S1.8` means for a *rejected* frame (that is O-3 and gap XW3-G1).

`M-1` correlates by packet fields — APID, service, subservice and the user data
taken from the stimulus — never by position in the capture and never by
comparing the capture with a copy of itself. `M-2` counts Service 1 packets by
service/subservice only.

## 4. Observations (recorded, not asserted)

| ID | Behaviour | Why not asserted | Recorded as |
| -- | --------- | ---------------- | ----------- |
| O-1 | Duplicate telecommand (same frame injected twice, 2 s apart) | the ICD does not fix duplicate-command response behaviour | `S17.2` and `S1.1` counts in the case verdict (`XW3-O1`) |
| O-2 | Frame split across two writes (8 octets, then the remaining 9) | the ICD does not fix link-layer reassembly | `S17.2` count and payload in the case verdict (`XW3-O2`) |
| O-3 | Presence/absence of `S1.8` for a rejected (bad-CRC) frame | SRS-REQ-005's Service 1 completion-failure wording conflicts with the current event-only behaviour; open under **NC-EXT-04** | `s1_8_reported` in every verdict |
| O-4 | Numeric Service 1 payload layout (TC type/subtype, service, subservice, error code) | authority-pending (**NC-EXT-14/15**) | `s1_payload_prefixes` in every verdict |
| O-5 | TM destination id, coarse/fine time per packet | not fixed for this test image; no time assertion is possible on an emulated target | `destination_ids` in every verdict |

## 5. Gaps (no assertion, tracked)

| Gap | Subject | Reason |
| --- | ------- | ------ |
| XW3-G1 | `S1.8` reporting for a malformed/rejected frame | NC-EXT-04 open: the requirement and the implemented behaviour are not reconciled by an authorized disposition |
| XW3-G2 | Numeric `S1` payload layout | NC-EXT-14/15 authority-pending |
| XW3-G3 | Duplicate-command response behaviour | not fixed by the ICD |
| XW3-G4 | Frame reassembly across writes | not fixed by the ICD |
| XW3-G5 | `TC[3,128]` 6-octet create form and other authority-pending service forms | NC-EXT-17/ESC-HK-01: provisional extension whose payload width is authority-pending, not exercised |

## 6. What this evidence is not

Real octets on the serial input of a **test-only emulated-target image** whose
watchdog device seam is substituted by the validated model. It is **not** a
device result, **not** HIL, **not** N18, **not** all-I/O coverage, **not** a
timing/WCET result, **not** a mission-profile result, and **not** a reset,
electrical-independence or airworthiness result. `QEMU virt` has no PolarFire
SoC peripherals. NC-PAL-01 and NC-EXT-198 remain **OPEN**.

## 7. History

| Rev | Date | Change |
| --- | ---- | ------ |
| 0.1 | 2026-10-26 | Initial requirement-to-oracle matrix (M-1…M-7, O-1…O-5, XW3-G1…G5). |
| 0.2 | 2026-10-26 | Add M-8: the injected golden frame's own CRC is asserted valid in accept/observe mode (the reject case already asserted its corrupt CRC invalid). Fail-closed identity metadata now requires the recorded capture size. No assertion was removed or weakened. |
| 0.3 | 2026-10-26 | Clause-attribution correction from the finishing R4 review: M-3 is ICD §3/§4 transport provenance (not an SRS-REQ-017 assertion), M-5 now carries the SRS-REQ-017/ICD §3 packet-format attribution, M-7 cites SRS-REQ-004/005, and the header lists SRS-REQ-017. No asserted behaviour or oracle check changed. |
