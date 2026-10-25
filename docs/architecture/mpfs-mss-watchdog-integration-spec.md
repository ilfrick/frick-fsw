# MPFS/MSS watchdog integration specification (for owner approval)

|                                   |                                        |
| --------------------------------- | :------------------------------------- |
| Project                           | frick-fsw                              |
| Document id                       | FRICK-ARCH-0024                        |
| Revision                          | 1.5                                    |
| Date                              | 2026-10-22                             |
| Status                            | **DRAFT SPECIFICATION — a REVIEW AGENDA for owner decision. It is *not* an implementation-ready design, not an implementation approval, and it authorizes no register write, no binding change and no production-code change.** |
| Norms                             | N2 (English), N8 (docs), N16 (history), N21 (findings) |
| Reviews of rev 1.0                | FRICK-REVIEW-0424 (embedded safety), FRICK-REVIEW-0425 (V&V) — retained unchanged; both reviewed **rev 1.0** and are not restated as reviews of later revisions |
| Confirmation of rev 1.4            | FRICK-REVIEW-0428 (embedded safety, confirmation addendum), FRICK-REVIEW-0429 (V&V, confirmation addendum) — dated addenda, retained and indexed; both verified the **rev 1.4** body identity by SHA-256. Their findings are itemised in §10; the corrections they triggered produced **rev 1.5** |
| Related                           | FRICK-ARCH-0023 (integration proposal, rev 1.5), FRICK-ADR-0021, `docs/status/escalations.md` (ESC-9/ESC-10), NC-PAL-01, NC-EXT-186..202, NC-TS-01 |

## 1. Authorization, and what this document is not

The owner (Nicola, Telegram 223694038) authorized **path A** of FRICK-ARCH-0023 —
wiring the MPFS/MSS watchdog — **starting with a controlled specification for
approval before any implementation or register write** (ESC-9). A second
message in the same exchange was a **request about the review/agent model, not an
additional watchdog approval**; it carries no parameter, threshold or binding
authorization. Both are recorded in `docs/status/escalations.md`.

This document is that specification: documentation, research and review only.

- **Not authorized and not done here:** any implementation of the binding, any
  register write, any change to production code (including its comments), any
  weakening of the watchdog's fatal behaviour, any "capability absent ⇒
  non-fatal" mode (the owner has **rejected** operating without a watchdog —
  ESC-10; the alternative is keeping the N18 gate blocked and FAIL), any pin
  change, any global configuration change, any dependency installation, any
  commit or push, and the closure of any NC or gate.
- **The 8 s timeout** is an owner-declared arbitrary placeholder (ESC-7,
  NC-EXT-187) and is **not** re-justified here.
- **The 250 ms pre-reset report bound** is an owner-authorised *upper-bound
  design target* (ESC-1/ESC-5) whose evidence is open (NC-EXT-186). It is
  **not** a required duration, **not** a lower bound, and is not re-justified
  here.
- **N18 remains FAIL and Lean remains PASS** per the retained evidence; neither
  was re-run in this phase.
**Change introduced by this revision (rev 1.4).** Official *issuer* documents
were retrieved from Microchip's public download path in this phase
(`docs/vv/evidence/2026-10-22-s8-fdir-wd-official-docs-retrieval.log`): the MSS
Technical Reference Manual rev S, the Power-Up/Resets User Guide rev M (its title
covers the **RT** PolarFire SoC), the PolarFire SoC data sheet rev E, the
**radiation-tolerant** product overview rev C, the **RTPFS160ZT-applicable**
errata rev A, and AN4903. They **change the instance and reset-scope facts**
(F-18, F-22) and therefore **supersede the rev 1.3 instance recommendation**
(R-1) and agenda item C. They do **not** close the device-documentation group:
the watchdog register map, the reset values and the design-time MSS configuration
remain in a document that is **not retrievable** from this environment. **No
parameter, clock, timeout, instance or reset policy is adopted by this
revision**; the owner decisions remain open and the agenda is updated, not
answered.


## 2. Target, board, BSP and hart trace (facts)

| Item | Value | Source |
| --- | --- | --- |
| Device (flight target) | Microchip RT PolarFire SoC **RTPFS160ZT (FCVG784)** | `config/platform/platform_mpfs.h` (`FF_BOARD_NAME`) |
| Console | the flight software's console is the RTEMS console device `/dev/console`; the N18 capture point is the MSS **UART1** instance (`mmuart1`) | `config/platform/platform_mpfs.h`; `tools/docker/renode/frick-renode.resc` |
| CPU subsystem | 4 × U54 application cores (RV64GC; 625 MHz maximum per the platform header) + E51 monitor | `config/platform/platform_mpfs.h` |
| Emulated target hart | **u54_1** is the only running hart in the N18 configuration (E51 and u54_2/3/4 halted) | `frick-renode.resc` |
| RTEMS | project label "RTEMS 6.2" = the RSB branch `3814cb0e…-modified`; the kernel **reports 6.0.0** at run time | `tools/scripts/setup_rtems.sh` (the "6.2" label); the runtime banner `RTEMS version: 6.0.0.3814cb0e…` is retained in `docs/vv/evidence/2026-10-21-s8-fdir-gate-verification.log`; the toolchain banner in `docs/vv/evidence/2026-10-20-n18-toolchain-identity.log` |
| BSP | `mpfs64imafdc`, multilib `rv64imafdc/lp64d/medany`; GCC 13.3.0 20240521, Newlib 1b3dcfd | `cmake/toolchains/rtems-riscv-mpfs.cmake`; the toolchain-identity evidence |
| Linker memory map | `RAM : ORIGIN = 0x1000000000, LENGTH = 256 MiB` | installed BSP `linkcmds`; transcript `docs/vv/evidence/2026-10-22-s8-fdir-wd-spec-bsp-inventory.log` |
| Watchdog timeout source | mission table `watchdog_timeout_s` (8 s — **unjustified placeholder**) | `config/tables/fsw_tables.c`; `src/apps/app_wd.c` |
| Feed period | `FF_AppWdPlan` → `timeout/4`; the feeder uses a **relative** delay | `src/apps/app_wd.c:85-96` |

### 2.1 Local reference inventory

| Reference | Location | Status |
| --- | --- | --- |
| Portable watchdog contract | `src/core/pal/pal_watchdog.c`, `src/core/pal/pal.h` | present, host-tested |
| RTEMS platform hooks (**unwired**) | `src/core/pal/pal_rtems.c:72-87` (`FF_NOT_IMPLEMENTED`, NC-PAL-01) | present, deliberately empty |
| Feeder application | `src/apps/app_wd.c:81-96` (plan call and the relative delay) | present |
| Mission timeout value | `config/tables/fsw_tables.c` | **unjustified placeholder** |
| **Microchip MSS register map / HAL / driver** | — | **ABSENT from the repository and from the installed BSP** |
| **RTPFS160ZT datasheet, errata, MSS configuration** | — | **ABSENT (owner input)** |

**Verified fact (BSP baseline)** — retained transcript
`docs/vv/evidence/2026-10-22-s8-fdir-wd-spec-bsp-inventory.log`: the installed
`mpfs64imafdc` BSP include tree has **519 files**; a recursive search for
`mpfs|mss_` matches **two** files (`bspopts.h`, `bsp/riscv.h`); there is **no**
MSS register map, no MSS peripheral table and no MSS watchdog driver; the only
watchdog headers are the generic RTEMS ones; `libchip` has no MPFS/MSS entry.
**Consequence:** the binding must be written against the target's own
documentation.

### 2.2 Public references (research only)

Retrieved 2026-10-21T21:50Z from `polarfire-soc/polarfire-soc-bare-metal-examples`,
branch `main`, commit **`5a890a5b5693be7ce415ccece2f2283dbc5a8851`** (2026-08-31);
the SHA-256 values were computed on the raw HTTP bodies.

| File | Path | SHA-256 |
| --- | --- | --- |
| `mss_watchdog.h` | `applications/mpfs-blank-baremetal/src/platform/drivers/mss/mss_watchdog/mss_watchdog.h` | `43550760a906d7ea30207bd754900a61c3ec9943621885f68a769243d16065d0` |
| `mss_watchdog.c` | `…/drivers/mss/mss_watchdog/mss_watchdog.c` | `08652c8151ca4d6fc468174bb94ed975a2da87344cb78204df80983262313dd9` |
| `mss_sysreg.h` | `…/mpfs_hal/common/mss_sysreg.h` | `ffa978902b27d06e149b42754ff429ba784fb72dcc508703aae3e4851c1565c9` |
| `mss_peripherals.c` | `…/mpfs_hal/common/mss_peripherals.c` | `8e588018860f5949882910be8adaf23099c370c2c88868191ca385c7ce99e064` |
| `mss_peripherals.h` | `…/mpfs_hal/common/mss_peripherals.h` | `9692da093f601f5a78f11266a9aa253c859d50d48ade3e4bef720663605ddeba` |

**Retention position (unchanged, owner decision pending):** the five files are
**not retained in this repository** — they are third-party content and the N13
boundary keeps them out of the tree. The hashes pin what was read; the
reproduction command is in §9.

**Applicability caveats:** issuer Microchip (MIT-licensed example driver);
revision = a *mutable* branch, pinned by commit and hashes; **device
applicability to the RTPFS160ZT is NOT established** (owner input). The
Microchip product page returned **HTTP 403** here and is not used as evidence.

### 2.3 Emulator model identity and the retained model probe

The pinned image is `antmicro/renode@sha256:7905c32…` (**Renode v1.16.1,
build d66b0c2a-202602160923**). Platform digests:
`polarfire-soc.repl` sha256 `2b664d86b821ae17c776784d42690f0aaf9fa878b9b65d5e2a8f0442df0f349e`;
`beaglev-fire.repl` sha256 `2bdf20e9924e8ef3630fb3520123cc1b9b0b6dcba7a4ae13548a66bd2bf4c182`.

**The model source is not shipped in the image, and no version-bound upstream
source is obtainable** (`renode-infrastructure` exposes no release ref for
build `d66b0c2a-202602160923`). Therefore every model statement in this
document is either a **retained measurement** from the probe retained at
`docs/vv/evidence/2026-10-22-s8-fdir-wd-model-probe.log`, or is explicitly
marked **not established by retained evidence**.

**Retained measurement (probe scope: monitor-driven register writes on
`wdog1` @ `0x20101000`, no SUT ELF loaded, machine not started):**

