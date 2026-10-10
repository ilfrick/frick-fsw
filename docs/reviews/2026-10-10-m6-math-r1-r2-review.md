# Milestone M6 Mathematics Library (libm) Review - profiles R1 and R2

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0019                       |
| Reviewer session                  | independent subagent, fresh context (norm N9) |
| Reviewer profile                  | R1 (Senior SW dev, design/code) **and** R2 (Senior SW V&V engineer), `docs/engineering/independent-review.md` |
| Reference revision                | working tree on base commit `17d9410` (branch `main`, uncommitted M6 delta) |
| Items under review                | `src/math/ff_math.{c,h}`; `CMakeLists.txt` (`FF_USE_LIBM`/`FRICK_USE_LIBM`/`-lm`), `tests/CMakeLists.txt`, `tests/unit/test_math.c`; `docs/requirements/traceability.yaml` (SRS-REQ-033, SSS-REQ-018), `docs/requirements/{SSS,SRS,traceability}.md`; `docs/adr/ADR-0012-libm-math.md`, `docs/architecture/numerics.md`, `docs/quality/licensing.md`, `docs/quality/ecss-compliance-matrix.md`, `docs/vv/verification-results.md` (M6 row/notes) |
| Date                              | 2026-10-10                             |
| Scope                             | Correctness and defensive behaviour of the libm facade; real-time/determinism safety of floating point; conformance to norms N1-N17 and ECSS-E-ST-40C / ECSS-Q-ST-80C; requirement quality/verifiability; bidirectional traceability; test correctness (tolerance not equality); licensing; acceptability of the `lean_note` for SRS-REQ-033 |
| Independence declaration          | I am a freshly spawned independent reviewer with no prior context. I took no part in authoring, building, qualifying or executing any item under review; I relied solely on the repository contents and the applicable norms/standards. I modified no file except this report. |
| Norms applied                     | N1-N17 (see `docs/rules.md`), with emphasis on N1, N3, N4, N8, N10, N11, N13, N15, N16 |

## 1. Scope

Reviewed the milestone-M6 ("mathematics library / libm integration") delta:
the portable mathematics facade (`src/math/ff_math.{c,h}`), its build wiring
(`FF_USE_LIBM`, `FRICK_USE_LIBM=1`, `-lm`), its unit test and CTest target, the
new requirements and manifest entries (SSS-REQ-018 / SRS-REQ-033), the ADR and
numerics/licensing policy documents, the compliance matrix reference, and the
M6 row/notes in the V&V record.

Reproduced on the review host (base commit `17d9410`, uncommitted M6 delta):

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` ->
  **PASS (65 requirements, 34 srs, 18 sss, 13 gaps, 45 Lean theorems, 9 CI
  claims)**, exit 0.
- `cmake -S . -B build/m6rev -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build
  build/m6rev && ctest --test-dir build/m6rev --output-on-failure` -> **100%
  tests passed, 0 failed out of 7** (`ccsds, sb, tbl, pus, time, hal, math`).
  The `math` binary reports `checks=18 failures=0`, matching the recorded
  `math18`.
- Cross-check of the optional switch:
  `-DFF_USE_LIBM=OFF` builds and yields **6/6** tests (no math target), and
  `ff_math.h` emits the intended `#error` when `FRICK_USE_LIBM` is undefined.
- `python3 tools/scripts/coverage.py` -> **lines 932/1575 (59.2%), functions
  141/184**, exactly reproducing the M6 row and README panel.
- Independent greps: no `#include <math.h>` and no `double`/`float` in the
  framework core (`src/core`, `src/pus`, `src/transport`, `src/hal`).

Excluded from scope: M4 driver implementations, on-target/HWIL qualification
(GAP-A5), the Lean gate re-execution, and the substantive content of unrelated
M6 periphery except where it bears on consistency.

**Limitations.** `lake`/`lean` are not installed on the review host, so
`make verify-lean` could not be re-executed here; the M6 delta adds no Lean
theorem and the committed transcript is unchanged (`19/19`), so N1 for
SRS-REQ-033 rests on the manifest `lean_note` (see R1-3). Re-running the Lean
gate in the pinned Docker V&V image is recommended for the record. The R3
(N14) and R4 (N17) reviews of this delta are separate profiles and are not part
of this report (see R2-6).

## 2. References

- Project norms: `docs/rules.md` (N1-N17); reviewer profiles:
  `docs/engineering/independent-review.md` (N9).
- Traceability manifest/gate: `docs/requirements/traceability.yaml`,
  `tools/scripts/trace.py`, `docs/requirements/traceability.md`.
- ECSS-E-ST-40C (software requirements/design/verification coverage, records),
  ECSS-Q-ST-80C (product assurance, tool qualification/independence),
  ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.
