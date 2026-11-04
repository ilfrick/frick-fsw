#!/usr/bin/env python3
# Copyright (C) 2026 Ilfrick
# SPDX-License-Identifier: MPL-2.0

"""Real-path negative controls for tools/scripts/renode_vv_verdict.sh (R3-04).

Executes the ACTUAL verdict script (POSIX sh, no emulator, no container, no
network) against synthetic run directories. The defect being guarded: the script
used to grant PASS on the aggregate `RENODE_TEST: PASS` marker *before* scanning
for fatal markers, so a console that says "suite passed" and then dies was
accepted.

Cases (each asserts the real exit AND the failure reason):
  A  fatal block BEFORE the aggregate marker          -> nonzero, fatal reason
  B  fatal block AFTER the aggregate marker           -> nonzero (the R3-04 case)
  C  aggregate marker absent, no fatal                -> nonzero
  D  malformed/truncated evidence                     -> nonzero
  E  true success console (marker + per-check, clean) -> 0   (discriminating)
  F  absent / unreadable / empty capture              -> nonzero
  G  fallback marker list parity with the shared scan -> the two sets match

The fatal strings are the shared seven-marker set; the parity test (G) proves
the shell fallback cannot silently diverge from `target_run_verdict.FATAL_MARKERS`.
"""

import os
import stat
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
VERDICT = ROOT / "tools" / "scripts" / "renode_vv_verdict.sh"

MARKER = "RENODE_TEST: PASS"

# A credible clean console: boot banner, per-check evidence, aggregate marker.
GOOD_CONSOLE = """\
*** Booting RTEMS ... ***
frick-fsw: 8 applications registered
hal_gpio=PASS hal_serial=PASS hal_mem=PASS
app CMD_INGEST started
RENODE_TEST: PASS
"""

FATAL_BLOCK = """\
watchdog: NOT WIRED on RTEMS (timeout 8000 ms) - NC-PAL-01
[FF-PANIC] watchdog: init failed
*** EXIT STATUS NOT ZERO ***
exit source: 5 (RTEMS_FATAL_SOURCE_EXIT)
"""

FATAL_AFTER = """\
[FF-PANIC] watchdog: init failed
*** FATAL ***
fatal source: 2 (RTEMS_FATAL_SOURCE_ASSERT)
"""


