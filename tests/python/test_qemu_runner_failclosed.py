#!/usr/bin/env python3
# Copyright (C) 2026 Ilfrick
# SPDX-License-Identifier: MPL-2.0

"""Real-path fail-closed fixture for tools/scripts/qemu_req_vv.sh (T2 §1, T3 M2).

These tests execute the **actual runner script**. Nothing is skipped: the test
image is synthesised in a temporary directory and supplied through the
documented, refused-by-default test-only override (`T1_ELF`), the QEMU command is
substituted with a stub, and the build is skipped - all of it gated by
`T1_ALLOW_QEMU_STUB=1`. Everything else (ELF identity check, nonce, run
directory, component verifier, whole-run verdict, exit-code logic) is the real
path.

T3-M2 corrected two defects of the previous version: the class used to be
`skipUnless(ELF)` (so with no target ELF it reported "Passed" while exercising
nothing, i.e. green and inert in CI), and it depended on an untracked retained
run directory. Both dependencies are gone: the fixture is hermetic (the image is synthesised in
a temporary directory, never the real build tree) and always exercises the
runner.

Cases:
  (a) clean nine-case console, stub exit 0        -> runner 0
  (b) the SAME clean console, stub exit 1         -> runner 1, QEMU-failure
      reason, no fatal marker present in the console (so the new control is
      what fails the run)
  (c) retained fatal console text, stub exit 0    -> runner 1
  (d) stub exit 124 (bounded timeout) + fatal     -> runner 1 (124 masks nothing)
  (e) missing ELF without the skip-build path     -> exit 2 (never 0)
  (f) missing toolchain image (SKIP_BUILD=0)      -> exit 2 (never 0)
  (g) overrides without the gate                  -> exit 2 REFUSED
  (h) T1_ELF override without the gate            -> exit 2 REFUSED

Offline; writes only into temporary directories.
"""

import json
import os
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RUNNER = ROOT / "tools" / "scripts" / "qemu_req_vv.sh"

# Retained 2026-10-26 transcript: the L9 gap wording predates the ICD rev 1.8
# correction and the verifier matches gap lines by requirement/case only.
CLEAN_CASES = """frick-fsw: 8 applications registered
TGTREQ: SRS-REQ-012:L1:PASS:per_frame_offsets_ok_S17.2_exact_negctl_3/3
TGTREQ-OBS: SRS-REQ-005:L2:S1.8_payload=01022A0109 (implementation-defined layout; NC-EXT-14/15)
TGTREQ: SRS-REQ-005:L2:PASS:one_S1.8_by_header_no_S17.2
TGTREQ: SRS-REQ-005:L3:PASS:one_S1.8_by_header_no_S17.2
TGTREQ: SRS-REQ-004:L4:PASS:parse_error_no_handler_no_S17.2
TGTREQ: SRS-REQ-005:L5:PASS:one_S1.2_by_header_no_handler
TGTREQ: SRS-REQ-003:L6:PASS:1024_echoed_1025_refused
TGTREQ: SRS-REQ-010:L7:PASS:CUC_set_value_at_coarse_resolution
TGTREQ: SRS-REQ-009:L8:PASS:dump_exact_statuses_distinct
TGTREQ: SRS-REQ-005:L10:PASS:chain_not_wedged
TGTREQ-GAP: SRS-REQ-004:L4-S18:NC-EXT-04: S1.8 for a malformed TC is not asserted
TGTREQ-GAP: SRS-REQ-005:L8-WIRE:NC-EXT-18: on-wire refusal failure code is interface-authority owned
TGTREQ-GAP: SRS-REQ-008:L9:NC-EXT-17: 6-byte TC[3,1] create is an unapproved extension (ICD defines 2 octets)
"""

# Verbatim from the retained T2 console (build/t1-run-20261026T055700Z/console.log).
FATAL_BLOCK = """watchdog: NOT WIRED on RTEMS (timeout 8000 ms) - NC-PAL-01
[FF-PANIC] watchdog: init failed

*** EXIT STATUS NOT ZERO ***
exit source: 5 (RTEMS_FATAL_SOURCE_EXIT)
exit code: 1 (0x00000001)
"""


