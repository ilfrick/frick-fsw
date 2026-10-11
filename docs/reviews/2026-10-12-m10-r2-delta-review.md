# M10 (QEMU emulated-target) — Independent V&V Delta Re-Review (N12)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0125                      |
| Reviewer session                  | (independent agent, norm N9)           |
| Reviewer profile                  | R2 (SW V&V)                            |
| Reference revision                | base commit `aee4135` + the M10 closure increment (working tree, 2026-10-12) |
| Items under review                | `tools/scripts/qemu_vv.sh`; `.github/workflows/nightly.yml`; `docs/vv/tool-qualification.md` (T18/T20 + evidence mapping); `docs/vv/emulated-target.md`; `docs/vv/verification-plan.md`; `docs/vv/verification-results.md`; `docs/requirements/traceability.yaml` + `traceability.md` (SRS-REQ-043) |
| Date                              | 2026-10-12                             |
| Scope                             | DELTA RE-REVIEW (norm N12) of the fixes to FRICK-REVIEW-0121 (R2 M10 QEMU review, **REJECTED**, Major **R2-01** + minors): whether R2-01 is now truly closed (T20 CI-exercised and falsifiable) and R2-02..R2-07 dispositioned. Adequacy/honesty of the QEMU evidence, the T20 qualification basis and the SRS-REQ-043 traceability, against ECSS-E-ST-40C, ECSS-Q-ST-80C (tool/tool-qualification), ECSS-E-ST-10-02C. |
| Independence declaration          | Fresh context; reviewer took no part in authoring, executing or dispositioning the reviewed items. No docker/Renode/QEMU run performed (read-only host commands only, per task). **No repository file other than this report was modified.** |
| Norms applied                     | N1-N20 (`docs/rules.md`)               |

## 1. Scope

Delta re-review of the N12 close-out increment for **FRICK-REVIEW-0121**. The
increment claims: R2-01 closed (binding QEMU check T20 exercised by the nightly
workflow, exact app count asserted); R2-02..R2-07 dispositioned (mirror row
updated, results preamble rewritten, exact-count assertion, image id recorded,
T18 reproducibility caveat documented, driver-scope made explicit).

Excluded: the Renode leg (norm N18, T17 — already closed), the C framework
sources, the container build itself (not run per task constraints), and the
R1/R4/PA deltas of this closure increment (separate profiles). The prior-review
frozen hashes were re-verified below; all match.

### Frozen baseline verified

All frozen hashes match the re-review manifest (verified with `sha256sum`):

| Artifact | sha256 |
| -------- | ------ |
| `tools/scripts/qemu_vv.sh` | `b67a6b12f30346b0dfa78765584a36d80745b76ec22dc7eea058c28a08a8594f` |
| `.github/workflows/nightly.yml` | `cf514729fdd46aa3d8183cca3de9ef990ac85e2ae46d8f9d03aa587d4d7f5eb1` |
| `tools/scripts/setup_rtems.sh` | `cebb200b3e53441bc56f6bf9f1be5deb7abd8accf163444934521503359c68e7` |
| `docs/vv/tool-qualification.md` | `54aa5890f723122d1f48d56276b775c0a12dee5032ffd7fe4023da53e9f78518` |
| `docs/vv/emulated-target.md` | `797769a68d0946188fbd2c06619efb47e45d492e52a609c57134acc931555e18` |
| `docs/vv/verification-plan.md` | `f8cf4ac12a30cc754844267baf9c7a884b30452100b7e69bf1e8bd4752428dd0` |
| `docs/vv/verification-results.md` | `caaccaaab9c2b57cedceb95d81db9bd88117aecd68dbae92e1850f1bcdc1313b` |
| `docs/requirements/traceability.yaml` | `87578828d3df84ad55592b4cb15d9f60b83b744bdbe9548912167169da1ca6da` |
| `docs/requirements/traceability.md` | `47ba361a5ddac54cf41494098171c460a324dd01b932bebd5d8ea34cf111be4d` |

