# Independent Avionics / Systems R1-R3 Review — S8-PUS-FU S1 Ingestion-Failure Test

| Field | Value |
|---|---|
| Review ID | SYS-R1R3-20261112-PUS-S1-INGEST |
| Review date | 2026-11-12 |
| Baseline | Committed `e5f01d4` plus the uncommitted candidate described below |
| Scope | Independent avionics/system-safety and requirements/interface trace review of `test_verification_failure_via_ingest`, its TS-1 case mappings and `SRS-REQ-017` traceability attribution |
| Evidence reviewed | Controlled SRS, ICD rev 1.7, `src/pus/pus.c`, `tests/unit/test_pus.c`, TS-1 rev 1.11, `docs/requirements/traceability.yaml`, sprint plan and the prior PA-R2, V&V-R2 and R4 reports named in the task |
| Result | **Technically useful characterization; one Major requirements-basis finding.** The test oracle is accurately tied to current implementation and the provisional destination is not represented as approved. The claim that its illegal-APID failure-report behavior is requirement-covered is not fully grounded by the controlled SRS/ICD. |

> **Authority limitation.** This is an independent engineering review, not an approval or a delegated design/safety authority. It does not approve a PUS profile, hazard classification, mission use, safety risk, release, qualification or flight readiness, and does not close any NC. No authority decision is requested or made here.

## 1. Review conclusion

The five sub-cases are internally coherent as host characterization of the current `FF_PusIngest` implementation. The new test directly observes the illegal-APID branch, captures its TM(1,2) response and confirms the handler is not dispatched. It uses symbolic failure-code constants; tests byte-level Request-ID/source and echoed fields without asserting the sequence-count bytes; asserts one TM(1,2), no TM(1,8), no acceptance-success TM and no ping reply. The four unknown-service/subservice sub-cases correctly separate the completion-ack cases from the ack-zero cases. The latter are characterized, not counted as coverage, and `N22-TS1-104` remains a gap. This treatment does not close NC-EXT-04.

The test does **not** demonstrate an approved mission PUS profile. In particular, destination `0x0001` is explicitly provisional, Request-ID payload ordering is implementation-derived rather than controlled by ICD §4, and the failure-code allocation is provisional. The fixture source ID `0x0102` is test input, not a claimed operational ground-station identifier. The reports’ refusal to claim clause-level ECSS conformance or authority acceptance is appropriate.

One important qualification remains: TS-1 currently treats the illegal-APID acceptance-failure response as requirement-grounded/“host covered.” The controlled requirements establish the configured APID values, but do not clearly establish the rejection policy or the required **acceptance-stage** TM(1,2) response for a validly encoded packet carrying a different APID. ICD §1 explicitly leaves primary-header acceptance policies pending NC-EXT-13. This is an assurance-basis issue, not evidence that the implementation is functionally wrong.

## 2. Findings

### SYS-R1R3-01 — Major — illegal-APID failure-report oracle exceeds controlled requirement/interface text

**Location:** `docs/vv/test-spec/ts-1-protocol-core.md:126`; `docs/requirements/SRS.md:61–69, 527–529`; `docs/interfaces/ICD.md:11–16, 30–32, 103–108`; `src/pus/pus.c:420–428`; `docs/requirements/traceability.yaml:181–187`.

**Facts.**

1. `SRS-REQ-017` specifies CCSDS packet APIDs `0x010 / 0x020` and mission identifier `0x01`; it does not state that an incoming non-`0x010` APID shall be rejected, nor prescribe an S1 acceptance-failure response for that input.
2. `SRS-REQ-005` requires no execution and a **completion failure** for unknown service/subservice or a malformed frame. It does not expressly map an otherwise well-formed packet with a nonconfigured APID to acceptance failure (1,2). Whether such a packet is a malformed frame, and which verification stage is applicable, is not resolved by this text.
3. ICD §2 lists the TC/TM APIDs, while ICD §1 explicitly says primary-header acceptance policies remain subject to NC-EXT-06/NC-EXT-13. ICD §4 records S1 field content but does not define this APID acceptance policy or the 9-octet S1 payload layout.
4. `FF_PusIngest` rejects any decoded `apid != FF_APID_TC`; when acceptance ack is set it emits an acceptance failure with `FF_PUS_FAIL_ILLEGAL_APID`, then returns `FF_ERROR` before dispatch. The new test accurately characterizes that implementation path.
5. TS-1 rev 1.11 labels N22-TS1-23 `host covered (…)` and states that SRS-REQ-005 requires the acceptance failure. That requirement attribution is broader than the controlled wording. The existing qualifications for implementation-derived layout, provisional code value and characterization return code do not qualify the **acceptance-policy / stage** requirement basis.

**Assessment / consequence.** This creates a traceability and assurance overclaim if “host covered” is read as requirement coverage of the acceptance-stage response. It could conceal the unresolved operational policy for commands addressed to another APID. No evidence here demonstrates a hazard or an incorrect product behavior; the operational consequence depends on the command routing and ground/spacecraft profile, which are not established in this review.

**Recommendation.** Preserve the test and its explicit implementation oracle, but qualify the TS-1 row as characterization of the current APID-rejection policy unless and until an authorized baseline explicitly requires the rejection and acceptance-failure semantics. Keep any resulting policy gap linked to NC-EXT-13 / the applicable requirements-interface disposition. Do not change the ICD or infer an approved PUS profile in this test increment. The `SRS-REQ-017` traceability mapping may retain `src/pus/pus.c` and `tests/unit/test_pus.c` as accurate implementation/evidence paths, but its justification should identify the observed rejection rule as implementation behavior pending policy, rather than as a rule directly imposed by SRS-REQ-017.

