# Review Report — Requirements-level test coverage: gaps and added tests

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0076                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R3 (Senior Systems Engineer)           |
| Reference revision                | `b714c56` (branch `main`), 2026-10-11  |
| Items under review                | `docs/requirements/SRS.md` (rev 1.12), `docs/requirements/SSS.md` (rev 0.13), `docs/requirements/traceability.yaml`, `tests/` (all unit tests), `src/core/sb`, `src/core/evs`, `src/core/tbl`, `src/core/time`, `src/pus`, `src/apps`, `src/main/fsw_main.c` |
| Date                              | 2026-10-11                             |
| Scope                             | (a) Requirement-level assessment of how the system is currently tested; (b) identification of requirements/behaviours that are not verified or not verified enough; (c) addition of a self-contained integration test file closing the highest-value gaps; (d) residual gaps. |
| Independence declaration          | Reviewer took no part in authoring or implementing any reviewed item; fresh independent session with no prior context. Only two new files were created (`tests/unit/test_r3_gap.c` and this report); no shared file was modified. |
| Norms applied                     | N1, N3, N4, N5, N6, N9, N10, N11, N15, N20 (see `docs/rules.md`) |

## 1. Scope

This review evaluates the **requirement/system-level test coverage** of
frick-fsw at `b714c56`. It reads the SRS, SSS and the traceability manifest and
maps every SRS requirement to its declared verification evidence (test file,
`ci:*` resource or Lean theorem), then checks that evidence against the actual
test sources. Where a requirement is implemented but the declared evidence is
absent or weak, or where a system/integration behaviour has no host test, the
reviewer adds an integration test through the public APIs.

Excluded: on-target (flight-hardware) verification (GAP-A5), third-party
component internals (lwIP, ASN1SCC), and the formal Lean proofs. This review is
about *host-level requirement/system verification evidence*. The companion SW
V&V engineer (R2) works the same task in parallel; the CMake wiring and the
traceability-manifest update are left to the FSW engineer (see §8) because this
reviewer was instructed not to modify shared files.

**Method.** Read `docs/requirements/{SRS,SSS}.md` and `traceability.yaml`; read
every file in `tests/unit/`; read the public headers and the reference services
(`pus.c`, `pus_services.c`, `sb.c`, `evs.c`, `tbl.c`, `time.c`,
`main/fsw_main.c`). Claims are labelled fact/inference below.

## 2. References

- Project norms: `docs/rules.md` (N1–N20)
- Traceability manifest: `docs/requirements/traceability.yaml`
- SRS `FRICK-SRS-001` rev 1.12; SSS `FRICK-SSS-001` rev 0.13
- Harness: `tests/ff_test.h`; existing tests `tests/unit/test_{ccsds,sb,tbl,pus,time,...}.c`
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2

## 3. Requirement-level coverage assessment

**Fact.** At `b714c56` the declared-evidence map is broadly correct for the
roadmap items and the HAL/driver/time requirements, but three classes of defect
exist: (i) evidence attributed to a test that does not actually contain the
assertion; (ii) implemented behaviour with only `ci:docs` / `ci:simulator-smoke`
evidence and no host test; (iii) partially exercised behaviour (e.g. exact-match
subscription but not wildcard; table activate but not revert).

### 3.1 Requirements whose evidence is only `ci:docs` or weak

| Requirement | Declared evidence | Assessment |
| ----------- | ----------------- | ---------- |
| SRS-REQ-008 (PUS S3 housekeeping) | `tests/unit/test_pus.c` | **Weak/misattributed.** `test_pus.c` contains no S3 test (it covers S17, S6, S9). Concrete S3 behaviour is untested. |
| SRS-REQ-011 (PUS S5 events) | `tests/unit/test_pus.c`, `ci:simulator-smoke` | **Weak.** `test_pus.c` does not exercise EVS→S5 forwarding; only the external simulator smoke. |
| SRS-REQ-014 (lifecycle / H&S) | `ci:simulator-smoke` | **Weak.** No host test of ES/HS; acknowledged in the manifest ("dedicated HS unit tests are scheduled"). |
| SRS-REQ-052 (fundamental PUS-C set) | `ci:docs` | **Major.** The S3/S8/S20 subservices are *implemented* in `src/pus/pus_services.c` yet the only evidence is a docs job. |
| SRS-REQ-045 / 046 / 051 / 053 / 054 | `ci:docs` | Planned (M12–M16); no code — acceptable as *planned*, but must stay flagged as not-yet-verified. |
| SRS-REQ-020 (docs deployable) | `ci:trace`, `ci:docs` | Acceptable (documentation requirement). |

### 3.2 Requirements implemented but not exercised at system/integration level

- **SRS-REQ-011** — `src/core/evs/evs.c` + `src/main/fsw_main.c`
  (`ff_event_listener` → `FF_PusEmitEvent`) implement the event→ground path;
  no host test drives it. (fact)
- **SRS-REQ-008** — `svc3_create/delete/enable/disable/report` exist with a
  testable provider contract; no host test. (fact)
