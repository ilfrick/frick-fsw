# Independent V&V Review — NC-EXT-18: PUS S6 Memory-Access Contract (software-only fix)

|                                   |                                                                 |
| --------------------------------: | :-------------------------------------------------------------- |
| Document id                       | FRICK-REVIEW-0437                                               |
| Reviewer session                  | Independent agent, fresh context (norm N9)                      |
| Reviewer profile                  | R2 — Senior Aeronautics and Space SW V&V Engineer                |
| Reference revision                | Uncommitted working tree at `HEAD 44d2af2` (2026-10-24)          |
| Items under review                | `src/pus/pus_services.c`, `tests/unit/test_pus.c`, `docs/interfaces/ICD.md`, `docs/adr/ADR-0020-provisional-pus-and-scheduler-policies.md`, `docs/vv/evidence/2026-10-24-ncext18-s6-refusal-causes-evidence.md` |
| Date                              | 2026-10-24                                                      |
| Scope                             | Adversarial verification of the S6 refusal-cause fix: reproduction claim, bound-check arithmetic, refusal-cause completeness, test oracle strength, documentation accuracy, authority-boundary containment, gate re-execution |
| Independence declaration          | I did not author, execute or review-in-place any item under review in its original cycle; this is a first independent read of this increment's delta. I produced my own reproduction harness, probe and mutants (§2) rather than relying on the author's transcripts |
| Norms applied                     | N1–N22 (`docs/rules.md`), with N9, N11, N15, N16, N17, N21, N22 decisive here |

**Authority statement.** This report is evidence and recommendation only. It does not approve qualification, acceptance, compliance, residual risk, release or flight readiness; it does not close any NC, refresh a frozen manifest, or authorize commit/push. NC-EXT-18 **remains OPEN** (§7).

---

## 1. Scope, candidate identity, constraints

### 1.1 SHA-256 verification (recomputed at review start *and* at review end)

| File | Stated candidate | Recomputed | Verdict |
|---|---|---|---|
| `src/pus/pus_services.c` | `055086117dfb0596397fbe78c860e008394685f54f06a488cf9df4bf188dcebb` | identical | **MATCH** |
| `tests/unit/test_pus.c` | `0d2e69dca4ed850c0f83ec880dccbaa0b3eb50e795da8fd37125d1d4099ee0b9` | identical | **MATCH** |
| `docs/interfaces/ICD.md` | `9bf447193070e1f179a7b6eccc5f5545734d26821275a51ab7ea6cd6d7fead0d` | identical | **MATCH** |
| `docs/adr/ADR-0020-…md` | `93aaa4f53c6776c90866b8a4ebd3796db138a30b25b4b2d87ee1e5c76ddf9907` | identical | **MATCH** |

All four recomputed byte-for-byte after the review concluded: **the review did not modify the repository.**

### 1.2 The reviewed delta is provably minimal (this is a stronger result than the author claimed)

The evidence record states the pre-fix source was `49bc86db…6cc3cf`. I reconstructed that state mechanically: take the final candidate and substitute the *pre-fix* `svc6_dump()` body taken from `git show HEAD:src/pus/pus_services.c`. The result is

```
49bc86db75762cc4face23012cd110fc921d1176fb39251f92dcb7842f6cc3cf  recon_prefix.c
```

**byte-identical to the cited pre-fix hash.** Consequences, both verified:

1. The pre-fix state is exactly recoverable from the final candidate + `HEAD`, so the evidence anchor is auditable rather than asserted.
2. Between the previously reviewed NC-EXT-17 candidate and this NC-EXT-18 candidate, **every changed byte of `pus_services.c` lies inside `svc6_dump()`**. There is no collateral reformat, no unrelated edit, and no silent change to `TC[6,5]` parsing or `TM[6,6]` emission (see §7).

`test_pus.c` is part of a larger uncommitted batch (`+361` lines vs `HEAD`), covering both the NC-EXT-17 HK tests and this S6 extension; only the S6 block is reviewed here. `git diff --name-only` shows the working tree carries several other increments; they are out of scope and not re-derived.

### 1.3 Constraints observed

- **N18 / Renode: NOT RUN.** Remains FAIL under NC-EXT-198, per owner directive. The ctest case `renode_vv_verdict` (label `renode`) did run as part of `make test`; it is a *shell verdict-file checker* (`tests/shell/test_renode_vv_verdict.sh`), **not** an emulated execution, and is not N18 evidence.
- NC-PAL-01 remains OPEN; not touched.
- No emulator, hardware, mission value, mission table, frozen manifest, NC closure, commit or push.
- No sanitizer (ASAN) or RTEMS cross build was re-run by me.
- Non-functional findings are recorded as open/deferred NCs (§9); I corrected no code, because I found no reproduced functional or test failure.

---

## 2. Method — independent reconstruction, not re-reading the author's transcript

I did not accept the evidence record's reproduction on its face. Everything below was executed on this host at review time; the repository was never written to (work done in `/tmp/opencode/ncext18`).