| Register (cited map) | Observation | Result |
|---|---|---|
| `STATUS` (0x08) | initial read-back | `0x00000000` |
| `STATUS` (0x08) | after the first write to 0x08 | `0x00000010` — bit 4 = `LOCKED` (the header's `MSS_WDOG_LOCKED`); constant `0x00000010` in every later read-back |
| `CONTROL` (0x04) | every read-back (never written by the probe) | `0x00000002` — constant |
| `STATUS` (0x08) | model warnings on the writes to 0x08 | `Unhandled write to offset 0x8. Unhandled bits: [6-7] when writing value 0xC8`; `… Unhandled bits: [5] … Tags: DEVRST (0x1)` (value `0x3C`); `… Unhandled bits: [24]` (value `0x1000000`) — i.e. bits 5–7 of `STATUS` are unimplemented in the model |
| `TRIGGER` (0x14) | re-write `0x0A` after configuration | read-back `0x0000000A` — **the re-write took effect** |
| `TRIGGER` (0x14) | write `0x1000` | warning `Unhandled bits: [12]`; read-back `0x00000000` — **12-bit field, truncated** |
| `TIME` (0x0c) | **never read or written by the probe** | — |
| `REFRESH` (0x00) | written (wrong key `0x1`, then the documented key), **never read back** | — |
| `MSVP` (0x10) | written (`0x32`), **never read back** | — |

> **Correction to the probe's own labels (recorded 2026-10-22 after
> FRICK-REVIEW-0426/0427).** The probe script labelled offset `0x08` "TIME" and
> `0x04` "STATUS". Per the cited register map (`mss_watchdog.h`,
> `WATCHDOG_TypeDef`: `REFRESH 0x00 | CONTROL 0x04 | STATUS 0x08 | TIME 0x0c |
> MSVP 0x10 | TRIGGER 0x14 | FORCE 0x18`) the correct attribution is
> `0x08 = STATUS`, `0x04 = CONTROL`, and `TIME` is `0x0c`. Three indicators in
> the retained output agree: the model's warning tags the 0x08 write with
> `DEVRST` (a `STATUS` bit), the 0x08 read-back changes to `0x10` (bit 4 =
> `LOCKED`) immediately after the first write to that register, and the 0x14
> access behaves as `TRIGGER` (12-bit truncation). **No statement about the
> model's `TIME` width is therefore established** — the `TIME` register was
> never probed.

**What follows from the retained measurement (and what does not):**

- The probe **could not discriminate the refresh key**: the machine was never
started, so the counter was not running, and `REFRESH` — the register whose
read-back exposes the counter — was never read back. There was therefore **no
reload observable** to compare the two writes against. The retained evidence
establishes **"not established by the retained probe"**, *not* that the model
lacks a key-sensitive observable. The key-sensitivity
  cases are placed at host and target level; whether a **running-machine** leg
  could witness them is an open V&V scoping question (see agenda item F).
- The model does **not enforce the write-once lock** in the probed state (a
  post-configuration `TRIGGER` re-write took effect), so the lock case is
  **target-only**; the emulator can evidence the write *sequence* only.
- The model **truncates `TRIGGER` to 12 bits** (measured). Nothing is measured
  about the `TIME` or `MSVP` widths, so the documented bound validation
  (`TRIGGER > 4095`, `TIME > 16 777 200`) is a **host-level** case (the
  binding's own argument checking).
- `STATUS` **did** vary (`0x00000000 → 0x00000010`, `LOCKED`); the constant
  `0x00000002` is the `CONTROL` register. `STATUS` is therefore *not*
  unusable, but no probed state distinguishes a forbidden-window condition.
- **Not established by retained evidence:** the expiry→reset behaviour, the
  reset scope, the peripheral gating, the timebase/sleep behaviour, the
  counter reload semantics and the lock enforcement. These require the target
  (or a version-bound model source).

### 2.4 Official issuer documents (retrieved 2026-10-22)

Retrieved from the issuer's public download path
(`ww1.microchip.com/downloads/aemDocuments/…`), all HTTP 200. The retrieval
record — URLs, HTTP status, sizes, SHA-256, device applicability, the blocked
URLs, the labelled inference and the reproduction commands — is retained at
`docs/vv/evidence/2026-10-22-s8-fdir-wd-official-docs-retrieval.log`. **The
issuer's PDFs are not copied into this repository** (N13 boundary); the hashes
pin what was read.

| Document | Issuer | Docid / revision | Device applicability |
| --- | --- | --- | --- |
| PolarFire SoC FPGA **MSS Technical Reference Manual** | Microchip | DS60001702S, rev **S** (06/2026) | the PolarFire SoC (non-RT) MSS; the RT device shares this MSS — the errata points to the RT "MS Description" for MS device descriptions, which is **not retrieved** |
| **PolarFire SoC and RT PolarFire SoC** FPGA Power-Up and Resets User Guide | Microchip | DS60001676M, rev **M** | title states RT PolarFire SoC → **applies to the flight device** |
| PolarFire SoC FPGA Data Sheet | Microchip | DS00004248E, rev **E** | the PolarFire SoC (non-RT) device |
| Radiation-Tolerant PolarFire SoC FPGA Product Overview | Microchip | DS00005395C, rev **C** | the RT family → **applies** |
| RT PolarFire FPGA and SoC FPGA Devices **Errata** | Microchip | DS00006185A, rev **A** | "applies to part number prefixes RTPF500T, RTPF500ZT, RTPFS460ZT and **RTPFS160ZT**" → **the flight device is covered** |
| AN4903 Differences Between RTPF500T and RTPF500ZT | Microchip | AN4903 | RT silicon differences |

**Still not retrievable (recorded as inaccessible, not as missing):** the
"PolarFire SoC **Device Register Map**" — the document that holds the watchdog
register descriptions, their reset values and the design-time MSS configuration.
The issuer's host is edge-blocked from this environment (HTTP 403, Akamai
"Access Denied", on the product pages and on `/downloads/en/DeviceDoc/`); the
only alternate located is a third-party, bot-protected mirror, which is not an
authority. **This is the outstanding controlled device input (M-1).**

**Genuinely owner-supplied, and not public at all:** the MSS configuration
flash-bit defaults of the flight board, the mission hart / watchdog instance /
clock assignment, and the board-level reset scope.

## 3. Verified facts — and their source class

> **Source-class banner.** F-1..F-8, F-10..F-16 rest on the **public
> Microchip example driver** of §2.2, whose **applicability to the RTPFS160ZT
> is NOT established**; they are verified facts *about the cited source*, not
> target behaviour. F-9, F-17 and the project rows are local or measured.
> Where a cited source contradicts itself, the contradiction is carried as an
> open item (M-11, M-12) rather than resolved.

