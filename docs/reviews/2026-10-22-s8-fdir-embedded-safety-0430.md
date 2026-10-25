# FRICK-REVIEW-0430 — MPFS/MSS watchdog integration specification (**final acceptance checkpoint**, embedded safety, read-only)

**Profile:** embedded-safety acceptance checkpoint, fresh independent context
**Date:** 2026-10-22
**Disposition:** **Read-only.** No file created, edited or deleted; no build, no
emulator, no probe, no target/HIL; no NC closed or opened by the reviewer; no
gate approved; no hardware parameter, clock, timeout, instance or reset policy
selected; no release or flightworthiness statement.

> **This addendum covers the rev 1.5 bodies only.** It does **not** extend,
> re-open, restate or supersede FRICK-REVIEW-0428/0429, which confirmed
> **rev 1.4** only. It is not a review of rev 1.5 in their place.

## A. Frozen body identity (verified before review)

`sha256sum` over the four paths → **exit 0**; `sha256sum -c
docs/vv/evidence/2026-10-22-s8-fdir-wd-spec-docs.sha256` → **19/19 OK, exit 0**.

| Body | Expected = observed SHA-256 | Result |
|---|---|---|
| `docs/architecture/mpfs-mss-watchdog-integration-spec.md` (FRICK-ARCH-0024 rev 1.5) | `9b09605cc706b2f60fd81d27503d27750f8d9dec21cc9f9972e96d4701673d40` | **match** |
| `docs/architecture/n18-watchdog-integration-proposal.md` (FRICK-ARCH-0023 rev 1.5) | `749ecafc046a25c286c48a6948f6d5f2e58e3333e4b1a0e8551514cc7b9f35e5` | **match** |
| `docs/vv/evidence/2026-10-22-s8-fdir-wd-official-docs-retrieval.log` | `54a449648e5465dd4d4df5b0cf8c6ad1d68e9ee1d6c133089cfae7a4e98e36ed` | **match** |
| `docs/vv/evidence/2026-10-22-s8-fdir-wd-model-probe.log` | `641395663479ef72284bfb987cf6bfecaf5baf75fac000969d2297eeb476c9bc` | **match** |

**Disclosure:** the rev 1.4 body is **not retained** in the frozen set (the
documents are untracked; no rev 1.4 copy or digest exists in the evidence set),
so the 1.4→1.5 delta was assessed against the rev 1.5 text and its own §10/§11
itemisation, not byte-for-byte against a rev 1.4 body.

## B. Delta confirmations (1.4 → 1.5)

| # | Item | Verdict | Evidence |
|---|---|---|---|
| 1 | **NC-1 oracle** detects on the positive case **HS-P2**; HS-P1 remains the non-discriminating negative case; **healthy control** stated | **CONFIRMED** (logic complete) | spec:678 (NC-1 → "**HS-P2 must fail**", nominal feed path); spec:654 (HS-P1 = wrong key ⇒ no reload, feed reported failed); spec:655 (HS-P2 = correct key ⇒ reload); the healthy control is stated in the §10 itemisation and history (spec:829, spec:860) — **not** inside the §5.2 table (see NEW-1, NEW-2) |
| 2 | **§5.2 host preamble** declares the key-checked reload semantics, the reload observable and the feed-outcome observable as a **precondition of the host suite**; **HS-P1** pins a non-zero wrong key ≠ `0xDEADC0DE`; **HS-P2** exercises a **periodic** feed | **CONFIRMED** | spec:644–650; spec:654; spec:655 |
| 3 | **HS-P5** relative-order oracle + stated delivery mechanism; **NC-3** discriminating | **CONFIRMED in substance** | spec:658 (TRIGGER → MSVP → TIME → CONTROL, other writes permitted; "The delivery mechanism is a test double of the binding's configuration entry point…", with the degenerate-case note); NC-3 at spec:680. Any wrong order breaks the subsequence, while a conforming binding (extra R-7 writes) is not falsely failed |
| 4 | **Proposal rev 1.5**: `wdog1` selection **withdrawn**; the issuer fact quoted next to the emulator instance table; cross-reference at rev 1.5 | **CONFIRMED** | proposal:107–113; proposal:76–82 (quote matches log:70–71); proposal:106, proposal:125 |
| 5 | **Official-fact scope** (§2.4, F-18..F-24) and **no adoption** of a clock/timeout/instance/reset policy; the ≈1.6 MHz / ≈2.6 ms figures only as a labelled inference with its assumptions | **CONFIRMED** | F-18 spec:383–394; F-19 spec:396–402; F-20 spec:404–410; F-21/26-bit field spec:412–423, spec:251–256; F-22 spec:425–434; F-24 spec:446–455; "no adoption" spec:51–54, spec:33–38, spec:755–757; agenda A–F all **OPEN** (spec:731–737); R-1 gives options without choosing (spec:459–483); the inference with both assumptions and the "not confirmed" items at log:124–132, spec:280–283, spec:721, spec:723. *Qualification:* the only constant carried in a recommendation is `prescaler = 256` (spec:489), explicitly conditional on M-2 and marked unconfirmed (spec:268–283) |
| 6 | **§1 status bullet** intact; **review-coverage attribution** accurate (rev 1.0 / rev 1.2 reviews vs the rev 1.4 addenda) | **CONFIRMED** | bullet intact as one sentence spec:39–40; header spec:11–12; §10 spec:793–801; 0428/0429 are never presented as covering rev 1.5 (see NEW-6 for two labelling inaccuracies) |
| 7 | **Quoted `RESET_SR` and `DEVRST_N`** present in the retrieval log and matching the quotation as used | **CONFIRMED** | log:79–80 (`RESET_SR`), log:81–86 (`DEVRST_N`), log:87–90 (Table 4-2, `WDOG_RESET` bit 5); used at spec:425–434 with matching bit numbers (1/3/4) and quote text |
| 8 | The three **§5.1 host cells** (lock enforcement, forbidden window, gating) have no host case and are recorded as the open **NC-EXT-205** — marked open, not claimed as covered | **CONFIRMED** | spec:620, spec:621, spec:624 all read "mock — **no host case yet** (open, NC-EXT-205)"; NC-EXT-205 registered **OPEN — partly corrected in spec rev 1.5** (docs/planning/sprint-plan.md:324) |

