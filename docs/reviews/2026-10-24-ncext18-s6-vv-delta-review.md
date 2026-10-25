# Independent V&V DELTA Review — NC-EXT-18 (PUS S6): verification of the corrections applied after FRICK-REVIEW-0437

|                                   |                                                                 |
| --------------------------------: | :-------------------------------------------------------------- |
| Document id                       | FRICK-REVIEW-0438 (delta of FRICK-REVIEW-0437)                   |
| Reviewer session                  | Independent agent, fresh context (norm N9)                      |
| Reviewer profile                  | R2 — Senior Aeronautics and Space SW V&V Engineer                |
| Reference revision                | Uncommitted working tree at `HEAD 44d2af2` (2026-10-24)          |
| Items under review                | `src/pus/pus_services.c`, `tests/unit/test_pus.c`, `docs/interfaces/ICD.md`, `docs/adr/ADR-0020-provisional-pus-and-scheduler-policies.md`, `docs/vv/evidence/2026-10-24-ncext18-s6-refusal-causes-evidence.md` |
| Predecessor                       | `docs/reviews/2026-10-24-ncext18-s6-vv-review.md` (FRICK-REVIEW-0437, ACCEPT / APPROVED WITH COMMENTS, findings F01–F10) |
| Date                              | 2026-10-24                                                      |
| Scope                             | **Corrections only.** Verify that the four claimed corrections (F01, F02, F05, F06) are present and correct; that they introduce no weakened assertion, no behavioural/wire change and no authority overreach; re-execute the host gates |
| Out of scope                      | Re-review of the accepted S6 design (settled in FRICK-REVIEW-0437); the deferred findings F03/F04/F07–F10; every other increment in the same uncommitted batch |
| Independence declaration          | I authored FRICK-REVIEW-0437. For this delta I re-derived the evidence mechanically instead of trusting the author's transcript: byte-exact reconstruction of the previously reviewed test artefact, an independent mutation re-run, an exhaustive aliasing enumeration of the new oracle pattern, and a static precedence derivation against the code |

**Authority statement.** This report is evidence and recommendation only. It does not approve qualification, acceptance, compliance, residual risk, release or flight readiness; it does not close any NC, refresh a frozen manifest, or authorize commit/push. **NC-EXT-18 remains OPEN.** **N18 remains FAIL (NC-EXT-198)** and was not run. **NC-PAL-01 remains OPEN.**

---

## 1. Candidate identity

### 1.1 SHA-256 verification (recomputed at review start *and* after all execution)

| File | Stated candidate | Recomputed | vs FRICK-REVIEW-0437 | Verdict |
|---|---|---|---|---|
| `src/pus/pus_services.c` | `055086117dfb0596397fbe78c860e008394685f54f06a488cf9df4bf188dcebb` | identical | **same hash** | **MATCH — production source unchanged** |
| `tests/unit/test_pus.c` | `ac6889745882b4c001a4913cfe2564aef8a17ff4ef7356c8b9a69eeac14f1d71` | identical | changed (`0d2e69dc…`) — F02 | **MATCH** |
| `docs/interfaces/ICD.md` | `67ae7584a4f8adb50210be0933d5ea6f10e5ac03466a3c09e67b105d452643ad` | identical | changed (`9bf44719…`) — F05 | **MATCH** |
| `docs/adr/ADR-0020-…md` | `b7e33e1af921a81ec0604fb464f9fd4c200bf16b9741d06e615ab07d540bcb29` | identical | changed (`93aaa4f5…`) — F06 | **MATCH** |
| evidence record (not a stated candidate hash) | — | `f3ab04d2ac3e182448e459b32fe28105390d918729717b043872895301433a4c` | changed (rev 0.2) | recorded |

Both recomputations are byte-identical: **the 43 MB of build output and the mutant runs below did not modify the repository.**

### 1.2 The delta is provably minimal (mechanical, not asserted)

`tests/unit/test_pus.c` grew by 8 lines relative to the artefact I reviewed. Deleting **exactly lines 177–184** (the F02 comment block, the fill loop and the separating blank line) from the candidate reproduces

