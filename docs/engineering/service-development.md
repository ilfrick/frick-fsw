# PUS service development and extension guide

| Document id | FRICK-ENG-0004 |
| ----------- | -------------- |
| Revision    | 0.1 |
| Date        | 2026-10-10 |
| Audience    | humans and agents extending the PUS-C service set |

This guide explains how to add a **PUS-C service or subservice** to frick-fsw
(ECSS-E-ST-70-41C) on top of the backend-agnostic dispatcher, following the
norms (N1-N16). It covers the handler contract, registration, telemetry
emission, telecommand verification, error reporting, the provider registration
hooks (housekeeping, parameters, functions, memory) and the tests.

Read first: `AGENTS.md`, `docs/architecture/pus-layer.md` (tailoring matrix),
`docs/interfaces/ICD.md` (packet layout).

## 1. How dispatch works

`FF_PusIngest` (`src/pus/pus.c`) is the single entry point for a received
telecommand:

1. the backend (`FF_Pus_Backend_t`) decodes the CCSDS frame into the TC
   secondary header (`FF_Pus_TcHdr_t`) and the application data;
2. **service 1 verification** is emitted automatically for every requested
   acknowledgement stage (acceptance / start / progress / completion), driven by
   `tc.ack_flags`;
3. the dispatcher looks up the handler for `(service, subservice)`;
4. the handler runs and returns `FF_SUCCESS` or an error code; the completion
   failure code follows the handler return;
5. the handler may itself emit telemetry with `FF_PusSendTm`.

You rarely emit service 1 by hand; you implement the handler and send replies.

## 2. The handler contract

```c
typedef FF_Status_t (*FF_Pus_Handler_t)(const FF_Pus_TcHdr_t *tc,
                                        const uint8_t *data, uint16_t len);
```

- `tc` carries `version`, `ack_flags`, `service`, `subservice`, `source_id`.
- `data`/`len` are the application data after the PUS secondary header.
- Return `FF_SUCCESS` for a good completion; return `FF_ERROR`/specific codes
  for a failed completion (mapped to `FF_PUS_FAIL_EXECUTION` by the dispatcher).

Keep handlers short and deterministic: no dynamic allocation, bounded loops,
validate every field (length first) before use (norm N10).

## 3. Registering a handler

```c
FF_Status_t FF_PusRegisterService(uint8_t service, uint8_t subservice,
                                  FF_Pus_Handler_t handler);
```

The reference set is registered in `FF_PusRegisterCoreServices`
(`src/pus/pus_services.c`) with a small table:

```c
struct { uint8_t service; uint8_t subservice; FF_Pus_Handler_t handler; } table[] = {
    {3u, 1u, svc3_create}, /* ... */
};
```

