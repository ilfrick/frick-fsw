# Integration Manual — independent documentation review (R4)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0160                      |
| Reviewer session                  | independent subagent, fresh context (norm N9) |
| Reviewer profile                  | R4 (Senior Aeronautics & Space Software Documentation Engineer) |
| Reference revision                | `docs/engineering/user-integration-manual.md` FRICK-ENG-0005, rev 0.6 (2026-10-12); repo baseline at base commit with the M3–M6 documentation set |
| Items under review                | `docs/engineering/user-integration-manual.md` — every file path, function name, type name, struct field name, build flag, make target, status label, GAP label, code snippet, attribution of behaviour, and cross-reference, checked against the source tree (`src/`, `config/`, `tests/`, `CMakeLists.txt`, `Makefile`) and against the project's own architecture/design documents of the same baseline |
| Date                              | 2026-10-13                             |
| Scope                             | Completeness, clarity, accuracy, internal consistency and integration-usefulness of the user & integration manual (FRICK-ENG-0005). Verified every header reference, function signature, type name, struct field, build flag, make target, status table, GAP classification, code snippet and behavioural attribution against the actual repository contents and the canonical architecture/design notes. Excluded: full build/test execution and on-target qualification (this is a documentation review, not a code or V&V review). Cross-references the independent FSW cross-check FRICK-REVIEW-0159 (R1) where findings overlap. |
| Independence declaration          | Reviewer had no part in authoring the manual or any referenced source; fresh context. Every claim below cites the manual section and the exact codebase evidence. |
| Norms applied                     | N1–N20 (see `docs/rules.md`), with emphasis on N8 (complete/current documentation), N9 (independent review), N12 (non-conformance resolution), N16 (revision history) |

## 1. Scope

The reviewer read the full 352-line manual (rev 0.6, 2026-10-12) and, for each
section, cross-checked every file path, function/type/field name, build flag,
make target, status label, GAP classification, code snippet and behavioural
attribution against:

- Headers: `src/core/es/es.h`, `src/core/sb/sb.h`, `src/core/evs/evs.h`,
  `src/core/tbl/tbl.h`, `src/core/time/time.h`, `src/core/time/pps.h`,
  `src/core/pal/pal.h`, `src/core/common/ff_types.h`, `src/pus/pus.h`,
  `src/transport/link/link.h`, `src/hal/hal_net.h`, `src/hal/hal_eth.h`,
  `src/hal/hal_tsn.h`, `src/security/ff_sdls.h`, `src/math/ff_math.h`,
  `src/apps/app.h`.
- Implementations: `src/main/fsw_main.c`, `src/core/sb/sb.c`, `src/core/tbl/tbl.c`,
  `src/transport/link/link.c`, `src/drivers/eth/gem/ff_gem_tsn.c`,
  `src/drivers/net/ff_net_loopback.c`, `src/drivers/net/ff_net_loopback.h`,
  `src/hal/net/lwip/ff_net_lwip.h`, `src/hal/net/lwip/ff_lwip_netif.c`,
  `src/hal/net/lwip/ff_lwip_netif.h`, `src/apps/app_tm_output.c`.
- Build: `CMakeLists.txt`, `Makefile`, `config/fsw_config.h`,
  `tests/CMakeLists.txt`, `tests/ff_test.h`.
- Tests: `tests/unit/test_netloop.c`, `tests/unit/test_gem_tsn.c`.
- Normative design/docs of the same baseline: `docs/engineering/software-description.md`
  (FRICK-ENG-0007 rev 0.5), `docs/architecture/network-stack.md` (FRICK-ARCH-0012 rev 0.6),
  `docs/engineering/driver-development.md`, `docs/engineering/service-development.md`,
  `docs/quality/class-a-gaps.md`, `docs/quality/ecss-compliance-matrix.md`,
  `docs/requirements/traceability.yaml`, `docs/planning/sprint-plan.md`,
  `tools/scripts/fetch_third_party.sh`, `third_party/README.md`,
  `tools/gsw/ff_gsw.py`, `docs/vv/tool-qualification.md`.

This review re-derives every finding independently and cites the prior independent
FSW report (FRICK-REVIEW-0159) where the same defect was already recorded.

## 2. References

- Project norms: `docs/rules.md` (N1–N20)
- Traceability manifest: `docs/requirements/traceability.yaml`
- Class-A gaps: `docs/quality/class-a-gaps.md`; compliance matrix: `docs/quality/ecss-compliance-matrix.md`
- Reviewed document: `docs/engineering/user-integration-manual.md` (FRICK-ENG-0005, rev 0.6, 2026-10-12)
- Prior independent cross-check: `docs/reviews/2026-10-13-integration-manual-fsw-review.md` (FRICK-REVIEW-0159, R1)