## C. Arithmetic observation (editorial only; not a design issue)

The retrieval log's labelled inference states the design-time default load
assumption as "16 777 200 (0xFFF3D8)" (log:126). The correspondence is
**incorrect**: 16 777 200 = **0xFFFFF0**, whereas `0xFFF3D8` = 16 774 104. The
specification body itself uses the correct form ("`0xFFFFF0 − 0x3e8`",
spec:365) and F-3 fixes the macro value at 16 777 200 (spec:244–250).
**Effect on conclusions: none** — at 16 777 200 the implied counter clock is
1.6026 MHz and at 16 774 104 it is 1.6021 MHz: the same ≈1.6 MHz figure, the
same ≈16× relation to the header's illustrative 25 MHz, and the same ≈2.6 ms
reaction-window ceiling (4095 ticks). The inference is **not used as a
parameter** (log:124–132; spec:723).

## D. NEW observations — for the project owner to record as OPEN/deferred

Structural and document-integrity only; no design is reopened and no wording,
refactoring or test plan is proposed.

| ID | Observation |
|---|---|
| **NEW-1** (Minor, structure) | The HS-P5 case has **no table row and no ID cell** in §5.2 — the text sits in a bare paragraph at spec:658 (no leading `|`, no "HS-P5" identifier); the string "HS-P5" does not occur in §5.2 (only at spec:680/826/830/839). The rendered host table reads HS-P1…HS-P4, then HS-P6…HS-P8, with the stimulus/oracle as an inter-table paragraph. The oracle **content** is correct (item 3) |
| **NEW-2** (Minor, structure) | The NC-1 row at spec:678 carries **3 of the table's 4 columns** (NC-2/NC-3/NC-4 carry 4, spec:679–681): the "Why this case is the discriminating one" cell is absent, so the in-table rationale and the healthy control survive only in the §10 itemisation (spec:829) and the history (spec:860) |
| **NEW-3** (Minor, structure) | The §5.4 RP-1 row (spec:705) carries **6 cells against a 4-column header**: the Quantity and Note cells are duplicated verbatim after the "Produced by" cell. The leg↔quantity mapping itself is present and correct |
| **NEW-4** (Minor, metadata) | §9 still cites the companion proposal as "FRICK-ARCH-0023 **rev 1.4**" (spec:779) although the proposal is rev 1.5 — the same class of currency defect that rev 1.4 corrected for rev 1.3→rev 1.4; the spec's own header cites rev 1.5 (spec:13) |
| **NEW-5** (Minor, metadata) | A second §9 row for the same BSP inventory artefact with an **empty Content** cell (spec:769) duplicates the populated row (spec:767), although the rev 1.5 itemisation records the duplicate row as corrected (spec:849) |
| **NEW-6** (Minor, labelling) | The §1 paragraph is headed "Change introduced by this revision (**rev 1.4**)" inside a rev 1.5 document (spec:41 vs spec:7); and the §10 correction record describes the header as separating "Reviews of rev 1.2", whereas the header row reads "Reviews of rev 1.0" (spec:11) — the rev 1.2 reviews (0426/0427) appear only in §9 (spec:777) and §10 (spec:795). The substantive attribution requirement of item 6 is met |

## E. Scope and authority

This addendum covers **only** the rev 1.5 bodies listed in §A (plus the
arithmetic observation in §C) and the 1.4→1.5 delta. It does **not** extend,
restate, re-open or supersede FRICK-REVIEW-0428/0429, which confirmed **rev 1.4
only**. **Nothing in this addendum approves the design, a gate, a parameter, a
clock, a timeout, an instance or a reset policy, and it closes no NC and selects
no hardware value.** Implementation remains blocked on the **controlled device
documentation** (G-A / M-1, including the "PolarFire SoC Device Register Map"
and the RT "MS Description", the lock rule M-11 and the timeout unit M-12), the
**board clock / boot configuration** (G-B / M-2, M-8, M-15, including the
design-time default load and the boot lock state), the **mission reset
assignment** (G-C / M-3, M-7, M-14 — instance, hart and board-level reset scope;
agenda item C) and the **HIL thresholds and acceptance criteria** (G-E / M-10,
M-13; agenda items B2, D, E, F). The N18 gate remains FAIL/pending per the
retained evidence; no emulator, build, probe or hardware run was performed for
this checkpoint.

**Confidence:** High for items 1–8, the hashes and the arithmetic (direct
reading of the configuration-matched frozen bytes); Medium for the *delta
attribution* of NEW-1…NEW-6 (no retained rev 1.4 body, so new-vs-pre-existing
cannot be established from the frozen set). **Detection limits:** text-level and
arithmetic inspection of the frozen bytes only — no build, emulator, probe, HIL
or tool run; Markdown table well-formedness assessed by cell-count parsing,
which cannot detect wording defects.
