# Review Report - M4 increment 3 (GEM native TSN backend) - delta re-review

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0031                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (SW dev) / R2 (SW V&V) / R4 (SW documentation) |
| Reference revision                | base commit `ef9c3d1` + uncommitted working tree (M4 increment 3 fixes, 2026-10-10) |
| Items under review                | fixes for `FRICK-REVIEW-0030`: `src/drivers/eth/gem/ff_gem_tsn.{h,c}`, `src/drivers/eth/gem/ff_gem_regs.h`, `tests/unit/test_gem_tsn.c`, `docs/architecture/hardware-abstraction.md`, `docs/requirements/SRS.md` (SRS-REQ-038) + `traceability.{yaml,md}`, `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md`, `CHANGELOG.md`, `README.md` |
| Predecessor review                | `docs/reviews/2026-10-10-m4-gem-tsn-r1-r2-r4-review.md` (FRICK-REVIEW-0030, REJECTED) |
| Date                              | 2026-10-10                             |
| Scope                             | Delta re-review (norm N12) of the ENST unit-conversion fix (R1-01), the test-oracle fix (R2-01) and the Qav/CBS documentation reconciliation (R4-01), plus collateral check of the remaining predecessor NCs. Register semantics re-verified against the Linux `macb` sources (declared source of truth). ECSS-E-ST-40C, ECSS-Q-ST-80C; norms N1-N17. |
| Independence declaration          | Reviewer took no part in the fixes; fresh context, independent session (N9). |
| Norms applied                     | N1-N17 (see `docs/rules.md`)           |

## 1. Scope

Delta re-review of the fixes applied to the milestone-M4 increment-3 (GEM native
TSN backend) non-conformances raised by FRICK-REVIEW-0030. The two Majors
(R1-01 ENST times in raw ns, R2-01 test oracle encoding the same wrong unit) and
the Minor R4-01 (Qav/CBS contradiction) were verified by inspection, by
re-derivation of the conversion from the upstream `macb.h` helper and by
re-running the gates. The remaining predecessor NCs were checked for
collateral change; R1-02/R1-04/R1-05 were partially addressed in the same delta.

