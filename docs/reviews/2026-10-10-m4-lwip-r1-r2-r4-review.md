# Review Report - M4 increment 5 (lwIP network-stack adapter)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-0033                      |
| Reviewer session                  | independent agent, fresh context (norm N9) |
| Reviewer profile                  | R1 (SW dev) / R2 (SW V&V) / R4 (SW documentation, N17) |
| Reference revision                | base commit `e3176bd` + uncommitted working tree (M4 increment 5, 2026-10-10) |
| Items under review                | `src/hal/net/lwip/ff_net_lwip.{h,c}`, `src/hal/net/lwip/lwipopts.h`, `tests/unit/test_lwip.c`, `CMakeLists.txt`, `tests/CMakeLists.txt`, `docs/requirements/SRS.md` (SRS-REQ-040) + `docs/requirements/traceability.{yaml,md}`, `CHANGELOG.md`, `README.md` (counts), `docs/vv/verification-results.md`, `docs/vv/tool-qualification.md`, `.github/workflows/ci.yml`, `.gitea/workflows/ci.yml`, `tools/scripts/fetch_third_party.sh` |
| Date                              | 2026-10-10                             |
| Scope                             | Correctness and robustness of the `FF_NetStack_t` adapter over the lwIP socket API vs the contract in `src/hal/hal_net.h` (SRS-REQ-027/040): address encoding, blocking semantics, resource cleanup, defensive behaviour; third-party build hygiene (default OFF, no `-Werror` on third-party); test validity (real data path, not self-referential); requirement quality and traceability (N3/N4); documentation completeness/correctness/consistency (N8/N17); CI fetch reproducibility and counts vs the trace gate (75 reqs / 41 srs / 20 sss / 14 gap / 51 theorems). ECSS-E-ST-40C, ECSS-Q-ST-80C; norms N1-N17. |
| Independence declaration          | Reviewer took no part in authoring, implementing or executing any reviewed item; fresh context, independent session, report-only (no repository file other than this report was modified). |
| Norms applied                     | N1-N17 (see `docs/rules.md`)           |

## 1. Scope

Reviewed the milestone-M4 increment-5 delta (lwIP network-stack adapter,
SRS-REQ-040) against base commit `e3176bd` and the uncommitted working tree,
under three independent profiles:

- **R1 (design/code)**: `src/hal/net/lwip/ff_net_lwip.{c,h}` - conformance to the
  `FF_NetStack_t` vtable (`src/hal/hal_net.h`, SRS-REQ-027), the 6-octet
  IPv4[4]+port[2] big-endian address encoding, blocking semantics vs the
  documented bounded contract, resource cleanup, defensive programming and the
  `lwipopts.h` configuration; the CMake wiring (`frick_lwip` third-party target
  + `frick_net_lwip` adapter target) and third-party build hygiene.
- **R2 (V&V)**: adequacy and independence of `tests/unit/test_lwip.c` (does it
  exercise a real data path through lwIP and not a self-referential stub?), the
  honesty of the verification claims for SRS-REQ-040 (including the `lean_note`),
  the manifest entry, and the recorded evidence in `verification-results.md` /
  `tool-qualification.md`.
- **R4 (documentation, N17)**: completeness, correctness and consistency of the
  new/touched documentation - README counts and metrics panel, document
  revisions and histories, cross-references, the CHANGELOG entry, and the CI
  fetch step in both workflows.

Executed in this session (host, Linux; independent of the author):

- `python3 tools/scripts/trace.py docs/requirements/traceability.yaml` ->
  **PASS** (75 requirements: 41 SRS / 20 SSS / 14 GAP; 51 Lean theorems;
  9 CI claims).
- `cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build
  build/native && ctest --test-dir build/native --output-on-failure` -> **11/11
  PASS**, no lwIP target/tests present in the default build (verified with
  `ninja -t targets`).
