# Review Report — Delta re-review (N12) of the M10 QEMU fixes (FRICK-REVIEW-0120 NCs)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0124                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (SW development)                    |
| Reference revision                | base commit `aee4135` + M10 closure increment (uncommitted working tree, 2026-10-12) |
| Items under review                | Deltas that close FRICK-REVIEW-0120 R1-01..R1-04: `config/platform/platform_qemu.h` (new), `config/fsw_config.h`, `CMakeLists.txt`, `src/bsp/mpfs/board.c`, `src/core/pal/pal_rtems.c`, `src/core/pal/pal_posix.c`, `src/transport/link/link.c`, `tools/scripts/qemu_vv.sh`; and the dependent `tools/scripts/emulated_run.sh`, `tools/docker/rtems/Dockerfile`, `tools/scripts/setup_rtems.sh`, `Makefile`, `.github/workflows/nightly.yml`, `docs/vv/tool-qualification.md`, `docs/vv/emulated-target.md`, `docs/vv/verification-results.md`, `docs/status/loop-audit.md`, `docs/status/status.json`, `docs/requirements/traceability.{yaml,md}` |
| Date                              | 2026-10-12                             |
| Scope                             | Delta re-review (norm N12) confirming R1-01..R1-04 closure: the generic-platform parameterization (no MPFS leakage; MPFS/Renode build still correct), the toolchain/Dockerfile change, the `qemu_vv.sh`/`emulated_run.sh`/Makefile/nightly changes, evidence-mapping corrections and the docs/status deltas; plus new-defect screening. ECSS-E-ST-40C, ECSS-Q-ST-80C. |
| Independence declaration          | The reviewer took no part in authoring, implementing or executing the increment; fresh independent session, no author context. **No tracked file was modified other than this report.** Docker was not run; `make qemu`/`make renode`/Renode/QEMU were **not** re-executed (host read-only commands only, per the task constraints). |
| Norms applied                     | N1-N20 (see `docs/rules.md`)           |

## 1. Scope

This is the **delta re-review (norm N12)** of the fixes addressing the four NCs
of the R1 M10 review **FRICK-REVIEW-0120** (`APPROVED WITH COMMENTS`). It also
touches the adjacent sibling NCs the same increment claims to close (R2-01,
R4-01..R4-09, PA-02..PA-10), but only where they intersect the R1 items.

**In scope:** correctness and completeness of the R1-01..R1-04 dispositions; the
generic-platform change and whether MPFS assumptions still leak into the
"generic" QEMU image/banner; that the MPFS/Renode (`FF_PLATFORM=rtems`) build
path is unchanged; the script/Makefile/nightly/Dockerfile changes; honesty of the
resulting evidence and register wording. **Out of scope / not re-run:** the actual
`make qemu`/`make renode` executions, `docker build`, Renode, and real silicon
(not available/prohibited). N18 (Renode) is unchanged by this increment and was
verified in the prior cycle.

### Frozen baseline (hashes verified read-only on the host, 2026-10-12)

| Path | Frozen sha256 | Verified |
| ---- | ------------- | -------- |
| `config/platform/platform_qemu.h` | `d9c57217f8d9c321b585946531f3944eb78efcccf59aa11f2e90385f5b583c27` | match |
| `config/fsw_config.h` | `612c1e766d3b5dc8127d55056e8ebcd2538cfa91a8d7d4fdc26c36d399fd80aa` | match |
| `CMakeLists.txt` | `315c58917c1f070ed525725ab347676c2982ce5688b5f1240c8da790f99a073b` | match |
| `src/bsp/mpfs/board.c` | `8ba75d36fde97750d038129c32627794ea8b9646379b41f85ec3306445fe6a44` | match |
| `src/core/pal/pal_rtems.c` | `5e44043e86eb735ba4b24836c9012daec9fdbabeb4af1f4fbd128a9fe97be406` | match |
| `src/core/pal/pal_posix.c` | `24dea1c736ee8eb3bb7df7188c0c63343e890f21bb12f044da5f0662a4979835` | match |
| `src/transport/link/link.c` | `6c1e56bd1b20e49e9d1ff40da7e421f69a29fc34bdd40272bb554b6a9c71f62a` | match |
| `tools/scripts/qemu_vv.sh` | `b67a6b12f30346b0dfa78765584a36d80745b76ec22dc7eea058c28a08a8594f` | match |

