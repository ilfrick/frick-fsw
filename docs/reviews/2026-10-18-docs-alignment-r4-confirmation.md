# Independent R4 confirmation — documentation-vs-implementation alignment (2026-10-18)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0378                      |
| Reviewer session                  | Independent R4 documentation review (fresh context, norm N9/N17) |
| Reviewer profile                  | R4 (SW documentation), with source-level cross-checks against `src/` |
| Reference revision                | HEAD `e7e48ee` on `main` (= `origin/main` = `gitea/main`); alignment commit `2111968`; metrics refresh `5be0e8c` |
| Items under review                | `docs/reviews/2026-10-18-docs-implementation-alignment.md` (FRICK-REVIEW-20261018-DOCS-ALIGN, F-018-01..11) and the NC-EXT-94/95/96 registrations in `docs/planning/sprint-plan.md` §NC register |
| Date                              | 2026-10-18                             |
| Scope                             | Bounded independent confirmation of the claimed NC-EXT-96 corrections, the NC-EXT-94/95 statements, the traceability gate, and immutability of prior records. **No** N17/N20 phase, no release, no qualification, no NC-closure claim. |
| Independence declaration          | Fresh review context; no part in authoring commit `2111968`, the alignment report, the NC registrations, or any reviewed source file. This report modifies no file other than itself. |
| Norms applied                     | N1–N22 (see `docs/rules.md`), with emphasis on N3/N4/N8/N9/N11/N16/N17/N21/N22 |
| Review-ID note                    | The numeric ledger in `docs/reviews/README.md` reaches `0377`; `0378` is the next free numeric ID. The two 2026-10-18 reports use date-style IDs (`FRICK-REVIEW-20261018-DOCS-ALIGN`, `FRICK-REVIEW-20261018-POST-DOCS-MANIFEST-PA`); that dual convention is recorded here as a Comment (F-0378-09), not corrected. |

## 1. Method

Read-only verification of the actual files at `e7e48ee` (not the alignment report's
own claims). Executed:

- `git show --stat 2111968` / `git show 2111968 -- <file>` for the alignment delta
  (15 paths: 13 edited + 1 new report + 1 review-index row).
- Line-level reads of every corrected artifact plus its own `History` table.
- Source cross-checks: `CMakeLists.txt:19-26`, `src/pus/pus.c:160-275`,
  `src/pus/pus_services.c:458,472,495,514-522`, `src/transport/link/`,
  `tests/CMakeLists.txt:133-202`.
