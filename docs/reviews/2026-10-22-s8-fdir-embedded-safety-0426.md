# FRICK-REVIEW-0426 — MPFS/MSS watchdog integration specification (embedded safety, final delta)

**Profile:** embedded safety review, fresh independent context  
**Disposition:** Review evidence only. No edits, no NC closure, no gate
approval, no implementation, no register access, no policy decision.
FRICK-REVIEW-0424 (which reviewed **rev 1.0**) is retained unchanged and its
conclusions are not restated as findings on a later revision.

**Subject:** `docs/architecture/mpfs-mss-watchdog-integration-spec.md`
(FRICK-ARCH-0024, reviewed at **rev 1.2**) with
`docs/architecture/n18-watchdog-integration-proposal.md` (FRICK-ARCH-0023 rev
1.3).

## Disposition

Rev 1.2 is a material and largely successful correction and is usable as a
**review agenda**; it is correctly *not* an implementation-ready design. Its
cited-source facts are accurate (re-verified against the pinned upstream
commit) and its framing — source classes, the "not established by retained
evidence" list, the grouped input register, the six-item agenda, the explicit
"no policy selected" statement — is honest and usable.

It was **not yet fully accurate in the one place the parent asked to have
provable**: its statement of what the retained probe measured. The probe's own
register labels were wrong, so three of the four observations the parent asked
to verify were not supported as written. That is a documentation correction,
not a policy question, and it reopens no owner decision.

## The parent's six concerns, fact-checked (not echoed)

| # | Concern | Verdict on rev 1.2 |
|---|---|---|
| 1 | Model behaviour only from the retained probe | **not fully resolved** (F-0426-01) and partly over-claimed (F-0426-02) |
| 2 | The 250 ms "provably cannot fit" overclaim | **resolved**; residual attribution error (F-0426-04) |
| 3 | F-12/R-7 order, R-4 `MSVP`, R-3 bound value | **resolved**, except the model's-field sub-note (F-0426-01) |
| 4 | F-11 independence, M-13 attribution, F-15 vs D-15 | **resolved**; residual wording (F-0426-03) |
| 5 | Current-vs-history (HL-1, D-01/D-10, cross-refs, revision drift) | **resolved**; two new cross-reference defects (F-0426-07, F-0426-16) |
| 6 | Safety adequacy as a review agenda | **resolved** — all six safety-critical residuals visible, correctly attributed and not softened; two additions needed |

## Findings

