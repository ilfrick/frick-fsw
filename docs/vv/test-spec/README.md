# Test Specification Work Packages (chunk coordination)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-TSPEC-002                     |
| Revision                         | 0.1                                    |
| Date                             | 2026-10-14                             |
| Norms                            | N8, N10, N11, N21, N22                 |
| Umbrella                         | `docs/vv/comprehensive-test-spec.md` (FRICK-VV-TSPEC-001) |
| Companion                        | `docs/vv/system-verification-architecture.md` (FRICK-VV-SVA-001) |

This index splits the N22 comprehensive-test work into bounded **work packages**,
each owned by one reviewer agent with a separate scope, so that the full scope
can be completed without a single oversized task. Each package writes one
self-contained file and returns a status; the author merges the packages into the
umbrella specification and records gaps under **NC-EXT-47**.

## 1. Why chunks

The full "every I/O individually and combined, nominal + out-of-bound, every
feature, plus software-only robustness" scope exceeds what one agent session
completes reliably. Each package below is independently scoped, reviewable and
mergeable.

## 2. Merge format (mandatory, machine-checkable)

Each package file MUST use exactly this row schema for cases:

```
| N22-<PKG>-<n> | <REQ id(s)> | <case name> | nominal|OOB | <inputs incl. boundaries> | <expected observable result (oracle)> | <env SIL/PIL/HIL> | <individual|combined> | <covered|gap> |
```

- Header row per package:
  `| Case | Requirement | Case | Class | Inputs (boundaries) | Expected result (oracle) | Env | Scope | Status |`
- Every `Requirement` value must be an id present in
  `docs/requirements/traceability.yaml`.
- Every `OOB` row's oracle must cite the requirement/interface clause that
  defines the expected out-of-bound behavior. If none exists, the row status is
  `gap` and the package notes the NC/TBD (never an invented oracle).
- `Area`/interface columns are free text but must name the HAL contract or
  module path.

## 3. Work packages

| PKG | Owner profile | Scope | Output file | Umbrella section |
| --- | ------------- | ----- | ----------- | ---------------- |
| TS-1 | V&V | CCSDS packet + PUS-C services + time/CUC | `docs/vv/test-spec/ts-1-protocol-core.md` | §4 (protocol rows) |
| TS-2 | V&V | Link layer (loopback/IP/TSN/UART) + SDLS | `docs/vv/test-spec/ts-2-link-security.md` | §4 (link rows) |
| TS-3 | V&V | I/O group A: GPIO, Serial, Mem/DMA, GEM ETH, netstack/lwIP, TSN, gPTP | `docs/vv/test-spec/ts-3-io-a.md` | §4 (I/O A rows) |
| TS-4 | V&V | I/O group B: CAN, I2C, SPI, MTD, Block, PCIe, SpaceWire, NVM | `docs/vv/test-spec/ts-4-io-b.md` | §4 (I/O B rows) |
| TS-5 | V&V | Core services: SB, TBL, EVS, ES, apps/scheduler, HS/FDIR, math | `docs/vv/test-spec/ts-5-core-services.md` | §4 (core rows) |
| TS-6 | V&V | Software-only robustness: memory safety, numeric, concurrency/timing, fault injection, resources | `docs/vv/test-spec/ts-6-sw-robustness.md` | §6 |
| SVA-1 | Avionics systems | Test levels, V-model, environments, acceptance criteria | `docs/vv/test-spec/sva-1-levels-vmodel.md` | system arch §1 |
| SVA-2 | Avionics systems | ICD/interface coverage + FDIR strategy | `docs/vv/test-spec/sva-2-interfaces-fdir.md` | system arch §2–3 |
| SVA-3 | Avionics systems | Timing/determinism + environmental + security + tool qualification | `docs/vv/test-spec/sva-3-timing-env-sec.md` | system arch §4–7 |

## 4. Series/parallel policy

- Packages are independent (separate files, separate scopes); they may run in
  series when agent capacity is limited.
- Each package is bounded so it can complete in one session; larger scopes were
  split precisely to avoid oversized tasks.
- Merge is performed by the author after all packages are returned; merged rows
  must not duplicate a `N22-<PKG>-<n>` id.

## 5. Status

| PKG | State | Notes |
| --- | ----- | ----- |
| TS-1..TS-6, SVA-1..SVA-3 | pending | dispatched per §3 |

## History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-14 | frick-fsw | Define the chunk plan and merge format for the comprehensive N22 test work; assign bounded work packages to reviewer profiles |
