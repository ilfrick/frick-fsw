# Review Report - milestone M9 (code quality: MISRA C:2012 + coding style) and the lwIP review fixes

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0034                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (SW dev) / R2 (SW V&V) / R4 (SW documentation, N17) |
| Reference revision                | base commit `e3176bd` + uncommitted working tree (M9 code-quality delta + lwIP review fixes, 2026-10-10) |
| Items under review                | `src/hal/net/lwip/ff_net_lwip.c`, `src/hal/net/lwip/lwipopts.h`, `tests/unit/test_lwip.c`; `docs/engineering/coding-style.md`, `.clang-format`; `docs/quality/misra-c.md`, `tools/scripts/misra.sh`; `CMakeLists.txt`, `tests/CMakeLists.txt`, `Makefile`, `tools/docker/Dockerfile`, `.github/workflows/ci.yml`, `.gitea/workflows/ci.yml`; `docs/requirements/{SSS.md,SRS.md,traceability.yaml,traceability.md}`; `docs/architecture/network-stack.md`; `README.md`; `docs/development-plan.md`, `docs/roadmap.md`, `docs/status/status.json`; `docs/vv/{verification-results.md,tool-qualification.md}`; `docs/quality/{class-a-gaps.md,ecss-compliance-matrix.md}`; `CHANGELOG.md` |
| Date                              | 2026-10-10                             |
| Scope                             | (a) Correctness of the lwIP adapter review fixes (address validation in `bind`/`connect`; `recv` bound via `SO_RCVTIMEO`/`LWIP_SO_RCVTIMEO`) and their tests. (b) The M9 code-quality delta: honesty and enforceability of the declared C style (`docs/engineering/coding-style.md` + normative `.clang-format`) and of the MISRA C:2012 process/tooling (`docs/quality/misra-c.md` + `tools/scripts/misra.sh`, cppcheck addon); deviation process; requirement quality/traceability (N3/N4); documentation completeness/correctness/consistency and counts vs the trace gate (N8/N16/N17). ECSS-E-ST-40C, ECSS-Q-ST-80C; norms N1-N17. |
| Independence declaration          | Reviewer took no part in authoring, implementing or executing any reviewed item; fresh context, independent session, report-only (no repository file other than this report was modified). |
| Norms applied                     | N1-N17 (see `docs/rules.md`)           |

## 1. Scope

Reviewed the delta against base commit `e3176bd` and the uncommitted working
tree under three independent profiles:

- **R1 (design/code)**: the two lwIP review fixes (prior `R1-01` timeout,
  `R1-02` address validation) for correctness against the `FF_NetStack_t`
  contract (`src/hal/hal_net.h`, SRS-REQ-027/040) and the sibling loopback
  backend (SRS-REQ-039); the `lwipopts.h` change; the CMake wiring and the
  accuracy of the "strict warnings" claim.
- **R2 (V&V)**: validity, coverage and honesty of `tests/unit/test_lwip.c`; the
  recorded check counts; the MISRA tooling honesty (cppcheck is not a certified
  MISRA checker) and whether the advisory/blocking statements match what the
  script and CI actually do; the requirement/evidence quality for SRS-REQ-041/042
  and their traceability.
- **R4 (documentation, N17)**: completeness, correctness and consistency of the
  new/touched documentation: the declared style vs the normative `.clang-format`,
  the MISRA process/deviations, README counts and doc index, `status.json`
  milestones, requirement/verification-record counts vs the trace gate, and
  cross-references.

