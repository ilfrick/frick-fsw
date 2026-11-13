# PA Review Report — PUS verification payload zero-initialization

**Product / config:** frick-fsw — cFS-inspired FSW framework, PUS-C TM/TC on CCSDS Space Packets,
target Microchip RT PolarFire SoC RTPFS160ZT (RISC-V) under RTEMS 6.2, target ECSS criticality B.
This change is **host-executable only** (native simulator/host build); not target/RTEMS/MPFS/RTPFS160ZT related.

**Repository / branch / HEAD:** /home/nicola/fsw/frick-fsw, `main`, `b6bf060` (working tree has uncommitted
change to `src/pus/pus.c`; nothing staged).

**Reviewer:** Senior Aeronautics and Space Software Product Assurance Engineer (independent N20 review;
subagent persona `aeronautics-space-sw-pa-engineer`). This review is advisory evidence only; it does not
certify compliance, accept residual risk, or authorize release/commit. Commit/push authority is reserved
to `aeronautics-space-fsw-engineer` under norm N7 after all applicable gates/norms and full sprint scope
pass.

**Review date:** 2026-11-12

**Applicable baseline:** Project norms N1-N22 (docs/rules.md, AGENTS.md); ECSS-E-ST-40C / ECSS-Q-ST-80C
context as invoked by the project; ECSS-E-ST-70-41C (PUS-C) referenced in code comments. Exact invoked
editions and tailoring are governed by the approved contract/baseline; this review does not alter them.

---

## 1. Change under review

| Item | Detail |
|---|---|
| File | `src/pus/pus.c` |
| Line | 301 (function `FF_PusSendVerification`) |
| Change | `uint8_t payload[7];` → `uint8_t payload[7] = {0};` |
| Intended effect (per context) | `payload[5]` was uninitialized; copied via `memcpy(extended, payload, 6)` into `extended[2]` sent over the wire as PUS Service 1 verification TM; high byte of `extended[2]` held uninitialized stack data. Fix zero-initializes all 7 bytes so `extended[2]`'s high byte is deterministically 0. |
| Scope per context | Host-executable software fix. NOT hardware/target. NOT GAP-A5 / NC-PAL-01 / OBCP engine related. NC-EXT-19/90 and N22-TS5-47 NOT touched. |

---

## 2. Gate evidence (independently re-verified in this review)

All gates executed in a fresh isolated build directory (`build/native-pa`) because the pre-existing
`build/native` CMake cache referenced `/workspace` (environment path mismatch, not a code defect) and
could not be removed due to filesystem permissions.

| Gate | Command | Result | Evidence |
|---|---|---|---|
| Build | `cmake -S . -B build/native-pa ... && cmake --build build/native-pa` | PASS, exit 0 | 0 compiler warnings |
| CTest | `cd build/native-pa && ctest` | PASS, exit 0 | all tests pass; `pus` label includes `test_verification_missing_subservices` (covers `FF_PusSendVerification` for all stages: acceptance/start/progress/completion, success/fail) |
| Lean | `make verify-lean` (`lake build` in `verification/lean`) | PASS, exit 0 | Build completed successfully |
| Trace | `make trace` | PASS | `traceability: PASS (120 requirements, 76 srs, 30 sss, 14 gaps, 68 lean theorems checked)` |
| Format | `FF_CLANG_FORMAT_ALLOW_UNPINNED=1 make format-check` | PASS, exit 0 | pinned clang-format major 18 rule; env ships clang-format 23, override used; `.clang-format` dry-run --Werror clean |
| License | `python3 tests/python/test_license_check.py` + `python3 tools/scripts/license_check.py` | PASS | 14 tests OK; MPL-2.0 SPDX check OK |

No new test scripts added; no T3 impact; no N10 re-qualification trigger (per context; consistent with
tree — `tests/unit/test_pus.c` unchanged by this change).

---

## 3. Technical verification

### 3.1 Fix correctness — uninitialized memory (Task 1)

Code inspection of `FF_PusSendVerification` (`src/pus/pus.c:298-354`):

- `src/pus/pus.c:301`: `uint8_t payload[7] = {0};` — all 7 bytes zero-initialized (post-fix).
- `src/pus/pus.c:335-343`: `payload[0]..payload[6]` are **all explicitly assigned** before use:
  - payload[0] = source_id >> 8; payload[1] = source_id & 0xFF
  - payload[2] = 0x00; payload[3] = 0x01
  - payload[4] = s_sequence_count >> 8; payload[5] = s_sequence_count & 0xFF
  - payload[6] = tc->service