- `bash tools/scripts/fetch_third_party.sh && cmake -S . -B build/lwiprev -G
  Ninja -DCMAKE_BUILD_TYPE=Debug -DFF_USE_LWIP=ON && cmake --build
  build/lwiprev && ctest --test-dir build/lwiprev -R lwip --output-on-failure`
  -> fetch OK, build **148/148**, test **1/1 PASS**.
- `./build/lwiprev/tests/test_lwip` -> `checks=15 failures=0` (matches the
  documented count); all 11 default suites re-run: 15+20+9+29+101+31+20+81+23+
  16+29 = **374** checks, all `failures=0` (matches the M4 cycle row).
- `python3 tools/scripts/metrics.py` -> computes `75 (srs=41, sss=20, gap=14)`,
  tests `61/75`, Lean `20/75` - which **differ** from the README panel (see
  R4-01).
- `gcc -std=gnu99 -Wall -Wextra -Wpedantic -c src/hal/net/lwip/ff_net_lwip.c
  -Isrc -Isrc/core -Iconfig -Isrc/hal/net/lwip
  -Ithird_party/lwip/src/include
  -Ithird_party/lwip/contrib/ports/unix/port/include` -> **no warnings**.
- `ninja -C build/lwiprev -t commands frick_lwip` -> compile lines carry no
  `-Werror`/`-Wall` (third-party hygiene); `ninja -t targets` shows the
  `lwipcore`/`lwipallapps`/`lwipmbedtls`/`lwipcontribportunix` targets are
  `EXCLUDE_FROM_ALL` and are **not** built (only `frick_lwip`).
- Trailing-newline and document-id scans; PNG-free source/doc inspection.

Excluded / not executable in this environment: clang-format and clang-tidy
(not installed here, no docker), so the format/static gates were re-checked only
by `-Wall -Wextra -Wpedantic` and inspection; `lake build` was not re-run here
(Lean theorem links are checked by the trace gate); on-target lwIP behaviour and
a real hardware netif are the documented GAP-A5 and were not executed.

## 2. References

- Project norms: `docs/rules.md` (N1-N17)
- HAL network contract: `src/hal/hal_net.h` (SRS-REQ-027)
- Reference stack (contract precedent): `src/drivers/net/ff_net_loopback.{c,h}` (SRS-REQ-039)
- Traceability manifest: `docs/requirements/traceability.yaml`
- Reviewer profiles: `docs/engineering/independent-review.md`
- Predecessor increment: `docs/reviews/2026-10-10-m4-netstack-r1-r2-r4-review.md`
  (open residuals D-04 / R4-03, R4-04)
- ECSS-E-ST-40C, ECSS-Q-ST-80C

## 3. Findings

