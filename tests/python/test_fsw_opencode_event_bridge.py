# Copyright (C) 2026 Ilfrick
# SPDX-License-Identifier: MPL-2.0
from __future__ import annotations

import hashlib
import hmac
import importlib.util
import json
import sys
from pathlib import Path

import pytest

REPO = Path(__file__).resolve().parents[2]
MODULE_PATH = REPO / "tools" / "scripts" / "fsw_opencode_event_bridge.py"
spec = importlib.util.spec_from_file_location("fsw_opencode_event_bridge", MODULE_PATH)
assert spec is not None and spec.loader is not None, "event bridge module must exist"
bridge = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = bridge
spec.loader.exec_module(bridge)


def make_event(**overrides):
    event = {
        "event_id": "evt-0001",
        "event_name": "session.idle",
        "session_id": "ses_test_001",
        "message_id": "msg_test_001",
        "permission_id": None,
        "project_dir": "/home/nicola/fsw/frick-fsw",
    }
    event.update(overrides)
    return event


def test_normalize_event_accepts_supported_fsw_session_event():
    record = bridge.normalize_event(make_event(), created_at="2026-10-29T12:00:00Z")
    assert record["event_id"] == "evt-0001"
    assert record["event_name"] == "session.idle"
    assert record["session_id"] == "ses_test_001"
    assert record["project_dir"] == "/home/nicola/fsw/frick-fsw"
    assert record["created_at"] == "2026-10-29T12:00:00Z"


def test_normalize_event_accepts_question_and_idle_status_events():
    question = bridge.normalize_event(make_event(event_name="question.asked", request_id="req_test_001"))
    assert question["event_name"] == "question.asked"
    assert question["request_id"] == "req_test_001"

    status = bridge.normalize_event(make_event(event_name="session.status", status_type="idle"))
    assert status["event_name"] == "session.status"
    assert status["status_type"] == "idle"


def test_normalize_event_rejects_busy_status_event():
    with pytest.raises(ValueError, match="not idle"):
        bridge.normalize_event(make_event(event_name="session.status", status_type="busy"))


def test_normalize_event_rejects_unrelated_event_or_project():
    with pytest.raises(ValueError, match="unsupported event"):
        bridge.normalize_event(make_event(event_name="message.part.updated"))
    with pytest.raises(ValueError, match="project root"):
        bridge.normalize_event(make_event(project_dir="/home/nicola/fsw/other"))


def test_queue_deduplicates_and_acknowledges_only_pending_events(tmp_path):
    event = bridge.normalize_event(make_event())
    assert bridge.append_event(tmp_path, event) is True
    assert bridge.append_event(tmp_path, event) is False
    assert [item["event_id"] for item in bridge.pending_events(tmp_path)] == ["evt-0001"]
    assert bridge.acknowledge_event(tmp_path, "evt-0001", "handled") is True
    assert bridge.acknowledge_event(tmp_path, "evt-0001", "handled") is False
    assert bridge.pending_events(tmp_path) == []


def test_acknowledgement_rejects_unknown_event(tmp_path):
    with pytest.raises(ValueError, match="unknown event"):
        bridge.acknowledge_event(tmp_path, "evt-missing", "handled")


def test_webhook_v2_signature_matches_timestamp_dot_body():
    secret = "test-secret-only"
    timestamp = "1793275200"
    body = b'{"event_type":"opencode"}'
    headers = bridge.webhook_headers(secret, timestamp, body, "evt-0001")
    expected = hmac.new(secret.encode(), timestamp.encode() + b"." + body, hashlib.sha256).hexdigest()
    assert headers["X-Webhook-Signature-V2"] == expected
    assert headers["X-Webhook-Timestamp"] == timestamp
    assert headers["X-Request-ID"] == "evt-0001"


def test_monitor_snapshot_is_stable_and_retries_pending_events(tmp_path):
    empty_a = bridge.monitor_snapshot(tmp_path, now=600)
    empty_b = bridge.monitor_snapshot(tmp_path, now=601)
    assert empty_a == empty_b

    bridge.append_event(tmp_path, bridge.normalize_event(make_event()))
    pending_a = bridge.monitor_snapshot(tmp_path, now=600)
    pending_b = bridge.monitor_snapshot(tmp_path, now=601)
    pending_next_retry = bridge.monitor_snapshot(tmp_path, now=900)
    assert pending_a == pending_b
    assert pending_a != empty_a
    assert pending_next_retry != pending_a
    assert "evt-0001" in json.dumps(pending_a)
