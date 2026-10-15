# SB/EVS structural sweep run record — 2026-10-14

## Claim and limitations

Exploratory native-host characterization of configured SB/EVS structural
capacity. It does not establish a mission arrival envelope, sustained event
rate, WCET, target timing, link availability, ground receipt, requirement
compliance, or NC-EXT-75/76 closure. The authorized user/system lead directed
that this characterization be published without inventing mission parameters.
Mission-controlled inputs and post-acceptance loss/counter semantics remain
TBD as listed in `docs/architecture/sb-evs-characterization.md`.

## Exact execution and identity

- Repository HEAD before changes in this loop: `57974d297ab8a1e295258d1f3054bbbe2dd0f323`.
- Working tree was dirty; it is not represented as a clean immutable baseline.
- V&V container image ID: `sha256:5cbc72aa4125cfdbd8c0baa4ac2a1cb9d1447ff3147ac0b94e2ed0d918b83e91`.
- Container compiler: GCC 13.3.0.
- Configure/build: `cmake -S . -B build/sb-sweep-final -G Ninja -DCMAKE_BUILD_TYPE=Debug`
  and `cmake --build build/sb-sweep-final --target test_sb_sweep -j$(nproc)`;
  clean configure/build completed 56 Ninja steps.
- Sweep command: `python3 tools/scripts/sb_evs_structural_sweep.py build/sb-sweep-final/tests/test_sb_sweep --output docs/vv/evidence/2026-10-14-sb-evs-structural-sweep.csv`.
- Result: **9,984 fan-out/depth/overflow-burst profiles**, plus a sweep of all
  **64 dispatch bounds** (1–64); **1,697,329 assertion checks, zero failures**.
- CSV: 9,985 data rows (9,984 profiles + 1 dispatch-sweep record), plus one
  header line (9,986 lines total); SHA-256
  `83eafc861ec1fd029043b34f244504bd858ffc0a85fa35431eb6de27773e623a`.
- Test source SHA-256:
  `447d45d531880aeb46f72229e501c6823fc046969c972a5d93037b829bf241d9`.
- Runner source SHA-256:
  `376a6fc53af48e0d028b071193b3ddac82c83f308e68b0ba4832072746a167c`.
- Built host executable SHA-256:
  `1c05436de2bb6d1115baca60701c1fba379db168e40b9471ab2b2289ddf72d83`.

The image's Ubuntu base/package inputs are mutable; these hashes identify the
selected source/result artifacts, not a complete reproducible-build manifest.

## Local CI/CTest exercise added after the initial characterization

- CTest case `sb_evs_structural_sweep` runs the same full 9,984-profile matrix
  and dispatch limits 1–64 through `make ci-local`; the latest run passed in
  18.70 s (30/30 CTest total). The output is generated under `build/ci/`.
- CTest registration (`tests/CMakeLists.txt`) SHA-256:
  `44cc9d205f1cabe71539f2a73067f4eec9a4dc7f4d625f12d63d52f4150f3d57`.
- Runner SHA-256:
  `376a6fc53af48e0d0280b071193b3ddac82c83f308e68b0ba4832072746a167c`;
  test source SHA-256:
  `447d45d531880aeb46f72229e501c6823fc046969c972a5d93037b829bf241d9`.
- CTest executable `build/ci/tests/test_sb_sweep` SHA-256:
  `578182770c589b1d509a3e95990516e1202951fa1b418a2b8386780146f15f72`.
- Generated CTest CSV SHA-256:
  `83eafc861ec1fd029043b34f244504bd858ffc0a85fa35431eb6de27773e623a`
  (byte-identical to the retained characterization CSV); 9,986 lines.
- Raw CTest `LastTest.log` is retained at
  `docs/vv/evidence/2026-10-14-sb-evs-ctest-lasttest.log`, SHA-256
  `1f131c8a5b89a00ad6bbfe4a93a0751b0e6ed06c997116e297e958017408e874`.
- Hosted GitHub/Gitea CI was not run. The local CTest exercise is evidence for
  local pipeline execution only; it is not an immutable build manifest, formal
  tool qualification approval, or mission/requirement compliance evidence.

## Result summary

