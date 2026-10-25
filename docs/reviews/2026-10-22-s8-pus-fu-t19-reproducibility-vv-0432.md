# FRICK-REVIEW-0432 — S8-PUS-FU T19 generator identity / byte-reproducibility evidence (post-action, V&V, read-only)

**Profile:** senior software **Verification & Validation** engineer (R2), independent,
fresh context, **read-only** (no file created/edited/deleted; no build, test, container
run, emulator, probe or generator execution; `docker image inspect` used in inspect mode only)
**Date:** 2026-10-22
**Reviewer session:** `ses_e9d88facfffe6PKKCD2zfWl4no`
**Items under review:** `docs/vv/evidence/2026-10-22-s8-pus-fu-t19-generator-reproducibility.log`
(reported sha256 `1ceaf8aeddb9bd2f58600b8c08ab21517a2b05c7852e2dc111c9136a05071701`,
202 lines) and the retained raw run outputs under `build/t19-repro-20261022/`
**Disposition:** Review evidence only. **No NC closed**, no gate, qualification,
acceptance or release approved, no parameter/clock/timeout/instance/reset-policy
adopted, no implementation, no commit or push. The reviewed evidence log is **not**
edited or overwritten by this review.

> **Scope.** This is the **post-action** review of the produced T19 provenance /
> reproducibility evidence and of the executed task's eligibility. It **does not repeat**
> the phase's pre-action V&V addendum (task selection, the three owner requests, the
> frozen-manifest collision handling); the pre-action addendum already exists and is not
> re-issued here. Owner-request consistency is confirmed in one line each (§6) against the
> frozen bodies only.

> **Reviewer-continuity note (honest record).** A first attempt to obtain this same
> post-action review was aborted by the model provider with a usage-quota error
> **before any work product was produced** (no file created, no finding issued); it
> proves neither engineering success nor failure. The review was then performed in the
> separate fresh reviewer context recorded above. **Exactly one completed post-action
> review of this evidence record exists**; there is no partial or duplicate review to
> reconcile.

## 1. Record identity (independently recomputed)

| Item | Reported | Recomputed | Result |
|---|---|---|---|
| `docs/vv/evidence/2026-10-22-s8-pus-fu-t19-generator-reproducibility.log` sha256 | `1ceaf8ae…1701` | `1ceaf8aeddb9bd2f58600b8c08ab21517a2b05c7852e2dc111c9136a05071701` | **match** |
| line count | 202 | 202 | **match** |
| size / mtime | — | 16189 B, 2026-10-22 08:53:13 +0200 | recorded |

## 2. Verdict per check

