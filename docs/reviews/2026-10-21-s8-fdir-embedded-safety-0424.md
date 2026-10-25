# FRICK-REVIEW-0424 — MPFS/MSS watchdog integration specification (embedded safety)

**Profile:** embedded safety review, fresh independent context  
**Disposition:** Review evidence only. No edits, no NC closure, no gate
approval, no implementation, no register access, no policy decision.

**Subject:** `docs/architecture/mpfs-mss-watchdog-integration-spec.md`
(FRICK-ARCH-0024 rev 1.0) with
`docs/architecture/n18-watchdog-integration-proposal.md` (FRICK-ARCH-0023
rev 1.1). The owner authorized path A (wiring the MPFS/MSS watchdog) **only
starting from a controlled specification for approval before any implementation
or register write**; the scope is documentation, research and review.

## What the reviewer confirmed

- **All five public reference hashes match** the spec: the files were re-fetched
  at commit `5a890a5b5693be7ce415ccece2f2283dbc5a8851` and re-hashed.
- **Every cited register/address/bound/key fact re-verified**: register order
  `REFRESH, CONTROL, STATUS, TIME, MSVP, TRIGGER, FORCE`; the ten instance base
  addresses; the write order `TRIGGER → MSVP → TIME → CONTROL → REFRESH` and the
  write-once lock; `4095` / `16 777 200`; `0xDEADC0DE` / `0xDEAD`; prescaler
  256 with the 25 MHz / 69 s / 171 s example; the `APBBUS_CR_WDOG0..4` bits
  0x5–0x9; the context-enable table entries; the BSP absence of any MSS
  register map or driver; the linker memory map; the toolchain identity; and
  the N18-FAIL / Lean-PASS status.
- The **separation of facts, recommendations and missing inputs is real**, the
  emulator's estimated frequency is properly quarantined, the 8 s and 250 ms
  values are not re-justified, and the withdrawn "capability absent ⇒ non-fatal"
  variant is consistently stated as rejected in both documents.

## Findings (5 Major, 5 Moderate, 6 Minor/Comment) and their disposition

| ID | Severity | Finding | Disposition in spec rev 1.1 |
|---|---|---|---|
| M-1 | Major | The **independence argument of SRS-REQ-073** (a reset independent of the software it resets) was absent, together with the residual single-hart dependency and the system-level reset obligation. | **Added** as F-11 (independence argument + residuals) and as the HL-3 system-level reset observation; D-03 now asks for the instance's *reset coverage*, not only its interrupt routing. |
| M-2 | Major | The **write-once lock's consequence** (a mis-armed watchdog is unrecoverable in software → permanent reset loop) was not analysed, and no read-back verification was proposed. | **Added** as F-12 and as the read-back requirement in R-7 (fatal init error on mismatch); the enforcement leg is target-only (F-17, EM-P9). |
| M-3 | Major | The interaction between the **hardware timeout, the 250 ms pre-reset report budget and the single reset path** was not analysed; the software reset request is *deferred* to counter zero, not immediate. | **Added** to R-3/R-7 and as M-13/D-13; the reaction-window ceiling (~42 ms at the cited example clock) is now stated and shown to be incompatible with the 250 ms budget, so the two paths are specified separately. |
| M-4 | Major | Several proposed **emulator oracles were wrong or non-executable** against the pinned model (the model performs no system reset on expiry, does not check the refresh key, reports but does not enforce the lock, and truncates out-of-range writes). | **Rewritten** in §5.1: EM-P1–P4 are model-faithful, EM-P5–P7 moved to host level, and F-17 records the model's measured limits with the statement that the emulator evidences the write *sequence* only. |
| M-5 | Major | The cited source's **"4095 µs" vs prescaled-tick contradiction** was not recorded, and the representable ranges (max reset timeout, max/min reaction window) were not derived. | **Added** as F-16/M-12 and as the R-3 range table. |
| Mo-2 | Moderate | The **permanent timeout NMI** and the mandatory timeout-event clear (double write) were missing. | **Added** as F-13 and as an R-7 obligation, with HL-4 as the target leg. |
| Mo-3 | Moderate | The **feeder-starvation false-reset hazard** (lowest-priority feeder starved first) was not named. | **Named** in R-5 with M-16 and D-14, and measured by HL-6. |
| Mo-4 | Moderate | The **"armed but inert"** case and the design-time (flash-bit) pre-configuration were missing; the context-enable bit values are absent from the retrieved files. | **Added** as F-8/F-15 and R-6, with M-8/M-15 and D-15. |
| Mo-5 | Moderate | The **timebase/power/sleep independence** assumptions were unstated. | **Added** to F-11 and M-13, with the constraint in R-5. |
| Mi-1…Mi-6 | Minor | Requirement attribution (period vs actual interval), the "RTEMS 6.2" label, the BSP file count, D-09's conditionality, EM-P6's observability, and the proposal's "FORCE/CLEAR" wording. | **Corrected** in rev 1.1 except Mi-3, which was checked against the retained BSP transcript: the search matches **two** files (`bspopts.h`, `bsp/riscv.h`), so the original statement stands. The proposal's sequence wording is corrected in FRICK-ARCH-0023 rev 1.2. |

## Reviewer disposition

**Qualified yes as a decision agenda; no as an approvable implementation
design.** The reviewer recommended D-01 option (b): approve the structure and
the reference inventory, and require the content corrections (now applied)
before any register access is authorized.

**Residual owner questions** (now carried in the spec as M-1..M-16 and
D-01..D-15): mission hart/instance and reset coverage; reset scope and
whether a second-level monitor is required; the reset-timeout budget versus the
250 ms report budget; false-reset tolerance; the NMI policy; the mission
timeout and reaction window with the representable-range constraints; the
watchdog clock source; the design-time configuration and boot lock state; the
authoritative lock rule and timeout unit; the gating ownership; what the N18
gate is expected to prove; the HIL thresholds; the escalation record; and the
review scheduling.

**Confidence:** high for the register/address/bound/key/hash/BSP/N18-Lean facts
and for the measured model behaviour; medium for the "no reset macro" finding
and for the SRS independence analysis; low only for any target-silicon claim.
