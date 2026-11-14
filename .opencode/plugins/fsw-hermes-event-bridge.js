import { spawn } from "node:child_process";
import { toBridgeEvent } from "../../tools/scripts/fsw_opencode_event_bridge_plugin_core.mjs";

const PYTHON = "/usr/bin/python3";
const HELPER = "/home/nicola/fsw/frick-fsw/tools/scripts/fsw_opencode_event_bridge.py";

function enqueueAndNotify(payload) {
  return new Promise((resolve, reject) => {
    const child = spawn(PYTHON, [HELPER, "emit"], {
      cwd: "/home/nicola/fsw/frick-fsw",
      env: process.env,
      stdio: ["pipe", "ignore", "ignore"],
    });
    let settled = false;
    const timer = setTimeout(() => {
      if (settled) return;
      settled = true;
      child.kill("SIGTERM");
      reject(new Error("event bridge helper timed out"));
    }, 4500);
    child.once("error", (error) => {
      if (settled) return;
      settled = true;
      clearTimeout(timer);
      reject(error);
    });
    child.once("close", (code) => {
      if (settled) return;
      settled = true;
      clearTimeout(timer);
      if (code === 0) resolve();
      else reject(new Error(`event bridge helper exited ${code}`));
    });
    child.stdin.end(JSON.stringify(payload));
  });
}

export const FswHermesEventBridge = async ({ directory }) => ({
  event: async ({ event }) => {
    const payload = toBridgeEvent(event, directory);
    if (!payload) return;
    try {
      await enqueueAndNotify(payload);
    } catch (error) {
      console.error(`[fsw-hermes-event-bridge] ${error?.message ?? "delivery failed"}`);
    }
  },
});
