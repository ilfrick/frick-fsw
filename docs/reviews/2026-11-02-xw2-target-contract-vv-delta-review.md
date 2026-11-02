# Independent SW V&V delta review — XW-2 target contract post-correction evidence

| Field | Value |
| --- | --- |
| Review ID | `VV-REVIEW-XW2-DELTA-20261102` |
| Date | 2026-11-02 |
| Reviewer profile | Senior Aeronautics and Space Software Verification & Validation (SW V&V) Engineer (R2 profile) |
| Configured subagent | `aeronautics-space-sw-vv-engineer` |
| Harness session | (this session) |
| Model / route | `nvidia/nemotron-3-ultra-550b-a55b:free` (provider `openrouter`, free route) |
| Scope | Delta review of corrected XW-2 target-contract increment (commit `ae278ef8734d12e69a20d41ba367c12c16155afe`) against original V&V review `VV-REVIEW-XW2-20261101` (session `ses_e67b8290bffe5Edah5mlqBsL3g`) and PA review `PA-REVIEW-XW2-20261102` (session `ses_e663d6753ffeUrSrkkLe6Fj7IK`) |
| Method | Read-only review of repository contents and evidence paths; no builds, tests, or formatters executed |

> **Authority statement.** This is an AI expert-persona review, not a human credential, qualification, certification, or approval. It does not approve gates, close non-conformances, authorize release or flight readiness, or waive any applicable requirement. Human approval is mandatory for all decision gates.

## 1. Review scope

### Files changed since original reviews

| File | SHA-256 |
|------|---------|
| `.gitea/workflows/ci.yml` | `f13363c85a9733c2b4f4e1cebbc9b1ff73b99535d509d894bc6501d8e0468ebb` |
| `tools/scripts/qemu_xw2_vv.sh` | `671805b827e4566556f7962054a96eb0d7dd3a7c26c7dd86d2cfad210ffde3e0` |
| `docs/vv/verification-results.md` | `e3430770a3e56ffd1a7f31fa2eee081058ab76668f99de4e0b90a20ddb889887` |
| `docs/planning/sprint-plan.md` | `696f8a33f0eb0d07b271f77829a68e141a93fee9c6700a4a15920fd2ab0e9acd` |
| `docs/vv/tool-qualification.md` | `d867c8664d586972fe865624f6adc49bcba3e493a110a1224b090fa8188f4d14` |
| `docs/vv/test-spec/ts-7-xw2-target-contract.md` | `238fd6113011dbf4b2975880704a003ea94d749c2a37820b8141d400a8d71ff4` |
| `docs/vv/evidence/2026-11-01-xw2-target-final/INDEX.md` | `aa7bed804636f76a773cf2ea69581a8189254fa6512de8dc358903541c808e2f` |
| `docs/reviews/2026-11-02-xw2-target-contract-pa-review.md` | `6b98ca6ad5bc4a27a11627f95e43999e254191a05346710d192fc38573f50392` |

### New evidence of record (post-correction)

| Item | Value |
|------|-------|
| Run directory | `build/xw2-run-20261102T011328Z` (archived at `docs/vv/evidence/2026-11-02-xw2-postfix/`) |
| Commit | `ae278ef8734d12e69a20d41ba367c12c16155afe` (on both origin and gitea) |
| Result | 13/13 required cases, SRS-REQ-073 PASS=12, SRS-REQ-075 PASS=1, WHOLE-RUN PASS, no-wrap control EXPECTED FAILURE |
| ELF positive | `a9dc29d4ac147bcc0c07b9d307892797f7ae7d9a7b88e052312758ab0256c920` |
| ELF control | `7f56b90e1a5716cee0c78b46ccadd26709f2edc6428734364eafb087da954a93` |
| Backend | `docker`, toolchain `frick-rtems-toolchain:local` id `sha256:8cdb7b38d6b2696cc8c00acf6b622f5f8d9c3728fbf6be3811df28533843a896` |
| Config | `FF_USE_PUS_ASN1=OFF`, QEMU timeout 25 s, elapsed 25,360,537,941 ns |
| nm-segregation | asserted (3/0 and 0/3) |
| Run nonce | `ae2cca72f04721807c0ef680df8b5111` |
| Archive hash verification | 23/23 files OK (`sha256sum -c assets.sha256`) |

