# FRICK-REVIEW-0411 — S8-FDIR requirement and system delta

**Profile:** R3 requirements/systems, fresh-context review  
**Disposition:** Draft findings; no requirements approval, safety acceptance,
or risk/NC closure.

## Findings

1. **Major — report obligation under failure/absence is ambiguous.** SRS-REQ-074
   says “exactly one” report but also permits notification failure/absence
   without preventing reset. It does not define whether exactly one is an
   attempted report, locally generated report, or completed write, nor its
   applicability when reporter installation/formatting fails.
2. **Major — mandatory S5 content envelope is undefined.** The requirement
   does not identify mandatory text, maximum reason length, or overlength
   behavior. Current PAL suppresses notification when formatting is
   incomplete, and the PUS event payload has a fixed text capacity.
3. **Major — 250 ms remains an unverified initial target.** The source threads a
   monotonic deadline, but available tests do not prove the end-to-end bound for
   all operations/backends.
4. **Major/safety-relevant — every pre-reset operation must be bounded.**
   Formatting and console precede reset. A bounded UART callback alone does not
   prove that all earlier work cannot strand reset; retain the existing safety
   authority review and evidence obligation.
5. **Major — SRS-REQ-073 does not define actual inter-feed timing.** The feeder
   remains relative-delay based; execution and scheduling latency add to the
   inter-feed interval. `FF_AppWdPlan` arithmetic is not task timing evidence.
6. **Major — monotonic feed/PPS policy is incomplete in requirement and code.**
   Owner ESC-6 requests absolute monotonic scheduling with execution/jitter
   margin and coherent CUC/PPS mapping, but the current loop uses relative
   `FF_PalTaskDelayMs`; loss/invalid PPS behavior and selected margin are open.
7. **Major — refresh-window authority is missing.** A public HAL header gives
   an audit lead, not the configured target's earliest valid refresh instant.
   Obtain controlled RTPFS160ZT/MSS documentation and hardware configuration
   evidence before selecting cadence.

The public vendor HAL reference is mutable and does not establish target
register mapping, units, clock/reset dependencies, or RTEMS binding. Keep the
8 s default explicitly an arbitrary placeholder.

**Review basis:** SRS, traceability, owner decisions, ADR-0021 and source
inspected 2026-10-21. **Confidence:** high for the requirement/source mismatch;
no flight suitability inference.