`git rev-parse HEAD` = `aee4135a68838def64663c4d4fa1267783470edb`; the increment
is an uncommitted working-tree delta on top of `aee4135` (single coherent delta).

## 2. References

- Project norms: `docs/rules.md` (N1-N20), especially N8, N9, N10, N11, N12, N15, N16, N18, N19.
- Predecessor: `docs/reviews/2026-10-12-m10-r1-review.md` (FRICK-REVIEW-0120).
- Sibling reports (same increment): `2026-10-12-m10-r2-review.md` (0121, REJECTED
  R2-01), `2026-10-12-m10-r4-review.md` (0122), `2026-10-12-m10-pa-review.md`
  (0123, REJECTED PA-01/PA-03); predecessor `2026-10-10-m10-emulated-r1-r2-r4-review.md`
  (FRICK-REVIEW-0035).
- Traceability manifest: `docs/requirements/traceability.yaml` (SRS-REQ-043).
- ECSS-E-ST-40C, ECSS-Q-ST-80C; RTEMS 6.2 `riscv/rv64imafdc` BSP; QEMU `riscv64 virt`.

## 3. Delta closure table (R1-01..R1-04)

| NC | Severity | Disposition | Independent verification (this session) | Status |
| -- | -------- | ----------- | --------------------------------------- | ------ |
| **R1-01** | Minor | **CLOSED** — platform parameterized; the generic QEMU build no longer claims MPFS hardware. | `config/platform/platform_qemu.h` exists (`FF_PLATFORM_NAME "qemu-virt-rtems"`, `FF_BOARD_NAME "QEMU virt (generic rv64imafdc RTEMS)"`, no MPFS MMIO bases); `config/fsw_config.h:14-20` selects it via `FRICK_PLATFORM_QEMU`; `CMakeLists.txt:36-42` defines it for `FF_PLATFORM=rtems-generic`; `board.c:39-43`, `pal_rtems.c:18`, `pal_posix.c:104-105`, `link.c:14` extended to the QEMU platform. Retained console `build/rtems-generic/qemu-console.log` now reads `platform : qemu-virt-rtems` and `board: QEMU virt (generic rv64imafdc RTEMS) booting (generic RTEMS BSP)` — no HSS/U54 fiction. `build/rtems-generic/build.ninja` defines only `FRICK_PLATFORM_QEMU`. (Residual observation D-04 below.) | **closed** |
| **R1-02** | Minor | **CLOSED (with residual, D-02)** — the clean `docker build` was not completed; the increment instead records the provenance caveat in the tool register (the alternative the PA review permitted). | `tools/docker/rtems/Dockerfile:21-22,41-43,51-53` adds the `riscv/rv64imafdc` kernel-bset build + `qemu-system-misc`/`cmake`/`ninja-build`; `docs/vv/tool-qualification.md:59-66` records the "assembled by applying the steps individually / clean `docker build` not completed locally" caveat and downgrades T18 to `QUALIFIED (see note)`. The nightly workflow (`setup_rtems.sh` + `make qemu-image` + `make qemu`) now exercises the QEMU path. Residual: the caveat is not owner-assigned/tracked and `docs/tooling.md` still says "Reproducible" (D-02). | **closed (residual Minor)** |
| **R1-03** | Comment | **CLOSED** — the check is now falsifiable and provenance is recorded. | `qemu_vv.sh:29,56-60` asserts the exact string `6 applications registered` and FAILs (exit 1) when the ELF is present but the marker is missing; `qemu_vv.sh:39-41` records the toolchain image id into the log/console; `emulated_run.sh:37` adds `showAnalyzer sysbus.mmuart1` to the advisory Renode leg. `bash -n` clean on both. Residual (Comment D-05): the "binding" wording and the 10 s vs 20 s timeout mismatch remain. | **closed** |
| **R1-04** | Comment | **CLOSED** — the evidence mapping is corrected and the stale loop-audit text removed. | `docs/vv/tool-qualification.md:44-45,82` maps T20 to the **nightly** QEMU job; `.github/workflows/nightly.yml:57,63-70` builds the generic image and runs `make qemu` with a `grep`; `docs/status/loop-audit.md:33,56,124,140` now say M10 **closed**, registry `T1-T20`, QEMU gap closed. | **closed** |

### R1-01..R1-04: all four substantively closed; residuals are new Minor/Comment items, not re-opened NCs.