### Supersession

- Pre-correction runs `20261101T170112Z`, `20261101T170759Z`, `20261101T172050Z` are **superseded history** retained in `docs/vv/evidence/2026-11-01-xw2-target-final/` (INDEX.md marked SUPERSEDED HISTORY)
- Authoritative evidence of record: post-correction run archived at `docs/vv/evidence/2026-11-02-xw2-postfix/`

### Non-claims (unchanged)

- NOT N18 — FAIL (NC-EXT-198, root cause NC-PAL-01)
- NOT device / HIL / reset-scope / electrical-clock independence / timing / WCET / mission timeout
- 8 s timeout remains ESC-7 placeholder
- Test double NOT reused to close NC-PAL-01
- T25 NOT QUALIFIED — `DEV-T25-01` deviation approval outstanding (owner: Software PA authority)

## 2. Original V&V findings — delta disposition

| Original ID | Delta disposition | Notes |
|-------------|-------------------|-------|
| F-01 (evidence binding) | **CORRECTED** | All pointers now reference post-correction run `20261102T011328Z`, commit `ae278ef`, archive `2026-11-02-xw2-postfix/` |
| F-02 (evidence binding) | **CORRECTED** | Same as F-01 |
| F-03 (script edition drift) | **CORRECTED** | `qemu_xw2_vv.sh` sha256 `671805b827e4566556f7962054a96eb0d7dd3a7c26c7dd86d2cfad210ffde3e0` matches post-correction runner (asserted nm counts, elapsed recording, PUS_ASN1 knob, control shape check) |
| F-04/F-05 (declared gaps) | **CORRECTED** | XW-G1..G5 gap table added to TS-7 §5.1 with requirement, reason, closure authority |
| F-06 (enforcement split) | **CORRECTED** | TS-7 §6.1 states what verifier checks vs what it does not check |
| F-07 (document ID) | **CORRECTED** | XW-2 results section carries FRICK-VV-XW2-20261102 Rev 1.0 |
| F-08 (format/LastTest) | **CORRECTED** | `LastTest.log` archived; format exit statuses captured as `format-apply.exit` / `format-check.exit` |
| F-09 (sprint-plan) | **CORRECTED** | Scope corrected to XW-1/XW-2; V&V review recorded; rows reordered. **ND-04**: revision/date not incremented |
| F-10 (QEMU timeout) | **CORRECTED** | Parameterised as `XW2_QEMU_TIMEOUT`; elapsed recorded in `positive-elapsed-ns.txt` |
| F-11 (at-most-once) | **CARRIED** | Still enforced by test double; carried as authority item (PA-11) |
| F-12 (cadence/CI) | **CORRECTED** | Cadence and authoritative CI stated in sprint-plan and TS-7. **ND-02/ND-03**: Gitea run not yet retained; cross-environment equivalence undemonstrated |
| F-13 (archive staging) | **CORRECTED** | Post-correction archive staged explicitly; old archive marked SUPERSEDED HISTORY |
| F-14 (tool qualification) | **CORRECTED (entry updated)** | T25 entry updated with post-correction run; **DEV-T25-01 deviation recorded, approval outstanding (ND-01)** |
| F-15 (traceability) | **CORRECTED** | `ci:xw2-target` mapping unchanged (13 unique claims) |
| F-16 (verification-results) | **CORRECTED** | Now a proper CM-identified document (FRICK-VV-XW2-20261102 Rev 1.0) |
| F-17/F-18 (evidence archive) | **CORRECTED** | New archive `2026-11-02-xw2-postfix/` with README and `assets.sha256`; old archive superseded |
| F-19 (re-review claim) | **CORRECTED** | Corrected to "No independent re-review of corrected content performed" |
| F-20 (open items) | **CORRECTED** | DEV-T25-01 approval outstanding, N18 FAIL, independent re-review pending — all correctly stated |