- Fan-out 1–32 and queue depth 1–32 were crossed with every overflow publish
  count from zero through the first EVS-notification saturation boundary.
- Smallest saturation-crossing overflow event count ranged from 65 (fan-out 1)
  to 96 (fan-out 32); EVS accepted 64 and accounted for 1–32 notification drops.
- Below saturation, the tested notification loss count was zero. An independent
  CSV self-check verified `notify_dropped = max(0, overflow_events - 64)` for
  all 9,984 profiles.
- For each limit 1–63, the batch sweep queued one event beyond the limit,
  confirmed exactly the limit was dispatched, then dispatched the one-event
  remainder. At limit 64, it filled and drained the 64-entry queue. This
  explicit dispatch-limit sweep record reports 2,207 callback dispatches. The
  `dispatch_calls` column sums to 341,151 across all 9,985 CSV data rows because
  each structural profile also dispatches its queued notifications.
- The CSV and C test assert per-profile copy/drop accounting, EVS queue
  acceptance/loss, log retention, expected message/app ordering, and explicit
  dispatch counts.

These are discrete, deterministic, per-process host profiles. No wall-clock
arrival intervals, concurrency/WCET measurements, target execution, network
loss/recovery, or ground receipt were exercised.

## Failed exploratory attempt retained

An earlier run used a CSV display calculation `overflow_events - 64` even when
the overflow count was below 64. Unsigned underflow made its reported
`notify_dropped` field invalid (for example, a zero-burst row). The C assertions
were not the source of that defect, but the dataset failed an independent CSV
consistency check and was discarded/overwritten. The formatter now reports the
asserted `expected_dropped` value; the final CSV self-check passes all rows.
The first malformed CSV was not retained as a separate file; the defect and
correction are recorded here.

The final runner reports compiled limits, rejects invalid configurations before
producing output, checks each profile against expected values, serializes to a
temporary CSV, re-reads the serialized output for row count/uniqueness/capacity/
arithmetic/status checks, and atomically replaces the retained CSV only after
validation. The final runner source also contains the required MPL-2.0 SPDX
header. An initial full CTest/Lean/trace cycle predates that comment-only header
correction; a second full `make ci-local` cycle passed after the correction.
The Python/license self-check, full structural sweep, format check, and local
diff check also passed after the correction. `--fanout-max 0` was rejected with
exit status 2 and no output file.

## Review status

- R3 FRICK-REVIEW-0270: bounded framework characterization accepted with
  comments; the CSV retention-location comment was resolved and rechecked.
  Mission event-performance semantics remain TBD.
- R2 FRICK-REVIEW-0271: corrected test/runner accepted for exploratory native
  structural characterization; initial dispatch-cap oracle and runner
  configuration/CSV-validation findings were corrected. Not mission/requirement
  compliance evidence.
- R2 FRICK-REVIEW-0274: final SPDX-header-only runner delta confirmed and the
  retained CSV reproduced byte-for-byte; bounded exploratory acceptance
  preserved, with formal tool-owner N10 acceptance still pending.
- R4 FRICK-REVIEW-0272: bounded documentation review found no blocking defect;
  final terminology/output-location comments were corrected and checked.
- PA FRICK-REVIEW-0273: scoped evidence consistency review found counts, hashes,
  CSV and limitations consistent. No NC closure or release approval.
- R2 FRICK-REVIEW-0275: full CTest runner wiring passed for local exploratory
  characterization, with a comment to preserve exact configuration/output
  identity and distinguish local from hosted CI.
- R4 FRICK-REVIEW-0276/0278: initial documentation findings R4-01..03 were
  corrected and closed in bounded delta review; no overall N17 approval.
- PA FRICK-REVIEW-0277/0279: initial evidence review approved with a runtime
  consistency comment; the 18.70 s discrepancy was corrected and closed in a
  bounded delta review; no qualification or release approval.

The local CI-exercise criterion and scoped review findings for T22 are recorded
as complete for configured-host characterization. This evidence does not
constitute an AI/human authorization to qualify the tool for requirements-
compliance use. Hosted CI was not run. NC-EXT-75/76, mission-profile authority
inputs and broader release gates remain open.
