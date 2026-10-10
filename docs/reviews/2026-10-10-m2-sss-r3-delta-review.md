# Independent SSS/SRS Delta Re-Review - Milestone M2 (Time Base & CUC, profile R3)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0011                      |
| Reviewer session                  | independent agent (subagent, norm N9)  |
| Reviewer profile                  | R3 (Senior Systems Engineer)           |
| Reference revision                | working tree on top of commit `a69732f` (base), milestone M2 (uncommitted) |
| Items under review                | `docs/requirements/SSS.md` (rev 0.3, SSS-REQ-007/016), `docs/requirements/SRS.md` (rev 0.3, SRS-REQ-021..024), `docs/requirements/traceability.yaml|md`, `docs/interfaces/ICD.md` (rev 0.3, §4/§5), `docs/architecture/time-and-timecode.md` (rev 0.3, §1.1/§2/§4/§5), `docs/quality/ecss-criticality-b.md` (rev 0.2), `docs/quality/class-a-gaps.md` (rev 0.2), `src/pus/pus_services.c` (S9), `tests/unit/test_pus.c`, `tests/unit/test_time.c` |
| Date                              | 2026-10-10                             |
| Scope                             | Delta re-review (norm N12/N14) of NC-1..NC-5 from `2026-10-10-m2-sss-r3-review.md`: independent re-application of the rules to the changed scope only |
| Predecessor report                | `docs/reviews/2026-10-10-m2-sss-r3-review.md` (FRICK-REVIEW-0006, REJECTED - NC-1 Major) |
| Independence declaration          | The reviewer took no part in authoring or executing the fixes; fresh context; every statement below is based on the repository contents and independent re-runs on the working tree. |
| Norms applied                     | N1-N17 (see `docs/rules.md`) |

## 1. Scope

This report verifies **only** the five non-conformances (NC-1..NC-5) raised by
the predecessor R3 review and their claimed dispositions. It does not re-audit
the whole M2 code base, the R1 code review or the R2 V&V review (recorded
closed in their own reports). Excluded as before: on-target PPS capture
(GAP-A7), hardware bring-up (GAP-A5), WCET/stack analysis (GAP-A4) and the
formal C-embedding (GAP-A1).

Independent re-runs performed in this session (no file was modified except this
report):

| Activity | Command / artefact | Result |
| -------- | ------------------ | ------ |
| Traceability gate | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** - 48 requirements (25 srs / 16 sss / 7 gap), **41** Lean theorems, `CI evidence claims: 9`, exit 0 |
| Build | `cmake --build build/native -j$(nproc)` | up to date / no errors |
| Full unit suite | `ctest --test-dir build/native --output-on-failure` | **100% tests passed, 0 failed of 5** (ccsds, sb, tbl, pus, time) |
| NC-1 focused suite | `ctest --test-dir build/native -R "pus|time"` | both `Passed` (the `test_service9_time` 6-octet-accept / 7-octet-reject case runs green) |
| Lean theorems | `grep` of `verification/lean/Frick/Requirements.lean` + gate link check + committed `verification/lean/lake-build.log` | all 10 linked `req_srs021_*`/`022_*`/`023_*`/`024_*` theorems exist as `theorem` declarations; transcript `Build completed successfully` (17/17). `lake` is not installed in this environment, so the build was not re-run (same limitation as the R2 delta review) |

## 2. References

- Project norms: `docs/rules.md` (N1-N17), `docs/engineering/independent-review.md` (profile R3), `docs/reviews/review-template.md`.
- Predecessor review: `docs/reviews/2026-10-10-m2-sss-r3-review.md` (FRICK-REVIEW-0006).
- Requirements & traceability: `docs/requirements/SSS.md`, `SRS.md`, `traceability.yaml`, `traceability.md`.
- Architecture & interfaces: `docs/architecture/time-and-timecode.md` (rev 0.3), `docs/interfaces/ICD.md` (rev 0.3), `docs/quality/class-a-gaps.md`, `docs/quality/ecss-criticality-b.md`.
- Implementation & evidence: `src/pus/pus_services.c`, `src/pus/pus.c`, `src/transport/ccsds/ccsds.c`, `src/pus/pus_types.h`, `tests/unit/test_pus.c`, `tests/unit/test_time.c`, `verification/lean/Frick/Requirements.lean`, `verification/lean/lake-build.log`.
- Standards: ECSS-E-ST-40C, ECSS-E-ST-70-41C (PUS-C), **CCSDS 301.0-B-4** (P-field 0x2E verified in the predecessor review, section 3.2.2).

