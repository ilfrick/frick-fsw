# S8-FU Engineering/V&V Delta Re-review

| | |
| ---: | :--- |
| Document id | FRICK-REVIEW-0173 |
| Reviewer session | Independent agents, fresh context (N9) |
| Reviewer profile | R1 (Senior FSW) + R2 (Senior SW V&V) |
| Reference revision | `57974d2` + uncommitted current S8-FU working tree |
| Items under review | Post-FRICK-REVIEW-0170 parser correction, CCSDS decoder acceptance fields, boundary tests and evidence |
| Date | 2026-10-13 |
| Scope | Re-review only; no flight-use or release authorization |
| Independence declaration | Reviewer reported fresh context and no authorship/modification of reviewed files |
| Norms applied | N1-N20 (see `docs/rules.md`) |

## 1. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | --- | --- | --- | --- | --- |
| 1 | Minor | NC-EXT-12 / GSW finite-buffer parsing | The new one-octet resynchronization prevents an incomplete PDL=0xFFFF candidate from hiding a following valid TM packet. The test independently passes 5/5. This verifies the bounded finite-buffer case, not incremental live framing or overall T7 qualification. | `tools/gsw/ff_gsw.py`; `tests/python/test_ff_gsw.py` | Keep NC-EXT-12 open until T7 qualification/PA delta review is complete; external E2E remains under NC-EXT-03. |
| 2 | Major | NC-EXT-13 / CCSDS primary-header validation | TC/TM decoders do not reject unsupported primary-header version, secondary-header flag, or sequence flags; no controlled profile decision or explicit acceptance requirement was identified. | `src/transport/ccsds/ccsds.c`; decoder tests | Obtain approved CCSDS/mission profile, decide required policy and add negative tests; do not guess tailoring. |
| 3 | Minor | NC-EXT-02 / maximum-size ingest | Codec maximum-payload round-trip and buffer sizing pass, but `FF_AppCmdIngestMain` path is not exercised end-to-end at maximum size. | `src/apps/app_cmd_ingest.c`; `tests/unit/test_r2_gap.c` | Keep the ingest evidence gap open or add a bounded integration test. |
| 4 | Minor | NC-EXT-03 / GSW interoperability | Local vectors are not evidence of independent ground-segment interoperability. | GSW test/tool register | Keep T7 claim bounded; obtain external E2E evidence before interoperability claim. |

## 2. Reproduced evidence

- Python unit suite: 5/5 PASS.
- Selected native CCSDS and `r2_gap` tests: PASS.
- Selected ASan/UBSan tests: PASS.
- Broader current-cycle evidence is recorded in `docs/vv/verification-results.md`;
  forge CI and independent ground-segment E2E were not run.

## 3. Conclusion

**PARTIAL PASS.** The finite-buffer parser correction is verified for the tested
case. NC-EXT-12 remains open for the complete qualification/assurance closure
loop; NC-EXT-13 is newly opened for authority/profile disposition. NC-EXT-02/03
and other findings remain open. No flight-use authorization or NC closure is
granted.
