# V&V Review Report — VV-R2-20261112-PUS-S20-PARAM-SET-BOUNDED

**Independent review under norm N10 / R2 profile — Senior Aeronautics & Space SW V&V Engineer.**
This persona has no approval, qualification, certification, or release authority. Safety-critical conclusions require qualified independent human review and the approvals defined by the governing baseline.

## Meta

| Field | Value |
|---|---|
| Repository | /home/nicola/fsw/frick-fsw |
| Branch / HEAD | main / 20068b0 (uncommitted working-tree change only) |
| Change scope | `tests/unit/test_pus.c` (+`test_param_set_bounded`, 19 new checks); `tests/CMakeLists.txt` (SPDX header formatting only, no functional change) |
| Nature | Test-only addition; host-executable unit test; NOT target/hardware; NOT GAP-A5/NC-PAL-01/OBCP engine; NC-EXT-19/90, N22-TS5-47 untouched |
| Review date | 2026-11-12 |
| Reviewer | Independent V&V (N10/R2 profile); no commit/push or approval authority |
| Gate results (per change author, independently verified where possible) | build exit 0 (0 warnings); CTest exit 0, 0 failures; Lean lake build; trace PASS 120/76/30/14/68; format-check PASS; license OK |
| Independent verification performed | `test_pus.c` recompiled with `-Wall -Wextra -Werror` (0 warnings); executed: 236 checks, 0 failures (ASN1-off configuration; ASN1-on excludes test_ping_roundtrip cross-backend block); `test_param_set_bounded` oracles cross-checked against `src/pus/pus_services.c:630-642` |

## 1. Change under review

`tests/unit/test_pus.c` adds `test_param_set_bounded` (lines 933-1012), registered in `FF_TEST_MAIN` at line 1017. It verifies PUS service 20 subservice 1 (TC[20,1] set parameter) end-to-end:

- **Case 1 — nominal set** (lines 952-972): register setter for parameter id 42 via `FF_PusRegisterParamSetter`, encode TC[20,1] with payload `{42, 0xAA, 0xBB}`, ingest, assert `FF_PusIngest == FF_SUCCESS`, setter invoked with `id==42`, `len==2`, `data[0]==0xAA`, `data[1]==0xBB`.
- **Case 2 — unknown parameter ID** (lines 974-992): payload `{99, 0x00}`, no setter registered for id 99, assert `FF_PusIngest == FF_NOT_FOUND`, setter not called.
- **Case 3 — bad size** (lines 994-1011): empty payload (`NULL`, len 0), assert `FF_PusIngest == FF_BAD_SIZE`, setter not called.

`tests/CMakeLists.txt` has an SPDX header formatting correction only (line 4 already correct; no functional change, no new test target, no CTest registration change).

No production source files (`src/`) modified. Existing `svc20_set` behavior (`src/pus/pus_services.c:630-642`) is verified, not changed.

## 2. Evidence reviewed

| Artifact | Relevance |
|---|---|
| `tests/unit/test_pus.c` | Change under review: `test_param_set_bounded` (lines 933-1012), registration (line 1017), recording globals (lines 26-29), test double setter (lines 31-42) |
| `src/pus/pus_services.c:630-642` | `svc20_set` production implementation — oracle ground truth |
| `src/pus/pus.h:46` | `FF_Pus_ParamSetter_t` type |
| `src/pus/pus_services.c:141-162` | `FF_PusRegisterParamSetter` registration API |
| `tests/ff_test.h` | `FF_TEST_ASSERT` harness (check/failure counting, no abort-on-fail, cumulative exit code) |
| `tests/CMakeLists.txt:16,229` | `test_pus` target and CTest registration `pus` |
| `docs/vv/tool-qualification.md` row T2/T3 | T3 qualification status for `tests/unit/*.c` including `test_pus.c` |
| `docs/requirements/traceability.yaml` | Requirement-to-test traceability manifest; SRS-REQ-049 (PUS-C service set incl. S20, planned M15) tests do not include `test_pus.c` |
| `docs/rules.md` | Norms N4, N9, N10, N11, N12, N15, N21, N22 |

## 3. Findings

### VV-R2-001 — T3 qualification invalidated by test_pus.c change (Minor — does NOT block commit/push)

**Finding.** `tests/unit/test_pus.c` is a recorded V&V script under tool-qualification register row T3 (`tests/unit/*.c`, SRS/SSS test evidence). The register states (norm N10): "Any change to a recorded V&V script invalidates its qualification and forces a re-qualification review before the evidence produced with it can be reused." `test_pus.c` has changed: a new test function (`test_param_set_bounded`, 19 new checks) was added. The current file sha256 is `cb9e7bb2c07f06baa5126aa14016d6b59b26d5907907fb39b7d35dc456fe3643`, which matches neither the previously reviewed revision `6f68de14…` (FRICK-REVIEW-0443) nor the earlier candidate `4ca0e2d0…` recorded in `docs/vv/tool-qualification.md` rev 1.3-1.5. Therefore T3 qualification for `test_pus.c` is invalidated for the current tree.

