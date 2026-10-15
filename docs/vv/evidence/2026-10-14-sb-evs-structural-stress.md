# SB/EVS structural stress evidence — 2026-10-14

## Scope and status

Exploratory SIL stress of the configured SB/EVS structural boundaries. This is
not an approved mission event-rate profile and does not establish WCET, target
timing, ground receipt, SRS-REQ-072 compliance, NC closure, or release readiness.

## Configuration and artifact identity

- Repository HEAD: `57974d297ab8a1e295258d1f3054bbbe2dd0f323`; working tree dirty.
- Fresh build directory: `build/stress-clean` (configured from the working tree).
- Configure/build: `cmake -S . -B build/stress-clean -G Ninja -DCMAKE_BUILD_TYPE=Debug`
  then `cmake --build build/stress-clean` (114 build steps).
- Test command: `ctest --test-dir build/stress-clean --output-on-failure`.
- `test_sb_stress` direct result: `checks=454 failures=0`.
- Full fresh CTest result: **28/28 passed**.
- V&V image ID: `sha256:5cbc72aa4125cfdbd8c0baa4ac2a1cb9d1447ff3147ac0b94e2ed0d918b83e91`.
- Selected artifact SHA-256 values:
  - `tests/unit/test_sb_stress.c`: `0afa21a0c5830730bd0d60e737201de9f5d4e31c7dc5772476cdc4334970c0fa`
  - `build/stress-clean/tests/test_sb_stress`: `bee7882506c2128fe4463b0b69858e575cae5c7818ed9353c1f2e59131a1905c`
  - `build/stress-clean/libfrick_fsw_core.a`: `e13588a87fc5db750635f602e4a9742c152ee3a1e3492e804d3ac63f5e4e5535`

These hashes identify selected inputs/artifacts, not a complete immutable
build manifest. The V&V image's Dockerfile uses mutable upstream base/package
inputs; no hosted CI or RTEMS cross-build was run in this evidence cycle.

## Stress profile and expected/observed results

The profile uses configured values from `config/fsw_config.h`: 32 SB pipes,
queue depth 32, and EVS log/deferred-notification depth 64.

1. Hold all 32 matched subscriber queue mutexes and publish once: `FF_TIMEOUT`,
   0 delivered copies, 32 SB drops, and 32 deferred busy reports. Release all
   queue locks and explicitly dispatch: 32 callbacks.
2. Publish 32 messages to 32 wildcard-matched pipes: 1,024 copies accepted,
   with no additional SB drops.
3. Publish three more messages to the now-full pipes using distinct message IDs
   `0x0B00`, `0x0B01`, and `0x0B02`: 96 more SB drops and EVS log records. EVS
   accepts the first 64 new notifications and counts the remaining 32 as
   notification drops. Together with the first phase, 96 notifications are
   queued/dispatched and 32 are dropped.
4. EVS log total is 128 records (32 busy reports + 96 full-queue reports). The
   circular log returns the newest 64: application IDs 100–131 in order for
   burst `0x0B01`, then 100–131 in order for burst `0x0B02`.

All asserted counts and suffix identifiers passed in the fresh test. This
characterizes a finite-storage saturation boundary under deterministic lock
injection; it is not a time-based workload, concurrency/WCET measurement, or
ground-link test.

## Other cycle evidence

- `make ci-local`: PASS (28/28 CTest, Lean build, trace 117 requirements / 51
  Lean theorems).
- `make smoke`: PASS (six applications registered).
- `make renode`: PASS (`RENODE_TEST: PASS`); aggregate marker only, not event
  envelope coverage.
- `clang-format --dry-run --Werror` on changed SB tests and `git diff --check`:
  PASS.
- Containerized `make docs`: PASS with existing repository-wide Doxygen warnings.
- RTEMS cross-build and hosted forge CI: NOT RUN.

## Test-design limitation retained

An early attempt put the configured-maximum stress case into the existing
`test_sb` process and failed because previous cases leave PAL queue slots
allocated across service reinitialization. The test was isolated in its own
CTest process; the clean build and complete CTest run then passed. The initial
failure was observed during this work but its raw console transcript was not
saved; its cause and correction are recorded here and in the verification log.
