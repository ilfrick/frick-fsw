# FRICK-PREFLIGHT-WDHOST-001 — Host watchdog model/contract design (embedded-safety preflight, read-only, independent)

**Profile:** embedded-safety preflight, independent, fresh context
**Date:** 2026-10-22 — **performed BEFORE implementation**
**Subject:** the proposed verification design for the three missing host watchdog cases (lock enforcement, forbidden refresh window, clock/gating) and the corresponding stateful mock/key semantics, under owner approval Telegram1447 (2026-10-22 10:28), narrow B1/F(a) slice
**Disposition:** **Read-only.** No file created, edited or deleted by the reviewer; no build, no ctest, no container, no emulator, no probe; no NC closed or opened by the reviewer; no gate approved; no hardware parameter, clock, timeout, instance or reset policy selected; no release or flightworthiness statement.

> This record is retained so that the changes the preflight demanded are auditable
> (N9/N11). The implementing session applied them; the disposition table in §4 states
> where each landed. The post-implementation reviews are FRICK-REVIEW-0433 (embedded
> safety) and FRICK-REVIEW-0434 (senior SW V&V).

## 1. Verdict as issued

**APPROVED WITH REQUIRED CHANGES.** The design was sound in its *approach* and compliant
with the owner's narrow scope: it changes no production file, fabricates no binding, keeps
the existing harness's oracles untouched, uses synthetic parameters only, and separates
nominal from mutated verdicts. It was **not acceptable as written**, because the *record*
around it — the id class, the requirement-coverage framing, the negative-control framing,
the manifest statement and the missing N13 gate — would have produced a document set that
a reader can misread as "lock, window and gating are verified" and "the frozen manifests
still verify 22/22 and 34/34". Both misreadings are avoidable at wording level.

## 2. Blocking points as issued (wording/structure, not scope)

| RC | Requirement |
|---|---|
| RC-A | A **three-class model-semantics register** (documented-and-cited / unresolved-and-parameterised / model-only), each entry naming its source and its "not device truth" sentence; class (iii) must be explicit and must include the clock-while-gated behaviour, the gated-access response, the tick domain, the synthetic values, the disposition precedence, the register widths and the benign power-up state |
| RC-B | State the **zero delta in production-code coverage** and that the three cases qualify a **test double**; cite the already-covering oracle `tests/unit/test_pal_watchdog.c:113-136` |
| RC-C | A **distinct case-id series** (the frozen `HS-P` series is binding-scoped) or a mandatory scope-class column plus a banner; the frozen HS-P1/HS-P2 identities must **not** be reused; HS-P1…HS-P8 remain **NOT RUN** |
| RC-D | **NC-1…NC-4 remain NOT RUN**; the mutations are labelled **fixture-sensitivity** evidence; the frozen §5.1 "no host case yet" cells reproduced with an explicit delta column; the qualification bound to a fixture digest with a re-qualification trigger |
| RC-E | **No `N22-*` id, no requirement-coverage delta**; SRS-REQ-073 unchanged; `N22-TS5-82` still a gap |
| RC-F | **Literal expected constants only**, no model-to-model comparison; the no-vacuity rule (exact disposition token + a non-token state observable + a precondition ledger + disposition precedence) written into the addendum |
| RC-G | Rename to a project-scoped fixture name with project-scoped symbols and no upstream driver identifiers (with a recorded grep guard); the mandatory "TEST DOUBLE — NOT A BINDING" banner; the key as a fixture constant with a cited source class; the SUT table; the two link-line proofs; the **wrap-level false-PASS guard** (link level **and** run time); the note that the wrap is executable-global |
| RC-H | The **eight forbidden readings**, each as a stated negative, in the addendum, the evidence log and the final report; the "no new production-code evidence" value statement; no "verified/validated" wording for the three properties |
| RC-I | The manifest statement corrected to **measured pre-phase baseline + measured post-phase result + enumeration of the changed pinned entries with the norm that required each**, with the new phase manifest as the authoritative phase identity record |
| RC-J | The **N13 licence check** added to the applicable gates (a gate, distinct from and without reference to the separate Telegram1441 licensing analysis); SPDX headers on the new files; the `clang-format` 18 identity recorded |
| RC-K | The concrete wiring details (`FF_TEST_TARGETS` membership, the platform/compiler guard, the ctest timeout and label, the ASN.1 tree digest) |
| RC-L | **Telegram1447 recorded with its exact scope and its explicit non-approvals**, following the escalations pattern (date, authority, scope, non-approvals) |
| RC-M | The **NC-EXT-205 disposition language**: a proposed partial disposition only, the NC left **OPEN**, no status/owner/severity change |

