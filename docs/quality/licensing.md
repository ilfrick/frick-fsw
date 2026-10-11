# Project Licensing (norm N13)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-LIC-001                          |
| Revision                          | 0.6                                    |
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
| SDLS cryptographic backend | user/mission-supplied algorithm (CCSDS 352.0-B) via `FF_CryptoBackend_t`; **not distributed** (M7) | mission/HW, typically proprietary | integration point only; the non-secure reference backend `src/security/ff_crypto_test.c` is AGPL and test-only |
| lwIP (M4) | reference TCP/IP stack behind `FF_NetStack_t`; fetched pinned, **not committed** | BSD-3-Clause | compatible with AGPL-3.0 (aggregate, attribution) |
| N7 Space / ESA `asn1-pusc-lib` (M4) | PUS-C ASN.1/ACN types behind `FF_Pus_Backend_t`; fetched pinned, **not committed** | GPL-3.0 | compatible with AGPL-3.0 (GPL-3.0 combination allowed) |

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

## 6. Project name / codename "Eärendil" (third-party names)

The project name is **frick-fsw**; the internal **codename** is "**Eärendil**"
(`FF_CODENAME`), shown in the banner and in `assets/`. "Eärendil" is a name from
J. R. R. Tolkien's legendarium.

- **Copyright**: a single name/title is **not** protected by copyright
  (copyright covers the expression of works, not names); no copyright permission
  is required to use the name as a codename, and no Tolkien text is included.
- **Trademark**: the relevant risk is **trademark** (use in commerce that is
  likely to cause confusion or imply affiliation). "TOLKIEN" and "J. R. R.
  TOLKIEN" are registered trademarks of **The Tolkien Estate**;
  "THE LORD OF THE RINGS", "THE HOBBIT", "MIDDLE-EARTH" and many related
  names/characters are trademarks / licensed marks controlled by
  **Middle-earth Enterprises** (a subsidiary of the Embracer Group; formerly
  The Saul Zaentz Company / Tolkien Enterprises). Whether "Eärendil"
  *specifically* is a registered trademark depends on jurisdiction/class and must
  be verified per territory (e.g. EUIPO, USPTO, WIPO) for the relevant goods and
  services (e.g. Nice class 9, software).
- **Position of this project**: the codename is used only as an internal/artistic
  identifier; the project is **not affiliated with, endorsed by, or associated
  with** the Tolkien Estate or Middle-earth Enterprises, claims no rights over
  the name, and does not use it as a trademark to designate the software or its
  origin. The disclaimer is reproduced in `NOTICE` and `README.md`.
- **If commercial use of the name is intended** (e.g. branding a product,
  distribution under the name, merchandise): verify the trademark
  registrations first and, if needed, seek a licence —
  **Middle-earth Enterprises** (licensing: <https://www.middleearth.com/licensing>)
  and **The Tolkien Estate** (<https://www.tolkienestate.com>). Alternatively,
  rename the product to avoid third-party marks entirely (the software name
  "frick-fsw" is unaffected and is the recommended product identifier).
- **This is not legal advice**; obtain qualified IP counsel before any
  commercial use of third-party names.

## History

| Rev | Date       | Author    | Reason |
| --- | ---------- | --------- | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial licence decision (norm N13) |
| 0.2 | 2026-10-10 | frick-fsw | Add Ilfrick copyright + project links (header, `NOTICE`); norm N13/N16 |
| 0.3 | 2026-10-10 | frick-fsw | M6: analyse platform `libm` (glibc/newlib) compatibility; scope the SPDX statement to source/config/build files |
| 0.4 | 2026-10-10 | frick-fsw | M7: SDLS cryptographic backend is mission-supplied and not distributed; reference backend is test-only AGPL |
| 0.5 | 2026-10-10 | frick-fsw | M4: lwIP (BSD-3) and N7/ESA asn1-pusc-lib (GPL-3.0) analysed; fetched pinned, not committed |
| 0.6 | 2026-10-10 | frick-fsw | §6: project codename "Eärendil" — copyright vs trademark, non-affiliation, owners/contacts, per-territory verification |