- V&V records: `docs/vv/verification-results.md`,
  `docs/vv/tool-qualification.md`, `docs/vv/verification-plan.md`.

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R1-1 | Minor | `docs/design/software-design.md` (SDD), `docs/engineering/user-integration-manual.md` (N8) | The new module `src/math/` (a user-facing capability introduced by SRS-REQ-033) is not reflected in the detailed design. The SDD architectural table and the per-module detailed design enumerate every other module (incl. the M3 HAL) but not the math facade; the user manual documents PAL/HAL/PUS/link but never the facade applications are told to use. M3 set the precedent of extending the SDD when a module was added. | `docs/design/software-design.md` §2 table lines 35-43 and §3 bullets lines 51-73 contain no `src/math`/`FF_Math` entry. `docs/engineering/user-integration-manual.md` lines 115-123 mention PAL/HAL only. `git log` shows the M3 commit `17d9410` added the HAL module to `software-design.md`. | Add a math-module row to SDD §2 and a detailed-design bullet to §3 (facade contract, no allocation, determinism boundary); add a short "floating-point math" subsection to the user manual. |
| R1-2 | Minor | `src/math/ff_math.c` `FF_MathClamp` (defensive behaviour) | `FF_MathClamp` does not define or guard the `lo <= hi` precondition and passes NaN through unchanged (`x < lo` and `x > hi` are both false for NaN). The header contract (lines 45-51) documents no precondition for `lo`/`hi`, so a caller passing `lo > hi` silently gets `lo` or `hi` depending on `x`. | `src/math/ff_math.c` lines 66-75; contract `src/math/ff_math.h` lines 45-48. No test covers `lo > hi` or a NaN input. | Document the `lo <= hi` precondition (or make the behaviour total), and state that NaN input yields NaN (detectable with `FF_MathIsFinite`). Add a test for the `lo > hi`/NaN cases. |
| R1-3 | Comment | `lean_note` for SRS-REQ-033 (N1/N3) | **Verdict: acceptable for the libm-backed functions, but overbroad.** The substantive behaviour of the 10 wrappers lives in third-party `libm`, which is not Lean-modelable, and interpretation note 1 in `docs/rules.md` (lines 313-321) admits `lean_note` for artifacts whose properties do not admit a formal model. However, the facade also contains pure helpers (`FF_MathClamp`, `FF_MathDeg2Rad`, `FF_MathRad2Deg`, `FF_MathIsFinite`) whose properties (bounds invariant, degree/radian round-trip, finiteness classification) *are* Lean-checkable; the blanket note "not Lean-checkable" is therefore wider than necessary. | `docs/requirements/traceability.yaml` SRS-REQ-033 `lean_note`; `src/math/ff_math.c` lines 57-81; interpretation note 1 in `docs/rules.md`. | Narrow the `lean_note` to the libm-backed functions and either add a small Lean model/theorem for the pure helpers (bounds + unit conversion) or state explicitly why they are excluded. |
| R1-4 | Comment | `docs/quality/licensing.md`, `CMakeLists.txt` (N8/N13) | The new licence row is directionally correct but imprecise: glibc is **LGPL-2.1-or-later** (not plain "LGPL-2.1") and newlib's licence is a BSD/MIT-style mixture (not cleanly "MIT-like"). Separately, `target_link_libraries(frick_fsw_core PUBLIC m)` hardcodes the linker name `m`, which is correct for the Linux/RTEMS targets but not portable (e.g. macOS has no `libm`); acceptable given the declared platforms. | `docs/quality/licensing.md` added row "Platform `libm` ... LGPL-2.1 (glibc) / MIT-like (newlib)"; `CMakeLists.txt` lines added around the `FF_USE_LIBM` block. | Tighten the licence wording ("LGPL-2.1-or-later"; "newlib BSD/MIT-style mixture"); optionally guard the library name for non-Linux hosts. |
| R1-5 | Comment | `docs/quality/ecss-compliance-matrix.md` (N8/N16) | The compliance matrix is in the review item set but unmodified. Its "Software reuse / COTS analysis" row names only ADR-0002/0003, and it has no M6 history entry, so a reader cannot see that the platform `libm` is now a reused component with an ADR/licence analysis. | `docs/quality/ecss-compliance-matrix.md` line 39 (`ADR-0002/0003`) and History line 107 (rev 0.1). `git status` shows the file unchanged in the M6 delta. | Add ADR-0012 / `docs/quality/licensing.md` (libm) to the reuse row and a History row for M6; or record explicitly that no change is required. |
| R1-6 | Comment | Determinism / real-time safety (N1/N4, ECSS-E-ST-40C) | Positive: the determinism boundary is sound and documented. No framework-core source includes `<math.h>` or uses `double`/`float`, so the core's integer/deterministic guarantee is intact; the facade is allocation-free and stateless. The policy explicitly forbids use in hard real-time critical paths without WCET/stack analysis and states the FPU context-switch responsibility on RISC-V. Cross-references resolve: WCET/stack -> GAP-A4 ("Code assurance and runtime evidence", `docs/quality/class-a-gaps.md` line 53) and FPU/BSP -> GAP-A5 ("On-target qualification"). | Grep of `src/core|pus|transport|hal` for `math.h`/`double`/`float` -> empty; `src/math/ff_math.{c,h}`; `docs/architecture/numerics.md` §4; `docs/quality/class-a-gaps.md` lines 48-64. | None required. |
| R2-1 | Minor | `tests/unit/test_math.c`, `docs/vv/tool-qualification.md` T3 (N10/N11) | The new test script was **not re-qualified in the register**. M6 adds `tests/unit/test_math.c` as V&V evidence for SRS-REQ-033, but `tool-qualification.md` stays at Rev 0.1.5 with no M6 history entry, even though the M3 reviewer raised exactly this class of finding (M3 NC-3) and M3 closed it by adding `test_hal.c` to the register. The register's T3 location is a wildcard (`tests/unit/*.c`), which partially mitigates, but the project's own practice is to record each added test explicitly. | `grep -n "test_math" docs/vv/tool-qualification.md` -> no match; History ends at 0.1.5 ("M3 gains `tests/unit/test_hal.c`"). `tests/CMakeLists.txt` adds `test_math` and the `math` CTest. | Add an explicit T3 re-qualification entry/history row for `test_math.c` (Rev 0.1.6) recording the review of the new test, per N10 step 5. |
| R2-2 | Minor | SRS-REQ-033 / SSS-REQ-018 (requirement quality, ECSS-E-ST-40C) | The requirement bundles several independently verifiable properties (expose the facade, link `libm`, document out-of-domain behaviour), states "portable"/"usable by applications" without a measurable criterion, and gives **no quantified accuracy**. The unit test's `1e-12`/`1e-9` tolerances are therefore not traced to any stated requirement, so a regression in accuracy would not be caught as a requirement violation. | `docs/requirements/SRS.md` SRS-REQ-033 (three clauses, `method: Tst | Inp`); `docs/requirements/SSS.md` SSS-REQ-018; `tests/unit/test_math.c` lines 25-42 (`eps` values). | State an accuracy expectation (e.g. "within a bounded ULP/tolerance of the platform libm") or explicitly delegate accuracy to the platform and cite the chosen tolerance basis; consider splitting the facade-vs-documentation clauses. |
| R2-3 | Minor | `tests/unit/test_math.c` (test coverage, N6) | Coverage of the declared interface is incomplete: `FF_MathTan` has **no test at all**, and the trigonometric functions are only exercised at `0` (`sin(0)`, `cos(0)`), with `pow` only at an integer exponent. The V&V record advertises "elementary functions within tolerance, helper behaviour, domain/finiteness" for the 18 checks, which overstates the actual function coverage. | `tests/unit/test_math.c` lines 23-33 (no `FF_MathTan`, sin/cos only at 0) and lines 35-48; `docs/vv/verification-results.md` M6 notes. | Add checks for `FF_MathTan` (and a non-trivial argument) so every declared elementary function has at least one tolerance-based test. |
| R2-4 | Comment | `tools/scripts/trace.py` vs N10 (pre-existing) | N10 states that "the gate `make trace` checks that every script referenced as evidence exists **and is listed in the qualification register**". The gate in fact only checks path existence (and `ci:` claims against workflows); it never opens `tool-qualification.md`. The register is therefore maintained manually, which is how the R2-1 omission passed the gate. Pre-existing, not introduced by M6, but it weakens the N10 assurance claimed in `docs/rules.md`. | `tools/scripts/trace.py` lines 285-294 (existence + CI-claim checks only); `grep -c tool-qualification tools/scripts/trace.py` -> 0; `docs/rules.md` lines 143-144. | Either implement the register check in `trace.py` or correct the N10 wording so it matches the implemented gate. |
| R2-5 | Comment | Reproducibility of recorded M6 evidence (N11) | Positive: the key M6 claims reproduce verbatim on an independent host. Trace PASS (65/34/18/13, 45 theorems, 9 CI claims); 7/7 CTest with `math checks=18`; coverage `932/1575 (59.2%)`, functions `141/184`; `FF_USE_LIBM=OFF` degrades cleanly to 6/6; `ff_math.h` `#error` guard works. | Commands and outputs reproduced during this review; `docs/vv/verification-results.md` M6 row/notes; `README.md` metrics panel. | None required. |
| R2-6 | Comment | Process (N14/N17/N11) | The M6 delta adds/modifies SSS-REQ-018, which per N14 triggers a Senior Systems Engineer (R3) review of the SSS, and every change triggers the R4 documentation review (N17). Neither report exists yet under `docs/reviews/` for M6, while `docs/status/status.json` already reports `reviews`/`documentation_review` "green" and the M6 V&V row reads PASS. Scope note: those profiles are outside this R1/R2 report. | `docs/status/status.json` lines 25-26; no `*m6*` file in `docs/reviews/`; `docs/rules.md` N14/N17. | Ensure the R3 review of SSS-REQ-018 and the R4 documentation review are executed and committed before push, and only then set the review semaphores green (N12 closed by re-review). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

