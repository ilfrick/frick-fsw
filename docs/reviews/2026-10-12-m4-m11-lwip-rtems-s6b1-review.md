# Review Report

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0150                      |
| Reviewer session                  | independent agent (norm N9), fresh context |
| Reviewer profile                  | **R1+R2+R3+R4+PA** — R1 (Senior FSW / real-time / space software developer) + R2 (Senior SW V&V engineer) + R3 (Senior avionics / systems engineer, N14) + R4 (Senior SW documentation engineer, N17) + PA (Senior SW Product Assurance engineer, N20) |
| Reference revision                | `3822f72` (HEAD) + **uncommitted** Sprint-6b-1 working-tree increment (in-repo lwIP RTEMS `sys_arch` port) |
| Items under review                | `src/hal/net/lwip/rtems/{sys_arch.c,lwipopts.h,arch/cc.h,arch/sys_arch.h}` (new); `CMakeLists.txt` (`if(FF_USE_LWIP)` block); requirement **SRS-REQ-040** in `docs/requirements/{traceability.yaml,traceability.md}`; `docs/planning/sprint-plan.md` (S6b row, NCs, rev 0.11); `docs/vv/{verification-results.md (0.39),renode-environment.md (0.18)}`; `docs/status/{handover.md,loop-audit.md,status.json}`; `CHANGELOG.md`; `README.md`; the NCs **NC-S6-01** (narrowed), **NC-S6-02**, **NC-S7-01** |
| Date                              | 2026-10-12                             |
| Scope                             | Combined R1/R2 (code + V&V) + R3 (systems/requirements, N14) + R4 (documentation, N17) + PA (product assurance, N20) review of the Sprint-6b-1 lwIP-RTEMS-`sys_arch` increment: correctness/robustness of the port against the lwIP 2.2.1 `sys` API and the RTEMS 6.2 classic API, whether the CMake port selection can break the host build, first-hand re-execution of the host (`FF_USE_LWIP=ON`), default and RTEMS cross builds, manifest/requirement accuracy, documentation currency, the honesty of the NC disposition and the residual gap (GAP-A5). ECSS-E-ST-40C Rev.1 (SW development / V&V), ECSS-Q-ST-80C Rev.2 (SW product assurance), project norms N1–N20 (`docs/rules.md`), reviewer contract `docs/engineering/independent-review.md` (rev 0.7) |
| Independence declaration          | The reviewer had no part in authoring, building-for-delivery, or previously reviewing the items under review. This review ran in a fresh context; every declared claim was re-executed or read first-hand (see §10 Evidence ledger). No author context was inherited and no clarifications were taken from the author. |
| Norms applied                    | N1–N20 (see `docs/rules.md`)           |

## 1. Scope

**In scope (verified first-hand):**

- **Part A — R1 (FSW code).** Correctness and robustness of the new lwIP 2.x
  `NO_SYS=0` system abstraction in `src/hal/net/lwip/rtems/sys_arch.c` against
  the **pinned** lwIP `STABLE-2_2_1_RELEASE` `src/include/lwip/sys.h` contract and
  the RTEMS 6.2 classic API semantics: completeness of the required `sys_*` entry
  points, semaphore/mutex/mailbox/task/time usage, absence of dynamic allocation,
  boundedness, ISR-callability of `sys_mbox_trypost_fromisr()`, monotonic
  `sys_now()`, the `arch/cc.h` platform macros, and the mailbox/thread sizing in
  the RTEMS `lwipopts.h`.
- **Part A/bis — CMake selection.** That the `if(FF_USE_LWIP)` branch selects the
  in-repo RTEMS port **only** for `FF_PLATFORM=rtems`, that the host build still
  uses the upstream lwIP unix port, that the single-`lwipopts.h` claim holds, and
  that the change cannot break the default (`FF_USE_LWIP=OFF`) or host builds.
- **Part B — R2 (V&V).** Independent re-execution of: (a) host
  `cmake -S . -B <dir> -DFF_USE_LWIP=ON && cmake --build && ctest`
  (expected 23/23), (b) the default native build + `ctest` (expected 22/22),
  (c) the RTEMS cross build in the pinned container
  (`frick-rtems-toolchain:local`,
  `-DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-mpfs.cmake -DFF_PLATFORM=rtems -DFF_USE_LWIP=ON`,
  expected a linked image), plus `trace`, `metrics --check` and `clang-format
  --dry-run`. Falsification attempts: does the host build still use the unix
  port? is `sys_arch.c` actually in the RTEMS link? is the Renode image
  affected?
- **Part C — R3 (systems/requirements, N14).** SRS-REQ-040 manifest/mirror
  accuracy after adding the port, counting consistency (114/71/29/14), and the
  milestone/objective tracking consistency of the plan set.
- **Part D — R4 (documentation, N17).** Currency and internal consistency of
  every touched document, header revision vs last `## History` row (N16),
  accuracy of the NC recording (owner + scheduled sprint) and absence of
  over-claim.
- **Part E — PA (product assurance, N20) + scope conformance.** Honesty and
  correct scoping of the NC dispositions, the evidence on which the increment is
  declared delivered, and the residual items a qualified human must still accept.

**Explicitly excluded (recorded so the scope is bounded):**

