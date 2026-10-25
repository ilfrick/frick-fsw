# Host watchdog model/contract cases — addendum (owner approval Telegram1447)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-VV-ADD-WDHOST-001                |
| Revision                         | 1.0                                    |
| Date                             | 2026-10-22                            |
| Norms                            | N8, N9, N10, N11, N20, N21, N22        |
| Status                           | **Bounded phase record — not a gate, not a qualification, not a commit** |
| Companion evidence               | `docs/vv/evidence/2026-10-22-wd-host3-host-model-contract.log` |
| Phase identity manifest          | `docs/vv/evidence/2026-10-22-wd-host3-identities.sha256` (new; the two original manifests are left unedited) |
| Frozen bodies (unchanged)        | FRICK-ARCH-0024 rev 1.5 (`9b09605c…73d40`), FRICK-ARCH-0023 rev 1.5 (`749ecafc…f35e5`), retrieval log (`54a44964…e36ed`), probe log (`64139566…6c9bc`) |

## 0. Authority record (Telegram1447)

**Owner approval Telegram1447, 2026-10-22 10:28 (reply to scope 1446), verbatim:**
*"IMPLEMENT AND VERIFY ONLY the three missing host watchdog cases (lock
enforcement, forbidden refresh window, clock/gating) AND corresponding stateful
mock/key semantics, narrow B1/F(a) scope. This is NOT all agenda approval, not
hardware design approval."*

**What this approval does and does not authorise.**

| Authorised (and done here) | NOT authorised (and not done here) |
| --- | --- |
| Write and execute the three missing **host** watchdog cases (lock enforcement, forbidden refresh window, clock/gating) as **host-model/contract** cases | Agenda items A, B1, B2, C, D, E, F are **not** approved; B1/F(a) approval is **not** inferred; the case set of FRICK-ARCH-0024 §5.2 is **not** approved |
| Write and pin the **corresponding stateful mock/key semantics** (host model fixture + link-time double) | The **MPFS/MSS binding is not written** (it does not exist — NC-PAL-01) and is **not** approved |
| Test-side files only: the fixture, the test, the minimal test registration/build wiring, this addendum, the evidence log, the new identity manifest, the review records | **No production code change**, no configuration change, no requirement change, no register access, no target/emulator/HIL probe |
| Synthetic test parameters only | **No** mission or device fact is adopted: no instance, reset scope, NMI policy, timeout, cadence, margin, clock, prescaler, window value, gating value or lock rule |
| The narrow B1/F(a) **scope** for those three cases | **No** gate, sprint, release or qualification pass; **no** NC closure or reassignment; **no** commit or push |

Retained results are untouched: **N18 (Renode) remains FAIL** (not re-run — no
target input changed) and **Lean remains PASS** (not re-run — no Lean input
changed; the Lean input set is `verification/lean/`, `src/`, `config/`, none of
which this phase touches).

## 1. Scope class, SUT and what is NOT exercised

> **HOST MODEL / PORTABLE-CONTRACT EVIDENCE ONLY.** The executable exercises
> (a) a **host test double** — `tests/stubs/ff_test_wd_model.c` plus the
> link-wrapped `ff_pal_wd_hw_*` backend in `tests/unit/test_wd_host_contract.c` —
> and (b) the **unchanged production PAL contract** (`src/core/pal/pal_watchdog.c`)
> on the host. It is **not** the MPFS/MSS binding (which does not exist,
> NC-PAL-01), **not** the Renode pinned model, and **not** target, HIL, board or
> register-level evidence. The properties checked are the internal consistency
> of the host model with the contract recorded in FRICK-ARCH-0024 rev 1.5
> (F-2, F-3/F-7, F-4, F-5, F-8) and the propagation of a backend refusal
> through the production PAL contract. No register value, timeout, clock,
> prescaler, instance, reset scope, gating value, window value or lock rule is
> claimed as a device or mission fact; the model's time/window/clock values are
> **synthetic test parameters only**.

**SUT table — exactly two *asserted* artefacts are exercised** (the host
monotonic clock is also called by `FF_PalWatchdogInit`, but its value is not
asserted and `FF_PalWatchdogCheck()` is not exercised — see R12).

| Artefact | Kind | What it contributes |
| --- | --- | --- |
| `src/core/pal/pal_watchdog.c` (unchanged) | production | the portable arm/feed/expiry/reset state machine and its feed accounting (`g_feeds` advances only after a successful backend feed) |
| `tests/stubs/ff_test_wd_model.c` + the link-time double | test fixture (V&V tool) | the peripheral register semantics: register file, down-counter, write-once lock, key-checked reload, forbidden window, gating, write observer |

**NOT exercised:** the MPFS/MSS binding (`pal_rtems.c` reports
`FF_NOT_IMPLEMENTED`), the emulator model, the target, the board, any register.

**Production-code coverage delta: zero.** The production-side property the new
cases reach — a refused backend feed is propagated and does not advance the feed
counter — is already established by `tests/unit/test_pal_watchdog.c:113-136`
(`test_watchdog_hardware_errors_propagate`), which is **preserved unchanged**.
What the three new cases add is a *model-level* qualification of the fixture and
a *contract-level* demonstration that the model is driven through the real PAL
entry point rather than by restating its own return value.

