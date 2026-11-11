# GAP-A4 canonical cppcheck inventory — 2026-11-02

| Field | Value |
| --- | --- |
| Document id | FRICK-EVID-CPPCHECK-20261102 |
| Revision | 1.0 |
| Date | 2026-11-02 |
| Norms | N10 (tool identity/qualification), N11 (evidence), N6 (reproducibility) |
| Base HEAD | `65e4a91` (working tree also carries the uncommitted XW-2 increment; no `src/` file in that increment is scanned here) |
| Tool | `frick-vv:local` → `Cppcheck 2.13.0` |
| Image id | `sha256:80cff001630cebae324370c28037a441fc93739536fa184172dd8773b958e08b` |
| Invocation | `bash tools/scripts/cppcheck_scan.sh` (pinned; see `command.txt` and `cppcheck_scan.sh`) |
| Path set | `src`, `config` (the same path set as the 2026-11-01 GAP-A4 baseline; `tests/` excluded) |
| Report | `cppcheck_report_20261102.txt`, 173 findings |
| Determinism | two consecutive runs produced byte-identical reports (`4b34324e6443ce571f823bbb8ddb9dc80036a1083ffbc638741c94fb37ae9460`) |

> **Authority note.** This is advisory static analysis performed with an
> **unqualified** open-source checker. It is not MISRA compliance evidence, not a
> proof of absence of defects, and not a qualification record (GAP-A4 remains
> open).

## 1. Why this artifact exists

The 2026-11-01 GAP-A4 inventory (`docs/vv/evidence/cppcheck_report_2026-11-01.txt`,
180 findings) and its post-fix re-scan
(`docs/vv/evidence/2026-11-01-cppcheck-fixes/cppcheck_postfix_2026-11-01.txt`,
169 findings) were archived **without the command line that produced them**. The
option set, include roots and path set are therefore not recoverable from the
retained artifacts, and the 180/169 figures cannot be re-derived from the record
alone. That is a reproducibility defect in the evidence chain (N10/N11), not in
the findings themselves.

`tools/scripts/cppcheck_scan.sh` now pins the invocation and fails closed
(exit 2) when the checker image is unavailable, so a future scan is attributable
and byte-comparable. This document records the first scan under that pinned
invocation and the reconciliation against the historical inventory.

## 2. Reconciliation with the 2026-11-01 post-fix inventory

Comparison is at `file:line:id` granularity, after normalising the container
prefix `/workspace/`.

| Item | Value |
| --- | --- |
| 2026-11-01 post-fix findings | 169 |
| Findings of that inventory reproduced exactly by the pinned invocation | **169 / 169 (100 %)** |
| Additional findings under the pinned invocation | **4** |
| Total under the pinned invocation | 173 |

The four additional findings are all genuine `const`-correctness / constant-folding
findings that the historical report did not list:

| Location | Id | Note |
| --- | --- | --- |
| `src/core/sb/sb.c:155` | `constVariablePointer` | `pipe` read-only in the dispatch loop |
| `src/core/sb/sb.c:206` | `constVariablePointer` | `FF_SbTestQueueLock`, test hook |
| `src/core/sb/sb.c:212` | `constVariablePointer` | `FF_SbTestQueueUnlock`, test hook |
| `src/core/time/time.c:256` | `knownConditionTrueFalse` | defensive check on `FF_TimeGetCuc()` |

The cause of the delta is **not determined** (the historical scan's preprocessor
configuration cannot be reconstructed). It is recorded as an open question rather
than explained away. Because the historical figure cannot be reproduced, this
173-finding inventory supersedes it as the GAP-A4 baseline; the 180/169 numbers
remain citable only as historical statements about a non-reproducible run.

## 3. Inventory by class

