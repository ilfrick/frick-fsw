# Gate reconciliation and candidate evidence - 2026-11-04

Status: evidence package for the NC-EXT-235 / S8-SCH-M17 resumption cycle.
This package records what was run, with original exit codes, and what was
withdrawn. It does not close any NC and does not assert sprint completion.

## 1. Purpose

The preceding cycle left two unresolved claims in the handover records:

1. an NC-EXT-235 candidate gate reported as "51/55" with exit 0;
2. a later claim of "52/52, 100% passed" for `build/gate5`.

Both figures were re-checked against the retained tree and logs.

## 2. Result of the 51/55 re-check

- A repository-wide search for the strings `51/55`, `51 / 55`, `55 tests`,
  `51 of 55` and `51/55` over `*.log`, `*.md`, `*.txt` and `*.json` (excluding
  `build/`) returns no match.
- No retained log records 55 registered tests, and no retained log records a
  `ctest` run with 55 tests and exit code 0.
- Conclusion: the "51/55 with exit 0" figure is **not demonstrable** from any
  retained artifact. It is corrected here without inventing a replacement
  history: the claim is withdrawn as unsupported.

## 3. `build/gate5` provenance (important)

`build/gate5` was configured inside the V&V container, not on the host:

- `build/gate5/CMakeCache.txt` records `CMAKE_HOME_DIRECTORY:INTERNAL=/workspace`
  and `CMAKE_CACHEFILE_DIR:INTERNAL=/workspace/build/gate5`;
- `build/gate5/tests/CTestTestfile.cmake` registers test executables with
  absolute `/workspace/build/gate5/tests/...` paths;
- the directory and its artifacts are root-owned (created by the container).

Consequences:

- `ctest --test-dir build/gate5` **cannot** be run meaningfully on the host: the
  absolute `/workspace/...` test paths do not exist there, and the host user
  cannot create `Testing/Temporary/LastTest.log`. A host-side attempt in this
  cycle exited 8 with "Cannot create log file".
- Any `build/gate5` result is only valid when produced inside the container
  image `frick-vv:local` with the repository mounted at `/workspace`.

## 4. Gate run of the pre-existing `build/gate5` (as found)

Command (original exit codes captured, no pipes):

```
docker run --rm -v "$PWD":/workspace -w /workspace frick-vv:local \
  bash -lc "ctest --test-dir build/gate5 --output-on-failure"
```

- Log: `gate5-ctest-20261104T071844Z.log`
- Exit code: **8** (recorded in `gate5-ctest-exit.txt`)
- Registered tests: **53**
- Passed: 52
- Failed: 1 (`pus_mutation`)

So the "52/52, 100%" claim is also not reproducible against the tree as found.
The tree as found had 53 registered tests and one failure.

## 5. Withdrawn test: `tests/unit/test_pus_mutation.c`

### 5.1 What it was

An untracked, never-reviewed experiment ("PUS layer mutation testing,
NC-EXT-235 follow-on") registered in `tests/CMakeLists.txt` as target
`test_pus_mutation` and ctest case `pus_mutation`.

### 5.2 Why it was withdrawn

1. **It does not compile at the revision on disk.**
   `gcc -std=c11 -fsyntax-only -Wall -Wextra -Werror` reports 8 errors:
   `hk_test_setup` and `hk_send_tc` are used before declaration (implicit
   declaration), `dummy_hk_provider` is defined twice, and `hk_send_tc` /
   `hk_test_setup` are declared implicitly then defined `static`. See
   `withdrawn/syntax-check.log` (exit 1). The failing ctest binary predates the
   last source edit, so the failure line numbers below refer to the older
   revision that was actually compiled, not to the file as archived.
2. **Its expected results do not match the product contract.**
   The stale binary reported 13 failures out of 1203 checks, for example
   expecting `FF_PusRegisterHkProvider(0u, ...)` and `(255u, ...)` to be
   rejected when the implementation accepts the full `uint8_t` id range, and
   expecting `FF_PusEmitPeriodicHk()` to return an error/success alternation
   that the service does not implement that way. Encoding oracles that are
   reverse-engineered from the implementation would be tautological; deriving
   them from requirements is a larger, separately scoped task.
3. **Part of it duplicates existing coverage.**
   The single-bit decoder mutation sweep already exists in
   `test_decoder_corpus.c`, which is registered and passing.
4. **It broke the gate.** It was the only failing test in the 53-test set.

No requirement in `docs/requirements/traceability.yaml` referenced
`test_pus_mutation.c`; a repository-wide search found no other reference. Its
withdrawal therefore removes no claimed verification coverage.

### 5.3 Disposition

- Source preserved verbatim at `withdrawn/test_pus_mutation.c`
  (sha256 `09a7a62c086475b48ac789b97f6cf79ea2a91914badc685d2ca40f54a1231cd9`,
  identical to the file removed from `tests/unit/`).
- Target and ctest registration removed from `tests/CMakeLists.txt`.
- Recorded as a test-tooling defect for later disposition; it is **not** a
  functional product defect and is not presented as coverage.

## 6. `test_frozen_utc_seam` wiring correction

`test_frozen_utc_seam` was registered inside `if(FF_USE_LWIP)`. The test only
uses `core/pal/pal.h` and `core/time/time.h` and has no lwIP dependency, so the
registration was moved to the platform-independent test list. `FF_TEST_HOOKS`
is exported `PUBLIC` by `frick_fsw_core` whenever `FF_BUILD_TESTS` is on and the
platform is not RTEMS (`CMakeLists.txt`), so the seam symbol is available to
every test target in the same configurations.

## 7. Files in this package

| File | Content |
| --- | --- |
| `README.md` | this record |
| `gate5-ctest-20261104T071844Z.log` | pre-existing gate5 run, as found |
| `gate5-ctest-exit.txt` | original exit code of that run |
| `ctest-N.txt` | host-side `ctest -N` attempt (fails: container-only build dir) |
| `withdrawn/test_pus_mutation.c` | withdrawn test, preserved verbatim |
| `withdrawn/syntax-check.log` | proof the withdrawn source does not compile |

Subsequent clean-build gate evidence for the corrected candidate is stored in
sibling files prefixed `candidate-`.
