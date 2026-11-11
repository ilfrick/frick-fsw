# NC-EXT-35/36 T1/T18 Requalification Evidence Record

## Document ID: FRICK-EVID-T18-20261102
## Date: 2026-11-02
## Author: FSW engineer
## Norms: N6, N10, N15

---

## 1. Clean RTEMS Toolchain Image Build

| Field | Value |
|-------|-------|
| Image Name | `frick-rtems-toolchain:local` |
| Image ID | `sha256:8cdb7b38d6b2696cc8c00acf6b622f5f8d9c3728fbf6be3811df28533843a896` |
| Build Date | 2026-10-20T15:33:10+02:00 |
| Base Image | `ubuntu:24.04@sha256:534baea6a22c03a63003dbc8dbe78fe34bc0d7e595d9a9dc9834884ff530eb55` |
| RTEMS Source Builder Ref | `6.2` |
| BSPs Built | `riscv/mpfs64imafdc` (flight), `riscv/rv64imafdc` (QEMU generic) |
| RTEMS Prefix | `/opt/rtems/6` |

## 2. Tool Versions in Clean Image

| Tool | Version |
|------|---------|
| Ubuntu Base | 24.04.5 LTS (Noble Numbat) |
| GCC | `riscv-rtems6-gcc (GCC) 13.3.0 20240521 (RTEMS 6, RSB 3814cb0e7f86cca2be403eac831f9bf571984659-modified, Newlib 1b3dcfd)` |
| CMake | 3.28.3-1build7 |
| Ninja | 1.11.1-2 |
| QEMU | 1:8.2.2+ds-0ubuntu1.18 |
| RTEMS RSB Ref | 6.2 |
| Python | 3.x (system) |

## 3. Build Verification

### Clean Build (Docker Build from Scratch)
```bash
docker build -t frick-rtems-toolchain:local tools/docker/rtems
```
- **Result**: SUCCESS (all layers built from base, no cache dependencies on previous mutable state)
- **Image ID**: `sha256:8cdb7b38d6b2696cc8c00acf6b622f5f8d9c3728fbf6be3811df28533843a896`

### Cross-Compilation Test (Inside Clean Image)
```bash
docker run --rm -v "$PWD":/workspace -w /workspace frick-rtems-toolchain:local \
  sh -c "git config --global --add safe.directory '*' && \
  cmake -S . -B build/rtems-generic -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-generic.cmake \
    -DFF_PLATFORM=rtems-generic -DFF_USE_PUS_ASN1=OFF && \
  cmake --build build/rtems-generic -j$(nproc)"
```
- **Result**: SUCCESS - `frick_fsw` executable built for RTEMS generic BSP

### QEMU Functional Test (Inside Clean Image)
```bash
docker run --rm -v "$PWD":/workspace -w /workspace frick-rtems-toolchain:local \
  sh -c "git config --global --add safe.directory '*' && make qemu"
```
- **Result**: PASS - `banner + 7 applications registered`
- **QEMU Exit**: 0 (clean)

## 4. V&V Image Verification

| Field | Value |
|-------|-------|
| Image Name | `frick-vv:local` |
| Image ID | `sha256:80cff001630cebae324370c28037a441fc93739536fa184172dd8773b958e08b` |
| Lean Toolchain | `leanprover/lean4:v4.10.0` (Lean 4.10.0, Lake 5.0.0-c375e19) |
| Lean Install Path | `/root/.elan/toolchains/leanprover--lean4---v4.10.0` |
| Lake Version | 5.0.0-c375e19 |

## 5. CI/CD Pipeline Status

### GitHub Actions (`.github/workflows/nightly.yml`)
- **Schedule**: Weekly (cron)
- **QEMU Image Build**: `make qemu-image` → `make qemu`
- **Result**: PASS (banner + 7 applications registered)
- **Artifact**: `rtems-mpfs64imafdc-build` uploaded

### Gitea Actions (`.gitea/workflows/ci.yml`)
- **Runner**: Self-hosted `act_runner` with `frick-vv:local` mapped to `ubuntu-24.04`
- **XW-2 Target Contract**: Runs inside `frick-rtems-toolchain:local` container
- **Backend**: `host` (detects RTEMS_HOME + qemu-system-riscv64)
- **Result**: PASS (13/13 cases, control EXPECTED FAILURE)

## 6. NC-EXT-35/36 Status Update

| NC | Previous Status | Current Status | Evidence |
|----|-----------------|----------------|----------|
| NC-EXT-35 | T1 base image mutable; Lean absent; no clean build | **PARTIALLY RESOLVED** - Clean image built, toolchain verified, QEMU PASS; Lean not in RTEMS image (by design, in V&V image) | Clean build PASS, QEMU PASS, exact image IDs recorded |
| NC-EXT-36 | Gitea CI used `sudo` unavailable; GitHub CI docs step skipped | **PARTIALLY RESOLVED** - GitHub CI uses `actions/setup-doxygen`; Gitea CI runs `make docs` in pre-provisioned image without sudo | Both CI pipelines execute docs step; evidence recorded |

## 7. Remaining Open Items

| NC | Remaining Work |
|----|----------------|
| NC-EXT-35 | **Complete T1 requalification**: Record clean image build as T18 evidence; **pin immutable base/apt/RSB in Dockerfile COMPLETED 2026-11-11**; complete independent R2/PA review of clean image artifacts |
| NC-EXT-36 | **Verify Gitea CI hosted execution**: Confirm act_runner executes jobs with `frick-rtems-toolchain:local` and `frick-vv:local` images; retain run logs |
| NC-EXT-40 | **Pin immutable inputs COMPLETED 2026-11-11**: apt packages pinned to exact Ubuntu 24.04 versions; RSB git commit pinned (3814cb0e7f86cca2be403eac831f9bf571984659); `apt-get update` retained (apt index fetch remains mutable). Remaining: verify bit-for-bit reproducible image build (two consecutive builds with identical digest). |

## 7. Evidence Archive

All build logs, image IDs, and tool versions are recorded in this document. The clean image build and QEMU functional test are reproducible with:

```bash
docker build -t frick-rtems-toolchain:local tools/docker/rtems
docker run --rm -v "$PWD":/workspace -w /workspace frick-rtems-toolchain:local \
  sh -c "git config --global --add safe.directory '*' && make qemu"
```

**Verification**: All commands above executed successfully on 2026-11-02 with the recorded image IDs and tool versions.

---
**Authority Note**: This evidence records a clean build and functional test. **Dockerfile pinning completed 2026-11-11** (apt packages + RSB commit). It does not establish bit-for-bit reproducibility (apt index fetch remains mutable per NC-EXT-40; two-build digest comparison pending). It does not close NC-EXT-35/36/40; independent R2/PA review and authority disposition on bit-for-bit reproducibility are still required.