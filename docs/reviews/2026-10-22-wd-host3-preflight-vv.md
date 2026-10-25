# FRICK-PREFLIGHT-WDHOST-002 — Host watchdog model/contract design (senior SW V&V preflight, read-only, independent)

**Profile:** senior Software Verification & Validation (R2), independent, fresh context
**Date:** 2026-10-22 — **performed BEFORE implementation**
**Subject:** the proposed verification design for the three missing host watchdog cases (lock enforcement, forbidden refresh window, clock/gating) and the corresponding stateful mock/key semantics, under owner approval Telegram1447 (2026-10-22 10:28), narrow B1/F(a) slice
**Disposition:** **Read-only.** No file created, edited or deleted by the reviewer; no build, test, container, emulator, probe or gate executed; no NC closed or reassigned; no gate, parameter, design, agenda item or release decision approved.

> This record is retained so that the acceptance conditions the preflight set are
> auditable (N9/N11). The implementing session applied them; the disposition table in §4
> states where each landed. The post-implementation reviews are FRICK-REVIEW-0433
> (embedded safety) and FRICK-REVIEW-0434 (senior SW V&V).

## 1. Verdict as issued

**APPROVED WITH REQUIRED CHANGES.** The approach was sound in its fundamentals — the
link-wrap seam exercises the **real, unchanged production PAL contract** against an
explicit host model, the labelling as HOST MODEL/CONTRACT is correct and necessary, the
mutation protocol is the right instrument, and the phase is correctly bounded to
test-side files. However, the design as stated contained **one defect that would silently
destroy the frozen NC-1 discrimination** (RC-2), **one defect that would let the
NC-EXT-205 residual survive while being reported as addressed** (RC-1), and **one factual
premise that cannot hold** (RC-3). RC-1…RC-10 were blocking for execution as specified;
RC-11…RC-16 were blocking for the evidence record's completeness and honesty; RC-13 (an
independent review of the model fixture under N10) was blocking before the phase's evidence
could be treated as verified.

## 2. The blocking points as issued

| RC | Requirement | Severity |
|---|---|---|
| RC-1 | The wrong-key stimulus must comprise **≥2 (recommended ≥3) distinct wrong-key values**, each individually asserted; a class predicate exercised with a single value does **not** close the NC-EXT-205 residual. Add the "reload on every key except one specific wrong value" mutant and record it as killed only if ≥2 wrong keys are used | Major |
| RC-2 | Every reload/no-reload assertion must be issued from a counter value **observably different from the reload value** (advance ticks first) and must compare against the **pre-write** value as well; otherwise the key-omission mutant is **survivable** and the positive case silently stops discriminating | Major |
| RC-3 | The evidence must **not** claim the original manifests "still verify 22/22 and 34/34"; record the honest **before/after** state of both, naming the deviating entries with the reason. "Original manifests untouched" = the manifest *files* are not edited | Major |
| RC-4 | The mutation protocol must distinguish an **assertion failure** from a build/link failure, crash or timeout: record the mutated artefact and exact semantic change, the mutated build's exit code 0, the rebuilt binary digest, the **specific failing assertion** of the designated case, the digest-verified restore and the re-run PASS; split the gating mutation into *access accepted* and *clock advances* | Major |
| RC-5 | The lock case must be exercised under **both** lock-rule variants with the same expected result, and the addendum must state that **neither variant is device truth** | Major |
| RC-6 | Isolation: a successful init and precondition assertions per case, the model reset per case, **no use** of the status-injection seam (it is inert under the wrap), and the starvation helper not used as a counter substitute | Major |
| RC-7 | The exact HOST MODEL / HOST-CONTRACT wording in the addendum header, the test-file header and the evidence log, with an evidence-class label on every case row and the non-claims list reproduced | Major |
| RC-8 | The two time domains must be explicitly separated; the model's tick domain has **no** defined relation to the PAL's millisecond domain; the model's values are **synthetic test parameters** | Major |
| RC-9 | The "counter does not advance while gated" must be stated as verification of the model's **declared assumption**, not of device behaviour | Major |
| RC-10 | The mutations must be labelled **model-level** evidence demonstrating the cases' discriminating power; the frozen NC-1…NC-4 stay **NOT RUN**; no claim of sensitivity to a defect in the production state machine | Major |
| RC-11 | Record the **ctest inventory delta** (exactly +1; none removed/renamed/disabled) | Moderate |
| RC-12 | The **NOT RUN** register must enumerate exactly the frozen items, and NC-EXT-205 must remain **OPEN** with a dated cross-reference only | Moderate |
| RC-13 | The model fixture is a **V&V tool** (N10): its qualification requires source review, inspection and configuration management with the independence the risk warrants; the phase must record the review status and must not describe the cases as "verified" while the fixture is unreviewed | Moderate |
| RC-14 | The ordering check must be labelled a **harness-capability self-check**; if reported as evidence it must carry its own order-swap mutation; it is **not** HS-P5 and **not** NC-3 evidence | Moderate |
| RC-15 | Traceability: do **not** add the new executable to a requirement's test list in a way that inflates requirement coverage — the cases verify a fixture against a specification, not a controlled requirement | Moderate |
| RC-16 | If the addendum uses the N22 merge schema it must declare the deviation (that schema mandates a requirement id and a SIL/PIL/HIL environment, neither of which exists for a host-model case) | Minor |