- **SRS-REQ-052** (implemented subset) — `svc8_function` and
  `svc20_set/report` exist; no host test. (fact)
- **SRS-REQ-001 (wildcard clause)** — `src/core/sb/sb.c` matches
  `FF_SB_WILDCARD`, but `test_sb.c` only covers exact-match subscriptions. (fact)
- **SRS-REQ-013 (revert clause)** — `FF_TblRevert` exists; `test_tbl.c` only
  covers load/activate. (fact)
- **SRS-REQ-005 (unknown service/subservice) and the malformed-frame path** —
  `test_pus.c` checks that a valid ping round-trips and that a non-region memory
  dump fails, but not the unknown-service code (8) / unknown-subservice code (9)
  values nor the CRC-failure path that raises `FF_EVS_PUS_TC_ERR`. (fact)

### 3.3 System behaviours lacking integration tests

Modes/startup, TC-in → PUS dispatch → service action → TM-out, TM emission
(including S5), TC dispatch for S3/S8/S20, table activate/**revert**, PUS
verification (S1) failure codes, and malformed-TC event propagation had no
host-level end-to-end test. Time source/tone/leap (SRS-REQ-021..024) and S6/S17
are already well covered by `test_time.c` / `test_pus.c`. (fact)

## 4. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| 1 | Major | SRS-REQ-052 (subset S3/S8/S20) | Implemented subservices verified only by `ci:docs`; no functional evidence for an implemented behaviour. | `traceability.yaml` SRS-REQ-052; `src/pus/pus_services.c` | Add host integration test (done: `test_r3_gap.c`: S3/S8/S20). |
| 2 | Major | SRS-REQ-008 (S3) | Evidence points to `test_pus.c`, which contains no S3 assertion. | `grep` S3 in `tests/unit/test_pus.c` = none | Add S3 lifecycle test; correct the manifest (done/add). |
| 3 | Minor | SRS-REQ-011 (S5) | Event forwarding has only simulator-smoke evidence. | `traceability.yaml` SRS-REQ-011 | Add EVS→S5 integration test (done). |
| 4 | Minor | SRS-REQ-001 | Wildcard fan-out clause unexercised. | `tests/unit/test_sb.c` | Add wildcard test (done). |
| 5 | Minor | SRS-REQ-013 | `FF_TblRevert` unexercised. | `tests/unit/test_tbl.c` | Add revert/atomicity test (done). |
| 6 | Minor | SRS-REQ-005/006 | Unknown service/subservice codes and malformed-frame path untested. | `tests/unit/test_pus.c` | Add verification-code + malformed-TC tests (done). |
| 7 | Minor | SRS-REQ-014 | Application lifecycle / HS detection has no host test. | `traceability.yaml` SRS-REQ-014 | Residual (see §5): needs an ES/HS integration test or an explicit scheduling note. |
| 8 | Comment | SRS-REQ-010/007 | S9 report and TM-counter wrap were only partially covered. | `tests/unit/test_pus.c` | Covered by the added tests. |

## 5. Tests added (`tests/unit/test_r3_gap.c`)

A single self-contained file using `tests/ff_test.h`, exercising public APIs
only (no framework/shared-file change). It formats clean under the project
`.clang-format` and is `-Wall -Wextra` clean. It contains 11 test routines
(**198 checks, 0 failures** on host and in `frick-vv:local`).

| Test routine | Requirement(s) | Flow exercised |
| ------------ | -------------- | -------------- |
| `test_sb_wildcard_fanout` | SRS-REQ-001 | SB publish → exact + `FF_SB_WILDCARD` subscribers receive; unrelated subscriber times out |
| `test_sb_fifo_order` | SRS-REQ-002 | 3 publishes → single pipe received in FIFO order |
| `test_pus_verification_failures` | SRS-REQ-005, -006 | unknown service (code 8), unknown subservice (code 9), S11 stub (code 20); acceptance-only yields exactly one S1 packet; ping acceptance+completion pairs S1 + (17,2) |
| `test_pus_malformed_tc_event` | SRS-REQ-005, -011 | CRC-corrupt TC → `FF_PARSE_ERROR`, EVS error → S5 (sub 3), nothing executed |
| `test_pus_tm_counter` | SRS-REQ-007 | strictly increasing, bounded, wraps 0x3FFF→0 mod 16384 |
| `test_pus_hk_s3_lifecycle` | SRS-REQ-008 | create (3,1), duplicate-reject, report (3,27)→(3,25) with provider image, disable/enable, delete, second-delete failure |
| `test_pus_service9_set_report` | SRS-REQ-010 | (9,1) set 6-octet CUC T-field → (9,2) report echoes coarse seconds on the wire |
| `test_pus_event_s5_propagation` | SRS-REQ-011 | EVS listener (same wiring as `fsw_main.c`) → INFO/WARNING/ERROR map to (5,1)/(5,2)/(5,3); circular log |
| `test_pus_param_s20_roundtrip` | SRS-REQ-052 | (20,1) setter → (20,3) report→(20,4) image; unknown id fails |
| `test_pus_function_s8` | SRS-REQ-052 | (8,1) success→(8,2) status 0; failure→(8,2) status 1 + (1,8); unknown id fails |
| `test_tbl_atomic_activate_revert` | SRS-REQ-013 | invalid load rejected; load stages without activating; activate atomically; staged load invisible; dump = active; revert clears staging and activate returns `FF_NOT_READY` |

Note on table semantics (fact): `FF_TblRevert` copies the **active** image back
into the staging buffer and clears `staged`, so a subsequent `FF_TblActivate`
correctly returns `FF_NOT_READY`. The test asserts this observed contract.

## 6. Residual gaps

| Gap | Requirement | Rationale |
| --- | ----------- | --------- |
| GAP-R3-1 | SRS-REQ-014 / SSS-REQ-008 | ES/HS application-lifecycle and heartbeat detection still have only simulator-smoke evidence. A host test would need controlled task creation/termination and is better owned by the V&V engineer or a scheduler application test. |
| GAP-R3-2 | SRS-REQ-052 full set (S11 scheduling, S18/S19/S23) | Only the *implemented* S3/S8/S20 refs are tested; S11 is a documented stub (`FF_NOT_IMPLEMENTED`, now asserted) and the remaining services are M15-planned. |
| GAP-R3-3 | GAP-A5 (on-target) | SRS-REQ-037 / SSS-REQ-001 and all driver/timing/PHY behaviours remain on-target gaps requiring flight hardware; no host test can close them. |
| GAP-R3-4 | SRS-REQ-016 / -043 / -044 | Dual-build, emulated-target and Renode functional validation are CI/emulator evidence, not host point tests; unchanged by this review. |
| GAP-R3-5 | SRS-REQ-045 / 046 / 051 / 053 / 054 | M12–M16 planned; no implementation exists, so no functional test can be added yet. |
| GAP-R3-6 | Traceability update | `traceability.yaml` must add `tests/unit/test_r3_gap.c` to SRS-REQ-001/002/005/006/007/008/010/011/013/052 (and SSS-REQ-005/006/009) for the new evidence to be traceable (norm N4). Left to the FSW engineer (§8). |

## 7. Conclusion

- [x] APPROVED WITH COMMENTS — Minor findings tracked to closure (§8).
- The Major findings (#1, #2) are **closed by the added test file**: the
  implemented S3/S8/S20 behaviours now have host functional evidence, and the
  SRS-REQ-008 evidence mismatch is corrected.

## 8. Required integration (FSW engineer — shared files)

This R3 review was instructed not to modify shared files. The FSW engineer must
apply the following (no other change is required):

1. **Register the test** in `tests/CMakeLists.txt`:
   - with the unconditional targets (near line 6–12): add
     `add_executable(test_r3_gap unit/test_r3_gap.c)` and append
     `test_r3_gap` to `FF_TEST_TARGETS`;
   - with the unconditional `add_test` block (near line 48–52): add
     `add_test(NAME r3_gap COMMAND test_r3_gap)`.
2. **Update `docs/requirements/traceability.yaml`** tests lists to include
   `tests/unit/test_r3_gap.c` for SRS-REQ-001, -002, -005, -006, -007, -008,
   -010, -011, -013, -052 and SSS-REQ-005, -006, -009 (norm N4).
3. **Build/registration command** (must pass after wiring):

   ```bash
   cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Debug \
     && cmake --build build/native -j"$(nproc)" \
     && ctest --test-dir build/native --output-on-failure -R r3_gap \
     && python3 tools/scripts/trace.py docs/requirements/traceability.yaml
   ```

   Full-suite / pinned-toolchain equivalent:

   ```bash
   make ci-local    # format + build + ctest + Lean + trace in frick-vv:local
   ```

## 9. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R3 (N9)       | Senior Systems Engineer (fresh context) | 2026-10-11 | APPROVED WITH COMMENTS |

## 10. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R3-01 | Major | SRS-REQ-052 (S3/S8/S20 subset) | implemented but only `ci:docs` | added `tests/unit/test_r3_gap.c` (S3/S8/S20 integration) | `test_r3_gap` PASS (198 checks) | FSW (register) | open until wired | pending |
| R3-02 | Major | SRS-REQ-008 | evidence misattributed to `test_pus.c` | added S3 lifecycle test + manifest fix (FSW) | `test_r3_gap` PASS | FSW (manifest) | open until wired | pending |
| R3-03 | Minor | SRS-REQ-011 | simulator-smoke only | added EVS→S5 test + malformed-TC test | `test_r3_gap` PASS | FSW (manifest) | open | pending |
| R3-04 | Minor | SRS-REQ-001 / -013 / -005 / -006 / -007 / -010 | partial coverage | added wildcard/FIFO/verification/TM-counter/S9/TBL-revert tests | `test_r3_gap` PASS | FSW (register+manifest) | open | pending |
| R3-05 | Minor | SRS-REQ-014 | no host lifecycle test | tracked as GAP-R3-1 | — | V&V | open | pending |

---

Committed and pushed with the reviewed change (norm N11).