| # | Severity | Item / Requirement | Finding | Evidence | Recommendation |
| - | -------- | ------------------ | ------- | -------- | -------------- |
| R1-01 | Minor | `src/hal/net/lwip/ff_net_lwip.c` (`ff_lwip_recv`); `lwipopts.h`; SRS-REQ-040 / `hal_net.h` | **The `recv` timeout is silently ignored, so the receive is not bounded.** `hal_net.h` documents the socket API as "blocking, bounded" and `recv` carries a `timeout_ms` argument, but `ff_lwip_recv` does `(void)timeout_ms` and calls the default blocking `lwip_recv`. A blocking TCP socket with no `SO_RCVTIMEO` will block indefinitely if the peer never sends, which can hang the owning task and defeats the documented bound. `lwipopts.h` does not enable `LWIP_SO_RCVTIMEO`, so implementing the timeout would also require a configuration change. | `ff_net_lwip.c:103-117` (`(void)timeout_ms`, `lwip_recv(..., 0)`); `hal_net.h:30,38-39`; `lwipopts.h:15-37` (no `LWIP_SO_RCVTIMEO`); the happy-path test always has data, so it cannot observe this. | Either implement the bound (`#define LWIP_SO_RCVTIMEO 1` + `lwip_setsockopt(SO_RCVTIMEO)` and map `EAGAIN`/`EWOULDBLOCK` to a documented status), or explicitly document that the adapter is unbounded-blocking and amend the contract comment. |
| R1-02 | Minor | `src/hal/net/lwip/ff_net_lwip.c` (`ff_lwip_fill_addr`, `ff_lwip_bind`, `ff_lwip_connect`) | **An invalid address is accepted and silently mapped to `0.0.0.0:0` instead of being rejected.** `ff_lwip_fill_addr` sets `AF_INET` unconditionally and only copies the address when `addr != NULL && addr_len >= 6`; a `NULL`/short address therefore becomes `bind`/`connect` on port 0 rather than `FF_INVALID_ARGUMENT`. The reference implementation rejects the same input, so the two backends of the same contract disagree on error behaviour. | `ff_net_lwip.c:19-27,54-66`; contrast `ff_net_loopback.c:79-90` (`addr == NULL or addr_len < 1` -> `FF_INVALID_ARGUMENT`). | Validate `addr != NULL && addr_len >= 6` in `bind`/`connect` and return `FF_INVALID_ARGUMENT`, matching the reference backend and the contract's defensive style. |
| R1-03 | Comment | `src/hal/net/lwip/ff_net_lwip.c` (`ff_lwip_attach_netif`); SRS-REQ-040 | `attach_netif` is a no-op, so the middleware offers no path to register a hardware netif through the adapter; a mission must still register it directly with lwIP. The SRS clause "so a mission can use lwIP without changing the middleware" and the manifest `lean_note` ("hardware netif are mission/GAP-A5") are honest but the limitation is only in the note, not the requirement/header. | `ff_net_lwip.c:40-44` (`(void)netif; return FF_SUCCESS;`); `traceability.yaml:387`; `hal_net.h:27-28` ("Optional"). | Document the intended netif-registration route in the (currently missing) adapter doc, or implement a minimal `attach_netif` hook; keep the GAP-A5 note pointing at it. |
| R1-04 | Comment | `src/hal/net/lwip/ff_net_lwip.c` (`ff_lwip_init`) | The lazy one-shot `g_inited` guard is a plain `static int` with no atomic/lock, and there is no `deinit`; the adapter's thread-safety and lifetime assumptions (single-threaded init; `tcpip_init` thread persists for process life) are undocumented. | `ff_net_lwip.c:17,29-38`; no mutex/atomic; no deinit symbol. | State "init once, single-threaded; no deinit" in the header, or guard with the platform once primitive. |
| R1-05 | Comment | `CMakeLists.txt:142` (comment) vs `:150-152` | The comment "Our adapter, compiled with the project's strict warnings" overstates the build: no `-Wall`/`-Wextra` are applied to first-party C at all, and `FF_ENABLE_WERROR` adds `-Werror` only to `frick_fsw_core`, not to `frick_net_lwip` (or the test targets). The adapter is shipped inside `frick_fsw_core` (via the PUBLIC link) yet is not warning-gated. | `CMakeLists.txt:138,142,150-152`; `ninja -C build/lwiprev -t commands frick_lwip` shows no `-Werror` on the third-party target (correct) but nothing gates the adapter either. | Extend the `-Werror`/warning set to `frick_net_lwip` (and ideally the first-party test targets), or correct the comment to state that the adapter is warning-checked only by the `-Wall -Wextra` probe. |
| R2-01 | Minor | `tests/unit/test_lwip.c`; SRS-REQ-040 (`method: Tst`) | **Coverage is a single-direction happy path; the adapter's own address-encoding on `accept` is never asserted and there are no negative cases.** The test sends 4 bytes client->server and checks the payload, but `peer`/`peer_len` from `accept` are never inspected (so `ntohs` splitting at `ff_net_lwip.c:83-84` is unverified); untested branches include `send`/`recv` with `NULL` out-params, invalid/unopened descriptors, connect to a non-listening port, the `recv` timeout path (R1-01), partial `send`, and `addr_len < 6` (R1-02). | `test_lwip.c:21-55`; `checks=15 failures=0`; driver branches `ff_net_lwip.c:73-87,89-117`; suite cannot detect R1-01/R1-02. | Add cases: assert `peer[0..3] == 127.0.0.1` and the big-endian port; NULL/bad-descriptor and short-address rejection; a bounded-recv expectation; a partial/oversize send. Re-record the check count. |
| R2-02 | Comment | `tests/unit/test_lwip.c:17-19` | The test re-declares `T_AF_INET 2` / `T_SOCK_STREAM 1` to "avoid pulling lwIP headers into the test". The test already links `frick_lwip` transitively, so including `lwip/sockets.h` would be faithful and remove duplicated magic numbers; the current form is brittle if the values ever diverge. | `test_lwip.c:17-19,36,40`; `tests/CMakeLists.txt:34,39` (links `frick_fsw_core` -> `frick_net_lwip` -> `frick_lwip`). | Include `lwip/sockets.h` and use `AF_INET`/`SOCK_STREAM`, or add a short comment justifying the literals. |
| R2-03 | Comment | `.github/workflows/ci.yml:51-64`, `.gitea/workflows/ci.yml:54-67` | The AddressSanitizer step runs only on the default core build; the lwIP adapter build/test is a separate step without sanitizers, so the new socket/adapter code carries no sanitizer or leak evidence (the loopback/SDLS increments had ASan evidence). | CI ASan step precedes the lwIP step in both workflows; `verification-results.md:27` reports only `clang-format clean` for M4. | Run the `FF_USE_LWIP=ON` build+tests under ASan (and, if convenient, a leak check on the socket lifecycle) and record it. |
| R2-04 | Comment | SRS-REQ-040 / manifest (positive) | The verification claims are **honest about scope and the data path is real**: `method: Tst or Inp`, `lean: []` with an explicit `lean_note` that separates the host loopback test from the on-target/GAP-A5 behaviour. The test drives a genuine TCP round trip through the lwIP `127.0.0.1` loopback (bind/listen/connect/accept then send `DE AD BE EF` on the client, recv on the accepted socket) - it is not self-referential and not a stub. | `traceability.yaml:380-387`; `SRS.md:197-200`; `test_lwip.c:21-55`; `test_lwip` `checks=15`; `trace.py` PASS. | (No action; keep the note accurate after R2-01/R2-03.) |
| R4-01 | Minor | `README.md:40-78` (metrics panel), `README.md:247-248` (N8/N16) | **The generated status panel is stale and contradicts its own "authoritative" statement.** The panel still reports "Requirements in manifest = 74 (srs=40, sss=20, gap=14)", "Test coverage 60/74", "Lean 20/74"; the very next section states `SRS = 41 ...` and calls the panel "authoritative", while `metrics.py` computes `75 (srs=41)`, `61/75`, `20/75` and the trace gate reports 75 reqs / 41 srs. The panel was evidently not regenerated after SRS-REQ-040 was added (only line 247 was edited in the delta). | `README.md:72-76` vs `:247`; `git diff README.md` (single-line change); `python3 tools/scripts/metrics.py` -> 75/41, 61/75, 20/75; `trace.py` -> 75 requirements. | Regenerate the panel with `python3 tools/scripts/metrics.py --readme` and re-run the trace gate/README consistency check in the same commit. |
| R4-02 | Minor | `src/hal/net/lwip/lwipopts.h:6`; SRS-REQ-040 (N8) | **Broken cross-reference and missing design documentation.** `lwipopts.h` says "see docs/architecture/network-stack.md", but that file does not exist and is not in the CI required-document list, so the reference is never caught. There is no architecture/design note for the adapter or the 6-octet address format (the only descriptions are source comments and the SRS/manifest). | `lwipopts.h:6`; `ls docs/architecture/` (no `network-stack.md`); CI `required` list (`.github/workflows/ci.yml:141-161`) does not include it. | Add `docs/architecture/network-stack.md` (loopback + lwIP adapter, address format, blocking semantics, netif route) and link it from the README doc index / CI list, or remove the dangling reference. |
| R4-03 | Minor | `docs/vv/verification-results.md:261-267` (N8) | **The M4 "Remaining" list and coverage note are stale.** The list still names "lwIP `FF_NetStack_t` adapter + GEM TSN" although the GEM TSN backend (increment 3) and the lwIP adapter (this increment) are delivered; only the N7/ESA PUS adapter and the other drivers remain. The "Coverage" paragraph cites only `test_netloop` and does not mention `test_lwip`; the M4 notes section has no increment-5 verification paragraph. This is the recurrence of the predecessor's open R4-04. | `verification-results.md:261-267`; `CHANGELOG.md:100-105`; `traceability.yaml:380-387`. | Update the remaining list (drop lwIP + GEM TSN), add `test_lwip` to the coverage paragraph, and add a short increment-5 verification note. |
| R4-04 | Comment | `docs/requirements/traceability.md:91` vs `traceability.yaml:384` | The manual traceability matrix abbreviates SRS-REQ-040 implementation as `hal/net/lwip/ff_net_lwip.c/h`, omitting `lwipopts.h` which the authoritative YAML lists. | `traceability.md:91`; `traceability.yaml:384`. | Add `lwipopts.h` to the row (or state that the matrix abbreviates). |
| R4-05 | Comment | `docs/requirements/SRS.md`, `docs/requirements/traceability.md`, `tests/CMakeLists.txt`, `docs/vv/tool-qualification.md` (N16) | Several re-touched files still end without a trailing newline (`\ No newline at end of file` in the delta diff) - the pre-existing D-07 / R4-05 item. Cosmetic. | `git diff` markers; direct byte check. | Add the trailing newline to the touched files. |
| R4-06 | Comment | `docs/reviews/README.md` (N11/N17) | This report's row has not been appended to the review log; the log also still shows the predecessor's "M4 netstack + M8 review ... REJECTED" without a delta row. This review was scoped to write the report only, so the log update remains an author action at cycle close. | `docs/reviews/README.md:33` (no increment-5 row). | Append the log row(s) and commit this report with the reviewed change (norm N11). |
| R4-07 | Comment | `README.md:128-131` (Low-level drivers), README doc index (N8) | The README "Low-level drivers" table lists only the loopback net stack and mentions lwIP parenthetically; there is no row for the delivered lwIP adapter, and the doc index has no network-stack entry. Recurrence of the predecessor's open R4-03. | `README.md:131`; predecessor review R4-03. | Add a network-stack row for `src/hal/net/lwip/ff_net_lwip.c` and update the doc index alongside R4-02. |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