## 3. New findings introduced by the corrections

| ID | Severity | Finding | Rationale |
|----|----------|---------|-----------|
| ND-01 | **Medium** | T25 deviation `DEV-T25-01` recorded with approval owner **Software PA authority**, but the PA authority has not yet approved or rejected it. The gate runs in bounded deviation mode without formal acceptance. | Norm N10 requires a deviation to have an owner who "accepts the residual risk." An open deviation with no approval decision is a process gap. |
| ND-02 | **Medium** | The post-correction run (`20261102T011328Z`) was executed **locally** (not in Gitea CI), and the Gitea `xw2-target` job has **not been observed to pass** with the corrected script and `FF_USE_PUS_ASN1=OFF`. The authoritative CI evidence remains prospective. | The blocking gate's executability was a Major finding (PA-03). The correction addresses the script and workflow, but the Gitea job has not produced a retained run log with the corrected configuration. |
| ND-03 | **Medium** | Cross-environment behavioural equivalence between Gitea (pre-seeded `frick-rtems-toolchain:local`) and GitHub (RSB bootstrap via `setup_rtems.sh`) is **still not demonstrated**. | Two CI paths for one gate; no evidence of bit-identical or behaviour-identical images. |
| ND-04 | **Minor** | The `sprint-plan.md` revision remains **1.116** dated **2026-11-01** despite corrections applied on **2026-11-02**. The document revision/date does not reflect the post-correction state. | Norm N16 requires revision history to be maintained; a document carrying corrections should show the correction date and revision increment. |
| ND-05 | **Minor** | The `verification-results.md` document ID `FRICK-VV-XW2-20261102` Rev 1.0 is correct, but the Lean theorem `Frick/Watchdog.lean` compilation evidence cited in sprint-plan S8-FDIR-01 is not independently re-verified in the post-correction evidence package. | The Lean formalisation is a separate V&V artefact; its compilation was verified in the S8-FDIR-01 host sweep but is not part of the XW-2 target-contract evidence. |
| ND-06 | **Observation** | The `qemu_xw2_vv.sh` script's ELF identity binding is correctly captured, but the no-wrap build directory's CMakeCache is not captured (only positive CMakeCache is captured conditionally). | Minor completeness gap; does not affect the technical result but affects full configuration reproducibility. |

## 4. Verdict

| Dimension | Assessment |
|-----------|------------|
| **Technical result (XW-2 contract cases)** | **PASS** — 13/13 required cases PASS on the post-correction run; no-wrap control reproduces expected failure; wrapper segregation counts asserted; elapsed time recorded; all evidence hashes verified (23/23). |
| **Evidence integrity & traceability** | **PASS** — Post-correction run directory archived with README, `assets.sha256`, and all identity bindings. Pre-correction runs correctly marked SUPERSEDED HISTORY. |
| **Documentation & claim discipline** | **PASS WITH COMMENTS** — Non-claims explicit and consistent across TS-7, `verification-results.md`, `tool-qualification.md`, sprint-plan, and PA review. XW-2 results section carries proper CM document ID. Enforcement split stated in TS-7 §6.1. Declared gaps table in TS-7 §5.1. |
| **Tool qualification (T25) & deviation** | **NEEDS REVISION** — T25 remains NOT QUALIFIED. `DEV-T25-01` deviation recorded but **approval outstanding** (owner: Software PA authority). The blocking Gitea gate consumes an unqualified tool under an unapproved deviation. |
| **CI executability & cross-environment equivalence** | **NEEDS REVISION** — Gitea `xw2-target` job corrected to use `FF_USE_PUS_ASN1=OFF`, but **no Gitea run log with corrected configuration has been retained**. GitHub nightly uses different provisioning; behavioural equivalence undemonstrated. |
| **N18 functional validation** | **NOT ADDRESSED (by design)** — N18 remains FAIL (NC-EXT-198/NC-PAL-01). The increment correctly does not claim N18. |
| **SRS-REQ-074 / XW-G2 (pre-reset ≤250 ms)** | **OPEN** — Declared gap XW-G2 correctly recorded; no real reset on this image; authority disposition required. |
| **Independent re-review of corrected content** | **NOT PERFORMED PRIOR TO THIS REVIEW** — PA review and sprint-plan confirm: "No independent re-review of corrected content performed." This delta review is the first independent assessment of the post-correction evidence. |