- `src/pus/pus.c:348-349`: `uint8_t extended[9]; memcpy(extended, payload, 6);` — copies payload[0..5],
  all of which are assigned.
- `src/pus/pus.c:350-353`: `extended[6]=service; extended[7]=subservice; extended[8]=code;`
  `FF_PusSendTm(FF_PUS_SERVICE_VERIFICATION, subservice, extended, sizeof(extended));`

**Assessment:** The fix correctly and deterministically zero-initializes `payload[7]`. It is sound
initialization hygiene and removes any exposure to uninitialized stack bytes.

**PA-001 (Minor) — root-cause narrative does not match the code as shipped.** The context describes
`payload[5]` as uninitialized and leaking into `extended[2]`. Inspection shows `payload[5]` **is**
assigned (`src/pus/pus.c:342`) and `extended[2] = payload[2] = 0x00` (`src/pus/pus.c:339`) — also
assigned. Therefore no live uninitialized-memory read exists in the current `FF_PusSendVerification`
code path; the build emits 0 warnings and no analyzer diagnostic was triggered. The change is
**defensive/hygienic**: it guarantees determinism and protects against future edits that might leave a
byte unassigned, but it does not alter observable behavior of the current code.

- Classification: **Minor**
- File:line: `src/pus/pus.c:301`; related `src/pus/pus.c:335-349`
- Recommended disposition: Accept the fix. Correct the commit message / change record to describe the
  change as deterministic zero-initialization hygiene (no live uninitialized read in the shipped path),
  rather than a fix for a live uninitialized-memory defect.
- Blocks commit/push: **No**

### 3.2 Safety / security / flight / hardware impact (Task 2)

- Host-executable only; telemetry TM field (PUS Service 1 verification TM).
- Change makes one TM field more deterministic; no control-loop, timing, memory-protection, fault-detection,
  or hardware-register effect.
- Not target/RTEMS/MPFS/RTPFS160ZT; no watchdog, PAL, link, or OBCP-engine involvement.
- No safety objective, security control, reliability assumption, or flight-hazard control is added,
  removed, or weakened.

**Finding:** No safety/security/flight/hardware impact. Behavior becomes more deterministic; risk effect is
beneficial or neutral.

- Blocks commit/push: **No**

### 3.3 NC / GAP / norm cross-reference (Task 3)

Repository-wide grep for `NC-PAL-01`, `GAP-A5`, `NC-EXT-19`, `NC-EXT-90`, `N22-TS5-47` (and `NC-EXT-19/90`):

- Matches occur only in `src/core/pal/*`, `docs/status/handover.md`, `docs/status/loop-audit.md`,
  `docs/reviews/*`, and third-party/build artifacts — i.e., PAL/target-binding, schedule/timing, and
  documentation-tracking areas.
- **Zero matches** in `src/pus/pus.c` or `tests/unit/test_pus.c`.

**Finding:** NC-PAL-01, GAP-A5, NC-EXT-19/90, and N22-TS5-47 are **not touched and not closed** by this
change. They remain open per the NC register/handover; no closure is implied.

- Blocks commit/push: **No**

### 3.4 Host gates (Task 4)

All green per Section 2 (build 0 warnings; CTest exit 0 incl. `pus` test; Lean PASS; trace PASS
120/76/30/14/68; format PASS; license OK).

- Blocks commit/push: **No** (gate evidence supports proceeding; commit/push authority remains with
  `aeronautics-space-fsw-engineer` under N7 after full sprint scope).

### 3.5 Test-coverage observation (Task 5 — additional)

`tests/unit/test_pus.c:413-480` (`test_verification_missing_subservices`) calls `FF_PusSendVerification`
for every stage/success combination and asserts `FF_SUCCESS`, but it does **not** capture or assert the
deterministic wire-byte content of `payload`/`extended` (e.g., `payload[5] == 0`, `extended[2] == 0`).
Because the fix changes no observable behavior of the current code, this is not a defect; it is a
coverage limitation for the determinism property the fix exists to guarantee.

