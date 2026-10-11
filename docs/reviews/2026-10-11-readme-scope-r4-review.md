# Review Report - Entry-point, integration and extension documentation (README scope)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0046                      |
| Reviewer session                  | independent agent (norm N9)            |
| Reviewer profile                  | R4 (Senior SW documentation reviewer)  |
| Reference revision                | `2fbdeec` (HEAD; "PUS-C ASN.1 (N7/ESA) default backend via build-time generation (SRS-REQ-050, ADR-0016)") |
| Items under review                | `README.md`, `AGENTS.md`, `docs/engineering/user-integration-manual.md`, `docs/engineering/service-development.md`, `docs/engineering/driver-development.md`, `docs/architecture/pus-layer.md`, `docs/architecture/overview.md`, `docs/architecture/component-gap-analysis.md`, `docs/development-plan.md`, `docs/roadmap.md`, `docs/tooling.md`, `third_party/README.md`, `docs/interfaces/ICD.md`; consistency with `docs/rules.md`, `docs/requirements/{SRS.md,traceability.yaml,traceability.md}`, `docs/status/{handover.md,loop-audit.md,status.json}`, `docs/adr/ADR-0016-*.md`, `src/pus/pus.h`; the `docs` CI job checks |
| Date                              | 2026-10-11                             |
| Scope                             | Entry-point and integration/extension documentation: does the entry point summarise AND enumerate the project scope (middleware vs applications; implemented vs interface-only vs planned; HAL/driver coverage; PUS-C; networking/TSN; SDLS; libm; Renode/RTEMS V&V; milestones) for humans and agents; are the integration seams and the PUS service-extension path documented with concrete steps; internal consistency (counts, revisions, history, norm range, terminology, links). ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2. |
| Independence declaration          | Fresh context; I took no part in authoring, executing or reviewing the items. All findings are based on the repository contents at `2fbdeec` and the commands recorded in §3.1. |
| Norms applied                     | N1-N19 (see `docs/rules.md`; note the review template still prints "N1-N18" - finding R4-04) |

## 1. Scope

