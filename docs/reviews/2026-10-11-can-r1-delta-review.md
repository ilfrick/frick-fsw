# Review Report — Reference MSS CAN driver (SRS-REQ-057), R1 delta re-review (norm N12)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0110                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (Senior FSW dev / Flight Software)  |
| Reference revision                | `a95cafb + CAN increment (post-NC fixes)` |
| Supersedes / relates to           | FRICK-REVIEW-0106 (initial R1 review, APPROVED WITH COMMENTS) |
| Items under review                | working-tree delta on `src/drivers/can/mpfs/ff_can_mpfs.{c,h}` (AMR accept-all fix, poll re-read, comment honesty); `tests/unit/test_can.c` (hardened mock, known-answer vectors, 127 checks); `config/platform/platform_mpfs.h` (`FF_CAN0_BASE`); `src/apps/app_renode_test.c` (`hal_can`); SRS-REQ-057 wording (`docs/requirements/SRS.md` 1.16); traceability/docs delta |
| Date                              | 2026-10-11                             |
| Scope                             | (a) closure of the original R1-01…R1-15 findings; (b) independent scrutiny of the code delta — AMR/register-map correctness and the new mask macros, the poll re-read, mock fidelity and known-answer vectors, TX round-robin/wraparound, bounds/negative handling, MISRA/style, and honesty of the new comments; (c) an explicit assessment of whether the initial R1 review should have caught the Major (the R1-05 "accept-all is correct" claim). |
| Independence declaration          | Fresh independent session (norm N9); no part in authoring, implementing, executing or prior-reviewing the items beyond the initial R1 report, which this delta re-review is chartered to scrutinise critically (including its own false negative). Read-only for the repository: only this report file was created; **no tracked file was modified**. Gates were re-run read-only; the regression discrimination was reproduced on a throw-away copy under `/tmp` (no repository file touched). |
| Norms applied                     | N1–N20 (`docs/rules.md`); focus N2, N4, N8, N9, N11, N12, N15, N18 |

## 1. Scope

Re-review, after the author applied review-driven fixes, of the uncommitted reference
MSS CAN driver increment (classic CAN 2.0 A/B, SRS-REQ-057, child of SSS-REQ-020).
The trigger for this delta re-review is norm **N12** and the sibling **Major** so
raised by R2 (FRICK-REVIEW-0107 R2-01) and adopted by PA (FRICK-REVIEW-0109 PA-09):
the **accept-all RX acceptance mask was mis-encoded** (written raw to a
left-aligned AMR field). That Major was **not** raised by R1-0106; R1-05 in fact
asserted the mask was correct. This report therefore (1) re-verifies the fix,
(2) audits the whole code delta, and (3) assesses R1-0106's false negative.

**Excluded:** V&V-method adequacy (R2 remit), documentation adjudication (R4),
process/records closure (PA), on-target silicon behaviour (GAP-A5). No build,
docker or Renode run was performed (reserved for other roles); gates were read
from the existing `build/` trees and re-run read-only.

## 2. References

