# S8-PUS-FU S1 ingestion failure-path evidence — superseded r3 record

This is a historical evidence bundle for the candidate after the first R1/R3 assurance-basis correction. It is **superseded** by the authoritative `r4/` package; do not use r3 alone to support current gate claims. Earlier `r2/` material is also retained as historical evidence.

## Run identity

- Repository HEAD: `e5f01d402af382a78d5a8242fe6b1c4e2eb6bb83`.
- Test source at the time: `tests/unit/test_pus.c`, SHA-256 `dc8ea3dbae907c43f5897daa799ad792165604eca8cddd119612b2b3a6d889c7`; production `src/pus/pus.c` was byte-identical to HEAD.
- Build directory: `build/evidence-20261112-pus-s1-r3`.
- Configure option: `FF_USE_PUS_ASN1=ON`; see hash-bound `CMakeCache.txt`.
- Host sequence includes a successful configure, an interrupted first build, a retained recovery build, CTest and focused `test_pus`; the fully ordered successful configure/build/test sequence is in r4. Supplemental checks and the corrected N18 rerun are retained here and copied into r4.

## Results and limits

- Configure/build exit 0. Full build output retained; project C targets use `-Wall -Wextra -Werror`. ASN1SCC configure-time diagnostics are generator warnings and are not C compiler warnings.
- CTest: 57/57 registered tests passed. `test_pus_s11_residual` is declared but not registered and is excluded (NC-EXT-240).
- Focused `test_pus`: 322 checks, 0 failures.
- Lean, traceability, host clang-format 23.1.3, N13 license/GSW self-check and smoke: exit 0.
- **N18 evidence chronology:** the initial r3 `gate-renode.log` PASS used stale ELF SHA-256 `55188f5a…` and is invalid for current-candidate evidence. A subsequent Docker RTEMS-toolchain rebuild produced ELF SHA-256 `e36aee30f381df32d9b085b11d616e35082a9650263e3b8614ed8f4004843a41`; fresh `make renode` passed. Raw logs are retained in `renode-run/`. The current authoritative N18 record is the same copied run in r4, hash-bound by its manifest.
- Pinned clang-format 18 passes the changed `tests/unit/test_pus.c`. The full-repository pinned check fails on unchanged initializer formatting in `src/transport/link/link_tsn.c:18`, `src/transport/link/link_ip.c:19`, and `tests/unit/test_link_tsn.c:24`; recorded as NC-EXT-244, OPEN/deferred under N21. The targeted pass is not claimed as a full-repository pass.
- Mutation controls detect the selected wrong-code changes; source restoration is hash-verified.
- Host/Renode evidence only. No physical target, HIL, WCET, release or qualification claim.

## Integrity limitation

No `r3/assets.sha256` was generated. The assertions in this historical README are not hash-bound as an r3 package. The current candidate's sources, build/test artifacts, gate outputs, fresh N18 ELF/raw run logs and current-cycle documentation are bound by `r4/assets.sha256`; verify that manifest from the repository root.
