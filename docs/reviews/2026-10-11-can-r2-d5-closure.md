# Independent Closure Review — R2-D5 (raw-AMR-write assertion independence), profile R2 (SW V&V)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                           | frick-fsw                              |
| Document id                       | FRICK-REVIEW-0118                      |
| Reviewer session                  | Independent agent, fresh context (norm N9) |
| Reviewer profile                  | R2 (Senior SW V&V engineer)            |
| Reference revision                | base commit `a95cafb` + CAN increment (working tree, uncommitted) |
| Items under review                | Closure of residual **R2-D5** (Minor) of FRICK-REVIEW-0115; artefacts `tests/unit/test_can.c` (sha256 `66c1405d…`, 129 checks), `src/drivers/can/mpfs/ff_can_mpfs.h` (sha256 `2ee9abed…`), `src/drivers/can/mpfs/ff_can_mpfs.c` (sha256 `342f8bfe…`); requirement SRS-REQ-057 |
| Date                              | 2026-10-11                             |
| Scope                             | Delta closure of the single R2-D5 Minor: confirm the added literal-bit assertion makes the raw-AMR-write regression independent of the shared `FF_CAN_ACCEPT_ALL` macro (ECSS-E-ST-40C verification, ECSS-E-ST-10-02C product verification, ECSS-Q-ST-80C PA, IEEE 1012 V&V framing) |
| Independence declaration          | I authored, designed, executed and dispositioned none of this change and had no access to the author's conversation, reasoning or intent beyond the repository records and the prior reviews. Every conclusion is grounded in a command I re-ran or a controlled artefact. The discrimination matrix was run in a private copy under `/tmp/opencode/r2d5`; the frozen files were never modified. Only this report file was created. I did not run Docker or `make renode`. |
| Norms applied                     | N1–N20 (`docs/rules.md`); focus N3/N4 (requirement + traceability), N9 (independence), N10/T3 (test qualification), N11 (evidence), N12 (NC closure), N15 (full cycle) |

## 1. Scope

Re-verification of the author's fix for **R2-D5** (FRICK-REVIEW-0115 §4): the
R2-D2 assertion `g_m.amr_written == (uint32_t)FF_CAN_ACCEPT_ALL`
(`test_can.c:326`) compares the captured raw AMR write to the *same macro* the
driver writes at `ff_can_mpfs.c:131`, so a macro-definition drop of the RTR/IDE
mask bits changes both sides together and escaped (128/0). The author added
immediately after it:

```c
FF_TEST_ASSERT((g_m.amr_written & 0x6u) == 0x6u); /* RTR+IDE mask bits, literal (R2-D5) */
```

(`test_can.c:327`), taking the suite from 128 to **129** checks.

Included: independent re-execution of the named gates; an independent
discrimination experiment in a **private** copy of the tree; a consistency read
of the changed test lines and header macro. Excluded: the R2-D3/R2-D4 optional
Comments (accepted in FRICK-REVIEW-0115); the R1/R3/R4 remits; the RTEMS/Lean
build chain; on-silicon (GAP-A5) evidence; and any re-review of the R2-01 fix
itself (already closed).

## 2. References

- Project norms: `docs/rules.md` (N1–N20); `docs/engineering/independent-review.md` (profiles, independence).
- Prior reviews: `docs/reviews/2026-10-11-can-r2-review.md` (FRICK-REVIEW-0107, R2-01); `docs/reviews/2026-10-11-can-r2-delta-review.md` (FRICK-REVIEW-0111); `docs/reviews/2026-10-11-can-r2-closure-review.md` (FRICK-REVIEW-0115, raised R2-D5).
- Requirement: SRS-REQ-057 (parent SSS-REQ-020); `docs/vv/verification-results.md` 0.30 (records `test_can` 129 checks).
- Harness contract: `tests/ff_test.h` (`FF_TEST_ASSERT` prints `FAIL <file>:<line>`; the check counter increments every assertion, so an added passing assertion raises the total).
- ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-E-ST-10-02C Rev.1 (1 Feb 2018), ECSS-Q-ST-80C Rev.2 (30 Apr 2025); target criticality B.

