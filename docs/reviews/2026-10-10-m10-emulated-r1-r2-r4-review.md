# Review Report - milestone M10 (emulated-target verification: Renode PolarFire SoC + QEMU)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0035                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (SW dev) / R2 (SW V&V) / R4 (SW documentation, N17) |
| Reference revision                | base commit `1eae292` + uncommitted working tree (M10 planning delta, 2026-10-10) |
| Items under review                | `docs/vv/emulated-target.md`; `tools/scripts/emulated_run.sh`; `docs/requirements/{SRS.md,traceability.yaml,traceability.md}` (SRS-REQ-043); `docs/development-plan.md`, `docs/roadmap.md`, `docs/status/status.json` (M10 / OBJ-18); `docs/quality/class-a-gaps.md` (GAP-A5); `docs/vv/tool-qualification.md` (T15/T16); `CHANGELOG.md`; `README.md`; `.github/workflows/ci.yml`, `.gitea/workflows/ci.yml`; `Makefile` |
| Date                              | 2026-10-10                             |
| Scope                             | (a) correctness/accuracy of the Renode `polarfire-soc` peripheral claims and the QEMU RTEMS claim; (b) honesty of what the advisory script/to is claimed to verify (not silicon; GAP-A5 partial); (c) SRS-REQ-043 requirement quality and traceability; (d) documentation completeness/correctness/consistency and counts vs the trace gate (N8/N11/N16/N17). ECSS-E-ST-40C, ECSS-Q-ST-80C; norms N1-N17. |
| Independence declaration          | Reviewer took no part in authoring, implementing or executing any reviewed item; fresh context, independent session. No repository file other than this report was modified. |
| Norms applied                     | N1-N17 (see `docs/rules.md`)           |

## 1. Scope

Reviewed the M10 planning delta against base commit `1eae292` and the
uncommitted working tree under three independent profiles:

- **R1 (design/code)**: the emulated-target script and the external-tool claims
  (Renode platform/peripherals, QEMU machine/BSP) for technical accuracy and
  internal consistency with the RTEMS build (`cmake/toolchains/rtems-riscv-mpfs.cmake`,
  `make rtems`) and the RTEMS 6.2 BSP variants.
- **R2 (V&V)**: honesty and falsifiability of the evidence behind SRS-REQ-043
  (`ci:emulated-run`), the advisory/proxy status, what the script actually
  checks versus what the requirement, manifest and V&V docs claim, and its
  traceability.
- **R4 (documentation, N17)**: completeness, correctness and consistency of the
  new/touched documentation; requirement/verification-record counts against the
  trace gate; cross-references and the README index/panel.

Verification actually executed in this session (host, Linux; independent of the
author):

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` -> **PASS**
  (`79 requirements, 44 srs, 21 sss, 14 gaps, 51 lean theorems checked`;
  `CI evidence claims: 11`).
- `cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build
  build/native && ctest --test-dir build/native --output-on-failure` -> **11/11
  PASS**, 0 failed.
- `bash -n tools/scripts/emulated_run.sh` -> **syntax OK**.
- `bash tools/scripts/emulated_run.sh` -> **advisory SKIP** (RTEMS toolchain,
  `qemu-system-riscv64`, `renode` absent), `exit 0`.
- `python3 tools/scripts/metrics.py` -> manifest **79 (srs=44, sss=21, gap=14)**,
  tests `65/79`, Lean `20/79` (differs from the committed README panel; R4-03).
- Independent extraction of CI `# evidence:` claim names: **11 unique workflow
  claim names** (`ci-*`), of which **10 referenced by the manifest** (`ci-lean`
  unclaimed) - contradicts `traceability.md:18-20` (R4-02).
- Fetched the upstream Renode platform definition
  (`renode/renode@master:platforms/cpus/polarfire-soc.repl`,
  `scripts/single-node/polarfire-soc.resc`, `scripts/single-node/icicle-kit.resc`)
  to check the peripheral/per-CPU claims (R1-02/R1-03).
- `git diff` of all tracked deltas and direct inspection of the new files.

