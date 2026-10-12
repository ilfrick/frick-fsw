# frick-fsw - convenience wrapper around CMake
#
# Full gates (norm N15) are aggregated by `make check`; `make ci-local` runs
# the same cycle inside the pinned Docker V&V image (norm N6).
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: AGPL-3.0-only

BUILD_DIR ?= build/native
CMAKE ?= cmake

.PHONY: all configure build test run check ci-local verify-rtems verify-lean \
        trace format docs doxygen rtems docker-image clean smoke gsw-selfcheck \
        coverage metrics coverage-configure misra misra-docker emulated renode renode-image renode-env rtems-toolchain rtems-docker pus-asn1 qemu qemu-image qemu-image-docker

all: build

configure:
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug

build: configure
	$(CMAKE) --build $(BUILD_DIR) -j$(shell nproc)

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

run: build
	./$(BUILD_DIR)/frick_fsw

smoke:
	bash tools/scripts/run_sim.sh

gsw-selfcheck:
	python3 tools/gsw/ff_gsw.py --ping

# Formal verification with Lean4 / lake (installed via elan).
verify-lean:
	cd verification/lean && lake build

# Traceability convergence check (rules N1..N4).
trace:
	python3 tools/scripts/trace.py docs/requirements/traceability.yaml

# MISRA C:2012 scan (open-source cppcheck addon; advisory). See docs/quality/misra-c.md.
misra:
	bash tools/scripts/misra.sh

# Containerised, reproducible MISRA scan (advisory, T14): runs the same script as
# CI in a pinned base image where cppcheck is installed, so build/misra-report.txt
# can be regenerated from the tree (see docs/vv/tool-qualification.md, T14). The
# certified checker remains GAP-A4.
misra-docker:
	docker run --rm -v "$(CURDIR)":/w -w /w ubuntu:24.04 bash -lc \
	  'apt-get update -qq && DEBIAN_FRONTEND=noninteractive apt-get install -y -qq cppcheck >/dev/null && bash /w/tools/scripts/misra.sh'

# Emulated-target verification (Renode PolarFire SoC / QEMU); advisory.
emulated:
	bash tools/scripts/emulated_run.sh

# Renode functional validation (norm N18); SKIP proxy when the RTEMS image is absent.
renode:
	bash tools/scripts/renode_vv.sh

# Renode environment self-test (norm N18): image runs + polarfire-soc platform loads.
renode-env:
	bash tools/scripts/renode_env_selftest.sh

# Complete local development & verification cycle (norm N15).
check: build test verify-lean trace smoke gsw-selfcheck
	@echo "frick-fsw: full local cycle PASS (norm N15)"

# Same cycle inside the pinned Docker V&V image (norm N6/N8, T1).
ci-local: docker-image
	docker run --rm -v $(CURDIR):/workspace -w /workspace frick-vv:local \
	  bash -lc 'cmake -S . -B /workspace/build/ci -G Ninja -DCMAKE_BUILD_TYPE=Debug \
	    && cmake --build /workspace/build/ci \
	    && ctest --test-dir /workspace/build/ci --output-on-failure \
	    && cd /workspace/verification/lean && lake build \
	    && cd /workspace && python3 tools/scripts/trace.py docs/requirements/traceability.yaml'

docker-image:
	docker build -t frick-vv:local -f tools/docker/Dockerfile tools/docker

# RTEMS 6.2 cross build (open-source RTEMS; requires the toolchain in /opt/rtems,
# installed by tools/scripts/setup_rtems.sh).
rtems:
	$(CMAKE) -S . -B build/rtems -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-mpfs.cmake \
	  -DFF_PLATFORM=rtems
	$(CMAKE) --build build/rtems -j$(shell nproc)

# Reproducible, containerised open-source RTEMS 6.2 toolchain (RTEMS project).
rtems-toolchain:
	docker build -t frick-rtems-toolchain:local tools/docker/rtems

# Cross build inside the toolchain image (no host toolchain needed).
rtems-docker:
	docker run --rm -v $(CURDIR):/workspace -w /workspace frick-rtems-toolchain:local \
	  bash -lc 'cmake -S . -B build/rtems -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-mpfs.cmake -DFF_PLATFORM=rtems && cmake --build build/rtems -j$$(nproc)'

verify-rtems:
	bash tools/scripts/ci_rtems_cross_build.sh

# Generate C from the N7/ESA asn1-pusc-lib ASN.1/ACN PUS-C model (needs Docker).
pus-asn1:
	bash tools/scripts/gen_pus_asn1.sh

# Build the RTEMS image for Renode functional validation (norm N18). Requires the
# RTEMS toolchain in /opt/rtems 6; output build/renode/frick_fsw_renode.elf.
renode-image:
	$(CMAKE) -S . -B build/renode -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-mpfs.cmake \
	  -DFF_PLATFORM=rtems -DFF_ENABLE_RENODE_TEST=ON
	$(CMAKE) --build build/renode -j$(shell nproc)
	cp build/renode/frick_fsw build/renode/frick_fsw_renode.elf

# Build the generic-BSP RTEMS image for QEMU `virt` emulated-target verification
# (milestone M10 / SRS-REQ-043). Requires the RTEMS toolchain with BSP
# rv64imafdc in /opt/rtems 6; output build/rtems-generic/frick_fsw.
qemu-image:
	$(CMAKE) -S . -B build/rtems-generic -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-generic.cmake \
	  -DFF_PLATFORM=rtems-generic
	$(CMAKE) --build build/rtems-generic -j$(shell nproc)

# Same, containerised (no host toolchain; the image has cmake + qemu + the BSP).
qemu-image-docker:
	docker run --rm -v $(CURDIR):/workspace -w /workspace frick-rtems-toolchain:local \
	  bash -lc 'cmake -S . -B build/rtems-generic -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-generic.cmake -DFF_PLATFORM=rtems-generic && cmake --build build/rtems-generic -j$$(nproc)'

# QEMU emulated-target verification (milestone M10, SRS-REQ-043); SKIP proxy when
# the generic image is absent.
qemu:
	bash tools/scripts/qemu_vv.sh

clean:
	rm -rf build

format:
	clang-format -i src config tests

docs:
	doxygen Doxyfile

coverage-configure:
	$(CMAKE) -S . -B build/coverage -G Ninja -DCMAKE_BUILD_TYPE=Debug -DFF_ENABLE_COVERAGE=ON

coverage: coverage-configure
	$(CMAKE) --build build/coverage -j$(shell nproc)
	ctest --test-dir build/coverage --output-on-failure
	python3 tools/scripts/coverage.py build/coverage

metrics: coverage
	python3 tools/scripts/metrics.py --readme
	python3 tools/scripts/metrics.py
