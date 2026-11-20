# Native build directory mismatch

- Date: 2026-11-15
- Command: `make build`
- Exit: 2, before compilation.
- Exact CMake diagnostic: `The current CMakeCache.txt directory /home/nicola/fsw/frick-fsw/build/native is different than the directory /workspace/build/native where CMakeCache.txt was created. This may result in binaries being created in the wrong place. If you are not sure, reedit the CMakeCache.txt` and `The source "/home/nicola/fsw/frick-fsw/CMakeLists.txt" does not match the source "/workspace/CMakeLists.txt" used to generate cache. Re-run cmake with a different source directory.`
- Cause: pre-existing `build/native` cache belongs to `/workspace`; not a candidate source/build defect.
- The first default `make smoke` attempt hit the same stale-cache mismatch before boot and exited 2; no simulator binary ran in that attempt.
- Corrective action: use the repository-local `build/s1-review` build directory with `BUILD_DIR=build/s1-review make build` and `BUILD_DIR=build/s1-review make smoke`; both passed. The complete test log is retained as `full-host-test.log` and the smoke log as `smoke.log`.
