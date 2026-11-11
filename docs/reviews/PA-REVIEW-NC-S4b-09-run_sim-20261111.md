# Independent Product Assurance Review — NC-S4b-09 (run_sim.sh CMake generator fix)

**Norm:** N20 (Senior SW Product Assurance independent review).  
**Reviewer role:** Senior Aeronautics and Space Software Product Assurance Engineer (independent PA; not a certification/approval authority — see authority statement below).  
**Repository / branch / HEAD:** frick-fsw / main / 0cc455f.  
**Review date:** 2026-11-11.  
**Change under review:** `tools/scripts/run_sim.sh` — NC-S4b-09 fix. The script no longer forces `-G Ninja` unconditionally; it configures with `cmake -S ... -B ... -DCMAKE_BUILD_TYPE=Debug -DFF_USE_PUS_ASN1=ON` (no `-G`) and builds with `cmake --build`, thereby reusing the generator already cached in `BUILD_DIR` (e.g. the Unix Makefiles cache produced by `make configure`/`make build`).  
**NC register reference:** sprint-plan.md row 135 (NC-S4b-09, Minor, S4b follow-up tool-gate audit N19/N10): "`run_sim.sh` forced Ninja against an existing Unix Makefiles cache, causing the documented smoke/check workflow to fail" — status "OPEN — fix implemented; independent closure pending."

---

## Authority statement

This report is an **independent product assurance evidence assessment only**. It does not:

- certify compliance, qualify a tool, accept residual risk, close a nonconformance, authorize release, or declare flightworthiness;
- close **NC-S4b-09** (closure decision rests with the FSW engineer / authorized PA authority; this report is a recommendation/evidence input only);
- establish any flight-target behaviour, timing, safety, security, or hardware scope.

The change is scoped to the **host/SIL smoke workflow** (`run_sim.sh`). It is a non-functional tooling fix; no safety-critical, security, flight, or hardware impact.

---

## 1. Scope and evidence base

| Artifact | Relevant content |
|---|---|
| `tools/scripts/run_sim.sh` | Fix under review: no `-G Ninja`; `cmake -S -B ... -DCMAKE_BUILD_TYPE=Debug -DFF_USE_PUS_ASN1=ON` (line 24); `cmake --build` (line 25); comment lines 20-23 documenting NC-S4b-09 and cached-generator reuse |
| `build/host/CMakeCache.txt` | `CMAKE_GENERATOR:INTERNAL=Unix Makefiles` — existing Makefiles cache the fix must reuse |
| `build/native/CMakeCache.txt` | `CMAKE_GENERATOR:INTERNAL=Unix Makefiles` — default `make smoke` build dir, same generator |
| `git show HEAD:tools/scripts/run_sim.sh` | Fix present in HEAD tree (comment lines 22-23 reference NC-S4b-09; no `-G Ninja` cmake line) |
| `git log --all -- tools/scripts/run_sim.sh` | No commit in history references NC-S4b-09 or documents the Ninja removal; v0.1 baseline (2025fd9) added `-G Ninja` (line 18); removal not traceable to an NC-referencing commit |
| `git log --all --grep="NC-S4b-09"` | Empty — no commit message references NC-S4b-09 |
| `CMakeLists.txt:79-113` | OBCP/N7 source-generation step executed at configure time when `FF_USE_PUS_ASN1=ON` |
| sprint-plan.md:135 | NC-S4b-09 register row: "OPEN — fix implemented; independent closure pending. Existing-cache and fresh-directory smoke runs pass." |
| docs/status/loop-audit.md:191 | "`run_sim.sh` generator conflict | NC-S4b-09 open pending delta review; smoke passes using both the cached Makefiles generator and a fresh build directory." |

Trace counts unaffected by this non-functional tooling change (no new requirements, Lean theorems, generated code, or algorithms).

---

## 2. Verification of review claims

### Claim 1 — Fix removes forced `-G Ninja` and reuses the cached generator: VERIFIED

- `run_sim.sh:24` runs `cmake -S "${ROOT}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug -DFF_USE_PUS_ASN1=ON` — **no `-G` flag**, so CMake selects/reuses the generator cached in `BUILD_DIR`.
- `run_sim.sh:25` builds with `cmake --build "${BUILD_DIR}"` — generator-agnostic.
- Comment lines 20-23 explain the rationale (NC-S4b-09) and state the generator is reused when the dir is already configured.
- `build/host` is configured with `Unix Makefiles`; the fix does not override it to Ninja.

### Claim 2 — Smoke test passes on the existing Makefiles cache: VERIFIED (independently re-executed)

Command: `BUILD_DIR=build/host bash tools/scripts/run_sim.sh`