class RunnerFailClosedTest(unittest.TestCase):
    """Hermetic: synthesises the image and never depends on build artifacts."""

    def run_runner(self, console_text=None, stub_exit=0, env_extra=None,
                   make_elf=True, skip_build=True, gate=True, docker_stub=None):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        tmpdir = Path(tmp.name)

        elf = tmpdir / "frick_fsw_reqtest"
        if make_elf:
            # A synthetic image: only its identity matters to the runner.
            elf.write_bytes(b"\x7fELF synthetic test image (T3 fixture)\n" * 4)

        stub = tmpdir / "stub-qemu.sh"
        if console_text is not None:
            console = tmpdir / "stub-console.txt"
            console.write_text(console_text, encoding="utf-8")
            stub.write_text(f"#!/usr/bin/env bash\ncat '{console}'\nexit {stub_exit}\n",
                            encoding="utf-8")
        else:
            stub.write_text(f"#!/usr/bin/env bash\nexit {stub_exit}\n", encoding="utf-8")
        stub.chmod(0o755)

        env = dict(os.environ)
        for name in ("T1_ALLOW_QEMU_STUB", "T1_QEMU_CMD", "T1_RUN_ROOT",
                     "T1_SKIP_BUILD", "T1_ELF"):
            env.pop(name, None)
        if gate:
            env["T1_ALLOW_QEMU_STUB"] = "1"
        env["T1_QEMU_CMD"] = f"bash '{stub}'"
        env["T1_RUN_ROOT"] = str(tmpdir / "runs")
        env["T1_ELF"] = str(elf)
        env["T1_SKIP_BUILD"] = "1" if skip_build else "0"
        if docker_stub is not None:
            bin_dir = tmpdir / "bin"
            bin_dir.mkdir()
            fake = bin_dir / "docker"
            fake.write_text(f"#!/usr/bin/env bash\n{docker_stub}\n", encoding="utf-8")
            fake.chmod(0o755)
            env["PATH"] = f"{bin_dir}:{env['PATH']}"
        if env_extra:
            env.update(env_extra)

        proc = subprocess.run(["bash", str(RUNNER)], capture_output=True, text=True,
                              timeout=180, env=env, cwd=str(ROOT))
        runs = sorted((tmpdir / "runs").glob("t1-run-*"))
        verdict = None
        verdict_path = runs[-1] / "verdict.json" if runs else None
        if verdict_path and verdict_path.exists():
            verdict = json.loads(verdict_path.read_text(encoding="utf-8"))
        return proc, verdict

    # ---- (a)-(d) console/exit semantics --------------------------------
    def test_clean_console_and_qemu_exit_0_passes(self):
        proc, verdict = self.run_runner(CLEAN_CASES, 0)
        self.assertEqual(proc.returncode, 0, msg=proc.stdout + proc.stderr)
        self.assertIn("COMPONENT: PASS 9/9", proc.stdout)
        self.assertIn("WHOLE-RUN: PASS", proc.stdout)
        self.assertEqual(verdict["result"], "PASS")
        self.assertEqual(verdict["qemu_exit"], 0)
        self.assertFalse(verdict["qemu_failed"])

    def test_same_clean_console_with_qemu_exit_1_fails(self):
        """The new control: a QEMU crash with a clean console must fail the run."""
        proc, verdict = self.run_runner(CLEAN_CASES, 1)
        self.assertEqual(proc.returncode, 1, msg=proc.stdout + proc.stderr)
        self.assertIn("QEMU PROCESS FAILURE: exit 1", proc.stderr)
        self.assertIn("QEMU-PROCESS: exit 1 (FAIL (QEMU process failure))", proc.stdout)
        self.assertIn("WHOLE-RUN: PASS", proc.stdout)      # console alone would pass
        self.assertIn("COMPONENT: PASS 9/9", proc.stdout)
        self.assertEqual(verdict["result"], "FAIL")
        self.assertTrue(verdict["qemu_failed"])

    def test_retained_fatal_console_with_qemu_exit_0_fails(self):
        proc, verdict = self.run_runner(CLEAN_CASES + FATAL_BLOCK, 0)
        self.assertEqual(proc.returncode, 1, msg=proc.stdout + proc.stderr)
        self.assertIn("WHOLE-RUN: FAIL (watchdog fatal)", proc.stdout)
        self.assertFalse(verdict["whole_run"]["ok"])
        self.assertFalse(verdict["qemu_failed"])

    def test_general_rtems_fatal_does_not_mask_a_pass(self):
        """R2-01: `*** FATAL ***` (non-EXIT source) must fail the T1 run too."""
        console = CLEAN_CASES + "*** FATAL ***\nfatal source: 3 (RTEMS_FATAL_SOURCE_EXCEPTION)\n"
        for exit_code in (0, 124):
            proc, verdict = self.run_runner(console, exit_code)
            self.assertEqual(proc.returncode, 1,
                             msg=f"exit {exit_code}: " + proc.stdout + proc.stderr)
            self.assertIn("WHOLE-RUN: FAIL", proc.stdout)
            self.assertEqual(verdict["result"], "FAIL")

    def test_bounded_timeout_124_does_not_mask_a_fatal(self):
        proc, verdict = self.run_runner(CLEAN_CASES + FATAL_BLOCK, 124)
        self.assertEqual(proc.returncode, 1, msg=proc.stdout + proc.stderr)
        self.assertIn("QEMU-PROCESS: exit 124 (accepted)", proc.stdout)
        self.assertIn("WHOLE-RUN: FAIL (watchdog fatal)", proc.stdout)
        self.assertEqual(verdict["result"], "FAIL")

    # ---- (e)-(h) prerequisites and the refusal gate --------------------
    def test_missing_elf_with_the_skip_build_path_exits_2(self):
        """Hermetic: the ELF check fails without touching the real build tree.

        The previous version used skip_build=False here, which made the runner
        perform a real docker cross build into ${ROOT}/build/t1-qemu (T3-M1).
        With the skip-build path the ELF branch is exercised directly and no
        build runs.
        """
        proc, _ = self.run_runner(CLEAN_CASES, 0, make_elf=False, skip_build=True)
        self.assertEqual(proc.returncode, 2, msg=proc.stdout + proc.stderr)
        self.assertIn("MISSING PREREQUISITE", proc.stderr)

    def test_missing_image_exits_2_without_building(self):
        """The image-missing branch is exercised with a docker stub, not a build."""
        proc, _ = self.run_runner(CLEAN_CASES, 0, skip_build=False, docker_stub="exit 1")
        self.assertEqual(proc.returncode, 2, msg=proc.stdout + proc.stderr)
        self.assertIn("MISSING PREREQUISITE", proc.stderr)

    def test_overrides_are_refused_without_the_gate(self):
        proc, _ = self.run_runner(CLEAN_CASES, 0, gate=False)
        self.assertEqual(proc.returncode, 2, msg=proc.stdout + proc.stderr)
        self.assertIn("REFUSED", proc.stderr)

    def test_elf_override_alone_is_refused_without_the_gate(self):
        env = dict(os.environ)
        for name in ("T1_ALLOW_QEMU_STUB", "T1_QEMU_CMD", "T1_RUN_ROOT", "T1_SKIP_BUILD"):
            env.pop(name, None)
        env["T1_ELF"] = "/tmp/not-an-image"
        proc = subprocess.run(["bash", str(RUNNER)], capture_output=True, text=True,
                              timeout=60, env=env, cwd=str(ROOT))
        self.assertEqual(proc.returncode, 2, msg=proc.stdout + proc.stderr)
        self.assertIn("REFUSED", proc.stderr)

if __name__ == "__main__":
    unittest.main(verbosity=2)
