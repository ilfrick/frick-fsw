# Review Report - M2 SSS/SRS Time Base & CUC (profile R3)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0006                       |
| Reviewer session                  | independent agent (subagent, norm N9)  |
| Reviewer profile                  | R3 (Senior Systems Engineer)           |
| Reference revision                | working tree on top of commit a69732f (base), milestone M2 (uncommitted) |
| Items under review                | `docs/requirements/SSS.md` (SSS-REQ-016), `docs/requirements/SRS.md` (SRS-REQ-021..024), `docs/requirements/traceability.yaml|md`, `docs/architecture/time-and-timecode.md` rev 0.2, `docs/interfaces/ICD.md` rev 0.2 (CUC), `docs/quality/class-a-gaps.md` (GAP-A7), `tests/unit/test_time.c`, `verification/lean/Frick/Time.lean`, `verification/lean/Frick/Requirements.lean`, `src/core/time/{time,pps}.{c,h}`, `src/pus/pus_services.c` (S9) |
| Date                              | 2026-10-10                             |
| Scope                             | Requirement quality (ECSS-E-ST-40C), SSS->SRS decomposition, verification-method consistency, leap-second wording, CUC conformance to CCSDS 301.0-B-4, GAP-A7 class-A scoping; interface consistency with the ICD and the PUS/CCSDS profile |
| Independence declaration          | The reviewer took no part in authoring/executing the change; fresh context; every finding is based solely on the repository contents and the applicable reference standards (CCSDS 301.0-B-4 text checked directly). |
| Norms applied                     | N1-N16 (see `docs/rules.md`) | 

## 1. Scope

Reviewed the milestone-M2 time-base delta on top of base commit `a69732f`:

- SSS-REQ-016 and SRS-REQ-021..024 for completeness, unambiguity, verifiability
  and absence of implementation over-specification (ECSS-E-ST-40C requirement
  quality), for the target platform RTPFS160ZT / RTEMS 6.2 (ECSS criticality B).
- Coherence of the SSS->SRS parent/child structure (`traceability.yaml`,
  `tools/scripts/trace.py` gate) and the human-readable matrices.
- Consistency of the stated verification methods (`Tst | Anl`) with
  `tests/unit/test_time.c` and the Lean theorems in `Frick/Time.lean` and
  the wrappers in `Frick/Requirements.lean` linked in the manifest.
- Leap-second scheduling wording (must not be read as mid-second) and the
  CUC words against **CCSDS 301.0-B-4** (verified against the standard text,
  section 3.2.2 of `ccsds.org/Pubs/301x0b4e1.pdf`).
- Scope and correctness of GAP-A7 as a class-A gap (not an NC for class B).
- Interface consistency of the CUC with the ICD and with the implemented
  PUS service 9 handlers and the TM secondary header.

Excluded from this review: the simulator/RTE behaviour of the software PPS
backend beyond its test evidence, on-target hardware bring-up (GAP-A5/A7),
and WCET/stack analysis (GAP-A4).

### Items verified with no finding (evidence recorded)

1. **CUC P-field agrees with CCSDS 301.0-B-4 §3.2.2.** The standard defines
   Octet 1 of the CUC P-field as: Bit 0 = extension flag (`0`); Bits 1-3 =
   time code identification (`010` = agency-defined epoch, Level 2); Bits 4-5 =
   *number of octets of the basic time unit minus one*; Bits 6-7 = *number of
   octets of the fractional time unit* (no minus one). For `0x2E = 0b00101110`:
   bits 1-3 = `010` (agency-defined epoch), bits 4-5 = `11` -> 4 coarse octets,
   bits 6-7 = `10` -> 2 fine octets, fixed 7-octet code. The wording in
   `time-and-timecode.md` §5.1, the ICD §5 table and SRS-REQ-024 is therefore
   faithful to the standard (the fine field is NOT coded as "octets minus one",
   and the docs correctly encode coarse as 3 -> 4 and fine directly as 2).
   `fine = 2^-16 s ~ 15.26 us` and the conversion formulas are arithmetically
   correct (verified against `time.c`, `FF_TimeGetCuc`).
