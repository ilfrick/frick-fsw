import test from "node:test";
import assert from "node:assert/strict";
import { toBridgeEvent } from "../../tools/scripts/fsw_opencode_event_bridge_plugin_core.mjs";

const root = "/home/nicola/fsw/frick-fsw";
const id = () => "evt-node-test";

function event(type, properties = {}) {
  return { type, properties };
}

test("idle session status becomes a durable FSW event", () => {
  const result = toBridgeEvent(event("session.status", {
    sessionID: "ses_123",
    status: { type: "idle" },
  }), root, id);
  assert.deepEqual(result, {
    event_id: "evt-node-test",
    event_name: "session.status",
    status_type: "idle",
    session_id: "ses_123",
    message_id: null,
    permission_id: null,
    request_id: null,
    project_dir: root,
  });
});

test("busy session status is ignored", () => {
  assert.equal(toBridgeEvent(event("session.status", {
    sessionID: "ses_123",
    status: { type: "busy" },
  }), root, id), null);
});

test("OpenCode questions and permissions preserve request identifiers", () => {
  const question = toBridgeEvent(event("question.asked", {
    sessionID: "ses_123", requestID: "question_7",
  }), root, id);
  assert.equal(question.event_name, "question.asked");
  assert.equal(question.request_id, "question_7");

  const permission = toBridgeEvent(event("permission.asked", {
    sessionID: "ses_123", id: "permission_9",
  }), root, id);
  assert.equal(permission.event_name, "permission.asked");
  assert.equal(permission.permission_id, "permission_9");
});

test("events from another project and unsupported event kinds are ignored", () => {
  assert.equal(toBridgeEvent(event("question.asked", { sessionID: "ses_123" }), "/home/nicola/fsw/other", id), null);
  assert.equal(toBridgeEvent(event("message.part.updated", { sessionID: "ses_123" }), root, id), null);
});
