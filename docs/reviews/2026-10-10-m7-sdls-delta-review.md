# Milestone M7 (CCSDS SDLS security) delta re-review — profiles R1/R2/R3/R4

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0024                      |
| Reviewer session                  | independent subagent, fresh context (norm N9) |
| Reviewer profiles                 | R1 (design/code), R2 (V&V), R3 (systems, N14), R4 (documentation, N17) |
| Reference revision                | branch `main`, uncommitted M7 working tree on base commit `3096241` |
| Items under review                | NC dispositions in `docs/reviews/2026-10-10-m7-sdls-r1-r2-review.md` (FRICK-REVIEW-0023: R1-01..R1-06, R2-01..R2-03, C-1..C-5) and `docs/reviews/2026-10-10-m7-sdls-r3-r4-review.md` (FRICK-REVIEW-0022: R3-1..R3-3, R4-1..R4-5); fix artifacts `src/security/ff_sdls.{c,h}`, `src/security/ff_crypto_test.{c,h}`, `tests/unit/test_sdls.c`, `docs/architecture/security-sdls.md`, `docs/adr/ADR-0013-sdls.md`, `docs/quality/ecss-criticality-b.md`, `docs/vv/tool-qualification.md`, `docs/vv/verification-results.md`, `docs/reviews/README.md`, `CHANGELOG.md` |
| Date                              | 2026-10-10                             |
| Scope                             | Re-apply all norms to the affected M7 scope (norm N12) and verify every recorded NC disposition against the working tree; reproduce the trace/build/ctest gates, independently reproduce the two Major PoCs under AddressSanitizer, re-derive the V&V counts and the document-id gate |
| Independence declaration          | I am a freshly spawned independent reviewer with no prior context. I took no part in authoring, fixing, building, executing or previously reviewing any item under review; every claim below derives from the repository contents and reproduced commands. I modified no file except this report. |
| Norms applied                     | N1–N17 (see `docs/rules.md`), emphasis N6, N8, N9, N10, N11, N12, N14, N16, N17 |

## 1. Scope

Delta re-review of the milestone-M7 CCSDS SDLS NC dispositions of the R1/R2
(FRICK-REVIEW-0023, **REJECTED**: R1-01, R1-02, R2-01 Major) and R3/R4
(FRICK-REVIEW-0022, **APPROVED WITH COMMENTS**: R3-3, R4-1, R4-2 Minor) reviews
against the current uncommitted working tree. Every recorded fix was located in
the tree, the requested gate chain was re-run, and the two Major code defects
were independently reproduced as PoCs under AddressSanitizer.

Included: the SDLS middleware and reference backend (code and header contracts),
the unit test, and the M7 documentation/records affected by the dispositions.
Excluded: the cryptographic algorithm / HSM / ground key management
(documented class-A gap GAP-A14), transport integration (the module is still not
wired into `src/transport/`), and the Lean gate re-execution (`lake`/`lean` are
unavailable on this host, as in the predecessor and M2/M3/M6 delta reviews).