| # | Check | Verdict | Evidence |
|---|---|---|---|
| 1 | **Generator identity** (image id, creation time, empty RepoDigests, linux/amd64, `ARG ASN1SCC_REF=3124590a…`, `asn1scc version 4.9.13.0`, `--help` exit 1) | **PASS** | `docker image inspect frick-asn1scc:local` → id `sha256:3efa44d08125efd576b1b00756daca6653bd4460601c07b4475c9d386ff1f028`, Created `2026-10-20T07:34:49.499151898+02:00`, RepoDigests empty, Os=linux Arch=amd64; `tools/docker/asn1scc/Dockerfile:17` = `ARG ASN1SCC_REF=3124590a1d57d7d7db97b8850a5e2687ef46d0c2`; `build/t19-repro-20261022/asn1scc_version.out` = `asn1scc version 4.9.13.0` (exit 0), `asn1scc_help.out` exit 1, no second version string. Mutable-input characterisation confirmed: Dockerfile:14 `FROM ubuntu:22.04`, :20-23 unpinned `apt-get install`, :26-27 `dotnet-install.sh --channel 10.0` |
| 2 | **Input identity** (pinned checkout, clean, 28 inputs, digests identical to the retained 2026-10-20 list) | **PASS** | `git -C third_party/asn1-pusc-lib rev-parse HEAD` → `2126df11cfe664b9f6f9d7d7a49312bb70768e75` (2024-01-24), `status --porcelain --untracked-files=all --ignored=matching` empty; 28 inputs (14 `.asn1` + 14 `.acn`); the 28 live digests are identical to the matching entries of the retained `docs/vv/evidence/2026-10-20-n18-build-input-hashes.sha256` (3601 entries, 689 asn1-pusc-lib related) — **referenced, not re-presented** |
| 3 | **Comparison claim** (36 files, 36/36 SHA-256 identical to the in-tree set) | **PASS** | §6 ≡ `runlocal.sha256` (`sha256sum -c` → 36/36 OK, exit 0); §7 ≡ `intree_pre.sha256` (`sha256sum -c` → 36/36 OK, exit 0); §6 ≡ §7 (identical); filename-set intersection is exact (no only-in-one-side); `comparison.txt` agrees (36/36, differing 0) |
| 4 | **Non-mutation** of the in-tree generated tree | **PASS** | `intree_pre.sha256` ≡ `intree_post.sha256` (36/36 equal); `git status --porcelain third_party/` empty; no new entry under `third_party/`; run-local output under `build/` is git-ignored (`.gitignore:2`); in-tree mtimes (2026-10-21 22:04:25) **predate** the run (2026-10-22 08:52:35) |
| 5 | **§3 stderr characterisation** (exit code, empty stdout, warning count and form) | **DEFECT (non-material)** — see **D-1** | `gen.exit` = `exit=0`; `gen.out` 0 B; warning count 12 correct; the quoted form string names one input file as the source of all 12, while the artefact shows 9 × `BasicTypes.acn`, 2 × `MessageType.acn`, 1 × `Parameter.acn` |
| 6 | **Scope / retitle claim** (not a discharge of NC-EXT-84; retitle is an NC-owner/PA decision) | **PASS** | `docs/planning/sprint-plan.md:221` — NC-EXT-84 subject = "N7 OBCP generated tree lacked recorded identity/provenance"; disposition "PA 0346 verified the 221-file observed flat tree … **Generator/tool version, reproducible generation and qualification implications remain unverified** … **OPEN**"; PA 0346 (:18-20) "does not establish immutable historical identity, reproducible generation, generator/tool version provenance or tool qualification. NC-EXT-84 remains OPEN for those residuals"; the "produced by the upstream `make flat`, not by asn1scc" attribution is verified at `third_party/n7s-obcp/Makefile:44` (`flat: embed`), pinned `55d6d97e7e7f`, 221 files in the flat tree; the T19 row `docs/vv/tool-qualification.md:42` is quoted verbatim and **not** superseded (byte-identical, manifest-verified) |
| 7 | **"What this record does NOT claim" (§8)** | **PASS** — all forbidden claims respected | full-record keyword sweep for `qualif\|approved\|closed\|closure\|reproduc\|attestation\|complian\|release\|commit\|push\|adopt` returns **only** disclaimer sentences; the record contains no T19 requalification, no image/build reproducibility, no signed attestation, no PUS-C/baseline compliance, no qualification/acceptance/release approval, no gate or NC closure, no parameter adoption, no commit/push, and no re-labelling of the retained N18 FAIL / Lean PASS results (the only occurrence of "N18"/"Lean" is the disclaimer itself, line 163) |
| 8 | **Manifest-collision disclosure (§9)** | **PASS** | the 8 named files are manifest entries and verify byte-identical; the manifest was **not** regenerated (its mtime 2026-10-22 01:17:16 predates the run 08:52:36); deferrals (i)–(vi) are enumerated; the staleness claim at `docs/vv/verification-results.md:114` ("19 entries") against the manifest's **22** is verified, the 19→22 delta being exactly the three entries added by the final acceptance checkpoint (0430, 0431, acceptance log); the "next free NC id = NC-EXT-207" claim is verified (highest registered = NC-EXT-206, `sprint-plan.md:325`) |
| 9 | **Manifest integrity** | **PASS** | `sha256sum -c docs/vv/evidence/2026-10-22-s8-fdir-wd-spec-docs.sha256` → **exit 0, 22 OK, 0 FAILED**; `sha256sum -c docs/vv/evidence/2026-10-21-s8-fdir-post-review-source.sha256` → **exit 0, 34 OK, 0 FAILED**; neither manifest lists the phase-new files (no manifest staleness introduced); the four frozen body digests recomputed and **all match**: spec `9b09605c…73d40`, proposal `749ecafc…f35e5`, retrieval log `54a44964…e36ed`, probe log `64139566…6c9bc` |
| 10 | **Command fidelity** vs `tools/scripts/gen_pus_asn1.sh` | **PASS** — one minor wording defect, see **D-2** | script :34-38 = the `docker run`, :40-45 = the six-file required check (`BasicTypes.h/.c`, `asn1crt.h/.c`, `asn1crt_encoding.c`, `asn1crt_encoding_acn.c`) — the record's list and **all six sizes match** the files exactly; :47-48 = `rm -rf "${OUT}"` / `mv "${TMP}" "${OUT}"` → the record's "the script itself was NOT executed" statement is exact, corroborated by the in-tree mtimes; the record labels its check "script lines 40-45 **equivalent**" and does not imply end-to-end script validation |
| 11 | **Task eligibility (post-action)** | **PASS** | within standing authority: the only artefact created is the new untracked evidence log; `find -newermt "2026-10-22 07:00"` over the repository (excluding `build/`) returns exactly that one file; no code, requirement, configuration, parameter, architecture or gate change; no NC row added, edited or closed (`sprint-plan.md` byte-identical); HEAD still `44d2af2…` with the log untracked (`??`); **not** a re-run of completed work (PA 0346 covered observed-tree identity only and explicitly disclaimed reproducible generation and generator/tool-version provenance); T19's own row invites the action ("retain new logs and obtain applicable independent delta review before qualification claim") |

