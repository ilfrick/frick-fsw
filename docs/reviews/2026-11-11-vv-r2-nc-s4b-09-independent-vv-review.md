# Independent V&V Review Report — NC-S4b-09 (run_sim.sh CMake generator fix)

**Reviewer profile:** Senior Aeronautics and Space Software V&V Engineer (norm N10 / R2)
**Repository:** /home/nicola/fsw/frick-fsw
**Branch:** main, HEAD 0cc455f
**Review date:** 2026-11-11
**NC under review:** NC-S4b-09 — "`run_sim.sh` forced Ninja against an existing Unix Makefiles cache, causing the documented smoke/check workflow to fail" (sprint-plan.md §NC-S4b-09, classified Minor)
**Fix status:** Implemented and committed. `tools/scripts/run_sim.sh` no longer passes `-G Ninja`; it reuses the generator cached in `BUILD_DIR`.

---

## 1. Scope and method

This review addresses whether the recorded V&V evidence supports closure of NC-S4b-09, a build/tooling smoke-workflow fix with **no functional software change, no new test scripts, no T3 impact, and no N10 re-qualification trigger beyond the existing T4/T9 pending item**.

Evidence basis (norms N6/N10/N11):
- Governing artifacts: `tools/scripts/run_sim.sh`, `Makefile` (smoke/check/ci-local/coverage targets), `docs/vv/tool-qualification.md` (T4/T9 register), `docs/planning/sprint-plan.md` (NC-S4b-09 row).
- Definitions: verification = objective evidence the work product satisfies its criteria; validation = fitness for intended use. NC-S4b-09 is a verification (workflow correctness) matter, not a validation matter.
- V&V does not prove defect absence; passing tests show observed outcomes matched defined oracles under recorded conditions.

Independent verification performed by this reviewer (not taken on authority):
1. `BUILD_DIR=build/host bash tools/scripts/run_sim.sh` → PASS, exit 0 (banner + "7 applications registered"). `build/host/CMakeCache.txt`: `CMAKE_GENERATOR:INTERNAL=Unix Makefiles` (the previously failing cached case).
2. Fresh directory `/tmp/opencode/fresh-smoke` → PASS, exit 0 (same assertions).
3. `ctest --test-dir build/host` → 100% passed, 0 failed out of 56, exit 0.
4. Source inspection of `run_sim.sh` and grep for other `-G Ninja` usages.

---

## 2. Independent verification results

| Check | Result | Details |
|---|---|---|
| Smoke, cached Unix Makefiles generator (`build/host`) | PASS, exit 0 | Banner + 7 apps registered; `CMAKE_GENERATOR=Unix Makefiles` |
| Smoke, fresh directory | PASS, exit 0 | Same assertions; no pre-existing cache |
| ctest, `build/host` | 56/56 PASS, exit 0 | 0 failed; test behavior unchanged |
| `run_sim.sh` forced `-G Ninja` removed | Confirmed | Line 24 invocation has no `-G`; lines 20-23 document NC-S4b-09 rationale |
| Other `-G Ninja` usages | No cache-conflict risk | All operate on fresh dedicated dirs (ci-local `build/ci`, coverage `build/coverage`, qemu/renode xw2/xw3 dirs) |

---

## 3. Findings

Each finding is classified Major/Minor with file:line references and a statement of whether it blocks commit/push and/or NC-S4b-09 closure.

### VV-R2-001 (Minor) — Fix verified: `-G Ninja` no longer forced; cache-conflict failure mode eliminated

**Evidence:** `tools/scripts/run_sim.sh:20-24`. The `cmake -S "${ROOT}" -B "${BUILD_DIR}" ...` invocation no longer supplies `-G Ninja`; the comment at lines 20-23 documents that forcing `-G Ninja` made `make smoke`/`make check` fail on any tree configured through the Makefile (NC-S4b-09).

**Independent verification:** Smoke PASS (exit 0) on both the cached Unix Makefiles generator (`build/host`) and a fresh directory (`/tmp/opencode/fresh-smoke`). Both print the banner and "7 applications registered."

**Blocks commit/push:** No (fix already committed).
**Blocks NC-S4b-09 closure:** No — technical evidence for the fix is adequate; closure additionally requires independent R2/PA acceptance (see VV-R2-004).

### VV-R2-002 (Minor) — No test-behavior regression; ctest 56/56 green confirms unchanged test outcomes

**Evidence:** `tools/scripts/run_sim.sh:24` (only CMake generator selection changed); `tests/` untouched. Smoke assertions unchanged (banner + "7 applications registered").

**Independent verification:** `ctest --test-dir build/host` → 100% passed, 0 failed out of 56, exit 0. This matches the standing ctest count and confirms the tooling change did not alter any test result.

**Blocks commit/push:** No.
**Blocks NC-S4b-09 closure:** No (positive confirmation; no regression to disposition).

### VV-R2-003 (Minor) — Remaining `-G Ninja` usages target fresh directories only; no latent cache-conflict regression

**Evidence:** `Makefile:111` (ci-local, fresh `/workspace/build/ci`), `Makefile:232` (coverage, fresh `build/coverage`), `tools/scripts/renode_xw2_vv.sh:84`, `tools/scripts/qemu_req_vv.sh:100`, `tools/scripts/qemu_xw2_vv.sh:100/114`, `tools/scripts/qemu_xw3_vv.sh:160` (all fresh dedicated `build/xw*` / `build/xw3*` dirs).

These are outside NC-S4b-09 scope. Because each targets a directory with no pre-existing CMake cache, `-G Ninja` is valid there and cannot reproduce the NC-S4b-09 failure mode (cache conflict).

**Blocks commit/push:** No.
**Blocks NC-S4b-09 closure:** No. Informational; no action required for this NC.

