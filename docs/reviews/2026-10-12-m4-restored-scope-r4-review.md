# Review Report — M4 restored scope + DDJF acronym (documentation increment)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0138                      |
| Reviewer session                  | independent agent, norm N9 (fresh context) |
| Reviewer profile                  | R4 — Senior SW documentation engineer (space/aeronautics) |
| Reference revision                | `ce0bff3` + uncommitted working tree   |
| Items under review                | `docs/engineering/software-description.md` (0.3), `docs/glossary.md` (0.2), `docs/engineering/user-integration-manual.md` (0.6), `docs/engineering/driver-development.md` (0.4), `docs/architecture/network-stack.md` (0.4), `docs/requirements/SSS.md` (0.14), `docs/requirements/SRS.md` (1.18), `docs/requirements/traceability.md` (0.30), `docs/requirements/traceability.yaml`, `docs/roadmap.md` (0.18), `docs/development-plan.md` (0.20), `docs/planning/sprint-plan.md` (0.6), `docs/quality/class-a-gaps.md` (0.8), `docs/status/handover.md` (1.10), `docs/status/loop-audit.md` (0.14), `docs/status/status.json`, `CHANGELOG.md`, `README.md`, `AGENTS.md` |
| Date                              | 2026-10-12                             |
| Scope                             | Documentation completeness, correctness, internal consistency and currency of the increment that restored the previously "absorbed" M4 scope as explicit deliverables and registered the **DDJF** acronym. Verified: header/`## History` revision agreement (N16), count consistency (69 srs / 29 sss / 14 gap / 51 theorems), absence of stale "absorbed" scope wording outside history rows, DDJF acronym registration, README/AGENTS/CHANGELOG registration, cross-reference and path resolution, TCP/IP/TSN/PUS-C status accuracy and absence of over-claim, and roadmap/development-plan/sprint-plan/requirements coherence. Assessed against ECSS-E-ST-40C (information/data-item and documentation obligations), ECSS-Q-ST-80C (assurance records) and ECSS-E-ST-70-41C / CCSDS 133.0-B-2, under project norms N1–N20. |
| Independence declaration          | The reviewer had no part in authoring or executing the items under review; it ran in a fresh context with only the material under review and the reference documents. It is read-only for the repository except for this single report. |
| Norms applied                     | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

Reviewed the **documentation** of a requirements/planning increment that (a) turned
previously "absorbed" prose scopes into explicit, traceable deliverables and (b)
registered the **DDJF** acronym. Specifically:

- the DDJF registration: retitled `docs/engineering/software-description.md`
  (FRICK-ENG-0007, 0.3) as the *Design Description and Justification File*, the
  acronym definition, the new justification dimension (§3.10), the §3.1 "PUS-C
  transport channels" paragraph, the `docs/glossary.md` (0.2) `DDJF` entry, the
  README index entry, the `AGENTS.md` name, and the `CHANGELOG.md` bullet;
- the restored M4 scope: `SSS.md` (0.14) SSS-REQ-028/029 + the SSS-REQ-015
  reconciliation; `SRS.md` (1.18) SRS-REQ-059..068; `traceability.{yaml,md}`
  (0.30); `roadmap.md` (0.18) / `development-plan.md` (0.20) explicit deliverables
  with OBJ-25..OBJ-30; `sprint-plan.md` (0.6) S4b/S4c/S4d and the S13-S14 extension;
  `class-a-gaps.md` (0.8) RS-485 DE/RE residual;
- the "absorbed" wording removals in `network-stack.md` (0.4),
  `user-integration-manual.md` (0.6) and `driver-development.md` (0.4);
- the status/registration updates: `handover.md` (1.10), `loop-audit.md` (0.14),
  `status.json`, `CHANGELOG.md`, `README.md` (metrics panel + DDJF index line).

**Excluded (not assessed here):** code, build files, tests and V&V evidence (none
changed by this increment); the technical *correctness* of the new requirements'
engineering content (profile R3, `FRICK-REVIEW-0137`); the open ECSS NC
`PA-67-01` (SDD design matrices). The increment is documentation/requirements-only,
so no Lean/V&V re-run was required by it (N1/N15 apply to the touched scope).

## 2. References

- Project norms: `docs/rules.md` (N1–N20); reviewer contract:
  `docs/engineering/independent-review.md` (norm N9/N17).