| Step | What was done | Why |
|---|---|---|
| M1 | Rebuilt `svc6_dump` pre-fix state (see §1.2), compiled it with the project's exact compile flags (`flags.make` of `frick_fsw_core.dir`), substituted the object into a *copy* of `libfrick_fsw_core.a`, and relinked `test_pus` | Reproduce the claimed defect without touching the reviewed tree |
| M2 | Compiled `test_pus.c` (candidate hash) independently with the same flags | Ensure the test object is the reviewed artifact, not a stale build |
| M3 | Ran both binaries; compared failure sites and counts | Focus 1 |
| M4 | Wrote an **independent probe** (`probe_s6.c`, ~120 lines) from the ICD text and the requirement, enumerating every input class and capturing status + `S1(1,1)`/`S1(1,8)`/`TM(6,6)` counts | Focus 3/4: an oracle not derived from the reviewed test |
| M5 | Four mutation probes (M_A–M_D) against the fixed source | Focus 2/4: does the suite actually discriminate? |
| M6 | `_Static_assert` type/ABI checks compiled for LP64 **and** ILP32 (`-m32`) | Focus 2: RV32/RV64 equivalence at the type level |
| M7 | `make test`, targeted `ctest`, `format-check` in `frick-vv:local`, `make trace` | Focus 7 |

Run of the reconstruction (M1–M3), verbatim:

```
=== PRE-FIX (S6 hunk reverted only) ===
/home/nicola/fsw/frick-fsw/tests/unit/test_pus.c checks=134 failures=10
FAIL …test_pus.c:230: FF_PusIngest(frame, frame_len) == FF_INVALID_ARGUMENT
FAIL …test_pus.c:263: st_unknown == FF_NOT_FOUND
FAIL …test_pus.c:264: st_out_of_range == FF_INVALID_ARGUMENT
FAIL …test_pus.c:266: st_unknown != st_out_of_range
FAIL …test_pus.c:276: FF_PusIngest(frame, frame_len) == FF_NOT_FOUND
FAIL …test_pus.c:290: FF_PusIngest(frame, frame_len) == FF_INVALID_ARGUMENT   (x3, loop)
FAIL …test_pus.c:312: FF_PusIngest(frame, frame_len) == FF_INVALID_ARGUMENT
FAIL …test_pus.c:323: FF_PusIngest(frame, frame_len) == FF_NOT_FOUND

=== BASELINE (final candidate) ===
/home/nicola/fsw/frick-fsw/tests/unit/test_pus.c checks=134 failures=0
```

---

## 3. Per-focus verdicts

| # | Focus | Verdict |
|---|---|---|
| 1 | Reproduction claim and discriminating power of the new assertions | **VERIFIED** (§3.1) |
| 2 | Wrap-free range check, RV32/RV64 identity, `<=` vs `<` at the boundary | **CORRECT — no latent boundary bug** (§3.2) |
| 3 | Refusal-cause mapping complete and unambiguous for every input class | **VERIFIED complete**; two documentation precision gaps (§3.3, F05) |
| 4 | Real (non-tautological) oracles; wire behaviour asserted for both causes | **PARTIAL** — status and wire oracles sound; **payload-content oracle is degenerate** (§3.4, F02) |
| 5 | ICD 1.6 / ADR 0.2 accuracy, no false authority claim, N16 discipline | **ACCURATE, no authority claim**, history present (§3.5, F05/F06) |
| 6 | Authority-blocked items untouched; protected NCs unmodified | **VERIFIED untouched** (§3.6) |
| 7 | Gate re-execution | 4 PASS, 1 PASS-with-note, 5 NOT RUN (§8) |

### 3.1 Focus 1 — the reproduction claim holds, and the new assertions genuinely discriminate

- **The two causes could not be distinguished pre-fix.** Old `svc6_dump()` returned `FF_ERROR` from a single exit after the loop; both "no such region" and "outside the region" reached that same `return`. It was structurally impossible for a caller to separate them. Confirmed by rebuilding that exact state (§2).
- **The new assertions fail on the old code.** Ten assertions fail pre-fix at sites that map one-for-one onto the author's list: 230, 263, 264, 266, 276, 290×3, 312, 323 (author: 230, 263, 264, 266, 276, 291×3, 313, 324). The uniform −1 offset from line 290 onward is consistent with a one-line reflow between the author's reproduction run and the final test file; the *same ten sites* fail, so the discriminating power is unchanged and is now demonstrated **on the final artifact**, which is stronger than the original evidence.
- **The old suite encoded the defect.** I also built `HEAD:tests/unit/test_pus.c` against the fixed library: 2 assertions fail (`FF_PusIngest(...) == FF_ERROR`). This corroborates the evidence's claim that the prior test suite asserted the collapsed status, which is why the defect survived. The change replaces those two expectations with **stricter** ones (`== FF_NOT_FOUND`, `== FF_INVALID_ARGUMENT`), and total executed checks rose 61 → 134. **No test was weakened or deleted to make a failure disappear** — the direction of change is from weaker to stronger.
- Assertion `st_unknown != st_out_of_range` is the direct statement of the NC and is the one that cannot be satisfied by any single-status implementation.

