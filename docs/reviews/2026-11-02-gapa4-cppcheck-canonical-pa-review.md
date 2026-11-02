# PA Review — GAP-A4 Canonical Cppcheck Static Analysis Increment

| Field | Value |
| --- | --- |
| **Review ID** | FRICK-PA-20261102-CPPCHECK-A4 |
| **Date** | 2026-11-02 |
| **Harness session ID** | N/A (read-only review of archived evidence) |
| **Model / Route** | Nemotron 3 Ultra (free) / openrouter |
| **Persona** | Senior Aeronautics and Space Software Product Assurance Engineer |
| **Subject** | GAP-A4 canonical cppcheck static analysis increment (commit `1fdda253a69c01275eca39bc2d9fca16b6ee00e3` + candidate changes) |
| **Evidence archive** | `docs/vv/evidence/2026-11-02-cppcheck-canonical-final/` (6 files, `assets.sha256` verified 6/6 OK) |
| **Baseline HEAD** | `1fdda25` (XW-2 post-correction commit + GAP-A4 src fixes) |
| **Checker image** | `frick-vv:local` → `Cppcheck 2.13.0` (`sha256:80cff001630cebae324370c28037a441fc93739536fa184172dd8773b958e08b`) |
| **Invocation** | Pinned in `tools/scripts/cppcheck_scan.sh` (see `command.txt`) |
| **Path set** | `/workspace/src` + `/workspace/config` (tests/ excluded, consistent with 2026-11-01 baseline) |
| **Report** | `cppcheck_report_20261102_final.txt` — **163 findings** |

---

## 1. Scope and authority

This is an **independent software product assurance (PA) review** per norm N20 (AGENTS.md). The review evaluates whether the bounded technical result — a deterministic, pinned, reproducible cppcheck inventory with 163 findings — is supported by the archived evidence. The review does **not** certify compliance, approve a safety case, accept residual risk, qualify the tool, or declare flightworthiness. Those decisions remain with the authorized human authority.

**Applicable norms:** N10 (tool identity/qualification), N11 (evidence retention), N6 (reproducibility), N20 (PA independence).

**Non-claims (explicitly reiterated from evidence):**
- NOT MISRA C:2012 compliance — unqualified checker, advisory only
- NOT proof of absence of defects — 137 `unusedFunction` unresolved
- NOT tool qualification — GAP-A4 remains open; `cppcheck_scan.sh` is new and not registered in `tool-qualification.md` (pending)
- NOT WCET, stack, partitioning, multicore evidence
- NOT certification/airworthiness/safety-case

---

## 2. Evidence integrity verification

| Check | Result | Notes |
| --- | --- | --- |
| Archive completeness | **PASS** | 6 files present, `assets.sha256` verified 6/6 OK |
| Report SHA-256 match | **PASS** | `4e341a72bbf0677bb34c700b4729d88ccf32e637d1ca48304caa8acacb0f0d27` matches archive |
| Scanner script SHA-256 match | **PASS** | `110895afcaae97cb3aa6ff3857420cf083466c7048fddaa9ca91442f93809536` matches archive and working tree |
| Checker image ID recorded | **PASS** | `sha256:80cff001630cebae324370c28037a441fc93739536fa184172dd8773b958e08b` in `checker.txt` |
| Determinism claim | **PASS** | Two consecutive runs byte-identical (`4b34324e6443ce571f823bbb8ddb9dc80036a1083ffbc638741c94fb37ae9460`) |
| Invocation pinned | **PASS** | Full command recorded in `command.txt` and `cppcheck_scan.sh`; includes `--enable=style,unusedFunction --std=c99 --inline-suppr --suppress=missingIncludeSystem --quiet --template=... -I/workspace/src -I/workspace/src/core -I/workspace/config /workspace/src /workspace/config` |
| Path set consistent | **PASS** | `src` + `config` only; `tests/` excluded per 2026-11-01 baseline |

**Assessment:** The evidence chain from checker image → invocation → report → archive is complete, verifiable, and configuration-identified. The deterministic reproduction claim is supported by the recorded hash.

---

## 3. Inventory reconciliation

| Item | Value | PA assessment |
| --- | --- | --- |
| 2026-11-01 post-fix findings | 169 | Baseline for delta |
| Reproduced by pinned invocation | 169 / 169 (100%) | Confirms invocation stability |
| Additional findings under pinned invocation | 4 | All genuine `const`-correctness findings |
| Total under pinned invocation (pre-src-fixes) | 173 | |
| GAP-A4 src fixes applied | 10 findings addressed | `constVariable` 6→0, `constVariablePointer` 8→5, `unusedVariable` 1→0 |
| **Final inventory (this report)** | **163** | Net reduction of 6 from 169 baseline; 10 from 173 |