**Seam.** The new target link-wraps `ff_pal_wd_hw_arm`, `ff_pal_wd_hw_feed` and
`ff_pal_wd_hw_reset` (`-Wl,--wrap=…`), the same host-only injection mechanism the
project already uses for `nanosleep` and `write`. The wrap applies to **that
executable only**; `test_pal_watchdog` keeps the production host backend and
all of its oracles. The `FF_TEST_HOOKS` status-injection seam
(`FF_PalWatchdogTestSetHardwareResults`) is deliberately **not** used, because
the link-time wrap bypasses it.

**Upstream-identifier guard (defined here, so that it is auditable).** The
three new files must contain no identifier of the cited public example driver
or of the flight peripheral (for example the driver's watchdog macros, its
peripheral headers, its instance base addresses or its force-reset key).
Command actually run, and its result:

```
grep -nE 'MSS_WDOG|mss_peripherals|MSS_WDOG_|mss_watchdog|MICROCHIP|mss_config' \
  tests/stubs/ff_test_wd_model.c tests/stubs/ff_test_wd_model.h \
  tests/unit/test_wd_host_contract.c
-> exit 1 (no match)
```

**False-PASS guards.** (i) *Link level*: `nm` shows `pal_watchdog.c.o` carrying
the undefined references `ff_pal_wd_hw_arm/feed/reset` and the test object
defining `__wrap_ff_pal_wd_hw_arm/feed/reset`; the linked binary contains both
the wrapper symbols and the (unreferenced) real ones. (ii) *Run time*: every
case asserts the double's call counters (`g_double_arm_calls == 1`,
`g_double_feed_calls`), so a case that never reached the double cannot pass.

## 2. Model-semantics register (three classes)

Every behaviour of the fixture **and of the link-time double** is classified.
**No class-(B) or class-(C) entry is device truth.**

