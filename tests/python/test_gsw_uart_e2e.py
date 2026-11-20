#!/usr/bin/env python3
# SPDX-License-Identifier: MPL-2.0
"""Host-SIL GSW↔PUS↔production-UART E2E test over bridged POSIX PTYs.

The test uses a test-only serial.Serial adapter. It exercises the GSW CLI's
PUS protocol path and production C UART framing, but is not pyserial, physical
UART, target, or HIL evidence.
"""
from __future__ import annotations

import errno
import os
import pty
import re
import select
import subprocess
import sys
import tempfile
import threading
import time
import tty
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "gsw"))
from ff_gsw import crc16  # noqa: E402

EXPECTED_SERVICES = [(1, 1), (1, 3), (17, 2), (1, 7)]


def _write_all(destination: int, data: bytes) -> None:
    view = memoryview(data)
    deadline = time.monotonic() + 1.0
    while view:
        if time.monotonic() >= deadline:
            raise TimeoutError("PTY bridge write timed out")
        try:
            written = os.write(destination, view)
            view = view[written:]
        except BlockingIOError:
            _r, writable, _x = select.select([], [destination], [], 0.05)
            if not writable:
                continue
        except OSError as exc:
            if exc.errno == errno.EIO:
                return
            raise


def _bridge(stop: threading.Event, left_master: int, right_master: int,
            *, drop_accept_ack: bool = False,
            dropped_ack: threading.Event | None = None) -> None:
    for fd in (left_master, right_master):
        os.set_blocking(fd, False)
    peer = {left_master: right_master, right_master: left_master}
    pending_tm = bytearray()
    dropped = False
    while not stop.is_set():
        try:
            readable, _writable, _exceptional = select.select(
                [left_master, right_master], [], [], 0.05)
        except OSError:
            return
        for source in readable:
            try:
                data = os.read(source, 4096)
            except BlockingIOError:
                continue
            except OSError as exc:
                if exc.errno == errno.EIO:
                    continue
                raise
            if not data:
                continue
            if drop_accept_ack and source == left_master:
                pending_tm.extend(data)
                outgoing = bytearray()
                while len(pending_tm) >= 6:
                    packet_len = 7 + int.from_bytes(pending_tm[4:6], "big")
                    if packet_len < 21 or packet_len > 1024:
                        outgoing.append(pending_tm.pop(0))
                        continue
                    if len(pending_tm) < packet_len:
                        break
                    packet = bytes(pending_tm[:packet_len])
                    del pending_tm[:packet_len]
                    if not dropped and packet[7:9] == b"\x01\x01":
                        dropped = True
                        if dropped_ack is not None:
                            dropped_ack.set()
                        continue
                    outgoing.extend(packet)
                data = bytes(outgoing)
                if not data:
                    continue
            _write_all(peer[source], data)


def _decode_tm_capture(raw: bytes):
    frames = []
    offset = 0
    while offset < len(raw):
        if len(raw) - offset < 6:
            raise AssertionError("truncated CCSDS primary header in TM capture")
        word = int.from_bytes(raw[offset:offset + 2], "big")
        sequence = int.from_bytes(raw[offset + 2:offset + 4], "big")
        pdl = int.from_bytes(raw[offset + 4:offset + 6], "big")
        packet_len = 7 + pdl
        end = offset + packet_len
        if end > len(raw):
            raise AssertionError("truncated CCSDS TM packet in capture")
        frame = raw[offset:end]
        if ((word & 0x07FF) != 0x020 or ((word >> 11) & 1) != 1 or
                (sequence >> 14) != 0x03 or crc16(frame) != 0):
            raise AssertionError("TM capture contains an invalid CCSDS packet")
        if (len(frame) < 21) or ((frame[6] >> 4) != 2):
            raise AssertionError("TM capture lacks the expected PUS-C secondary header/CRC")
        pus_counter = int.from_bytes(frame[9:11], "big")
        frames.append((pus_counter & 0x3FFF, frame[7], frame[8], frame[19:-2]))
        offset = end
    return frames


def _assert_ack_payloads(frames) -> None:
    acks = [(sub, data) for _count, service, sub, data in frames if service == 1]
    if [sub for sub, _data in acks] != [1, 3, 7]:
        raise AssertionError(f"unexpected ACK subservices: {[sub for sub, _data in acks]}")
    for expected_sequence, (_sub, data) in enumerate(acks, start=1):
        expected_prefix = bytes.fromhex("01020001")
        if (len(data) != 9 or data[:4] != expected_prefix or
                int.from_bytes(data[4:6], "big") != expected_sequence or
                data[6:8] != bytes((17, 1)) or data[8] != 0):
            raise AssertionError(f"invalid positive ACK payload: {data.hex()}")


