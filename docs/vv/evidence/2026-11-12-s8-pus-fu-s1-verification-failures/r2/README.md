# S8-PUS-FU S1 ingestion failure-path evidence — correction round 2

This is the authoritative retained gate run for the corrected candidate after the independent PA and documentation findings were addressed. Earlier top-level logs in the parent directory are retained as historical evidence from the first candidate; they are **not** the basis for the current claims below.

## Run identity

- Repository HEAD: `e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83`.
- Working-tree candidate: test file SHA-256 is recorded in `assets.sha256`; production `src/pus/pus.c` remains byte-identical to HEAD.
- Build directory: `build/evidence-20261112-pus-s1-r2`.
- Configure option: `FF_USE_PUS_ASN1=ON` (`CMakeCache.txt` is hash-bound in the manifest).
- Sequence: configure → full build → CTest → focused `test_pus`; Lean, traceability, format, N13 license check, N18 Renode and mutation controls are retained separately.
- The initial CTest invocation was interrupted by the execution tool; see `interrupted-ctest-attempt.md`. It is not represented as a verdict. `gate-ctest.log` is the completed recovery run.

## Results and limits

- Configure/build: exit 0. Full build output retained. No C compiler warning lines were observed in that build log. Project C targets are compiled with `-Wall -Wextra -Werror`; configure-time ASN1SCC endianness diagnostics are generator warnings, not C compiler warnings.
- CTest: 57/57 registered tests passed. `test_pus_s11_residual` is not registered with CTest and is not included; see NC-EXT-240.
- Focused test: 322 checks, 0 failures.
- Lean, traceability, N13 license/GSW self-check, smoke and N18 Renode: exit 0; see individual gate logs.
- Host clang-format 23.1.3 passes the broad dry-run. Pinned clang-format 18 passes the changed `tests/unit/test_pus.c`, but the full repository check fails on unchanged initializers at `src/transport/link/link_tsn.c:18`, `src/transport/link/link_ip.c:19` and `tests/unit/test_link_tsn.c:24`; this is recorded as open NC-EXT-244, deferred under N21. Do not interpret the targeted format pass as a full-repository pinned-format pass.
- Mutation controls: both selected wrong-code mutations are detected and the source is restored; details and hashes are in `mutation-check.log`.
- Host and Renode evidence only. No physical target, RTEMS runtime, HIL, timing, or qualification claim.

## Integrity binding

`assets.sha256` binds the source/test/document candidate, build cache, `test_pus` executable, retained gate outputs, mutation record and this interruption record by SHA-256. The manifest intentionally does not hash itself or this README, avoiding a circular digest claim. Regenerate and independently verify the manifest if any listed input or evidence changes.