## 3. Findings

| # | Severity | Item | Finding | Evidence | Recommendation |
|---|---|---|---|---|---|
| D-1 | **Minor (non-material)** | Evidence log §3 (lines 45-47) | The record states "stderr: 12 generator warnings, all of the form `'/pusclib/common/BasicTypes.acn:<line>:<col>: warning: endianness property has no effect here.'`". The **count (12) is correct**, but `BasicTypes.acn` is named as the source of **every** warning, while the retained artefact shows 12 warnings from **three** input files: 9 × `BasicTypes.acn` (35:11, 56:10, 75:8, 81:8, 35:11, 35:11, 35:11, 56:10, 56:10), 2 × `MessageType.acn` (26:16, 27:19), 1 × `Parameter.acn` (30:11); each warning is followed by the explanation line "Little/big endian can be applied only for fixed size encodings and size must be 16 or 32 or 64" (not quoted). The exit code (0), the empty stdout, the warning **count** and the 36/36 SHA-256 equality are unaffected — the defect is confined to the descriptive attribution | `build/t19-repro-20261022/gen.err` (36 lines = 12 warnings × 3) vs evidence log §3 | **Do not edit the retained log.** Carry as a documented limitation with the sentence in §4 below |
| D-2 | **Minor (non-material)** | Evidence log §3 (lines 40, 43-44) | The record calls the executed invocation "byte-identical to the script's docker invocation except that the output is mounted into the run-local directory". The retained `gen_cmd.txt` also adds **`--pull=never`** and uses literal absolute paths and a literal image name in place of the script's `${LIB}`, `${TMP}` and `${IMG}` (`gen_pus_asn1.sh:16,34-35`). Docker's default pull policy ("missing") would not pull an already-present local image, so the generator behaviour is identical and the image identity is separately confirmed by `docker image inspect`; the stated difference set is merely incomplete | `gen_cmd.txt` vs `tools/scripts/gen_pus_asn1.sh:16,34-38` | **Do not edit the retained log.** Carry as a documented limitation with the sentence in §4 below |
| O-1 | Comment | Evidence log §0 (line 6) | The verb "**discharges**" (the *generator/tool-version + reproducible-generation* part) sits in tension with §8's "NC-EXT-84 is **NOT** closed and **NOT** partially discharged". The substantive requirement (no closure / no partial discharge claimed) **is** met and is stated twice; the verb is not repeated elsewhere | evidence log §0 vs §8 | For the next authorised revision of the record, prefer "provides evidence for". Carried as a limitation; **no correction required in this phase** |
| O-2 | Comment | Evidence log §8 inventory | The non-claims inventory does not name **NC-EXT-136** (`sprint-plan.md:331`), whose registered residual is "**Byte-identity of a regeneration is not proven**; the hash records the set actually used" — the residual this evidence directly addresses. The record makes **no** closure or disposition claim on NC-EXT-136, so no forbidden claim is made; the inventory is incomplete | `docs/planning/sprint-plan.md:331` | NC-EXT-136 remains **OPEN** and un-dispositioned; its owner/PA disposition is **not** taken here. Registration of any NC in this phase is **deferred** (see §7) |
| O-3 | Comment | Evidence log §5 | `git status --porcelain third_party/` is **non-probative** for a wholly ignored path (it would be empty even after a mutation). The operative proof of non-mutation is the pre/post digest equality stated in the same section, independently reproduced here, plus the in-tree mtimes predating the run | `.gitignore:10-12`; `intree_pre.sha256` ≡ `intree_post.sha256` | No action; attribution note only |
| O-4 | Comment | Evidence log §10 | The reproduction recipe omits the `asn1scc --help` probe, three `docker image inspect` formats and the six-file required-presence check that §1/§3 report. §10 is labelled "(this phase, in order)" and is a subset | evidence log §1/§3/§10 | No action; completeness note only |
| O-5 | Comment | Evidence log §1 (NC-EXT-35 pointer) | NC-EXT-35 (`sprint-plan.md:172`) is registered against **T1** (the `frick-vv:local` image), not T19. The record's "see NC-EXT-35" is a **precedent pointer** for the mutable-input class, not a claim that NC-EXT-35 covers the T19 image; the record explicitly claims the image "NOT … reproducible, only *identified*" | `docs/planning/sprint-plan.md:172` | No action; cross-reference precision note |