Result:
```
[smoke] building
[smoke] booting simulator (8 s window)
[smoke] PASS (banner + 7 applications registered)
```
Exit code: **0**. Banner (`frick-fsw`) and "7 applications registered" both present in the simulator log. The pre-existing `build/host` Unix Makefiles cache was reused (no Ninja override), and the build + boot succeeded.

### Claim 3 — No safety/security/flight/hardware impact: VERIFIED

- The change touches only `tools/scripts/run_sim.sh` (host/SIL smoke script). No `src/` source, design, requirements, interface, build flag affecting flight behaviour, PAL/watchdog hook, security control, or hardware register is modified.
- NC-PAL-01 (flight MPFS MSS watchdog binding), NC-EXT-19/90, GAP-A5, OBCP engine selection, and all safety-critical/class-B flight scope are unaffected (confirmed by scope of diff and task context).
- This is a tool-gate/smoke-workflow fix on the host/SIL evidence base only.

### Claim 4 — Fresh-directory smoke run: NOT PASSED (pre-existing, unrelated to Ninja fix)

Command: `rm -rf /tmp/opencode/ff-fresh-smoke; BUILD_DIR=/tmp/opencode/ff-fresh-smoke bash tools/scripts/run_sim.sh`

Result: **FAIL, exit 1** — configure/build aborts at `CMakeLists.txt:94` ("N7 OBCP source generation failed (2)") during the `make flat`/MicroPython embed source-generation step. `run_sim.sh` does **not** fetch third-party dependencies (`make pus-asn1` / `fetch-third-party`), so a truly fresh tree without pre-fetched/prepared `third_party/n7s-obcp` content fails at configure time.

This failure is **pre-existing and unrelated to the `-G Ninja` conflict** (it occurs in the OBCP source-generation step, not the CMake generator selection). It does not invalidate the NC-S4b-09 fix, but it contradicts the sprint-plan/loop-audit evidence claim that "fresh-directory smoke runs pass." See Finding PA-001.

### Claim 5 — Other `-G Ninja` usages are out of scope: VERIFIED

Remaining `-G Ninja` occurrences (`.github/workflows/ci.yml`, `.gitea/workflows/ci.yml`, `Makefile:111/232`, `tools/scripts/qemu_*.sh`, `tools/scripts/renode_xw2_vv.sh`, docs/evidence) all target **fresh/dedicated CI build directories** (or a reset tree), not an existing Unix Makefiles cache. They do not exhibit the NC-S4b-09 cache-conflict failure mode and are outside the scope of this NC.

---

## 3. Findings

| ID | Severity | File:line | Finding | Recommended disposition | Blocks commit/push? | Blocks NC-S4b-09 closure? |
|---|---|---|---|---|---|---|
| **PA-001** | Minor | sprint-plan.md:135; docs/status/loop-audit.md:191 | Evidence claim overbroad: both records state "fresh-directory smoke runs pass." Independently re-tested: fresh-directory smoke **fails** at OBCP source generation (`CMakeLists.txt:94`), a pre-existing dependency-fetch matter unrelated to the Ninja fix. Only the existing-cache/Makefiles-configured smoke run passes. | Correct the sprint-plan NC row and loop-audit entry to state that the existing-cache smoke passes and that a fresh-directory run requires `make pus-asn1`/`fetch-third-party` first (dependency fetch), then record the corrected evidence. This is an evidence-record correction, not a defect in the fix. | **No** | **No** (closure evidence should be accurate, but the NC itself — Ninja conflict — is resolved) |
| **PA-002** | Minor | Traceability / revision history (norms N4/N16) | NC-S4b-09 run_sim.sh fix is present in the HEAD tree, but no commit message references NC-S4b-09 or documents the Ninja removal (`git log --all --grep="NC-S4b-09"` empty; `git log -- run_sim.sh` shows no Ninja-removal commit; v0.1 baseline 2025fd9 added `-G Ninja`). For audit traceability, the NC fix should be identifiable in revision history. | Record the NC-S4b-09 fix commit/hash in the sprint-plan NC row / NC register. If the fix is not yet committed under an NC-referencing message despite being present in the tree, commit it with an NC-S4b-09 reference before push (normal N7/N15 gates apply). | **No** | **No** (traceability observation; supports audit evidence) |

**No Major findings.** No finding questions the correctness or boundedness of the fix.

---

## 4. NC-S4b-09 resolution assessment (PA perspective)

| NC | Status after this change | PA basis |
|---|---|---|
| **NC-S4b-09** (`run_sim.sh` forced `-G Ninja` against existing Unix Makefiles cache; smoke/check workflow failure) | **Resolved from a PA perspective on the host/SIL evidence base.** | Fix verified correct (no `-G Ninja`; reuses cached generator). Existing-cache smoke (`build/host`, Unix Makefiles) independently re-verified PASS (exit 0, banner + 7 apps). Bounded, non-functional tooling change; no safety/security/flight/hardware impact. Other `-G Ninja` usages are fresh-directory CI builds, out of scope. | 
| **Fresh-directory smoke precondition** (observed during review, not part of NC-S4b-09) | **Remains a precondition note, not an NC.** | Fresh-directory smoke fails at OBCP source generation because `run_sim.sh` does not fetch third-party deps. Pre-existing, unrelated to Ninja fix. Tracked via PA-001 (evidence-record correction). | 

