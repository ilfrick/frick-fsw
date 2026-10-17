# Independent avionics/systems requirement grounding review (profile R3)

| Field | Value |
| --- | --- |
| Document id | FRICK-REVIEW-0371 |
| Reviewer session | Independent agent, norm N9 |
| Reviewer profile | R3 (Systems Engineering) |
| Reference revision | git commit b8c3aee + uncommitted S8-SCH / S8-VV-FU / S8-PUS-FU deltas |
| Items under review | ADR-0020, SRS.md/SSS.md (relevant deltas), ts-1-protocol-core.md (rev 1.8), ts-5-core-services.md (rev 1.2), ICD.md, pus-layer.md, scheduling.md, sprint-plan.md §3 NCs |
| Date | 2026-10-17 |
| Scope | Requirement grounding and system-engineering consistency of provisional PUS/S6/scheduler policy; oracles vs approved baseline; authority ownership; interface/traceability consistency (norms N3/N9/N14). |

## 1. Independence declaration

The reviewer worked only from repository artifacts. No files were modified except this report. No commits/pushes performed.

## 2. Scope and exclusions

Reviewed the uncommitted delta set (S8-SCH/S8-VV-FU/S8-PUS-FU) as specified. Excluded: mission profile approval, phase/release approval, full ECSS clause-level compliance, target hardware evidence, qualification, human-rating. Did not assume any approved mission PUS profile/ICD.

## 3. References

- Project norms: `docs/rules.md` (N1-N22)
- Traceability manifest: `docs/requirements/traceability.yaml`
- ECSS-E-ST-70-41C (2016), pinned SHA-256 `fed627dae2aa2a1e9dbfb873861002fbee40a37a3883019d6701af34852ea51d`
- ADR-0020: `docs/adr/ADR-0020-provisional-pus-and-scheduler-policies.md` (rev 0.1, 2026-10-16)
- SRS: `docs/requirements/SRS.md` (rev 1.36, 2026-10-16)
- SSS: `docs/requirements/SSS.md` (rev 0.20, 2026-10-16)
- ICD: `docs/interfaces/ICD.md` (rev 1.2, 2026-10-16)
- TS1: `docs/vv/test-spec/ts-1-protocol-core.md` (rev 1.8, 2026-10-16)
- TS5: `docs/vv/test-spec/ts-5-core-services.md` (rev 1.2, 2026-10-16)
- pus-layer.md rev 0.1.7; scheduling.md rev 0.3