### Reproduction checks run (read-only)

- `bash -n tools/scripts/qemu_vv.sh` → **OK** (exit 0).
- `ctest --test-dir build/native` → **19/19 pass**, 0 failed.
- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` → **PASS**
  (`99 requirements, 58 srs, 27 sss, 14 gaps, 51 lean theorems checked`;
  `CI evidence claims: 12`) — consistent with `traceability.md` ("12 unique … 11
  referenced"; `ci-lean` unclaimed, norm N1).
- `python3 tools/scripts/metrics.py --check` → `metrics panel: current`.
- Read `build/rtems-generic/qemu-console.log`: line 1 records the toolchain image
  id (`frick-rtems-toolchain:local id sha256:90f81d0806112a0ddd95f60c9e9cc857de4acf8e71e3dbc9434f6612dcc2f9a0`),
  then the generic banner (`frick-fsw "Eärendil" v0.1.0`, `platform : qemu-virt-rtems`,
  `board: QEMU virt (generic rv64imafdc RTEMS) booting (generic RTEMS BSP)`),
  `frick-fsw: 6 applications registered`, per-app `app … started` lines and
  steady-state `TM -> link` frames.
- Inspected `.github/workflows/nightly.yml` (QEMU steps), `Makefile` (`qemu`,
  `qemu-image`), `tools/scripts/setup_rtems.sh` (step 3/3 builds `riscv/rv64imafdc`),
  `docs/vv/tool-qualification.md`, `docs/vv/emulated-target.md`,
  `docs/requirements/traceability.md`/`.yaml` (SRS-REQ-043).

Not performed (per task constraints): `docker build` / `make rtems-toolchain` /
`make qemu` / Renode / QEMU execution; CI (nightly) not run.

## 2. References

- Project norms: `docs/rules.md` (N1-N20); independent-review process
  `docs/engineering/independent-review.md`.
- Prior review under closure: `docs/reviews/2026-10-12-m10-r2-review.md`
  (`FRICK-REVIEW-0121`).
- Predecessor M10 review: `docs/reviews/2026-10-10-m10-emulated-r1-r2-r4-review.md`
  (`FRICK-REVIEW-0035`).
- `docs/vv/emulated-target.md` (FRICK-VV-EMU-001, Rev 0.3);
  `docs/vv/tool-qualification.md` (FRICK-VV-TOOLS-001, Rev 0.3.16);
  `docs/vv/verification-results.md` (FRICK-VV-RES-001, Rev 0.32);
  `docs/requirements/traceability.yaml` / `traceability.md` (FRICK-TRACE-001);
  `docs/requirements/SRS.md` (SRS-REQ-043).
- ECSS-E-ST-40C, ECSS-Q-ST-80C (tool/tool-qualification), ECSS-E-ST-10-02C.

## 3. Closure verification (R2-01..R2-07)

| Prior NC | Severity | Claimed disposition | Independent re-verification | Delta status |
| -------- | -------- | ------------------- | --------------------------- | ------------ |
| R2-01 | **Major** | `ci:emulated-run` produced by a **binding** QEMU check (T20) exercised by the **nightly** workflow; `qemu_vv.sh` asserts the exact app count and FAILs when absent; register records the T20 basis + T18 caveat | **Confirmed.** `nightly.yml:57-70`: `# evidence: ci-emulated-run …` → install `qemu-system-misc` → `make qemu-image` (generic `rv64imafdc`) → `make qemu` (`qemu_vv.sh`, exit code propagates) → `grep -aq "applications registered" build/rtems-generic/qemu-console.log`. `qemu_vv.sh:29,56` requires the exact `6 applications registered` **and** `frick-fsw`; a boot without them **FAILs (exit 1)**; absent image/runtime → SKIP (exit 0, not a pass). `tool-qualification.md:44-45` maps T20 to `ci:emulated-run` and names the nightly job. `setup_rtems.sh:33-75` (step 3/3) builds the QEMU BSP the nightly image needs. Local checks green (19/19, trace PASS). | **CLOSED** |
| R2-02 | Minor | `traceability.md` SRS-REQ-043 mirror row lists `qemu_vv.sh` + generic toolchain file | **Confirmed.** `traceability.md:105` lists `tools/scripts/emulated_run.sh, tools/scripts/qemu_vv.sh, cmake/toolchains/rtems-riscv-generic.cmake, docs/vv/emulated-target.md`; revision row `0.28` (2026-10-12) records the change. Matches `traceability.yaml:412`. | **CLOSED** |
| R2-03 | Minor | Results preamble states emulator console logs are regenerable (untracked) build outputs | **Confirmed.** `verification-results.md:14-17` now states the Renode/QEMU console captures are regenerable build outputs (not tracked), reproduced by `make renode`/`make qemu` from committed sources/toolchain/scripts. No longer claims "committed artifacts / no external state" for the console. | **CLOSED** |
| R2-04 | Minor | `qemu_vv.sh` asserts the exact count (not substring-only) | **Confirmed.** `qemu_vv.sh:29` `APPS_EXPECTED="6 applications registered"`; `:56` matches it exactly (plus `frick-fsw`) and PASSes only then. `tool-qualification.md:44` and `verification-results.md` row M10 now agree. | **CLOSED** |
| R2-05 | Comment | Toolchain image id recorded | **Confirmed.** `qemu_vv.sh:41` tees the `docker image inspect` id into the log; the retained console line 1 shows the id+digest. Provenance of the run is now tied to an image id. | **CLOSED** |
| R2-06 | Comment | T18 reproducibility caveat documented + tracked | **Documented:** `tool-qualification.md:59-66` records that the `frick-rtems-toolchain:local` image was assembled step-by-step and that a clean `docker build` was not completed locally. **"Tracked" is asserted but not anchored** (no owner/milestone in `loop-audit.md`/`class-a-gaps.md`/`development-plan.md`); see new finding **R2-D2**. | **CLOSED with comment** (R2-D2) |
| R2-07 | Comment | QEMU console banner attribution | **Confirmed (fixed at source).** The retained console now shows the generic banner `platform : qemu-virt-rtems` / `board: QEMU virt (generic rv64imafdc RTEMS)` instead of the MPFS-flavoured string, so the console is self-identifying; attribution no longer rests only on the build path. | **CLOSED** |

