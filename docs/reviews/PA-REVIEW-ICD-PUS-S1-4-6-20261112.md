# PA Review Report — ICD PUS Service 1 Subservice 4/6 Annotation (NC-EXT-26)

**Norm:** N20 (independent Senior SW Product Assurance review)
**Reviewer:** Senior Aeronautics and Space Software Product Assurance Engineer (AI assurance assistant — not an approval authority)
**Repository:** /home/nicola/fsw/frick-fsw
**Branch / HEAD:** main / b6bf060
**Review date:** 2026-11-12
**Change under review:** One documentation/interface annotation in `docs/interfaces/ICD.md:106`

---

## 1. Change under review

`docs/interfaces/ICD.md` line 106, ICD §4 "Implemented PUS services and subservices (open backend)":

```
- | 1  | 4 / 6 | start / progress failure | TM | errcode |
+ | 1  | 4 / 6 | start / progress failure (reserved, not emitted) | TM | errcode |
```

`git status` confirms **only** `docs/interfaces/ICD.md` is modified. No `src/` change, no behaviour change, no traceability (`traceability.yaml`) change, no CMake/toolchain change.

Registered NC: **NC-EXT-26** (`docs/planning/sprint-plan.md:163`) — "Major (functional) NC — PUS S1 subservice mapping discrepancy … TM[1,4] and TM[1,6] are never emitted." The NC remains **OPEN**; this review does not close it.

---

## 2. Verification results (task items 1–5)

| # | Task item | Result | Evidence |
|---|-----------|--------|----------|
| 1 | ICD annotation accurately reflects implementation | **CONFIRMED** | `FF_PusSendVerification()` (`src/pus/pus.c:298-354`) emits subservice 4 only for `FF_PUS_STAGE_START` with `success=0` (line 314) and subservice 6 only for `FF_PUS_STAGE_PROGRESS` with `success=0` (line 317). Both current call sites pass `success=1` (START line 451, PROGRESS line 456), so TM[1,4] and TM[1,6] are never emitted by the current build. ICD §4 row now annotates "(reserved, not emitted)". Accurate. |
| 2 | No safety/security/flight/hardware impact | **CONFIRMED** | Documentation/interface annotation only. No source, binary, build, traceability, safety, security, or hardware artifact touched. No target/HIL relevance (GAP-A5 untouched). |
| 3 | NC-PAL-01, GAP-A5, NC-EXT-19/90, N22-TS5-47 not touched or closed | **CONFIRMED** | All four remain registered OPEN in `docs/planning/sprint-plan.md` (NC-PAL-01 ~line 240; GAP-A5 throughout class-a-gaps.md / traceability.yaml:973; NC-EXT-19 ~line 154 and NC-EXT-90 ~line 227; N22-TS5-47 within NC-EXT-236 ~line 323 and NC-EXT-237 ~line 324). None is referenced by the change; none is dispositioned or closed here. **No authority-owned NC is closed by this review.** |
| 4 | Host gates green | **PARTIALLY DIRECTLY CONFIRMED; remainder verified-by-inspection** | `make trace`: **PASS (120/76/30/14/68)** — directly re-run. `make format-check`: exit 0 with `FF_CLANG_FORMAT_ALLOW_UNPINNED=1` (host clang-format is 23 vs pinned 18 — environment pin caveat, not a formatting defect; format-check scans C files only, so the doc change cannot affect it). `python3 tools/scripts/license_check.py`: **OK**. Build/CTest/Lean: not re-runnable in-place because pre-existing `build/native` artifacts are owned by another user (`rm -rf` permission denied — environment limitation unrelated to this change); verified-by-inspection as unaffected because the change touches no C source, CMake, test, or Lean file. User-reported gate figures (build 0 warnings, CTest 56/56, Lean lake build) are consistent with a doc-only change. |
| 5 | PA findings that block commit/push | **NONE** | No Major finding. One Minor clarification (PA-001), non-blocking. |

---

## 3. Findings

### PA-001 — Minor (interface wording clarification) — non-blocking

**Criterion:** ICD §4 shall accurately and unambiguously describe wire-observable behaviour for the current build (ICD §4 intro; ECSS-E-ST-40C interface documentation intent; ECSS-Q-ST-80C documentation-control intent).

**Observed fact:** `docs/interfaces/ICD.md:106` reads `| 1 | 4 / 6 | start / progress failure (reserved, not emitted) | TM | errcode |`. The implementation in `src/pus/pus.c:314,317` *can* emit subservice 4 (`START`, `success=0`) and subservice 6 (`PROGRESS`, `success=0`) via `FF_PusSendVerification()`; no current call site passes `success=0` for those stages (START line 451, PROGRESS line 456), so TM[1,4]/TM[1,6] are not emitted *by the current build*.

**Gap / risk:** The phrase "(reserved, not emitted)" can be misread as a permanent wire prohibition ("subservice 4/6 are reserved and will never be emitted"), whereas the codes are implemented and available if a future caller passes `success=0`. This is an interface-precision issue, not a defect in the current annotation (which is accurate for the current build).

**Recommended disposition:** Reword the ICD cell to scope the claim to the current build, e.g. `start / progress failure (implemented; not emitted by current call sites — success=1 only)` or add an ICD note stating TM[1,4]/TM[1,6] are available in `FF_PusSendVerification()` but unused by current call sites. Do **not** hard-code "reserved" in a way that contradicts the implementation capability.