**Verdict: VERIFIED.** One records inconsistency, not a scientific one (F01).

### 3.2 Focus 2 — the bound check cannot wrap and is ABI-identical; `<=` is not a latent bug

Code (`src/pus/pus_services.c:500`):

```c
if ((offset <= region->size) && ((uint32_t)size <= (region->size - offset))) {
    const uint8_t *src = region->base + (ptrdiff_t)offset;
```

Types: `offset` `uint32_t`, `size` `uint16_t` (parameter), `region->size` `uint32_t` (`ff_mem_region_t`, line 63). Therefore:

| Aspect | Finding |
|---|---|
| `region->size - offset` | Both operands `uint32_t`. On ILP32 and LP64, `uint32_t` is `unsigned int` (32-bit) and both have the same rank; **no widening to 64-bit and no signed conversion occurs**. Result is 32-bit unsigned. Because `offset <= region->size` has already short-circuited, the result lies in `[0, size]` — **it cannot wrap**. |
| `offset + length` | Never formed. The subtraction form is used precisely so no addition can wrap. |
| `(uint32_t)size` | `uint16_t` promoted to 32-bit unsigned; comparison is 32-bit unsigned on both ABIs. |
| ABI equivalence | Proven mechanically: a `_Static_assert` suite over `__typeof__(region->size - offset)` compiles and passes under `-m32` (ILP32) **and** default LP64: the difference type is 32-bit and unsigned, `ptrdiff_t` equals pointer width in both. So the **evaluation is identical on RV32 and RV64**. |
| `offset == region->size` | First operand passes; second evaluates `size >= 1 <= 0` → false → `FF_INVALID_ARGUMENT`. Correct. |
| `offset` very large (`0xFFFFFFFF`, `0xFFFFFFFE`, `0xFFFFFFFD`, `0xFFFFFFFC`) | `offset <= region->size(64)` false → `FF_INVALID_ARGUMENT` without evaluating the subtraction. Confirmed by probe (§4). |
| `length` large (`1024` at `offset 1`) | `1024 <= 64 - 1` false → `FF_INVALID_ARGUMENT` (correctly *not* a `BAD_SIZE`, because `1024 == FF_PUS_MAX_TM_FRAME`). |

**Is `offset <= region->size` (rather than `<`) correct?** Yes, and it is **not** a latent boundary bug.

- With `length == 0` rejected earlier (line 472), the only way `offset == region->size` could pass is `length == 0`, which is unreachable. Hence `<=` and `<` accept exactly the same set.
- I proved the equivalence empirically: a mutant replacing `<=` with `<` passes **all 134 checks**. So the two forms are observationally identical for every reachable input; **there is no off-by-one that the tests are hiding, and no defect to fix**.
- `<=` is additionally the form recorded in ADR-0020 ("validate `offset <= region_size` and `length <= region_size - offset`"), so `<=` is the text-consistent choice and the code matches the controlled document. Keeping it is correct; changing it to `<` would be gratuitous.
- Consequence worth noting: because `offset == size` can never reach the pointer expression, `base + size` (a one-past-the-end pointer) is never formed.

**Portability note (Comment, not a defect):** `(ptrdiff_t)offset` is 64-bit on RV64, so every reachable `offset ≤ 2^32−1` converts exactly. On an ILP32 target `ptrdiff_t` is 32-bit and the conversion would be implementation-defined for `offset > PTRDIFF_MAX`; that requires `region->size > 2 GiB`, which cannot be a real single object on ILP32 and is therefore unreachable through `FF_PusRegisterMemoryRegion` (which takes a real `base`/`size`). The claim in the code comment ("`offset` is already bounded by the region size before the pointer is formed") is therefore adequate — but it is a *reasoning* claim, not machine-checked (F09).

**Verdict: CORRECT.** Actual RV32/RV64 execution of this path: **NOT RUN** (no target or emulator permitted in this review).

### 3.3 Focus 3 — refusal-cause mapping is complete and unambiguous

Independent probe (M4), region 1 = 64 octets registered; `S1(1,8)` = completion-failure TM, `TM(6,6)` = dump:

