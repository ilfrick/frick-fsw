# FRICK-REVIEW-0467 — Independent V&V Delta Review (N22/R2)

**Date:** 2026-11-05 (Europe/Rome)  
**Baseline commit:** 3947801  
**Working tree:** a47935f + uncommitted sprint-plan.md changes  
**Scope:** Delta review of `docs/planning/sprint-plan.md` changes per N22/R2 mandate  
**Review authority:** Independent V&V agent (Nemotron 3 Ultra free, session-bound)  
**Independence declaration:** This review was performed by an independent agent instance with no authorship of the changes under review. No shared context, oracles, or management pressure with the development team. The review uses only repository evidence and fresh test execution.

---

## 1. VERDICT

**Overall:** PASS WITH COMMENTS

| Dimension | Verdict | Rationale |
|-----------|---------|-----------|
| Canonical table test claims | **MOSTLY ACCURATE** | 10/12 check-count claims verified by fresh execution; 2 discrepancies (test_link_uart: 72 vs 77; test_es_hs_lifecycle reset-path: 180 claimed, not substantiated) |
| V&V review traceability | **ACCURATE WITH GAPS** | Referenced reviews exist; S8-SCH row lists 0462–0465 which exist but relevance not all confirmed; review index has ID collisions (0438, 0439, 0440) |
| V&V criteria weakening | **NONE DETECTED** | NC-EXT-236 strengthened (EVIDENCE DELIVERED); T3 NOT FULLY QUALIFIED and N18 FAIL explicitly retained; no silent weakening |
| Oracle/independence honesty | **ACKNOWLEDGED** | test_es_hs_lifecycle.c documents coupling to implementation literals (GAP-R3-1 OPEN); frozen-UTC seam declared test-only |
| Evidence retention | **SATISFACTORY** | Raw test outputs retained, check counts dynamically reported, anomalies printed with seed/iteration context |

---

## 2. SEVERITY COUNTS

| Severity | Count | Items |
|----------|-------|-------|
| **Major** | 0 | — |
| **Minor** | 3 | F-0467-01 (test_link_uart check count), F-0467-02 (reset-path 180 unsupported), F-0467-03 (review ID overstatement in S8-SCH row) |
| **Observation** | 4 | O-0467-01 (review index collisions), O-0467-02 (oracle coupling documented but open), O-0467-03 (frozen-UTC seam test-only), O-0467-04 (N18 FAIL / NC-PAL-01 correctly retained) |

---

## 3. FINDINGS

### F-0467-01 — Minor — Canonical table: S8-FDIR-01 row (line 723)
**test_link_uart.c check count discrepancy.**  
The table claims "test_link_uart.c (72)" checks. Fresh execution of `./tests/test_link_uart` in `build/gate11-test` reports **checks=77 failures=0**. The static `FF_TEST_ASSERT` count is 69; dynamic count is 77. The claimed 72 is not supported by either static or dynamic evidence.

**Evidence:** `build/gate11-test/tests/test_link_uart` output: `/home/nicola/fsw/frick-fsw/tests/unit/test_link_uart.c checks=77 failures=0`

**Recommendation:** Correct the table to 77 (dynamic) or 69 (static) with a footnote explaining the counting method.

---

### F-0467-02 — Minor — Canonical table: S8-FDIR-01 row (line 723)
**test_es_hs_lifecycle.c reset-path claim of 180 checks unsupported.**  
The table claims "test_es_hs_lifecycle.c reset-path (180)". The two reset-path test functions (`test_wd_feed_failure_reset_path` at line 851, `test_hs_reset_report_before_reset_ordering` at line 927) contain **22 `FF_TEST_ASSERT` statements** combined. The full test suite reports **208 checks** total. No test run produces a 180-check subset for the reset path alone.

**Evidence:** 
- `grep -c FF_TEST_ASSERT` on lines 851–979 → 22
- `./tests/test_es_hs_lifecycle` → `checks=208 failures=0`

**Recommendation:** Remove the "180" claim or clarify it refers to a different measurement (e.g., a prior iteration). If the reset-path coverage is meant to be tracked separately, add a dedicated test target or labeled sub-suite.

---

### F-0467-03 — Minor — Canonical table: S8-SCH row (line 724)
**Review ID list includes IDs not all confirmed relevant to S8-SCH.**  
The "Independent Reviews" cell lists: `FRICK-REVIEW-0351..0353, 0360, 0443, 0450/0451, 0454/0455, 0456/0457, 0458/0459, 0460/0461, 0462, 0463, 0464, 0465`. Reviews 0462–0465 exist in `docs/reviews/` but their direct relevance to S8-SCH (OBJ-31/M17 Scheduler) is not verified in this review. FRICK-REVIEW-0467-PA-20261105-DELTA.md (PA delta review of this same sprint-plan delta) flags the same issue.