| Id | Count | Disposition class |
| --- | --- | --- |
| `unusedFunction` | 137 | **Decision needed (API surface vs dead code).** Whole-program analysis over `src`+`config` only; functions called exclusively from `tests/` appear unused. Requires an explicit exclusion policy or per-symbol justification before it can be treated as a defect list. |
| `knownConditionTrueFalse` | 17 | 14 are constant-folding artefacts in `src/apps/app_renode_test.c` (all `renode_hal_*_check()` collapse to constants under cppcheck's macro set — configuration false positives); 2 in `src/drivers/can/mpfs/ff_can_mpfs.c` (see §4); 1 in `src/core/time/time.c:256` (defensive check, keep). |
| `constVariablePointer` | 8 | Safe local `const` additions (no interface change): `sb.c` ×3, `ff_gem.c` ×1, `ff_gem_tsn.c` ×1, `pus_services.c` ×1, `ff_sdls.c` ×2. |
| `constVariable` | 6 | Safe `const` on local arrays: `src/apps/app_req_test.c` ×4, `src/drivers/spacewire/mpfs/ff_spacewire_mpfs.c` ×2. |
| `constParameterCallback` | 3 | **Deferred.** Changing a callback parameter to `const` ripples into function-pointer types (`ff_net_loopback.c`, `ff_net_lwip.c`, `ff_spacewire_mpfs.c`) and is an interface decision, not a local fix. |
| `unusedVariable` | 1 | `src/drivers/spacewire/mpfs/ff_spacewire_mpfs.c:247` `rx_len` — genuine dead variable. |
| `badBitmaskCheck` | 1 | `src/apps/app_renode_test.c:222` — constant-folded (`want[1] == 0`), not a defect. |

## 4. Findings examined and deliberately *not* "fixed"

`src/drivers/can/mpfs/ff_can_mpfs.c`:

- `:76` `presc == 0u` — **provably unreachable** given the guard at `:71`
  (`bitrate > CLK/tq` ⇒ `bitrate*tq <= CLK` ⇒ rounded `presc >= 1`). Removal is
  semantically null; retained pending the increment that records the proof.
- `:79` `presc > 0x7FFFu` — reported as always false, but it is **reachable**
  (`presc` can reach `80 MHz / 16 = 5 000 000` for a 1 bit/s request). Deleting
  it would silently truncate a 15-bit register field. The correct disposition is
  a justified suppression under `docs/quality/misra-c.md` §4, **not** removal.

This is the reason the inventory is not being "driven to zero" mechanically: two
of the 17 `knownConditionTrueFalse` findings sit next to a real input-validation
boundary, and treating the checker as an oracle would introduce a defect.

## 5. What this artifact does not establish

- No MISRA C:2012 compliance (needs `--addon=misra` and a qualified checker).
- No absence-of-defect claim: an unqualified checker with 137 unresolved
  `unusedFunction` results establishes neither dead code nor live code.
- No WCET, stack, partitioning or multicore evidence.
- No qualification of cppcheck 2.13.0 or of `frick-vv:local`. GAP-A4 remains
  open: `docs/vv/tool-qualification.md` T14 qualifies only the **MISRA addon**
  scan (`tools/scripts/misra.sh`, advisory) and explicitly leaves a certified
  checker as GAP-A4. T14 does **not** cover this inventory invocation.

> **Registration status (N10).** `tools/scripts/cppcheck_scan.sh` is a new
> evidence-producing script and is therefore **not yet registered** in
> `docs/vv/tool-qualification.md`. Until that entry exists, this report is
> advisory working evidence only and must not be cited as qualified tool output.
> The registration entry is a pending action that was deliberately not applied
> while `tool-qualification.md` sits inside an open independent PA review (an
> edit would invalidate that review instead of delta-reviewing it).

## 6. Reproduction

```
bash tools/scripts/cppcheck_scan.sh --out build/cppcheck-report.txt
```

Exit 0 with a report; exit 2 means the checker could not be produced and no
clean result may be claimed. `--include-tests` widens the path set to `tests/`
(produces a different, larger inventory; not comparable with §2).
