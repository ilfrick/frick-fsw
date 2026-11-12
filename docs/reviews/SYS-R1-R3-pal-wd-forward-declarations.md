# Independent Systems Review — PAL watchdog hardware-hook forward declarations (NC-EXT-239)

**Reviewer profile:** Senior Aeronautics and Space Avionics Systems Engineer, independent R1/R3 review  
**Repository:** /home/nicola/fsw/frick-fsw, branch main, HEAD 1c4b323 (working tree contains the change)  
**Target:** Microchip RT PolarFire SoC RTPFS160ZT (RISC-V) under RTEMS 6.2; ECSS class B target  
**Change scope:** `src/core/pal/pal.h` only — forward declarations of `ff_pal_wd_hw_arm`/`ff_pal_wd_hw_feed`/`ff_pal_wd_hw_reset` (lines 147-149)  
**NC addressed:** NC-EXT-239 (`docs/planning/sprint-plan.md:326`), Minor (build hygiene / declaration), disposition required  
**Review date:** 2026-11-12  
**Authority:** Independent engineering review only. No certification, airworthiness, human-rating, qualification or release approval is conveyed. Approved humans retain all safety-critical and regulatory decisions.

---

## 1. Change under review

Only `src/core/pal/pal.h` is modified. Added, in the "Platform watchdog hardware hooks" section (lines 139-149), immediately before the pre-reset notifier section:

```
147: FF_Status_t ff_pal_wd_hw_arm(uint32_t timeout_ms);
148: FF_Status_t ff_pal_wd_hw_feed(void);
149: FF_Status_t ff_pal_wd_hw_reset(void);
```

`pal.h` is already included by `pal_rtems.c` (line 49), `pal_watchdog.c` (line 24) and `pal_posix.c` (line 13). The prototypes precede the first use in `pal_rtems.c` (call to `ff_pal_wd_hw_feed` at line 167 inside `ff_pal_wd_hw_arm`), eliminating the implicit-declaration diagnostic. No function bodies, no register access, no timing/power/thermal/resource change — pure declarations.

---

## 2. Findings

Classification key: **Major** = would block commit/push if unresolved, or affects a safety-critical boundary; **Minor** = build-hygiene/documentation/non-functional finding that does not block commit/push.

| ID | Class | Blocks commit/push? | File:line | Finding |
|----|-------|---------------------|-----------|---------|
| **SYS-R1-001** | Minor | **No** | `pal.h:147-149` | System-level risk of the forward declarations: **NONE**. Pure declarations, no logic, no memory/timing/power/thermal/radiation/EMC effect, no new fault-containment or independence path. The watchdog binding abstraction (host simulated backend vs unwired RTEMS/MPFS binding, `pal.h:120-127`) is unchanged. This resolves NC-EXT-239's compile diagnostic only. |
| **SYS-R1-002** | Minor | **No** | `pal.h:147-149`; `pal_watchdog.c:29-31`; `pal_rtems.c:111-113` | Cross-TU declaration consistency. `pal.h` is now the single header declaring the three hooks, matching the existing pattern (`pal_posix.c` includes `pal.h` at line 13; definitions at `pal_posix.c:673-694`). `pal_watchdog.c:29-31` and `pal_rtems.c:111-113` retain identical local redeclarations — valid C, zero diagnostic under the current build (`make build` exit 0, 0 warnings; RTEMS cross-build exit 0, zero warnings). Call sites `pal_watchdog.c:45/65/93` and `pal_rtems.c:167` see prototypes before first use. No inconsistency between declaration, definition and call sites. |
| **SYS-R1-003** | Minor | **No** | `pal_rtems.c:278-304`; `pal.h:123`; `docs/status/escalations.md:62` | NC-PAL-01 / GAP-A5 / target hardware: **NOT affected**. The flight-target path (`#else` branch, `pal_rtems.c:278-304`) returns `FF_NOT_IMPLEMENTED` for arm/feed/reset; hooks remain deliberately unwired per NC-PAL-01. No register write, clock, gating, timeout or MPFS binding change. GAP-A5 (on-target/HIL evidence) unchanged — this is a host-executable/cross-build declaration fix, not target evidence. |
| **SYS-R1-004** | Minor | **No** | `pal.h:129-137` (public API unchanged); `pal.h:151-156` (pre-reset notifier); `pal.h:158-170` (test seams, `FF_TEST_HOOKS` only) | GNC, safety, FDIR and operations interfaces: **NOT impacted**. No functional software change. The public PAL watchdog API, the single reset path (`pal.h:124-127`), pre-reset notifier seam and test seams are unchanged. No TM/TC, event-id, ICD or ground-interface impact. |
| **SYS-R1-005** | Major (resolved by this change) | **No** | `pal_rtems.c:156` (prior); NC-EXT-239 (`sprint-plan.md:326`) | Pre-existing implicit declaration of `ff_pal_wd_hw_feed` at `pal_rtems.c:156` was ill-formed in C99 and would fail under `-Werror`; NC-EXT-239 required declaring the hooks in one PAL-watchdog header included by `pal_rtems.c`, then re-running the RTEMS cross-build with zero warnings. Both conditions met: `pal.h:147-149` declarations added; RTEMS 6.2 cross-build exit 0 with zero warnings. Technical condition satisfied; finding closed by the change. NC-EXT-239 remains OPEN in the register pending owner disposition but is not a commit/push blocker per project policy (`docs/status/status.json:5`). |
| **SYS-R1-006** | Minor | **No** | `pal_watchdog.c:29-31`; `pal_rtems.c:111-113` | Redundant local forward declarations now duplicated by `pal.h:147-149`. Identical redeclarations are harmless in C and produce no diagnostic. Removal is an optional follow-up cleanup under the normal N8/N15 cycle, not required for this change. |
| **SYS-R1-007** | Minor | **No** | `sprint-plan.md:326`; `docs/status/handover.md` | NC disposition/traceability: NC-EXT-239 disposition (resolution evidence: gates green, zero warnings) should be recorded when the owner dispositiones the NC. This change alone closes no NC; NC-PAL-01, NC-EXT-198 and GAP-A5 remain OPEN. No blocking systems finding. |