**Evidence:** `ls docs/reviews/FRICK-REVIEW-046*` confirms 0462–0465 exist; content inspection needed for each to confirm S8-SCH scope.

**Recommendation:** Qualify each review ID with its scope (e.g., "0462 (PA reconciliation)") or restrict the list to reviews explicitly covering S8-SCH.

---

### O-0467-01 — Observation — Review index collisions
**Duplicate review IDs in `docs/reviews/README.md` index.**  
FRICK-REVIEW-0461-PA-20261104-CONFIRM.md documents: two distinct reports share ID 0438 (`2026-10-24-ncext18-s6-vv-review.md` and `2026-10-31-root-cause-audit-study-provenance-pa-review.md`); two share ID 0439 (`FRICK-REVIEW-0439-VV-NC-EXT-235.md` and `2026-10-31-renode-verdict-test-vv-review-0439.md`); 0437 exists with no index row; 0460 not indexed. This impairs traceability when reviews are cited by ID alone.

**Impact:** Low for this delta (context disambiguates), but systemic.

---

### O-0467-02 — Observation — Oracle coupling documented but open
**test_es_hs_lifecycle.c explicitly acknowledges oracle coupling.**  
The test header (lines 16–21) states: "The heartbeat event id (`0x0502`) and the 'HS: app %u failed' diagnostic text this harness matches on are implementation-defined literals of `src/apps/app_hs.c` (no controlled requirement/interface clause fixes them), so the event-identification oracle is coupled to the implementation under test... A controlled identifier/text contract would be required to remove that coupling. GAP-R3-1 stays OPEN."

**Assessment:** This is **correct V&V practice** — the limitation is declared, not hidden. The sprint plan correctly records "GAP-R3-1 stays OPEN" and does not claim independent oracle validity.

---

### O-0467-03 — Observation — Frozen-UTC seam declared test-only
**NC-EXT-235 correction uses `FF_TEST_HOOKS` frozen-UTC seam absent from flight/RTEMS builds.**  
The sprint plan (line 322) correctly states: "Correction: test-only frozen-UTC seam (`FF_TEST_HOOKS`, absent from flight/RTEMS builds) with inline cppcheck suppression." The seam is excluded from the RTEMS ELF (verified by `make rtems-docker` build). This is an honest characterization — not a weakening, but a documented test-harness concession with explicit fidelity gap.

---

### O-0467-04 — Observation — N18 FAIL and NC-PAL-01 correctly retained
**No false closure claimed for target validation.**  
The canonical table and NC register consistently record: N18 FAIL (NC-EXT-198), NC-PAL-01 (MPFS binding unwired), S8-FDIR-01 "BOUNDED INCREMENT / N18 FAIL", S8-SCH "SPRINT CLOSURE NOT RECOMMENDED / No target/HIL (N18 FAIL)". The XW-2 task (line 737) explicitly states: "N18 remains **FAIL** (NC-EXT-198, root cause NC-PAL-01); no device/reset-scope/independence/timing evidence exists."

**Assessment:** This is **correct V&V practice** — target validation gaps are not papered over.

---

## 4. TEST EVIDENCE VERIFICATION (FRESH EXECUTION)

All tests executed in fresh `build/gate11-test` (native host, Debug, gcc 15.2.0):

| Sprint | Test | Claimed Checks | Actual Checks (dynamic) | Static `FF_TEST_ASSERT` | Verdict |
|--------|------|----------------|------------------------|------------------------|---------|
| S8-VV-FU | test_es_hs_lifecycle.c | 208 | **208** | 163 | ✓ |
| S8-VV-FU | test_decoder_corpus.c | 14941 | **14941** | 75 | ✓ |
| S8-VV-FU | test_pal_watchdog.c | 320 | **320** | 85 | ✓ |
| S8-FDIR-01 | test_pal_watchdog.c | 320 | **320** | 85 | ✓ |
| S8-FDIR-01 | test_es_hs_lifecycle.c reset-path | 180 | **N/A (22 in reset functions, 208 total)** | 22 | ✗ F-0467-02 |
| S8-FDIR-01 | test_link_uart.c | 72 | **77** | 69 | ✗ F-0467-01 |
| S8-FDIR-01 | test_time | 113 | **113** | 113 | ✓ |
| S8-SCH | test_sched.c | 114 | **114** | 114 | ✓ |
| S8-SCH | test_pus_fu.c | 172 | **172** | 153 | ✓ |
| NC-EXT-236 | test_tm_output_chain.c | 47 | **47** | — | ✓ |

