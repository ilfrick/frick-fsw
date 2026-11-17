# NC-S9-01 Authority Decision Package — M12 OBCP

| Field | Value |
| --- | --- |
| Document ID | FRICK-DEC-NC-S9-01-001 |
| Revision | 0.4 |
| Date | 2026-11-17 |
| Status | Owner direction recorded; NC-EXT-43 verification deferred indefinitely, OPEN/non-blocking for unrelated development only |
| Related records | `docs/planning/sprint-plan.md` (M12, NC-S9-01); `docs/architecture/obcp-engine-comparison-study.md`; `docs/architecture/obcp-integration-assessment.md`; `docs/interfaces/ICD.md`; `docs/quality/licensing.md`; `docs/quality/safety-and-fdir.md` (GAP-A10) |

## 1. Purpose and decision boundary

This package records owner direction and captures remaining system, mission,
safety, target, and license inputs needed to plan disposition of NC-S9-01. It
does not approve an interface, tailor a requirement, grant a license, establish
target qualification, or close M12/NC-S9-01. The owner confirmed Route A on
2026-11-17. The comparison study remains a technical comparison, not an
adoption record; the current owner direction is recorded here and in the sprint
plan. This does not constitute mission S18 approval, safety-authority
acceptance, target qualification, or license disposition. Any additional or
superseding decision and source evidence must be recorded before affected
requirements, architecture, or mission configuration are baselined.

**Owner direction and continuing recommendations:** Route A (controlled N7
runtime patch with execution quota and safe abort/cleanup) is the owner-selected
engine direction. Keep PUS Service 18 provisional and disabled by default; use
host/emulator evidence only for its stated scope and do not present it as target
qualification. On 2026-11-17, the owner directed that NC-EXT-43 license/provenance
verification be deferred indefinitely and marked non-blocking for unrelated
development only. NC-EXT-43 remains OPEN; no reassessment date is scheduled,
and the required disposition remains a release/redistribution gate. This
instruction is not license clearance or a compatibility conclusion. These owner
directions do not close NC-S9-01 or NC-EXT-43.

## 2. Decision A — OBCP engine and execution-safety approach

The study `docs/architecture/obcp-engine-comparison-study.md` is a technical
comparison and does not itself select/adopt an option. The M12 row in
`docs/planning/sprint-plan.md` records Route A as selected by the owner, and the
owner reconfirmed Route A on 2026-11-17. Options B/C below are retained only as
historical alternatives from the comparison; this package does not propose them
as replacements or reopen the selection.

| Option | Summary | Authority consequence / unresolved evidence |
| --- | --- | --- |
| A — controlled runtime patch | Add/enforce VM execution quota and safe abort/cleanup in the selected runtime; propagate interpreter failures. | Define patch owner/scope, independent review, regression strategy, target resource/WCET evidence, and qualification/requalification impact. Upstream acceptance is uncertain. |
| B — cooperative-only restriction | Retain cooperative step-boundary abort and accept the current limitations for a restricted procedure set. | Requires explicit authority acceptance of the residual unbounded execution/fault-containment risk, enforceable procedure restrictions, compensating controls, and requirement tailoring/waiver where applicable. |
| C — in-house M16 fallback | Defer selection to an in-house engine with an approved scope behind `FF_Obcp_t`. | Requires approved M16 scope, schedule, assurance plan, resource model, and proof that M12 obligations are either met or formally deferred. It is not implemented as a production S18 engine. |

**Disposition:** the owner confirmed Route A on 2026-11-17. Record Route A as
the engine direction; retain the comparison study's status as a technical study,
not an adoption authority. This decision does not by itself approve operational
S18 use, safety acceptance, or qualification. Remaining assurance conditions,
resource limits, safe-abort criteria, and required evidence still need their
respective authorized baselines. Spacebel remains excluded per the recorded
owner direction; do not reopen it without a new owner decision.

### Requested response

- Engine direction (owner confirmed 2026-11-17): **Route A** — recorded; no further selection response needed unless the scope changes.
- Safety/assurance authority, criteria, and controlled approval evidence for implementation: `________________________`
- Authorized decision-maker and delegated authority: `________________________`
- Scope of Route A selection (engine only, implementation, operational use, or
  other) and conditions: `__________________________________________________`
