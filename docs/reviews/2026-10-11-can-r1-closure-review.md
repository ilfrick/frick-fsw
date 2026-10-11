# Review Report — Reference MSS CAN driver (SRS-REQ-057), R1 closure review (norm N12)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0114                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (Senior FSW dev / Flight Software)  |
| Reference revision                | `a95cafb + CAN increment (clock/std fixes)` |
| Supersedes / relates to           | FRICK-REVIEW-0110 (R1 delta re-review, APPROVED WITH COMMENTS); initial FRICK-REVIEW-0106 |
| Items under review                | working-tree delta on `src/drivers/can/mpfs/ff_can_mpfs.h` (clock), `ff_can_mpfs.c` (presc branch comment), `tests/unit/test_can.c` (raw AMR assert, 128 checks); `docs/vv/verification-results.md` 0.30, `CHANGELOG.md` |
| Date                              | 2026-10-11                             |
| Scope                             | Closure of R1-D-01 (Minor, clock) and disposition of R1-D-02/R1-D-03 (Comments) of FRICK-REVIEW-0110; independent verification of the 80 MHz reference and the exactness claim; re-check of the `configure()` rate guard and prescaler-overflow rejection at 80 MHz; whole-delta consistency sweep for the clock change. Cross-checked the author's R2-D1/R2-D2 dispositions (R2 remit) for consistency. |
| Independence declaration          | Fresh independent session (norm N9); no part in authoring, implementing, executing or prior-reviewing this delta beyond FRICK-REVIEW-0110 (and its 0106 predecessor), which this closure review is chartered to confirm. Read-only for the repository: only this report file was created; **no tracked file was modified**. Gates were re-run read-only; no build, docker or Renode run was performed. |
| Norms applied                     | N1–N20 (`docs/rules.md`); focus N2, N4, N8, N9, N11, N12, N15, N18 |

## 1. Scope

Closure re-review, after the author applied the fix for the single Minor of
FRICK-REVIEW-0110 (**R1-D-01**: the reference bit-timing clock was an assumed
125 MHz, contradicted by the PolarFire SoC datasheet's 80 MHz CAN reference
clock), of the reference MSS CAN driver increment (classic CAN 2.0 A/B,
SRS-REQ-057, child of SSS-REQ-020). The task is to (a) confirm R1-D-01 is
resolved and independently verify the new 80 MHz value and the exactness claim,
(b) dispose of the two residual Comments R1-D-02/R1-D-03, (c) re-check that the
`configure()` rate guard (`bitrate > clk/tq`) and the prescaler-overflow
rejection remain correct at 80 MHz, and (d) confirm the clock change introduced
no inconsistency in the live documents or the test suite. The author's
dispositions of the R2 findings **R2-D1** (`presc == 0` defensive comment) and
**R2-D2** (raw AMR write assertion) are inspected for consistency with the R1
scope; their formal closure is the R2 remit.

**Excluded:** V&V-method adequacy (R2), documentation adjudication (R4),
process/records closure and the pre-commit `status.json`/review-log bookkeeping
(PA, FRICK-REVIEW-0113), on-target silicon timing (GAP-A5). No build, docker or
Renode run was performed; gates were re-run read-only from the existing `build/`
trees and the retained Renode logs.

## 2. References

- Project norms: `docs/rules.md` (N1–N20); review template `docs/reviews/review-template.md`.
- Prior reports: FRICK-REVIEW-0110 (R1 delta, this closure's parent), 0106 (initial R1), 0107/0111 (R2), 0113 (PA).
- Requirement: SRS-REQ-057 (`docs/requirements/SRS.md` rev 1.16); parent SSS-REQ-020.
- Primary technical references (independently corroborated this review):
  - PolarFire SoC MSS Technical Reference Manual: "The CAN clock is derived from
    MSS PLL output… The supported frequencies in MHz are 8, 16, 24, 32, 40, 48,
    56, 64, 72, and 80."
  - PolarFire SoC MSS Configurator User Guide: CAN output clock requirement "must
    be a multiple of 8 (maximum 80 MHz)".
  - PolarFire SoC Data Sheet DS00004248 (Table 5-22, CAN): `FCANCKL` typ = max = 80 MHz.
- ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025).

