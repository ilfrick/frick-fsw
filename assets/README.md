# Project marks

Brand assets for **frick-fsw**, codename **"Eärendil"**.

## Meaning

**Eärendil** is the mariner who, in the legendarium that inspired the codename,
sails the night sky bearing a shining jewel on his brow and becomes the morning
star. The mark renders that idea as a white, stylized ship whose mast is crowned
by a sparkling star, riding gentle waves on a **midnight-blue** field.

- Field: midnight blue gradient `#12224f` → `#0a1434` → `#050a1c`.
- Figure: pure white `#ffffff` (the star is drawn as an eight-pointed sparkle).

## Files

| File | Type | Size | Use |
| ---- | ---- | ---- | --- |
| `logo.svg` | vector | 512 x 512 | square mark, app icon, social preview |
| `logo.png` | raster | 512 x 512 | square mark for consumers without SVG |
| `logo-wordmark.svg` | vector | 1200 x 320 | horizontal lockup (mark + name) |
| `logo-wordmark.png` | raster | 1200 x 320 | lockup for consumers without SVG |

The SVG files are the source of truth; the PNG files are generated from them
(`cairosvg logo.svg -o logo.png -W 512 -H 512`).

## Usage

- Keep adequate clear space around the mark; do not stretch or rotate it.
- Do not recolor the mark; it is monochrome white on midnight blue by design.
- Use the lockup (`logo-wordmark`) where the project name must appear, the mark
  (`logo`) where space is limited.

## License

The original project-authored SVG source artwork is within the owner's
**MPL-2.0** authorization for project code and materials. The PNG files are
generated outputs, outside that authorization; their provenance and applicable
terms must be dispositioned under NC-EXT-43 before distribution.

## History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial marks for codename "Eärendil" (norm N8) |
| 0.2 | 2026-10-13 | frick-fsw | Update asset license statement to MPL-2.0 candidate; NC-EXT-43 remains open |
| 0.3 | 2026-10-14 | frick-fsw | Restore prior AGPL scope for artwork after owner limited MPL authorization to original software code |
| 0.4 | 2026-10-14 | frick-fsw | Correct scope to owner-authorized original project materials; leave generated PNG outputs outside authorization |
