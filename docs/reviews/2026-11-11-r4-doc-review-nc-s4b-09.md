# R4 Documentation Review Report — NC-S4b-09 (smoke-workflow generator fix)

**Repository:** /home/nicola/fsw/frick-fsw, branch `main`, HEAD `0cc455f`
**Reviewer persona:** Senior Aeronautics and Space Software Documentation Engineer (R4 profile)
**Review norm:** N17 (documentation review on every change); supporting norms N2, N7, N8, N11, N15, N16, N19, N20, N21, N22
**NC under review:** NC-S4b-09 — Minor — `run_sim.sh` forced `-G Ninja` against an existing Unix Makefiles cache, causing the documented smoke/check workflow to fail (sprint-plan §3, S4b follow-up tool-gate audit, N19/N10)
**Fix status:** Fix committed and present at HEAD (`tools/scripts/run_sim.sh` reuses the cached CMake generator; no forced `-G Ninja`). Smoke gate verified PASS.
**Review date:** 2026-11-11
**Classification of this review:** No Major findings. No documentation finding blocks commit/push. NC-S4b-09 closure is contingent on an administrative NC-register update, not on a documentation defect.

---

## 1. Scope and items reviewed

The change under review is a one-line behavioral change to `tools/scripts/run_sim.sh`: the unconditional `-G Ninja` generator flag was removed so `cmake -S/-B` reuses the generator already cached in `BUILD_DIR` (e.g. the Unix Makefiles cache produced by `make configure`/`make build`). Per the task context, no docs, API, interface, traceability, or requirements change is needed.

Items examined:

| Item | File:lines | Relevance |
|---|---|---|
| Fixed script | `tools/scripts/run_sim.sh:1-39` | The fix itself; cmake invocation and comments |
| NC register row | `docs/planning/sprint-plan.md:135` | NC-S4b-09 disposition row (still reads OPEN) |
| Loop-audit row | `docs/status/loop-audit.md:191` | N19 audit row recording NC-S4b-09 open |
| Build wrapper | `Makefile:43-44` | `make smoke` → `bash tools/scripts/run_sim.sh` |
| Handover build example | `docs/status/handover.md:699` | Example `cmake ... -B build/native -G Ninja ...` command |
| Tool qualification register | `docs/vv/tool-qualification.md:27,171` | T4/T9 entries referencing `run_sim.sh` and the forced-Ninja fix |
| Verification plan/results | `docs/vv/verification-plan.md:144,176,222`; `docs/vv/verification-results.md:264` | Smoke evidence and build commands |
| Other `run_sim.sh`/`-G Ninja` references | Multiple (see §4.5) | Reviewed for stale forced-Ninja descriptions |

---

## 2. Evidence and independent verification

### 2.1 Fix is present at HEAD (Fact, high confidence)

`git show HEAD:tools/scripts/run_sim.sh` line 24:

```bash
cmake -S "${ROOT}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug -DFF_USE_PUS_ASN1=ON >/dev/null
```

There is **no** `-G Ninja` flag. The commit `8c84c7f` (fix recorded in tree) is an ancestor of HEAD, verified with `git merge-base --is-ancestor`. Baseline `2025fd9` introduced `-G Ninja`; the current HEAD version reuses the cached generator. Line 22 of the comment block documents the NC-S4b-09 rationale (see §3.1).

### 2.2 Smoke gate independently re-verified (Fact, high confidence)

A fresh-build-directory smoke run was executed:

```
BUILD_DIR=/tmp/opencode/ff-smoke-fresh bash tools/scripts/run_sim.sh
[smoke] building
[smoke] booting simulator (8 s window)
[smoke] PASS (banner + 7 applications registered)
EXIT=0
```

This validates the no-`-G Ninja` configure path on a fresh directory. The earlier `make smoke` failure observed in this shell (`CMake Error: The current CMakeCache.txt directory ... /workspace/build/native ... does not match ... /home/nicola/fsw/frick-fsw`) is a **stale-cache path artifact** — `build/native` was configured under a different source path (`/workspace`) in a prior containerized run — and is unrelated to the NC-S4b-09/Ninja defect. It is not a documentation finding.