## 4. Findings (new / residual)

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| D-01 | **Minor** | N8 — platform/build docs not updated with the change | The new generic platform and `FF_PLATFORM=rtems-generic` are absent from the architecture/build docs. `docs/architecture/platform.md` (rev 0.3) lists only `native` and `rtpfs160zt`, cites only `platform_native.h`/`platform_mpfs.h`, and does not mention `platform_qemu.h` or the QEMU platform; `docs/engineering/driver-development.md:133` still says platform selection is only `-DFF_PLATFORM=rtems` (sets `FRICK_PLATFORM_MPFS`) or the default `native`. N8 requires the affected docs to be updated in the same change; the increment touched the platform selection but not these docs. | `docs/architecture/platform.md:20-25` (unmodified by the increment); `docs/engineering/driver-development.md:132-134`; `CMakeLists.txt:27,38-42`; `config/fsw_config.h:16-17`; `git status` (neither file modified) | Add a `qemu-virt-rtems` / `rtems-generic` row (and `platform_qemu.h`) to `platform.md`; add `rtems-generic` to the `driver-development.md` platform-selection bullet. (Primarily R4 scope; flagged for cross-consistency.) |
| D-02 | **Minor** | N8/N10/N16/N19 — T18 caveat not owner-assigned/tracked; "reproducible" wording not aligned | The T18 reproducibility caveat is disclosed in `tool-qualification.md:59-66` (good, closes the "undisclosed" concern), but the PA-02 condition — an **owner-assigned, tracked** limitation with the "reproducible" wording aligned — is only partially met: the note names no owner and appears in no tracked register row (`loop-audit.md` §5, `class-a-gaps.md`); `docs/tooling.md:72` still labels the T18 image "**Reproducible** open-source RTEMS 6.2 toolchain + BSP image" and `docs/tooling.md:67` still lists only `riscv/mpfs64imafdc` (no `rv64imafdc`/QEMU). | `docs/vv/tool-qualification.md:59-66`; `docs/tooling.md:67,72`; `docs/status/loop-audit.md` §5 (no T18 row); `grep` for the caveat/owner finds only the register note | Record the caveat as an owner-assigned row (loop-audit §5 dispositions and/or `class-a-gaps.md`) with a closure action, and align `docs/tooling.md:67,72` (drop/qualify "Reproducible", add the `rv64imafdc` BSP). |
| D-03 | Comment | N10/N4 — the nightly `# evidence:` marker is not consumed by the gate | `tools/scripts/trace.py:36` defines `WORKFLOWS = [".github/workflows/ci.yml", ".gitea/workflows/ci.yml"]`; it does not scan `.github/workflows/nightly.yml`. The new `# evidence: ci:emulated-run ...` marker added to `nightly.yml:57` is therefore **inert** for the trace gate (the `ci:emulated-run` claim is still satisfied because it is also declared in `ci.yml:76`). The register's manual mapping (T20 → nightly) is honest, but the marker creates a false impression that the gate tracks the nightly exercise. | `tools/scripts/trace.py:36,160-184`; `.github/workflows/nightly.yml:57`; `.github/workflows/ci.yml:76`; `trace.py` output `CI evidence claims: 12` | Either add `nightly.yml` to `WORKFLOWS` (so the nightly claim is actually validated) or annotate that the nightly marker is documentation-only. |
| D-04 | Comment | R1-01 residual — dormant MPFS driver TUs in the "generic" image | The generic QEMU build still **compiles and links** the MSS MPFS driver object files (`ff_gem.c`, `ff_gpio_mpfs.c`, `ff_serial_mpfs.c`, `ff_nvm_mpfs.c`, `ff_mtd_mpfs.c`, `ff_can_mpfs.c`) because `FF_BUILD_DRIVERS` defaults ON and `CMakeLists.txt:126-136` adds them unconditionally, regardless of platform. They are **dormant** (not instantiated at boot; only `FF_BoardInit` logs) and use a bus-function-pointer abstraction, so no MPFS MMIO is accessed and the `platform_qemu.h` "no MPFS MMIO / genuinely generic" claim holds at the MMIO/config level. Still, the image's translation-unit set is not restricted to the generic platform. | `build/rtems-generic/build.ninja` (MPFS driver TUs present); `CMakeLists.txt:126-136`; `config/platform/platform_qemu.h:5-9,34`; console shows no driver init | Optional: gate the `src/drivers/*/mpfs` sources on the platform (or `-DFF_BUILD_DRIVERS=OFF` for the QEMU build), or state in `emulated-target.md` that the generic image links dormant MPFS driver TUs. No impact on the boot/registration evidence. |
| D-05 | Comment | R1-03 residual — "binding" wording / timeout / FAIL-message conflation | `docs/vv/emulated-target.md:52` still calls `qemu_vv.sh` "the **binding** QEMU check" (though lines 57-58 now explain SKIP-vs-FAIL); `emulated_run.sh:56` uses `timeout 10` for the host QEMU fallback while `qemu_vv.sh:43,46` uses `timeout 20`; `qemu_vv.sh:60` still reports one FAIL message for both "QEMU failed to launch" and "banner missing". | `docs/vv/emulated-target.md:52-58`; `tools/scripts/emulated_run.sh:56`; `tools/scripts/qemu_vv.sh:43,46,60` | Word it as "binding when the image is present, SKIP otherwise"; align the two timeouts; distinguish launch failure from missing-banner. Optional. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