## 3. NC-by-NC verification (norm N12)

| NC | Severity | Claimed disposition | Independent verification | Status |
| -- | -------- | ------------------- | ------------------------ | ------ |
| **NC-1** | Major | Two CUC exchange forms documented: self-describing 7-octet P+T (`FF_TimeCucEncode/Decode`, P=0x2E) and PUS-C on-wire 6-octet T with implicit P (TM secondary header + service 9); `svc9_set_time` rejects `len != 6` with `FF_BAD_SIZE` | **Contract is now single-valued and matches the code.** (i) ICD rev 0.3 §5 (`ICD.md:75-119`) defines form 1 as the **6-octet PUS-C on-wire T-field, P-field implicit**, "the form inside the TM secondary header and the PUS service 9 payloads", and form 2 as the **explicit 7-octet P+0x2E+T** produced only by `FF_TimeCucEncode/Decode`; §4 (`ICD.md:68`) lists service 9,1/9,2 as "6-octet CUC T-field (P-field implicit)". (ii) `time-and-timecode.md:147-210` states the same two forms, and §5.2 (`:202-204`) locates the 6-octet T at offsets 7..12 of the 13-byte TM secondary header. (iii) SRS-REQ-024 rev 0.3 (`SRS.md:106-113`) defines forms (a) 7-octet self-describing (`FF_TIME_CUC_LEN`, reject wrong length/P-field) and (b) 6-octet on-wire T-field accepted "only in its exact 6-octet length". (iv) `pus_services.c:338-352` rejects any length other than 6 (`if (len != 6u) return FF_BAD_SIZE;`) before parsing `d[0..3]`/`d[4..5]`; `svc9_report_time` (`:354-366`) emits exactly 6 octets. (v) `ccsds.c:177-178` serialises `sec[7..10]`=coarse, `sec[11..12]`=fine with `FF_PUS_TM_SEC_HDR_LEN 13` (`pus_types.h:71`), i.e. **no P byte on the wire**. (vi) Regression test `test_pus.c:143-187` (`test_service9_time`): a valid 6-octet T-field is accepted and sets CUC coarse to 5, while the documented 7-octet P-prefixed input returns `FF_BAD_SIZE`, emits completion failure (1,8) and leaves the CUC unchanged (`coarse == 5u`). The suite passes. A repo-wide grep found **no residual claim** that the 7-octet form is carried on the wire or used by service 9. | **CLOSED** |
| **NC-2** | Minor | SRS-REQ-021 reworded: switchable sources + explicit switch semantics + boundary non-regression | `SRS.md:87-94` now separates the two properties and defines the switch: "Selecting a source is an explicit authority change: after the switch the exported CUC is continuous with the newly selected source and **may step by at most one second at the switch instant**; across a tone boundary the exported CUC seconds shall not regress under a non-negative scheduled leap change." The architecture gives the analysis for the <=1 s bound (`time-and-timecode.md:63-73`: all sources derive from the same PAL UTC offset; TONE carries the +1 s first-edge convention), and `test_source_selection` (`test_time.c:128-138`) exercises runtime switching plus out-of-enum rejection. Lean `req_srs021_utc_non_regression` is retained for the within-source boundary guarantee. The previously unspecified switch discontinuity is now explicitly bounded. | **CLOSED** |
| **NC-3** | Minor | SRS-REQ-022 "tone second counter" wording fixed | `SRS.md:95-100`: "each valid 1 Hz tone shall advance the **tone second counter (the disciplined second epoch)** by exactly one second and strictly increase the tone epoch ... Under TONE mode the exported CUC coarse seconds shall equal the tone second counter **plus the accumulated leap offset** (SRS-REQ-023); the first tone establishes the first disciplined second boundary (`latched correlation + 1 s`)." The exported CUC is now explicitly distinguished from the counter, so the "exactly one second" claim cannot be misread (the exported CUC steps by two across a boundary carrying a +1 leap, as `test_leap_at_tone_boundary` asserts). Mirrored in `time-and-timecode.md:94-98` and discharged by `req_srs022_tone_second_advance`/`req_srs022_tone_count_strict`. | **CLOSED** |
| **NC-4** | Minor | SRS-REQ-023 no-tone clause added; `test_leap_pending_without_tone` added | `SRS.md:101-105`: "a pending leap change (single second at a time) shall be inert: it shall never alter the exported CUC while pending, and shall be applied and cleared exactly at the first valid tone that follows; **if no valid tone ever arrives the pending leap shall remain pending and shall never be applied**." Documented in `time-and-timecode.md:139-142`. Test present and registered: `test_time.c:204-224` schedules +1, asserts the pending value is 1, `leap_seconds == 0` and `coarse == 0` with no tone, then that the first tone applies/clears it; listed in `FF_TEST_MAIN` (`test_time.c:285-288`). The `time` suite passes. | **CLOSED** |
| **NC-5** | Minor | `ecss-criticality-b.md` references GAP-A1..A7 | `ecss-criticality-b.md:10` now reads "`docs/quality/class-a-gaps.md` (GAP-A1..A7)"; rev 0.2 history (`:75`) records the NC-5 update. Consistent with `class-a-gaps.md:10` (tracking GAP-A1..A7) including the §GAP-A7 entry (`:73-82`) and `traceability.yaml:415-421`. | **CLOSED** |

