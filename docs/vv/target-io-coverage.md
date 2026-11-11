# Target I/O coverage matrix (increments T1–T3 and XW-2/XW-3)

|                        |                                                    |
| ---------------------- | -------------------------------------------------- |
| Document id            | FRICK-VV-TGTIO-001                                 |
| Revision               | 0.8                                                |
| Date                   | 2026-10-26                                         |
| Applies to             | incr. T1 (`target_req_verify.py`, `qemu_req_vv.sh`, `src/apps/app_req_test.c`, `fsw_main_reqtest.c`), T2/T3 (`target_run_verdict.py`, `qemu_uart_tc_vv.sh`, `uart_tm_verify.py`), XW-2/XW-3 (`qemu_xw2_vv.sh`, `qemu_xw3_vv.sh`, `xw3_verdict.py`) |
| Source of the inventory| `ASSESSMENT.md` §5 (target I/O assessment, 2026-10-25) |

## 1. What this document is, and what it is not

This is a **durable inventory of every I/O path** the framework contains, with
the honest status of what the T1 run does and does not exercise. It exists so
that "the framework supports I/O X" is never silently converted into "I/O X was
tested on target".

**T1 does not satisfy norm N18. T1 is not all-I/O coverage.** T1 runs the
*logical* chain (CCSDS framing/CRC → PUS service dispatch → CCSDS telemetry) in
an in-image self-test on a generic RTEMS image under QEMU. The suite runs in a
**test-only pre-production-init PIL harness** (`fsw_main_reqtest.c`, before
`FF_EsStartAll()` and therefore before watchdog init), and the image then aborts
at watchdog init: **the whole target run is FAIL** (RTEMS fatal exit 5) while
the component cases pass. Nothing in this document is a production
functional-boot result. QEMU `virt` has **no
MPFS peripherals**: no GPIO, GEM, CAN, I2C, SPI, MTD/QSPI, SD/block, PCIe,
SpaceWire, PPS or watchdog model. Any peripheral row below is at best
`UNMODELLED` or `BLOCKED`; none is `TESTED`.

Status vocabulary:

| Status | Meaning |
| ------ | ------- |
| `TESTED-LOGICAL` | Asserted by a T1 case on target, with exact wire-byte oracles, in the logical chain only |
| `HOST-ONLY` | Covered by host unit/integration tests; no target evidence |
| `NOT TESTED` | Reachable code with no executing test at any level |
| `UNMODELLED` | The emulated environment has no model, so the path cannot be exercised there |
| `BLOCKED` | Cannot be exercised without an authority decision or a missing controlled input |
| `INFRASTRUCTURE READY / BLOCKED` | The test infrastructure exists and has been executed, but the path cannot complete because of a blocking condition (here: the watchdog-init abort). Never a PASS |
| `HOST CONTRACT / MODEL-VALIDATED` | The software contract is exercised on the host against an independently validated abstract model, with the production API unchanged. Proves the contract only: no device, no hardware reset, no target, no N18 |
| `EMULATED-TARGET (abstract contract, test-only image)` | The same contract runs on the RTEMS/QEMU image with the three device hooks link-substituted by the validated model. Proves the contract on the RTEMS build only: still no device, no real reset, no HIL, no N18, no timing |
| `EMULATED-TARGET (real external TC/TM, test-only image)` | Real telecommand octets cross the QEMU serial device into the unchanged link/ingest/PUS path and the captured telemetry is verified by an independent oracle. Proves the external byte path on the emulated target only: still no device, no HIL, no N18, no timing, no mission profile |

## 2. Logical / protocol I/O