**PA does not close NC-S4b-09.** Closure is the decision of the FSW engineer / authorized PA authority. This report is a recommendation/evidence input only. No authority-owned NC is closed by this review.

---

## 5. PA concerns with committing / conditions for commit

From a PA perspective, **there are no findings that block commit/push.** The fix is small, bounded, correct at source level, independently smoke-verified (exit 0), and does not alter any flight behaviour or close any safety-relevant NC.

**Recommendations before commit/push (non-blocking):**

1. If any further change is made to `run_sim.sh` as part of this cycle, it must go through the normal gates (build/test/Lean/trace/format/license per N7/N15) before push; only the `aeronautics-space-fsw-engineer` agent (or explicit user delegation) may commit/push.
2. Fold PA-001 into the sprint-plan/loop-audit evidence record so the documented evidence matches the verified result (existing-cache pass; fresh-directory requires dependency fetch first).
3. Fold PA-002 into the revision history / NC register so the NC-S4b-09 fix is traceable to a commit referencing NC-S4b-09 (norms N4/N16).
4. Do not imply that the fresh-directory smoke run passes without the `make pus-asn1`/`fetch-third-party` precondition; do not extend the NC scope to OBCP dependency fetching.
5. No NC is closed by this increment; NC-S4b-09 closure remains an authority decision.

---

## 6. Uncertainty and confidence report

| Field | Content |
|---|---|
| Decision/product and configuration | NC-S4b-09 fix; `tools/scripts/run_sim.sh` at HEAD 0cc455f; host/SIL smoke (`BUILD_DIR=build/host`, Unix Makefiles cache) |
| Claim type | Facts (source inspection, git history, direct test execution) + Inference (fresh-directory failure is pre-existing/dependency-related) |
| Applicable baseline and approved tailoring | Project norms N1-N22 (N20 independent PA review); ECSS-Q-ST-80C Rev.2 / ECSS-E-ST-40C Rev.1 as invoked by project norms. No flight certification basis implicated. |
| Evidence consulted | `run_sim.sh` (full), `git log -- run_sim.sh`, `git log --all --grep="NC-S4b-09"`, `git show HEAD:run_sim.sh`, `build/host/CMakeCache.txt`, `build/native/CMakeCache.txt`, `CMakeLists.txt:79-113`, sprint-plan.md:135, loop-audit.md:191; direct execution of existing-cache smoke (PASS) and fresh-directory smoke (FAIL) |
| Cross-check and reconciliation | Existing-cache smoke PASS confirmed twice (task context + independent re-run). Fresh-directory smoke FAIL directly observed (OBCP source generation, unrelated to Ninja). Other `-G Ninja` usages confirmed fresh-directory CI builds. |
| Confidence | **High** for Claims 1–3 (direct source inspection + executed test, no conflict). **Medium** for Claim 4 root cause (OBCP `make flat`/MicroPython embed failure directly observed; exact pre-existing dependency state not fully traced). **High** for "no flight impact" (scope-limited tooling change). |
| Missing/inaccessible evidence | None material to NC-S4b-09. Fresh-directory OBCP failure root cause not fully traced (build-environment/third-party fetch state), but it is out of scope for the Ninja NC. |
| Assumptions and expiry | Assumes `build/host` cache state is representative of a Makefiles-configured tree. Re-check if `run_sim.sh` or CMake generator handling changes. |
| Consequences if wrong | If `-G Ninja` were inadvertently reintroduced, `make smoke`/`make check` on a Makefiles-configured tree would fail (caught at build/test gate); no flight-safety impact. |
| Required independent human reviewer/authority | FSW engineer (commit/push per N7); authorized PA/human authority for NC-S4b-09 closure decision. |

---

## Authority statement

This report is an independent product assurance assessment, not a certification, approval, qualification, release authorization, or flightworthiness declaration. It does not close NC-S4b-09, accept residual risk, or authorize release. Independent qualified humans retain those decisions.

**PA review outcome:** NC-S4b-09 is **resolved from a PA perspective on the host/SIL evidence base** (existing-cache smoke PASS; fix correct and bounded; no safety/security/flight/hardware impact). **No PA finding blocks commit/push or NC-S4b-09 closure.** PA-001 (evidence-record accuracy) and PA-002 (traceability) are non-blocking recommendations. NC-S4b-09 closure decision rests with the FSW engineer / authorized PA authority.
