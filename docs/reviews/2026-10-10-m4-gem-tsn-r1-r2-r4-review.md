# Review Report - M4 increment 3 (GEM native TSN backend)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0030                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (SW dev) / R2 (SW V&V) / R4 (SW documentation) |
| Reference revision                | base commit `ef9c3d1` + uncommitted working tree (M4 increment 3, 2026-10-10) |
| Items under review                | `src/hal/hal_tsn.h` (`FF_TsnBackend_t` self-pointer), `src/drivers/eth/gem/ff_gem_tsn.{h,c}`, `src/drivers/eth/gem/ff_gem_regs.h` (ENST/TSU/SCRT2), `tests/unit/test_gem_tsn.c`, `CMakeLists.txt`, `tests/CMakeLists.txt`, `docs/requirements/SRS.md` (SRS-REQ-038) + `traceability.{yaml,md}`, `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md`, `CHANGELOG.md`, `README.md` |
| Date                              | 2026-10-10                             |
| Scope                             | Correctness of the ENST/TSU/SCRT2 register programming against the Cadence GEM layout (Linux `macb.h` / `macb_main.c`), honest handling of the unimplemented 802.1Qav CBS, test independence, traceability, documentation completeness/counts (N8/N17). ECSS-E-ST-40C, ECSS-Q-ST-80C; norms N1-N17. |
| Independence declaration          | Reviewer took no part in authoring, implementing or executing any reviewed item; fresh context, independent session. |
| Norms applied                     | N1-N17 (see `docs/rules.md`)           |

## 1. Scope

Reviewed the milestone-M4 increment-3 delta (GEM native TSN backend) against the
base commit `ef9c3d1` and the uncommitted working tree, as three independent
profiles:

- **R1 (design/code)**: the HAL TSN contract change in `src/hal/hal_tsn.h`
  (all vtable pointers now take a `void *self`), the reference backend in
  `src/drivers/eth/gem/ff_gem_tsn.{c,h}`, and the new register
  offsets/fields in `src/drivers/eth/gem/ff_gem_regs.h`. The ENST (802.1Qbv),
  TSU (802.1AS/1588) and screener type 2 (802.1Q) programming was checked
  field-by-field against the Cadence GEM layout used by the Linux `macb`
  driver, which the header itself declares as the source of truth.
- **R2 (V&V)**: independence and adequacy of `tests/unit/test_gem_tsn.c`
  (mock MMIO), the honesty of the verification claims for SRS-REQ-038, the
  traceability manifest entry, and the verification record
  (`docs/vv/verification-results.md`).
- **R4 (documentation, N17)**: completeness/correctness/consistency of the new
  and touched documentation, counts, revision histories, cross-references and
  the README metrics panel.