| I/O | Module | Ingress | Egress | Production wiring | T1 status | Requirements |
| --- | ------ | ------- | ------ | ----------------- | --------- | ------------ |
| Software Bus | `src/core/sb/sb.{c,h}` | `FF_SbPublish`, subscribe queues | `FF_SbReceive` | Yes (apps) | `HOST-ONLY` (`test_sb`, `test_sb_stress`, `test_sb_sweep`, `test_r2_gap`) | SRS-REQ-001, SSS-REQ-002 |
| Event Services | `src/core/evs/evs.{c,h}` | `FF_EvsSend` | listener → PUS S5 | Yes | `HOST-ONLY` (`test_es_hs_lifecycle`); T1 emits events but asserts no event ID (NC-EXT-102) | SRS-REQ-013 (S5 path) |
| Executive Services | `src/core/es/es.{c,h}` | `FF_EsRegisterApp` / start | app table | Yes | `NOT TESTED` as an oracle: the boot log shows the app count, which is not a requirement assertion | SRS-REQ-002 |
| Time / CUC | `src/core/time/time.c` | `FF_TimeSetUtcNs`, tone | `FF_TimeGetCuc`, `FF_TimeGetMetNs` | Yes (time); **HW PPS no** | `TESTED-LOGICAL` at **coarse-second resolution** (L7: S9.1 set → S9.2 read-back of the set coarse value; the fine field carries elapsed time and is an observation; the case saves and restores both the UTC value and the time source) | SRS-REQ-010, SRS-REQ-021..024 (host) |
| Hardware PPS | `src/core/time/pps.c`, `board.c` | tone input | time discipline | **No** (not registered) | `UNMODELLED` | SRS-REQ-022 (blocked) |
| Scheduler | `src/core/sched/ff_sched.c` | table-driven ticks | SB releases | Yes | `HOST-ONLY` (`test_sched`); no cadence/timing oracle on target | SRS-REQ-020 (host) |
| PUS core | `src/pus/pus.c` | `FF_PusIngest` | `FF_PusSendTm`, S1 | Yes | `TESTED-LOGICAL` (L1–L5, L10) | SRS-REQ-005, -006, -007 |
| PUS services | `src/pus/pus_services.c` | TC 3, 6, 8, 9, 11, 17, 20 | TM 3,25; 6,6; 9,2; 17,2; 20,4; S1 | Yes | `TESTED-LOGICAL` for S17 (L1, L6, L10), S6 dump (L8), S9 (L7, coarse). **S3 one-shot is a GAP (L9, NC-EXT-17: the 6-byte TC[3,1] create is an unapproved extension).** S1 responses are counted by the **PUS TM header** only; their payload prefix and numeric codes are observations (NC-EXT-14/15) | SRS-REQ-003, -008 (GAP), -009, -010, -012 |
| CCSDS codec | `src/transport/ccsds/ccsds.c` | `FF_CcsdsDecodeTc/Tm` | `FF_CcsdsEncodeTc/Tm`, CRC | Yes | `TESTED-LOGICAL` — **target evidence is self-consistency** (the same codec builds and parses the frames; header fields, counter order, PDL and CRC are asserted at fixed offsets, with a mutated-frame negative control). The *independent* cross-check is a **host** Python fixture that decodes the GSW golden TC, not a target check | SRS-REQ-003, -004 |
| Link core | `src/transport/link/link.c` | `FF_LinkRecv` | `FF_LinkSend`, `SendUntil` | Yes | `HOST-ONLY` (link tests) | SRS-REQ-043 (host) |
| Link UART | `link_uart.c` | `/dev/console` RX, PDL extract | `write()` | Yes (default) | `INFRASTRUCTURE READY / BLOCKED (watchdog)` on the **flight-shaped** image: the external path is implemented and executed (`tools/scripts/qemu_uart_tc_vv.sh` + `uart_tm_verify.py`); on the real serial interface **15 CRC-valid TM packets were captured and decoded** (S5.1=14, S5.3=1), but the TC injected after boot never produced S1/S17.2 because the image aborts at watchdog init (T2; **not a target PASS**). **`EMULATED-TARGET (real external TC/TM, test-only image)`**: on the accepted XW-2 test-only image (watchdog device seam substituted) the golden TC written to the QEMU serial input is read from `/dev/console` by the unchanged backend, dispatched by the unchanged `CMD_INGEST`/PUS path, and answered with exactly one S1.1, S1.3, S1.7 and one S17.2 carrying the injected user data; a corrupt-CRC frame produces no S17.2 and no S1 acknowledgement (XW-3 run `build/xw3-run-20261026T104750Z`; 15/15 negative controls). Proves the external byte path on the emulated target only — not a device, not HIL, not N18, no timing. T2's `-nographic` stimulus limitation is NC-EXT-223 (open, outside XW-3 scope; exact dropped-octet count withdrawn as unretained). | SRS-REQ-004/005/006/007/012, SRS-REQ-017 (interface) |
| Link loopback | `link_loopback.c` | in-process inject | in-process capture | Host default | `NOT TESTED` on target | — |
| Link IP | `link_ip.c` | UDP/TCP + length | same | Selectable | `HOST-ONLY` (`test_link_ip`) | SRS-REQ-060 |
| Link TSN | `link_tsn.c` | EtherType `0x88F5` | same | Selectable | `HOST-ONLY` (`test_link_tsn`) | SRS-REQ-061 |
| SDLS | `src/security/ff_sdls.c` | unprotect | protect | **No caller** | `NOT TESTED` (host contract tests only) | SRS-REQ-035 (host model) |
| OBCP | `src/obcp/ff_obcp.c` | `ExecutePy` | status | **No caller** | `HOST-ONLY` | — |
| HAL registry | `src/hal/hal.c` | register/get | — | Yes | `HOST-ONLY` (`test_hal`) | SRS-REQ-025 (host) |
| Ground station | `tools/gsw/ff_gsw.py` | golden TC, serial injection | TM decode | Ground tool | `HOST-ONLY`; its golden TC is the independent framing cross-check for T1 | SRS-REQ-017 |

