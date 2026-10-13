# Handover / Current Context (resume point)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-HANDOVER-001                     |
| Revision                          | 1.20                                   |
| Date                              | 2026-10-13                             |
| Norms                             | N8 (entry point), N16 (history)        |

This document is the **authoritative resume point** for an agent picking up the
work without prior conversation context. It summarises objectives, rules, how to
build/test/verify, milestones and the immediate next steps. It is updated on
every significant increment (norm N8/N16).

## 0. How to resume in a new session

Start a fresh session in the repository root and use a kickoff prompt like:

> "Read `AGENTS.md`, `docs/rules.md` and `docs/status/handover.md`; then continue
> the frick-fsw roadmap from the handover's §5 next steps, respecting norms N1-N20
> (full cycle + independent review + trace + Renode N18), and keep pushing to
> both remotes."

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
(housefz.com/git/ilfrick/frick-fsw). Branch: **main only**. Push every cycle
(norm N7). Credentials in `/home/nicola/.netrc` (+ `gh` helper).

## 2. Non-derogable rules

`docs/rules.md` (N1-N20). Key gates before every commit/push:

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
# Host build + unit tests (27/27) + trace + format
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build build/native
ctest --test-dir build/native --output-on-failure
python3 tools/scripts/trace.py docs/requirements/traceability.yaml   # PASS 114/71/29/14/51
docker run --rm -v "$PWD:/workspace" -w /workspace frick-vv:local \
  bash -lc 'find src config tests -name "*.[ch]" -print0 | xargs -0 -n1 clang-format --dry-run --Werror'

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

## 4. Current state (as of commit `64961ac` + NET follow-up + S8/M8 gPTP + Qci (SRS-REQ-068/066); norm N20 reviews in `docs/reviews/`)

- **M1..M7, M8, M10 delivered; M9, M11(11/13), M12-M16 planned; S4b delivered** (see `docs/roadmap.md` / `docs/development-plan.md`).
- **N18 in force**: open RTEMS 6.2 toolchain built and validated; real cross
  build OK; **Renode functional run of the RTEMS image PASS**
  (`RENODE_TEST: PASS`). Boot: U54_1 at entry `0x1000000000`, console `mmuart1`.
- **M10 done (emulated-target verification, OBJ-18):** Renode real run PASS plus a
  **QEMU run** of the generic `riscv/rv64imafdc` RTEMS image
  (`make qemu-image` + `make qemu`; tool T20 `tools/scripts/qemu_vv.sh`; console
  `build/rtems-generic/qemu-console.log`) — banner + 6 applications registered on
  `qemu-system-riscv64 -M virt`. The toolchain image (`tools/docker/rtems`, T18)
  now also builds the `rv64imafdc` BSP and carries QEMU.
- **PUS-C ASN.1 default (SRS-REQ-050, ADR-0016):** `FF_PusBackendN7` (N7/ESA
  `asn1-pusc-lib`, ASN1SCC **build-time** generation, `make pus-asn1`, T19) is the
  default PUS backend (`FF_USE_PUS_ASN1` ON; fallback to open backend with a
  warning when the generated code is absent).
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
- **M4-residual MSS drivers**: GEM Ethernet (+TSN), MSS GPIO, MSS MMUART, the
  static RAM pool/DMA helper, the eNVM NVM backend, the NOR/QSPI MTD backend, the
  MSS CAN driver and the MSS I2C driver so far. **Restored M4 scope** (S4b/S4c/S4d):
  PUS-C over IP/TSN (SRS-REQ-059/060/061), CAN-FD/CANopen/RS-485 DE
  (SRS-REQ-062/063/064), PUSopen(R) adapter (SRS-REQ-065), fabric contract
  (SRS-REQ-067); M8 gPTP/CoreTSN (SRS-REQ-068) + GEM TSN Qav/Qci/Qbb
  (SRS-REQ-066). **S4b delivered** (see below).