- Traceability manifest: `docs/requirements/traceability.yaml`; matrices:
  `docs/requirements/traceability.md`; gate: `tools/scripts/trace.py`.
- Status panel generator: `tools/scripts/metrics.py` (T13).
- ECSS-E-ST-40C (documentation/data-item obligations, DRDs); ECSS-Q-ST-80C
  (assurance and records); ECSS-E-ST-70-41C (PUS-C); CCSDS 133.0-B-2.
- `docs/reviews/review-template.md`; `docs/reviews/README.md`;
  `docs/reviews/2026-10-12-m4-restored-scope-r3-review.md` (FRICK-REVIEW-0137).

**Gate re-run (evidence).**

- `python3 tools/scripts/metrics.py --check` → `metrics panel: current`, exit 0
  (the README metrics panel is current against the manifest/status.json).
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` →
  `traceability: PASS (112 requirements, 69 srs, 29 sss, 14 gaps, 51 lean
  theorems checked)`, exit 0.

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R4-01 | Minor | `docs/requirements/traceability.md` §intro | **The parent list for the restored scope is wrong.** The intro states the restored scope is "(SRS-REQ-059..068 with SSS parents **SSS-REQ-015**/017/020/028/029)". No restored-scope requirement is parented to SSS-REQ-015: SRS-REQ-065 (PUSopen) is parented to **SSS-REQ-029**, and SSS-REQ-015 has only two children (SRS-REQ-018, SRS-REQ-050). The correct set is SSS-REQ-017/020/028/029. | `traceability.md:24`; `traceability.yaml` `parent_sss` for SRS-REQ-059..068 (059/060/061→028, 062/063/064/065→029, 066/068→020, 067→017); SSS-REQ-015 children at `traceability.yaml:188,473`; gate output `SSS-REQ-015 <- 2 srs children`. | Drop `SSS-REQ-015` from the intro list (should read `SSS-REQ-017/020/028/029`). Prefer deriving §1 and the intro list from the manifest to prevent recurrence (N4/N8). |
| R4-02 | Minor | `docs/requirements/SRS.md` §6 SRS-REQ-018 (+ manifest justification) | **Stale "open backend" default left unreconciled** after SSS-REQ-015 was corrected in this increment. SRS-REQ-018 still reads "the reference build uses the **open backend** for PUS", and the manifest justification for SRS-REQ-018 says "abstraction layer with **open default** and optional PUSopen adapter". This contradicts the reconciled SSS-REQ-015 ("the N7/ESA ASN.1 default backend") and SRS-REQ-050/ADR-0016 (N7/ESA ASN.1 is the default; the open backend is the fallback). This is the residual of the R3-02 class. | `SRS.md` §6 (SRS-REQ-018 text); `traceability.yaml` SRS-REQ-018 `justification`; `SSS.md` SSS-REQ-015; `SRS.md` SRS-REQ-050; `docs/adr/ADR-0016-pus-c-asn1-default.md`. | Correct SRS-REQ-018 (and its manifest justification) to state the N7/ESA ASN.1 default with the open backend as the fallback, consistent with SSS-REQ-015 / SRS-REQ-050 / ADR-0016 (N8/N4). |
| R4-03 | Minor | `docs/engineering/driver-development.md` §1 | **Inaccurate default-link mapping.** §1 states the default is "(`FRICK_PLATFORM_MPFS` → UART, otherwise → loopback)", but `link.c` selects UART for `FRICK_PLATFORM_MPFS` **or** `FRICK_PLATFORM_QEMU`. The statement therefore wrongly implies the generic QEMU RTEMS target defaults to loopback, contradicting the user manual §7.1 ("UART on the RTEMS platforms (MPFS and generic QEMU)") and the DDJF §3.1. | `driver-development.md:29-31`; `src/transport/link/link.c:15` (`#if defined(FRICK_PLATFORM_MPFS) || defined(FRICK_PLATFORM_QEMU)`); `user-integration-manual.md` §7.1; `software-description.md` §3.1. | Add `FRICK_PLATFORM_QEMU → UART` (or "UART on the RTEMS platforms") to the mapping. Pre-existing, but the file was revised in this increment and the link-backend description is central to its scope (N8). |
| R4-04 | Comment | `README.md` "Highlights" (Networking) | The bullet lists "native **TSN** on the GEM (**802.1Q/Qbv/802.1AS**)" without the "time-read seam" qualifier used by the DDJF §3.7, `network-stack.md` §6 and the user manual §9.1. A reader may over-read this as the full 802.1AS gPTP stack, which is M8/planned. | `README.md:118-120`; `software-description.md` §3.7; `network-stack.md` §6; `user-integration-manual.md` §9.1. | Qualify 802.1AS (e.g. "802.1AS time-read seam, gPTP = M8") to keep the no-over-claim discipline used elsewhere. |
| R4-05 | Comment | `docs/status/status.json` / `README.md` sprints metric | The sprint denominator (26) is a manual field and the sprint plan (0.6) now adds **S4b/S4c/S4d** rows. `metrics.py --check` passes (it consumes `status.json`), so no gate fails, but the "3/26" panel figure may no longer match the plan's sprint set. | `status.json` `sprints.total=26`; `README.md:51`; `docs/planning/sprint-plan.md` §2 (S4b/S4c/S4d). | Confirm whether S4b/S4c/S4d are sub-sprints of S4 (total unchanged) or new sprints (total 29) and align `status.json`/panel. |
| R4-06 | Comment | `docs/engineering/user-integration-manual.md` History 0.5 | The rev-0.5 reason says §7.1 "cross-reference[s] the DDJF (`software-description.md` §3.1) rather than duplicating", but §7.1 contains no DDJF reference; the cross-reference runs the other way (DDJF §3.1 → manual §7.1). | `user-integration-manual.md` History 0.5; §7.1 (no DDJF reference); `software-description.md` §3.1 ("User-facing details: … §7.1"). | Align the history wording with the actual (one-directional) cross-reference, or add the DDJF reference in §7.1 (N16). |
| R4-07 | Comment | `docs/reviews/2026-10-12-m4-restored-scope-r3-review.md` (FRICK-REVIEW-0137) | The R3 review record (untracked, to be committed per N11) describes the reviewed state as "110 requirements (67 srs)" and "SRS-REQ-059 … 066", while the tree it would be committed with is 112 (69 srs) / SRS-REQ-059..068. Its follow-up table (R3-01..R3-12) is still "open". | `2026-10-12-m4-restored-scope-r3-review.md` §2/§7 (67 srs; 059..066); current `traceability.yaml` (69 srs; 059..068). | Close the R3 NCs (N12) and perform the R3 delta re-review of the post-fix state before commit, so the committed review record matches the reviewed revision (N11/N12). |
| R4-08 | Comment | `docs/architecture/network-stack.md` header | The header "Requirements" field lists `SRS-REQ-027, SRS-REQ-039, SRS-REQ-040`, but §6 (revised this increment) now discusses the M4-residual link backends SRS-REQ-059/060/061. | `network-stack.md:10`; §6. | Add SRS-REQ-059/060/061 to the header "Requirements" field for completeness (N8). |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