| Input class | Observed status | `S1(1,1)` | `S1(1,8)` | `TM(6,6)` | Distinct? |
|---|---|---|---|---|---|
| payload len 6 (short) | `FF_BAD_SIZE` | 1 | 1 | 0 | ✔ malformed |
| payload len 8 (long) | `FF_BAD_SIZE` | 1 | 1 | 0 | ✔ malformed |
| region id 0, offset 0, len 4 | `FF_BAD_SIZE` | 1 | 1 | 0 | ✔ malformed (id 0 reserved) |
| id 1, len 0 | `FF_BAD_SIZE` | 1 | 1 | 0 | ✔ malformed |
| id 1, len 1025 (> `FF_PUS_MAX_TM_FRAME`) | `FF_BAD_SIZE` | 1 | 1 | 0 | ✔ malformed |
| **unknown id 9 + len 0** | `FF_BAD_SIZE` | 1 | 1 | 0 | precedence: malformed wins |
| **unknown id 9 + len 1025** | `FF_BAD_SIZE` | 1 | 1 | 0 | precedence: malformed wins |
| id 255 (unregistered) | `FF_NOT_FOUND` | 1 | 1 | 0 | ✔ cause A |
| id 2 (unregistered) | `FF_NOT_FOUND` | 1 | 1 | 0 | ✔ cause A |
| id 2 + offset `0xFFFFFFFF` | `FF_NOT_FOUND` | 1 | 1 | 0 | ✔ **resolution precedes range check** |
| id 255 + len 1024 | `FF_NOT_FOUND` | 1 | 1 | 0 | ✔ cause A |
| id 1, offset 64 (= size), len 1 | `FF_INVALID_ARGUMENT` | 1 | 1 | 0 | ✔ cause B |
| id 1, offset 63, len 2 | `FF_INVALID_ARGUMENT` | 1 | 1 | 0 | ✔ cause B |
| id 1, offset `0xFFFFFFFF`/`0xFFFFFFFE`/`0xFFFFFFFD`/`0xFFFFFFFC`, len 4 | `FF_INVALID_ARGUMENT` | 1 | 1 | 0 | ✔ cause B, no wrap |
| id 1, offset 1, len 1024 | `FF_INVALID_ARGUMENT` | 1 | 1 | 0 | ✔ cause B |
| id 1, offset 0, len 64 (whole region) | `FF_SUCCESS` | 1 | 0 | 1 | ✔ valid |
| id 1, offset 0/4/63 with fitting len | `FF_SUCCESS` | 1 | 0 | 1 | ✔ valid (incl. last byte) |
| refusal with `ack_flags = 0` (both causes) | cause status | 0 | 0 | 0 | ✔ no response at all |

- Every class maps to **exactly one** status; no class is unreachable and no two classes collide.
- The mapping matches the ICD row text after the exceptions documented in F05.
- One class is **not derived from the code by the requirement**: `FF_BAD_SIZE` is returned even for an *unknown* region whenever the request is also malformed. That precedence is reasonable and is deliberately ordered (cheap syntactic validation before registry lookup), but it is not stated in the ICD (F05).

**Verdict: COMPLETE and UNAMBIGUOUS.** Documented precedence gap → F05 (Minor).

### 3.4 Focus 4 — oracles: status and wire are real; the payload oracle is vacuous

| Oracle | Assessment |
|---|---|
| Status oracles (`== FF_NOT_FOUND`, `== FF_INVALID_ARGUMENT`, `== FF_BAD_SIZE`, `== FF_SUCCESS`) | **Real.** Exact enum comparisons against the reviewed implementation; independently reproduced by my probe. Not tautological. |
| Region-resolution precedence (`st_unknown` with adversarial offset) | **Real.** Fails if resolution is moved after the range check (my probe and the pre-fix run both discriminate). |
| Wire: `TM(6,6)` absent for refusals | Asserted at 231, 277, 291, 313, 323. **Real.** |
| Wire: `S1(1,8)` emitted for refusals | Asserted at 232, 292, 314 — i.e. for out-of-range, for the adversarial loop and for unknown id 2. **Both refusal causes have at least one case asserting both wire properties**, so the "unchanged on the wire" claim is pinned. ✔ |
| Wire: `S1(1,1)` acceptance, and "no response when the completion flag is clear" | **Not asserted.** My probe shows both are as the ICD describes (acceptance ack still produces `S1(1,1)`; `ack_flags = 0` produces nothing), but the suite does not pin this. F07. |
| **Payload-content oracle** (`stream_has_memory_bytes`) | **Degenerate.** `g_region` is a zero-filled `static uint8_t g_region[64]` that nothing ever writes; the oracle is `data_len == length && memcmp(data, &g_region[offset], length) == 0`. Comparing zeros with zeros cannot distinguish *which* offset the octets came from. |
| Proof of the gap | Mutant **M_D** (`src = region->base;` — the offset is ignored when forming the pointer, while the range check still passes) survives with **`checks=134 failures=0`**. A second-byte-off or wrong-sign pointer bug of the same shape would ship undetected by this suite. |

Mutation sensitivity summary (M5):

