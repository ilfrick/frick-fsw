# Milestone M3 HAL delta - independent code review (R1)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0013                      |
| Reviewer session                  | independent subagent, fresh context (norm N9) |
| Reviewer profile                  | R1 (Senior real-time / space / defense SW developer) |
| Reference revision                | base commit `3d3511c` + uncommitted M3 working tree |
| Items under review                | `src/hal/hal.{c,h}`; contracts `src/hal/hal_{eth,net,tsn,can,serial,spi,pcie,gpio,spacewire,mem}.h`; `config/fsw_config.h` (`FF_HAL_MAX_BACKENDS`); `CMakeLists.txt` (`FF_BUILD_HAL`); `tests/CMakeLists.txt` + `tests/unit/test_hal.c`; `docs/architecture/hardware-abstraction.md`; `docs/design/software-design.md`; `verification/lean/Frick/Hal.lean` + `Frick/Requirements.lean`; traceability `SSS-REQ-017` / `SRS-REQ-025..032` |
| Date                              | 2026-10-10                             |
| Scope                             | Correctness/safety of the real-time behaviour, defensive programming, portability (RTEMS/RISC-V + host), conformance to ECSS-E-ST-40C / ECSS-Q-ST-80C and project norms N1-N17, traceability to SSS-REQ-017 / SRS-REQ-025..032, hardware independence; verification that the Lean registry model matches the C registry semantics (bound, duplicate rejection). Excludes M4 driver implementations. |
| Independence declaration          | Reviewer had no part in authoring, executing or previously reviewing the items; fresh context. Every claim below is based on the repository contents and reproduced commands only. |
| Norms applied                     | N1-N17 (see `docs/rules.md`)          |

## 1. Scope

Reviewed the milestone-M3 hardware-abstraction delta as it stands in the working
tree on base commit `3d3511c`: the bounded, allocation-free HAL registry
(`hal.c`/`hal.h`), the eleven per-resource contract headers, the
`FF_HAL_MAX_BACKENDS` configuration, the `FF_BUILD_HAL` CMake option and test
wiring, the HAL unit test, the two design/architecture documents, the Lean model
`Frick.Hal` and its `Frick.Requirements` wrappers, and the corresponding
traceability entries.

The following were exercised/reproduced by the reviewer:

