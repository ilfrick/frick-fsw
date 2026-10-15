#!/usr/bin/env python3
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0
"""Focused tests for the N13 license identity checker."""

import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "scripts"))
import license_check


class LicenseCheckTests(unittest.TestCase):
    def test_current_repository_license_policy(self):
        self.assertEqual(license_check.validate(), [])

    def test_non_code_artwork_remains_outside_mpl_code_header_scope(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / "LICENSE").write_bytes((license_check.ROOT / "LICENSE").read_bytes())
            (root / "assets").mkdir()
            (root / "assets" / "mark.svg").write_text(
                '<svg>\n<!-- SPDX-License-Identifier: AGPL-3.0-only -->\n</svg>\n'
            )
            errors = license_check.validate(root)
            self.assertEqual(errors, [])

    def test_unreadable_managed_source_fails_closed(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / "LICENSE").write_bytes((license_check.ROOT / "LICENSE").read_bytes())
            (root / "src").mkdir()
            (root / "src" / "bad.c").write_bytes(b"int bad;\xff\n")
            errors = license_check.validate(root)
            self.assertTrue(any(
                "src/bad.c: cannot read managed file for SPDX validation" in error
                for error in errors
            ))

    def test_svg_xml_declaration_precedes_valid_spdx_comment(self):
        header = '<?xml version="1.0" encoding="UTF-8"?>\n<!-- SPDX-License-Identifier: MPL-2.0 -->'
        self.assertEqual(
            license_check._header_spdx_expressions(header, Path("mark.svg")),
            ["MPL-2.0"],
        )

    def test_non_mpl_and_non_exact_spdx_expressions_are_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / "LICENSE").write_bytes((license_check.ROOT / "LICENSE").read_bytes())
            (root / "src").mkdir()
            marker = "SPDX-License-Identifier" + ": "
            for bad_expression in (
                "BSD-2-Clause",
                "MPL-2.0 OR GPL-3.0-only",
                "MPL-2.0 unexpected",
            ):
                with self.subTest(expression=bad_expression):
                    (root / "src" / "example.c").write_text(
                        "/* " + marker + bad_expression + " */\n"
                    )
                    errors = license_check.validate(root)
                    self.assertTrue(any("unexpected SPDX" in error for error in errors))
                    self.assertTrue(any("missing MPL-2.0 SPDX" in error for error in errors))

    def test_spdx_phrase_in_comment_prose_is_not_a_header(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / "LICENSE").write_bytes((license_check.ROOT / "LICENSE").read_bytes())
            (root / "src").mkdir()
            (root / "src" / "example.c").write_text(
                "// Documentation says SPDX-License-Identifier: MPL-2.0\n"
            )
            errors = license_check.validate(root)
            self.assertTrue(any("missing MPL-2.0 SPDX header" in error for error in errors))

    def test_spdx_declaration_after_code_is_not_a_header(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / "LICENSE").write_bytes((license_check.ROOT / "LICENSE").read_bytes())
            (root / "src").mkdir()
            (root / "src" / "example.c").write_text(
                "int main(void) { return 0; }\n/* SPDX-License-Identifier: MPL-2.0 */\n"
            )
            errors = license_check.validate(root)
            self.assertTrue(any("missing MPL-2.0 SPDX header" in error for error in errors))

    def test_c_preprocessor_directive_ends_header_region(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / "LICENSE").write_bytes((license_check.ROOT / "LICENSE").read_bytes())
            (root / "src").mkdir()
            (root / "src" / "example.c").write_text(
                "#include <stdint.h>\n/* SPDX-License-Identifier: MPL-2.0 */\nint value;\n"
            )
            errors = license_check.validate(root)
            self.assertTrue(any("missing MPL-2.0 SPDX header" in error for error in errors))

    def test_c_family_star_line_requires_open_block_comment(self):
        for filename in ("example.c", "example.h", "example.dts"):
            with self.subTest(filename=filename):
                text = " * SPDX-License-Identifier: MPL-2.0\nint value;\n"
                self.assertEqual(
                    license_check._header_spdx_expressions(text, Path(filename)), []
                )
                valid = "/* Copyright */\n/*\n * SPDX-License-Identifier: MPL-2.0\n */\n"
                self.assertEqual(
                    license_check._header_spdx_expressions(valid, Path(filename)),
                    ["MPL-2.0"],
                )

    def test_shebang_is_allowed_only_for_script_formats(self):
        for filename, expected in (("example.py", True), ("example.sh", True),
                                   ("example.c", False), ("example.svg", False)):
            with self.subTest(filename=filename):
                header = "#!/usr/bin/env tool\n# SPDX-License-Identifier: MPL-2.0"
                actual = license_check._header_spdx_expressions(header, Path(filename))
                self.assertEqual("MPL-2.0" in actual, expected)

    def test_supported_comment_header_forms_are_recognized_by_format(self):
        for filename, header in (
            ("example.c", "// SPDX-License-Identifier: MPL-2.0"),
            ("example.py", "# SPDX-License-Identifier: MPL-2.0"),
            ("example.c", "/*\n * SPDX-License-Identifier: MPL-2.0\n */"),
            ("example.svg", "<!-- SPDX-License-Identifier: MPL-2.0 -->"),
            ("Example.lean", "-- SPDX-License-Identifier: MPL-2.0"),
        ):
            with self.subTest(filename=filename, header=header):
                self.assertEqual(
                    license_check._header_spdx_expressions(header, Path(filename)),
                    ["MPL-2.0"],
                )

    def test_all_managed_format_comment_policies(self):
        cases = (
            ("example.c", "// SPDX-License-Identifier: MPL-2.0"),
            ("example.h", "/* SPDX-License-Identifier: MPL-2.0 */"),
            ("example.dts", "// SPDX-License-Identifier: MPL-2.0"),
            ("example.py", "# SPDX-License-Identifier: MPL-2.0"),
            ("example.sh", "# SPDX-License-Identifier: MPL-2.0"),
            ("example.cmake", "# SPDX-License-Identifier: MPL-2.0"),
            ("example.resc", "# SPDX-License-Identifier: MPL-2.0"),
            ("example.yml", "# SPDX-License-Identifier: MPL-2.0"),
            ("example.yaml", "# SPDX-License-Identifier: MPL-2.0"),
            ("example.lean", "-- SPDX-License-Identifier: MPL-2.0"),
            ("example.svg", "<!-- SPDX-License-Identifier: MPL-2.0 -->"),
            ("CMakeLists.txt", "# SPDX-License-Identifier: MPL-2.0"),
            ("Makefile", "# SPDX-License-Identifier: MPL-2.0"),
            ("Doxyfile", "# SPDX-License-Identifier: MPL-2.0"),
            ("Dockerfile", "# SPDX-License-Identifier: MPL-2.0"),
        )
        for filename, header in cases:
            with self.subTest(filename=filename):
                self.assertEqual(
                    license_check._header_spdx_expressions(header, Path(filename)),
                    ["MPL-2.0"],
                )

    def test_comment_syntax_must_match_file_format(self):
        for filename, header in (
            ("example.py", "/* SPDX-License-Identifier: MPL-2.0 */"),
            ("example.svg", "# SPDX-License-Identifier: MPL-2.0"),
            ("Example.lean", "# SPDX-License-Identifier: MPL-2.0"),
        ):
            with self.subTest(filename=filename, header=header):
                self.assertEqual(
                    license_check._header_spdx_expressions(header, Path(filename)), []
                )

    def test_mismatched_comment_terminators_are_not_declarations(self):
        for header in (
            "# SPDX-License-Identifier: MPL-2.0 */",
            "// SPDX-License-Identifier: MPL-2.0 -->",
        ):
            with self.subTest(header=header):
                self.assertNotEqual(license_check._spdx_expressions(header), ["MPL-2.0"])


if __name__ == "__main__":
    unittest.main()