2. **SSS->SRS decomposition is coherent.** `make trace` (norm N4 gate) passes:
   every SRS has exactly one SSS parent, every SSS at least one child;
   **SSS-REQ-016 has exactly children SRS-REQ-021, -022, -023, -024**, and no
   SRS-REQ-021..024 parents any other SSS. Top-down matrix in
   `traceability.md` matches the manifest. Counts are internally consistent
   (25 SRS, 16 SSS, 7 GAP, 41 Lean theorems).
3. **Methods Tst | Anl are consistent with the evidence.** `test_time.c`
   covers source switching and rejection of out-of-enum values (SRS-REQ-021),
   tone second-advance/strict-epoch/rejection (SRS-REQ-022), leap
   schedule/apply/clear at boundary (SRS-REQ-023), CUC encode/decode round trip,
   fixed `{0x2E,0,0,0,5,0x40,0x00}` reference frame and malformed-input
   rejection (SRS-REQ-024). The Lean theorems `req_srs021_*`,
   `req_srs022_*`, `req_srs023_*`, `req_srs024_*` exist in
   `Frick/Requirements.lean`/`Frick.Time` and all links in the manifest resolve
   (trace gate PASS). Anl is qualified (model-level arithmetic), consistent
   with the project's class-A scoping of full formal C-embedding (GAP-A1/A4).
4. **Leap wording cannot be read as mid-second.** SRS-REQ-023 "applied and
   cleared exactly at the next tone boundary" is single-valued; the code
   (`FF_TimeToneSignal`) applies and clears `g_pending_leap` only at a tone
   edge. (See F-4 for the complementary case: no tone at all.)
5. **GAP-A7 is correctly scoped as class A**, not an NC for class B. It is
   `level: gap`, `gap: true`, justified by flight-hardware dependency, and it
   is documented in `class-a-gaps.md` and `traceability.md`; the class-B target
   (criticality B) may operate on INTERNAL/GROUND sources at run time and the
   on-target hardware capture remains an A gap. The stale reference in
   `ecss-criticality-b.md` is raised as F-5.

## 2. References

- Project norms: `docs/rules.md` (N1-N16), `docs/engineering/independent-review.md`
  (profile R3), `docs/reviews/review-template.md`.
- Requirements & traceability: `docs/requirements/SSS.md`, `SRS.md`,
  `traceability.yaml`, `traceability.md`.
- Architecture & interfaces: `docs/architecture/time-and-timecode.md` rev 0.2,
  `docs/interfaces/ICD.md` rev 0.2, `docs/quality/class-a-gaps.md` (GAP-A7),
  `docs/quality/ecss-criticality-b.md`.
- Implementation & evidence: `src/core/time/time.c/h`, `src/core/time/pps.c/h`,
  `src/pus/pus_services.c` (S9), `src/pus/pus_types.h`, `src/transport/ccsds/ccsds.c`,
  `config/fsw_config.h`, `tests/unit/test_time.c`, `tests/unit/test_pus.c`,
  `verification/lean/Frick/Time.lean`, `verification/lean/Frick/Requirements.lean`,
  `docs/vv/verification-results.md` (M2 notes), `docs/roadmap.md` (M2 done).
