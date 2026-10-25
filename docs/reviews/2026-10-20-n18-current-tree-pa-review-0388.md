# FRICK-REVIEW-0388 — N18 current-tree evidence and status (PA)

| Field | Value |
| --- | --- |
| Date | 2026-10-20 |
| Profile | Independent Product Assurance (N20) |
| Scope | Current-tree N18 evidence/status delta; verification-results rev 2.98, tool-qualification T17, sprint-plan NC-TS-01, handover, loop-audit, Renode environment and retained `2026-10-20-n18-*` evidence |
| Conclusion | Bounded evidence review only; no phase/release/qualification approval |

## Assessment

The current-tree Renode PASS is supported within the stated emulator-only scope.
The retained record identifies a fresh build tree, compiler/toolchain image, ELF
hash, Renode image, UART output and process log. The selected build-input manifest
and stated log hashes were checked. The chronology distinguishes the earlier
misleading stale-ELF PASS, the subsequent true SKIP after removal, and the later
fresh-build `make renode` PASS. The runner still checks ELF **existence**, not
freshness; the run-specific mitigation does not repair or requalify it.

The documentation appropriately leaves NC-TS-01 and NC-EXT-40 OPEN and makes no
hardware, watchdog, HIL, reproducibility or tool-requalification claim.

## Finding PA-0388-01 — Minor — PCIe on-target claim lacks aligned evidence

`docs/vv/renode-environment.md` §5 states that the full PCIe enumeration/DMA path
is confirmed on-target, while the current handover states that no physical
RTPFS160ZT evidence is established. No configuration-matched target evidence
reference was identified in this bounded review. The Renode PCIe check is
explicitly register-logic/contract scope because the board repl does not
instantiate the PCIe root complex. The sprint plan records this as **NC-EXT-114
OPEN** pending controlled evidence reconciliation or separate reviewed
disposition. No target evidence is inferred.

## Limits

This review did not rerun the cross-build or Renode test and does not close
NC-TS-01, NC-EXT-40, NC-EXT-114, T17 requalification, N15, or any hardware,
qualification, safety, release or phase gate.