- Applicable mission/profile and affected requirements: `_____________________`
- Required safety analysis (including GAP-A10) and acceptance criteria: `_______`
- Required verification/qualification evidence and independent reviewers: `____`
- Conditions, expiry/revisit trigger, and evidence reference: `________________`
- Decision date/signature or controlled approval record: `_____________________`

## 3. Decision B — provisional PUS Service 18 profile

The current implementation exposes provisional subservices TC[18,1] (load/start),
TC[18,3] (cooperative abort), and TC[18,5] (status), gated by
`FF_USE_N7_OBCP` and `FF_USE_PUS_S18_PROVISIONAL`; the latter defaults OFF.
`docs/interfaces/ICD.md` records the current provisional payload and TM layouts.
Those values are an implementation description, not an approved mission
interface. Service 11 remains a `FF_NOT_IMPLEMENTED` stub and is not the OBCP
execution interface.

**Owner direction (2026-11-17):** retain the recommendation to keep Service 18
provisional, default-off, and non-operational pending authorized mission-profile
acceptance or replacement of the ICD values. The owner's “ok” accepts this
conservative boundary; it does not approve the provisional wire interface for
mission use. Interface acceptance is separate from the Route A engine direction.

### Requested response

- Is OBCP control required in the mission PUS profile? `yes / no / undecided`
- Authorized profile/specification and revision: `___________________________`
- Accept current provisional TC/TM mapping unchanged, accept with changes, or
  replace it: `______________________________________________________________`
- Required APID, source/destination, authentication/access-control, ACK,
  sequencing, duplicate/retry, abort, and failure-reporting policy: `__________`
- Permitted script language/subset, size, resource limits, and procedure
  approval/update controls: `________________________________________________`
- Ground segment compatibility owner and end-to-end test oracle: `____________`
- Decision-maker, approval evidence, date: `_________________________________`

Until these fields have an authorized disposition, do not enable the provisional
profile in a mission configuration or describe it as operationally approved.

## 4. Target/HIL availability and evidence plan

**Availability is unknown.** The owner accepted the proposed interim approach:
if target/HIL access is unavailable, continue only with host/emulator evidence
within its stated scope and track target evidence as an open dependency. This
acceptance does not establish that target/HIL access is unavailable. No board,
target lab, HIL booking, custodian, or schedule is asserted by this package. Host tests and emulation are not physical
target/HIL evidence and do not establish N18 or mission qualification.

| Evidence item | Minimum planned evidence if target access is available | Current disposition |
| --- | --- | --- |
| Cross-build and link | Controlled RTEMS 6.2 `riscv/mpfs64imafdc` build; record toolchain/BSP/configuration, source identity, warnings, and retained log. | Required; availability/status to be confirmed for the candidate. |
| Target boot and service integration | On the RTPFS160ZT/approved target, verify startup, selected OBCP adapter/runtime, S18 gating, and approved TC/TM exchange using the authorized profile and explicit expected results. | Not evidenced by this package. |
| Bounds and abort behavior | Nominal and out-of-bound scripts: zero/limit/exhausted budget, abort at safe points, malformed/oversize input, engine error propagation, cleanup/context reuse; capture state and emitted TM against controlled oracle. | Host evidence exists for bounded slices; target evidence and authority-approved oracle remain outstanding. |
| Resources and timing | Measure/justify stack, heap, instruction/time budget and worst-case response/abort latency under target load; compare against approved limits. | Limits and target measurements not supplied; do not infer from host/emulator. |
| Fault containment/recovery | Demonstrate defined behavior for runtime error, abort, timeout/budget exhaustion, and restart/recovery; link approved safety analysis and verify watchdog/reset interactions. | GAP-A10 / safety-authority input and target evidence outstanding. |
| HIL/ground interface | Exercise the actual ground path, transport/security configuration, TC acceptance, ACK/TM ordering and timeout/retry cases with recorded equipment/configuration identities. | Hardware, HIL, and ground-system availability unknown. |
| Independent verification | Independent R1/R2/PA review of exact candidate identity and retained evidence; R4 review of controlled documentation as applicable. | Pending; this package does not claim review. |

### Requested response

- Is RTPFS160ZT target hardware available? `yes / no / unknown`
- Board/SoC revision, custodian, lab location/access constraints: `____________`
- RTEMS/BSP and toolchain versions/configuration: `__________________________`
- HIL, link/ground equipment, and test-support availability: `________________`
- Earliest access window and responsible test owner: `_______________________`
- Approved target acceptance limits (timing/resources/recovery): `____________`
- If unavailable, authority/owner for the gap, milestone, and interim evidence
  boundary: `_______________________________________________________________`