Also recommended and taken: the model-level negative key case exercises **two distinct
wrong-key values** so that the fixture does not reproduce, inside itself, the
single-special-value weakness NC-EXT-205 records for the binding.

## 3. Principal safety-relevant points as issued

- **A1 — the gated-clock behaviour is in the unsafe direction.** A device that stops
  counting while gated is an *armed-but-inert* false-protection case (R-6/F-11); the
  fixture's behaviour must not be read as the benign case.
- **A2/A3 — the lock rule and the power-up state are not device truth.** The upstream
  lock documentation contradicts itself (M-11) and the device comes out of reset already
  armed at ≈10.47 s (F-19), so the fixture's benign power-up state differs in the unsafe
  direction.
- The production property the new cases can reach is narrow — the backend status is
  propagated and a failed feed is not counted — and it is **already covered** by
  `tests/unit/test_pal_watchdog.c:113-136`.
- The three model semantics carry the same property names as the target cases (TG-P1
  "lock enforcement", TG-P3 "forbidden window"), so a reader scanning a case list can
  easily conclude that lock, window and gating are **verified** — hence RC-H.

## 4. Disposition of the preflight points (recorded by the implementing session)

| RC | Where it landed |
|---|---|
| RC-A | addendum §2 — the register now carries rows A1…A6, A1b, B1…B4, C1…C12, each with its source and its "not device truth" sentence; the class-(iii) list is explicit |
| RC-B | addendum §1 — "Production-code coverage delta: zero", with the already-covering oracle cited |
| RC-C | addendum §3 — the **HM-P** series with a scope-class statement and the banner; HS-P1…HS-P8 NOT RUN |
| RC-D | addendum §4 and §5 — MUT-1…MUT-6 labelled model-level fixture-sensitivity evidence; NC-1…NC-4 NOT RUN; the §5.1 cells reproduced with a delta column; the qualification bound to the fixture digests with a re-qualification trigger |
| RC-E | addendum §9 — no `N22-*` id, no requirement-coverage delta, `traceability.yaml` unchanged, `N22-TS5-82` still a gap |
| RC-F | addendum §3 — literal constants, the no-vacuity rule and the disposition precedence are stated; the test source uses literals only |
| RC-G | `tests/stubs/ff_test_wd_model.{h,c}` (project-scoped name and symbols), the "TEST DOUBLE — NOT A BINDING" banner in all three files, the key as `FF_TEST_WD_REFRESH_KEY` with its source class, the SUT table, the link-level and run-time proofs in the evidence log §4, and the grep guard defined and run (addendum §1) |
| RC-H | addendum §8 — the eight forbidden readings; the value statement reworded to "pending N10 qualification" |
| RC-I | addendum §11 and evidence log §2 — the measured pre-phase baseline and the measured post-phase result, with the single deviating entry named and attributed |
| RC-J | addendum §13 and the evidence log §7 — the N13 licence check and the licence-policy test are executed and recorded; SPDX headers are present; the clang-format 18.1.3 identity is recorded |
| RC-K | `tests/CMakeLists.txt` and the evidence log §1/§7 — target registration, the platform/compiler guard, the ctest timeout and labels, the generated ASN.1 tree digest with its command |
| RC-L | addendum §0 — Telegram1447 verbatim with the authorised/not-authorised table |
| RC-M | addendum §7 — the proposed NC-EXT-205 wording, explicitly **not** applied; the NC stays OPEN with no status, owner, severity or disposition change |
| R2 strengthening | addendum §3 and the test source — **three** distinct wrong-key values, with MUT-4b and MUT-4c proving that a model reloading on everything except one specific wrong value is killed |

**Authority statement.** This preflight approves no gate, no parameter, no design, no
hardware fact, no NC, no sprint, no release, and no commit/push. The safety-relevant
decisions remain with the authorised humans.