```
0d2e69dca4ed850c0f83ec880dccbaa0b3eb50e795da8fd37125d1d4099ee0b9  prior_test_pus.c
```

**byte-identical to the `tests/unit/test_pus.c` hash recorded in FRICK-REVIEW-0437 §1.1.** Consequences:

1. F02 is the **only** change to the test artefact. No assertion was added, deleted, reordered, relaxed or re-indented; the executed check count is therefore provably unchanged.
2. The pre-fix failure sites recorded in FRICK-REVIEW-0437 (230, 263, 264, 266, 276, 290×3, 312, 323) map one-for-one onto the candidate at 238, 271, 272, 274, 284, 298×3, 320, 331 — all shifted by exactly the 8 inserted lines, which independently confirms the same assertion set.
3. `src/pus/pus_services.c` is byte-identical to the reviewed candidate, so the wrap-freedom, ABI and refusal-cause analyses of FRICK-REVIEW-0437 §3.2–§3.3 remain valid without re-derivation.

---

## 2. Per-correction verdicts

| Finding | Claimed correction | Verdict | Basis |
|---|---|---|---|
| **F01** | pre-fix count corrected to `134 checks / 10 failures`, with 108 labelled the pre-extension count | **PASS** | §2.1 |
| **F02** | `g_region` filled with `0xA5 ^ (i & 0xFF)` before registration; content oracle now discriminating; mutant proof | **PASS** (with one records residual) | §2.2 |
| **F05** | ICD states **no** refusal cause is wire-distinguishable and records the validation precedence | **PASS** (two advisory residuals) | §2.3 |
| **F06** | ADR header decision date restored to 2026-10-16, change carried in the revision history | **PASS** | §2.4 |

### 2.1 F01 — check-count correction: **PASS**

- The internal contradiction is gone. §2 now reports `checks=134 failures=10` for the reproduction and the former §5 claim ("108 checks / 10 failures pre-fix") no longer exists anywhere in the record.
- The new note is now verifiable rather than unexplained: "the pre-extension suite had 108 checks". An independent retained artefact corroborates it — `docs/vv/evidence/2026-10-24-hk-periodic-ncext17-evidence.md` records `tests/unit/test_pus.c checks=108 failures=0` for the predecessor (NC-EXT-17) increment, i.e. 108 is the check count of `test_pus` **before** the S6 extension and 134 is the extended count. This closes the gap I could not explain in FRICK-REVIEW-0437 §5/F01 (where the pre-extension `HEAD` figure was 61 checks).
- Arithmetic consistency: 134 − 108 = 26 NC-EXT-18 assertion executions added by the extension; the reproduction's 10 failures are a subset of those executions, which is consistent with the failure list (10 sites, one of them in a 3-iteration loop).

**Residual:** none. Records-only correction, no code impact, as recommended.

### 2.2 F02 — payload-content oracle: **PASS** — the oracle now genuinely binds content to offset

**Presence.** `tests/unit/test_pus.c:177–183` fills the region before registration (`FF_PusRegisterMemoryRegion` stores the *pointer*, `pus_services.c:201`, so the ordering claimed is also the ordering that would be required if it copied). The comment attributes the change to "independent V&V finding F02".