## 4. Defect dispositions (retained evidence is not overwritten)

The reviewed evidence log is **retained** and must not be edited or overwritten (it is a
phase-new file whose digest is reported to the owner and the orchestrator). Both defects
are therefore carried here as documented limitations with the exact sentences to be used:

- **D-1:** "The retained record's §3 attributes all 12 generator warnings to `BasicTypes.acn`; the retained artefact `build/t19-repro-20261022/gen.err` shows the 12 warnings are distributed 9 × `/pusclib/common/BasicTypes.acn`, 2 × `/pusclib/common/MessageType.acn` and 1 × `/pusclib/common/Parameter.acn`, so the quoted form string over-generalises the source file; the warning count (12), the exit code (0) and the empty stdout are correct, the defect is confined to the descriptive attribution, and it is not material to the 36/36 byte-reproducibility claim."
- **D-2:** "The retained record's §3 describes the executed invocation as 'byte-identical to the script's docker invocation except that the output is mounted into the run-local directory'; the retained `gen_cmd.txt` also adds `--pull=never` and uses literal paths/image name in place of the script's `${LIB}`/`${TMP}`/`${IMG}`, so the stated difference set is incomplete; for an already-cached image the behaviour is unchanged and the claim is not material."

**Corrective wording for a future authorised revision of the log (§3, replacement for lines 45-47):**

> `exit=0   stdout: (empty)   stderr: 12 generator warnings, each of the form '<input file>:<line>:<col>: warning: endianness property has no effect here.' followed by the explanation line 'Little/big endian can be applied only for fixed size encodings and size must be 16 or 32 or 64'; by source file 9 warnings come from /pusclib/common/BasicTypes.acn (35:11, 56:10, 75:8, 81:8, 35:11, 35:11, 35:11, 56:10, 56:10), 2 from /pusclib/common/MessageType.acn (26:16, 27:19) and 1 from /pusclib/common/Parameter.acn (30:11) (no error output; the warnings are generator diagnostics on the pinned input model)`

## 5. Non-claims audit (§8 of the reviewed record)

| Forbidden claim | Respected? | Basis |
|---|---|---|
| T19 requalification / qualification | **Yes** | §8 first bullet quotes the T19 row verbatim and states "NOT requalified or qualified … This record is evidence for it, not the review itself"; `tool-qualification.md:42` byte-identical and not superseded |
| Generator-image reproducibility | **Yes** | §1 "NOT claimed reproducible, only *identified*"; §8 third bullet |
| Project / RTEMS build reproducibility | **Yes** | §8 fourth bullet (NC-EXT-40; NC-EXT-29 deferred to the last sprint) |
| Signed run attestation | **Yes** | §8 "No signed run attestation exists or is claimed (PA 0364:10)" — citation verified |
| PUS-C / S1-S3-S6 baseline compliance | **Yes** | §8 fifth bullet; no compliance language elsewhere |
| Qualification / acceptance / release approval | **Yes** | §8 fifth bullet; §9(iii)(iv): phase "stays not commit-eligible", "no N20 approval is claimed" |
| Gate closure | **Yes** | §8 "No gate is closed"; §9(iii) |
| NC closure | **Yes** | §8 "no NC is closed"; §0/§8: NC-EXT-84 explicitly not closed and not partially discharged; §9(v) defers NC registration |
| Parameter / clock / timeout / instance / cadence / reset-policy adoption | **Yes** | §8 fifth bullet lists all six as not adopted |
| Commit or push | **Yes** | §8 "Nothing is committed or pushed"; HEAD `44d2af2…` (2026-10-19), log still `??` |
| Re-labelling of prior results (N18 = FAIL, Lean = PASS) | **Yes** | §8 last bullet: retained N18 FAIL and Lean PASS evidence untouched and not re-run; the record contains **no** new N18 or Lean result |
| Re-presenting the 2026-10-20 digest lists as new | **Yes** | §2: the retained list "is REFERENCED here; it is not re-presented as new evidence" |