## 4. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | --- | --- | --- | --- | --- |
| F-001 | Major | ADR-0020 (provisional PUS/S6/scheduler policy) | Policy is presented as provisional framework defaults and explicitly disclaims mission authority (ADR-0020 §3-6). However, some downstream artifacts (tests/specs) cite the provisional profile as their oracle without re-stating authority boundary in each affected row. | ADR-0020; TS1-103,45-49; TS5-77; arch/pus-layer.md §6 | Keep ADR-0020 as the single authority-boundary record; ensure TS1/TS5 status lines continue to carry "provisional ... authority/profile disposition remain open" when oracles derive from ADR-0020. No change to ADR-0020 content; boundary language already exists. |
| F-002 | Major | S6 wire profile: ICD vs delta (ICD.md §4 line 107) | ICD §4 still documents S6 TC[6,5] as `addr[4]+size[2] big-endian`. Delta implements/adopts provisional profile: `region_id:u8, byte_offset:u32 BE, length:u16 BE` (7-byte payload for TC[6,5], TC expects len==7). TS1 cites ADR-0020 for S6 and does not treat ICD as authoritative. ICD not updated. | ICD.md l.107; pus-layer.md §6; svc6_dump (pus_services.c); TS1-1 interfaces-cited row; TS1-45-49 | Update ICD §4 to reflect the provisional S6 profile (region ID + byte offset + length) with a note that it is provisional per ADR-0020 and requires authority approval; or explicitly mark S6 row as provisional/tailored and deprecate the old addr+size form. Include traceability/tailoring record for the interface change. |
| F-003 | Major | S3 subservice mapping and ICD description | Delta adopts ECSS-E-ST-70-41C (2016) aligned mapping: TC[3,1] create, TC[3,3] delete, TC[3,5] enable periodic, TC[3,6] disable periodic, TC[3,27] one-shot request (matches ECSS 2016). ICD §4 line 105 describes S3 admin as generic "(admin) create/delete/enable/disable" with no subservice codes — vague but not directly contradictory; architecture and tests specify exact codes. | pus_services.c svc3 table and handlers; TS1-103; TS1-42-44,87; arch/pus-layer.md §6; ICD.md l.104-105 | Update ICD §4 to list concrete S3 subservice codes (1,3,5,6,27) for the implemented admin/report behavior and state provisional nature per ADR-0020/approved profile. |
| F-004 | Major | Scheduler exhaustion/time domain (ADR-0020 + scheduling.md + TS5-77) | Policy: checked non-wrapping deadline arithmetic; unrepresentable initial/next target disables entry and increments time_exhaustions; activation clears targets (must re-init); missed counter saturates at UINT32_MAX; host tests cover behavior (TS5-77, test_sched). This is explicitly provisional (ADR-0020 §3, scheduling.md rev 0.3, TS5-77). | ADR-0020; scheduling.md §5; TS5-77; src/core/sched/ff_sched.c; test_sched time_exhaustion | Ensure authority disposition (Systems/FSW authority) is obtained for monotonic time domain, exhaustion/recovery behavior, and target evidence before operational use; policy wording already disclaims mission approval. No code change needed; evidence/authority gates remain open (NC-EXT-90). |
| F-005 | Minor | TS1-103 clause citation precision | TS1-103 cites "ECSS-E-ST-70-41C clauses 6.3.3.4.1/.2 and public subservice labels". For ECSS-E-ST-70-41C 2016, enable/disable are 6.3.3.4.1/.2 (and create/delete 6.3.3.5.1/.2). The citation is reasonable for the pinned 2016 edition in that context, but the Rev.1 DIR1 renumbering can confuse readers. | TS1-103; pinned ECSS 2016 text (6.3.3.4.1/.2 for enable/disable) | Clarify citation to state "ECSS-E-ST-70-41C (2016) 6.3.3.4.1/.2 for enable/disable periodic generation" or include edition note. Minor doc clarity only; no oracle change. |
| F-006 | Comment | ICD/TS1 trace de-linking | TS1 §1 interfaces-cited row de-links S6 from ICD §4 and references ADR-0020. This is consistent with treating ADR-0020 as controlling provisional profile, but leaves ICD stale (F-002). | TS1-1; ICD.md | Accept TS1's explicit de-linking as a pragmatic boundary in test spec; address ICD staleness via F-002. |
| F-007 | Comment | NC dispositions in sprint-plan §3 (scoped rows) | For NC-EXT-14-18,19,20,21,22,88,89,90,92: dispositions explicitly state authority ownership where applicable (14-18/22 authority-dependent; 19/90 require authority/target evidence; 88/89 narrowly closed with no phase/compliance claim; 92 pre-existing). The narrow closures preserve boundaries (explicit "no PUS-C compliance claim", "full-cycle/PA/R4/release gates remain separate"). | sprint-plan.md lines ~146-157, ~222-226, ~501 (rev 1.68) | Dispositions stay within evidence support. Authority-owned items (profile/timing/exhaustion/S1/S3/S6/S11 policy, ICD alignment) remain clearly assigned to appropriate authorities (FSW+system/ICD authority, Systems+FSW). No recommendation to close authority-owned NCs. |

## 5. Authority-owned open items (explicit list)