**Overall delta review verdict: PASS WITH COMMENTS AND OPEN FINDINGS**

The post-correction evidence **technically satisfies** the XW-2 target-contract test specification (TS-7 Rev 1.1) for the 13 registered cases and the negative control. The corrections requested by the PA review (PA-01…PA-13) and the original V&V review (F-01…F-20) have been **implemented in the artefacts and evidence**.

**However**, the following **open items prevent unconditional use of this evidence for criticality-B V&V gates**:

1. **DEV-T25-01 approval outstanding** — Software PA authority must approve or reject the deviation before the blocking gate's status is process-compliant.
2. **Gitea `xw2-target` job not yet executed and retained** with the corrected configuration — the authoritative per-commit CI evidence is prospective.
3. **Cross-CI equivalence (Gitea vs GitHub) undemonstrated** — two provisioning paths for one gate.
4. **No independent re-review of the corrected content** prior to this delta review.
5. **Sprint-plan revision/date not incremented** for the 2026-11-02 corrections (minor, N16).

These are **process and evidence-chain findings**, not technical result findings. The technical result (13/13 PASS, control EXPECTED FAILURE, wrapper segregation asserted, evidence hashes verified) is sound and correctly bounded.

## 5. Recommendations to decision authorities

| Rec # | Recommendation | Owner | Gate |
|-------|----------------|-------|------|
| R-01 | **Approve or reject `DEV-T25-01`** — the bounded deviation for using unqualified T25 as a blocking exit-status gate. Until approved, the Gitea `xw2-target` gate runs without process-compliant risk acceptance. | Software PA authority | N10 / N20 |
| R-02 | **Execute and retain a Gitea `xw2-target` run** with the corrected workflow (`FF_USE_PUS_ASN1=OFF`, no `make pus-asn1`) and the corrected `qemu_xw2_vv.sh`. Archive the run directory as the authoritative per-commit evidence. | CI/CD engineer + FSW engineer | N6 / N11 |
| R-03 | **Demonstrate cross-environment behavioural equivalence** between Gitea (pre-seeded `frick-rtems-toolchain:local`) and GitHub (RSB bootstrap) for the XW-2 gate, or designate one as authoritative and the other as supplementary with explicit scope. | CI/CD engineer + V&V engineer | N6 |
| R-04 | **Conduct independent re-review of the corrected content** (this delta review satisfies the technical evidence review; a process/PA re-review of the deviation and CI executability is recommended). | V&V engineer (R2) / PA engineer (N20) | N9 / N20 |
| R-05 | **Increment `sprint-plan.md` revision to 1.117 (or 1.116.1) with date 2026-11-02** to reflect the post-correction state. | FSW engineer | N16 |
| R-06 | **Obtain authority disposition on SRS-REQ-074 (XW-G2) and SRS-REQ-075 software at-most-once enforcement** — these remain open requirements gaps correctly declared but not closed. | System/FSW authority | Requirements baseline |

## 6. Non-claims (reaffirmed)

This delta review **does not establish**, and the evidence must never be cited as establishing:

