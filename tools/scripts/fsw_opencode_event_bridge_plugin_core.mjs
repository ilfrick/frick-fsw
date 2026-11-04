import { randomUUID } from "node:crypto";
import { resolve } from "node:path";

export const FSW_PROJECT_ROOT = "/home/nicola/fsw/frick-fsw";
const ALLOWED_TYPES = new Set([
  "session.idle",
  "session.status",
  "question.asked",
  "permission.asked",
  "session.error",
]);

export function toBridgeEvent(event, directory, idFactory = randomUUID) {
  if (resolve(directory ?? "") !== FSW_PROJECT_ROOT) return null;
  if (!event || !ALLOWED_TYPES.has(event.type)) return null;

  const properties = event.properties ?? {};
  const statusType = properties.status?.type ?? properties.statusType ?? null;
  if (event.type === "session.status" && statusType !== "idle") return null;

  const sessionId = properties.sessionID ?? properties.session?.id ?? null;
  if (event.type !== "session.error" && (typeof sessionId !== "string" || !sessionId)) return null;

  const permissionId = properties.permissionID ??
    (event.type === "permission.asked" ? properties.id : null) ?? null;
  const requestId = properties.requestID ??
    (event.type === "question.asked" ? properties.id : null) ?? null;
  const messageId = properties.messageID ?? properties.info?.id ?? null;

  return {
    event_id: idFactory(),
    event_name: event.type,
    status_type: event.type === "session.status" ? statusType : null,
    session_id: sessionId,
    message_id: typeof messageId === "string" ? messageId : null,
    permission_id: typeof permissionId === "string" ? permissionId : null,
    request_id: typeof requestId === "string" ? requestId : null,
    project_dir: FSW_PROJECT_ROOT,
  };
}