**F-1 — register block and instance addresses.** `REFRESH, CONTROL, STATUS,
TIME, MSVP, TRIGGER, FORCE`; bases `0x20001000, 0x20101000, 0x20103000,
0x20105000, 0x20107000` ("LO") and `0x28001000 … 0x28107000` ("HI"). The five
"LO" bases coincide with the five model instances (corroboration of the
**Corroborated by an issuer document:** the MSS Technical Reference Manual's
peripheral address table lists WDOG0 `20001000`, WDOG1 `20101000`, WDOG2
`20103000`, WDOG3 `20105000`, WDOG4 `20107000` — the same five bases, from the
issuer rather than from the family example driver (F-18's source class).
instance set only).

**F-2 — configuration sequence and the write-once lock (upstream
self-contradiction).** The reference implementation writes `TRIGGER`, `MSVP`,
`TIME`, `CONTROL`, then `REFRESH` with the key; its comment states that after any
write to offsets 0x00–0x0c the `TRIGGER`, `MSVP` and `TIME` registers are
locked and that `MSVP` must be ≤ `TIME`. **The upstream documentation is
self-contradictory about the lock condition** (`mss_watchdog.h`: "After
performing the write operation into the TIME register, the TIME, TRIGGER, and
MSVP register values are frozen" — versus `mss_watchdog.c`: "After any write to
register from offset 0x00 to 0x0c the TRIGGER, MSVP and TIME registers are
locked"). **Open item M-11.** The measured behaviour of the *pinned model* is
in F-17 (the lock was **not enforced** in the probed state).

**F-3 — counter bounds and keys.** `TRIGGER` ≤ `MSS_WDOG_TRIGGER_MAX` =
**4095**; `TIME` ≤ `MSS_WDOG_TIMER_MAX` = **16 777 200** (the macro's own
upstream comment `0xFFFFFF` is loose — 0xFFFFFF is 16 777 215; the binding
must use the macro value); `MSS_WDOG_REFRESH_KEY` = **0xDEADC0DE**;
`MSS_WDOG_FORCE_RESET_KEY` = **0xDEAD**. *This document uses the macro value
16 777 200 throughout; `0xFFFFFF` appears only as the upstream comment's
loose annotation.*
**Hardware field width (issuer document):** the TRM's watchdog block diagram
labels the load path `WDOGLOAD[25:0]` and the MVRP path `WDOGMVRP[31:0]`. The
**26-bit hardware load field is wider than the cited driver's 16 777 200
cap**, so that cap is a *driver* limit, not a hardware field width; the
"not representable" remark in R-3 is therefore a statement about the cited
driver's limit, and must not be read as a hardware limit.

**F-4 — counter semantics.** A **down-counter** reloaded from `TIME` by a
`REFRESH` write; the **timeout interrupt** is raised at the `TRIGGER` crossing;
the counter keeps counting and a **reset is generated at zero**; the MVRP
interrupt is raised at the `MSVP` crossing.

**F-5 — forbidden (refresh) window.** `MSVP` is the maximum value up to which
refresh is permitted: a refresh is **forbidden while the counter is greater
than `MSVP`** and attempting one asserts the timeout interrupt. The window is
**disabled by default**; when disabled, a refresh is permitted at any time.

**F-6 — clock and prescaler (attribution-limited).** The header states a
**prescaler of 256 on the MSS AXI clock**, with an *example* at "AXI clock =
25 MHz" (MVRP 69 s, timeout 171 s for the example values). The header names the
*MSS AXI clock* as the prescaler input; the **device's own watchdog clock path,
its frequency and the applicability of the 256 prescaler to the RTPFS160ZT are
not established** (owner input M-2). The model's 156 250 Hz figure is an
**estimate in the platform file** and is not a target clock.
**An issuer document does not supply the missing value either:** the MSS TRM's
watchdog chapter states the APB interface is clocked by `PCLK` and gives the
counter's behaviour, but **does not state the watchdog counter clock or the
prescaler**; the register defaults and the clock relationship are in the
"Device Register Map" document that could not be retrieved. The TRM's power-up
default (F-15) is *consistent* with a prescaler of 256 at a much higher AXI
clock than the header's illustrative 25 MHz — that consistency check is
recorded as a **labelled inference** in the retrieval log and is **not** a
parameter.

**F-7 — refresh and forced-reset behaviour.** `REFRESH` with the key reloads
the counter; the reference driver performs the initial reload only when the
forbidden status is clear and reports an error otherwise. A `FORCE` write forces
a reset **if the watchdog has already timed out**; any other `FORCE` value, or
a `TRIGGER` write at other times, triggers the watchdog **NMI sequence**.

**F-8 — peripheral gating (values partly missing).** The MSS peripheral
subsystem gates peripherals with a **context enable** bit (`CONTEXT_EN_INDEX =
0x00`) and a sub-block clock control; the public table lists
`CONTEXT_EN_MASK_WDOG0..4` with `SUBBLK_CLOCK_NA_MASK`, and the register map
defines `APBBUS_CR_WDOG0..4` bits (offsets **0x5–0x9**) at `APBBUS_CR_OFFSET =
0x80`. **The `CONTEXT_EN_MASK_WDOGx` values are not defined in the five
retrieved files**, so *which* bit to write is unknown. Whether the boot ROM/HSS
or the BSP already performs the gating for the flight configuration is
likewise unestablished (M-8).

**F-9 — the current software state (local).** The RTEMS hooks are
`FF_NOT_IMPLEMENTED`; the feeder treats an arm failure as fatal
(`FF_PalPanic("watchdog: init failed")`) — the approved behaviour, not to be
weakened. The emulated target therefore aborts at start-up and the N18 marker
is absent (NC-EXT-198).

**F-10 — the portable contract to be preserved (local).** The PAL owns the
arm/feed/expiry/reset state machine and its counters; the platform file owns
only the three hardware hooks. No PAL-contract change is proposed.

**F-11 — the independence question (unproved, not assumed).** The
protection the MSS watchdog would offer is a **hardware down-counter** that
reaches zero independently of the hart's execution state — the property
SRS-REQ-073 asks for. **Three dependencies are unproved and are not assumed
here:** (a) the **clock/gating independence** — the counter is clocked from a
source whose relation to the flight software's clock and peripheral gating is
**not established** (M-2 clock source, M-8 gating: whether the flight software
can alter the clock, and whether the peripheral is gated at boot, are both
unproved); (b) the **reset scope** — whether the reset covers the *mission
function* or only the hart is unknown (M-7); and (c) the optional "react before
reset" leg is executed **by the same hart** that may be the thing that hung.
**A system-level reset observation is an obligation, not an inference.**

**F-12 — the lock's consequence (derived from F-2).** Because `TIME`,
`TRIGGER` and `MSVP` become unwritable after the configuration sequence, a
**wrong configuration is not recoverable in software**: it produces a permanent
reset loop (or an immediate reset during initialisation) until a system reset.
The mitigation is a **read-back of `TIME`/`TRIGGER`/`MSVP` immediately after the
configuration writes and before the first `REFRESH`**, treating any mismatch as a
fatal initialisation error. (The pinned model does not enforce the lock — F-17 —
so this mitigation is verifiable only on the target; the emulator can evidence
the *sequence* only.)

**F-13 — the timeout interrupt is permanent and is an NMI (cited header).**
The timeout interrupt **cannot be disabled** and is connected to the **RISC-V
NMI**; the handler must **clear the timeout event** (write `STATUS |=
WDOG_TRIPPED`; the reference implementation writes it **twice** to flush the
posted AHB write) to avoid re-triggering.

**F-14 — timeout-event discrimination at start-up (cited header).** The
application is expected to read the timeout event at start-up to distinguish a
**watchdog reset** from a power-on reset, and to clear it.

**F-15 — design-time pre-initialisation: cited fact vs unknown target state.**
The cited header states the MSS watchdog is **pre-initialised by the flash bits
at the design time** and is **not enabled at reset** — i.e. the software's
configuration write is what arms it. **The target's actual power-on values for
`TIME`/`TRIGGER`/`MSVP`, and whether the registers are already locked at boot,
are not established** (M-15). The two statements are kept distinct: the cited
*mechanism* (design-time defaults, software arms) versus the *unknown target
**The issuer document adds the design-time value:** the MSS TRM states that
"the MSS watchdogs are **not enabled initially when the MSS comes out of
Reset**" and that "when the device is powered up, the watchdog timer is
**enabled with the timeout period set to approximately 10.47 seconds (if VDD =
1.2 V)**". This is consistent with the product overview's "not enabled at
reset", and it means a **power-up timeout exists before the software arms
anything** — a live-system reset hazard that the software must close (see
R-7 and HL-8). The *exact* default load and its register value are still in the
non-retrievable document (M-1).
state*.

**F-16 — the timeout unit is contradictory in the cited source (open item).**
The header's Theory of Operation says the timeout value "can be set to a
maximum value of **4095 µs**", while its own worked example computes the
timeout in **prescaled AXI ticks** (`(0xFFFFF0 − 0x3e8) × (1/25 MHz/256) =
171 s`). These are incompatible: 4095 prescaled ticks at 25 MHz is 41.9 ms, not
4095 µs. **Open item M-12**; §4 uses the tick interpretation as a *working
assumption* to be confirmed against the target documentation.

**F-17 — the pinned emulator model: measured limits (retained probe, §2.3).**
The probe ran in a **non-running machine without a SUT image**, so it witnesses
**register-level** behaviour only. Measured: a post-configuration `TRIGGER`
re-write **took effect** (the model does not enforce the write-once lock in
that state) and `TRIGGER` is **truncated to 12 bits**; `STATUS` moved
`0x00000000 → 0x00000010` (`LOCKED`) after the first write to it. **Not
established by retained evidence:** the refresh-key semantics (the counter was
not running and `REFRESH` was never read back), the `TIME`/`MSVP` widths, the
counter reload behaviour, expiry→reset, the reset scope, peripheral gating, the
timebase/sleep behaviour and the **device's** lock enforcement. The model is therefore usable
for the software **write sequence** and for the state transitions it actually
exposes, and for nothing else.

**F-18 — only WDOG0 can reset the MSS (issuer document, TRM DS60001702S §3.12.12
and §3.12.12.2.3).** "Watchdog_0 is associated with the E51 core and is the only
one out of the five MSS watchdogs capable of resetting the MSS when it
triggers. Each of the other four watchdogs is maintained by a dedicated U54
core and is only capable of interrupting the E51 upon triggering." The TRM
repeats it as a note: "**Only watchdog_0 can reset the MSS.** The other
watchdogs can only generate interrupts to the E51 core." **Consequence for this
specification:** a watchdog owned by a U54 core can detect a hang of that core
but **cannot reset the MSS**; the timeout interrupt of a U54 watchdog is
delivered to the **E51**, not to the U54. The mission-function reset
protection therefore exists **only** on the E51's watchdog. This supersedes
R-1's instance recommendation and re-opens agenda item C.

**F-19 — the power-up default timeout (issuer document, TRM §3.12.12.2.2).**
"…the watchdog timer is enabled with the timeout period set to approximately
**10.47 seconds** (if VDD = 1.2 V)." A **live-system reset hazard**: from power-up
until the software programs the counter, the watchdog is running on its
design-time value, so a software path that fails to arm (or to refresh) is
reset by the hardware at ≈10.47 s, not by the software's own error path. The
*exact* default load value is in the non-retrievable document (M-1).

**F-20 — debug-halt and WFI behaviour (issuer document, TRM §3.12.12.1).**
"The watchdog timer counter is **halted when the processor enters the Debug
state**", and it "can be configured to generate a **wake-up interrupt when the
processor is in WFI mode**". A debug session therefore suspends the timeout
(the emulator's HIL legs and any target debug run are *not* representative of
the running case), and the WFI behaviour is an explicit configuration choice
that belongs to the sleep/low-power input (G-B).

**F-21 — the counter semantics and the hardware field widths (issuer document,
TRM §3.12.12.2.2/2.3 and Figure 3-36).** "The MSS watchdog timeout is implemented
as a **locked down-counter** configuration. The software loads a start value
into the `WDOG_TIME` register, programs the trigger threshold and periodically
refreshes the counter before the trigger level is crossed. **If the counter
crosses the trigger value, a non-maskable timeout interrupt is asserted; if it
continues to zero, a reset is generated.**" The block diagram labels the load
path `WDOGLOAD[25:0]`, the MVRP path `WDOGMVRP[31:0]`, and shows the signal labels
`PROGRAMMING` and `WDOGENABLE` (the retained record carries the **labels only**,
neither their adjacency nor their function).
This corroborates F-4 and F-13 from an issuer source and shows the **26-bit
hardware load field** (F-3).

**F-22 — the reset scope and the reset-reason bit (issuer document, Power-Up
and Resets User Guide DS60001676M §4.2.2.3 and Table 4-2).** "The MSS can be
configured such that the **E51 processor's watchdog timer causes a reset of the
MSS** when the timer runs out." The MSS reset reasons include **`WDOG_RESET`,
reason bit 5** in `RESET_SR` ("This indicates that the watchdog reset has been
activated"), alongside `SCB_MSS_RESET` (1), `DEBUGGER_RESET` (3) and
`FABRIC_RESET` (4). So: a **WDOG0** timeout resets the **MSS** (not the whole
device — the full-device reset is the separate `DEVRST_N` path), and the
software **can distinguish a watchdog reset at start-up** from other reset
causes. This answers part of M-7 and corroborates F-14 from an issuer source.

**F-23 — the peripheral soft-reset release step (issuer document, Power-Up and
Resets User Guide §4.2.2.4).** "Each MSS peripheral has a soft reset register
(`SOFT_RESET_CR`) bit associated with it in the MSS system registers and this
bit **must be written to "1" and then "0" to allow the peripheral to be used**."
This is a **candidate mandatory step** before the watchdog can be programmed
(R-7). **Whether the watchdog peripheral has such a bit, and its address, is
not established** — the retrieved table excerpt did not list it, and the
register map is the non-retrievable document (M-1); it is therefore an open
input, not a step to be implemented on the strength of the general rule.

**F-24 — no published watchdog erratum for the flight device (issuer document,
errata DS00006185A).** The errata that **explicitly covers RTPFS160ZT** contains
three items — the deprecation of external `VERIFY_DIGEST` with the fabric POR
digest check, the SCSM re-entry restriction for RTPF500ZT/RTPFS460ZT/RTPFS160ZT,
and a pointer to the Libero SoC software errata. **None concerns the MSS
watchdog.** The absence of an erratum is not evidence of correctness, and the
claim is bounded by **two** documents that are **not retrieved** here: the
**RT PolarFire SoC "MS Description"**, which the errata names for the MS device
descriptions, and the **Libero SoC software errata**, to which the errata's
third item points.

## 4. Recommendations (proposals for approval — not decisions)

**R-1 — Instance selection (rewritten in rev 1.4: the rev 1.3 recommendation
is superseded).** The rev 1.3 recommendation (`WDOG1` for the emulated
configuration) rested on the family example driver's interrupt routing and is
**withdrawn**: the issuer's MSS Technical Reference Manual states that **only
`WDOG0` — the E51's watchdog — can reset the MSS**, while each of the four U54
watchdogs "is only capable of interrupting the E51 upon triggering" (F-18). A
U54 watchdog is therefore a **hang detector, not a reset provider**. The
consequences are architectural and are the owner's to decide, so this document
states the options rather than choosing among them:
- **(a) `WDOG0` (E51) as the reset provider** — the only path that can reset the
  MSS; the reset is an **MSS** reset, not a full-device reset (F-22), and the
  E51 must itself be alive to service it.
- **(b) a U54 watchdog as a hang detector feeding the E51** — the timeout
  interrupt reaches the **E51** (F-18), so the E51 owns the response; this
  **cannot** by itself reset the MSS.
- **(c) both** — a U54 detector for the mission hart plus the E51 watchdog as
  the reset provider; the only combination that both *detects* a mission-hart
  hang and can *act* on it, at the cost of a two-level timeout budget.
- **(d) an out-of-band monitor** (a second-level watchdog outside the MSS, or a
  board/fabric-level supervisor) for the hang classes a MSS watchdog cannot
  cover (F-11).
The **emulated** configuration's running hart is `u54_1` and the emulator
platform places `WDOG1` at `0x20101000`; that remains a statement about the
emulator, **not** a protection choice. The mission hart, the instance and the
board-level reset scope remain owner inputs (M-3, M-7).


**R-2 — Clock source and counter encoding.** The counter is clocked from the
watchdog's clock source through the documented prescaler of 256 (F-6, whose
applicability here is unconfirmed). Recommend
`t_tick = prescaler / f_wd` with `prescaler = 256` and `f_wd` from the **target**
documentation, then `TIME = round(timeout_target / t_tick)` and
`TRIGGER = round(reaction_window_target / t_tick)`, subject to F-3's bounds.
**Every numeric value requires `f_wd` and the mission decisions (M-2, M-4).**

**R-3 — Timeout, reaction window and the representable ranges.** Two mission
quantities must be separated: the **reset timeout** (governed by `TIME`) and the
**reaction window** between the timeout interrupt and the reset, which is
governed by **`TRIGGER` ticks** — *not* `TIME − TRIGGER` (the timeout interrupt
fires at the `TRIGGER` crossing and the reset follows `TRIGGER` ticks later).
The symbolic consequences, valid only for the cited prescaler and **only once
`f_wd` is known**:

| Quantity | Symbolic form | Bound from the cited source |
| --- | --- | --- |
| reset timeout | `TIME × t_tick` | ≤ `16 777 200 × 256 / f_wd` — a longer mission timeout is **not representable** |
| reaction window | `TRIGGER × t_tick` | ≤ `4095 × 256 / f_wd` |
| earliest valid refresh (window enabled) | `(TIME − MSVP) × t_tick` | `MSVP ≤ TIME` |

**What this does and does not say about the 250 ms report budget.** At an
*illustrative* 25 MHz the reaction-window ceiling would be ≈41.9 ms. That is a
**conditional, clock-dependent figure on an unconfirmed clock and an unconfirmed
unit (M-2, M-12)**; it **does not prove** that the pre-reset report cannot fit
inside a watchdog-triggered reaction window, because (i) the actual clock and
unit are unknown, (ii) the report path and the watchdog-timeout path may be
different paths, and (iii) whether the pre-reset report is required **on the
watchdog-timeout path at all** is an open owner decision (M-13). What *can* be
said: the 250 ms figure is an **upper-bound design target**, so a path that
needs ≤250 ms may fit a ≤250 ms budget, but a reaction window that is shorter
than the actual path duration would not — and **that actual duration is
unknown** (M-10 and the report-path measurement **RP-1** of §5.4) and would
need target evidence, or a separate policy/path decision. **No policy is
selected here.**

**R-4 — MVRP / refresh window.** With the forbidden window **disabled**
(default, F-5) a refresh is permitted at any time, so **no mission-chosen
`MSVP` value is required** in that case. The reference implementation,
however, **always writes `MSVP`** as part of its configuration sequence, so the
register write is still performed; **which value to program while the window is
disabled is an open engineering/owner item** (M-6), not a "no value needed"
conclusion. (Whether the emulator's behaviour depends on `MSVP` while the
window is disabled is **not established by retained evidence** — the probe
never read `MSVP` back.) If the window is enabled, the earliest permitted
refresh is `(TIME − MSVP) × t_tick` after the last reload, and `MSVP` must be
compatible with the feed cadence and the MVRP interrupt lead.

**R-5 — Feeder cadence, margins and the false-reset hazard.** SRS-REQ-073
constrains the **feed period** to ≤ T/4 of the timeout; the *actual* inter-feed
interval of the current implementation (a relative delay plus execution and
scheduling time) is **larger** than that period — the open NC-EXT-187 gap.
Recommend an **absolute monotonic** schedule with the feed completing before
`TIME × t_tick − margin`, where the margin must cover three distinct
quantities: the **MMIO write latency**, the **scheduling jitter** of the feeder
task under representative load, and any interrupt latency. **These are not one
task WCET** (M-5, and the measurement items in §5.4). Additionally the
feeder is the **lowest-priority** task, so a busy higher-priority workload
starves it **first**, and a watchdog reset caused by feeder starvation is a
**false reset of a live system** — a named hazard whose acceptable worst case
and tolerance are owner inputs (M-16). The current relative delay stays until
`f_wd` and the jitter/latency bounds exist.

**R-6 — Reset scope and the "armed but inert" case.** The reference describes a
hardware reset at counter zero and an NMI sequence on `TRIGGER`/`FORCE`
writes; the **reset scope** (system-wide vs hart-local; which harts or masters
observe it) is **not established** (M-7). A distinct failure is "**armed but
**Now partially answered (F-18, F-22):** a `WDOG0` timeout resets the **MSS**;
a U54 watchdog resets **nothing**; the cause is observable as `WDOG_RESET`
(reason bit 5) in `RESET_SR`; the full-device reset is a **separate**
`DEVRST_N` path, not a watchdog path. The part that remains unestablished is
the **board-level** scope (which external masters observe the MSS reset) (M-7).
inert**": the software believes the watchdog is armed while the peripheral is
clock-gated (F-8), or while the peripheral carries **pre-initialised**
design-time values that the software never read (F-15) — whether such a state
constitutes arming on this device is exactly what M-15 leaves open — a **false
sense of protection**. Recommend assuming no reset scope,
exposing only what the PAL contract needs, and **verifying arming by read-back**
rather than by the write's return value.

**R-7 — Init, feed, error, reset and NMI sequences (proposed shape).**
*Init:* (1) read the design-time state and **fail fatally** if the peripheral
is not in a writable, unconfigured state (F-15, F-2, F-12); (2) perform the
peripheral gating **only** if the target documentation says the software must
(F-8, M-8); (3) write `TRIGGER`, `MSVP`, `TIME` in the documented order (F-2),
with `MSVP` always programmed and the relations `TIME > MSVP > TRIGGER` stated
and checked; (4) write `CONTROL`; (5) **read back `TIME`/`TRIGGER`/`MSVP` and
treat any mismatch as a fatal init error — the read-back happens BEFORE the
first `REFRESH`** (F-12); (6) perform the first `REFRESH` with the key, and
treat a forbidden-window status as an **init failure** (F-7). *Feed:* write
(0) if the device documentation shows a peripheral soft-reset bit for the
watchdog, release it with the documented write "1" then "0" (F-23) — **not
established for the watchdog; do not implement it on the strength of the
general rule alone**. (7) read the MSS reset-reason register and record
`WDOG_RESET` (F-22) so that a watchdog reset is distinguishable from a
power-on reset at start-up (F-14).
`REFRESH` with the key; on a read-back that shows no reload, report an error
and enter the existing fatal path. *Error taxonomy:* the PAL contract already
distinguishes "not armed", "backend failure" and "reset requested"; the binding
should add *not implemented*, *not writable (clock/context off)*, *forbidden
window* and *hardware fault*. *Reset request:* per F-7 the documented `FORCE`
semantics apply **only after a timeout**; any other `FORCE` value raises the
**NMI sequence** — a software reset request is therefore **deferred to counter
zero**, not immediate, and the binding must not present it as an immediate reset
(M-14; input group G-C, agenda item C). *NMI/timeout event:* the timeout interrupt is permanent
and is an NMI (F-13); a handler must clear the timeout event with the
documented double write, and the application must read the timeout event at
start-up to distinguish a watchdog reset from a power-on reset (F-14) — both are
**proposed obligations**, currently absent from the sequence.

**R-8 — Where the binding lives.** Recommend implementing the three hooks in
`src/core/pal/pal_rtems.c` with a small, self-contained MSS register block, and
keeping `pal_watchdog.c` untouched — preserving the F-10 seam and the existing
host/emulator behaviour. No production change is made by this document.

**R-9 — Documentation corrections belonging to the implementation phase.** Two
controlled records still state the superseded "Renode has no watchdog model"
claim (`src/core/pal/pal_rtems.c:69`; the SRS-REQ-073 lean note in
`docs/requirements/traceability.yaml`). Correcting them touches production
sources and the trace manifest, so it is **not** done in this
documentation-only phase: registered as **NC-EXT-200**.

## 5. Proposed verification sets (for approval; not yet written)

### 5.1 Case placement — what each environment can actually witness

| Property | Emulator (pinned model) | Host (mock model) | Target / HIL |
|---|---|---|---|
| software write **sequence** (the registers written, and the model's per-write warnings) | **yes** (register-level read-backs and warnings) | yes | yes |
| software write **order** | **no** — the model exposes no ordered write log; the order observable is the **host mock's write observer** (§5.2) | **yes** | yes |
| `TIME`/`TRIGGER`/`MSVP` **bound validation** (reject before any write) | **no** — the model truncates `TRIGGER` (measured); the `TIME`/`MSVP` widths are not established (F-17) | **yes** | yes |
| `MSVP > TIME` **rejection** | **not established** — the probe never wrote `MSVP > TIME` nor read `MSVP` back (F-17) | **yes** | yes |
| refresh-**key sensitivity** (reload only on the valid key) | **not established** — the probe could not discriminate the key (counter not running, `REFRESH` never read back); a *running-machine* leg might | **yes** | yes |
| write-once **lock enforcement** | **no** — the model did not enforce it in the probed state (F-17) | mock — **no host case yet** (open, NC-EXT-205) | **yes** |
| forbidden-**window** behaviour (refresh refused) | **not established** — the probe never enabled the window (F-17); a running-machine leg might | mock — **no host case yet** (open, NC-EXT-205) | **yes** |
| **expiry → reset** | **not established by retained evidence** (F-17) | n/a | **yes** |
| **reset scope** (system vs hart) | **not established** (F-17) | n/a | **yes** |
| **peripheral gating** | **not established** — the probe never exercised gating (F-17) | mock — **no host case yet** (open, NC-EXT-205) | **yes** |
| **timebase / sleep** behaviour | **not established** | n/a | **yes** |

### 5.2 Proposed cases

*Emulator-scoped* (evidence: register-level read-backs and the model's own
warnings, taken in a **non-running machine without a SUT image** — so the
counter was not running and no counter value was observed):

| ID | Case | Discriminating observation |
|---|---|---|
| EM-P1 | configure in the documented order and read the registers back | the sequence executes and the **model's "Unhandled bits" warnings** identify which fields the model implements (evidence about the model, not about the target) |
| EM-P2 | re-write `TRIGGER` after configuration | the re-write **takes effect** in the model — i.e. the model does **not** enforce the lock, so this case demonstrates a **model limitation**, not a target property |

*Host-scoped* (mock MSS register model; the binding's own behaviour). The
mock model exposes (i) the register values the binding wrote, (ii) a **write
observer**: an ordered log of every register write the binding performed, and
(iii) a programmable response to each write (accepted, rejected, no-op). The
observer is what makes the write-order and the write-set of a case observable
without depending on the emulator model. **The mock must also implement the
key-checked reload semantics that the host cases rely on** — a `REFRESH` write
reloads the counter **only** when it carries the documented key — and the mock
exposes the resulting **reload observable** (the counter value after the write)
and a **feed-outcome observable** (reload / no reload). Without that capability
the key-sensitivity cases (HS-P1, HS-P2) and the NC-1 discrimination have no
defined oracle; the mock's key semantics are therefore a **precondition of the
host suite**, to be pinned when the mock is written.

| ID | Case | Expected result |
|---|---|---|
| HS-P1 | a **periodic** feed with a wrong key — a **non-zero** value that is **not** `0xDEADC0DE` (so that a binding reloading on any non-zero key cannot satisfy it) | no reload (the mock's reload observable is unchanged); the binding reports the feed as failed |
| HS-P2 | a **periodic** feed (not only the initial configuration `REFRESH`) with the correct key `0xDEADC0DE` | the mock's reload observable shows the reload; the feed succeeds |
| HS-P3 | `MSVP > TIME` | the binding **rejects** the configuration before any write |
| HS-P4 | `TRIGGER > 4095`, `TIME > 16 777 200` | the binding **rejects** before any write |
**stimulus:** the binding is asked to configure the watchdog with a **deliberately reversed configuration request** (`TIME` before `TRIGGER`/`MSVP`); the mock model's **write observer** records every register write the binding performs, in order. The delivery mechanism is a test double of the binding's configuration entry point that hands over the reversed order — if the entry point carries no order parameter (as the PAL contract of F-10 does not require), the stimulus degenerates to the plain configuration request and the case reduces to the NC-3 check. **Oracle:** the observer's recorded sequence must show the four configuration writes in the binding's **own** relative order — `TRIGGER` → `MSVP` → `TIME` → `CONTROL` — as a **relative-order (subsequence) requirement**; other writes (the peripheral soft-reset release, the gating writes, the first `REFRESH` of R-7) may appear before or after, and the configuration read-back must then match. (The observer records the binding's writes, not the caller's intent, so the case distinguishes "the binding performed its own order" from "the binding performed the caller's order"; the relative-order form is used so that a conforming binding, which also performs the R-7 `REFRESH` write, is not failed.) |
| HS-P6 | read-back mismatch after configuration | the binding treats it as a **fatal init error** (F-12) |
| HS-P7 | `FORCE` with a wrong value | the binding reports the documented NMI-sequence semantics, not an immediate reset |
| HS-P8 | peripheral access while gated | the binding must not report success |

*Target-only under the current emulator scope* (a **running-machine** emulator leg might witness some of them — that is the open owner choice in agenda item F(e); the target set below is the minimum that only the device can witness):

| ID | Case | Expected result |
|---|---|---|
| TG-P1 | lock enforcement (re-write `TIME`/`TRIGGER` after configuration) | the write is **ignored** — the target's real behaviour |
| TG-P2 | expiry → reset | the processor is reset by the watchdog alone, with the reset scope observed |
| TG-P3 | forbidden window | a refresh inside the window is refused and the timeout interrupt is asserted |
| TG-P4 | key sensitivity on the target | a wrong key does not reload |
| TG-P5 | design-time pre-configuration | the power-on `TIME`/`TRIGGER`/`MSVP` and the lock state at boot are recorded |

*Negative control* (mutation-verified, **against the binding only**, so that the
control is discriminating for observables the model actually supports):

| ID | Mutation | Required effect | Why this case is the discriminating one |
|---|-----------|-----------------|------------------------------------------|
| **NC-1** | the binding omits the refresh key on the **nominal** feed path (the mutation is meaningful only if the host mock implements the key-checked reload semantics — §5.2 preamble) | **HS-P2 must fail** (the nominal feed must stop reloading) |
| NC-2 | the binding omits the bound validation | HS-P3/HS-P4 must fail | both cases expect a rejection before any write, so removing the check makes the binding accept an invalid configuration |
| NC-3 | the binding writes the registers in the wrong order | HS-P5 must fail | HS-P5 asserts the binding's **own** write order against the mock model's write observer (§5.2 preamble), so a wrong order is observable |
| NC-4 | the binding omits the configuration read-back | HS-P6 must fail | HS-P6 expects a fatal init error on a read-back mismatch, which the omitted read-back cannot detect |

### 5.3 Target / HIL legs

| ID | Leg | Status of the criterion |
|---|---|---|
| HL-1 | arm/feed/expiry/reset with the **interrupts masked** and in a **tight loop** | the criterion is **specified**; it needs the target and a threshold (M-10). This is the first **NC-PAL-01 closure condition** |
| HL-2 | the **pre-reset report reaches the ground interface before the reset** | specified; needs the target and the ground interface. The second **NC-PAL-01 closure condition** |
| HL-3 | the reset is observed at **system** level | specified; needs the target (F-11) |
| HL-4 | the timeout NMI is delivered and the timeout event is **cleared** | specified; needs the target (F-13, F-14) |
| HL-5 | the boot state is characterised | specified; needs the target (F-15) |
| HL-6 | the **actual inter-feed interval** and the worst-case feeder starvation are measured | specified; needs the target (M-16) |
| HL-7 | the reset → boot → application recovery path is timed | specified; needs the target and the recovery budget (M-10) |
| HL-8 | the **power-up default timeout** (≈10.47 s, F-19) is observed on the target: the device does not reset unexpectedly from a power-up in which the software arms correctly, and the software's arming completes inside that window | specified; needs the target and the design-time default (M-1). Closes the hazard named in F-19 |
| HL-9 | a **debug-halted** hart does not trip the watchdog and a **WFI** hart's wake-up behaviour is as configured (F-20) | specified; needs the target. A HIL leg run under a debugger is **not** representative of the running case |

### 5.4 Quantities to measure on the target (five distinct items)

| ID | Quantity | Note | Produced by |
|---|---|---|---|
| WC-1 | feeder-task **execution** bound (wake-up → `REFRESH` write → status read-back) | a software execution bound, **not** a scheduling guarantee | HL-6 (same instrumentation window) |
| LC-1 | **MMIO write latency** to the peripheral | a different quantity from WC-1 | HL-1 / HL-6 (instrumented) |
| JT-1 | feeder-task **scheduling jitter** under representative load | a third distinct quantity | HL-6 |
| IF-1 | the **actual inter-feed interval** | the requirement-level quantity (SRS-REQ-073) | HL-6 (the same measurement; cross-referenced) |
| **RP-1** | the **pre-reset report path** duration: panic-event formatting + console/UART write-completion-or-timeout + retries (ESC-1/ESC-5, NC-EXT-186) | the quantity the 250 ms upper-bound target applies to; **not** a feeder quantity, and currently unmeasured (see R-3) | HL-2 (the report-path leg; the leg must be extended to record the **duration**, not only the delivery) | the **pre-reset report path** duration: panic-event formatting + console/UART write-completion-or-timeout + retries (ESC-1/ESC-5, NC-EXT-186) | the quantity the 250 ms upper-bound target applies to; **not** a feeder quantity, and currently unmeasured (see R-3) |

The margin of R-5 is the combination of LC-1, JT-1 and the reaction-window
requirement, and it is **not computable** until M-2 and M-5 exist; the
acceptance thresholds are therefore owner inputs (M-10), not assumptions.

## 6. Owner input register (grouped — the missing controlled inputs)

*Identifier note:* the **M-9** identifier is **not used** — it was carried by
the rev 1.0/1.1 wording and was retired without a replacement, so the series
M-1..M-16 has one intentional hole. The retained reviews 0424/0425 refer to
the M-1..M-16 *range*; no finding depends on M-9.

| Group | Inputs | Why they block | Consequence while open |
| --- | --- | --- | --- |
| **G-A Controlled device documentation** | **narrowed in rev 1.4** to what is still not public: the **"PolarFire SoC Device Register Map"** — the watchdog register descriptions, reset values, the design-time MSS configuration — and the **RT PolarFire SoC "MS Description"** the errata points to (M-1); the authoritative **lock rule** (M-11); the authoritative **timeout unit** — µs versus prescaled ticks (M-12); the **design-time default load** behind the ≈10.47 s power-up timeout and the boot lock state (M-15). *Retrieved and now cited (§2.4):* the RTPFS160ZT-applicable errata (no watchdog item), the RT product overview, the MSS TRM, the Power-Up/Resets UG, the data sheet, AN4903 | the register semantics, counter units, lock behaviour, reset values and boot state are still only in the non-retrievable document; the family example driver remains the source for the register block | no `TIME`/`TRIGGER`/`MSVP` value can be chosen; the init sequence cannot be finalised; the binding cannot be validated; the ≈10.47 s power-up default cannot be confirmed |
| **G-B BSP / clock / gating** | the watchdog **clock source and frequency** for the flight configuration, and whether the prescaler is 256 on this device (M-2) — *the issuer's MSS TRM does not state the counter clock or the prescaler, and the register defaults are in the non-retrievable document; the TRM's ≈10.47 s power-up default is only **consistent with** a prescaler of 256 at a higher AXI clock — a **labelled inference**, not a value: it assumes that the design-time default load equals the cited driver's maximum (16 777 200) and that the prescaler is 256, and **neither** assumption is confirmed (the default load is in the non-retrievable register-map document and the prescaler is not stated in any retrieved document)*; the **peripheral gating state at boot** and the missing `CONTEXT_EN_MASK_WDOGx` values (M-8), and whether the watchdog has a **peripheral soft-reset bit** to release (F-23); whether the flight software can alter the watchdog clock or gating (F-11); the **counter/timebase behaviour in sleep, WFI or debug-halt states** (F-20) | every counter encoding, the refresh window and the independence argument depend on it | no timeout, no reaction window, no margin and no independence claim is possible; the "armed but inert" case cannot be excluded; the reaction-window ceiling stays unknown |
| **G-C Mission assignment** | the **mission application hart**, the watchdog **instance** and its **reset coverage** (M-3) — *reframed by F-18: only `WDOG0` (E51) can reset the MSS, and a U54 watchdog's timeout interrupt is delivered to the **E51**; the instance choice is therefore architectural, not a numbering preference*; the **board-level reset scope** — which external masters observe an MSS reset (M-7, M-14); whether a **second-level or out-of-band monitor** is required for the hang classes a MSS watchdog cannot cover (F-11) | instance selection and the FDIR protection argument | the wrong watchdog could be armed, and the protection of the mission function is unestablished |
| **G-D Mission timing values** | the **reset timeout** and the required **reaction window** (M-4) — *the window ceiling is `TRIGGER` ticks at an **unconfirmed** counter clock; the ≈41.9 ms figure in R-3 is conditional on the header's illustrative 25 MHz, and the TRM's power-up default is **consistent with** a roughly 16× higher counter clock, under which the same 4095-tick ceiling would be ≈2.6 ms — a **labelled inference**, not a value, resting on two unconfirmed assumptions (default load = the cited driver's maximum; prescaler = 256)*; the **feed cadence and margins** — jitter bound and MMIO write latency (M-5); the **false-reset tolerance** and the worst-case feeder starvation (M-16); the **forbidden-window** requirement and the admissible `MSVP` value to program while it is disabled (M-6); whether the **power-up default timeout** (≈10.47 s, F-19) is acceptable as the pre-arming behaviour | these are mission values, not engineering facts | the 8 s stays an unjustified placeholder; the cadence stays on the relative delay; the false-reset risk is unquantified; the report-vs-window question (M-13) cannot be answered without the clock |
| **G-E Verification** | the **HIL plan and acceptance thresholds** — reset-accuracy tolerance, trial counts, recovery budget (M-10); the decision on whether the **pre-reset report is also required on the watchdog-timeout path**, and with which budget (M-13) | without thresholds no HIL leg has a pass condition; the two paths' budgets are unrelated | no hardware-protection claim is possible; the N18 gate stays FAIL |
| **G-F Process** | the owner **decision on the controlled documentation basis** — the controlled datasheet, or the public-source-only basis with the recorded limitations (decision A) | the public driver is a mutable-branch, family-level source whose own lock documentation contradicts itself | the binding cannot be written against an uncontrolled basis without an explicit decision |

## 7. Owner decision agenda (grouped — a suggestion is not an approval)

| # | Decision | Options | Recommendation (not an approval) | Blocked by | Status |
| --- | --- | --- | --- | --- | --- |
| **A** | Documentation basis for the binding | (a) the controlled RTPFS160ZT datasheet + errata; (b) the public-source-only basis with the recorded limitations | (a); under (b) the counter values, the lock behaviour and the unit remain closed | G-A, G-F | **OPEN** |
| **B1** | Approve the **structure, the reference inventory and the separation** as the basis for an implementation design | (a) approve now; (b) return for more evidence | (a) — these parts are independent of the device documents and are sound | — | **OPEN** |
| **B2** | Approve the **content** decisions as the design basis | (a) approve once G-A..G-E are closed; (b) defer | (a), after the groups close — the content cannot be approved while the device-dependent inputs are missing | G-A..G-F | **OPEN** |
| **C** | Instance, hart and reset coverage — **reframed in rev 1.4 by F-18** | (a) `WDOG0` (E51) as the **only** MSS reset provider plus a U54 watchdog as a mission-hart hang detector (R-1(c)); (b) `WDOG0` alone; (c) a U54 detector only — accepting that it **cannot** reset the MSS; (d) an out-of-band monitor in addition; (e) the contingency if the target's `TIME`/`TRIGGER`/`MSVP` are **already locked at power-on** (the software then cannot arm and the fatal-init path of F-12/R-7 triggers) | **no recommendation** — the choice is architectural. What the evidence settles is the *negative*: a U54 watchdog alone delivers no MSS reset, so (c) alone does not meet the SRS-REQ-073 protection property | G-C, M-15 for (e) | **OPEN** |
| **D** | Mission timeout, reaction window, cadence and forbidden window | (a) keep 8 s and the relative delay as placeholders; (b) set justified values once G-B/G-D are supplied; (c) enable the forbidden window; (d) accept the **power-up default timeout** (≈10.47 s) as the pre-arming behaviour, or require the software to arm before it expires | (b) for the timeout and cadence once the clock is known; the window stays **disabled** (its `MSVP` value still has to be programmed). For (d) the evidence gives no preference: the software must close the pre-arming window (R-7, HL-8) whatever the value is | G-B, G-D | **OPEN** |
| **E** | Timeout-interrupt policy and false-reset tolerance | (a) the NMI is a "reset imminent" notification with a mandatory clear; (b) the reset is unconditional and the interrupt is informational; (c) accept a feeder-starvation reset as a system failure, with the worst case measured | (b) unless a reaction window is required — its ceiling is clock-dependent and currently unknown; (c) with HL-6 as the measurement. Note the routing fact from F-18: a **U54** watchdog's timeout interrupt is delivered to the **E51**, so the "notification" recipient depends on the instance chosen in item C | G-B, G-D, G-E | **OPEN** |
| **F** | Verification and the N18 gate | (a) approve the case placement of §5.1 and the sets of §5.2; (b) approve the HL legs with thresholds; (c) keep the N18 gate blocked/FAIL until the hardware inputs exist; (d) accept a **watchdog-specific** N18 assertion and an ELF-freshness binding; (e) decide whether the emulator leg stays **non-running** (write-sequence evidence only, as scoped in §5.2) or whether a running-machine leg is authorised, which could witness counter/timing behaviour the retained probe could not; (f) accept the new **HL-8** (power-up default timeout) and **HL-9** (debug-halt / WFI) legs | (a)+(b) once the thresholds exist; (c) as the interim, since the owner rejected operating without a watchdog; (d) is required for a future green N18 to mean anything; (e) changes the HIL burden and is a scoping decision; (f) follows from the retrieved issuer documents | G-E | **OPEN** |

**Implementation-ready design — not yet achieved.** The groups above are the
gap: without G-A (device documentation) and G-B (clock and gating) the
counter values, the init sequence and the independence argument cannot be
derived, so this document is a **review agenda**, not an implementation-ready
design. The owner decisions that remain are exactly those listed; no mission
value is proposed anywhere in this document.

## 8. What this document does not do

- It does not implement, and does not authorize the implementation of, the
  binding; it performs no register write.
- It does not change production code, the watchdog's fatal behaviour, the pins,
  the global configuration or the mission table.
- It does not introduce a "capability absent ⇒ non-fatal" mode: the owner has
  **rejected** starting without a watchdog (ESC-10); the alternative is to keep
  the N18 gate **blocked** and FAIL.
- It does not ratify the 8 s timeout, the 250 ms report bound, the feed cadence
  or any refresh-window value; the 250 ms figure is an **upper-bound design
  target**, not a required duration.
- It does not claim target, HIL or WCET evidence, and it does not use the
  emulator's estimated clock frequency as a target parameter.
- It closes no NC, passes no gate, and commits nothing.

## 9. Evidence index

| Artifact | Content |
| --- | --- |
| `docs/vv/evidence/2026-10-22-s8-fdir-wd-model-probe.log` | the retained pinned-image model probe: exact script, exact output, the measured read-backs and the scope limits (F-17) |
| `docs/vv/evidence/2026-10-22-s8-fdir-wd-spec-bsp-inventory.log` | the retained BSP inventory transcript (519 include files, the `mpfs|mss_` search, the watchdog headers, the `linkcmds` memory map, the `libchip` MPFS check) |
| `docs/vv/evidence/2026-10-22-s8-fdir-wd-official-docs-retrieval.log` | the **official issuer-document retrieval record**: the six Microchip documents (MSS TRM rev S, Power-Up/Resets UG rev M, data sheet rev E, RT product overview rev C, RTPFS160ZT-applicable errata rev A, AN4903) with URLs, HTTP status, sizes and SHA-256; the blocked URLs and the non-authoritative mirror; the quoted extracts behind F-18..F-24; the labelled inference; and the reproduction commands. The issuer's PDFs are **not** retained in the tree (N13 boundary) |
| `docs/vv/evidence/2026-10-22-s8-fdir-wd-spec-bsp-inventory.log` |
| `docs/vv/evidence/2026-10-22-s8-fdir-wd-spec-docs.sha256` | the documentation-phase identity manifest (this document, the proposal, the retained reviews, the escalation/status/V&V records, the changelog) |
| `docs/vv/evidence/2026-10-21-s8-fdir-post-review-source.sha256` | the code-scope manifest of the increment (34 entries) — **content unchanged and re-verified (34/34 `OK`, exit 0)**; it was **not re-derived** in this documentation phase, and no record pins the manifest file's own byte identity |
| `docs/vv/evidence/2026-10-21-s8-fdir-lean-build.log` | the retained Lean evidence on which §1's "Lean remains PASS per the retained evidence" rests (2026-10-21T09:58Z; 14/14 source hashes re-verified) |
| `docs/vv/evidence/2026-10-21-s8-fdir-gate-verification.log` | the N18 false-PASS transcript, the truthful re-execution, the corrected-runner runs, the Lean gate and the NC-EXT-197 TSan pair |
| `docs/vv/evidence/2026-10-21-s8-fdir-n18-current-*.log` | the current run's Renode process log and UART console (no marker; the fatal exit) |
| `docs/vv/evidence/2026-10-20-n18-*.log` | the genuine recorded 2026-10-20 successful run |
| `docs/reviews/2026-10-21-s8-fdir-embedded-safety-0424.md`, `…-vv-0425.md` | the reviews of **rev 1.0** (conclusions unchanged) |
| `docs/reviews/2026-10-22-s8-fdir-embedded-safety-0426.md`, `…-vv-0427.md` | the final-delta reviews of **rev 1.2** (conclusions unchanged; not restated as reviews of this revision) |
| `docs/reviews/2026-10-22-s8-fdir-embedded-safety-0428.md`, `…-vv-0429.md` | the dated **confirmation addenda of rev 1.4** (embedded safety and V&V), each recording the reviewed body identity by SHA-256; their findings are itemised in §10 |
| `docs/architecture/n18-watchdog-integration-proposal.md` | FRICK-ARCH-0023 rev 1.4, the options this specification refines |
| `docs/planning/sprint-plan.md` | NC-PAL-01, NC-EXT-186..202 (all OPEN) |

**Reproduction of the public reference hashes** (the files are not retained in
the tree — N13 boundary):

```
curl -sS -o mss_watchdog.h \
  https://raw.githubusercontent.com/polarfire-soc/polarfire-soc-bare-metal-examples/5a890a5b5693be7ce415ccece2f2283dbc5a8851/applications/mpfs-blank-baremetal/src/platform/drivers/mss/mss_watchdog/mss_watchdog.h
sha256sum mss_watchdog.h
```

## 10. Review findings → correction itemisation

The reviews of **rev 1.0** (FRICK-REVIEW-0424 embedded safety, FRICK-REVIEW-0425
V&V) are retained unchanged; their conclusions are not restated as reviews of a
later revision. The final-delta reviews **of rev 1.2** are FRICK-REVIEW-0426 (embedded safety)
and FRICK-REVIEW-0427 (V&V); they are **not** restated as reviews of a later
revision. The **confirmation addenda of rev 1.4** are FRICK-REVIEW-0428
(embedded safety) and FRICK-REVIEW-0429 (V&V): dated, independent, each
recording the reviewed body identity by SHA-256, each distinguishing the earlier
findings it confirmed as corrected from the points it re-checked independently.
Their findings are itemised below.

| Source | Finding | Severity | Correction in this revision |
|---|---|---|---|
| 0424 M-1 | independence argument and the single-hart residual absent | Major | F-11 rewritten: three **unproved** dependencies (clock/gating, reset scope, same-hart reaction leg); the clock-control assumption removed; HL-3 added |
| 0424 M-2 | the write-once lock's consequence (unrecoverable mis-arm) absent | Major | F-12 + R-7 read-back **before** the first `REFRESH`; TG-P1; M-11 carries the upstream contradiction |
| 0424 M-3 | hardware timeout vs the 250 ms report budget; deferred reset | Major | R-3 separates the reset timeout from the reaction window; R-7 states the deferral; M-13/D-item E |
| 0424 M-4 | emulator oracles wrong or non-executable | Major | §5.1 placement split per environment; §5.2 re-scoped |
| 0424 M-5 | µs-vs-tick contradiction; representable ranges | Major | F-16 + M-12; the R-3 range table |
| 0424 Mo-2..Mo-5 | NMI/timeout-event, feeder starvation, armed-but-inert, timebase | Moderate | F-13/F-14, R-5 (named hazard), R-6, F-11, G-B |
| 0425 A-01..A-08 | evidence chain unbound (references, BSP artefact, authorization record, NC list, citations, model identity) | Major/Minor | §2.2 retention position + reproduction command; retained BSP transcript; ESC-9/ESC-10 in the escalation record; NC-EXT-199/NC-TS-01 listed; model digests recorded |
| 0425 B-01..B-08 | emulator oracle defects and the `MSVP` deviation | Major | §5.1/§5.2 rewritten; R-4/R-7 always program `MSVP` |
| 0425 B-09..B-13 | missing cases and no negative control | Major | §5.2 EM/HS/TG sets and NC-1..NC-4 (mutating the **binding**) |
| 0425 C-01..C-06 | HIL plan, acceptance criteria, WCET/jitter conflation, missing legs, what would make N18 meaningful | Major | HL-1..HL-7 (incl. the two NC-PAL-01 closure conditions), §5.4 with WC-1/LC-1/JT-1/IF-1/**RP-1**, agenda item F |
| 0425 D-01/D-02 | the 250 ms mis-described; two stale "no watchdog model" records | Minor | §1 wording corrected; NC-EXT-200 registered for the production-source correction |
| 0425 E-01 | D-04/D-05 not actionable (contradictory reaction-window relation) | Major | R-3 corrected to `TRIGGER` ticks with the range table |
| 0425 F-01..F-04 | bound value, clock attribution, lock contradiction, gating values | Minor | F-3, F-6, F-2, F-8 |
| 0425 G-01 | neither document had an independent review | Major | 0424/0425 retained for rev 1.0; 0426/0427 obtained for this revision |
| parent (NC-EXT-202) | rev 1.1 internal contradictions: unevidenced model claims, the 250 ms overclaim, F-12/R-7 order, R-4's `MSVP` wording, R-3's bound, F-11's assumption, F-15/D-15, HL-1/D-01/D-10 wording, cross-reference drift | Major | all corrected in this revision: §2.3 + F-17 now rest on the **corrected** probe attribution; the 250 ms overclaim removed; the sequencing/bound/independence items reconciled; the agenda consolidated to A–F; the proposal aligned to rev 1.3/1.4 |
| 0426 F-0426-01 | the probe's register labels were wrong (0x08 is STATUS, 0x04 CONTROL, 0x0c TIME unprobed) | Major | §2.3 table, F-17 and the retained probe log corrected; the `TIME`-width claim withdrawn |
| 0426 F-0426-02 | the key-sensitivity conclusion exceeded the probe | Moderate | §2.3/F-17/§5.1 restated as "not established by the retained probe"; agenda item F(e) makes the running-machine leg an owner scoping choice |
| 0426 F-0426-03 | R-6 reintroduced the design-time-arming assumption | Moderate | R-6 reworded to "pre-initialised … whether that constitutes arming is M-15" |
| 0426 F-0426-04 | the unknown "actual path duration" was attached to feeder quantities | Moderate | R-3 now points at the **report-path** duration and §5.4 gains **RP-1** |
| 0426 F-0426-05/0427 F-12 | the documentation manifest identified rev 1.1, not rev 1.2 | Moderate/Major | the manifest is regenerated after the final text freeze; the code manifest is verified, not re-derived |
| 0426 F-0426-06 | the proposal's "lock after the first write" contradicts the cited source | Moderate | proposal rev 1.4 corrects the lock wording to the cited sources |
| 0426 F-0426-07..F-12, 0427 F-5..F-11, F-14..F-16 | cross-reference drift, the dangling F-2 pointer, the wrong RTEMS artefact, the unevidenced model parenthetical, absent review files, the `TIME`-read-back wording, the emulator order oracle, the M-9 hole, source line ranges, HS-P5 stimulus/oracle split, the probe header date, the missing Lean evidence row, the "not rewritten" wording | Minor | all corrected here: cross-references (§9, §5.4), F-2 pointer, §2 RTEMS row, R-4 parenthetical, the 0426/0427 reports and index entries, §2.3 wording, §5.1 order row, the M-9 note, the citations, HS-P5, the probe log header, the evidence index, and the manifest wording |
| 0427 F-13 | the revision cited review reports that did not exist | Major | the 0426/0427 reports are filed and indexed with this revision |
| 0427 F-17 | the surrounding records still pointed at rev 1.1 / D-01..D-15 | Minor | escalations, handover, loop-audit, verification-results, tool-qualification, sprint-plan and CHANGELOG updated to rev 1.3 / the A–F agenda |
| parent acceptance (round 2) | **the NC-1 negative-control oracle was not discriminating**: it required HS-P1 to fail, but HS-P1 is the *negative* case (wrong key ⇒ no reload), which a binding that omits the key entirely still satisfies | Major | **NC-1 now detects on the positive case HS-P2** (correct key ⇒ reload): omitting the key removes the reload and HS-P2 fails. HS-P1 remains the negative case, and the **healthy control** is stated explicitly: with the key present, HS-P1 and HS-P2 must **both** pass |
| parent acceptance (round 2) | the host case HS-P5 did not separate the caller's order from the binding's own order | Moderate | HS-P5 now states the **stimulus** (a deliberately wrong caller-supplied order) and the **oracle** (the mock model's ordered **write observer** must show the binding's own documented order `TRIGGER`, `MSVP`, `TIME`, `CONTROL`); the §5.2 preamble defines the observer and the programmable per-write response |
| parent acceptance (round 2) | metadata drift: the §5.4 heading said "four items" while the table had five (with RP-1); the evidence index cited the proposal at rev 1.3 (final rev 1.4); a pointer to a non-existent §7.5 | Minor | all three corrected; the §5.4 heading now says **five distinct items** |
| retrieval (this revision) | the **issuer's** documents were not obtained, so the instance/reset-scope question rested on a family example driver | Major | **Official Microchip documents retrieved** via the issuer's public download path (§2.4): the MSS TRM rev S, the Power-Up/Resets UG rev M, the data sheet rev E, the RT product overview rev C, the **RTPFS160ZT-applicable** errata rev A, AN4903. New facts **F-18..F-24**: only `WDOG0` (E51) can reset the MSS; the ≈10.47 s power-up default timeout; debug-halt and WFI behaviour; the 26-bit load field; `WDOG_RESET` reason bit 5 and the MSS (not device) reset scope; the peripheral soft-reset release mechanism; and no published watchdog erratum. **R-1's instance recommendation is withdrawn and agenda item C is reframed**; G-A is narrowed to the non-retrievable register-map document; HL-8/HL-9 are added |
| retrieval (this revision) | the "PolarFire SoC Device Register Map" (register descriptions, reset values, design-time configuration) and the RT "MS Description" | **OPEN** | recorded as **inaccessible from this environment** (issuer host edge-blocked, HTTP 403; the only alternate is a third-party, bot-protected mirror) and retained as the outstanding controlled input **M-1**, narrowed accordingly |
| 0428/0429 confirmation | the **§1 status bullet was textually corrupted** by the inserted rev-1.4 paragraph (the N18/Lean sentence was split) | Moderate | the paragraph was moved below the complete bullet; §1 reads as one sentence again |
| 0428/0429 confirmation | the **documentation identity manifest did not verify** and did not pin the reviewed body (8 of 16 entries FAILED) | Moderate | the manifest is **regenerated after this revision's text freeze**; the regeneration and its verification are recorded in the evidence index and in the handover |
| 0428/0429 confirmation | the **review-coverage attribution overstated** the coverage of the presented revision (the header and §10 called 0426/0427 "reviews of this revision", but they reviewed rev 1.2) | Moderate | the header now separates "Reviews of rev 1.2" from "Confirmation of rev 1.4", and §10 names the addenda as the rev 1.4 coverage |
| 0428/0429 confirmation | the **companion proposal still selected the superseded `wdog1` instance** and cited the specification at rev 1.3 | Moderate | proposal rev 1.5: the instance selection is withdrawn from Option A, the `WDOG0`-only reset capability of F-18 is stated, and the specification cross-reference is aligned to rev 1.5 |
| 0428/0429 confirmation | the **host mock's declared capability set** did not contain the key-checked reload semantics that NC-1's discrimination rests on | Moderate | the §5.2 preamble now declares the key-checked reload semantics, the reload observable and the feed-outcome observable as a **precondition of the host suite** |
| 0428/0429 confirmation | the **HS-P5 oracle was over-strict** (whole-sequence equality against the observer's full log, while R-7 also performs the first `REFRESH`) | Moderate | HS-P5 now requires the four configuration writes in the binding's own **relative order** (subsequence), which preserves NC-3's discrimination and removes the false-failure risk; the stimulus's delivery mechanism is stated |
| 0428/0429 confirmation | the **§5.1 emulator row** claimed the model witnesses write "sequence **and order**" (it exposes no ordered write log) | Minor | split into two rows: the sequence (register-level, yes) and the order (**host mock's write observer** only) |
| 0428/0429 confirmation | `RESET_SR` and `DEVRST_N` were used as facts but were **not carried by the retained retrieval record** | Minor | the two quotations are added to the retrieval log (the reset-reason register and the external device-reset path) |
| 0428/0429 confirmation | the **§5.1 host cells** for lock enforcement, forbidden window and gating had no host case | Minor | those cells now read "mock — **no host case yet** (open, NC-EXT-205)"; the functional plan is not expanded in this revision |
| 0428/0429 confirmation | no **leg ↔ quantity mapping** between §5.3 and §5.4 | Minor | §5.4 gained a "Produced by" column naming the leg that measures each quantity |
| 0428/0429 confirmation | the HS-P1/HS-P2 inputs were **unpinned** (wrong-key value; which feed instance is exercised) | Minor | HS-P1 pins a non-zero wrong key; HS-P2 exercises a **periodic** feed, not only the initial configuration `REFRESH` |
| 0428/0429 confirmation | the "**Target-only (the emulator cannot witness these)**" heading pre-empted the open owner choice F(e) | Minor | the heading is now scoped to the current emulator scope and points at agenda item F(e) |
| 0428/0429 confirmation | the §2.3 reason for the key non-discrimination was loosely worded ("not in the same model state") | Minor | reworded: the machine was never started and `REFRESH` was never read back, so **no reload observable** existed |
| 0428/0429 confirmation | F-21's "`PROGRAMMING` input (the design-time configuration path)" exceeded the retained record (labels only) | Minor | reworded to signal **labels only** |
| 0428/0429 confirmation | F-24's absence-of-erratum claim named only one of the two un-retrieved bounding documents | Minor | now bounded by both the RT "MS Description" and the Libero SoC software errata |
| 0428/0429 confirmation | editorial: §2.4 preceded §2.3; the revision-history rows were out of order; the §9 BSP row was duplicated; the negative-control table had a stray separator row; `verification-results.md` still described the manifest as 13 entries | Minor | all corrected |
| 0428/0429 confirmation | the labelled inference's **assumptions** were only in the retrieval log, not in the body | Minor | the assumptions (default load = the cited driver's maximum; prescaler = 256) and the "not confirmed" items are now stated in the body (G-B, G-D) |

## 11. Revision history

| Revision | Date | Change |
| --- | --- | --- |
| 1.0 | 2026-10-21 | Initial controlled specification for owner approval: reference inventory with hashes and applicability caveats, verified facts, recommendations, missing inputs, a proposed emulator test set, a verification plan and a decision table. No implementation, no register write, no policy decided. |
| 1.1 | 2026-10-22 | Applied FRICK-REVIEW-0424 and 0425: the reaction-window relation and representable ranges; the independence argument, the lock consequence, the timeout-versus-report budget, the NMI/timeout-event obligations, the feeder-starvation hazard, the design-time pre-configuration and the timebase assumptions; a rewritten emulator/host test set; the missing cases and a negative control; the NC-PAL-01 closure legs; the escalation record; the retained BSP inventory transcript and the model digests; an extended decision table and missing-input list. |
| 1.2 | 2026-10-22 | Parent acceptance correction: the model behaviour is now stated **only** from the retained pinned-image probe (no unevidenced assertions), with the unsupported claims downgraded and the case placement split per environment; the 250 ms overclaim ("provably cannot fit") removed and restated as an unknown actual path duration; F-12/R-7 read-back ordering, R-4's `MSVP` wording and R-3's bound reconciled against the pinned source; F-11's independence assumption and the M-13 attribution corrected; F-15/D-15 and the HL-1/D-01/D-10 wording aligned to current-vs-history; the decision table consolidated into a six-item agenda with a grouped owner input register (G-A..G-F); the authorization framing corrected (path A ≠ parameter approval; the model request ≠ watchdog approval). The reviews of rev 1.0 (0424/0425) are retained unchanged; this revision is reviewed by FRICK-REVIEW-0426/0427. |
| 1.3 | 2026-10-22 | Final-delta correction after FRICK-REVIEW-0426 (embedded safety) and 0427 (V&V): the model statements are re-derived from the **corrected** probe attribution (0x08 = STATUS, 0x04 = CONTROL, `TIME` 0x0c unprobed — the `TIME`-width claim is withdrawn, the key-sensitivity claim is downgraded to "not established by the retained probe"); the case-placement reasons are made probe-accurate; the 250 ms restatement is re-attached to the **report-path** duration with a new §5.4 item **RP-1**; the read-back ordering, the `MSVP` wording and the bound value are reconciled with the cited sources; the independence wording, the design-time-arming wording and the current-vs-history wording are corrected; the agenda is split into A, B1/B2, C, D, E, F with the emulator-leg scope and the boot-lock contingency made owner-visible; the M-9 identifier hole is explained; the citations, cross-references and evidence index are corrected; the §11 finding→correction itemisation is reinstated. No implementation, register write, production change, policy decision, NC closure or gate approval. |
| 1.4 | 2026-10-22 | Acceptance correction plus the official-document retrieval. **Test-plan logic:** NC-1's negative-control oracle was non-discriminating (it required the *negative* case HS-P1 to fail); it now detects on the **positive** case **HS-P2** (correct key ⇒ reload), HS-P1 remains the negative case, and the **healthy control** (both cases pass with the key present) is stated explicitly. HS-P5 now separates the **stimulus** from the **oracle** and is defined against the mock model's ordered **write observer**. **Metadata:** the §5.4 heading ("five distinct items"), the evidence index's proposal revision (1.4) and a dangling §7.5 pointer are corrected. **New evidence:** the six **official issuer documents** (§2.4) with their URLs, HTTP status, sizes, SHA-256 and the quoted extracts behind the new facts **F-18..F-24** — only `WDOG0` can reset the MSS; the ≈10.47 s power-up default timeout; debug-halt/WFI behaviour; the 26-bit load field; `WDOG_RESET` reason bit 5; the MSS (not device) reset scope; the peripheral soft-reset release mechanism; no published watchdog erratum. **Consequence:** R-1's instance recommendation is **withdrawn** and agenda item C is reframed; G-A is narrowed to the non-retrievable register-map document; HL-8/HL-9 are added. The "PolarFire SoC Device Register Map" and the RT "MS Description" are recorded as **inaccessible from this environment** and remain the outstanding controlled input M-1. **No parameter, clock, timeout, instance or reset policy is adopted; no implementation, register write, production change, NC closure, gate approval, commit or push.** |
| 1.5 | 2026-10-22 | Correction after the **confirmation addenda of rev 1.4** (FRICK-REVIEW-0428 embedded safety, FRICK-REVIEW-0429 V&V): the corrupted §1 status bullet repaired; the review-coverage attribution corrected (rev 1.2 reviews and rev 1.4 confirmations are now distinguished); the companion proposal aligned (rev 1.5 — the superseded instance selection withdrawn); the host mock's key-checked reload semantics, the reload observable and the feed-outcome observable declared as a precondition of the host suite; the HS-P5 oracle relaxed to a **relative-order** requirement and its stimulus's delivery mechanism stated; the §5.1 emulator row split (sequence vs order); `RESET_SR`/`DEVRST_N` added to the retained retrieval record; the §5.1 host cells without a host case marked as an open item; the §5.4 leg↔quantity mapping added; the HS-P1/HS-P2 inputs pinned; the target-only heading scoped to the current emulator scope; the §2.3, F-21 and F-24 wordings corrected; the labelled inference's assumptions stated in the body; and the editorial defects (§2.4 order, history order, §9 duplicate row, stray table row, the stale manifest entry count) corrected. The documentation identity manifest is regenerated after this text freeze. **No parameter, clock, timeout, instance or reset policy is adopted; no implementation, register write, production change, NC closure, gate approval, commit or push.** |
