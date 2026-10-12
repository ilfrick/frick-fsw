# Handover / Current Context (resume point)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-HANDOVER-001                     |
| Revision                          | 1.14                                   |
| Date                              | 2026-10-12                             |
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
# Host build + unit tests (20/20) + trace + format
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build build/native
ctest --test-dir build/native --output-on-failure
python3 tools/scripts/trace.py docs/requirements/traceability.yaml   # PASS 100/59/27/14/51
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

## 4. Current state (as of commit `3822f72` + the Sprint-6b-1 (lwIP RTEMS sys_arch port) increment; norm N20 reviews in `docs/reviews/`)

- **M1..M11 delivered/partial** (see `docs/roadmap.md` / `docs/development-plan.md`).
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
- **M11 in progress (Renode HAL coverage, OBJ-19):** **10/13 peripheral-bound**
  classes done — **GPIO** (`ff_gpio_mpfs`, SRS-REQ-047), **SERIAL**
  (`ff_serial_mpfs`, SRS-REQ-048), **MEM** (`ff_mem_static`, SRS-REQ-049),
  **ETH** (Cadence GEM **identification** via the read-only Module ID,
  SRS-REQ-036), **NVM** (`ff_nvm_mpfs`, SRS-REQ-055; eNVM data write/read check),
  **CAN** (`ff_can_mpfs`, SRS-REQ-057; MMIO register round trip), **I2C**
  (`ff_i2c_mpfs`, SRS-REQ-058; MMIO register round trip), **SPI**
  (`ff_spi_mpfs`, SRS-REQ-069; MMIO register round trip) and **BLOCK**
  (`ff_block_mpfs`, SRS-REQ-070; MMIO register round trip) and **TSN** (GEM
  native TSN backend on the emulated Cadence GEM, SRS-REQ-038; init + VLAN/PCP +
  802.1AS TSU time). Additionally
  a **NET `FF_NetStack_t` contract check** through the reference loopback stack
  (SRS-REQ-039; in-process — **not** peripheral-bound).
  GPIO/SERIAL/MEM/NVM/CAN/I2C/SPI/BLOCK go through their HAL backends; the ETH check is an MMIO
  identification (full GEM driver/DMA path host-tested, `test_gem`). Renode
  checks: `hal_gpio`/`hal_serial`/`hal_mem`/`hal_eth`/`hal_net`/`hal_nvm`/`hal_can`/`hal_i2c`/`hal_spi`/`hal_block`/`hal_tsn=PASS`. Remaining
  M11 peripheral classes: NET(lwIP-on-GEM, **NC-S6-01**, sprint S6b)/PCIE/SPACEWIRE.
- **Project codename "Eärendil"**: third-party-name (Tolkien) non-affiliation
  disclaimer in `NOTICE`, `README.md`, `docs/quality/licensing.md §6`.
- **M4-residual MSS drivers**: GEM Ethernet (+TSN), MSS GPIO, MSS MMUART, the
  static RAM pool/DMA helper, the eNVM NVM backend, the NOR/QSPI MTD backend, the
  MSS CAN driver and the MSS I2C driver so far.
- **M12** (OBCP engine, uPython/MicroPython; candidate `n7space/n7s-obcp`, MIT,
  ESA), **M13** (reference mission services: equipment manager + fault
  management), **M14** (multicore/SMP), **M15** (in-house fundamental PUS-C
  service set + CFDP + virtual filestore), **M16** (in-house OBCP engine
  development) are **planned**.
- Requirements: **114 (71 srs / 29 sss / 14 gap)**; Lean 51 theorems.
- Recent reviews: FRICK-REVIEW-0040 (real RTEMS+Renode), 0041 (M12/M13 plan),
  0042 (GPIO), 0043 (SERIAL), 0044 (MEM), 0045 (PUS-C ASN.1), 0046 (README/docs),
  0101..0105 (NVM/MTD + board repl), **0106..0119** (M4/M11 MSS CAN: R1/R2/R4
  initial + delta + closure reviews and the PA freeze sign-off), **0120..0128**
  (M10 QEMU closure: R1/R2/R4/PA initial, delta and final closure) and
  **0129..0136** (Sprint-3 M4/M11 MSS I2C: R1/R2/R3/R4/PA initial + R1/R2 delta +
  PA delta; PA converted REJECTED -> APPROVED WITH COMMENTS, loop closed) — all
  closed (see `docs/reviews/README.md`).

## 5. Immediate next steps (priority order)

> Execution view: `docs/planning/sprint-plan.md` (FRICK-PLAN-0002). S1 is complete
> (NVM + NOR/QSPI MTD + MSS CAN done, **M10 closed via the QEMU run**); S2 (MSS CAN)
> done; **S3 (MSS I2C) delivered** (`hal_i2c=PASS`, SRS-REQ-058), **S4 (MSS SPI)
> delivered** (`hal_spi=PASS`, SRS-REQ-069), **S5 (MSS block device) delivered**
> (`hal_block=PASS`, SRS-REQ-070) and **S6a (GEM TSN peripheral-bound) delivered**
> (`hal_tsn=PASS`, M11 10/13, ctest 22/22). Next per plan: **S6b — NET lwIP-on-GEM
> peripheral-bound** (**NC-S6-01**, deferred) and **S7 — PCIe + SpaceWire**, then
> the **restored M4 scope**: S4b (PUS-C over IP/TSN, SRS-REQ-059/060/061), S4c
> (CAN-FD/CANopen/RS-485 DE, SRS-REQ-062/063/064) and S4d (PUSopen(R) adapter,
> SRS-REQ-065).

1. **M11 next HAL class** (M4-residual driver + Renode check + requirement +
   review). Suggested order by ease of emulated validation:
   - [done: NVM (SRS-REQ-055), NOR/QSPI MTD driver (SRS-REQ-056), MSS CAN (SRS-REQ-057), MSS I2C (SRS-REQ-058), MSS SPI (SRS-REQ-069), MSS block device (SRS-REQ-070); board repl loaded; on-SoC QSPI check pending the controller command driver]; remaining:
   - **PCIe**, **SpaceWire**, **ETH/NET/TSN** (instantiate the existing GEM/lwIP backends).
   For each: driver behind `hal_*.h`, host unit test with an independent mock,
   a binding in `frick-renode.resc`, a clause in `app_renode_test.c`, SRS req,
   trace, docs, review, commit.
2. **M4 residual** MSS drivers as above.
3. **M8** gPTP (802.1AS) + fabric CoreTSN.
4. **M12** OBCP engine; **M13** mission services.
5. Keep N18 blocking (nightly real run) and close GAP-A4 (certified MISRA checker)
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
