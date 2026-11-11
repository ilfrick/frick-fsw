# XW-2 Renode loop — pull authorization, execution, and disappearance record

|                                 |                                             |
| -------------------------------: | :------------------------------------------ |
| Project                          | frick-fsw                                   |
| Document id                      | FRICK-VV-XW2-RENODE-EVID                    |
| Revision                         | 0.4                                         |
| Date                             | 2026-10-27                                  |
| Scope                            | Session execution record for the owner-authorised 2026-10-27 pull-and-execute loop (loop-audit rev 10.49) |
| Standing                         | Agent-session excerpts, labelled verbatim; not regenerated or recomputed |

This file records — exclusively from executable session transcripts, labelled
verbatim — (a) the authorized single pull of the pinned Renode image and its
verification, (b) the disappearance of the pulled image before any run, and
(c) the loop artefacts that remain in the tree.

## 1. Authorized pull — SUCCESS, digest verified

Command (excerpts verbatim):

```
$ docker pull antmicro/renode@sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061
Digest: sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061
Status: Downloaded newer image for antmicro/renode@sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061
PULL_EXIT=0
```

Verification immediately after the pull:

```
$ docker image inspect -f '{{json .RepoDigests}}' antmicro/renode@sha256:7905c32...
["antmicro/renode@sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061"]
$ docker image inspect -f 'ID={{.Id}}' …
ID=sha256:e278e663fd9516284f1ed3c1da5e7ff727db3bd9808b44a91e4325ffa1a0b858
$ docker run --rm antmicro/renode@sha256:7905c32… renode --version
Renode v1.16.1.16973
  build: d66b0c2a-202602160923
  build type: Release
```

The digest pin was therefore pulled exactly once and verified as authorized.

## 2. Disappearance — image absent before any run attempt

The first runner invocation (before any `docker run` of Renode) failed closed
at its prerequisite check:

```
[xw2-renode] run directory: /home/nicola/fsw/frick-fsw/build/xw2-renode-run-20261027T210640Z
[xw2-renode] MISSING PREREQUISITE: pinned Renode image 7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061 is not present (no pull attempted)
(RUNNER_EXIT=2)
```

Replication attempts (victim confirmed):

```
$ docker image inspect "antmicro/renode@sha256:7905c32…"
Error response from daemon: No such image: antmicro/renode@sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061
```

Diagnostics performed (all read-only): `docker context ls` (single default
context), `docker events --since 30m` (no delete/renode event surfaced),
systemd timers (no docker/prune timer), `crontab -l` (no prune entry), disk
usage (168 G free — no space-triggered GC), `docker ps -a` (no running cleanup
container; nextcloud-aio-watchtower exited 3 days ago), containerd namespaces
(no accessible image entry). No removal/disappearance source was identified
by diagnostics; this session issued no removal/`prune`/`system prune` command.
The absence cause and actor are UNCONFIRMED and unresolved — no external
actor is asserted or inferred.

## 3. Consequence and standing of the pull authorization

The owner's authorization was "exactly one pull" of this digest. That pull was
consumed, and the image was absent from the local Docker image store after the
verified pull and before any run. The removal/disappearance cause and actor
are unconfirmed, and no source was identified by diagnostics. Re-pulling is a
second network download and is NOT exercised without a fresh owner
authorization.

The only run attempts made are **fail-closed prerequisite prechecks**; no
Renode run of any image form was ever started:

1. `tools/scripts/renode_xw2_vv.sh` invocation 1 (2026-10-27, before the
   safety/CM patch): exit 2 — "MISSING PREREQUISITE: pinned Renode image … is
   not present (no pull attempted)". Transcript:
   `renode_xw2_run_transcript.log`.
2. `tools/scripts/renode_xw2_vv.sh` invocation 2 (2026-10-27, after the
   safety/CM patch by the owner-directed correction): exit 2 — identical
   fail-closed wording. Transcript:
   `renode_xw2_precheck_transcript_r2.log`. The invocation-2 window was
   observed with `docker events`: the only activity was an unrelated running
   container's internal exec (`netbird-crowdsec`); no image pull, no create
   start, no renode container appeared.

The runner, `tools/scripts/renode_xw2_vv.sh`, is digest-anchored and fails
closed when the image is absent, so a re-authorized pull immediately enables:

```
bash tools/scripts/renode_xw2_vv.sh
```

The runner's Renode execution is explicitly local
(`docker run -d --pull=never --network=none`) with a UNIQUE per-run container
name (`frick-xw2-vv-<label>-<UTC-timestamp>-<random>`), checked free before
start; a collision fails closed without removing anything; only the container
the runner itself created, only after it exits, is removed, and a still-running
own container is never stopped/removed (its name is recorded for manual
disposition).