- Behaviour of the lwIP stack **on the target** (netif/PHY binding, ISR input
  path, TCP correctness, timing, stack high-water): not runnable in this
  increment and already registered as **GAP-A5** / **NC-S6-01**/**NC-S6-02**.
  The runtime `tcpip_init()` path cannot even succeed yet (NC-S6-02).
- The lwIP third-party sources themselves (`third_party/lwip`, fetched, pinned,
  BSD-3): reviewed only as the API contract the port must satisfy.
- Unchanged increments re-verified only as aggregate regressions (default build
  22/22, Renode image identity).
- `verification/lean` (`lake build`) was not re-run: the increment adds no
  Lean-modelled artifact (`lean: []` + `lean_note` for SRS-REQ-040, norm N1
  interpretation note 1) and touches no Lean source.
- The SSS (`docs/requirements/SSS.md`) was **not** modified, so the N14
  SSS-modification trigger is not active; the R3 part of this review covers the
  SRS/manifest/plan side of the increment only.

## 2. References

- Project norms: `docs/rules.md` (N1–N20); reviewer contract and profiles:
  `docs/engineering/independent-review.md` (rev 0.7); `AGENTS.md`, `README.md`.
- Traceability manifest/gate: `docs/requirements/traceability.yaml`,
  `tools/scripts/trace.py` (N4); sprint plan `docs/planning/sprint-plan.md`.
- ECSS-E-ST-40C Rev.1 (30 Apr 2025), ECSS-Q-ST-80C Rev.2 (30 Apr 2025);
  ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2 (context only).
- lwIP `STABLE-2_2_1_RELEASE` (pinned): `src/include/lwip/sys.h`, `opt.h`,
  `arch.h`, `src/api/tcpip.c`, `src/core/{init,sys,timeouts}.c`, `src/Filelists.cmake`.
- RTEMS 6.2 classic API (verified at the issuer, `docs.rtems.org` main branch,
  last updated 2026-06-17, accessed 2026-10-12): Message Manager directives
  (`rtems_message_queue_create/send/receive/delete`), Semaphore Manager
  directives (`rtems_semaphore_create/obtain/release/delete`), configuring a
  system (`CONFIGURE_MAXIMUM_MESSAGE_QUEUES`).
- Project artefacts: `config/rtems/rtems_config.c`, `docs/architecture/network-stack.md`
  (FRICK-ARCH-0012), `docs/development-plan.md`, `docs/quality/{licensing.md,class-a-gaps.md}`.

## 3. Part A — R1 (FSW code) findings

### R1-01 — **Minor** — `sys_thread_new()` fails silently; `tcpip_init()` then reports success with a dead stack

`frick_lwip_slot_acquire()` reserves one of `FRICK_LWIP_THREAD_SLOTS` (=8) static
start records and `sys_thread_new()` returns `{RTEMS_ID_NONE}` on pool
exhaustion, on `rtems_task_create()` failure or on `rtems_task_start()` failure
(`sys_arch.c:53,124-140,448-492`). The pinned lwIP
`tcpip_init()` **ignores** the return value and asserts only on mbox/mutex
creation failure (`src/api/tcpip.c`, `tcpip_init`: last statement
`sys_thread_new(...);` with no use of the result). Consequence: if the single
`TCPIP_THREAD_PRIO` task cannot be created (pool exhausted, `CONFIGURE_MAXIMUM_TASKS`
exhausted, invalid priority), `tcpip_init()` returns success, the LOCK/UNLOCK
core is never serviced and every subsequent netconn/socket call blocks or
degrades with **no diagnostic** — the same class of "silent configuration
failure" the port's own design principle (fail loudly, as in `cc.h`'s fatal
assert) intends to avoid. Evidence: `sys_arch.c:467-472,480-489`;
`third_party/lwip/src/api/tcpip.c` (`tcpip_init`).
**Required correction:** make the failure loud — `LWIP_PLATFORM_ASSERT()` /
`rtems_fatal(RTEMS_FATAL_SOURCE_*)` on slot exhaustion and on task-create/start
failure (the port already declares fatal asserts for lwIP invariants), or
document the silent-invalid-handle contract and its consequence in the port
header. Bounded, low-risk fix; suitable for the S6b-1 delta.

### R1-02 — **Comment** — mailbox wait-queue discipline and the full-queue busy-wait are undocumented policy choices

The port creates mailboxes with `RTEMS_FIFO | RTEMS_LOCAL`
(`sys_arch.c:315-316`). FIFO is the RTEMS default, but for a priority-scheduled
flight image the priority discipline (`RTEMS_PRIORITY`) is the RTEMS-recommended
choice for the task wait queue (verified at the issuer: Message Manager,
"the priority discipline is selected by the `RTEMS_PRIORITY` attribute"), and
the framework otherwise reasons in priorities (a priority-inheriting mutex is
used for the lwIP core lock, `sys_arch.c:270-272`). In addition,
`sys_mbox_post()` cannot block by construction, so a full queue is handled by an
**unbounded** retry loop with `rtems_task_wake_after(1)` (`sys_arch.c:340-350`)
— functionally correct (the lwIP contract is "may not fail… blocks if full") but
implemented as a 1 ms poll whose worst-case latency/priority-inversion behaviour
is unanalysed. Recommendation: choose the wait discipline deliberately
(priority vs FIFO) and record the full-queue policy + its boundedness argument
in `docs/architecture/network-stack.md`; note that all port objects share the
4-character name `'LWIP'` (`sys_arch.c:43`), which degrades observability in
RTEMS object listings — consider distinct names for mbox/sem/mutex/task.

### R1-03 — **Comment** — constant `1` returned by `sys_arch_sem_wait()`/`sys_arch_mbox_fetch()` is contract-compliant; record it

Both wait functions return `1u` on success instead of the elapsed milliseconds
(`sys_arch.c:255,400`). This **complies** with the pinned lwIP 2.2.1 contract
(`sys.h`: "SYS_ARCH_TIMEOUT on timeout, **any other value** if a message has been
received"), and independent inspection confirms no caller depends on the
magnitude: the only in-core consumers compare the result against
`SYS_ARCH_TIMEOUT` (`src/api/tcpip.c` `tcpip_mbox_fetch`), while timeout
accounting is computed from `sys_now()` via `sys_check_timeouts()`. The value is
therefore **not** the timer-accuracy hazard it would be in older ports.
Recommendation: a one-line comment in the port (or a note in the architecture
doc) stating "constant success value; elapsed time deliberately not measured,
compliant with the 2.2.1 contract" so a future maintainer does not port the
older "elapsed ms" convention requirement.

### R1-04 — **Comment** — fatal/assert path uses `printf` and a non-project fatal code

`LWIP_PLATFORM_ASSERT` prints via `printf` and calls
`rtems_fatal_error_occurred(0xDEADBEEFu)` (`arch/cc.h:43-47`). lwIP asserts can
fire in interrupt context (netif input path, `sys_mbox_trypost_fromisr` chain),
where `printf` is not ISR-safe and can deadlock on the console driver; and the
project's other RTEMS code uses the documented fatal-source API
(`src/core/pal/pal_rtems.c:56`, `rtems_fatal(RTEMS_FATAL_SOURCE_EXIT, 0)`).
Recommendation: document/align the fatal policy (either a documented RTEMS
`FATAL_SOURCE`, or an explicit note that the assert path is task-context-only
plus an ISR-safe alternative).

### R1-05 — **Comment** — the optional lwIP stack changes the compile environment of the whole framework (interference/partitioning hygiene)

The lwIP and port include directories propagate `PUBLIC` from `frick_lwip`
through `frick_net_lwip` to `frick_fsw_core` (`CMakeLists.txt:184-207`).
Verified in the generated flags: with `FF_USE_LWIP=ON` **every**
`frick_fsw_core` translation unit is compiled with
`-I…/src/hal/net/lwip/rtems -I…/third_party/lwip/src/include` (so `lwipopts.h`,
`arch/cc.h`, `arch/sys_arch.h` and the whole third-party `lwip/` header tree sit
on the global search path); with the flag OFF they do not. The behaviour is
pre-existing (the unix-port includes were already propagated), but the increment
touches exactly this block and now adds an `arch/` directory to the path.
Constructive code generation is unaffected (independently checked: sampled core
objects are `.text`-identical with the flag ON and OFF), so this is a hygiene
finding, not a defect. Recommendation: scope the lwIP includes to the lwIP
targets (`PRIVATE`/`INTERFACE` via the adapter only, or
`$<BUILD_INTERFACE:>`) so the optional third-party stack cannot influence the
framework's compile environment.

### R1-06 — **Comment** — TCP ISN entropy source is a non-cryptographic PRNG (mission-network security consideration)

`lwip_rtems_rand()` (xorshift32 seeded from the boot tick counter,
`sys_arch.c:170-186`) is bound to `LWIP_RAND()` in `arch/cc.h:56-57` and feeds
the TCP initial sequence numbers/ephemeral ports. It is not a CSPRNG (RFC 6528
recommends a strong ISN source) and the seed is predictable. It is **not a
regression** (lwIP's default is `rand()`), and the port is not yet networked on
target, so this is a Comment: record it as a security consideration and plan a
mission-provided CSPRNG where TCP is exposed.

### R1 factual assessment of the port (verified, no finding)

- **API completeness:** all `NO_SYS=0` entry points required by the pinned
  `sys.h` are defined exactly once: `sys_init`, `sys_arch_protect`,
  `sys_arch_unprotect`, `sys_sem_{new,free,signal}`, `sys_arch_sem_wait`,
  `sys_mutex_{new,lock,unlock,free}`, `sys_mbox_{new,free,post,trypost,trypost_fromisr}`,
  `sys_arch_mbox_{fetch,tryfetch}`, `sys_thread_new`, `sys_now`, `sys_jiffies`,
  plus the `arch/sys_arch.h` `sys_*_valid`/`sys_*_set_invalid` macro set
  (correctly defined before lwIP's `#ifndef` fallbacks). No duplicate symbol
  exists with the unix port (the RTEMS build links cleanly; the host build does
  not compile the RTEMS port — see Part B).
- **No dynamic allocation**: the port allocates only RTEMS kernel objects; no
  `malloc`/`calloc`/`realloc` appears in the four new files. Sizing is static and
  bounded (8 thread start slots, 8 KB minimum stack, positive mailbox depths).
- **RTEMS API usage is valid** (verified at the issuer): counting semaphore with
  `RTEMS_PRIORITY` discipline and local scope is a legal attribute set;
  `RTEMS_BINARY_SEMAPHORE | RTEMS_PRIORITY | RTEMS_INHERIT_PRIORITY` is the
  documented mutex combination (local scope + priority discipline present);
  `priority_ceiling = 0` is ignored without a ceiling protocol.
- **ISR contract**: `sys_mbox_trypost_fromisr()` delegates to
  `rtems_message_queue_send()`, which the RTEMS Message Manager documents as
  callable from interrupt context and which does **not** block (`RTEMS_TOO_MANY`
  when full) — the correct try-post-from-ISR semantics.
- **Timeout conversion**: `frick_lwip_ms_to_ticks()` rounds up and never returns
  0 for a non-zero millisecond timeout (`sys_arch.c:77-90`); with
  `CONFIGURE_MICROSECONDS_PER_TICK 1000` (`config/rtems/rtems_config.c:48`) the
  tick is 1 ms, so lwIP's millisecond timeouts are representable and
  `sys_now()` has 1 ms resolution (adequate for lwIP timers).
- **Monotonic time**: `sys_now()`/`sys_jiffies()` derive from
  `rtems_clock_get_ticks_since_boot()` (`sys_arch.c:152-166`), monotonic over the
  run; the 32-bit millisecond wrap (~49.7 days) is the standard lwIP `u32`
  time-base contract (lwIP compares with wrap-safe macros), so this is
  acceptable and worth one line of documentation rather than a finding.
- **Message size consistency**: created with `sizeof(void *)` and posted/received
  with the same size, satisfying RTEMS' rule that the receive buffer must be
  large enough for the queue's maximum message size.
- **Style/licence**: the four files are `-Wall -Wextra` clean on host *and*
  RTEMS (fresh builds, 0 warnings), `clang-format --dry-run --Werror` clean, and
  carry the AGPL-3.0-only SPDX header (N13). The port is project-authored; the
  aggregate with BSD-3 lwIP is already analysed in `docs/quality/licensing.md`
  (line 45) — a one-line note that the port is now *linkable into the target
  image* would complete the licence record (Comment).

## 4. Part B — R2 (V&V) findings

### R2-01 — **Minor** — the recorded RTEMS evidence is not what the delivered build does: the image contains **no** lwIP code, yet the record claims `tcpip_init`/`lwip_socket` are "link-checked"

`docs/vv/verification-results.md` (rev 0.39), `CHANGELOG.md`, `docs/status/status.json`,
`docs/status/handover.md`, `docs/status/loop-audit.md`, `docs/planning/sprint-plan.md`
and `README.md` state that the RTEMS image "**builds and links** with lwIP …
`tcpip_init`/`lwip_socket` **link-checked**" (85 lwIP objects). Independent
re-execution shows the first half is true and the second half is **not**:

- The fresh RTEMS cross build (`-DFF_PLATFORM=rtems -DFF_USE_LWIP=ON`) succeeds
  and produces `frick_lwip` with **85 objects** (84 lwIP-core + `sys_arch.c.obj`)
  — the object count is accurate.
- **No lwIP/tcpip/`sys_*` symbol exists in the linked image.**
  `riscv-rtems6-nm build/rtems-lwip-rev/frick_fsw` yields **0** matches for
  `lwip|tcpip` (0 of 1390 symbols); the same is true of the author's own
  `build/rtems-lwip/frick_fsw`. The archives *do* contain the symbols
  (`libfrick_lwip.a`: `T tcpip_init`, `T udp_input`, `T sys_arch_mbox_fetch`, …)
  and both archives are on the link line
  (`libfrick_net_lwip.a libfrick_lwip.a` in `link.txt`), but nothing in the
  RTEMS application references `FF_NetStackLwip` (only
  `tests/unit/test_lwip.c` does), so **no archive member is extracted**.
- Decisive control: the loadable sections of the `FF_USE_LWIP=ON` image are
  **byte-identical** to the `FF_USE_LWIP=OFF` image (`.text` 408134 B,
  `.data` 6920 B, `.bss` 268017216 B, all sections `cmp`-identical) — enabling
  the flag changes nothing in the delivered image.
- Constructive control (the claim *can* be made true): re-linking with
  `-Wl,--undefined=FF_NetStackLwip -Wl,--undefined=tcpip_init
  -Wl,--undefined=sys_arch_mbox_fetch` **succeeds**, resolves all ten probed
  symbols (`tcpip_init`, `lwip_socket`, `sys_thread_new`, `sys_arch_mbox_fetch`,
  `sys_mbox_trypost_fromisr`, `sys_now`, `sys_jiffies`, `lwip_rtems_rand`, …) and
  grows `.text` from 408134 to 492258 B (+84 KB) — i.e. the port and the stack
  are complete and linkable for the RTEMS target; they are simply not yet pulled
  into the image (that is exactly S6b-2's netif-binding work).

**Assessment.** The *deliverable* claim ("S6b-1: the in-repo port makes
`FF_USE_LWIP=ON` build for the RTEMS target") is **substantiated** — configure,
compile, archive and (when referenced) link all pass, and the port is
API-complete. The *evidence detail* "`tcpip_init`/`lwip_socket` link-checked" is
**false as written** for the delivered artefact, and "the image builds and links
with lwIP" reads as "the stack is in the image", which it is not. No test result
is false (23/23, 22/22, `trace`, `clang-format` all reproduced, see §10) and no
requirement coverage is overstated at manifest level (SRS-REQ-040's evidence
remains the host `test_lwip.c`, with NC-S6-01/NC-S6-02 and GAP-A5 recorded).
Graded **Minor** per the template definition ("must be corrected") and the
project's calibration for over-claiming evidence without a functional false pass
(FRICK-REVIEW-0035 R2-01, re-raised and closed in FRICK-REVIEW-0121 R2-01;
FRICK-REVIEW-0043 R2-02). **It must be corrected before
the record is relied upon**; per the FRICK-REVIEW-0035 R2-01 escalation clause it
becomes **Major** if the increment is presented as target-verified with that
wording unchanged.
**Required correction (pre-commit delta):** either (a) re-word to the verified
facts — "the port compiles and archives for RTEMS; the adapter is not referenced
from the RTEMS application yet, so the stack is not in the image; linkability
verified out-of-band" — and, if kept, attach the link-check evidence
(`-Wl,--undefined=…` link, or a minimal reference) and its command; or (b) add a
real link check to the build/CI and record it. Both readings must propagate to
**all seven** documents listed above (plus `docs/architecture/network-stack.md`,
R4-01) in the same cycle (N8).

### R2-02 — **Minor** — the RTEMS+lwIP configuration is exercised by no gate, CI job or registered tool

`tools/scripts/ci_rtems_cross_build.sh` (`ci:rtems-cross-build`) configures
`-DFF_PLATFORM=rtems` **without** `-DFF_USE_LWIP=ON`; `make rtems-docker` and the
CI `rtems-cross-build` job are the same; the CI lwIP job
(`.github/workflows/ci.yml`, `.gitea/workflows/ci.yml`) is **host-only**
(`ctest -R lwip`). No script, workflow or `docs/vv/tool-qualification.md` entry
reproduces the increment's headline evidence, so the only record of the RTEMS
port building is a one-off manual statement — and any later S6b-2 regression in
the port would go unnoticed. **Required correction:** add `FF_USE_LWIP=ON` to the
RTEMS cross-build script/job (a second configure/build in the same script is
enough) and, if the linkability claim is kept, the `-Wl,--undefined` link check;
register/re-qualify per N10 if the script changes (a changed script invalidates
its qualification).

### R2-03 — **Comment** — the `ASan/UBSan clean` cell in the 0.39 row is not this increment's evidence

`build/asan` (`Flags: -fsanitize=address -fno-omit-frame-pointer`,
`FF_USE_LWIP=OFF`) was last built 12:44 and `build/ubsan` on 2026-10-10, i.e.
**before** the increment's files (13:53); the sanitizer configurations do not
compile the new port at all (it is RTEMS-only, and a cross build cannot be
sanitized). The cell is therefore carried over from the previous cycle and says
nothing about this change. Recommendation: state "host suite unchanged; the
sanitizer configurations exclude the RTEMS-only port" or re-run and record.

### R2-04 — **Comment** — the N18 cell is labelled `REAL` although no Renode run was performed in this cycle (the no-impact basis is nevertheless sound)

`build/renode/console.log` (containing `RENODE_TEST: PASS`) is dated 12:52, i.e.
from the Sprint-6a cycle, an hour before the increment's files (13:53); the 0.39
row nevertheless marks the column `REAL ("make renode" unaffected)`. The
no-impact argument is **verified independently** (and is the reason this is a
Comment, not a finding): a fresh build of the working tree with
`-DFF_PLATFORM=rtems -DFF_ENABLE_RENODE_TEST=ON` produces an ELF that is
**bit-identical** (`sha256 ba94525508e4847658b58428e87e8e23ef50ac1d6b633f121eecb3d3fc5b12de`)
to `build/renode/frick_fsw_renode.elf`, i.e. the artefact validated by the Sprint-6a
`RENODE_TEST: PASS` run is exactly the artefact this increment would ship
(`FF_USE_LWIP` OFF by default; the changed block is not entered). Recommendation:
record that basis in the cell (ELF hash unchanged → Sprint-6a run remains valid
evidence) or re-run `make renode` and paste the marker.

### R2 positive assessment (re-executed first-hand, no finding)

- **Host with `FF_USE_LWIP=ON`: 23/23 PASS** — fresh directory/build from scratch
  (`build/lwip-host-rev`, 159 objects compiled, 0 warnings) plus the existing
  `build/lwip-host`; test `23: lwip` passes.
- **Default build: 22/22 PASS** — `build/native` (up to date) and a fresh
  `build/native-rev` configured with `-DFF_ENABLE_WERROR=ON` (the strongest
  host configuration) both 22/22.
- **RTEMS cross build with `FF_USE_LWIP=ON`: PASS** — fresh
  `build/rtems-lwip-rev` (exit 0), 134 objects; `sys_arch.c` compiled with
  `-I…/src/hal/net/lwip/rtems` (the only `lwipopts.h` on the path, as claimed) and
  `-Wall -Wextra`, 0 compiler warnings.
- **Falsification 1 — host still uses the unix port: CONFIRMED.** The host
  `frick_lwip` contains `contrib/ports/unix/port/{sys_arch.c.o,perf.c.o}`; the
  host adapter's include list is `src`, `src/core`, `src/hal/net/lwip`,
  `third_party/lwip/src/include`, `contrib/ports/unix/port/include` — no RTEMS
  port directory; no object from `src/hal/net/lwip/rtems` exists in the host
  build tree.
- **Falsification 2 — is `sys_arch.c` in the RTEMS link? Answered: compiled and
  archived, but *not* in the linked image** (R2-01) — the object exists
  (`…/frick_lwip.dir/src/hal/net/lwip/rtems/sys_arch.c.obj`) and the archive
  resolves, but the image contains no lwIP code.
- **Falsification 3 — is the Renode image affected? NO** (bit-identical ELF,
  R2-04).
- `trace.py`: **PASS** (114 requirements, 71 srs, 29 sss, 14 gap, 51 Lean
  theorems, 12 CI claims); `metrics.py --check`: "metrics panel: current";
  `clang-format --dry-run --Werror` on the four new files: clean.

## 5. Part C — R3 (systems/requirements, N14) findings

### R3-01 — **Minor** — the development plan was not updated although the increment re-scoped M11/S7

`docs/development-plan.md` §M11/OBJ-19 still reads "Pending: NET(lwIP-on-GEM,
NC-S6-01)/PCIE/SPACEWIRE" and its last history row is 0.23, while this increment
changed the M11/S7 disposition (SpaceWire → contract-level check, **NC-S7-01**)
and the S6b split (**S6b-1 delivered**); `docs/status/loop-audit.md` and
`docs/planning/sprint-plan.md` were updated instead. The declared-vs-actual
tracking is therefore inconsistent across the plan set (N8/N19), which is the
same defect class as **SCOPE-01** of FRICK-REVIEW-0146 (fixed by updating the
`development-plan.md` M4/M11 rows + history). **Required correction:** update the
M11 row (S6b-1 delivered, SpaceWire contract-level/NC-S7-01) and add a history
row in the same cycle.

### R3-02 — **Comment** — `status.json.deferred` is still empty while three NCs are deferred (recurrence of FRICK-REVIEW-0148 PA-02)

`docs/status/status.json` `deferred: []` and the generated README panel reports
"Issues deferred by the user: none", so the S6b-2/S7 deferrals
(NC-S6-01/NC-S6-02/NC-S7-01) are invisible at panel level. The panel is
internally consistent (it mirrors the field), so this is advisory: consider
recording the deferrals (owner/milestone) so the status line does not read as
"nothing deferred".

### R3 factual assessment (verified, no finding)

- **SRS-REQ-040 manifest/mirror accuracy: correct.** `traceability.yaml`
  implementation now lists `src/hal/net/lwip/rtems/{sys_arch.c,arch/cc.h,arch/sys_arch.h,lwipopts.h}`
  alongside the adapter files (all paths exist — `trace.py` PASS);
  `traceability.md:105` mirrors it; the `lean_note` accurately describes the
  in-repo port, the host-build invariant, **NC-S6-01**, **NC-S6-02** and GAP-A5.
- **Requirement text unchanged and still accurate** (`docs/requirements/SRS.md:197-200`:
  adapter over the lwIP socket API built with `FF_USE_LWIP`) — no SRS change was
  warranted; no N14 SSS trigger (SSS untouched).
- **Counts consistent: 114/71/29/14** (manifest gate output) and quoted
  identically in `sprint-plan.md`, `handover.md` (1.14), `verification-results.md`
  (0.39), `status.json` and the README panel (51 theorems / 12 CI claims match).

## 6. Part D — R4 (documentation, N17) findings

### R4-01 — **Minor** — the architecture note referenced by the new code was not updated (N8)

`docs/architecture/network-stack.md` (FRICK-ARCH-0012, rev 0.4) is the design
reference cited by `sys_arch.c:25`, `arch/cc.h:12`, `arch/sys_arch.h:10`,
`lwipopts.h:10` and by `traceability.yaml`/`traceability.md` for SRS-REQ-040, yet
its §4 still describes the lwIP build as "**lwIP core + the unix port
`sys_arch`**" with no RTEMS port, no `FF_PLATFORM` selection and no statement of
the port's runtime prerequisites (the classic-API object budget of NC-S6-02).
The increment's substantive design content currently lives only in source
comments and a manifest note. **Required correction:** update §4 (and §7 gaps) of
`network-stack.md` in the same cycle — per-platform port selection, the RTEMS
object prerequisites, the "compiled/archived but not linked until S6b-2"
statement (R2-01) and its history row.

### R4-02 — **Minor** — NC-S7-01's origin is attributed to a report that does not raise it, and the NCs live only in `sprint-plan.md` §3

`docs/planning/sprint-plan.md` §3 records `NC-S7-01 | FRICK-REVIEW-0148 (scope
conformance)`. FRICK-REVIEW-0148 does **not** raise the SpaceWire emulator
limitation: it mentions SpaceWire only in the "next sprint ordering" context
(R4-04) and its §7 confirms the S6 split/NC-S6-01 recording. NC-S7-01 in fact
originates from the Sprint-6b re-plan/N19 audit (root cause: the Renode
PolarFire SoC model has no SpaceWire peripheral, so OBJ-19's "every class
peripheral-bound" cannot be met). Equally, NC-S6-02 and NC-S7-01 appear only in
`sprint-plan.md` §3 and this report; FRICK-REVIEW-0148's follow-up table still
shows NC-S6-01 as "open (scheduled S6b)" with no narrowing and no reference to
the S6b-1/S6b-2 split. **Required correction:** correct the origin
(the Sprint-6b re-plan / N19 audit / this review) and have the 0148 delta
re-review record the narrowing of NC-S6-01 and the new NCs (N11/N12).

### R4-03 — **Comment** — `loop-audit.md` §2 (rules audit) was not re-performed for this loop

`docs/status/loop-audit.md` was touched for this loop (header, §1 M11 row,
history 0.18), but §2's rules table is unchanged: its N20 row still cites
"FRICK-REVIEW-0052/0053 findings closed, PA PASS (FRICK-REVIEW-0063)" (Sprint-0
era) and its N18 row predates the S6b-1 increment. N19 requires the audit of the
objectives **and** of N1–N20 against the actual implementation at every loop
start. Pre-existing pattern, advisory: refresh §2 or state explicitly "no change
this loop".

### R4-04 — **Comment** — touched-document discipline is otherwise correct; the review-log row is still missing

- **Header vs `## History` (N16):** 6/6 touched documents match —
  `sprint-plan.md` 0.11 = 0.11, `traceability.md` 0.34 = 0.34,
  `handover.md` 1.14 = 1.14, `loop-audit.md` 0.18 = 0.18,
  `renode-environment.md` 0.18 = 0.18, `verification-results.md` 0.39 = 0.39.
- **README metrics panel:** current (`metrics.py --check` exit 0), updated
  "Current focus" text with the S6b-1 wording, NC-S6-01 narrowing and the
  NC-S6-02/NC-S7-01 statements.
- **English only (N2):** all new/changed content is English.
- **Missing, and not writable by this reviewer:** the `docs/reviews/README.md`
  log row for FRICK-REVIEW-0150 (and the semaphore refresh) must be added with
  this report in the same commit (N11/N20); the reviewer was instructed to write
  exactly one file (this report) and modified nothing else.

## 7. Part E — PA (product assurance, N20) and scope conformance

### 7.1 Scope conformance (standing rule)

**Verdict: conformant — no scope-deviation NC raised.** The declared scope
("the pinned lwIP has no RTEMS port, so `FF_USE_LWIP=ON` could not build for the
RTEMS target; add an in-repo RTEMS `sys_arch` port and select it for
`FF_PLATFORM=rtems`; host keeps the lwIP unix port; `FF_USE_LWIP` stays OFF by
default") **is** what is implemented:

- the four new files implement the port and are the *only* new code (no scope
  creep into netif/interrupt work that belongs to S6b-2);
- `CMakeLists.txt` selects the port exactly for `FF_PLATFORM=rtems` and leaves the
  host branch functionally unchanged (verified: host 23/23, default 22/22;
  the RTEMS-mode artefact is bit-identical to the `FF_USE_LWIP=OFF` artefact);
- `FF_USE_LWIP` default remains `OFF` (`CMakeLists.txt:23`), so no delivered
  configuration changes behaviour;
- the limitation of the increment (no runtime/target execution) is declared in
  the traceability note and the NCs, not hidden.

The **only** conformance defect found is evidential, not scope-related: the
record over-states the link evidence (R2-01).

### 7.2 Honesty and scoping of the NC disposition

- **NC-S6-01 (narrowed, open, scheduled S6b)** — honest. Root cause (pinned lwIP
  ships only unix/win32/freertos ports; a Renode network path is additionally
  needed) is correctly stated and independently confirmed (`contrib/ports/`
  contains no `rtems` directory). The narrowing does not claim the
  peripheral-bound check is done, and the adapter stays host-tested. *Caveat:*
  origin/narrowing not reflected in FRICK-REVIEW-0148's follow-up table (R4-02).
- **NC-S6-02 (open, scheduled S6b-2)** — honest and **verified**:
  `config/rtems/rtems_config.c` has `CONFIGURE_MAXIMUM_TASKS 16`,
  `CONFIGURE_MAXIMUM_TIMERS 8`, `CONFIGURE_MAXIMUM_SEMAPHORES 16` and **no**
  `CONFIGURE_MAXIMUM_MESSAGE_QUEUES` (default 0); the RTEMS Message Manager
  returns `RTEMS_TOO_MANY` when no inactive message queue is available, so
  `sys_mbox_new()` returns `ERR_MEM`, `tcpip_init()` asserts
  ("failed to create tcpip_thread mbox") and `LWIP_PLATFORM_ASSERT` is fatal —
  i.e. "a runtime `tcpip_init` would fail" is exactly right. The NC is correctly
  scoped to S6b-2 and already mentions the semaphore/task headroom, which is
  indeed tight (one mutex + one semaphore per netconn for up to
  `MEMP_NUM_NETCONN 8`, plus the tcpip task, against 16 semaphores/16 tasks).
- **NC-S7-01 (open, scheduled S7)** — the SpaceWire emulator limitation is a
  legitimate objective-deviation NC (`OBJ-19` "every class peripheral-bound"
  cannot be met) and is correctly resolved by a contract-level check in S7 with
  GAP-A5 recorded; only the **origin attribution** is wrong (R4-02).
- All three NCs carry an **owner** (`frick-fsw authoring agent`) and a
  **scheduled sprint** in `sprint-plan.md` §3, as the template requires.

### 7.3 PA findings

- **PA-01 (Minor)** — = **R2-01** (evidence integrity). PA position: the
  increment may proceed to the pre-commit delta, but the "link-checked" claim is
  a **mandatory** correction (not advisory) in all seven documents before the
  record is relied upon or the increment is presented as target-verified; if it
  is not corrected, it escalates to Major and blocks commit/push per N12.
- **PA-02 (Comment)** — the N11/N20 cycle records are incomplete in the working
  tree: this report is written but `docs/reviews/README.md` (log row) and the
  status/semaphore panel are not (the reviewer is read-only except for this
  file). The author must add them in the same commit (N11/N20).
- **PA-03 (Comment)** — `status.json.deferred` empty (see R3-02).
- **PA-04 (Comment)** — process: the documented cross-build entry points
  (`make rtems-docker`, `ci:rtems-cross-build`) do not exercise
  `FF_USE_LWIP=ON`, so the increment's build path is reachable only by a
  hand-typed command (see R2-02). For a criticality-B product, opt-in build
  configurations that are not gated tend to regress silently; wire it into a
  gate in S6b-2 at the latest.
- **PA-05 (Comment)** — licensing: the port is project-authored AGPL-3.0-only
  and is now *linkable* into the delivered target image together with BSD-3
  lwIP; `docs/quality/licensing.md` already covers the aggregate compatibility
  (line 45) — add one line recording the new link-level aggregation + the
  attribution obligation when a mission enables it (N13/N8).

### 7.4 What a qualified human must still accept (PA statement)

The reviewer cannot and does not accept these; they remain with the responsible
human authority:

1. **On-target stack behaviour = GAP-A5 / NC-S6-01.** No lwIP code runs on the
   target or the emulated SoC in this increment; there is **no on-target lwIP
   run** (only compile/archive, plus the reviewer's out-of-band forced link).
   Netif/PHY binding, ISR-driven input, TCP/UDP correctness, throughput,
   latency, stack high-water and interrupt-timing behaviour are all unverified.
2. **NC-S6-02 (runtime object sizing).** As delivered, a runtime `tcpip_init()`
   on the target would fail (no message queues configured). The stack is
   therefore not merely untested on target: it cannot start there yet.
3. **The corrected evidence wording** (R2-01) and the independent
   reviewer-run link check as acceptable objective evidence (it was not produced
   by a project gate; if the author adopts it, it should be re-produced by the
   build/CI per R2-02).
4. **The RTEMS port's fitness for the mission's real network** (ISN entropy
   R1-06, mailbox wait discipline and full-queue policy R1-02, fatal-path policy
   R1-04) and the acceptance of the residual risks recorded here.
5. **That no waiver is implied** for the class-A gaps (GAP-A4/A5) or for the
   NC-S6-01/NC-S6-02/NC-S7-01 schedule.

## 8. Findings summary

| # | Severity | Profile | Item | Finding | Required action |
| - | -------- | ------- | ---- | ------- | --------------- |
| **R2-01 / PA-01** | **Minor** (mandatory correction; Major if left uncorrected) | R2 + PA | `verification-results.md` 0.39, `CHANGELOG.md`, `status.json`, `handover.md`, `loop-audit.md`, `sprint-plan.md`, `README.md` | RTEMS image "builds and links with lwIP … `tcpip_init`/`lwip_socket` link-checked" — the delivered image contains **no** lwIP code (loadable sections byte-identical to the `FF_USE_LWIP=OFF` image; 0 lwIP symbols in the ELF) | Re-word to the verified facts **or** add a real link check; propagate to all documents; attach the command/evidence |
| R1-01 | Minor | R1 | `sys_arch.c:467-492` | `sys_thread_new()` silently returns an invalid handle on pool/task-create failure; `tcpip_init()` ignores it → dead stack, no diagnostic | Fail loudly (assert/fatal) or document the contract; delta |
| R2-02 | Minor | R2 | `tools/scripts/ci_rtems_cross_build.sh`, CI workflows | No gate/CI/tool exercises `FF_PLATFORM=rtems + FF_USE_LWIP=ON` | Add the configuration (and the link check) to the RTEMS cross build; re-qualify per N10 |
| R3-01 | Minor | R3 + R4 | `docs/development-plan.md` M11/OBJ-19 row + history | Not updated for S6b-1/SpaceWire contract-level/NC-S7-01 (N8/N19) | Update the M11 row + add a history row |
| R4-01 | Minor | R4 | `docs/architecture/network-stack.md` §4/§7 | Still says the lwIP build is "core + unix port"; no RTEMS port/selection/prerequisites, although the new code cites it | Update the design note in the same cycle |
| R4-02 | Minor | R4 | `sprint-plan.md` §3; `docs/reviews/2026-10-12-m4-m11-tsn-s6a-review.md` §6 | NC-S7-01 attributed to FRICK-REVIEW-0148 (which does not raise it); new/narrowed NCs not in any review report's follow-up table | Correct the origin; record the NC-S6-01 narrowing + NC-S6-02/NC-S7-01 in the 0148 delta table |
| R1-02 | Comment | R1 | `sys_arch.c:315-316,340-350,43` | Mailbox wait discipline FIFO (not priority) and 1 ms busy-wait full-queue policy undocumented; every RTEMS object named `'LWIP'` | Document/rationalise; consider `RTEMS_PRIORITY` + distinct names |
| R1-03 | Comment | R1 | `sys_arch.c:255,400` | Constant `1` returned on success (contract-compliant in 2.2.1) | Record as deliberate |
| R1-04 | Comment | R1 | `arch/cc.h:43-47` | `printf` + `rtems_fatal_error_occurred(0xDEADBEEF)`; asserts may fire in ISR context; project uses `rtems_fatal(RTEMS_FATAL_SOURCE_*)` | Align the fatal/assert policy |
| R1-05 | Comment | R1 | `CMakeLists.txt:184-207` | lwIP includes propagate `PUBLIC` to every core TU | Scope to the lwIP targets |
| R1-06 | Comment | R1 + PA | `sys_arch.c:170-186`, `arch/cc.h:56-57` | Non-cryptographic TCP ISN source (RFC 6528 consideration) | Record; mission CSPRNG where TCP is exposed |
| R2-03 | Comment | R2 | `verification-results.md` 0.39 | "ASan/UBSan clean" is a carried-over claim; sanitizer builds exclude the new port | Re-run/record or state the exclusion |
| R2-04 | Comment | R2 | `verification-results.md` 0.39 (Renode cell) | Labelled `REAL` although no Renode run in this cycle (no-impact basis verified: bit-identical ELF) | Record the hash basis or re-run |
| R3-02 / PA-03 | Comment | R3 + PA | `docs/status/status.json` | `deferred` empty while three NCs are deferred | Consider recording the deferrals |
| R4-03 | Comment | R4 | `docs/status/loop-audit.md` §2 | Rules audit not re-performed this loop (stale N20/N18 rows) | Refresh or state "no change" |
| PA-02 | Comment | PA | `docs/reviews/README.md`, status panel | Review-log row + semaphore refresh missing (reviewer is read-only) | Add with this report in the same commit |
| PA-04 | Comment | PA | `Makefile`/CI (`rtems-docker`, `ci:rtems-cross-build`) | Opt-in build configuration not gated | Wire it into a gate by S6b-2 |
| PA-05 | Comment | PA | `docs/quality/licensing.md:45` | New lwIP-into-image aggregation not recorded | Add one line (N13/N8) |

**Severity definition** (from `docs/reviews/review-template.md`): **Major** — a
non-conformity that disqualifies the item for the applicable criticality, or a
defect that can defeat a requirement or a safety barrier. **Minor** — a
non-conformity that does not invalidate the requirement but must be corrected or
justified. **Comment** — observation/recommendation, no derogation.

No **Major** finding was raised: the increment's deliverable claim is
substantiated, no functional check is a false pass, and no requirement coverage
is overstated. The one boundary finding (R2-01) is graded Minor by the template
definition and the project's calibration for evidence over-claim without a
functional false pass, and is flagged with an explicit escalation clause.

## 9. Conclusion

- [ ] APPROVED — no findings or Comments only
- [x] **APPROVED WITH COMMENTS** — Minor findings tracked to closure
- [ ] REJECTED — one or more Major findings

**Rationale.** The Sprint-6b-1 increment does what it declares: lwIP 2.2.1 builds
for the RTEMS target through an in-repo, API-complete, allocation-free,
`-Wall -Wextra`-clean `sys_arch` port, selected by CMake only for
`FF_PLATFORM=rtems`; the host build provably still uses the upstream unix port
(23/23), the default build is unaffected (22/22, also with `-DFF_ENABLE_WERROR=ON`),
the RTEMS cross build compiles and archives the port (85 lwIP objects, 0
warnings), `trace` is PASS with consistent counts (114/71/29/14), the touched
documents are history-consistent, and the new NCs are honestly recorded with
owners and scheduled sprints. The review found **no Major**: the port is
substantively correct and the increment's stated limitations (no target
execution, no runtime object sizing) are declared rather than hidden.

**The conditions of this verdict (must be met in the pre-commit delta, N12):**

1. **R2-01/PA-01 — correct or make true the "link-checked" evidence claim** in
   all seven documents (and align `network-stack.md`): the delivered RTEMS image
   contains no lwIP code. The reviewer's independent forced-extraction link
   (`-Wl,--undefined=…`, all symbols resolved, `.text` +84 KB) is available as
   ready-made evidence, but it should be adopted as a build/CI step (R2-02).
2. R1-01 (loud failure in `sys_thread_new`), R3-01 (`development-plan.md`),
   R4-01 (`network-stack.md`), R4-02 (NC origins/follow-up tables) —
   corrections in the same cycle.
3. Add the `docs/reviews/README.md` row for this report and refresh the status
   panel/semaphores (PA-02, N11/N20).
4. Re-review the deltas (same profiles, fresh context) before commit/push
   (N12/N7).

**The loop is not closed at this revision:** no NC raised by this report is
closed yet (all are Minors/Comments to be disposed in the delta), the
pre-existing **NC-S6-01 (narrowed)**, **NC-S6-02** and **NC-S7-01** remain
open/scheduled (S6b-2 / S7), and GAP-A5 stays open for human acceptance (§7.4).

## 10. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9) | Senior FSW / real-time / space software developer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — port correct, complete and allocation-free; R1-01 Minor + Comments R1-02…R1-06 |
| R2 (N9) | Senior SW V&V engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — all gates re-executed (23/23, 22/22, RTEMS PASS, trace PASS); R2-01 Minor (evidence accuracy) + R2-02 Minor + R2-03/R2-04 Comments |
| R3 (N14) | Senior avionics / systems engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — manifest/mirror/counts accurate; R3-01 Minor (development plan) + R3-02 Comment |
| R4 (N17) | Senior SW documentation engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — headers vs history 6/6, panel current; R4-01/R4-02 Minors + R4-03 Comment |
| PA (N20) | Senior SW Product Assurance engineer (independent context) | 2026-10-12 | **APPROVED WITH COMMENTS** — scope conformance verified, NC disposition honest and correctly scoped; PA-01 = R2-01 (mandatory correction), PA-02…PA-05 Comments; on-target behaviour remains with the human authority (GAP-A5, NC-S6-02) |

## 11. Follow-up records

Non-conformances and their dispositions (norm N12: NCs are resolved by
re-applying **all** rules to the affected scope and re-reviewing the deltas until
full compliance). Every row below is **open** at this revision; the pre-commit
delta review is the closure vehicle.

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| **NC-150-01** (R2-01 / PA-01) | Minor | Evidence record: `verification-results.md` 0.39, `CHANGELOG.md`, `status.json`, `handover.md`, `loop-audit.md`, `sprint-plan.md`, `README.md`, `network-stack.md` | "`tcpip_init`/`lwip_socket` link-checked" / "image builds and links with lwIP" is false for the delivered image (0 lwIP symbols; loadable sections byte-identical to `FF_USE_LWIP=OFF`) | Re-word to the verified facts **or** add a real link check; propagate to all seven documents + the design note | (delta) corrected texts; if kept, the `-Wl,--undefined` link output; this report §4 | frick-fsw authoring agent | **open** | **S6b-1 delta (pre-commit)** |
| NC-150-02 (R1-01) | Minor | `src/hal/net/lwip/rtems/sys_arch.c:467-492` | `sys_thread_new()` silent failure → `tcpip_init()` "succeeds" with a dead stack | Assert/fatal on pool/task-create failure, or document the contract | (delta) port diff + RTEMS rebuild | frick-fsw authoring agent | **open** | **S6b-1 delta** |
| NC-150-03 (R2-02) | Minor | `tools/scripts/ci_rtems_cross_build.sh`, `.github/workflows/ci.yml`, `.gitea/workflows/ci.yml` | No gate exercises `FF_PLATFORM=rtems + FF_USE_LWIP=ON` | Add the configuration (+ link check) to the RTEMS cross build; re-qualify per N10 | (delta) script diff + run log | frick-fsw authoring agent | **open** | **S6b-1 delta** |
| NC-150-04 (R3-01) | Minor | `docs/development-plan.md` (M11/OBJ-19 + history) | Plan row/history not updated for S6b-1/SpaceWire/NC-S7-01 | Update row + history | (delta) doc diff | frick-fsw authoring agent | **open** | **S6b-1 delta** |
| NC-150-05 (R4-01) | Minor | `docs/architecture/network-stack.md` §4/§7 | Design note stale (unix-port-only build description) although cited by the new code | Update the design note (+ history) | (delta) doc diff | frick-fsw authoring agent | **open** | **S6b-1 delta** |
| NC-150-06 (R4-02) | Minor | `sprint-plan.md` §3; FRICK-REVIEW-0148 follow-up table | NC-S7-01 origin mis-attributed; NC narrowing/new NCs not in a review report's table | Correct origin; record in the 0148 delta table | (delta) plan/table diff | frick-fsw authoring agent | **open** | **S6b-1 delta** |
| NC-150-07 (R1-02…R1-06, R2-03, R2-04, R3-02, R4-03, PA-02…PA-05) | Comment | see §8 | Observations/advisories (no derogation) | Consider/record; no blocking | (delta) as applicable | frick-fsw authoring agent | **open (advisory)** | S6b-1 delta / S6b-2 |
| **NC-S6-01** | Minor (scope) | SRS-REQ-040 / Sprint S6b | NET lwIP-on-GEM peripheral-bound deferred (root cause: pinned lwIP has no RTEMS port; S6b-1 delivers the port) | Narrowed honestly; adapter stays host-tested; **no claim** of a completed peripheral-bound check | `sprint-plan.md` §2/S6b row + §3; `traceability.yaml` note; this review §7.2 | frick-fsw authoring agent | **open (scheduled S6b/S6b-2)** | S6b-2 |
| **NC-S6-02** | Minor | `config/rtems/rtems_config.c`; runtime `tcpip_init` | No `CONFIGURE_MAXIMUM_MESSAGE_QUEUES` (default 0) → runtime `tcpip_init` fails (verified against the RTEMS Message Manager); sizing + semaphore/task headroom to wire in S6b-2 | Correctly scoped; owner + sprint recorded | this review §7.2; `sprint-plan.md` §3 | frick-fsw authoring agent | **open (scheduled S6b-2)** | S6b-2 |
| **NC-S7-01** | Minor (scope) | OBJ-19 / SRS-REQ-032 (SpaceWire) | No SpaceWire peripheral in the Renode PolarFire SoC model → "every class peripheral-bound" cannot be met; resolve with a contract-level check in S7 (GAP-A5) | Correctly scoped (origin attribution to correct, R4-02); owner + sprint recorded | `sprint-plan.md` §2/S7 + §3; `renode-environment.md` 0.18 | frick-fsw authoring agent | **open (scheduled S7)** | S7 |

## 12. Evidence ledger (all re-executed/obtained first-hand on 2026-10-12)

| # | Command / artefact | Observed result |
| - | ------------------ | --------------- |
| E1 | `git rev-parse HEAD`, `git status --porcelain` | HEAD `3822f721c2f4588dab1ded10e0e21103bd50aa8d`; 11 modified files + untracked `src/hal/net/lwip/rtems/` (4 files, exactly the declared set) |
| E2 | `cmake -S . -B build/lwip-host-rev -DFF_USE_LWIP=ON -DCMAKE_BUILD_TYPE=Debug && cmake --build … && ctest …` | configure+build exit 0 (159 objects compiled, **0 warnings**); **23/23 PASS** incl. test 23 `lwip` |
| E3 | `ctest --test-dir build/lwip-host` (author's tree) | **23/23 PASS** |
| E4 | `cmake -S . -B build/native-rev -DCMAKE_BUILD_TYPE=Debug -DFF_ENABLE_WERROR=ON && … && ctest` | **22/22 PASS** (no regression, strongest host flags) |
| E5 | `cmake --build build/native && ctest --test-dir build/native` | build exit 0; **22/22 PASS** |
| E6 | host port identity: `find build/lwip-host*/CMakeFiles/frick_lwip.dir -name "*.o"`; `cat …/frick_net_lwip.dir/flags.make` | unix port objects present (`contrib/ports/unix/port/sys_arch.c.o`, `perf.c.o`); adapter includes `src/hal/net/lwip` + unix port include; **no** RTEMS-port object anywhere in the host tree |
| E7 | `docker run … frick-rtems-toolchain:local` `cmake -S . -B build/rtems-lwip-rev -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-mpfs.cmake -DFF_PLATFORM=rtems -DFF_USE_LWIP=ON && cmake --build …` | configure+build **exit 0**; 134 objects; `frick_lwip` = **85 objects** (84 lwIP core + `…/rtems/sys_arch.c.obj`); `sys_arch.c` compiled with `-I…/src/hal/net/lwip/rtems -I…/third_party/lwip/src/include -Wall -Wextra`; **0 compiler warnings**; linked `frick_fsw` (3072288 B) |
| E8 | duplicated run into the author's `build/rtems-lwip` | up to date, no-op build (same tree state); its ELF also 3072288 B |
| E9 | `riscv-rtems6-nm build/rtems-lwip-rev/frick_fsw` (and the author's) | 1390 symbols, **0** matching `lwip|tcpip|sys_arch|sys_mbox|sys_sem` |
| E10 | `riscv-rtems6-nm build/rtems-lwip-rev/libfrick_lwip.a`; `cat …/frick_fsw.dir/link.txt` | archive defines `tcpip_init`, `udp_input`, `ip4_input`, `sys_arch_mbox_fetch`, `sys_thread_new`, …; link line includes `libfrick_net_lwip.a libfrick_lwip.a` |
| E11 | `objcopy --only-section` + `cmp` vs the `FF_USE_LWIP=OFF` image (`build/rtems/frick_fsw`) | `.text` 408134 B, `.data` 6920 B, `.bss` 268017216 B — **byte-identical** in all loadable sections |
| E12 | forced-extraction link (`-Wl,--undefined=FF_NetStackLwip -Wl,--undefined=tcpip_init -Wl,--undefined=sys_arch_mbox_fetch`) | **LINK OK**; 10/10 probed symbols present (`tcpip_init`, `lwip_socket`, `sys_thread_new`, `sys_arch_mbox_fetch`, `sys_mbox_trypost_fromisr`, `sys_now`, `sys_jiffies`, `lwip_rtems_rand`); `.text` 408134 → 492258 B (+84 KB) |
| E13 | fresh `-DFF_PLATFORM=rtems -DFF_ENABLE_RENODE_TEST=ON` build (`build/renode-rev`) + `sha256sum` | ELF **bit-identical** to `build/renode/frick_fsw_renode.elf`: `ba94525508e4847658b58428e87e8e23ef50ac1d6b633f121eecb3d3fc5b12de` (the artefact of the Sprint-6a `RENODE_TEST: PASS`, `build/renode/console.log`, 12:52) |
| E14 | `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` | **PASS** — 114 requirements (71 srs / 29 sss / 14 gap), 51 Lean theorems, 12 CI-evidence claims |
| E15 | `python3 tools/scripts/metrics.py --check` | "metrics panel: current" (exit 0) |
| E16 | `clang-format --dry-run --Werror src/hal/net/lwip/rtems/**` | clean (exit 0) on all four new files |
| E17 | header vs History scan of the six touched documents | 0.11/0.11, 0.34/0.34, 1.14/1.14, 0.18/0.18, 0.18/0.18, 0.39/0.39 — all match |
| E18 | RTEMS interface verification at the issuer (`docs.rtems.org`, main branch, updated 2026-06-17, accessed 2026-10-12) | message queue `send` "may be called from within interrupt context" and returns `RTEMS_TOO_MANY` when full; `create` returns `RTEMS_TOO_MANY` when `CONFIGURE_MAXIMUM_MESSAGE_QUEUES` is exhausted; counting semaphore + `RTEMS_PRIORITY` legal; binary + `RTEMS_PRIORITY` + `RTEMS_INHERIT_PRIORITY` legal (local + priority discipline); receive buffer must fit the queue's maximum message size; `RTEMS_TIMEOUT` on timeout |
| E19 | lwIP 2.2.1 source read (pinned tree) | `sys.h` 2.2.1 contract ("any other value on success"); `opt.h` `SYS_LIGHTWEIGHT_PROT` defaults to **1** (so the port's protect/unprotect *is* used); `tcpip_init()` ignores `sys_thread_new()`'s return and asserts on mbox/mutex failure; `tcpip_mbox_fetch()` uses the fetch return only against `SYS_ARCH_TIMEOUT` |
| E20 | `config/rtems/rtems_config.c` read | `CONFIGURE_MAXIMUM_TASKS 16`, `TIMERS 8`, `SEMAPHORES 16`, no `CONFIGURE_MAXIMUM_MESSAGE_QUEUES`; `CONFIGURE_MICROSECONDS_PER_TICK 1000` |
| E21 | timestamps of the pre-existing evidence trees | `build/asan` 12:44 (`-fsanitize=address`, `FF_USE_LWIP=OFF`), `build/ubsan` 2026-10-10, `build/renode/console.log` 12:52 — all **before** the increment's files (13:53–13:55) |

## 13. Norm-by-norm audit

| N | Assessment for this increment |
| - | ----------------------------- |
| N1 | **OK with justification** — the port is non-executable-API glue; SRS-REQ-040 keeps `lean: []` + an explicit `lean_note` (no silent waiver); no Lean source touched |
| N2 | **OK** — all new content is English |
| N3 | **OK** — SRS/SSS unchanged; SRS-REQ-040 text remains accurate for the increment |
| N4 | **OK** — `trace.py` PASS (114/71/29/14); manifest lists the new implementation paths (paths exist); mirror row and note updated |
| N5 | **OK** — GAP-A5 kept open; no level-A claim is made; class-A gaps untouched |
| N6 | **Partly gated** — CI covers the host lwIP build/test and the RTEMS cross build, but **not** the RTEMS+lwIP combination (R2-02) |
| N7 | **Not applicable to the reviewer** — nothing was committed or pushed by this review; the author must push both remotes with the corrected delta |
| N8 | **Gap** — one affected design note not updated (R4-01) and one plan document stale (R3-01) |
| N9 | **OK** — this review ran in a fresh, independent context over the exact revision; the author's context was not inherited |
| N10 | **OK, with a required action** — no V&V script changed in this increment (`tool-qualification.md` untouched, correctly); if `ci_rtems_cross_build.sh` changes per R2-02 it must be re-qualified |
| N11 | **Gap (actionable)** — this report is in `docs/reviews/` but the `docs/reviews/README.md` row and the status/panel refresh are still to be added with the commit (PA-02) |
| N12 | **Open** — NC-150-01…07 + NC-S6-01/02/S7-01 remain open; the delta re-review closes the ones raised here |
| N13 | **OK** — AGPL-3.0-only SPDX on all four new files; aggregate with BSD-3 lwIP already analysed (one-line addition recommended, PA-05) |
| N14 | **Not triggered / satisfied** — SSS not modified; the SRS/manifest side reviewed here (R3) |
| N15 | **Partly** — host + RTEMS cross builds and the default suite were re-run; the Renode gate was not re-run but the artefact is bit-identical (R2-04); Lean not applicable |
| N16 | **OK** — header = last History row in 6/6 touched documents; CHANGELOG entry present |
| N17 | **Required** — the documentation deltas of NC-150-01/04/05/06 must be re-reviewed by R4 in a fresh context before commit/push |
| N18 | **Satisfied in substance / record to qualify** — the emulated run for this artefact is the Sprint-6a `RENODE_TEST: PASS` on a **bit-identical** ELF; the cell should record that basis or the run repeated (R2-04) |
| N19 | **Gap (minor)** — the loop audit updated §1 but not §2 (R4-03); the objectives audit did surface the root causes now recorded as NC-S6-01/S6b-2 and NC-S7-01 (good), with NC-S7-01's origin mis-cited (R4-02) |
| N20 | **Discharged** — PA reviewed the increment's phase products (code, build config, requirements mirror, plans, status, V&V records) in independent context; findings above; the review loop remains open until the delta passes |

## Facts, Assumptions, Inferences, Recommendations, Confidence, Open gaps

**Facts (verified first-hand at `3822f72` + working tree, 2026-10-12).**

1. The increment is exactly the declared set: four new files under
   `src/hal/net/lwip/rtems/` + the `if(FF_USE_LWIP)` block of `CMakeLists.txt` +
   11 documentation/status files.
2. Host build with `FF_USE_LWIP=ON`: fresh configure/build/`ctest` → **23/23
   PASS**, 0 warnings; the host build still compiles/links the **lwIP unix port**
   and never the RTEMS port.
3. Default build: **22/22 PASS**, including a fresh build with
   `-DFF_ENABLE_WERROR=ON`.
4. RTEMS cross build with `FF_PLATFORM=rtems -DFF_USE_LWIP=ON`: **exit 0**, 85
   lwIP objects (incl. `sys_arch.c.obj`), `sys_arch.c` compiled with the RTEMS
   port include directory only and `-Wall -Wextra`, 0 compiler warnings.
5. **The delivered RTEMS image contains no lwIP code**: 0 lwIP/tcpip/`sys_*`
   symbols in the ELF; its loadable sections are byte-identical to the
   `FF_USE_LWIP=OFF` image; with forced extraction the same objects link
   successfully and add 84 KB of text. Therefore the record's
   "`tcpip_init`/`lwip_socket` **link-checked**" claim is not what the delivered
   build does (R2-01).
6. The port is API-complete for lwIP 2.2.1 `NO_SYS=0`, allocation-free, bounded,
   uses legal RTEMS attribute sets, provides an ISR-callable try-post, and has a
   monotonic `sys_now()`; the constant success value of the wait functions is
   contract-compliant in 2.2.1.
7. `trace` PASS (114/71/29/14, 51 theorems, 12 CI claims); `metrics --check`
   current; `clang-format` clean; header = last History row in 6/6 touched docs.
8. The Renode-mode ELF built from the working tree is **bit-identical** to the
   artefact that produced the Sprint-6a `RENODE_TEST: PASS`.
9. `NC-S6-02` is technically exact: no `CONFIGURE_MAXIMUM_MESSAGE_QUEUES`
   (default 0) ⇒ runtime `tcpip_init()` fails per the RTEMS Message Manager.
10. No CI job/script/tool exercises `FF_PLATFORM=rtems + FF_USE_LWIP=ON`.
11. No other file was modified, created or deleted by this reviewer; nothing was
    committed or pushed (the only write is this report).

**Assumptions.**

1. The pinned `third_party/lwip` tree (STABLE-2_2_1_RELEASE) is the stack that
   would be used for a mission; its `sys.h`/`tcpip.c`/`opt.h` are the contract the
   port must satisfy (read directly).
2. The `frick-rtems-toolchain:local` image (`90f81d080611`) is the project's
   pinned RTEMS 6.2/mpfs64imafdc toolchain (as declared in
   `tools/docker/rtems/Dockerfile`); results are valid for that toolchain.
3. The Renode `polarfire-soc` model behaviour for the (not-yet-bound) GEM netif is
   out of scope; nothing in this review depends on it.
4. The author's tool-use statements (e.g. that `make renode` was "unaffected") are
   treated as claims, not evidence; every one relied upon here was re-verified.

**Inferences.**

1. The intended meaning of "link-checked" was most plausibly "the lwIP symbols
   are resolvable in the built archives / the link with the lwIP inputs
   succeeded", not "the symbols are present in the image" — an ambiguous/incorrect
   formulation rather than a fabricated result; the fix is editorial plus (if
   kept) one build step (R2-01).
2. The absence of any gate for the RTEMS+lwIP configuration (R2-02) is the
   likeliest reason the "link-checked" statement was not noticed to be wrong.
3. The debug-metadata difference between the ON/OFF images (24 extra DWARF
   compile units) is a consequence of the leaked lwIP include paths in every core
   TU (R1-05), not of linked lwIP code — the loadable sections are identical.

**Recommendations.**

1. Correct or substantiate the link-evidence wording in all affected documents
   (NC-150-01) and adopt the forced-extraction link as a build/CI step
   (NC-150-03); the reviewer's link output is ready to be re-produced.
2. Add `FF_USE_LWIP=ON` to the RTEMS cross-build script/job so the increment's
   headline evidence is gated and reproducible (N6/N10/N15).
3. Fix `sys_thread_new()` to fail loudly (NC-150-02) and document the mailbox
   wait discipline/full-queue policy (R1-02).
4. Update `docs/architecture/network-stack.md` §4/§7 (NC-150-05) and
   `docs/development-plan.md` M11/OBJ-19 (NC-150-04); correct the NC-S7-01 origin
   and record the NC narrowing (NC-150-06).
5. Keep GAP-A5 and NC-S6-02 open; in S6b-2 size `CONFIGURE_MAXIMUM_MESSAGE_QUEUES`
   (and semaphore/task headroom) with the actual netconn/socket budget before the
   first on-target `tcpip_init`, and record a stack high-water measurement.
6. Record the review-log row + status panel with this report (PA-02) and re-review
   the deltas in fresh contexts (N12/N17/N20).

**Confidence.**

- **High** for every factual statement in §12 (each was re-executed or read at the
  issuer on 2026-10-12): the host/default/RTEMS builds, the symbol/section
  evidence about the RTEMS image, the trace/metrics/format gates, the manifest
  and history consistency, the RTEMS API legality, the NC-S6-02 mechanism.
- **Medium** for the *significance* grading of R1-02…R1-06, R2-03/R2-04, R3-02,
  R4-03, PA-02…PA-05 (advisory/process, no functional impact at this revision).
- **Medium** for NC-S7-01's resolution route (S7 contract-level check) — an
  accepted plan, not evidence; **Low** confidence is *not* claimed for anything
  stated as Fact above. Confidence is not a probability of safety.

**Open gaps** (not closed by this review).

- **NC-150-01…07** — open; Minors to be corrected in the S6b-1 pre-commit delta,
  Comments advisory (§11).
- **The loop is not closed**: no NC raised here is closed; the delta re-review
  (R1/R2/R4/PA at minimum) is required before commit/push (N12/N17/N20).
- **NC-S6-01 (narrowed)** open, scheduled S6b/S6b-2; **NC-S6-02** open, scheduled
  S6b-2; **NC-S7-01** open, scheduled S7.
- **GAP-A5** (on-target behaviour) — no on-target lwIP run, no netif/PHY, no
  timing/stack-high-water evidence; the stack cannot even start today
  (NC-S6-02). **For human acceptance.**
- **No independent oracle** for the RTEMS+lwIP runtime behaviour exists in this
  project (the reviewer's forced link is a build-level check only).
- **Not verifiable by the reviewer:** the author's intent behind the ambiguous
  evidence wording (Inference 1) and any evidence the author may hold outside the
  repository (none is recorded); paywalled/absent sources: none relied upon.

---

*This report is authored to be committed and pushed with the reviewed change
(norm N11). The review was read-only with respect to repository content: the
reviewer wrote exactly one file (this report); **no other file was modified,
created or deleted**, and nothing was committed or pushed. The gate commands
regenerated only git-ignored `build/` artifacts (`build/{lwip-host-rev,lwip-host,native,native-rev,rtems-lwip,rtems-lwip-rev,renode-rev}`) and the reviewer's link-check artefact under `/tmp/opencode/`. Independence is declared per norm N9 (fresh context; no part in authoring, building-for-delivery or previously reviewing the items under review).*

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-12 | independent reviewer (FRICK-REVIEW-0150) | Initial combined R1+R2+R3+R4+PA review of the Sprint-6b-1 lwIP RTEMS `sys_arch` increment at `3822f72` + working tree |