## 4. Positive verification evidence (reproduced in this session)

- `trace.py`: `traceability: PASS (75 requirements, 41 srs, 20 sss, 14 gaps,
  51 lean theorems checked)`; the SRS-REQ-040 -> SSS-REQ-020 parent, all three
  implementation paths (`ff_net_lwip.{c,h}`, `lwipopts.h`), the test path
  (`test_lwip.c`) and the lean entry resolve; counts match the task baseline and
  the verification record.
- Default host build + `ctest`: **11/11 PASS**; no lwIP target or test is built
  with `FF_USE_LWIP` OFF (`ninja -t targets` shows none), confirming the
  "not built by default" requirement.
- lwIP path: `fetch_third_party.sh` populated `third_party/lwip`
  (`STABLE-2_2_1_RELEASE`, commit `77dcd25`, `contrib` present in the tree);
  `-DFF_USE_LWIP=ON` configured, **148/148** build targets, `ctest -R lwip`
  **1/1 PASS**; `test_lwip` reports `checks=15 failures=0`, matching
  `CHANGELOG.md:104`, `tool-qualification.md:79` and `verification-results.md:27`.
  The default check arithmetic reproduces exactly: 15+20+9+29+101+31+20+81+23+
  16+29 = **374** (all `failures=0`).
- **Third-party build hygiene is correct**: `frick_lwip` is compiled from
  `lwipnoapps_SRCS`/`lwipcontribportunix_SRCS` with no `-Werror`/`-Wall`
  (`ninja -t commands frick_lwip`), and the lwIP-shipped `lwipcore` /
  `lwipcontribportunix` targets (which do carry `-Werror`) are declared
  `EXCLUDE_FROM_ALL` and are not built. The adapter compiles clean under
  `gcc -Wall -Wextra -Wpedantic`.
