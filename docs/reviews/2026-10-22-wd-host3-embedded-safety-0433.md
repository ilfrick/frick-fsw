# FRICK-REVIEW-0433 — Host watchdog model/contract cases (post-implementation, embedded safety, read-only, independent)

**Profile:** embedded-safety (senior aeronautics/space avionics), independent, fresh context
**Date:** 2026-10-22
**Subject reviewed:** `tests/stubs/ff_test_wd_model.{h,c}`, `tests/unit/test_wd_host_contract.c`, `tests/CMakeLists.txt` (wiring only), `docs/vv/host-wd-model-contract-addendum-2026-10-22.md`, `docs/vv/evidence/2026-10-22-wd-host3-host-model-contract.log`
**Authority reviewed under:** owner approval Telegram1447 (2026-10-22 10:28), narrow B1/F(a) slice
**Disposition:** **Read-only.** No file created, edited or deleted by the reviewer; no build, no test execution, no container, no emulator, no probe, no target/HIL access; no NC closed, reopened or reassigned; no gate, parameter, design or hardware fact approved; no commit or push.

> **Artefact state at the time of this review (digests recomputed by the reviewer):**
> `ff_test_wd_model.h` `bc69cb69…ad77`, `ff_test_wd_model.c` `c24b8b45…43868`,
> `test_wd_host_contract.c` `6a454b13…618a8`, evidence log `afd42cd0…8b96` (293 lines);
> frozen bodies `9b09605c…73d40` / `749ecafc…f35e5` / `54a44964…e36ed` / `64139566…6c9bc`
> all byte-identical; the 22-entry documentation manifest 22/22 OK (exit 0); the 34-entry
> code manifest 33 OK / 1 FAILED (`tests/CMakeLists.txt`).
>
> **This review was issued against those digests.** The blocking findings below were
> applied afterwards by the executing agent, which produced a **new** artefact state
> (`ff_test_wd_model.c` `b07163c3…a384`, `ff_test_wd_model.h` `fbfa9251…d77`,
> `test_wd_host_contract.c` `bfa1e020…b23dc`, addendum `be598b96…d8f6`, evidence log
> `40befa05…ccb6`). **The reviewer has not re-reviewed that state**, and this record does
> not claim otherwise; the corrections and the re-run gates are recorded in the addendum
> (§4, §13) and in the evidence log (§6, §7). The findings below are reproduced as issued.

## A. What the reviewer verified independently (reproduction, not trust)