- `ctest -N` in `build/native` and `build/ci` (configured 2026-10-17, after the last
  change to `tests/CMakeLists.txt` at `1fdbb83`, 2026-10-17 22:50).
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` (read-only).
- `python3 tools/scripts/metrics.py --check` (read-only; verified against
  `tools/scripts/metrics.py:362-401` that `--check` never writes).
- `git diff 1fdbb83 2111968 -- docs/requirements/traceability.yaml docs/interfaces/ICD.md docs/requirements/traceability.md`
  (empty → no requirement/interface manifest edit).

Excluded from scope: the ICD itself (governed, not edited by the alignment),
all target/hosted/qualification gates, requirement completeness, and closure of
any NC (owner authority).

## 2. Per-item verification

### 2.1 README (NC-EXT-96 item: PUS-C default backend + S3 subservice codes) — CONFIRMED

| Claim | Evidence |
|---|---|
| PUS-C default backend now stated as N7/ESA (ADR-0019) | `README.md:159` — "PUS-C \| services 1/3/5/6/8/9/11/17/20; **default backend N7/ESA ASN.1 adapter** (`FF_USE_PUS_ASN1=ON`); open CCSDS codec alternative"; `README.md:172-175` — "the default backend is the N7/ESA ASN.1 adapter, with the open CCSDS codec as the built-in alternative" |
| Matches implementation default | `CMakeLists.txt:25` `option(FF_USE_PUS_ASN1 ... ON)`; `CMakeLists.txt:26` `option(FF_USE_N7_OBCP ... ON)`; `docs/adr/ADR-0019-n7-pus-backend-default.md:1-5` (Accepted, 2026-10-16) |
| S3 subservice code list is `1,3,5,6,27` | `README.md:180` — "1 create, 3 delete, 5 enable, 6 disable, 27 report (provider-registered; 5/6 set only the periodic state flag — no periodic emission path exists)" |
| Matches registration | `src/pus/pus_services.c:517-518` — `{3u,1u,svc3_create}, {3u,3u,svc3_delete}, {3u,5u,svc3_enable}, {3u,6u,svc3_disable}, {3u,27u,svc3_report}` |
| Added parenthetical is accurate | `svc3_enable` sets only `g_hk_reports[i].enabled = true` (`src/pus/pus_services.c:220-231`); `enabled` is written at `:226,:256,:271` and **never read** anywhere in `src/pus/` → no periodic TM[3,25] emission path |

Delta check: `git show 2111968 -- README.md` shows the two PUS rows and the
metrics-panel date changed; the S1 row was **not** touched (see F-0378-03).

### 2.2 SDD (OBCP row implemented; link backends) — CONFIRMED

| Claim | Evidence |
|---|---|
| OBCP no longer "planned" | `docs/design/software-design.md:121` — "OBCP adapter (M12 candidate) \| `src/obcp/ff_obcp.{c,h}` behind `FF_Obcp_t` ... default-built with `FF_USE_N7_OBCP=ON`; deferred, NC-S9-01"; `:126` still correctly reserves "In-house OBCP engine (M16) - planned" |
| Implementation present | `src/obcp/ff_obcp.c`, `src/obcp/ff_obcp.h` exist; `CMakeLists.txt:26` default ON |
| Link-backend list extended | `docs/design/software-design.md:67-69` — loopback (`link_loopback.c`), UART (`link_uart.c`), PUS-over-IP (`link_ip.c`), TSN (`link_tsn.c`), segmentation/reassembly (`link_seg.c`) |
| Matches tree | `ls src/transport/link/` → `link.c, link.h, link_ip.c/.h, link_loopback.c, link_seg.c/.h, link_tsn.c/.h, link_uart.c` |

Naming note (Comment, no defect): the task brief said "`ff_link_ip`/`tsn`/`seg`";
the real filenames are `link_ip.c` / `link_tsn.c` / `link_seg.c`. The document uses
the correct names.

### 2.3 `docs/architecture/pus-layer.md` §6 (S20 row) — CONFIRMED

- `docs/architecture/pus-layer.md:99` is the `## 6. Implemented services` heading;
  `:111` — "20 \| Parameter: set (TC[20,1], report id + value via a registered
  setter) and report (TC[20,3] -> TM[20,4], provider/table-backed)".
- Code agrees: `src/pus/pus_services.c:521` registers `{20u,1u,svc20_set}` and
  `{20u,3u,svc20_report}`; `svc20_report` returns `FF_PusSendTm(FF_PUS_SERVICE_PARAMETER, 4u, ...)`
  (`:495`) → TM[20,4].
- Header/history self-consistent: header `0.1.8 / 2026-10-18` (`:7-8`) matches the
  last history row `0.1.8 | 2026-10-18` (`:165`).

### 2.4 `docs/vv/verification-plan.md` §9 (CTest count) — CONFIRMED (43/43)

- `:207` S1 → "43/43 CTest (incl. `test_es_hs_lifecycle`, `test_decoder_corpus`)";
  `:210` S4 → "the S8-VV-FU coverage CTest subset passes (43/43)";
  history row `0.1.7 | 2026-10-18` (`:234`) records the correction from 40/40.
- Independent count: `grep -c add_test tests/CMakeLists.txt` = **43**, and
  `ctest -N` reports `Total Tests: 43` in both `build/native` and `build/ci`.
  Default options (`CMakeLists.txt:19-26`) enable every `if()` branch
  (`FF_BUILD_HAL/FF_USE_LIBM/FF_BUILD_SDLS/FF_BUILD_DRIVERS/FF_USE_LWIP/FF_USE_PUS_ASN1/FF_USE_N7_OBCP` all ON),
  so 43 is the configured and default suite size — the 40/40 figure in the
  superseded `0.1.6` row is correctly described as stale.
- Retained evidence agrees: `docs/vv/verification-results.md:40,475,556,593,601` and
  `docs/vv/evidence/2026-10-17-s8-reconciliation-ci-local.log` (43/43 + Lean + trace).

### 2.5 Six revision headers — CONFIRMED for the Revision field, Minor caveat on Date

