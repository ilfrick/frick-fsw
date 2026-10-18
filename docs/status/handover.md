# Handover / Current Context (resume point)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-HANDOVER-001                     |
| Revision                          | 3.06                                   |
| Date                              | 2026-10-18                             |
| Norms                             | N8 (entry point), N16 (history)        |

This document is the **authoritative resume point** for an agent picking up the
work without prior conversation context. It summarises objectives, rules, how to
build/test/verify, milestones and the immediate next steps. It is updated on
every significant increment (norm N8/N16).

## 0. How to resume in a new session

Start a fresh session in the repository root and use a kickoff prompt like:

> "Read `AGENTS.md`, `docs/rules.md` and `docs/status/handover.md`; then continue
> the frick-fsw roadmap from the handover's §5 next steps, respecting norms N1-N22
> (full cycle + independent review + trace + Renode N18). Do not commit/push
> unless you are `aeronautics-space-fsw-engineer` after all gates and full sprint
> scope pass, or the user explicitly delegated that authority."

Everything needed is in the repo (no external state): objectives/roadmap
(`docs/development-plan.md`, `docs/roadmap.md`), rules (`docs/rules.md`), status
and this handover. Verify the tree is clean (`git status`) and the gates are green
before starting a new increment.

## 1. What the project is

A **cFS-inspired flight-software middleware** (not an application): PUS-C TM/TC on
CCSDS Space Packets, POSIX host simulator + Microchip RT PolarFire SoC
RTPFS160ZT (RISC-V) under **open-source RTEMS 6.2**, target ECSS criticality B.
Users write their own applications; `src/apps/*` are reference/demo apps.

Remotes: `origin` (GitHub: ilfrick/frick-fsw) and `gitea`
(housefz.com/git/ilfrick/frick-fsw). Branch: **main only**. Under N7, only
`aeronautics-space-fsw-engineer` commits/pushes after all gates/norms and full
sprint scope pass; exceptions require explicit user delegation. Credentials in
`/home/nicola/.netrc` (+ `gh` helper).

## 2. Non-derogable rules

`docs/rules.md` (N1-N22). Key gates before every commit/push:

1. `make build` + `make test` green (native).
2. `bash tools/scripts/trace.py docs/requirements/traceability.yaml` PASS.
3. Lean: `lake build` in `verification/lean` (host lacks lake; run in the V&V
   image or skip with note).
4. `clang-format --dry-run --Werror` over `src config tests`.
5. **N18**: functional validation in **Renode** must pass — `make renode`
   (blocking when `build/renode/frick_fsw_renode.elf` exists; per-commit SKIP
   proxy otherwise; nightly does the real run).
6. N9/N12: independent review (R1/R2/R3/R4/PA as applicable) + NC closure; reports
   under `docs/reviews/` and logged in `docs/reviews/README.md`.
7. N8/N16: update all affected docs + revision histories.
8. N17: documentation review (R4).
9. **N20**: independent **Senior SW PA** review of every phase product; findings
   fixed and reviews repeated until pass (`docs/engineering/reviewer-agents/`).

## 3. How to build / test / verify (exact commands)

```bash
# Host build + unit tests (default N7 backend) + trace + format
make pus-asn1
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Debug -DFF_USE_PUS_ASN1=ON && cmake --build build/native
ctest --test-dir build/native --output-on-failure
python3 tools/scripts/trace.py docs/requirements/traceability.yaml   # PASS 117/73/30/14/62
docker run --rm -v "$PWD:/workspace" -w /workspace frick-vv:local \
  bash -lc 'find src config tests -name "*.[ch]" -print0 | xargs -0 -n1 clang-format --dry-run --Werror'
```

The canonical cycle is `make ci-local` in `frick-vv:local` (configure/build/CTest/
Lean/trace; test count varies with the candidate). For the AddressSanitizer leg and coverage/metrics regeneration, run
inside the container (`build/asan`, `make coverage`, `make metrics`) so the
panel and the `ci-local` cache agree; the README panel is green only when
`tools/scripts/metrics.py --check` passes after `make coverage` in the image.

# Renode V&V environment self-test (fast)
make renode-env

