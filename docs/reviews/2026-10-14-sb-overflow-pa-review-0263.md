# Software Bus Full-Pipe Overflow — PA Review

|                                   |                                        |
| --------------------------------- | :------------------------------------- |
| Project                           | frick-fsw                              |
| Document id                       | FRICK-REVIEW-0263                      |
| Date                              | 2026-10-14                             |
| Reviewer profile                  | `aeronautics-space-sw-pa-engineer`     |
| Scope                             | SB full-pipe overflow event behavior and associated N22 test oracle |
| Baseline                          | Uncommitted working tree at HEAD `57974d2` |
| Norms                             | N1, N8, N9, N11, N12, N15, N20, N21, N22 |

> **Authority statement.** This is a bounded PA review result from the configured
> reviewer agent. It is not a human risk acceptance, safety approval,
> qualification, or release decision.

## 1. Finding

| # | Classification | Severity | Finding | Evidence | Required action |
|---|---|---|---|---|---|
| 1 | Functional | Major (provisional) | The software bus architecture requires that publishing to a full subscriber pipe drops the new copy, increments the dropped statistic, and raises an EVS error event. The implementation incremented the statistic but did not raise the required event. The robustness case was marked covered although the observable was absent. | `docs/architecture/software-bus.md` §2; pre-correction `src/core/sb/sb.c` `FF_SbPublish`; `docs/vv/test-spec/ts-6-sw-robustness.md` N22-TS6-44. | Open NC-EXT-75; correct event behavior safely, add a regression oracle, update the test case/docs, and independently verify. Do not count the full behavior as passing until all specified observables are demonstrated. |

## 2. Initial conclusion

**NOT APPROVED for the reviewed scope**, pending correction and independent
re-verification. The functional discrepancy required immediate correction under
N21. No immediate safety/security escalation was identified by this bounded
review; this is not a safety conclusion or residual-risk acceptance.

## 3. Scope boundaries

The review did not assess overall N20 phase approval, release readiness, target
timing, environmental/radiation qualification, or closure of NC-EXT-47. The N22
case tables are test specifications, not results by themselves.

## History

| Rev | Date | Author | Reason |
|---|---|---|---|
| 0.1 | 2026-10-14 | frick-fsw | Retain the PA agent's initial SB overflow finding; preserve the pre-correction NOT APPROVED point-in-time result for later delta review. |