## 3. R2-D5 closure confirmation

| NC | Severity | Item / Requirement | Finding (FRICK-REVIEW-0115) | Fix at this revision | Independent re-verification | Status |
| -- | -------- | ------------------ | --------------------------- | -------------------- | --------------------------- | ------ |
| R2-D5 | Minor | SRS-REQ-057 / T3 (regression independence) | The R2-D2 raw-AMR-write assertion is not independent of the `FF_CAN_ACCEPT_ALL` macro: a macro-definition drop of the RTR/IDE mask bits changes driver write and expected value together and passes (128/0) | Added `FF_TEST_ASSERT((g_m.amr_written & 0x6u) == 0x6u);` at `test_can.c:327` (bits 1=RTR, 2=IDE, literal); suite 128 → **129** checks | Independent literal anchors bit positions 1 and 2; the private-build discrimination matrix (§4) shows every macro drop variant now fails at `:327` while the macro-coupled `:326` still passes → the new check is the discriminating one | **CLOSED** |

## 4. Independent discrimination experiment (private copy `/tmp/opencode/r2d5`)

Method: the frozen tree was copied (excluding `.git`/`build`) to a private
directory; `cmake -S . -B build/native -G Ninja` then
`cmake --build build/native --target test_can`. The header was backed up and each
variant patched at a **single** site, rebuilt and re-run. The copied header was
restored to its frozen sha256 `2ee9abed…` after each variant (final re-verified).
The real repository files were never touched (re-hashed intact, §6).

| Variant | Patch to `ff_can_mpfs.h` | `test_can` result | Which assertion fires |
| ------- | ------------------------ | ----------------- | --------------------- |
| M0 baseline (frozen) | none | **129 / 0** | — |
| M1 macro drops RTR **and** IDE | `FF_CAN_ACCEPT_ALL = (FF_CAN_ID_MASK << FF_CAN_ID_SHIFT)` | **129 / 1** | `:327` only |
| M2 macro drops RTR only | `… | FF_CAN_RX_AMR_IDE` | **129 / 1** | `:327` only |
| M3 macro drops IDE only | `… | FF_CAN_RX_AMR_RTR` | **129 / 1** | `:327` only |

Interpretation: in M1–M3 the macro-coupled assertion at `:326`
(`amr_written == FF_CAN_ACCEPT_ALL`) stays green because both operands lose the
bits together — this is exactly the R2-D5 failure mode. The literal-bit
assertion at `:327` now fails for every drop (full or partial), independently of
the macro. The same mutation run against the *pre-fix* suite (128 checks, `:327`
absent) would have remained 128/0 (reproduced as M1-equivalent in
FRICK-REVIEW-0115 §5), confirming the new line is what closes the gap.

**Residual observation (not part of R2-D5).** A swap of the two bit
*assignments* (RTR↔IDE) is not distinguished by `& 0x6 == 0x6`: both variants set
bits 1 and 2, so the mutant passes (reproduced: mutate `FF_CAN_RX_AMR_RTR` to
`(1u<<2)` and `FF_CAN_RX_AMR_IDE` to `(1u<<1)` → 129/0). The literal check anchors
the *presence* of the RTR/IDE mask bits, not the mapping of each name to its bit.
R2-D5 asked only that a **drop** be caught, and it is; the frozen macro is
correct, and the emulator/`MPFS_CAN` does not model these bits (silicon mapping
is GAP-A5). Recorded as optional **Comment C1**, no derogation.

## 5. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| C1 | Comment | SRS-REQ-057 / T3 | The literal check verifies bits 1 and 2 are set but not which name maps to which bit; a RTR↔IDE swap escapes both host assertions (129/0) | §4 M4; `ff_can_mpfs.h:116-117`; `test_can.c:327` | Optional: if the bit-to-name mapping ever needs host protection, assert the two bits through independent literals distinguished per name; otherwise leave to GAP-A5 on-silicon. No action required for R2-D5. |

