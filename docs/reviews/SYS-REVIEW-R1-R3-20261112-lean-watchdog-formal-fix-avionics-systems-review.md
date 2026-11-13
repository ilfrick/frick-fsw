# Avionics Systems Independent Review — R1/R3 Profile

**frick-fsw — Microchip RT PolarFire SoC RTPFS160ZT (RISC-V) / RTEMS 6.2 — ECSS class B**

---

## Review header

| Field | Value |
|---|---|
| Reviewer role | Senior Aeronautics and Space Avionics Systems Engineer — independent review (R1/R3 profile) |
| Repository | /home/nicola/fsw/frick-fsw, branch `main` |
| Tree reviewed | HEAD `ad2a4aa` (contains fix commit `8c84c7f` as an ancestor; context reference HEAD `1c4b323` is an earlier point on the same line) |
| Change under review | Lean formal verification fix — NC-EXT-120 correction (commit `8c84c7f`) |
| Review date | Thu Nov 12 2026 |
| Reported gate status (this change) | build exit 0 / 0 warnings; CTest exit 0; Lean `lake build` OK; `make trace` PASS (120/76/30/14/68); format-check PASS; license OK |
| Authority | AI engineering assistant. No licensure, delegated design authority, certification privileges, or flight experience. Independent qualified humans retain all safety-critical, regulatory, certification, human-rating, and release decisions. |

---

## 1. Scope and method

Reviewed from an avionics/system-level perspective:

1. Whether the forward Lean theorem fix introduces any system-level risk.
2. Whether NC-PAL-01, GAP-A5, or any target/hardware item is affected.
3. Whether GNC, safety, FDIR, or operations interfaces are impacted.
4. Cross-disciplinary consistency: Lean formal verification coverage of the watchdog FDIR requirements SRS-REQ-073/074/075.
5. Any systems findings that block commit/push.

Evidence examined: `verification/lean/Frick/Watchdog.lean`, `verification/lean/Frick/Requirements.lean`, `verification/lean/lake-build.log`, `docs/requirements/traceability.yaml`, and the NC register in `docs/planning/sprint-plan.md` (NC-EXT-120 and the NC-EXT-121..129 / 166..169 family). Gates were reported as passed by the FSW engineer and were not independently re-executed by this review.

---

## 2. Change under review

Forward Lean theorem fix (commit `8c84c7f`, ancestor of HEAD). It is a **Lean/formal-verification fix only**: no C source, no platform binding, no hardware register, no flight parameter, no ICD, and no safety claim are changed.

- `verification/lean/Frick/Requirements.lean:21` — added `import Frick.Watchdog`, making the module import-reachable so `lake build` elaborates it.
- `verification/lean/Frick/Watchdog.lean` — new file, 117 lines, six theorems discharging SRS-REQ-073/074/075:
  - `:52` `req_srs073_not_armed_never_expires`
  - `:58` `req_srs073_feed_clears_expiry`
  - `:76` `req_srs073_feed_within_period_not_expired`
  - `:106` `req_srs074_report_precedes_reset` — core Lean 4.10 `List` indexing, `recoverySequence[0]? = some .report ∧ recoverySequence[1]? = some .reset`
  - `:110` `req_srs075_single_reset_path` — `rfl` alias lemma (known weaker assurance content, tracked as NC-EXT-125)
  - `:114` `req_srs075_one_reset_action` — `(recoverySequence.filter (· == .reset)).length = 1`
- `verification/lean/lake-build.log` — `:18` `✔ [8/25] Built Frick.Watchdog`; `:27` `✔ [17/25] Built Frick.Watchdog:c.o`; `:31-32` `✔ [21/25] Built Frick.Requirements` / `:c.o`; `:36` `Build completed successfully.`
- All six theorem names unchanged → `trace.py` name-match contract preserved; `make trace` PASS (68 Lean theorems).

Previously (NC-EXT-120): `Frick.Watchdog` was not import-reachable (`Frick/Requirements.lean` did not import it; `lake build` builds `Main` which imports only `Frick.Requirements`/`Frick.Ccsds`); the tracked `lake-build.log` enumerated 21 steps with no `Frick.Watchdog`; no `Watchdog.olean` was produced; once reachable, the module failed to compile because `List.idxOf`/`List.count` do not exist in Lean 4.10 core `List`. The six SRS-REQ-073/074/075 theorems were asserted but never elaborated, so the N1/N3 gate was not actually satisfied for this increment.

---

## 3. Findings