Executed in this session (host, Linux):

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` ->
  **PASS** (73 requirements: 39 SRS / 20 SSS / 14 GAP; 51 Lean theorems;
  9 CI evidence claims).
- `cmake -S . -B build/dtsn -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build
  build/dtsn && ctest --test-dir build/dtsn --output-on-failure` -> **10/10
  PASS** (ccsds, sb, tbl, pus, time, hal, math, sdls, gem, gem_tsn).
- `./build/dtsn/tests/test_gem_tsn` -> `checks=15 failures=0`.
- `make coverage` -> **1529/2353 lines (65.0%)**, functions **207/250**
  (this does **not** reproduce the values recorded in `README.md` /
  `verification-results.md`; see D-02).
- ENST conversion re-derived against Linux `drivers/net/ethernet/cadence/macb.h`
  (`master`): `enst_ns_to_hw_units(ns, speed) = DIV_ROUND_UP(ns*speed,
  ENST_TIME_GRANULARITY_NS*1000)` with `ENST_TIME_GRANULARITY_NS 8`, and
  `GEM_ON_TIME_SIZE`/`GEM_OFF_TIME_SIZE` = 17, `GEM_START_TIME_SEC_SIZE` = 2,
  `GEM_START_TIME_NSEC_SIZE` = 30, `GEM_ENST_DISABLE_QUEUE_OFFSET` = 16 — all
  matched field-by-field; `macb_main.c` `macb_taprio_setup_replace()`
  (disable -> program -> enable) also inspected.

Excluded / not executable: on-target timing/PTP accuracy (documented GAP-A5);
the Microchip MSS TRM (DS60001702) was not fetched, so register claims are
checked against the Linux `macb` sources only, consistent with the declared
source of truth.

## 2. References

- Project norms: `docs/rules.md` (N1-N17); template `docs/reviews/review-template.md`.
- Predecessor: `docs/reviews/2026-10-10-m4-gem-tsn-r1-r2-r4-review.md` (FRICK-REVIEW-0030).
- Linux `drivers/net/ethernet/cadence/macb.h` (`master`): helper
  `enst_ns_to_hw_units()`, `enst_max_hw_interval()`, `ENST_TIME_GRANULARITY_NS 8`,
  `GEM_ON/OFF_TIME_SIZE 17`, `GEM_START_TIME_SEC/NSEC_*`,
  `GEM_ENST_DISABLE_QUEUE_OFFSET 16`, `struct macb_queue_enst_config.on_time_bytes`.
- Linux `drivers/net/ethernet/cadence/macb_main.c` (`master`),
  `macb_taprio_setup_replace()` / `macb_taprio_destroy()`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C.

## 3. Findings (remaining after the fixes)

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| D-01 | Minor | `CHANGELOG.md`, `docs/vv/tool-qualification.md`, `docs/vv/verification-results.md` (N8/N10/N11/N16) | The recorded `test_gem_tsn` check count is stale: the fix added two assertions (out-of-range gate + `get_time_ns(NULL)`), so the suite now reports **15** checks, but the three documents still state **13** and the cycle total is recorded as **342** (should be **344**, since `15+20+9+29+101+31+20+81+23+15 = 344`). The evidence is correct and reproducible; only the recorded figures are out of date. | `./build/dtsn/tests/test_gem_tsn` -> `checks=15 failures=0`; `CHANGELOG.md:94`; `tool-qualification.md:77`; `verification-results.md:27`. | Update the three counts to 15 / 344 and regenerate the record. |
| D-02 | Minor | `README.md`, `docs/vv/verification-results.md` (N8/N16) | The coverage panel is stale: the fix added lines to `ff_gem_tsn.c` and to the test, so `make coverage` now reports **1529/2353 lines (65.0%)**, functions **207/250**; the documents still state **1518/2339 (64.9%)**, functions **206/249** (the predecessor's pre-fix values). | `make coverage` (this session); `README.md:76`; `verification-results.md:27,261`. | Regenerate the panel (`make metrics`) from the fresh coverage run. |
| D-03 | Comment | `src/drivers/eth/gem/ff_gem_tsn.c` (`tsn_configure_tas`); SRS-REQ-038 | R1-02 is substantively addressed (writing `0` to `FF_GEM_ENST_CONTROL` clears all queue enable bits before reprogramming, so stale enables are removed), but the explicit hardware **disable command** bits are not asserted: Linux first writes `queue_mask << GEM_ENST_DISABLE_QUEUE_OFFSET` (bits 16-23) before programming. Clearing the enable mask is the essential part; the disable command is the documented stop sequence for a running list. | `ff_gem_tsn.c:72-73`; Linux `macb_main.c:4453-4456`; `macb.h` `GEM_ENST_DISABLE_QUEUE_OFFSET`. | Optional hardening: mirror `(0xFFu << 16)` before reprogramming, as upstream. |
| D-04 | Minor | `src/drivers/eth/gem/ff_gem_tsn.c` (`tsn_configure_tas`) | R1-04 only partially closed: `start_nsec` is now validated against the 30-bit mask, but `start_sec` is still silently **masked** (`(g->start_sec & 0x3u)`), so a start time beyond 3 s is accepted and truncated instead of rejected. Linux returns `-ERANGE` above `GENMASK(GEM_START_TIME_SEC_SIZE-1,0)`. | `ff_gem_tsn.c:90`; Linux `macb_main.c:4400-4407`; `GEM_START_TIME_SEC_SIZE 2`. | Reject `start_sec > 3` with `FF_INVALID_ARGUMENT`. |
| D-05 | Comment | `src/drivers/eth/gem/ff_gem_tsn.c` (`tsn_configure_tas`) | R1-05 only partially closed: the on/off bounds are now validated against the 17-bit field, but `len % sizeof(FF_TsnGate_t) != 0` is still accepted and the trailing partial entry silently ignored. | `ff_gem_tsn.c:65`. | Reject a non-multiple `len` with `FF_INVALID_ARGUMENT`. |
| D-06 | Comment | `docs/engineering/driver-development.md` (N8) | Predecessor R4-02 open: the "extension points" table still lists only PAL/Link/Board and no HAL row, although two HAL reference backends (`ff_gem.c`, `ff_gem_tsn.c`) are now delivered. | `driver-development.md:23-27`. | Add a HAL/`src/hal/` row pointing at the reference backends. |
| D-07 | Comment | several docs / `tests/CMakeLists.txt` (N16) | Predecessor R4-03 open: `docs/requirements/SRS.md`, `traceability.md`, `traceability.yaml`, `verification-results.md`, `tool-qualification.md` and `tests/CMakeLists.txt` still end without a trailing newline. | `tail -c1` on each file. | Add the trailing newline. |
| D-08 | Comment | `docs/vv/verification-results.md` (N11) | Predecessor R2-03 open: the single "M4 drivers + RTEMS integration + GEM TSN" cycle row was edited again to fold increment 3 into it rather than adding a distinct increment-3 row (recurrence of residual D-04 from the increment-2 review). | `verification-results.md:27`; `2026-10-10-m4-rtems-delta-review.md:74`. | Add a distinct increment row or record the explicit rationale and close D-04. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

No Major finding remains. D-01/D-02 are new documentation-accuracy Minor findings
introduced by the delta (the fix changed the counts without regenerating the
records). D-03 carries the residual of R1-02; D-04/D-05 carry R1-04/R1-05;
D-06/D-07/D-08 carry R4-02/R4-03/R2-03.

## 4. Positive verification evidence (reproduced in this session)

- **R1-01 closed.** `FF_TsnGem_t` now carries `speed_mbps` (`ff_gem_tsn.h:37`)
  and `FF_TsnGemSetup` takes it (`ff_gem_tsn.h:40`, `ff_gem_tsn.c:128-136`).
  `tsn_ns_to_hw` computes `(ns*speed + 7999)/8000` in 64-bit arithmetic
  (`ff_gem_tsn.c:53-57`), which is exactly `DIV_ROUND_UP(ns*speed, 8*1000)` =
  `enst_ns_to_hw_units()` in the Linux `macb.h` source of truth. The on/off
  results are validated against `FF_GEM_ENST_TIME_MASK` (0x1FFFF, the 17-bit
  `GEM_ON/OFF_TIME_SIZE`) and rejected with `FF_INVALID_ARGUMENT` instead of
  being truncated (`ff_gem_tsn.c:84-89`). `start_nsec` is likewise validated
  against the 30-bit `FF_GEM_ENST_START_NSEC_MASK`.
- **R2-01 closed.** `test_gem_tsn.c` sets up at **1000 Mbps**
  (`FF_TsnGemSetup(&t, &bus, 0u, 1000u)`, `test_gem_tsn.c:48`) and asserts the
  speed-scaled oracle `ENST_ON_TIME(3) == 125` for 1000 ns and
  `ENST_OFF_TIME(3) == 250` for 2000 ns (`test_gem_tsn.c:65-68`) — independent
  of the code under test, derived from the register contract. A boundary case
  (`on_nsec = 2e9` -> hw `2.5e8 > 0x1FFFF`) now asserts
  `FF_INVALID_ARGUMENT` (`test_gem_tsn.c:70-73`). Direct run:
  `checks=15 failures=0`.
- **R4-01 closed.** `hardware-abstraction.md` (rev 0.3) now states the
  reference backend implements 802.1Q VLAN/PCP, 802.1Qbv (ENST) and 802.1AS and
  that **802.1Qav CBS is not exposed in the register subset used**
  (`hardware-abstraction.md:51,125`). The code comment's cross-reference to that
  document is now supported (`ff_gem_tsn.c:102-103`), and the SRS-REQ-038 text
  (`SRS.md:187-191`) is consistent.
- **R1-02 substantially closed.** `tsn_configure_tas` disables the control
  register before programming the per-queue times (`ff_gem_tsn.c:72-73`),
  removing stale enables; see D-03 for the only nuance.
- **Gates green:** `trace.py` PASS (73 requirements, 39/20/14, 51 theorems,
  9 CI claims); host build + `ctest` **10/10**; `test_gem_tsn` 15/0;
  ENST/TSU/SCRT2 offsets and bitfields match Linux `macb.h`
  (`ENST_CONTROL 0x0880`; `ENST_START_TIME/ON/OFF(q) 0x0800/0x0820/0x0840 +
  q*4`; `START_SEC_SHIFT 30`, 30-bit nsec; 17-bit time; `TSH/TSL/TN`;
  `SCRT2 0x0540`).
- **802.1Qav honesty retained:** `configure_cbs` returns
  `FF_NOT_IMPLEMENTED` and the test asserts it (`ff_gem_tsn.c:104-110`,
  `test_gem_tsn.c:84`).

## 5. Conclusion

Graded conclusions by profile:

| Profile | Conclusion | Basis |
| ------- | ---------- | ----- |
| R1 (design/code) | **APPROVED WITH COMMENTS** | Major R1-01 closed and independently re-derived against `macb enst_ns_to_hw_units`. R1-02 substantively closed (D-03 Comment). R1-04/R1-05 only partially closed (D-04 Minor, D-05 Comment); R1-03/R1-06 remain open Minor/Comment. |
| R2 (V&V) | **APPROVED WITH COMMENTS** | Major R2-01 closed (oracle re-derived with a known link speed plus a boundary case). New Minor D-01 (stale check count / record total); R2-02 partial, R2-03 open (D-08). |
| R4 (documentation, N17) | **APPROVED WITH COMMENTS** | R4-01 closed (Qav/CBS reconciled, rev 0.3, cross-reference now valid). New Minor D-02 (stale coverage panel); R4-02/R4-03 open (D-06/D-07). |

Overall: **APPROVED WITH COMMENTS**. The two disqualifying defects are resolved:
the ENST ON/OFF registers are now programmed in the link-speed-scaled hardware
unit (`ceil(ns*speed/8000)`, validated against the 17-bit field) and the unit
test encodes an independent, correct oracle. No Major finding remains; the
delta may be committed/pushed with the Minor findings (D-01, D-02, D-04) and the
Comments (D-03, D-05..D-08) tracked to closure under norm N12.

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent reviewer agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS (Major R1-01 closed; R1-04/R1-05 partial) |
| R2 (N9/N10)   | independent reviewer agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS (Major R2-01 closed; D-01 Minor) |
| R4 (N17)      | independent reviewer agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS (R4-01 closed; D-02 Minor) |

## 7. Follow-up records

(NC-by-NC disposition after the fixes; norm N12: NCs are resolved by re-applying
ALL rules to the affected scope and re-reviewing the deltas until full
compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | Major | `ff_gem_tsn.c`, SRS-REQ-038 (Qbv) | ENST ON/OFF in raw ns, not speed-scaled hardware units | Add speed; convert `ceil(ns*speed/8000)`; validate 17-bit field | `speed_mbps` added; `tsn_ns_to_hw` matches `enst_ns_to_hw_units`; 125/250 @1G; out-of-range rejected | frick-fsw | **closed** | Closed |
| R2-01 | Major | `test_gem_tsn.c`, SRS-REQ-038 | Test oracle encoded the wrong (raw-ns) unit | Re-derive oracle with a known speed; add boundary case; re-record | Setup at 1000 Mbps; asserts 125/250; 2e9 ns -> `FF_INVALID_ARGUMENT`; 15/0 | frick-fsw | **closed** | Closed |
| R4-01 | Minor | `hardware-abstraction.md`, SRS-REQ-038, `ff_gem_tsn.c` | Qav/CBS capability claim vs "not exposed"; unsupported cross-reference | Reconcile text; fix comment reference | TSN row reworded (Qav CBS not exposed), rev 0.3; code reference valid | frick-fsw | **closed** | Closed |
| R1-02 | Minor | `ff_gem_tsn.c` | No ENST disable-before-reconfigure; stale enables | Mirror disable -> program -> enable | Control register zeroed before programming (enable bits cleared); disable-command bits not asserted | frick-fsw | **closed** | Substantively closed; residual D-03 |
| R1-03 | Minor | `ff_gem_tsn.c` (`set_vlan`) | VLAN id discarded, returns success; only PCP/queue 0; RX/TX unclear | Implement VID match or document/reject; state semantics | Comment now states the VID is recorded by the EtherType config; match still not implemented | frick-fsw | open | Open |
| R1-04 | Minor | `ff_gem_tsn.c` | `start_sec`/`start_nsec` silently truncated | Validate ranges, return error | `start_nsec` now validated; `start_sec` still `& 0x3u` (silent) | frick-fsw | open | Partially closed; residual D-04 |
| R1-05 | Comment | `ff_gem_tsn.c` | Partial trailing gate entry ignored; no on/off bounds | Validate `len % sizeof`; validate bounds | On/off bounds validated; `len % sizeof` still unchecked | frick-fsw | open | Partially closed; residual D-05 |
| R1-06 | Comment | `ff_gem_tsn.c` (`tsn_get_time_ns`) | TSU read without coherence retry | Document risk or retry | Unchanged | frick-fsw | open | Open (Comment) |
| R2-02 | Minor | `test_gem_tsn.c` | Error/boundary paths and multi-queue not tested | Extend negative cases | Adds invalid PCP, out-of-range gate, NULL out; still no NULL/0/>8/queue>=8/len cases | frick-fsw | open | Partially closed |
| R2-03 | Comment | `verification-results.md` | Combined M4 cycle row (recurrence of D-04) | Add increment row or record rationale | Single combined row remains | frick-fsw | open | Open (Comment; D-08) |
| R4-02 | Comment | `driver-development.md` | Extension-points table omits HAL/reference drivers | Add HAL row | Table unchanged | frick-fsw | open | Open (Comment; D-06) |
| R4-03 | Comment | several docs / `tests/CMakeLists.txt` | Missing trailing newline | Add newline | Still absent in 6 files | frick-fsw | open | Open (Comment; D-07) |
| D-01 | Minor | `CHANGELOG.md`, `tool-qualification.md`, `verification-results.md` | Check count 13 vs actual 15; total 342 vs 344 | Update counts; regenerate record | Direct run `checks=15`; docs say 13/342 | frick-fsw | open | New delta Minor |
| D-02 | Minor | `README.md`, `verification-results.md` | Coverage panel stale (1518/2339, 206/249) | Regenerate `make metrics` | `make coverage` = 1529/2353 (65.0%), funcs 207/250 | frick-fsw | open | New delta Minor |
| D-03 | Comment | `ff_gem_tsn.c` | Disable-command bits not asserted before reconfigure | Optional: write disable mask as upstream | `ff_gem_tsn.c:72-73` writes `0` only | frick-fsw | open | New delta Comment |
| D-04 | Minor | `ff_gem_tsn.c` | `start_sec` still silently masked | Reject `start_sec > 3` | `ff_gem_tsn.c:90` | frick-fsw | open | New delta Minor (carries R1-04) |
| D-05 | Comment | `ff_gem_tsn.c` | `len % sizeof` still unchecked | Reject non-multiple `len` | `ff_gem_tsn.c:65` | frick-fsw | open | New delta Comment (carries R1-05) |
| D-06 | Comment | `driver-development.md` | HAL row missing | Add HAL row | Table unchanged | frick-fsw | open | New delta Comment (carries R4-02) |
| D-07 | Comment | docs / `tests/CMakeLists.txt` | Missing trailing newlines | Cosmetic fix | 6 files affected | frick-fsw | open | New delta Comment (carries R4-03) |
| D-08 | Comment | `verification-results.md` | Combined cycle row | Separate increment row | `verification-results.md:27` | frick-fsw | open | New delta Comment (carries R2-03) |

---

Committed and pushed with the reviewed change (norm N11).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | independent reviewer agent (R1/R2/R4) | Delta re-review of FRICK-REVIEW-0030 fixes (GEM native TSN backend): Majors R1-01/R2-01 closed, APPROVED WITH COMMENTS |