| Document | Header revision (file lines) | Last `History` row | Revision matches | Date consistency |
|---|---|---|---|---|
| `docs/architecture/component-gap-analysis.md` | `0.4` (`:7`) | `0.4 \| 2026-10-16` (`:140`) | yes | **header Date `2026-10-11` (`:8`) ≠ 2026-10-16** |
| `docs/quality/licensing.md` | `1.7` (`:7`) | `1.7 \| 2026-10-16` (`:151`) | yes | **header Date `2026-10-14` (`:8`) ≠ 2026-10-16** |
| `docs/requirements/SSS.md` | `0.20` (`:7`) | `0.20 \| 2026-10-16` (`:215`) | yes | **header Date `2026-10-15` (`:8`) ≠ 2026-10-16** |
| `docs/requirements/SRS.md` | `1.36` (`:7`) | `1.36 \| 2026-10-16` (`:568`) | yes | **header Date `2026-10-15` (`:8`) ≠ 2026-10-16** |
| `docs/vv/verification-plan.md` | `0.1.7` (`:7`) | `0.1.7 \| 2026-10-18` (`:234`) | yes | consistent (`2026-10-18`) |
| `docs/vv/test-spec/ts-9-obcp-integration.md` | `0.4` (`:5`) | `0.4 \| 2026-10-16` (`:56`) | yes | consistent (`2026-10-16`) |

The header-sync deltas for these six documents touch **only** the revision field
(verified with `git show 2111968 -- <each file>`); no content, requirement or
history row was modified. Four of the six now carry a header `Date` that disagrees
with their own last history-row date → F-0378-04 (Minor).

### 2.6 Status/state documents (baseline-commit state) — acceptable point-in-time; Minor currentness residual

- `docs/status/status.json` `meta.updated = 2026-10-18`, and the note correctly
  states the S8 batch was committed as `1fdbb83` with pre-commit cycle evidence.
- `docs/status/loop-audit.md:9` baseline row = `1fdbb83`; history row `10.8` added
  (`:459`) and explicitly declares "no objective/norm disposition changed".
- `docs/status/handover.md:98-100` §4 = "latest pushed baseline `1fdbb83`"; hosted-CI
  bullet `:171-172` corrected to reference the pushed batch.
- `docs/planning/sprint-plan.md` `1.71 / 2026-10-18` (`:7-8`) with history row `1.71`
  (`:510`) registering NC-EXT-94/95/96.

**Assessment (material vs point-in-time).** These edits were factually correct when
authored (`1fdbb83` *was* the pushed baseline at `2111968`). The tree has since
advanced by two commits that add no source or requirement change: `5be0e8c`
(README metrics/status panel refresh) and `e7e48ee` (the PA manifest review report).
`origin/main` = `gitea/main` = `e7e48ee`, so:

- **Acceptable point-in-time records (no finding):** the `loop-audit.md` revision
  row/`Baseline commit` field (an N19 per-loop audit snapshot), the `sprint-plan.md`
  dated history row, and `status.json`'s note (it describes the batch commit, not a
  "latest pushed" superlative).
- **Minor residual currentness drift:** `handover.md` §4 heading/body asserts the
  *latest pushed baseline* is `1fdbb83`, which is no longer true (F-0378-05). This is
  not material to the delivered product scope because the two later commits are
  documentation/report-only, but `handover.md` is declared the authoritative resume
  point (N8), so the pointer should be refreshed at the next cycle rather than left
  as a stale superlative.

### 2.7 Independent re-derivation of NC-EXT-94 / NC-EXT-95 — both FACTUALLY ACCURATE

**NC-EXT-94** (`sprint-plan.md:231`): "`FRICK-ICD-001` §4 lists Service 1 subservice 6
... `FF_PusSendVerification` maps `FF_PUS_STAGE_PROGRESS` to subservice 5 only and no
code path emits TM[1,6]".

- `src/pus/pus.c:169-186` maps ACCEPTANCE→{1,2}, START→{3,4}, PROGRESS→`5u` (`:179`,
  unconditional), COMPLETION→{7,8}; there is no case producing subservice 6.
