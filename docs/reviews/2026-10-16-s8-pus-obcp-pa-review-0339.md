# Bounded PA review 0339 — S8-PUS-FU / selected N7 OBCP

**Date:** 2026-10-16  
**Scope:** Uncommitted S8-PUS-FU changes and selected pinned N7 OBCP integration/evidence.  
**Conclusion:** No functional defect or immediate safety/security concern identified in this bounded review. N20 phase/product approval is **not** granted; configuration-specific assurance remains limited.

## Findings

### PA-0339-01 — Candidate identity does not match current evidence

**Severity:** Major assurance/evidence gap. **Effect:** Blocks using these results
as configuration-matched evidence for phase/release decisions; under N21 it does
not stop unrelated development.

The available S8 candidate manifest covers an earlier UART/GSW scope and does
not inventory the current PUS/OBCP delta (`docs/vv/evidence/2026-10-16-s8-fu-r1-candidate-manifest.md`). Newer logs are not tied to a digest of this exact candidate. Provide a current manifest covering source/configuration, generated inputs and artifact identities before relying on the run for a gate decision.

### PA-0339-02 — Generator provenance is thinner than source pin provenance

**Severity:** Minor assurance gap. CMake invokes upstream `make flat` and builds
generated files, but the current log does not identify the generation tool
version or hash the generated source set (`CMakeLists.txt:73–108`). Track an NC
for reproducibility/provenance; this is not a functional-development blocker.

## Bounded observations

- PUS and OBCP evidence claims are scoped to host tests and common generated
  types/CRC; profile compatibility and full service mapping are not claimed.
- OBCP cooperative abort, unbounded arbitrary Python, and unreliable upstream
  interpreter-result reporting are explicitly open; production bindings and
  target suitability are not claimed.
- Failed oracle/stale-cache attempts are retained and distinguished from passes.

This report is not a qualification, safety, release or risk-acceptance decision.