The task context also records the smoke gate as verified PASS in the controlled environment; this independent fresh-directory run is consistent with that.

### 2.3 Sources

- Primary: repository working tree at HEAD (committed `run_sim.sh`, `sprint-plan.md`, `loop-audit.md`, `handover.md`, `Makefile`).
- Secondary: task context (NC-S4b-09 description, smoke-gate PASS, fix committed). No paywalled or inaccessible normative text is relied upon. No AI-generated citations are used.

---

## 3. Norm compliance assessment

### 3.1 Norm N2 — English (Compliant)

All comments in `tools/scripts/run_sim.sh` are in English:
- Lines 1–12: banner/purpose/exit-code/copyright header.
- Lines 20–23: NC-S4b-09 rationale ("Forcing `-G Ninja` unconditionally made `make smoke` ... fail ... (NC-S4b-09)").

No non-English comments, commit-message-relevant text, or user-facing strings are introduced by this change. **N2 satisfied.**

### 3.2 Norm N8 — complete, current, deployable documentation (Satisfied — no doc change required)

The change is shell-script behavior only; it alters no requirements, interfaces, API, traceability, user-facing procedure, or controlled document content. Therefore no documentation update is required *for the change itself*. `docs/planning/sprint-plan.md` header is current (Rev 1.131, date 2026-11-11, norms N3/N4/N8/N15/N19). **N8 satisfied; the correct N8 action is to record that no doc change is needed (this report), not to edit product documentation.**

### 3.3 Norms N7 / N15 / N16 / N19 (Ownership and records)

- **N7:** Only the `aeronautics-space-fsw-engineer` agent may commit/push after all gates/norms pass. I am the R4 documentation reviewer; I will **not** commit or push. The NC-register update (DOC-R4-004/DOC-R4-005) must be performed by the authorized committer.
- **N15:** Full applicable cycle re-run — the change is a smoke-workflow fix; build/test/Lean/trace/smoke gates are the applicable cycle. Smoke PASS confirmed; no doc-relevant gate remains open from this review.
- **N16:** Revision history — if the NC-register rows are updated, the change must include a revision bump and history entry (sprint-plan.md Rev 1.131 → 1.132).
- **N19:** Loop-audit — `docs/status/loop-audit.md:191` records NC-S4b-09 as open; it should be updated in the same register pass.

### 3.4 Norms N20 / N21 / N22

- **N20 (Senior SW PA review):** The fix is a non-safety shell-script workflow change (no safety/security/limits/warning content). No PA-blocking documentation finding is raised. The sprint-plan row lists verifier R2/PA for the broader NC set; PA disposition of NC-S4b-09 itself is outside this documentation review's scope.
- **N21:** Functional smoke fix completed and verified; no non-functional findings require deferral.
- **N22:** The smoke script asserts explicit expected results (banner present; 7 applications registered) against a nominal case; consistent with the norm.

---

## 4. Findings

Finding IDs: `DOC-R4-001` … `DOC-R4-007`. Class: **Major** or **Minor**. Each finding states whether it **blocks commit/push** and whether it **blocks NC-S4b-09 closure**.

### DOC-R4-001 (Minor — compliant, no action)
**Fix present; no documentation change required for the change itself.**

`tools/scripts/run_sim.sh:24` invokes `cmake -S/-B` without `-G Ninja`, reusing the cached generator. The fix is committed at HEAD. No docs, API, interface, traceability, requirements, user procedure, or controlled-document content is affected by the behavioral change.

- Blocks commit/push: **No**
- Blocks NC-S4b-09 closure: **No**

### DOC-R4-002 (Minor — compliant, no action)
**Norm N2 (English): all `run_sim.sh` comments are in English.**

Lines 1–12 (header/purpose) and 20–23 (NC-S4b-09 rationale) are English. No translation or terminology issue.

- Blocks commit/push: **No**
- Blocks NC-S4b-09 closure: **No**

