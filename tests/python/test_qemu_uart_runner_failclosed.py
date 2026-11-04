#!/usr/bin/env python3
# Copyright (C) 2026 Ilfrick
# SPDX-License-Identifier: MPL-2.0

"""Real-path fail-closed fixture for tools/scripts/qemu_uart_tc_vv.sh (T3 M1).

Executes the **actual UART runner** with its documented, refused-by-default stub
mechanism (`T2_ALLOW_QEMU_STUB=1` gates `T2_QEMU_CMD`, `T2_RUN_ROOT`,
`T2_SKIP_PREREQ`). The telemetry frames below are independently specified golden
bytes built by this file's own encoder (never captured from the target), so the
U1 set is satisfied by construction in the positive case.

The defect this fixture guards (REVIEW-T2 T2-M1): the runner recorded the QEMU
exit code but gated only on the verifier, so a crashed QEMU with a valid capture
reported PASS. Cases:

  (a) valid U1 capture, stub exit 0  -> script 0
  (b) the same capture, stub exit 1  -> script 1, distinct QEMU PROCESS FAILURE
  (c) empty capture, stub exit 0     -> script 1, no_tm_received
  (d) overrides without the gate     -> script 2, REFUSED
  (e) 124 (bounded timeout) + valid  -> script 0 (accepted code)
  (f) real-path blocked outcome text -> the failure reason names the watchdog abort
  (g) a full U1 capture *plus* the retained panic/fatal block -> script 1, i.e. a
      component U1 result can never override a dead run (N10-M2 negative control)

Hermetic: the image is synthesised in a temporary directory and supplied through
the gated `T2_ELF` override, so no repository build artefact (and no docker
image) is required; writes only into temporary directories.
"""

import os
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RUNNER = ROOT / "tools" / "scripts" / "qemu_uart_tc_vv.sh"
TM_APID = 0x020


