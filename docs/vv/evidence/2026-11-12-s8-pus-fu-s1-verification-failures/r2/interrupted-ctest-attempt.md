# Interrupted CTest attempt (retained)

- Candidate: working tree after the S1 test correction round; repository HEAD `e5f01d4`.
- Build directory: `build/evidence-20261112-pus-s1-r2`.
- Attempt: `ctest --test-dir build/evidence-20261112-pus-s1-r2 --output-on-failure`.
- Observed output before tool interruption: tests 1–4 passed; test 5 (`sb_evs_structural_sweep`) had started.
- The execution tool returned `aborted`; no CTest exit status or complete output was returned. This is an interrupted evidence attempt, **not a PASS or a CTest failure verdict**.
- Recovery: CTest was run again to completion in the same configured/built directory. The complete result and `CTEST_EXIT=0` are in `gate-ctest.log` (57/57 passed).
- Limitation: the tool did not expose the full transient output, so this record preserves the available transcript and explicitly records the missing exit status rather than reconstructing one.