## 3. Closure table — R1 findings of FRICK-REVIEW-0110

| NC | Sev (0110) | Item / Requirement | Disposition | Independent re-verification | Status |
| -- | ---------- | ------------------ | ----------- | --------------------------- | ------ |
| **R1-D-01** | Minor | SRS-REQ-057 bit timing / `FF_CAN_MPFS_CLK_HZ` | Assumed 125 MHz replaced by **80 MHz** (`ff_can_mpfs.h:153`); header comment now cites the MSS TRM and states the standard rates are exact (prescaler 40/20/10/5) | Value verified against the MSS TRM + DS00004248 Table 5-22; exactness recomputed (see §4.1); guard/overflow re-checked at 80 MHz (§4.2); live docs consistent (§4.4) | **CLOSED** |
| R1-D-02 | Comment | `FF_CanMpfsPoll` MsgAv race | optionally "state the limitation or latch the snapshot" | No explicit limitation text added to the driver header or `verification-results.md`; the header already records polling-only RX and no bus-off/error/RX-loss handling. No code change; the re-read-after-callback mitigation from the prior delta is retained (`ff_can_mpfs.c:179-185`) | **carried (optional Comment, no derogation)** |
| R1-D-03 | Comment | `test_can.c` oracle independence | "keep/strengthen the Renode cross-check; optionally pin literals" | Test header still states honestly it is "a convenient fake, not an independent oracle for the layout" (`test_can.c:10-12`); the Renode `MPFS_CAN` model remains the independent cross-check; no regression in this delta | **carried (optional Comment, no derogation)** |

**Net:** the only Minor (R1-D-01) is closed; R1-D-02/R1-D-03 remain optional
Comments with no action required (they never carried a derogation).

### 3.1 Consistency of the author's R2 dispositions (R2 remit; inspected here)

| Item | Author disposition | Consistency check (R1 view) |
| ---- | ------------------ | --------------------------- |
| R2-D1 (Minor) | `presc == 0` branch now marked "defensive: unreachable…" (`ff_can_mpfs.c:72-73`) | **Sound.** The reachability argument holds at 80 MHz: the guard `bitrate > clk/tq` rejects any `bitrate > 5e6`, so `divisor = bitrate·tq ≤ clk` and `presc = (clk + divisor/2)/divisor ≥ 1`. The comment is accurate and matches R2's recommended disposition. |
| R2-D2 (Minor) | `test_can.c` asserts the **raw** AMR write `g_m.amr_written == FF_CAN_ACCEPT_ALL` (`:326`); mock records the raw value before masking (`:138`) | **Sound and discriminating.** `FF_CAN_ACCEPT_ALL = 0xFFFFFFFE` sets the RTR (`1<<1`) and IDE (`1<<2`) mask bits; a regression dropping either would yield `0xFFFFFFF8` and fail the assertion, which the emulator cannot detect (it does not model those bits). This closes the gap R1-D-03 flagged as a mock-fidelity/independence limit. |

## 4. Independent verification of the delta

### 4.1 Clock value and exactness claim — verified exact

`FF_CAN_MPFS_CLK_HZ = 80000000u` (`ff_can_mpfs.h:153`). With the fixed 16-tq
split (1 sync + TSEG1 13 + TSEG2 2), `presc = round(clk / (bitrate·tq))`:

| bit rate | divisor = bitrate·16 | presc | actual = 80e6/(presc·16) | error |
| -------- | -------------------: | ----: | -----------------------: | ----: |
| 125 kbit/s | 2 000 000 | **40** | 125 000.000 | 0.000 % |
| 250 kbit/s | 4 000 000 | **20** | 250 000.000 | 0.000 % |
| 500 kbit/s | 8 000 000 | **10** | 500 000.000 | 0.000 % |
| 1 Mbit/s | 16 000 000 | **5** | 1 000 000.000 | 0.000 % |