## 5. Decision C — N7 OBCP license, provenance, and distribution disposition

`docs/quality/licensing.md` records upstream MIT as stated for n7s-obcp and
MicroPython, identifies the project overlay
`tools/patches/n7s-obcp/src/modifications/py/vm.c` as `MIT AND MPL-2.0`, and
explicitly leaves complete notices, provenance, generated outputs, and combined
distribution compatibility open under NC-EXT-43. The comparison study also
records an ESA contract/ECL-3 provenance assertion for the N7 candidate. MIT and
MPL-2.0 are open-source licenses. The unresolved question is whether they govern
every pinned, modified, and generated artifact and permit the intended combined
distribution; the study's ESA/ECL-3 assertion is not verified for these exact
artifacts. No legal conclusion is made.

**Owner disposition (2026-11-17):** indefinitely defer artifact-specific
license/provenance/compatibility verification; no reassessment date is scheduled.
NC-EXT-43 remains OPEN and is non-blocking only for unrelated development under
N21. This does not satisfy N13, close the NC, establish license clearance or
compatibility, or waive the required disposition before release/redistribution.
Do not run further license verification under this deferral; do not make license
or compatibility claims for the unresolved artifacts.

The response fields below remain unresolved and dormant during unrelated
development. Reopen the verification only if the owner changes this deferral or
if a release/redistribution is proposed; no release or redistribution may rely
on this deferral as approval. Do not alter third-party terms in this package.

### Requested response

- Authoritative source of the n7s-obcp and MicroPython inputs; exact revisions,
  acquisition path, and chain-of-custody evidence: `________________________`
- Does an ESA contract/ECL-3 or other additional grant apply to these exact
  artifacts? Attach controlling agreement/text or authorized determination:
  `_________________________________________________________________________`
- Confirm upstream notices and complete source/license files for every pinned
  component/submodule: `____________________________________________________`
- Identify project patches/overlays, generated/flattened outputs, retained
  upstream code, and source-to-output provenance: `__________________________`
- Intended distribution/deployment profile (source, binary, firmware, notices,
  recipients, commercial/mission restrictions): `____________________________`
- Written combined-distribution compatibility disposition, authority/counsel,
  scope, caveats, evidence reference, and date: **DEFERRED INDEFINITELY by owner
  direction; no reassessment date. Required before release/redistribution.**
- NC-EXT-43 status: **OPEN; non-blocking for unrelated development only; no
  license clearance or compatibility conclusion.**

## 6. Required follow-up and exit criteria

1. Record the authority responses with identity, delegated authority, date, and
   controlled evidence; unresolved fields remain open questions, not inferred
   approvals.
2. Update the SRS/SSS tailoring, architecture, ICD, safety analysis, license
   record, test specifications, and traceability only to the extent authorized.
3. Re-baseline the exact candidate and produce nominal/out-of-bound tests with
   explicit oracles for the approved behavior; retain command logs and hashes.
4. Obtain required independent R1/R2/R4/PA reviews and close their findings by
   the applicable rules. Complete target/HIL and N18 evidence where required.
5. Keep NC-S9-01 open until its closure criteria and authority gates are met.
   NC-EXT-43 remains OPEN with verification deferred indefinitely and is
   non-blocking for unrelated development only; its disposition remains a
   prerequisite for any release/redistribution or compatibility claim. No M12
   completion, release, or mission qualification is claimed here.

## 7. Revision history

| Revision | Date | Author | Change |
| --- | --- | --- | --- |
| 0.4 | 2026-11-17 | frick-fsw | Record owner direction to defer NC-EXT-43 license/provenance verification indefinitely, OPEN/non-blocking for unrelated development only; preserve release/redistribution gate and make no license/compatibility conclusion. |
| 0.3 | 2026-11-17 | frick-fsw | Record owner confirmation of Route A; acceptance of provisional/default-off S18 boundary and conditional host/emulator interim evidence plan. Preserve unknown target availability and pending license/safety/mission authority dispositions. |
| 0.2 | 2026-11-16 | frick-fsw | Reconcile records: sprint plan records owner-selected Route A; comparison study remains comparison-only. Request scope/authority reconciliation; no decision was made in this package revision. |
| 0.1 | 2026-11-16 | frick-fsw | Initial authority-response package; records proposals and questions only. |
