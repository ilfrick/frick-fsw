# Interface Control Document (ICD) - TM/TC Ground Interface

| Document id | FRICK-ICD-001 |
| ----------- | ------------- |
| Revision    | 0.3 |
| Date        | 2026-10-10 |
| Profile     | PUS-C (ECSS-E-ST-70-41C) over CCSDS Space Packets (CCSDS 133.0-B-2) |

## 1. Scope

This ICD defines the space-to-ground and ground-to-space packet interface of
frick-fsw for the reference mission profile. It is the tailoring baseline for
SRS-REQ-017/SSS-REQ-014.

## 2. Configuration summary (from `config/fsw_config.h`)

| Parameter | Value |
| --------- | ----- |
| PUS version | 2 (PUS-C) |
| Mission identifier | `0x01` |
| TC APID | `0x010` |
| TM APID | `0x020` |
| Packet CRC | CRC-16/CCITT-FALSE, trailer appended (enable `FF_FRAME_WITH_CRC=1`) |
| Max TM frame / TC frame | `FF_PUS_MAX_TM_FRAME` / `FF_PUS_MAX_TC_FRAME` (1024) |
| In/out-of-band transport | UART byte stream (target `/dev/console`, 115200 baud); loopback on host simulator |

## 3. TM/TC frame field layout (CCSDS Space Packet + PUS secondary header)

```
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |version| type |  sec-hdr flag |   APID (TC 0x010, TM 0x020)    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  seq flags |      sequence count (per type)                   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |        packet data length field (PDL, 16 bit)                 |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  PUS version  | (spare) | ack |  service    | subservice      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  source/application id [2] |     user data ...                |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                 ... user data ...        |  CRC-16 (2 octets) |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

- PDL counts the bytes after the PDL field: `PUS secondary header (6) +
  user data + CRC (2)`, i.e. PDL = len - 7 (packet without primary header).
- CRC-16/CCITT-FALSE (poly `0x1021`, init `0xFFFF`, no reflection) covers the
  whole packet; a receiver treats `crc16(packet with trailer) == 0` as valid
  (SRS-REQ-004).

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

1. **PUS-C on-wire T-field — 6 octets, P-field implicit.** This is the form
   inside the TM secondary header and the PUS service 9 payloads. The PUS-C
   interface agreement fixes the tailoring, so the P-field is not transmitted:

   | Field | Bytes | Value |
   | ----- | ----- | ----- |
   | coarse | 4 | whole seconds big-endian, `utc_ns / 1e9 + leap_seconds` |
   | fine | 2 | fraction of a second in units of 2^-16 s big-endian |

   In the TM secondary header the T-field occupies user-data offsets 7..12
   (coarse 7..10, fine 11..12); the TM secondary header is 13 bytes
   (`FF_PUS_TM_SEC_HDR_LEN`). PUS service 9,1 (set time) accepts exactly this
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