## 6. Owner-request consistency (one line each, frozen bodies only)

- **(a) Agenda A(b) public-source-only does *not* remove the hardware/documentation blockers (M-1 / M-2 / M-8 / M-15).** Consistent with the frozen bodies: spec:731 (agenda A) states that under (b) "the counter values, the lock behaviour and the unit remain closed"; spec:49-52 states the retrieved documents "do **not** close the device-documentation group"; G-A (spec:720) retains M-1, M-11, M-12, M-15; G-B (spec:721) retains M-2 and M-8.
- **(b) The mission reset-coverage requirement and the agenda-C option-set approval come first; the watchdog instance follows the approved architecture.** Consistent with the frozen bodies: G-C (spec:722) reframes the instance as architectural; agenda C (spec:734) carries "**no recommendation** — the choice is architectural" with the option set (a)–(e) OPEN; the record adopts no instance.
- **(c) B1 and F(a) still require explicit owner approval; the three NC-EXT-205 host cases are therefore NOT permitted yet.** Consistent with the frozen bodies: B1 (spec:732) and F(a) (spec:737) are OPEN; `sprint-plan.md:324` (NC-EXT-205) records the three host cases as "**not written**" and their residual as "to be dispositioned with agenda item F(a) when the case set is written".

## 7. Remaining PUS-side candidates — none eligible now

| Candidate | NC id | Disposition | Eligible now? |
|---|---|---|---|
| OBCP flat-tree generator/tool-version + byte-reproducibility (the direct analogue of T19, and the residual NC-EXT-84 was opened for) | NC-EXT-84 (`sprint-plan.md:221`) | owner-scheduled ("sprint: follow-up", owner FSW/CM, verifier PA) and inside §3.2's blanket deferral of open NCs without scheduled action | **No** — scheduling is the owner's call; technically within standing authority for *evidence generation* (the pinned OBCP checkout would first have to be copied out of tree, because `Makefile:44` `flat:` does `rm -rf $(FLAT_DIR)` **inside** the checkout) |
| ASN.1-set byte-identity residual | NC-EXT-136 (`sprint-plan.md:331`) | OPEN, "corrected; verify-then-close", inside the §3.2 catch-all deferral | **No** — evidence now exists but no disposition is claimed; owner/PA disposition required |
| PUS baseline / tailoring / service-allocation defects | NC-EXT-14, 15, 16, 18 (also 17, 19, 20, 21, 26) | authority-blocked (controlled PUS baseline, approved tailoring, ICD/interface authority) **and** deferred (§3.2 range "NC-EXT-08/10..28") | **No** |
| PUS S11 service (FF_NOT_IMPLEMENTED stub); full PUS-C service set | NC-EXT-22; GAP-A2 | postponed to M15 / S18-S25 | **No** |
| Generated-N7-SDK / PUSopen-SDK compatibility and adapter build profile | NC-EXT-20, NC-EXT-21 | driver-side (generated SDK) + M15/GAP-A2; independent V&V delta review still required | **No** |
| Ground-tool wire format, CCSDS sequence-count profile, UART framing, evidence provenance, licence boundaries, N22 mapping, CANopen serialization guard | NC-EXT-03, 06, 07, 29, 43, 47, NC-S4c-04 | owner-postponed to the last planned sprint (§3.2) | **No** |
| Narrowly closed / out of scope | NC-EXT-23, 24, 88, 89, 91 | already completed (closed narrowly by re-reviews 0355/0356 and PA 0345/0346) | n/a |
| Watchdog / FDIR / target items | NC-EXT-165..199, 201..206, NC-PAL-01, GAP-A5 | out of scope for the PUS side; blocked on M-1/M-2/M-8/M-15 plus owner agenda A/B1/C (and the N18 gate, NC-EXT-165) | **No** |

