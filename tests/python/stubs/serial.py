# SPDX-License-Identifier: MPL-2.0
"""Minimal pyserial-compatible adapter for the POSIX PTY GSW integration test.

This test-only shim exercises the GSW CLI protocol flow without claiming
pyserial, physical UART, target baud-rate, or HIL qualification.
"""
from __future__ import annotations

import os
import select
import termios
import time


class Serial:
    def __init__(self, port, baudrate=9600, timeout=None):
        if baudrate != 115200:
            raise ValueError("PTY integration supports only the configured 115200 baud profile")
        self._fd = os.open(port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
        self.timeout = timeout
        self._capture_tx = os.environ.get("FF_GSW_CAPTURE_TC_PATH")
        self._capture_rx = os.environ.get("FF_GSW_CAPTURE_TM_PATH")
        attrs = termios.tcgetattr(self._fd)
        attrs[0] = 0
        attrs[1] = 0
        attrs[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
        attrs[3] = 0
        attrs[4] = termios.B115200
        attrs[5] = termios.B115200
        attrs[6][termios.VMIN] = 0
        attrs[6][termios.VTIME] = 0
        termios.tcsetattr(self._fd, termios.TCSANOW, attrs)

    @staticmethod
    def _capture(path, data):
        if path is not None and data:
            with open(path, "ab") as capture:
                capture.write(data)

    def write(self, data):
        payload = memoryview(data)
        written = 0
        deadline = time.monotonic() + 1.0
        while written < len(payload):
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise TimeoutError("PTY serial write timed out")
            _readable, writable, _exceptional = select.select([], [self._fd], [], remaining)
            if not writable:
                raise TimeoutError("PTY serial write timed out")
            written += os.write(self._fd, payload[written:])
        self._capture(self._capture_tx, bytes(payload))
        return written

    def read(self, size=1):
        timeout = self.timeout
        readable, _writable, _exceptional = select.select(
            [self._fd], [], [], timeout if timeout is not None else None)
        if not readable:
            return b""
        try:
            data = os.read(self._fd, size)
        except BlockingIOError:
            return b""
        self._capture(self._capture_rx, data)
        return data

    def close(self):
        if self._fd >= 0:
            os.close(self._fd)
            self._fd = -1

    def __enter__(self):
        return self

    def __exit__(self, _exc_type, _exc_value, _traceback):
        self.close()