| Mutant | Change | Result | Interpretation |
|---|---|---|---|
| M_A | `offset <= size` → `offset < size` | **passes 134/0** | Confirms §3.2: the two forms are observationally equivalent; no hidden boundary bug |
| M_B | subtractive check → `offset + size <= size` (wrap-prone) | **crashes (exit 139)** | Detected, but by a wild-pointer read rather than a clean assertion. A future regression of this shape is caught only because the process dies |
| M_C | drop the length term (`offset <= size` only) | **6 failures** (230, 231, 232, 264, 312, 313) | Good sensitivity; the half-open pair (`size−1`/`2`) is what kills it |
| M_D | ignore `offset` when forming `src` | **passes 134/0** | **Oracle gap** — see F02 |

**Verdict: PARTIAL.** The status/wire oracles are sound and requirement-based; the payload oracle does not bind content to offset, and the task description's "positive markers" should be read as positive *assertions*, not content markers — there are none (F02, Minor, deferred).

### 3.5 Focus 5 — ICD rev 1.6 and ADR-0020 rev 0.2 are accurate and make no authority decision

Checked sentence by sentence against the code and my probe:

| Document claim | Verified against |
|---|---|
| `FF_BAD_SIZE` for `region id == 0`, `length == 0`, `length > FF_PUS_MAX_TM_FRAME` | code 472; probe rows 3–5 ✔ |
| `FF_NOT_FOUND` when no registered region carries the identifier | code 490–492; probe ✔ |
| `FF_INVALID_ARGUMENT` when the region exists but `offset + length` lies outside it | code 504; probe ✔ |
| "region resolution precedes the range check, so an unknown region is never reported as a range refusal" | probe: id 2 + `0xFFFFFFFF` → `FF_NOT_FOUND` ✔ |
| "distinct in software but … indistinguishable on the wire by design: both surface as S1(1,8) with `FF_PUS_FAIL_EXECUTION` … and produce no response when it is clear" | `pus.c:353-356`; probe: identical `S1(1,8)=1, TM(6,6)=0` for both causes; `ack_flags = 0` → nothing ✔ |
| "The PUS-C failure-code allocation … is an interface-authority decision and is not made here (NC-EXT-18)" | No `FF_PUS_FAIL_*` delta anywhere in the tree diff; no new failure code; NC-EXT-18 row unchanged ✔ |
| "No wire behavior changed by this revision" | Probe + byte-exact delta (§1.2) ✔ |
| ADR: "`FF_NOT_FOUND` … `FF_INVALID_ARGUMENT` … resolution precedes the range check … checked by ordered operands and subtraction in the region width, so neither `offset + length` nor `region_size - offset` can wrap on a 32-bit or 64-bit target" | §3.2 ✔ |
| ADR: "Provisional profile values unchanged" | Field widths/layout untouched ✔ |

- **No authority decision is claimed.** Both documents explicitly defer the failure-code allocation; neither asserts mission baselining, ECSS compliance, or that the provisional profile is ratified. The ICD provisional-profile note covering every §4 row is retained.
- **N16 discipline:** ICD header rev 1.6 + history row 1.6 (with 1.5 preceding it) ✔. ADR history row 0.2 added ✔.
- Two precision points: the ICD says "*these two* causes" are wire-indistinguishable, which leaves `FF_BAD_SIZE` looking distinguishable although it is not (`S1(1,8)`/`FF_PUS_FAIL_EXECUTION`, identical to the other two); and the malformed-before-lookup precedence is undocumented (F05). The ADR header `Date` was overwritten 2026-10-16 → 2026-10-24, which erases the original decision date from the header (preserved only in the history table) (F06).

**Verdict: ACCURATE, no authority overreach.** Two Minor/one Comment documentation items.

### 3.6 Focus 6 — authority-blocked items are untouched; protected NCs unmodified

The byte-exact reconstruction in §1.2 is the decisive evidence: **every changed byte of `pus_services.c` is inside `svc6_dump()`'s refusal logic.**

| Authority-blocked item | State | Evidence |
|---|---|---|
| `TC[6,5]` field widths/layout (`region id[1] + offset[4 BE] + length[2 BE]`, 7-octet payload) | **unchanged** | parse block (lines 466–474) byte-identical to `49bc86db`; ICD row field text unchanged |
| Memory-access authorization model | **unchanged** | no authorization check added or removed; `FF_PusRegisterMemoryRegion` untouched (`git diff` shows no hunk) |
| PUS-C failure-code allocation | **unchanged** | `git diff` contains **no** added/removed `FF_PUS_FAIL_*` symbol; both causes still `FF_PUS_FAIL_EXECUTION` |
| `TM[6,6]` wire format (raw region octets) | **unchanged** | emission call `FF_PusSendTm(FF_PUS_SERVICE_MEMORY, 6u, src, size)` unchanged; probe shows identical TM |
| NC-EXT-18 status | **still OPEN**, disposition text untouched | `docs/planning/sprint-plan.md:153` unchanged |
| NC-EXT-14 / 16 / 22 | **not modified** | no diff line touches their rows (`git diff docs/planning/sprint-plan.md`, `docs/status/loop-audit.md` C-04/C-06/C-14 rows: no changes) |
| NC-EXT-207 / 208 | **not modified by this increment** | they do not exist at `HEAD` (count 0) — they were registered by the NC-EXT-17 HK increment inside the *same* uncommitted batch. This increment's own delta is the 4 files + evidence record, so it cannot have altered them |