1. **ADR-0020 policy approval** (all provisional choices): mission PUS profile, S3 subservice allocation/semantics (including one-shot vs periodic behavior boundaries), S6 logical-region ID/byte-offset/length profile and memory access authorization model, scheduler checked-arithmetic/exhaustion/recovery policy and monotonic time domain. Owner: Systems authority (with FSW/ICD/HAL). Status: OPEN (ADR-0020 disclaims approval).
2. **NC-EXT-14 (S1 Request ID construction)** — reconcile against controlled PUS baseline and approved tailoring; owner FSW+system/ICD authority; verifier R1/R2/R3. OPEN.
3. **NC-EXT-15 (S1 verification sequencing/ack flags; unknown-service classification)** — authority disposition required where SRS-REQ-005 conflicts with oracle; owner FSW+system authority; R1/R2/R3/R4. OPEN.
4. **NC-EXT-16 (S3/S9/S20 subservice assignments vs controlled profile)** — mission service profile needed; owner FSW+PUS/ICD authority; R1/R2. OPEN.
5. **NC-EXT-17 (S3 behavior broader than corrected first-failure)** — broader S3 ID/enable/remove/provider/periodic semantics subject to controlled profile/interface reconciliation; bounded correction closed under NC-EXT-88 but not the profile. Owner FSW+system authority. OPEN.
6. **NC-EXT-18 (S6 memory access/region contract/address width)** — define address width and memory-region ID in controlled interface; owner FSW+HAL/system authority; V&V. OPEN.
7. **NC-EXT-19 (scheduler starvation correction implemented; mission timing inputs)** — controlled mission-table/timing inputs, Table Services integration, target timing/fault-policy evidence remain; owner FSW+system authority; M17/S8-SCH. OPEN (functional correction present).
8. **NC-EXT-20 (N7 adapter CRC policy)** — bounded CRC/PDL checks implemented and tested; generated N7 SDK compatibility and profile configuration need disposition; owner FSW+PUS authority; V&V. OPEN.
9. **NC-EXT-21 (PUSopen APID validation)** — bounded seam present; commercial SDK compatibility remains unverified; owner FSW; V&V. OPEN.
10. **NC-EXT-22 (S11 stub)** — tailored S11 capability vs reject/disable; not substitute for M17; owner FSW+PUS authority; R1/R2/R3 (M15). OPEN.
11. **NC-EXT-90 (scheduler exhaustion/time domain)** — obtain Systems/FSW authority disposition and target/mission evidence; do not infer mission epoch/recovery action. Owner Systems+FSW; M17/S8-SCH. OPEN.
12. **ICD authority alignment (F-002,F-003)** — S6 profile and S3 subservice codes in ICD require authority approval/tailoring; ICD update or explicit tailoring record needed. Owner ICD/system authority with FSW. OPEN.
13. **Mission profile/ICD approval for provisional choices** — ADR-0020 defaults must be replaced/accepted by approved mission ICD and baseline; no mission procedure enablement from ADR alone. Owner mission/customer authority. OPEN.

## 6. System-level consistency assessment

- **Units/widths**: S6 offset 32-bit BE, length 16-bit BE; region_id u8 — internally consistent in code/arch/tests/ADR. Mismatched vs ICD (4+2 addr+size). Widths are project choices (provisional) — not baseline-approved. (F-002)
- **Time domains**: monotonic ns uint64_t; checked arithmetic with exhaustion policy — consistent within delta; explicitly provisional. (F-004)
- **Service identifiers**: S3 mapping matches ECSS-E-ST-70-41C 2016 (3,1/3,3/3,5/3,6/3,27). S6 uses logical region_id. S11 registered as stub. Consistent across implementation/tests/arch; ICD lags. (F-003)
- **Request/response pairing**: S3 report-all (3,27) preserves first failure, continues processing, emits TM(3,25) for valid reports; bounded oracles cover orders. S6 TC(6,5)->TM(6,6) with region bounds. Consistent. (covered by NC-EXT-88/89 evidence)
- **Mode/state**: one-shot independent of periodic enable/disable; enable/disable affect periodic state. Consistent.
- **External interface**: ICD not updated to match provisional profile — inconsistency remains. TS1 de-links S6 from ICD (boundary marker). (F-002, F-006)

## 7. Traceability/tailoring

- Requirements SRS-REQ-008/009 unchanged; SRS-REQ-071 unchanged (states mission-controlled inputs). SRS-REQ-050 modified earlier with ADR-0019 trace. No requirement encoded as provisional baseline; requirements remain bounded.
- ADR-0020 added as decision record with disclaimer and public sources (ECSS 2016 pinned). Trace to provisional policy exists.
- ICD not updated; no tailoring record showing ICD supersession for S6/S3 codes. (gap relative to interface change) (F-002,F-003)
- Test specs cite ADR-0020 explicitly for provisional oracles (TS1-103, TS5-77) — good boundary marking. (F-001)

