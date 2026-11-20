# frick-fsw Pi subagent profiles

These Pi-discoverable profile adapters make the project's five reviewer personas available to `pi-subagents` from a trusted project session. The authoritative, version-controlled persona texts remain in `docs/engineering/reviewer-agents/`; each adapter names the corresponding source and requires the child to read it and `AGENTS.md` before acting. Keep adapter names and source paths synchronized if a persona is renamed.

Available Pi agent names:

- `aeronautics-space-fsw-engineer` — FSW architecture, real-time behavior, code, drivers, and FDIR.
- `aeronautics-space-sw-vv-engineer` — requirements-based V&V, test design, traceability, and evidence.
- `aeronautics-space-avionics-systems-engineer` — systems, avionics, interfaces, and integration.
- `aeronautics-space-sw-documentation-engineer` — documentation, records, traceability, and claims.
- `aeronautics-space-sw-pa-engineer` — product assurance, process, configuration, and evidence integrity.

The adapters grant read-only tools (`read`, `grep`, `find`, `ls`); they do not grant shell or file-editing capability. A subagent review is not organizationally independent merely because it runs in a fresh context, and it cannot approve, qualify, close findings, accept risk, or authorize release. Follow `docs/engineering/independent-review.md` and `AGENTS.md` for actual review and approval gates.

In a normal Pi session with `pi-subagents` loaded and the project trusted, invoke an agent by its exact name through the `subagent` tool, for example:

```text
subagent({ agent: "aeronautics-space-sw-vv-engineer", task: "Review the specified test changes read-only against the controlling requirements and report evidence, findings, and limitations." })
```

Project revision history: Initial Pi adapters for the five existing reviewer personas; no source persona content changed.