## 3. Hardware / PAL I/O and reference drivers

No row in this table is `TESTED` by T1. QEMU `virt` carries no MPFS model; the
Renode MPFS path is currently aborted at watchdog init (NC-EXT-198).

| I/O | Module | Production wiring | T1 status | Blocking reason / model limit | Requirements |
| --- | ------ | ----------------- | --------- | ----------------------------- | ------------ |
| PAL tasks/mutex/queue/time | `pal_posix.c`, `pal_rtems.c` | Yes | `BOOT-ONLY` (the image boots and starts tasks; **no T1 case asserts a PAL property**, so this row is not a requirement oracle) | RTEMS timing not measured | SRS-REQ-001/002 (host) |
| Hardware watchdog | `pal_watchdog.c`, `pal_rtems.c` | Hooks return `FF_NOT_IMPLEMENTED` | `HOST CONTRACT / MODEL-VALIDATED` (XW-1) · **`EMULATED-TARGET (abstract contract, test-only image): exercised`** (XW-2: 13 contract cases on the RTEMS/QEMU image with the device seam substituted by the validated model; not a device, not HIL, not N18) · internal MPFS gate still `BLOCKED` | NC-PAL-01/NC-EXT-198: binding not wired; **both** the MPFS Renode run and the generic QEMU run abort at watchdog init (`[FF-PANIC] watchdog: init failed`, RTEMS fatal exit 5). The T1 suite is executed before that init in the test-only harness, so its component result is pre-safety-init | SRS-REQ-073 (blocked) |
| Board init | `src/bsp/mpfs/board.c` | Yes but stub | `NOT TESTED` | Log-only stub; no functional oracle | — |
| GPIO | `hal_gpio.h`, `ff_gpio_mpfs` | Test app only | `UNMODELLED` | QEMU has no MSS GPIO | SRS-REQ-030 (host) |
| Serial MMUART | `hal_serial.h`, `ff_serial_mpfs` | Test app only | `UNMODELLED` | No MMUART model; no RX injection | SRS-REQ-044 (host) |
| Static RAM / DMA | `hal_mem.h`, `ff_mem_static` | Test app only | `UNMODELLED` | Cache hooks are a no-op seam | SRS-REQ-045 (host) |
| GEM Ethernet (+ DMA) | `hal_eth.h`, `ff_gem` | Test app only | `UNMODELLED` | Register-only model; **no DMA/TX/RX** | SRS-REQ-031 (host) |
| TSN (Qci/Qav, TSU) | `hal_tsn.h`, `ff_gem_tsn` | Test app only | `UNMODELLED` | Qci/Qav/VLAN VID not fully modelled | SRS-REQ-038, -061 (host) |
| gPTP | `ff_gptp` | No | `UNMODELLED` | Host mock MAC/TSU only | — |
| Net stack (loopback) | `hal_net.h`, `ff_net_loopback` | Test app contract | `UNMODELLED` | In-process contract, not a peripheral | SRS-REQ-060 (host) |
| lwIP | `hal/net/lwip/*` | Test app only | `UNMODELLED` | No packet path without GEM DMA | SRS-REQ-032 (host) |
| CAN | `hal_can.h`, `ff_can_mpfs` | Test app only | `UNMODELLED` | Register self-test only; no medium | SRS-REQ-057 (host) |
| CAN-FD fabric / CANopen | `ff_canfd_fabric`, `ff_canopen` | No | `UNMODELLED` | Contract only; no peripheral | — |
| I2C | `hal_i2c.h`, `ff_i2c_mpfs` | Test app only | `UNMODELLED` | No slave model | SRS-REQ-058 (host) |
| SPI | `hal_spi.h`, `ff_spi_mpfs` | Test app only | `UNMODELLED` | No slave; transfer cannot run | SRS-REQ-069 (host) |
| MTD / QSPI NOR | `ff_mtd_mpfs` | **No binding** | `UNMODELLED` | No controller/driver path in the application; no model bound | SRS-REQ-056 (host) |
| Block / SD | `ff_block_mpfs` | Test app only | `UNMODELLED` | Register + capability bits only; no card | SRS-REQ-070 (host) |
| PCIe | `ff_pcie_mpfs` | **No** | `BLOCKED` | Root complex not instantiated in the emulated board; the legacy check returned `true` unconditionally (ASSESSMENT F1) | SRS-REQ-033 (host) |
| SpaceWire | `ff_spacewire_mpfs` | Test app only (mock) | `UNMODELLED` | Mock contract; **no model** | SRS-REQ-034 (host) |
| NVM / eNVM | `ff_nvm_mpfs` | Test app only | `UNMODELLED` | Scratch byte + slot table model only | SRS-REQ-055 (host) |
| Fabric FIC/AXI | `ff_fabric_mpfs` | No | `UNMODELLED` | No fabric model | — |
| RS-485 DE/RE | `ff_serial_mpfs` | No board line | `UNMODELLED` | Board-specific line not modelled | — |