- PA-002 (Minor) — test asserts function success, not deterministic byte content.
- File:line: `tests/unit/test_pus.c:413-480`
- Recommended disposition: Accept existing coverage for this cycle (consistent with N21/N22: do not
  expand a functional fix into a perfection review loop). Optional future improvement: add a byte-content
  assertion capturing the TM frame to pin `extended[2] == 0` / `payload[5] == 0`.
- Blocks commit/push: **No**

---

## 4. Findings summary

| ID | Classification | File:line | Summary | Disposition | Blocks commit/push |
|---|---|---|---|---|---|
| PA-001 | Minor | `src/pus/pus.c:301` (ctx narrative) | Documented root cause (payload[5] uninitialized → extended[2]) not borne out by code; all payload bytes assigned at lines 335-343 before memcpy at line 349. Fix is correct defensive initialization. | Accept fix; correct commit message/change record to describe deterministic-initialization hygiene. No re-test beyond existing pus test. | **No** |
| PA-002 | Minor | `tests/unit/test_pus.c:413-480` | Test asserts FF_SUCCESS only; does not assert deterministic wire bytes (payload[5], extended[2]). | Accept existing coverage; optional future byte-content assertion. | **No** |

**Major findings: 0. Minor findings: 2. Neither blocks commit/push.**

---

## 5. Conclusion and recommendation

The change `src/pus/pus.c:301` (`uint8_t payload[7] = {0};`) is a correct, minimal, defensive
zero-initialization of the PUS Service 1 verification payload. All host assurance gates are green
(build 0 warnings; CTest 56/56; Lean PASS; trace PASS 120/76/30/14/68; format PASS; license OK). It has
no safety/security/flight/hardware impact and touches none of NC-PAL-01, GAP-A5, NC-EXT-19/90, or
N22-TS5-47 (none closed).

**Recommendation to the human release/commit authority:** proceed with commit/push per norm N7 (only
`aeronautics-space-fsw-engineer` may commit/push, and only after all applicable gates/norms and full
sprint scope pass), with PA-001 addressed in the commit message/change record and PA-002 noted as an
optional improvement. This PA review does not approve release, qualify tools, accept residual risk, or
substitute for the authorized human decision.

---

## 6. Uncertainty and confidence (per PA assurance standard)

Decision/product and configuration: frick-fsw `main` @ b6bf060, change `src/pus/pus.c:301`, host build.
Claim types: Fact (code inspection, gate re-execution), Inference (impact characterization),
Recommendation (PA-001/PA-002 dispositions), Open issue (none blocking).
Applicable baseline and approved tailoring: project norms N1-N22; ECSS context as invoked by project
(exact editions/tailoring governed by contract — not altered here).
Evidence consulted: `src/pus/pus.c:298-354`; `tests/unit/test_pus.c:413-480`; gate re-execution logs
(fresh `build/native-pa`); repository grep for NC/GAP identifiers.
Cross-check and reconciliation: gate re-run in fresh dir bypassed stale `/workspace` CMake cache;
format used `FF_CLANG_FORMAT_ALLOW_UNPINNED=1` because env ships clang-format 23 vs pinned 18 (override
records the pinned-major intent).
Confidence: **High** for gate results and "NCs not touched"; **High** that the fix zero-initializes
payload; **Medium-High** on the impact characterization (based on provided context plus code inspection;
flight/hardware scope asserted by change author, not independently re-derived from target evidence).
Missing/inaccessible evidence: none material; target/HIL evidence is out of scope for this host-only
change (GAP-A5 remains open per register, not closed here).
Assumptions: the change author's scope assertion (host-only, not GAP-A5/NC-PAL-01/OBCP) is accepted as
input; this review does not re-derive target/fidelity claims. Expiry/recheck: re-review if the change
is retargeted at RTEMS/target or if NC-PAL-01/GAP-A5/NC-EXT-19/90/N22-TS5-47 are dispositioned.
Consequences if wrong: minimal for this host-only TM-field change; a wrong scope assertion would be
caught at target/HIL and release gates, not by this fix alone.
Required independent human reviewer/authority: `aeronautics-space-fsw-engineer` (commit/push, N7);
release/safety/security acceptance remains with the authorized human authority; this PA engineer does
not approve risk/release.

---

*End of PA review report. Retain in working tree per norm N11; do not commit/push (authority reserved
under N7).*