## 8. Conclusion

**CHANGES REQUIRED**

Rationale:
- Interface documentation drift: ICD §4 S6 profile (addr[4]+size[2]) is inconsistent with the delta's provisional profile (region_id[1]+byte_offset[4]+length[2]) and implementation; ICD S3 admin lacks concrete subservice codes. No corresponding ICD update or tailoring record exists in the delta (F-002,F-003).
- Authority ownership is clearly stated in ADR-0020 and many NC dispositions, but ICD remains stale and could mislead readers about the wire profile; update is required to maintain coherent traceability.
- All authority-owned items listed in §5 remain OPEN and must be dispositioned by appropriate authorities before treating provisional choices as baseline/mission profile. Narrow NC closures (88/89/91) are appropriate and bounded.

Required actions before accepting this delta as interface-consistent:
1. Update `docs/interfaces/ICD.md` §4 S6 row to provisional profile (region_id[1], byte_offset[4] BE, length[2] BE) with note "provisional per ADR-0020; requires mission authority approval; not a mission ICD". 
2. Update `docs/interfaces/ICD.md` §4 S3 row to list implemented subservice codes (3,1 create; 3,3 delete; 3,5 enable periodic; 3,6 disable periodic; 3,27 one-shot report; TM 3,25) and note provisional/authority-dependent where applicable.
3. Add a short tailoring note in ICD (or link to ADR-0020) explaining de-linkage rationale and that old S6 form (addr[4]+size[2]) is superseded by provisional profile in this framework reference.
4. Verify TS1/ICD cross-references remain consistent after update.

Other findings (F-005 minor doc clarity) are non-blocking but recommended.

## 9. What this review does not establish

- No requirement baseline approval, no mission PUS/S6/scheduler profile approval, no Space-to-Ground ICD approval.
- No phase/release approval, no flight readiness, no certification/human-rating claim.
- No ECSS compliance determination for a mission; only that provisional public references are cited with disclaimer and internal consistency assessed.
- No target hardware/timing qualification evidence; Renode/host evidence only with explicit scope limits.
- No closure of authority-owned NCs (14-18,19-22,90) or broader PUS completeness (GAP-A2 remains M15). Narrow closures (88/89) do not waive gates.
- No guarantee ICD update is sufficient without mission authority sign-off.

## 10. Review artifacts inspected

- ADR-0020: rev 0.1 (2026-10-16)
- SRS.md: rev 1.36 (2026-10-16) — SRS-REQ-008/009/050/071 context
- SSS.md: rev 0.20 (2026-10-16) — SSS-REQ-008/015/025/030 context
- ICD.md: rev 1.2 (2026-10-16)
- ts-1-protocol-core.md: rev 1.8 (2026-10-16) — cases 42-49,97-103
- ts-5-core-services.md: rev 1.2 (2026-10-16) — scheduler cases 43-48,77
- pus-layer.md: rev 0.1.7 (2026-10-16)
- scheduling.md: rev 0.3 (2026-10-16)
- sprint-plan.md: rev 1.68 (2026-10-16), §3 NC rows 14-24,88-92
- implementation: src/pus/pus_services.c (S3/S6 changes), src/core/sched/ff_sched.* (exhaustion), tests/unit/test_pus_fu.c, tests/unit/test_sched.c
- ECSS-E-ST-70-41C 2016 (pinned SHA-256 verified)

## 11. Summary

- Provisional policy is clearly bounded by ADR-0020 and status qualifiers in test specs/arch.
- S3 subservice mapping is ECSS-E-ST-70-41C 2016-consistent; S6 is a logical-region abstraction (provisional). 
- Major interface inconsistency: ICD not updated for S6 (addr[4]+size[2]) vs provisional (region_id+offset+length); S3 codes missing. **CHANGES REQUIRED** (ICD update + tailoring record).
- Scheduler exhaustion policy implemented with host tests, explicitly provisional; NC-EXT-90 remains OPEN for authority.
- Narrow NC closures (88/89) defensible; authority-owned items (14-18,19-22,90, ICD/profile approval) remain OPEN. No mission profile approval implied.