## 3. The specific questions as answered

- **Are the oracles independent and non-tautological?** Every negative case must carry a
  **state** observable (register read-back / counter value / flag transition) and a
  **production observable** (the feed counter), with the double's return value and the
  observer's disposition demoted to corroborating evidence; the healthy half is
  load-bearing because it is the only thing that can fail for a do-nothing mock.
- **Is the mutation set the right one?** The frozen NC-1…NC-4 are mutations *against the
  binding*; the proposed mutations act on the model and therefore demonstrate the cases'
  sensitivity to a defect of the same shape in the double — they are not NC-1…NC-4.
- **Would a wrong-key negative survive a model that reloads on any key?** No — provided
  the counter was advanced first (RC-2). But a model that reloads on everything *except
  one specific wrong value* **survives a single-value stimulus**, which is exactly the
  NC-EXT-205 residual: a class stated as a predicate and exercised with one value does not
  close it (RC-1).
- **Is the positive case discriminating for key omission?** Conditionally yes, with the
  RC-2 construction detail: with the counter advanced away from the reload value, a
  key-omitting feed leaves it there and the assertion fails; without that construction
  the key-omitting feed and the correct feed are indistinguishable and the case passes
  under the mutation. The wrong-key negative alone is insufficient — it is satisfied by
  any model that never reloads.
- **Are the existing oracles preserved?** Yes, with an isolation requirement: the wrap
  options are attached only to the new target; the existing harness file is a manifest
  entry and must remain byte-identical; each case must start from a successful init with
  the preconditions asserted, because the harness continues after a failing case;
  the status-injection seam must not be used because the wrap bypasses it.

## 4. Disposition of the preflight conditions (recorded by the implementing session)

| RC | Where it landed |
|---|---|
| RC-1 | the test source uses **three** distinct wrong keys; MUT-4b and MUT-4c are added and both are killed by the key-class case |
| RC-2 | every reload/no-reload assertion is issued after an explicit tick advance and compares against the pre-write value; the addendum states the two initial-state observations that are not reload/no-reload discriminations |
| RC-3 | the evidence log records the measured pre-phase baseline (22/22 and 34/34) and the measured post-phase result (22/22 and 33/34, the single deviation named with its norm); the manifest files are unedited and not re-baselined |
| RC-4 | the mutation protocol records the artefact, the exact change, the mutated build exit code, the failing assertion lines, the digest-verified restore and the passing re-run; the gating mutation is split into *access accepted* (MUT-3a) and *clock advances* (MUT-3b) |
| RC-5 | the lock case runs under both variants, and a **variant-discriminating leg** was added with MUT-6 (the TIME-variant branch removed) so that the rule dimension is covered |
| RC-6 | every PAL-touching case starts from a successful init and asserts its preconditions; the status-injection seam is unused and documented as inert under the wrap; the starvation helper is not used |
| RC-7 | the evidence-class wording appears in the addendum header, the test-file header, the fixture header and the evidence log, with a class column in the case tables |
| RC-8 | the two domains are separated explicitly in the addendum and in the test header; the timeout argument is never converted |
| RC-9 | the gated-clock behaviour is registered as a model-only assumption with the "armed but inert" caveat |
| RC-10 | the mutations are labelled model-level fixture-sensitivity evidence; NC-1…NC-4 remain NOT RUN; no sensitivity to the production state machine is claimed |
| RC-11 | the ctest inventory delta (45 → 46, exactly +1) is recorded in the evidence log |
| RC-12 | the NOT RUN register enumerates the frozen items, and NC-EXT-205 remains OPEN with a dated cross-reference only |
| RC-13 | the addendum records the fixture as a V&V tool pending N10 qualification, and the two independent post-implementation reviews are filed |
| RC-14 | the ordering check is labelled a harness-capability self-check and carries its own order-swap mutation (MUT-5) |
| RC-15 | `traceability.yaml` is deliberately **not** updated, with the reason stated, and `make trace` passes unchanged |
| RC-16 | the addendum is **not** a package file under `docs/vv/test-spec/` and declares the deviation from the N22 merge schema |

**Authority statement.** This preflight approves the *verification design* subject to the
changes above. It approves no gate, parameter, design, NC disposition, agenda item or
release decision, and it asserts no target, HIL or hardware evidence. Telegram1447 remains
bounded to the three host cases and the corresponding mock/key semantics.