## 3. Findings

| # | Severity (Blocker/Major/Minor/Comment) | Manual section | Finding | Evidence (actual codebase) | Recommendation |
| --- | --------------------------------------- | -------------- | ------- | -------------------------- | -------------- |
| DOC-R4-001 | **Blocker** | §4 (Tables), line 82 | The type name `FF_TblValidateFn_t` does not exist; a user copying it gets a compile error. | `src/core/tbl/tbl.h` line 22: `typedef FF_Status_t (*FF_TblValidateFn)(const void *image, size_t size);` — actual type is `FF_TblValidateFn` (no `_t`). Repo-wide grep matches none. | Rename `FF_TblValidateFn_t` → `FF_TblValidateFn` in §4 and §9.1-style references. |
| DOC-R4-002 | **Major** | §4 (Tables), line 80 | `FF_TblValidate` is listed as a table service function. No such function exists in the API. | `src/core/tbl/tbl.h` — declared functions: `FF_TblInit`, `FF_TblRegister`, `FF_TblLoad`, `FF_TblActivate`, `FF_TblRevert`, `FF_TblGet`, `FF_TblDump`. No `FF_TblValidate`. `src/core/tbl/tbl.c` lines 69–74: validation is invoked inside `FF_TblLoad` via the `g_tables[h].validate` callback. | Remove `FF_TblValidate` from the function list; state that validation runs during `FF_TblLoad` through the `validate` callback in `FF_Tbl_Def_t`. |
| DOC-R4-003 | **Major** | §1 (Application model), lines 33–36 | §1's start-up summary is materially incomplete: it omits `FF_BoardInit`, `FF_MissionTablesInit`, `FF_LinkInit`, `FF_EvsSetListener` and the provider/HK init hooks, mis-groups `FF_PusInit` into "core init", and omits Link init entirely. | `src/main/fsw_main.c`: block 72–77 = core init (`FF_PalInit`→`FF_SbInit`→`FF_EvsInit`→`FF_EsInit`→`FF_TblInit`→`FF_TimeInit`→**`FF_BoardInit`**→**`FF_MissionTablesInit`**); block 80–88 = PUS init + backend (`FF_PusInit`, `FF_PusSetBackend`); line 90 = `FF_LinkInit`; lines 95–97 = `FF_EvsSetListener(ff_event_listener)` + `FF_AppTmOutputInit()` + `FF_AppHkRegister()`; 100–112 = app registration; 116 = `FF_EsStartAll`. | Rewrite §1 to match `fsw_main.c` lines 72–98 exactly (core → PUS → Link → providers → apps → start). |
| DOC-R4-004 | **Major** | §7.1 line 134 & §8 table line 188 | Link backends attributed to `link.c` / `link.{c,h}`; that file only holds the management API. The backends live in separate files. | `src/transport/link/link.c` lines 12–54: only `FF_LinkInit`, `FF_LinkSelect`, `FF_LinkGet`, `FF_LinkSend`, `FF_LinkRecv`. Backends: `src/transport/link/link_loopback.c` lines 49–53 (`FF_LinkLoopbackBackend`) and `src/transport/link/link_uart.c` lines 29–34 (`FF_LinkUartBackend`); `CMakeLists.txt` lines 87–89 list all three as separate sources. | Cite `src/transport/link/{link_loopback,link_uart}.c` for the backends and `link.c` for management. (Same root cause as FRICK-REVIEW-0159 FSW-REV-003.) |
| DOC-R4-005 | **Major** | §8, line 224 | The manual states "formal verification depth is GAP-A4". This mislabels the gap; the canonical mapping is formal-verification depth = **GAP-A1**. | `docs/quality/class-a-gaps.md` line 17: "## GAP-A1 - Deep formal verification"; `docs/quality/ecss-compliance-matrix.md` line 77: "GAP-A1 deep formal verification", line 80: "GAP-A4 MISRA, WCET/stack, fault injection, toolchain". `docs/engineering/software-description.md` (FRICK-ENG-0007 rev 0.5) line 307: "(e.g. GAP-A1 formal-verification depth, GAP-A4 on-target runtime evidence, GAP-A5 on-target hardware/PHY)". The manual also itself notes GAP-A5/A-GAP-A4 for the netif/PHY/stack at line 224. | Change "formal verification depth is GAP-A4" → "GAP-A1"; keep "on-target code assurance / runtime evidence is GAP-A4". Align §8 with software-description.md §3.7 and class-a-gaps.md. |
| DOC-R4-006 | Minor | §3 (Events), line 74 | The type name `FF_EvsEvent` omits the `_t` suffix. | `src/core/evs/evs.h` line 26: `typedef struct { ... } FF_EvsEvent_t;`. Manual: "FF_EvsEvent/FF_EvsGetLog/FF_EvsGetTotalCount". | `FF_EvsEvent` → `FF_EvsEvent_t`. |
| DOC-R4-007 | Minor | §1 (Application model), line 27 | Descriptive field "stack" does not match the struct field `stack_bytes`; field-name initialization would fail. | `src/core/es/es.h` lines 21–28: `uint32_t stack_bytes;` (line 25). Manual text: "name, entry, stack, priority, queue depth". | "stack" → "stack_bytes". |
| DOC-R4-008 | Minor | §1, line 26 | `FF_PalTaskFunc_t` is pointed to `src/apps/app.h`; the typedef is defined in `src/core/pal/pal.h`. `app.h` only declares the six reference entry points and mentions the type in a comment. | `src/core/pal/pal.h` line 29: `typedef void (*FF_PalTaskFunc_t)(void *arg);`. `src/apps/app.h` lines 14–31: no `FF_PalTaskFunc_t` definition. | Point `FF_PalTaskFunc_t` to `src/core/pal/pal.h` (and note `app.h` shows the entry-point pattern). |
| DOC-R4-009 | Minor | §9 table, line 236 | `test_gem_tsn.c` cited as "16 checks"; the file contains 22 `FF_TEST_ASSERT` (no loops, so runtime count = 22). | `tests/unit/test_gem_tsn.c`: `grep -c 'FF_TEST_ASSERT'` = 22, single test fn `test_tsn_vlan_and_time`. | "16 checks" → "22 checks". (Note: §8's `test_netloop.c` "(328 checks)" is *correct* — 70 source asserts minus the 2 in-loop source lines plus 256 + 4 loop iterations = 328 runtime, matching `docs/architecture/network-stack.md` §5; no change needed there.) |
| DOC-R4-010 | Minor | §8.1 line 199 & §8 table line 184 | §8.1 tells the integrator "Addresses are 6 octets (IPv4[4] big-endian, then port[2] big-endian)" as a contract rule, but the cited loopback reference backend and its test use a 1-byte port in `addr[0]`. | `docs/architecture/network-stack.md` §2 (same baseline) states the 6-octet format. But `src/drivers/net/ff_net_loopback.h` comment: "Addresses are single-byte ports in addr[0]."; `src/drivers/net/ff_net_loopback.c` `netloop_bind`: `s->port = addr[0]` with `addr_len < 1u`; `tests/unit/test_netloop.c`: `uint8_t addr[1] = {7u};`. A 6-octet address passed to the loopback yields `addr[0]` (the IPv4 first octet) as the port. | Qualify the address format as backend-specific: 6 octets for the lwIP adapter (`src/hal/net/lwip/ff_net_lwip.h`), 1-byte port for the loopback reference backend; do not present a single universal format. |
| DOC-R4-011 | Minor | §3, line 76 | The manual attributes the event→PUS-service-5 mapping to "the reference `TM_OUTPUT` application". That mapping is performed by `ff_event_listener` registered via `FF_EvsSetListener()` in `fsw_main.c`; TM_OUTPUT only installs the TM sink. | `src/main/fsw_main.c` line 51 `FF_PusEmitEvent(...)` inside `ff_event_listener`; line 95 `FF_EvsSetListener(ff_event_listener)`; `src/apps/app_tm_output.c` lines 23–27 `FF_AppTmOutputInit()` installs `FF_PusSetTmSink(ff_to_sink)` and `FF_AppTmOutputMain` only delays. | State that events are forwarded to PUS service 5 by the listener installed with `FF_EvsSetListener()` (example: `ff_event_listener` in `fsw_main.c`); TM_OUTPUT emits the encoded TM over the link via the TM sink. |
| DOC-R4-012 | Minor | §9.1 lines 244–247 & §9 table line 235 | `FF_TsnBackend_t` is documented with only four vtable methods; the contract in `hal_tsn.h` (and the test) also defines `set_stream_filter` (802.1Qci) and `enable_pfc` (802.1Qbb), both returning `FF_NOT_IMPLEMENTED` in the GEM backend. | `src/hal/hal_tsn.h` lines 18–44: `set_stream_filter` and `enable_pfc` are part of the vtable. `src/drivers/eth/gem/ff_gem_tsn.c` lines 119–179 (`set_stream_filter`) and 185–190 (`enable_pfc` → `FF_NOT_IMPLEMENTED`). `tests/unit/test_gem_tsn.c` lines 97/108 exercise them. | Add `set_stream_filter` (802.1Qci) and `enable_pfc` (802.1Qbb) to §9.1, noting both return `FF_NOT_IMPLEMENTED` in the GEM reference backend. |
| DOC-R4-013 | Minor | §8 table line 187 & §8.1 lines 205–209 | The lwIP adapter is cited only as `ff_net_lwip.{c,h}`; the adapter also comprises the netif glue `ff_lwip_netif.{c,h}` (bound to `FF_EthMac_t`). | `src/hal/net/lwip/ff_lwip_netif.c` and `ff_lwip_netif.h` exist; `docs/architecture/network-stack.md` §3/§4 lists both file pairs; `CMakeLists.txt` line 182 compiles `ff_net_lwip.c` and `ff_lwip_netif.c` as `frick_net_lwip`. | Add `ff_lwip_netif.{c,h}` to the lwIP adapter file list in §8 and §8.1. |

## 4. Comments (no conformance impact, recommended)

| # | Manual section | Comment |
| --- | -------------- | ------- |
| DOC-COMMENT-014 | §7, lines 114–116 | The PUS backend fallback is not documented. With `FF_USE_PUS_ASN1=ON` (CMake default) and the generated ASN.1 code (`third_party/asn1-pusc-lib-generated/`) absent, CMake warns and falls back to `FF_PusBackendOpen`. `third_party/README.md` and `AGENTS.md` state this; the manual should, so first-time builders know they may silently get the open backend. |
| DOC-COMMENT-015 | §13, line 317 | `make coverage metrics` is valid (two targets) but redundant: `metrics` already depends on `coverage` (Makefile line 151), so `coverage` runs twice. The canonical quickstart (`README.md`/`AGENTS.md` §3) uses `make coverage` then `make metrics`. Not an error, but tighten to `make metrics` or the two-step form. |
| DOC-COMMENT-016 | §1, line 17 | "The six `src/apps/*`" is ambiguous: `src/apps/` holds six reference apps plus `app_renode_test.c` (conditional, `FF_ENABLE_RENODE_TEST`) and `app.h`. Spell the reference set (cmd_ingest, hk, hs, sample, scheduler, tm_output) and note the conditional renode-test app. (Same observation as FRICK-REVIEW-0159 FSW-COMMENT-008.) |
| DOC-COMMENT-017 | §8 table row 1 & §8.1 line 199 | `address = 6 octets` is stated as if it were a hard contract rule in `hal_net.h`, but `src/hal/hal_net.h` deliberately takes an opaque `const uint8_t *addr, uint16_t addr_len` (the 6-octet convention is an adapter agreement documented in `network-stack.md`, not the vtable). Wording should reference the adapter agreement rather than imply the HAL enforces a format. |

## 5. Findings confirmed correct (no change needed)

For completeness, the reviewer explicitly verified these manual claims and found
them accurate against the baseline:

- `FF_SB_WILDCARD` = `0xFFFFu` (`config/fsw_config.h` line 32) and wildcard match
  in `src/core/sb/sb.c` line 142 — §2 is accurate.
- `FF_SB_MSG_PAYLOAD_MAX` = 512u (`config/fsw_config.h` line 27) — §2 accurate.
- `FF_Success`/`FF_Status_t`, `FF_AppId_t`, `FF_NOT_IMPLEMENTED`/`FF_INVALID_ARGUMENT`
  — §1/§2/§9 accurate.
- §5 PUS registration API (`FF_PusRegisterHkProvider`, `FF_PusRegisterParamProvider`,
  `FF_PusRegisterParamSetter`, `FF_PusRegisterFunction`, `FF_PusRegisterMemoryRegion`)
  all exist in `src/pus/pus.h` lines 65–69 — accurate; matches
  `docs/engineering/service-development.md` lines 97–101.
- §6/§7/§15 APIs (`FF_TimeGetCuc`/`FF_TimeCucEncode`/`FF_TimeCucDecode`,
  `FF_PusIngest`/`FF_PusSendTm`/`FF_Pus_Backend_t`/`FF_PusBackendN7`/`FF_PusBackendOpen`/
  `FF_PusSetBackend`, `FF_CryptoBackend_t`, `FF_Link_t`/`FF_LinkSelect`/`FF_LinkInit`/
  `FF_LinkGet`/`FF_LinkLoopbackBackend`/`FF_LinkUartBackend`,
  `FF_TsnBackend_t`/`FF_TsnGem_t`/`FF_TsnGemSetup`/`FF_TsnGate_t`, the `FF_Math*` set)
  all exist with matching signatures — accurate.
- §8/§9 status labels: no TCP/IP/TSN/CAN/SpaceWire `FF_Link_t` backend exists in
  `src/transport/link/` (only `link_loopback.c`, `link_uart.c`); `configure_cbs`
  returns `FF_NOT_IMPLEMENTED` (`ff_gem_tsn.c` line 112); gPTP/CoreTSN are S8/M8
  (unexecuted; `docs/roadmap.md` line 117, `docs/planning/sprint-plan.md` S8);
  PUS-C-over-IP segmentation is SRS-REQ-059/060/061 and the active sprint is S8,
  so "not implemented / planned" is consistent with the source and sprint plan.
- §8 table: `FF_NetStack_t` (`src/hal/hal_net.h` SRS-REQ-027), `FF_EthMac_t`/`FF_NetIf_t`
  (`src/hal/hal_eth.h` SRS-REQ-026), `FF_NetStackLoopback` (`src/drivers/net/ff_net_loopback.{c,h}`
  SRS-REQ-039), `FF_NetStackLwip` (`src/hal/net/lwip/ff_net_lwip.{c,h}` SRS-REQ-040),
  lwIP pin `STABLE-2_2_1_RELEASE`/BSD-3-Clause (`tools/scripts/fetch_third_party.sh`
  line 19; `third_party/README.md`), `FF_TsnBackend_t` (`src/hal/hal_tsn.h` SRS-REQ-028),
  GEM TSN (`src/drivers/eth/gem/ff_gem_tsn.{c,h}` SRS-REQ-038) — all accurate.
- §8 table: ground station serial-only, `tools/gsw/ff_gsw.py`, V&V tool T7
  (`docs/vv/tool-qualification.md` line 31) — accurate (file uses pyserial
  `--connect /dev/ttyUSB0`).
- §8/§9 "Absence from ICD": `docs/interfaces/ICD.md` contains no TCP/UDP/IPv4,
  EtherType or TSN content (grep returns none) — accurate.
- §8 gap context: netif/PHY/stack = GAP-A5 (`class-a-gaps.md` GAP-A5 "On-target
  qualification"); on-target code assurance/runtime evidence = GAP-A4 — accurate.
  Only the "formal verification depth" label (→GAP-A1) is wrong (DOC-R4-005).
- §13 make targets (`make verify-lean`, `make trace`, `make pus-asn1`,
  `make ci-local`), test registration in `tests/CMakeLists.txt`, and
  `docs/requirements/traceability.yaml` requirement links — accurate.
- §8.1 step 1 socket API (`init/attach_netif/open/connect/bind/listen/accept/send/recv/
  close`, `*sent` partial send) — matches `src/hal/hal_net.h` exactly.
- Minimal application (§1): struct field order and the `FF_EsRegisterApp`/
  `FF_SbCreatePipe`/`FF_SbSubscribe`/`FF_SbReceive`/`FF_EsGetId` calls compile
  against `es.h`/`sb.h` — API-accurate (only the descriptive "stack" label and the
  `FF_PalTaskFunc_t`→`app.h` pointer need fixing, DOC-R4-007/DOC-R4-008).

## 6. Conclusion

- [ ] APPROVED - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - only Minor/Comment findings, all tracked to closure
- [x] **REJECTED** - one or more Blocker/Major findings present and unresolved

**Rationale:** The manual contains one **Blocker** finding (DOC-R4-001: a type
name that does not compile) and four **Major** findings that materially mislead an
integrator (DOC-R4-002 a non-existent table function; DOC-R4-003 an incomplete
boot sequence; DOC-R4-004 wrong link-backend source attribution; DOC-R4-005 a
mislabeled GAP classification that contradicts the canonical design document
`docs/engineering/software-description.md` §3.7 and `docs/quality/class-a-gaps.md`).
These overlap the independent FSW cross-check (FRICK-REVIEW-0159, which already
REJECTED the manual on FSW-REV-001/002/003/004) and are independently confirmed
here. The documentation review therefore cannot be APPROVED until the
Blocker/Major NCs are closed and the manual delta is re-reviewed (norm N12/N17).
The Minor and Comment findings must also be corrected (nor N12) before closure.

## 7. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
| ---- | ---------------- | ---- | ---------- |
| R4 (Senior SW documentation engineer, norm N9/N17) | independent subagent (fresh context) | 2026-10-13 | REJECTED |

## 8. Non-Conformance records

Open NCs from the prior FSW cross-check (FRICK-REVIEW-0159) that the manual
delta must close (norm N12):

| NC | Severity | Manual section / Item | Finding | Correction | Owner | Status |
| -- | -------- | --------------------- | ------- | ---------- | ----- | ------ |
| NC-159-01 | Blocker | §4 `FF_TblValidateFn_t` | Type name does not exist (actual: `FF_TblValidateFn`) | Rename to `FF_TblValidateFn` | authoring agent | open |
| NC-159-02 | Major | §4 `FF_TblValidate` | Listed function does not exist; validation is internal to `FF_TblLoad` | Remove from list; document validation via `validate` callback in `FF_Tbl_Def_t` | authoring agent | open |
| NC-159-03 | Major | §7.1 & §8 link backends | Backends attributed to `link.c`; actual files are `link_loopback.c`/`link_uart.c` | Correct file references | authoring agent | open |
| NC-159-04 | Major | §1 init sequence | Omits `FF_BoardInit`, `FF_MissionTablesInit`, `FF_LinkInit`, provider hooks; `FF_PusInit` mis-grouped | Rewrite §1 to match `fsw_main.c` 72–98 | authoring agent | open |
| NC-159-05 | Minor | §1 `FF_AppConfig_t` field | "stack" → `stack_bytes` | Correct field name | authoring agent | open |
| NC-159-06 | Minor | §3 `FF_EvsEvent` | → `FF_EvsEvent_t` | Add `_t` suffix | authoring agent | open |
| NC-159-07 | Minor | §9 `test_gem_tsn.c` count | 16 → 22 | Correct check count | authoring agent | open |

New NCs opened by this R4 review (to be closed by re-applying all rules, norm N12):

| NC | Severity | Manual section / Item | Finding | Correction | Owner | Status |
| -- | -------- | --------------------- | ------- | ---------- | ----- | ------ |
| NC-DOC-001 | Major | §8 line 224 | "formal verification depth is GAP-A4" mislabels the gap (should be GAP-A1 per software-description.md §3.7 / class-a-gaps.md) | Align gap label to GAP-A1 | authoring agent | open |
| NC-DOC-002 | Minor | §9.1 & §9 table | `FF_TsnBackend_t` documented with 4 methods; omits `set_stream_filter` (Qci) and `enable_pfc` (Qbb) | Document the two omitted vtable methods | authoring agent | open |
| NC-DOC-003 | Minor | §8.1 & §8 table | "address = 6 octets" presented as a universal contract rule, inconsistent with the loopback reference backend (1-byte port in `addr[0]`) | Qualify addressing as backend-specific | authoring agent | open |
| NC-DOC-004 | Minor | §1 line 26 | `FF_PalTaskFunc_t` pointed to `src/apps/app.h`; defined in `src/core/pal/pal.h` | Correct the reference | authoring agent | open |
| NC-DOC-005 | Minor | §3 line 76 | Event→PUS-S5 mapping misattributed to `TM_OUTPUT` | Attribute to `ff_event_listener` via `FF_EvsSetListener` | authoring agent | open |
| NC-DOC-006 | Minor | §8 table & §8.1 | lwIP adapter cites only `ff_net_lwip.{c,h}`; omits `ff_lwip_netif.{c,h}` | Add the netif glue files | authoring agent | open |

**Disposition flow (norm N12):** each NC is resolved by correcting the manual, then
the documentation review loop (norm N17) re-reviews the delta until this report is
APPROVED or APPROVED WITH COMMENTS with all NCs closed. The four Comment items
(DOC-COMMENT-014…017) are corrected alongside for completeness.

---

Committed as a review record under `docs/reviews/` with the reviewed change (norm N11). This report does not by itself alter FRICK-ENG-0005 rev 0.6; the manual must be revised to the next revision (N16) and re-reviewed to APPROVED/APPROVED WITH COMMENTS with all NCs closed before it can serve as an integration contract.

**Confidence:** High — every finding is anchored to an exact file path and line in
the repository baseline of 2026-10-13.