# Open RTEMS 6.2 toolchain + RMTS image + Renode functional run
make rtems-toolchain            # build open RTEMS toolchain image (long, ~75 min)
make rtems-docker               # cross compile inside the toolchain image
#   (or, if the toolchain image lacks cmake, install at runtime in the container)
make renode-image               # build build/renode/frick_fsw_renode.elf (FF_ENABLE_RENODE_TEST=ON)
make renode                     # -> "[renode] PASS (RENODE_TEST: PASS)"
```

Toolchain image `frick-rtems-toolchain:local` (open RTEMS 6.2, RSB tag 6.2, BSP
`riscv/mpfs64imafdc`); VM image `frick-vv:local` (cmake/ninja/gcc/lean/…); Renode
image `antmicro/renode@sha256:7905c3…`. Container runs as root → after mounted
edits run `chown -R 1000:1000` (no sudo on host). Host lacks cmake-format/lake on
occasion; prefer Docker.

## 4. Current state (latest pushed baseline `1fdbb83`; S8 batch committed)

- **Committed baseline:** latest pushed baseline is `1fdbb83` (S8 reconciliation
  batch: S8-SCH + S8-VV-FU + S8-PUS-FU + selected N7 OBCP integration), pushed to
  `origin` and `gitea`. S8-SCH and S8-VV-FU are delivered on the pushed history.
  The batch's cycle evidence was produced on the pre-commit working tree, so it
  must not be conflated with prior cycle evidence; candidate verification and
  generated-input identity are recorded in `docs/vv/verification-results.md`.
- **S8-SCH delivered:** `ff_sched` table-driven dispatcher (`ff_sched.{c,h}`,
  `app_scheduler.c`) with validation, atomic double-buffer activation, absolute
  per-entry release targets, bounded per-poll work, missed-slot/heartbeat/
  publish-error stats; `test_sched` 57 checks. NC-EXT-19 functionally corrected;
  remains OPEN pending independent R4/PA review.
- **S8-VV-FU implemented/committed:** `test_es_hs_lifecycle.c` (ES/HS lifecycle +
  FAILED-application detection host integration with an EVS-listener oracle) and
  `test_decoder_corpus.c` (bounded random-byte/length-boundary/single-bit-
  mutation/cross-type/round-trip/`FF_PusIngest` corpus with requirement-grounded
  oracles). GAP-R3-1 and NC-EXT-25 materially closed; both remain OPEN pending the
  independent R2/R3 delta review (owner disposition §3.2 is tracking only).
- **Batch reviews 0341–0343 completed; delta closure is in progress.** They
  covered S8-SCH + S8-VV-FU + S8-PUS-FU + selected N7 OBCP integration. Remaining
  controlled-profile/interface work and focused delta reviews mean the sprint is
  not complete. The pinned N7 engine is integrated into the
  default core build with host callback-connectivity evidence, but production
  mission service/PUS S18 binding and M12 completion are not claimed. NC-S9-01
  remains deferred/open. See `docs/architecture/obcp-integration-assessment.md`.
 - **Review batch 0338–0340:** R4 found a stale current-cycle count; PA identified
   candidate-identity and generated-source provenance gaps; V&V found missing
   OBCP READY and post-final-assertion oracles. The test/trace and candidate
   product-set identity findings were addressed in subsequent bounded reviews.
   NC-EXT-84 remains open for generated-source/tool provenance.
 - **Full S8 batch reviews 0341–0347:** R4 identified status.json, current-cycle,
  and S8-SCH plan-status reconciliation NCs; PA reaffirmed NC-EXT-83/84; V&V
  found missing S20 zero/exact-capacity oracles and an overstatement that the
  decoder corpus CRC check was independent. The S20 cases and CRC wording are
  corrected in the working tree. Post-0343 CI-local and applicable auxiliary
  verification legs have passed; see the candidate manifest and evidence logs.
   PA 0345 accepts product-set identity for the manifest's 23 enumerated files
   and local results only (NC-EXT-83 narrow closure). PA 0346 verifies the 221-file
   generated tree identity but not tool provenance; NC-EXT-84 remains open.
   R4 0347 closes NC-EXT-85 only for current-status reconciliation. No phase or
   release approval follows.
 - **S8-PUS-FU R1/R2 findings 0349/0350:** R1 found S3 report-all could replace
  an earlier failure with a later `FF_NOT_FOUND`; code now retains the first
  failure and TS1-102 exercises both error orders. R2 identified missing exact
  capacity+1, HK encoded-length, duplicate-retention, and SRS-REQ-050 trace
  evidence; bounded test/spec/trace corrections are in the working tree. The
   focused `pus_fu` test passes after retained compile/oracle failures. R1/R2
   delta re-reviews confirm the bounded fixes; PA considers its delta acceptable
   conditionally on configuration-matched full-cycle evidence. Current CI-local
   and auxiliary legs pass; PA 0361 verified listed-file identity only. NC-EXT-
   88/89 remain OPEN pending formal dispositions. R4 0357–0359 and 0362–0363
   reviewed the scoped TS1/status/audit/manifest corrections; no aggregate N17
   approval follows. NC-EXT-14/15/16/18 still need controlled inputs.
 - **S8-SCH review delta:** R2 scheduler review found mismatched missed-slot
  inputs and insufficient rejected-image/output oracles. Tests/specs now align
  on a 1000 ms period, assert exact validation statuses, dispatched old-image
  first-entry output and missed-slot message fields. Focused `sched` passed 1/1;
  R2 re-review 0353 accepts these bounded oracles after narrowing the claim to
  the tested entry and fixing duplicate TS6 history numbering. R1 identified
  unchecked `uint64_t` release-target overflow; NC-EXT-90 records the open time
  domain/exhaustion policy. NC-EXT-19 remains OPEN for independent review,
  mission inputs, target timing, and authority decisions.
- **Latest local cycle legs:** final `make ci-local` passed 43/43 + Lean/trace
  after correcting old S3 test vectors and formatting (verification-results
  2.89; latest post-loop-audit log hash retained there). Smoke, GSW/license, pinned format and
  format-check passed. Containerized docs generation, Renode PASS and RTEMS 6.2
  cross-build passed; Doxygen emitted unresolved-reference/list/entity warnings
  while output generation succeeded. PA independently verified the supplemental
  19-file PUS/Scheduler delta identity and cited log/cache/executable hashes;
  NC-EXT-91 is closed narrowly for that delta only. Broader candidate identity,
  provenance, physical-target and hosted CI evidence remain open.
- **Hosted GitHub CI:** last recorded run `38793643786` passed all eight jobs
  for the earlier baseline `13ebf92`; no hosted CI run covers the pushed
  default-N7 S8 batch (`1fdbb83`).
- **Known limits:** NC-EXT-01/02/03/04/06/07/13/19/25/47/90 and all target/authority/
  phase/safety/security/release gates remain OPEN. Renode is emulation; RTEMS
  cross-build is not target runtime evidence; no physical RTPFS160ZT evidence is
  established (owner-postponed, README-disclosed). No commit/push of the next
  batch increments until the batch review passes and gates clear.
- **Specific sprint inputs still missing:** S8-PUS-FU cannot finish NC-EXT-14/15/16
  (PUS request/verification/subservice oracle) or NC-EXT-18 (S6 address width and
  region-ID contract) by guessing; controlled PUS profile/SRS/ICD and system
  authority dispositions are needed before those implementation decisions.
  Implemented bounded NC corrections remain open pending independent review.
  For OBCP, NC-S9-01 is deferred and is not a stop on unrelated work, but missing
  production bindings and an accepted execution envelope block M12/SRS-REQ-045
  closure and mission use. Batch reviews and product-assurance approval are also
  required before commit under N7/N9/N20.

- **Historical snapshot (2026-10-14):** M1..M7, M8, M10 delivered; M9 in progress; M11(11/13) in progress; M12-M17 planned; S4b delivered. Current M4 S4c/S4d implementation status is given above and in the roadmap.
- **N18 in force**: open RTEMS 6.2 toolchain built and validated; real cross
  build OK; **Renode functional run of the RTEMS image PASS**
  (`RENODE_TEST: PASS`). Boot: U54_1 at entry `0x1000000000`, console `mmuart1`.
- **M10 done (emulated-target verification, OBJ-18):** Renode real run PASS plus a
  **QEMU run** of the generic `riscv/rv64imafdc` RTEMS image
  (`make qemu-image` + `make qemu`; tool T20 `tools/scripts/qemu_vv.sh`; console
  `build/rtems-generic/qemu-console.log`) — banner + 6 applications registered on
  `qemu-system-riscv64 -M virt`. The toolchain image (`tools/docker/rtems`, T18)
  now also builds the `rv64imafdc` BSP and carries QEMU.
- **PUS backend decision:** ADR-0019 records the owner clarification that
  `FF_USE_PUS_ASN1=ON` is the default. ADR-0018 remains applicable to lwIP only.
  Current N7 integration uses generated common type-width definitions and the
  N7/ESA CRC wrapper, but delegates packet framing and PUS field coding to the
  reference codec; full service-codec mapping is not claimed.
- **OBCP integration (S9, working tree):** pinned `n7space/n7s-obcp` engine is
   now built/linked by default behind `src/obcp/ff_obcp.{h,c}`; `make flat`
   generates ignored embedded sources at configure time. The connectivity test
   exercises the engine with test callback doubles across datapool, EVS, output,
   step-control, time and packet modules. It does not establish production
   service bindings, PUS S18, target portability, resource bounds or mission
   suitability. Abort is cooperative at script-defined begin-step callbacks;
   arbitrary Python loops can run indefinitely, and the pinned engine's execute
   return does not propagate interpreter errors. NC-S9-01 is DEFERRED/OPEN;
   SRS-REQ-045 is not claimed complete and mission procedures must not be enabled
   from this evidence. Assessment: `docs/architecture/obcp-integration-assessment.md`.
   Default `make ci-local` passes 43/43 CTest + Lean + trace; `make rtems-docker`
   cross-build, rebuilt-image Renode run (`RENODE_TEST: PASS`) and pinned-container
   format-check, docs generation, smoke and GSW self-check pass. Renode does not
   execute an OBCP script. Evidence and limits are recorded in
   `docs/vv/verification-results.md`.
   S11-S12/M16 in-house replacement-engine development is deferred per owner
   direction; keep SRS-REQ-054 as a separately governed open fallback requirement.
- **M11 in progress (Renode HAL coverage, OBJ-19):** **11/13 peripheral-bound**
  classes done — **GPIO** (`ff_gpio_mpfs`, SRS-REQ-047), **SERIAL**
  (`ff_serial_mpfs`, SRS-REQ-048), **MEM** (`ff_mem_static`, SRS-REQ-049),
  **ETH** (Cadence GEM **identification** via the read-only Module ID,
  SRS-REQ-036), **NVM** (`ff_nvm_mpfs`, SRS-REQ-055; eNVM data write/read check),
  **CAN** (`ff_can_mpfs`, SRS-REQ-057; MMIO register round trip), **I2C**
  (`ff_i2c_mpfs`, SRS-REQ-058; MMIO register round trip), **SPI**
  (`ff_spi_mpfs`, SRS-REQ-069; MMIO register round trip) and **BLOCK**
  (`ff_block_mpfs`, SRS-REQ-070; MMIO register round trip), **TSN** (GEM
  native TSN backend on the emulated Cadence GEM, SRS-REQ-038; init + VLAN/PCP +
  802.1AS TSU time) and **PCIe** (MPFS PCIe root complex, SRS-REQ-031/032; ECAM
  config-space round-trip on the emulated Cadence GEM, `hal_pcie=PASS`). Additionally
  a **NET `FF_NetStack_t` contract check** through the reference loopback stack
  (SRS-REQ-039; in-process — **not** peripheral-bound).
  GPIO/SERIAL/MEM/NVM/CAN/I2C/SPI/BLOCK go through their HAL backends; the ETH check is an MMIO
  identification (full GEM driver/DMA path host-tested in `test_gem` + the new `test_lwip_netif` mock-MAC loopback for the TX/RX data path). Renode
  checks: `hal_gpio`/`hal_serial`/`hal_mem`/`hal_eth`/`hal_net`/`hal_nvm`/`hal_can`/`hal_i2c`/`hal_spi`/`hal_block`/`hal_tsn`/`hal_pcie=PASS`; on the **lwIP-enabled image** also `hal_net_lwip=PASS` (lwIP netif bound to the emulated GEM; full GEM DMA round-trip = GAP-A5).
  M11 peripheral classes: NET(packet round-trip = NC-S6-01 residual, mock-MAC host test) and SpaceWire (contract-level, NC-S7-01).
- **Project codename "Eärendil"**: third-party-name (Tolkien) non-affiliation
  disclaimer in `NOTICE`, `README.md`, `docs/quality/licensing.md §6`.
- **M4-residual MSS drivers delivered so far**: GEM Ethernet (+TSN), MSS GPIO,
  MSS MMUART, the static RAM pool/DMA helper, the eNVM NVM backend, the NOR/QSPI
  MTD backend, the MSS CAN, MSS I2C, MSS SPI and MSS MMC/SD (BLOCK) drivers, plus
  PCIe and the SpaceWire contract-level backend (S7). **Restored M4 scope** (S4b/S4c/S4d):
  PUS-C over IP/TSN (SRS-REQ-059/060/061), CAN-FD/CANopen/RS-485 DE
  (SRS-REQ-062/063/064), PUSopen(R) adapter (SRS-REQ-065), fabric contract
  (SRS-REQ-067); M8 gPTP/CoreTSN (SRS-REQ-068) + GEM TSN Qav/Qci/Qbb
  (SRS-REQ-066). **S4b delivered** (see below).
- **S4b delivered (PUS-C over IP/TSN, SRS-REQ-059/060/061):**
  - segmentation/reassembly layer `link_seg.c/h` (2-byte BE length-prefix,
    allocation-free);
  - IP `FF_Link_t` backend `link_ip.c/h` (UDP default + TCP with seg) over
    `FF_NetStack_t`;
  - TSN `FF_Link_t` backend `link_tsn.c/h` (EtherType 0x88F5, callback-driven RX
    ring) over `FF_EthMac_t + FF_TsnBackend_t`.
  - ctest 27/27 green (test_link_ip **20** test functions, test_link_tsn **15**
    test functions, incl. the `test_pus_ip_roundtrip` / `test_pus_tsn_roundtrip`
    PUS-C TM/TC round-trip demonstrations); Lean PASS; smoke + GSW PASS; Renode
    SKIP (host-only code).
  - `-Wall -Wextra -Werror` is enforced on every test target
    (`tests/CMakeLists.txt`, `FF_TEST_TARGETS`) and all `tests/unit/*.c` compile
    clean under it (N9/M9 quality gate). The host IP test drives a mock
    `FF_NetStack_t`; the reference `FF_NetStackLoopback` is a socket pair (it
    models a peer link, not a single-link self-loop), so the UDP/TCP behaviour is
    established at the `FF_NetStack_t` seam. `link.h` declares the new backends.
    Scope: on-target link-level = GAP-A5 (no Renode image change).
  - Delta re-review: R1+R2 (FRICK-REVIEW-0165-D1) + PA (FRICK-REVIEW-0166-D1)
    both APPROVED; all findings from initial REJECTED reviews resolved (2 Major,
    10 Minor, 6 V&V gaps, 4 PA Minor, 3 Comments + NF-1/NF-2/NF-3 closed).
  - **M8** gPTP (802.1AS) and Qci are delivered (S8); fabric CoreTSN is deferred to
    **M14** (SRS-REQ-068). The repository owner authorized MPL-2.0 for original
    project content; S8-FU addresses remaining corrective work. The owner's
    general instruction to resume with the next sprint as planned is interpreted
    here as selecting S8-FU ahead of competing driver-first sequencing; it did
     not explicitly name S8-FU. R4 review 0311 accepted this bounded
     interpretation; NC-EXT-30/65 are closed for this planning wording issue
     only. The later M12/M16/M13/M9/M14/M15 sequence is recorded in
     `docs/planning/sprint-plan.md` rev 1.27, subject to applicable gates.
- Requirements: **117 (73 srs / 30 sss / 14 gap)**; current `make trace`
  run found/checked 62 Lean theorems.
- Recent reviews: FRICK-REVIEW-0040 (real RTEMS+Renode), 0041 (M12/M13 plan),
  0042 (GPIO), 0043 (SERIAL), 0044 (MEM), 0045 (PUS-C ASN.1), 0046 (README/docs),
  0101..0105 (NVM/MTD + board repl), **0106..0119** (M4/M11 MSS CAN: R1/R2/R4
  initial + delta + closure reviews and the PA freeze sign-off), **0120..0128**
  (M10 QEMU closure: R1/R2/R4/PA initial, delta and final closure),
  **0129..0136** (Sprint-3 M4/M11 MSS I2C: R1/R2/R3/R4/PA initial + R1/R2 delta +
  PA delta; PA converted REJECTED -> APPROVED WITH COMMENTS, loop closed) and
  **FRICK-REVIEW-0148..0161** (S6a GEM TSN, S6b-1 lwIP RTEMS port, S6b-2 lwIP
  netif on GEM, S7 PCIe + SpaceWire: all review/closure sets completed and closed),
  **FRICK-REVIEW-0162** (NET follow-up: NC-S6-03 co-implemented + NC-154-01
  closed — APPROVED WITH COMMENTS; all 8 Minor findings corrected) and
  **FRICK-REVIEW-0163** (NET follow-up delta re-review — APPROVED; all
  corrections verified) — all closed (see `docs/reviews/README.md`).
- **FRICK-REVIEW-0159** (FSW cross-check of `docs/engineering/user-integration-manual.md`
  rev 0.6 — REJECTED) and **FRICK-REVIEW-0160** (R4 documentation review of the
  same manual — REJECTED): both reviewers found 1 Blocker + 7 Major + 9 Minor
  findings (34 NCs). The corrective manual review FRICK-REVIEW-0164 is recorded
  as completed in the review log; it is not a pending next step. NCs logged in
  `docs/nc/2026-10-13-integration-manual-fix-nc.md`. **FRICK-REVIEW-0165** (S4b
  R1+R2, initial — REJECTED, 2 Major + 10 Minor + 6 V&V gaps) and
  **FRICK-REVIEW-0166** (S4b PA, initial — REJECTED, same 2 Major + 10 Minor +
  6 V&V gaps + 4 PA Minor) both closed: delta re-reviews **FRICK-REVIEW-0165-D1**
  (APPROVED) and **FRICK-REVIEW-0166-D1** (APPROVED) resolve all findings including
  3 new findings (NF-1/NF-2/NF-3) raised and closed in the delta.

## 5. Historical next-step ledger and current resume actions

> Items 1–14 below preserve prior handover decisions and snapshots; items 1–12
> are superseded historical records, not current instructions. For present
> priorities, use item 15 and the live state in §4 plus
> `docs/planning/sprint-plan.md`.

1. The current S4c/S4d/defaults code is committed and pushed; GitHub CI is green
   for `13ebf92`. Gitea Actions status remains unverified, and no release,
   qualification, or flight-readiness conclusion follows from the GitHub run.
2. Keep NC-S4c-01/03/04/05 open as applicable. In particular, the CANopen guard
   has host-test evidence, but needs independent review and integration evidence;
   Renode does not model the CAN-FD fabric peripheral, and no physical-hardware
   result is claimed. PUSopen remains user-supplied; vendor SDK compatibility is
   not established.
3. The owner's instruction to resume with the next sprint as planned is
   interpreted here as selecting **S8-FU**, the first sprint in the proposed
   corrective sequence; the instruction did not explicitly name that sprint.
   Independent R4 review 0311 accepted this bounded interpretation and closed
   NC-EXT-30/65 for this planning comment only; no broader gate is closed.
 4. T23/NC-EXT-82 closure remains narrowly scoped; broader provenance, licensing,
    independent-review, safety/security, authority, and release gates remain
    open. This is not authorization for release, flight, or operational action.
5. S8-FU factors the production receive iteration into
   `FF_AppCmdIngestReceiveOne`, called by `FF_AppCmdIngestMain`; N22-TS1-96 drives
   a maximum-size TC through that exact helper using a controlled link and checks
    the receive capacity and dispatched bytes. V&V review 0316 accepted this
    bounded host evidence; NC-EXT-01/02 remain OPEN pending authorized NC-owner
   recording of that bounded disposition.
   NC-EXT-03 remains open for external ground-segment E2E; no target/hardware
    claim follows from the local Renode run.
6. **Historical 2026-10-15 UART status, superseded by items 8–9:** the first
   host PTY TS2-81 run reported 46 checks and V&V 0319 accepted its enumerated
   PDL behavior. The later R1 correction adds disconnect handling and unique
   test paths (51 checks); R1/R2 and current NC-EXT-07 limits are recorded in
   items 8–9 and `docs/vv/verification-results.md`.
7. R4 review 0323 accepted the bounded TS-2/NC-status documentation corrections;
   PA review 0324 accepted its bounded findings 1/3 corrections. Neither is
   general N17/N20 or phase approval. The candidate source/build/artifact
   identity and retained CTest logs are in
    `docs/vv/evidence/2026-10-16-s8-fu-r1-candidate-manifest.md`; the Oct-15
    manifest is retained as the earlier source snapshot. NC-EXT-29 remains
   open because this is not complete reproducibility/provenance evidence.
8. Independent R1 review 0325 identified a zero-read busy-spin on terminal
   disconnect, ambiguous malformed in-range PDL recovery that can consume a
   following valid TC, and a shared PTY test path. R1 delta 0326 accepted the
   fail-fast zero-read behavior and unique temporary PTY path for bounded host
   scope. The malformed-candidate recovery remains authority-blocked under
   NC-EXT-07; no inner-boundary heuristic was introduced. The user confirmed
   that no approved authority decisions are available and selected to keep
   S8-FU blocked; no policy is inferred. See the sprint register and review
   reports 0325/0326.
9. Independent V&V reviews 0327–0329 accepted the disconnected-PTY host oracle,
   required TS-2 case 81 to be characterization-only (not SRS-REQ-016 coverage),
   and reconciled TS-2/SVA-1 as 48 covered, 32 gaps, 1 characterization. The
   updated SVA-2 interface summary, ICD and test/tool records reflect this. The
   user chose to keep authority-blocked; no controlled recovery/profile/NC-owner
   decisions were supplied. The current code candidate digest and logs are in
   `docs/vv/evidence/2026-10-16-s8-fu-r1-candidate-manifest.md`. NC-EXT-07,
   NC-EXT-01/02 and broader S8-FU remain open; there is no commit/push eligibility.
10. R4 review 0330 and PA review 0331 raised metadata/effectivity/ledger findings.
    Bounded re-reviews 0332 and 0333 accepted their corrections, but explicitly
    require a final full applicable cycle against the current documentation.
    This is not broad N17 or N20 phase approval. Run and retain that cycle, then
    reconcile its result in verification/status records without changing the
    user-directed authority-blocked state.
11. The applicable local cycle was rerun after those corrections: CI-local 37/37,
    Lean/trace, smoke, GSW/license, metrics, docs, formatting, JSON/diff, RTEMS
    cross-build, Renode and focused UART PTY (51/0) passed. Retained logs and
    hashes are referenced by verification-results 2.53 and the current
    candidate manifest. This does not close S8-FU authority/external/target
    items or authorize commit/push.
12. On 2026-10-16 the user decided to postpone physical-target tests and require
    a homepage disclaimer. README now states that no physical RTPFS160ZT runtime,
    timing, or physical UART evidence is established and distinguishes cross-build
    from Renode emulation. This is a deferral, not an NC closure or general
    verification waiver. NC-EXT-07 remains open for recovery policy; NC-EXT-01/02
    owner dispositions, NC-EXT-03 external E2E/profile, and other sprint/release
    gates remain unresolved. Scoped R4/PA reviews 0336/0337 accepted only the
    metadata/effectivity corrections. The applicable local cycle then passed;
    logs/hashes are recorded in verification-results 2.58. This is not target
    evidence or sprint completion. No commit/push is currently eligible.

13. **Historical S8-VV-FU completion snapshot:** baseline is now HEAD
    `b8c3aee` (pushed to `origin` and `gitea`): it adds S8-SCH `ff_sched` and the
    S8-VV-FU ES/HS lifecycle host integration harness + deterministic bounded
    CCSDS/PUS decoder corpus. Local gates are green on `f0e1083` (40/40 CTest
    default + 40/40 ASan, `make trace` PASS 117/62, `make format-check` +
    `git diff --check` clean, `status.json` valid, coverage/README regenerated
    10238/18419 statements). Evidence is under
    `docs/vv/evidence/2026-10-16-s8-vv-fu-{ci-local,asan,trace}.log` and in
    verification-results.md (S8-VV-FU section). GAP-R3-1 and NC-EXT-25 are
    materially closed but remain OPEN pending independent R2/R3 review (owner
    disposition §3.2 is tracking only); NC-EXT-19 functionally corrected, stays
    OPEN pending R4/PA. At that historical snapshot S8-PUS-FU was next. Current
    active status and review/cycle results are in §§4–5 and
    `docs/planning/sprint-plan.md`. No phase/safety/security/release approval is
    inferred.

14. **Historical candidate snapshot (2026-10-16, pre-0343; superseded for current
    status by §4 above and §5 next steps):** uncommitted changes added
    duplicate-registration rejection, provider-output-length checks, visible
    S11-stub status, N7-adapter TC CRC/PDL guards, and HK provider-failure
    propagation. Tests `test_pus_fu`/`test_pus_n7_adapter` and TS-1 cases 97–101
    were added. At that snapshot, `make ci-local` and ASan passed 42/42;
    format-check, smoke, GSW/license checks and docs generation passed.
    Coverage/README refreshed to 10388/18736 statements and 1145/1400 functions.
    A then-current Renode result was from a build configured before the
    default-N7 transition and is retained only as historical evidence. Later
    post-0343 Renode evidence is in the current candidate manifest and remains
    emulation only, not physical hardware or an OBCP procedure oracle. The current
    host adapter test links pinned generated
    common ASN1SCC files and the real N7/ESA CRC wrapper, not a controlled CRC
    double. No mission SDK/profile compatibility is claimed.
    This remains an incomplete S8-PUS-FU increment: C-04..C-06, C-09, C-15/C-16
    and related interface items remain. PUSopen C-12 has controlled-stub APID
    validation evidence but NC-EXT-21 remains OPEN pending independent review and
    commercial SDK compatibility is unverified. NC-EXT-17 has bounded host
    evidence for provider-failure propagation but remains OPEN for broader S3
    mapping and independent review. NC-EXT-20/22/23/24 remain OPEN pending
     independent review/disposition. No commit/push is eligible
     until the sprint scope, applicable gates and batch R4+PA+V&V review pass.

15. **Current resume actions (2026-10-17):**
     - Preserve verified local-cycle evidence. R4 0357–0369 covered earlier
       deltas and do not establish aggregate N17 or phase approval. The four
       reconciliation reviews 0370 (R2 V&V delta), 0371 (R3 requirement
       grounding), 0372 (R4 documentation) and 0373 (PA) were processed on
       2026-10-17: their bounded findings are corrected (TS-1/TS-5/TS-6 §3 gap
       lists and histories, oracle-independence wording, sprint-plan counts and
       references, review-log ledger), `FRICK-ICD-001` rev 1.4 now records the       implemented S3 subservice codes and S6 wire profile as **provisional**
       (ADR-0020), and NC-EXT-93 records the review-log ID collision. Two
       bounded follow-ups are queued: an independent review of the `N22-TS5-77`
       scheduler oracle (PA F-0373-02) and a PA re-verification of the refreshed
       candidate manifest (PA F-0373-04). PA granted no approval; the narrow
       R1/R2 closures are not PA full-cycle acceptance.
     - ADR-0020 and the candidate now record bounded public-reference defaults:
       ECSS-referenced S3 subservice mapping and one-shot list request, a
       logical region-ID/byte-offset S6 profile, and checked scheduler deadline
       exhaustion. These are framework implementation choices only, not
       mission baselines. Obtain controlled PUS profile/SRS/ICD inputs or
       authorized dispositions for NC-EXT-14/15/16/18; keep these NCs open and
       do not enable mission procedures from the provisional choices.
    - Keep NC-EXT-19/90 open pending approved schedule/time-domain/exhaustion
      policy, Table Services operational integration evidence, and applicable
      target timing/fault-response evidence. No schedulability or target runtime
      claim follows from host tests, cross-build, or Renode.
    - Continue only bounded selected-N7 OBCP integration. NC-S9-01, production
      service/PUS S18 bindings, and accepted script execution/resource envelope
      block mission use and SRS-REQ-045 closure. Do not implement a replacement
      engine or claim SRS-REQ-054 complete.
     - Keep NC-EXT-84 provenance, hosted CI, physical-target, full sprint,
       independent-review and release gates open. Do not commit or push.
     - Reconciliation cycle evidence: `make ci-local` 43/43 + Lean (62 theorems)
       + trace PASS, pinned `format-check`, containerized `docs`, `gsw-selfcheck`
       and `smoke` all green on 2026-10-17; logs retained under
       `docs/vv/evidence/2026-10-17-s8-reconciliation-*.log` and the refreshed
       candidate manifest `2026-10-17-s8-reconciliation-candidate-manifest.md`
       supersedes the stale 49/19-file inventories (verification-results 2.90,
       loop-audit 10.7, sprint-plan 1.69). Renode/RTEMS/QEMU legs must be
       re-recorded for this tree before any claim cites them.

### Historical 2026-10-14 status narrative (superseded for current actions by §4/§5)

> Execution view: `docs/planning/sprint-plan.md` (FRICK-PLAN-0002). S1 is complete
> (NVM + NOR/QSPI MTD + MSS CAN done, **M10 closed via the QEMU run**); S2 (MSS CAN)
> done; **S3 (MSS I2C) delivered** (`hal_i2c=PASS`, SRS-REQ-058), **S4 (MSS SPI)
> delivered** (`hal_spi=PASS`, SRS-REQ-069), **S5 (MSS block device) delivered**
> (`hal_block=PASS`, SRS-REQ-070), **S6a (GEM TSN peripheral-bound) delivered**
> (`hal_tsn=PASS`, M11 10/13, ctest 22/22), **S6b-2 (lwIP netif on the GEM;
> `hal_net_lwip=PASS`)** delivered; **S7 (PCIe + SpaceWire) delivered**
> (`hal_pcie=PASS`, `hal_spacewire=CONTRACT`, M11 11/13) and the **NET follow-up
> sprint** done (NC-S6-03 co-implemented: GEM 64-bit `gem_phys`/`gem_wr_qbqp`/
> `TBQP_HI`/`RBQP_HI`/`DMACFG.ADDR64`; NC-154-01 closed: `test_lwip_netif` 26/26
> host mock-MAC loopback; on-Renode GEM DMA still GAP-A5). **S8 (M8) delivered**:
> gPTP stack (`test_gptp` 58 checks host) + 802.1Qci via GEM screener type 1 (`test_gem_tsn`
> 22 checks); 802.1Qav CBS + 802.1Qbb PFC documented `FF_NOT_IMPLEMENTED` (NC-S8-01/NC-S8-02).
> **S4b implementation delivered** (SRS-REQ-059/060/061); the historical delivery
> evidence remains in verification-results.md. Follow-up review reports
> FRICK-REVIEW-0167/0168/0169 are historical APPROVED WITH COMMENTS reports; their
> delta reviews remain pending. FRICK-REVIEW-0221..0228 are NOT APPROVED for
> their reviewed scopes; 0229 is bounded R2 review, 0230 scoped R4 approval, and
> 0231 scoped PA consistency only, and 0232 scoped handover R4 pass. Transition
> audits 0233..0234 and follow-ups 0235..0259 have open findings; R2 0242,
> 0245 and 0249 are bounded results only, not T21 qualification. R4 0243/0244 and
> 0246 is NOT APPROVED; 0247 verifies the DDJF correction only. PA 0248 found a
> fail-open unreadable-file case; R2 0249 bounded review passed the author fix.
> R4 0250/0252/0254 and PA 0251/0253/0255 found active-document scope/status
> inconsistencies; 0252..0257 were NOT APPROVED for their reviewed scopes;
> bounded R4/PA follow-ups 0258/0259 found no new status/history finding but
> close no NC or overall gate. R4 0260 and PA 0261 are bounded N21/N22 reviews;
> the duplicate report ID was resolved by renumbering PA to 0261. NC-EXT-48..68
> are recorded in the sprint plan. R4 0262 reviewed the N22 document set and
> recorded NC-EXT-69..74; no overall N17/PA gate is closed. PA 0263 found the
> SB overflow EVS event was missing; the initial approach that changed all
> `FF_EvsSend` delivery was rejected by R1 follow-up FRICK-REVIEW-0264. The
> current tree restores synchronous ordinary sends and uses explicit
> `FF_EvsSendDeferred` only for SB overflow, dispatched through the bounded
> TM-output task queue. Tests cover normal synchronous send, overflow delivery,
> callback re-entry/dispatch bounds, and saturation accounting. Listener-driven
> publisher waiting on the SB or EVS mutex was removed by try-locks; strict
> approved queue-lock correction now uses `FF_PalQueueTryPut`: queue-mutex busy
> skips only that pipe copy, increments dropped accounting, continues fan-out,
> requests a busy-specific deferred event and returns `FF_TIMEOUT`; full queue
> remains distinct. The synchronized test confirms return while the lock is held
> and another matched pipe receives its copy. Whole-publish WCET/freedom remains
> unproven. R1 FRICK-REVIEW-0264 also found saturation can lose
> SRS-REQ-011 ground forwarding. R3 FRICK-REVIEW-0265 did not approve the first
> requirements draft for baselining; event/loss/measurement boundaries were
> corrected in the working tree. FRICK-REVIEW-0266 approves continued
> requirements development with comments only, not baseline/N14 closure.
> SSS-REQ-006 and SRS-REQ-072 allocate `Bmax`, `Rmax`, `Dmin`, `Lmax`, service
> interval, link profile and ground-receipt boundary, all authority-owned TBDs.
> NC-EXT-77 records the corrected trace-count comment. NC-EXT-75/76 remain open
> pending profile authority, loss-counter/reporting implementation, full-cycle,
> and independent evidence. T21 remains NOT FULLY QUALIFIED; no overall
> N17/PA approval or NC closure is implied.
> These do not close NCs or qualify T13; NC-EXT-34..81 remain open. The owner
> authorized MPL-2.0 for original project-authored code and materials; NC-EXT-43 remains open only
> for external/generated and combined-distribution compatibility. No additional
> license authorization is pending from the owner.
> NC-S4b-01..09, NC-EXT-01..81
> and GAP-R3-1 remain open pending independent review or authority disposition.
> Sprint precedence is unresolved: the corrective sequence is proposed, but it
> conflicts with “drivers first”; do not start a competing sprint until the plan
> and human owner resolve ordering/approval.

**Historical resumption snapshot (SB/EVS configured structural stress; point-in-time text, superseded by current §4/§5; do not use its hosted-CI/NC statuses as current):**

 1. Continue using `docs/planning/sprint-plan.md` rev 1.10. Under N21,
     correct functional bugs and implement missing features committed for the
     current or previous sprints; record other findings as NCs and continue
     unrelated development. The owner has
     authorized MPL-2.0 for original project-authored code and materials; no further authorization
     is needed for that scope. Obtain R4/PA re-review of the current scope/status
     corrections; complete NC-EXT-43 external/generated compatibility disposition;
     obtain remaining independent R4/PA and hosted/configuration/immutable evidence
     before any applicable release gate (bounded R2 0249 is complete only for its scope).
        The narrowly scoped `FF_EvsSendDeferred` SB overflow correction restores
       normal synchronous-send behavior following PA FRICK-REVIEW-0263 and R1
       FRICK-REVIEW-0264. The initial event-envelope requirement draft was
       NOT APPROVED by FRICK-REVIEW-0265; FRICK-REVIEW-0266 approves continued
       development only, not baselining. NC-EXT-75/76 remain open pending
       authorized rate/service/latency/link values, loss-report implementation,
        applicable verification and independent delta reviews. SB queue-mutex
        busy/partial-fan-out behavior is implemented and host-tested; full queues
        retain the distinct overflow path. SRS-REQ-072 end-to-end loss counter,
        reporting and approved event profile remain unimplemented/open, and
        SSS/SRS authority-owned TBDs are unchanged. Keep all NCs open until their
       criteria are met.
     A host-only exploratory structural stress test now holds all 32 subscriber
     queue locks for one publish (32 drops, `FF_TIMEOUT`), fills 32 pipes × depth
     32 (1024 queued copies), then creates three 32-event overflow bursts:
     96 more SB drops, 96 accepted EVS notifications cumulatively, 32 counted
     EVS notification losses and 128 total log entries (newest 64 retained).
     Dispatch invokes 96 callbacks across the two phases. This is configuration-bound
     saturation observation, not an event-rate/WCET/ground-delivery guarantee.
     `make ci-local` (28/28, Lean, trace), `make smoke`, `make renode` and targeted
     clang-format/diff checks pass after this addition. RTEMS cross-build and
     hosted CI were not run. The initial test placement failed due PAL queue-slot
     exhaustion across the pre-existing in-process test suite; separate-process
     isolation resolved that test-harness limitation.
     **Commit/push remains blocked**: applicable independent R1/R3/R4/PA reviews,
     N14 baseline authority, open NC-EXT-75/76, end-to-end loss semantics and
     implementation, full sprint scope, and remaining configuration/hosted/RTEMS
     evidence are not complete. Passing this stress test does not close those gates.
      Scoped R4 FRICK-REVIEW-0268 and PA FRICK-REVIEW-0269 have since reviewed the
      stress-test documentation/evidence with comments only; they are not overall
      N17/N20 approval. Fresh clean Debug build and 28/28 CTest passed; selected
      artifact identities and limits are in `docs/vv/evidence/2026-10-14-sb-evs-structural-stress.md`.
      The authorized user then directed a reproducible framework-only SB/EVS
      parameter characterization without inventing mission inputs. A retained
      sweep covers fan-out/depth 1–32 and overflow-burst counts from zero through
      first EVS notification saturation (9,984 profiles), plus dispatch limits
      1–64 with excess pending for limits 1–63. It passed 1,697,329 assertions;
      see `docs/architecture/sb-evs-characterization.md` and
      `docs/vv/evidence/2026-10-14-sb-evs-structural-sweep.md` + `.csv`.
      R3/R2/R4/PA scoped reports FRICK-REVIEW-0270..0273 are recorded. This is
      native-host structural characterization only, not mission performance,
      WCET, target timing, link availability or ground receipt. The first
      malformed CSV was overwritten and is not retained separately. T22 remains
      pending formal tool-owner N10 acceptance; NC-EXT-75/76, mission values,
      detection/counter/reporting semantics, N14 mission baseline, hosted CI,
      and broader sprint/release gates remain open. A fresh RTEMS 6 cross-build
      (`build/rtems-sweep-final`, ELF64 RISC-V) and real Renode functional check
       (`RENODE_TEST: PASS`) had passed at the time; hosted CI on that then-current
       uncommitted tree was unrun. R2 source-identity delta FRICK-REVIEW-0274 confirms the
      final script hash and byte-identical retained CSV.
 2. Then continue from the open NC register in the same plan. Keep
        NC-S4b-01..09, NC-EXT-01..81 and GAP-R3-1 open until independent delta reviews or
   authorized requirements/profile dispositions are recorded. Initial
   FRICK-REVIEW-0167/0168/0169 remain APPROVED WITH COMMENTS; no D1 reports exist.
  3. The external assessment's PDL wrap finding has a widened decoder fix and
   boundary tests; an independent reviewer found the TM decoder tests needed
   parity with TC and a separate GSW stream-offset parsing defect. Both were
   corrected; rerun full tests and independent delta reviews.
  4. Evidence now includes native build/test, trace, GSW vectors, smoke, RTEMS
    Docker cross-build and a real Renode `RENODE_TEST: PASS`. ASan/UBSan and the Docker-image `make ci-local` have passed after the final C/Python
   test changes; `make ci-local` supplies the Lean `lake build` evidence. The host
   `make verify-lean` attempt failed with `lake: not found`; distinguish that local
    limitation from the observed Docker-container PASS. Forge CI is still unrun on this tree.
  5. FRICK-REVIEW-0173 independently verified the finite-buffer GSW resynchronization
    case but did not close T7 overall; it opened NC-EXT-13 for the CCSDS primary
    header profile/acceptance decision. Its severity is provisional pending the
     controlled criterion and rationale tracked by NC-EXT-31. At this historical
     snapshot NC-EXT-02 lacked maximum-size application-ingest evidence; current
     status is in §4 and `docs/planning/sprint-plan.md`. NC-EXT-03 lacks external
     ground-segment E2E.
  6. Status gates for documentation review, PA review, review closure, loop audit
     and V&V records are intentionally yellow while this loop is open. Do not
     commit/push until N15/N17/N20 and required independent re-reviews pass.
     Although the Docker-image `make ci-local`, smoke/GSW, format, metrics and Renode
     commands passed, their status semaphores remain yellow pending NC-EXT-29's
     complete immutable configuration/tool/artifact/log identity. Host
      `make check` is blocked at `verify-lean` because `lake` is absent; Docker
      `lake build` passed in the observed local image.
   7. The proposed sprint sequence is S8-FU closure → S8-VV-FU (GAP-R3-1,
   NC-EXT-25) → S8-PUS-FU (NC-EXT-14..18,
   20..21, 23..24 where requirements/profile permit autonomous correction) →
     S8-SCH (NC-EXT-19/M17) → M12. The current plan interprets the owner's
     general resumption instruction as selecting S8-FU first; bounded R4 review
     0311 accepted this wording, closing NC-EXT-30/65 only for this plan comment.
    NC-EXT-22 (PUS S11 stub) is assigned to M15;
    it is not closed by the application scheduler. Existing GAP-A1..A14 remain
     open in `class-a-gaps.md`; mission-specific licensing/adoption and
      SAVOIR/OSRA applicability require authority/legal decisions before any
      conformance/adoption claim; NC-EXT-43 tracks external/generated component
      boundaries and combined-distribution compatibility, not pending owner
      authorization for original project content.
      GAP-A1..A14
     closure work is mapped in sprint-plan §3.1; those records remain OPEN.
  8. N14 is OPEN: the current SSS-REQ-030 change lacks an independent R3 review
    (NC-EXT-33). Latest author corrections address the ICD TC/TM diagram and time offsets,
    the PUS backend signature, a stale ICD source-comment path, and review-report
    publication instructions (NC-EXT-26/27/32). All remain OPEN pending
    independent re-review; do not infer closure from these edits.

- **S8/M8 gPTP + Qci completed** (SRS-REQ-068/066). gPTP stack: BMCA,
   Sync/Follow_Up, PDelay, PI servo (`test_gptp` 58/58 host) — integrates with GEM TSU
   `get_time_ns` + `FF_EthMac_t`; on-Renode GEM TSU register accessible (host-tested
   mock-MAC frame exchange; full GEM DMA = GAP-A5). Qci: screener type 1 entry per
   traffic-class; `test_gem_tsn` 22/22. Qav CBS + Qbb PFC = `FF_NOT_IMPLEMENTED`
    (NC-S8-01/NC-S8-02). Integration-manual R1/R4 reviews (FRICK-REVIEW-0159/0160)
     corrected; its R4 delta review is recorded as FRICK-REVIEW-0164. The
     broader S8-FU R4 gate remains open; licensing/status review 0252 was
      NOT APPROVED and was followed by 0254/0255 and 0256/0257, all NOT
      APPROVED. Their active-status corrections are recorded; independent
      re-review remains open.
2. **M4 residual** restored M4 scope (S4b/S4c/S4d): PUS-C over IP/TSN
   (SRS-REQ-059/060/061), CAN-FD/CANopen/RS-485 DE (SRS-REQ-062/063/064),
   PUSopen adapter (SRS-REQ-065), fabric contract (SRS-REQ-067).
3. **M12** OBCP engine; **M13** mission services; **M14** multicore/SMP;
   **M15** in-house PUS-C + CFDP + VFS; **M16** in-house OBCP engine;
   **M17** RTEMS-backed application scheduler.
4. Keep N18 blocking (nightly real run) and close GAP-A4 (certified MISRA checker)
   when available.

## 6. Key facts / gotchas

- Renode MPFS platform peripheral base addresses: uarts `mmuart0..4`
  (0x20000000, 0x20100000, …); console = **mmuart1**; gpio`0/1/2`
  (0x20120000/…); CAN 0x2010C000/0xD000; SPI 0x20108000/0x9000; I2C
  0x2010A000/0xB000; GEM 0x20110000/0x12000; SD 0x20008000; eNVM cfg
  0x20200000 / data 0x20220000.
- **MSS GPIO register map** (validated): per-pin `GPIO_CFG[pin]` at `0x00+pin*4`
  (bit0 output enable, bit1 input enable, bit3 int enable, bits5-7 int type);
  `GPIO_IN=0x84`, `GPIO_OUT=0x88`. GPIO0 has **14** pins.
- **MSS MMUART register map** (validated): the Renode `polarfire-soc.repl`
  instantiates the UARTs as `UART.NS16550` with **`wideRegisters: true`**, so the
  MPFS MMUART uses **4-byte register spacing** on hardware *and* in Renode (the
  model divides `offset` by 4 when `offset % 4 == 0`, so wide offsets
  `0x00/0x04/0x08/0x0C/0x10/0x14/0x18/0x1C` alias the compact registers). A serial
  driver must use the wide offsets (hardware-correct) — compact offsets also
  "work" in Renode by aliasing but are wrong on silicon.
- RTEMS image entry point is **0x1000000000** (HSS high DDR alias); set
  `u54_1 PC 0x1000000000`, halt `e51`/`u54_2..4`.
- `make renode` passes only when `build/renode/frick_fsw_renode.elf` exists
  (git-ignored); otherwise it SKIPs (proxy). Rebuild it after C changes.
- `trace.py` requires every non-derived requirement to have `tests`/CI evidence
  and existing implementation paths; planned reqs use `ci:docs`.
- Independent reviews tend to find **real defects**; run them before every
  commit and close NCs (N12).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial handover/resume document (N18 in force; M11 GPIO done) |
| 0.2 | 2026-10-10 | frick-fsw | M11 serial deferred/reverted; codename "Eärendil" disclaimer added |
| 0.3 | 2026-10-10 | frick-fsw | M11 serial re-implemented with hardware-correct 4-byte MMUART offsets (Renode wideRegisters); key fact recorded |
| 0.4 | 2026-10-10 | frick-fsw | M11 GPIO + SERIAL done (SRS-REQ-047/048); counts 86/49/23/14 |
| 0.5 | 2026-10-11 | frick-fsw | Add §0 "How to resume in a new session" (kickoff prompt) |
| 0.6 | 2026-10-11 | frick-fsw | M11 GPIO + SERIAL + MEM done (SRS-REQ-047/048/049); counts 87/50/23/14 |
| 0.7 | 2026-10-11 | frick-fsw | PUS-C ASN.1 default backend (SRS-REQ-050, ADR-0016); M14/M15 added; counts 94/54/26/14 |
| 0.8 | 2026-10-11 | frick-fsw | M11 ETH HAL class exercised in Renode (`hal_eth=PASS`) |
| 0.9 | 2026-10-11 | frick-fsw | M11 NET `FF_NetStack_t` contract check exercised in Renode (`hal_net=PASS`) |
| 1.0 | 2026-10-11 | frick-fsw | Norm N20 (Senior SW PA review of every phase product); counts refreshed 94/54/26/14/51; PA gate added |
| 1.1 | 2026-10-11 | frick-fsw | N12 loop closed: independent R1/R2/R4 reviews of the N20 set and FRICK-REVIEW-0032 to closure; PA review PASS (FRICK-REVIEW-0063) |
| 1.2 | 2026-10-11 | frick-fsw | Milestone M16 added; counts 96/55/27/14; sprint plan published; next steps point to Sprint 0/1 |
| 1.3 | 2026-10-11 | frick-fsw | Sprint 1.1: reference eNVM NVM backend (SRS-REQ-055), M11 5/13, counts 97/56/27/14 |
| 1.4 | 2026-10-11 | frick-fsw | Sprint 1.2: reference NOR/QSPI MTD backend (SRS-REQ-056); counts 98/57/27/14 |
| 1.5 | 2026-10-11 | frick-fsw | Sprint 1.3: reference MSS CAN backend (SRS-REQ-057); M11 6/13 (`hal_can=PASS`); counts 99/58/27/14 |
| 1.6 | 2026-10-11 | frick-fsw | CAN review set closed (FRICK-REVIEW-0106..0119: R1/R2/R4 + PA initial/delta/closure + freeze); accept-all mask, clock 80 MHz, test 129 checks |
| 1.7 | 2026-10-11 | frick-fsw | M10 done: QEMU generic-BSP (riscv/rv64imafdc) emulated-target run (`make qemu`, T20); toolchain image gains the rv64imafdc BSP + QEMU |
| 1.8 | 2026-10-12 | frick-fsw | Sprint 3: reference MSS I2C backend (SRS-REQ-058); M11 7/13 peripheral-bound (`hal_i2c=PASS`); counts 100/59/27/14/51; ctest 20/20 (test_i2c 144 checks); next S4 (SPI) |
| 1.9 | 2026-10-12 | frick-fsw | Sprint-3 review set closed (FRICK-REVIEW-0129..0136: R1/R2/R3/R4/PA initial + deltas; PA REJECTED -> APPROVED WITH COMMENTS); user-facing TCP/IP + TSN sections and the expanded framework description (FRICK-ENG-0007) added; T14 reproducible via `make misra-docker`; next S4 (SPI) |
| 1.10 | 2026-10-12 | frick-fsw | Restored explicit M4 scope (N19 audit): PUS-C over IP/TSN (SRS-REQ-059/060/061), fabric contract (SRS-REQ-067), CAN-FD/CANopen/RS-485 DE (SRS-REQ-062/063/064), PUSopen adapter (SRS-REQ-065); M8 gPTP/CoreTSN (SRS-REQ-068) + GEM TSN Qav/Qci/Qbb (SRS-REQ-066); counts 112/69/29/14/51; DDJF acronym registered (FRICK-ENG-0007 0.3); sprint S4b/S4c/S4d — FRICK-REVIEW-0137 |
| 1.11 | 2026-10-12 | frick-fsw | Sprint 4: reference MSS SPI backend (SRS-REQ-069); M11 8/13 peripheral-bound (`hal_spi=PASS`); counts 113/70/29/14/51; ctest 21/21 (test_spi 126 checks); next S5 (BLOCK eMMC/SD) |
| 1.12 | 2026-10-12 | frick-fsw | Sprint 5: reference MSS block-device backend (SRS-REQ-070); M11 9/13 peripheral-bound (`hal_block=PASS`); counts 114/71/29/14/51; ctest 22/22 (test_block 85 checks); next S6 (NET/TSN peripheral-bound) |
| 1.13 | 2026-10-12 | frick-fsw | Sprint 6a: GEM native TSN backend exercised peripheral-bound (`hal_tsn=PASS`, M11 10/13; SRS-REQ-038); NC-S6-01 opened (NET lwIP-on-GEM peripheral-bound deferred to sprint S6b); next S6b/S7 |
| 1.14 | 2026-10-12 | frick-fsw | Sprint 6b-1: in-repo lwIP RTEMS `sys_arch` port (SRS-REQ-040) — host 23/23 with `FF_USE_LWIP=ON`, RTEMS build produces the lwIP archive (forced-extraction link resolves the stack; the image does not reference the adapter yet); NC-S6-02 opened (RTEMS runtime object sizing → S6b-2); NC-S7-01 (SpaceWire no model → S7 contract-level) |
| 1.15 | 2026-10-12 | frick-fsw | Sprint 6b-2: RTEMS object sizing wired (NC-S6-02 closed) + lwIP netif bound to the emulated GEM (`hal_net_lwip=PASS` on the lwIP image); NC-151-02/05 closed; **NC-S6-03** opened (latent GEM 64-bit descriptor DMA); NC-S6-01 narrowed (packet round-trip residual) |
| 1.16 | 2026-10-12 | frick-fsw | Sprint-6b-2 review close-out (FRICK-REVIEW-0153/0154/0155): netif MTU fixed (1500), glue-test gap tracked (NC-154-01), traceability History bumped (NC-154-02), stale doc sites + nightly label corrected |
| 1.17 | 2026-10-12 | frick-fsw | Sprint 7 delivered + pushed (`64961ac`): PCIe (`hal_pcie=PASS`) + SpaceWire contract-level (`hal_spacewire=CONTRACT`, M11 11/13); NET follow-up sprint: NC-S6-03 co-implemented (GEM 64-bit descriptor addressing: `gem_phys` returns `uint64_t`, `gem_wr_qbqp`, `TBQP_HI`/`RBQP_HI`, `DMACFG.ADDR64`; per-descriptor truncation explicit; full 3-word descriptor format deferred to M14); NC-154-01 closed (`test_lwip_netif` 26/26 host, mock-MAC loopback); `test_gem` extended for 64-bit register writes; trace/lean/format/redline all green; combined R1+R2+R3+R4+PA review APPROVED WITH COMMENTS (FRICK-REVIEW-0162, 8 Minor findings, all corrected); delta re-review APPROVED (FRICK-REVIEW-0163, all corrections verified); commit + push to both remotes completed |
| 1.18 | 2026-10-12 | frick-fsw | Handover updated to record FRICK-REVIEW-0162 (NET follow-up confirmation, APPROVED WITH COMMENTS) and FRICK-REVIEW-0163 (delta re-review, APPROVED) in the recent-reviews list; all findings closed; next sprint S8 = M8 gPTP + CoreTSN |
| 1.19 | 2026-10-13 | frick-fsw | **S8/M8 delivered:** gPTP stack (BMCA, Sync/Follow_Up, PDelay, PI servo) in `src/drivers/eth/gptp/` (58 checks host); 802.1Qci via GEM screener type 1 in `ff_gem_tsn.c` (test_gem_tsn 22 checks); 802.1Qav CBS + 802.1Qbb PFC documented `FF_NOT_IMPLEMENTED` (NC-S8-01/NC-S8-02); integration-manual R1/R4 reviews (FRICK-REVIEW-0159/0160) corrected; traceability + SRS updated for SRS-REQ-066/068 |
| 1.20 | 2026-10-13 | frick-fsw | **S4b delivered:** PUS-C over IP/TSN (SRS-REQ-059/060/061) — seg/reassembly layer (`link_seg.c/h`, 2-byte BE length-prefix, allocation-free), IP backend (`link_ip.c/h`, UDP default + TCP with seg) over `FF_NetStack_t`, TSN backend (`link_tsn.c/h`, EtherType 0x88F5, callback-driven RX ring) over `FF_EthMac_t + FF_TsnBackend_t`; `link.h` declares new backends; ctest 27/27 + trace + Lean + smoke + GSW all PASS; Renode SKIP (host-only code). Traceability + SSS-REQ-028 updated from planned to implemented. |
| 1.21 | 2026-10-13 | frick-fsw | **S4b delta re-review (N12 closure):** initial R1+R2 (FRICK-REVIEW-0165) and PA (FRICK-REVIEW-0166) reviews REJECTED (2 Major + 10 Minor + 6 V&V gaps + 4 PA Minor); all findings fixed; 3 new findings raised and closed (NF-1 unused variable, NF-2 Major buffer overflow in `rx_partial` copy-back, NF-3 doc omission); R1+R2 delta (FRICK-REVIEW-0165-D1) APPROVED; PA delta (FRICK-REVIEW-0166-D1) APPROVED; `-Wall -Wextra -Werror` clean; max-size fragmented regression test added; status.json/status/handover/loop-audit/verification-results/CHANGELOG/reviews all updated |
| 1.22 | 2026-10-13 | frick-fsw | S4b host PUS-C round-trip demonstrations added and documentation corrections applied; NC-S4b-01..05 remain open pending independent delta-review evidence (initial FRICK-REVIEW-0167/0168/0169 are APPROVED WITH COMMENTS). |
| 1.23 | 2026-10-13 | frick-fsw | README/network/handover scope/status corrections applied for NC-S4b-06..08; independent delta review remains pending. |
| 1.24 | 2026-10-13 | frick-fsw | Current handover reset to uncommitted baseline `57974d2` + S8-FU changes; record NC-S4b/NC-EXT open status, latest review follow-ups, corrected GSW stream-offset parser and pending gates; document RTEMS Docker cross-build/Renode PASS and local Lean unavailability. |
| 1.25 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0173 partial re-review and NC-EXT-13; clarify maximum-ingest and external-GSW evidence gaps; align the current S8-FU open status with sprint-plan rev 0.26 and verification-results rev 0.48. |
| 1.26 | 2026-10-13 | frick-fsw | Align resume point to sprint-plan rev 0.28; track NC-EXT-14..24 and sequence S8-PUS-FU/S8-SCH, while preserving unresolved authority and assurance gaps as OPEN |
| 1.27 | 2026-10-13 | frick-fsw | Align handover with sprint-plan rev 0.29; map existing GAP-A1..A14 closure work and authority/resource requests |
| 1.28 | 2026-10-13 | frick-fsw | Align handover with sprint-plan rev 0.30; include GAP-R3-1 and NC-EXT-25 in S8-VV-FU before PUS/scheduler work |
| 1.29 | 2026-10-13 | frick-fsw | Align handover with sprint-plan rev 0.32; include NC-EXT-26/27 and maintain open independent-review gates |
| 1.30 | 2026-10-13 | frick-fsw | Record latest R4 REJECTED/PA FAIL verdicts and NC-EXT-28..31; synchronize current summary, M17, counts and unresolved owner sequencing |
| 1.31 | 2026-10-13 | frick-fsw | Correct stale integration-manual review statement; reflect separate R4 gate verdict and keep all closure reviews open |
| 1.32 | 2026-10-13 | frick-fsw | Record follow-up review IDs and qualify NC-EXT-13 as an open authority policy issue pending controlled acceptance and severity basis |
| 1.33 | 2026-10-13 | frick-fsw | Identify FRICK-REVIEW-0174 as the latest R4 verdict; update sprint-plan revision reference and include planned M17 in the milestone summary |
| 1.34 | 2026-10-13 | frick-fsw | Record latest R4/PA reports 0176/0177 and maintain explicit blocked status |
| 1.35 | 2026-10-13 | frick-fsw | Reconcile M9 status with status.json and distinguish pending evidence configuration from test command outcomes |
| 1.36 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0178/0179 and NC-EXT-32; retain current release gates open |
| 1.37 | 2026-10-13 | frick-fsw | Explain passing command results versus yellow verification semaphores and record the host lake limitation |
| 1.38 | 2026-10-13 | frick-fsw | Align sprint-plan revision reference and record author corrections for NC-EXT-26/27 without claiming closure |
| 1.39 | 2026-10-13 | frick-fsw | Record latest failed review verdicts and NC-EXT-26/27 re-review status |
| 1.40 | 2026-10-13 | frick-fsw | Capture author corrections and pending re-review scope for NC-EXT-26/27/32 |
| 1.41 | 2026-10-13 | frick-fsw | Synchronize latest recorded review IDs/verdicts and sprint-plan rev 0.38 |
| 1.42 | 2026-10-13 | frick-fsw | Align handover with sprint-plan rev 0.39 and expanded N11/NC-EXT-32 clarification |
| 1.43 | 2026-10-13 | frick-fsw | Align resume pointer with sprint-plan rev 0.39 and preserve pending owner precedence decision |
| 1.44 | 2026-10-13 | frick-fsw | Align live sprint pointer and proposed sequence wording with sprint-plan rev 0.41; no owner priority inferred |
| 1.45 | 2026-10-13 | frick-fsw | Correct live sprint pointer after rev 0.41 and make unresolved precedence wording explicit |
| 1.46 | 2026-10-13 | frick-fsw | Track new N14/R3 review gap and label proposed sprint order as pending owner decision |
| 1.47 | 2026-10-13 | frick-fsw | Align resume point with sprint-plan rev 0.42 and explicitly record N14/NC-EXT-33 as open |
| 1.48 | 2026-10-13 | frick-fsw | Correct stale live sprint-plan pointer to rev 0.42 after R4 delta review |
| 1.49 | 2026-10-13 | frick-fsw | Align live pointer with sprint-plan rev 0.43 and record ICD authority scope remains under NC-EXT-13 |
| 1.50 | 2026-10-13 | frick-fsw | Align live sprint pointer with sprint-plan rev 0.44; record FRICK-REVIEW-0180 corrections pending review |
| 1.51 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0180/0181 as latest verdicts and the corrective re-review scope |
| 1.52 | 2026-10-13 | frick-fsw | Correct current sprint-plan pointer to rev 0.44 and align PA gate wording with N12 |
| 1.53 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0182/0183 and align live sprint-plan pointer with rev 0.45 |
| 1.54 | 2026-10-13 | frick-fsw | Align current resume pointer with sprint-plan rev 0.45 after review follow-up |
| 1.55 | 2026-10-13 | frick-fsw | Align with sprint-plan rev 0.46 and record FRICK-REVIEW-0184/0185 plus NC-EXT-34 |
| 1.56 | 2026-10-13 | frick-fsw | Correct handover NC range and live sprint-plan pointer to rev 0.46 |
| 1.57 | 2026-10-13 | frick-fsw | Align sprint-plan pointer to rev 0.47 after NC-EXT-34 source correction |
| 1.58 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0186/0187 and align current pointer with sprint-plan rev 0.48 |
| 1.59 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0188/0189 scoped results and align plan pointer to rev 0.49 |
| 1.60 | 2026-10-13 | frick-fsw | Align to sprint-plan rev 0.50 and include T1 requalification NC-EXT-35/36 in resumption record |
| 1.61 | 2026-10-13 | frick-fsw | Align pointer with sprint-plan rev 0.51 and record FRICK-REVIEW-0190 T1 findings |
| 1.62 | 2026-10-13 | frick-fsw | Align to sprint-plan rev 0.52 and update latest T1/NC-EXT-34 status wording |
| 1.63 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0191..0193 and align pointer to sprint-plan rev 0.53 |
| 1.64 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0194 and align pointer to sprint-plan rev 0.54 |
| 1.65 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0195..0197 scoped results; retain NC-EXT-34..37 open |
| 1.66 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0198/0199 and pending re-review of active T1 claim corrections |
| 1.67 | 2026-10-13 | frick-fsw | Reconcile handover evidence wording with observed, mutable Docker-image limitation under NC-EXT-35 |
| 1.68 | 2026-10-13 | frick-fsw | Reconcile active Docker/T1 reproducibility language across handover; NC-EXT-35 remains open |
| 1.69 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0200/0201 and pending independent review of active T1 wording corrections |
| 1.70 | 2026-10-13 | frick-fsw | Bound `make ci-local` description to actual stages and retain separate smoke/docs commands |
| 1.71 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0202/0203 and NC-EXT-38; preserve open review/hosted gates |
| 1.72 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0204/0205 and align handover to sprint-plan rev 0.56; NCs remain open |
| 1.73 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0206/0207; retain qualification, evidence, and scope findings open |
| 1.74 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0208/0209; align plan pointer to rev 0.58; retain all NCs open |
| 1.75 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0210/0211; point to sprint-plan rev 0.59; retain NC-EXT-38/41 open |
| 1.76 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0212/0213, align NC range to 41 and retain guide-history findings open |
| 1.77 | 2026-10-13 | frick-fsw | Update plan pointer to rev 0.60 and current failed review references; NCs remain open |
| 1.78 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0214/0215 and point to sprint-plan rev 0.62; retain open findings |
| 1.79 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0216/0217 and point to sprint-plan rev 0.63; retain all NCs open |
| 1.80 | 2026-10-13 | frick-fsw | Record ledger correction and point to sprint-plan rev 0.64; NC-EXT-41 remains open |
| 1.81 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0218/0219 and point to sprint-plan rev 0.66; no NC closed |
| 1.82 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0221..0223; update plan pointer/range to rev 0.68 and NC-EXT-42 |
| 1.83 | 2026-10-13 | frick-fsw | Refresh current-review summary and NC-EXT-42 status after T13/metric corrections |
| 1.84 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0224..0226 and update plan pointer to rev 0.70; retain all findings open |
| 1.85 | 2026-10-13 | frick-fsw | Point to sprint-plan rev 0.71 and preserve latest independent review status |
| 1.86 | 2026-10-13 | frick-fsw | Correct duplicate review-status sentence, include T13 qualification limit, and point to sprint-plan rev 0.72 |
| 1.87 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0227 and point to sprint-plan rev 0.73; retain all NCs open |
| 1.88 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0228 and preserve T13 review/qualification caveats |
| 1.89 | 2026-10-13 | frick-fsw | Align active plan pointers to sprint-plan rev 0.74 after R4 finding |
| 1.90 | 2026-10-13 | frick-fsw | Record scoped reviews 0229..0231 and update active plan pointer to rev 0.75 |
| 1.91 | 2026-10-13 | frick-fsw | Align handover review-status summary with scoped reports 0229..0231; retain T13/NC caveats |
| 1.92 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0232 and point to sprint-plan rev 0.76; no NC closed |
| 1.93 | 2026-10-13 | frick-fsw | Prioritize candidate MPL-2.0 transition and record NC-EXT-43 gates; point to plan rev 0.77 |
| 1.94 | 2026-10-13 | frick-fsw | Record transition reviews 0235..0237, NC-EXT-44, and point to plan rev 0.78; no NC closed |
| 1.95 | 2026-10-13 | frick-fsw | Record T21 reviews 0238..0242; update current plan pointer to rev 0.82; no NC closed |
| 1.96 | 2026-10-13 | frick-fsw | Record explicit candidate-only corrections from R4/PA findings; point to sprint plan rev 0.84; NC-EXT-43/44 remain open |
| 1.97 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0244 findings, point to sprint plan rev 0.85, and retain NC-EXT-43/44/T21 gates open |
| 1.98 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0245 bounded SVG result; point to sprint plan rev 0.86 and retain all release gates |
| 1.99 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0246 DDJF range finding, point to plan rev 0.87, and retain NC-EXT-43/44/45 open |
| 2.00 | 2026-10-13 | frick-fsw | Record DDJF T1..T21 author correction, point to plan rev 0.88, and retain NC-EXT-45 pending independent review |
| 2.01 | 2026-10-13 | frick-fsw | Record FRICK-REVIEW-0247 bounded DDJF pass; retain NC-EXT-45 pending PA and all other gates |
| 2.02 | 2026-10-14 | frick-fsw | Record owner authorization scope, PA-0248 unreadable-file finding and local correction; update S8-FU resume point to plan rev 0.89 |
| 2.03 | 2026-10-14 | frick-fsw | Record bounded R2 0249 result and R4/PA scope findings 0250/0251; correct active scope and update review status |
| 2.04 | 2026-10-14 | frick-fsw | Record scope/status re-review findings 0252/0253; clarify current resumption metadata, NC-EXT-43/45 scope, and pending PA review |
| 2.05 | 2026-10-14 | frick-fsw | Record follow-up review findings 0254/0255; clarify broader S8-FU gate versus latest scoped outcome; update T21 status history |
| 2.06 | 2026-10-14 | frick-fsw | Correct active review summary after 0254/0255, distinguish historical broad gate from scoped latest review, and record re-review status |
| 2.07 | 2026-10-14 | frick-fsw | Correct sprint-plan pointers, clarify current review delta, and extend NC-EXT-45 PA source trail |
| 2.08 | 2026-10-14 | frick-fsw | Record status follow-ups 0256/0257 and latest re-review state; keep all gates open |
| 2.09 | 2026-10-14 | frick-fsw | Record bounded no-new-finding status reviews 0258/0259; retain NCs and qualification gates open |
| 2.10 | 2026-10-14 | frick-fsw | Align active plan pointers to rev 0.95 and preserve bounded 0258/0259 limits |
| 2.11 | 2026-10-14 | frick-fsw | Record N21 development sequencing rule, NC-EXT-46 deferral, and sprint-plan rev 0.96 pointer |
| 2.12 | 2026-10-14 | frick-fsw | Record N22 test rule, NC-EXT-47, and sprint-plan rev 0.97 pointer |
| 2.13 | 2026-10-14 | frick-fsw | Record N21 wording propagation, N7 designated-agent commit/push authority, FRICK-REVIEW-0260/0261 reconciliation, NC-EXT-48..68, and N22 work-package state; point to sprint-plan rev 1.00 |
| 2.15 | 2026-10-14 | frick-fsw | Record FRICK-REVIEW-0263 and NC-EXT-75 for the SB overflow EVS functional correction; update active NC range and preserve delta-review/gate status |
| 2.16 | 2026-10-14 | frick-fsw | Update resumption pointer to sprint-plan rev 1.02 and record the SB functional correction as implemented but open pending full-cycle/independent verification |
| 2.17 | 2026-10-14 | frick-fsw | Record R1 follow-up correction: defer EVS listener callbacks to bounded TM-output dispatch, document explicit queue overflow accounting, and keep NC-EXT-75 open pending full-cycle/independent delta review |
| 2.18 | 2026-10-14 | frick-fsw | Restore synchronous ordinary EVS sends and scope deferred API to SB overflow; record exact test evidence and leave strict mutex nonblocking interpretation and NC-EXT-75 open |
| 2.19 | 2026-10-14 | frick-fsw | Record FRICK-REVIEW-0264 and NC-EXT-76; update resumption pointer to sprint-plan rev 1.04 and preserve the unresolved no-block/ground-forwarding conflict |
| 2.20 | 2026-10-14 | frick-fsw | Record initial R3 review FRICK-REVIEW-0265, corrected requirement boundaries, current SRS-REQ-072 trace count and sprint-plan rev 1.07; retain authority/profile and review gates open |
| 2.21 | 2026-10-14 | frick-fsw | Record R3 delta FRICK-REVIEW-0266 (continued development only), NC-EXT-77 count correction, NC-EXT-76 remaining loss semantics, and sprint-plan rev 1.09 |
| 2.22 | 2026-10-14 | frick-fsw | Record user-approved PAL queue try-put behavior, partial fan-out, SRS-REQ-001 update, current verification evidence/limitations and sprint-plan rev 1.10; retain NC-EXT-75/76 and SRS-REQ-072 profile/loss-reporting work open |
| 2.23 | 2026-10-14 | frick-fsw | Record isolated configured-limit SB/EVS saturation stress results, initial test-harness resource collision and resolution; preserve commit/push blockers and open event-envelope authority/loss-reporting gates |
| 2.24 | 2026-10-14 | frick-fsw | Record fresh clean structural-stress evidence and scoped R4/PA reviews; retain N14, NC-EXT-75/76, full-sprint and broader release gates as open |
| 2.25 | 2026-10-14 | frick-fsw | Record 9,984-profile SB/EVS host structural characterization, retained CSV/evidence identities, bounded R3/R2/R4/PA reports and final CI-local cycle; retain mission authority inputs, NC-EXT-75/76, T22 N10 acceptance and release gates as open |
| 2.26 | 2026-10-15 | frick-fsw | Record the committed/pushed state (`a4c436c`, `03773f7`, `620fff8`), green GitHub Actions run #83, and the `make docs` output-existence guard delta with scoped R4 FRICK-REVIEW-0285 / PA FRICK-REVIEW-0286 reviews (both APPROVED WITH COMMENTS; R4 NC-01/NC-02 verified closed by bounded delta re-review FRICK-REVIEW-0287); retain hosted-evidence (T21), NC-EXT-43/75/76/78..81 and release gates open |
| 2.27 | 2026-10-15 | frick-fsw | Record the committed guard delta (`77cd548`) and its green hosted GitHub Actions run #84 (all 8 jobs); Gitea run log unevidenced (NC-EXT-36); retain hosted-CI qualification evidence (T21), NC-EXT-43/75/76/78..81 and release gates open |
| 2.28 | 2026-10-15 | frick-fsw | Record hosted run #85 for the record-only commit `03366f1` (all 8 jobs) and the loop-audit N15 record-only convention that terminates the record → run → record regress; Gitea run log unevidenced (NC-EXT-36); retain hosted-CI qualification evidence (T21), NC-EXT-43/75/76/78..81 and release gates open |
| 2.29 | 2026-10-15 | frick-fsw | Resume point moved to HEAD 6ce6bd7 plus uncommitted S4c/S4d/default work; record focused and fresh-default host results, NC-S4c-01 limitation, ADR-0018 defaults and outstanding full-cycle/independent/hosted gates |
| 2.30 | 2026-10-15 | frick-fsw | Record completed local Debug/Release/ASan, Lean/trace, docs/format/smoke/GSW, RTEMS/Renode/QEMU cycle and update next steps to independent reviews and hosted CI; no commit/push or release approval claimed |
| 2.31 | 2026-10-15 | frick-fsw | Record 169-check CANopen test oracle completion, final count and clarified RS-485 release-call hold semantics; review and hosted gates remain pending |
| 2.32 | 2026-10-15 | frick-fsw | Complete exact response-frame assertions for absent-index CANopen download and update the final test count to 171; reviews and hosted CI remain open |
| 2.33 | 2026-10-15 | frick-fsw | Record final 171-check full verification cycle and completed bounded independent review set; retain open authority, dependency evidence, provenance, hosted-CI, target and vendor gates |
| 2.34 | 2026-10-15 | frick-fsw | Record scoped closures for NC-S4c-02 and NC-EXT-82, final T23 behavior review, and update next actions to remaining PA/R4, authority and hosted gates |
| 2.35 | 2026-10-15 | frick-fsw | Record R2 tracked-state follow-up 0299, index 0298 as point-in-time, and retain PA/R4/hosted/authority gates |
| 2.36 | 2026-10-15 | frick-fsw | Record PA tracked-state review 0300 and remaining R4 scope; retain T23 qualification, hosted, target/vendor, authority and provenance gates |
| 2.37 | 2026-10-15 | frick-fsw | Record bounded R4 status/evidence review 0301 and preserve remaining overall documentation, hosted, target/vendor and authority gates |
| 2.38 | 2026-10-15 | frick-fsw | Record final bounded R4/PA evidence reviews 0302/0303; keep phase/release and hosted/provenance/target/authority gates open |
| 2.39 | 2026-10-15 | frick-fsw | Record latest R4/PA timing chronology follow-ups 0304/0305 and keep all broader gates open |
| 2.40 | 2026-10-15 | frick-fsw | Record R4/PA timing text follow-ups 0306/0307 and retain open phase/release, hosted, provenance and authority gates |
| 2.41 | 2026-10-15 | frick-fsw | Reconcile current baseline to pushed HEAD 13ebf92; record green GitHub run 38793643786, prior CI formatting/metrics failures and correction, Gitea API verification limit, CANopen guard/test delta, and unresolved sprint precedence; retain NC-S4c-04 and broader gates open |
| 2.42 | 2026-10-15 | frick-fsw | Clarify committed baseline versus current uncommitted documentation delta, state that prior GitHub CI does not cover it, and avoid asserting historical N7 pre-push compliance without supporting records |
| 2.43 | 2026-10-15 | frick-fsw | Record initial scoped R4/PA reviews 0308/0309, their corrected findings and pending delta re-reviews; retain N7 and hosted-CI limits |
| 2.44 | 2026-10-15 | frick-fsw | Record bounded final re-review 0310 of the 0308/0309 wording corrections; retain uncommitted-delta, historical N7, Gitea, and broader review limits |
| 2.45 | 2026-10-15 | frick-fsw | Record the owner's sequencing direction: resume with S8-FU; keep NC-EXT-30 open pending independent R4 review of the plan change |
| 2.46 | 2026-10-15 | frick-fsw | Clarify S8-FU as this plan's interpretation of the owner's general instruction, not a verbatim owner selection; preserve pending NC-EXT-30 review |
| 2.47 | 2026-10-15 | frick-fsw | Reconcile current and prior resume passages to sprint-plan rev 1.26 and distinguish owner instruction from the plan's S8-FU interpretation |
| 2.48 | 2026-10-15 | frick-fsw | Record R4 review 0311 closure of NC-EXT-30/65 for bounded sequencing wording and align current sprint pointer to S8-FU |
| 2.49 | 2026-10-15 | frick-fsw | Record N22-TS1-96 as partial host evidence for NC-EXT-02, retain receive-loop limitation, and label older handover passage as point-in-time history |
| 2.50 | 2026-10-15 | frick-fsw | Record the production receive-helper test, V&V 0316 accepted bounded evidence, and remaining NC-owner/target/NC-EXT-03 limits |
| 2.51 | 2026-10-15 | frick-fsw | Record UART PDL framing, host PTY V&V 0319, current local verification results and NC-EXT-07 authority/target limits |
| 2.52 | 2026-10-15 | frick-fsw | Clarify NC-EXT-01/02 remain open until authorized owner dispositions are recorded; record current PA/R4 findings |
| 2.53 | 2026-10-15 | frick-fsw | Link S8-FU candidate manifest and retained logs; record R4/PA corrective delta and remaining review/authority blockers |
| 2.54 | 2026-10-15 | frick-fsw | Record R4 0323 and PA 0324 bounded acceptance; preserve NC-owner and broader N17/N20 gates |
| 2.55 | 2026-10-15 | frick-fsw | Reconcile exact NC-EXT-01/02 evidence wording and record current R4/PA bounded re-review plus candidate manifest pointer |
| 2.56 | 2026-10-16 | frick-fsw | Record fresh post-review CI-local, smoke, tools, docs, RTEMS cross-build and Renode results; S8-FU authority gates remain open |
| 2.57 | 2026-10-16 | frick-fsw | Record independent R1 UART findings, bounded corrections, unresolved malformed-PDL recovery and user's authority-blocked direction |
| 2.58 | 2026-10-16 | frick-fsw | Record V&V 0327–0329 case-mapping/count correction, SVA-2 update, current manifest and authority-blocked sprint status |
| 2.59 | 2026-10-16 | frick-fsw | Mark the 46-check UART summary historical, point to the current 51-check candidate manifest and reconcile active baseline claims |
| 2.60 | 2026-10-16 | frick-fsw | Record bounded R4/PA finding re-review acceptance and identify final applicable verification cycle as next action |
| 2.61 | 2026-10-16 | frick-fsw | Record final applicable local verification cycle and preserve authority/external/target blockers |
| 2.62 | 2026-10-16 | frick-fsw | Record physical-target test postponement, homepage disclaimer and remaining independent blockers |
| 2.63 | 2026-10-16 | frick-fsw | Record scoped R4/PA findings on target-deferral documentation; sprint header/effectivity corrected and latest cycle/re-review pending |
| 2.64 | 2026-10-16 | frick-fsw | Record bounded R4/PA re-review 0336/0337; final applicable verification and non-target sprint gates remain open |
| 2.65 | 2026-10-16 | frick-fsw | Record post-deferral local verification cycle and keep all authority/external/release gates open |
| 2.68 | 2026-10-16 | frick-fsw | Record in-progress S8-PUS-FU N7 adapter CRC/PDL correction, 42/42 host and ASan cycle, and controlled-SDK/emulation evidence limits |
| 2.69 | 2026-10-16 | frick-fsw | Record S3 provider-failure propagation, TS1-101, updated coverage and 42/42 cycle; retain S8-PUS-FU and independent-review gates open |
| 2.70 | 2026-10-16 | frick-fsw | Extend S3 TC oracle to provider and telemetry-encoding failures, refresh current evidence and coverage; no NC or sprint closure claimed |
| 2.71 | 2026-10-16 | frick-fsw | Record ADR-0019 default-N7 decision, actual generated-source host test, stale-cache cycle correction and current Renode/ASan evidence status; sprint remains incomplete |
| 2.72 | 2026-10-16 | frick-fsw | Record bounded R4/PA/V&V reviews 0338–0340, status reconciliation, and applied test/trace fixes pending delta review and fresh evidence cycle |
| 2.73 | 2026-10-16 | frick-fsw | Record full-batch reviews 0341–0343, S20 boundary test/CRC-oracle corrections, and the pending refreshed CI, candidate manifest and focused delta reviews |
| 2.74 | 2026-10-16 | frick-fsw | Record completed post-0343 local/ASan/auxiliary cycle, corrected current-cycle status and candidate-manifest identity pending PA acceptance |
| 2.75 | 2026-10-16 | frick-fsw | Reconcile R4 delta findings on review status and cycle counts; record bounded PA acceptance of enumerated product-set identity without phase/release approval |
| 2.76 | 2026-10-16 | frick-fsw | Record PA 0345 narrow closure of NC-EXT-83, leave NC-EXT-84 open, and update the active batch review/cycle status |
| 2.77 | 2026-10-16 | frick-fsw | Record exact generated-tree hash-list verification, reconcile NC-EXT-83/84 status and queue final R4 currentness delta |
| 2.78 | 2026-10-16 | frick-fsw | Mark pre-0343 42/42 PUS candidate narrative as historical and reconcile it with the current 43/43 post-0343 cycle and Renode evidence |
| 2.79 | 2026-10-16 | frick-fsw | Record PA 0346 generated-tree identity verification and R4 0347 narrow NC-EXT-85 closure; retain generator, authority, target and release gates |
| 2.80 | 2026-10-16 | frick-fsw | Record final R4 0347 NC-EXT-85 narrow disposition and update current status references; no overall N17 or phase approval |
| 2.81 | 2026-10-16 | frick-fsw | Record R1/R2 findings 0349/0350, S3 report-all first-error fix and new PUS test/trace deltas; full verification and delta reviews remain pending |
| 2.82 | 2026-10-16 | frick-fsw | Record focused PUS pass and retained compile/oracle attempts; update current candidate identity and full-cycle gates as pending |
| 2.83 | 2026-10-16 | frick-fsw | Record S8-PUS R1/R2/PA delta outcomes, S8-SCH R2 oracle corrections/re-review and NC-EXT-90 time-range residual; current cycle and R4 remain pending |
| 2.84 | 2026-10-16 | frick-fsw | Record R4 0357–0359 bounded documentation corrections and current CI-local state; auxiliary evidence and latest audit review remain pending |
| 2.85 | 2026-10-16 | frick-fsw | Record current candidate ASan/smoke/GSW/format/docs/Renode passes, host-tool failures and pending RTEMS cross-build/identity review |
| 2.86 | 2026-10-16 | frick-fsw | Record RTEMS 6.2 cross-build pass and 49-file candidate identity aid; preserve pending independent identity review, current CI rerun and target/hosted gates |
| 2.87 | 2026-10-16 | frick-fsw | Record PA 0361 bounded identity verification and latest CI-local identity-cycle pass; keep provenance, full review, authority, hosted and target gates open |
| 2.88 | 2026-10-16 | frick-fsw | Record R4 0362/0363 re-review of current audit/manifest evidence status and clarify current cycle and NC closure state |
| 2.89 | 2026-10-16 | frick-fsw | Record PA 0364 bounded final run-linkage review and current R4 gate for verification-results 2.82/status updates |
| 2.90 | 2026-10-16 | frick-fsw | Correct latest CI-local hash pointer per R4 0365; retain current bounded review/phase gates |
| 2.91 | 2026-10-16 | frick-fsw | Record R4 0366 confirmation of corrected final CI-local hash pointer; no aggregate N17/phase approval |
| 2.92 | 2026-10-16 | frick-fsw | Refresh immediate next steps after current reviews and local verification; record authority/target/OBCP blockers and prohibit commit/push pending gates |
| 2.93 | 2026-10-16 | frick-fsw | Record latest post-0366 CI-local log pointer and update verification-results revision 2.83 reference |
| 2.94 | 2026-10-16 | frick-fsw | Mark stale items 1–12 in §5 as historical/superseded per R4 0367 and direct resume to current item 15 |
| 2.95 | 2026-10-16 | frick-fsw | Record ADR-0020 provisional public-reference implementation boundaries and stale-vector failure correction; fresh full CI-local remains pending |
| 2.96 | 2026-10-16 | frick-fsw | Record corrected 43/43 CI-local/Lean/trace pass; clarify auxiliary cycle and candidate identity review remain pending |
| 2.97 | 2026-10-16 | frick-fsw | Reconcile R4 current-action and sequence-note findings; retain non-aggregate review and phase limits |
| 2.98 | 2026-10-16 | frick-fsw | Correct local-cycle leg effectivity and report current host/pinned formatting limitations, NC-EXT-91 and remaining auxiliary reruns |
| 2.99 | 2026-10-16 | frick-fsw | Record successful docs/Renode/RTEMS legs and corrected format recipe; retain pending recursive format/CI retry and exact-candidate identity gap |
| 3.00 | 2026-10-16 | frick-fsw | Record successful pinned format/CI retry, auxiliary gates, and PA narrow closure of NC-EXT-91; preserve broader identity, mission and release limits |
| 3.01 | 2026-10-16 | frick-fsw | Update current verification-results reference after PA narrow identity disposition; preserve broader candidate and release gates |
| 3.02 | 2026-10-16 | frick-fsw | Fix stale results revision pointer identified by scoped R4 review |
| 3.03 | 2026-10-16 | frick-fsw | Point current-cycle summary to post-reference CI-local rerun and verification-results 2.88 |
| 3.04 | 2026-10-16 | frick-fsw | Point current-cycle summary to post-loop-audit CI-local rerun and verification-results 2.89 |
| 3.05 | 2026-10-17 | frick-fsw | Process the four reconciliation reviews (0370 R2, 0371 R3, 0372 R4, 0373 PA), record ICD rev 1.4 provisional S3/S6 profile, NC-EXT-93, the corrected TS-1/TS-5/TS-6 §3 lists/histories and the sprint-plan 1.69 counts, and point the current-cycle summary to the 2026-10-17 reconciliation logs, the refreshed candidate manifest and verification-results 2.90 |
| 3.06 | 2026-10-18 | frick-fsw | Documentation-vs-implementation audit (N21 non-functional only): refresh the §4 committed-baseline fact to the pushed S8 batch `1fdbb83` and correct the flagged §4 hosted-CI wording; see `docs/reviews/2026-10-18-docs-implementation-alignment.md`. No NC closed and no gate, phase or release approval implied |