**Positive confirmations (verified, no finding):**

- **Header ↔ History agreement (N16).** Every touched document's header
  `Revision`/`Date` equals its last `## History` row: `software-description.md`
  0.3/2026-10-12; `glossary.md` 0.2/2026-10-12; `SRS.md` 1.18/2026-10-12;
  `SSS.md` 0.14/2026-10-12; `traceability.md` 0.30/2026-10-12; `roadmap.md`
  0.18/2026-10-12; `development-plan.md` 0.20/2026-10-12; `sprint-plan.md`
  0.6/2026-10-12; `class-a-gaps.md` 0.8/2026-10-12; `network-stack.md`
  0.4/2026-10-12; `user-integration-manual.md` 0.6/2026-10-12;
  `driver-development.md` 0.4/2026-10-12; `handover.md` 1.10/2026-10-12;
  `loop-audit.md` 0.14/2026-10-12.
- **Counts consistent (69 srs / 29 sss / 14 gap / 51 theorems).** Confirmed in
  `traceability.md` intro, `traceability.yaml` (112 = 69+29+14), `README.md`
  metrics panel (112), `handover.md` §4, `loop-audit.md` (N3 row + rev-0.14 row),
  `status.json` note, `CHANGELOG.md`. `metrics.py --check` current; `trace.py`
  PASS. No stale `59 srs`/`27 sss`/`100 (srs…` text outside historical
  `verification-results.md`/History rows.
