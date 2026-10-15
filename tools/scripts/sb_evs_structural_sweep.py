#!/usr/bin/env python3
# frick-fsw - Software Bus / EVS parameterized structural characterization
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0

"""Run an exploratory SB/EVS structural parameter sweep; not mission assurance."""

from __future__ import annotations

import argparse
import csv
import os
import re
import subprocess
from pathlib import Path


PROFILE = re.compile(
    r"SB_EVS_PROFILE fanout=(\d+) depth=(\d+) overflow_publishes=(\d+) "
    r"overflow_events=(\d+) notify_capacity=(\d+) notify_dropped=(\d+) "
    r"dispatch_sweep=(\d+) dispatch_calls=(\d+) checks=(\d+) failures=(\d+)"
)
CONFIG = re.compile(r"SB_EVS_CONFIG max_pipes=(\d+) max_depth=(\d+) evs_depth=(\d+)")


def describe(executable: Path) -> tuple[int, int, int]:
    result = subprocess.run(
        [str(executable), "--describe"], check=True, capture_output=True, text=True
    )
    match = CONFIG.fullmatch(result.stdout.strip())
    if not match:
        raise RuntimeError(f"unrecognized executable configuration: {result.stdout!r}")
    return tuple(map(int, match.groups()))


def run(executable: Path, *args: str) -> dict[str, int]:
    result = subprocess.run(
        [str(executable), *args], check=True, capture_output=True, text=True
    )
    match = PROFILE.search(result.stdout)
    if not match:
        raise RuntimeError(f"missing profile record for args={args}: {result.stdout!r}")
    row = dict(
        zip(
            (
                "fanout",
                "depth",
                "overflow_publishes",
                "overflow_events",
                "notify_capacity",
                "notify_dropped",
                "dispatch_sweep",
                "dispatch_calls",
                "checks",
                "failures",
            ),
            map(int, match.groups()),
            strict=True,
        )
    )
    return row