This is the dedicated R4 (norm N17) review of the **project entry point and the
integration/extension guides**, requested in the scope deferred by
`docs/reviews/2026-10-11-pus-c-asn1-default-r1-r2-r4-review.md`
(FRICK-REVIEW-0045, addendum §7: "Docs in the scope of the concurrent
README/integration R4 review (`README.md`, `docs/architecture/pus-layer.md`,
`overview.md`, `docs/tooling.md`, `third_party/README.md`) are reconciled in that
same review pass"). That pass is this review.

Reviewed: whether `README.md` works both as a concise summary and as a
sufficiently exhaustive scope statement for a fresh human and a fresh agent; the
user integration manual (seams: HAL contracts, `FF_Pus_Backend_t`,
`FF_NetStack_t`, tables, application registration); the PUS service extension
guide and the PUS layer architecture; the driver development guide; the
architecture, plan/roadmap, tooling and third-party entry points; and
cross-document consistency (counts, revisions, history, norm range, terminology,
links).

Excluded (not re-reviewed here): source-code correctness and the behavioural
defect register of FRICK-REVIEW-0045 (R1/R2), coverage instrumentation, CI
execution. Documented class-A gaps and planned milestones are assessed only for
whether the documentation states them honestly and distinguishes them from what
is implemented.

## 2. References

- Project norms: `docs/rules.md` (N1-N19)
- Requirements: `docs/requirements/SRS.md`, `traceability.yaml`, `traceability.md`
- Decision: `docs/adr/ADR-0016-pus-c-asn1-default.md`; `docs/adr/ADR-0003-*`, `ADR-0014-*`
- Product boundary: `docs/development-plan.md` §1, `docs/roadmap.md`
- Predecessor (deferred scope): `docs/reviews/2026-10-11-pus-c-asn1-default-r1-r2-r4-review.md` (FRICK-REVIEW-0045)
- Reviewer profile: `docs/engineering/independent-review.md` (R4)
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| R4-01 | **Major** | `README.md` (N8; OBJ-01; SRS-REQ-050/SSS-REQ-015) | **The primary human entry point states the wrong current default PUS-C backend and omits how to enable the real one.** `README.md` says the backend is "open codec (default)" and never mentions the N7/ESA ASN.1 default (SRS-REQ-050, ADR-0016) nor the build-time generation step (`make pus-asn1`). A fresh human/agent reading the entry point concludes the open CCSDS codec is the baseline and has no path to the actual default. This is exactly the class of stale statement this review was chartered to close and that FRICK-REVIEW-0045 explicitly deferred here. | `README.md:84-85` ("pluggable backend: open codec (default) and documented commercial PUSopen adapter (optional, `FRICK_USE_PUSOPEN`)"); `README.md:100-101` ("PUS-C ... over the open CCSDS backend"); README Quickstart `README.md:238-258` contains no `make pus-asn1`. Contradicts `SRS.md:251-255` ("shall use the N7 Space / ESA `asn1-pusc-lib` ... as the **default**"), `ADR-0016:17-27` (`FF_USE_PUS_ASN1` default ON), `CMakeLists.txt:25` (`option(FF_USE_PUS_ASN1 ... ON)`), `AGENTS.md:59` (`make pus-asn1`). FRICK-REVIEW-0045 §7 defers these README/architecture docs to this pass. | State in Highlights: default = `FF_PusBackendN7` (N7/ESA `asn1-pusc-lib`, ASN1SCC-generated at build time; SRS-REQ-050, ADR-0016), with the open backend as the always-available fallback/alternative and PUSopen(R) optional. Add `make pus-asn1` to the Quickstart with the one-line caveat that a checkout without generated code falls back to the open backend with a warning, and note `FF_USE_PUS_ASN1` (ON default; `=OFF` forces open). |
| R4-02 | Minor | Stale default-backend docs, all affected files (N8; recurrence of FRICK-REVIEW-0045 NC-4) | The same stale statement persists in every document named by FRICK-REVIEW-0045 finding 4, **although NC-4 is recorded CLOSED**. The repository at `2fbdeec` still declares the open backend the default in the architecture/tooling/code-comment set, and `third_party/README.md` still uses the obsolete option name. | `docs/architecture/pus-layer.md:25` ("Default open backend (`FF_PusBackendOpen`)"); `docs/tooling.md:27` ("open backend (default)"); `docs/architecture/overview.md:100,109` ("open backend vtable", "PUS init with the open backend"); `docs/architecture/rtems-integration.md:81` ("PUS (open backend)"); `src/pus/pus.h:9-12` ("Two backends are defined ... `FF_PusBackendOpen` ... (default ...)" and no `FF_PusBackendN7`); `third_party/README.md:23,25` (`FF_USE_N7PUS=ON`, "Both default OFF"). FRICK-REVIEW-0045 follow-up records (`...pus-c-asn1-default-r1-r2-r4-review.md:136`) mark NC-4 "CLOSED" - the closure is not substantiated. | Update all six locations to name `FF_PusBackendN7` as the default (open retained as fallback/alternative); fix `third_party/README.md` to `FF_USE_PUS_ASN1` (default ON) and correct the "Both default OFF" sentence. Re-verify the NC-4 closure against the files. |
| R4-03 | Minor | `docs/requirements/traceability.md`; `traceability.yaml` SSS-REQ-015 (N4/N8/N16; recurrence of FRICK-REVIEW-0045 NC-5) | The human-readable traceability matrix still carries the **old** SRS-REQ-050 row and the SSS-REQ-015 justification still says the open backend is the default, directly contradicting SRS-REQ-050 (and the machine-readable manifest). NC-5 is recorded CLOSED but the matrix is unchanged (still rev 0.20, `Date 2026-10-10`). | `docs/requirements/traceability.md:109` (`SRS-REQ-050 \| docs/status/loop-audit.md, third_party/asn1-pusc-lib \| ci:docs`) vs `traceability.yaml:475-476` (`src/pus/adapters/pus_adapter_n7.c, tools/scripts/gen_pus_asn1.sh, tools/docker/asn1scc/Dockerfile` / `tests/unit/test_pus.c, ci:renode`); `traceability.yaml:618` ("Norm N1 keeps the auditable open backend as the default"); `traceability.md:7-8` (header rev/date). | Regenerate/align `traceability.md` (SRS-REQ-050 row, counts, history/date) from the manifest and correct the SSS-REQ-015 justification and implementation list. |
| R4-04 | Minor | Norm-range references (N8/N16/N17; recurrence of FRICK-REVIEW-0036 F-08 / -0037 D-01) | **The norm-range sweep was not repeated for N19.** Current documents (and the review template) still enumerate "N1-N18" although norm N19 now exists; `docs/rules.md` itself is titled "N1-N18" while defining N19. `README.md` is internally inconsistent (line 20 says N1-N19, line 283 says N1-N18). | `docs/rules.md:1` ("Non-derogable Norms (Rules N1-N18)") vs `docs/rules.md:301-318` (defines N19); `README.md:283` vs `README.md:20`; `docs/development-plan.md:10,47`; `docs/engineering/independent-review.md:53,108,139`; `docs/engineering/service-development.md:11`; `docs/engineering/driver-development.md:11`; `docs/engineering/user-integration-manual.md:15`; `docs/reviews/review-template.md:13,22`. | Bump the current docs, the `rules.md` title and the review template to N1-N19; leave point-in-time `docs/reviews/2026-10-10-*`/`*m4-m11*` reports unchanged. Add the template and `rules.md` title to the N16/N8 change-sweep. |
| R4-05 | Minor | `README.md` scope section (N8; OBJ-01/OBJ-09) | **The scope statement is not exhaustive enough for its stated purpose.** The Highlights omit the **security (SDLS)** and **math (libm)** layers entirely (they appear only as path tokens in the repository layout and as documentation-index links), and there is no single consolidated "implemented here / interface-only / planned (Mx) / user-provided / hardware" table. A reader cannot tell from the Highlights that M6 (libm) and M7 (SDLS) are delivered. | `README.md:81-96` (Highlights: PUS-C, cFS-style core, same sources, Lean, V&V, backwards docs - no security/math); `README.md:271` (only `... math, security ...` inside the `src/` layout string); `README.md:295,321` (only documentation-index links). `README.md:151-195` architecture diagram has no security/math nodes while its legend (`:192-195`) is the only implemented/interface/planned key. Delivered status: `docs/status/loop-audit.md:29-30` (M6 delivered, M7 delivered); `docs/development-plan.md:155-169`. | Add security (SDLS) and math (libm) to the Highlights and add a consolidated scope table with a status column (Implemented / Interface (M3) / Planned (M4/M8/M12/M13/M14/M15) / User-provided / Hardware), covering core services, PUS, transport, HAL, drivers, networking/TSN, security, math, CFDP/VFS/S23, OBCP and SMP. |
| R4-06 | Minor | `docs/status/handover.md` (N8/N16; README `:325` names it the "resume point") | **The authoritative agent resume point is stale.** Counts, base commit, milestone status and the recent-review list no longer match the repository, and the newest change (PUS-C ASN.1 default / M14 / M15) is absent, so a fresh agent resuming from it would work from a wrong picture. | `handover.md:21,43` ("N1-N18"); `:64` ("trace ... PASS 85/48/23/14/51") vs the gate output `94/54/26/14/51` (`docs/requirements/traceability.md` counts are `SRS=54, SSS=26`); `:85` ("as of commit 12abdf4") vs HEAD `2fbdeec`; `:87` ("M1..M10 delivered") vs `development-plan.md:186-191`/`loop-audit.md:33` (M10 planned/partial); `:104` ("87 (50 srs / 23 sss / 14 gap)") vs 94 (54/26/14); `:105-106` lists reviews only up to FRICK-REVIEW-0042 (0043/0044/0045 exist); no mention of the PUS-C ASN.1 default, M14 or M15. | Refresh §0/§2 norm range, §3 trace counts, §4 base commit/milestone status/counts/recent reviews, and add the PUS-C ASN.1 default (SRS-REQ-050) plus M14/M15 to §4/§5. |
| R4-07 | Minor | `docs/engineering/service-development.md` (N8; service extension path) | **The extension guide contains an API error and dead/placeholder cross-references**, so following it verbatim will not build. | `service-development.md:76` "emit through EVS (`FF_EvsReport`)" - no such symbol; the EVS API is `FF_EvsSend` (`src/core/evs/evs.h:40`), as used in `user-integration-manual.md:73`. `service-development.md:116-117` references `src/fs/` (does not exist: `ls src/fs` fails) and `ff_vfs_create` (not implemented; M15) with the literal placeholder "`docs/engineering/...`". `service-development.md:125` "Milestone M3 of `docs/roadmap.md` builds this [S23] on the virtual filestore" - the filestore/S23 is M4-absorbed then M15 (`roadmap.md:80-82`, `development-plan.md:243-254`), not M3. | Replace `FF_EvsReport` with `FF_EvsSend`; mark the S23/VFS example as **planned (M15)** and reference a real path (`docs/architecture/pus-c-from-scratch-assessment.md`), or use an existing service as the worked example; replace the placeholder and fix the milestone reference. |
| R4-08 | Minor | `docs/architecture/pus-layer.md` §6 (N8; consistency with README/ICD) | The implemented-service matrix is incomplete/inconsistent: it omits **S11** (delivered as a stub) and abbreviates S9/S20 relative to the README and ICD matrices, so the "single" PUS catalogue disagrees across the three entry documents. | `pus-layer.md:71-80` (lists S1,3,5,6,8,9,17,20 - no S11) vs `README.md:105-113` (S11 "1,2 stub"; S9 "1 set, 2 report") and `ICD.md:55-70`. | Align the §6 matrix with `README.md` and `ICD.md` (include S11 stub; state S9/S20 subservices) and add the backend-default note (see R4-02). |
| R4-09 | Comment | `README.md` driver section (N8) | The phrase "the framework hides the hardware behind **three** interfaces" is inconsistent with the HAL design, which defines one contract per resource (~20 contracts) and classifies PAL/Link/Board as the three *extension points*. | `README.md:121`; `driver-development.md:20-27` (three extension points) but `:23-27` table + `:136-139` and `development-plan.md:60-92` (per-resource HAL contracts); `README.md:172` (diagram node H1 "HAL registry + contracts"). | Rephrase as "three extension points (PAL, link, board/BSP) plus the HAL contract set (M3), one contract per resource". |
| R4-10 | Comment | `docs/engineering/user-integration-manual.md` (N8) | The integration manual documents the seams well (app model, SB/EVS/TBL, providers, link, `FF_Pus_Backend_t`, `FF_NetStack_t`) but never states which PUS backend is the current default nor how to select/override it, and gives no pointer to the security (SDLS) integration seam. | `user-integration-manual.md:106-113` (mentions `FF_Pus_Backend_t` but not `FF_PusSetBackend`/`FF_USE_PUS_ASN1`/`make pus-asn1`); no SDLS/crypto-backend section (`FF_CryptoBackend_t`/`FF_Sa_t` appear only in `development-plan.md:92`). | Add a short "Select the PUS backend" note (default `FF_PusBackendN7`, `FF_PusSetBackend`, `FF_USE_PUS_ASN1`) and a one-line pointer to `docs/architecture/security-sdls.md` for the pluggable crypto backend. |
| R4-11 | Comment | `AGENTS.md` §3 quickstart (N8) | `make pus-asn1` is listed but its consequence is unstated, so an agent may treat it as optional and unknowingly build the open fallback. | `AGENTS.md:59` vs `CMakeLists.txt:25,50-57` (`FF_USE_PUS_ASN1` ON default; warning + fallback when generated code is absent). | Add a one-line note at `AGENTS.md:59` that this generates the default PUS backend and that its absence triggers the open fallback. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### 3.1 Verification performed (evidence)

| Check | Result |
| ----- | ------ |
| `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** (exit 0): `94 requirements, 54 srs, 26 sss, 14 gaps, 51 lean theorems`, 12 CI evidence claims |
| `docs` CI required-document list (both forges; `.github/workflows/ci.yml:157-185`, `.gitea/workflows/ci.yml:198-...`) | **PASS**: `missing: NONE` (37 required `.md` paths, both workflows) |
| Document-id uniqueness (CI `docs` job, norm N8/N16) | **PASS**: 91 unique `FRICK-*` ids, `dups: NONE` |
| Relative markdown links in `README.md`, `AGENTS.md`, `user-integration-manual.md`, `service-development.md`, `driver-development.md` | **PASS**: all resolve (`broken: NONE`) - **no dead links found** |
| Stale-backend scan `grep -rn "open codec (default)\|open backend (default)\|Default open backend\|PUS (open backend)"` | **FAIL (findings R4-01/R4-02)**: `README.md:84`, `docs/tooling.md:27`, `docs/architecture/pus-layer.md:25`, `docs/architecture/rtems-integration.md:81`; `src/pus/pus.h:9-12`; `third_party/README.md:23,25` |
| Wrong-option scan `grep -rn "FF_USE_N7PUS"` | **FAIL (finding R4-02)**: `third_party/README.md:23`, `docs/adr/ADR-0014-drivers-and-third-party.md:46` |
| Norm-range scan `grep -rn "N1-N18"` (current docs) | **FAIL (finding R4-04)**: `docs/rules.md:1`, `README.md:283`, `docs/development-plan.md:10,47`, `docs/engineering/independent-review.md:53,108,139`, `docs/engineering/service-development.md:11`, `docs/engineering/driver-development.md:11`, `docs/engineering/user-integration-manual.md:15`, `docs/reviews/review-template.md:13,22` |
| API existence `grep -n "FF_EvsReport" src/` | **FAIL (finding R4-07)**: no definition; `src/core/evs/evs.h:40` defines `FF_EvsSend` |
| `ls src/fs` | **FAIL (finding R4-07)**: no such directory (referenced by `service-development.md:116`) |
| `git status` / `git log` | Clean tree at `2fbdeec`; FRICK-REVIEW-0045 changes are committed (contradicting the "pending" closure evidence for NC-4/NC-5) |

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [x] **REJECTED** - one or more Major findings

**Graded conclusion**

- **R4 (SW documentation, N17): REJECTED.** The integration and extension guides
  are on the whole well structured and the requirement/V&V/architecture
  documentation is strong and internally traceable; the documentation/CI gates
  pass (trace PASS, required-document list complete, 0 duplicate ids, 0 dead
  links). However the **primary human entry point still states the wrong current
  default PUS-C backend and omits how to enable the real one (R4-01, Major)**,
  and this is not an isolated line: the same stale statement persists in the
  architecture/tooling set the predecessor review's NC-4 declared closed
  (R4-02), the human traceability matrix and SSS-REQ-015 justification still
  contradict SRS-REQ-050 (R4-03), the norm-range sweep was not repeated for N19
  (R4-04), and the README scope statement does not yet surface the security and
  math layers or a consolidated implemented/interface/planned view (R4-05). The
  authoritative agent resume point is stale (R4-06) and the service extension
  guide has a real API error plus dead/placeholder references (R4-07).

**Overall: REJECTED** (1 Major, 7 Minor, 3 Comments). The fixes are small and
localised: make the README (and the five stale backend locations) state the N7/
ESA ASN.1 default and `make pus-asn1`, align the traceability matrix/SSS
justification, repeat the norm-range sweep for N19, refresh the handover, and
add the security/math scope lines plus a consolidated scope table. After the
Major is closed and the Minors are dispositioned, a delta re-review (norm N12)
should close the loop.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R4 (N9/N17)   | independent agent (fresh context) | 2026-10-11 | **REJECTED** (R4-01 Major) |

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| NC-01 | Major | `README.md` / SRS-REQ-050 | README states open backend as default and omits `make pus-asn1` | to dispose | pending | frick-fsw | **CLOSED** | pending |
| NC-02 | Minor | stale backend docs | pus-layer/overview/tooling/rtems-integration/pus.h/third_party still open-default; wrong option name | to dispose | pending | frick-fsw | **CLOSED** | pending |
| NC-03 | Minor | traceability.md + SSS-REQ-015 | matrix row and SSS justification contradict SRS-REQ-050 | to dispose | pending | frick-fsw | **CLOSED** | pending |
| NC-04 | Minor | norm range | current docs/template still N1-N18 after N19 | to dispose | pending | frick-fsw | **CLOSED** | pending |
| NC-05 | Minor | README scope | security/math not surfaced; no consolidated implemented/interface/planned table | to dispose | pending | frick-fsw | **CLOSED** | pending |
| NC-06 | Minor | `docs/status/handover.md` | stale counts/base commit/milestones/reviews; missing PUS-C ASN.1/M14/M15 | to dispose | pending | frick-fsw | **CLOSED** | pending |
| NC-07 | Minor | `service-development.md` | `FF_EvsReport` wrong API; `src/fs/`/`ff_vfs_create` dead/placeholder; wrong milestone (M3) | to dispose | pending | frick-fsw | **CLOSED** | pending |
| NC-08 | Minor | `pus-layer.md` §6 | S11 missing/inconsistent vs README/ICD | to dispose | pending | frick-fsw | **CLOSED** | pending |

> Author action required (norm N11): append the FRICK-REVIEW-0046 row to
> `docs/reviews/README.md` (and the still-missing FRICK-REVIEW-0045 row) and
> commit this report with the reviewed change. This reviewer is restricted to
> writing this report only.

---

## 7. Closure addendum (author)

- **R4-01 (Major) fixed:** README now states the **N7/ESA ASN.1 default** backend
  (Highlights + a consolidated scope table) and the Quickstart includes
  `make pus-asn1` / `FF_USE_PUS_ASN1`.
- **R4-02 fixed:** the stale "open default" is corrected in `pus-layer.md`,
  `overview.md`, `rtems-integration.md`, `tooling.md`, `pus.h` and
  `third_party/README.md` (obsolete `FF_USE_N7PUS` → `FF_USE_PUS_ASN1`, default ON).
- **R4-03 fixed:** `traceability.md` and `traceability.yaml` (SSS-REQ-015)
  reconciled with SRS-REQ-050.
- **R4-04 fixed:** N1-N18 → N1-N19 swept across `rules.md` title, README,
  development-plan and the engineering guides / review template.
- **R4-05 fixed:** README Highlights now include SDLS, libm, networking/TSN and
  the Renode V&V, plus a consolidated **implemented / interface-only / planned /
  user-provided** scope table (PAL, core, PUS, transport, HAL, drivers, net,
  security, math, CFDP/VFS, OBCP, SMP, V&V).
- **R4-06 fixed:** `handover.md` refreshed (commit, counts 94/54/26/14, M11-M15,
  PUS-C ASN.1, reviews to 0046).
- **R4-07 fixed:** `service-development.md` uses the real `FF_EvsSend`, and the S23
  example is marked as future (M15, `ff_vfs_create` intended API, not yet
  implemented).
- Comments addressed: README "three interfaces" → PAL/link/HAL contracts;
  integration manual points at the default backend/override and SDLS; AGENTS
  states the fallback consequence.

Re-verified: trace PASS (94/54/26/14), docs required-list, format clean.

---

Committed and pushed with the reviewed change (norm N11).