## 5. Positive evidence

- **Frozen-baseline integrity:** all eight frozen files match the declared
  sha256; the increment is a single coherent delta on `aee513`… `aee4135`; no
  tracked file other than this report was modified by the reviewer.
- **Generic platform is genuinely generic at config/MMIO level:** the QEMU build
  selects `FRICK_PLATFORM_QEMU` only (`build.ninja`); `platform_qemu.h` carries no
  MPFS MMIO base; the console banner is environment-honest
  (`platform : qemu-virt-rtems`, `board: QEMU virt (generic rv64imafdc RTEMS)`).
- **MPFS/Renode path unchanged and correct:** `FF_PLATFORM=rtems` still defines
  `FRICK_PLATFORM_MPFS` (`CMakeLists.txt:36-37`); `board.c` still prints the HSS
  message under `FRICK_PLATFORM_MPFS`; `pal_rtems.c:18`, `pal_posix.c:104`,
  `link.c:14` all keep MPFS in the RTEMS branch; `platform_mpfs.h` is unmodified.
  No regression of the Renode/`make rtems` build is visible in the delta.
- **Script mechanics:** `qemu_vv.sh` bounds QEMU with `timeout` and asserts the
  exact `6 applications registered` count and can FAIL; provenance (image id) is
  recorded; `bash -n` clean on both scripts.
- **Gate re-execution (host, this session):** `ctest --test-dir build/native`
  **19/19 PASS**; `trace.py` **PASS (99/58/27/14/51; 12 CI claims)**;
  `metrics.py --check` → `metrics panel: current`; `git diff --check` clean
  (trailing-newline NC closed); the four sibling reports and 0124 are logged in
  `docs/reviews/README.md` and the FRICK-REVIEW-0035 follow-up table is closed.

## 6. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**Verdict: APPROVED WITH COMMENTS.** All four FRICK-REVIEW-0120 NCs are
substantively closed: the generic QEMU platform is now genuinely generic in its
identity and configuration (R1-01), the toolchain caveat is disclosed and the
QEMU check is exercised by the nightly workflow (R1-02), the binding check asserts
the app count and records provenance (R1-03), and the evidence mapping /
loop-audit stale text is corrected (R1-04). The MPFS/Renode build path is
unchanged. No Major finding and no defect that defeats SRS-REQ-043 were found.
Two new **Minor** findings (D-01 platform docs; D-02 T18 caveat tracking/wording)
and three **Comments** (D-03 nightly marker not scanned; D-04 dormant MPFS driver
TUs; D-05 binding wording/timeouts) are to be dispositioned in the same N12
closure cycle; none blocks the increment once tracked.

## 7. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent reviewer agent (FRICK-REVIEW-0124) | 2026-10-12 | APPROVED WITH COMMENTS (no Major) |