Excluded / not executable here: Renode and QEMU are not installed and the RTEMS
6.2 cross toolchain is absent, so no actual emulated boot was performed; the
emulated path was exercised only on its SKIP branch and by inspection. The
Renode claims were checked against the upstream platform sources, not by running
Renode.

## 2. References

- Project norms: `docs/rules.md` (N1-N17)
- Traceability manifest: `docs/requirements/traceability.yaml`
- RTEMS 6.2 user manual, `riscv` BSP variants (`riscv/rv64imafdc`,
  `riscv/mpfs64imafdc`); Microchip PolarFire SoC.
- Renode upstream: `renode/renode@master:platforms/cpus/polarfire-soc.repl`,
  `scripts/single-node/polarfire-soc.resc`, `scripts/single-node/icicle-kit.resc`.
- Predecessor review: `docs/reviews/2026-10-10-m9-codequality-r1-r2-r4-review.md`
  (`FRICK-REVIEW-0034`, open residuals R4-01, R4-02, R4-03).
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2.

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| R1-01 | Minor | `docs/vv/emulated-target.md`, `tools/scripts/emulated_run.sh`, SRS-REQ-043; `cmake/toolchains/rtems-riscv-mpfs.cmake` | **The QEMU path runs the `mpfs64imafdc` image while the documentation states it runs the `riscv/rv64imafdc` BSP.** `emulated-target.md:24` says `qemu-system-riscv64 -M virt -bios <rtems.exe>` "runs the RTEMS `riscv/rv64imafdc` BSP", `SRS.md:213-214` and `tool-qualification.md:40` repeat it, but `emulated-target.md:33-34` states **both** tools run the **same** binary from `make rtems`, and `make rtems`/the toolchain builds only `riscv/mpfs64imafdc` (`rtems-riscv-mpfs.cmake:24`). `emulated_run.sh:22-24` configures exactly that toolchain and `EXE=build/rtems/frick_fsw`, so QEMU loads the MPFS BSP image, not an `rv64imafdc` image. The MPFS BSP is not a documented QEMU `virt` target (it boots from hart 1, `RISCV_BOOT_HARTID=1`, and embeds the MPFS DTB), so the documented QEMU verification cannot occur as described. | `emulated-target.md:24,33-34`; `emulated_run.sh:22-24,30-33`; `rtems-riscv-mpfs.cmake:24,33`; `SRS.md:211-216`; `tool-qualification.md:40`; RTEMS `riscv` BSP docs (rv* variants run on QEMU `virt`; `mpfs64imafdc` targets the Icicle Kit). | Either add a **second** RTEMS build for `riscv/rv64imafdc` and run *that* on QEMU, or correct every copy (SRS, manifest note, `emulated-target.md`, roadmap/plan, CHANGELOG, tool register) to say the `mpfs64imafdc` image is loaded on `virt`, with the boot-hart/DTB/caveats made explicit. |
| R1-02 | Comment | `docs/vv/emulated-target.md:23` (Renode peripheral claims) | **Two peripheral/CPU claims do not match the cited Renode platform.** (a) The doc lists "5 RiscV64 CPUs (E51 + 4xU54, `rv64imafdc`)"; in `polarfire-soc.repl` the E51 is `cpuType: "rv64imac_zicsr_zifencei"` (no F/D) and only the four U54s are `rv64gc_zicsr_zifencei` (= `rv64imafdc`). (b) "QSPI + MT25Q flash" is **not** modelled by the cited open platforms: `polarfire-soc.repl` contains no QSPI/MT25Q peripheral (only eNVM, L2 LIM, DDR and the SD/eMMC controller; `icicle-kit.resc` adds an eMMC image on `mmc`). The rest of the list (5x NS16550 UART, 2x SPI, 2x I2C, 2x CAN, 2x CadenceGEM + PHY, 3x GPIO, watchdog/RTC/timer, eNVM, USB, 2x PCIe, crypto, DDR mock, CLINT/PLIC/PDMA) matches the platform. | Upstream `renode/renode@master:platforms/cpus/polarfire-soc.repl` (`e51`/`u54_*` `cpuType`); `scripts/single-node/icicle-kit.resc`; `emulated-target.md:23`. | State the per-core ISA accurately (E51 = RV64IMAC, U54 = RV64GC/`rv64imafdc`) and either drop "QSPI + MT25Q flash" or attribute it to the board/vendor model rather than the open `polarfire-soc` platform. |
| R1-03 | Comment | `tools/scripts/emulated_run.sh:38-45` (Renode) | **The Renode invocation as written will not capture the boot banner.** The upstream ready-to-use script (`polarfire-soc.resc`) enables `showAnalyzer mmuart0` before running; `emulated_run.sh` creates the machine, loads the ELF and `start`s, but never attaches an analyzer/console backend, so `grep -qi "frick-fsw" "${LOG}"` cannot see MMUART output even if the image boots. (Likely rather than certain; Renode not installed here.) | `emulated_run.sh:40-45`; upstream `scripts/single-node/polarfire-soc.resc` (`showAnalyzer mmuart0`). | Add `showAnalyzer mmuart0` (and/or route the UART to a file) so the banner check is meaningful, and validate once with Renode installed before relying on it. |
| R2-01 | Minor | SRS-REQ-043; `tools/scripts/emulated_run.sh`; `traceability.yaml:415`; `docs/vv/emulated-target.md` | **The SRS-REQ-043 evidence is non-falsifiable and over-claims what is verified.** The script unconditionally `exit 0` and prints SKIP when the RTEMS toolchain/QEMU/Renode are absent - which is the CI case - so a green `ci:emulated-run` is produced with **zero** emulated execution. When it does run, it only greps `frick-fsw`; it does **not** check "applications registered" (unlike the simulator smoke job) and does not exercise any driver. Yet the requirement says "verifying the boot sequence, the PAL/RTEMS backend and **the drivers**", the manifest `lean_note` says "checking the boot banner **and app registration**", and `emulated-target.md:28-32` claims UART/GEM/CAN/SPI/I2C/GPIO/PCIe/SD/eMMC driver verification plus "TM/TC over an emulated link", while `emulated-target.md:47` also says the job "checks the boot banner + app registration". The requirement's "and/or" between Renode and QEMU makes the pass criterion ambiguous, and the advisory step cannot fail. | `emulated_run.sh:30-51`; `traceability.yaml:415`; `emulated-target.md:28-34,43-49`; `SRS.md:211-216`; live run: `[emulated] SKIP ...`, `EXIT=0`. | (a) Make the manifest note/row and the docs state plainly "advisory proxy; SKIP when tools absent; banner-only, no driver verification"; (b) actually grep `applications registered`; (c) only claim driver verification once Renode `.resc` tests exercise the peripherals and the step can fail; (d) remove the "and/or" ambiguity or define the pass criteria and evidence. |
| R2-02 | Comment | CI workflows; `traceability.yaml`; `docs/requirements/traceability.md:95` | **Proxy disclosure is inconsistent with the sibling `ci:rtems-cross-build` proxy.** The RTEMS cross build is explicitly annotated as a per-commit proxy/SKIP marker (`ci.yml:225-226`, SRS-REQ-037 `lean_note`), whereas `ci:emulated-run` is presented as ordinary test evidence in the manifest and in the SRS summary row (line 95) with no "proxy/SKIP/advisory" caveat, so the trace matrix reads as if emulated verification were demonstrably executed. | `.github/workflows/ci.yml:72-74,224-226`; `.gitea/workflows/ci.yml:75-77,265`; `traceability.yaml:408-415`; `traceability.md:95`. | Mirror the RTEMS-proxy wording for `ci:emulated-run` in the manifest note and the traceability row. |
| R4-01 | Minor | `docs/requirements/traceability.md:38` | **The top-down matrix omits the new child.** `SSS-REQ-013` is shown with only `SRS-REQ-020`, but the manifest makes `SRS-REQ-043` a second child of `SSS-REQ-013` (`traceability.yaml:410`), and the gate's hierarchy summary prints `SSS-REQ-013 <- 2 srs children`. | `traceability.md:38` vs `traceability.yaml:410`; `trace.py` output. | Update the cell to `SRS-REQ-020, SRS-REQ-043 (M10)`. |
| R4-02 | Minor | `docs/requirements/traceability.md:18-20` | **Stale CI-claim count - a regression introduced by M10.** The line says "CI evidence claims = 10 unique workflow claim names (9 referenced by the manifest)"; the gate now reports **`CI evidence claims: 11`** and independent extraction finds **11 unique `ci-*` names, 10 referenced** (the new `ci:emulated-run`). Adding the claim without updating the count leaves the human-readable mirror inconsistent with the authoritative gate (N4/N16). | `traceability.md:18-20`; `trace.py` (`CI evidence claims: 11`); independent `# evidence:` extraction. | State `11 unique` / `10 referenced` and note the new claim. |
| R4-03 | Minor | `README.md:40-78` (generated metrics panel) | **The generated status panel is stale and contradicts the manually-edited line above it.** Panel line 72 still reads `78 (srs=43, sss=21, gap=14)`, tests `64/78`, Lean `20/78`, while `tools/scripts/metrics.py` now produces `79 (srs=44, sss=21, gap=14)`, tests `65/79`, Lean `20/79`; `README.md:247` already says "SRS = 44". The panel was not regenerated after adding SRS-REQ-043 (recurrence of M9 `R4-01`). | `README.md:72,76` vs `python3 tools/scripts/metrics.py`; `README.md:247`. | Run `python3 tools/scripts/metrics.py --readme` in the same commit. |
| R4-04 | Minor | `docs/vv/verification-results.md` | **No M10 verification record; the cycle log is stale.** The last row (line 27, labelled "M4 ... + lwIP") reports `78 reqs / 43 srs / 21 sss / 14 gap`, `51`, `10 CI claims`, with no M10 row/note, although `emulated-target.md:47-49` promises the emulated evidence is recorded there and N11/N15 require the evidence in the same cycle (recurrence of M9 `R4-02`). | `verification-results.md:18-27`; `emulated-target.md:47-49`; `trace.py` (`79/44/21/14`, `11 CI claims`). | Add an M10 cycle row/note (host build, ctest 11/11, trace gate, advisory emulated SKIP) and refresh the counts to `79/44/21/14/51/11`. |
| R4-05 | Minor | `docs/vv/tool-qualification.md:54-65` | **The "Evidence claims mapping" table was not extended for the new tools.** No entry maps T15/T16 (nor the earlier T14) to the claim they support (`ci:emulated-run` / `ci:misra`), so the register does not complete the tool-to-evidence traceability it establishes for T1-T11. | `tool-qualification.md:39-40` vs `54-65`. | Add `T15/T16 -> ci:emulated-run` (and `T14 -> ci:misra`). |
| R4-06 | Comment | `docs/requirements/traceability.md:104-125` | The SSS verification summary (section 3) stops at `SSS-REQ-020`; `SSS-REQ-021` is absent (pre-existing from M9), although it exists in SSS.md, the manifest and the top-down matrix. | `traceability.md:104-125` vs `traceability.yaml:569+`; `SSS.md:93-95`. | Add the `SSS-REQ-021` row. |
| R4-07 | Comment | SRS-REQ-043 parentage | `SRS-REQ-043` is parented to `SSS-REQ-013` ("*Continuous verification*: every committed change shall pass the automated pipeline"). The emulated path is advisory and, when the tools are absent (CI), gates nothing, so the parentage is looser than the SSS text; it could equally sit under `SSS-REQ-011` (multi-target build). Defensible for a V&V capability, but the choice should be justified. | `traceability.yaml:410`; `SSS.md:62-64`. | Keep or move with an explicit one-line justification (N4). |
| R4-08 | Comment | `docs/vv/verification-plan.md:99-112,114-135` | The V&V environment/activity sections were not extended for M10, so the new path has no entry in the verification plan (the natural home for the emulated activity and its evidence location). | `verification-plan.md:103-112` (environment table ends at GitHub CI); no M10/emulated reference. | Add an "Emulated target (Renode/QEMU, advisory)" environment/activity row and cross-link `docs/vv/emulated-target.md`. |
| R4-09 | Comment | multiple touched files | Trailing newline still missing on several touched files (e.g. `traceability.md`, `SRS.md`, `tool-qualification.md`, `docs/development-plan.md`), recurrence of M9 `R4-08`. Cosmetic. | `tail -c1` on the listed files; `git diff` shows `\ No newline at end of file`. | Add the trailing newline. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### Positive evidence

