#!/usr/bin/env python3
# Copyright (C) 2026 Ilfrick
# SPDX-License-Identifier: MPL-2.0
"""Durable OpenCode event queue and loopback Hermes webhook bridge for FSW."""
from __future__ import annotations

import argparse
import fcntl
import hashlib
import hmac
import json
import os
import stat
import sys
import time
import urllib.error
import urllib.request
import uuid
from contextlib import contextmanager
from pathlib import Path
from typing import Any, Iterator

PROJECT_ROOT = Path("/home/nicola/fsw/frick-fsw").resolve()
ROUTE_NAME = "fsw-opencode"
SUPPORTED_EVENTS = {"session.idle", "session.status", "question.asked", "permission.asked", "session.error"}
ACK_STATUSES = {"handled", "no_action", "escalated", "awaiting_owner"}


def hermes_home() -> Path:
    return Path(os.environ.get("HERMES_HOME", str(Path.home() / ".hermes"))).expanduser().resolve()


def queue_dir() -> Path:
    configured = os.environ.get("FSW_OPENCODE_QUEUE_DIR")
    return Path(configured).expanduser().resolve() if configured else hermes_home() / "state" / "fsw-opencode-events"


def _ensure_private_dir(path: Path) -> None:
    path.mkdir(mode=0o700, parents=True, exist_ok=True)
    os.chmod(path, 0o700)


@contextmanager
def _locked(directory: Path) -> Iterator[None]:
    _ensure_private_dir(directory)
    lock_path = directory / ".lock"
    fd = os.open(lock_path, os.O_CREAT | os.O_RDWR, 0o600)
    try:
        os.chmod(lock_path, 0o600)
        fcntl.flock(fd, fcntl.LOCK_EX)
        yield
    finally:
        fcntl.flock(fd, fcntl.LOCK_UN)
        os.close(fd)


def _read_jsonl(path: Path) -> list[dict[str, Any]]:
    if not path.exists():
        return []
    records: list[dict[str, Any]] = []
    with path.open("r", encoding="utf-8") as stream:
        for line_number, line in enumerate(stream, 1):
            if not line.strip():
                continue
            try:
                record = json.loads(line)
            except json.JSONDecodeError as exc:
                raise ValueError(f"invalid queue record at {path.name}:{line_number}") from exc
            if not isinstance(record, dict):
                raise ValueError(f"non-object queue record at {path.name}:{line_number}")
            records.append(record)
    return records


def normalize_event(payload: Any, *, created_at: str | None = None) -> dict[str, Any]:
    if not isinstance(payload, dict):
        raise ValueError("event payload must be an object")
    name = payload.get("event_name")
    if name not in SUPPORTED_EVENTS:
        raise ValueError("unsupported event")
    status_type = payload.get("status_type")
    if name == "session.status" and status_type != "idle":
        raise ValueError("session status event is not idle")
    supplied_root = payload.get("project_dir")
    if not isinstance(supplied_root, str) or Path(supplied_root).resolve() != PROJECT_ROOT:
        raise ValueError("event project root does not match the FSW repository")
    session_id = payload.get("session_id")
    if name != "session.error" and (not isinstance(session_id, str) or not session_id.strip()):
        raise ValueError("session event is missing a session id")
    if session_id is not None and (not isinstance(session_id, str) or len(session_id) > 256):
        raise ValueError("invalid session id")
    event_id = payload.get("event_id") or uuid.uuid4().hex
    if not isinstance(event_id, str) or not event_id or len(event_id) > 128:
        raise ValueError("invalid event id")
    def optional_id(key: str) -> str | None:
        value = payload.get(key)
        if value is None or value == "":
            return None
        if not isinstance(value, str) or len(value) > 256:
            raise ValueError(f"invalid {key}")
        return value
    record = {
        "event_id": event_id,
        "event_name": name,
        "session_id": session_id,
        "message_id": optional_id("message_id"),
        "permission_id": optional_id("permission_id"),
        "request_id": optional_id("request_id"),
        "project_dir": str(PROJECT_ROOT),
        "created_at": created_at or payload.get("created_at") or time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
    }
    if name == "session.status":
        record["status_type"] = status_type
    return record


def append_event(directory: Path, event: dict[str, Any]) -> bool:
    normalized = normalize_event(event, created_at=event.get("created_at"))
    with _locked(directory):
        events_path = directory / "events.jsonl"
        known = {row.get("event_id") for row in _read_jsonl(events_path)}
        if normalized["event_id"] in known:
            return False
        line = (json.dumps(normalized, sort_keys=True, separators=(",", ":")) + "\n").encode("utf-8")
        fd = os.open(events_path, os.O_CREAT | os.O_APPEND | os.O_WRONLY, 0o600)
        try:
            os.chmod(events_path, 0o600)
            written = os.write(fd, line)
            if written != len(line):
                raise OSError("short write while appending event")
            os.fsync(fd)
        finally:
            os.close(fd)
    return True


def pending_events(directory: Path) -> list[dict[str, Any]]:
    with _locked(directory):
        events = _read_jsonl(directory / "events.jsonl")
        acks = _read_jsonl(directory / "acks.jsonl")
    acknowledged = {row.get("event_id") for row in acks}
    return [row for row in events if row.get("event_id") not in acknowledged]