**Result: 1 Major + 4 Minor = 5/5 NCs closed.**

Additional cross-checks made while re-applying the rules:

- The traceability gate is bidirectionally converged: SSS-REQ-016 still has
  exactly the four children SRS-REQ-021..024 (`traceability.yaml:212-242`,
  `traceability.md:41`), and every implementation/test/Lean link resolves.
- The predecessor **Comment F-8** (count wording) was also addressed:
  `traceability.md:18-20` now states "CI evidence claims = 9 unique workflow
  claim names (8 referenced by the manifest; `ci-lean` ... not claimed)",
  matching the tool's `CI evidence claims: 9`.
- The Lean theorems named by the requirement manifest are real `theorem`
  declarations, not comments (`Requirements.lean:106-149`), and the committed
  transcript reports a clean 17/17 rebuild.

## 4. New findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| N-1 | Comment | ICD rev 0.3 §5.1 / §3 | Residual wording ambiguity in the corrected CUC section: ICD §5.1 says the T-field occupies "**user-data** offsets 7..12", while `time-and-timecode.md:202` says "byte offsets 7..12" and ICD §3 uses "user data" for the bytes *after* the PUS secondary header. The parenthetical "(coarse 7..10, fine 11..12)" plus "the TM secondary header is 13 bytes" makes it recoverable, so no contract risk remains. The same §3 PDL note still says "PUS secondary header (6)", which matches neither the implemented TC (`FF_PUS_TC_SEC_HDR_LEN 5`) nor TM (`FF_PUS_TM_SEC_HDR_LEN 13`) length; that line is unchanged from base `a69732f` (pre-existing, out of the NC-1 delta). | `ICD.md:90-91`, `ICD.md:47-48`, `time-and-timecode.md:202-204`, `pus_types.h:70-71`, `ccsds.c:177-178` | Reword to "secondary-header byte offsets 7..12 (packet-data offsets 7..12)"; and reconcile the §3 secondary-header/PDL note with the `FF_PUS_*_SEC_HDR_LEN` values in a later docs pass. Non-derogatory. |
| N-2 | Comment | SSS-REQ-016 | SSS-REQ-016 says the tailoring "shall pin a 4+2 octet agency-epoch frame **(P-field 0x2E)** for the on-wire forms", which can still be read as the P byte being present on the wire - the exact confusion NC-1 removed. Intent is clearly to identify the tailoring. | `SSS.md:70-71`; `SRS.md:106-113`; `ICD.md:81-94` | Reword to "a 4+2 octet agency-epoch frame *identified by* P-field 0x2E for the on-wire forms". Non-derogatory. |
| N-3 | Comment | Norm N11 hygiene | The fixes, the regenerated evidence and this report are still in the **uncommitted** working tree on `a69732f` (`git status`: `M docs/.../SRS.md`, `M docs/interfaces/ICD.md`, `M src/pus/pus_services.c`, `M tests/unit/test_pus.c`, ...). The NCs close in the record only when the whole cycle, including this report and the `docs/reviews/README.md` log row, is committed and pushed to both remotes together. | `git status --short`; `docs/reviews/README.md` (no R3 delta row yet) | Commit/push the closure cycle as a single change (N7/N11) and add the review-log row. |

