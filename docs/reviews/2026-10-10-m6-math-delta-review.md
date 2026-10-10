# Milestone M6 (mathematics library / libm) delta re-review — profiles R1/R2/R3/R4

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0021                      |
| Reviewer session                  | independent subagent, fresh context (norm N9) |
| Reviewer profiles                 | R1 (design/code), R2 (V&V), R3 (systems, N14), R4 (documentation, N17) |
| Reference revision                | branch `main`, uncommitted M6 working tree on base commit `17d9410` |
| Items under review                | NC dispositions in `docs/reviews/2026-10-10-m6-math-r1-r2-review.md` (FRICK-REVIEW-0019: R1-1..R1-6, R2-1..R2-6) and `docs/reviews/2026-10-10-m6-math-r3-r4-review.md` (FRICK-REVIEW-0020: R3-1..R3-4, R4-1..R4-6); fix artifacts `src/math/ff_math.{c,h}`, `tests/unit/test_math.c`, `docs/design/software-design.md`, `docs/engineering/user-integration-manual.md`, `docs/vv/{tool-qualification,verification-results}.md`, `docs/requirements/{SRS,SSS,traceability}.{md,yaml}`, `docs/architecture/numerics.md`, `docs/quality/{licensing,ecss-compliance-matrix}.md`, `docs/development-plan.md`, `README.md`, `CHANGELOG.md` |
| Date                              | 2026-10-10                             |
| Scope                             | Re-apply all norms to the affected M6 scope (norm N12) and verify every recorded NC disposition against the working tree; reproduce the trace, build, ctest and document-id gates; independently re-derive the recorded V&V numbers and the normative error-signalling contract |
| Independence declaration          | I am a freshly spawned independent reviewer with no prior context. I took no part in authoring, fixing, building, executing or previously reviewing any item under review; every claim below derives from the repository contents and reproduced commands. I modified no file except this report. |
| Norms applied                     | N1–N17 (see `docs/rules.md`), emphasis N3, N4, N8, N10, N11, N12, N16, N17 |

## 1. Scope

Delta re-review of the milestone-M6 libm-integration NC dispositions of the R1/R2
(FRICK-REVIEW-0019) and R3/R4 (FRICK-REVIEW-0020) reviews against the current
uncommitted working tree. Each recorded fix was located in the tree, the full
gate chain was re-run, and the recorded V&V numbers and the new normative
statements were independently re-derived. Excluded: M4 driver implementations,
on-target/HWIL qualification (GAP-A5), the Lean gate re-execution (`lake`/`lean`
are unavailable on this host, as in both predecessor reviews and the M3/M2 delta
reviews), and unrelated M6 periphery except where it carries NC evidence.