---

## 3. Cross-disciplinary consistency summary

| Discipline | Interface touched? | Status |
|---|---|---|
| FSW / PAL | `pal.h` declarations only | Consistent — declarations match definitions in `pal_watchdog.c`, `pal_rtems.c`, `pal_posix.c` |
| Safety / FDIR | None | Watchdog binding still `FF_NOT_IMPLEMENTED` in flight builds (NC-PAL-01 open) |
| GNC | None | No control-law, timing or actuator impact |
| Operations / ground | None | No TM/TC, event-id or ICD change |
| EMC / radiation / power / thermal | None | No hardware, register, clock or power change |
| V&V / labs | Build gates only | RTEMS cross-build zero warnings; CTest/Lean/trace/format/license green |

---

## 4. Gate evidence (per change author, consistent with tree state)

- `make build`: exit 0, 0 warnings (host)
- CTest: exit 0 (including `test_link_uart.c`)
- `make verify-lean` (`lake build`): PASS
- `make trace`: PASS (120/76/30/14/68)
- format-check: PASS; license OK
- RTEMS 6.2 cross-build (N18 profile): exit 0, zero warnings — implicit-declaration warning eliminated

---

## 5. Conclusion

**No avionics/system-level finding blocks commit or push.**

- Forward declarations introduce no system-level risk — SYS-R1-001 (Minor).
- NC-PAL-01, GAP-A5 and all target/hardware items are unaffected; hooks remain `FF_NOT_IMPLEMENTED` for flight builds — SYS-R1-003 (Minor).
- GNC, safety, FDIR and operations interfaces are not impacted — no code change — SYS-R1-004 (Minor).
- Cross-disciplinary declaration/definition/call-site consistency is satisfied — SYS-R1-002 (Minor).
- NC-EXT-239's technical condition (zero-warning RTEMS cross-build) is met; the register disposition remains with the owner and is not a commit/push blocker — SYS-R1-005/SYS-R1-007.

**Recommendation:** proceed to commit/push per N7/N15 (all gates green). Retain this review evidence in the working tree (`docs/reviews/`). Leave NC-EXT-239 owner disposition and NC-PAL-01/GAP-A5 target/HIL closure to the authorized authorities. This review conveys no qualification, certification, airworthiness or flight-readiness approval.