- Standards: ECSS-E-ST-40C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2,
  **CCSDS 301.0-B-4 (checked textually via `ccsds.org/Pubs/301x0b4e1.pdf`)**.

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| 1 | **Major** | ICD rev 0.2 §5 CUC vs PUS service 9 and TM secondary header | The ICD/architecture describe the **7-byte explicit P+T code** as the code "carried in the PUS-C TM secondary header" and used by "PUS service 9,1 (set time) and 9,2 (report time)", but the implementation exchanges a **6-byte T-field (coarse[4]+fine[2]) with an implicit P-field**. `svc9_set_time` parses `coarse = d[0..3]`, `fine = d[4..5]` with only `len < 6` rejected, so a ground station that follows the published ICD and sends the 7-byte code `{0x2E, coarse[4], fine[2]}` is accepted and **silently mis-parses the 0x2E byte as the MSB of the coarse field**, corrupting the on-board UTC correlation (no range or P-field validation). The ICD is also internally contradictory: coarse/fine at "offsets 7..12 of the user data" is only possible if the P-field is absent from the wire (offset 6 carries `destination_id`). | ICD.md §5 ("fixed length 7 bytes ... P-field followed by 6-byte T-field"; "PUS service 9,1 ... and 9,2 ... use this code"; "coarse/fine bytes are at offsets 7..12"); `pus_services.c` `svc9_set_time` (len<6, `rd_u32(&d[0])`, `rd_u16(&d[4])`) and `svc9_report_time` (6-byte payload); `pus_types.h` `FF_PUS_TM_SEC_HDR_LEN 13`; `ccsds.c` `sec[7..10]`, `sec[11..12]` (no P byte); `time-and-timecode.md` §5 ("This is the time code carried in the PUS-C TM secondary header"); SRS-REQ-024/`test_time.c` define the 7-byte explicit code as the canonical frame | Align the contract and the code: either (i) state explicitly in the ICD/arch that the TM secondary header and services 9,1/9,2 carry only the **6-byte T-field with the P-field (0x2E) implicit by interface agreement**, keep the 7-byte form for `FF_TimeCucEncode`/service-9 file recording, and make `svc9_set_time` reject (or strip) a leading P byte; or (ii) put the explicit P-field on the wire (TM header 14 bytes, offsets shifted) and adapt CRC/length constants and tests. Add a unit test that a documented 7-byte 9,1 input can never silently shift the coarse field |
| 2 | Minor | SRS-REQ-021 / SSS-REQ-016 | The requirement bundles two independent properties ("switchable at run time" AND "non-regression across a tone boundary under non-negative leap") into one clause, and it leaves the behaviour of the exported CUC **at the instant of a runtime source switch** unspecified. The implementation allows the exported CUC seconds to revert/jump on a switch (e.g. TONE -> INTERNAL falls back to the pre-latch PAL UTC correlation, which is strictly behind the disciplined time; ground set-time moves the correlation backward) with no constraint and no analysis. CCSDS 301.0-B-4 §3.2.1 requires the T-field to "increase monotonically without reversion"; the requirement set establishes that only for the tone-boundary case (Lean `req_srs021_utc_non_regression`), not for switches. | `Time.c` `FF_TimeGetUtcNs` (TONE branch vs fallback to `FF_PalGetUtcNs`), `FF_TimeSetUtcNs` implicit GROUND switch; `time-and-timecode.md` §1.1 (source table); Lean `req_srs021_utc_non_regression` proves only `cucSeconds s <= cucSeconds (onToneLeap s)`; CCSDS 301.0-B-4 §3.2.1 | Reword SRS-REQ-021 into two verifiable clauses (switchable sources; non-regression at boundary) and explicitly define the allowed discontinuity at a switch (e.g. "a source switch shall be equivalent to a new authority set, after which the exported CUC is continuous from the new source; the switch shall be reported") or extend the theorem/analysis to cover the switch event |
| 3 | Minor | SRS-REQ-022 | "each valid 1 Hz tone shall advance the **second counter** by exactly one second" is ambiguous: the exported CUC coarse field advances by **two** across a boundary carrying a pending +1 leap (tone epoch `+1` AND `g_leap_seconds +1`), while only the internal tone-epoch counter advances by exactly one. The Lean model (`ToneState.met_s`) and the C code verify the tone-epoch counter, not the exported CUC seconds. | `time.c` `FF_TimeToneSignal` (`g_tone_utc_ns += TONE_NS` then `g_leap_seconds += pending`) and `FF_TimeGetCuc` (`ns/1e9 + leap_seconds`); `Frick/Time.lean` `onTone`/`cucSeconds`; `test_leap_at_tone_boundary` (`coarse == 2u` after one tone + leap) | Define "second counter" explicitly as the tone/second-epoch counter and state that the leap offset is applied on top of it at the boundary (or scope the "exactly one second" claim to the tone epoch), so the requirement cannot be misread against SRS-REQ-023 |
| 4 | Minor | SRS-REQ-023 | The requirement (and docs) do not define the case where **no tone ever arrives**: a pending leap is applied and cleared only inside `FF_TimeToneSignal`, so under the INTERNAL or GROUND sources a scheduled leap stays pending indefinitely, is never applied, never cleared, and is silently absent from the exported CUC. The behaviour in this configuration is unspecified (completeness). | `time.c` `FF_TimeToneSignal` (leap apply/clear only there); `test_time.c` `test_leap_at_tone_boundary` (tone-driven only); SRS config allows INTERNAL/GROUND at run time (SRS-REQ-021) | Add a clause to SRS-REQ-023 defining the behaviour without tones (e.g. "the pending leap shall remain scheduled and be applied at the next tone boundary whenever one occurs; no leap is applied in its absence" or reject scheduling while no tone source is active), and cover it with a test |
| 5 | Minor | `ecss-criticality-b.md` / GAP tracking | The class-B baseline document still lists "Gaps to class A: GAP-A1..A6" in its header while GAP-A7 (on-target PPS capture and TAI/CCSDS epoch) was added in M2 to `class-a-gaps.md`, the manifest and `traceability.md`; the class-B reference set is stale (norm N8). | `ecss-criticality-b.md` line 10 ("GAP-A1..A6"); `class-a-gaps.md` header (GAP-A1..A7) and §GAP-A7; `traceability.yaml` GAP-A7 | Update `ecss-criticality-b.md` to reference GAP-A1..A7 and close the loop next commit |
| 6 | Comment | SSS-REQ-016 | At SSS level the wording "provide a PPS / 1 Hz tone capture path" names an implementation mechanism (capture path), which is borderline for a system-level requirement (ECSS-E-ST-40C free-of-implementation-over-specification). Acceptable here given the criticality of the mechanism, but could be capability-level ("the system time shall be disciplined from an external 1 Hz reference..."). | `SSS.md` SSS-REQ-016 text | Consider capability-level wording; keep the mechanism detail in the SRS/architecture |
| 7 | Comment | SSS-REQ-007 vs SSS-REQ-016 | The split of CUC framing between SSS-REQ-016 (SRS-REQ-024, framing/discipline) and SSS-REQ-007 (SRS-REQ-010, PUS S9 encoding/reporting) is workable but the boundary is implicit; both trace to `time.c` and the same exported CUC. | `SSS.md` SSS-REQ-007/016; `traceability.yaml` SRS-REQ-010 (parent SSS-REQ-007) vs SRS-REQ-024 (parent SSS-REQ-016) | One sentence in SSS clarifying "CUC encoding/reporting (PUS S9, SSS-REQ-007) vs disciplined time base and CUC framing (SSS-REQ-016)" would remove ambiguity |
| 8 | Comment | `traceability.md` counts | "Counts (current) ... CI evidence claims = 8" while the trace gate reports "CI evidence claims: 39". Both are defensible (8 unique `ci:*` names vs 39 total claims) but the document does not say which, and a reviewer cannot tell them apart. | `traceability.md` line 18; `tools/scripts/trace.py` output "CI evidence claims: 39"; `traceability.yaml` `ci:*` occurrences | Align the count line with the tool output or label it "unique CI evidence claims" |
| 9 | Comment | `time.c` / `fsw_config.h` | `FF_TIME_CUC_MAX_LEAP_S` (documented in `time-and-timecode.md` §6 as "max scheduled leap change (single second)") is not used by `FF_TimeScheduleLeap`, which validates against hard-coded `-1/0/1`; the macro can silently drift from the code. | `config/fsw_config.h` (macro); `time.c` `FF_TimeScheduleLeap`; `time-and-timecode.md` §6 | Use the configuration macro in the validation (and in the test) or drop it from the documented configuration table |