class RenodeVerdictFailClosedTest(unittest.TestCase):
    def make_run(self, console, process_log="renode process log: running\n",
                 status=None, console_name="uart-console.log", make_console=True,
                 console_mode=None):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        run = Path(tmp.name) / "run"
        run.mkdir()
        (run / "renode-process.log").write_text(process_log, encoding="utf-8")
        if make_console:
            path = run / console_name
            path.write_text(console, encoding="utf-8")
            if console_mode is not None:
                os.chmod(path, console_mode)
        if status is not None:
            (run / "status.env").write_text(status, encoding="utf-8")
        return run

    def verdict(self, run):
        return subprocess.run(["sh", str(VERDICT), str(run)], capture_output=True,
                              text=True, timeout=60)

    def assert_fail(self, proc, reason_token):
        self.assertNotEqual(proc.returncode, 0,
                            msg=f"expected nonzero, got {proc.returncode}: {proc.stdout}")
        self.assertIn(reason_token, proc.stdout, msg=proc.stdout + proc.stderr)

    # ---- A/B: the R3-04 defect -----------------------------------------
    def test_fatal_before_marker_fails(self):
        """The image died before the suite: no marker, fatal present."""
        proc = self.verdict(self.make_run(FATAL_BLOCK + "boot ok\nhal_gpio=PASS\n"))
        self.assert_fail(proc, "startup fatal exit")

    def test_fatal_before_marker_with_marker_also_fails(self):
        """Fatal text preceding the marker line is still a FAIL."""
        proc = self.verdict(self.make_run(FATAL_BLOCK + GOOD_CONSOLE))
        self.assert_fail(proc, "may not override a dead run")

    def test_fatal_after_marker_fails(self):
        """R3-04: a clean PASS line followed by a death must not be a PASS."""
        proc = self.verdict(self.make_run(GOOD_CONSOLE + FATAL_AFTER))
        self.assert_fail(proc, "may not override a dead run")

    def test_fatal_after_marker_in_the_other_capture_fails(self):
        """The scan covers both captures, not only the console."""
        proc = self.verdict(self.make_run(GOOD_CONSOLE, process_log=FATAL_AFTER))
        self.assert_fail(proc, "may not override a dead run")

    def test_each_shared_marker_alone_fails(self):
        for marker in ("[FF-PANIC]", "RTEMS_FATAL_SOURCE_EXIT", "*** EXIT STATUS NOT ZERO ***",
                       "*** FATAL ***", "fatal source:", "rtems_fatal", "Fatal error"):
            proc = self.verdict(self.make_run(GOOD_CONSOLE + marker + "\n"))
            self.assertNotEqual(proc.returncode, 0,
                                msg=f"marker {marker!r} did not fail: {proc.stdout}")

    # ---- C/D: missing and malformed evidence ---------------------------
    def test_marker_absent_fails(self):
        proc = self.verdict(self.make_run("boot ok\nhal_gpio=PASS\n"))
        self.assert_fail(proc, "marker absent")

    def test_truncated_marker_fails(self):
        proc = self.verdict(self.make_run("boot ok\nRENODE_TEST: PAS\n"))
        self.assert_fail(proc, "marker absent")

    def test_empty_console_fails(self):
        proc = self.verdict(self.make_run(""))
        self.assert_fail(proc, "empty")

    # ---- E: the control is discriminating ------------------------------
    def test_true_success_passes(self):
        proc = self.verdict(self.make_run(GOOD_CONSOLE))
        self.assertEqual(proc.returncode, 0, msg=proc.stdout + proc.stderr)
        self.assertIn("PASS", proc.stdout)
        self.assertIn("no fatal marker", proc.stdout)

    def test_success_with_a_benign_use_of_the_word_fatal_passes(self):
        """The scan must not fire on ordinary text (no false FAIL direction)."""
        console = GOOD_CONSOLE + "link statistics: fatal=0 recoverable=3\n"
        proc = self.verdict(self.make_run(console))
        self.assertEqual(proc.returncode, 0, msg=proc.stdout)

    # ---- F: capture integrity ------------------------------------------
    def test_missing_console_fails(self):
        proc = self.verdict(self.make_run(GOOD_CONSOLE, make_console=False))
        self.assert_fail(proc, "capture incomplete")

    def test_missing_process_log_fails(self):
        run = self.make_run(GOOD_CONSOLE)
        (run / "renode-process.log").unlink()
        proc = self.verdict(run)
        self.assert_fail(proc, "capture incomplete")

    def test_unreadable_console_fails(self):
        run = self.make_run(GOOD_CONSOLE, console_mode=0)
        proc = self.verdict(run)
        if proc.returncode == 0:  # running as root: permissions cannot bite
            self.skipTest("running as root; unreadable-file control is inert")
        self.assert_fail(proc, "unreadable")

    def test_stale_run_directory_from_another_run_is_not_consulted(self):
        """Only the given run directory is read (NC-EXT-199 scoping)."""
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        root = Path(tmp.name)
        good = root / "good-run"
        good.mkdir()
        (good / "renode-process.log").write_text("proc\n", encoding="utf-8")
        (good / "uart-console.log").write_text(GOOD_CONSOLE, encoding="utf-8")
        empty = root / "new-run"
        empty.mkdir()
        (empty / "renode-process.log").write_text("proc\n", encoding="utf-8")
        (empty / "uart-console.log").write_text("boot only\n", encoding="utf-8")
        proc = self.verdict(empty)
        self.assert_fail(proc, "marker absent")

    # ---- F1/F2: interpreter failure and ANSI in the fallback ------------
    def _run_with_path(self, run, path):
        env = dict(os.environ)
        env["PATH"] = str(path)
        env["PYTHONDONTWRITEBYTECODE"] = "1"
        return subprocess.run(["/bin/sh", str(VERDICT), str(run)], capture_output=True,
                              text=True, timeout=60, env=env)

    def test_failing_interpreter_does_not_restore_a_pass(self):
        """R3-04-F1: a broken python3 must not be read as 'no fatal marker'."""
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        fake = Path(tmp.name) / "bin"
        fake.mkdir()
        shim = fake / "python3"
        shim.write_text("#!/bin/sh\necho 'python3: interpreter unavailable' >&2\nexit 1\n",
                        encoding="utf-8")
        shim.chmod(0o755)
        for name in ("sed", "grep", "cut", "tail", "printf", "dirname", "cat", "tr", "sh"):
            for base in ("/usr/bin", "/bin"):
                if (Path(base) / name).exists():
                    (fake / name).symlink_to(Path(base) / name)
                    break
        run = self.make_run(GOOD_CONSOLE + FATAL_AFTER)
        proc = self._run_with_path(run, fake)
        self.assertNotEqual(proc.returncode, 0,
                            msg="a failing interpreter must not pass: " + proc.stdout)
        self.assertIn("may not override a dead run", proc.stdout)

    def test_silent_zero_interpreter_does_not_restore_a_pass(self):
        """A python3 that exits 0 with no output is also not evidence of clean."""
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        fake = Path(tmp.name) / "bin"
        fake.mkdir()
        shim = fake / "python3"
        shim.write_text("#!/bin/sh\nexit 0\n", encoding="utf-8")
        shim.chmod(0o755)
        for name in ("sed", "grep", "cut", "tail", "printf", "dirname", "cat", "tr", "sh"):
            for base in ("/usr/bin", "/bin"):
                if (Path(base) / name).exists():
                    (fake / name).symlink_to(Path(base) / name)
                    break
        run = self.make_run(GOOD_CONSOLE + FATAL_AFTER)
        proc = self._run_with_path(run, fake)
        self.assertNotEqual(proc.returncode, 0, msg=proc.stdout)

    def test_ansi_wrapped_marker_is_caught_by_the_fallback(self):
        """R3-04-F2: the literal fallback strips ANSI before matching."""
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        fake = Path(tmp.name) / "bin"
        fake.mkdir()
        for name in ("sed", "grep", "cut", "tail", "printf", "dirname", "cat", "tr", "sh"):
            for base in ("/usr/bin", "/bin"):
                if (Path(base) / name).exists():
                    (fake / name).symlink_to(Path(base) / name)
                    break
        # No python3 in this PATH at all -> literal fallback.
        console = GOOD_CONSOLE + "\x1b[31m*** FATAL ***\x1b[0m\n"
        run = self.make_run(console)
        proc = self._run_with_path(run, fake)
        self.assertNotEqual(proc.returncode, 0,
                            msg="ANSI-wrapped fatal must fail in the fallback: " + proc.stdout)
        self.assertIn("may not override a dead run", proc.stdout)

    # ---- F8/F9/F10: fallback tooling, control bytes, cwd independence ---
    def _sandbox(self, tools):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        fake = Path(tmp.name) / "bin"
        fake.mkdir()
        for name in tools:
            for base in ("/usr/bin", "/bin"):
                if (Path(base) / name).exists():
                    (fake / name).symlink_to(Path(base) / name)
                    break
        return fake

    def test_missing_sanitiser_fails_closed(self):
        """R3-04-F8: if the fallback cannot sanitise, it must not report PASS."""
        fake = self._sandbox(("grep", "cut", "tail", "printf", "dirname", "cat", "sh"))
        run = self.make_run(GOOD_CONSOLE + FATAL_AFTER)
        proc = self._run_with_path(run, fake)
        self.assertNotEqual(proc.returncode, 0, msg=proc.stdout)
        self.assertIn("fatal scan unavailable", proc.stdout)

    def test_failing_sed_fails_closed(self):
        """R3-04-F8: a sed that errors must not be read as a clean capture."""
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        fake = Path(tmp.name) / "bin"
        fake.mkdir()
        for name in ("grep", "cut", "tail", "printf", "dirname", "cat", "tr", "sh"):
            for base in ("/usr/bin", "/bin"):
                if (Path(base) / name).exists():
                    (fake / name).symlink_to(Path(base) / name)
                    break
        shim = fake / "sed"
        shim.write_text("#!/bin/sh\nexit 1\n", encoding="utf-8")
        shim.chmod(0o755)
        run = self.make_run(GOOD_CONSOLE + FATAL_AFTER)
        proc = self._run_with_path(run, fake)
        self.assertNotEqual(proc.returncode, 0, msg=proc.stdout)
        self.assertIn("capture sanitising error", proc.stdout)

    def test_control_byte_inside_marker_is_caught(self):
        """R3-04-F9: control bytes are stripped like the shared scanner does."""
        console = GOOD_CONSOLE + "*** FATAL\x07 ***\n"
        proc = self.verdict(self.make_run(console))
        self.assertNotEqual(proc.returncode, 0, msg=proc.stdout)
        self.assertIn("may not override a dead run", proc.stdout)

    def test_verdict_is_independent_of_the_working_directory(self):
        """R3-04-F10: a file named 'C' or 'P' in the cwd must not become a marker."""
        run = self.make_run(GOOD_CONSOLE)
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        cwd = Path(tmp.name)
        for name in ("C", "P", "F"):
            (cwd / name).write_text("x", encoding="utf-8")
        fake = self._sandbox(("grep", "cut", "tail", "printf", "dirname", "cat", "tr",
                              "sed", "sh"))  # no python3 -> fallback loop
        env = dict(os.environ)
        env["PATH"] = str(fake)
        env["PYTHONDONTWRITEBYTECODE"] = "1"
        proc = subprocess.run(["/bin/sh", str(VERDICT), str(run)], capture_output=True,
                              text=True, timeout=60, env=env, cwd=str(cwd))
        self.assertEqual(proc.returncode, 0, msg=proc.stdout)

    # ---- G: the fallback list cannot diverge ---------------------------
    def test_shell_fallback_matches_the_shared_marker_set(self):
        sys.path.insert(0, str(ROOT / "tools" / "scripts"))
        import target_run_verdict as trv  # noqa: E402

        script = VERDICT.read_text(encoding="utf-8")
        line = next(l for l in script.splitlines() if l.startswith("FATAL_PATTERNS="))
        shell_patterns = line.split("=", 1)[1].strip().strip("'\"").split("|")
        self.assertEqual(sorted(shell_patterns), sorted(trv.FATAL_MARKERS),
                         "the sh fallback list diverged from target_run_verdict.FATAL_MARKERS")


if __name__ == "__main__":
    unittest.main(verbosity=2)