- **"Absorbed" scope wording removed.** No stale "absorbed"/"M4-absorbed" scope
  wording remains outside `## History` rows (and the CHANGELOG bullet that
  *describes* the removal); `network-stack.md` §6, `user-integration-manual.md`
  §8/§10, `driver-development.md` §2 and `roadmap.md` M4 now reference the
  explicit M4-residual scope (SRS-REQ-059/060/061).
- **DDJF acronym registered and consistent.** Defined in the title, §1 ("Name"),
  `glossary.md`, README index and `AGENTS.md`; the former "no repository acronym
  defined" note survives only as historical rows (`software-description.md`
  History 0.1/0.3) and the CHANGELOG change description. §3.10 justification
  dimension and the ADR cross-references (ADR-0001…0014/0016; ADR-0015 unused)
  are correct (`docs/adr/` contains no ADR-0015 file).
- **Registration present.** README index names the DDJF; `AGENTS.md` names it;
  `CHANGELOG.md` [Unreleased] carries both the restored-scope and DDJF bullets.
- **Cross-references/paths resolve.** Spot-checked paths and symbols:
  `src/transport/link/link.{h,c}` + `FF_LinkLoopbackBackend`/`FF_LinkUartBackend`/
  `FF_LinkSelect`/`FF_LinkInit`/`FF_LinkGet`; `FF_TsnGem_t`/`FF_TsnGemSetup`;
  `FF_CanOpenStack_t` (`hal_can.h`), `FF_Spi_t`/`FF_I2c_t` (`hal_spi.h`),
  `set_rs485`/`de_assert_us`; `docs/interfaces/ICD.md`, `docs/verification/`,
  `docs/design/software-design.md`, `docs/engineering/service-development.md`,
  `docs/engineering/coding-style.md`, `docs/adr/ADR-0016-…` all exist.
- **TCP/IP / TSN / PUS-C status accurate and free of over-claim.** The DDJF §3.1,
  `network-stack.md` §6, `user-integration-manual.md` §7.1/§8/§9/§10 and
  `driver-development.md` §2 consistently state that only loopback and UART link
  backends ship, that PUS-C over IP/TSN is planned (SRS-REQ-059/060/061), that
  802.1Qav returns `FF_NOT_IMPLEMENTED`, and that 802.1AS is a time-read seam
  (gPTP = M8).
- **Restored-scope coherence.** `roadmap.md` (M4 OBJ-25..28/OBJ-30; M8 OBJ-29),
  `development-plan.md` (§2 objectives, §4 milestones, §6 status) and
  `sprint-plan.md` (S4b/S4c/S4d; S13-S14) agree on the M4-residual/M8 placement;
  the file-transfer set remains explicitly M15; `class-a-gaps.md` records the
  RS-485 DE/RE on-target residual in GAP-A5.
- **SSS-REQ-029 decomposition reconciled** (R3-01 closed in-tree): SSS-REQ-029 now
  parents all four named components (SRS-REQ-062/063/064/065); SSS-REQ-015's
  default-backend wording corrected; traceability §1 rows for SSS-REQ-015 and
  SSS-REQ-020 include SRS-REQ-050/066/068.

## 4. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] APPROVED WITH COMMENTS — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**Conclusion: APPROVED WITH COMMENTS.** No Major finding. Three Minor findings
(R4-01 traceability intro parent list; R4-02 SRS-REQ-018 stale "open backend"
default; R4-03 driver-development default-link mapping) and five Comments
(R4-04…R4-08) are raised; the Minors are local documentation-consistency defects
correctable without rework of the delivered baseline. The required gates are green
(`metrics.py --check` current; `trace.py` PASS 112/69/29/14/51), the DDJF acronym
is registered consistently, no stale "absorbed" scope wording remains, and the
TCP/IP/TSN/PUS-C status statements are accurate and free of over-claim. No
certification, qualification or approval is claimed by this review.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R4 (N9/N17)   | Senior SW documentation engineer (independent agent) | 2026-10-12 | APPROVED WITH COMMENTS |

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R4-01 | Minor | `traceability.md` intro | Parent list wrongly includes SSS-REQ-015 | To be dispositioned | — | frick-fsw | open | — |
| R4-02 | Minor | `SRS.md` SRS-REQ-018 + manifest justification | Stale "open backend (default)" vs N7/ESA default | To be dispositioned | — | frick-fsw | open | — |
| R4-03 | Minor | `driver-development.md` §1 | Default-link mapping omits `FRICK_PLATFORM_QEMU` → UART | To be dispositioned | — | frick-fsw | open | — |
| R4-04 | Comment | `README.md` Highlights | 802.1AS listed without the "time-read seam" qualifier | Recommended | — | frick-fsw | open | — |
| R4-05 | Comment | `status.json` / README sprints | Sprint denominator (26) vs added S4b/S4c/S4d rows | Confirm/adjust | — | frick-fsw | open | — |
| R4-06 | Comment | `user-integration-manual.md` History 0.5 | Claims a DDJF cross-reference not present in §7.1 | Align wording | — | frick-fsw | open | — |
| R4-07 | Comment | `FRICK-REVIEW-0137` (R3) | Review record state (67 srs / 059..066) vs committed tree (69 srs / 059..068); NCs open | Close R3 NCs + delta re-review (N12) | — | frick-fsw | open | — |
| R4-08 | Comment | `network-stack.md` header | Header "Requirements" field omits SRS-REQ-059/060/061 | Add for completeness | — | frick-fsw | open | — |