**Verdict: VERIFIED untouched.** Explicitly recorded so the increment cannot be misread as having dispositioned the authority half of NC-EXT-18.

---

## 4. Gate results (exact)

| # | Gate | Command / scope | Result |
|---|---|---|---|
| 1 | Candidate identity | `sha256sum` on the 4 files, start and end of review | **PASS** (4/4 exact, unchanged) |
| 2 | Build + full unit/integration suite | `make test` | **PASS** — `100% tests passed, 0 tests failed out of 46` |
| 3 | Targeted S6 suite | `ctest --test-dir build/native -R "^pus$"` and `./build/native/tests/test_pus` | **PASS** — `checks=134 failures=0` |
| 4 | Defect reproduction | my reconstruction + relink (§2) | **PASS (reproduced)** — `checks=134 failures=10`, same 10 sites |
| 5 | Format check | `docker run --rm -v "$PWD:/workspace" -w /workspace frick-vv:local make format-check` → `Ubuntu clang-format version 18.1.3 (1ubuntu1)` | **PASS** — `exit=0`, no findings |
| 6 | Traceability | `make trace` | **PASS** — `120 requirements, 76 srs, 30 sss, 14 gaps, 68 lean theorems checked` |
| 7 | `make verify-lean` | not re-run by me (author: PASS, `lake build`) | **NOT RUN** (author claim not re-verified) |
| 8 | `make gsw-selfcheck` | not re-run | **NOT RUN** |
| 9 | `make renode` (N18) | prohibited by owner directive; remains FAIL under NC-EXT-198 | **NOT RUN (by instruction)** |
| 10 | RTEMS 6.2 cross build | not run | **NOT RUN** |
| 11 | RV32/RV64 execution of the bound check | not runnable without target/emulator | **NOT RUN** (type-level only, §3.2) |
| 12 | Sanitizer / ASAN leg | not run | **NOT RUN** |

Note on gate 2: the ctest case `renode_vv_verdict` carries the `renode` label and passed; it is a verdict-file checker script, **not** an emulated run, and is not N18 evidence. N18 remains FAIL.

---

## 5. Findings

