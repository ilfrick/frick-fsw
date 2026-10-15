# SB/EVS structural sweep — R2 source-identity delta review

**Review ID:** FRICK-REVIEW-0274  
**Date:** 2026-10-14  
**Profile:** Scoped R2 software V&V source-identity delta review  
**Scope:** Final runner header-only change after FRICK-REVIEW-0271; current test/runner/CSV identities; repeat of the bounded host sweep.  
**Disposition:** ACCEPTABLE for the already-bounded exploratory native-host structural characterization only. This is not formal tool-owner N10 acceptance or qualification.

## Identity and rerun

Current SHA-256 values independently recomputed from the working tree:

| Artifact | SHA-256 |
|---|---|
| `tests/unit/test_sb_sweep.c` | `447d45d531880aeb46f72229e501c6823fc046969c972a5d93037b829bf241d9` |
| `tools/scripts/sb_evs_structural_sweep.py` | `376a6fc53af48e0d028b071193b3ddac82c83f308e68b0ba4832072746a167c` |
| `docs/vv/evidence/2026-10-14-sb-evs-structural-sweep.csv` | `83eafc861ec1fd029043b34f244504bd858ffc0a85fa35431eb6de27773e623a` |

The runner begins with the Python shebang followed by the project copyright
notice and `SPDX-License-Identifier: MPL-2.0`; these are comments. Consistent
with the stated delta description, no executable statement or test source
change is implicated. The exact earlier runner bytes were not available here
for a bytewise old/new comparison; the review therefore relies on the bounded
change description plus current source inspection and execution, not an
independently reconstructed historical diff.

Executed the current runner against `build/sb-sweep-final/tests/test_sb_sweep`
to a separate temporary output. The rerun reported 9,984 fan-out/depth/burst
profiles, dispatch bounds 1–64, and PASS. The temporary CSV SHA-256 matched the
retained CSV SHA-256 above; `cmp` confirmed byte identity. The recorded result
is 1,697,329 assertions, zero failures, 9,985 data records including the
dispatch record, and 2,207 callback dispatches in that explicit dispatch-sweep
record. The test source hash is unchanged from the run record. This check
confirms source/result identity for this bounded run, not immutable build
provenance; the execution image inputs are mutable as already disclosed.

## Logic conclusions retained

- Fan-out and queue depth are crossed over 1–32, with zero overflow publishes
  through the first integral burst that exceeds EVS notification capacity.
- Assertions check SB publish/delivery/drop accounting, EVS accepted and dropped
  notifications, log retention/content/order, no inline listener delivery,
  and pending-event dispatch counts.
- For each dispatch limit 1–63, the test queues one extra notification, checks
  that exactly the requested limit dispatches, then checks the remainder can
  dispatch. At limit 64 it fills and drains the configured queue. This closes
  the specific prior dispatch-cap oracle gap within the tested host fixture.
- The runner obtains configured bounds from the executable, rejects invalid or
  inconsistent settings before output, compares profile data with expected
  arithmetic, validates the serialized CSV, and atomically replaces output
  only after validation. The reproduced CSV passes this bounded validation.

## Claim boundary

The final header-only runner source delta does not change executable behavior,
the test oracle, or the retained sweep data. It therefore preserves FRICK-
REVIEW-0271's acceptance for exploratory native-host structural
characterization. This does **not** qualify T22 as evidence for requirements or
mission compliance, nor establish arrival/service envelope, sustained event
rate, WCET, target timing, concurrency, network/link behavior, ground receipt,
or acceptance of notification loss. Mission inputs and NC-EXT-75/76 remain
open.

This review is not formal tool-owner N10 acceptance, project/authority
qualification or acceptance, mission/compliance qualification, NC closure, or
release approval. Those decisions remain with the designated human authority.