## 7. Facts / Assumptions / Inferences / Recommendations / Confidence / Open gaps

**Facts (verified with evidence):**

- `metrics.py --check` → `metrics panel: current` (exit 0); `trace.py` → PASS,
  112 requirements (69 srs / 29 sss / 14 gaps), 51 Lean theorems (N4).
- Every touched document's header revision/date equals its last History row (N16).
- The current manifest parents of SRS-REQ-059..068 are SSS-REQ-028/029/020/017;
  SSS-REQ-015 has exactly two children (SRS-REQ-018, SRS-REQ-050).
- `SRS.md` §6 SRS-REQ-018 and its manifest justification still say the "open
  backend" is the reference default; SSS-REQ-015 (reconciled) and SRS-REQ-050 /
  ADR-0016 state the N7/ESA ASN.1 backend is the default.
- `src/transport/link/link.c:15` selects UART for `FRICK_PLATFORM_MPFS` **or**
  `FRICK_PLATFORM_QEMU`; `driver-development.md` §1 states MPFS→UART, "otherwise
  →loopback".
- No stale "absorbed"/"M4-absorbed" scope wording remains outside History rows
  (and the CHANGELOG change description); the DDJF acronym is registered in the
  title, glossary, README index and AGENTS.md.
- The link/TSN symbols and referenced doc paths resolve.

**Assumptions:**

- The working tree under review is exactly `ce0bff3` + the modifications listed by
  `git status`; the increment is documentation/requirements-only (no code/build/
  test change).
- `docs/reviews/2026-10-12-m4-restored-scope-r3-review.md` is the R3 record of
  this same change (FRICK-REVIEW-0137) and is intended to be committed with it.

**Inferences:**

- R4-01, R4-02 and R4-03 are local residues of a rapid re-plan/reconciliation of
  previously-absorbed scopes; they do not affect the delivered baseline.
- R4-02 is the same class as R3-02 (default-backend status): the SSS side was
  corrected but the SRS child + manifest justification were not.

**Recommendations:**

- Resolve R4-01..R4-03 with recorded dispositions and a delta re-review by R4
  (N12) before commit/push; track R4-04..R4-08 to closure.
- Prefer generating `traceability.md` §1 and the intro parent list from the
  manifest to prevent matrix/manifest divergence recurring (R4-01).
- Ensure the R3 delta re-review (N12) closes the FRICK-REVIEW-0137 NCs so the
  committed review record matches the reviewed revision (R4-07).

**Confidence:** **High** for the facts, the header/History agreement, the count
consistency, the DDJF registration and the traceability findings (directly
reproduced from the files and the two gates). **Medium** for R4-05 (sprint-total
intent) and R4-03 (whether the simplification is intended), which depend on
author intent rather than a single source of truth.

**Open gaps (not closed by this review):**

- Whether S4b/S4c/S4d change the sprint total (R4-05).
- The R3 NC close-out and delta re-review of the post-fix requirement set
  (R4-07; profile R3, not R4).

---

**Independence declaration (norm N9).** I performed this review in an
independent, fresh context with no part in authoring or executing the items under
review, based only on the evidence in the repository at `ce0bff3` + the
uncommitted working tree and the reference documents. I am read-only for the
repository and, per norm N11 and the task, I wrote **exactly one** file — this
report — and modified, created or deleted no other file; I did not commit or push.

Committed and pushed with the reviewed change (norm N11).