### DOC-R4-003 (Minor — compliant, no action)
**Norm N8: no documentation updates required for this smoke-workflow fix.**

Sprint-plan.md header (Rev 1.131, 2026-11-11) is current. The R4 determination is that the correct N8 outcome is "no doc change needed," recorded in this review report; no product document must be edited for NC-S4b-09.

- Blocks commit/push: **No**
- Blocks NC-S4b-09 closure: **No**

### DOC-R4-004 (Minor — register update required)
**`docs/planning/sprint-plan.md:135` NC-S4b-09 row still reads OPEN.**

Current cell: `**OPEN — fix implemented; independent closure pending.** Existing-cache and fresh-directory smoke runs pass. Owner: FSW author; sprint: S8-FU; verifier: independent R2/PA.`

The independent R4 review evidence is complete (fix committed, smoke PASS, N2/N8 satisfied). The row should be updated to reflect closure: disposition `CLOSED`, citing this R4 review (`docs/reviews/2026-11-11-r4-doc-review-nc-s4b-09.md`), evidence (fresh-directory smoke PASS, EXIT=0), and a revision bump (1.131 → 1.132) with a history entry per N16. Related: the S4b sprint summary row at `docs/planning/sprint-plan.md:88` lists `NC-S4b-04/05/06/07/08/09 pending independent review`; once NCs 04–08 also close, that summary cell should be revised, but for NC-S4b-09 alone it remains factually accurate (04–08 still pending).

This is an NC-register update, not a product-documentation content change. It must be committed by the authorized `aeronautics-space-fsw-engineer` (N7).

- Blocks commit/push: **No**
- Blocks NC-S4b-09 closure: **Yes — closure is contingent on this register row being updated to CLOSED with this review cited as disposition (administrative action, not a documentation defect).**

### DOC-R4-005 (Minor — register update required)
**`docs/status/loop-audit.md:191` N19 row still records NC-S4b-09 open.**

Current cell: `| \`run_sim.sh\` generator conflict | NC-S4b-09 open pending delta review; smoke passes using both the cached Makefiles generator and a fresh build directory. |`

Should be updated in the same register pass as DOC-R4-004 to record the closure disposition and reference this R4 review and the smoke evidence.

- Blocks commit/push: **No**
- Blocks NC-S4b-09 closure: **Part of the same register-update dependency as DOC-R4-004.**

### DOC-R4-006 (Minor — observation, recommendation)
**`docs/status/handover.md:699` contains a `cmake ... -B build/native -G Ninja ...` example that could reintroduce the generator-conflict class.**

`build/native` is the default cache directory created by `make configure` (`Makefile:28`, Unix Makefiles generator, no `-G Ninja`). If the handover example is run sequentially after `make configure` without clearing the cache, CMake errors with a generator mismatch — the same conflict class fixed in NC-S4b-09. This example predates the fix and is not introduced by it; it targets a fresh-directory pattern in context. Recommend aligning it with the cached-generator reuse pattern (or a explicitly fresh/separate `BUILD_DIR`) in a future documentation delta. Not required for NC-S4b-09 closure.

- Blocks commit/push: **No**
- Blocks NC-S4b-09 closure: **No**

### DOC-R4-007 (Minor — compliant, no action)
**Other `run_sim.sh` / `-G Ninja` documentation references reviewed; none require change.**

| File:lines | Content | Disposition |
|---|---|---|
| `docs/vv/tool-qualification.md:27,171` | T4/T9 reference `run_sim.sh`; records forced-Ninja fix re-qualified, verified cached/fresh generator paths | Consistent with fix; no change |
| `docs/vv/verification-plan.md:144,176,222` | Smoke evidence via `run_sim.sh`; 7 apps, EVS→PUS S5 | Accurate; no change |
| `docs/vv/verification-results.md:264` | `cmake -S . -B build/gate6 -G Ninja ...` (fresh dir `build/gate6`) | Fresh directory; no conflict class; no change |
| `docs/engineering/software-description.md:350,388-410` | Lists `run_sim.sh` as T4/T9 tool; ci-local/scope notes | Accurate; no change |
| `docs/tooling.md:61,67` | Tool entries for `run_sim.sh` (T4/T9) | Accurate; no change |
| Evidence logs (`docs/vv/evidence/*`, `docs/reviews/*`) | Various `cmake ... -G Ninja` on fresh/build-specific directories | Fresh targets; no conflict class; no change |

