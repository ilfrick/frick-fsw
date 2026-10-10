#!/usr/bin/env python3
"""frick-fsw - gcov line coverage collector (norm N10, qualified tool).

Usage:
  python3 tools/scripts/coverage.py [build-dir]

Expects a CMake build configured with -DFF_ENABLE_COVERAGE=ON and its ctest suite
already executed. Runs gcov over every .gcda, parses the .gcov reports and writes
<build-dir>/coverage.json consumed by tools/scripts/metrics.py.

Pure stdlib, runs on the host or in the pinned V&V image.
"""
import glob
import json
import os
import re
import subprocess
import sys

EXECUTABLE = ("-",)          # gcov marker for non-executable lines
NOT_COVERED = {"#####", "=====", "$$$$$"}


def main():
    build = sys.argv[1] if len(sys.argv) > 1 else "build/coverage"
    gcda = glob.glob(os.path.join(build, "**", "*.gcda"), recursive=True)
    if not gcda:
        print(f"error: no .gcda files under {build} (build with FF_ENABLE_COVERAGE=ON "
              f"and run ctest first)", file=sys.stderr)
        return 1

    outdir = os.path.join(build, "gcov")
    os.makedirs(outdir, exist_ok=True)
    nopipe = subprocess.DEVNULL
    for f in gcda:
        subprocess.run(["gcov", "-b", "-c", "-p", "-o", os.path.abspath(os.path.dirname(f)),
                        os.path.abspath(f)],
                       cwd=outdir, stdout=nopipe, stderr=nopipe)

    lines_total = lines_covered = 0
    funcs_total = funcs_covered = 0
    for gcov in glob.glob(os.path.join(outdir, "*.gcov")):
        for line in open(gcov, encoding="utf-8", errors="ignore"):
            fm = re.match(r"^\s*function\s+\S+\s+called\s+(\d+)", line)
            if fm:
                funcs_total += 1
                if int(fm.group(1)) > 0:
                    funcs_covered += 1
                continue
            m = re.match(r"^\s*([^:]+):\s*(\d+):", line)
            if not m:
                continue
            token = m.group(1).strip()
            if token == "-":
                continue
            lines_total += 1
            if token not in NOT_COVERED and token.isdigit() and int(token) > 0:
                lines_covered += 1

    report = {
        "lines_total": lines_total,
        "lines_covered": lines_covered,
        "funcs_total": funcs_total,
        "funcs_covered": funcs_covered,
        "source": "gcov (make coverage)",
    }
    with open(os.path.join(build, "coverage.json"), "w", encoding="utf-8") as fh:
        json.dump(report, fh, indent=2)
    pct = 100.0 * lines_covered / lines_total if lines_total else 0.0
    print(f"line coverage: {lines_covered}/{lines_total} ({pct:.1f}%) "
          f"- report: {build}/coverage.json")
    return 0


if __name__ == "__main__":
    sys.exit(main())