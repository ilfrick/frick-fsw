# Project Licensing (norm N13)

|                                   |                                        |
| --------------------------------: | :------------------------------------- |
| Project                          | frick-fsw                              |
| Document id                      | FRICK-LIC-001                          |
| Revision                          | 1.7                                    |
| Date                              | 2026-10-14                             |
| License                           | MPL-2.0 (Mozilla Public License 2.0)   |
| Copyright                         | Copyright (C) 2026 Ilfrick — [GitHub](https://github.com/ilfrick/frick-fsw) · [Gitea](https://www.housefz.com/git/ilfrick/frick-fsw) |
| Norm                              | N13 (open source; license documented, reviewed and updated) |

## 1. Requested project license and transition status

The repository owner directed and authorized the **Mozilla Public License,
version 2.0** (`MPL-2.0`) for original project-authored code and materials
created for frick-fsw; the canonical text is in root `LICENSE`. This record
captures the owner decision and does not make an independent legal or
provenance determination. **NC-EXT-43 remains OPEN** for third-party, generated,
mission-supplied, commercial, and other externally controlled components and
for combined/distribution-profile compatibility. Their terms are not changed by
the project owner's authorization. The CFF `date-released` value refers to the
historical v0.1.0 release and does not purport to describe a release under MPL.

SPDX headers on original project-authored source, configuration, build, test,
tool and authored asset source files in scope use
`SPDX-License-Identifier: MPL-2.0`. Documentation and formats that do not
support comments are covered by `LICENSE`/`NOTICE`. Generated outputs and
external components are outside the authorization and must retain or establish
their own terms; their distribution compatibility remains open. Historical
review reports retain point-in-time AGPL observations; prior recipients' grants
are not changed.

The text copied to `LICENSE` was retrieved from the Mozilla Foundation's
official MPL 2.0 page on 2026-10-13:
<https://www.mozilla.org/en-US/MPL/2.0/>. Retrieved plain-text source:
<https://www.mozilla.org/media/MPL/2.0/index.815ca599c9df.txt> (SHA-256
`fab3dd6bdab226f1c08630b1dd917e11fcb4ec5e1e020e2c16f83a0a13863e85`). The
repository copy strips trailing whitespace only; its SHA-256 is
`1f256ecad192880510e84ad60474eab7589218784b9a50bc7ceee34c2b91f1d5`.

## 2. Reused, generated, and external components — disposition pending

The previous AGPL compatibility analysis is not evidence that each dependency
or generated artifact may be distributed with MPL-2.0 files. The repository
records these boundaries, but exact versions, licensing terms, linkage,
generation provenance, and distribution profile require a controlled review.

| Component | Repository-described boundary | Recorded license | MPL transition disposition |
| --------- | ----------------------------- | --------------- | --------------------------- |
| NASA cFS (Draco) | Architectural inspiration; project claims no code copied | Apache-2.0 | Verify provenance claim; no cFS code is intentionally relicensed |
| RTEMS / RSB | External OS/toolchain and build tooling | BSD/GPL components reported | Not relicensed; verify distributed artifacts and notices |
| PUSopen | Optional mission-provided commercial library; adapter in repository | Commercial | Vendor library excluded from this project grant; verify boundary |
| Lean / Mathlib | Development tools/theorems | Apache-2.0 reported | Not included in flight binary by project description; verify generated material scope |
| Platform `libm` | Toolchain-provided host/RTEMS library | LGPL/BSD-style reported | Not relicensed; distribution/link mode not independently verified |
| lwIP | Pinned fetched stack, not committed per repository record | BSD-3-Clause reported | Preserve upstream terms; verify exact tag, notices, and integration/distribution |
| N7 Space / ESA `asn1-pusc-lib` | Fetched GPL-3.0 code; generated output described separately | GPL-3.0 reported | Compatibility and generated-code/runtime-exception applicability require legal review; do not infer from the prior AGPL analysis |
| N7 Space `n7s-obcp` / MicroPython | Pinned fetched OBCP candidate; source includes a pinned MicroPython submodule | MIT stated upstream for engine and MicroPython | Verify complete notices, provenance, modifications/patches, generated outputs, and combined-distribution compatibility before integration/release; NC-EXT-43 remains open |
| ASN1SCC generator/runtime | External generator and generated-code conditions described in ADR-0016 | GPL/runtime exception reported | Verify exact version and applicable generated-code license before distributing output |
| SDLS mission backend | Mission-supplied interface; project reference backend is local | Mission/HW-specific | Mission code is outside the project license unless separately contributed and authorized |

