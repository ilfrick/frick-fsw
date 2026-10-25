# FRICK-REVIEW-0428 — MPFS/MSS watchdog integration specification (embedded safety, **confirmation addendum**)

**Profile:** embedded-safety confirmation, fresh independent context
**Date:** 2026-10-22
**Disposition:** Review evidence only. No edits, no implementation, no register
access, no product build, no emulator execution, no probe re-run, no NC closed
or opened by the reviewer, no gate passed, no hardware parameter, clock,
timeout, instance or reset policy selected, no release or flightworthiness
statement.

> **This is a dated confirmation addendum to FRICK-REVIEW-0426; the original
> 0426 report text is retained unchanged and its conclusions are not restated
> here.** The same holds for FRICK-REVIEW-0424/0425 (rev 1.0) and
> FRICK-REVIEW-0427: they are read-only inputs, cited by identifier only.

## Reviewed body identity (verified by the reviewer, not taken on trust)

| Artefact | Document id | Revision | sha256 as verified | matches expected |
|---|---|---|---|---|
| `docs/architecture/mpfs-mss-watchdog-integration-spec.md` | FRICK-ARCH-0024 | 1.4 | `cbfea6352ec9d20838698b3cf13a5417d532449064bea399260105621f1d8fb4` | yes |
| `docs/architecture/n18-watchdog-integration-proposal.md` | FRICK-ARCH-0023 | 1.4 | `57ead0e1266c7f2274e67ecec9c78983446e0dc47b61f435d3182a2a01966be0` | yes |
| `docs/vv/evidence/2026-10-22-s8-fdir-wd-model-probe.log` | — | — | `641395663479ef72284bfb987cf6bfecaf5baf75fac000969d2297eeb476c9bc` | yes |
| `docs/vv/evidence/2026-10-22-s8-fdir-wd-official-docs-retrieval.log` | — | — | `2436766fc4b7ad2c0d8c3df293f2f6a9388392cb63d14eeb4845c65258f52c52` | yes |

`sha256sum` exit 0; all four matched; no file substituted. The corrections
this addendum triggered produced **rev 1.5**; the findings are itemised in the
specification's §10.

## Confirmation table

