# Hermes runtime configuration snapshot

| Field | Value |
|---|---|
| Project | frick-fsw |
| Document id | FRICK-ENG-0008 |
| Revision | 0.3 |
| Date | 2026-10-28 |
| Scope | Machine-local Hermes configuration observed for the project operator; not a portable project default |
| Norms | N2, N8 |

This note records the observed Hermes profile/model configuration used with this
project. It contains no credentials. These are operator-home settings, not
frick-fsw runtime requirements; re-check the live profile before relying on this
snapshot after a Hermes or profile change.

## 1. Chat models by profile

Observed with `hermes profile list` and profile-scoped `hermes config get` on
2026-10-28. The active session profile was `default`.

| Hermes profile | `model.default` | `model.provider` | `model.base_url` | External memory provider |
|---|---|---|---|---|
| `default` (active) | `gpt-6-luna` | `openai-codex` | `https://chatgpt.com/backend-api/codex` | Built-in only |
| `jeeves` | `gpt-6-luna` | `openai-codex` | `https://chatgpt.com/backend-api/codex` | `ollama_memory` |
| `researcher` | `gpt-6-luna` | `openai-codex` | `https://chatgpt.com/backend-api/codex` | Built-in only |
| `reviewer` | `gpt-6-luna` | `openai-codex` | `https://chatgpt.com/backend-api/codex` | Built-in only |
| `writer` | `gpt-6-luna` | `openai-codex` | `https://chatgpt.com/backend-api/codex` | Built-in only |

No explicit `delegation.model` or `delegation.provider` is configured in these
profiles. The checked auxiliary slots (`vision`, `compression`, and
`title_generation`) have provider `auto` and no explicit model. This snapshot
does not assert which model was selected for every auxiliary call.

## 2. Jeeves memory-provider configuration

The active `default` profile has no external memory provider; the read-only
`hermes -p default memory status` reports built-in memory only. The separate
`jeeves` profile has `memory.provider: ollama_memory`, and its read-only memory
status reports the provider available. Observed settings:

- Ollama endpoint: `http://127.0.0.1:11434`
- Embedding model: `qwen3-embedding:4b` (2560 dimensions; a real local embedding
  request was verified on 2026-10-28)
- Collection: `hermes_memory`
- `auto_embed_writes`: `true`
- Profile database: `/home/nicola/.hermes/profiles/jeeves/ollama_memory.db`

The current plugin implementation's `sync_turn` stores each user/assistant turn
(up to 2000 characters from each side) when `auto_embed_writes` is true. This is
separate from `memory.write_approval`; that setting was observed as `false` and
does not disable the plugin's automatic turn sync. Do not infer that the memory
database contains only explicitly saved facts.

The plugin is enabled in the `jeeves` profile at
`/home/nicola/.hermes/profiles/jeeves/plugins/ollama_memory/` (Hermes reports
version `1.0.0`, source `git`). `hermes -p jeeves plugins validate` passes, with
a warning that `config_schema.py` imports private Hermes core APIs. The local
checkout's current Git HEAD is `4784936fb3a2b494ee53bee866414f8f9fe49923`;
`__init__.py` and `plugin.yaml` have uncommitted source changes, and the working
tree is not a clean, provenance-tracked update. A prior snapshot cited commit
`b8ad9e2bf63b3a79f8eb333dbe4cafbce2f81333`, but that is not the installed
checkout's current Git HEAD. The plugin is not configured for the active
`default` profile, and a live gateway reload is not verified; this note does
not claim a clean deployment to the current session.

## 3. Configuration paths and read-only checks

- Active `default` profile config: `/home/nicola/.hermes/config.yaml`
- `jeeves` profile config: `/home/nicola/.hermes/profiles/jeeves/config.yaml`
- Other profile paths: query `hermes -p <profile> config path`
- Model values: `hermes -p <profile> config get model.default` and
  `hermes -p <profile> config get model.provider`
- Memory provider: `hermes -p <profile> memory status`
- Captured read-only CLI output (credentials excluded):
  [`hermes-config-current-20261028.txt`](../vv/evidence/2026-10-28-resume-gates/hermes-config-current-20261028.txt)

Hermes documents `model.default` and `model.provider` as the main model
configuration and supports profile-scoped settings. The authoritative settings
remain the live Hermes configs, not this dated snapshot. See the Hermes
[model configuration guide](https://hermes-agent.nousresearch.com/docs/user-guide/configuring-models)
and [configuration guide](https://hermes-agent.nousresearch.com/docs/user-guide/configuration/).

## History

| Revision | Date | Author | Change |
|---|---|---|---|
| 0.1 | 2026-10-28 | frick-fsw | Record live Hermes profile models, the Jeeves Ollama embedding setup, deployment/validation state, storage behavior, and safe CLI checks; omit credentials. |
| 0.2 | 2026-10-28 | frick-fsw | Recheck all profile model settings and memory status; correct plugin provenance/deployment claims and distinguish the active default profile from the separate Jeeves profile. |
| 0.3 | 2026-10-28 | frick-fsw | Retain the credential-free read-only profile/model/provider/plugin/memory CLI observation as linked evidence; no model or plugin configuration changed. |