### 3.1 Test-only state left registered (R3)

The suite registers memory region 1 (needed by L8) and takes over the PUS TM
sink for the duration of the run, handing it back to the production
telemetry-output sink afterwards. There is **no unregister API** for a memory
region, so region 1 remains registered in the test-only image. This is
documented leakage of a test-only image; no production image is affected (the
option is OFF by default).

## 4. What T1 adds to this matrix, precisely

- `TESTED-LOGICAL` rows above are asserted by the nine required T1 cases
  (L1–L8, L10) with exact oracles: SAR/APID/sequence-counter/PDL/CRC on the wire bytes, exact PUS
  payloads where the requirement fixes them, and handler-status distinctions
  where the requirement is a contract rather than a wire format.
- Three sub-aspects are **declared gaps, not coverage**: S1.8 reporting for a
  malformed TC (NC-EXT-04), the on-wire refusal failure code for service 6
  (NC-EXT-18) and the S3 one-shot HK report whose create command uses the
  unapproved 6-byte TC[3,1] extension (NC-EXT-17).
- No peripheral row changes status as a result of T1. The Renode MPFS run that
  would be required for those rows remains FAIL (NC-EXT-198) and the image is
  absent (ASSESSMENT D6).

## 5. Non-claims

T1 does **not**:

- satisfy norm N18, nor replace the Renode MPFS functional validation;
- provide peripheral, driver, DMA, bus or timing/WCET evidence;
- qualify the `ff_gsw.py` ground segment or any ground interface;
- remove the need for the human decisions D1–D8 listed in `ASSESSMENT.md` §11;
- assert S5 event identifiers, unresolved S1 failure-code allocation, or the
  periodic HK cadence.