**Severity definition** (per template): Major = non-conformity that disqualifies
the item for the applicable criticality, or a defect that can defeat a
requirement or a safety barrier; Minor = must be corrected or justified but does
not invalidate the requirement; Comment = observation, no derogation.
N-1..N-3 are all **Comments**: none blocks closure of NC-1..NC-5.

## 5. Conclusion

- [x] **APPROVED** - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

Rationale: the Major wire-format non-conformity (NC-1) is resolved at the
contract, documentation and code levels: the ICD, architecture, SRS and
service 9 code all now distinguish the self-describing 7-octet P+T code from
the PUS-C 6-octet implicit-P T-field, and `svc9_set_time` rejects any non-6-octet
input with `FF_BAD_SIZE` - independently confirmed by the green
`test_service9_time` case (6-octet accepted, 7-octet P-prefixed input rejected
with no state change). The four Minor NCs are each closed by a concrete
requirement/architecture edit supported by test or Lean evidence (NC-2 switch
semantics, NC-3 tone second counter, NC-4 no-tone clause + new test, NC-5
GAP-A1..A7 reference). The traceability gate passes (48 requirements, 41 Lean
theorems, 9 CI claims) and the full unit suite is green. The only residual items
are non-blocking documentation/wording observations (N-1..N-3). Under norm N14
the SSS/SRS/ICD delta is now compliant; the predecessor R3 rejection is
**superseded**.

## 6. Sign-off

| Role          | Agent / Reviewer                | Date       | Conclusion |
| ------------- | ------------------------------- | ---------- | ---------- |
| R3 (N9/N14)   | independent subagent, profile R3 | 2026-10-10 | APPROVED   |

## 7. Follow-up records (norm N12)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | ------ | --------------- |
| NC-1 | Major | ICD §5; SRS-REQ-024; `pus_services.c` S9; `pus_types.h`; `ccsds.c` | CUC wire-format contradiction: 7-byte explicit P+T documented vs 6-byte implicit-P implemented; documented 9,1 input silently mis-parsed | Two exchange forms documented (explicit 7-octet self-describing vs PUS-C 6-octet implicit-P); `svc9_set_time` rejects `len != 6`; regression test added; ICD §4/§5 + arch §5 + SRS-REQ-024 aligned | `ICD.md:68,75-119`; `time-and-timecode.md:147-210`; `SRS.md:106-113`; `pus_services.c:338-352`; `ccsds.c:177-178`; `pus_types.h:71`; `test_pus.c:143-187`; ctest `pus` Passed; gate PASS | frick-fsw (M2 owner) | **closed** | done (R3, independent; see N-1) |
| NC-2 | Minor | SRS-REQ-021 | Bundled clause; source-switch behaviour of exported CUC unspecified/possibly regressing | Split clause; explicit authority-change switch semantics with a <=1 s step bound; boundary non-regression retained | `SRS.md:87-94`; `time-and-timecode.md:63-73`; `test_time.c:128-138`; `req_srs021_utc_non_regression` | frick-fsw | **closed** | done (R3, independent) |
| NC-3 | Minor | SRS-REQ-022 | "second counter" ambiguity vs exported CUC (steps 2 at a leap boundary) | "tone second counter (disciplined second epoch)" defined; exported CUC = counter + leap offset stated | `SRS.md:95-100`; `time-and-timecode.md:94-98`; `req_srs022_tone_second_advance`, `req_srs022_tone_count_strict` | frick-fsw | **closed** | done (R3, independent) |
| NC-4 | Minor | SRS-REQ-023 | No-tone configuration undefined (leap never applied/cleared) | No-tone clause added (pending stays pending, never applied); `test_leap_pending_without_tone` added | `SRS.md:101-105`; `time-and-timecode.md:139-142`; `test_time.c:201-224,285-288`; ctest `time` Passed | frick-fsw | **closed** | done (R3, independent) |
| NC-5 | Minor | `ecss-criticality-b.md` | Stale "GAP-A1..A6" reference, GAP-A7 missing | Updated to GAP-A1..A7 | `ecss-criticality-b.md:10,75`; `class-a-gaps.md:10,73-82`; `traceability.yaml:415-421` | frick-fsw | **closed** | done (R3, independent) |

---

Committed and pushed with the reviewed change (norm N11).