| Claim | Method | Result |
|---|---|---|
| Digests of the three new sources + evidence log | `sha256sum` | all four match |
| Frozen bodies byte-identical | `sha256sum` | all four match |
| 22-entry documentation manifest | `sha256sum -c` | 22/22 OK, exit 0 |
| 34-entry code manifest | `sha256sum -c` | 33 OK, 1 FAILED (`tests/CMakeLists.txt`), exit 1 |
| The single deviation is **only** this phase's wiring | the pre-phase `tests/CMakeLists.txt` was **reconstructed** by deleting exactly the phase's three blocks and hashed | reconstruction = `aa11c7be…e5a2` = **the manifest entry exactly**; the log's deviation analysis is correct and complete |
| Manifest files unedited / not re-baselined | `sha256sum -c` on both | true |
| No other file touched by the phase | `find -newermt` over the phase window (excluding `build/`, `.git/`) | **exactly six files**: the 3 new sources, `tests/CMakeLists.txt`, the addendum, the evidence log — this independently confirms the log §0 "NOT changed" list and all deferrals (every deferred file's mtime predates the phase start) |
| Link-level false-PASS guard | `nm` on both binaries | `test_wd_host_contract`: real hooks **and** `__wrap_*` present; `test_pal_watchdog`: real hooks only, **no** `__wrap_*` → the production host backend and its oracles are untouched |
| "113 checks" nominal count | static re-derivation from the harness count | 24×2 + 22 + 15 + 3+4×3 + 10 + 3 = **113 — exact match** |
| Mutation records | re-derivation of each mutated failing set from the model semantics | **all eight match the log exactly** (including the two lines MUT-1 correctly omits) |
| Restore digests | `sha256sum` of the pristine copies and of the current sources | pristine = current → "digest-verified restore" holds |
| Format gate | `clang-format --dry-run --Werror` with a **newer** clang-format (23.1.3) than the pinned 18.1.3 | exit 0 on all three files — clean on a newer version is a stronger signal |
| Upstream-identifier guard (substance) | `grep` for vendor identifiers over the three new files | only three hits, all in **disclaimer** text ("the MPFS/MSS binding is NOT WIRED"); no `MSS_WDOG_*`, `CONTEXT_EN_MASK_WDOGx`, `SUBBLK_CLOCK_NA_MASK`, `APBBUS_CR_*`, instance bases or force-reset key → the substance holds (but see B-4) |

**The technical substance of the phase is sound.** The oracles are literal-based and
independent, the mutation evidence is reproducible, the labelling is disciplined, and the
manifest statement is honest — verified, not taken on trust.

## B. Blocking findings (as issued)

| # | Finding | Corrective change demanded |
|---|---|---|
| **B-1** | The **phase identity manifest does not exist** (`docs/vv/evidence/2026-10-22-wd-host3-identities.sha256`), although the addendum (header, §4, §11) and the log (§0, §12) name it as the authoritative identity record and as the qualification anchor. The owner's "a NEW phase manifest for the phase's own files" constraint is unmet; the digests survive inline, so no technical result is lost, but the artefact that makes the "digest-pinned test double" auditable is missing | create the manifest with the phase files, plus the pre- and post-phase digests of the changed pinned entry and of the eight deferred manifest entries |
| **B-2** | The log lists **two review records as "created" that do not exist** (`…-embedded-safety-0433.md`, `…-vv-0434.md`); the log's §0 file inventory is false as written | file the records; make the inventory match the filed state |
| **B-3** | The **preflight review is claimed "completed" but no record of it exists** in `docs/reviews/`, so a reader cannot check which wording changes it demanded (N11 requires review results to be stored in the repository) | file the preflight records, or reword the claim and register the gap |
| **B-4** | The **"RC-G grep guard" is not a traceable artefact** — the identifier `RC-G` exists nowhere in the repository, so a compliance requirement the phase claims to satisfy is undefined | define the guard in the addendum as the concrete command actually run, and record its result |
| **B-5** | Register row **A6 is mis-classified and contradicts C2 and forbidden reading #3**: it attributes to F-8 the statement "a gated peripheral refuses accesses", which F-8 does not make (F-8: a gating mechanism exists, its bit values are undefined); the "must not report success" obligation is the **binding** case HS-P8, which the addendum itself lists as NOT RUN. The register thus re-introduces, through its class column, the very reading §8 forbids | split A6 into a documented row ("a peripheral gating mechanism exists; its bit values are undefined", F-8) and a model-only row ("a gated access is refused, ahead of the key and window checks"); amend the fixture header comment accordingly |
| **B-6** | **F-4's zero-reset clause is omitted from A2 and the fixture implements the opposite, unregistered**: F-4 states "the counter keeps counting and **a reset is generated at zero**"; the fixture decrements to zero and stops, and has no reset observable. The register's completeness claim ("every behaviour … is classified") is false, and the omission is in the safety-relevant direction | reword A2 as "F-4 minus the zero-reset clause" and add a model-only row (C8) for the counter halting at zero with no reset generated |
| **B-7** | The addendum **over-claims the fixture as "qualified"** ("a digest-pinned, mutation-anchored test double **qualified** for future binding work") while §14 states that the N10 qualification review is still required and the tool register is deferred | replace "qualified" with "**pending N10 qualification**" |

## C. Required-but-deferrable findings (as issued)

| # | Finding |
|---|---|
| **C-1** | HM-P1's two lock-rule variants are **not discriminated**: under both readings the lock engages on the `TIME` write, so no assertion distinguishes them, and no mutation covers the rule dimension (a defect deleting the TIME-variant branch would survive). The *code* claim ("both readings implemented") is true; the *evidence* claim ("exercised under both variants") overstates it |
| **C-2** | The fixture keeps "success-programmed" defaults on out-of-range and unused seams: the out-of-range accessors return the benign `applied` token and the `STATUS` register; the 64-record observer truncates silently; the double's reset hook returns `FF_SUCCESS` unconditionally with no observable |
| **C-3** | The double's feed-success criterion is **degenerate when the counter already equals `TIME`**: a refused feed would be reported as success. The current cases are immune (they always move the counter first), but the degeneracy lives in the reusable tool and is unregistered |
| **C-4** | The forbidden-window **boundary is untested and undisclosed**: F-5 turns on the strict inequality "forbidden while the counter is greater than `MSVP`", but the cases use only `MSVP`+50 and `MSVP`−10; `counter == MSVP` and `MSVP`+1 are never exercised (N22 requires exact/near-boundary values where relevant) |
| **C-5** | The **TRIGGER crossing and the MVRP crossing** are implemented but never witnessed: the counter never descends below 890 (`TRIGGER` = 10), and the MVRP crossing is never asserted |

## D. Minor findings (as issued)

D-1 A1 over-claims the offset map (F-1 gives names and instance **base** addresses, not a per-register offset map; 0x10/0x14/0x18 are inferred by stride) · D-2 A3's source attribution is imprecise (the negative form comes from the §5.2 preamble) · D-3 A5's "not device truth" cell is empty · D-4 un-registered model behaviours (the never-written `REFRESH` register and its unreachable switch case; the `loaded` term inside the clock-running observable; ticks discarded beyond zero) · D-5 the double's comment over-claims a documentary basis ("the documented feed rule") · D-6 the register's scope sentence covers only "the fixture", not the double · D-7 F-19's issuer condition ("if VDD = 1.2 V") is dropped · D-8 "exactly two artefacts are exercised" is imprecise (the host monotonic clock is called) · D-9 HM-P2's comment says the counter "reached" the maximum-value register when it fell below it · D-10 the `n22`-label rationale conflates "requirement-based" with N22 · D-11 the "+1 / 46 of 46" figure is conditional on the platform/compiler guard · D-12 `traceability.yaml`'s "unchanged" claim rests on the author's word (it is in neither manifest) · D-13 the per-mutation logs' "failing assertions" extraction block is empty (the list is carried by the evidence log) · D-14 the test binary's stdout carries no evidence class.

## E. Point-by-point answers

1. **Does the implementation match the addendum?** Mostly yes, with three divergences: A6 contradicts C2 and forbidden reading #3 (B-5); A2 omits F-4's zero-reset clause and the fixture's contrary behaviour is unregistered (B-6); A1 attributes a full 0x00–0x18 offset map to F-1, which documents names and instance bases only (D-1). The code banners are **more** careful than the register in two rows — the register is where the discipline slips.
2. **Model-semantics register audit.** The five behaviours specifically examined are all disclosed (clock-while-gated, gated-access response, disposition precedence, tick domain, power-up state). Not disclosed, and registered late: the counter halting at zero with no reset (B-6), the never-written `REFRESH` register (D-4a), the `loaded` term in the clock observable (D-4b), the observer truncation (C-2.2), the out-of-range "applied" default (C-2.1), the double's reset hook (C-2.3), the double's feed rule (D-5) and its degeneracy (C-3). Three items read as device facts: A6, the offset map, and the zero-reset omission — all *citation* defects, none reaching §8's forbidden readings except A6.
3. **Is any safety claim implied that must not be?** Not from the case list: the ctest name, the labels, the case ids, the fixture name, the observer vocabulary and the source banner all carry the HOST-MODEL qualifier. The one leakage path is the register (B-5).
4. **Does the "no fabrication of the binding" line hold?** Yes, in substance: the banners are explicit and accurate; there is no MMIO, no register access, no vendor header and no SDK call in the three new files; the `--wrap` is target-scoped and `test_pal_watchdog` demonstrably keeps the production host backend. The double is binding-shaped but that is disclosed in three places and it "contains no peripheral semantics". Two caveats: the guard has no traceable definition (B-4) and the double's feed rule is labelled "the documented feed rule" without documentary support (D-5).
5. **Is the "no success-programmed mock / no self-restating test" rule satisfied?** In the executed code yes; in the tool as a reusable artefact not yet (C-2, C-3). In the test body: every expected value is a literal constant; no assertion compares one model-derived value with another; the feed outcome is genuinely derived from a post-write read-back; the production feed counter is an oracle independent of the model; every negative leg asserts a state observable besides the disposition token, plus the exact token; every reload/no-reload assertion is issued from a counter value observably different from `TIME`.
6. **Residual truthfulness.** Yes — the strongest part of the phase. NC-EXT-205 is correctly left OPEN with no status/owner/severity change (its file is a manifest entry and verifies OK); the NOT RUN register is complete against the frozen body's own inventory; all seven agenda items read OPEN and the addendum keeps them OPEN; the manifest statement is honest and independently verifiable (the reviewer reconstructed the pre-phase file to the manifest digest); the "production-coverage delta: zero" claim is verified against `tests/unit/test_pal_watchdog.c:113-136`, whose file is a manifest entry preserved unchanged. Residual gaps in the residual list: the forbidden-window boundary (C-4) and the un-witnessed F-4 crossings (C-5); D-7 drops the F-19 issuer condition.
7. **Defects that must be corrected before the evidence is treated as sound:** B-1…B-7 and C-1's wording half. C-1's stimulus, C-2, C-3, C-4, C-5 and D-1…D-14 are deferrable. No production, configuration, requirement, traceability, target or emulator artefact is implicated, and no test result needs to be re-run because of any finding in this record.
8. **Verdict (as issued):** **APPROVED WITH REQUIRED CHANGES** — the *technical substance* is approved subject to the blocking corrections; the phase as a *record* is not yet sound.

## F. Explicitly not approved and not conferred

No gate (N18 remains **FAIL**); no agenda item (A, B1, B2, C, D, E, F all **OPEN**); no mission, device or design parameter (no instance, reset scope, NMI policy, timeout, cadence, margin, clock, prescaler, window value, gating value or lock rule); no hardware, board, target, HIL, emulator or register-level fact; no requirement-coverage delta (`traceability.yaml` unchanged, no `N22-*` identifier created, `N22-TS5-82` remains a gap); **no NC closed, reopened or reassigned** (NC-EXT-205 remains **OPEN**, as do NC-PAL-01, NC-EXT-198 and R-6 "armed but inert"); no sprint, release, qualification or flight-readiness statement; **no commit and no push**; Lean remains **PASS by retention** and N18 remains **FAIL by retention**; the fixture is **not** N10-qualified.

**Confidence:** High for the digests, the manifest reconstruction, the check-count and mutation re-derivations, the `nm` seam proof, the file-scope finding and the code-level findings (direct inspection of the bytes, with the frozen bodies and the two manifests as the control). Medium for C-1's severity and for D-10's classification.

**Detection limits:** text and byte-level inspection only — no build, execution, emulation, probe or instrumentation; the mutation outcomes, the ctest counts, the format-check result and the licence/trace gates were assessed from the retained logs and from independent static re-derivation. The non-retrievable "PolarFire SoC Device Register Map" (M-1) was not consulted, so no register, offset, lock, gating or window semantic could be confirmed against the device; the frozen body and the retrieval log are the only documentary basis used.