- **Adapter/contract conformance**: every `FF_NetStack_t` member is implemented
  with the exact `hal_net.h` signature; `open`/`accept` return a negative
  `FF_NetStack_t` on failure as the contract requires; the address helper
  encodes IPv4 in network order and the port big-endian
  (`htons((addr[4]<<8)|addr[5])`) and decodes it symmetrically on `accept`
  (`ntohs`); `send`/`recv` report the transferred count via the out-parameter
  and reject `NULL` out-params; `close` maps to `lwip_close`.
- **Test validity**: a real TCP round trip over the lwIP loopback
  (`127.0.0.1:12345`) - not self-referential, not a mock. CI fetch+build+test
  steps are present and byte-identical in both workflows
  (`ci.yml:59-64` GitHub, `:62-67` Gitea).
- **Documentation set mostly updated and consistent**: README requirements-set
  line (41), SRS rev 1.1 (+ history), traceability.md rev 0.10 (+ counts/history),
  traceability.yaml entry, CHANGELOG entry, verification-results rev 0.11
  (+ history), tool-qualification rev 0.2.1 (T3 + test_lwip). Document-id
  scan across `docs/**` shows no duplicates. The `lean_note` for SRS-REQ-040 is
  explicit and honest about the on-target/GAP-A5 residual.

