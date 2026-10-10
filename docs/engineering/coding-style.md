# C Coding Style

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-ENG-0006                         |
| Revision                          | 0.1                                    |
| Date                              | 2026-10-10                             |
| Norms                             | N2, N8, N10                            |
| Requirement                       | SRS-REQ-042                            |

## 1. Declared style

frick-fsw C sources follow the **frick-fsw C style**, a NASA cFS-inspired,
MISRA C:2012-friendly style. The **normative definition is `.clang-format`** at
the repository root; this document describes it and the surrounding conventions.
It applies to the **whole authored codebase** (`src/`, `config/`, `tests/`);
third-party sources (fetched under `third_party/`) are excluded.

| Aspect | Rule |
| ------ | ---- |
| Base | LLVM, adapted to the cFS house style |
| Indentation | 4 spaces, no tabs |
| Braces | function braces on their own line; control-statement braces on the same line; `else` on a new line; struct braces on their own line; enum braces on the same line (`AfterEnum: false`) |
| Line length | 100 columns |
| Pointers | right-aligned (`char *p`) |
| Short functions/if | never on a single line |
| Includes | not re-sorted (explicit order) |

## 2. Language and API conventions

- **C99**, `-Wall -Wextra` clean (CI also runs clang-tidy and MISRA).
- **Fixed-width integer types** (`uint8_t`, `uint16_t`, `uint32_t`, `uint64_t`);
  `int`/`unsigned` only for non-persistent locals.
- **Prefixes**: public API `FF_`, types `FF_*_t`, macros `FF_*`; module-static
  helpers `ff_*`. Public functions are declared in the matching header.
- **Error handling**: fallible functions return `FF_Status_t`; callers check it.
- **No dynamic allocation** after init; no VLAs; bounded, static storage
  (SSS-REQ-010). Queues/pools are fixed size.
- **`const`-correctness** at every pointer; parameters that are read-only are
  `const`.
- **Headers**: SPDX header + `Copyright (C) 2026 Ilfrick`; include guard
  `FF_<MODULE>_H`; every `.c` documents its purpose.
- **Comments**: English (N2); explain *why*, keep them current (N8).

## 3. Enforcement

```bash
make format              # clang-format -i over the whole codebase (apply)
make misra               # cppcheck MISRA scan (advisory) — docs/quality/misra-c.md
```

CI (both forges) runs the **format gate** (`clang-format --dry-run --Werror`
over `src config tests`) and the MISRA scan; a non-formatted file fails the
build. The style is therefore applied and enforced codebase-wide.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Declare the frick-fsw C style (cFS-inspired, MISRA-friendly); apply codebase-wide via clang-format |
