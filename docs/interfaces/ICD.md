# Interface Control Document (ICD) - TM/TC Ground Interface

| Document id | FRICK-ICD-001 |
| ----------- | ------------- |
| Revision    | 1.2 |
| Date        | 2026-10-16 |
| Profile     | PUS-C (ECSS-E-ST-70-41C) over CCSDS Space Packets (CCSDS 133.0-B-2) |

## 1. Scope

This ICD documents the implemented space-to-ground and ground-to-space packet
interface as an internal reference profile for SRS-REQ-017/SSS-REQ-014. It is
not an approved mission tailoring baseline; applicable profile values and
acceptance rules require approval by the authorized mission/customer authority.
In particular, unresolved sequence-count and primary-header acceptance policies
remain subject to NC-EXT-06 and NC-EXT-13.

Board-level peripheral interfaces (Ethernet/MAC, CAN, RS-422/485, SPI, I2C,
PCIe, GPIO, SpaceWire) and their register/address maps are **not** part of this
TM/TC ICD: their contracts live in the HAL headers (`src/hal/`) and the
reference driver headers, and their platform addresses in
`config/platform/platform_mpfs.h`.

## 2. Configuration summary (from `config/fsw_config.h`)

| Parameter | Value |
| --------- | ----- |
| PUS version | 2 (PUS-C) |
| Mission identifier | `0x01` |
| TC APID | `0x010` |
| TM APID | `0x020` |
| Packet CRC | CRC-16/CCITT-FALSE, trailer appended (enable `FF_FRAME_WITH_CRC=1`) |
| Max TM / TC PUS payload | `FF_PUS_MAX_TM_FRAME` / `FF_PUS_MAX_TC_FRAME` (1024 octets); complete CCSDS packet capacities also include the primary header, PUS secondary header, and configured CRC trailer (`FF_CCSDS_MAX_TM_PACKET` / `FF_CCSDS_MAX_TC_PACKET`; 1037 octets for TC with the current configuration) |
| In/out-of-band transport | UART byte stream (target `/dev/console`, 115200 baud), framed by CCSDS primary-header PDL into one complete TC packet per receive call; partial packets persist across timeout and coalesced packets are returned one per call. Reference timeout uses termios VTIME in 100 ms units (rounded up), so requested short timeouts can overrun by <100 ms plus scheduling/driver latency; a single receive owner is required. A zero-byte read substantially before the configured VTIME chunk is treated as `FF_ERROR` (host disconnect case); RTEMS runtime semantics are not measured. A plausible in-range but malformed PDL candidate can retain and eventually consume bytes that include a following valid TC. No inner-boundary recovery is guessed because no independent sync delimiter is defined; authority must decide timeout/partial-discard/recovery policy before mission use. Loopback on host simulator. These are internal software reference behaviors pending approved profile/authority decisions. |

## 3. TC/TM packet field layout (CCSDS Space Packet + PUS secondary header)

```
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |version| type |  sec-hdr flag |   APID (TC 0x010, TM 0x020)    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |  seq flags | sequence count (currently encoded as zero)       |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |        packet data length field (PDL, 16 bit)                 |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    | packet data: PUS secondary header + user data + optional CRC   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |             (CRC trailer is part of packet data)                |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  ```

  The diagram is schematic. The packet-data length counts every octet after
  the six-octet primary header, including the optional CRC trailer; its
  encoded value is that count minus one. Sequence-count fields are currently
  emitted as zero by both TC and TM encoders pending the controlled profile
  decision (NC-EXT-06); no per-type counter behavior is specified here.

- TC secondary-header byte layout (5 octets, `FF_PUS_TC_SEC_HDR_LEN`):

  | Secondary-header byte | Content |
  | --------------------- | ------- |
  | 0 | PUS version in high nibble; acknowledgement flags in low nibble |
  | 1 | Service |
  | 2 | Subservice |
  | 3–4 | Source/application identifier, big-endian |

- TM has a distinct 13-octet secondary header (`FF_PUS_TM_SEC_HDR_LEN`):

  | Secondary-header byte(s) | Content |
  | ------------------------- | ------- |
  | 0 | PUS version in high nibble |
  | 1 | Service |
  | 2 | Subservice |
  | 3–4 | Packet counter, big-endian |
  | 5–6 | Destination identifier, big-endian |
  | 7–10 | Coarse time, big-endian |
  | 11–12 | Fine time, big-endian |

  This is not the TC header with a time field appended.
- PDL is the CCSDS packet-data length minus one. Let `crc_len` be 2 when
  `FF_FRAME_WITH_CRC` is enabled and 0 otherwise. For TC,
  `PDL = 5 + user_data_len + crc_len - 1`; for TM,
  `PDL = 13 + user_data_len + crc_len - 1`. Both are equivalently total packet
  length minus 7. The 6-octet primary header is not included in PDL.
- When `FF_FRAME_WITH_CRC` is enabled, the encoder computes CRC-16/CCITT-FALSE
  (poly `0x1021`, init `0xFFFF`, no reflection) over the packet bytes preceding
  the two-octet CRC trailer (`total - 2` bytes); it does not include the trailer
  in that calculation. The decoder independently computes the same CRC over
  bytes before the trailer and compares it with the transmitted two-octet
  value; it does not use a zero-residue check (SRS-REQ-004).

## 4. Implemented PUS services and subservices (open backend)