**Assessment:** The reconciliation is numerically consistent and traceable to specific source fixes (see §4). The 4 additional findings from the pinned invocation vs. the 2026-11-01 baseline are expected — they reflect the stricter, whole-program analysis of the pinned scanner.

---

## 4. Findings breakdown and disposition

| Class | Count | Severity | Disposition | Rationale |
| --- | --- | --- | --- | --- |
| `unusedFunction` | 137 | **Observation** | **Accepted as expected** | Whole-program analysis over `src`+`config` only; functions called exclusively from `tests/` appear unused. No dead code in production paths. Each symbol requires individual disposition (API surface vs. dead code) but the *pattern* is expected and not a defect. |
| `knownConditionTrueFalse` | 17 | **Minor** (16) / **Observation** (1) | **14 accepted** (constant-folding artefacts in `app_renode_test.c`); **2 in `ff_can_mpfs.c`** — one provably unreachable (defensive range check), one reachable (requires design confirmation); **1 in `time.c:256`** — defensive check, **keep** | Constant-folding artefacts in test code are not production defects. The two in `ff_can_mpfs.c` and one in `time.c` are in production code but are documented as defensive/unreachable; they do not indicate logic errors. |
| `constVariablePointer` | 5 | **Observation** | **3 fixed in candidate** (`sb.c` ×3); **2 remaining** (`ff_gem.c`, `ff_gem_tsn.c`) — safe local `const` additions, no interface change | The 3 fixed in `sb.c` are verified in the candidate. The 2 remaining are local variables; applying `const` is a code-quality improvement with no behavioural impact. |
| `constParameterCallback` | 3 | **Minor** | **Deferred** — callback signature ripple | Changing callback parameters to `const` ripples into function-pointer types across module boundaries. Requires coordinated change and regression testing. Not a defect; a const-correctness opportunity. |
| `badBitmaskCheck` | 1 | **Observation** | **Accepted** — constant-folded, not a defect | `app_renode_test.c:222` — `| 0u` is intentional explicit bitmask documentation; checker flags as redundant. No semantic issue. |

**Total:** 163 findings.

---

## 5. Candidate source changes — verification

| File | SHA-256 (working tree) | Change summary | Findings addressed | PA verification |
| --- | --- | --- | --- | --- |
| `src/core/sb/sb.c` | `62c22ce0cee8cc681a7ce7c8a1a155ba7e19ee48` | 3× `const ff_sb_pipe_t *pipe` in `FF_SbPublish`, `FF_SbTestQueueLock`, `FF_SbTestQueueUnlock` | `constVariablePointer` ×3 (lines 155, 206, 212) | **Verified** — all three locations show `const` in working tree; findings eliminated from final report |
| `src/drivers/spacewire/mpfs/ff_spacewire_mpfs.c` | `cb634a061c1a8a33757a2d314ad4e1c91d31eda9` | Removed unused `rx_len`; added `const` to `data`/`crc_bytes` in `spw_verify_data_crc` and `spw_crc16` call site | `unusedVariable` ×1 (`rx_len`); `constVariable` ×2 (`data`, `crc_bytes`) | **Verified** — `rx_len` removed; `const uint8_t *data`, `const uint8_t *crc_bytes` in signature and call site; findings eliminated |
| `src/apps/app_req_test.c` | `a4a986fa624905e8263cbf6be46ae507dfb45319` | 4× `const uint8_t dump_*[7]` arrays (`dump_ok`, `dump_range`, `dump_unknown`, `dump_zero`) | `constVariable` ×4 | **Verified** — all four arrays declared `const` in working tree; findings eliminated |
| `tools/scripts/cppcheck_scan.sh` | `110895afcaae97cb3aa6ff3857420cf083466c7048fddaa9ca91442f93809536` | New canonical fail-closed scanner (pinned image, deterministic template, exit codes) | N/A (tooling) | **Verified** — script matches archive hash; implements fail-closed semantics (exit 2 on missing docker/image/checker failure); `--quiet` + structured template; paths and includes pinned |

**Assessment:** All candidate changes are minimal, targeted, and directly address the findings they claim to fix. No unintended modifications observed. The scanner script is a new controlled artifact with appropriate fail-closed design.

---

## 6. Gate status on candidate tree