Executed in this session (host, Linux; independent of the author):

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` ->
  **PASS** (`78 requirements, 43 srs, 21 sss, 14 gaps, 51 lean theorems
  checked`; `CI evidence claims: 10`).
- `cmake -S . -B /tmp/opencode/build-native -G Ninja -DCMAKE_BUILD_TYPE=Debug &&
  cmake --build ... && ctest --output-on-failure` -> **11/11 PASS**; no lwIP
  target/test in the default build.
- `cmake -S . -B /tmp/opencode/build-lwip -G Ninja -DCMAKE_BUILD_TYPE=Debug
  -DFF_USE_LWIP=ON && cmake --build ... && ctest -R lwip` -> build **148/148**,
  test **1/1 PASS**; `test_lwip` reports **`checks=19 failures=0`**.
  All 11 default suites re-run: 15+20+9+29+101+31+20+81+23+16+29 = **374**
  checks, all `failures=0` (matches the recorded M4 row).
- **Format gate**: obtained `clang-format` (pip, v23.1.3) and ran the exact CI
  command `find src config tests -name '*.[ch]' -print0 | xargs -0 -n1
  clang-format --dry-run --Werror` -> **exit 0, clean** over the whole authored
  codebase.
- `python3 tools/scripts/metrics.py` -> `78 (srs=43, sss=21, gap=14)`,
  tests `64/78`, Lean `20/78` - which **differ** from the README panel (R4-01).
- `bash tools/scripts/misra.sh` -> `cppcheck not installed ...` **exit 0**
  (the advisory no-op path); `bash -n tools/scripts/misra.sh` syntax OK.
- CI `# evidence:` claim extraction (reproduced) -> **10 unique claims**
  (`ci-pipeline, docs, dual-build, host-build, lean, misra, rtems-cross-build,
  simulator-smoke, static-analysis, trace`), i.e. 9 referenced by the manifest
  - which contradicts `traceability.md:19` (R4-03).
- Document-id uniqueness scan (the CI `docs` job logic) -> `73 unique document
  ids, 0 duplicates`; the three new ids (`FRICK-ARCH-0012`, `FRICK-ENG-0006`,
  `FRICK-QUAL-MISRA-001`) are unique.
- `git diff` of all tracked deltas + direct byte/line inspection of the new
  files; trailing-newline check on touched files.

Excluded / not executable in this environment: the MISRA scan itself
(`cppcheck` is not installed and `apt-get` is not permitted here), so
`misra.sh` was exercised only on its no-op path and by inspection. `lake build`
was not re-run (Lean theorem links are checked by the trace gate). On-target
lwIP/PHY behaviour and a real hardware netif are documented GAP-A5 and were not
executed. The format gate was run with clang-format 23.1.3 (pip) rather than the
distro clang-format pinned in `tools/docker/Dockerfile`; the local result is
therefore strong evidence but not a byte-identical reproduction of CI.

## 2. References

- Project norms: `docs/rules.md` (N1-N17)
- HAL network contract: `src/hal/hal_net.h` (SRS-REQ-027)
- Reference backend: `src/drivers/net/ff_net_loopback.c` (SRS-REQ-039)
- Traceability manifest: `docs/requirements/traceability.yaml`
- Reviewer profiles: `docs/engineering/independent-review.md`
- Predecessor increment: `docs/reviews/2026-10-10-m4-lwip-r1-r2-r4-review.md`
  (`FRICK-REVIEW-0033`, open residuals R1-01..R1-05, R2-01..R2-03, R4-01..R4-07)
