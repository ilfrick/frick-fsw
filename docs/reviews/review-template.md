# Review Report Template

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Document id                      | FRICK-REVIEW-XXXX                      |
| Reviewer session                  | (independent agent, norm N9)           |
| Reviewer profile                  | R1 (SW dev) / R2 (SW V&V) / R3 (Sys Eng) / R4 (SW documentation) |
| Reference revision                | (commit or ref)                        |
| Items under review                | (paths / documents / requirement ids)  |
| Date                              | YYYY-MM-DD                             |
| Scope                             | (what was verified and against which ECSS references) |
| Independence declaration          | (reviewer had no part in authoring/executing the items; fresh context) |
| Norms applied                     | N1-N19 (see docs/rules.md)             |

## 1. Scope

(Describe precisely what was reviewed and what was excluded, with any
assumptions.)

## 2. References

- Project norms: `docs/rules.md` (N1-N19)
- Traceability manifest: `docs/requirements/traceability.yaml`
- ECSS-E-ST-40C, ECSS-Q-ST-80C, ECSS-E-ST-70-41C (PUS-C), CCSDS 133.0-B-2

## 3. Findings

| # | Severity (Major/Minor/Comment) | Item / Requirement | Finding | Evidence | Recommendation |
| - | ------------------------------ | ------------------ | ------- | -------- | -------------- |
| 1 |                                |                    |         |          |                |

**Severity definition**

- **Major**: non-conformity that disqualifies the item for the applicable
  criticality, or a defect that can defeat a requirement or a safety barrier.
- **Minor**: non-conformity that does not invalidate the requirement but must
  be corrected or justified.
- **Comment**: observation, recommendation, no derogation.

## 4. Conclusion

- [ ] APPROVED - no findings or Comments only
- [ ] APPROVED WITH COMMENTS - Minor findings tracked to closure
- [ ] REJECTED - one or more Major findings

## 5. Sign-off

| Role          | Agent / Reviewer | Date       | Conclusion |
| ------------- | ---------------- | ---------- | ---------- |
| R1 / R2 / R3 / R4 (N9) |        |            |            |

## 6. Follow-up records

(non-conformances and their dispositions, updated until closure; norm N12: NCs
are resolved by re-applying ALL rules to the affected scope and re-reviewing
the deltas until full compliance.)

| NC | Severity | Item / Requirement | Finding | Disposition | Re-verification evidence | Owner | Status (open/closed) | Delta re-review |
| -- | -------- | ------------------ | ------- | ----------- | ------------------------ | ----- | --------------------- | --------------- |
|    |          |                    |         |             |                          |       |                       |                 |

---

Committed and pushed with the reviewed change (norm N11).