Executed in this session (host, Linux):

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml`
  -> **PASS** (73 requirements: 39 SRS / 20 SSS / 14 GAP; 51 Lean theorems;
  9 CI claims).
- `cmake -S . -B build/tsnr -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build
  build/tsnr && ctest --test-dir build/tsnr --output-on-failure` -> **10/10
  PASS**.
- `test_gem_tsn` executed directly -> `checks=13 failures=0`.
- `make coverage` -> lines **1518/2339 (64.9%)**, functions **206/249**
  (matches the README panel and `docs/vv/verification-results.md`).
- Document-id uniqueness scan (`docs/**/*.md`, root `*.md`) -> **67 unique ids,
  no duplicates**; markdown relative-link scan -> **0 broken links**.
- Reference fetch: Linux `drivers/net/ethernet/cadence/macb.h` and
  `macb_main.c` (`master`), used for the ENST/TSU/SCRT2 field comparison.

Excluded / not executable in this environment: on-target execution and real
TSN/PTP timing accuracy (documented GAP-A5); the Microchip MSS TRM
(DS60001702) was not fetched, so all register claims are checked against the
Linux `macb` sources only, consistent with the stated source of truth.

## 2. References

- Project norms: `docs/rules.md` (N1-N17)
- Traceability manifest: `docs/requirements/traceability.yaml`
- Linux `drivers/net/ethernet/cadence/macb.h` (`master`):
  `GEM_ENST_CONTROL 0x0880`, `GEM_ENST_START_TIME/ON_TIME/OFF_TIME`,
  `GEM_ENST_DISABLE_QUEUE_OFFSET 16`, `GEM_START_TIME_SEC_OFFSET 30`,
  `GEM_ON_TIME_SIZE`/`GEM_OFF_TIME_SIZE` = 17,
  `ENST_TIME_GRANULARITY_NS 8`, `enst_ns_to_hw_units()`,
  `struct macb_queue_enst_config.on_time_bytes`, `GEM_TSH/TSL/TN`,
  `GEM_SCRT2` with `GEM_QUEUE/VLANPR/VLANEN_OFFSET`.
- Linux `drivers/net/ethernet/cadence/macb_main.c` (`master`),
  `macb_taprio_setup_replace()` / `macb_taprio_destroy()`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C.

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R1-01 | **Major** | `src/drivers/eth/gem/ff_gem_tsn.c`; SRS-REQ-038 (802.1Qbv) | **The ENST ON_TIME/OFF_TIME registers are programmed in raw nanoseconds, but the hardware field is a link-speed-scaled byte count, not nanoseconds.** `tsn_configure_tas` writes `g->on_nsec & FF_GEM_ENST_TIME_MASK` and `g->off_nsec & FF_GEM_ENST_TIME_MASK` directly. Per the declared source of truth, the ON/OFF time fields are expressed in units of `ns * speed_mbps / (ENST_TIME_GRANULARITY_NS * 1000)` (`ENST_TIME_GRANULARITY_NS = 8`, i.e. one byte-time at the current line rate): the Linux driver computes `on_time_bytes = enst_ns_to_hw_units(entry->interval, speed)` and `off_time_bytes = enst_ns_to_hw_units(cycle_time - entry->interval, speed)` before writing. At 1 Gb/s the correct register value is `ns/8` (raw ns over-programs by 8x); at 100 Mb/s it is `ns/80`. The 17-bit field also truncates, so a legitimate 1 ms gate (correct value 125000 at 1 Gb/s) is written as `1000000 & 0x1FFFF = 82496`. `FF_TsnGem_t` carries no link speed, so the backend cannot compute the correct value at all. | `ff_gem_tsn.c:73-74`; `ff_gem_regs.h:49` (`FF_GEM_ENST_TIME_MASK 0x1FFFF`); `ff_gem_tsn.h:28-29` (`on_nsec`/`off_nsec`); Linux `macb.h`: `ENST_TIME_GRANULARITY_NS 8`, `enst_ns_to_hw_units()`, `struct macb_queue_enst_config.on_time_bytes` (`"on" time nsec expressed in bytes`), `GEM_ON_TIME_SIZE 17`; `macb_main.c:4430-4433,4463-4466`. | Add the link speed to the backend (parameter or read `NCFGR.GBE`/speed) and convert with `DIV_ROUND_UP(ns * speed_mbps, 8000)`, validating against `enst_max_hw_interval`; update the unit comment and the test expectation accordingly. |
| R1-02 | Minor | `src/drivers/eth/gem/ff_gem_tsn.c`; SRS-REQ-038 | The gate-control-list update never writes the ENST disable bits. Linux disables every queue (`queue_mask << GEM_ENST_DISABLE_QUEUE_OFFSET`) before reprogramming the per-queue time registers, and again on destroy; the delta only writes the enable mask (`enable & 0xFFFF`) after programming. A queue that was enabled by a previous call and is omitted from a later schedule keeps its stale enable bit and stale start/on/off values, and reconfiguring a running list is not atomic. | `ff_gem_tsn.c:75-78`; Linux `macb.h`: `GEM_ENST_DISABLE_QUEUE_OFFSET 16`; `macb_main.c:4453-4470,4491-4501`. | Mirror the disable -> program -> enable sequence; clear the enable bits of queues not present in the new list. |
| R1-03 | Minor | `src/drivers/eth/gem/ff_gem_tsn.c` (`tsn_set_vlan`); SRS-REQ-038 | `set_vlan` accepts a VLAN id but explicitly discards it (`(void)vid;`) and returns `FF_SUCCESS`; only PCP and the VLAN-enable bit are written to one screener type-2 entry, always to queue 0. Restricting the match to a specific VID requires the type-2 compare registers (`GEM_T2CMPW0/1`, `GEM_ETHT`) which are not programmed, so the "VLAN classification" clause is only partially realised and the API silently reports success for a VID it does not apply. The comment also labels this an egress "PCP -> queue 0" mapping while screener type 2 is an RX classifier, blurring the semantic. | `ff_gem_tsn.c:36-49`; `ff_gem_regs.h:52-55`; Linux `macb_main.c:3851-3874,3951-3958` (`gem_writel_n(bp, SCRT2, index, ...)`, `GEM_T2CMPW*`). | Either program the VID compare registers or document the limitation and reject/ignore non-default VID explicitly (dedicated status), and state whether classification is RX or TX. |
| R1-04 | Minor | `src/drivers/eth/gem/ff_gem_tsn.c` | Input ranges are silently coerced instead of validated. `start_sec` is masked to 2 bits (`& 0x3u`) and `start_nsec` to 30 bits; a start time beyond 3 s, or a nanoseconds part >= 1e9, is accepted and truncated rather than rejected. Linux validates `start_time_sec <= GENMASK(GEM_START_TIME_SEC_SIZE-1,0)` and returns `-ERANGE`. | `ff_gem_tsn.c:70-71`; Linux `macb_main.c:4400-4407`. | Validate `start_sec <= 3`, `start_nsec < 1000000000`, and return `FF_INVALID_ARGUMENT` otherwise. |
| R1-05 | Comment | `src/drivers/eth/gem/ff_gem_tsn.c` (`tsn_configure_tas`) | `count = len / sizeof(FF_TsnGate_t)` silently ignores a trailing partial entry when `len` is not a multiple of the entry size, and there is no check that `on_nsec`/`off_nsec` are non-zero or within the hardware maximum. | `ff_gem_tsn.c:57-63`. | Reject `len % sizeof(FF_TsnGate_t) != 0`; validate on/off bounds once R1-01 is fixed. |
| R1-06 | Comment | `src/drivers/eth/gem/ff_gem_tsn.c` (`tsn_get_time_ns`) | The TSU is read as `TSH`, `TSL`, then `TN` with no coherence retry. If the seconds register rolls over between the `TSL` and `TN` reads, the returned time is stale by up to one second. Reading the seconds first is the low-risk order (TSL wraps only after 2^32 s), so the practical exposure is small, but the 802.1AS consumer should be aware. | `ff_gem_tsn.c:102-104`. | Document the read order/risk, or re-read `TSL` and retry when the value changed (as PTP stacks do). |
| R1-07 | Comment | `src/hal/hal_tsn.h` | The change adding a `void *self` parameter to every `FF_TsnBackend_t` vtable entry is **correct and consistent** with the existing `FF_EthMac_t`/`FF_NetIf_t` contracts, and fixes a contract that could not reach instance state. It compiles against the sole placeholder consumer (`test_hal.c`). No production vtable implementation other than the new backend exists, so no caller breaks. | `git diff src/hal/hal_tsn.h`; `src/hal/hal_eth.h:27-32`; `tests/unit/test_hal.c:75`. | (No action.) Consider a sentence in the TSN contract header noting the self-pointer convention for backend authors. |
| R2-01 | **Major** | `tests/unit/test_gem_tsn.c`; SRS-REQ-038 | **The unit test encodes the implementation's (incorrect) ENST time interpretation, so it cannot detect R1-01 and gives false confidence in the 802.1Qbv clause.** With `on_nsec = 1000`, the test asserts `ENST_ON_TIME(3) == 1000`, i.e. it asserts that the raw nanosecond value is what the hardware wants; the hardware contract (R1-01) requires a speed-scaled value (`125` at 1 Gb/s). The mock-MMIO approach is otherwise sound (no real hardware dependency), but the oracle is derived from the code under test rather than from the register specification. | `test_gem_tsn.c:63-65`; `macb_main.c:4430-4433`; `macb.h` `enst_ns_to_hw_units`. | After fixing R1-01, give the test a known link speed and assert the converted ON/OFF values (and a truncation-boundary case); re-record the evidence. |
| R2-02 | Minor | `tests/unit/test_gem_tsn.c` | Error paths and multi-queue behaviour are not exercised: no case for `configure_tas(NULL, ...)`, `count == 0`, `count > 8`, `queue >= 8`, a non-multiple `len`, or a null-bus `init`. Only a single gate (queue 3) and a single PCP are tested. The error-return branches added by the driver are therefore uncovered. | `test_gem_tsn.c:35-78` (single test function); driver branches `ff_gem_tsn.c:28-30,61-63,67-69`. | Add negative/ boundary cases per the driver's declared contract. |
| R2-03 | Comment | `docs/vv/verification-results.md` (N11/N16) | The single "M4 drivers + RTEMS integration" cycle row was edited **again** to fold increment 3 into it (now "M4 drivers + RTEMS integration + GEM TSN", 342 checks, 73 reqs) rather than adding a distinct increment-3 row; the per-increment History entries (0.7/0.8/0.9) remain. This repeats the already-open residual D-04 from the increment-2 review. The counts themselves are correct and reproduce. | `git diff docs/vv/verification-results.md:27`; `docs/reviews/2026-10-10-m4-rtems-delta-review.md` (D-04, open). | Add a distinct increment row (or record the explicit rationale for a single per-milestone row and close D-04). |
| R2-04 | Comment | SRS-REQ-038 / manifest | Verification claims are **honest about scope**: `method: Tst / Inp`, `lean: []` with the `lean_note` stating that host unit tests with a mock MMIO bus verify the register/table logic and that on-target timing/PTP accuracy requires the board (GAP-A5). No silent waiver. The `trace` gate reproduces 73/39/20/14/51. | `traceability.yaml` (SRS-REQ-038); `trace.py` output. | (No action; the note should remain accurate after R1-01/R2-01 are fixed.) |
| R4-01 | Minor | `docs/architecture/hardware-abstraction.md`; SRS-REQ-038; `ff_gem_tsn.c` (N8/N17) | **Contradictory documentation of 802.1Qav/CBS.** The architecture catalogue states the GEM internal TSN block supports `802.1Q/Qbv/Qav/Qci/Qbb/802.1AS/1588/CB`, while SRS-REQ-038 and `ff_gem_tsn.h/.c` state that Qav "is documented as not exposed in the GEM register subset". The code comment points to `docs/architecture/hardware-abstraction.md` as the place where the limitation is documented, but that document contains no such limitation (only the capability claim). The cross-reference is therefore unsupported. | `hardware-abstraction.md:51`; `SRS.md:187-191`; `ff_gem_tsn.c:82-83`; `grep Qav docs/architecture/hardware-abstraction.md` -> only line 51. | Reconcile the two: state precisely that the MSS GEM supports Qav/CBS per the TRM but the Linux `macb` register map used as the source of truth does not expose it here, and that on-target CBS is out of scope (GAP-A5); fix the code comment reference. |
| R4-02 | Comment | `docs/engineering/driver-development.md` (N8) | The driver guide's "extension points" table lists only PAL/Link/Board and not the HAL/reference drivers; with two HAL reference backends now delivered (`ff_gem.c`, `ff_gem_tsn.c`) the guide no longer reflects the extension surface it claims to cover. | `driver-development.md:22-33`. | Add a HAL/`src/hal/` row pointing at the reference backends and their traceability mapping. |
| R4-03 | Comment | `docs/requirements/traceability.md`, `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md`, `tests/CMakeLists.txt` (N16) | Re-touched files still end without a trailing newline (pre-existing D-07), surfacing as `\ No newline at end of file` in the delta diff. Cosmetic. | `git diff`; `tail -c1` of the four files. | Add a trailing newline. |
| R4-04 | Comment | `README.md`, `SRS.md`, `CHANGELOG.md`, `traceability.md`, `tool-qualification.md`, `verification-results.md` (N8/N16) | Documentation set is **updated and consistent**: README panel regenerated to 73/39/20/14, 59/73 tests, 20/73 Lean, coverage 1518/2339; SRS rev 0.9; traceability rev 0.8; tool-qualification rev 0.1.9 (+ T3 re-qualification row); verification-results rev 0.9; CHANGELOG entry with the 13-check count. Counts match `trace.py` and `coverage.py`. | `git diff` of the six files; `trace.py`; `make coverage`. | (No action.) |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

## 4. Positive verification evidence (reproduced in this session)

- `trace.py`: `traceability: PASS (73 requirements, 39 srs, 20 sss, 14 gaps,
  51 lean theorems checked)`; the new SRS-REQ-038 -> SSS-REQ-020 parent link,
  the implementation paths (`ff_gem_tsn.{c,h}`) and the test path
  (`test_gem_tsn.c`) all resolve.
- Host build + `ctest`: **10/10** suites PASS; `test_gem_tsn` reports
  `checks=13 failures=0`, matching `CHANGELOG.md`, `tool-qualification.md` and
  `verification-results.md` (342 checks total: the arithmetic
  15+20+9+29+101+31+20+81+23+13 = 342 is correct).
- `make coverage` reproduces **1518/2339 lines (64.9%)**, **206/249 functions**;
  the README panel and `verification-results.md` agree.
- Register **offsets and bitfields match the Linux `macb.h` layout**:
  `ENST_CONTROL 0x0880`; `ENST_START_TIME/ON/OFF(q)` at `0x0800/0x0820/0x0840 +
  q*4`; `START_SEC_SHIFT 30`, `START_NSEC`/`TN` masks 30 bits; `TIME_MASK`
  17 bits; `TSH 0x01c0`, `TSL 0x01d0`, `TN 0x01d4`; `SCRT2 0x0540` with
  `QUEUE/VLANPR/VLANEN` at 0/4/8. The TSU seconds/high composition
  (`sec = TSH<<32 | TSL`, `ns = TN & 0x3FFFFFFF`) is correct.
- The unimplemented 802.1Qav CBS is handled **honestly at the API level**:
  `configure_cbs` returns `FF_NOT_IMPLEMENTED` rather than a fake success, and
  the test asserts exactly that (the wording/consistency issue is R4-01, not a
  false success).
- Document-id uniqueness (67 ids) and markdown relative links (0 broken)
  verified across the whole doc set.
- The `FF_TsnBackend_t` self-pointer change matches the established HAL vtable
  convention and does not break the build or the existing contract-presence
  test.

## 5. Conclusion

Graded conclusions by profile:

| Profile | Conclusion | Basis |
| ------- | ---------- | ----- |
| R1 (design/code) | **REJECTED** | One Major finding: R1-01 (ENST ON/OFF times programmed in nanoseconds, not the speed-scaled hardware units; the field is truncated and the backend has no link speed). R1-02..R1-06 are Minor/Comment. |
| R2 (V&V) | **REJECTED** | One Major finding: R2-01 (the unit test's ENST oracle encodes the same wrong unit and therefore cannot detect R1-01; the 802.1Qbv verification evidence is not valid). R2-02 is Minor, R2-03/R2-04 Comment. |
| R4 (documentation, N17) | **APPROVED WITH COMMENTS** | Minor R4-01 (Qav/CBS contradiction and unsupported code cross-reference) and Comments R4-02..R4-04 only; all tracked to closure. Documentation counts and revision histories are otherwise correct (R4-04). |

Overall: **REJECTED**. Per norm N12 the change must not be committed/pushed
until R1-01 and R2-01 are resolved (fix the ENST unit conversion and re-derive
the test oracle) and the delta is re-reviewed. The Minor/Comment findings are to
be dispositioned and tracked to closure.

- [ ] APPROVED - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [x] REJECTED - one or more Major findings

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent reviewer agent (fresh context) | 2026-10-10 | REJECTED (R1-01 Major) |
| R2 (N9/N10)   | independent reviewer agent (fresh context) | 2026-10-10 | REJECTED (R2-01 Major) |
| R4 (N17)      | independent reviewer agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS |

## 7. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | Major | `ff_gem_tsn.c`, SRS-REQ-038 (Qbv) | ENST ON/OFF programmed in raw ns, not speed-scaled hardware units; no link speed available; 17-bit truncation | Add speed; convert `ns*speed/8000`; validate against max interval | (pending) | frick-fsw | open | (pending) |
| R2-01 | Major | `test_gem_tsn.c`, SRS-REQ-038 | Test asserts raw-ns ENST value (same oracle as the defect); cannot detect R1-01 | Re-derive oracle with a known speed; add boundary case; re-record | (pending) | frick-fsw | open | (pending) |
| R1-02 | Minor | `ff_gem_tsn.c` | No ENST disable-before-reconfigure; stale enables/times retained | Mirror disable -> program -> enable; clear omitted queues | (pending) | frick-fsw | open | (pending) |
| R1-03 | Minor | `ff_gem_tsn.c` (`set_vlan`) | VLAN id discarded, returns success; only PCP/queue 0; RX vs TX semantics unclear | Implement VID match or document/reject; state semantics | (pending) | frick-fsw | open | (pending) |
| R1-04 | Minor | `ff_gem_tsn.c` | `start_sec`/`start_nsec` silently truncated, no range validation | Validate ranges, return error | (pending) | frick-fsw | open | (pending) |
| R2-02 | Minor | `test_gem_tsn.c` | Error/boundary paths and multi-queue not tested | Extend negative cases | (pending) | frick-fsw | open | (pending) |
| R4-01 | Minor | `hardware-abstraction.md`, SRS-REQ-038, `ff_gem_tsn.c` | Qav/CBS capability claim vs "not exposed"; unsupported code cross-reference | Reconcile text; fix comment reference | (pending) | frick-fsw | open | (pending) |
| R1-05 | Comment | `ff_gem_tsn.c` | Partial trailing gate entry silently ignored; no on/off bounds | Validate `len % sizeof`; validate bounds | (pending) | frick-fsw | open | (pending) |
| R1-06 | Comment | `ff_gem_tsn.c` (`tsn_get_time_ns`) | TSU read without coherence retry; up to 1 s error at second boundary | Document risk or retry | (pending) | frick-fsw | open | (pending) |
| R2-03 | Comment | `verification-results.md` | Combined M4 cycle row edited again (recurrence of open D-04) | Add increment row or record rationale | (pending) | frick-fsw | open | (pending) |
| R4-02 | Comment | `driver-development.md` | Extension-points table omits HAL/reference drivers | Add HAL row | (pending) | frick-fsw | open | (pending) |
| R4-03 | Comment | several docs / `tests/CMakeLists.txt` | Missing trailing newline (pre-existing D-07) | Add newline | (pending) | frick-fsw | open | (pending) |

---

Committed and pushed with the reviewed change (norm N11).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | independent reviewer agent (R1/R2/R4) | Initial milestone-M4 increment-3 review (GEM native TSN backend) |
