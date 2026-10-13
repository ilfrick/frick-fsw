# Integration Manual fsw-cross-check — independent FSW review

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0159                      |
| Reviewer session                  | independent subagent, fresh context (norm N9) |
| Reviewer profile                  | R1 (Senior FSW engineer)               |
| Reference revision                | `docs/engineering/user-integration-manual.md` rev 0.6 (2026-10-12); repo `main` at base commit with M3–M6 documentation set |
| Items under review                | `docs/engineering/user-integration-manual.md` — every claim, API name, file path, build flag, make target, status label, and code snippet cross-checked against the actual source tree |
| Date                              | 2026-10-13                             |
| Scope                             | Correctness of the user-integration manual as a contract for an integrator building applications, drivers and pluggable stacks. Verified every header reference, function signature, struct field name, status label, build flag, make target, and code snippet against the actual repository contents (`src/`, `config/`, `tests/`, `CMakeLists.txt`, `Makefile`). Did not verify conceptual/architectural claims that have no source-level anchor. |
| Independence declaration          | Reviewer had no part in authoring the manual or any referenced source; fresh context. Every claim below cites the manual section and the exact codebase evidence. |
| Norms applied                     | N1–N20 (see `docs/rules.md`), with emphasis on N8 (documentation currency), N9 (independent review), N12 (non-conformance resolution), N16 (revision history) |

## 1. Scope

The reviewer read the full 352-line manual and, for each section, cross-checked
every file path, function name, type name, struct field name, return value,
build flag, CMake option, make target, status label (Implemented / Planned /
Not implemented), and code snippet against the actual repository contents.

Verification was performed by direct file reads of:

- All referenced headers: `src/core/es/es.h`, `src/core/sb/sb.h`,
  `src/core/evs/evs.h`, `src/core/tbl/tbl.h`, `src/core/time/time.h`,
  `src/core/time/pps.h`, `src/core/pal/pal.h`, `src/core/common/ff_types.h`,
  `src/pus/pus.h`, `src/pus/pus_types.h`, `src/transport/link/link.h`,
  `src/hal/hal_net.h`, `src/hal/hal_eth.h`, `src/hal/hal_tsn.h`,
  `src/hal/hal_can.h`, `src/hal/hal_spacewire.h`, `src/hal/hal.h`,
  `src/security/ff_sdls.h`, `src/math/ff_math.h`, `src/apps/app.h`.
- All referenced implementations: `src/main/fsw_main.c`, `src/core/sb/sb.c`,
  `src/core/tbl/tbl.c`, `src/transport/link/link.c`,
  `src/transport/link/link_loopback.c`, `src/transport/link/link_uart.c`,
  `src/drivers/eth/gem/ff_gem_tsn.c`, `src/drivers/net/ff_net_loopback.h`,
  `src/hal/net/lwip/ff_net_lwip.h`, `src/pus/adapters/pus_adapter_open.c`,
  `src/apps/app_tm_output.c`, `src/apps/app_hk.c`, `src/apps/app_cmd_ingest.c`.
- Build files: `CMakeLists.txt`, `Makefile`, `config/fsw_config.h`,
  `config/platform/platform_native.h`, `config/platform/platform_mpfs.h`,
  `tests/CMakeLists.txt`, `tests/ff_test.h`.
- Test files: `tests/unit/test_netloop.c`, `tests/unit/test_gem_tsn.c`.
- Supporting docs: `docs/engineering/driver-development.md`,
  `docs/engineering/service-development.md`, `docs/planning/sprint-plan.md`,
  `docs/quality/class-a-gaps.md`, `docs/vv/tool-qualification.md`,
  `third_party/README.md`, `tools/scripts/fetch_third_party.sh`,
  `tools/gsw/ff_gsw.py`.

Excluded: full build/test execution, formal verification proofs, and on-target
qualification. This is a documentation correctness review, not a code or V&V
review of the framework itself.

## 2. References

- Project norms: `docs/rules.md` (N1–N20)
- Traceability: `docs/requirements/traceability.yaml`
- Reviewed document: `docs/engineering/user-integration-manual.md` (FRICK-ENG-0005, rev 0.6, 2026-10-12)
- Source files as cited in each finding below
- ECSS-E-ST-40C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2