The header's "prescaler 40/20/10/5" and "exact at the standard rates" claims
recompute **exactly** (all four rates, zero error) — a strict improvement over
the former 125 MHz assumption, which PA-02/0113 quantified as −0.79 %/+0.81 % at
125/250 kbit/s and −2.34 % at 500 kbit/s and 1 Mbit/s. The chosen reference is
also corroborated by three issuer documents (§2), the strongest being DS00004248
Table 5-22 which fixes `FCANCKL` at 80 MHz, and MSS Configurator's "multiple of 8,
max 80 MHz" rule. **Confidence: High.**

### 4.2 `configure()` rate guard and prescaler-overflow rejection at 80 MHz — remain correct

Re-derived from `ff_can_mpfs.c:66-80`:

- **Rate guard** `bitrate > clk/tq` → `bitrate > 80e6/16 = 5 000 000` rejected. A
  16-tq split cannot represent a bus rate above 5 Mbit/s at this clock; unchanged
  correctness, now with margin over the 1 Mbit/s maximum classic rate. Test
  `configure(8000000u,…)` → `FF_INVALID_ARGUMENT` exercises it (`test_can.c:247`).
- **Prescaler overflow**: `divisor = bitrate·tq`; `presc > 0x7FFF` rejected. The
  overflow threshold is `bitrate < 80e6/(32767·16) ≈ 152.6 bit/s`; test
  `configure(100u,…)` → `FF_INVALID_ARGUMENT` exercises it via the 15-bit field
  (`test_can.c:248`). No `uint32_t` overflow in `bitrate·tq`: the guard bounds
  `bitrate ≤ 5e6` before the multiply, so `divisor ≤ 8e7`.
- **`presc == 0`**: unreachable under the guard (`presc ≥ 1`); now documented as
  defensive (R2-D1).
- CONFIG field packing (SJW[3:2], TSEG2[7:5], TSEG1[11:8], BITRATE[30:16])
  unchanged and matched by the test (`test_can.c:252-257`).

No regression introduced by the clock change.

### 4.3 Test suite and gates (read-only re-run)

- `./build/native/tests/test_can` → **`checks=128 failures=0`** (127 at 0110 + the
  raw-AMR assertion `:326`) — confirms the R2-D2 addition and the 128 claim.
- `ctest --test-dir build/native` → **19/19 PASS** (incl. `can`).
- `clang-format --dry-run --Werror` on the two driver files + `test_can.c` → **clean**.
- `trace.py docs/requirements/traceability.yaml` → **PASS (99/58/27/14, 51 theorems, 12 CI claims)**.
- Renode retained evidence: `build/renode/console.log` contains `[renode-test] hal_can=PASS` and `RENODE_TEST: PASS`; the ELF mtime (22:52:31) precedes the console log (22:52:52), consistent with a rebuild-then-run order (not re-executed here; assessed from the retained logs as R2 did).

### 4.4 Whole-delta consistency sweep for the clock change — no live-document inconsistency

- `grep` for `125000000|125 MHz|125MHz` finds **no live document** still asserting
  the old value: hits are confined to the historical review records
  (`2026-10-11-can-r4-review.md`, `...-r1-delta-...`, `...-r2-delta-...`,
  `...-pa-*.md`), which are point-in-time records and correctly not retro-edited
  (N16). The recomputations in PA FRICK-REVIEW-0113 (V11, "125 MHz") and the
  R2 delta FRICK-REVIEW-0111 (`test_can` = 127) are likewise **historical** — they
  describe the tree as it stood before this clock/std fix.
- `docs/vv/verification-results.md:41` and `:426` now state the 80 MHz basis,
  the exact rates and the 128-check count, and attribute the correction to
  R1-D-01; `CHANGELOG.md` records the 125 → 80 MHz fix in the [[Unreleased]] /
  Fixed section. `SRS.md` (1.16) and `traceability.yaml` (`lean_note`) describe the
  bit timing as a "reference derivation from the configured MSS CAN clock" without
  hardcoding a value — so they required no change and remain consistent.
- No other code references `FF_CAN_MPFS_CLK_HZ`; `config/platform/platform_mpfs.h`
  does not duplicate it.