## 4. Loop artefacts retained in the tree (hashed in `assets.sha256`)

| Artefact | Purpose |
| --- | --- |
| `tools/scripts/renode_xw2_vv.sh` | fail-closed runner: toolchain build (cached image, no network), ELF/nonce/image identity binding, nm wrapper-segregation probe, Renode positive run + console capture, existing `target_req_verify.py`/`target_run_verdict.py` oracles against `xw2_manifest.json`, no-wrap expected-failure control, composite verdict |
| `tools/docker/renode/frick-xw2-renode.resc` | Resc for the positive XW-2 ELF (same BeagleV-Fire platform/boot mechanics as the production resc; no device model invented) |
| `tools/docker/renode/frick-xw2-nowrap-renode.resc` | Resc for the un-wrapped control ELF (expected watchdog-init panic) |
| `build/xw2-renode/frick_fsw_xw2` | MPFS RTEMS positive ELF (sha256 `1a80e6e9…`), entry `0x1000000000` |
| `build/xw2-renode/frick_fsw_xw2_nowrap` | MPFS RTEMS control ELF (sha256 `b5a16667…`) |
| `xw2-build-renode.log` | full cached-toolchain cross-build transcript (exit 0; both test ELFs + flight-shaped `frick_fsw` linked, 304 steps) |
| `nm-segregation-renode.txt` | wrapper segregation for the MPFS build: XW image 3 wrapper call sites / 0 real-hook call sites; flight-shaped image 0 wrappers / 3 real-hook call sites |
| `renode_xw2_run_transcript.log` | the fail-closed runner transcript (invocation 1, exit 2, no pull attempted) |
| `renode_xw2_precheck_transcript_r2.log` | the fail-closed runner transcript after the safety/CM patch (invocation 2, exit 2, no pull attempted) |

## 5. Explicit non-claims

No N18 result was produced: the positive image never ran under Renode, no
console was captured, and no oracle verdict exists. The QEMU XW-2 result
(T33, 13/13) remains a QEMU-scoped record. Nothing is closed: NC-PAL-01 and
NC-EXT-198 stand unchanged; no new NC is registered here; the XW-2-Renode gate
is NOT PASSED and NOT FAILED — it is NOT EXECUTED (blocked).

## History

| Rev | Date | Author | Change |
| --- | ---- | ------ | ------ |
| 0.1 | 2026-10-27 | frick-fsw | Verbatim-labelled session record of the authorized pull, its verification, the unexplained post-pull removal, the fail-closed stop and the retained artefacts |
| 0.2 | 2026-10-27 | frick-fsw | Owner-directed safety/CM correction: the patched runner uses `--pull=never --network=none` and a unique per-run container name (`frick-xw2-vv-<label>-<UTC>-<random>`) checked free before start with a fail-closed collision guard; only the runner-created container, only after exit, is removed, and a still-running own container is never stopped/removed; §3 corrected to state that the only run attempts are the two fail-closed prerequisite prechecks (invocation 1 `renode_xw2_run_transcript.log`, invocation 2 `renode_xw2_precheck_transcript_r2.log` with its observed docker-events window); no pull, no run, at any point |
| 0.3 | 2026-10-27 | frick-fsw | Evidence-wording precision (owner-directed): replace the "removed by an unidentified external actor" statement with the exact factual form — the image was absent from the local Docker image store after the verified pull and before any run; the removal/disappearance cause and actor are unconfirmed and no source was identified by diagnostics; no external actor is asserted or inferred |
| 0.4 | 2026-10-27 | frick-fsw | Second owner-authorized pull (2026-10-27): exact digest `antmicro/renode@sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061` pulled and verified; local tag `frick-fsw-vv/renode:1.16.1.16973` created; both resolve to image ID `sha256:e278e663fd9516284f1ed3c1da5e7ff727db3bd9808b44a91e4325ffa1a0b858`, size 908150693 bytes. XW-2 executed 20261027T232000Z: 12/13 component PASS, XW-B1 missing (cause undetermined; 60s RESC wait did not yield post-boot feeder observation); whole-run PASS (no fatal markers), aggregate FAIL; no-wrap expected-failure control reproduced (watchdog-init panic, component verifier exit 1). N18 criterion NOT SATISFIED. T37 remains NOT QUALIFIED. First-pull disappearance cause/actor finding (rev 0.3) unchanged: cause/actor unconfirmed, no external actor asserted or inferred. |
