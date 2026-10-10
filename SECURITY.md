# Security Policy (frick-fsw)

## Supported versions

| Version | Status |
| ------- | ------ |
| 0.1.x   | Supported (current baseline) |

## Reporting a vulnerability

frick-fsw is space/aeronautics software that may run on critical systems.
Security (including robustness/fault containment, PUS command injection risks,
CRC/integrity protections) is part of the V&V scope (norm N6) and of the
review process (norms N9/N10/N12).

- Report privately to the maintainers (GitHub security advisories on the
  primary mirror, or the repository owners).
- Do **not** open public issues for vulnerabilities before a fix is released.
- Include: affected version, subsystem, minimal reproduction, and impact.

## Response

- Acknowledgment within 48h; assessment by an R1/R2 profile reviewer
  (`docs/engineering/independent-review.md`).
- Fixes follow the full development/verification cycle (norm N15), are traced
  (norm N4) and recorded in `CHANGELOG.md` (norm N16).
- Release dates: as required by the criticality; security fixes back-ported to
  the supported stream.

## Threat model highlights

- Command authentication is a **mission** concern: the ICD (`docs/interfaces/ICD.md`)
  baseline does not implement TC authentication; missions must add it at the
  link level per ECSS-E-ST-70-41C tailoring (tracked as a class-A review item).
- Frame integrity relies on CRC-16/CCITT-FALSE (SRS-REQ-004); it detects random
  corruption, not adversarial forgery (GAP-A1 deep formalisation).