**Conclusion of the sweep:** the clock change introduced **no** live-document
correctness inconsistency; it supersedes the arithmetic records of the earlier
reports, which remain valid as history.

## 5. Residual findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| C-1 | Comment | SRS-REQ-057 / `ff_can_mpfs.c` BITRATE field | The exactness result assumes the CONFIG `BITRATE` field equals the prescaler directly (`presc << 16`). Some CAN controllers encode `BRP-1`; if the MSS CAN does, the actual prescaler would be `presc-1` and the rates would not be exact. The header already requires the mission to "re-derive the TSEG/prescaler split per rate against the datasheet before flight", so this is covered by GAP-A5, but the field offset is not itself pinned. | `ff_can_mpfs.c:78`; `ff_can_mpfs.h:34-39,148-152`; GAP-A5 | No action for this closure; keep under GAP-A5 (confirm the BITRATE field encoding on silicon). |
| C-2 | Comment | N11/N12 records | The R2 delta (0111) and PA delta (0113) reviews were written before this clock/std fix, so their recorded figures (`test_can` 127, 125 MHz recomputation) and PA-02's acceptance note now lag the tree. The live records (`verification-results.md`, `CHANGELOG.md`) are updated; the historical reports are not retro-edited. A brief PA/R2 re-touch (or a note in the commit) is advisable so the commit package's cross-references are not read as current. | `2026-10-11-can-r2-delta-review.md:100`; `2026-10-11-can-pa-delta-review.md:224,264-266` | Integrator: add the `docs/reviews/README.md` row for this report and, if required by N12/N20, a one-line PA/R2 re-touch. |
| R1-D-02 | Comment | `FF_CanMpfsPoll` | (carried) inherent MsgAv-acknowledge race not explicitly stated as a limitation | §3; `ff_can_mpfs.c:179-185` | Optional; no derogation. |
| R1-D-03 | Comment | `test_can.c` | (carried) mock not an independent oracle for the layout | §3; `test_can.c:10-12` | Optional; Renode cross-check retained. |

**Severity definition** — **Major**: disqualifying non-conformity or a defect that can defeat a requirement/safety barrier. **Minor**: non-conformity that must be corrected or justified. **Comment**: observation, no derogation.

No **Major** or open **Minor** finding remains in the R1 scope.

## 6. Conclusion

- [x] **APPROVED** — R1-D-01 (Minor) closed with objective evidence; the residual items are Comments only (no derogation).
- [ ] APPROVED WITH COMMENTS
- [ ] REJECTED

The only Minor of FRICK-REVIEW-0110 (R1-D-01) is **resolved and independently
verified**: the reference clock is now 80 MHz, a value corroborated by the MSS
TRM, the MSS Configurator guide and DS00004248 Table 5-22, at which the four
standard rates become **exact** (prescaler 40/20/10/5, zero error). The
`configure()` rate guard and the prescaler-overflow rejection remain correct at
80 MHz, the `presc == 0` branch is soundly dispositioned as unreachable
defensive code, and the 128-check suite (including the raw-AMR assertion that
protects the RTR/IDE mask bits) is green. The clock change introduced **no**
live-document inconsistency. R1-D-02 and R1-D-03 remain optional Comments with
no action required. **No new Major or Minor finding.**

**One-line rationale:** the R1-D-01 clock fix is correct, exact and well-sourced;
the guard/overflow logic is unchanged and correct at 80 MHz; the delta is
format-clean, traceable and consistent; only optional Comments remain.

## 7. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
| ---- | ---------------- | ---- | ---------- |
| R1 (SW dev, N9) | Senior FSW reviewer (independent context) | 2026-10-11 | **APPROVED** (R1-D-01 closed; R1-D-02/R1-D-03 carried as optional Comments; C-1/C-2 recorded) |