- Call sites of `FF_PusSendVerification`: `:240` (ACCEPTANCE/0 → 2), `:246`
  (ACCEPTANCE/1 → 1), `:254` (COMPLETION/0), `:260` (START/1 → 3), `:263`
  (PROGRESS → 5), `:269` (COMPLETION per result). Emitted set = {1,2,3,5,7,8}.
- `docs/interfaces/ICD.md:103-107` lists S1 subservices 1, 2, 3/5, **4/6**, 7, 8 as
  answered on the wire.
- **Accurate.** Scope note: the NC names only subservice 6, but the ICD row bundles
  "4 / 6 start / progress failure" and **no code path emits TM[1,4] either** (the
  only START call passes success=1). The discrepancy is therefore broader than the
  NC text; the owner disposition should cover both codes → F-0378-03.

**NC-EXT-95** (`sprint-plan.md:232`): "ICD §4 Service 20 row lists only the parameter
report (TM) side although `src/pus/pus_services.c` registers `{20,1}` set and `{20,3}`
report (`TM[20,4]`)".

- `docs/interfaces/ICD.md:121` — "`| 20 | - | parameter report | TM | table-backed parameters |`"
  (no TC side, no subservice column values).
- `src/pus/pus_services.c:521` registers `{20,1}`→`svc20_set` and `{20,3}`→`svc20_report`;
  `svc20_report` emits TM[20,4] (`:495`).
- **Accurate**, and correctly registered as an interface/completeness NC whose
  disposition belongs to the ICD authority (the ICD was deliberately not edited;
  `git diff 1fdbb83 2111968 -- docs/interfaces/ICD.md` is empty). Both NCs are
  correctly registered as **OPEN**, authority-owned, non-functional under N21.

### 2.8 Traceability gate and requirement/interface integrity — PASS / no change

```
python3 tools/scripts/trace.py docs/requirements/traceability.yaml
traceability: PASS  (117 requirements, 73 srs, 30 sss, 14 gaps, 62 lean theorems checked)
exit=0
```

The counts match the documents that quote them (`loop-audit.md` N4 row `:55`,
`status.json` note). `git diff 1fdbb83 2111968 -- docs/requirements/traceability.yaml
docs/requirements/traceability.md docs/interfaces/ICD.md` is empty, and the only
change to `SSS.md`/`SRS.md` is the header revision line (4 `+`/`-` characters total),
so the claim "no requirement IDs/semantics/traceability links changed" is supported.

### 2.9 Immutability of prior records — CONFIRMED

`git show --stat --name-status 2111968` lists `A docs/reviews/2026-10-18-docs-implementation-alignment.md`
and `M docs/reviews/README.md` (one added index row). **No pre-existing
`docs/reviews/*` report file was modified.** The alignment did not falsify any
immutable report; `docs/reviews/2026-10-18-post-docs-alignment-manifest-pa.md` was
added separately at `e7e48ee` and does not restate the alignment findings.

### 2.10 Generated/status surfaces — no drift found

`python3 tools/scripts/metrics.py --check` → `metrics panel: current` (exit 0), i.e.
the README metrics panel matches `status.json` + the traceability manifest + the
review log for this tree (the check ignores cross-compiler coverage rows). Working
tree was unchanged by the check (only the pre-existing modification noted in
F-0378-10).

## 3. Findings