**Severity definition** (per template): Major = non-conformity that disqualifies
the item for the applicable criticality, or a defect that can defeat a
requirement or a safety barrier; Minor = must be corrected or justified;
Comment = observation, no derogation.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [x] **REJECTED** - one or more Major findings (F-1)

Rationale: the requirements delta (SSS-REQ-016, SRS-REQ-021..024), the
traceability structure, the verification-method mapping, the CCSDS 301.0-B-4
P-field wording and the GAP-A7 scoping are all individually sound and correctly
machine-gated. However **F-1 is a contract-level CUC wire-format contradiction**
between the published ICD (§5: 7-byte explicit P+T code on PUS service 9 and in
the TM secondary header) and the implemented 6-byte implicit-P exchange, with a
concrete failure mode (ground-issued 9,1 time-set silently shifts the coarse
field by the 0x2E byte). Under norm N14 the SSS/SRS/ICD delta must be re-worked
and this review repeated after closure of F-1 (and, to the letter, the Minors).

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R3 (N9/N14)   | independent subagent, profile R3 | 2026-10-10 | REJECTED (F-1 Major; see NC table) |
| R3 (N9/N14) delta | independent subagent, profile R3 (fresh) | 2026-10-10 | APPROVED (all NCs closed) |

> Delta re-review: `docs/reviews/2026-10-10-m2-sss-r3-delta-review.md` —
> APPROVED (all 5 NCs closed; CUC two-form truth verified in ICD §5, ARCH §5,
> SRS-REQ-024 and `svc9_set_time`; `trace` PASS). This supersedes the REJECTED
> conclusion above for the current working tree (norm N12/N14).

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-1 | Major | ICD §5; SRS-REQ-024; `pus_services.c` S9; `ccsds.c` | CUC wire-format contradiction (7-byte P+T documented vs 6-byte implicit-P implemented) | Documented the two exchange forms (self-describing 7-octet P+T for `FF_TimeCucEncode/Decode`; PUS-C on-wire 6-octet T-field with implicit P for the TM secondary header and S9) in ICD §5, ARCH §5 and SRS-REQ-024; `svc9_set_time` rejects `len != 6` (`FF_BAD_SIZE`) | `trace` PASS; `test_pus.c` 6-octet accepted / 7-octet rejected; ICD/ARCH/SRS rev 0.3 | frick-fsw | closed | APPROVED (`2026-10-10-m2-sss-r3-delta-review.md`) |
| NC-2 | Minor | SRS-REQ-021 | Bundled clause; source-switch semantics unspecified | Split into switchable-source + explicit authority-change switch semantics (<=1 s step) + non-regression at boundary; ARCH §1.1 analysis | SRS-REQ-021 rev 0.3; ARCH §1.1; tests | frick-fsw | closed | APPROVED (R3 delta) |
| NC-3 | Minor | SRS-REQ-022 | "second counter" ambiguity | Defined "tone second counter" and exported CUC = counter + leap offset | SRS-REQ-022 rev 0.3 | frick-fsw | closed | APPROVED (R3 delta) |
| NC-4 | Minor | SRS-REQ-023 | No-tone configuration undefined | Added no-tone clause (leap stays pending, applied at first tone); test `test_leap_pending_without_tone` | SRS-REQ-023 rev 0.3; `test_time.c:204-224` | frick-fsw | closed | APPROVED (R3 delta) |
| NC-5 | Minor | `ecss-criticality-b.md` | Stale "GAP-A1..A6", GAP-A7 missing | Updated to GAP-A1..A7 | `ecss-criticality-b.md:10` rev 0.2 | frick-fsw | closed | APPROVED (R3 delta) |

---

Committed and pushed with the reviewed change (norm N11).