def validate_csv(path: Path, expected_profiles: int, capacity: int) -> None:
    with path.open(newline="", encoding="utf-8") as source:
        rows = list(csv.DictReader(source))
    if len(rows) != expected_profiles + 1:
        raise RuntimeError(f"serialized CSV has {len(rows)} rows, expected {expected_profiles + 1}")
    profiles = rows[:-1]
    dispatch = rows[-1]
    seen: set[tuple[int, int, int]] = set()
    for raw in profiles:
        f, d, b = int(raw["fanout"]), int(raw["depth"]), int(raw["overflow_publishes"])
        events = int(raw["overflow_events"])
        queued = min(events, capacity)
        expected = (f, d, b)
        if expected in seen:
            raise RuntimeError(f"duplicate serialized profile: {expected}")
        seen.add(expected)
        if events != f * b or int(raw["notify_dropped"]) != max(0, events - capacity):
            raise RuntimeError(f"serialized accounting mismatch: {raw}")
        if int(raw["notify_capacity"]) != capacity or int(raw["dispatch_calls"]) != queued:
            raise RuntimeError(f"serialized EVS capacity/dispatch mismatch: {raw}")
        if int(raw["failures"]) != 0 or int(raw["checks"]) <= 0:
            raise RuntimeError(f"serialized profile failed: {raw}")
    expected_dispatch = capacity + capacity * (capacity + 1) // 2 + capacity - 1
    if (
        int(dispatch["dispatch_sweep"]) != 1
        or int(dispatch["dispatch_calls"]) != expected_dispatch
        or int(dispatch["failures"]) != 0
        or int(dispatch["checks"]) <= 0
    ):
        raise RuntimeError(f"serialized dispatch record mismatch: {dispatch}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path, help="built test_sb_sweep executable")
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("build/sb-evs-structural-sweep.csv"),
    )
    parser.add_argument("--fanout-max", type=int, default=32)
    parser.add_argument("--depth-max", type=int, default=32)
    parser.add_argument("--evs-depth", type=int, default=64)
    args = parser.parse_args()

    executable = args.executable.resolve()
    if not executable.is_file():
        parser.error(f"test executable does not exist: {executable}")
    max_pipes, max_depth, compiled_evs_depth = describe(executable)
    if not (1 <= args.fanout_max <= max_pipes):
        parser.error(f"--fanout-max must be in 1..{max_pipes}")
    if not (1 <= args.depth_max <= max_depth):
        parser.error(f"--depth-max must be in 1..{max_depth}")
    if args.evs_depth != compiled_evs_depth:
        parser.error(
            f"--evs-depth={args.evs_depth} disagrees with compiled EVS depth "
            f"{compiled_evs_depth}"
        )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    rows: list[dict[str, int]] = []
    for fanout in range(1, args.fanout_max + 1):
        saturation_bursts = (args.evs_depth + fanout) // fanout
        for depth in range(1, args.depth_max + 1):
            for burst_count in range(saturation_bursts + 1):
                row = run(
                    executable, str(fanout), str(depth), str(burst_count)
                )
                expected_events = fanout * burst_count
                expected_queued = min(expected_events, compiled_evs_depth)
                expected_dropped = max(0, expected_events - compiled_evs_depth)
                expected = {
                    "fanout": fanout,
                    "depth": depth,
                    "overflow_publishes": burst_count,
                    "overflow_events": expected_events,
                    "notify_capacity": compiled_evs_depth,
                    "notify_dropped": expected_dropped,
                    "dispatch_sweep": 0,
                    "dispatch_calls": expected_queued,
                }
                if any(row[key] != value for key, value in expected.items()):
                    raise RuntimeError(f"profile disagrees with expected {expected}: {row}")
                if row["failures"] != 0 or row["checks"] <= 0:
                    raise RuntimeError(f"profile assertions failed: {row}")
                rows.append(row)
    expected_profiles = args.depth_max * sum(
        ((args.evs_depth + fanout) // fanout) + 1
        for fanout in range(1, args.fanout_max + 1)
    )
    dispatch_row = run(
        executable,
        str(args.fanout_max),
        str(args.depth_max),
        str((args.evs_depth + args.fanout_max) // args.fanout_max),
        "--dispatch-sweep",
    )
    expected_dispatch_calls = compiled_evs_depth + (
        compiled_evs_depth * (compiled_evs_depth + 1) // 2
    ) + compiled_evs_depth - 1
    if (
        dispatch_row["fanout"] != args.fanout_max
        or dispatch_row["depth"] != args.depth_max
        or dispatch_row["dispatch_sweep"] != 1
        or dispatch_row["dispatch_calls"] != expected_dispatch_calls
        or dispatch_row["failures"] != 0
        or dispatch_row["checks"] <= 0
    ):
        raise RuntimeError(f"dispatch sweep disagrees with expectation: {dispatch_row}")
    rows.append(dispatch_row)

    if len(rows) != expected_profiles + 1:
        raise RuntimeError(f"expected {expected_profiles + 1} rows, got {len(rows)}")
    keys = [(r["fanout"], r["depth"], r["overflow_publishes"]) for r in rows[:-1]]
    if len(set(keys)) != expected_profiles:
        raise RuntimeError("duplicate or missing fanout/depth/burst profile")

    temp_output = args.output.with_name(args.output.name + ".tmp")
    try:
        with temp_output.open("w", newline="", encoding="utf-8") as output:
            writer = csv.DictWriter(output, fieldnames=list(rows[0]))
            writer.writeheader()
            writer.writerows(rows)
            output.flush()
            os.fsync(output.fileno())
        validate_csv(temp_output, expected_profiles, compiled_evs_depth)
        temp_output.replace(args.output)
    except Exception:
        temp_output.unlink(missing_ok=True)
        raise

    print(
        f"SB/EVS structural sweep PASS: {expected_profiles} fanout/depth/burst "
        f"profiles (fanout 1..{args.fanout_max}, depth 1..{args.depth_max}, "
        f"from zero overflow through first EVS saturation), dispatch caps 1..{args.evs_depth}; "
        f"CSV={args.output}"
    )
    print(
        "Scope: configured host structural behavior only; no mission arrival-rate, "
        "WCET, target timing, link availability, or ground-receipt guarantee."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
