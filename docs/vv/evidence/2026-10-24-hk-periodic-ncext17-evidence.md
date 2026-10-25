# HK periodic emission (NC-EXT-17) - reproduced defect and final fix evidence

Date: 2026-10-24
Tree: /home/nicola/fsw/frick-fsw (uncommitted working tree)
Constraint reminder: N18 Renode gate remains FAIL (NC-EXT-198); NC-PAL-01 remains OPEN.
No emulator, no hardware and no mission parameter values were introduced.
No NC was closed, no frozen manifest refreshed, no commit/push performed.

## 1. Defect reproduced by strengthening the test (V&V-FIND-001 / SF-08)

The two independent delta reviews (`docs/reviews/2026-10-24-hk-periodic-vv-delta-review.md`
and `docs/reviews/2026-10-24-hk-periodic-safety-delta-review.md`) both recorded that the
*positive* time-due emission path was never exercised, so the previously claimed
SF-04 / V&V-FIND-004 fix could not be confirmed by test evidence.

A discriminating positive-path test was added to `tests/unit/test_pus.c`
(`test_hk_periodic_emission`) using a synthetic fixture base time (`FF_TimeSetUtcNs`,
a test value, not a mission time parameter) and an exact occurrence-count oracle
(`stream_count(3,25)`).

Observed failure against the candidate reviewed by the delta reviews
(`src/pus/pus_services.c` sha256 `6f435d8f...`):

```
FAIL tests/unit/test_pus.c:472: FF_PusEmitPeriodicHk() == FF_SUCCESS
FAIL tests/unit/test_pus.c:473: stream_count(3u, 25u) == 1u
tests/unit/test_pus.c checks=108 failures=2
```

Root cause: `time_add_period()` added `period_s` to the coarse field *and* carried the
period scaled into the fine field, double-counting the period. With a 10 s period and a
base of 1000 s the due instant computed as 1020 s instead of 1010 s, so the report never
became due at the correct instant. This is a functional defect, not a test artefact: the
periodic emission cadence would have been one full period late for every report.

## 2. Fix applied (minimal, within delegated software authority)

`src/pus/pus_services.c`, `time_add_period()`:

```diff
     uint64_t total_fine = (uint64_t)fine + ((uint64_t)period_s * 65536ULL);
-    *out_coarse = coarse + period_s + (uint32_t)(total_fine >> 16);
+    *out_coarse = coarse + (uint32_t)(total_fine >> 16);
     *out_fine = (uint16_t)(total_fine & 0xFFFFu);
```

The period scaled into the fine field already carries into the coarse field, so the
explicit `period_s` addition was removed. Coarse modular wrap and fine-carry semantics
are unchanged.

## 3. Post-fix gate results (affected gates only)

| Gate | Result |
| --- | --- |
| `make build` | PASS |
| `make test` | PASS - 100% tests passed, 0 failed out of 46 |
| `test_pus` | PASS - checks=108 failures=0 |
| `test_pus_fu` | PASS |
| `test_r2_gap` | PASS |
| `test_r3_gap` | PASS |
| `make format-check` | PASS (no `clang-format --dry-run --Werror` output) |
| `make trace` | PASS (120 requirements, 76 srs, 30 sss, 14 gaps, 68 lean theorems) - unaffected, re-run for the record |
| `make verify-lean` | PASS (`lake build` completed successfully) - unaffected, re-run for the record |
| `make gsw-selfcheck` | PASS (12+9+14 python checks OK; `license_check` OK; GSW ping frame self-check OK) |
| `make renode` (N18) | **FAIL - unchanged**, per owner directive (NC-EXT-198). Not run in this cycle. |
| RTEMS cross-build | NOT RUN in this cycle (`make rtems` host toolchain not exercised here) |

## 4. Final candidate hashes (SHA-256)

```
49bc86db75762cc4face23012cd110fc921d1176fb39251f92dcb7842f6cc3cf  src/pus/pus_services.c
e520bb33fa40082aa8dbe79acd13e4f525370fbda5fc4c4c67a887e27b7345fd  src/pus/pus.h
57db92de95a464aa551b6eff22bf6ece8bd120cb7dfef7e37024a20ad0eb5789  tests/unit/test_pus.c
4e7177266062288e54efc12a105a66c886cf667a551971b29a3ddb8b61c2981f  tests/unit/test_pus_fu.c
6111cd610b06039eb9140c5b53ecb42875867057fad0245fb3823fef12abfab4  tests/unit/test_r2_gap.c
4c7d6c38e37a67d2cac9ca880d1e58c4420aebcb087c1805ab5b0dfd28c31260  tests/unit/test_r3_gap.c
```

## 5. Retained independent review reports (SHA-256)

```
2e615b8c50c94d7ca7b06656a2042c43f60b42b4c12fc1893581914c92ff039b  docs/reviews/2026-10-24-hk-periodic-vv-review.md
4c1d7392c7fca9e3c70b00349e20baec3b02a3f8c3c0e14a8b796da64027bbd4  docs/reviews/2026-10-24-hk-periodic-safety-review.md
1d214650e205c1e7d91218d599f7d9dfd036932f7eb6e4389f563765028c594b  docs/reviews/2026-10-24-hk-periodic-vv-delta-review.md
4bb5120f9cf160e77a8eb6c8f4a31c437b34f0321abcb7f0828556ed2ce0a799  docs/reviews/2026-10-24-hk-periodic-safety-delta-review.md
```

## 6. Authority boundary (unchanged by this increment)

- The 6-byte `TC[3,1]` payload is an authority-pending private extension (ICD §4 defines
  2 bytes). No ECSS compliance is claimed for this wire format.
- `FF_PusEmitPeriodicHk()` still has no scheduler/application caller; that integration is
  authority-pending (ESC-HK-02) and is not a defect of this increment.
- Periodic HK emission has no requirements/traceability entry yet (ESC-HK-05).
- Hazard analysis for periodic HK emission has not been performed (ESC-HK-03).

## 7. Revision history

| Rev | Date | Author | Change |
| --- | --- | --- | --- |
| 0.1 | 2026-10-24 | frick-fsw (delegated) | Initial record: reproduced `time_add_period` double-count defect, applied minimal fix, recorded affected-gate results and candidate hashes. |