**File:line:** `docs/interfaces/ICD.md:106`; related implementation `src/pus/pus.c:314,317,451,456`.

**Blocks commit/push:** No. Recommended before commit as interface hygiene; may be dispositioned in the same or a following documentation cycle at the documentation authority's discretion.

---

### (No Major findings)

No safety, security, flight, hardware, traceability, build, or compliance finding. No authority-owned NC (NC-PAL-01, GAP-A5, NC-EXT-19/90, N22-TS5-47, NC-EXT-26) is closed, weakened, or dispositioned by this review.

---

## 4. Requirements traceability note (accuracy of record)

In this repository **SRS-REQ-073/074/075 are watchdog/FDIR requirements** (watchdog arm/feed, pre-reset event report, single reset path; `traceability.yaml:681-699`), not PUS service-1 verification requirements. The PUS service-1 verification subservice mapping is implemented in `src/pus/pus.c:298-354` (`FF_PusSendVerification`) and documented in ICD §4. This review verified the PUS mapping directly against the implementation and call sites; SRS-REQ-073/074/075 are unaffected by the ICD change. The requirement references in the review request are therefore noted for the record but are not traced to this ICD row.

---

## 5. Gate evidence summary

| Gate | Requirement | Result | How confirmed |
|------|-------------|--------|---------------|
| Traceability | `make trace` PASS | PASS (120/76/30/14/68) | Directly re-run 2026-11-12 |
| Format | `make format-check` exit 0 | exit 0 | Directly re-run (unpinned override; host clang-format 23 vs pinned 18 is an environment pin caveat) |
| License | `license_check.py` OK | OK | Directly re-run 2026-11-12 |
| Build | exit 0, 0 warnings | Verified-by-inspection | Doc-only change; no source/CMake touched; clean rebuild blocked by pre-existing root-owned `build/native` artifacts (environment limitation, unrelated to this change) |
| CTest | exit 0, 56/56 | Verified-by-inspection | Doc-only change; no test touched |
| Lean | `lake build` | Verified-by-inspection | Doc-only change; no Lean file touched |

---

## 6. Uncertainty and confidence

```text
Decision/product and configuration: frick-fsw, main @ b6bf060, docs/interfaces/ICD.md:106 PUS S1(4/6) annotation (NC-EXT-26)
Claim type: Fact (implementation mapping verified); Inference (gate figures for build/CTest/Lean by inspection)
Applicable baseline and approved tailoring: ECSS-E-ST-40C / ECSS-Q-ST-80C interface documentation intent; project norms N1-N22; NC-EXT-26 registered open
Evidence consulted: docs/interfaces/ICD.md:106; src/pus/pus.c:298-354 (esp. 314,317,451,456); docs/planning/sprint-plan.md:163 (NC-EXT-26), ~240 (NC-PAL-01), ~154/227 (NC-EXT-19/90), NC-EXT-236/237 (N22-TS5-47); docs/requirements/traceability.yaml:681-699 (SRS-REQ-073/074/075); trace PASS log; format-check exit 0; license_check.py OK
Cross-check and reconciliation: ICD annotation matches implementation capability + current call-site behaviour; no trace row for S1(4/6) emission; NCs untouched
Confidence: High for items 1-3 and trace/format/license; Medium for build/CTest/Lean (verified-by-inspection as doc-only; in-place rebuild blocked by environment permissions)
Missing/inaccessible evidence: none for this change; build/CTest/Lean re-execution blocked by root-owned build/native artifacts (environment, not product)
Assumptions: the user-reported build 0 warnings / CTest 56/56 / Lean lake build figures apply to the pre-change tree and are unchanged by a doc-only edit
Expiry/recheck date: re-verify if any src/, traceability.yaml, CMake, or Lean file changes; re-review PA-001 wording at next documentation cycle
Consequences if wrong: mis-scoped ICD wording could lead a downstream reader to treat TM[1,4]/TM[1,6] as permanently unavailable; minimal safety/security impact because no behaviour changes
Required independent human reviewer/authority: documentation authority / interface authority for PA-001 disposition; release authority for commit/push (N7); this PA review recommends, does not approve
```

---

## 7. PA conclusion and recommendation

The ICD correction is **accurate, in scope, and doc-only**. It correctly records that TM[1,4] and TM[1,6] are not emitted by the current build because `FF_PusSendVerification()` emits subservice 4/6 only on `success=0` for `START`/`PROGRESS`, and both call sites pass `success=1`. There is no safety, security, flight, or hardware impact, and no traceability requirement is affected.

**Recommendation:** Proceed with commit/push subject to the separately applicable gates (N7) and the documentation authority's disposition of PA-001. PA-001 is **Minor and non-blocking**. No authority-owned NC is closed by this review; NC-EXT-26, NC-PAL-01, GAP-A5, NC-EXT-19/90, and N22-TS5-47 remain OPEN and dispositioned only by their authorized owners.

*This PA review assesses sufficiency, consistency, provenance, and closure of the submitted evidence. It does not certify compliance, accept residual risk, or authorize release. Independent qualified human authority retains those decisions.*