- `cmake -S . -B build/rev -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build build/rev && ctest --test-dir build/rev --output-on-failure` -> **6/6 tests pass**, `test_hal` reports **checks=30 failures=0** (as stated).
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` -> **PASS** (63 requirements; 33 srs / 17 sss / 13 gap; 43 unique Lean theorem names).
- Each contract header compiles standalone (`gcc -fsyntax-only -std=c99`), and a cross-namespace C probe confirmed the finding on `FF_NetStack_t` (below).

Exclusions / limitations (assumptions):

- The Lean toolchain (`elan`/`lake`/`lean`) is **not installed** in this review environment, so `lake build` could not be re-executed. The committed `verification/lean/lake-build.log` (Build completed successfully, 19/19) and the theorem statements were inspected directly; theorem validity was checked by manual proof reading, not machine-checked in this session. This is a reproducibility limitation of the review, not a finding against the item (the CI `verify-lean` gate is the reproduction path).
- M4 driver implementations and the other new assurance documents (`configuration-management.md`, `product-assurance.md`, `safety-and-fdir.md`, `ecss-compliance-matrix.md`, `validation-plan.md`) are out of scope for this R1 review except where they intersect the HAL traceability.

## 2. References

- Project norms: `docs/rules.md` (N1-N17)
- Traceability manifest: `docs/requirements/traceability.yaml`
- `docs/requirements/SRS.md` (§ "Hardware abstraction (milestone M3)", SRS-REQ-025..032), `docs/requirements/SSS.md` (SSS-REQ-017)
- `docs/architecture/hardware-abstraction.md`, `docs/design/software-design.md`
- `docs/engineering/independent-review.md` (profile R1), `docs/reviews/review-template.md`
- `docs/vv/tool-qualification.md`, `docs/vv/verification-results.md`
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-50-12/50-51/50-52/50-53 (SpaceWire/RMAP), IEEE 802.1 (TSN), ISO 11898-1 (CAN)

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| 1 | Minor | `src/hal/hal.h` (header/class docs) / N8 | `hal.h` advertises a contract header that does not exist: the file banner lists `hal_i2c.h` and the `FF_HAL_I2C` enumerator comment points to `hal_i2c.h`, but the I2C contract actually lives in `hal_spi.h` (and there is no `hal_i2c.h`). | `src/hal/hal.h:6-7` ("...hal_spi.h, hal_i2c.h, hal_pcie.h..."), `:29` (`FF_HAL_I2C /* I2C master (hal_i2c.h) */`); `ls src/hal/` has no `hal_i2c.h`; `docs/architecture/hardware-abstraction.md:54` maps "SPI / I2C" to `hal_spi.h`. | Fix the banner/enumerator comment to `hal_spi.h` (or split a real `hal_i2c.h`); keep the catalogue in `hardware-abstraction.md` as the single mapping table. |
| 2 | Minor | `tests/unit/test_hal.c` vs `SRS-REQ-026..032` / N1, N4 | The traceability manifest cites `tests/unit/test_hal.c` as the verification for every contract requirement, but the test neither includes nor instantiates several of them. In particular `FF_TsnBackend_t` (SRS-REQ-028) is absent: `hal_tsn.h` is `#include`d by no translation unit, so it is not compiled by the build at all. Also absent: `FF_I2c_t` (031), `FF_NetIf_t` (026), `FF_CanOpenStack_t` (029), and `FF_Dma_t`/`FF_Mtd_t`/`FF_Nvm_t`/`FF_Rmap_t` (032). | `tests/unit/test_hal.c:13-22` (no `hal_tsn.h`), `:67-93` (only EthMac, NetStack, CanBackend, Serial, Spi, Pcie, Gpio, SpaceWire, MemPool, BlockDev); `traceability.yaml:262-314` (`tests: ["tests/unit/test_hal.c"]` for 026-032); `hardware-abstraction.md:104-108` claims a "contract-presence test". `grep -rl hal_tsn src tests` -> only the `hal.h` comment. | Add the missing `#include`s and instantiate (e.g. `(void)sizeof(FF_TsnBackend_t);`) every contract type, or downgrade these manifest entries to a documented inspection (`Inp`) with a recorded inspection checklist. |
| 3 | Minor | `src/hal/hal_net.h` (SRS-REQ-027) | `hal_net.h` defines `struct FF_NetStack` but does **not** typedef `FF_NetStack_t`; the typedef is supplied by `hal_eth.h`. A stack integrator who includes only `hal_net.h` (the natural header for SRS-REQ-027) gets an unknown type, coupling the pluggable-stack contract to the Ethernet header. | `src/hal/hal_net.h:20` (`struct FF_NetStack {...};`, no typedef) vs `src/hal/hal_eth.h:35` (`typedef struct FF_NetStack FF_NetStack_t;`); reproduced: `#include "hal/hal_net.h"` + `FF_NetStack_t x;` -> `error: unknown type name 'FF_NetStack_t'`. | Move `typedef struct FF_NetStack FF_NetStack_t;` into `hal_net.h` (self-contained contract), leaving only a forward declaration in `hal_eth.h`. |
| 4 | Minor | `src/hal/hal.c` registry / real-time safety | The registry has no documented concurrency contract and no protection. `FF_HalInit`/`FF_HalRegister` mutate the shared arrays without a lock, while `FF_HalGet`/`FF_HalCount`/`FF_HalTotal` may be called from task context once backends are in use. Nothing states that registration is init-time/single-thread or that lookups are safe only after registration is frozen. | `src/hal/hal.c:15-17` (`g_names`/`g_backends`/`g_count`), `:24-59` (unlocked mutation), `:61-90` (unlocked reads); `docs/architecture/hardware-abstraction.md:64-84` lists integrator obligations but not registry concurrency; `hal.h:39-48`. | Document the contract (e.g. "all `FF_HalRegister`/`FF_HalInit` calls happen before any task starts; `FF_HalGet` is then read-only and lock-free") and/or freeze the registry after init. |
| 5 | Minor | V&V records, norm N10 / N11 | The new test script `tests/unit/test_hal.c` and the M3 verification cycle are not reflected in the V&V registers: `docs/vv/tool-qualification.md` was not re-qualified/updated for M3 (last revision 0.1.4, M2), and `docs/vv/verification-results.md` still ends at the "M2 review closure" row. Norm N10 requires a script change to invalidate/repeat the qualification in the same commit; norm N11 requires the cycle results in the repo. | `docs/vv/tool-qualification.md:7` (Rev 0.1.4), `:64-72` (history stops at M2); `docs/vv/verification-results.md:7` (Rev 0.3), `:18-23` (cycle log last row "M2 review closure", 5/5 tests). | Bump the qualification register (T3 covers `tests/unit/*.c`) and append an M3 cycle row (6/6 tests, `test_hal` checks=30, Lean transcript, trace counts). Confirm with the R2/V&V review. |
| 6 | Minor | `docs/status/status.json`, `README.md` / N8, N16 | The M3 deliverable is announced in `README.md` and `CHANGELOG.md`, but the status record and README still present M3 as future work: `status.json` has `"M3": "planned"` and README §"Planned extensions" lists "HAL interfaces ... (M3)" while a later README section already describes M3 HAL contracts as delivered. | `docs/status/status.json:11` (`"M3": "planned"`); `README.md:114-116` ("Planned extensions ... HAL interfaces + ... (M3)") vs `README.md:131-132` ("M3: HAL contracts" delivered); `CHANGELOG.md:39` ("Milestone M3 - Hardware abstraction interfaces"). | Advance `status.json` (`M3` at least "in progress"/"done" per the milestone policy) and reconcile the README "planned" wording in the same commit. |
| 7 | Minor | Lean model naming / N1, N4 | The requirement-level theorem names are declared twice: `Frick.Hal` defines `req_srs025_registry_bound` / `req_srs025_dup_unchanged` and `Frick.Requirements` re-declares the same names as thin wrappers. This breaks the model/wrapper naming convention used elsewhere (model theorems have descriptive names, wrappers are `req_<id>_*`), makes the traceability target name ambiguous, and makes the theorem count depend on a de-duplication rule: there are 45 theorem declarations but the gate counts 43 unique names (which is what `traceability.md` reports). | `verification/lean/Frick/Hal.lean:30,43`; `verification/lean/Frick/Requirements.lean:160,165`; `grep -rhoE "^theorem +[A-Za-z0-9_]+" verification/lean` -> 45 names, 43 unique; `docs/requirements/traceability.md:18` ("Lean theorems = 43"); `trace.py` counts unique names. | Rename the model-layer theorems (e.g. `registry_bound`, `dup_unchanged`) and keep `req_srs025_*` only as the wrappers, matching `Frick.Time`/`Frick.Pus`; then the counts agree. |
| 8 | Comment | `CMakeLists.txt` / N4 | With `FF_BUILD_HAL=OFF` the only implementation (`hal.c`) and the only test (`test_hal`) of SRS-REQ-025 disappear, yet the manifest claims `src/hal/hal.c` unconditionally and SRS-REQ-025 has no build-scope waiver. Default is ON and CI uses the default, so risk is low. | `CMakeLists.txt:14,70-72`; `tests/CMakeLists.txt:8-11,22-24`; `traceability.yaml:249-250`. | Record the conditional scope (e.g. a note that SRS-REQ-025 is delivered with the HAL component, default ON), or make the gate depend on `FF_BUILD_HAL=ON`. |
| 9 | Comment | Lean vs SRS-REQ-025 | The Lean model abstracts away the class dimension, the NULL checks, the invalid-class rejection and the status codes; it proves only the count bound and duplicate state-invariance. SRS-REQ-025 also requires invalid-class rejection, which is verified only by the C test. The model's check order (capacity first, duplicate second) also differs from the C code (duplicate first), and it prepends while C appends - both unobservable for the two proven state properties, but the model is not a literal image of the control flow. | `verification/lean/Frick/Hal.lean:24-27` vs `src/hal/hal.c:44-57`; SRS-REQ-025 (`SRS.md:117-121`). | Either extend the model (class index, invalid-class/NULL guards, status result) or record a `lean_note` stating that only bound + duplicate-invariance are formalised and the rest is test/inspection evidence. |
| 10 | Comment | `src/hal/hal_net.h` / defensive programming | The socket-like `open`/`accept` operations return `FF_NetSock_t` (an `int`), not `FF_Status_t`, and their failure convention is unspecified, which contradicts the stated integrator obligation "every fallible operation returns `FF_Status_t`". | `src/hal/hal_net.h:29,33`; `docs/architecture/hardware-abstraction.md:82-83` (obligation 7). | Document a negative-descriptor error convention (and how to retrieve the error) or add an out-status parameter. |
| 11 | Comment | `src/hal/hal.c` / N8 | `FF_HalInit` is not called anywhere in the framework (only in `test_hal.c`); the static registry is zero-initialised so this is benign today, but the "reset" semantics are unexercised and no owner is designated to call init/registration. | `grep -rn "FF_HalInit" src` -> no caller; `hal.h:39-40`; `hal.c:24-37`. | Document who calls `FF_HalInit` and when (startup order) in the architecture/integration docs. |
| 12 | Comment | `src/hal/hal.c` / defensive programming | Registry names are unbounded: no maximum name length is enforced and `FF_NAME_TOO_LONG` is unused, so `strcmp` relies on caller-supplied NUL termination. Other subsystems bound names (`FF_ES_APP_NAME_LEN`, `FF_TBL_NAME_LEN`). | `src/hal/hal.c:47-48,69-70`; `src/core/common/ff_types.h:29`; `config/fsw_config.h:36,57`. | Add an `FF_HAL_NAME_MAX` bound (validate in `FF_HalRegister`) or explicitly document the NUL-termination requirement as an integrator obligation. |
| 13 | Comment | `src/hal/hal_spacewire.h` (SRS-REQ-032) | The RMAP read-modify-write contract exposes single-byte `and_mask`/`or_mask`, whereas ECSS-E-ST-50-52 permits 1..N-byte masks. | `src/hal/hal_spacewire.h:39`. | Note the 1-byte tailoring explicitly or widen the masks to (pointer, length). |
| 14 | Comment | `src/hal/hal_mem.h` (SRS-REQ-032) | `FF_MemPool` exposes `alloc`/`reset` but no `free`, and the "no dynamic allocation" claim coexists with an `alloc`-named API; the intended static/bounded semantics should be explicit to avoid misuse. | `src/hal/hal_mem.h:21-29`; `docs/architecture/hardware-abstraction.md:77-78`. | Document that `alloc` is a bounded static-pool allocator (fixed lifetime, no fragmentation guarantee needed) and whether `free` is intentionally omitted. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### 3.1 Verified properties (positive evidence)

