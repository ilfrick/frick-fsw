# Review Report — N12 delta re-review: SRS-REQ-055 reference eNVM NVM backend

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0087                      |
| Reviewer session                  | (independent agent, norm N9)           |
| Reviewer profile                  | R2 (SW V&V)                            |
| Reference revision                | HEAD `03962d6`                         |
| Items under review                | Prior review FRICK-REVIEW-0082 (`docs/reviews/2026-10-11-nvm-r2-review.md`, findings 1-7): `tests/unit/test_nvm.c`; `src/drivers/nvm/mpfs/ff_nvm_mpfs.{c,h}`; `config/platform/platform_mpfs.h`; `docs/vv/verification-results.md` (row/0.26); `docs/vv/tool-qualification.md` (T3/T17 + History); `docs/requirements/traceability.md` + `.yaml` (SRS-REQ-055); `CHANGELOG.md`; `src/apps/app_renode_test.c` (`hal_nvm`); `.github/workflows/ci.yml`; `.gitea/workflows/ci.yml` |
| Date                              | 2026-10-11                             |
| Scope                             | Delta re-review (norm N12) of the fixes for FRICK-REVIEW-0082 Minor 1-4 and Comments 5-7 at HEAD `03962d6`; independently re-executed gates; no new scope. Against ECSS-E-ST-40C Rev.1, ECSS-Q-ST-80C Rev.2, ECSS-E-ST-10-02C Rev.1. |
| Independence declaration          | Fresh context; the reviewer authored/executed none of the items under review and read only the repository at `03962d6`. |
| Norms applied                     | N1-N20 (see `docs/rules.md`); emphasis N4, N6, N9, N10, N11, N12, N18 |

## 1. Scope

Re-reviewed only the deltas introduced by commit `03962d6` that address
FRICK-REVIEW-0082:

- NC-01 / finding 1: executed-check count in the records;
- NC-02 / finding 2: `FF_NvmMpfsSelfTest` scratch region and its doc/comment
  claim;
- NC-03 / finding 3: `nvm_init` dead code; corrupt-length clamp coverage;
  max-legal-length (`len == FF_NVM_MAX_VALUE`) coverage;
- NC-04 / finding 4: T17 register row lists `hal_nvm`;
- finding 5 (Comment): `ci-renode` provenance parentheticals;
- finding 6 (Comment): Renode `hal_nvm` scope honesty;
- finding 7 (Comment): host mock modulo/bounds robustness.

Excluded (unchanged by this delta, previously dispositioned): real-silicon
behaviour, eNVM endurance/retention (GAP-A5), the other M11 peripheral classes,
the Lean proofs, and the M4/M11 HAL classes other than NVM.

Commands executed independently on 2026-10-11 at `03962d6`:

- `cmake --build build/native` → up to date; `ctest --test-dir build/native` →
  **`100% tests passed, 0 tests failed out of 17`** (T17 sets include `nvm`,
  `17/17`).
