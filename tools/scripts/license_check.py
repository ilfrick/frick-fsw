#!/usr/bin/env python3
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0
"""Check the canonical MPL text and authored-file SPDX policy (norm N13)."""

from __future__ import annotations

import hashlib
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LICENSE_SHA256 = "1f256ecad192880510e84ad60474eab7589218784b9a50bc7ceee34c2b91f1d5"
CODE_SUFFIXES = {
    ".c", ".h", ".py", ".sh", ".lean", ".cmake", ".dts", ".resc",
    ".yml", ".yaml",
}
COMMENTED_BUILD_FILES = {"CMakeLists.txt", "Makefile", "Doxyfile", "Dockerfile"}
MANAGED_ROOTS = (
    "src", "config", "tests", "verification", "tools", "cmake", "board",
    ".github", ".gitea",
)
EXCLUDED_PARTS = {".git", "build", "third_party"}
HISTORICAL_REPORTS = Path("docs/reviews")
HISTORICAL_LICENSE_DECISIONS = {Path("docs/adr/ADR-0011-agpl-license.md")}
SPDX_DECLARATION = re.compile(
    r"^\s*(?:(?://|#|;|--)\s*SPDX-License-Identifier:\s*(.*?)\s*"
    r"|/\*\s*SPDX-License-Identifier:\s*(.*?)\s*\*/\s*"
    r"|\*\s*SPDX-License-Identifier:\s*(.*?)\s*"
    r"|<!--\s*SPDX-License-Identifier:\s*(.*?)\s*-->\s*)$"
)


def _spdx_expressions(text: str) -> list[str]:
    expressions: list[str] = []
    in_html_comment = False
    for line in text.splitlines():
        match = SPDX_DECLARATION.fullmatch(line)
        if match:
            expressions.append(next(group for group in match.groups() if group is not None).strip())
        elif in_html_comment:
            block_match = re.fullmatch(r"\s*SPDX-License-Identifier:\s*(.*?)\s*", line)
            if block_match:
                expressions.append(block_match.group(1).strip())
        if "<!--" in line and "-->" not in line:
            in_html_comment = True
        elif "-->" in line:
            in_html_comment = False
    return expressions


def _header_spdx_expressions(text: str, path: Path) -> list[str]:
    """Return declarations in the initial comment/blank/shebang header only."""
    if path.suffix in {".c", ".h", ".dts"}:
        comment_prefixes = ("//", "/*", "*")
    elif path.suffix == ".lean":
        comment_prefixes = ("--",)
    elif path.suffix == ".svg":
        comment_prefixes = ("<!--",)
    elif path.suffix in {".py", ".sh", ".cmake", ".yml", ".yaml", ".resc"} or path.name in {
        "CMakeLists.txt", "Makefile", "Doxyfile", "Dockerfile"
    }:
        comment_prefixes = ("#",)
    else:
        return []

    header_lines: list[str] = []
    c_family = path.suffix in {".c", ".h", ".dts"}
    in_block_comment = False
    in_html_comment = False
    for line in text.splitlines():
        stripped = line.lstrip()
        if not stripped:
            header_lines.append(line)
            continue
        if path.suffix == ".svg" and not header_lines and stripped.startswith("<?xml ") and stripped.endswith("?>"):
            header_lines.append(line)
            continue
        if path.suffix == ".svg":
            if in_html_comment:
                header_lines.append(line)
                if "-->" in stripped:
                    in_html_comment = False
                continue
            if stripped.startswith("<!--"):
                header_lines.append(line)
                in_html_comment = "-->" not in stripped
                continue
            break
        if c_family:
            if in_block_comment:
                header_lines.append(line)
                if "*/" in stripped:
                    in_block_comment = False
                continue
            if stripped.startswith("//"):
                header_lines.append(line)
                continue
            if stripped.startswith("/*"):
                header_lines.append(line)
                in_block_comment = "*/" not in stripped
                continue
            break
        if stripped.startswith("#!"):
            if path.suffix in {".py", ".sh"}:
                header_lines.append(line)
                continue
            break
        if stripped.startswith(comment_prefixes):
            header_lines.append(line)
            continue
        break
    return _spdx_expressions("\n".join(header_lines))


def validate(root: Path = ROOT) -> list[str]:
    errors: list[str] = []
    license_path = root / "LICENSE"
    if not license_path.is_file():
        return ["LICENSE is missing"]
    license_bytes = license_path.read_bytes()
    if hashlib.sha256(license_bytes).hexdigest() != LICENSE_SHA256:
        errors.append("LICENSE does not match the controlled Mozilla MPL-2.0 text")

    files = [
        p for p in root.rglob("*")
        if p.is_file() and not any(part in EXCLUDED_PARTS for part in p.relative_to(root).parts)
    ]
    for path in files:
        rel = path.relative_to(root)
        if (
            rel == Path("LICENSE")
            or rel.is_relative_to(HISTORICAL_REPORTS)
            or rel in HISTORICAL_LICENSE_DECISIONS
        ):
            continue
        try:
            text = path.read_text(encoding="utf-8")
        except (UnicodeDecodeError, OSError):
            continue
        ids = _spdx_expressions(text)
        is_managed_code = (
            bool(rel.parts)
            and rel.parts[0] in MANAGED_ROOTS
            and (path.suffix in CODE_SUFFIXES or path.name in COMMENTED_BUILD_FILES)
            and not rel.is_relative_to(Path("tools/third_party"))
        )
        if is_managed_code and ids and any(identifier != "MPL-2.0" for identifier in ids):
            errors.append(f"{rel}: unexpected SPDX license identifier(s): {', '.join(sorted(set(ids)))}")

    for path in files:
        rel = path.relative_to(root)
        if not rel.parts or rel.parts[0] not in MANAGED_ROOTS:
            continue
        if path.suffix not in CODE_SUFFIXES and path.name not in COMMENTED_BUILD_FILES:
            continue
        if rel.is_relative_to(Path("tools/third_party")):
            continue
        try:
            text = path.read_text(encoding="utf-8")
        except (UnicodeDecodeError, OSError):
            errors.append(f"{rel}: cannot read managed file for SPDX validation")
            continue
        if "MPL-2.0" not in _header_spdx_expressions(text, path):
            errors.append(f"{rel}: missing MPL-2.0 SPDX header")
    return errors


def main() -> int:
    errors = validate()
    if errors:
        print("MPL-2.0 license check FAILED:")
        print("\n".join(f"- {item}" for item in errors))
        return 1
    print("MPL-2.0 license identity and authored-file SPDX check: OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