## 5. Conclusion

Graded conclusions by profile:

| Profile | Conclusion | Basis |
| ------- | ---------- | ----- |
| R1 (design/code) | **APPROVED WITH COMMENTS** | No Major: the adapter is a correct, minimal mapping of the 6-octet address format onto the lwIP socket API and conforms to the vtable. R1-01 (unbounded blocking `recv`; `timeout_ms` ignored) and R1-02 (invalid address silently becomes `0.0.0.0:0`) are Minor; R1-03..R1-05 are Comments. R1-01 should be closed or explicitly justified before commit (it is the only item with a plausible task-hang impact). |
| R2 (V&V) | **APPROVED WITH COMMENTS** | The `Tst` evidence is valid and the data path is real; check counts and the trace gate reproduce. R2-01 (single-direction happy path; `accept` peer address and all negative/timeout paths untested; the suite cannot detect R1-01/R1-02) is Minor; R2-02/R2-03 Comments; R2-04 records honest claims. |
| R4 (documentation, N17) | **APPROVED WITH COMMENTS** | Minor R4-01 (stale README metrics panel contradicting its "authoritative" statement and the trace gate), R4-02 (dangling `docs/architecture/network-stack.md` reference and missing adapter design doc), R4-03 (stale M4 "Remaining"/coverage note; recurrence of predecessor R4-04); R4-04..R4-07 Comments (recurrence of predecessor R4-03). Counts in the requirements sections, revisions, histories and id integrity are otherwise correct. |

Overall: **APPROVED WITH COMMENTS**. Per norm N12 the Minor findings are to be
dispositioned and tracked to closure (and the deltas re-reviewed) before the
cycle is closed; there is no Major finding blocking the increment. Priority
order: R4-01 and R4-02 (documentation correctness, cheap and unambiguous),
R1-01/R1-02 with R2-01 (contract semantics + the missing negative tests that
would have caught them), then the remaining Comments.

- [ ] APPROVED - no findings or Comments only
- [x] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

## 6. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 (N9)       | independent reviewer agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS (R1-01/R1-02 Minor) |
| R2 (N9/N10)   | independent reviewer agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS (R2-01 Minor) |
| R4 (N17)      | independent reviewer agent (fresh context) | 2026-10-10 | APPROVED WITH COMMENTS (R4-01/R4-02/R4-03 Minor) |