To add a service: write the handlers, add rows to the table (or call
`FF_PusRegisterService` from your application's init), and raise
`FF_PUS_MAX_REG` in `src/pus/pus_services.c` if you exceed the current 16
registration slots.

## 4. Emitting telemetry and events

- Reply telemetry: `FF_PusSendTm(service, subservice, data, len)`. By convention
  a report is `request subservice + 1` (e.g. S17 `ping` → S17,2);
  housekeeping dumps use S3,25; memory dumps S6,6.
- Events: emit through EVS (`FF_EvsReport`), which the core maps to service 5
  (`FF_PusEmitEvent`). Do not build service 5 packets by hand.
- Timestamps: the TM secondary header CUC is filled by the core
  (`FF_PusSendTm` → `FF_TimeGetCuc`); never stamp manually.

## 5. Error and failure codes

Service 1 completion failures use `FF_PUS_FAIL_*` (`src/pus/pus_types.h`):
`ILLEGAL_APID`, `ILLEGAL_LENGTH`, `CRC`, `UNKNOWN_SERVICE`, `UNKNOWN_SUBSERVICE`,
`BAD_DATA`, `EXECUTION`. The dispatcher already reports unknown services
(`UNKNOWN_SERVICE`/`UNKNOWN_SUBSERVICE`) and maps a handler error to
`EXECUTION`; use `BAD_DATA` from a handler when the payload is well-framed but
invalid.

## 6. Provider hooks (S3 / S6 / S8 / S20)

Several services are generic and are fed by registration calls, so an
application can supply data without writing a service handler:

| Call | Service | Payload |
| ---- | ------- | ------- |
| `FF_PusRegisterHkProvider(id, fn)` | S3 housekeeping | `fn(id, out, &len)` fills the parameter block |
| `FF_PusRegisterParamProvider(id, fn)` | S20 report | same shape |
| `FF_PusRegisterParamSetter(id, fn)` | S20 set | `fn(id, data, len)` applies a new value |
| `FF_PusRegisterFunction(id, fn)` | S8 function | `fn(args, len)` invokes a named function |
| `FF_PusRegisterMemoryRegion(base, size)` | S6 memory | allow-list for dumps (out-of-region refused) |

Example: applications register their housekeeping blocks in `app_hk.c`; the
S3 handler in `pus_services.c` walks the providers and builds the structure.

## 7. Worked example — a new file-management service (S23)

```c
/* returns the created file length on success */
static FF_Status_t svc23_create(const FF_Pus_TcHdr_t *tc, const uint8_t *d, uint16_t n)
{
    if (n < 2u) { return FF_ERROR; }          /* validate first */
    uint16_t name_len = (uint16_t)((d[0] << 8) | d[1]);
    if ((uint16_t)(n) < (uint16_t)(2u + name_len)) { return FF_ERROR; }

    /* call the virtual filestore (see docs/engineering/... and src/fs/) */
    if (ff_vfs_create((const char *)&d[2], name_len, d[2 + name_len]) != FF_SUCCESS) {
        return FF_ERROR;                       /* → S1 completion failure */
    }
    return FF_PusSendTm(23u, 2u, d, n);        /* success telemetry */
}
```

Then register `{23u, 1u, svc23_create}` and add the S23 rows next to the others.
Milestone M3 of `docs/roadmap.md` builds this on the virtual filestore.

## 8. Testing a service

Use the loopback/open backend as in `tests/unit/test_pus.c`: build a CCSDS TC
with the open codec, call `FF_PusIngest`, and assert on the emitted stream. The
TM service/subservice live at byte offsets **7** and **8** in the packet; the
test harness helper `stream_has(service, subservice)` shows the pattern.

```c
/* build TC for service 17, subservice 1, then: */
FF_PusIngest(tc, tc_len);
assert(stream_has(17u, 2u));   /* ping reply */
```

Add the test to `tests/CMakeLists.txt` and give it a `add_test` name.

## 9. Traceability, Lean and V&V

1. Add/refine the **SRS** item (`docs/requirements/SRS.md`) and its SSS parent.
2. Link implementation + test paths in
   `docs/requirements/traceability.yaml`; sync `traceability.md`.
3. If the service has pure logic (length checks, arithmetic, state), model it in
   `verification/lean/Frick/` and link the theorem (norm N1).
4. Update `docs/architecture/pus-layer.md` (tailoring matrix) and the ICD when
   the packet layout is mission-specific (norm N8).
5. Re-run the full cycle (`make ci-local`), record evidence in
   `docs/vv/verification-results.md`, and request an independent review when the
   criticality requires it (`docs/engineering/independent-review.md`, N9).

## 10. Checklist

- [ ] Handler signature respected; input validated before use.
- [ ] Registered in the table (or via `FF_PusRegisterService`); slot limit raised
      if needed.
- [ ] Reply telemetry via `FF_PusSendTm`; events via EVS; no manual timestamps.
- [ ] Failure codes chosen from `FF_PUS_FAIL_*`.
- [ ] Unit test added (service/subservice at offsets 7/8) and green.
- [ ] SRS/SSS + `traceability.yaml` (+ Lean if logic) updated.
- [ ] `docs/architecture/pus-layer.md` / ICD updated if needed.
- [ ] `CHANGELOG.md` + this document's History updated; full cycle green;
      committed and pushed to both remotes.

## 11. Heritage references

- NASA cFS application model (command/telemetry pipes) as inspiration.
- ECSS-E-ST-70-41C (PUS-C) service definitions; CCSDS 133.0-B-2 (Space Packet).

## History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial PUS service extension guide (norm N8) |