def crc16(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


def tm_frame(service: int, subservice: int, counter: int, payload: bytes) -> bytes:
    pdl = 13 + len(payload) + 2 - 1
    w0 = (0 << 13) | (0 << 12) | (1 << 11) | TM_APID
    frame = bytearray()
    frame += w0.to_bytes(2, "big")
    frame += ((3 << 14) | 0).to_bytes(2, "big")
    frame += pdl.to_bytes(2, "big")
    frame += bytes([(2 << 4) | 0, service, subservice])
    frame += counter.to_bytes(2, "big")
    frame += (1).to_bytes(2, "big")
    frame += (1000).to_bytes(4, "big")
    frame += (0).to_bytes(2, "big")
    frame += payload
    frame += crc16(bytes(frame)).to_bytes(2, "big")
    return bytes(frame)


def u1_capture() -> bytes:
    out = bytearray(b"\r\n+--- banner ---+\r\n")
    counter = 40
    for service, subservice in ((1, 1), (1, 3), (17, 2), (1, 7)):
        payload = b"\x01\x02\x03\x04" if (service, subservice) == (17, 2) else b"\x01\x02\x01\x01\x00"
        out += tm_frame(service, subservice, counter, payload)
        counter += 1
    return bytes(out)


class UartRunnerFailClosedTest(unittest.TestCase):
    def run_runner(self, capture_bytes, stub_exit=0, gate=True, extra_env=None):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        tmpdir = Path(tmp.name)

        capture = tmpdir / "stub-capture.bin"
        capture.write_bytes(capture_bytes)
        stub = tmpdir / "stub-qemu.sh"
        stub.write_text(f"#!/usr/bin/env bash\ncat '{capture}'\nexit {stub_exit}\n",
                        encoding="utf-8")
        stub.chmod(0o755)

        elf = tmpdir / "frick_fsw_reqtest"
        elf.write_bytes(b"\x7fELF synthetic test image (T3 fixture)\n")

        env = dict(os.environ)
        for name in ("T2_ALLOW_QEMU_STUB", "T2_QEMU_CMD", "T2_RUN_ROOT", "T2_SKIP_PREREQ",
                     "T2_ELF"):
            env.pop(name, None)
        if gate:
            env["T2_ALLOW_QEMU_STUB"] = "1"
        env["T2_QEMU_CMD"] = f"bash '{stub}'"
        env["T2_RUN_ROOT"] = str(tmpdir / "runs")
        env["T2_SKIP_PREREQ"] = "1"
        env["T2_ELF"] = str(elf)
        if extra_env:
            env.update(extra_env)

        proc = subprocess.run(["bash", str(RUNNER)], capture_output=True, text=True,
                              timeout=180, env=env, cwd=str(ROOT))
        return proc

    def test_valid_capture_and_exit_0_passes(self):
        proc = self.run_runner(u1_capture(), 0)
        self.assertEqual(proc.returncode, 0, msg=proc.stdout + proc.stderr)
        self.assertIn("uart-tm: PASS", proc.stdout)
        self.assertIn("RESULT: PASS", proc.stdout)

    def test_same_valid_capture_with_qemu_exit_1_fails(self):
        """The T2-M1 regression: a QEMU failure must fail even with a valid U1 set."""
        proc = self.run_runner(u1_capture(), 1)
        self.assertEqual(proc.returncode, 1, msg=proc.stdout + proc.stderr)
        self.assertIn("uart-tm: PASS", proc.stdout)          # component passed ...
        self.assertIn("QEMU PROCESS FAILURE: exit 1", proc.stderr)  # ... run did not
        self.assertNotIn("RESULT: PASS", proc.stdout)

    def test_empty_capture_with_exit_0_fails(self):
        proc = self.run_runner(b"", 0)
        self.assertEqual(proc.returncode, 1, msg=proc.stdout + proc.stderr)
        self.assertIn("no_tm_received", proc.stdout)

    def test_elf_override_alone_is_refused_without_the_gate(self):
        env = dict(os.environ)
        for name in ("T2_ALLOW_QEMU_STUB", "T2_QEMU_CMD", "T2_RUN_ROOT", "T2_SKIP_PREREQ",
                     "T2_ELF"):
            env.pop(name, None)
        env["T2_ELF"] = "/tmp/not-an-image"
        proc = subprocess.run(["bash", str(RUNNER)], capture_output=True, text=True,
                              timeout=60, env=env, cwd=str(ROOT))
        self.assertEqual(proc.returncode, 2, msg=proc.stdout + proc.stderr)
        self.assertIn("REFUSED", proc.stderr)

    def test_missing_elf_with_skip_prereq_exits_2(self):
        """A missing image is a prerequisite failure (2), never a green result."""
        proc = self.run_runner(u1_capture(), 0,
                              extra_env={"T2_ELF": "/tmp/t2-definitely-absent-image"})
        self.assertEqual(proc.returncode, 2, msg=proc.stdout + proc.stderr)
        self.assertIn("MISSING PREREQUISITE", proc.stderr)

    def test_overrides_without_gate_are_refused(self):
        proc = self.run_runner(u1_capture(), 0, gate=False)
        self.assertEqual(proc.returncode, 2, msg=proc.stdout + proc.stderr)
        self.assertIn("REFUSED", proc.stderr)

    def test_bounded_timeout_124_is_accepted(self):
        proc = self.run_runner(u1_capture(), 124)
        self.assertEqual(proc.returncode, 0, msg=proc.stdout + proc.stderr)
        self.assertIn("qemu exit: 124", proc.stdout)
        self.assertIn("RESULT: PASS", proc.stdout)

    def test_watchdog_abort_capture_reports_blocked_not_pass(self):
        abort = (b"watchdog: NOT WIRED on RTEMS (timeout 8000 ms) - NC-PAL-01\r\n"
                 b"[FF-PANIC] watchdog: init failed\r\n")
        proc = self.run_runner(abort, 0)
        self.assertEqual(proc.returncode, 1, msg=proc.stdout + proc.stderr)
        self.assertIn("RESULT: FAIL", proc.stderr + proc.stdout)
        self.assertIn("watchdog init", proc.stderr)
        self.assertIn("BLOCKED", proc.stderr)


    def test_u1_capture_with_fatal_block_fails(self):
        """N10-M2: a U1 set plus a panic/fatal block must NOT be a PASS."""
        fatal = (b"[FF-PANIC] watchdog: init failed\r\n"
                 b"*** EXIT STATUS NOT ZERO ***\r\n"
                 b"exit source: 5 (RTEMS_FATAL_SOURCE_EXIT)\r\n")
        proc = self.run_runner(u1_capture() + fatal, 0)
        self.assertEqual(proc.returncode, 1, msg=proc.stdout + proc.stderr)
        self.assertIn("uart-tm: PASS", proc.stdout)          # component U1 was fine
        self.assertIn("console fatal markers: 3", proc.stdout)
        self.assertIn("WHOLE-RUN: the console contains a panic/fatal marker", proc.stderr)
        self.assertNotIn("RESULT: PASS", proc.stdout + proc.stderr)

    def test_u1_capture_with_general_rtems_fatal_fails(self):
        """R2-01: `*** FATAL ***` / `fatal source:` (assert, exception, ...)."""
        for exit_code in (0, 124):
            fatal = (b"*** FATAL ***\r\n"
                     b"fatal source: 2 (RTEMS_FATAL_SOURCE_ASSERT)\r\n")
            proc = self.run_runner(u1_capture() + fatal, exit_code)
            self.assertEqual(proc.returncode, 1,
                             msg=f"exit {exit_code}: " + proc.stdout + proc.stderr)
            self.assertIn("console fatal markers: 2", proc.stdout)
            self.assertNotIn("RESULT: PASS", proc.stdout + proc.stderr)

    def test_u1_capture_with_fatal_block_and_124_also_fails(self):
        fatal = b"[FF-PANIC] watchdog: init failed\r\n"
        proc = self.run_runner(u1_capture() + fatal, 124)
        self.assertEqual(proc.returncode, 1, msg=proc.stdout + proc.stderr)
        self.assertIn("qemu exit: 124", proc.stdout)


if __name__ == "__main__":
    unittest.main(verbosity=2)