No Major or Minor finding. R2-D5 is fully closed.

## 6. Gates re-run (real repository, frozen revision)

| # | Check | Command | Result |
| - | ----- | ------- | ------ |
| G1 | Full suite | `ctest --test-dir build/native` | **19/19 PASS**, 0 failed |
| G2 | CAN suite | `./build/native/tests/test_can` | **checks=129 failures=0** |
| G3 | Frozen hashes | `sha256sum src/…/ff_can_mpfs.h tests/unit/test_can.c src/…/ff_can_mpfs.c` | `2ee9abed…` / `66c1405d…` / `342f8bfe…` — **unchanged** before and after |
| G4 | Private-copy baseline | `./build/native/tests/test_can` in `/tmp/opencode/r2d5` | 129 / 0 |

## 7. Verdict

- [x] **APPROVED** — R2-D5 (Minor) closed. The literal-bit assertion
  `(g_m.amr_written & 0x6u) == 0x6u` makes the raw-AMR-write regression
  independent of the `FF_CAN_ACCEPT_ALL` macro: a macro-definition drop of the
  RTR/IDE mask bits (full or partial) is now caught at `test_can.c:327` (M1–M3,
  129/1), where the macro-coupled assertion at `:326` remains green. Suite 129
  checks, ctest 19/19, frozen hashes intact.
- [ ] APPROVED WITH COMMENTS
- [ ] REJECTED

## 8. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
| ---- | ---------------- | ---- | ---------- |
| R2 (SW V&V, N9) | Independent agent (fresh context) | 2026-10-11 | APPROVED — R2-D5 closed; one optional Comment C1 (RTR↔IDE bit-assignment not host-distinguishable) |

## 9. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | ------ | --------------- |
| R2-D5 | Minor | SRS-REQ-057 / T3 | R2-D2 assertion not independent of the macro: macro drop passes 128/0 | fixed (literal-bit assertion at `test_can.c:327`; suite → 129) | §4 M1–M3; §6 G2 | frick-fsw | **closed** | this report |
| C1 | Comment | SRS-REQ-057 / T3 | RTR↔IDE bit-assignment swap not distinguished by `& 0x6` (host) | accepted (optional; frozen macro correct; silicon mapping = GAP-A5) | §4 M4; `ff_can_mpfs.h:116-117` | frick-fsw | **accepted (open)** | — |

## 10. Evidence consulted (exact commands and results)

- `git rev-parse HEAD` → `a95cafb876819331fc7a12aed117d7ea84a3bada` (working tree, uncommitted CAN increment).
- `sha256sum src/drivers/can/mpfs/ff_can_mpfs.h tests/unit/test_can.c src/drivers/can/mpfs/ff_can_mpfs.c` → `2ee9abed…`, `66c1405d…`, `342f8bfe…`.
- `grep -n "amr_written\|0x6u" tests/unit/test_can.c` → capture at `:138` (raw write), assertions at `:326`–`:327`.
- `read src/drivers/can/mpfs/ff_can_mpfs.h:112-141` → AMR layout; `FF_CAN_RX_AMR_RTR (1u<<1)`, `FF_CAN_RX_AMR_IDE (1u<<2)`, `FF_CAN_ACCEPT_ALL` macro.
- `cmake --build build/native --target test_can` → "no work to do" (build current with frozen sources).
- `ctest --test-dir build/native` → `100% tests passed, 0 tests failed out of 19`.
- `./build/native/tests/test_can` → `checks=129 failures=0`.
- `rsync -a --exclude=.git --exclude=build …/frick-fsw/ /tmp/opencode/r2d5/` then configure/build; private-copy baseline `checks=129 failures=0`.
- Mutants M1–M3 (macro drop) → `FAIL tests/unit/test_can.c:327: (g_m.amr_written & 0x6u) == 0x6u`; 129/1.
- Mutant M4 (RTR↔IDE swap) → 129/0 (Comment C1).
- Post-experiment restore → private header sha256 `2ee9abed…`.

---

Committed and pushed with the reviewed change (norm N11).