### VV-R2-004 (Minor) — T4/T9 (`run_sim.sh`) independent delta qualification review remains pending; this fix does not change that status

**Evidence:** `docs/vv/tool-qualification.md:27` (T4 row: "Independent delta qualification review pending (S8-FU)") and `:32` (T9 row: "Traced to T4; author-retested cached/fresh generator paths; independent delta review pending"). Both are "reviewed + CM", internal independence, status not qualified.

**Norm basis:** Norm N10 (tool qualification per ECSS-Q-ST-80C / ECSS-E-ST-40C) — a change to a tool invalidates previously produced evidence until re-qualification; independence of the qualification review is required per norm N9 profile R2. Norm N11 requires the review repeated and register updated in the same commit on tool change. The task context confirms no *new* N10 re-qualification trigger beyond the existing pending T4/T9 item.

**Blocks commit/push:** No (fix already committed; push authority rests with the aeronautics-space-fsw-engineer per norm N7).
**Blocks NC-S4b-09 closure:** YES — the sprint-plan NC-S4b-09 row requires independent R2/PA closure, and the tool-qualification register keeps T4/T9 pending independent delta review. This is a procedural/independence gate, not a deficiency in the fix or its test evidence.

### VV-R2-005 (Minor) — `make check` bundle legs beyond smoke/ctest (Lean, trace, GSW) not re-executed in this review; standing gate, not a regression from this fix

**Evidence:** `Makefile:105` (`check: build test verify-lean lean-receipt trace smoke gsw-selfcheck`).

This review independently verified the gates affected by NC-S4b-09 (smoke, ctest). Lean/trace/GSW depend on toolchain availability and are part of the standing N15 gate set; they are unrelated to the CMake-generator fix and were not identified as affected by the task context.

**Blocks commit/push:** No.
**Blocks NC-S4b-09 closure:** No. Recommend `make check` green as a matter of course before any dependent commit; not a precondition specific to NC-S4b-09.

---

## 4. Answers to the review questions

**Q1. Is the smoke-test PASS (exit 0, banner + 7 apps) sufficient verification evidence for NC-S4b-09?**

Yes — **for the technical scope of NC-S4b-09**. The NC is specifically that `make smoke`/`make check` fails because `run_sim.sh` forces `-G Ninja` against an existing Unix Makefiles cache. The smoke test directly exercises that exact path (cmake configure + build + boot + oracle checks), and it now passes on both the previously failing cached generator and a fresh directory. Together with ctest 56/56 (no test regression) and source confirmation that no other `-G Ninja` cache-conflict path exists in `run_sim.sh`, the evidence is sufficient to establish that the reported defect is fixed.

**Q2. Are any additional tests/gates needed?**

No new test scripts are required (the change is tooling-only, no functional software change). For completeness before NC-S4b-09 closure:
- Fresh-directory smoke evidence — **obtained in this review** (PASS, exit 0).
- ctest 56/56 — **obtained in this review** (PASS, exit 0).
- `make check` full bundle — standing gate; smoke and ctest legs verified; Lean/trace/GSW are environment-dependent and not affected by this fix.

No additional test type (unit, integration, HIL, Renode, formal) is applicable to a CMake-generator-selection fix.

**Q3. Does the change affect any existing test behavior?**

No. The change is isolated to `tools/scripts/run_sim.sh:24` (CMake generator selection). It does not touch production source, test sources, test oracles, check counts, or pass/fail criteria. Independently confirmed: ctest 56/56 identical, smoke assertions unchanged.

**Q4. Tool qualification: T4/T9 (`run_sim.sh`) — what is the status?**

T4 and T9 remain **"independent delta qualification review pending"** (`docs/vv/tool-qualification.md:27,:32`). This fix does not alter that status; no new N10 re-qualification trigger is introduced beyond the existing pending item (per task context). Author retesting of cached/fresh generator paths is documented in the register, but independent R2 delta review is still outstanding.

**Q5. Are there V&V findings that block commit/push or NC-S4b-09 closure?**

- **Commit/push:** Not blocked by this change. The fix is committed; push authority is reserved to the aeronautics-space-fsw-engineer per norm N7 after all applicable gates/norms and full sprint scope pass.
- **NC-S4b-09 closure:** Blocked **solely** by the pending independent T4/T9 delta qualification review and independent R2/PA acceptance (VV-R2-004). The technical test evidence (smoke cached + fresh, ctest 56/56, no regression) is sufficient; there is no open technical defect or insufficient-evidence finding.

---

## 5. Conclusion and recommendation

**Verdict:** The technical verification evidence for NC-S4b-09 is **sufficient**. The fix is implemented, committed, and independently verified (smoke PASS on cached and fresh CMake generators; ctest 56/56 green; no test-behavior change; no other `-G Ninja` cache-conflict exposure in `run_sim.sh`).

**Closure condition:** NC-S4b-09 may be closed only after the independent R2/PA delta qualification review of T4/T9 (`run_sim.sh`) is completed and recorded in `docs/vv/tool-qualification.md` and the sprint-plan NC row, per norms N9/N10/N11 and the sprint-plan entry. This review (VV-R2-004) does not itself grant that acceptance.

**Recommended next step:** Record this review's acceptance of the technical evidence, then route T4/T9 to the independent R2/PA authority for delta qualification; upon acceptance, mark NC-S4b-09 CLOSED and update `docs/vv/tool-qualification.md` and `docs/planning/sprint-plan.md` in the same controlled update (norm N11).

---
*This report is V&V evidence only. It does not approve qualification, acceptance, certification, compliance, anomaly closure, or flight readiness. Those decisions require the approvals defined by the governing baseline.*
