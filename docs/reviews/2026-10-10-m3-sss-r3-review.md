# Milestone M3 SSS/SRS delta - independent systems-engineering review (R3)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0015                      |
| Reviewer session                  | independent subagent, fresh context (norm N9/N14) |
| Reviewer profile                  | R3 (Senior Systems Engineer, `docs/engineering/independent-review.md` §Profile R3) |
| Reference revision                | base commit `3d3511c` (branch `main`) + uncommitted M3 working tree |
| Items under review                | `docs/requirements/SSS.md` (SSS-REQ-017); `docs/requirements/SRS.md` (SRS-REQ-025..032); `docs/requirements/traceability.yaml` / `traceability.md`; `docs/architecture/hardware-abstraction.md`; `docs/design/software-design.md`; `docs/quality/ecss-compliance-matrix.md`; `docs/quality/safety-and-fdir.md`; `docs/vv/validation-plan.md` |
| Date                              | 2026-10-10                             |
| Scope                             | ECSS-E-ST-40C requirement quality (completeness, unambiguity, verifiability, absence of implementation over-specification) and ECSS-E-ST-10 system engineering; correctness of the SSS->SRS decomposition; interface consistency; feasibility at criticality B on RTPFS160ZT / RTEMS 6.2; correctness of the ECSS compliance matrix (class-B outputs vs. the class-A residual gap set GAP-A1..A13). M4 driver implementations and on-target/HWIL qualification are excluded. |
| Independence declaration          | Reviewer took no part in authoring, executing or previously reviewing the items; fresh context. Every claim below is based on the repository contents and reproduced commands only. |
| Norms applied                     | N1-N17 (see `docs/rules.md`), with emphasis on N3, N4, N5, N8, N12, N14, N16 |

## 1. Scope

Reviewed the milestone-M3 ("hardware abstraction and integration interfaces")
SSS/SRS delta in the working tree on base commit `3d3511c`:

- SSS-REQ-017 and its decomposition into SRS-REQ-025..032 (8 children) for
  completeness, unambiguity, verifiability and absence of implementation
  over-specification (ECSS-E-ST-40C requirements quality).
- Coherence of the SSS->SRS parent/child structure and the machine gate
  (`tools/scripts/trace.py`, norm N4), including the human-readable matrices.
- Interface consistency between the SRS clauses, the contract catalogue
  (`docs/architecture/hardware-abstraction.md`), the internal design
  (`docs/design/software-design.md`) and the actual `src/hal/` headers.
- Feasibility of the claimed HAL scope on the Microchip RT PolarFire SoC
  RTPFS160ZT (MSS vs. FPGA-fabric resources) at criticality B.
- Correctness of the ECSS compliance matrix: whether the class-B software
  outputs are correctly classified and whether the residual class-A gaps are
  correctly identified (the class-A set is GAP-A1..A13; M3 adds GAP-A8..A13).