| ID | Severity | Finding | Disposition |
|---|---|---|---|
| F-0426-01 | **Major** | **The probe's register labels are wrong.** Per the cited map (`REFRESH 0x00, CONTROL 0x04, STATUS 0x08, TIME 0x0c, MSVP 0x10, TRIGGER 0x14, FORCE 0x18`), the probe's "TIME" (0x08) is **STATUS** and its "STATUS" (0x04) is **CONTROL**; `TIME` was never probed. Three indicators agree: the model's warning tags the 0x08 write with `DEVRST` (a `STATUS` bit), the 0x08 read-back changes to `0x10` (bit 4 = `LOCKED`) right after the first write to it, and 0x14 behaves as `TRIGGER`. Consequently: no statement about the model's `TIME` width is supported; the `[6-7]`/`[5]`/`[24]` warnings are STATUS-write warnings, not TIME-width evidence; "`STATUS` did not vary" is false (it moved `0x00 → 0x10`; the constant `0x02` is `CONTROL`); and "identical `TIME`/`STATUS` read-backs" is an observation of `STATUS`/`CONTROL`. The *conclusion* (no key-sensitive observable exposed) survives, the stated reason does not. | **Corrected in rev 1.3**: §2.3, F-17 and the retained probe log carry the corrected attribution, the `TIME`-width claim is withdrawn, and `TIME` is explicitly marked unprobed. |
| F-0426-02 | Moderate | "The model exposes no refresh-key-sensitive observable" and "cannot be emulator cases" exceed the probe: the machine was not started (a stopped counter cannot show a reload), `REFRESH` was never read back, and the wrong-key write was the *first* write, so the two writes were not in the same model state. The retained evidence establishes "not established by the retained probe", not a model capability. | **Corrected in rev 1.3**; whether a running-machine leg could witness it is now an explicit owner scoping choice (agenda item F(e)). |
| F-0426-03 | Moderate | R-6's "was **pre-armed** with design-time values" reintroduces the assumption F-15 removed. | **Corrected**: R-6 now says "pre-initialised … whether that constitutes arming on this device is M-15". |
| F-0426-04 | Moderate | The 250 ms restatement is sound, but the still-unknown "actual path duration" was attached to the **feeder** quantities (M-5/M-10) although the 250 ms budget belongs to the **pre-reset report** path; §5.4 had no item for it. | **Corrected**: R-3 points at the report-path duration and §5.4 gains **RP-1**. |
| F-0426-05 | Moderate | The documentation identity manifest identified the rev 1.1 state, not rev 1.2 (four mismatches). | **Corrected**: the manifest is regenerated after the final text freeze. |
| F-0426-06 | Moderate | The proposal's "those registers lock **after the first write**" contradicts the cited source (the lock follows a write in 0x00–0x0c, and the header ties the freeze to the `TIME` write) and licenses writing `TIME` first — the trap the upstream comment warns against. | **Corrected** in proposal rev 1.4. |
| F-0426-07 | Minor | R-7's "(M-14, decision B)" pointed at the wrong agenda item (the reset scope lives in G-C / item C). | **Corrected**. |
| F-0426-08 | Minor | F-2's "(see the model author's note quoted in the reviews)" is a dangling pointer — no retained review quotes it; the contradiction is header-vs-implementation-comment. | **Corrected**: F-2 now names the two sources. |
| F-0426-09 | Minor | §2's RTEMS row cited the toolchain-identity log for the runtime kernel version; that banner is in the gate-verification log. | **Corrected**. |
| F-0426-10 | Minor | R-4's parenthetical "the pinned model participates in a limit computation whatever the enable state is" is not covered by the probe. | **Corrected**: marked not established. |
| F-0426-11 | Minor | The forward reference to the 0426/0427 reports was false at review time. | **Closed** by filing and indexing both reports with this revision. |
| F-0426-12 | Minor | The watchdog's sleep/low-power timebase behaviour had no owner input. | **Corrected**: added to G-B. |
| F-0426-13 | Comment | §5.1's "partial/unconfirmed" forbidden-window placement is conservative; the probe never enabled the window, so this is a statement about the probe, not a model limit. | Placement retained; the wording is now probe-accurate. |
| F-0426-14 | Comment (positive) | The emulator identity and instance facts are independently corroborated: both platform digests reproduced from upstream, and the file independently confirms the five instance bases, the "estimated" frequency comment and `wdog1`'s IRQ wiring. | Raises confidence in R-1's emulator-instance recommendation and in F-6's quarantine of the 156 250 Hz figure. |
| F-0426-15 | Comment (positive) | **All cited-source technical facts re-verified — no factual error found**: register order and offsets, the ten instance bases, the write order and the lock comment, the bounds and keys, the counter semantics, the forbidden window, the prescaler and the worked example, the `FORCE` semantics, the gating entries, the permanent NMI, the start-up timeout-event read, the design-time pre-initialisation, the µs/tick contradiction, plus the local facts (board, table, feeder, hooks, `.resc`, BSP transcript, toolchain identity, trace note) and the NC register state. | Confirms the citation basis. |
| F-0426-16 | Minor | The proposal's header date contradicted its own revision history. | **Corrected** (proposal rev 1.4). |

## Residual owner questions

Unchanged and still OPEN: **A** documentation basis; **B** approve the
specification as a design basis; **C** instance/hart/reset coverage and whether
a second-level monitor is required; **D** mission timeout, reaction window,
cadence and forbidden window; **E** timeout-interrupt policy and false-reset
tolerance; **F** verification and the N18 gate — plus the device-document,
clock/gating, mission-timing, HIL-threshold and basis inputs (M-1..M-16).
Two additions for owner visibility: the emulator's usable scope must be stated
as measured, and the 250 ms decision needs the report-path duration
measurement (**RP-1**).

## Limits

Document-and-evidence review only: no product build, no emulator execution, no
probe re-run, no target/HIL; no NC opened or closed, no gate passed, no policy
decided, and no alteration or restatement of FRICK-REVIEW-0424/0425.