- **S4b delivered (PUS-C over IP/TSN, SRS-REQ-059/060/061):** segmentation/\n  reassembly layer `link_seg.c/h` (2-byte BE length-prefix, allocation-free);\n  IP `FF_Link_t` backend `link_ip.c/h` (UDP default + TCP with seg) over\n  `FF_NetStack_t`; TSN `FF_Link_t` backend `link_tsn.c/h` (EtherType 0x88F5,\n  callback-driven RX ring) over `FF_EthMac_t + FF_TsnBackend_t`. ctest 27/27\n  green (test_link_ip 19 test functions, test_link_tsn 14 test functions); Lean PASS;\n  smoke + GSW PASS; Renode SKIP (host-only code). `-Wall -Wextra -Werror` clean\n  on authored files (N9/M9 quality gate). Reference loopback models both UDP\n  and TCP (connect best-effort). `link.h` updated to declare new backends.\n  Scope: on-target link-level = GAP-A5 (no Renode image change).\n  Delta re-review: R1+R2 (FRICK-REVIEW-0165-D1) + PA (FRICK-REVIEW-0166-D1) both\n  APPROVED; all findings from initial REJECTED reviews resolved (2 Major,\n  10 Minor, 6 V&V gaps, 4 PA Minor, 3 Comments + NF-1/NF-2/NF-3 closed).\n- **M8** (S8 complete) gPTP (802.1AS) + fabric CoreTSN (SRS-REQ-068) — **next** after\n  S4b: documentation re-review (FRICK-REVIEW-0164) for the integration manual\n  R4 closure. **M12** (OBCP engine, uPython/MicroPython; candidate
  `n7space/n7s-obcp`, MIT, ESA), **M13** (reference mission services: equipment
  manager + fault management), **M14** (multicore/SMP), **M15** (in-house
  fundamental PUS-C service set + CFDP + virtual filestore), **M16** (in-house
  OBCP engine development) are **planned** (S13-S14, S11-S12, S16-S17, S18-S25,
  S9-S10 respectively per the sprint plan).
- Requirements: **114 (71 srs / 29 sss / 14 gap)**; Lean 51 theorems.
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
  findings (34 NCs). S8 resolves all Blocker/Major/Minor issues; the R4 review
  loop will close after the next documentation re-review (FRICK-REVIEW-0164 covering
  rev 0.7 + rev 0.8 / S4b changes). NCs logged in
  `docs/nc/2026-10-13-integration-manual-fix-nc.md`. **FRICK-REVIEW-0165** (S4b
  R1+R2, initial — REJECTED, 2 Major + 10 Minor + 6 V&V gaps) and
  **FRICK-REVIEW-0166** (S4b PA, initial — REJECTED, same 2 Major + 10 Minor +
  6 V&V gaps + 4 PA Minor) both closed: delta re-reviews **FRICK-REVIEW-0165-D1**
  (APPROVED) and **FRICK-REVIEW-0166-D1** (APPROVED) resolve all findings including
  3 new findings (NF-1/NF-2/NF-3) raised and closed in the delta.

## 5. Immediate next steps (priority order)

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
> **S4b delivered** (SRS-REQ-059/060/061, ctest 27/27 + Lean + smoke + GSW PASS;
> `-Wall -Wextra -Werror` clean; R1+R2 delta (FRICK-REVIEW-0165-D1) APPROVED,
> PA delta (FRICK-REVIEW-0166-D1) APPROVED, all findings closed);
> Renode SKIP). Next per plan: **S4c** (CAN-FD/CANopen/RS-485 DE, SRS-REQ-062/063/064)
> and **S4d** (PUSopen(R) adapter, SRS-REQ-065).

1. **S8/M8 gPTP + Qci completed** (SRS-REQ-068/066). gPTP stack: BMCA,
   Sync/Follow_Up, PDelay, PI servo (`test_gptp` 58/58 host) — integrates with GEM TSU
   `get_time_ns` + `FF_EthMac_t`; on-Renode GEM TSU register accessible (host-tested
   mock-MAC frame exchange; full GEM DMA = GAP-A5). Qci: screener type 1 entry per
   traffic-class; `test_gem_tsn` 22/22. Qav CBS + Qbb PFC = `FF_NOT_IMPLEMENTED`
   (NC-S8-01/NC-S8-02). Integration-manual R1/R4 reviews (FRICK-REVIEW-0159/0160)
   corrected; documentation re-review pending.
2. **M4 residual** restored M4 scope (S4b/S4c/S4d): PUS-C over IP/TSN
   (SRS-REQ-059/060/061), CAN-FD/CANopen/RS-485 DE (SRS-REQ-062/063/064),
   PUSopen adapter (SRS-REQ-065), fabric contract (SRS-REQ-067).
3. **M12** OBCP engine; **M13** mission services; **M14** multicore/SMP;
   **M15** in-house PUS-C + CFDP + VFS; **M16** in-house OBCP engine.
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