The change context's assertion of "no T3 impact, no N10 re-qualification trigger" is **not supported** by `docs/rules.md` N10 or by `docs/vv/tool-qualification.md`. Adding a test function to a qualified test script is a change to that script.

**Recommendation.** Update `docs/vv/tool-qualification.md` row T3 with the current `test_pus.c` sha256 and obtain a delta N10 review (source review + inspection + configuration management, per criticality B minimum), or record a bounded deviation with PA authority approval. This finding does not block commit/push by itself, but the test evidence produced by `test_pus.c` cannot be claimed as N10-qualified until the register/delta is resolved.

### VV-R2-002 — Traceability gap: test_param_set_bounded not linked to a controlled requirement (Minor — does NOT block commit/push)

**Finding.** Unlike other tests in `test_pus.c` (e.g. `test_memory_dump_bounded` → SRS-REQ-009, `test_service9_time` → SRS-REQ-010, `test_hk_periodic_emission` → NC-EXT-17), `test_param_set_bounded` contains no requirement/hazard/NC reference in its comment block and is not added to any `tests:` list in `docs/requirements/traceability.yaml`. Service 20 is within the scope of SRS-REQ-049 ("Fundamental PUS-C service set … S1/3/5/6/8/9/11/12/17/18-surface/19/20 … milestone M15, planned"), but SRS-REQ-049's `tests:` list is `["ci:docs", "tests/unit/test_r3_gap.c"]` and does not include `test_pus.c`. Per norms N4 (bidirectional traceability) and N22 (requirement-based test linked to controlled requirements), a test without a requirement link is unverified-as-requirements-evidence.

**Assessment.** The test is valid as a regression/characterization test for existing `svc20_set` behavior (no production change), but it does not currently satisfy N4 traceability as requirement-based evidence for a controlled requirement.

**Recommendation.** Either (a) add `tests/unit/test_pus.c` to SRS-REQ-049's `tests:` list with an explicit `lean_note` justification, or (b) annotate `test_param_set_bounded` as regression/characterization evidence (not requirement-based) with a traceability manifest entry marked `derived: true` + `justification`. Do not claim it as verified evidence for a controlled SRS-REQ until linked.

### VV-R2-003 — Test oracles correct (Comment — no action required)

**Finding.** All oracles in `test_param_set_bounded` were independently verified against `svc20_set` (`src/pus/pus_services.c:630-642`):

| Test case | Expected | Production behavior (`svc20_set`) | Match |
|---|---|---|---|
| Nominal (id 42, 2 bytes) | `FF_SUCCESS`, setter called with `id=42`, `len=2`, `data={0xAA,0xBB}` | `len>=1`, setter found → `fn(d[0], &d[1], len-1)` = `fn(42, {0xAA,0xBB}, 2)`, returns setter's `FF_SUCCESS` | Yes |
| Unknown id (99) | `FF_NOT_FOUND`, setter not called | No setter with id 99 → `FF_NOT_FOUND`, setter never invoked | Yes |
| Empty payload (len 0) | `FF_BAD_SIZE`, setter not called | `len < 1` → `FF_BAD_SIZE` before any setter lookup | Yes |

The test double `g_param_test_setter` (lines 31-42) correctly records `id`, `len`, and copies `data`. Its `memcpy` guard (`len != 0 && data != NULL`) is safe because `svc20_set` only invokes it with `len >= 1` and a valid `&d[1]` pointer.

**Recommendation.** None. Oracles are exact, independent of the implementation algorithm, and match production behavior.

### VV-R2-004 — Determinism confirmed (Comment — no action required)

**Finding.** `test_param_set_bounded` is single-threaded, host-executable, with no concurrency, no randomization, no time dependency (`svc20_set` does not call time services), and no external I/O. The static recording globals (`g_param_set_id`, `g_param_set_len`, `g_param_set_called`, `g_param_set_data`) are reset at the start of the test (lines 943-946) and between error cases (lines 975, 995). Each `FF_PusIngest` call is followed by immediate state assertions with no reliance on scheduling, wall-clock, or thread interleaving.

**Assessment.** Deterministic under the host/unit conditions. Repeated execution produces identical results.

### VV-R2-005 — No regression in existing test behavior (Comment — no action required)

**Finding.** The change adds a new test function and registers it in `FF_TEST_MAIN`; it does not modify any existing test function, production source, or CTest target. Independent recompilation and execution produced 0 failures across all 9 test functions (236 checks in the ASN1-off verification build; 244 checks claimed in the ASN1-on CI configuration). CTest `pus` target is unchanged (`tests/CMakeLists.txt:229`). No existing test behavior regressed.

### VV-R2-006 — Coverage sufficiency adequate for svc20_set's implemented behavior (Comment — no action required)