- Project norms: `docs/rules.md` (N1–N20).
- Requirement: SRS-REQ-057 (`docs/requirements/SRS.md` rev **1.16**); parent SSS-REQ-020.
- Register model (independent cross-check): Renode `MPFS_CAN.cs` copy at `/tmp/mpfs_can.cs`
  — AMR identifier field `WithValueField(3, 29,…)` (register bits **3–31**), RTR/IDE
  as bits **1–2**, reserved bit 0; acceptance `(~AcceptanceMask & (message.Id ^ AcceptanceCode)) == 0` (mask = 1 ⇒ don't-care); read-only `RX_STATUS`/`TX_STATUS`; TX requires loaded data.
- Microchip MSS CAN filter layout (`mss_can_filterobject.AMR {N_A:1, RTR:1, IDE:1, ID:29}`,
  `MSS_CAN_get_msg_filter_mask()` left-aligns the ID) — as independently cited by
  R2-0107 and PA-0109.
- PolarFire SoC Data Sheet DS00004248 (Table 5-22, CAN), CAN reference clock — see R1-D-01.
- ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2.

## 3. Delta findings — closure of the original R1-01…R1-15

| Original # | Sev (0106) | Status now | Evidence / rationale |
| ---------- | ---------- | ---------- | -------------------- |
| R1-01 | Comment | **Closed — unchanged** | `can_configure(fd!=0)` → `FF_NOT_IMPLEMENTED`; consistent with the HAL contract. `ff_can_mpfs.c:63-65`. |
| R1-02 | Minor | **Closed — unchanged** | Std `id>0x7FF` / ext `id>0x1FFFFFFF`, `len>8`, NULL/data consistency all retained (`ff_can_mpfs.c:93-98`). |
| R1-03 | Comment | **Closed — unchanged** | Little-endian pack/unpack retained; now also locked by literal known-answer register words (`test_can.c:263-264,279-282`). |
| R1-04 | Minor | **Closed — improved** | MsgAv write-1-to-clear semantics retained; the poll now re-reads the RX control word after the callback before acknowledging, so a callback reconfiguration is not clobbered (`ff_can_mpfs.c:179-185`). |
| **R1-05** | Minor | **WITHDRAWN — the claim was WRONG; fixed by this delta** | 0106 stated the accept-all was "correct for identifier accept-all as modeled". It was not: the raw `0x1FFFFFFF` set only AMR identifier bits 3–28 ⇒ ID-mask bits 0–25, rejecting extended IDs ≥ `0x04000000`. The delta now writes `(FF_CAN_ID_MASK << FF_CAN_ID_SHIFT) | RTR | IDE` (`ff_can_mpfs.h:139-140`). See §4.1 (verified). |
| R1-06 | Comment | **Closed, with new R1-D-01** | Reference bit timing retained; the assumed 125 MHz clock is contradicted by the PolarFire SoC datasheet's 80 MHz CAN reference clock — see R1-D-01. |
| R1-07 | Minor | **Closed — improved** | Poll contract unchanged; the re-read improves robustness (see R1-D-02 for the residual race). |
| R1-08 | Comment | **Closed — unchanged** | Self-test deliberately uses `AMR_DATA` (emulator models only its low 16 bits); comment now also records that the AMR RTR/IDE bits are not modelled (`ff_can_mpfs.c:228-235`). |
| R1-09 | Comment | **Closed — unchanged** | Allocation-free / single instance confirmed (`grep` for `malloc` family: none). |
| R1-10 | Minor | **Closed — now documented** | Round-robin TX with no TX-status poll is now an explicit "known limitation" in the header (`ff_can_mpfs.h:40-46`), addressing PA-05. |
| R1-11 | Minor | **Closed — unchanged** | `renode_hal_can_check()` wires `FF_CAN0_BASE` (0x2010C000), `hal_can_ok` added to the aggregate (`app_renode_test.c`). |
| R1-12 | Minor | **Closed — substantially strengthened** | Mock hardened (read-only status regs, TX requires loaded data, AMR/AMR_DATA field masking, identifier-domain acceptance helper); coverage 60 → **127** runtime checks, with boundary/negative cases, parked-TX-buffer identity, 32-buffer wraparound and a mask-fix regression. See §4. | 
| R1-13 | Comment | **Closed — unchanged** | `clang-format --dry-run --Werror` on the three files: exit 0 (re-verified). |
| R1-14 | Comment | **Closed — not re-runnable** | `cppcheck` absent on this host; MISRA posture unchanged (no dynamic memory, fixed bounds, defensive checks). |
| R1-15 | Minor | **Closed — extended** | `trace.py` PASS (99 reqs / 58 srs / 27 sss / 14 gaps / 51 theorems, 12 CI claims); SRS bumped to 1.16; traceability/SDD/VV/status docs updated. |

**Net:** all 0106 findings are closed; **R1-05 is withdrawn and converted to "fixed"** (it was a false negative, not merely a stale note).

## 4. Independent scrutiny of the code delta

### 4.1 AMR accept-all encoding (the former Major) — verified correct

- `FF_CAN_ACCEPT_ALL` is now `((FF_CAN_ID_MASK << FF_CAN_ID_SHIFT) | FF_CAN_RX_AMR_RTR | FF_CAN_RX_AMR_IDE)`
  = `(0x1FFFFFFF << 3) | 0x2 | 0x4` = **`0xFFFFFFFE`**; bit 0 (reserved N_A) clear (`ff_can_mpfs.h:115-116,139-140`).
- The AMR identifier field is register bits **3–31** (29 bits), so the stored field value is
  `(0xFFFFFFFE >> 3) & 0x1FFFFFFF = 0x1FFFFFFF` — all 29 ID-mask bits "don't care", which is exactly what the
  acceptance semantics (`~mask & (id ^ code) == 0`) require to accept every identifier.
- Verified against the model copy `/tmp/mpfs_can.cs` (`WithValueField(3, 29,…)`; RTR/IDE as tagged bits 1–2)
  and reconciled with the driver's own ID write `(id & FF_CAN_ID_MASK) << FF_CAN_ID_SHIFT` (`ff_can_mpfs.c:105`)
  — **mask and identifier now live in the same (identifier) domain**.
- The header/comment honesty is **correct**: the arithmetic in `ff_can_mpfs.h:129-138` ("without `<< FF_CAN_ID_SHIFT`
  the mask would cover only identifier bits 0–25, silently rejecting extended identifiers ≥ `0x04000000`")
  is exactly reproducible: raw `0x1FFFFFFF` sets register bits 0–28 ⇒ ID-mask field bits 0–25, leaving field
  bits 26–28 = 0 (must match ACR = 0) ⇒ IDs ≥ 2²⁶ = `0x04000000` rejected. The statement that the emulator does not
  model the RTR/IDE mask bits (and that the self-test does not exercise the AMR path) is also accurate.
- **Verdict on the fix: correct, and the comment is honest.**

### 4.2 Poll re-read after callback

`FF_CanMpfsPoll` now re-reads `RX_CMD` after dispatch and writes `(cmd & ~MSGAV) | MSGAV`
(`ff_can_mpfs.c:183-185`). The mock models `MsgAv` as write-1-to-clear and preserves the other
bits (`test_can.c:127-135`); the driver preserves buffer-enable/DLC/IDE. This correctly prevents a
callback-performed reconfiguration of that buffer from being clobbered — a strict improvement over 0106's
description. Residual (inherent) race is recorded as R1-D-02.

### 4.3 Test/mock fidelity and known-answer vectors

- Mock now (a) ignores writes to `RX_STATUS`/`TX_STATUS` (read-only, matching the model), (b) only raises
  TX_MSG when a message was loaded (`tx_valid`), and (c) masks the AMR (`& 0xFFFFFFF8`) and `AMR_DATA`
  (`& 0xFFFF`) fields — addressing R2-04 / PA-03 (`test_can.c:67-70,76-118,123-146`).
- **Known-answer register words** replace convention-echo assertions: `DATA_LO == 0x44332211`,
  `DATA_HI == 0`, and ext `0x04030201 / 0x08070605` (`test_can.c:263-264,279-282`) — addressing R2-05.
- **Identifier-domain acceptance helper** `mock_rx_accepts()` mirrors the model formula and is asserted at
  `0x00000000`, `0x000007FF`, `0x04000000`, `0x1FFFFFFF` (`test_can.c:159-167,324-327`).
- **Regression discrimination reproduced independently** (on a `/tmp` copy of the header patched back to the old
  constant, repository untouched): `checks=127 failures=9`, with failures on the AMR-field assert (`:322`),
  the ≥ `0x04000000` and `0x1FFFFFFF` accept checks (`:326-327`), and the high-extended-id poll block
  (`:356-365`). The 0109/verification-results claim "9 failures at the old constant" is **exact**.
- TX round-robin/wraparound is now asserted by buffer identity: 32 succeeding transmits land on buffers 0…31
  and the 33rd wraps to buffer 0 (`test_can.c:304-314`); bounds/negatives cover `len>8`, NULL data, bad std/ext
  IDs, zero-length frame with NULL payload, DLC>8 clamp, no-callback poll and `init()` idempotence.
- `tests/unit/test_can.c` still **imports the driver's register-map constants**, so it is a "convenient fake, not
  an independent oracle" (the file header now says so honestly) — the genuine independent cross-check remains the
  Renode `MPFS_CAN` model. R1-D-03.

### 4.4 Bounds, arithmetic, style

- `can_configure()` bounds: `bitrate==0` rejected; `bitrate > clk/tq` rejected; prescaler clamped/`0→1` and
  rejected above 15 bits; CONFIG field packing (SJW[3:2], TSEG2[7:5], TSEG1[11:8], BITRATE[30:16]) matches the
  documented map (`ff_can_mpfs.c:60-79`). `can_transmit()` masks the ID before shifting; DLC/IDE packing correct.
- No dynamic allocation; fixed bounds; defensive NULL checks; braces/pointer style per project convention.
- `clang-format --dry-run --Werror` clean on all three files.

### 4.5 New findings (R1-D-xx)

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R1-D-01 | **Minor** | SRS-REQ-057 (bit timing) / `FF_CAN_MPFS_CLK_HZ` | The reference bit timing rests on an **assumed 125 MHz** MSS CAN clock (`ff_can_mpfs.h:34,149`). The PolarFire SoC Data Sheet DS00004248 Table 5-22 gives the **CAN reference clock `FCANCKL` = 80 MHz** ("the reference clock should be fixed to achieve the 1 Mbps bus rate"). If 80 MHz is the peripheral clock, the driver's prescaler is wrong at every rate — e.g. `configure(500000)` computes `presc = round(125e6/(500e3·16)) = 16` ⇒ actual ≈ `80e6/(16·16) = 312.5 kbit/s` (−37.5 %), and 125 MHz would exceed the datasheet's stated max. (At 80 MHz the standard rates 125/250/500 k/1 M become exactly reachable with `presc = 40/20/10/5`, which is the intended 16-tq split.) The on-target residual remains GAP-A5, but the *assumed* constant is a code-level value the mission can fix now. | `ff_can_mpfs.h:32-38,149`; `ff_can_mpfs.c:66-80`; PolarFire SoC DS00004248 Table 5-22 (`FCANCKL` 80 MHz), web | Confirm the MSS CAN clock against the datasheet / MSS Configurator; replace the assumed constant (or parameterise the TSEG/prescaler per rate) and record the achieved bit-rate error. Keep GAP-A5 for silicon timing. **Confidence: Medium** (datasheet extracted via search snippet; full PDF not parsed). |
| R1-D-02 | Comment | `FF_CanMpfsPoll` | The re-read-after-callback fix is correct, but an inherent race remains: if the controller latches a new frame into the same RX buffer between the callback dispatch and the MsgAv acknowledge, the acknowledge clears the new MsgAv and that frame is silently lost. Acceptable for a reference polling backend, but it should be a stated limitation (or the buffer snapshot latched before dispatch). | `ff_can_mpfs.c:160-185` | Add the limitation to the header / `verification-results.md`, or latch the frame before dispatch. Owner: R1/V&V. |
| R1-D-03 | Comment | `tests/unit/test_can.c` (oracle independence) | The mock and the acceptance helper still derive from the driver's own `FF_CAN_*` constants, so the suite cannot detect a *layout-constant* error end-to-end; only the four Renode round trips are a truly independent cross-check. The file header is now honest about this ("convenient fake, not an independent oracle"). | `test_can.c:10-14,24-36,159-167` | Keep/strengthen the Renode cross-check; optionally pin literal register addresses/bit positions in the test. (Corroborates PA-03 / R2-04; no new work beyond their closure.) |

**Severity definition** — **Major**: disqualifying non-conformity or a defect that can defeat a requirement/safety barrier. **Minor**: non-conformity that must be corrected or justified. **Comment**: observation, no derogation.

### 4.6 Assessment: would the initial R1 review have caught the Major? (R1-05)

**No — and it should have.** R1-05 of FRICK-REVIEW-0106 concluded the accept-all was "correct for
identifier accept-all as modeled". That was a **false negative**, and a self-inflicted one:

- The defect was **internally visible in the very file under review**: `can_transmit()` writes the identifier
  as `(id & FF_CAN_ID_MASK) << FF_CAN_ID_SHIFT` (`:105`), while `can_set_rx_callback()` wrote `FF_CAN_ACCEPT_ALL = 0x1FFFFFFF`
  **un-shifted** to the AMR. Comparing the two lines — the classic "is the mask in the same domain as the ID?" check —
  exposes the bug without any external source.
- 0106 even cited the AMR field as "bits 3–29" in the model (line 36), i.e. the reviewer **knew the field started at
  bit 3** yet did not perform the left-alignment comparison. The field is in fact bits 3–**31** (29 bits); the off-by-two
  in the note is itself a sign the mask width/alignment was not checked.
- The host assertion available at the time was a tautology on the written constant, and the mock implemented no
  filtering — but that is exactly the kind of V&V weakness R1 should flag (R2/PA did), and R1-05 instead endorsed it.

**Conclusion:** R2's Major (R2-01) and PA's adoption (PA-09) were correctly escalated; R1-0106's "no code change
required / APPROVED WITH COMMENTS" verdict on the mask was **too weak**. This delta re-review corrects the record:
R1-05 is withdrawn, and the rigorous check (mask-vs-identifier alignment) that should have been applied is now
documented in §3/§4.1. No new code defect was found in the fix.

## 5. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — original findings R1-01…R1-15 closed (R1-05 withdrawn/corrected); the former Major is **verified fixed**; new findings R1-D-01 (Minor) + R1-D-02/R1-D-03 (Comments) tracked to closure.

The AMR accept-all fix is **correct** and its explanatory comment is **honest**; the poll re-read is a genuine
robustness improvement; the test rewrite (127 checks, hardened mock, known-answer vectors, wraparound, negative
cases, and a regression that genuinely discriminates the fix — independently reproduced as 9 failures at the old
constant) closes the V&V-adequacy gap that let the Major through. The only new substantive item is the
**assumed 125 MHz CAN clock** (R1-D-01), which is a reference-value/on-target concern already bounded by GAP-A5
but which the datasheet's 80 MHz value challenges directly and is cheap to resolve.

**One-line rationale:** all 0106 findings close, the R2/PA Major is fixed and independently re-verified, the
delta is defensive/format-clean/traceable, and the residual issues are a reference-clock assumption (Minor) and
two observations — no new Major.

## 6. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
| ---- | ---------------- | ---- | ---------- |
| R1 (SW dev, N9) | Senior FSW reviewer (independent context) | 2026-10-11 | **APPROVED WITH COMMENTS** (R1-D-01 Minor; R1-D-02/R1-D-03 Comments; R1-05 withdrawn) |

## 7. Follow-up records (norm N12)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | ------ | --------------- |
| R1-0106-05 (was Minor) | — | SRS-REQ-057 / `ff_can_mpfs.h` | Accept-all AMR mask mis-encoded (R1-05 was wrong) | Fixed: left-aligned mask + honest comment + behaviour test | §4.1, §4.3; `test_can` 127/0; 9 failures reproduced at old constant | frick-fsw | **closed** | this report |
| R1-D-01 | Minor | SRS-REQ-057 bit timing | Assumed 125 MHz vs datasheet 80 MHz CAN reference clock | open — confirm/fix clock + record bit-rate error | header/verification-results update | frick-fsw | **open** | pending |
| R1-D-02 | Comment | `FF_CanMpfsPoll` | Inherent RX-buffer race on MsgAv acknowledge | open — state limitation or latch snapshot | header/VV record | frick-fsw | open | optional |
| R1-D-03 | Comment | `test_can.c` | Mock oracle shares driver constants | open — corroborates PA-03/R2-04 | Renode cross-check retained | frick-fsw | open | optional |

## 8. Evidence consulted

**Commands run (read-only; no build/docker/Renode):**

- `git status`, `git log --oneline -5`, `git diff`/`--stat` on the delta and the docs.
- `ctest --test-dir build/native` → **19/19 PASS** (incl. `can`); `./build/native/tests/test_can` → **`checks=127 failures=0`**.
- `grep -c FF_TEST_ASSERT tests/unit/test_can.c` → 97 (static; the round-robin loop expands the runtime count to 127).
- `clang-format --dry-run --Werror src/drivers/can/mpfs/ff_can_mpfs.{c,h} tests/unit/test_can.c` → exit 0.
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` → **PASS (99 requirements, 58 srs, 27 sss, 14 gaps, 51 theorems, 12 CI claims)**.
- Regression discrimination reproduced on a **/tmp copy** (temp header patched back to `0x1FFFFFFF`, repo untouched):
  `gcc -std=c11 -I … -I repo/src/core -I repo/src …` → **`checks=127 failures=9`** (fails at `test_can.c:322,326,327,356,361-365`).
- `grep` for `FF_CAN_MPFS_CLK_HZ` / `125 MHz` usage; `which cppcheck clang-tidy` → absent (MISRA not re-runnable).
- Web: PolarFire SoC DS00004248 Table 5-22 (CAN reference clock 80 MHz) — see R1-D-01.

**Files read (working tree on `a95cafb`):** `src/drivers/can/mpfs/ff_can_mpfs.{c,h}`; `tests/unit/test_can.c`; `tests/ff_test.h`; `config/platform/platform_mpfs.h` (diff); `src/apps/app_renode_test.c` (diff); `docs/requirements/SRS.md` 1.16 + diff; `docs/requirements/traceability.yaml` (SRS-REQ-057) + `traceability.md`; `docs/reviews/README.md` (diff) and `2026-10-11-can-{r1,r2,r4,pa}-review.md`; `docs/vv/verification-results.md`; `docs/reviews/review-template.md`; `/tmp/mpfs_can.cs`.

**Not executed / inaccessible:** host rebuild, docker, Renode (reserved for other roles); `cppcheck`/MISRA (tool absent); PolarFire SoC datasheet full PDF (content type unsupported — value taken from the official search extract, hence Medium confidence on R1-D-01).

---

Uncertainty statement: R1-D-01 is **Medium** confidence (the 80 MHz value is from the official Microchip datasheet table as surfaced by search; the full PDF could not be parsed here and the true MSS CAN module clock depends on the configured clock source — hence the recommendation to confirm before acting). All other facts are **High** confidence: reproduced from the repository, the read-only gates, or the emulator model copy. The AMR-field layout on silicon remains governed by GAP-A5. This report is an independent assessment and confers no approval, qualification or certification authority; the closure/commit decision is the project integrator's.

---