**R2-01 closure assessment.** The Major was an *evidence/qualification
overclaim*: T20 was mapped to `ci:emulated-run` and marked QUALIFIED while no CI
job exercised `qemu_vv.sh`, so the register's own qualification procedure step 4
("exercised in the CI pipeline") was unmet. The increment resolves this at the
correct level: the binding check is now exercised by a real CI workflow (nightly),
it can genuinely **fail** (exact marker + non-zero exit), and the mapping names
that workflow. Design and documentation are now aligned with the actual
provenance; **R2-01 is CLOSED**. (Residual: the nightly is weekly, not
per-commit, and has not yet produced a recorded pass in this session — but the
qualification basis is now truthful and falsifiable, which was the finding.)

## 4. Findings (new, in the delta)

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R2-D1 | Minor | `docs/vv/verification-results.md:7-8`; `docs/vv/tool-qualification.md:7-8`; `docs/requirements/traceability.md:7-8` | **Header `Date` not advanced with the closure revision (N16/N8).** The closure increment bumped the revisions to `0.32` / `0.3.16` / `0.28` and added the matching `## History` rows dated **2026-10-12**, but the document header `Date` still reads **2026-10-11** on all three. `emulated-target.md` (0.3, 2026-10-12) is consistent. The V&V evidence record therefore carries a header date one day behind the row it documents. | `verification-results.md:7-8` + history `0.32` (2026-10-12); `tool-qualification.md:7-8` + history `0.3.16` (2026-10-12); `traceability.md:7-8` + history `0.28` (2026-10-12) | Refresh the header `Date` to 2026-10-12 in the three documents (same commit). Primarily R4 (N17) scope; raised here because these are V&V/qualification records. |
| R2-D2 | Minor | `docs/vv/tool-qualification.md:59-66` (T18 note); N10/N19 | **The T18 reproducibility caveat is disclosed but its "closure action" is not tracked (N19).** The note states re-qualification "is tracked as a closure action", yet no owner/due milestone/tracker id exists in `docs/status/loop-audit.md`, `docs/quality/class-a-gaps.md` or `docs/development-plan.md` (grep finds the caveat only in the register itself). N19 requires gaps to be tracked with an owner and a milestone, or closed in-loop. The disclosure itself is an improvement (the prior PA-02 "no disclosure" is addressed); only the tracking anchor is missing. | `tool-qualification.md:65-66`; negative grep for a tracking entry in `loop-audit.md`/`class-a-gaps.md`/`development-plan.md`; `docs/rules.md` N19 | Add a tracked entry (owner + milestone) for the clean `make rtems-toolchain` → `make qemu-image-docker` → `make qemu` re-qualification, e.g. in `loop-audit.md`/`class-a-gaps.md`, or state the caveat as an open action of the class-A/qualification register. Primarily PA (N20) scope; raised here for the T20 qualification record. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### Positive evidence