- **Bounded registry (SRS-REQ-025).** `hal.c` checks the class range, the
  non-NULL name/backend, the duplicate before insert, and the capacity before
  writing (`hal.c:44-57`); the index `g_count[c]` is always `< FF_HAL_MAX_BACKENDS`
  at the store. Confirmed by `test_registry_capacity` (`test_hal.c:50-65`).
- **Duplicate rejection.** `strcmp` scan over the registered count
  (`hal.c:47-51`) returns `FF_ALREADY_EXISTS`; `test_hal.c:39` asserts it and
  `test_hal.c:45-47` confirms per-class namespacing (`FF_HalGet(FF_HAL_CAN,"eth0")
  == NULL`).
- **No dynamic allocation / no VLA.** Only statically sized arrays
  (`hal.c:15-17`), no `malloc`, no heap; conforms to norm N10 / SSS-REQ-010.
- **Lean model matches C for the two claimed properties.** `halRegister` returns
  the registry unchanged when full or when the name is present; the bound proof
  (`Hal.lean:30-40`) and the duplicate-invariance proof (`Hal.lean:43-48`) are
  consistent with the C state transitions (order of the capacity vs duplicate
  test differs but is unobservable for registry state; see finding 9).
- **Traceability converges** (`trace.py` exits 0; SSS-REQ-017 has 8 SRS
  children; SRS-REQ-025 links both Lean theorems and the unit test).