- ECSS-E-ST-40C, ECSS-Q-ST-80C

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| R1-01 | Minor | `src/hal/net/lwip/ff_net_lwip.c` (`ff_lwip_recv`), `docs/architecture/network-stack.md`; SRS-REQ-040 / `hal_net.h` | **The `recv` bound was added but its semantics are inconsistent and contradict the new design doc.** The fix sets `SO_RCVTIMEO` only when `timeout_ms > 0` and maps the expiry to `FF_ERROR`; when `timeout_ms == 0` it does not touch the option, so on a fresh socket the call **blocks indefinitely** (lwIP default) and, because `SO_RCVTIMEO` is sticky and never cleared, a later `timeout_ms == 0` silently inherits the previous timeout. The contract/design doc states the opposite: `docs/architecture/network-stack.md` §2 says `recv` "is bounded by `timeout_ms` (0 = non-blocking) and returns `FF_SUCCESS` with `*got == 0` when nothing is pending". The sibling loopback backend also ignores `timeout_ms` and is always non-blocking (`*got == 0`), so the two backends of the same vtable implement `timeout_ms` differently. | `ff_net_lwip.c:109-128` (guard `timeout_ms > 0u`, `(void)lwip_setsockopt(...)` unchecked, `return FF_ERROR` on `n < 0`); `network-stack.md:24-26`; `ff_net_loopback.c:173-192` (`(void)timeout_ms; /* loopback is non-blocking */`); `hal_net.h:30,38-39` ("blocking, bounded"). | Define `timeout_ms` once (0 = non-blocking vs 0 = default-blocking), make loopback + lwIP + the design doc agree, and clear/restore `SO_RCVTIMEO` when `timeout_ms == 0`; map the timeout `EAGAIN`/`EWOULDBLOCK` to a documented status. |
| R1-02 | Comment | `src/hal/net/lwip/ff_net_lwip.c` (`ff_lwip_send`, `ff_lwip_recv`); SRS-REQ-040 | **Residual input-validation asymmetry with the reference backend.** `send` validates only `sent != NULL` and `recv` only `got != NULL`; neither rejects `buf == NULL`, and neither rejects an invalid/unopened descriptor. The loopback backend returns `FF_INVALID_ARGUMENT` for `buf == NULL` and for a bad handle, so a `NULL` buffer is passed straight into lwIP here. (The prior `R1-02` address validation itself is correctly fixed - see positive evidence.) | `ff_net_lwip.c:95-107,109-128` vs `ff_net_loopback.c:150-171,173-192` (`buf == NULL`/`s == NULL` -> `FF_INVALID_ARGUMENT`). | Validate `buf != NULL` (and optionally the descriptor) to match the loopback backend and the contract's defensive style. |
| R1-03 | Minor | `CMakeLists.txt:142`, `docs/engineering/coding-style.md`; SRS-REQ-042 | **The "strict warnings" claim remains false and the new style doc repeats it.** `CMakeLists.txt` still says "Our adapter, compiled with the project's strict warnings", but no `-Wall`/`-Wextra` (and no `-Werror`) is applied to `frick_net_lwip` or to any first-party target; `-Werror` reaches only `frick_fsw_core` under `FF_ENABLE_WERROR` (OFF by default). `docs/engineering/coding-style.md` §2 asserts "C99, `-Wall -Wextra` clean (CI also runs clang-tidy and MISRA)", yet the build applies none of `-Wall`/`-Wextra`, and the CI clang-tidy/MISRA steps are non-blocking (`|| true`; `misra.sh` always exits 0). This is the recurrence of the predecessor's open `R1-05`. | `CMakeLists.txt:142,151`; `ninja -C build-lwip -t commands frick_net_lwip` carries only `-std=gnu99`; `coding-style.md:32`; `.github/workflows/ci.yml:87-90` (`clang-tidy ... || true`). | Apply `-Wall -Wextra` (and, with `FF_ENABLE_WERROR`, `-Werror`) to the first-party targets including `frick_net_lwip`, or correct the comment and the style doc to state the real warning gate. |
| R1-04 | Comment | `src/hal/net/lwip/ff_net_lwip.c` (`ff_lwip_attach_netif`, `ff_lwip_init`) | Prior `R1-03`/`R1-04` are only partly dispositioned. The new design doc now describes the netif route ("the adapter assumes a mission-provided netif on the target (`attach_netif`)"), which is an improvement; but `attach_netif` is still a no-op and `ff_lwip_init`'s lazy `static int g_inited` is still non-atomic with no `deinit`, and the header still does not state the "init once / single-threaded / no deinit" lifetime contract. | `ff_net_lwip.c:29-44`; `ff_net_lwip.h:1-19`; `network-stack.md:53-56`. | State the init/lifetime assumptions in `ff_net_lwip.h`, or guard with the platform once primitive. |
| R2-01 | Minor | `tests/unit/test_lwip.c`; `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md`, `CHANGELOG.md`, `docs/architecture/network-stack.md`; SRS-REQ-040 | **The recorded check count is stale and the added negative cases only partly close the predecessor's `R2-01`.** The suite now reports **19 checks** (predecessor: 15) and the two new cases assert short-address rejection for `connect`/`bind`; but every document still records **15 checks** (`CHANGELOG.md:104`, `tool-qualification.md:80`, `verification-results.md:27`, `network-stack.md:49`), so the committed verification evidence contradicts the artifact (N10/N11). Still untested: `NULL` address, `NULL` out-param/`NULL` buffer, invalid descriptor, and the `recv` timeout path (R1-01); and the `accept` `peer`/`peer_len` address decode (`ntohs`) is still never asserted (`peer` is declared and passed but never checked). | `test_lwip.c:28-29,43,48,56-63` (`peer` unused after `accept`); `test_lwip` -> `checks=19 failures=0`; docs say "15 checks". | Update all check-count references to 19 and add cases for `NULL`/bad-handle rejection, the bounded-`recv` timeout, and the `accept` peer address (127.0.0.1 + big-endian port). |
| R2-02 | Comment | `tests/unit/test_lwip.c:17-19` | The test still re-declares `T_AF_INET 2` / `T_SOCK_STREAM 1` to avoid lwIP headers, although it links lwIP transitively - the predecessor's open `R2-02`. | `test_lwip.c:17-19,36,40`. | Include `lwip/sockets.h` and use `AF_INET`/`SOCK_STREAM`, or justify the literals. |
| R2-03 | Minor | `tools/scripts/misra.sh`; `docs/quality/misra-c.md`; SRS-REQ-041 | **Tooling honesty is good, but the script's own contract is inaccurate.** `misra-c.md` correctly states that cppcheck "**is not a qualified MISRA checker**" and is used "advisory-then-blocking", with certified tooling + a formal report tracked as GAP-A4 - that is honest and adequate. However `misra.sh`'s header documents "Exit code: number of reported findings (0 = clean), capped for CI" while the script unconditionally does `exit 0`, so the scan can never block CI and the documented exit contract is dead. The deviation table is still empty ("(none yet)") and no baseline/current violation count is recorded anywhere. | `misra.sh:6,22-31`; `misra-c.md:26,29-31,39-54`; `bash tools/scripts/misra.sh` -> `cppcheck not installed ...` exit 0. | Fix the script header to match (`always exit 0 while advisory`), and record the agreed violation baseline before flipping the gate to blocking. |
| R2-04 | Comment | CI workflows (`build` job); SRS-REQ-040 | The lwIP build/test still runs as a separate step without AddressSanitizer, so the new `recv`/`setsockopt` path carries no sanitizer evidence (predecessor's open `R2-03`). | `.github/workflows/ci.yml:51-64` (ASan precedes the lwIP step); `verification-results.md:27` reports only `clang-format clean`. | Run the `FF_USE_LWIP=ON` build + `test_lwip` under ASan and record it. |
| R2-05 | Comment | `docs/requirements/SRS.md` (SRS-REQ-041); manifest | SRS-REQ-041 ("the C sources **shall be MISRA C:2012 compliant**") is not verifiable with the available tooling; the manifest's `tests: ["ci:misra"]` points at an advisory, non-certified scan and the `lean_note`/GAP-A4 disclose the limitation, so the claim is honest - but the "blocking" state is aspirational and the requirement reads as a compliance statement stronger than the evidence supports. | `SRS.md` (SRS-REQ-041); `traceability.yaml:390-397`; `misra-c.md:19-20,39-41`; `class-a-gaps.md` (GAP-A4). | Either phrase SRS-REQ-041 as a *process* requirement (scan + documented deviations) or keep the compliance wording but make the GAP-A4 reference explicit in the requirement/`lean_note`. |
| R4-01 | Minor | `README.md:72-76` (metrics panel) vs `:247-248` | **The generated status panel is stale again (recurrence of predecessor `R4-01`).** The panel still reports "Requirements in manifest = 74 (srs=40, sss=20, gap=14)", "Test coverage 60/74", "Formal verification 20/74", while `metrics.py`/`trace.py` compute `78 (srs=43, sss=21, gap=14)`, `64/78`, `20/78`; the next section calls the panel "authoritative". Only the single requirement-count line was edited. | `README.md:72-75` vs `:247`; `python3 tools/scripts/metrics.py` -> 78/43/21/14, 64/78, 20/78; `trace.py` -> 78 requirements; `git diff README.md` (one line changed). | Regenerate the panel with `python3 tools/scripts/metrics.py --readme` in the same commit. |
| R4-02 | Minor | `docs/vv/verification-results.md:27,176-177,263-267` | **Stale M4/M9 verification records (recurrence of predecessor `R4-03`).** The M4+lwIP row still says "**75 reqs / 41 srs / 20 sss** ... **9 CI claims**" and "`test_lwip` **15 checks**" (actual: 78/43/21/14, 10 claims, 19 checks); there is **no M9 verification row/note** for SRS-REQ-041/042; and the §"Remaining (M4)" list still names the delivered "lwIP `FF_NetStack_t` adapter + GEM TSN"; the coverage paragraph cites only `test_netloop`. | `verification-results.md:27,176-177,261,263-267`; `trace.py` (78/43/21/14, 10 claims); `test_lwip` checks=19. | Add the M9 row/note, refresh the counts to 78/43/21/14 + 10 claims, drop the delivered items from "Remaining", and add `test_lwip` to the coverage paragraph. |
| R4-03 | Minor | `docs/requirements/traceability.md:19,92` | **The manual trace doc is stale and incomplete vs the authoritative YAML.** Line 19 still says "CI evidence claims = **9 unique** (8 referenced)" while the gate now finds **10** (9 referenced - `ci:misra`); the SRS-REQ-040 row (line 92) still omits `lwipopts.h`, which the YAML lists (predecessor's open `R4-04`). | `traceability.md:19,92` vs `traceability.yaml:384`; reproduced claim extraction (10 unique). | State 10 claims (9 referenced) and add `lwipopts.h` to the SRS-REQ-040 implementation cell. |
| R4-04 | Minor | `README.md:274-298` (doc index) | **The three new documents are not linked from the README documentation index** (nor from the CI `required` list semantics): `docs/architecture/network-stack.md`, `docs/engineering/coding-style.md`, `docs/quality/misra-c.md` are absent from the Architecture/Quality/Engineering-guides bullet lists (N8 completeness); the CI `docs` step is named "exist and are linked" but only checks existence, so the omission is not caught. | `README.md:274-298`; CI `docs` job `required` list (`.github/workflows/ci.yml:141-166`) checks `Path(p).exists()` only. | Add the three links to the README index (and, if desired, teach the docs gate to check linkage). |
| R4-05 | Minor | `docs/status/status.json:8-16` | **Milestone M9 is missing from `status.json`.** The plan (`docs/development-plan.md` §M9), the roadmap (§M9 / rev 0.9) and the CHANGELOG add M9/OBJ-17, but `status.json` stops at `"M8": "planned"`; the status file is inconsistent with the two planning documents it feeds. | `status.json:8-16` (M1..M8 only) vs `development-plan.md`/`roadmap.md` M9 section. | Add `"M9": "in progress"` (and keep it current) in `status.json`. |
| R4-06 | Minor | `docs/engineering/coding-style.md:24` vs `.clang-format:8` | **The declared style contradicts the normative `.clang-format` it references.** The style table says "struct/enum braces on their own line", but `.clang-format` sets `AfterEnum: false` and the formatted codebase uses `typedef enum {` (brace on the same line); only structs use the own-line brace. The prose is thereby wrong about enums. | `coding-style.md:24`; `.clang-format:8` (`AfterEnum: false`); `src/pus/pus_types.h:75` (`typedef enum {`), vs `src/pus/pus_types.h:48` (`typedef struct` newline brace). | Correct the row (e.g. "struct braces on their own line; enum braces on the same line"), keeping `.clang-format` normative. |
| R4-07 | Comment | `docs/reviews/README.md:3-36` | The review log still has no row for the predecessor M4 lwIP report (`2026-10-10-m4-lwip-r1-r2-r4-review.md`) and no row for this M9 review (recurrence of the predecessor's `R4-06`). Scoped out of this report (report-only), but normal N11 requires the log rows at commit. | `docs/reviews/README.md:33` (last M4 row is the netstack review); no lwIP/M9 rows. | Append the M4-lwIP and M9 log rows when committing this cycle (norm N11). |
| R4-08 | Comment | `tests/CMakeLists.txt`, `docs/requirements/SRS.md`, `docs/requirements/SSS.md`, `docs/requirements/traceability.md`, `docs/vv/tool-qualification.md`, `docs/vv/verification-results.md`, `docs/reviews/README.md`, `README.md` | Touched files still end without a trailing newline (pre-existing `D-07`/predecessor `R4-05`). Cosmetic. | Direct byte check (`tail -c1`) on the listed files. | Add the trailing newline to the touched files. |
| R4-09 | Comment | `docs/engineering/coding-style.md:32`; `CMakeLists.txt:27-29` | Secondary accuracy points in the style doc: it claims "C99" while the build uses `CMAKE_C_EXTENSIONS ON` (i.e. `-std=gnu99`, not strict `c99`), and it cites CI static analysis as an enforcement argument although clang-tidy and MISRA are advisory (see R1-03). The MISRA `--std=c99` in `misra.sh` is therefore stricter than the build's dialect. | `coding-style.md:32`; `CMakeLists.txt:27-29`; `ninja -t commands` -> `-std=gnu99`; `misra.sh:22` (`--std=c99`). | State the dialect as `gnu99`/C99-with-extensions, or pin `CMAKE_C_EXTENSIONS OFF` if strict C99 is intended. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

## 4. Positive verification evidence (reproduced in this session)

- **Traceability gate**: `trace.py` -> `traceability: PASS (78 requirements,
  43 srs, 21 sss, 14 gaps, 51 lean theorems checked)`, 10 CI evidence claims;
  the new `SRS-REQ-041 -> SSS-REQ-021`, `SRS-REQ-042 -> SSS-REQ-021` edges and
  the `ci:misra`/`ci:static-analysis` test claims all resolve. Counts match the
  task baseline and the trace doc's requirement-count line.
- **Default build/tests**: configure + build + `ctest` -> **11/11 PASS**; no
  lwIP target/test exists with `FF_USE_LWIP` OFF (default). All suite checks
  re-run: 374, `failures=0`.
- **lwIP path**: `-DFF_USE_LWIP=ON` configured, **148/148** build targets,
  `ctest -R lwip` **1/1 PASS**, `test_lwip` -> **`checks=19 failures=0`** (the
  number the task expects; the docs' "15" is stale, R2-01).
- **Format gate (SRS-REQ-042)**: the exact CI command
  (`clang-format --dry-run --Werror` over `src config tests`) -> **exit 0**;
  the declared style is genuinely applied codebase-wide and enforced by a
  hard-failing gate (when clang-format is present). The `.clang-format` matches
  the described indentation/brace/pointer/line-width rules apart from the enum
  row (R4-06).
- **Fix R1-02 (address validation) is correct and tested**: `ff_lwip_bind`/
  `ff_lwip_connect` now reject `addr == NULL || addr_len < 6` with
  `FF_INVALID_ARGUMENT` (`ff_net_lwip.c:54-72`), matching the loopback backend,
  and `test_lwip.c:56-63` asserts it for `addr_len` 3 and 0.
- **Fix R1-01 (recv bound) is partially realised**: `LWIP_SO_RCVTIMEO 1` is
  enabled (`lwipopts.h:28`) and `ff_lwip_recv` sets `SO_RCVTIMEO` for
  `timeout_ms > 0` (`ff_net_lwip.c:116-121`); the remaining discrepancy is the
  `timeout_ms == 0` semantics (R1-01).
- **MISRA honesty is correct**: `misra-c.md` §2 states plainly that cppcheck is
  **not a qualified MISRA checker** and that a certified tool + formal report is
  a gap (GAP-A4); `class-a-gaps.md` and `misra.sh` repeat this. The procedure
  (§4: triage, deviation record, `// cppcheck-suppress <id>` with the deviation
  record) is defined and consistent with norm N12. The tool is qualified as an
  advisory scanner (T14) with that limitation noted.
- **CI wiring is present and symmetric**: both `.github` and `.gitea` workflows
  add the identical lwIP fetch/build/test step and the MISRA step, and both add
  the three new documents to the `docs` required list; the `# evidence: ci-misra
  (SRS-REQ-041, SSS-REQ-021)` marker is consumed by the gate (10 claims).
- **Third-party hygiene preserved**: `frick_lwip` is built without
  `-Wall`/`-Werror`; only `frick_net_lwip` + `frick_lwip` are added, gated by
  `FF_USE_LWIP` (default OFF).
- **Document-id integrity**: the CI uniqueness logic reproduces `73 unique
  document ids, 0 duplicates`; the three new ids are unique; the README
  requirements-set line (43/21/14), SRS/SSS/traceability revisions and history
  rows, CHANGELOG M4-lwIP + M9 entries, plan/roadmap M9, and the compliance
  matrix/class-A-gap updates are internally consistent.

## 5. Conclusion

Graded conclusions by profile:

| Profile | Conclusion | Basis |
| ------- | ---------- | ----- |
| R1 (design/code) | **APPROVED WITH COMMENTS** | No Major. The two prior fixes are essentially correct: R1-02 (address validation) is fully implemented and tested; R1-01 (recv bound) is implemented via `SO_RCVTIMEO`/`LWIP_SO_RCVTIMEO` but its `timeout_ms == 0` semantics contradict the new design doc and diverge from the loopback backend (R1-01, Minor). R1-03 (false "strict warnings" comment, now echoed by `coding-style.md`) is Minor; R1-02/R1-04 are Comments. |
| R2 (V&V) | **APPROVED WITH COMMENTS** | The `Tst` evidence is real and reproduces (`test_lwip` 19/0, 11/11 default); the MISRA tooling is described honestly (cppcheck not certified; advisory). R2-01 (stale 15-vs-19 check count across four docs; only partial negative-path coverage; `accept` peer address still unasserted) and R2-03 (`misra.sh` documented exit code never returned) are Minor; R2-02/R2-04/R2-05 are Comments. |
| R4 (documentation, N17) | **APPROVED WITH COMMENTS** | The declared style is applied and the format gate is enforced; the MISRA process/deviations and the new architecture note are present; document-id integrity holds. Six Minor findings: R4-01 (stale README metrics panel, recurrence), R4-02 (stale verification-results row/Remaining list, no M9 note, recurrence), R4-03 (stale CI-claim count + SRS-REQ-040 row, recurrence), R4-04 (new docs absent from the README index), R4-05 (`status.json` missing M9), R4-06 (style doc contradicts `.clang-format` on enum braces); R4-07..R4-09 are Comments. |

Overall: **APPROVED WITH COMMENTS**. There is no Major finding. The lwIP review
fixes are correct in substance (the requested address validation and the recv
bound are in place), but the M9 delta repeats the recurring bookkeeping defect:
several counts/records were not regenerated after the requirements and test
changed (`README` panel, `verification-results`, `traceability.md`, check counts,
`status.json`) and three new documents were not indexed. Per norm N12 the Minor
findings must be dispositioned and tracked to closure (with delta re-review)
before the cycle is closed. Suggested priority: R4-01/R4-02/R4-03 (regenerate
the derived counts and records with `metrics.py --readme` + the trace gate),
R4-04/R4-05/R4-06 (one-line doc corrections), R2-01 (fix the recorded check
counts and add the missing negative/timeout/peer cases), then R1-01/R1-03 and
the remaining Comments.

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent reviewer agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS (R1-01/R1-03 Minor) |
| R2 (N9/N10)   | independent reviewer agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS (R2-01/R2-03 Minor) |
| R4 (N17)      | independent reviewer agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS (R4-01..R4-06 Minor) |

## 7. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | Minor | `ff_net_lwip.c` (`ff_lwip_recv`), `network-stack.md`, SRS-REQ-040 | `timeout_ms == 0` blocks and is sticky; impl vs design doc vs loopback disagree | Define `timeout_ms` once; align loopback/lwIP/doc; clear `SO_RCVTIMEO` | (pending) | frick-fsw | open | (pending) |
| R1-02 | Comment | `ff_net_lwip.c` (`send`/`recv`) | No `buf == NULL`/handle validation (loopback rejects) | Validate `buf` (and handle) as loopback does | (pending) | frick-fsw | open | (pending) |
| R1-03 | Minor | `CMakeLists.txt:142`, `coding-style.md:32` | "strict warnings" claim false; no `-Wall/-Wextra/-Werror` on first-party targets | Apply the warning gate or correct the comment/doc | (pending) | frick-fsw | open | (pending) |
| R1-04 | Comment | `ff_net_lwip.c` (`init`/`attach_netif`) | Lazy `g_inited`, no deinit, lifetime undocumented; netif no-op | Document lifetime; guard once primitive | (pending) | frick-fsw | open | (pending) |
| R2-01 | Minor | `test_lwip.c`, `verification-results.md`, `tool-qualification.md`, `CHANGELOG.md`, `network-stack.md` | Recorded "15 checks" vs actual 19; residual untested paths; `accept` peer unasserted | Update counts; add negative/timeout/peer-address cases | (pending) | frick-fsw | open | (pending) |
| R2-02 | Comment | `test_lwip.c:17-19` | Hardcoded `AF_INET`/`SOCK_STREAM` | Include `lwip/sockets.h` or justify | (pending) | frick-fsw | open | (pending) |
| R2-03 | Minor | `tools/scripts/misra.sh` | Header documents an exit code the script never returns; no baseline recorded | Fix header; record agreed baseline | (pending) | frick-fsw | open | (pending) |
| R2-04 | Comment | CI workflows | lwIP path not under ASan | Add sanitizer evidence | (pending) | frick-fsw | open | (pending) |
| R2-05 | Comment | SRS-REQ-041 | Compliance wording stronger than the advisory evidence | Clarify as process req / cite GAP-A4 in-requirement | (pending) | frick-fsw | open | (pending) |
| R4-01 | Minor | `README.md:72-76` | Stale metrics panel (74/40/20, 60/74, 20/74) vs 78/43/21/14, 64/78, 20/78 | Regenerate with `metrics.py --readme` | (pending) | frick-fsw | open | (pending) |
| R4-02 | Minor | `verification-results.md:27,263-267` | Stale M4 row (75/41/20, 9 claims, 15 checks), no M9 note, stale Remaining list | Refresh counts, add M9 note, fix Remaining | (pending) | frick-fsw | open | (pending) |
| R4-03 | Minor | `traceability.md:19,92` | CI claims 9 vs 10; SRS-REQ-040 row omits `lwipopts.h` | Update count; add file | (pending) | frick-fsw | open | (pending) |
| R4-04 | Minor | `README.md:274-298` | New docs missing from the doc index | Add the three links | (pending) | frick-fsw | open | (pending) |
| R4-05 | Minor | `status.json:8-16` | M9 missing | Add `"M9": "in progress"` | (pending) | frick-fsw | open | (pending) |
| R4-06 | Minor | `coding-style.md:24`, `.clang-format:8` | Prose says enum braces own-line; format says same-line | Correct the row | (pending) | frick-fsw | open | (pending) |
| R4-07 | Comment | `docs/reviews/README.md` | M4-lwIP and M9 log rows absent | Append rows at commit (N11) | (pending) | frick-fsw | open | (pending) |
| R4-08 | Comment | several touched files | Missing trailing newline (recurrence) | Add newline | (pending) | frick-fsw | open | (pending) |
| R4-09 | Comment | `coding-style.md:32`, CMake dialect | "C99" vs `-std=gnu99`; advisory static analysis cited as enforcement | State the dialect accurately | (pending) | frick-fsw | open | (pending) |

---

Committed and pushed with the reviewed change (norm N11).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | independent reviewer agent (R1/R2/R4) | Initial milestone-M9 code-quality delta + lwIP review-fix review (FRICK-REVIEW-0034) |
