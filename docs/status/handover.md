# Handover / Current Context (resume point)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-HANDOVER-001                     |
| Revision                          | 0.7                                    |
| Date                              | 2026-10-11                             |
| Norms                             | N8 (entry point), N16 (history)        |

This document is the **authoritative resume point** for an agent picking up the
work without prior conversation context. It summarises objectives, rules, how to
build/test/verify, milestones and the immediate next steps. It is updated on
every significant increment (norm N8/N16).

## 0. How to resume in a new session

Start a fresh session in the repository root and use a kickoff prompt like:

> "Read `AGENTS.md`, `docs/rules.md` and `docs/status/handover.md`; then continue
> the frick-fsw roadmap from the handover's §5 next steps, respecting norms N1-N18
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

`docs/rules.md` (N1-N18). Key gates before every commit/push:

1. `make build` + `make test` green (native).
2. `bash tools/scripts/trace.py docs/requirements/traceability.yaml` PASS.
3. Lean: `lake build` in `verification/lean` (host lacks lake; run in the V&V
   image or skip with note).
4. `clang-format --dry-run --Werror` over `src config tests`.
5. **N18**: functional validation in **Renode** must pass — `make renode`
   (blocking when `build/renode/frick_fsw_renode.elf` exists; per-commit SKIP
   proxy otherwise; nightly does the real run).
6. N9/N12: independent review (R1/R2/R3/R4 as applicable) + NC closure; reports
   under `docs/reviews/` and logged in `docs/reviews/README.md`.
7. N8/N16: update all affected docs + revision histories.
8. N17: documentation review (R4).

## 3. How to build / test / verify (exact commands)

```bash
# Host build + unit tests (12/12) + trace + format
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build build/native
ctest --test-dir build/native --output-on-failure
python3 tools/scripts/trace.py docs/requirements/traceability.yaml   # PASS 85/48/23/14/51
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

## 4. Current state (as of commit 2fbdeec)

- **M1..M11 delivered/partial** (see `docs/roadmap.md` / `docs/development-plan.md`).
- **N18 in force**: open RTEMS 6.2 toolchain built and validated; real cross
  build OK; **Renode functional run of the RTEMS image PASS**
  (`RENODE_TEST: PASS`). Boot: U54_1 at entry `0x1000000000`, console `mmuart1`.
- **PUS-C ASN.1 default (SRS-REQ-050, ADR-0016):** `FF_PusBackendN7` (N7/ESA
  `asn1-pusc-lib`, ASN1SCC **build-time** generation, `make pus-asn1`, T19) is the
  default PUS backend (`FF_USE_PUS_ASN1` ON; fallback to open backend with a
  warning when the generated code is absent).
- **M11 in progress (Renode HAL coverage, OBJ-19):** three classes done — **GPIO**
  (`ff_gpio_mpfs`, SRS-REQ-047), **SERIAL** (`ff_serial_mpfs`, SRS-REQ-048) and
  **MEM** (`ff_mem_static`, SRS-REQ-049), each with an independent host test and a
  Renode check (`hal_gpio=PASS`, `hal_serial=PASS`, `hal_mem=PASS`) folded into
  the marker. Remaining M11 classes: ETH, NET/TCP-IP, TSN, CAN, SPI, I2C, PCIE,
  SPACEWIRE, BLOCK, NVM.
- **Project codename "Eärendil"**: third-party-name (Tolkien) non-affiliation
  disclaimer in `NOTICE`, `README.md`, `docs/quality/licensing.md §6`.
- **M4-residual MSS drivers**: GEM Ethernet (+TSN), MSS GPIO, MSS MMUART and the
  static RAM pool/DMA helper so far.
- **M12** (OBCP engine, uPython/MicroPython; candidate `n7space/n7s-obcp`, MIT,
  ESA), **M13** (reference mission services: equipment manager + fault
  management), **M14** (multicore/SMP), **M15** (in-house fundamental PUS-C
  service set + CFDP + virtual filestore) are **planned**.
- Requirements: **94 (54 srs / 26 sss / 14 gap)**; Lean 51 theorems.
- Recent reviews: FRICK-REVIEW-0040 (real RTEMS+Renode), 0041 (M12/M13 plan),
  0042 (GPIO), 0043 (SERIAL), 0044 (MEM), 0045 (PUS-C ASN.1), 0046 (README/docs)
  — all closed.

## 5. Immediate next steps (priority order)

1. **M11 next HAL class** (M4-residual driver + Renode check + requirement +
   review). Suggested order by ease of emulated validation:
   - **NVM (eNVM)** or **MEM/NOR MTD** via `hal_mem.h` (memory-backed, robust);
   - **SERIAL (MMUART/RS-422/485)** via `hal_serial.h` (NS16550 scratch reg);
   - **CAN** (`CAN.MPFS_CAN`), **SPI** (`SPI.MPFS_SPI`), **I2C** (loopback made
     explicit in `frick-renode.resc`), **BLOCK (eMMC/SD)**, **PCIe**,
     **SpaceWire**, **ETH/NET/TSN** (instantiate the existing GEM/lwIP backends).
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
