# GEM TSN mock-fixture sanitizer review (FRICK-REVIEW-0318)

| Field | Value |
| --- | --- |
| Profile | Independent SW V&V review |
| Date | 2026-10-15 |
| Scope | NC-EXT-09; `tests/unit/test_gem_tsn.c` mock register-array extent vs exercised ENST offsets |
| Baseline | Source HEAD `13ebf92` plus uncommitted 4096-byte fixture correction |
| Verdict | **ACCEPTED for the exercised host test fixture only** |

## Evidence

- `MOCK_REGS` is 4096 bytes (1024 `uint32_t` words). The exercised queue-3
  ENST accesses include start `0x080C`, on-time `0x082C`, off-time `0x084C`,
  and control `0x0880`; the highest 4-byte access ends at `0x0884`, within the
  fixture. The test mock callbacks themselves do not bounds-check offsets.
- A fresh out-of-tree Debug AddressSanitizer build was configured from the
  candidate tree with `-fsanitize=address -fno-omit-frame-pointer`.
- `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ctest --test-dir build/s8-asan
  --output-on-failure -R '^gem_tsn$'` passed 1/1; direct executable reported
  22 checks, 0 failures.

## Disposition and limits

The corrected extent resolves the previously undersized fixture for the
register offsets exercised by `test_gem_tsn`; accept NC-EXT-09 at that bounded
host-test scope. This is not evidence for untested offsets, the GEM hardware,
Renode fidelity, or target execution. The independent review found no basis to
claim a full ASan/UBSan suite pass from this focused run.