Reproduced commands (all from `/home/nicola/fsw/frick-fsw`):

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` →
  **PASS, exit 0** (65 requirements: 34 srs / 18 sss / 13 gap; **45** Lean
  theorems; **9** CI evidence claims); `SSS-REQ-018 <- 1 srs children`.
- `cmake -S . -B build/d6 -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build
  build/d6 && ctest --test-dir build/d6 --output-on-failure` → **7/7 pass**
  (`ccsds, sb, tbl, pus, time, hal, math`), 0 failed.
- Per-suite check counts re-derived from the built binaries: ccsds 15, sb 20,
  tbl 9, pus 29, time 101, hal 31, **math 20** = **225** total. The `math`
  binary reports `checks=20 failures=0`, not the `18`/`223` recorded in the V&V
  and CHANGELOG records (finding **D-1**).
- Document-id uniqueness scan (the exact CI `docs`-job regex over
  `docs/**/*.md`) → **55 unique document ids, 0 duplicates** (56 after this
  report adds FRICK-REVIEW-0021).
- Normative-contract probe: a compiled program calling the facade
  (`FF_MathSqrt(-1)`, `FF_MathLog(0)`, `FF_MathExp(1000)`) with glibc reports
  `errno=33` (EDOM), `errno=34` (ERANGE), `errno=34` respectively — i.e. the
  platform libm **does** set `errno`, contradicting the new `numerics.md`
  statement (finding **D-2**).

## 2. References

- Project norms `docs/rules.md` (N1–N17); review template
  `docs/reviews/review-template.md`.
- Predecessor reports: `docs/reviews/2026-10-10-m6-math-r1-r2-review.md`
  (FRICK-REVIEW-0019) and `docs/reviews/2026-10-10-m6-math-r3-r4-review.md`
  (FRICK-REVIEW-0020).
- `docs/requirements/{SSS,SRS,traceability}.{md,yaml}`; `docs/vv/verification-results.md`,
  `docs/vv/tool-qualification.md`; `docs/architecture/numerics.md`;
  `docs/adr/ADR-0012-libm-math.md`; `docs/quality/{licensing,ecss-compliance-matrix}.md`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Findings (delta)

| # | Severity | Profile | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------- | ------------------ | ------- | -------- | -------------- |
| D-1 | Minor | R2 (N11/N16/N17) | `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md`, `CHANGELOG.md` | The recorded M6 check counts are **stale by two**: the R2-3 fix added the two `FF_MathTan` checks but the records still state `math 18` / `223 checks`, whereas the tree builds `math` with **20** checks (**225** total). The recorded V&V evidence is therefore not reproducible as written, and the same "18" appears in three files (and in the R2-1 re-qualification entry). | Reproduced `test_math` → `checks=20 failures=0`; sum 15+20+9+29+101+31+20 = 225. `docs/vv/verification-results.md:25` (`math 18`, `223 checks`) and `:193-194` (`math 18 checks`, `Total 223 checks`); `docs/vv/tool-qualification.md:74` (`18 checks`); `CHANGELOG.md:58` (`18 checks`). | Update the four locations to `math 20` / `225 checks` (and the tool-qualification rev 0.1.6 entry) so the M6 record reproduces. |
| D-2 | Minor | R3 (N8) | `docs/architecture/numerics.md` §4 | The new **error-signalling contract is factually wrong**. §4 states "the facade does not set `errno`"; but the facade is a thin wrapper over the platform libm, and glibc libm sets `errno` (`EDOM`/`ERANGE`) on domain/range errors. A caller reading this may wrongly assume `errno` is preserved across a facade call (or miss the documented error). The IEEE-754 `NaN`/`±inf` signalling itself is correct and the `FF_MathIsFinite()` guidance is sound, so this is a wording/contract defect, not a defeated requirement. | `docs/architecture/numerics.md:53` ("the facade does not set `errno`"); `src/math/ff_math.c:16-55`; empirical probe with the built facade: `FF_MathSqrt(-1)` → `errno=33` (EDOM), `FF_MathLog(0)` → `errno=34` (ERANGE), `FF_MathExp(1000)` → `errno=34`. | Reword to "the facade inherits the platform libm's `errno` behaviour (glibc sets `EDOM`/`ERANGE`); callers shall not rely on `errno` and shall use `FF_MathIsFinite()` for a defined fallback." Optionally state the target-newlib behaviour separately. |
| D-3 | Minor | R1 (N8) | `docs/design/software-design.md` (R1-1 residual) | The M6 fix added the math facade to the **per-module detailed design (§3)** and to the user manual, but not to the **architectural-design table (§2)** that R1-1 explicitly called out (the table still enumerates every other module, incl. the M3 HAL). The architectural-layer view of the new dependency is still missing. Substantively the module is now documented, so the residual is narrow. | `docs/design/software-design.md:35-43` (§2 table: Applications/PUS-C/Core/Transport/OS-abstraction/HAL/Board-support, no `src/math`); the §3 bullet was added at `:74-77`. | Add one `Mathematics`/`src/math/` row to SDD §2 (cf. the M3 HAL row), or record explicitly that the facade is intentionally not an architectural layer. |
| D-4 | Minor | R4 (N8/N13) | `README.md` (R4-3 residual) | The README SPDX sentence was correctly re-scoped to exclude engineering documentation (the R4-3 defect), but the **new wording is itself inaccurate**: it claims "Every authored source, configuration and build file carries the … `SPDX-License-Identifier`", yet the two CMake build files (`CMakeLists.txt`, `tests/CMakeLists.txt`) carry no SPDX header. The root gap is pre-existing (`docs/rules.md` N13 makes the same repo-wide claim), but the M6 change rewrote the sentence and reasserts it. | `README.md:324-327`; `head -12 CMakeLists.txt` / `head -12 tests/CMakeLists.txt` (no `SPDX`); `grep SPDX` on both = 0; `docs/rules.md:188-190`. | Either add the SPDX header to the CMake build files (aligning the tree with N13/README) or drop "build file" from the sentence; record the pre-existing N13 gap for a follow-up. |
| D-5 | Comment | R1 (N2) | `docs/engineering/user-integration-manual.md` | Typo in the new §12: "WCAP/WCET analysis" — `WCAP` is not an established term; the intended reference is WCET (matching the rest of the delta). Cosmetic. | `docs/engineering/user-integration-manual.md:164`. | Change "WCAP/WCET" to "WCET/stack". |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

No Major finding was raised at any profile. D-1 (stale V&V evidence), D-2
(incorrect `errno` contract), D-3 (SDD §2 omission) and D-4 (README build-file
SPDX claim) are Minor and must be corrected or explicitly justified before the
records are final (norm N12).

## 4. Conclusion (per profile)

- [ ] APPROVED — no findings or Comments only
- [x] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**Profile R1 (design/code): APPROVED WITH COMMENTS.** The facade fixes are
substantively correct: `FF_MathClamp` now documents the `lo <= hi` precondition
and the NaN passthrough (`ff_math.h:45-46`), `FF_MathTan` is implemented and
exposed, the facade is allocation-free/stateless, and no floating point leaks
into the core. Residual Minor **D-3** (SDD §2 architectural row still missing;
R1-1 only partially applied) and **D-5** (user-manual typo). The `FF_MathClamp`
recommended test for the `lo > hi`/NaN cases was not added; since the finding's
substantive remediation (document the contract) is complete, this is recorded as
a residual Comment, not a new Minor.

**Profile R2 (V&V): APPROVED WITH COMMENTS.** R2-1 is closed (T3 re-qualified as
rev 0.1.6 with an explicit `test_math.c` entry), R2-2/R3-1 are closed (an
accuracy bound is now stated in SRS-REQ-033: "within the accuracy of the
platform `libm`; unit tests assert `<= 1e-9` for the covered inputs", consistent
with the test tolerances), R2-3 is closed (`FF_MathTan` is now tested at `0` and
at `pi/4`), and the trace/build/ctest gates reproduce (65 reqs, 45 theorems, 7/7
tests). Residual Minor **D-1**: the added checks were not propagated into the
recorded counts (`18`/`223` → **20**/`225`), so the M6 V&V evidence is stale.

**Profile R3 (systems, N14): APPROVED WITH COMMENTS.** R3-1 is closed by the
same SRS-REQ-033 accuracy bound; the reentrancy/thread-safety statement added to
`numerics.md` §4 is accurate (stateless, no shared mutable state, no
allocation). Residual Minor **D-2**: the accompanying `errno` contract is
factually wrong — the facade inherits libm's `errno` (glibc `EDOM`/`ERANGE`) and
must not be presented as "does not set `errno`". R3-3 (closed function list) and
R3-4 (positive) carry no action.

**Profile R4 (documentation, N17): APPROVED WITH COMMENTS.** R4-1 is closed
(`development-plan.md` header `0.3` matches its History), R4-2 is closed
(`verification-results.md` header `0.5` matches its History), R4-3's core defect
(README claiming SPDX for engineering docs) is closed, and the document-id scan
and traceability counts are consistent (55 ids, 0 duplicates; 34/18/13/45).
Residual Minor **D-4**: the re-scoped README sentence is still not literally
true for the CMake build files. The optional Comments R4-4 (M6/M5 section
ordering), R4-5 (trailing newlines) and R4-6 (`façade` spelling in ADR-0012)
remain unaddressed and are accepted as non-derogatory.

Milestone-M6 closure is unblocked. D-1..D-4 are Minor and must be dispositioned
before the M6 records are considered final (norm N12).

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent subagent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS (D-3 Minor; D-5 Comment) |
| R2 (N9)       | independent subagent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS (D-1 Minor) |
| R3 (N9/N14)   | independent subagent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS (D-2 Minor) |
| R4 (N9/N17)   | independent subagent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS (D-4 Minor) |

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing the
deltas until full compliance.)

### 6.1 R1 delta — R1-1..R1-6

| NC | Severity | Item | Delta verdict | Evidence |
| -- | -------- | ---- | ------------- | -------- |
| R1-1 | Minor | SDD §2/§3 + user manual | **PARTIAL** (D-3) | SDD §3 bullet added `software-design.md:74-77`; user manual §12 added `user-integration-manual.md:155-164` (header 0.2, History row); SDD §2 architectural table still has no math row (`:35-43`). |
| R1-2 | Minor | `FF_MathClamp` contract | **CLOSED (residual Comment)** | `ff_math.h:45-46` documents `lo <= hi` and NaN passthrough; behaviour correct (`ff_math.c:66-75`). No test added for `lo > hi`/NaN (residual, non-blocking). |
| R1-3 | Comment | `lean_note` overbroad | **CLOSED** | `traceability.yaml` SRS-REQ-033 `lean_note` narrowed: libm-backed functions "third-party (not Lean-checkable)"; pure helpers explicitly covered "by the same tests and inspection". |
| R1-4 | Comment | licensing / CMake | **CLOSED** | `licensing.md:43` now "LGPL-2.1-or-later (glibc) / BSD-style (newlib)"; header 0.3 + History. (CMake `m` linkage left as-is, acceptable.) |
| R1-5 | Comment | compliance matrix reuse row | **CLOSED** | `ecss-compliance-matrix.md:39` reuse row now `ADR-0002/0003/0012`; header 0.2 + History row added. |
| R1-6 | Comment | determinism (positive) | **CLOSED** | Core still free of `<math.h>`/`double`/`float`; facade allocation-free. |

### 6.2 R2 delta — R2-1..R2-6

| NC | Severity | Item | Delta verdict | Evidence |
| -- | -------- | ---- | ------------- | -------- |
| R2-1 | Minor | T3 re-qualification | **CLOSED (with D-1)** | `tool-qualification.md:7` rev 0.1.6, History `:74` explicitly records `tests/unit/test_math.c`; the "18 checks" figure is D-1. |
| R2-2 | Minor | SRS-REQ-033 accuracy | **CLOSED** | `SRS.md:148-154` states "within the accuracy of the platform `libm`; unit tests assert within a documented tolerance (`<= 1e-9`)"; header 0.5 + History. Tolerances in `test_math.c` are all `<= 1e-9`. |
| R2-3 | Minor | `FF_MathTan` untested | **CLOSED (with D-1)** | `test_math.c:28-29` adds `FF_MathTan(0)` and `FF_MathTan(pi/4)`; every declared elementary function now has a tolerance test. Count not propagated (D-1). |
| R2-4 | Comment | `trace.py` vs N10 register | **OPEN (accepted, not required)** | `trace.py` unchanged; still existence/CI-claim checks only. Pre-existing; no M6 action. |
| R2-5 | Comment | reproducibility (positive) | **CLOSED** | Trace PASS (65/34/18/13, 45, 9) and 7/7 ctest reproduced; counts aside (D-1). |
| R2-6 | Comment | R3/R4 reviews pending | **CLOSED** | FRICK-REVIEW-0019/0020 now exist; this delta review (FRICK-REVIEW-0021) closes the loop for M6. `status.json` semaphores unchanged. |

### 6.3 R3 delta — R3-1..R3-4

| NC | Severity | Item | Delta verdict | Evidence |
| -- | -------- | ---- | ------------- | -------- |
| R3-1 | Minor | accuracy requirement | **CLOSED** | SRS-REQ-033 accuracy bound (`SRS.md:148-154`); SSS-REQ-018 0.5 + History. |
| R3-2 | Minor | reentrancy/errno contract | **NOT CLOSED** (D-2) | `numerics.md:50-52` reentrancy/thread-safety correct; `:53-56` errno clause factually wrong (glibc sets `EDOM`/`ERANGE`; probe confirms). |
| R3-3 | Comment | closed function list | **OPEN (accepted, not required)** | No `"at least"`/extension clause added; list still closed by design. |
| R3-4 | Comment | determinism boundary (positive) | **CLOSED** | `numerics.md:19-26,38-49`; `SSS-REQ-018`/`SSS-REQ-010` boundary intact. |

### 6.4 R4 delta — R4-1..R4-6

| NC | Severity | Item | Delta verdict | Evidence |
| -- | -------- | ---- | ------------- | -------- |
| R4-1 | Minor | `development-plan.md` header | **CLOSED** | Header `:7` = `0.3`; History `:190-193` rows 0.1/0.2/0.3; OBJ-13 + M6 section + matrix row added. |
| R4-2 | Minor | `verification-results.md` header | **CLOSED (with D-1)** | Header `:7` = `0.5`; History `:242-246` rows 0.1..0.5; M6 row/notes added; counts stale (D-1). |
| R4-3 | Minor | README SPDX scoping | **CLOSED (residual D-4)** | `README.md:324-327` now scopes to "source, configuration and build file" and excludes engineering docs; but build files lack the header (D-4). |
| R4-4 | Comment | M6/M5 ordering | **OPEN (accepted, not required)** | `development-plan.md:144-155` (M6 before M5); `roadmap.md` likewise. |
| R4-5 | Comment | trailing newlines | **OPEN (accepted, not required)** | Still no trailing newline in `licensing.md`, `traceability.md`, `traceability.yaml`, `tooling.md`, `verification-results.md`. |
| R4-6 | Comment | `façade` spelling | **OPEN (accepted, not required)** | `ADR-0012-libm-math.md:28` still "façade" vs "facade" elsewhere. |

### 6.5 New delta non-conformances

| NC | Severity | Profile | Summary | Status |
| -- | -------- | ------- | ------- | ------ |
| D-1 | Minor | R2 | M6 check counts stale (`math 18`/`223` vs actual `20`/`225`) in `verification-results.md`, `tool-qualification.md`, `CHANGELOG.md` | open |
| D-2 | Minor | R3 | `numerics.md` §4 `errno` contract false for the platform libm | open |
| D-3 | Minor | R1 | SDD §2 architectural table lacks the math-module row (R1-1 partial) | open |
| D-4 | Minor | R4 | README SPDX sentence reasserts a claim not met by the CMake build files | open |
| D-5 | Comment | R1 | user-manual `WCAP` typo | open |

### 6.6 Conformance verified (no finding)

- **Traceability gate:** `trace.py` exit 0 — 65 requirements (34 srs / 18 sss /
  13 gap), 45 Lean theorems, 9 CI claims; `SSS-REQ-018 <- 1 srs children`.
- **Counts coherent** across `README.md` (34/18/13/45), `traceability.md:18-20`,
  `verification-results.md:25` and the manifest.
- **SSS-REQ-018 → SRS-REQ-033** decomposition present in the manifest, the
  bottom-up matrix (`traceability.md:43`) and §2/§3 tables.
- **Document ids:** 55 unique (CI regex), 0 duplicates; `numerics.md`
  (FRICK-ARCH-0009) linked from `README.md:275` and required by both CI
  workflows (`.github/workflows/ci.yml:138`, `.gitea/workflows/ci.yml:179`).
- **Build/test:** Debug build clean; 7/7 ctest pass; `test_math` 20/20.
- **Header vs History** coherent for the new/edited docs (`numerics.md` 0.1,
  `ADR-0012` has no revision table by design, `SRS` 0.5, `SSS` 0.5,
  `development-plan` 0.3, `verification-results` 0.5, `tool-qualification`
  0.1.6, `licensing` 0.3, `compliance-matrix` 0.2, `traceability` 0.4,
  `user-integration-manual` 0.2).

---

> **Closure update.** D-1 (math checks 20 / 225 propagated to
> `verification-results.md`, `tool-qualification.md`, `CHANGELOG.md`), D-2
> (`numerics.md` errno contract corrected: `libm` may set `EDOM`/`ERANGE`; the
> facade does not clear it; use `FF_MathIsFinite()`), D-3 (SDD §2 architectural
> table now has the Mathematics row) and D-4 (SPDX headers added to
> `CMakeLists.txt` and `tests/CMakeLists.txt`) were corrected; D-5
> ("WCAP/WCET" -> "WCET") fixed. All M6 NCs closed (norm N12).

Committed and pushed with the reviewed change (norm N11).