No Major findings were raised by either profile.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**Profile R1 (design/code): APPROVED WITH COMMENTS.** The facade is a correct,
defensible thin wrapper: allocation-free, stateless, no floating point leaking
into the deterministic core, an explicit compile-time dependency guard, and a
documented determinism/licensing philosophy. The `lean_note` for SRS-REQ-033 is
acceptable under interpretation note 1 (third-party `libm`), with the caveat
that it is broader than necessary (R1-3). Minor findings R1-1 (SDD/user-manual
not updated) and R1-2 (`FF_MathClamp` precondition) must be corrected or
justified; R1-3..R1-5 are recommendations.

**Profile R2 (V&V): APPROVED WITH COMMENTS.** The required results reproduce
exactly (trace PASS 65 reqs; 7/7 tests; `math` 18 checks; coverage
932/1575/59.2%, functions 141/184) and the tests correctly use tolerances rather
than equality for transcendentals. Minor findings R2-1 (test script not
re-qualified in the N10 register), R2-2 (requirement has no quantified accuracy
and bundles clauses) and R2-3 (`FF_MathTan` untested) must be corrected or
justified; R2-4..R2-6 are recommendations/process notes. This grade does **not**
substitute for the R3 (N14) and R4 (N17) reviews required for this delta.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent subagent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS |
| R2 (N9)       | independent subagent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS |
| R3 / R4 (N9)  | (separate profiles, not in this report) | -          | pending for this delta |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-1 | Minor | SDD / user manual (N8) | Math module absent from detailed design and user manual | Add module row + detailed-design bullet + user-manual subsection | updated docs + R4 re-review | frick-fsw | open | required |
| R1-2 | Minor | `FF_MathClamp` (defensive) | `lo <= hi` precondition undocumented; NaN passes through | Document/guard precondition; add tests | updated `ff_math.{c,h}` + test | frick-fsw | open | required |
| R1-3 | Comment | SRS-REQ-033 `lean_note` (N1/N3) | Note overbroad; pure helpers are Lean-modelable | Narrow note or add small Lean model | trace/manifest delta | frick-fsw | open | optional |
| R1-4 | Comment | licensing / CMake (N8/N13) | Imprecise licence wording; hardcoded `m` | Tighten wording | updated `licensing.md` | frick-fsw | open | optional |
| R1-5 | Comment | compliance matrix (N8/N16) | Reuse row / History not updated for libm | Add ADR-0012/libm reference + History row | updated matrix | frick-fsw | open | optional |
| R1-6 | Comment | Determinism (N1/N4) | Positive: core free of floating point | none | reproduced greps | frick-fsw | closed | n/a |
| R2-1 | Minor | `test_math.c` / T3 (N10/N11) | New test script not re-qualified in register | Add T3 re-qualification entry (Rev 0.1.6) | updated `tool-qualification.md` | frick-fsw | open | required |
| R2-2 | Minor | SRS-REQ-033 / SSS-REQ-018 | No quantified accuracy; bundled clauses | State accuracy/tolerance basis; consider splitting | updated SRS/SSS + trace | frick-fsw | open | required |
| R2-3 | Minor | `test_math.c` (N6) | `FF_MathTan` untested; trig only at 0 | Add tolerance tests | updated test + V&V row | frick-fsw | open | required |
| R2-4 | Comment | `trace.py` vs N10 | Gate does not check the qualification register | Implement check or fix N10 wording | trace/Rules delta | frick-fsw | open | optional |
| R2-5 | Comment | Reproducibility (N11) | Positive: M6 evidence reproduces | none | reproduced commands | frick-fsw | closed | n/a |
| R2-6 | Comment | Process (N14/N17) | R3/R4 reviews pending; semaphores premature | Run and commit R3/R4 before push | `docs/reviews/*m6*` | frick-fsw | open | required |

---

Committed and pushed with the reviewed change (norm N11).