## 7. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
| R1-01 | Minor | `ff_net_lwip.c` (`ff_lwip_recv`), `lwipopts.h`, SRS-REQ-040 | `timeout_ms` ignored -> unbounded blocking `recv` (contract: "blocking, bounded") | Implement `SO_RCVTIMEO` bound or document/amend the contract | (pending) | frick-fsw | open | (pending) |
| R1-02 | Minor | `ff_net_lwip.c` (bind/connect) | Invalid/short address silently becomes `0.0.0.0:0` instead of `FF_INVALID_ARGUMENT` | Validate `addr`/`addr_len`; match the loopback backend | (pending) | frick-fsw | open | (pending) |
| R2-01 | Minor | `test_lwip.c`, SRS-REQ-040 | Happy path only; `accept` peer address + all negative/timeout paths untested; cannot detect R1-01/R1-02 | Add negative/boundary/address cases; re-record | (pending) | frick-fsw | open | (pending) |
| R4-01 | Minor | `README.md` metrics panel | Stale panel (74/srs=40, 60/74, 20/74) contradicts line 247 and `metrics.py` (75/41, 61/75, 20/75) | Regenerate with `metrics.py --readme` | (pending) | frick-fsw | open | (pending) |
| R4-02 | Minor | `lwipopts.h:6`, SRS-REQ-040 | Dangling `docs/architecture/network-stack.md`; no adapter design doc | Add the doc + link it, or drop the reference | (pending) | frick-fsw | open | (pending) |
| R4-03 | Minor | `verification-results.md:261-267` | Stale M4 "Remaining" (still lists lwIP/GEM TSN); no `test_lwip` coverage note; recurrence of predecessor R4-04 | Update the list and coverage note; add increment-5 note | (pending) | frick-fsw | open | (pending) |
| R1-03 | Comment | `ff_net_lwip.c` (`attach_netif`), SRS-REQ-040 | No netif attachment route through the adapter; limitation only in the manifest note | Document the route or implement the hook | (pending) | frick-fsw | open | (pending) |
| R1-04 | Comment | `ff_net_lwip.c` (`ff_lwip_init`) | Lazy `g_inited` not thread-safe; no deinit; lifetime undocumented | Document init/lifetime assumptions | (pending) | frick-fsw | open | (pending) |
| R1-05 | Comment | `CMakeLists.txt:142,150-152` | "strict warnings" comment overstated; `-Werror` covers only `frick_fsw_core` | Extend the warning gate or fix the comment | (pending) | frick-fsw | open | (pending) |
| R2-02 | Comment | `test_lwip.c:17-19` | Hardcoded `AF_INET`/`SOCK_STREAM` to avoid lwIP headers (test links lwIP anyway) | Include `lwip/sockets.h` or justify | (pending) | frick-fsw | open | (pending) |
| R2-03 | Comment | CI workflows | lwIP build/test runs without ASan | Add sanitizer evidence for the lwIP target | (pending) | frick-fsw | open | (pending) |
| R4-04 | Comment | `traceability.md:91` | SRS-REQ-040 row omits `lwipopts.h` (YAML includes it) | Add it or note the abbreviation | (pending) | frick-fsw | open | (pending) |
| R4-05 | Comment | SRS.md, traceability.md, tests/CMakeLists.txt, tool-qualification.md | Missing trailing newline (pre-existing D-07) | Add newline | (pending) | frick-fsw | open | (pending) |
| R4-06 | Comment | `docs/reviews/README.md` | Review-log row for this report not appended | Append at commit and push (N11) | (pending) | frick-fsw | open | (pending) |
| R4-07 | Comment | `README.md`, doc index | lwIP adapter missing from the driver table/doc index (recurrence of predecessor R4-03) | Add the network-stack row | (pending) | frick-fsw | open | (pending) |

---

Committed and pushed with the reviewed change (norm N11).

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | independent reviewer agent (R1/R2/R4) | Initial milestone-M4 increment-5 review (lwIP network-stack adapter, SRS-REQ-040) |