## 8. Follow-up records

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | Minor | generic platform / banner (N8); SRS-REQ-043 | MPFS platform/identity leaked into the "generic" QEMU image | Fixed: `platform_qemu.h` + `FRICK_PLATFORM_QEMU`; banner now `qemu-virt-rtems` | §3; console log; FRICK-REVIEW-0124 | frick-fsw | **closed** | FRICK-REVIEW-0124 |
| R1-02 | Minor | `tools/docker/rtems/Dockerfile`; T18 register/evidence (N10) | T20/QEMU evidence not from a clean Dockerfile build | Caveat disclosed (clean build tracked); residual D-02 | §3; `tool-qualification.md:59-66` | frick-fsw | **closed (D-02 tracked)** | FRICK-REVIEW-0124 |
| R1-03 | Comment | `qemu_vv.sh`; `emulated-target.md` | Non-falsifiable; FAIL conflates launch/banner; timeout mismatch | Count assertion + FAIL + image id + Renode analyzer added; residual D-05 | §3 | frick-fsw | **closed (D-05 tracked)** | FRICK-REVIEW-0124 |
| R1-04 | Comment | CI mapping; `loop-audit.md` | `ci:emulated-run` ↔ T20 loose; stale "QEMU pending" | Mapping corrected (nightly); loop-audit refreshed | §3; `tool-qualification.md:44-45`; `loop-audit.md:33,124,140` | frick-fsw | **closed** | FRICK-REVIEW-0124 |
| D-01 | Minor | `docs/architecture/platform.md`; `docs/engineering/driver-development.md` (N8) | New QEMU platform / `platform_qemu.h` absent from platform docs | open | — | frick-fsw | open | — |
| D-02 | Minor | `docs/tooling.md`; T18 caveat tracking (N8/N10/N19) | T18 caveat not owner-assigned/tracked; "Reproducible" wording not aligned | open | — | frick-fsw | open | — |
| D-03 | Comment | `tools/scripts/trace.py` `WORKFLOWS` (N10/N4) | nightly `# evidence:` marker not scanned by the trace gate | open | — | frick-fsw | open | — |
| D-04 | Comment | generic build source set (R1-01 residual) | Dormant MSS MPFS driver TUs linked into the generic image | open | — | frick-fsw | open | — |
| D-05 | Comment | `emulated-target.md` / `emulated_run.sh` (R1-03 residual) | "binding" wording, timeout mismatch, FAIL-message conflation | open | — | frick-fsw | open | — |

---

## Evidence consulted

- Frozen artifacts (hashes verified, §1 table): `config/platform/platform_qemu.h`,
  `config/fsw_config.h`, `CMakeLists.txt`, `src/bsp/mpfs/board.c`,
  `src/core/pal/pal_rtems.c`, `src/core/pal/pal_posix.c`,
  `src/transport/link/link.c`, `tools/scripts/qemu_vv.sh`.
- Read: `tools/scripts/emulated_run.sh`, `tools/docker/rtems/Dockerfile`,
  `tools/scripts/setup_rtems.sh`, `Makefile`, `.github/workflows/nightly.yml`,
  `.github/workflows/ci.yml`, `config/platform/platform_mpfs.h`,
  `cmake/toolchains/rtems-riscv-generic.cmake`, `docs/vv/tool-qualification.md`,
  `docs/vv/emulated-target.md`, `docs/vv/verification-results.md`,
  `docs/vv/verification-plan.md`, `docs/status/loop-audit.md`,
  `docs/status/status.json`, `docs/status/handover.md`,
  `docs/requirements/traceability.{yaml,md}`, `docs/architecture/platform.md`,
  `docs/engineering/driver-development.md`, `docs/tooling.md`, `README.md`,
  `AGENTS.md`, `CHANGELOG.md`, `docs/reviews/README.md`,
  `docs/reviews/2026-10-12-m10-r1-review.md` (0120),
  `docs/reviews/2026-10-12-m10-r2-review.md` (0121),
  `docs/reviews/2026-10-12-m10-r4-review.md` (0122),
  `docs/reviews/2026-10-12-m10-pa-review.md` (0123),
  `docs/reviews/2026-10-10-m10-emulated-r1-r2-r4-review.md` (0035),
  `tools/scripts/trace.py`, `build/rtems-generic/qemu-console.log`,
  `build/rtems-generic/build.ninja`.
- Executed (read-only, host): `ctest --test-dir build/native` (19/19);
  `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` (PASS,
  12 CI claims); `python3 tools/scripts/metrics.py --check` (current);
  `bash -n` on both scripts; `sha256sum`; `git status`/`git rev-parse`/`git diff`/
  `git diff --check`; `grep`/`readelf`-equivalent ELF inspection.
- Not performed (per task constraints): `docker build`/`make rtems-toolchain`/
  `make qemu`/`make renode`; no Renode/QEMU execution.

---

Committed and pushed with the reviewed change (norm N11).
