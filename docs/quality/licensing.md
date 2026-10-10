# Project Licensing (norm N13)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-LIC-001                          |
| Revision                          | 0.3                                    |
| Date                              | 2026-10-10                             |
| License                           | AGPL-3.0-only (GNU Affero GPL v3)      |
| Copyright                         | Copyright (C) 2026 Ilfrick — [GitHub](https://github.com/ilfrick/frick-fsw) · [Gitea](https://www.housefz.com/git/ilfrick/frick-fsw) |
| Norm                              | N13 (open source; license documented, reviewed and updated) |

## 1. Decision

The whole frick-fsw project is released under the **GNU Affero General Public
License version 3** (`AGPL-3.0-only`), **Copyright (C) 2026 Ilfrick**. The
canonical license text is the file `LICENSE` at the repository root (verbatim
text from the Free Software Foundation); the copyright holder and project links
are also recorded in `NOTICE`.

Every authored source, configuration and build file carries the header
(engineering documentation is covered by `LICENSE`/`NOTICE`):

```text
Copyright (C) 2026 Ilfrick
https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
SPDX-License-Identifier: AGPL-3.0-only
```

## 2. Why AGPL-3.0 (maximum permitted)

Norm N13 requires "AGPL-3.0 **or the maximum permitted by the licenses of any
reused code parts**". The analysed reused/derived material and the resulting
license compatibility:

| Component | Nature | License | Compatibility with AGPL-3.0 project |
| --------- | ------ | ------- | ----------------------------------- |
| frick-fsw authored code | original (this repo) | n/a | licensed AGPL-3.0-only by this document |
| NASA cFS (Draco) | architectural inspiration only; **no code copied** | Apache-2.0 | AGPL-3.0 is stricter; a project may aggregate Apache-2.0 components while remaining AGPL-3.0 (attribution preserved) |
| RTEMS 6.2 + RSB | external toolchain / OS | BSD-2-Clause (kernel), GPL-2.0 parts in RSB tooling | LGPL/BSD parts are combinable; RSB is a build tool, not distributed with the flight binary |
| PUSopen(R) | commercial library, **optional adapter only** (`FRICK_USE_PUSOPEN`), never distributed | commercial | not part of this repository's distribution; documented in `docs/interfaces/ICD.md` and `src/pus/adapters/pus_adapter_pusopen.c` |
| Lean 4 / Mathlib-theorems | development toolchain | Apache-2.0 | build-time tool, not distributed in binaries |
| Platform `libm` (glibc host / newlib RTEMS) | C standard math library, part of the toolchain already used; facade in `src/math/` (M6) | LGPL-2.1-or-later (glibc) / BSD-style (newlib) | dynamically linked toolchain library; no new component distributed by frick-fsw; combinable with AGPL-3.0 |

Conclusion: no reused part restricts frick-fsw below AGPL-3.0. Any new third-
party component added to this repository in the future must be AGPL-3.0
compatible or explicitly analysed here before being committed (norm N13).

## 3. PUSopen commercial adapter boundary

`src/pus/adapters/pus_adapter_pusopen.c` is an interface adapter against an
external commercial library. It is compiled only when `FF_USE_PUSOPEN` is set
and the library is provided by the mission; the adapter is distributed under
AGPL-3.0 (the adapter code itself), while the commercial library remains the
property of its vendor and is not part of this distribution. See
`docs/interfaces/ICD.md`.

## 4. Review procedure

Any change that affects licence-relevant material triggers a licence review in
the same commit (norm N8):

1. new dependency, reused code, curated third-party artefacts, or a change to
   this document or `LICENSE`;
2. run the licence check (CI `docs` job): `LICENSE` is the AGPL-3.0 text and
   no `Apache-2.0` SPDX headers remain anywhere in the repository;
3. record the outcome of the compatibility analysis in this document;
4. update `CITATION.cff` if the version/release changed.

## 5. SPDX expression

```text
Copyright (C) 2026 Ilfrick (https://github.com/ilfrick/frick-fsw, https://www.housefz.com/git/ilfrick/frick-fsw)
SPDX-License-Identifier: AGPL-3.0-only
```

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial licence decision (norm N13) |
| 0.2 | 2026-10-10 | frick-fsw | Add Ilfrick copyright + project links (header, `NOTICE`); norm N13/N16 |
| 0.3 | 2026-10-10 | frick-fsw | M6: analyse platform `libm` (glibc/newlib) compatibility; scope the SPDX statement to source/config/build files |