| ID | Class | File:line | Blocks commit/push? | Finding |
|---|---|---|---|---|
| **SYS-R1/R3-001** | Minor | `verification/lean/Frick/Requirements.lean:21`; `Watchdog.lean:52-114`; `lake-build.log:18,27,31-32,36` | **No** | The forward Lean fix introduces **no system-level risk**. It is a host-side formal-model change; no C code, platform binding, runtime behavior, or flight parameter is altered. All six SRS-REQ-073/074/075 theorems now compile and are elaborated (`Watchdog.olean` produced), closing the NC-EXT-120 functional defect in which they were asserted but never elaborated. |
| **SYS-R1/R3-002** | Minor | `Watchdog.lean:16-19` | **No** | **NC-PAL-01, GAP-A5, and target/hardware items are NOT affected.** The model scope note explicitly states that the hardware watchdog (arm/feed MMIO, independent clock, reset assertion) is not modelled; it is a platform binding not yet wired for the MPFS target (NC-PAL-01), and Renode has no watchdog model (GAP-A5). The C implementation is not formally refined against this model. No flight parameter or hardware item changes. |
| **SYS-R1/R3-003** | Minor | `Watchdog.lean` (scope notes only); no C/interface file touched | **No** | **GNC, safety, FDIR, and operations interfaces are NOT impacted.** No ICD, command/telemetry interface, flight parameter, safety allocation, or runtime behavior changes. The FDIR function itself is unchanged; only its formal evidence status changes from "asserted" to "elaborated/compile-verified." |
| **SYS-R1/R3-004** | Minor | `traceability.yaml:681-697` (SRS-REQ-073/074/075 `lean:` lists); `Watchdog.lean:52,58,76,106,110,114`; `Requirements.lean:21` | **No** | **Cross-disciplinary consistency is RESTORED.** Lean formal verification now correctly covers the watchdog FDIR requirements SRS-REQ-073/074/075 that were previously asserted but not elaborated. All six theorem names match the traceability mapping exactly; `make trace` PASS (120/76/30/14/68, 68 Lean theorems). This is a positive correction closing the technical part of NC-EXT-120. |
| **SYS-R1/R3-005** | Major (pre-existing, **not introduced** by this change) | `sprint-plan.md` NC-EXT-168; `pal_posix.c:546-558` (context) | Blocks N7 / safety-release input; **not a commit/push blocker introduced by this change** | Pre-existing Blocker, safety-relevant: the pre-reset report path can block in stdio/`write()` before the reset is enforced (NC-EXT-168), contradicting SRS-REQ-074 ("failure or absence of that notification shall not prevent the reset from being enforced"). Already OPEN and escalated to the authorized human owner; not dispositioned by the FSW engineer; not caused by the Lean fix. |
| **SYS-R1/R3-006** | Major (pre-existing, **not introduced** by this change) | `sprint-plan.md` NC-PAL-01; `pal_rtems.c` (`FF_NOT_IMPLEMENTED`) | **Not a commit/push blocker** (per register); flight/HIL closure requires authorized-human sign-off | Pre-existing Major FDIR: flight MPFS MSS watchdog binding unwired (NC-PAL-01); target/HIL timing open (NC-EXT-186/188); mission values provisional (NC-EXT-238). The register states NC-PAL-01 is **NOT a blocker for commit or push**. Not caused by the Lean fix. |

---

## 4. Cross-disciplinary consistency assessment

| Discipline | Assessment of this change |
|---|---|
| FSW / Lean (N1/N3) | Gate now genuinely satisfied for SRS-REQ-073/074/075: module import-reachable, elaborated, `.olean` produced, trace name-match preserved. |
| V&V / traceability | `traceability.yaml` SRS-REQ-073/074/075 `lean:` lists now have corresponding compiled theorems; `make trace` PASS (120/76/30/14/68). |
| Safety / FDIR | Formal model covers arming/feed/expiry, report-before-reset ordering, and single reset path. Known limitations are disclosed in-scope (NC-EXT-125 alias-lemma weakness; NC-EXT-166/167 quarter-period margin not proved by the discrete model). No over-claim: scope notes explicitly bound the model. |
| Hardware / target (NC-PAL-01, GAP-A5) | Untouched by this change; remain OPEN and tracked. |
| GNC / operations interfaces | No interface, parameter, or behavior change. |

No new cross-disciplinary inconsistency is introduced by this change.

---

## 5. Systems findings that block commit/push

**None introduced by this change.** All reported gates for this change pass (build exit 0 / 0 warnings; CTest exit 0; Lean `lake build`; `make trace` PASS 120/76/30/14/68; format-check PASS; license OK).

Pre-existing OPEN blockers unrelated to this change — NC-EXT-168 (Blocker, safety-relevant) and NC-PAL-01 (Major FDIR) — remain escalated to the authorized human owner and are inputs to release/safety gates (N7), but per the NC register they do not block commit/push for this change and are not caused by it.

---

## 6. Conclusion and recommendation

The Lean formal verification fix is **avionics-systems neutral**: it introduces no system-level risk, affects no target/hardware/GNC/safety/FDIR/operations interface, and restores cross-disciplinary consistency by making the SRS-REQ-073/074/075 watchdog FDIR theorems genuinely elaborated and compile-verified.

**Recommendation:** the change may proceed to commit/push from a systems perspective, subject to the pre-existing OPEN safety-relevant NCs (NC-EXT-168, NC-PAL-01) remaining under authorized-human ownership for the release/safety gates. No new systems blocking finding is raised.
