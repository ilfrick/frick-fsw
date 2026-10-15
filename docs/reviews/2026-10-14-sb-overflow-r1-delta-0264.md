# Software Bus Overflow EVS Dispatch — R1 Delta Review

|                                   |                                        |
| --------------------------------- | :------------------------------------- |
| Project                           | frick-fsw                              |
| Document id                       | FRICK-REVIEW-0264                      |
| Date                              | 2026-10-14                             |
| Reviewer profile                  | `aeronautics-space-fsw-engineer`       |
| Scope                             | Deferred EVS notification for SB full-pipe behavior after FRICK-REVIEW-0263 |
| Baseline                          | Uncommitted working tree at HEAD `57974d2` |
| Norms                             | N1, N8, N9, N11, N12, N15, N21, N22    |

> **Authority statement.** Bounded independent-context engineering review only;
> not human approval, safety acceptance, qualification, or release approval.

## 1. Scope reviewed

Reviewed `src/core/sb/sb.c`, `src/core/evs/evs.c/.h`,
`src/apps/app_tm_output.c`, `tests/unit/test_sb.c`, FRICK-ARCH-0002, TS-5/TS-6,
and associated V&V records. No files were modified by the reviewer.

## 2. Findings

| # | Classification | Severity | Finding | Evidence | Required disposition |
|---|---|---|---|---|---|
| 1 | Functional | Major, provisional | Literal `publish never blocks` remains unproven: `FF_SbPublish` locks `g_bus_lock` with `FF_PAL_WAIT_FOREVER`; `FF_EvsSendDeferred` locks the EVS mutex with the same wait. PAL implements this as blocking `pthread_mutex_lock`. | `src/core/sb/sb.c` `FF_SbPublish`; `src/core/evs/evs.c` `FF_EvsSendDeferred`; `src/core/pal/pal_posix.c` mutex-lock implementation; FRICK-ARCH-0002 §2. | Keep under NC-EXT-75. Define contention status/no-partial-fanout behavior and demonstrate forced-contention return before lock release, without weakening the controlled no-block contract. |
| 2 | Functional | Major, provisional | EVS deferred-queue saturation logs the event but drops listener/ground forwarding; `FF_SbPublish` discards the notification failure. This conflicts with SRS-REQ-011, which requires every on-board EVS event to be forwarded to ground via PUS service 5. | `src/core/evs/evs.c` deferred queue-full branch; `src/core/sb/sb.c`; `docs/requirements/SRS.md` SRS-REQ-011; `tests/unit/test_sb.c` saturation case. | Track as NC-EXT-76. Establish bounded event-rate/queue/dispatch assumptions and a controlled overload policy that reconciles nonblocking publish with SRS-REQ-011; no silent loss or unilateral requirements change. |

## 3. Corrections confirmed in this delta

- Ordinary `FF_EvsSend` is synchronous again; only SB overflow uses the explicit
  deferred API.
- The SB publisher no longer synchronously invokes listener/TM/UART work.
- Deferred dispatch processes only a bounded entry snapshot and prevents
  recursive dispatcher entry; unit tests exercise listener-triggered SB
  re-entry and saturation accounting.
- Those corrections do not resolve the two functional findings in §2.

## 4. Review conclusion

**NOT APPROVED** for strict FRICK-ARCH-0002 §2 and SRS-REQ-011 compliance.
NC-EXT-75 and NC-EXT-76 remain open. The reviewer did not independently execute
the recorded verification commands; existing `verification-results.md` claims
are cited as recorded evidence only. No separate safety/security escalation is
established by this bounded review, and no risk acceptance is made.

## History

| Rev | Date | Author | Reason |
|---|---|---|---|
| 0.1 | 2026-10-14 | frick-fsw | Retain independent R1 delta review of the deferred SB overflow event path and the two remaining functional contract gaps. |
