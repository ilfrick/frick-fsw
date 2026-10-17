# OBCP candidate integration assessment (S9)

| Field | Value |
| ----- | ----- |
| Document ID | FRICK-ARCH-OBCP-001 |
| Revision | 0.4 |
| Date | 2026-10-16 |
| Status | Adapter integrated in default build; assurance and mission use remain open |
| Requirements | SRS-REQ-045, SRS-REQ-054, SSS-REQ-022, SSS-REQ-027 |

## 1. Scope and conclusion

This assessment records integration of the M12 first candidate,
[`n7space/n7s-obcp`](https://github.com/n7space/n7s-obcp), into the default
frick-fsw build. The fetched parent revision is
`55d6d97e7e7ff08a130698f7a922bcf58acb3d99`; its MicroPython submodule is pinned
to `647c8b96cae7e202c7a020395b7cfe65e5b8ce04`. These identities are recorded in
`third_party/README.md` and checked by the `obcp-only` mode of
`tools/scripts/fetch_third_party.sh`.

**Integration status:** the pinned engine is compiled and linked into
`frick_fsw_core` by default (`FF_USE_N7_OBCP=ON`); `FF_ObcpInit`,
`FF_ObcpExecutePy`, `FF_ObcpRequestAbort`, and `FF_ObcpGetStatus` form the
project adapter. The connectivity test executes a Python script through the
engine and checks integer/enum/float/Boolean datapool, event, output,
step-control, relative/absolute time, and packet callback paths. It uses test
doubles, not production mission-service bindings. The test checks callback side
effects after execution rather than trusting the engine return as an interpreter
success oracle.
`FF_USE_N7_OBCP=OFF` is an explicit build escape hatch, not the project default.
“Default” here means the dependency and adapter are compiled into the default
core build; `fsw_main` does not yet initialize the adapter or register a runtime
OBCP application because production mission-service and command-policy bindings
are not established.

The engine itself has not been revalidated. The upstream control module exposes
begin/end-step callbacks intended to permit cooperative blocking or abort at
script-defined step boundaries. This does not impose a VM-level execution
quota inside arbitrary Python code between callbacks. In addition,
`obcpengine_execute_py` returns `true` after the interpreter call without
propagating interpreter failure. The adapter exposes the upstream Boolean as
`engine_reported_success`; it does not claim a script-level success oracle.
Abort requests are cooperative and only observed at a subsequent script-defined
begin-step callback. An infinite loop between callbacks can prevent the call
from returning. The adapter serializes access to one engine context and does
not enable upstream concurrent-OBCP mode. The heap buffer and script-length
limit are caller-provided/static integration limits, not evidence of total
memory/stack accounting, WCET or processor margin. Code-size impact is also not
budgeted. NC-S9-01 remains deferred/open for these limitations and missing
target/service assurance; this does not block unrelated S9-S12 work. The
fallback SRS-REQ-054 is a distinct M16 product, not evidence that M12 has
completed.

## 2. Observed upstream interface

At the pinned revision, `src/obcp_engine/obcpengine.h` exposes engine context and
functions for initialization, buffers, and Python/bytecode execution. The
implementation in `src/obcp_engine/obcpengine.c` invokes
`mp_embed_exec_str(script)` and returns `true` following that call. The
`obcpcontrol` interface includes `obcp_beginstep` and `obcp_endstep` callbacks;
upstream comments describe begin-step as able to "block or abort if necessary."
These are procedure-defined cooperative points, not preemption of an arbitrary
VM instruction stream. Inspection of the corresponding MicroPython VM loop and
upstream configuration did not find an engine-enforced instruction quota. This
is a source/API observation, not a proof that no upstream branch or future patch
could provide such a mechanism.

The README describes an embedded engine, callback-provided application services,
and a concurrent mode with an execution task per OBCP. A separate RTEMS task
boundary alone does not prove bounded termination: forcibly deleting a task
while interpreter/heap state is active can leave unsafe shared/global runtime
state and does not establish cleanup or restart integrity.

## 3. Evidence retained and limits

- `docs/vv/evidence/2026-10-16-s9-n7s-obcp-upstream-host-tests.log` retains an
  initial failed run caused by the upstream Makefile leaving an incomplete
  ignored clone directory after an interrupted generation/build attempt.
- `docs/vv/evidence/2026-10-16-s9-n7s-obcp-upstream-host-tests-retry.log`
  retains the clean retry. It builds the upstream native test harness and
  reports the included flat datapool test passing. This is not a test of
  frick-fsw bindings, RTEMS/RISC-V execution, PUS S18, target timing, abort
  latency, memory bounds, or requirement closure.
- `docs/vv/evidence/2026-10-16-s9-obcp-fetch.log` records a subsequent clean
  `obcp-only` identity/state check after generated outputs were removed. The
  strict fetch check intentionally rejects ignored build artifacts; the
  upstream test Makefile writes such artifacts inside the checkout, so clean
  them before another fetch or general dependency update.
- A separate explicit `FF_USE_PUS_ASN1=OFF` host configuration/build/test cycle
  passed 41/41 CTest tests (`docs/vv/evidence/2026-10-16-s8-pus-fu-open-backend-explicit-check.log`);
  that result verifies the alternate PUS backend path only and is unrelated to
  OBCP execution assurance.

The upstream Makefile and test selection are not treated as qualified project
V&V tools. The fetched source is ignored third-party material and is not a
committed or modified code artifact.

## 4. Required decision and exit evidence

Integration should proceed with the existing candidate, while the system/OBCP
authority and FSW owner disposition the residual arbitrary-code execution
bound. The choices for that separate residual are:

1. A controlled upstream/runtime patch that supplies an execution quota and
   safe abort/cleanup for arbitrary code; include license/provenance and
   independent review; or
2. Retain the limitation for mission-specific restricted procedures that use
   approved cooperative step boundaries, if the authorized system/OBCP
   authority accepts that operational envelope; or
3. The separately governed in-house fallback, with its own approved scope and
   evidence under M16/SRS-REQ-054.

For any future closure, required evidence includes positive, malformed, failed-script,
resource-limit, execution-budget-expiry, and cancellation cases with explicit
expected results; memory/stack/accounting bounds; deterministic state reporting;
production frick-fsw service bindings; and representative RTEMS/RISC-V
integration. A host timeout wrapper that merely stops waiting is not an abort
test. No mission OBCP may be enabled on the basis of this adapter connectivity
test.

## 5. Integration connectivity evidence

`tests/unit/test_obcp_integration.c` is a frick-fsw-owned integration test. It
initializes the pinned engine through the adapter and executes a bounded
connectivity script against test callback implementations. Explicit results
check parameter write, event ID, text output, begin/end step, wait duration,
packet availability/transmit/receive, and adapter script-length rejection.
This is adapter/engine connectivity evidence only; it does not re-run the N7
engine's own test suite or establish production binding correctness, exception
handling, WCET, heap/stack bounds, arbitrary-script termination, target
portability, or flight suitability. Build evidence is retained in
`docs/vv/verification-results.md`.

## 6. Change history

| Revision | Date | Author | Change |
| -------- | ---- | ------ | ------ |
| 0.1 | 2026-10-16 | frick-fsw | Record pinned candidate identity, limited upstream test, and initial suitability assessment |
| 0.2 | 2026-10-16 | frick-fsw | Correct the candidate assessment to include cooperative begin/end-step callbacks; distinguish step-boundary abort from arbitrary VM instruction/time quotas and clarify that M12 integration of the existing engine remains the requested work |
| 0.3 | 2026-10-16 | frick-fsw | Integrate the pinned engine into the default build, add FF_Obcp adapter and connectivity test, document callback/evidence boundaries and defer NC-S9-01 pending later assurance/authority disposition |
| 0.4 | 2026-10-16 | frick-fsw | Record single-context serialization and unbudgeted code-size/heap/stack/WCET limitations explicitly |