- The binding check is now **falsifiable and CI-exercised**: `qemu_vv.sh` requires
  the exact `6 applications registered` marker (plus `frick-fsw`) and exits 1
  otherwise; the nightly workflow runs it and guards the console, so a SKIP /
  absent marker fails the job.
- Provenance recorded end-to-end: toolchain image id+digest in the retained
  console; generic (self-identifying) banner; driver scope explicitly excluded
  (`emulated-target.md:64-66`, `traceability.yaml` `lean_note`).
- The SRS-REQ-043 mirror row, the results preamble and the register/mapping are
  now consistent with the authoritative manifest and the actual pipeline.
- All host reproduction checks green (ctest 19/19, trace PASS at declared counts,
  `metrics.py --check` current, `bash -n` clean).

## 5. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Major R2-01 **closed**; no new Major; two new Minor findings (R2-D1, R2-D2) tracked to closure
- [ ] REJECTED — one or more Major findings

**Verdict (profile R2): APPROVED WITH COMMENTS.**

The FRICK-REVIEW-0121 Major **R2-01 is CLOSED**: the QEMU leg is now produced by
a binding, falsifiable check (T20) exercised by the nightly workflow, the exact
application-registration count is asserted, and the tool register/evidence
mapping name that provenance. R2-02, R2-03, R2-04, R2-05 and R2-07 are **closed**;
R2-06 is **closed with a comment** (documented, but its "tracked" claim is not
anchored — R2-D2). Two new Minor findings (R2-D1 header dates, R2-D2 tracking)
are raised; both are documentation/tracking accuracy in the V&V records and do
not affect the adequacy of the QEMU evidence. No safety-barrier defect was found
and no open Major blocks the closure (norm N12).

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R2 (N9)       | `aeronautics-space-sw-vv-engineer` (independent) | 2026-10-12 | APPROVED WITH COMMENTS (R2-01 Major closed; R2-D1/R2-D2 Minor, tracked) |