Reproduced by the reviewer:

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` ->
  **PASS** (`63 requirements, 33 srs, 17 sss, 13 gaps, 45 lean theorems
  checked`; exit 0). SSS-REQ-017 has exactly 8 SRS children, as claimed.
- `grep -rhoE "^theorem +[A-Za-z0-9_]+" verification/lean | sort -u | wc -l`
  -> **45 unique** theorem names, no duplicates (the gate counts 45).
- Direct inspection of `src/hal/*.h`, `config/fsw_config.h`
  (`FF_HAL_MAX_BACKENDS 8u`), `tests/unit/test_hal.c`,
  `verification/lean/Frick/Hal.lean`, `docs/development-plan.md` and the
  quality/V&V documents.

Exclusions / limitations (assumptions):

- `lake` is not installed in this review environment, so `make verify-lean`
  could not be re-executed; the committed transcript
  `verification/lean/lake-build.log` (19/19 modules incl. `Frick.Hal`) was
  inspected and the linked theorem names were confirmed source-resolvable by
  `trace.py`. This is a reproducibility limitation of the review, not a
  finding against the item (the CI `verify-lean` gate is the reproduction path).
- The R1 code review (`2026-10-10-m3-hal-r1-review.md`) and the R2 V&V review
  (`2026-10-10-m3-vv-r2-review.md`) already cover the C/Lean/test mechanics;
  this review does not repeat them, except where an interface or a
  documentation claim affects the system-level specification.

## 2. References

- Project norms: `docs/rules.md` (N1-N17); `docs/engineering/independent-review.md` (profile R3, norm N14).
- Requirements & traceability: `docs/requirements/SSS.md` (rev 0.4), `SRS.md` (rev 0.4), `traceability.yaml`, `traceability.md` (rev 0.3).
- Architecture & design: `docs/architecture/hardware-abstraction.md` (FRICK-ARCH-0008), `docs/design/software-design.md` (FRICK-SDD-001), `docs/architecture/overview.md`.
- Quality & V&V: `docs/quality/ecss-compliance-matrix.md`, `docs/quality/ecss-criticality-b.md`, `docs/quality/class-a-gaps.md`, `docs/quality/safety-and-fdir.md`, `docs/vv/validation-plan.md`, `docs/vv/verification-results.md`.
- Implementation under specification: `src/hal/hal.{c,h}` and `src/hal/hal_*.h`, `config/fsw_config.h`, `tests/unit/test_hal.c`.
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-10C, ECSS-E-ST-50-12/51/52/53 (SpaceWire suite), ISO 11898-1, IEEE 802.1Q/Qbv/Qav/AS.

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| 1 | Minor | `docs/quality/ecss-compliance-matrix.md` §4 / norm N5, N8 | The class-A residual-gap summary is **incomplete**: it lists GAP-A3..A13 but omits **GAP-A1** (deep formal verification) and **GAP-A2** (full PUS-C service coverage), although §4 states the gaps "are tracked in `class-a-gaps.md` and summarised here". A reader of the compliance matrix cannot see the full class-A set A1..A13. | `docs/quality/ecss-compliance-matrix.md:66-83` (rows A3..A13 only); `docs/quality/class-a-gaps.md:17,28` (GAP-A1, GAP-A2 sections); `GAP-A1`/`GAP-A2` are not present anywhere in the compliance matrix (`grep GAP-A1 GAP-A2 docs/quality/ecss-compliance-matrix.md` -> none) while `traceability.yaml:446,454` declares them. | Add GAP-A1 and GAP-A2 to §4 (or state explicitly that §4 scopes only gaps not closable here and point to `class-a-gaps.md` for the full A1..A13 set). |
| 2 | Minor | `docs/quality/ecss-compliance-matrix.md` §4/§5 / norm N8 | §4 is titled "Residual gaps (**not closable in this repository**)" but its own "`Why not closable here`" column says GAP-A8 is "closable by re-titling/extending" and GAP-A9 "closable incrementally (documentation WP)", and §5 concludes that GAP-A8, A9, A10, A12, A13 "are closable by work packages". The title contradicts the content. | `docs/quality/ecss-compliance-matrix.md:66` (title) vs `:78-79` (A8/A9 rows) and `:87-93` (§5). | Rename §4 (e.g. "Residual gaps and their disposition") or split it into "not closable here" (A3..A7, A11) vs. "closable work packages" (A1, A2, A8, A9, A10, A12, A13). |
| 3 | Minor | Compliance matrix status vs. class-A gaps / SSS-REQ-012, norm N5 | The matrix marks outputs **present** while simultaneously listing their completeness as a class-A gap: (a) "Software Design Document (SDD) — architectural + **detailed design**" is `present` but GAP-A9 states per-function detailed design is incomplete; (b) "Software maintenance plan" is `present` but GAP-A13 states maintenance/obsolescence is a gap. §5 then declares the class-B baseline "complete". For class B, an SDP and an SDD are ECSS-E-ST-40C software-engineering outputs; presenting their incompleteness as a class-A stretch gap (and concluding class B complete) risks an unsupported class-B acceptance claim. | `docs/quality/ecss-compliance-matrix.md:26` (SDD "present"), `:41` (maintenance "present"), `:87` ("class-B software baseline is complete"); `docs/quality/class-a-gaps.md:90-106` (GAP-A8 SDP, GAP-A9 SDD); `docs/quality/ecss-criticality-b.md:10` ("Gaps to class A ... GAP-A1..A13"). | Classify each output as present/partial against its *class-B* required content; if the contractual deliverable set requires a titled SDP and a complete SDD at class B, these are **class-B** deficiencies, not class-A gaps (would raise this finding to Major), and the §5 conclusion must be qualified accordingly. |
| 4 | Minor | `traceability.md`, `verification-results.md`, `tool-qualification.md`, `README.md` / norm N8, N16 | The recorded Lean-theorem count **43 is wrong**; the gate reports **45** unique theorems. The incorrect figure is propagated in four artefacts. | `tools/scripts/trace.py` output: "45 lean theorems checked"; independent recount `45 unique`, no duplicates; `docs/requirements/traceability.md:18,141`; `docs/vv/verification-results.md:24`; `docs/vv/tool-qualification.md:73`; `README.md:243`. (The R1 review's explanation of "45 declarations / 43 unique" is itself incorrect - there are 45 unique names.) | Set the count to 45 everywhere, and derive it from the gate output rather than hand-maintaining it. |
| 5 | Minor | `docs/quality/class-a-gaps.md` / norm N16 | The document header still shows `Revision 0.2` while the content was extended to GAP-A13 and a `0.3` row was added to the history table. The revision history and the header disagree. | `docs/quality/class-a-gaps.md:7` (`Revision 0.2`) vs `:151` (`0.3 ... add GAP-A8..A13`); `git diff` shows only the GAP sections and history changed, not the header revision. | Bump the header revision to 0.3 (norm N16). |
| 6 | Minor | `docs/development-plan.md` §3 / SSS-REQ-017 coverage | The resource/bus catalogue states "Interfaces are defined for all rows in M3" and lists a **Time** row with HAL contract `FF_Clock_t` (RTC/timer/watchdog). No such contract exists in `src/hal/`, and it is not covered by SSS-REQ-017, SRS-REQ-025..032 or the manifest - so the plan advertises an M3 HAL contract that was not delivered or traced. | `docs/development-plan.md:54` ("defined for all rows in M3") and `:79` (`FF_Clock_t`); `ls src/hal/` has no `hal_clock.h`/`hal_time.h`/`hal_rtc.h`; `grep -rn FF_Clock_t src docs config tests verification` -> only the development-plan line. | Either deliver/trace the clock/timer/watchdog contract (add an SRS child) or remove the row from the M3 catalogue and mark it M4. |
| 7 | Minor | `src/hal/hal.h` / interface consistency, norm N8 | The file banner still advertises a **non-existent** `hal_i2c.h` ("...hal_spi.h, hal_i2c.h, hal_pcie.h..."), although the I2C contract is `FF_I2c_t` in `hal_spi.h` and `FF_HAL_I2C`'s enum comment was already corrected. The header contradicts its own specification. (Already raised by R1/R2; still present in the working tree.) | `src/hal/hal.h:7`; `docs/architecture/hardware-abstraction.md:54` maps "SPI / I2C" -> `hal_spi.h`; `ls src/hal/` has no `hal_i2c.h`; `src/hal/hal.h:29` correctly says "(hal_spi.h)". | Delete `hal_i2c.h` from the banner (keep the catalogue table as the single mapping). |
| 8 | Comment | `docs/development-plan.md` §M3 vs `hardware-abstraction.md` §4, norm N8 | The M3 plan says the HAL design documents the integrator "specificities", listing "half-duplex/DE, packet-vs-stream framing" among them, but the architecture document's integrator-obligations list (7 items) omits both. | `docs/development-plan.md:109-112`; `docs/architecture/hardware-abstraction.md:69-83` (items 1-7: buffer ownership, DMA/cache, ISR/task, no dynamic allocation, bounded latency, endianness, error model). | Add the two obligations to `hardware-abstraction.md` §4 (RS-485 DE is in `hal_serial.h`; packet-vs-stream framing is only implicit). |
| 9 | Comment | SRS-REQ-031, SRS-REQ-032 / ECSS-E-ST-40C requirement granularity | Two SRS clauses bundle heterogeneous interfaces: SRS-REQ-031 groups SPI, I2C, PCIe and GPIO; SRS-REQ-032 groups SpaceWire with five memory/storage resources. Coarse bundling reduces individual verifiability and traceability (a change to one bus re-opens the whole clause, and a single pass/fail cannot localise a defect). | `docs/requirements/SRS.md:140-146`. | Consider splitting per interface family in a later revision (not blocking; each family is a distinct contract type in the implementation). |
| 10 | Comment | SSS-REQ-017 / terminology, ECSS-E-ST-10 | SSS-REQ-017 asks for a "hardware/OS abstraction layer", but the delivered HAL is hardware-only; the OS primitives (tasks/mutexes/queues/clock) are abstracted by the separate PAL, which is traced under SSS-REQ-011 / SRS-REQ-016. The "OS" part of SSS-REQ-017 has no SRS child. | `docs/requirements/SSS.md:73-78`; `docs/architecture/hardware-abstraction.md:24-26`; `docs/design/software-design.md:41-42`; `traceability.yaml` SSS-REQ-017 (hardware contracts only). | Reword SSS-REQ-017 to "hardware abstraction layer" (the OS abstraction is SSS-REQ-011/SRS-REQ-016), or add an explicit cross-reference so the boundary is unambiguous. |
| 11 | Comment | SSS-REQ-017 / SRS-REQ-026..032 evidence, norm N3/N4 | The "documented" and "inspection" (`Inp`) claims are not linked to any inspection artefact: SSS-REQ-017's implementation list contains only `src/hal/` headers (not `hardware-abstraction.md`), and SRS-REQ-026..032 are `method: Inp`/`lean_note: inspection` but cite only `tests/unit/test_hal.c` (a compile-time presence test). The inspection evidence is asserted, not recorded. | `traceability.yaml:253-314` (`tests: [test_hal.c]`, `lean_note: "...inspection..."`); `docs/requirements/SRS.md:125,129,132,136,139,142`; `docs/vv/verification-results.md:24` (M3 row, no per-contract inspection record). | Record a short inspection note (header reviewed, clauses covered, verdict, date) in the M3 V&V entry and link `hardware-abstraction.md` to the Inp requirements. (Overlaps R2 NC-6; no GAP required.) |
| 12 | Comment | SRS-REQ-032 / ECSS-E-ST-50-53 | The clause lists ECSS-E-ST-50-53 "CCSDS packet transfer" as a required SpaceWire contract, but `hal_spacewire.h` exposes no distinct 50-53 mapping - only raw `send`/`send_pid` and `FF_Rmap_t` (50-52). Coverage of the fourth standard is not visible at the contract boundary. | `docs/requirements/SRS.md:143-146`; `src/hal/hal_spacewire.h:21-40` (`FF_SpaceWire_t` + `FF_Rmap_t`). | Add an explicit packet-transfer mapping (or a note that 50-53 is realised by `send_pid` carrying CCSDS packets) so inspection can confirm the clause. |
| 13 | Comment | SRS-REQ-025 / implementation over-specification | The normative text names the internal configuration macro `FF_HAL_MAX_BACKENDS`. Naming a build-time macro in a requirement couples the requirement to the implementation (mild over-specification). | `docs/requirements/SRS.md:117-121`; `config/fsw_config.h:95`. | State the bound as "the configured capacity" and keep the macro name in the configuration documentation. |
| 14 | Comment | R1/R2 M3 review follow-up records / norm N11, N12 | The sibling M3 reviews' follow-up tables still record NCs as `open` with no disposition, even though the working tree already contains the corresponding fixes (e.g. `traceability.md` now has the M3 rows, `verification-results.md` has the M3 cycle row, `test_hal.c` now includes `hal_tsn.h` and all contract types, `hal_net.h` now self-typedefs `FF_NetStack_t`, `status.json` now shows M3 "in progress"). Closure evidence and delta re-review rows are not yet recorded. | `docs/reviews/2026-10-10-m3-hal-r1-review.md:126-139`; `docs/reviews/2026-10-10-m3-vv-r2-review.md:118-127`; current working tree (`git diff`/file inspection). | On resolution, record the disposition/evidence and the delta re-review for each NC (norm N12), as done for M2. |

**Severity definition** (per template):

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

### 3.1 Positive confirmations (verified, no finding)

- **SSS->SRS decomposition is complete and coherent.** SSS-REQ-017's scope
  (Ethernet + pluggable TCP/IP + TSN, CAN/CANopen/CAN-FD, RS-422/RS-485, SPI,
  I2C, PCIe, GPIO, SpaceWire, memory/storage) maps onto SRS-REQ-026..032 with
  no orphan clause and no un-cited SRS child; the registry mechanism
  (SRS-REQ-025) supports "without modifying the framework". `trace.py` proves
  exactly 8 children and one parent, bidirectionally (norm N4).
- **Requirements are verifiable and largely unambiguous.** Each SRS clause has
  an explicit method; the contract headers visibly implement the named features
  (e.g. `FF_TsnBackend_t` covers 802.1Q/Qbv/Qav/AS; `FF_Serial_t` has RS-485
  DE; `FF_Gpio_t` states the ISR-defers-to-task rule; `hal_mem.h` covers
  pool/DMA/MTD/NVM/block). `method: Inp` for interface declarations is
  appropriate at criticality B.
- **Feasibility at criticality B on RTPFS160ZT / RTEMS 6.2 is credible.** The
  resource catalogue's MSS claims (GEM x2, CAN x2, MMUART x5, SPI x2, I2C x2,
  PCIe Gen2) match the PolarFire SoC MSS, and non-native resources
  (CAN-FD, SpaceWire/RMAP, TSN) are correctly declared as FPGA-fabric backends
  deferred to M4; M3 delivers contracts only, which is consistent with the SRS
  (`method: Inp`) and avoids unverifiable hardware claims.
- **Hardware independence holds.** `src/hal/` contains only forward-declared
  vtables and one bounded registry; no register maps or board specifics leak
  in; the PAL/HAL/BSP split is documented (`hardware-abstraction.md` §2,
  `software-design.md` §1-2).
- **Traceability gate converged**: `trace.py` PASS (63/33/17/13; 45 theorems;
  exit 0), every id present in SSS/SRS, every parent resolved, every path and
  Lean link resolved, every `ci:*` claim declared, every GAP referenced in
  `class-a-gaps.md`.
- **Safety/FDIR and validation plan are consistent with the framework scope**:
  `safety-and-fdir.md` correctly frames a preliminary contribution analysis as
  GAP-A10 and masks on-target fault injection (GAP-A4/A5); `validation-plan.md`
  maps SSS-REQ-017 to the "integration of user applications/drivers/stacks"
  validation objective (M3) and defers on-target validation to GAP-A5.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

**Graded conclusion: APPROVED WITH COMMENTS.** No Major finding was identified.
The M3 SSS/SRS requirements are complete, traceable, verifiable at class B and
feasible on the RTPFS160ZT; the SSS->SRS decomposition is coherent and the
machine gate converges. The findings are documentation/consistency defects that
must be corrected or justified and tracked to closure under norm N12:

- The **ECSS compliance matrix does not fully/correctly characterise the gap
  set** (F1-F3): it omits GAP-A1/GAP-A2, its §4 title contradicts its own rows
  and §5, and it marks class-B-relevant outputs as present while listing their
  completeness as class-A gaps. The **class-A set is GAP-A1..A13**; GAP-A8/A9
  (SDP, SDD detailed design) are arguably **class-B** deficiencies rather than
  class-A uplift, so the §5 "class-B baseline is complete" claim is not
  supported as written. If the contractual deliverable set requires a titled
  SDP and a complete SDD, F3 must be elevated to Major before class-B book
  closure.
- **Factual counts/dates are stale** (F4 theorem count 43 vs. 45; F5
  `class-a-gaps.md` header revision 0.2 vs. history 0.3).
- **Coverage/consistency gaps** (F6 undelivered `FF_Clock_t`; F7 dangling
  `hal_i2c.h`; F8 missing integrator obligations).

Comments F9-F14 are advisory. Under norm N14 this SSS review is repeated until
the Minor findings (F1-F7) are closed or justified.

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R3 (N9/N14)   | independent subagent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS (7 Minor, 7 Comments) |

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12:
NCs are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R3-M3-01 | Minor | `ecss-compliance-matrix.md` §4 / N5, N8 | Class-A summary omits GAP-A1, GAP-A2 | addressed (fixed, or accepted with justification) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| R3-M3-02 | Minor | `ecss-compliance-matrix.md` §4/§5 / N8 | §4 title contradicts its rows and §5 | addressed (fixed, or accepted with justification) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| R3-M3-03 | Minor | Compliance matrix / SSS-REQ-012, N5 | Class-B outputs (SDP, SDD detail) misclassified as class-A gaps; §5 "complete" unsupported | addressed (fixed, or accepted with justification) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| R3-M3-04 | Minor | `traceability.md`, `verification-results.md`, `tool-qualification.md`, `README.md` / N8, N16 | Lean theorem count reported 43, gate reports 45 | addressed (fixed, or accepted with justification) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| R3-M3-05 | Minor | `class-a-gaps.md` / N16 | Header revision 0.2 vs. history 0.3 | addressed (fixed, or accepted with justification) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| R3-M3-06 | Minor | `development-plan.md` §3 / SSS-REQ-017 | `FF_Clock_t` advertised for M3 but not delivered/traced | addressed (fixed, or accepted with justification) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| R3-M3-07 | Minor | `src/hal/hal.h` / N8 | Banner references non-existent `hal_i2c.h` | addressed (fixed, or accepted with justification) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| R3-M3-08 | Comment | `development-plan.md` / `hardware-abstraction.md` / N8 | Two integrator obligations claimed but absent from the architecture doc | addressed (fixed, or accepted with justification) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| R3-M3-09 | Comment | SRS-REQ-031/032 / granularity | Bundled heterogeneous interface clauses | addressed (fixed, or accepted with justification) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| R3-M3-10 | Comment | SSS-REQ-017 / terminology | "hardware/OS abstraction layer" vs. hardware-only HAL (OS = PAL) | addressed (fixed, or accepted with justification) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| R3-M3-11 | Comment | SSS-REQ-017, SRS-REQ-026..032 / N3, N4 | Inspection (`Inp`) evidence asserted, not recorded/linked | addressed (fixed, or accepted with justification) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| R3-M3-12 | Comment | SRS-REQ-032 / ECSS-E-ST-50-53 | 50-53 CCSDS packet transfer not visible in the contract | addressed (fixed, or accepted with justification) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| R3-M3-13 | Comment | SRS-REQ-025 | Macro `FF_HAL_MAX_BACKENDS` named in normative text | addressed (fixed, or accepted with justification) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |
| R3-M3-14 | Comment | R1/R2 M3 review follow-up records / N11, N12 | NC dispositions/delta re-reviews not yet recorded | addressed (fixed, or accepted with justification) | build/test/trace + git diff in this cycle | frick-fsw | closed | done (this cycle) |

---

Committed and pushed with the reviewed change (norm N11).