- Trace gate **PASS** at the declared counts (`79/44/21/14`, 51 Lean theorems,
  11 CI claims); SRS-REQ-043 is present in both SRS.md and the manifest, with
  `implementation`, `tests` and an explicit `lean_note` (gate satisfied).
- `ctest` **11/11 PASS**; `bash -n` clean; `emulated_run.sh` exits **0** on the
  advisory SKIP branch as documented.
- The **advisory status is disclosed** in the CI step name ("Renode/QEMU;
  advisory"), the script header and `emulated-target.md` §4-6; GAP-A5 is
  correctly kept **open** with "partial mitigation" in `class-a-gaps.md:61-70`.
- Document plumbing is otherwise complete: `emulated-target.md` has a unique id
  (`FRICK-VV-EMU-001`) and history; `README` index, both CI workflows'
  required-docs lists, `CHANGELOG`, `development-plan` (M10/OBJ-18), `roadmap`
  (M10), `status.json`, `SRS.md`, `traceability.yaml/.md` and
  `tool-qualification.md` (T15/T16) are all updated in the delta.

## 4. Conclusion

- **R1 (design/code)**: **APPROVED WITH COMMENTS**. The advisory script is
  syntactically valid and consistent with its stated purpose, and the Renode/QEMU
  toolchain claims are mostly accurate against upstream sources. R1-01
  (documented QEMU BSP differs from the image the script builds/runs) must be
  corrected before M10 can be presented as delivered; R1-02/R1-03 are
  accuracy/validation Comments. No Major findings.
- **R2 (V&V)**: **APPROVED WITH COMMENTS**. The banner-only, always-`exit 0`
  proxy does not provide the verification SRS-REQ-043 and the maintainers' notes
  claim (R2-01), so `ci:emulated-run` currently reads as stronger evidence than
  it is; R2-02 asks for the proxy disclosure used for `ci:rtems-cross-build`.
  Both are Minor/Comment **while M10 remains `planned` and the path is declared
  advisory**. If the milestone is declared done/verified, or the advisory flag is
  removed, **R2-01 escalates to Major and the profile conclusion becomes
  REJECTED**.
- **R4 (documentation, N17)**: **APPROVED WITH COMMENTS**. The new V&V note is
  well-formed and the requirement/tool/document plumbing is broadly complete, but
  the human-readable trace mirror and generated panel were not regenerated for
  the new requirement/claim (R4-01..R4-05, recurrences of open M9 NCs), so the
  committed documentation counts currently disagree with the trace gate.

No Major findings were raised; all Minor findings are tracked to closure in
section 6 (norm N12).

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 / R2 / R4 (N9) | independent reviewer agent (FRICK-REVIEW-0035) | 2026-10-10 | APPROVED WITH COMMENTS (per profile; R2 escalates to REJECTED if the advisory proxy is claimed as verification) |

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | Minor | `emulated-target.md`, `emulated_run.sh`, SRS-REQ-043 | QEMU doc BSP `rv64imafdc` vs script builds `mpfs64imafdc` | **closed**: the generic `riscv/rv64imafdc` BSP is now built for QEMU (`make qemu-image`, `FF_PLATFORM=rtems-generic`) and actually run on QEMU `virt` | `make qemu` PASS (`platform : qemu-virt-rtems`); `docs/vv/verification-results.md` M10 row; FRICK-REVIEW-0120/0123 | frick-fsw | **closed** | FRICK-REVIEW-0123 (PA) |
| R1-02 | Comment | `emulated-target.md:23` | E51 ISA and QSPI/MT25Q claims inaccurate | **closed**: per-core ISA corrected (E51 `rv64imac_zicsr_zifencei` no F/D; U54 `rv64imafdc`); QSPI/MT25Q attributed to the `beaglev-fire` board repl | `docs/vv/emulated-target.md` rev 0.3 | frick-fsw | **closed** | FRICK-REVIEW-0122 (R4) |
| R1-03 | Comment | `emulated_run.sh:38-45` | Renode banner not captured (no `showAnalyzer`) | **closed**: `showAnalyzer sysbus.mmuart1` added to the advisory Renode leg (the console is MMUART1) | `tools/scripts/emulated_run.sh` | frick-fsw | **closed** | FRICK-REVIEW-0120 (R1) |
| R2-01 | Minor | SRS-REQ-043, `emulated_run.sh`, `traceability.yaml` | Evidence non-falsifiable / over-claims drivers+app registration | **closed**: a binding QEMU check (`tools/scripts/qemu_vv.sh`, T20) asserts the app count and can FAIL; the advisory `emulated_run.sh` disclosure kept; the QEMU driver scope explicitly excluded | `make qemu`; `docs/vv/emulated-target.md` §5; nightly workflow | frick-fsw | **closed** | FRICK-REVIEW-0121 (R2) |
| R2-02 | Comment | CI + manifest + `traceability.md:95` | `ci:emulated-run` not labelled proxy vs `ci:rtems-cross-build` | **closed**: the advisory/proxy wording is retained for the CI step; the binding QEMU check is documented separately (nightly, T20) | `docs/requirements/traceability.md`; `tool-qualification.md` | frick-fsw | **closed** | FRICK-REVIEW-0121 (R2) |
| R4-01 | Minor | `traceability.md:38` | Top-down matrix omits SRS-REQ-043 | **closed**: SSS-REQ-013 row lists SRS-REQ-043 | `docs/requirements/traceability.md:42` | frick-fsw | **closed** | prior cycle |
| R4-02 | Minor | `traceability.md:18-20` | CI-claim count stale (10/9 vs 11/10) | **closed**: counts now 12 unique / 11 referenced | `traceability.md:18` | frick-fsw | **closed** | prior cycle |
| R4-03 | Minor | `README.md:40-78` | Generated metrics panel stale | **closed**: `metrics.py --readme` regenerated every cycle | `README.md` panel; `metrics.py --check` | frick-fsw | **closed** | prior cycle |
| R4-04 | Minor | `verification-results.md` | No M10 row; counts stale | **closed**: M10 cycle row added (rev 0.32) | `docs/vv/verification-results.md` | frick-fsw | **closed** | FRICK-REVIEW-0122 (R4) |
| R4-05 | Minor | `tool-qualification.md:54-65` | T15/T16 absent from evidence-claims mapping | **closed**: mapping extended (T15/T16/T20) | `docs/vv/tool-qualification.md` | frick-fsw | **closed** | FRICK-REVIEW-0122 (R4) |
| R4-06 | Comment | `traceability.md:104-125` | SSS-REQ-021 missing from SSS summary | **closed**: SSS-REQ-021 row present | `traceability.md:150` | frick-fsw | **closed** | prior cycle |
| R4-07 | Comment | SRS-REQ-043 parentage | Parent SSS-REQ-013 looser than its text | **closed**: parentage justified in the manifest note | `traceability.yaml` SRS-REQ-043 `lean_note` | frick-fsw | **closed** | FRICK-REVIEW-0122 (R4) |
| R4-08 | Comment | `verification-plan.md` | Emulated path not in V&V plan/environment | **closed**: environment row + "Emulated target (Dem, M10)" activity added | `docs/vv/verification-plan.md` rev 0.1.3 | frick-fsw | **closed** | FRICK-REVIEW-0122 (R4) |
| R4-09 | Comment | touched files | Missing trailing newlines | **closed**: trailing newlines added (incl. `sprint-plan.md`) | `git diff --check` | frick-fsw | **closed** | FRICK-REVIEW-0122 (R4) |

Closed in the M10 closure increment (2026-10-12) with the independent reviews
FRICK-REVIEW-0120..0123 (norm N12).

---

Committed and pushed with the reviewed change (norm N11).
