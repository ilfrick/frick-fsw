# Milestone M7 (CCSDS SDLS security) - Independent R1/R2 Review

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0023                      |
| Reviewer session                  | (independent agent, norm N9)           |
| Reviewer profile                  | R1 (SW dev) **and** R2 (SW V&V)        |
| Reference revision                | branch `main`, working tree on base commit `3096241` (uncommitted M7 delta) |
| Items under review                | `src/security/ff_sdls.{c,h}`, `src/security/ff_crypto_test.{c,h}`, `tests/unit/test_sdls.c`, `CMakeLists.txt` (`FF_BUILD_SDLS`), `tests/CMakeLists.txt`, `verification/lean/Frick/Sdls.lean`, wrappers in `verification/lean/Frick/Requirements.lean`, `docs/requirements/traceability.yaml` (SSS-REQ-019, SRS-REQ-034/-035, GAP-A14), `docs/architecture/security-sdls.md`, `docs/adr/ADR-0013-sdls.md` |
| Date                              | 2026-10-10                             |
| Scope                             | Real-time/determinism and safety, defensive programming, constant-time MAC compare, IV uniqueness/counter handling on rotation, zeroisation robustness, bounded/no-alloc, conformance to norms N1-N17 and ECSS, traceability, test correctness, acceptability of the `lean_note` for SRS-REQ-034. ECSS-E-ST-40C, ECSS-Q-ST-80C, CCSDS 355.0-B/352.0-B |
| Independence declaration          | The reviewer had no part in authoring, designing or executing the reviewed items; this review ran in a fresh, independent context (norm N9). |
| Norms applied                     | N1-N17 (see `docs/rules.md`)           |

## 1. Scope

This review covers the M7 delta (CCSDS SDLS security sub-layer) as found in the
uncommitted working tree on base commit `3096241`. It covers the SDLS middleware
(`ff_sdls.{c,h}`), the non-secure reference backend (`ff_crypto_test.{c,h}`,
test-only), the unit test `tests/unit/test_sdls.c`, the build wiring
(`CMakeLists.txt`, `tests/CMakeLists.txt`), the Lean model and requirement
wrappers, and the associated requirements/architecture/ADR/traceability
documents. The cryptographic algorithm, HSM/keystore and ground key management
are explicitly out of scope (documented class-A gap GAP-A14) and are not counted
as findings here except where the middleware itself fails to bound or validate
what it claims to guarantee.

Both the R1 (design/code) and R2 (V&V) profiles were applied to the same delta,
as required for this milestone.

**Excluded:** runtime integration into the transport chain (the module is not
yet wired into `src/transport/`, verified by inspection), coverage regeneration
(not requested), the R3 SSS review (N14) and R4 documentation review (N17),
which are separate reviews.

### Verification evidence executed

