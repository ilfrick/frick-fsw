#!/usr/bin/env python3
# SPDX-License-Identifier: MPL-2.0
"""Review-stream closure semantics for the status-panel generator."""
# Copyright (C) 2026 Ilfrick

import importlib.util
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location(
    "frick_metrics", ROOT / "tools/scripts/metrics.py"
)
metrics = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(metrics)


class ReviewVerdictTests(unittest.TestCase):
    def test_scoped_pa_pass_does_not_close_overall_stream(self):
        self.assertFalse(metrics._review_verdict_closes(
            "PASS FOR NARROW LEDGER/HISTORY/STATUS CORRECTION ONLY; "
            "NO OVERALL N20 APPROVAL OR NC CLOSURE"
        ))

    def test_unscoped_pass_closes_review_stream(self):
        self.assertTrue(metrics._review_verdict_closes("PASS"))

    def test_not_approved_does_not_close_review_stream(self):
        self.assertFalse(metrics._review_verdict_closes("NOT APPROVED"))

    def test_partial_and_scope_only_passes_do_not_close(self):
        for verdict in ("PARTIAL PASS", "APPROVED FOR SCOPE ONLY",
                        "APPROVED FOR T13 DELTA ONLY",
                        "PASS FOR CORRECTION SCOPE ONLY",
                        "PASS FOR CORRECTION ONLY"):
            with self.subTest(verdict=verdict):
                self.assertFalse(metrics._review_verdict_closes(verdict))

    def test_negative_substrings_do_not_close(self):
        for verdict in ("DISAPPROVED", "UNAPPROVED", "NOT-APPROVED",
                        "BYPASS REQUIRED", "NO PASS",
                        "PASS (NO PASS of full scope)",
                        "APPROVED WITH COMMENTS (DISAPPROVED for overall closure)",
                        "PASS - FAILED overall review",
                        "APPROVED, DENIED for NC closure",
                        "APPROVED WITH COMMENTS (REJECTED for overall closure)"):
            with self.subTest(verdict=verdict):
                self.assertFalse(metrics._review_verdict_closes(verdict))

    def test_rejected_stream_closes_only_on_explicit_closed_disposition(self):
        self.assertTrue(metrics._review_verdict_closes("REJECTED -> CLOSED"))
        self.assertFalse(metrics._review_verdict_closes("REJECTED -> fixed"))

    def test_explicit_pending_or_open_actions_override_affirmative_prefix(self):
        for verdict in ("APPROVED WITH COMMENTS (delta review pending)",
                        "APPROVED WITH COMMENTS; NCs remain OPEN",
                        "PASS - NC closure pending",
                        "APPROVED WITH COMMENTS (findings remain UNRESOLVED)",
                        "APPROVED WITH COMMENTS (scope-limited to T13 delta)",
                        "APPROVED WITH COMMENTS (delta correction only)"):
            with self.subTest(verdict=verdict):
                self.assertFalse(metrics._review_verdict_closes(verdict))

    def test_approved_with_comments_without_open_actions_closes(self):
        self.assertTrue(metrics._review_verdict_closes(
            "APPROVED WITH COMMENTS (all findings corrected)"
        ))

    def test_latest_scoped_pa_pass_does_not_close_stream(self):
        contents = (
            "| 2026-10-12 | PA review | PA | APPROVED |\n"
            "| 2026-10-13 | PA follow-up | PA | PASS FOR NARROW SCOPE ONLY; "
            "NO OVERALL APPROVAL |\n"
        )
        with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8") as stream:
            stream.write(contents)
            stream.flush()
            self.assertEqual(metrics.review_streams(stream.name), (0, 1))


if __name__ == "__main__":
    unittest.main()