1. **Norm N18 functional validation** — remains FAIL (NC-EXT-198, root cause NC-PAL-01).
2. **Device, HIL, or physical target behaviour** — the three PAL hooks are substituted at link time by a test double.
3. **Real reset, reset scope, or electrical/clock independence** — C3 independence is modelled only.
4. **Timing, WCET, jitter, interrupt latency, or on-target cadence** — XW-B1 asserts counts only; QEMU `virt` is not timing-representative.
5. **Mission timeout** — the 8 s table value remains an ESC-7 placeholder.
6. **T25 tool qualification** — T25 remains NOT QUALIFIED per N10; `DEV-T25-01` deviation approval outstanding.
7. **SRS-REQ-074 target evidence** — XW-G2 has no real reset on this image.
8. **SRS-REQ-075 software at-most-once enforcement** — enforcement in this model is by the device; authority interpretation open.
9. **Certification, airworthiness, safety-case, security, residual-risk, waiver, or release/flight approval** — AI expert-persona review only; human approval mandatory.
10. **Equivalence of Gitea and GitHub toolchains/runs** — cross-environment equivalence undemonstrated (ND-03).

## 7. Confidence assessment

| Aspect | Confidence | Justification |
|--------|------------|---------------|
| Technical result (13/13 PASS, control EXPECTED FAILURE) | **High** | Direct retained evidence: run directory archived, 23/23 hashes verified, ELF identities bound, nm-segregation asserted, elapsed recorded, nonce recorded, backend/toolchain identified. |
| Evidence integrity & supersession discipline | **High** | Pre-correction runs correctly marked SUPERSEDED HISTORY; post-correction archive complete with README and `assets.sha256`; all document pointers reconciled. |
| Claim discipline & non-claims | **High** | Explicit non-claims in TS-7, `verification-results.md`, `tool-qualification.md`, sprint-plan, PA review; no overclaim detected. |
| Tool qualification (T25) & deviation process | **Medium** | Deviation correctly recorded with named owner and compensating controls, but **approval outstanding**; gate runs without formal acceptance. |
| CI executability (Gitea) | **Medium** | Workflow corrected statically; **no executed retained run** with corrected config. |
| Cross-CI equivalence | **Low** | Two provisioning paths; no behavioural equivalence evidence. |
| Independent re-review of corrected content | **Medium** | This delta review is the first independent assessment; no prior R2/PA re-review of post-correction artefacts. |
| Requirements gaps (XW-G1..G5) | **High** | Correctly declared with requirement, reason, closure authority; no false closure. |

## 8. Archival provenance

- **Review ID**: `VV-REVIEW-XW2-DELTA-20261102`
- **Date**: 2026-11-02
- **Reviewer profile**: Senior Aeronautics and Space SW Verification & Validation (SW V&V) Engineer (R2 profile)
- **Configured subagent**: `aeronautics-space-sw-vv-engineer`
- **Harness session**: (this session)
- **Model / route**: `nvidia/nemotron-3-ultra-550b-a55b:free` (provider `openrouter`)
- **Scope**: Delta review of corrected XW-2 target-contract increment (commit `ae278ef8734d12e69a20d41ba367c12c16155afe`) against original V&V review `VV-REVIEW-XW2-20261101` (session `ses_e67b8290bffe5Edah5mlqBsL3g`) and PA review `PA-REVIEW-XW2-20261102` (session `ses_e663d6753ffeUrSrkkLe6Fj7IK`).
- **Method**: Read-only review of repository contents and evidence paths; no builds, tests, or formatters executed.
- **Evidence verified**: All 8 changed files read; post-correction evidence archive README and `assets.sha256` (23/23 files OK) verified; original V&V review and PA review findings cross-referenced; script sha256 verified against `qemu_xw2_vv.sh` content.

> **Authority statement.** This is an AI expert-persona review, not a human credential, qualification, certification, or approval. It does not approve gates, close non-conformances, authorize release or flight readiness, or waive any applicable requirement. Human approval is mandatory for all decision gates. The findings and verdict above are advisory to the human decision authorities.

---

*End of delta review document.*