| Command | Result |
| ------- | ------ |
| `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** - 69 requirements, 36 srs, 19 sss, 14 gaps, 51 Lean theorems, 9 CI claims (`EXIT=0`) |
| `cmake -S . -B build/srev -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build build/srev` | **PASS** (48/48 targets, no warnings) |
| `ctest --test-dir build/srev --output-on-failure` | **PASS** 8/8 (`ccsds, sb, tbl, pus, time, hal, math, sdls`) |
| `./build/srev/tests/test_sdls` | `checks=53 failures=0` |
| `cc -std=c99 -Wall -Wextra -Wconversion -Wshadow -Werror ... ff_sdls.c` | clean (no diagnostics) |
| ASan PoC (large `in_len`) | **stack-buffer-overflow** at `ff_sdls.c:275` (finding R1-01) |
| GDB PoC (remove all SAs) | stale key material survives in tail slot (finding R1-02) |
| PLAIN PoC (protect then unprotect) | `unprotect = FF_NOT_FOUND (-7)` (finding R1-03) |

The two gate commands requested by the milestone pass exactly as stated (8/8,
sdls checks=53, trace 69/51). The defects below are not detected by these gates.

## 2. References

- Project norms: `docs/rules.md` (N1-N17), interpretation notes 1-3
- Traceability manifest: `docs/requirements/traceability.yaml`
- Architecture: `docs/architecture/security-sdls.md`, `docs/adr/ADR-0013-sdls.md`
- V&V records: `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md`
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2, CCSDS 355.0-B, CCSDS 352.0-B

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R1-01 | **Major** | SRS-REQ-034; safety / bounded-no-alloc | **16-bit length overflow defeats the `out_cap` bound in `FF_SdlsProtect`, causing an out-of-bounds write.** `total = (uint16_t)(2u + sa->iv_len + in_len + mac_len)` is computed in 16-bit and wraps when `in_len > 65535 - (2+iv_len+mac_len)` (e.g. `in_len=65506`, `iv_len=12`, `mac_len=16` → `65536 → 0`). The guard `if (total > out_cap)` then passes for an arbitrarily small `out_cap`, and the code writes `in_len` bytes into `out`. | `src/security/ff_sdls.c:266-269` (compute/check) and `:275` (write). ASan PoC (built from the reviewed sources, unmodified) reports: `ERROR: AddressSanitizer: stack-buffer-overflow ... WRITE of size 1 at ... ff_sdls.c:275 in FF_SdlsProtect`, `out` (8 bytes) overflowed. | Compute the required size in `size_t`/`uint32_t`; reject when `in_len > out_cap - (2u + sa->iv_len + mac_len)`; perform the check before any write. Add a unit test at the wrap boundary. |
| R1-02 | **Major** | SRS-REQ-035 ("zeroise key material on ... removal"); header contract | **`FF_SdlsSaRemove` leaves a stale copy of key material in the tail slot.** It wipes `g_sa[i]`, then `g_sa[i] = g_sa[g_sa_count-1]` and decrements the count, but never wipes the vacated tail slot. When the moved SA is later removed, its master/session key bytes remain in `g_sa[old_count-1]` (beyond the live range) - directly contradicting the header comment "removing an SA zeroises its key material". | `src/security/ff_sdls.c:168-180`. GDB PoC: after removing all SAs (`count=0`), `g_sa[1].master_key[0] = 0xbb` and `g_sa[1].session_key[0] = 0xa6` (SA B's key material) survive. | After the move, `ff_sdls_wipe(&g_sa[g_sa_count-1], sizeof(FF_Sa_t))` (or compact with `memmove` and wipe the tail). Add a zeroisation-after-removal test. |
| R1-03 | Minor | SRS-REQ-034 / SSS-REQ-019 | **`FF_SdlsProtect` and `FF_SdlsUnprotect` are asymmetric for `FF_SDLS_PLAIN`.** Protect emits only the payload (`in_len` bytes, no SPI/IV/MAC), whereas Unprotect always parses a leading 2-byte SPI and copies the whole input. A plain protect→unprotect round trip therefore fails: Unprotect reads the first two payload bytes as SPI (`0x0102`) and returns `FF_NOT_FOUND`. | `src/security/ff_sdls.c:250-258` (protect) vs `:301-329` (unprotect). PoC: `protect st=0 wlen=3 wire={11,22,33}` then `unprotect st=-7 olen=0`. | Define PLAIN semantics explicitly and make both directions symmetric (either both carry the SPI header, or PLAIN Unprotect copies without an SPI lookup). Cover with a round-trip test. |
| R1-04 | Minor | SRS-REQ-034; defensive programming | **The backend contract is not validated for security minima.** `FF_SdlsSetBackend` accepts `mac_len == 0` and `iv_len < 4`. For `AUTH`/`AUTH_ENC` a zero `mac_len` means "protect" appends no MAC and "unprotect" verifies nothing, silently disabling authentication while returning `FF_SUCCESS`; an `iv_len < 4` means `ff_sdls_build_iv` never embeds the per-frame counter, so IVs never vary within a session key (IV reuse). | `src/security/ff_sdls.c:80-93` (validation), `:263-265`/`:333-335` (`mac_len = g_crypto->mac_len`), `:48-53` (`build_iv` writes the counter only when `iv_len >= 4`). | Require `mac_len > 0` for auth services and `iv_len >= 4` (or reject `mac_len == 0`), or document and enforce per-service minimums. |
| R1-05 | Minor | Real-time/safety; consistency with `tbl.c` | **No synchronisation on shared mutable state.** `g_sa`, `g_sa_count`, `g_crypto` are global and mutated without a lock (`FF_SdlsProtect`/`Unprotect`/`Tick`/`SaAdd`/`Remove`). Other core registries protect shared state with a PAL mutex (e.g. `src/core/tbl/tbl.c:21-23`); SDLS does not, so concurrent uses from different tasks race and can corrupt the registry or read a half-rotated key. The architecture note does not state a single-threaded contract. | `src/security/ff_sdls.c:15-17`; compare `src/core/tbl/tbl.c:21-23,40-53`. | Add a PAL mutex around SDLS state (as in `tbl.c`) or document an enforceable single-threaded caller contract in `docs/architecture/security-sdls.md`. |
| R1-06 | Minor | SRS-REQ-035; defensive programming | **Rotation failure leaves the SA inconsistent and is ignored.** `FF_SdlsRotateKey` wipes the old session key *before* deriving, then increments the epoch and resets counters; if `key_derive` fails it returns `FF_ERROR` with the old key already destroyed and the session key possibly partially written. `FF_SdlsTick` discards the return value (`(void)FF_SdlsRotateKey(...)`), so a broken SA keeps operating silently. | `src/security/ff_sdls.c:196-204`; `:217`. | Derive into a temporary and swap only on success (wipe-on-failure), and propagate/handle Tick errors. |
| R2-01 | **Major** | test correctness / V&V confidence (norm N6/N10) | **The unit suite passes (53 checks) while the Major defects above are live and untested.** There is no boundary/overflow test for `out_cap` vs `in_len`, no zeroisation-after-removal check (only `FF_SdlsZeroize` on the whole table is tested), and no PLAIN round-trip test (only PLAIN protect). The suite therefore certifies a build that contains an OOB write and a key-retention defect. | `tests/unit/test_sdls.c:125-154` (PLAIN tests protect only), `:156-166` (zeroise tests the whole table only), `test_protect_roundtrip` uses AUTH_ENC with a generous buffer. | Add negative/boundary tests: `out_cap` exactly/one-less than required, large `in_len` wrap, PLAIN protect→unprotect, remove-then-inspect (or a debug accessor) for zeroisation, ENC-only and AUTH-only round trips, invalid arguments and derive-failure paths. Run an ASan build in CI for the security module. |
| R2-02 | Minor | norm N10/N11 (V&V tool qualification) | **`test_sdls.c` was not registered/re-qualified.** `docs/vv/tool-qualification.md` is at rev 0.1.6 whose latest history entry is M6 (`test_math.c`); there is no M7 entry recording the new test script, although T3 covers `tests/unit/*.c` generically. N10 requires any new/changed V&V script to be qualified and the register updated in the same commit (N11). | `docs/vv/tool-qualification.md:73` (last history entry M6); no `test_sdls` row/entry. | Add a T3 history entry for M7 (`test_sdls.c`, 53 checks) with the source-review disposition. |
| R2-03 | Minor | norm N8/N11/N14/N17; documentation accuracy | **`verification-results.md` claims M7 reviews ahead of the repository evidence.** The M7 section states "Reviewed by R1/R2 (code/V&V), R3 (SSS, N14) and R4 (documentation, N17); NCs dispositioned". At the time this R1/R2 review was performed, no M7 review report existed in `docs/reviews/` and `docs/reviews/README.md` had no M7 row; a concurrent R3/R4 M7 report (`2026-10-10-m7-sdls-r3-r4-review.md`) appeared during this review with Minor findings still open, and neither report is committed yet. | `docs/vv/verification-results.md:232-236`; `docs/reviews/README.md` has no M7 row; the M7 reports are untracked (`git status`) and the R3/R4 report records open Minors. | Commit both M7 review reports with a review-log row in the same cycle as the change (N11), and keep the "NCs dispositioned" claim consistent with the reports' actual state (N8/N12). |

**Positive observations (no action):** constant-time MAC comparison is correctly
implemented with no early exit (`ff_sdls.c:351-361`); MAC verification precedes
decryption/plaintext copy, so unauthenticated plaintext is never released; the
module uses no dynamic memory and all loops are bounded by `FF_SDLS_MAX_SA`,
`iv_len`/`mac_len` or the caller length; the non-secure reference backend is
compiled only into the test target and never into `frick_fsw_core` (structurally
enforcing "not for flight"); SPDX/AGPL headers are present on all new files.

**Findings graded Comment (no derogation, tracked as observations):**

- **C-1 (lean_note for SRS-REQ-034).** The `lean_note`
  (`traceability.yaml:333`) is **acceptable** under norm N1: it is explicit and
  justifies verification by test evidence rather than a Lean theorem, which the
  interpretation note 1 permits (no silent waiver). Two remarks: (a) SDLS
  framing/MAC coverage is a codec-like, Lean-modelable property, so a
  `Frick.Sdls` framing/coverage theorem would strengthen N1 beyond the SA/key
  model now provided; (b) the note conflates the pluggable algorithm (GAP-A14)
  with the middleware processing logic, which is what the tests actually
  exercise. Recommend a small framing theorem and a more precise wording in a
  later increment.
- **C-2 (Lean fidelity / traceability shape).** `Frick.Sdls` models `epoch` and
  `uses` as unbounded `Nat`, while the C uses `uint32`; the wrap-around (and the
  resulting key/IV reuse) is outside the model and is only covered by GAP-A14.
  Also `SSS-REQ-019` links the SRS-level theorem
  `req_srs035_epoch_monotonic` (`traceability.yaml:485`); this resolves, but a
  distinct SSS wrapper would keep the level hierarchy clean.
- **C-3 (key exposure / least privilege).** `FF_SdlsSaGet` copies the full SA,
  including `master_key` and `session_key`, to the caller. Acceptable for the
  test, but consider a metadata-only accessor for non-test use.
- **C-4 (policy thresholds / time).** `FF_SdlsProtect` can exceed `max_tx` until
  `FF_SdlsTick` runs (the threshold is not a hard bound), and the unsigned
  `now_ns - activation_ns` in `Tick` wraps to a huge value if the clock steps
  backwards, causing an immediate rotation. Document or harden both.
- **C-5 (API robustness).** No aliasing check for `in == out`; calling
  `FF_SdlsSetBackend` with a new backend does not re-derive existing session
  keys (switching backend with different `key_len` silently invalidates them).

## 4. Conclusion

### R1 - Design/code (Senior real-time/space SW developer profile)

- [ ] APPROVED - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [x] **REJECTED** - one or more Major findings

Justification: two Major findings are confirmed by executable evidence -
R1-01 (out-of-bounds write via the 16-bit length overflow) and R1-02
(incomplete zeroisation of key material on SA removal, contradicting a stated
requirement and the header contract). Both are bounded-code/security defects at
criticality B and must be fixed and re-reviewed (norm N12) before commit/push
(norm N7). All other design/defensive items are Minor or Comment.

### R2 - Software V&V (Senior SW V&V engineer profile)

- [ ] APPROVED - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [x] **REJECTED** - one or more Major findings

Justification: the requested gates pass (8/8 tests, `sdls` 53 checks, trace
69/51), but the V&V evidence is incomplete: R2-01 (Major) shows the suite cannot
detect the two live Major code defects, so the "all tests green" evidence does
not establish the required confidence; R2-02 (Minor) shows the V&V tool
qualification register was not updated for `test_sdls.c`; R2-03 (Minor) shows
`verification-results.md` claims closed M7 reviews ahead of the committed review
evidence and review-log rows. V&V cannot be
accepted until the tests are extended, the defects are re-verified after fix, and
the qualification/reporting records are updated (norm N12).

Overall for the M7 delta: **REJECTED** pending closure of R1-01, R1-02 and
R2-01, and re-review of the deltas (norm N12/N15).

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent agent (this session) | 2026-10-10 | REJECTED (2 Major) |
| R2 (N9)       | independent agent (this session) | 2026-10-10 | REJECTED (1 Major, 2 Minor) |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | ------ | --------------- |
| M7-NC-01 | Major | SRS-REQ-034 | R1-01 16-bit length overflow → OOB write | Fix size computation in 32/64-bit; validate before write; add wrap-boundary test | ASan PoC now returns `FF_BAD_SIZE`; new unit case | frick-fsw | open | pending |
| M7-NC-02 | Major | SRS-REQ-035 | R1-02 stale key in tail after swap-remove | Wipe vacated tail slot; add zeroisation-after-removal test | GDB/unit check shows zeroised tail | frick-fsw | open | pending |
| M7-NC-03 | Major | norm N6/N10 | R2-01 suite passes with live Major defects | Extend tests (boundary/overflow, PLAIN round trip, zeroisation, service variants, invalid args); add ASan build | New tests fail before fix / pass after; ASan clean | frick-fsw | open | pending |
| M7-NC-04 | Minor | SRS-REQ-034 | R1-03 PLAIN protect/unprotect asymmetry | Define PLAIN semantics; make symmetric; add round-trip test | New PLAIN round-trip test | frick-fsw | open | pending |
| M7-NC-05 | Minor | SRS-REQ-034 | R1-04 backend security minima not validated | Validate `mac_len > 0` / `iv_len >= 4` per service | Backend-rejection unit cases | frick-fsw | open | pending |
| M7-NC-06 | Minor | real-time/safety | R1-05 shared state not synchronised | Add PAL mutex (as `tbl.c`) or document single-threaded contract | Race review / doc update | frick-fsw | open | pending |
| M7-NC-07 | Minor | SRS-REQ-035 | R1-06 rotation failure leaves SA inconsistent; Tick ignores error | Derive-then-swap; propagate error | Inject KDF failure in test | frick-fsw | open | pending |
| M7-NC-08 | Minor | norm N10/N11 | R2-02 `test_sdls.c` not registered/re-qualified | Add M7 T3 entry in `tool-qualification.md` | Register rev bump | frick-fsw | open | pending |
| M7-NC-09 | Minor | norm N8/N11/N14/N17 | R2-03 `verification-results.md` claims run ahead of committed review evidence | Commit M7 R1/R2 + R3/R4 reports with review-log rows; reconcile the claim with the reports' state | `docs/reviews/` + log updated, reports tracked | frick-fsw | open | pending |
| M7-NC-10 | Comment | norms | C-1..C-5 observations | Consider in a later increment | n/a | frick-fsw | open | n/a |

---

Reviewer note on repository hygiene: this report must be committed with the
reviewed change (norm N11), and a row added to `docs/reviews/README.md`. A
concurrent M7 R3 (N14) / R4 (N17) report
(`2026-10-10-m7-sdls-r3-r4-review.md`) exists but had not been committed at the
time of writing and records open Minor findings; both M7 reports and their log
rows are required before the change may be committed and pushed (norms
N11/N14/N17). No file other than this report was modified during the review;
PoCs were built under `/tmp/opencode/`.