| # | Behaviour | Class | Source | "Not device truth" statement |
| --- | --- | --- | --- | --- |
| A1 | Register **identifiers** (REFRESH, CONTROL, STATUS, TIME, MSVP, TRIGGER, FORCE) and the offsets 0x00–0x0c | documented-and-cited | F-1 (names and instance base addresses); F-2 (the quoted "offset 0x00 to 0x0c" range) | The names are those of the cited public example driver; their applicability to the RTPFS160ZT is not established (M-1) |
| A1b | The offsets 0x10 (MSVP), 0x14 (TRIGGER), 0x18 (FORCE) | **model-only (inferred by 4-byte stride)** | — | F-1 documents the register names and the instance **base** addresses, not a per-register offset map; only the 0x00–0x0c range is quoted upstream. The remaining offsets are inferred and **not** device facts. No oracle depends on them: the lock boundary 0x0c is the documented one |
| A2 | Down-counter reloaded from `TIME`; timeout interrupt at the `TRIGGER` crossing; MVRP interrupt at the `MSVP` crossing — **F-4 minus its fourth clause** ("a reset is generated at zero"), which the fixture does **not** model | documented-and-cited (partial) | F-4 | Re-implements a documented statement; it is not evidence that the device behaves so. The zero-reset clause is not modelled — see C8 |
| A3 | Refresh reloads only when the write carries the documented key `0xDEADC0DE` | documented-and-cited | F-3 (the constant), F-7 (the positive statement), **§5.2 preamble** (the negative form "only when") | The published constant of the cited driver; the flight device's key is not independently established (M-1) |
| A4 | A refresh is forbidden while the counter is greater than `MSVP`, and attempting one asserts the timeout interrupt | documented-and-cited | F-5 | The device's status-bit encoding is not established; the model exposes booleans |
| A5 | The forbidden window is **disabled by default** | documented-and-cited | F-5 | The *enable mechanism* is a model flag (B2); F-5's statement is about the device's default, and the model reproduces that default only |
| A6 | A peripheral **gating mechanism exists** (a context-enable bit and a sub-block clock control); which bit to write is unknown | documented-and-cited | F-8 | The `CONTEXT_EN_MASK_WDOGx` values are **not defined** in the retrieved files (F-8, M-8); the boot gating state is unestablished. **F-8 says nothing about a gated access being refused** — that is C2 |
| B1 | The write-once lock of `TIME`/`TRIGGER`/`MSVP` — **both** upstream readings implemented, the default being the offsets-variant | unresolved-and-parameterised | F-2 / **M-11** | The upstream documentation contradicts itself; the default is a test convenience; the binding's rule must come from the controlled register map (M-1) and be verified on target (TG-P1) |
| B2 | The window-**enable** mechanism is an explicit model flag | unresolved-and-parameterised | F-5 | The device's enable mechanism is in no retrieved document; enabling it in the model is **not** the adoption of agenda item D(c) (OPEN) |
| B3 | The gating state is an explicit model flag | unresolved-and-parameterised | F-8 / M-8 | The gating bit values and the boot gating state are unestablished |
| B4 | The counter timeout unit is a third, synthetic domain | unresolved-and-parameterised | F-6 / **M-12** | µs versus prescaled ticks is unresolved; the fixture's tick domain has **no** defined relation to any device timebase, to the PAL's millisecond domain, or to the mission timeout |
| C1 | **The peripheral clock does not run while the peripheral is gated** | model-only | — | **Not established for the device.** A device that stops counting while gated is an *armed-but-inert* false-protection case (R-6/F-11); the fixture's behaviour must **not** be read as the benign case |
| C2 | A gated access is **refused**, and the refusal precedes the key and window checks | model-only | — | The disposition precedence and the refusal response are a fixture property, not a device taxonomy; the device's behaviour under gating is not established (M-8); the "armed but inert" case (R-6) is **not** closed |
| C3 | The tick domain, the synthetic `TIME`/`TRIGGER`/`MSVP` values (1000/10/900 ticks) and the explicit tick advance | model-only | — | No frequency, prescaler, unit, timeout, cadence or margin is implied; the real `(TIME−MSVP)·t_tick` window is not computable |
| C4 | The benign power-up state (unconfigured, not running) | model-only | — | The flight device comes out of reset **already armed** at ≈10.47 s (if VDD = 1.2 V) (F-19) — a difference in the unsafe direction that the fixture does not model; the fixture must never be used to reason about the pre-arming window (HL-8) |
| C5 | The disposition vocabulary `applied / ignored-locked / refused-gated / refused-window / refused-key` and its precedence (gated ▸ locked ▸ window ▸ key) | model-only | — | The taxonomy proposed in R-7 (*not writable*, *forbidden window*, *hardware fault*) is a proposal, not a device fact, and is not implemented by the fixture |
| C6 | The register widths and the control-register content | model-only | — | The control-register bit layout is not in the retrieved documents |
| C8 | The counter **halts at zero**; **no reset is generated at zero** and the fixture has no reset observable | model-only | — | F-4 states that a reset **is** generated at zero. The fixture does not represent it, and the reset-at-zero / "armed but inert" behaviour is **not** characterised here (the reset path is not exercised; HS-P7 stays NOT RUN) |
| C9 | The peripheral clock is modelled as running only **after** a first accepted keyed reload (the `loaded` term of the clock-running observable) | model-only | — | A fixture property of the clock-running observable; it is not a statement about the device's clock behaviour |
| C10 | A tick advance beyond zero is discarded (the counter stops at zero) | model-only | — | A fixture property of the tick advance |
| C11 | The write observer has a bounded capacity (64 records); an overflow is **flagged**, not silent; the **disposition** accessor returns an explicit *invalid* token for a non-existent write (the register/value/state accessors return a benign value — `STATUS` / `0` — and **none** of them can report the benign "applied" result); the out-of-range accessor paths are exercised by **no** case | model-only | — | The observer is a fixture facility; the *invalid* token and the overflow flag exist so that a query can never be answered with the benign "applied" result |
| C12 | The double's feed-success rule (counter read back after the write equals the reload value **and** differs from its pre-write value) and its reset hook (fail-closed, `FF_NOT_IMPLEMENTED`) | model-only | — | The feed rule is a host-model rule; the documented read-back obligation is the **configuration** read-back of F-12 / HS-P6, which is NOT RUN. The reset hook's device semantics (F-7) are not modelled |
| C7 | The configuration order reproduced by the double (`TRIGGER`, `MSVP`, `TIME`, `CONTROL`, `REFRESH`) | model-only (the cited driver's order) | F-2 | Reproduced so the write observer has something to observe; the binding's order must come from the controlled device documentation (M-1) |

## 3. The cases (HM-P1 … HM-P6)

**Case-id class.** The `HS-P` series of FRICK-ARCH-0024 §5.2 is **binding-scoped**.
The cases below use a **distinct series** (`HM-P`, *host model*) and discharge,
substitute and satisfy **no** frozen `HS-Pn`. The frozen `HS-P1`…`HS-P8` remain
**NOT RUN**.

**Common preconditions (every case that calls the PAL).** The model and the
double are reset; the synthetic parameters are set; `FF_PalWatchdogInit(12345)`
succeeds (the only path that resets the PAL's feed/reset counters). The full
precondition set — `GetFeeds() == 0`, `GetResets() == 0`, `GetTimeoutMs() ==
12345`, the double's arm counter == 1 — is asserted **in full only in HM-P1**;
the other cases rely on the successful init resetting the PAL's counters and
assert the feed counter at their first post-refusal observation.
`12345` is an arbitrary non-zero argument: it exercises the PAL's argument
validation only, it is **not** a timeout, and it is never converted into the
model's tick domain.

**No-vacuity rule.** Every expected value in the test source is a **literal
constant**; no assertion compares one model-derived value with another. Every
negative leg asserts at least one **state** observable that is not the
disposition token, plus the **exact** disposition token, so that a refusal for
the wrong reason fails the case. Every assertion that **discriminates** a reload from a
non-reload is issued from a counter value **observably different from `TIME`**
(the counter is moved first) and is compared against the pre-write value; the
two initial-state observations (the counter at the reload value, right after
the initialisation) rest on the initial keyed reload (the counter's value after the model's first accepted refresh) and are not
reload/no-reload discriminations.

| Case | Class | Stimulus | Discriminating observable (primary) | Expected result | Healthy control | Fault / negative | Mutation that must kill it |
| --- | --- | --- | --- | --- | --- | --- | --- |
| **HM-P1** write-once lock | nominal + OOB, **executed under both lock-rule variants**; the two readings are **asserted** to be discriminated by the variant leg (a control-register write lies inside the offsets-variant's lock range and outside the TIME-variant's) | after the configuration sequence, re-write `TIME` with 500 and `TRIGGER` with 20 | register read-back of `TIME`/`TRIGGER` | read-back stays 1000 / 10; observer disposition `ignored-locked`; model reports locked | the same write on a **not yet configured** model **is** applied (read-back 500) — "ignored" is distinguishable from "applied"; the configuration writes themselves are applied and read back | the re-writes are ignored | **MUT-1** (model stops enforcing the lock) and **MUT-6** (model drops the TIME-variant branch) → HM-P1 must fail |
| **HM-P2** forbidden refresh window | nominal + OOB | window enabled, counter advanced to 950 (> `MSVP` 900), refresh with the documented key | counter read-back | counter stays 950 (**not** reloaded to 1000); forbidden status raised; timeout interrupt raised; `GetFeeds() == 0`; disposition `refused-window` | **boundary**: at `MSVP`+1 (901) the refresh is still refused, at exactly `MSVP` (900) it is permitted and the maximum-value crossing is witnessed; below `MSVP` (890) the refresh is permitted → counter 1000; with the window disabled a refresh is permitted at a counter value above `MSVP` (950) | the refresh inside the window, and the one tick above the boundary | **MUT-2** (model drops the window check) → HM-P2 must fail |
| **HM-P3** peripheral gating and clock | nominal + OOB | gate the peripheral, then feed and advance the clock | counter value across an explicit tick advance + clock-running flag | the counter does **not** advance while gated (950 stays 950); the feed is refused and **not reported as success**; `GetFeeds() == 0`; disposition `refused-gated` | after ungating the clock runs again (940), the feed is accepted → counter 1000, `GetFeeds() == 1` | the gated access | **MUT-3a** (access accepted while gated) and **MUT-3b** (clock runs while gated) → HM-P3 must fail |
| **HM-P4** refresh-key class (negative) | OOB | refresh with **three distinct wrong keys** (`0x00000001`, `0xDEADC0DF`, `0xFFFFFFFF`; each non-zero and ≠ the documented key) | counter read-back unchanged and ≠ `TIME` | counter stays 950; `GetFeeds() == 0`; disposition `refused-key` for each | — (the negative case) | the wrong keys | **MUT-4b** (reloads on any non-zero key) and **MUT-4c** (reloads on every key except one specific wrong value) → HM-P4 must fail. *Why three values:* a single-value stimulus would not exclude the MUT-4c defect — this is the NC-EXT-205 HS-P1 wrong-key-class residual, closed **inside the model scope only** |
| **HM-P5** refresh with the documented key (positive) | nominal | counter advanced to 950, then refresh with the documented key | counter read-back | counter reloads to 1000; the feed is accepted; `GetFeeds() == 1`; disposition `applied` | — (the positive case) | the key omission | **MUT-4** (the double omits the refresh key on the nominal feed path) → **HM-P5 must fail** while HM-P4 still passes |
| **HM-P6** write-observer order | **harness-capability self-check** | the configuration sequence | a matched-subsequence **count** over the observer's ordered log, plus the recorded value of two writes | the configuration writes appear in the relative order `TRIGGER → MSVP → TIME → CONTROL`, checked as a **subsequence** so documented surrounding writes may appear before or after; the observer records the written values and did not overflow (a **guard**, not a discriminating assertion: the observer capacity is 64 and the case performs five writes, so the overflow path is exercised by no case); the total write count is additionally pinned to **5** (the five writes of the configuration sequence), so the only write whose position the subsequence check leaves free is the initial `REFRESH` record | — | the swapped order | **MUT-5** (the double swaps `MSVP` and `TIME`) → HM-P6 must fail |

**HM-P6 is not HS-P5 evidence and not NC-3 evidence.** Both the actor and the
oracle are the fixture; it demonstrates that the write observer can express a
relative order, nothing more.

## 4. Mutation protocol, nominal and mutated verdicts (recorded separately)

**Protocol.** Each mutation is applied **temporarily** to the fixture or to the
double, the single new test target is rebuilt, the new test is executed, and
the source is then restored and **digest-verified** against the pristine copy,
rebuilt and re-run. No production file is mutated at any point. A mutation
whose build fails is **not** a kill.

| Mutation | Artefact and change | Mutated build | Mutated run | Killed case (designated) | Other cases affected | Restore digest | Re-run |
| --- | --- | --- | --- | --- | --- | --- | --- |
| **MUT-1** | model: the locked branch is disabled | exit 0 | exit 1 (assertion failures) | **HM-P1** | none | match | exit 0 |
| **MUT-2** | model: the window check is dropped | exit 0 | exit 1 | **HM-P2** | none | match | exit 0 |
| **MUT-3a** | model: a gated access is accepted | exit 0 | exit 1 | **HM-P3** | none | match | exit 0 |
| **MUT-3b** | model: the counter advances while gated | exit 0 | exit 1 | **HM-P3** | none | match | exit 0 |
| **MUT-4** | double: the refresh key is omitted on the nominal feed path | exit 0 | exit 1 | **HM-P5** | HM-P2's and HM-P3's healthy legs also fail (they depend on a keyed feed) — **HM-P4 still passes**, as required | match | exit 0 |
| **MUT-4b** | model: a reload on any non-zero key | exit 0 | exit 1 | **HM-P4** | none | match | exit 0 |
| **MUT-4c** | model: a reload on every key except one specific wrong value | exit 0 | exit 1 | **HM-P4** | none | match | exit 0 |
| **MUT-5** | double: the configuration writes are issued in the swapped order | exit 0 | exit 1 | **HM-P6** | HM-P1 (the swapped write lands inside the lock window) and HM-P2's legs | match | exit 0 |
| **MUT-6** | model: the TIME-variant lock branch is removed | exit 0 | exit 1 | **HM-P1** (the lock observables under the TIME variant: the healthy-control lock assertion and the negative-lock legs) | none outside HM-P1; the variant-discriminating leg does **not** fail under this mutation — see R24 | match | exit 0 |

**Mutation harness retention.** The protocol is driven by
`build/wd-host3-mutations/run_mutations.sh`, a **run-local** script inside the
git-ignored `build/` tree (sha256 `ec465e5a…b4050`). It is **not** retained in
the repository: the owner authorisation for this phase covers the test
registration/build wiring and the mock fixtures, and adding a new tool under
`tools/scripts/` would extend that scope and add an N10 tool-record item. Its
retention is **deferred** with the tool record (§12) and its digest is recorded
in the phase identity manifest so that the exact harness is identifiable.

**Scope of the mutation evidence.** MUT-1…MUT-6 are **model-level
fixture-sensitivity evidence**: they prove that the new oracles are evaluated
and are not vacuous. They are **not** the frozen negative controls **NC-1…NC-4**,
which are defined *against the binding* (FRICK-ARCH-0024 §5.2). Because no
production file is mutated, this phase does **not** claim that the new cases
are sensitive to a defect in `pal_watchdog.c`.

**Qualification anchor.** The model qualification is bound to the fixture
digests recorded in the phase identity manifest
(`ff_test_wd_model.c` `b07163c3…a384`, `ff_test_wd_model.h` `fbfa9251…d77`,
`test_wd_host_contract.c` `bfa1e020…b23dc`; the phase identity manifest holds
the full digests). **Any change to the fixture
invalidates this qualification and requires re-qualification**; the mutation
protocol's restore digest is the re-qualification anchor.

## 5. NOT RUN register

| Item | Status | Reason |
| --- | --- | --- |
| HS-P1, HS-P2 (frozen, binding-scoped) | **NOT RUN** | the binding does not exist; superseded *in the model scope only* by HM-P4/HM-P5 |
| HS-P3, HS-P4, HS-P5, HS-P6, HS-P7, HS-P8 | **NOT RUN** | binding absent |
| NC-1, NC-2, NC-3, NC-4 | **NOT RUN** | defined against the binding; MUT-1…MUT-6 are model-level and are not NC-1…NC-4 |
| EM-P1, EM-P2 | **NOT RUN** | no emulator leg in this phase |
| TG-P1…TG-P5 | **NOT RUN** | device required (lock enforcement, expiry→reset, forbidden window, key sensitivity on the target, design-time pre-configuration) |
| HL-1…HL-9 (incl. HL-8, HL-9), WC-1, LC-1, JT-1, IF-1, RP-1 | **NOT RUN** | target/HIL required; no threshold or budget is adopted |
| §5.1 target cells: expiry→reset, reset scope, timebase/sleep behaviour | **NOT RUN** | device required |
| R-6 "armed but inert", NC-PAL-01 | **OPEN** | out of scope |
| N18 (Renode) | **FAIL (retained, not re-run)** | no target input changed in this phase |
| Lean | **PASS (retained, not re-run)** | no Lean input changed in this phase |
| Agenda A, B1, B2, C, D, E, F | **OPEN** | Telegram1447 is **not** B1/F(a) approval and not an all-gates approval |
| NC-EXT-205 | **OPEN** | see §7 |

## 6. §5.1 correspondence (frozen cell text reproduced; delta stated)

| FRICK-ARCH-0024 §5.1 cell (frozen, unchanged) | Delta supplied by this addendum | Nature of the delta |
| --- | --- | --- |
| write-once **lock enforcement** — *Host (mock model)*: "mock — **no host case yet** (open, NC-EXT-205)" | **HM-P1** (model scope, both lock-rule variants) | a **model-level substitute**; the binding-level host case remains unwritten; the device case remains TG-P1 |
| forbidden-**window** behaviour (refresh refused) — *Host (mock model)*: "mock — **no host case yet** (open, NC-EXT-205)" | **HM-P2** (model scope) | a **model-level substitute**; the device case remains TG-P3 |
| **peripheral gating** — *Host (mock model)*: "mock — **no host case yet** (open, NC-EXT-205)" | **HM-P3** (model scope) | a **model-level substitute**; the device case remains TG-P1/TG-P3 scope |
| refresh-**key sensitivity** — *Host (mock model)*: "yes" (the §5.2 preamble's precondition) | **HM-P4 / HM-P5** pin the mock's key semantics, which the §5.2 preamble requires "to be pinned when the mock is written" | the mock's semantics are now pinned **in the model scope**; the binding's key omission remains NC-1 (NOT RUN) |

The frozen bodies are **byte-identical**; this addendum does not modify them and
does not re-state or supersede them.

## 7. NC-EXT-205 disposition language (proposed only — the NC stays OPEN)

**Proposed partial disposition wording for the NC owner and the PA (NOT applied
here, because the NC register is a frozen-manifest entry):** *"A model-level
substitute for the three host properties was written under owner approval
Telegram1447 (FRICK-VV-ADD-WDHOST-001): HM-P1/HM-P2/HM-P3 pin the host model's
lock, forbidden-window and gating semantics, and HM-P4/HM-P5 pin the mock's
key-checked reload semantics as required by the §5.2 preamble. The
binding-level host cases remain unwritten, the frozen §5.1 cells still read
'no host case yet', the wrong-key-class residual (N-9) is closed in the model
scope only (three distinct wrong keys), and the frozen negative controls
NC-1…NC-4 remain NOT RUN. **The NC remains OPEN**, with no change of status,
owner, severity or disposition made by this phase."*

## 8. Forbidden readings (stated negatives)

Each of the following readings is **false** and must not be inferred from any
artefact of this phase:

- ❌ "the watchdog configuration lock is enforced" — only the **fixture's** lock
  rule is pinned; the device's lock rule is unresolved (M-11) and is target-only
  (TG-P1).
- ❌ "a forbidden-window refresh is refused" — only the **fixture** refuses it;
  the device's window-enable mechanism is not established; the device case is TG-P3.
- ❌ "access while gated is refused" / "the watchdog is protected against a
  gated peripheral" — only the **fixture** refuses it; the device's gating bits
  are undefined (F-8/M-8) and the device's counter behaviour while gated is not
  established; the "armed but inert" false-protection case (R-6) is **not** closed.
- ❌ "the watchdog is effective" / "the FDIR reset path is protected" /
  "hardware watchdog protection is verified" — no hardware watchdog protection
  is demonstrated by anything in this phase; NC-PAL-01, NC-EXT-188 and
  NC-EXT-198 are unchanged and `N22-TS5-82` remains a gap.
- ❌ "the reset or timeout semantics are verified" — the fixture has boolean
  flags; the device's timeout is a permanent NMI (F-13) and its reset scope is
  unestablished (M-7/M-14).
- ❌ "the feed period, timeout or reaction window is validated" — no cadence,
  margin, timeout or window is established; the PAL's millisecond domain and
  the model's tick domain are unrelated by construction.
- ❌ "SRS-REQ-073 is (partially) verified" — SRS-REQ-073's hardware clauses are
  not addressed; its traceability row and `N22-TS5-82` are unchanged.
- ❌ "the host suite now covers lock, window and gating" — the **binding-level**
  host cases are still unwritten; NC-EXT-205 remains OPEN; what exists is a
  model-level substitute.

**Value statement.** This phase produces **no new production-code evidence**;
its deliverable is a digest-identified, mutation-anchored host test double
**pending N10 qualification** (see §14), offered for future binding work, plus
an honestly labelled record.

## 9. Non-claims (reproduced verbatim in the evidence log)

No MPFS/MSS register behaviour, read-back or timing is claimed. No reset
independence, reset scope or reset coverage is claimed. No hardware or board
fidelity is claimed. No clock, prescaler, timeout, cadence, margin, instance,
window value, gating value or lock rule is claimed as a device or mission fact.
No bound-validation, read-back-mismatch, NMI/`FORCE` or peripheral-gating
**binding** behaviour is claimed (HS-P3…HS-P8 remain NOT RUN). No emulator
(EM-P1/EM-P2), target (TG-P1…TG-P5) or HIL (HL-1…HL-9) evidence is produced or
implied. N18 is not re-run and remains FAIL; Lean is not re-run and remains PASS
by retention. No NC is closed or reassigned; no gate, agenda item, parameter or
design is approved; no commit or push is made. No requirement coverage delta is
created: `docs/requirements/traceability.yaml` is **unchanged**, and this
addendum creates **no** `N22-*` identifier and is **not** a package file under
`docs/vv/test-spec/` (it therefore does not use the N22 merge schema of
`docs/vv/test-spec/README.md` §2 — the deviation is declared here).

## 10. Residuals carried by this phase

| # | Residual |
| --- | --- |
| R1 | **NC-EXT-205 remains OPEN** — the binding-level host cases are unwritten; this phase supplies a model-level substitute only |
| R2 | NC-EXT-205's HS-P1 wrong-key-class residual (N-9) is closed **in the model scope only** (three distinct wrong keys, MUT-4c killed); the binding-phase residual is inherited unchanged |
| R3 | **M-11** lock-rule contradiction unresolved; the fixture's default is a test convenience; neither variant is device truth |
| R4 | **M-12** timeout unit unresolved; the fixture's tick domain is a third, synthetic domain |
| R5 | **M-2 / M-8** counter clock, prescaler applicability, gating values and boot gating state unresolved; "gated ⇒ not counting" is a **model-only** assumption |
| R6 | **M-1** the "PolarFire SoC Device Register Map" remains non-retrievable: register descriptions, reset values, design-time MSS configuration, the authoritative lock rule, the window-enable mechanism, the power-up default load and the boot lock state are unestablished |
| R7 | **M-7 / M-14** reset scope unestablished; the fixture has no reset scope |
| R8 | **M-6** which `MSVP` to program while the window is disabled is unresolved; the fixture's 900 is synthetic and must not be read as the value to program |
| R9 | **M-4 / M-5 / M-10 / M-16** mission timeout, cadence, margins, acceptance thresholds and false-reset tolerance are untouched; this phase adopts **no** mission instance, reset, NMI, timeout, cadence or margin choice |
| R10 | The fixture's benign power-up state does not model the device's ≈10.47 s power-up default (F-19, **if VDD = 1.2 V**); the pre-arming window (HL-8) is untouched |
| R11 | "Armed but inert" (R-6/F-11) is **not** closed |
| R12 | The two time bases are unrelated by construction; `FF_PalWatchdogCheck()` is not exercised (it would use the host monotonic clock, not the model's counter) |
| R13 | The frozen negative controls NC-1…NC-4 remain NOT RUN; the frozen HS-P5 relative-order case remains NOT RUN; the new ordering assertion is a harness-capability self-check |
| R14 | The frozen bodies stay byte-identical and the two original manifests are left **unedited** — but their post-phase verification **result** changes (see §11) |
| R15 | Agenda items A, B1, B2, C, D, E, F remain OPEN; Telegram1447 authorises only the narrow slice implemented here |
| R16 | Production-coverage delta = zero; the eleven cases and oracles of `tests/unit/test_pal_watchdog.c` are preserved unchanged |
| R17 | The model qualification is bound to a digest; any fixture change invalidates it |
| R18 | N18 remains FAIL and Lean remains PASS — retained, not re-run — and neither may be cited as a result of this phase |
| R19 | This phase's deliverables are **not** a protection claim, an FDIR claim, a certification item, or evidence for any requirement |
| R20 | The F-4 **`TRIGGER` crossing is not exercised** (the counter never reaches the trigger value in any case); only the maximum-value crossing is witnessed, and the timeout interrupt that is asserted is the one raised by the window refusal |
| R21 | The F-4 **zero-reset clause is not modelled** (C8): the counter halts at zero and no reset is generated or observed; the reset path is not exercised (HS-P7 stays NOT RUN) |
| R22 | The mutation harness is **not retained in the repository** (run-local only, digest recorded); its retention is deferred with the N10 tool record |
| R23 | The forbidden-window **boundary is now pinned** (`MSVP`+1 refused, `MSVP` permitted); the boundary legs pin the model's strict inequality behaviourally (a `>=` defect would fail the exactly-at-`MSVP` leg), but the comparison is **not covered by a mutation** in this protocol |
| R24 | The **variant-discriminating leg** of HM-P1 (the assertion that contrasts the two upstream lock readings) is **not** killed by any of the nine retained mutations: under MUT-6 it **passes**, because the mutant makes the TIME variant stop locking altogether, which is exactly what that leg asserts. MUT-6 is killed by HM-P1's healthy-control lock assertion and its negative-lock legs, so the defect "the TIME-variant branch is deleted" does **not** survive; a mutant that collapses the two readings (so that the TIME variant also locks on a `CONTROL` write) would be required to evidence the variant leg's discriminating power, and it is not in the retained set |

## 11. Manifest statement (measured, not asserted)

**Pre-phase baseline, measured by read-only per-entry digest comparison on
2026-10-22 before any edit:** the 22-entry watchdog documentation manifest
verified **22/22** (exit 0) and the 34-entry code manifest verified **34/34**
(exit 0); the four frozen body digests matched.

**Post-phase result, measured.** The 22-entry documentation manifest verifies
**22/22** (exit 0) — none of its entries was modified. The 34-entry code
manifest verifies **33/34** (exit 1): the single deviating entry is
`tests/CMakeLists.txt` (**manifest entry 22**; entry 21 is
`src/transport/link/link_uart.c`), whose digest moved from `aa11c7be…e5a2` to
`51f525b5…5707`. The change is the registration and build wiring this phase
was authorised to make; it is required for the new case to be executable at
all (N8 completeness, N22 executable test). The manifest file is **not** edited
and **not** re-baselined. The
**manifest files themselves are not edited** and are not re-baselined: they
remain the historical immutable proof of the reviewed inputs.

**New phase identity manifest.** `docs/vv/evidence/2026-10-22-wd-host3-identities.sha256`
covers every file this phase created or changed, plus the pre- and post-phase
digests of the pinned entries whose content changed. It is the authoritative
identity record for this phase's own files.

## 12. Deferred global metadata (collision disclosed, not silently resolved)

The files that would normally carry this phase's records are entries of the
frozen watchdog documentation manifest and were therefore left byte-identical.
The following are **DEFERRED**, not skipped, and are not to be read as done:

| Deferred item | File (frozen entry) | Norm that would require it |
| --- | --- | --- |
| revision-history row for this phase | `CHANGELOG.md` | N16 |
| handover / resume-point note | `docs/status/handover.md` | N8 |
| loop-audit conformance entry | `docs/status/loop-audit.md` | N19 |
| V&V record entry for this phase | `docs/vv/verification-results.md` | N11 |
| tool record note for the new fixture/test (N10 status) | `docs/vv/tool-qualification.md` | N10 |
| NC-EXT-205 proposed partial disposition (see §7) | `docs/planning/sprint-plan.md` | N12 |
| owner-approval escalation record (Telegram1447) | `docs/status/escalations.md` | process |
| review index entry for the two new reviews | `docs/reviews/README.md` | N17 |
| retention of the mutation harness | `tools/scripts/` | N10 — **deliberately not made** in this phase (the owner authorisation covers the test wiring and the mock fixtures); the harness stays run-local with its digest recorded |
| requirement-coverage link for the new test file | `docs/requirements/traceability.yaml` | N4 — **deliberately not made**: the new cases cover a fixture against a specification, not a controlled requirement, so adding the file to a requirement's test list would inflate requirement coverage (RC-15). `make trace` passes unchanged. |

## 13. Gates executed in this phase

| Gate | Command | Result |
| --- | --- | --- |
| Configure (fresh run-local tree, canonical options; the in-tree generated ASN.1 tree used **as is**) | `cmake -S . -B build/wd-host3 -G Ninja -DCMAKE_BUILD_TYPE=Debug -DFF_USE_PUS_ASN1=ON` | exit 0 |
| Build (all targets) | `cmake --build build/wd-host3 -j$(nproc)` | exit 0 |
| Full host regression | `ctest --test-dir build/wd-host3 --output-on-failure` | exit 0 — **46/46 passed** (45 before, **exactly +1** added, none removed, renamed, skipped or disabled). The counts hold for the configuration the new target is built in (`FF_PLATFORM = native` and a GNU/Clang compiler); on other configurations the target does not exist and the inventory is unchanged |
| New case set | `ctest --test-dir build/wd-host3 -R wd_host_model_contract --output-on-failure` | exit 0 — **129 checks, 0 failures** |
| Existing watchdog oracle set (preserved) | `ctest --test-dir build/wd-host3 -R pal_watchdog --output-on-failure` | exit 0 |
| Format check (pinned clang-format 18, V&V image, `--pull=never`) | `docker run --rm --pull=never -v $PWD:/workspace -w /workspace frick-vv:local make format-check` | exit 0 |
| Traceability | `make trace` | exit 0 — PASS (120 requirements, 68 Lean theorems) |
| N13 licence check | `python3 tools/scripts/license_check.py` | exit 0 |
| N13 licence-policy test | `python3 tests/python/test_license_check.py` | exit 0 — 14 tests OK |
| Mutation protocol | `bash build/wd-host3-mutations/run_mutations.sh` (run-local) | 9 mutations, each killed by an assertion failure with a build exit of 0; every restore digest-verified; every re-run exit 0 |

Not executed, and why: **Lean** (no Lean input changed), **N18 / Renode** (no
target input changed), **target / HIL / QEMU** (out of scope and not permitted),
**RTEMS cross build** (no target-side change), **`make smoke`** (unrelated to
this phase; the pre-existing third-party blocker is untouched), **MISRA**
(advisory scan, unchanged scope).

## 14. Review status

| Review | Profile | Scope | Status |
| --- | --- | --- | --- |
| Preflight | embedded safety (independent, fresh context) + senior SW V&V (independent, fresh context) | verification design, observable/oracle design, the test-versus-binding distinction, the required wording changes | **completed before implementation**; the records are retained as `docs/reviews/2026-10-22-wd-host3-preflight-embedded-safety.md` and `docs/reviews/2026-10-22-wd-host3-preflight-vv.md`, and their required changes are incorporated in this addendum |
| Post-implementation | embedded safety (independent, fresh context) | the final tests, the oracles, the exact input identity, the residuals | `docs/reviews/2026-10-22-wd-host3-embedded-safety-0433.md` — APPROVED WITH REQUIRED CHANGES; the required changes are applied in the final artefacts and re-verified by the gates in §13 |
| Post-implementation | senior SW V&V (independent, fresh context) | the final tests, the oracles, the exact input identity, the residuals | `docs/reviews/2026-10-22-wd-host3-vv-0434.md` — APPROVED WITH REQUIRED CHANGES; the required changes are applied in the final artefacts and re-verified by the gates in §13 |
| **Delta re-review** | embedded safety (independent, fresh context) | the **corrected** artefacts, by exact digest | `docs/reviews/2026-10-22-wd-host3-embedded-safety-0433-delta.md` — **DELTA ACCEPTED WITH RESIDUAL FINDINGS**; no blocker; its required record-only corrections are applied in this revision (MUT-6 kill attribution, mutation range, R24) |
| **Delta re-review** | senior SW V&V (independent, fresh context) | the **corrected** artefacts, by exact digest | `docs/reviews/2026-10-22-wd-host3-vv-delta-0435.md` — **DELTA ACCEPTED WITH RESIDUAL FINDINGS**; no blocker; its required record-only corrections are applied in this revision (C-1…C-4) |

The fixture is a **V&V tool** (N10): its correctness is the entire evidence
basis of these cases, so its qualification requires an independent review of the
fixture implementation. Until that review is recorded, the cases must not be
described as "verified".