| Service | Subservice | Name | TC/TM | Data content |
| ------- | ---------- | ---- | ----- | ------------ |
| 1  | 1 | acceptance success | TM | TC type/subtype, source id, errcode 0 |
| 1  | 2 | acceptance failure | TM | errcode |
| 1  | 3 / 5 | start / progress success | TM | errcode 0 |
| 1  | 4 / 6 | start / progress failure | TM | errcode |
| 1  | 7 | completion success | TM | errcode 0 |
| 1  | 8 | completion failure | TM | errcode |
| 3  | 25 | housekeeping report | TM | report id + data |
| 3  | (admin) | create/delete/enable/disable | TC | report parameters |
| 5  | - | event report | TM | event id[@2], type, text |
| 6  | 5 / 6 | memory dump (TC 6,5 -> TM 6,6) | TC/TM | addr[4, big-endian] + size[2, big-endian]; bounded to registered regions |
| 8  | - | function call | TC (dispatch) | function id |
| 9  | 1 / 2 | time set / CUC report | TC/TM | 6-octet CUC T-field (P-field implicit; section 5) |
| 17 | 1 / 2 | test (ping / pong) | TC/TM | echo of the TC user data |
| 20 | - | parameter report | TM | table-backed parameters |

Verification error codes: `FF_PUS_FAIL_*` (unknown service/subservice,
illegal APID, execution failure).

## 5. Time code (CUC, CCSDS 301.0-B-4)

On-board time is a **CCSDS Unsegmented Time Code (CUC)** tailored to a 4-octet
coarse + 2-octet fine T-field (agency-defined epoch). Two exchange forms are
defined (R3 review NC-1):

1. **Reference on-wire T-field — 6 octets, P-field not transmitted by this
   implementation.** This representation is used inside the TM secondary
   header and PUS service 9 payloads here; mission acceptance of the reference
   tailoring remains subject to authority approval:

   | Field | Bytes | Value |
   | ----- | ----- | ----- |
   | coarse | 4 | whole seconds big-endian, `utc_ns / 1e9 + leap_seconds` |
   | fine | 2 | fraction of a second in units of 2^-16 s big-endian |

    In the TM secondary header, relative offsets 7..12 contain the T-field
    (coarse 7..10, fine 11..12). Since the primary header is 6 octets, the
    corresponding absolute packet offsets are 13..18 (coarse 13..16, fine
    17..18). The TM secondary header is 13 bytes (`FF_PUS_TM_SEC_HDR_LEN`).
    PUS service 9,1 (set time) accepts exactly this
   6-octet T-field; any other length is rejected with `FF_BAD_SIZE`, so a
   leading P byte can never be mis-parsed as the coarse MSB (SRS-REQ-024).

2. **Explicit self-describing form — 7 octets (P-field + T-field).** Produced
   and consumed only by `FF_TimeCucEncode`/`FF_TimeCucDecode` (`FF_TIME_CUC_LEN`
   = 7) for self-describing interchange:

   ```
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |   P-field (0x2E)  |  coarse[4], big-endian                    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                    coarse[4]    |        fine[2], big-endian  |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   ```

   | Field | Bytes | Value |
   | ----- | ----- | ----- |
   | P-field | 1 | `0x2E`: unsegmented, agency-defined epoch, coarse=4 (`coarse-1`=3), fine=2 |
   | coarse | 4 | whole seconds big-endian, `utc_ns / 1e9 + leap_seconds` |
   | fine | 2 | fraction of a second in units of 2^-16 s big-endian |

   A wrong-length code is rejected with `FF_BAD_SIZE` and an unknown P-field
   with `FF_PARSE_ERROR` (SRS-REQ-024).

- PUS service 9,2 (report time) returns the 6-octet on-wire T-field.
- The time base (source selection, 1 Hz tone discipline, leap-at-boundary) is
  described in `docs/architecture/time-and-timecode.md` (SSS-REQ-016,
  SRS-REQ-021..024).

## 6. Mission tailoring

Tailoring is a documented change to `config/fsw_config.h` plus this document,
traced through the manifest (SRS-REQ-017, SSS-REQ-014). Release review of the
ICD is performed by the R1/R3 reviewer profiles when the profile changes.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial ground interface baseline |
| 0.2 | 2026-10-10 | frick-fsw | Milestone M2: CUC P-field/T-field layout, time base reference |
| 0.3 | 2026-10-10 | frick-fsw | R3 review NC-1: distinguish PUS-C 6-octet on-wire T-field from explicit 7-octet P+T CUC; correct set-time rejection codes |
| 0.4 | 2026-10-12 | frick-fsw | Scope note: board-level peripheral interfaces and their register maps are not part of the TM/TC ICD (they live in the HAL/driver headers and platform headers) — FRICK-REVIEW-0130 R3-06 |
| 0.5 | 2026-10-13 | frick-fsw | Correct TC/TM secondary-header lengths and PDL arithmetic; NC-EXT-26 correction pending independent review |
| 0.6 | 2026-10-13 | frick-fsw | Clarify CRC inclusion in PDL and label implemented zero sequence count pending profile decision (NC-EXT-26/NC-EXT-06) |
| 0.7 | 2026-10-13 | frick-fsw | Identify ICD as an internal reference profile pending authority approval and distinguish CRC generation from residue validation (NC-EXT-13) |
| 0.8 | 2026-10-13 | frick-fsw | Remove implication of approved CUC tailoring and document decoder CRC comparison behavior (NC-EXT-13/26) |
| 0.9 | 2026-10-15 | frick-fsw | Define internal reference UART receive framing, timeout granularity and single-reader assumption; this remains a project reference profile, not mission-tailoring approval |
| 1.0 | 2026-10-15 | frick-fsw | Record head-of-line behavior for plausible incomplete PDL candidates and the pending authority disposition on timeout/recovery policy |
| 1.1 | 2026-10-15 | frick-fsw | Distinguish the 1024-octet configured PUS payload maximum from the 1037-octet maximum complete TC packet capacity |
| 1.2 | 2026-10-16 | frick-fsw | Document disconnected-UART early-zero handling and explicit malformed plausible-PDL packet-loss/recovery limitation from R1 review |