def acknowledge_event(directory: Path, event_id: str, status: str) -> bool:
    if status not in ACK_STATUSES:
        raise ValueError("invalid acknowledgement status")
    with _locked(directory):
        events = _read_jsonl(directory / "events.jsonl")
        if not any(row.get("event_id") == event_id for row in events):
            raise ValueError("unknown event")
        acks_path = directory / "acks.jsonl"
        acks = _read_jsonl(acks_path)
        if any(row.get("event_id") == event_id for row in acks):
            return False
        ack = {"event_id": event_id, "status": status, "acknowledged_at": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())}
        line = (json.dumps(ack, sort_keys=True, separators=(",", ":")) + "\n").encode("utf-8")
        fd = os.open(acks_path, os.O_CREAT | os.O_APPEND | os.O_WRONLY, 0o600)
        try:
            os.chmod(acks_path, 0o600)
            written = os.write(fd, line)
            if written != len(line):
                raise OSError("short write while appending acknowledgement")
            os.fsync(fd)
        finally:
            os.close(fd)
    return True


def monitor_snapshot(directory: Path, *, now: int | None = None) -> dict[str, Any]:
    pending = pending_events(directory)
    result: dict[str, Any] = {
        "pending": [
            {key: row.get(key) for key in ("event_id", "event_name", "session_id", "message_id", "permission_id", "request_id", "status_type")}
            for row in pending
        ]
    }
    # If processing failed, keep retrying on the 5-minute fallback instead of losing a static event.
    result["retry_epoch"] = (int(now if now is not None else time.time()) // 240) if pending else None
    return result


def webhook_headers(secret: str, timestamp: str, body: bytes, request_id: str) -> dict[str, str]:
    signature = hmac.new(secret.encode("utf-8"), timestamp.encode("ascii") + b"." + body, hashlib.sha256).hexdigest()
    return {
        "Content-Type": "application/json",
        "X-Webhook-Timestamp": timestamp,
        "X-Webhook-Signature-V2": signature,
        "X-Request-ID": request_id,
    }


def _route_secret(home: Path) -> str:
    subscriptions = home / "webhook_subscriptions.json"
    try:
        mode = stat.S_IMODE(subscriptions.stat().st_mode)
        if mode & 0o077:
            raise PermissionError("webhook subscriptions file permissions are too broad")
        data = json.loads(subscriptions.read_text(encoding="utf-8"))
        secret = data[ROUTE_NAME]["secret"]
    except (OSError, KeyError, TypeError, json.JSONDecodeError) as exc:
        raise RuntimeError("FSW OpenCode webhook route is unavailable") from exc
    if not isinstance(secret, str) or not secret:
        raise RuntimeError("FSW OpenCode webhook secret is missing")
    return secret


def dispatch_webhook(event: dict[str, Any], *, home: Path | None = None, url: str | None = None, timeout: float = 2.0) -> int:
    home = home or hermes_home()
    secret = _route_secret(home)
    body_obj = {
        "event_type": "opencode",
        "event_id": event["event_id"],
        "event_name": event["event_name"],
        "session_id": event.get("session_id"),
        "message_id": event.get("message_id"),
        "permission_id": event.get("permission_id"),
        "request_id": event.get("request_id"),
        "status_type": event.get("status_type"),
        "project_dir": event["project_dir"],
    }
    body = json.dumps(body_obj, sort_keys=True, separators=(",", ":")).encode("utf-8")
    timestamp = str(int(time.time()))
    target = url or os.environ.get("FSW_OPENCODE_WEBHOOK_URL", "http://127.0.0.1:8644/webhooks/fsw-opencode")
    request = urllib.request.Request(target, data=body, headers=webhook_headers(secret, timestamp, body, event["event_id"]), method="POST")
    try:
        with urllib.request.urlopen(request, timeout=timeout) as response:
            return int(response.status)
    except urllib.error.HTTPError as exc:
        return int(exc.code)
    except (urllib.error.URLError, TimeoutError, OSError):
        return 0


def _main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("emit", help="append one OpenCode event and attempt immediate webhook delivery")
    sub.add_parser("pending", help="list unacknowledged OpenCode events")
    monitor = sub.add_parser("monitor", help="emit stable monitor state for pending events")
    monitor.add_argument("--now", type=int, default=None, help=argparse.SUPPRESS)
    ack = sub.add_parser("ack", help="acknowledge an event after handling or escalation")
    ack.add_argument("event_id")
    ack.add_argument("status", choices=sorted(ACK_STATUSES))
    args = parser.parse_args(argv)
    directory = queue_dir()
    if args.command == "emit":
        try:
            payload = json.load(sys.stdin)
            event = normalize_event(payload)
            added = append_event(directory, event)
            status = dispatch_webhook(event) if added else None
            print(json.dumps({"event_id": event["event_id"], "queued": True, "duplicate": not added, "webhook_status": status}, sort_keys=True))
            return 0
        except Exception as exc:
            print(json.dumps({"queued": False, "error": type(exc).__name__}), file=sys.stderr)
            return 1
    if args.command == "pending":
        print(json.dumps(pending_events(directory), sort_keys=True, separators=(",", ":")))
        return 0
    if args.command == "monitor":
        print(json.dumps(monitor_snapshot(directory, now=args.now), sort_keys=True, separators=(",", ":")))
        return 0
    if args.command == "ack":
        try:
            changed = acknowledge_event(directory, args.event_id, args.status)
            print(json.dumps({"event_id": args.event_id, "acknowledged": changed, "status": args.status}, sort_keys=True))
            return 0
        except Exception as exc:
            print(json.dumps({"acknowledged": False, "error": type(exc).__name__}), file=sys.stderr)
            return 1
    return 2


if __name__ == "__main__":
    raise SystemExit(_main(sys.argv[1:]))
