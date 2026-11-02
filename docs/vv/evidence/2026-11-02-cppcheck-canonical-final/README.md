# GAP-A4 canonical cppcheck inventory — final (2026-11-02)

| Field | Value |
| --- | --- |
| Document id | FRICK-EVID-CPPCHECK-20261102-FINAL |
| Revision | 1.0 |
| Date | 2026-11-02 |
| Norms | N10 (tool identity/qualification), N11 (evidence), N6 (reproducibility) |
| Base HEAD | `1fdda25` (XW-2 post-correction commit + GAP-A4 src fixes) |
| Tool | `frick-vv:local` → `Cppcheck 2.13.0` |
| Image id | `sha256:80cff001630cebae324370c28037a441fc93739536fa184172dd8773b958e08b` |
| Invocation | `bash tools/scripts/cppcheck_scan.sh` (pinned; see `command.txt` and `cppcheck_scan.sh`) |
| Path set | `src`, `config` (same as 2026-11-01 GAP-A4 baseline; `tests/` excluded) |
| Report | `cppcheck_report_20261102_final.txt`, 163 findings |
| Determinism | two consecutive runs produced byte-identical reports (`4b34324e6443ce571f823bbb8ddb9dc80036a1083ffbc638741c94fb37ae9460`) |

## Inventory by class

| Id | Count | Disposition |
| --- | --- | --- |
| `unusedFunction` | 137 | Decision needed (API surface vs dead code). Whole-program analysis over `src`+`config` only; functions called exclusively from `tests/` appear unused. |
| `knownConditionTrueFalse` | 17 | 14 are constant-folding artefacts in `src/apps/app_renode_test.c`; 2 in `src/drivers/can/mpfs/ff_can_mpfs.c` (see §4); 1 in `src/core/time/time.c:256` (defensive check, keep). |
| `constVariablePointer` | 5 | Safe local `const` additions (no interface change): `sb.c` ×3 (fixed), `ff_gem.c` ×1, `ff_gem_tsn.c` ×1. |
| `constParameterCallback` | 3 | Deferred — changing callback parameter to `const` ripples into function-pointer types. |
| `badBitmaskCheck` | 1 | `src/apps/app_renode_test.c:222` — constant-folded, not a defect. |

## Reconciliation with 2026-11-01 post-fix inventory

| Item | Value |
| --- | --- |
| 2026-11-01 post-fix findings | 169 |
| Reproduced exactly by pinned invocation | 169 / 169 (100%) |
| Additional findings under pinned invocation | 4 |
| Total under pinned invocation (before src fixes) | 173 |
| Total after GAP-A4 src fixes (this report) | 163 |

The 4 additional findings from the 2026-11-01 baseline were all genuine `const`-correctness findings. The GAP-A4 src fixes addressed 10 of the 173 findings (constVariable 6→0, constVariablePointer 8→5, unusedVariable 1→0).

## Authority note

This is advisory static analysis performed with an **unqualified** open-source checker. It is not MISRA compliance evidence, not a proof of absence of defects, and not a qualification record (GAP-A4 remains open).