## 7. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | ------ | --------------- |
| R2-01 | Major (prior) | T20 evidence mapping / `ci:emulated-run` | No CI job exercised `qemu_vv.sh`; T20 mapped to `ci:emulated-run` though qualification step 4 unmet | **closed**: nightly workflow runs the binding QEMU check; exact app-count assertion; mapping names the nightly job | `.github/workflows/nightly.yml:57-70`; `tools/scripts/qemu_vv.sh:29,56`; `docs/vv/tool-qualification.md:44-45` | frick-fsw | **closed** | FRICK-REVIEW-0125 (this report) |
| R2-02 | Minor (prior) | `traceability.md` SRS-REQ-043 mirror | Stale mirror omitted `qemu_vv.sh` + generic toolchain file | **closed**: mirror row + rev 0.28 updated | `docs/requirements/traceability.md:105,216` | frick-fsw | **closed** | FRICK-REVIEW-0125 |
| R2-03 | Minor (prior) | Evidence retention | Preamble overstated committed console | **closed**: preamble states regenerable build outputs | `docs/vv/verification-results.md:14-17` | frick-fsw | **closed** | FRICK-REVIEW-0125 |
| R2-04 | Minor (prior) | `qemu_vv.sh` marker scope | Marker did not verify the count | **closed**: exact `6 applications registered` asserted | `tools/scripts/qemu_vv.sh:29,56` | frick-fsw | **closed** | FRICK-REVIEW-0125 |
| R2-05 | Comment (prior) | Tool provenance | Image id/digest not recorded | **closed**: id+digest recorded in the console/log | `tools/scripts/qemu_vv.sh:41`; `build/rtems-generic/qemu-console.log:1` | frick-fsw | **closed** | FRICK-REVIEW-0125 |
| R2-06 | Comment (prior) | T18/T20 reproducibility caveat | Clean `make rtems-toolchain` not demonstrated | **closed with comment**: caveat documented; tracking anchor missing (R2-D2) | `docs/vv/tool-qualification.md:59-66` | frick-fsw | **closed (see R2-D2)** | FRICK-REVIEW-0125 |
| R2-07 | Comment (prior) | Console banner attribution | MPFS-flavoured banner in the QEMU console | **closed**: generic `qemu-virt-rtems` banner (fixed at source) | `build/rtems-generic/qemu-console.log`; `docs/vv/emulated-target.md` | frick-fsw | **closed** | FRICK-REVIEW-0125 |
| R2-D1 | Minor (new) | `verification-results.md`, `tool-qualification.md`, `traceability.md` headers | Header `Date` (2026-10-11) not advanced with closure revision/history (2026-10-12) | open | N/A | frick-fsw (R4 owner) | open | — |
| R2-D2 | Minor (new) | T18 note / N19 | Caveat disclosed but closure action not tracked (no owner/milestone) | open | N/A | frick-fsw (PA owner) | open | — |

---

## Evidence consulted

- Frozen artifacts (hashes verified with `sha256sum`): `tools/scripts/qemu_vv.sh`,
  `.github/workflows/nightly.yml`, `tools/scripts/setup_rtems.sh`,
  `docs/vv/tool-qualification.md`, `docs/vv/emulated-target.md`,
  `docs/vv/verification-plan.md`, `docs/vv/verification-results.md`,
  `docs/requirements/traceability.yaml`, `docs/requirements/traceability.md`.
- Read: `docs/rules.md` (N1-N20), `docs/reviews/review-template.md`,
  `docs/reviews/2026-10-12-m10-r2-review.md` (FRICK-REVIEW-0121),
  `docs/reviews/2026-10-10-m10-emulated-r1-r2-r4-review.md` (FRICK-REVIEW-0035),
  `Makefile` (`qemu`/`qemu-image`/`qemu-image-docker`),
  `.github/workflows/ci.yml`, `.gitea/workflows/ci.yml`, `docs/status/handover.md`.
- Executed (read-only, host): `bash -n tools/scripts/qemu_vv.sh`;
  `ctest --test-dir build/native` (19/19);
  `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` (PASS,
  99/58/27/14, 51 theorems, 12 CI claims);
  `python3 tools/scripts/metrics.py --check` (current); `sha256sum`; `grep`;
  read of `build/rtems-generic/qemu-console.log`.
- Not performed (per task constraints): `docker build`/`make rtems-toolchain`/
  `make qemu`; no Renode/QEMU execution; no CI/nightly run.

Committed and pushed with the reviewed change (norm N11). Logged in
`docs/reviews/README.md` by the owner (the reviewer was constrained to modify
only this report; norm N11 record-keeping is the owner's action).