**Full test suite:** 53/53 tests PASS (ctest exit 0).

---

## 5. V&V REVIEW TRACEABILITY

| Sprint | Review IDs Claimed | Existence Verified | Notes |
|--------|-------------------|-------------------|-------|
| S4b | FRICK-REVIEW-0164 (R4), 0165/0166 (R1/R2) | Yes (in 2026-10-13-integration-manual-*-review.md) | Host scope only, correctly qualified |
| S8 | FRICK-REVIEW-0159/0160/0164 | Yes | Host scope only, correctly qualified |
| S8-VV-FU | FRICK-REVIEW-0379/0380 | Yes (in 2026-10-19-easy-fixes-*-review.md) | R2 core + R3 annex; V&V/PA "PASS WITH COMMENTS" |
| S8-FDIR-01 | FRICK-REVIEW-0405/0408/0413/0422/0423/0424/0425 | Yes (individual files in docs/reviews/) | 7 reviews covering R2, R4, PA, embedded safety, V&V |
| S8-SCH | FRICK-REVIEW-0351..0353, 0360, 0443, 0450/0451, 0454/0455, 0456/0457, 0458/0459, 0460/0461, **0462, 0463, 0464, 0465** | 0351–0461 yes; 0462–0465 exist but relevance unconfirmed | F-0467-03 |
| S9-S10 | FRICK-REVIEW-0438 (PA), 0439 (V&V), 0440 (PA) | Yes (2026-10-31-obcp-study-audit-pa-0440.md, FRICK-REVIEW-0439-VV-NC-EXT-235.md, 2026-10-31-root-cause-audit-study-provenance-pa-review.md) | 0438/0439/0440 have ID collisions (O-0467-01) |

---

## 6. V&V CRITERIA — NO WEAKENING DETECTED

| Criterion | Baseline (3947801) | Current (working tree) | Delta |
|-----------|-------------------|------------------------|-------|
| MC/DC required | Not claimed | Not claimed | — |
| Structural coverage targets | Not claimed | Not claimed | — |
| Test independence | N22 enforced | N22 enforced; frozen-UTC seam declared test-only | Honest disclosure |
| Oracle independence | GAP-R3-1 OPEN | GAP-R3-1 OPEN (documented in test header) | No change |
| N18 target validation | FAIL | FAIL (explicitly retained) | No false closure |
| T3 tool qualification | NOT FULLY QUALIFIED | NOT FULLY QUALIFIED (rev 1.3, FRICK-REVIEW-0443) | Honest retention |
| NC closure | Open NCs tracked | Open NCs tracked; NC-EXT-236 EVIDENCE DELIVERED (strengthened) | Improvement |

**No evidence of silently weakened acceptance criteria, coverage targets, independence requirements, or qualification claims.**

---

## 7. EVIDENCE RETENTION

- Raw test outputs: Available via `ctest --output-on-failure` and direct test executable runs (checks=XXX failures=0 format retained).
- Anomaly context: `test_decoder_corpus.c` prints `CORPUS ANOMALY seed=0x... template=X iter=Y` for every failure.
- Gate logs: `docs/vv/evidence/2026-11-03-gate-diagnosis/`, `2026-11-04-gate-reconciliation/` retained.
- Review records: All cited reviews present in `docs/reviews/` (modulo index collisions).
- Build artifacts: Fresh `build/gate11-test` created for this review; original `build/gate11` permission-blocked (root-owned).

---

## 8. CONCLUSION

The sprint-plan.md delta adds a **Canonical Sprint Status Summary table** that is **largely evidence-backed** with **two minor check-count discrepancies** (F-0467-01, F-0467-02) and **one review-ID overstatement** (F-0467-03). The document **honestly records** open gaps (N18 FAIL, NC-PAL-01, GAP-R3-1, T3 NOT FULLY QUALIFIED, frozen-UTC seam test-only) and **does not weaken** any V&V criterion. The NC-EXT-236 update from OPEN to EVIDENCE DELIVERED represents a genuine evidence improvement.

**Recommendation:** Correct the three Minor findings before next baseline. No blocker for continued development under N21 (functional bugs corrected; non-functional findings tracked as NCs).

---

**Report path:** `docs/reviews/FRICK-REVIEW-0467-VV-20261105-DELTA.md`  
**Reviewer:** Independent V&V agent (Nemotron 3 Ultra free)  
**Session:** Independent subagent invocation per N22/R2 mandate  
**Authority boundary:** This review reports findings only; it does not approve, close NCs, authorize commit/push, or waive gates. Human PA authority disposition required for NC closures.