## 3. Findings

| # | Severity (Blocker/Major/Minor/Comment) | Manual section | Finding | Evidence (actual codebase) | Recommendation |
| - | --------------------------------------- | -------------- | ------- | -------------------------- | -------------- |
| FSW-REV-001 | **Blocker** | §4 (Tables), line 82 | The type name `FF_TblValidateFn_t` does not exist anywhere in the codebase. A user copying this name will get a compilation error. | `src/core/tbl/tbl.h` line 22: `typedef FF_Status_t (*FF_TblValidateFn)(const void *image, size_t size);` — the actual type is `FF_TblValidateFn` (no `_t` suffix). A repo-wide grep confirms no `FF_TblValidateFn_t` exists. | Change `FF_TblValidateFn_t` → `FF_TblValidateFn` in the manual. |
| FSW-REV-002 | **Major** | §4 (Tables), line 80 | The manual lists `FF_TblValidate` as a table service function. No such function exists in the API. Validation is performed internally inside `FF_TblLoad`, not as a standalone call. | `src/core/tbl/tbl.h` — the declared functions are: `FF_TblInit`, `FF_TblRegister`, `FF_TblLoad`, `FF_TblActivate`, `FF_TblRevert`, `FF_TblGet`, `FF_TblDump`. There is no `FF_TblValidate`. `src/core/tbl/tbl.c` lines 69–74 show validation as a callback invoked during `FF_TblLoad`: if `g_tables[h].validate != NULL`, it calls `g_tables[h].validate(g_staging[h], len)`. | Remove `FF_TblValidate` from the function list. Explain that validation occurs during `FF_TblLoad` via the `validate` callback registered in `FF_Tbl_Def_t`. |
| FSW-REV-003 | **Major** | §7.1 (Channels implemented today), line 134 | The manual states "Only two reference link backends ship (`src/transport/link/link.c`)" attributing both `FF_LinkLoopbackBackend` and `FF_LinkUartBackend` to `link.c`. The backends are defined in separate files; `link.c` contains only the link management API. | `src/transport/link/link.c` lines 12–54: contains only `FF_LinkInit`, `FF_LinkSelect`, `FF_LinkGet`, `FF_LinkSend`, `FF_LinkRecv`. The backend definitions are: `src/transport/link/link_loopback.c` lines 64–69 (`FF_LinkLoopbackBackend`) and `src/transport/link/link_uart.c` lines 57–62 (`FF_LinkUartBackend`). CMakeLists.txt lines 87–89 list all three as separate sources. `docs/engineering/driver-development.md` line 26 also correctly identifies the backends as being in `link_loopback.c` and `link_uart.c`. | Correct the file reference to `src/transport/link/{link_loopback.c,link_uart.c}`. Clarify that `link.c` provides the link management API, not the backend implementations. |
| FSW-REV-004 | **Major** | §1 (Application model), lines 33–36 | The manual describes the init sequence as "initialises the core (`FF_PalInit`, `FF_SbInit`, `FF_EvsInit`, `FF_EsInit`, `FF_TblInit`, `FF_TimeInit`, `FF_PusInit`), registers the providers, then registers and starts the applications." The actual `fsw_main.c` sequence has additional steps and a different grouping: `FF_PusInit` is in a separate block with backend selection, and several init functions are omitted. | `src/main/fsw_main.c` lines 72–97: Core init (line 72–77) calls `FF_PalInit`, `FF_SbInit`, `FF_EvsInit`, `FF_EsInit`, `FF_TblInit`, `FF_TimeInit`, **`FF_BoardInit`** (board.h line 13), **`FF_MissionTablesInit`** (fsw_tables.h line 29). PUS init (lines 80–88) calls `FF_PusInit` + `FF_PusSetBackend(...)`. **`FF_LinkInit()`** (link.h line 26) is called at line 90. **`FF_EvsSetListener(...)`** (evs.h line 39) at line 95. **`FF_AppTmOutputInit()`** and **`FF_AppHkRegister()`** (app.h lines 29–30) at lines 96–97. The omitted functions are real, declared, and called — their absence from the manual's description prevents an integrator from understanding the actual boot sequence. | Rewrite §1 to match the actual fsw_main.c sequence: core init (PAL→SB/EVS→ES→TBL/TIME→Board→MissionTables), PUS init + backend selection, Link init, EVS listener install, provider/HK registration, app registration, `FF_EsStartAll()`. |
| FSW-REV-005 | **Minor** | §1 (Application model), line 27 | The field name "stack" does not match the actual struct field name `stack_bytes`. A user initializing `FF_AppConfig_t` by field name would write `.stack` instead of `.stack_bytes` and get a compilation error. | `src/core/es/es.h` lines 21–28: the struct field is `uint32_t stack_bytes;` (line 25). The manual text says "name, entry, stack, priority, queue depth". | Change "stack" → "stack_bytes" in the descriptive text. |
| FSW-REV-006 | **Minor** | §3 (Events), line 74 | The type name `FF_EvsEvent` omits the standard `_t` suffix. The actual type is `FF_EvsEvent_t`. | `src/core/evs/evs.h` line 26: `typedef struct { ... } FF_EvsEvent_t;`. The manual says "FF_EvsEvent/FF_EvsGetLog/FF_EvsGetTotalCount". | Change `FF_EvsEvent` → `FF_EvsEvent_t`. |
| FSW-REV-007 | **Minor** | §9 table, line 236 | The manual claims `tests/unit/test_gem_tsn.c` has "16 checks". The actual test file contains 22 `FF_TEST_ASSERT` invocations (no loops, so the count equals the literal count). | `tests/unit/test_gem_tsn.c`: `grep -c 'FF_TEST_ASSERT'` returns 22. (By contrast, `test_netloop.c`'s claimed "328 checks" was verified correct: 70 literal invocations minus 2 in-loop calls plus 256 + 4 loop iterations = 328.) | Change "16 checks" → "22 checks". |

**Severity definition:**
- **Blocker**: defect that directly prevents correct integration (compilation failure, missing API).
- **Major**: defect that materially misleads an integrator about the architecture or API (wrong file paths, missing init steps, non-existent functions).
- **Minor**: defect in a name, label, or count that a careful integrator would catch but that adds friction.
- **Comment**: observation with no conformance impact.

## 4. Additional comments (no conformance impact)

| # | Manual section | Comment |
| - | -------------- | ------- |
| FSW-COMMENT-008 | §1, line 17 | "The six `src/apps/*`" is ambiguous. The directory `src/apps/` contains 7 `.c` files (6 reference apps + `app_renode_test.c`) plus `app.h`. The number 6 is correct for the default reference set, but the phrasing `src/apps/*` implies file count. Clarify: "six reference applications (`app_cmd_ingest`, `app_hk`, `app_hs`, `app_sample`, `app_scheduler`, `app_tm_output`), plus the conditional `app_renode_test.c`". |
| FSW-COMMENT-009 | §3, line 76 | "the reference `TM_OUTPUT` application maps them to PUS service 5 telemetry" is inaccurate. The event→PUS-service-5 mapping is performed by `ff_event_listener` in `fsw_main.c` (line 51: `FF_PusEmitEvent(...)`), which is installed via `FF_EvsSetListener(ff_event_listener)` at line 95. The TM_OUTPUT application (`app_tm_output.c` lines 23–27) only installs the TM sink (`FF_PusSetTmSink`) that sends encoded packets over the link. |
| FSW-COMMENT-010 | §8.1/§10, lines 187/277 | The lwIP adapter file list omits `ff_lwip_netif.{c,h}`. The manual references only `ff_net_lwip.{c,h}` but the adapter consists of both files. CMakeLists.txt line 182 compiles `ff_net_lwip.c` and `ff_lwip_netif.c` as `frick_net_lwip`. |
| FSW-COMMENT-011 | §7, line 114 | The manual states the default PUS backend is the N7/ESA ASN.1 one but does not document the fallback: when `FF_USE_PUS_ASN1=ON` (the CMake default) and the generated ASN.1 code at `third_party/asn1-pusc-lib-generated/` is absent, CMake warns and falls back to `FF_PusBackendOpen` (CMakeLists.txt lines 59–71). First-time builders who have not run `make pus-asn1` will silently get the open backend. |
| FSW-COMMENT-012 | §9.1, lines 244–247 | The `FF_TsnBackend_t` vtable in `src/hal/hal_tsn.h` (lines 18–44) also defines `set_stream_filter` (802.1Qci) and `enable_pfc` (802.1Qbb), neither mentioned in §9.1. Both return `FF_NOT_IMPLEMENTED` in the GEM reference backend. |
| FSW-COMMENT-013 | §7.1, line 144 | `FF_LinkSelect` is described as "stores the pointer and calls the backend's `init`" but the actual implementation (`link.c` lines 26–33) also rejects a NULL `link` argument with `FF_INVALID_ARGUMENT` and only calls `init` when the function pointer is non-NULL. |

## 5. Conclusion

- [ ] APPROVED - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [x] **REJECTED** - one or more Blocker/Major findings

**Rationale:** The manual contains one **Blocker** finding (FSW-REV-001: a type name that does not compile), two **Major** findings that materially mislead integrators about non-existent functions and wrong file locations (FSW-REV-002, FSW-REV-003), and one **Major** finding of a materially incomplete boot-sequence description (FSW-REV-004). These must be corrected before the manual can serve its integration purpose.

## 6. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
| ---- | ---------------- | ---- | ---------- |
| R1 (Senior FSW engineer) | independent subagent (fresh context, norm N9) | 2026-10-13 | REJECTED |

## 7. Non-Conformance records

| NC | Severity | Manual section / Item | Finding | Correction | Assign to | Sprint | Status |
| -- | -------- | --------------------- | ------- | ---------- | --------- | ------ | ------ |
| NC-159-01 | Blocker | §4 `FF_TblValidateFn_t` | Type name does not exist (actual: `FF_TblValidateFn`) | Rename to `FF_TblValidateFn` in manual §4 | frick-fsw authoring agent | S0 | open |
| NC-159-02 | Major | §4 `FF_TblValidate` | Listed function does not exist; validation is internal to `FF_TblLoad` | Remove `FF_TblValidate` from the function list; document that validation occurs during `FF_TblLoad` via the `validate` callback in `FF_Tbl_Def_t` | frick-fsw authoring agent | S0 | open |
| NC-159-03 | Major | §7.1 link backends | Backends attributed to `link.c`; actual implementations are in `link_loopback.c` / `link_uart.c` | Correct file references to `src/transport/link/{link_loopback.c,link_uart.c}` | frick-fsw authoring agent | S0 | open |
| NC-159-04 | Major | §1 init sequence | Core init sequence omits `FF_BoardInit`, `FF_MissionTablesInit`, `FF_LinkInit`, `FF_EvsSetListener`, `FF_AppTmOutputInit`, `FF_AppHkRegister`; `FF_PusInit` mis-grouped | Rewrite §1 to match `fsw_main.c` lines 72–98 exactly | frick-fsw authoring agent | S0 | open |
| NC-159-05 | Minor | §1 `FF_AppConfig_t` field "stack" | Field name should be `stack_bytes` | Correct field name in descriptive text | frick-fsw authoring agent | S0 | open |
| NC-159-06 | Minor | §3 `FF_EvsEvent` | Should be `FF_EvsEvent_t` | Add `_t` suffix | frick-fsw authoring agent | S0 | open |
| NC-159-07 | Minor | §9 `test_gem_tsn.c` count | 16 → 22 | Correct check count | frick-fsw authoring agent | S0 | open |

**Comments to close in S0 (no NC required):** FSW-COMMENT-008 through FSW-COMMENT-013 are documentation-quality observations recommended for correction alongside the NCs. Per norm N12, the manual delta re-review closes all NCs before the document is considered APPROVED.

---

*Committed and pushed with the reviewed change (norm N11).*