## History

| Rev | Date | Author | Change |
| --- | ---- | ------ | ------ |
| 0.8 | 2026-10-26 | frick-fsw | XW-3 finishing: add the NC-EXT-223 T2 `-nographic` stimulus limitation and the withdrawn unretained dropped-octet count to the `Link UART` row; no coverage status or non-claim changed |
| 0.7 | 2026-10-26 | frick-fsw | XW-3 review corrections (record currency): cite the exact-final run `build/xw3-run-20261026T104750Z` and 15/15 negative controls (the 0.6 text cited an earlier run and 14 controls); no row status, coverage or non-claim changed |
| 0.6 | 2026-10-26 | frick-fsw | XW-3: the `Link UART` row gains `EMULATED-TARGET (real external TC/TM, test-only image)` — golden TC over the QEMU serial input, exact S1.1/S1.3/S1.7/S17.2 response set verified by an independent oracle, corrupt-CRC rejection, 14/14 negative controls; the T2 flight-shaped result is unchanged (watchdog abort, BLOCKED), every peripheral row and the MPFS gate stay `BLOCKED`, and no device/HIL/N18/all-I/O claim is made |
| 0.5 | 2026-10-26 | frick-fsw | XW-2 (owner-approved, test-only): the watchdog row gains `EMULATED-TARGET (abstract contract, test-only image): exercised` — 13 contract cases run on the RTEMS/QEMU image with the three device hooks substituted by the validated model via `-Wl,--wrap`; the flight backend, every peripheral row and the internal MPFS gate are unchanged (`BLOCKED`), and no device/HIL/N18 claim is made |
| 0.4 | 2026-10-26 | frick-fsw | XW-1: the watchdog row gains `HOST CONTRACT / MODEL-VALIDATED` (abstract external-device contract `docs/vv/external-watchdog-test-contract.md`, harness-clocked model, production PAL API unchanged) with `EMULATED-TARGET: NOT WIRED`; the internal MPFS gate remains `BLOCKED` and is not relabelled |
| 0.3 | 2026-10-26 | frick-fsw | T2: `Link UART` moves from `NOT TESTED` to `INFRASTRUCTURE READY / BLOCKED (watchdog)` with the exact evidence (15 CRC-valid S5 packets on the external serial path, no S1/S17.2 before the abort); explicitly not a target PASS |
| 0.2 | 2026-10-26 | frick-fsw | Round-2 corrections after independent review: whole-run FAIL / PIL-harness scope stated up front; L9 becomes a GAP (NC-EXT-17); S1 responses counted by TM header only (payload prefix and codes are observations, NC-EXT-14/15); time row is coarse CUC with source restore; PAL row is BOOT-ONLY and not a requirement oracle; target CRC evidence labelled self-consistency with the independent check being the host golden-frame fixture; watchdog row cites both the MPFS and the generic QEMU aborts; test-only registration leakage recorded |
| 0.1 | 2026-10-26 | frick-fsw | Initial durable I/O coverage matrix for increment T1: inventory from ASSESSMENT §5, per-path status, declared gaps, explicit non-claims |