**Aliasing analysis (independent re-derivation, not the author's argument).** The pattern `p[i] = 0xA5 ^ (i & 0xFF)` is a bijection on `0..255` and therefore strictly injective on `0..63`. Two slices `[o1, o1+L)` and `[o2, o2+L)` are equal iff `(o1+k) ≡ (o2+k) (mod 256)` for all `k < L`, which forces `o1 ≡ o2 (mod 256)`. Every reachable offset here satisfies `o1, o2 < 64`, so **no distinct offsets can alias for any length**. Verified exhaustively over all ordered offset pairs and all fitting lengths:

```
pattern byte-range: A5..9A ; distinct values=64
pairs*lengths checked=87360 aliasing cases=0
injective over [0,64)=1
```

So the pattern cannot be "too weak by construction" for the asserted cases (offsets 0, 4, 63; lengths 1, 4, 64).

**Slice identity.** `stream_has_memory_bytes()` still compares against `&g_region[offset]` (`test_pus.c:41`) — the correct region slice, not a shifted or base slice — and `data_len == length` is retained, so the oracle still binds length *and* content.

**Mutation re-run on the final candidate (my own run, project compile flags, reviewed test object, mutant object linked into a copy of the archive; tree untouched).**

```
mutant: region->base + (ptrdiff_t)0   (offset dropped when forming src)
  checks=134 failures=3
  FAIL tests/unit/test_pus.c:209: stream_has_memory_bytes(4u, 4u)
  FAIL tests/unit/test_pus.c:228: stream_has_memory_bytes(sizeof(g_region) - 1u, 1u)
  FAIL tests/unit/test_pus.c:311: stream_has_memory_bytes(sizeof(g_region) - 1u, 1u)
restored source (candidate binary): checks=134 failures=0
```

This is the decisive delta result: mutant **M_D**, which survived with `134/0` in FRICK-REVIEW-0437, is now **killed by content**, not by a crash and not by an indirect length assertion. A second probe (`src = base + (offset - 1u)`, an off-by-one rather than a wrong-base pointer) is also detected: it fails at line 209 before terminating on a wild read for the `offset == 0` case — i.e. the oracle catches both "wrong base" and "wrong index" pointer defects of this shape.

**Residual (records only, deferred under N21):** the evidence record §4 enumerates **two** failing sites (228, 311) while correctly reporting `failures=3`; the third is line 209. The count and the kill verdict are right, the enumerated list is incomplete. No engineering impact; the transcript should list all three sites.

### 2.3 F05 — ICD wire-indistinguishability and validation precedence: **PASS** on both required elements

**Presence and correctness of "none".** `docs/interfaces/ICD.md` §4 row 6 now reads: "These causes are **distinct in software** but **none of them is distinguishable on the wire by design**". This is correct and I re-verified the mechanism in the unchanged production source: `pus.c:353–356` maps every non-`FF_SUCCESS` handler status to a single `S1(1,8)` with `FF_PUS_FAIL_EXECUTION`. Therefore `FF_BAD_SIZE`, `FF_NOT_FOUND` and `FF_INVALID_ARGUMENT` are all wire-identical, exactly as the revised row states. The previous "these two" implication is gone.

**Precedence: independently re-derived against the code (delta focus 1).**

| Order | ICD statement | Code (candidate) |
|---|---|---|
| 1 | payload length | `len != 7u` → `FF_BAD_SIZE` (`pus_services.c:466–468`) |
| 2 | `region id`/`length` range | `(region_id == 0u) \|\| (size == 0u) \|\| (size > FF_PUS_MAX_TM_FRAME)` → `FF_BAD_SIZE` (`:472–474`) |
| 3 | region resolution | registry scan, then `region == NULL` → `FF_NOT_FOUND` (`:479–492`) |
| 4 | region bounds | `offset <= region->size && (uint32_t)size <= (region->size - offset)` → else `FF_INVALID_ARGUMENT` (`:504–508`) |

The row's stated order ("payload length, then `region id`/`length` range, then region resolution, then region bounds") **matches the code exactly**, including the counter-intuitive but deliberate consequence that an *unknown* region combined with a zero or over-maximum `length` returns `FF_BAD_SIZE`, not `FF_NOT_FOUND` — the class I flagged in FRICK-REVIEW-0437 §3.3/§3.4 and which the tests do not assert. Because the production source is byte-identical to the reviewed artefact, my independent probe results for those combinations (BAD_SIZE precedence) remain valid.

**Residuals (advisory, deferred; no behaviour change):**

1. F05's optional third element — quoting the numeric interface value (1024) instead of the internal macro `FF_PUS_MAX_TM_FRAME` — was **not** applied. The row still cites the macro. Minor; the macro resolves to `1024u` (`config/fsw_config.h:83`).
2. The ICD history row 1.6 describes the refusal-cause distinction and the wire non-distinguishability but does **not** mention the added precedence or the "none of them" sharpening, and the document remains at rev 1.6 although its body changed between the two candidates. Since the increment is uncommitted and the row covers the same substantive change, I record this as an N16 precision observation rather than a non-conformity; if the row is touched before commit it should name the precedence.

### 2.4 F06 — ADR decision date: **PASS**

- Header now reads `| Date   | 2026-10-16 |` — the original decision date restored, not overwritten.
- The change is carried in the change history: row `0.2 | 2026-10-24 | … "Record the S6 refusal-cause distinction … the ordered/subtractive bound check … on-wire failure codes remain an interface-authority decision (NC-EXT-18). Provisional profile values unchanged"`. Rows 0.1 (2026-10-16) and 0.2 are both present and in order; nothing was lost.
- F06 offered two acceptable options (separate `Revised` field, or keep the header date and rely on the history table). The second was chosen and is implemented correctly. The ADR still states "Provisional profile values unchanged" and defers the failure-code allocation, so no authority is claimed.

**N16 discipline for both documents: PASS with observation** — ADR fully consistent; ICD as per §2.3 residual 2.

---

## 3. Non-regression checks (delta focus 3)

| Check | Result | Evidence |
|---|---|---|
| Check count did not drop | **PASS** — still **134** executed | direct run `checks=134 failures=0`; `HEAD` (61) → pre-extension (108) → extended (134) |
| No assertion added/removed/relaxed | **PASS** — mechanically proven | removing lines 177–184 reproduces the previously reviewed file hash byte-exactly (§1.2) |
| No expectation weakened to hide a failure | **PASS** | the only semantic change is a *stronger* content oracle; the 10 pre-fix failure sites all remain as exact-status assertions |
| No previously passing case disturbed | **PASS** | `test_pus` 134/0; full suite 46/46 |
| No test-only shortcut introduced (sleep/skip/ignore) | **PASS** | no change outside the fill block; no new conditional, no seed/randomness |
| Oracle still independent of the implementation | **PASS with residual** | expected bytes come from the region contents, not from the handler; residual: expected data is read from the same buffer object the SUT reads, so a hypothetical SUT that *mutated* the region would be self-consistent (not exercised; recorded as observation, no action for this increment) |

---

## 4. Authority-boundary containment (delta focus 5)

| Authority-blocked item | State | Evidence |
|---|---|---|
| `TC[6,5]` field widths/layout (`region id[1] + offset[4 BE] + length[2 BE]`, 7-octet payload) | **untouched** | production source hash identical to FRICK-REVIEW-0437 (`055086…`); ICD row field text unchanged |
| Memory-access authorization model | **untouched** | no authorization logic added/removed; source hash unchanged |
| PUS-C failure-code allocation | **untouched** | no `FF_PUS_FAIL_*` symbol added/removed; the only occurrence in the delta is the pre-existing comment text; test file contains no failure-code expectation (comment only) |
| `TM[6,6]` wire format (raw region octets) | **untouched** | emission call unchanged; ICD "raw region octets only" retained |
| Reviewed production source identity | **unchanged** | `src/pus/pus_services.c` = `055086117dfb…dcebb`, same as FRICK-REVIEW-0437 §1.1 |
| NC-EXT-18 / NC-PAL-01 / N18 status | **unchanged** | no disposition text edited by this delta; N18 not run; NC-EXT-198 still FAIL |

No authority-blocked item was touched. The corrections are confined to one test oracle and two descriptive documents.

---

## 5. Gates re-executed (exact results) (delta focus 6)

| # | Gate | Command / scope | Result |
|---|---|---|---|
| 1 | Candidate identity | `sha256sum` on the 4 files + evidence, start and end | **PASS** (4/4 exact; unchanged by the review) |
| 2 | Build + full suite | `make build`; `make test` | **PASS** — build exit 0; `100% tests passed, 0 tests failed out of 46` |
| 3 | Targeted S6 suite | `ctest --test-dir build/native -R "^pus$"` | **PASS** — `1/1 Test #8: pus … Passed` |
| 4 | Direct S6 run | `./build/native/tests/test_pus` | **PASS** — `checks=134 failures=0` |
| 5 | Format check | `docker run --rm … frick-vv:local make format-check` (`clang-format --dry-run --Werror`) | **PASS** — exit 0, no findings |
| 6 | Traceability (N4) | `make trace` | **PASS** — `120 requirements, 76 srs, 30 sss, 14 gaps, 68 lean theorems checked` |
| 7 | Mutation re-proof (F02) | offset-dropping mutant, project flags, archived relink | **PASS (killed)** — `134 checks / 3 failures` at 209, 228, 311 |
| 8 | Oracle aliasing enumeration | exhaustive over 87 360 offset/length pairs | **PASS** — 0 aliasing cases; pattern injective on `[0,64)` |
| 9 | `make verify-lean` | not re-run | **NOT RUN** — no Lean/requirement delta in this increment (author PASS not re-verified) |
| 10 | `make gsw-selfcheck` | not re-run | **NOT RUN** |
| 11 | `make renode` (N18) | prohibited by owner directive; remains FAIL under NC-EXT-198 | **NOT RUN (by instruction)** |
| 12 | RTEMS 6.2 cross build | not run | **NOT RUN** |
| 13 | RV32/RV64 execution of the bound check | not runnable without target/emulator | **NOT RUN** (type-level argument in FRICK-REVIEW-0437 §3.2 stands; source unchanged) |
| 14 | Sanitizer / ASAN leg | not run | **NOT RUN** |

The `renode`-labelled ctest case `renode_vv_verdict` ran inside gate 2 as a verdict-file checker; it is **not** an emulated execution and is not N18 evidence. N18 remains FAIL.

---

## 6. Residual observations (non-blocking, deferred under N21)

| # | Item | Observation | Disposition |
|---|---|---|---|
| O-1 | Evidence record §4 | Mutant transcript enumerates 2 failing sites (228, 311) but reports `failures=3`; the omitted site is 209 (confirmed by my re-run). | Records precision; deferred. Count and kill verdict are correct. |
| O-2 | NC register | The record states the remaining findings are "registered as NC-EXT-214..219", but no such rows exist in the live register (`docs/planning/sprint-plan.md` NC table ends at **NC-EXT-213**); the identifiers appear only in FRICK-REVIEW-0437 §9 and in this increment's evidence record. Moreover FRICK-REVIEW-0437 §9 had proposed 214=F01, 215=F02, 216=F03, 217=F04, 218=F05, so the present 214..219 mapping **collides** with the published proposal (and NC-EXT-218 denoted F05, now corrected). | Records/traceability; must be reconciled in the register (with fresh, non-colliding numbers allocated from the live register) before this batch is committed. Not a technical defect; no engineering change implied. |
| O-3 | ICD §4 + history | F05's optional numeric value (1024) not adopted; rev 1.6 history row does not mention the added precedence (§2.3 residuals). | Advisory; deferred. |
| O-4 | Oracle independence | Expected content is read from the same `g_region` object the SUT reads. A hypothetical handler that corrupted the buffer before returning would be self-consistent. Not reachable by any current mutation class. | Observation only; no action for this increment. |
| O-5 | Source comment vs ICD | The `svc6_dump` comment in `pus_services.c:476–479` still says "the two refusal causes … both causes", which is accurate for the two *software-distinct* causes but sits beside the ICD's "none of them is wire-distinguishable" (three causes). Source is byte-identical to the accepted artefact; no change recommended now. | Advisory; deferred. |

**No Major finding and no reproduced functional or test failure remains in the reviewed delta.** The corrections introduce no behaviour change, no weakened assertion and no authority decision.

---

## 7. What this delta does and does not support

**Supported**

- All four claimed corrections (F01, F02, F05, F06) are present in the exact candidate hashes and are correct.
- F02 is effective: the content oracle is injective over the entire reachable offset domain (0 aliasing cases in 87 360 combinations) and now kills the offset-dropping mutant on content (`134/3`) where it previously survived (`134/0`).
- The test artefact's delta is exactly 8 added lines; the executed check count is unchanged at 134 and no assertion was relaxed.
- The ICD's "none wire-distinguishable" claim and its validation precedence both match the (unchanged) production code, including the BAD_SIZE-before-resolution precedence.
- The ADR decision date is restored with the change carried in history.
- Production source and all authority-blocked items are untouched relative to FRICK-REVIEW-0437.

**Not supported (and not claimed)**

- No target, timing, WCET, coverage, fault-injection or qualification evidence for the S6 path; all evidence remains **host-native**.
- No N18 Renode, RTEMS cross-build, RV32/RV64 execution or sanitizer evidence.
- The corrections do not close NC-EXT-18, whose authority half (address width and region-ID in the controlled interface; PUS-C failure-code allocation) is untouched.
- No statement about the deferred findings F03/F04/F07–F10, other than that they remain open.
- No compliance, acceptance, residual-risk, release or flight-readiness conclusion.

---

## 8. Uncertainty and limitations

- The reconstruction in §1.2 deletes a contiguous 8-line block identified by inspection; it proves the *rest* of the file is byte-identical to FRICK-REVIEW-0437's artefact, which is a stronger statement than a line-by-line diff of a file I no longer hold. It assumes the block boundaries I chose (comment + loop + blank) are the entirety of the insertion — confirmed by the exact hash match, which would not occur if any other byte differed.
- The mutant was built by replacing one object inside a *copy* of the static archive with the project's exact compile flags, then relinking the reviewed test object. That is faithful for the handler logic; it is not a from-scratch CMake build of a mutated tree (deliberately, to keep the reviewed tree read-only).
- The aliasing enumeration is a property of the pattern and the asserted domain, not of every conceivable future test offset; it is valid for regions of ≤ 64 octets. A larger region filled with the same pattern would alias at offsets differing by 256, which is worth remembering if `g_region` grows.
- `make verify-lean`, `make gsw-selfcheck` and the RTEMS leg were not re-executed; the evidence record's PASS entries for them are author claims not verified here. They are unaffected by a test-file and documentation delta.
- NC-numbering and register state were checked in `docs/planning/sprint-plan.md`; another register location would change O-2.

---

## 9. Recommendation

**ACCEPT** the corrected increment (delta verdict: **PASS on all four corrections**; house taxonomy: **APPROVED WITH COMMENTS**). No correction was found to be wrong, incomplete in a way that affects the software evidence, or to have weakened the suite.

Conditions attached:

1. O-1 and O-2 are records actions to be completed **before commit**: list line 209 in the mutant transcript, and register the deferred findings in the live NC register under non-colliding identifiers (the evidence record's "registered as NC-EXT-214..219" is not yet true and its numbering collides with FRICK-REVIEW-0437 §9).
2. O-3/O-5 remain advisory/deferred under N21; they are not conditions for continuing unrelated development.
3. This ACCEPT covers **only** the four corrections in the exact candidate hashes of §1.1. It does not extend FRICK-REVIEW-0437's accepted technical scope, and it is **not** a qualification, acceptance, compliance, release or flight-readiness statement.
4. **NC-EXT-18 remains OPEN** (authority half), **N18 remains FAIL (NC-EXT-198) and was not run**, **NC-PAL-01 remains OPEN**.
5. Commit/push authority and all release gates remain with the designated FSW engineer and the authorized human authority (N7). This reviewer neither commits nor approves.

---

## 10. Revision history

| Rev | Date | Author | Change |
| --- | --- | --- | --- |
| 0.1 | 2026-10-24 | Independent R2 V&V reviewer (FRICK-REVIEW-0438) | Delta review of the four corrections applied after FRICK-REVIEW-0437: candidate identity, byte-exact proof that F02 is the only test-file change (prior hash reconstructed), aliasing enumeration of the new oracle pattern (0/87 360), independent mutation re-run (offset-dropped mutant killed 134/3 at 209/228/311), precedence re-derivation of the ICD text against the unchanged code, ADR date/history check, authority-boundary containment, gate re-execution (`make build`, `make test` 46/46, `pus` 134/0, format-check, `make trace`). All four corrections PASS; five non-blocking residual records/observation items; ACCEPT (approved with comments) with NC-EXT-18 still open and N18 still FAIL |