| # | Severity | Item / reference | Finding | Evidence | Recommendation |
|---|---|---|---|---|---|
| F-0378-01 | Minor | `docs/requirements/SRS.md:500-502` — **SRS-REQ-018 (derived)**; ref. F-018-01, NC-EXT-96 | SRS-REQ-018 still reads "the reference build uses the open CCSDS backend by default; the N7/ESA ASN.1 backend and PUSopen(R) adapter are opt-in ... derived justification: ADR-0003/ADR-0018". This contradicts SRS-REQ-050 (`:271-279`, N7/ESA default per ADR-0019), SSS-REQ-015 (`SSS.md:181-184`), the manifest justification for the same ID (`traceability.yaml:193` — "N7/ESA is the configured default"), and the implemented default (`CMakeLists.txt:25`). Pre-existing drift from the ADR-0019 default change (introduced with commit `642ca71`, 2026-10-15; ADR-0019 is dated 2026-10-16) — **not** introduced by `2111968`. Non-functional under N21. | quoted lines as cited | Register a requirement-text NC (owner FSW + requirements authority; verifier R3 per N14) and correct SRS-REQ-018's text/justification to ADR-0019. Do not close by documentation edit alone without the requirements owner's disposition. |
| F-0378-02 | Minor | `docs/engineering/user-integration-manual.md:160-163` ("Backend note (PUS-C)"); ref. F-018-01 | The note states "The open CCSDS backend (`FF_PusBackendOpen`) is the default. The N7/ESA ASN.1 backend (`FF_PusBackendN7`) is optional ... (see `ADR-0018`)". It contradicts (a) the same document's §7 lead-in at `:121-125` ("The **default** backend is the N7/ESA ASN.1 one (`FF_PusBackendN7` ...)"), (b) ADR-0019, and (c) `CMakeLists.txt:25`. Pre-existing (introduced `642ca71`, 2026-10-15; stale since ADR-0019, 2026-10-16). Mitigation: the correct statement appears earlier in the same document. Non-functional under N21. | quoted lines as cited | Register/extend an NC (owner FSW/documentation) and align the note with §7 and ADR-0019. Flag to the documentation reviewer for the user-manual next delta. |
| F-0378-03 | Minor | `README.md:179` (S1 row) and `docs/interfaces/ICD.md:106` (S1 "4 / 6" row); ref. NC-EXT-94 | README's "Implemented subservices" list for Service 1 is "1,2,4,8 acceptance/start/progress/completion", but the code emits {1,2,3,5,7,8} (`pus.c:169-186,240-269`); code 4 is listed although never emitted, and codes 3/5 are omitted although emitted. The ICD's S1 row likewise presents "4 / 6 start / progress failure" as answered on the wire while neither 4 nor 6 is emitted. NC-EXT-94 therefore names only half the S1 gap (subservice 6). | as cited | Route to the NC-EXT-94 owner to (a) widen the NC (or the ICD row annotation) to cover subservice 4, and (b) register the README S1 row as a separate documentation NC. |
| F-0378-04 | Minor | Header/metadata sync of six documents; ref. F-018-07 | The header sync corrected the `Revision` field only. Four documents now show a header `Date` that disagrees with their own last history-row date: `component-gap-analysis.md` (header `2026-10-11` vs row `0.4` @2026-10-16), `licensing.md` (`2026-10-14` vs `1.7` @2026-10-16), `SSS.md` (`2026-10-15` vs `0.20` @2026-10-16), `SRS.md` (`2026-10-15` vs `1.36` @2026-10-16). The alignment report's stated method ("History tables are the canonical source for each doc's last revision/date") implies the date should have been synced too. | header/history lines as listed in §2.5 | Include the four date fields in the bounded header-metadata correction (editorial, N16/N17) and record it in each document's history or as a single bounded entry. |
| F-0378-05 | Minor | `docs/status/handover.md:98-100` (and `:171-172`); ref. F-018-08, NC-EXT-96 | "Current state (latest pushed baseline `1fdbb83` ...)" is stale: `origin/main` = `gitea/main` = `e7e48ee` (two later commits: `5be0e8c` metrics refresh, `e7e48ee` PA report). `handover.md` is declared the authoritative resume point (N8), so a wrong "latest pushed baseline" superlative is a currentness defect; it is **not** material to the delivered product scope (no source/requirement change in either later commit). Non-functional under N21. | `git rev-parse origin/main gitea/main` = `e7e48ee…`; `git log --oneline -3` | Refresh the §4 baseline pointer at the next cycle (state `e7e48ee` or describe `1fdbb83` plus the two doc/report-only commits). No retro-correction of the historical commits is required. |
| F-0378-06 | Minor | `docs/status/loop-audit.md:46` and `:70` vs its own header `:9` | The header baseline row now says `1fdbb83`, but the objectives-table row "Working-tree scope boundary" (`:46`, "Latest pushed baseline `b8c3aee`") and the N19 rules row (`:70`, "pushed HEAD `b8c3aee`") still assert the older baseline. Revision `10.8` deliberately changed only the baseline row, so this is an unchanged-scope partial refresh rather than a false new claim — but the document is now internally inconsistent. | as cited | Correct the two body cells (or explicitly scope them as historical) in a bounded follow-up; no re-audit of dispositions required. |
| F-0378-07 | Minor | `README.md` "Roadmap & sprint plan" prose (`:467-472`) | The paragraph still reads "it does not cover this uncommitted documentation delta" and "Current execution scope: S4c/S4d/defaults are committed at `13ebf92`". At `e7e48ee` (pushed) the tree is neither an uncommitted delta nor at `13ebf92`; the hand-written text (outside the generated `METRICS-BEGIN/END` panel) was not covered by F-018-08. Non-functional under N21. | as cited; `metrics.py` contains no writer for this paragraph | Register as a documentation NC and refresh the paragraph's baseline/currentness wording together with F-0378-05. |
| F-0378-08 | Minor | `docs/vv/test-spec/ts-6-sw-robustness.md:274-279` (§3.1) | §3.1 "Current automated evidence (as-is)" states "29 base ctest targets from `tests/CMakeLists.txt` ... 31 with `-DFF_USE_LWIP=ON` ... (`make test`, 40/40 default, 40/40 ASan)". The current suite is 43 registered tests (43 default and with lwIP; `ctest -N` = 43). Same class as the `verification-plan` §9 count corrected by F-018-06, but TS-6 was not updated. | `ctest -N`; `tests/CMakeLists.txt:133-202` | Register/extend a documentation-count NC for TS-6 §3.1 (owner test-spec author; verifier R2/R4) alongside NC-EXT-92-style count reconciliation. |
| F-0378-09 | Comment | `docs/reviews/2026-10-18-docs-implementation-alignment.md:59,79`; `docs/reviews/README.md:13`; review-ID convention | (a) The report's §5 residual gap and open question 4 state that `docs/reviews/README.md` was **not** indexed with the report ("deferred ... FSW engineer action"), yet commit `2111968` does index it (row inserted at `README.md:13`) — the report and its own commit disagree. (b) `docs/reviews/2026-10-18-post-docs-alignment-manifest-pa.md` is not indexed in `docs/reviews/README.md` (same class as prior F-0377-01). (c) The review log now mixes numeric IDs (…0377) with date-style IDs (two 2026-10-18 reports); this report adopts numeric `0378` to keep the ledger monotonic. | as cited | Correct the report's §5 wording or the index row (whichever is intended) in the same bounded documentation delta; index the PA report; keep or explicitly document the ID convention. |
| F-0378-10 | Comment | Uncommitted working tree | `git status` shows ` M docs/vv/evidence/2026-10-17-s8-reconciliation-candidate-manifest.md` — a refresh of the candidate identity manifest (filename dated 2026-10-17, content re-dated 2026-10-18) satisfying action `A-PA-2026-10-18-01` from `docs/reviews/2026-10-18-post-docs-alignment-manifest-pa.md`. This review was performed against HEAD `e7e48ee` **plus** that uncommitted modification; the file was not touched by this reviewer. Note for the owner: the filename/content date mismatch and the manifest's own bootstrap-hash convention should be as-declared when the refresh is committed. | `git status --porcelain`; manifest head | Confirm the manifest refresh is committed with the next compliant change (N11) and that its filename/date usage is intentional. |