No compatibility or legal conclusion is made here for combined or distributed
artifacts. NC-EXT-43 remains open until exact inputs and deployment/distribution
profiles have an authorized disposition.

## 3. PUSopen commercial adapter boundary

`src/pus/adapters/pus_adapter_pusopen.c` is an interface adapter against an
external commercial library. It is compiled only when `FF_USE_PUSOPEN` is set
and the library is provided by the mission. Original project-authored adapter
source is within the owner's MPL-2.0 authorization. The commercial library
remains the property of its vendor, is excluded from that authorization, and
retains its own terms. Combined distribution compatibility remains under
NC-EXT-43. See
`docs/interfaces/ICD.md`.

## 4. Review procedure

Any change that affects licence-relevant material triggers a licence review in
the same commit (norm N8):

1. new dependency, reused code, curated third-party artefacts, or a change to
   this document or `LICENSE`;
2. run both CI license checks: canonical MPL-2.0 text and project SPDX coverage
   must pass; preserve historical reports and third-party license identifiers;
3. record the exact dependency/generated-code compatibility disposition in this
   document, with authority and evidence references;
4. update `CITATION.cff` if the version/release changed.

## 5. SPDX expression

```text
Copyright (C) 2026 Ilfrick (https://github.com/ilfrick/frick-fsw, https://www.housefz.com/git/ilfrick/frick-fsw)
SPDX-License-Identifier: MPL-2.0
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
| 0.7 | 2026-10-13 | frick-fsw | Record requested MPL-2.0 candidate transition, official license source, third-party review limits, and open NC-EXT-43; no release approval implied |
| 0.8 | 2026-10-13 | frick-fsw | Tighten N13 checker to require exact SPDX declaration in initial comment header; document NC-EXT-44 and pending independent qualification |
| 0.9 | 2026-10-13 | frick-fsw | Require format-specific comment syntax and script-only shebang handling in N13 header validation; T21 remains pending independent review |
| 1.0 | 2026-10-13 | frick-fsw | Require C-family opening block-comment state before accepting starred SPDX lines; independent T21 review remains pending |
| 1.1 | 2026-10-13 | frick-fsw | Explicitly prohibit an MPL grant in the candidate, qualify CFF release date, and remove present-distribution wording for the PUSopen adapter |
| 1.2 | 2026-10-14 | frick-fsw | Record owner authorization for original project-authored content; limit NC-EXT-43 to external/generated component boundaries and combined distribution compatibility |
| 1.3 | 2026-10-14 | frick-fsw | Align owner-decision date and clarify the PUSopen adapter authorization boundary |
| 1.4 | 2026-10-14 | frick-fsw | Temporarily narrowed owner authorization to software code and restored AGPL wording for docs/artwork; superseded by 1.5 after reconciliation to the owner's stated code-and-materials scope |
| 1.5 | 2026-10-14 | frick-fsw | Reconcile authorization to original project code and materials; exclude generated/external material and correct over-narrow code-only interpretation |
| 1.6 | 2026-10-14 | frick-fsw | Restore the omitted 1.4 history event as a superseded interim correction; make revision chronology continuous |
| 1.7 | 2026-10-16 | frick-fsw | Record pinned N7 OBCP/MicroPython candidate and defer combined-distribution/legal compatibility to independent/legal review |