None describe forced `-G Ninja` against an existing cache as current behavior.

- Blocks commit/push: **No**
- Blocks NC-S4b-09 closure: **No**

---

## 5. Blocking determination

| Question | Determination | Basis |
|---|---|---|
| Does any documentation finding block commit/push? | **No** | No Major or Minor finding prevents commit. N17 documentation-review loop is closed on the basis of "no product documentation change required" (this report is the review evidence). |
| Does any documentation finding block NC-S4b-09 closure? | **No documentation defect blocks closure.** | Closure is contingent on the administrative NC-register update (DOC-R4-004 sprint-plan.md:135; DOC-R4-005 loop-audit.md:191) being committed by the authorized agent. The independent technical review itself is complete. |

**Overall R4 verdict:** The documentation associated with NC-S4b-09 is complete, consistent, and in English; no documentation rework is required for the fix. NC-S4b-09 may be closed once the authorized committer updates the NC register rows to CLOSED citing this review.

---

## 6. Required actions and ownership

| # | Action | Owner | Norm | Blocks NC-S4b-09 closure? |
|---|---|---|---|---|
| 1 | Update `docs/planning/sprint-plan.md:135` NC-S4b-09 row to CLOSED; disposition citing this R4 review; rev bump 1.131→1.132; history entry | `aeronautics-space-fsw-engineer` (authorized committer; not this R4 reviewer per N7) | N7, N16 | Yes |
| 2 | Update `docs/status/loop-audit.md:191` to record closure disposition + evidence reference | `aeronautics-space-fsw-engineer` | N19 | Yes (same pass as #1) |
| 3 | (Optional, future doc delta) Align `docs/status/handover.md:699` `-G Ninja` example with cached-generator reuse or separate fresh `BUILD_DIR` | FSW author | N8 (future) | No |
| 4 | No edit to `tools/scripts/run_sim.sh`; fix complete | — | — | No |

---

## 7. Uncertainty and confidence

- Fix presence at HEAD: **High** (verified via `git show HEAD` and `git merge-base`).
- Smoke PASS on fresh directory: **High** (independent re-execution, EXIT=0).
- Stale-cache `make smoke` failure in this shell: **Low relevance** — environment path artifact (`/workspace` cache), not a product defect; excluded from findings.
- Register-update dependency: **High** — rows currently read OPEN; closure requires the update, but no technical documentation gap exists.

No material conflict between sources. No paywalled or inaccessible normative text is relied upon. No AI-generated citations are used.

---

## 8. Reviewer sign-off

**Reviewer:** Senior Aeronautics and Space Software Documentation Engineer (R4 profile) — independent documentation review under norm N17  
**Date:** 2026-11-11  
**Repository:** /home/nicola/fsw/frick-fsw, HEAD `0cc455f`  
**NC reviewed:** NC-S4b-09 (Minor)

**Statement:** I reviewed the documentation impact of the NC-S4b-09 smoke-workflow fix (`tools/scripts/run_sim.sh`: removal of forced `-G Ninja`, cached-generator reuse). I find no documentation changes required for the change itself, Norm N2 (English) satisfied, Norm N8 satisfied with no doc update needed, and no documentation finding that blocks commit/push. NC-S4b-09 closure is contingent solely on the administrative NC-register update (sprint-plan.md:135; loop-audit.md:191) to CLOSED with this review cited as disposition, to be committed by the authorized `aeronautics-space-fsw-engineer` per norm N7. This review report is retained in `docs/reviews/` per norm N11.

This persona has no delegated engineering, safety, certification, records, export-control, legal, or release authority. Technical content and safety-critical decisions require review and approval by qualified authorized humans.
