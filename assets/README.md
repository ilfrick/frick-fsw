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

The brand assets are part of frick-fsw and are released under
**AGPL-3.0-only** (norm N13), like the rest of the repository.

## History

| Rev | Date | Author | Reason |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-10 | frick-fsw | Initial marks for codename "Eärendil" (norm N8) |