- `./build/native/tests/test_nvm` → **`checks=36 failures=0`** (exit 0).
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` →
  **`traceability: PASS (97 requirements, 56 srs, 27 sss, 14 gaps, 51 lean
  theorems checked)`** — matches the claimed `97/56/27/14/51`.
- `make coverage` → `line coverage: 2903/4070 (71.3%)`; fresh `gcov` on
  `ff_nvm_mpfs.c` inspected directly (clamp executed; `nvm_init` absent).
- `clang-format --dry-run --Werror` on the three changed C/H files → exit 0.

## 2. References

- Project norms: `docs/rules.md` (N1-N20)
- Prior review: `docs/reviews/2026-10-11-nvm-r2-review.md` (FRICK-REVIEW-0082)
- Traceability manifest: `docs/requirements/traceability.yaml` (SRS-REQ-055)
- ECSS-E-ST-40C Rev.1 (software verification), ECSS-Q-ST-80C Rev.2 (software
  product assurance, tool qualification), ECSS-E-ST-10-02C Rev.1 (verification)
- HAL contract: `src/hal/hal_mem.h`

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| D1 | Minor | NC-02 residual — `ff_nvm_mpfs.h` doc claim (norms N8/N11) | The scratch relocation is correct, but the **header doc-comment still over-claims**: `FF_NvmMpfsSelfTest` is documented as *"the ... self-test used by the Renode functional test"*, yet the Renode `hal_nvm` check still only performs `setup` + `read` and never calls it (no caller anywhere outside `test_nvm.c`). This is the exact "comment/claim" sub-item of NC-02; the in-function comment was fixed, the header claim was not. | `src/drivers/nvm/mpfs/ff_nvm_mpfs.h:47-48`; `src/apps/app_renode_test.c:225-236` (`renode_hal_nvm_check`: setup+read only); `app_renode_test.c:289`; grep shows no `FF_NvmMpfsSelfTest` caller outside `tests/unit/test_nvm.c` | Reword to "available for a future Renode/diagnostic use; not currently wired" **or** wire it (the scratch is now outside the id table) — then close NC-02. |
| D2 | Comment | NC-01 residual — evidence-history accuracy (norm N11) | The **current** count is now correct everywhere (`test_nvm` 36 checks, verified `checks=36`). However the revision-history/commit narrative states `14 -> 36`; the pre-fix *executed* count was **29**, not 14 (the "14" was the misrecorded figure that NC-01 itself was raised against). The history thus perpetuates the wrong baseline. | `docs/vv/tool-qualification.md:108` ("14 -> 36"), `docs/vv/verification-results.md` row/History, `CHANGELOG.md:14`, commit `03962d6` message ("test_nvm 14 -> 36"); `docs/reviews/2026-10-11-nvm-r1-review.md:69` and FRICK-REVIEW-0082 reproduce `checks=29` pre-fix | Restate as "29 -> 36 (record previously misstated as 14)". Advisory; the delivered count is correct. |
| D3 | Comment | Host-test evidence strength for scratch isolation (finding 7, still open) | `FF_NVM_SCRATCH_OFF` = `32 x 32` = **1024**, exactly `sizeof(mock mem)` (`test_nvm.c:18`), and the mock indexes with `off % sizeof(m->mem)` (`:27`,`:33`). The host self-test therefore aliases offsets 1024/1025 onto slot 0 (indices 0/1). No assertion is corrupted today (slot 0 is not asserted at `:105`), but the host suite **cannot evidence the "scratch outside the id table" property** — that rests on source inspection only. The modulo also still masks any future out-of-bounds offset. | `tests/unit/test_nvm.c:18,27,33,105`; `src/drivers/nvm/mpfs/ff_nvm_mpfs.h:35`; gcov `FF_NvmMpfsSelfTest` exercised at `:80-82` | Size the mock to include the scratch window and replace the modulo with a hard tripwire (`assert(off < sizeof(m->mem))`), so the isolation claim is positively demonstrated. Advisory (not a driver defect). |
| D4 | Comment | NC finding 5 residual — CI provenance (norm N6) | Both workflow `ci-renode` provenance markers still omit `SRS-REQ-055` (they list 036/039/044/047/048/049/050 + SSS-013/020). The gate (`trace.py`) only checks claim-name existence, so this is advisory and was not enforced. | `.github/workflows/ci.yml:80`; `.gitea/workflows/ci.yml:83` | Add `SRS-REQ-055` to both parentheticals (advisory, non-blocking). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### Verified closures and positives

- **NC-01 (count):** closed on the substantive point. The record now reads
  `test_nvm` **36 checks** (`docs/vv/verification-results.md:38`; also
  `CHANGELOG.md:14`), matching `./build/native/tests/test_nvm checks=36
  failures=0`. Only the history narrative (`14 -> 36`) is inaccurate (D2).
- **NC-02 (scratch / destructive slot):** the safety-relevant part is **fixed**.
  The scratch is now `FF_NVM_SCRATCH_OFF = FF_NVM_MAX_IDS * FF_NVM_SLOT_SIZE`
  (`ff_nvm_mpfs.h:35`), i.e. offset **1024**, outside the slot table
  (id 31 occupies offsets 992..1023, so no valid id is clobbered at source; ids
  `0..31` all remain writable). The false in-code comment ("last slot (not part
  of the value space)") was corrected (`ff_nvm_mpfs.c:79`). **Residual:** the
  header claim that it is "used by the Renode functional test" is still false
  (D1), so the NC-02 comment/claim sub-item is **not fully closed**.
- **NC-03 (coverage):** closed. `nvm_init` is removed from the driver (fresh
  `gcov` has no `nvm_init` symbol; functions are `nvm_off`/`nvm_write`/
  `nvm_read`/`nvm_erase`/`FF_NvmMpfsSelfTest`/`FF_NvmMpfsSetup`). The
  corrupt-length clamp `ff_nvm_mpfs.c:49-51` is now executed
  (`gcov`: line 50 count 2; branch `n > FF_NVM_MAX_VALUE` taken 2). The
  max-legal-length success (`len == FF_NVM_MAX_VALUE`) is exercised
  (`test_nvm.c:84-96`) and the clamp is asserted (`test_nvm.c:98-102`);
  `nvm_read` shows 100% function coverage.
- **NC-04 (T17):** closed. The T17 register row now lists the HAL checks as
  `hal_gpio`/`hal_serial`/`hal_mem`/`hal_eth`/`hal_net`/**`hal_nvm`**
  (`docs/vv/tool-qualification.md:41`), consistent with the 0.3.9/0.3.10 history.
- **finding 6 (Renode scope):** verified **honestly scoped** — the code comment
  states it is an "MMIO presence/identification check", defers write/erase
  register logic to `test_nvm` and endurance to GAP-A5, and the evidence row
  says "MMIO presence check" (`app_renode_test.c:221-224`;
  `docs/vv/verification-results.md:38`). No write/endurance over-claim. (D1
  slightly muddies this by implying a self-test wiring that does not exist.)
- **Traceability:** `trace.py` green at `97/56/27/14/51`; SRS-REQ-055 lists
  existing implementation/test/CI paths (`traceability.yaml:521`;
  `traceability.md:116`).
- **Format:** `clang-format --dry-run --Werror` clean on the changed C/H files.

### Assessment against the review questions

- **Is the check count now 36 in the records?** Yes — records and binary agree
  at 36; only the change narrative retains the wrong pre-fix baseline (D2).
- **Are the corrupt-length clamp and max-legal-length now tested?** Yes — both
  are asserted and gcov-confirmed executed.
- **Does the self-test scratch no longer overwrite a valid id?** At source,
  yes (offset 1024 > slot table). The host test does not positively demonstrate
  it (mock modulo, D3), and the header still over-claims a Renode caller (D1).
- **Does T17 list `hal_nvm`?** Yes.
- **Is the Renode check still honestly scoped?** Yes — presence only, not
  write/endurance.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

No Major findings. The safety-relevant defect of FRICK-REVIEW-0082 (destructive
write to a valid slot) is corrected at source, and NC-01/NC-03/NC-04 are closed.
One residual **Minor** (D1: the header doc-comment still claims a Renode caller
that does not exist) means **FRICK-REVIEW-0082 is NOT fully closed**. D2-D4 are
advisory Comments. The functionally observable behaviour required by SRS-REQ-055
(bounded ids/slots, `FF_Nvm_t` conformance, argument/error handling, clamp
robustness) is demonstrated within the honestly stated evidence envelope.

**Closure statement.** FRICK-REVIEW-0082: NC-01 closed (D2 narrative nit),
NC-02 **partially closed / re-opened** as D1, NC-03 closed, NC-04 closed
(Comments 5/7 remain open advisories). Therefore FRICK-REVIEW-0082 remains
**open** pending D1 (and, optionally, the D2/D3/D4 advisories).

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R2 (N9)       | DeepSeek V4.1 Flash (independent SW V&V agent) | 2026-10-11 | APPROVED WITH COMMENTS |

## 6. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-0082-01 | Minor | SRS-REQ-055 evidence | `test_nvm` recorded as 14; actual 29 | Correct record | Record + binary now **36 checks**; only history text `14 -> 36` inaccurate | frick-fsw | closed (narrative nit D2) | FRICK-REVIEW-0087 |
| NC-0082-02 | Minor | `ff_nvm_mpfs.{c,h}` self-test | Comment/claim false; overwrote valid slot 31 | Relocate scratch; fix comment/claim | Scratch now `FF_NVM_SCRATCH_OFF=1024` outside id table; in-code comment fixed; **header claim still false (D1)** | frick-fsw | **open (residual D1)** | FRICK-REVIEW-0087 |
| NC-0082-03 | Minor | new-driver coverage | `nvm_init` dead; clamp/max-legal untested | Remove/wire `nvm_init`; add tests | `nvm_init` removed; clamp executed (gcov line 50); max-legal tested (`test_nvm.c:84-102`) | frick-fsw | closed | FRICK-REVIEW-0087 |
| NC-0082-04 | Minor | `tool-qualification.md` T17 | T17 row omitted `hal_nvm` | Add `hal_nvm` | `tool-qualification.md:41` lists `hal_nvm` | frick-fsw | closed | FRICK-REVIEW-0087 |
| NC-0082-05 | Comment | CI provenance | `ci-renode` marker omits SRS-REQ-055 | Add to both workflows | `.github/workflows/ci.yml:80`; `.gitea/workflows/ci.yml:83` still omit (D4) | frick-fsw | open (advisory) | FRICK-REVIEW-0087 |
| NC-0082-07 | Comment | Host mock robustness | Mock masks OOB offset with modulo | Hard bounds tripwire | `test_nvm.c:27,33` unchanged; now also aliases scratch to slot 0 (D3) | frick-fsw | open (advisory) | FRICK-REVIEW-0087 |

---

Committed with the delta re-review record (norm N11). No repository file other
than this report was modified.