| # | findings-corrected (earlier id → resolved?) | independently rechecked in rev 1.4 | new findings surviving in rev 1.4 |
|---|---|---|---|
| **A** — NC-1 oracle | parent acceptance round 2 — the NC-1 oracle required the *negative* case HS-P1 to fail, which a key-omitting binding still satisfies → **resolved**: NC-1 detects on HS-P2, healthy control stated | the discriminating argument itself: the mutation removes the key from the nominal feed write, so the mock's key check refuses the reload and HS-P2's expectation (reload; feed succeeds) is violated; the mutation is **invariant** on HS-P1's observable (a key-omitting write is still a non-key write ⇒ still no reload ⇒ still "feed reported failed"), so HS-P1 cannot detect it; a correct binding passes both | **Comment (advisory):** NC-1's discrimination presupposes that the **host mock implements the key check**; that is asserted in §5.1's host column but not restated as an explicit precondition in the NC-1 row, so the healthy control could be read as unconditional |
| **B** — HS-P5 | 0427 F-11 (stimulus/oracle not separated) and the round-2 finding (caller order vs binding order) → **resolved** | §5.2's preamble defines the observer as "an ordered log of **every register write the binding performed**"; HS-P5 states the **stimulus** (a deliberately wrong *caller-supplied* order) and the **oracle** (the binding's **own** documented order); because the observer records the binding's writes, not the caller's intent, the case distinguishes the two; NC-3 is discriminating against the same oracle | **Minor:** the oracle is **whole-sequence equality** against the observer's full log, while R-7's init sequence also performs the first `REFRESH` — a conforming binding would fail HS-P5; and the stimulus presupposes a caller-supplied order **parameter** the PAL contract does not require. A **relative-order (subsequence)** requirement preserves the discrimination and removes the false-failure risk |
| **C** — probe attribution | 0426 F-0426-01 (probe register labels wrong; `TIME` width claim; "STATUS did not vary") → **resolved** | every row of the §2.3 measurement table re-derived from the retained log: `0x08` `0x00000000` → `0x00000010` after the first write to `0x08` (bit 4 = `LOCKED`); `0x04` constant `0x00000002`; the three model warnings; `TRIGGER` re-write `0x0A` → `0x0000000A` (re-write took effect); `TRIGGER` `0x1000` → `Unhandled bits: [12]`, read-back `0x00000000` (12-bit truncation); `TIME` 0x0c never accessed; `REFRESH` and `MSVP` written but never read back. F-17's measured/not-established split and all nine §5.1 placement rows are probe-accurate | **Comment:** F-17's "not established" list says "the **lock enforcement**" while the preceding sentence records as *measured* that the model does not enforce the lock — the list should read "the **device's** lock enforcement" |
| **D** — official issuer documents | round 2, class 2 (the instance/reset-scope questions rested on a family example driver) → **resolved as to retrieval, citation and the safety-critical claims** | all 14 quoted extracts used in the spec are present **verbatim** in the retrieval log (`grep -F`, 14/14 OK). The four safety-critical claims are stated correctly: only `WDOG0` (E51) can reset the MSS and a U54 watchdog only interrupts the E51 (F-18); the ≈10.47 s power-up default **with the "if VDD = 1.2 V" condition retained** (F-19); the MSS-scope reset with `WDOG_RESET` reason bit 5 (F-22); the debug-halt behaviour (F-20); plus no published watchdog erratum for RTPFS160ZT (F-24). **No parameter, clock, timeout, instance or reset policy is adopted**: the ≈1.6026 MHz / ≈2.6 ms figures appear only in the log's labelled-inference block and in the body only as "a labelled inference, not a value" | **Minor (evidence-chain closure):** two issuer-attributed identifiers — `RESET_SR` and `DEVRST_N` — are **absent from the retained record** (`grep` → no match), while the substantive claims are supported; the "not the whole device" distinction rests on a name the record does not show. **Comment:** the inference's assumptions live only in the log's §5, not in the body |
| **E** — reframed recommendation | round 2, class 2 (the rev 1.3 instance recommendation superseded) and the re-opening of agenda item C → **resolved** | **no hidden preference:** every recommendation cell is either a documentation/structure preference (A, B1, B2), an evidence-based **negative** (C: a U54 watchdog alone delivers no MSS reset, so it does not meet the SRS-REQ-073 protection property — an elimination, not a preference), a preference **conditioned on an open input** (D, E, F), or an explicit refusal to prefer (D(d)). The residuals remain visible and correctly attributed (M-2/M-8, M-7, M-15, M-16, M-13 + RP-1, F-19/HL-8, F-20/HL-9, F-23, F-11's three unproved dependencies, M-11/M-12) | **Minor (currency, in the proposal):** `n18-watchdog-integration-proposal.md:97-98` still names the **wdog1** instance as the Option A implementation target, `:118` still lists "wdog1 on the running hart" as an instance candidate, and `:111` points to **FRICK-ARCH-0024 rev 1.3** — the revision whose instance recommendation rev 1.4 withdraws and whose reset-scope facts rev 1.4 changes. A U54-owned watchdog **cannot reset the MSS** (F-18), so Option A as written is in tension with the controlling specification |
| **F** — metadata / manifest | round 2 metadata row → the §5.4 heading now reads "**five distinct items**" and the table holds exactly five; the evidence index cites the proposal at rev 1.4; the dangling `§7.5` pointer is gone | the §9 code-manifest claim ("re-verified (34/34 `OK`, exit 0)") is **accurate** — re-verified by the reviewer: 34/34 OK, exit 0 | **Moderate:** the documentation-phase identity manifest does **not** verify (8 of 16 FAILED, exit 1), including the reviewed body itself — it was not regenerated after the rev 1.4 text freeze. **Moderate:** §1 of the reviewed body is **textually corrupted** — the N18/Lean bullet is cut mid-sentence by the inserted rev-1.4 paragraph, leaving the tail orphaned. **Minor:** §2.4 precedes §2.3; the rev 1.3 history row points at §11 for an itemisation that is at §10 |

## Statement on rev 1.4

**Rev 1.4 is accurate on the substantive confirmation scope A–E.** The three
parent-acceptance test-plan defects and the 0426 probe-attribution defect are
genuinely resolved, confirmed independently rather than by re-reading the
disposition table: the NC-1 oracle is discriminating and logically complete; the
probe attribution, the measured/not-established split and the §5.1 placement are
probe-accurate and **no model claim exceeds the retained probe**; the retrieved
issuer documents are quoted faithfully and the four safety-critical claims are
stated correctly, with **no clock, timeout, instance or reset policy adopted**;
and the reframed recommendation is honest — the `WDOG1` recommendation is
withdrawn, agenda C carries **no** recommendation and settles only a negative,
the safety-critical residuals remain visible, and the document still says
plainly that it is a review agenda and not an implementation-ready design.

**Genuine unresolved findings** (the reviewer's own; none alters the A–E
confirmations): 1 (Moderate) the identity manifest does not verify; 2 (Moderate)
§1 textual corruption; 3 (Minor) HS-P5's whole-sequence equality; 4 (Minor)
`RESET_SR`/`DEVRST_N` not carried by the retained record; 5 (Minor) the
proposal's currency; 6 (Minor) §2.4/§2.3 order and the stale §11 pointer;
7 (Comment) F-17's lock wording, NC-1's mock precondition, the inference's
assumptions. Items 1–6 were corrected in spec rev 1.5 / proposal rev 1.5 and
the retrieval record; item 7 is recorded in the specification's §10.

**No hardware parameter, clock, timeout, instance or reset policy is chosen by
this addendum. No NC is closed, no gate is approved, no implementation is
authorized, and no release or flightworthiness statement is made.** The owner
agenda (A, B1/B2, C, D, E, F) remains OPEN, and M-1 (the "PolarFire SoC Device
Register Map" and the RT "MS Description") remains the outstanding controlled
input.

**Limits:** document-and-evidence review only — no build, no emulator, no probe
re-run, no target/HIL, no re-derivation of the issuer PDFs (not retained, N13
boundary), no reading of the non-retrievable "Device Register Map"; the
quotation sampling was 14/14 of the extracts the specification uses, not an
exhaustive clause-by-clause audit of the six documents. Confidence: **High**
for A–E, **Medium** for the manifest and §1 items (reproducible, but the
disposition belongs to the authoring agent).

**Exact commands and exits:** `sha256sum` over the four artefacts → exit 0, 4/4
match; `sha256sum -c …-post-review-source.sha256` → exit 0, 34/34 `OK`;
`sha256sum -c …-wd-spec-docs.sha256` → exit 1, 8 `OK` / 8 `FAILED`;
`stat -c '%y %n'` on the manifest and the body → the manifest predates the
rev 1.4 text freeze; `grep -n "RESET_SR"` / `grep -n "DEVRST"` on the retrieval
log → no match, exit 1; 14 × `grep -qF` of the specification's quoted extracts
against the retrieval log → 14/14 OK; `grep -n "^### 2\."` → §2.4 before §2.3.