## 8. Follow-up records (norm N12)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | ------ | --------------- |
| R1-0110-D-01 | Minor | SRS-REQ-057 bit timing | assumed 125 MHz vs datasheet 80 MHz | fixed: `FF_CAN_MPFS_CLK_HZ = 80e6` + TRM-cited comment | §4.1 (exact), §4.2 (guard), tests 128/0, ctest 19/19, trace PASS | frick-fsw | **closed** | this report |
| R1-0110-D-02 | Comment | `FF_CanMpfsPoll` | MsgAv-ack race not stated | no action required | §3; `ff_can_mpfs.c:179-185` | frick-fsw | **carried (optional)** | this report |
| R1-0110-D-03 | Comment | `test_can.c` | mock oracle shares driver constants | no action required | §3; `test_can.c:10-12` | frick-fsw | **carried (optional)** | this report |
| C-1 | Comment | BITRATE field encoding | exactness assumes field = presc (not presc−1) | tracked under GAP-A5 | §5 | programme | open (informational) | — |
| C-2 | Comment | N11/N12 records | pre-fix R2/PA figures now historical; README row for 0114 pending | integrator action on commit | §4.4, §5 | integrator | open (pre-commit) | — |

## 9. Evidence consulted

**Commands run (read-only; no build/docker/Renode):**

- `./build/native/tests/test_can` → **`checks=128 failures=0`**.
- `ctest --test-dir build/native` → **100% tests passed, 0 failed out of 19**.
- `clang-format --dry-run --Werror src/drivers/can/mpfs/ff_can_mpfs.c src/drivers/can/mpfs/ff_can_mpfs.h tests/unit/test_can.c` → exit 0.
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` → **PASS (99/58/27/14, 51 theorems, 12 CI claims)**.
- `grep -rn "125000000|125 MHz|125MHz"` and `grep` for `FF_CAN_MPFS_CLK_HZ|80000000|80 MHz` across `src tests docs config` → old value only in historical review records.
- `grep` renode logs: `build/renode/console.log` → `[renode-test] hal_can=PASS`, `RENODE_TEST: PASS`; ELF (22:52:31) precedes console log (22:52:52).
- `grep -c FF_TEST_ASSERT tests/unit/test_can.c` → 98 (static; loop-expanded to 128 at run time).
- Re-derivation of `presc = round(clk/(bitrate·tq))` for the four standard rates, the rate-guard and prescaler-overflow thresholds (Python, §4.1/§4.2).
- Web (issuer sources): PolarFire SoC MSS TRM (CAN clock 8…80 MHz), MSS Configurator UG ("multiple of 8, max 80 MHz"), DS00004248 Table 5-22 (`FCANCKL` 80 MHz).

**Files read (working tree on `a95cafb`):** `src/drivers/can/mpfs/ff_can_mpfs.{c,h}`; `tests/unit/test_can.c`; `docs/reviews/2026-10-11-can-r1-delta-review.md`, `...-can-r2-delta-review.md`, `...-can-r4-delta-review.md`, `...-can-pa-delta-review.md`; `docs/vv/verification-results.md` (0.30); `docs/requirements/SRS.md` (1.16) + `traceability.yaml` (SRS-REQ-057); `CHANGELOG.md`; `docs/reviews/README.md`; `docs/reviews/review-template.md`.

**Not executed / inaccessible:** host rebuild, docker, Renode (reserved for other roles; evidence assessed from retained logs); `cppcheck`/MISRA (tool absent); PolarFire SoC datasheet full PDF body (values taken from official search extracts and the TRM; corroborated across three issuer documents).

---

Uncertainty statement: **High** confidence in the R1-D-01 closure — the 80 MHz value is corroborated by three independent Microchip issuer documents and the exactness is reproduced arithmetically (zero error at all four standard rates); the guard/overflow and the test result are reproduced directly from the repository. **Medium** confidence on the residual C-1 (the CONFIG `BITRATE` field encoding on silicon: `presc` vs `presc-1`) — no datasheet register-level parse was possible here, and the header already defers per-rate re-derivation to GAP-A5. The Renode and MISRA evidence are assessed from retained artifacts, not re-run. This report is an independent assessment and confers no approval, qualification or certification authority; the commit/closure decision remains the project integrator's, and on-target bit timing remains governed by GAP-A5 and a qualified human.

---