Reproduced commands (all from `/home/nicola/fsw/frick-fsw`, no repository file
modified):

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` →
  **PASS, exit 0** (69 requirements: 36 srs / 19 sss / 14 gap; **51** Lean
  theorems; **9** CI evidence claims); `SSS-REQ-019 <- 2 srs children`.
- `cmake -S . -B build/d7 -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-fsanitize=address
  -fno-omit-frame-pointer" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address" &&
  cmake --build build/d7 && ctest --test-dir build/d7 --output-on-failure` →
  **8/8 pass** (`ccsds, sb, tbl, pus, time, hal, math, sdls`), 0 failed.
- `./build/d7/tests/test_sdls` → `checks=69 failures=0`.
- **R1-01 PoC** (unmodified reviewed sources, ASan, `in_len=65506`, `out_cap=8`,
  `AUTH_ENC`, `iv_len=12`, `mac_len=16`) → `FF_SdlsProtect` returns
  `FF_BAD_SIZE (-9)`, `out_len` untouched, **no ASan report**.
- **R1-02 PoC** (implementation TU included so `g_sa` is visible, ASan) →
  vacated tail slot `g_sa[2]` has **0 non-zero bytes** after a middle-of-table
  removal; after removing all SAs the whole `g_sa` has **0 non-zero bytes**.
- Document-id scan over `docs/**/*.md` → **0 duplicate `Document id`
  declarations**; the two M7 reports are now `FRICK-REVIEW-0022` (R3/R4) and
  `FRICK-REVIEW-0023` (R1/R2).
- `python3 tools/scripts/metrics.py` → README metrics panel matches
  byte-for-byte.

## 2. References

- Project norms: `docs/rules.md` (N1–N17); review template
  `docs/reviews/review-template.md`.
- Predecessor reports: `docs/reviews/2026-10-10-m7-sdls-r1-r2-review.md`
  (FRICK-REVIEW-0023) and `docs/reviews/2026-10-10-m7-sdls-r3-r4-review.md`
  (FRICK-REVIEW-0022).
- `src/security/ff_sdls.{c,h}`, `src/security/ff_crypto_test.{c,h}`,
  `tests/unit/test_sdls.c`, `docs/architecture/security-sdls.md`,
  `docs/adr/ADR-0013-sdls.md`.
- `docs/requirements/{SSS,SRS,traceability}.{md,yaml}`; `docs/vv/{tool-qualification,verification-results}.md`;
  `docs/quality/ecss-criticality-b.md`; `CHANGELOG.md`, `docs/reviews/README.md`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2,
  CCSDS 355.0-B, CCSDS 352.0-B.

## 3. Findings (delta)

| # | Severity | Profile | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------- | ------------------ | ------- | -------- | -------------- |
| D-1 | Minor | R4 (N8/N16/N11) | `CHANGELOG.md` M7 entry | The M7 CHANGELOG entry records `test_sdls` **53 checks**, but the delta added 16 checks and the built binary, `verification-results.md` and `tool-qualification.md` all state **69**. The V&V record is internally inconsistent and not reproducible as written. | `CHANGELOG.md:69` (`unit test `test_sdls` (53 checks)`); `./build/d7/tests/test_sdls` → `checks=69 failures=0`; `docs/vv/verification-results.md:224` and `docs/vv/tool-qualification.md:75` both `69`. | Update the CHANGELOG M7 entry to `test_sdls` (69 checks) so all records agree. |
| D-2 | Minor | R2 (N6/N10/N12) | `tests/unit/test_sdls.c` (R2-01 residual) | R2-01 was closed for the two live Major defects, but not all of its recommended tests were added, so parts of the suite still cannot detect a regression: (a) there is **no byte-level zeroisation-after-removal assertion** (only count + `FF_NOT_FOUND`); (b) **no AUTH-only or ENC-only round trip** (round trip is `AUTH_ENC` only); (c) **no derive-failure path** for `FF_SdlsRotateKey`/`FF_SdlsTick`; (d) the **ASan build is not wired into CI** despite being claimed as evidence. | `tests/unit/test_sdls.c:182-201` (`test_remove_reuse`: count/get/re-add, no byte snapshot); `:59-85` (`test_protect_roundtrip`, `AUTH_ENC`); `ff_crypto_test.c` always succeeds; no `asan`/`sanitize` reference in `.github/workflows/ci.yml` or `.gitea/workflows/ci.yml` (local `build/asan/` exists; `verification-results.md:224` claims "AddressSanitizer clean"). | Add a test-only `FF_Sdls` debug accessor (or analysis note) and assert the wiped bytes; add AUTH-only/ENC-only round trips; inject a failing KDF; add an ASan CI job for the security module. |
| D-3 | Minor | R1 (N8/N12) | `src/security/ff_sdls.c` (R1-06 residual) | R1-06 is only **half** applied. `FF_SdlsTick` now propagates the rotation failure (`st = FF_ERROR`), but `FF_SdlsRotateKey` still wipes `session_key` **before** deriving and increments `epoch`; if the KDF fails the SA is left `active == 1` with an all-zero `session_key` and the previous `session_key_len`, so a later `FF_SdlsProtect` would run with a zeroed key (silent key loss rather than an atomic rotate-then-swap or a disabled SA). | `src/security/ff_sdls.c:201-208` (wipe at `:201`, `epoch++` at `:202`, derive at `:206`); `:223-224` (Tick propagation, correct). No failure-injection test exists. | Derive into a temporary and swap only on success (or set `active = 0` on failure) and add a failing-KDF test. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

**No Major finding was raised at any profile.** Both predecessor Major defects
are fixed and independently reproduced as ASan-clean PoCs; the remaining items
are Minors (D-1, D-2, D-3 and the still-open base Minors R1-05/R1-06/R2-03) or
non-derogatory Comments (R3-1, R3-2, R4-3, R4-4, and the README-log part of
R4-5).

## 4. Conclusion (per profile)

- [ ] APPROVED — no findings or Comments only
- [x] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**Profile R1 (design/code): APPROVED WITH COMMENTS.** The two Majors are closed:
R1-01 (length overflow) is now computed in 32-bit with an explicit
`> out_cap || > 0xFFFF` rejection before any write (`ff_sdls.c:278-284`) and my
ASan PoC at the original wrap boundary returns `FF_BAD_SIZE` with no OOB; R1-02
(the vacated tail slot) is wiped after the swap (`ff_sdls.c:174-180`) and my PoC
shows zero survivor bytes. R1-03 (PLAIN symmetry: SPI header emitted/parsed on
both directions, `ff_sdls.c:258-271` / `:335-349`) and R1-04
(`FF_SdlsSetBackend` rejects `mac_len == 0` and `iv_len < 4`, `:82-86`) are
closed. Two Minors remain: **R1-05** (no SDLS mutex and no documented
single-threaded contract — neither alternative of the predecessor
recommendation was taken) and **R1-06** (partial — D-3).

**Profile R2 (V&V): APPROVED WITH COMMENTS.** R2-01's Major is closed: both
live Major defects are fixed and the suite now has an overflow/boundary test
(`test_bounds`, large `in_len` rejected and too-small `out_cap`) and a
removal/reuse test (`test_remove_reuse`), plus a PLAIN protect→unprotect round
trip (`test_capacity_and_plain:143-162`). The gates reproduce exactly (trace
69/36/19/14, 51 theorems; 8/8 ctest; `sdls` **69** checks; ASan clean; total
**294** checks = 15+20+9+29+101+31+20+69). R2-02 is closed (`tool-qualification.md`
rev **0.1.7**, M7 T3 entry naming `test_sdls.c`). Residual Minor **D-2**
(incomplete regression coverage and no ASan CI) and still-open Minor **R2-03**
(see R4/commit-time item below).

**Profile R3 (systems, N14): APPROVED WITH COMMENTS.** R3-3 is closed: the
IV-uniqueness statement in `security-sdls.md` §4 is now explicitly bounded by
`iv_len >= 4` and tied to the enforced `FF_SdlsSetBackend` contract
(`security-sdls.md:58-60`; `ff_sdls.c:84`), matching the code. Comments **R3-1**
(SSS/SRS `Applicable standards` still omit CCSDS 355.0-B — `SSS.md:9`,
`SRS.md:9`) and **R3-2** (zeroisation still verified only indirectly — same gap
as D-2(a)) remain open and non-blocking; no Major.

**Profile R4 (documentation, N17): APPROVED WITH COMMENTS.** R4-1 is closed
(`ecss-criticality-b.md` header **0.4** with an M7 `History` row, `:7,:84`).
R4-2 is closed (ADR-0013 and `ff_crypto_test.h` now say test-only / not built
into the flight or simulator binaries, `ADR-0013-sdls.md:25-27`,
`ff_crypto_test.h:3-9`). R4-5's document-id defect is closed (the R1/R2 report
is renumbered `FRICK-REVIEW-0023`, `:5`). New Minor **D-1** (stale CHANGELOG
check count). Comments **R4-3** (baseline doc `ecss-criticality-b.md` §1/§3
still lack the SDLS standards and a Security deliverable row) and **R4-4**
(missing trailing newline in `class-a-gaps.md` and `licensing.md`) remain open;
the README-log part of R4-5 is unfulfilled (see R2-03).

**Overall for the M7 delta: the R1/R2 rejection is lifted** (no Major remains;
both Major fixes independently reproduced under ASan). Closure is blocked only
on Minor findings — R1-05, R1-06 (D-3), R2-03, D-1, D-2 — and the non-blocking
Comments; norm N12 requires these to be dispositioned before the M7 records are
final. The commit/push step (norm N11) must also log both M7 reports in
`docs/reviews/README.md` and reconcile the M7 wording in `verification-results.md`.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent subagent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS (R1-05; R1-06/D-3 Minor) |
| R2 (N9)       | independent subagent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS (D-2; R2-03 Minor) |
| R3 (N9/N14)   | independent subagent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS (R3-3 closed; R3-1/R3-2 Comments) |
| R4 (N9/N17)   | independent subagent, fresh context | 2026-10-10 | APPROVED WITH COMMENTS (D-1 Minor; R4-3/R4-4 Comments) |

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

### 6.1 R1/R2 base NCs (FRICK-REVIEW-0023)

| NC | Ref | Severity | Item | Delta verdict | Evidence |
| -- | --- | -------- | ---- | ------------- | -------- |
| M7-NC-01 | R1-01 | Major | 16-bit length overflow → OOB write | **CLOSED** | `ff_sdls.c:278-284` computes `total32` in `uint32_t`, rejects `> out_cap || > 0xFFFF` before any write; ASan PoC (`in_len=65506`, `out_cap=8`) → `FF_BAD_SIZE (-9)`, no OOB; `test_bounds` (`test_sdls.c:164-180`). |
| M7-NC-02 | R1-02 | Major | Stale key material in vacated tail | **CLOSED** | `ff_sdls.c:174-180` wipes `g_sa[g_sa_count-1]` after the swap; ASan PoC shows 0 non-zero bytes in `g_sa[2]` and across `g_sa` after removing all SAs; `test_remove_reuse` (`:182-201`). |
| M7-NC-03 | R2-01 | Major | Suite blind to the live Major defects | **CLOSED (residual D-2)** | `test_bounds` catches the R1-01 class; PLAIN round trip added (`:155-161`); removal path exercised (`:182-201`). Residual: no byte-level zeroisation, service-variant or derive-failure tests; ASan not in CI (D-2). |
| M7-NC-04 | R1-03 | Minor | PLAIN protect/unprotect asymmetry | **CLOSED** | Protect emits `[SPI][payload]` (`ff_sdls.c:258-271`); Unprotect parses the SPI and strips it (`:335-349`); round-trip test passes (`:155-161`). |
| M7-NC-05 | R1-04 | Minor | Backend security minima not validated | **CLOSED** | `FF_SdlsSetBackend` rejects `mac_len == 0`, `iv_len < 4`, and the other minima (`ff_sdls.c:82-88`); `FF_SdlsSetBackend(NULL)` covered (`test_sdls.c:40`). |
| M7-NC-06 | R1-05 | Minor | Shared state not synchronised / contract not documented | **NOT CLOSED** | No PAL mutex in `ff_sdls.c` and no single-threaded-contract text in `ff_sdls.h` or `security-sdls.md` §5 (grep for `mutex`/`lock`/`thread`/`concurren` returns nothing in the SDLS sources). Neither predecessor alternative was applied. |
| M7-NC-07 | R1-06 | Minor | Rotation failure leaves SA inconsistent; Tick ignores error | **PARTIAL (D-3)** | Tick now propagates (`ff_sdls.c:223-224`), but `RotateKey` still wipes before deriving/incrementing epoch (`:201-208`); no derive-failure test. |
| M7-NC-08 | R2-02 | Minor | `test_sdls.c` not registered/re-qualified | **CLOSED** | `tool-qualification.md:7` rev **0.1.7**; History `:75` records `tests/unit/test_sdls.c` (69 checks incl. bounds and removal/reuse; ASan). |
| M7-NC-09 | R2-03 | Minor | M7 reviews claimed ahead of committed evidence; no review-log row | **NOT CLOSED (commit-time)** | `docs/reviews/README.md` still has no M7 row; `verification-results.md:232-236` still states "NCs dispositioned and deltas re-reviewed" and the review-evidence table (`:246-270`) has no M7 row; the M7 reports are still untracked. Must be completed at commit (N11). |
| M7-NC-10 | C-1..C-5 | Comment | Lean note / model fidelity / key exposure / policy-time / API robustness | **CLOSED (accepted, no action)** | Non-derogatory observations; no required change in this increment. |

### 6.2 R3/R4 base NCs (FRICK-REVIEW-0022)

| NC | Severity | Item | Delta verdict | Evidence |
| -- | -------- | ---- | ------------- | -------- |
| R3-3 | Minor | IV-uniqueness claim unbounded vs `iv_len >= 4` | **CLOSED** | `security-sdls.md:58-60` states "the backend must provide `iv_len >= 4` (enforced by `FF_SdlsSetBackend`)"; `ff_sdls.c:84` enforces it. |
| R4-1 | Minor | `ecss-criticality-b.md` revision/History not bumped | **CLOSED** | Header **0.4** (`:7`); M7 `History` row (`:84`, "gap reference GAP-A1..A14 (SDLS security)"). |
| R4-2 | Minor | ADR/header overstate reference-backend use | **CLOSED** | `ADR-0013-sdls.md:25-27` "used only by the unit tests (not built into the flight or simulator binaries)"; `ff_crypto_test.h:3-9` same. |
| R3-1 | Comment | SDLS standards absent from `Applicable standards` | **OPEN (non-blocking)** | `SSS.md:9` and `SRS.md:9` still list only ECSS + CCSDS 133.0-B-2. |
| R3-2 | Comment | Zeroisation clause verified only indirectly | **OPEN (non-blocking; = D-2a)** | No byte-level zeroisation assertion in `test_sdls.c`. |
| R4-3 | Comment | Baseline doc §1/§3 not extended for M7 | **OPEN (non-blocking)** | `ecss-criticality-b.md` §1/§3 have no CCSDS 355.0-B/352.0-B reference and no Security deliverable row (only the History mentions SDLS). |
| R4-4 | Comment | Missing trailing newline in `class-a-gaps.md`, `licensing.md` | **OPEN (non-blocking)** | `git diff` still shows `\ No newline at end of file` on both. |
| R4-5 | Comment | Sibling review id non-`FRICK-REVIEW-00nn`; M7 reports not logged | **PARTIAL** | Id defect **CLOSED** (R1/R2 report now `FRICK-REVIEW-0023`); README log rows still missing (see M7-NC-09). |

### 6.3 New delta non-conformances

| NC | Severity | Profile | Summary | Status |
| -- | -------- | ------- | ------- | ------ |
| D-1 | Minor | R4 | `CHANGELOG.md:69` says `test_sdls` (53 checks); actual is 69 | open |
| D-2 | Minor | R2 | R2-01 residual: no byte-level zeroisation / AUTH-only / ENC-only / derive-failure tests; ASan not in CI | open |
| D-3 | Minor | R1 | R1-06 residual: `FF_SdlsRotateKey` still wipes-before-derive; failed KDF leaves an active SA with an all-zero session key | open |

### 6.4 Conformance verified (no finding)

- **Traceability gate:** `trace.py` exit 0 — 69 requirements (36 srs / 19 sss /
  14 gap), 51 Lean theorems, 9 CI claims; `SSS-REQ-019 <- 2 srs children`.
- **Build/test/ASan:** Debug build clean; 8/8 ctest pass; `test_sdls`
  `checks=69 failures=0`; total 294 checks consistent
  (15+20+9+29+101+31+20+69) with `verification-results.md:26,:224`.
- **Major fixes reproduced independently** under ASan (R1-01 → `FF_BAD_SIZE`,
  no OOB; R1-02 → tail and whole table zeroed).
- **Document ids:** 0 duplicate `Document id` declarations; M7 reports
  `FRICK-REVIEW-0022`/`-0023`.
- **README metrics panel** matches `metrics.py` byte-for-byte; coverage
  `1192/1925` lines, `172/214` functions (`build/coverage/coverage.json`).
- **Lean:** `verification/lean/lake-build.log` ends `Build completed
  successfully` (21/21); `Frick.Sdls` defines `sa_add_bound`,
  `key_rotate_epoch_monotonic`, `key_rotate_uses_reset`, wrapped as
  `req_srs035_*` in `Requirements.lean:172-184`.
- **CI wiring:** `security-sdls.md` is in the required-docs list of both
  workflows (`.github/workflows/ci.yml:139`, `.gitea/workflows/ci.yml:180`).
- **PLAIN symmetry, backend minima, Tick propagation** match the reported
  dispositions (see §6.1).

---

> **Closure update (M7 cycle).** R1-06/D-3: `FF_SdlsRotateKey` now derives into a
> temporary and only swaps on success (a failed KDF leaves the active key
> intact). R1-05: the `ff_sdls.h` concurrency contract is documented
> (control-plane single-threaded; datapath on the link task). R2-03: M7 rows
> added to `docs/reviews/README.md` and `verification-results.md`. D-1:
> `CHANGELOG.md` corrected to 81 checks. D-2: AUTH-only/ENC-only round-trip
> tests added, and **AddressSanitizer is wired into CI** (both forges, build
> job). All M7 NCs are closed (norm N12).