| # | Severity | Item | Finding | Evidence | Recommendation |
|---|---|---|---|---|---|
| **F01** | **Minor** | Evidence record §5 | Arithmetic inconsistency inside the record: §2 states the pre-fix run as `checks=134 failures=10`, while §5 states "was 108 checks / 10 failures pre-fix". My reproduction gives **134/10**, matching §2. 108 is unexplained (the pre-extension `HEAD` suite is 61 checks; the NC-EXT-17 candidate's figure is not reconstructable). | §2 reproduction transcript above; `HEAD:tests/unit/test_pus.c` built and run → `checks=61 failures=2` | Correct §5 to `134 checks / 10 failures pre-fix`. Records-only, no code impact. |
| **F02** | **Minor** | `tests/unit/test_pus.c` payload oracle | `stream_has_memory_bytes()` compares the dumped octets with `&g_region[offset]`, but `g_region` is a zero-filled static that is never written. Content equality is therefore vacuous and cannot bind the payload to the requested offset. Mutant **M_D** (offset ignored when forming `src`) survives with `134/0`. The task summary's "positive markers" do not exist as content; only the length is effectively checked. | M5 mutant table; `grep` shows no write to `g_region` | Fill `g_region` with a deterministic non-uniform pattern (e.g. `g_region[i] = (uint8_t)(i * 7u + 1u)`) so `memcmp` genuinely discriminates offset. Test-strength improvement; **not** a functional defect, so deferred under N21. |
| **F03** | **Minor** | Records (N8/N11/N16) | This increment is recorded only in the ICD, the ADR and its own evidence record. It is absent from `docs/status/handover.md` (the documented resume point), `docs/vv/verification-results.md`, `docs/planning/sprint-plan.md`, and the `docs/reviews/README.md` index (this report and FRICK-REVIEW-0437 are not indexed). The co-resident NC-EXT-17 increment *did* add a handover addendum, so the omission is inconsistent within the same batch. | `git diff` scan: only the 4 reviewed files mention NC-EXT-18 | Add the increment's record (hashes, gates, N18-FAIL status, open dependent NCs) to handover/verification-results/sprint-plan and index this review. Documentation-only; deferred under N21. |
| **F04** | **Minor** | `docs/vv/test-spec/ts-5-core-services.md` (N22/N10) | TS-5 has **no test-spec row for `SRS-REQ-009`** at all. The S6 dump cases — including the newly added nominal/boundary/adversarial classes and their explicit expected results — are therefore absent from the controlled test catalogue, even though N22 requires exactly such a catalogue with explicit expected results. | `grep SRS-REQ-009 ts-5-core-services.md` → no match; only `SSS-REQ-009` (table service) rows exist | Add `N22-TS5-nn` row(s) for `SRS-REQ-009` covering: valid dumps (0/4/63), half-open boundary pair, `offset == size`, adversarial offsets, unknown-region precedence, and the malformed classes, each with the observed expected status and wire outcome. Pre-existing gap; deferred. |
| **F05** | **Minor** | `docs/interfaces/ICD.md` §4 S6 row | Two precision gaps: (a) the row says "*these two* causes" are indistinguishable on the wire, implying `FF_BAD_SIZE` is distinguishable, but my probe shows all three refusal causes emit an identical `S1(1,8)` with `FF_PUS_FAIL_EXECUTION`; (b) the precedence of the malformed-request checks over region resolution is undocumented (an unknown region combined with `length == 0` or `length > FF_PUS_MAX_TM_FRAME` yields `FF_BAD_SIZE`, not `FF_NOT_FOUND`). Also, the row quotes the internal macro `FF_PUS_MAX_TM_FRAME` rather than a numeric interface value. | probe rows 1–7; `pus.c`/`svc6_dump` ordering | Reword to "all refusal causes are indistinguishable on the wire", state the validation order explicitly, and give the numeric maximum (1024) as the interface value. No behaviour change; deferred. |
| **F06** | **Comment** | `ADR-0020` header | The header `Date` was changed 2026-10-16 → 2026-10-24, overwriting the original decision date; only the history table preserves it. ADR practice normally keeps the decision date and adds a separate revision date. | `git diff` ADR header hunk | Prefer `Date (decision)` + `Revised` fields, or leave the header date and rely on the history table. |
| **F07** | **Comment** | `tests/unit/test_pus.c:277, 323` | `FF_TEST_ASSERT(!stream_has(6u, 6u))` is a purely negative assertion with no positive control in the same block; it would pass vacuously if nothing had been emitted. At 231/291 the sibling `stream_has(1u, 8u)` assertion supplies that control; 277 and 323 have none. | test source inspection; probe shows `S1(1,8)` is always emitted when the completion ack is set | Add the matching `stream_has(1u, 8u)` assertion (or assert acceptance `S1(1,1)`) so each negative oracle is anchored. |
| **F08** | **Comment** | `src/pus/pus_services.c:501` | `(ptrdiff_t)offset` is exact on RV64 for every reachable offset. On an ILP32 target the conversion would be implementation-defined for `offset > PTRDIFF_MAX`, which is unreachable only because a `> 2 GiB` single object cannot exist there. The reasoning is sound but rests on an unstated platform assumption. | §3.2 `_Static_assert` runs on LP64 and ILP32 | Optional one-line note in the ADR/comment ("`offset` is bounded by an object smaller than `PTRDIFF_MAX` on every supported ABI"). No code change warranted. |
| **F09** | **Comment** | Norm N1 / `verification/lean` | `SRS-REQ-009` carries `lean: []` with a note (a pre-existing, trace-accepted baseline convention), so the new subtractive bound is established by host tests only. The property `offset + length <= size ⟺ length <= size - offset` over `u32` with `offset <= size` is exactly the kind of wrap-freedom claim a theorem would discharge permanently. | `docs/requirements/traceability.yaml:110-117`; `make trace` PASS | Opportunity, not a non-conformity: consider a small `Frick.Pus` theorem before this path gains a caller or a target claim. Deferred. |
| **F10** | **Comment** | `tests/unit/test_pus.c:64-71` (`stream_has`) | The helper indexes `g_rx[pos + 7]`/`[pos + 8]` under the loop condition `pos < g_rx_count`, so at the tail it can read up to 7 bytes past the valid data (inside the 1024-byte static buffer in practice, but outside the buffer if `g_rx_count` reaches 1024). Pre-existing, not introduced here, and not exercised by the current cases. | source inspection | Guard with `(pos + 9u) <= g_rx_count`, as `stream_count()` already does. |

No **Major** finding was identified. No reproduced functional defect and no test failure remains in the reviewed scope.

---

## 6. What this evidence does and does not support

**Supported claims**

- The defect was real, is reproduced independently, and the fix separates the two refusal causes in software.
- The fix is minimal: the only bytes changed in `pus_services.c` are inside `svc6_dump()`, and the pre-fix state is recoverable byte-exactly (`49bc86db…6cc3cf`).
- The bound check cannot wrap, is ABI-identical on ILP32/LP64 by type, and is correct at `offset == size`, at the last byte, and for adversarial 32-bit offsets.
- The on-wire behaviour is unchanged, including the "no response" case with the completion flag clear.
- No authority-blocked interface decision was taken, and no protected NC was modified.

**Not supported (and not claimed)**

- No target/RV32/RV64 execution, timing, WCET or instrumentation evidence for this path; all evidence is **host-native**.
- No coverage measurement (statement/branch/MC-DC) for `svc6_dump()`; structural coverage was not assessed.
- No fault-injection, soak or robustness campaign on the S6 path.
- No N18 Renode evidence, no RTEMS cross-build evidence, no sanitizer evidence.
- The payload *content* bound to offset is not tested (F02); therefore "the correct octets were returned" is not established beyond length.
- NC-EXT-18 is **not** closed: the address-width/region-ID contract and on-wire cause separation remain with the interface authority.
- No qualification, acceptance, compliance, residual-risk, release or flight-readiness conclusion is made or implied.

---

## 7. Uncertainty and limitations of this review

- My reproduction replaced one object file inside a *copy* of the static archive, using the project's exact compile flags. That is faithful for the handler logic but is **not** a from-scratch CMake build of the pre-fix tree; a full rebuild was not performed (the reviewed tree was intentionally held read-only).
- Line-number offsets between the author's pre-fix transcript and the final test file are attributed to a one-line reflow. I did not recover the author's intermediate test-file revision; I verified the *same ten assertion sites* instead.
- The author's `MAKE verify-lean`, `gsw-selfcheck`, RTEMS and indicative Len figures were not re-executed; they are recorded as author claims.
- The `108` figure in §5 could not be explained by any state I could reconstruct (F01).
- The "no authority decision" conclusion is based on the four controlled files plus a tree-wide `git diff` scan for `FF_PUS_FAIL_*` and the protected NC identifiers; other increments in the same uncommitted batch were not audited.

---

## 8. Recommendation

**ACCEPT** this increment — i.e. **APPROVED WITH COMMENTS** in the house taxonomy of `docs/reviews/review-template.md`: no Major findings, no surviving functional or test failure, and the fix is minimal, verified and documentation-consistent.

Conditions attached to that recommendation:

1. F01–F05 are recorded as **open NCs** for later disposition under N21; F02 (payload oracle), F03 (records), F04 (TS-5 row) and F05 (ICD precision) should be dispositioned before this path acquires a caller, a mission profile or a qualification claim.
2. This ACCEPT covers **only the software-visible refusal-cause distinction and the host-verified bound check** for the configuration under review. It is **not** a qualification, acceptance, compliance, release or flight-readiness statement, and it does **not** close NC-EXT-18.
3. NC-EXT-18 remains **OPEN** (authority half: address width and memory-region ID in the controlled interface, and the PUS-C failure-code allocation that would separate the causes on the wire).
4. N18 Renode remains **FAIL** (NC-EXT-198). NC-PAL-01 remains **OPEN**.
5. Commit/push authority and the applicable release gates remain with the designated FSW engineer and the authorized human authority (N7). This reviewer neither commits nor approves.

---

## 9. Follow-up records

| NC (proposed) | Severity | Item | Finding | Disposition | Re-verification evidence | Owner | Status |
|---|---|---|---|---|---|---|---|
| NC-EXT-214 | Minor | Evidence record §5 | Numeric inconsistency `108` vs `134` checks (F01) | Open — records correction only | Re-read of §5 after correction | FSW | OPEN |
| NC-EXT-215 | Minor | `tests/unit/test_pus.c` | Payload-content oracle is vacuous; M_D mutant survives (F02) | Open — deferred (test strength, no functional defect) | Mutant re-run producing ≥1 failure after the pattern fill | FSW + V&V | OPEN |
| NC-EXT-216 | Minor | Records | Increment missing from handover / verification-results / sprint-plan / review index (F03) | Open — deferred (N8/N11/N16) | Presence check of the four records | FSW + documentation | OPEN |
| NC-EXT-217 | Minor | `docs/vv/test-spec/ts-5-core-services.md` | No `SRS-REQ-009` test-spec row; new cases uncatalogued (F04) | Open — deferred (N22/N10) | TS-5 row review by R2 | V&V + PA | OPEN |
| NC-EXT-218 | Minor | `docs/interfaces/ICD.md` §4 | `FF_BAD_SIZE` also wire-indistinguishable; validation precedence undocumented (F05) | Open — deferred; no behaviour change | R3 delta review of the wording | FSW + interface authority (wording only) | OPEN |

(Numbers are proposed; the FSW engineer must allocate from the live register to avoid the collisions this log has previously suffered — see NC-EXT-93.)

Comments F06–F10 need no NC; they are advisory.

## 10. Revision history

| Rev | Date | Author | Change |
| --- | --- | --- | --- |
| 0.1 | 2026-10-24 | Independent R2 V&V reviewer (FRICK-REVIEW-0437) | Initial independent review of the NC-EXT-18 software-only fix: candidate identity and byte-exact minimal-delta proof, independent defect reproduction (134/10 → 134/0), wrap-freedom and ABI analysis, complete refusal-cause map from an independent probe, four mutation probes, ICD 1.6 / ADR-0020 0.2 accuracy and authority-boundary verification, gate re-execution, five Minor + five Comment findings, ACCEPT (approved with comments) with NC-EXT-18 remaining open |