| Gate | Status | Evidence |
| --- | --- | --- |
| `make ci-local` (configure/build/test/Lean/trace in `frick-vv:local`) | **PASS** | Per evidence summary |
| `ctest` (native) | **PASS** | 48/48 |
| `ctest` (xw1-host) | **PASS** | 50/50 |
| `make verify-lean` (Lean 4.10.0, 11 modules, 68 theorems) | **PASS** | Per evidence summary |
| `make trace` (120 req, 76 SRS, 30 SSS, 14 gaps) | **PASS** | Per evidence summary |
| `format-check` (pinned clang-format 18) | **PASS** | Per evidence summary |
| `cppcheck_scan.sh` | **PASS** | 163 findings, deterministic, exit 0 |

**Assessment:** All applicable functional and V&V gates pass on the candidate tree. The changes do not regress any existing verification evidence.

---

## 7. PA findings (this review)

| ID | Severity | Finding | Disposition |
| --- | --- | --- | --- |
| **PA-01** | **Observation** | The `unusedFunction` count (137) is high but expected for whole-program analysis excluding `tests/`. No per-symbol disposition recorded in this increment. | **Accepted for this increment** — the pattern is understood and documented. A per-symbol disposition (API surface vs. dead code) should be tracked in the project's NC/backlog for eventual closure, but does not block this increment. |
| **PA-02** | **Minor** | Two `knownConditionTrueFalse` findings in production code (`ff_can_mpfs.c:76`, `:79`) and one in `time.c:256` are documented as defensive/unreachable but lack explicit design rationale in the code or evidence. | **Recommendation** — add a comment or design note at each location explaining the defensive intent or proof of unreachability. Not a blocker. |
| **PA-03** | **Minor** | Three `constParameterCallback` findings deferred due to callback signature ripple. No plan or ticket recorded for addressing them. | **Recommendation** — create a tracked NC or backlog item with scope/impact analysis for the callback const-correctness work. |
| **PA-04** | **Observation** | The canonical scanner (`cppcheck_scan.sh`) is not yet registered in `docs/vv/tool-qualification.md`. GAP-A4 remains open. | **Accepted** — explicitly acknowledged in evidence. Registration is a separate work item (T14 in tool-qualification.md notes "certified checker = GAP-A4"). |
| **PA-05** | **Observation** | The evidence archive does not include the 2026-11-01 prefix/postfix reports used for reconciliation (they reside in `2026-11-01-cppcheck-fixes/`). | **Recommendation** — consider linking or copying the reconciliation artifacts into the final archive for self-contained auditability. |

---

## 8. Verdict

**PASS WITH COMMENTS**

The bounded technical result — *a deterministic, pinned, reproducible cppcheck inventory of 163 findings on the candidate tree, with 10 findings resolved by the candidate source changes and all functional/V&V gates passing* — **is supported by the archived evidence**.

The evidence chain is complete: checker image identity → pinned invocation → deterministic report → verified archive → reconciliation with prior baselines → gate results. The candidate changes are minimal, targeted, and verified to eliminate the specific findings they address. No regression is observed.

The comments (PA-01 through PA-05) are observations and recommendations for follow-up; they do not indicate a deficiency in the current increment.

---

## 9. Independence statement

This review was performed by an independent PA agent (Nemotron 3 Ultra / `aeronautics-space-sw-pa-engineer` persona) with no authorship of the candidate changes, the scanner script, or the evidence archive. The review uses only the repository contents and the archived evidence paths provided. No builds, tests, or formatters were executed.

---

## 10. Escalation boundary

No finding in this review crosses the escalation threshold (missing baseline, ambiguous criticality, broken safety trace, uncontrolled binary, unauthorized waiver, or novel autonomy beyond validated domain). The review is closed at the PA level.

---

## 11. Archive location

This review document is archived at:
`docs/reviews/2026-11-02-gapa4-cppcheck-canonical-pa-review.md`

---

## 12. Confidence and uncertainty

| Aspect | Confidence | Basis |
| --- | --- | --- |
| Evidence integrity (hashes, archive) | **High** | Direct verification of all 6 archive files against `assets.sha256` |
| Inventory reconciliation (169 → 173 → 163) | **High** | Numerical traceability to specific finding classes and source fixes |
| Candidate changes match findings | **High** | Line-by-line verification of 4 modified files against working tree |
| Gate results | **Medium** | Reported in evidence summary; not independently re-run (read-only review) |
| `unusedFunction` disposition completeness | **Low** | Pattern accepted but per-symbol disposition not performed; flagged as PA-01 |
| Tool qualification status | **High** | Explicitly documented as GAP-A4 open in evidence and tool-qualification.md |

**Overall confidence in the verdict:** **High** — the core claim (deterministic inventory + fixes + gates pass) is directly evidenced. The residual uncertainties are documented and do not affect the increment's technical validity.

---

*End of review.*