Severity definition used: template `docs/reviews/review-template.md:32-38`
(Major disqualifies or defeats a requirement/safety barrier; Minor must be
corrected or justified but does not invalidate the requirement; Comment is an
observation). All findings are documentation/records findings; none alters
implemented wire behaviour, requirement semantics, or a safety barrier.
Per N21 they are non-functional and do not block unrelated development; none is
closed by this report.

## 4. Verdict (bounded)

- **NC-EXT-96 corrections: CONFIRMED present and correct at `e7e48ee`** for every
  enumerated item, independently re-derived from files rather than from the
  alignment report: README PUS-C default backend + S3 codes (2.1), SDD OBCP/link
  (2.2), `pus-layer` §6 S20 (2.3), `verification-plan` §9 CTest 43/43 (2.4, verified
  against `ctest -N` = 43), the six revision headers (2.5, Revision field only), and
  the status/handover/loop-audit baseline-state edit (2.6). No requirement ID,
  requirement semantics, traceability link, or governed ICD was changed by
  `2111968`; `trace.py` PASS (117/73/30/14, 62 Lean theorems); `metrics.py --check`
  = current; prior `docs/reviews/*` reports are unmodified.
- **No further drift was introduced by `2111968`/`5be0e8c`.** The findings above are
  pre-existing drift of the same class (F-0378-01, -02, -03, -07, -08), residual
  incompleteness of the correction itself (F-0378-04, -05, -06), or record/index
  hygiene (F-0378-09, -10).