**Disposition:** Major assurance-basis finding. It blocks the claim that N22-TS1-23 establishes requirement coverage for the acceptance-failure response. It does not invalidate the test as characterization or establish a product defect. Any commit disposition remains with the project’s authorized process; this report makes no approval decision.

### SYS-R1R3-02 — Minor — explicit no-(1,8) assertion is implementation-path evidence, not a general rejection policy

**Location:** `tests/unit/test_pus.c:722–747`; `docs/vv/test-spec/ts-1-protocol-core.md:126`.

**Facts.** The test sends a TC with acceptance acknowledgement only. The production APID-rejection branch returns before handler lookup and completion processing; therefore the no-(1,8) observation is correct for this exact input. It does not exercise a completion-ack request with a non-TC APID, nor establish what policy should apply to that combination. The TS-1 input correctly says acceptance ack only, and the completion flag is not asserted.

**Recommendation / disposition.** Keep the assertion but bound prose to the tested input and current implementation. Do not describe it as proof of general Service 1 rejection sequencing or as a mission-profile rule. Non-blocking; no additional test is needed for the current bounded claim.

### SYS-R1R3-03 — Comment — SRS-REQ-005 ack semantics remain ambiguous and safety-relevant to interface closure

**Location:** `docs/requirements/SRS.md:61–69`; `src/pus/pus.c:414–446`; `docs/vv/test-spec/ts-1-protocol-core.md:125–126,146`; existing NC-EXT-04 / NC-EXT-13.

**Facts.** The SRS wording requires a completion failure for specified malformed/unknown-service cases without explicitly conditioning that requirement on the completion-ack bit. The code gates the handler-not-found completion-failure report on that bit. The candidate correctly records the ack-zero variants as characterization checks and a `gap` under N22-TS1-104; it does not count them as requirement coverage. For illegal APID, the observed path separately gates an acceptance-failure report on the acceptance-ack bit.

**Systems assessment.** This is a policy/interface ambiguity rather than a finding that one ack behavior is correct. Depending on the ground command protocol and operator response model, absence or stage of a negative verification report can affect command-state awareness and recovery. The current evidence does not establish operational timing, command retry/timeout behavior, or a safety consequence, so no hazard classification is assigned here.

**Recommendation / disposition.** Preserve the `gap` classification and NC-EXT-04 linkage; do not silently change SRS or code from this review. When the authorized system/interface baseline resolves the requirement/profile semantics, reassess N22-TS1-104 and the corresponding characterization assertions together. Comment only; no independent block beyond SYS-R1R3-01’s coverage-attribution limit.

## 3. Requested review questions

| Question | Review answer |
|---|---|
| Requirements/interface grounding and bounded claims | The implementation behavior and payload-octet assertions are accurately derived from `pus.c`; source/layout/code/return-code qualifications are materially improved. The specific attribution that SRS-REQ-005 requires an acceptance failure for wrong APID is not established; see SYS-R1R3-01. |
| Unapproved mission PUS profile or destination | No mission-profile claim is evident in the reviewed candidate. Destination `0x0001` is expressly provisional; layout is implementation-derived and codes provisional. Preserve these qualifiers. |
| TC APID rejection / S1 ack hazards or ambiguity | Primary-header acceptance policy is explicitly open in ICD §1. Ack-zero completion behavior conflicts with unconditional SRS wording and remains a gap. This merits authorized requirements/interface disposition, but this review does not assign hazard severity or claim a demonstrated hazard. |
| `traceability.yaml` attribution | File paths `src/pus/pus.c` and `tests/unit/test_pus.c` are factually accurate for the observed behavior/test. The justification currently says the APID acceptance rule as if settled; qualify that rule as implementation behavior pending acceptance-policy disposition. The listed config/CCSDS paths remain relevant to APID definition and packet coding. |
| Gap vs requirement coverage | N22-TS1-104 is correctly a `gap`, not covered; assertions are explicitly described as characterization and no covered count is added. N22-TS1-23’s current “host covered” status overstates the requirement grounding of the acceptance-failure leg until qualified as in SYS-R1R3-01. |
| R1/R3 blockers | No production defect or direct safety hazard is demonstrated. SYS-R1R3-01 blocks treating the illegal-APID response as requirement-coverage evidence, not the test’s use as implementation characterization. SYS-R1R3-02 and -03 are non-blocking comments. |

## 4. Authority, uncertainty and confidence

- **Facts** are from the controlled repository artifacts and exact candidate scope stated above. No external standard clause was independently consulted; no clause-level ECSS interpretation is made.
- **Inference:** the current TS-1 requirement-coverage attribution exceeds the SRS/ICD statement because APID values do not unambiguously specify receive rejection/stage semantics, and the ICD calls acceptance policy unresolved. Confidence: **High** for the textual mismatch; **Medium** for whether an external or separately approved project interpretation resolves it, since none was supplied for this review.
- **Recommendation:** narrow the coverage claim while retaining characterization evidence; preserve open authority-dependent dispositions. This is not an authority decision.
- **Limitations:** no target/RTEMS/Renode/HIL execution, no new independent hazard analysis, no review of the full ECSS text, and no evaluation of mission operations/ground command procedures.

## 5. Overall disposition

The test increment is a sound characterization of the current implementation path and demonstrates useful non-dispatch and telemetry evidence. The provisional destination, payload layout, failure-code allocation, and ack-zero gap are appropriately bounded. The remaining Major finding is specifically that N22-TS1-23 should not present acceptance-failure-on-illegal-APID as requirement-covered on the basis of the cited SRS/ICD text as currently written. This report neither directs an authority decision nor closes any NC; any applicable project commit/review gate remains with its designated human authorities.