**Finding.** Per norm N22, requirement-based/integration tests shall exercise nominal and applicable out-of-bound operation with explicit expected results. `test_param_set_bounded` provides:

- **Nominal:** valid TC[20,1] with registered setter, correct parameter ID and data asserted.
- **Out-of-bound:** unknown parameter ID (rejection path, `FF_NOT_FOUND`), empty payload (rejection path, `FF_BAD_SIZE`).

The boundary value `len == 1` (parameter ID only, zero data bytes, setter called with `len==0`) is not explicitly tested. However, `svc20_set` handles `len==1` by calling `fn(d[0], &d[1], 0)`, and the test double's zero-length path is benign (no `memcpy`, sets `g_param_set_called=true`, returns `FF_SUCCESS`). This is a minor coverage gap relative to full boundary analysis but does not invalidate the nominal/out-of-bound coverage already present.

**Assessment.** Coverage is sufficient for the claimed verification of `svc20_set` (nominal + applicable out-of-bound with explicit expected results). Not a finding requiring correction.

## 4. Behavior / regression summary

| Question | Answer |
|---|---|
| Does `svc20_set` still return FF_SUCCESS / FF_NOT_FOUND / FF_BAD_SIZE as implemented? | Yes; verified against source, no production change |
| Nominal set invokes setter with correct id/len/data? | Yes (lines 967-971) |
| Unknown parameter ID → FF_NOT_FOUND, setter not called? | Yes (lines 990-991) |
| Empty payload → FF_BAD_SIZE, setter not called? | Yes (lines 1009-1010) |
| Deterministic, single-threaded? | Yes |
| Any existing test regressed? | No; CTest exit 0, 0 failures |
| Any new test scripts/tools added? | No; no T3 tool change, but test_pus.c script itself changed (see VV-R2-001) |
| N10 re-qualification triggered? | Yes; test_pus.c is a T3 script and has changed (see VV-R2-001) |

## 5. Conclusion and commit/push recommendation

**APPROVED WITH COMMENTS — Minor findings tracked to closure; no finding blocks commit/push.**

**No Major findings.** All applicable gates pass (build with 0 warnings, CTest exit 0, Lean `lake build`, trace PASS 120/76/30/14/68, format-check PASS, license OK). The test oracles are correct, the test is deterministic, and no existing behavior regressed.

**Conditions before proceeding:**

1. The change is committed/pushed **only** by the designated `aeronautics-space-fsw-engineer` after all gates/norms pass and the full sprint scope is complete (norm N7/N15); this V&V review does not waive any gate or approve risk/release.
2. Review evidence retained in the working tree immediately (this file; norm N11); revision history updated (norm N16).
3. VV-R2-001 (T3 qualification): update `docs/vv/tool-qualification.md` row T3 with the current `test_pus.c` sha256 (`cb9e7bb2…`) and obtain a delta N10 review, or record a PA-approved bounded deviation, before `test_pus.c` evidence is claimed as N10-qualified.
4. VV-R2-002 (traceability): add `tests/unit/test_pus.c` to SRS-REQ-049's `tests:` list (or annotate as regression/characterization evidence with `derived: true` + `justification`) in `docs/requirements/traceability.yaml`, and re-run `make trace` (norm N4).
5. Norms N9 (independent R2), N17 (documentation R4), N20 (PA) reviews remain applicable per their own triggers and are not substituted by this V&V review.

**V&V statement:** Objective evidence supports that `test_param_set_bounded` correctly verifies the existing `svc20_set` behavior (TC[20,1] set parameter) under the recorded host/unit conditions: nominal set with correct setter invocation, and both out-of-bound rejection paths with exact return codes. It does not establish N10 tool qualification for the modified `test_pus.c` (VV-R2-001) nor N4 traceability to a controlled requirement (VV-R2-002); both are Minor and tracked to closure.

---

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
|--|----------|-------------------|---------|-------------|-------------------------|-------|----------------------|-----------------|
| VV-R2-001 | Minor | `docs/vv/tool-qualification.md` T3 / `tests/unit/test_pus.c` | T3 qualification invalidated by test_pus.c change; sha256 `cb9e7bb2…` not in register | Update register sha256 + delta N10 review, or PA-approved bounded deviation | tool-qualification.md row T3; FRICK-REVIEW-0443 delta pattern | FSW engineer / PA | open | pending |
| VV-R2-002 | Minor | `docs/requirements/traceability.yaml` SRS-REQ-049 / N4 | test_param_set_bounded not linked to controlled requirement | Add to SRS-REQ-049 tests list or annotate `derived: true` + justification; re-run `make trace` | traceability.yaml; trace PASS | FSW engineer | open | pending |

---
*End of report. Classification key: Major = significant evidence gap or defect that may block commit; Minor = issue/observation that does not block commit but requires tracking or a documented basis; Comment = observation, no derogation. Committed and pushed with the reviewed change (norm N11).*