- **Conclusion: APPROVED WITH COMMENTS** for the bounded scope of this review
  (template option 2). All findings are Minor or Comment; **no Major** finding was
  identified. **No N17/N20 phase pass, no release, no qualification, and no NC
  closure is claimed or implied** — this is a bounded confirmation of a bounded
  correction.

## 5. Recommended NC-EXT-96 disposition (owner decision; this reviewer cannot close an NC)

Recommendation, in order:

1. **Confirm NC-EXT-96 as verified for its enumerated corrections** (documentation
   grounds are satisfied: every claimed correction exists, is correct, and matches
   the implementation/current suite).
2. **Do not close it as "fully corrected" yet.** Two conditions should be met first,
   because they fall inside the NC's own wording ("six document revision headers
   lagging their own history"; "status/handover/loop-audit baseline-commit state"):
   - bounded editorial delta aligning the four header `Date` fields (F-0378-04);
   - baseline-pointer correction in `handover.md` (F-0378-05) and the
     `loop-audit.md` body cells (F-0378-06).
3. **Register a successor NC** (or explicitly widen NC-EXT-96) for the pre-existing
   residual drift found by this confirmation: SRS-REQ-018 text vs ADR-0019/SRS-REQ-050
   (F-0378-01 — route through the requirements authority, R3/N14), the
   user-integration-manual backend note (F-0378-02), the README S1 subservice list and
   the wider ICD S1 subservice-4 gap (F-0378-03 — coordinate with the NC-EXT-94 owner),
   the README current-execution-scope prose (F-0378-07), and TS-6 §3.1 counts
   (F-0378-08). Non-functional under N21: none of these needs to block the current
   functional work.
4. Then close **NC-EXT-96 narrowly**, with this report as the independent R4
   confirmation evidence, and without implying any gate, phase, release, or
   compliance status.
5. Housekeeping: correct the alignment report's §5/index inconsistency and index the
   PA report (F-0378-09); commit the manifest refresh (F-0378-10) with the next
   compliant change.

## 6. Uncertainty, confidence, and limitations

- **Confidence: High** for the confirmations in §2: all claims were re-derived from
  the files at `e7e48ee` with line-level citations, the CTest count was independently
  enumerated with `ctest -N`, and the trace gate was re-run read-only.
- **Confidence: Medium-High** for the findings' characterisation as "pre-existing,
  not introduced": established with `git log -S` for F-0378-01/-02/-03/-08 and with
  `git show` deltas for F-0378-04/-05/-06; the exact authoring intent of the
  header-sync method is inferred from the alignment report's own §6 assumption text.
- **Limitations.** This review did not re-verify the entire living document set for
  drift (only the enumerated NC-EXT-96 items plus the PUS/backend, S1/S20, CTest and
  status surfaces); it did not execute any build, test or emulation leg; it did not
  run `make metrics` (write mode) and did not assess the governed ICD's authority
  status; it makes no clause-level ECSS/PUS-C conformance statement. The
  `metrics.py --check` result ignores cross-compiler code-coverage rows.
- **Recheck trigger.** Any change to `src/pus/pus.c` / `pus_services.c` (S1/S3/S20),
  `tests/CMakeLists.txt` (suite size), `CMakeLists.txt` PUS/OBCP defaults, or the
  IC/vs-implementation status documents; and the next documentation delta touching
  the six header-synced documents.

## 7. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
| ---- | ---------------- | ---- | ---------- |
| R4 (N9/N17) | Independent R4 documentation review (this report, FRICK-REVIEW-0378) | 2026-10-18 | APPROVED WITH COMMENTS (bounded scope only; no phase/release claim) |
| Owner (NC closure) | *not this reviewer* | — | pending |

---

Retained report written 2026-10-18 by an independent R4 documentation review
(fresh context). No file other than this report was modified. Evidence preserved
in the working tree; commit/push remains the exclusive authority of the
`aeronautics-space-fsw-engineer` per N7. No compliance, certification,
qualification, approval, or NC-closure claim is made.