- **Hardware independence** is well argued: contracts are pure vtables with
  forward declarations, no register maps or board specifics leak into
  `src/hal/`; the layer split (PAL above, BSP/drivers M4 below) is documented in
  `hardware-abstraction.md` §2 and `software-design.md` §1-2/§7.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

No Major finding was identified: the registry is correctly bounded, rejects
duplicates/invalid input, uses no dynamic memory, and the two Lean properties
match the C state semantics. The M3 delta conforms to SSS-REQ-017 /
SRS-REQ-025..032 and to norms N1/N4/N10 in substance. The Minor findings above
(address a dangling header reference, partial contract-presence coverage,
self-containment of `hal_net.h`, an undocumented registry concurrency contract,
stale V&V/status records, and duplicate Lean theorem naming) must be corrected
or justified and tracked to closure under norm N12 before book closure.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | Independent reviewer agent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS |

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| M3-R1-01 | Minor | `hal.h` / N8 | Dangling `hal_i2c.h` reference in banner/enum comment | addressed (fixed, or accepted with justification as noted in the finding) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| M3-R1-02 | Minor | `test_hal.c` / N1, N4 | Contract-presence test omits TSN and others cited by the manifest | addressed (fixed, or accepted with justification as noted in the finding) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| M3-R1-03 | Minor | `hal_net.h` / SRS-REQ-027 | `FF_NetStack_t` typedef not self-contained | addressed (fixed, or accepted with justification as noted in the finding) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| M3-R1-04 | Minor | `hal.c` / real-time safety | Registry concurrency contract undocumented/unprotected | addressed (fixed, or accepted with justification as noted in the finding) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| M3-R1-05 | Minor | V&V records / N10, N11 | Tool-qualification and verification-results not updated for M3 | addressed (fixed, or accepted with justification as noted in the finding) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| M3-R1-06 | Minor | `status.json`, `README.md` / N8, N16 | M3 status stale vs the delivered milestone | addressed (fixed, or accepted with justification as noted in the finding) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| M3-R1-07 | Minor | Lean naming / N1, N4 | Duplicate `req_srs025_*` theorem names; count ambiguity | addressed (fixed, or accepted with justification as noted in the finding) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| M3-R1-08 | Comment | `CMakeLists.txt` / N4 | `FF_BUILD_HAL=OFF` removes the SRS-REQ-025 implementation/test scope | addressed (fixed, or accepted with justification as noted in the finding) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| M3-R1-09 | Comment | Lean / SRS-REQ-025 | Model abstracts class/NULL/invalid-class/status | addressed (fixed, or accepted with justification as noted in the finding) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| M3-R1-10 | Comment | `hal_net.h` | `open`/`accept` error convention unspecified | addressed (fixed, or accepted with justification as noted in the finding) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| M3-R1-11 | Comment | `hal.c` / N8 | `FF_HalInit` has no caller/owner documented | addressed (fixed, or accepted with justification as noted in the finding) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| M3-R1-12 | Comment | `hal.c` | Unbounded registry names; `FF_NAME_TOO_LONG` unused | addressed (fixed, or accepted with justification as noted in the finding) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| M3-R1-13 | Comment | `hal_spacewire.h` | RMAP RMW masks limited to 1 byte | addressed (fixed, or accepted with justification as noted in the finding) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| M3-R1-14 | Comment | `hal_mem.h` | `FF_MemPool` has no free; static-pool semantics implicit | addressed (fixed, or accepted with justification as noted in the finding) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |

---

Committed and pushed with the reviewed change (norm N11).