def run(server_path: Path, *, drop_accept_ack: bool = False) -> None:
    fsw_master, fsw_slave = pty.openpty()
    gsw_master, gsw_slave = pty.openpty()
    fsw_path = os.ttyname(fsw_slave)
    gsw_path = os.ttyname(gsw_slave)
    tty.setraw(fsw_slave)
    tty.setraw(gsw_slave)
    os.close(fsw_slave)
    os.close(gsw_slave)

    with tempfile.TemporaryDirectory(prefix="frick-gsw-e2e-") as temp:
        temp_path = Path(temp)
        tm_capture = temp_path / "tm.bin"
        tc_capture = temp_path / "tc.bin"
        env = os.environ.copy()
        env["FRICK_E2E_UART_DEVICE"] = fsw_path
        env["FRICK_E2E_TM_CAPTURE_PATH"] = str(tm_capture)
        env["FF_GSW_CAPTURE_TC_PATH"] = str(tc_capture)
        env["FF_GSW_CAPTURE_TM_PATH"] = str(temp_path / "gsw-tm.bin")
        env["PYTHONPATH"] = os.pathsep.join((
            str(ROOT / "tests" / "python" / "stubs"),
            str(ROOT / "tools" / "gsw"),
            env.get("PYTHONPATH", ""),
        )).rstrip(os.pathsep)

        server = subprocess.Popen([str(server_path)], cwd=ROOT, env=env,
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                  text=True)
        cli_timeout = "0.25" if drop_accept_ack else "3.0"
        gsw = subprocess.Popen(
            [sys.executable, str(ROOT / "tools" / "gsw" / "ff_gsw.py"),
             "--connect", gsw_path, "--ping", "--ack", "accept,start,completion",
             "--timeout", cli_timeout],
            cwd=ROOT, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            text=True)
        stop = threading.Event()
        dropped_ack = threading.Event()
        relay = threading.Thread(
            target=_bridge, args=(stop, fsw_master, gsw_master),
            kwargs={"drop_accept_ack": drop_accept_ack, "dropped_ack": dropped_ack},
            daemon=True)
        relay.start()
        try:
            try:
                gsw_out, gsw_err = gsw.communicate(timeout=7)
            except subprocess.TimeoutExpired:
                gsw.kill()
                gsw_out, gsw_err = gsw.communicate()
                raise AssertionError(f"GSW CLI timed out\nstdout:\n{gsw_out}\nstderr:\n{gsw_err}")
            try:
                server_out, server_err = server.communicate(timeout=2)
            except subprocess.TimeoutExpired:
                server.kill()
                server_out, server_err = server.communicate()
                raise AssertionError(
                    f"PUS UART server timed out\nstdout:\n{server_out}\nstderr:\n{server_err}")
        finally:
            stop.set()
            relay.join(timeout=1)
            os.close(fsw_master)
            os.close(gsw_master)

        if server.returncode != 0:
            raise AssertionError(
                f"PUS UART server failed ({server.returncode})\n{server_out}\n{server_err}")
        expected_gsw_services = EXPECTED_SERVICES
        if drop_accept_ack:
            if gsw.returncode != 1 or not dropped_ack.is_set():
                raise AssertionError(
                    f"expected the dropped acceptance ACK to be rejected; rc={gsw.returncode}\n"
                    f"{gsw_out}\n{gsw_err}")
            if "received 3 TM packets" not in gsw_out or "ERROR: missing, duplicate, unexpected, or failed PUS ping response/ACK" not in gsw_out:
                raise AssertionError(f"GSW did not fail closed on the missing ACK:\n{gsw_out}")
            expected_gsw_services = EXPECTED_SERVICES[1:]
        elif gsw.returncode != 0:
            raise AssertionError(f"GSW CLI failed ({gsw.returncode})\n{gsw_out}\n{gsw_err}")
        expected_received = len(expected_gsw_services)
        if f"received {expected_received} TM packets" not in gsw_out:
            raise AssertionError(f"GSW received an unexpected TM count:\n{gsw_out}")

        observed = [(int(svc), int(sub)) for svc, sub in re.findall(
            r"^  S(\d+)\.(\d+) payload=", gsw_out, flags=re.MULTILINE)]
        if observed != expected_gsw_services:
            raise AssertionError(f"unexpected GSW service sequence {observed}:\n{gsw_out}")
        expected_tc = bytes.fromhex("1810c000000a2b1101010201020304f72b")
        if tc_capture.read_bytes() != expected_tc:
            raise AssertionError("GSW CLI TC bytes differ from the expected accept/start/completion vector")

        frames = _decode_tm_capture(tm_capture.read_bytes())
        if [(service, sub) for _count, service, sub, _data in frames] != EXPECTED_SERVICES:
            raise AssertionError(f"unexpected PUS TM sequence: {frames!r}")
        counters = [count for count, _service, _sub, _data in frames]
        if any(((right - left) & 0x3FFF) != 1 for left, right in zip(counters, counters[1:])):
            raise AssertionError(f"TM sequence counter is not incrementing modulo 16384: {counters}")
        if frames[2][3] != bytes.fromhex("01020304"):
            raise AssertionError(f"unexpected S17 echo payload: {frames[2][3].hex()}")
        _assert_ack_payloads(frames)

        if drop_accept_ack:
            print("PASS: GSW CLI rejects a deliberately dropped requested S1 acceptance ACK")
        else:
            print("PASS: GSW CLI ↔ POSIX PTY bridge ↔ production UART backend ↔ PUS")
            print("PASS: exact S1/S17 services, ACK request IDs/codes, and modulo-16384 TM counters")
        print("LIMIT: test-only serial adapter; not pyserial, physical UART, RTEMS, or HIL evidence")


def main() -> int:
    if len(sys.argv) not in (2, 3):
        print("usage: test_gsw_uart_e2e.py <server> [drop-accept-ack]", file=sys.stderr)
        return 2
    try:
        if len(sys.argv) == 3 and sys.argv[2] != "drop-accept-ack":
            raise AssertionError(f"unknown E2E test mode: {sys.argv[2]}")
        run(Path(sys.argv[1]).resolve(), drop_accept_ack=len(sys.argv) == 3)
    except (AssertionError, OSError, TimeoutError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