## 8. Conclusion

- [x] **APPROVED WITH COMMENTS** — the reviewed evidence record is **accepted as retained
  proof** of the T19 generator identity and the 36/36 byte-reproducibility **within its
  stated scope** (this cached image, this input checkout), with the listed corrections
  carried as documented limitations in this review record (D-1, D-2 non-material; O-1…O-5
  comments). **The retained evidence log is not edited or overwritten.**
- [ ] APPROVED — not applicable (two Minor findings raised)
- [ ] REJECTED — not applicable (no Major finding)

**Explicit non-approvals.** This review is **not** a T19 requalification, **not** an
NC-EXT-84 closure or partial discharge, **not** a gate, qualification, acceptance or
release approval, **not** a mission-parameter or architecture decision, and **not** any
change to the retained **N18 = FAIL** or **Lean = PASS** results, which remain as
retained and were neither re-run nor re-labelled.

**Scope of the reproducibility claim.** The demonstrated result is the **generator-level**
reproducibility of the ASN.1 (PUS-C) generated tree for one cached image and one pinned
input checkout. It is **not** an end-to-end validation of `tools/scripts/gen_pus_asn1.sh`
(the script was deliberately not executed because it replaces the in-tree tree), **not** a
claim that the generator image is reproducible (mutable base/apt/.NET inputs), **not** a
claim that the project or RTEMS build is reproducible, and **not** a tool qualification.

## 9. Sign-off

| Role | Agent / Reviewer | Date | Conclusion |
|---|---|---|---|
| R2 / SW V&V (N9) | independent reviewer context `ses_e9d88facfffe6PKKCD2zfWl4no` | 2026-10-22 | APPROVED WITH COMMENTS (scope-limited, as stated above) |

**Confidence:** High for checks 1-4 and 8-10 (every claim independently reproduced from
primary artefacts: image inspect output, retained raw run outputs, digest lists verified
with `sha256sum -c` at exit 0, manifest digests recomputed, register and PA-0346 bodies
read directly); Medium-High for check 7 (exhaustive for this record by keyword sweep plus
per-line reading, but cannot exclude an implication arising from juxtaposition); High for
check 11's standing-authority determination (repository state independently observed) and
**Medium** for the "no remaining eligible candidate" conclusion, because §3.2's blanket
deferral and NC-EXT-84's "sprint: follow-up" are in tension — resolving that tension is
the owner's decision, not this reviewer's.

## 10. Follow-up records and deferred metadata

| Item | Severity | Finding | Disposition | Owner | Status |
|---|---|---|---|---|---|
| D-1 / D-2 / O-1 … O-5 | Minor / Comment | see §3 | carried as documented limitations in this record; the retained evidence log is **not** edited (owner instruction: retained evidence must not be overwritten). Corrective wording for a future authorised revision is given in §4 | FSW (log author) | **open as observations**; no NC raised in this phase |
| O-2 (NC-EXT-136 inventory omission) | Comment | the reviewed record's §8 inventory does not name NC-EXT-136, whose residual this evidence addresses; no disposition is claimed | NC registration **deferred**: the NC register (`docs/planning/sprint-plan.md`) is an entry of the frozen watchdog documentation manifest and must not be modified in this phase; the next free id for the next authorised revision is **NC-EXT-207** | NC owner + PA | **NC-EXT-136 remains OPEN**, un-dispositioned |
| Review index / revision history / V&V records / handover | Comment | the index entry for this review, the N16 history rows, the V&V-record entry, the T19 tool-row update and the handover note would normally be written now; all those files are entries of the frozen watchdog documentation manifest | **deferred, not skipped**; the manifest was **not** regenerated and no manifest-listed file was modified. The manifest verifies 22/22 (exit 0) and 34/34 (exit 0) for the code manifest | FSW / PA / R4 | **deferred to the next authorised manifest regeneration** |

---

Review evidence retained in the working tree; the phase remains **not commit-eligible**
(no N17 index closure, no N20 entry, no NC registration). Nothing committed or pushed.
