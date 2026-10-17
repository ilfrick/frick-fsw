# frick-fsw - convenience wrapper around CMake
#
# `make check` is a local check bundle (build/test/Lean/trace/smoke/GSW), not the
# complete N15 verification cycle. `make ci-local` runs configure/build/test/
# Lean/trace in the controlled Docker image. Separate target families include
# static analysis, format, smoke/GSW, coverage/metrics, RTEMS build/verification,
# Renode environment/image/run, QEMU, PUS ASN.1 generation, and docs. This is an
# orientation list, not exhaustive; applicability is defined by
# `docs/vv/verification-plan.md`. Findings NC-EXT-35/38 remain open.
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0

BUILD_DIR ?= build/native
CMAKE ?= cmake

.PHONY: all fetch-third-party configure build test run check ci-local verify-rtems verify-lean sb-evs-sweep \
        trace format format-check docs doxygen rtems docker-image clean smoke gsw-selfcheck \
        coverage metrics coverage-configure misra misra-docker emulated renode renode-image renode-env rtems-toolchain rtems-docker pus-asn1 qemu qemu-image qemu-image-docker

all: build

fetch-third-party:
	bash tools/scripts/fetch_third_party.sh all

configure: pus-asn1
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -DFF_USE_PUS_ASN1=ON

build: configure
	$(CMAKE) --build $(BUILD_DIR) -j$(shell nproc)

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

# Exploratory SB/EVS structural parameter matrix; not a mission-rate or WCET claim.
sb-evs-sweep: build
	python3 tools/scripts/sb_evs_structural_sweep.py ./$(BUILD_DIR)/tests/test_sb_sweep

run: build
	./$(BUILD_DIR)/frick_fsw

smoke: pus-asn1
	bash tools/scripts/run_sim.sh

gsw-selfcheck:
	python3 tests/python/test_ff_gsw.py
	python3 tests/python/test_metrics.py
	python3 tests/python/test_license_check.py
	python3 tools/scripts/license_check.py
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

# Containerised MISRA scan (advisory, T14): installs cppcheck from the current
# Ubuntu package repositories and runs the shared script. Checker/package input
# identity is not pinned; see the T14 qualification record. Certified checker
# remains GAP-A4.
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

# Local check bundle; it is not the complete norm-N15 gate set. See the file
# header and verification plan for separate applicable targets.
check: build test verify-lean trace smoke gsw-selfcheck
	@echo "frick-fsw: local check bundle PASS; other N15 gates are separate"

# Configure/build/test/Lean/trace subset inside controlled Docker image (norm N6/N8, T1).
ci-local: pus-asn1 docker-image
	docker run --rm -v $(CURDIR):/workspace -w /workspace frick-vv:local \
	  bash -lc 'cmake -S . -B /workspace/build/ci -G Ninja -DCMAKE_BUILD_TYPE=Debug \
	    -DFF_USE_PUS_ASN1=ON \
	    && cmake --build /workspace/build/ci \
	    && ctest --test-dir /workspace/build/ci --output-on-failure \
	    && cd /workspace/verification/lean && lake build \
	    && cd /workspace && python3 tools/scripts/trace.py docs/requirements/traceability.yaml'

docker-image:
	docker build -t frick-vv:local -f tools/docker/Dockerfile tools/docker

# RTEMS 6.2 cross build (open-source RTEMS; requires the toolchain in /opt/rtems,
# installed by tools/scripts/setup_rtems.sh).
rtems: pus-asn1
	$(CMAKE) -S . -B build/rtems -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-mpfs.cmake \
	  -DFF_PLATFORM=rtems -DFF_USE_PUS_ASN1=ON
	$(CMAKE) --build build/rtems -j$(shell nproc)

# Docker-based open-source RTEMS 6.2 toolchain build; mutable apt/RSB inputs do
# not establish bit-for-bit reproducibility (NC-EXT-40).
rtems-toolchain:
	docker build -t frick-rtems-toolchain:local tools/docker/rtems

# Cross build inside the toolchain image (no host toolchain needed).
rtems-docker: pus-asn1
	docker run --rm -v $(CURDIR):/workspace -w /workspace frick-rtems-toolchain:local \
	  bash -lc 'cmake -S . -B build/rtems -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-mpfs.cmake -DFF_PLATFORM=rtems -DFF_USE_PUS_ASN1=ON && cmake --build build/rtems -j$$(nproc)'

verify-rtems: pus-asn1
	bash tools/scripts/ci_rtems_cross_build.sh

# Generate C from the N7/ESA asn1-pusc-lib ASN.1/ACN PUS-C model (needs Docker).
pus-asn1: fetch-third-party
	bash tools/scripts/gen_pus_asn1.sh

# Build the RTEMS image for Renode functional validation (norm N18). Requires the
# RTEMS toolchain in /opt/rtems 6; output build/renode/frick_fsw_renode.elf.
renode-image: pus-asn1
	$(CMAKE) -S . -B build/renode -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-mpfs.cmake \
	  -DFF_PLATFORM=rtems -DFF_ENABLE_RENODE_TEST=ON -DFF_USE_PUS_ASN1=ON
	$(CMAKE) --build build/renode -j$(shell nproc)
	cp build/renode/frick_fsw build/renode/frick_fsw_renode.elf

# Build the generic-BSP RTEMS image for QEMU `virt` emulated-target verification
# (milestone M10 / SRS-REQ-043). Requires the RTEMS toolchain with BSP
# rv64imafdc in /opt/rtems 6; output build/rtems-generic/frick_fsw.
qemu-image: pus-asn1
	$(CMAKE) -S . -B build/rtems-generic -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-generic.cmake \
	  -DFF_PLATFORM=rtems-generic -DFF_USE_PUS_ASN1=ON
	$(CMAKE) --build build/rtems-generic -j$(shell nproc)

# Same, containerised (no host toolchain; the image has cmake + qemu + the BSP).
qemu-image-docker: pus-asn1
	docker run --rm -v $(CURDIR):/workspace -w /workspace frick-rtems-toolchain:local \
	  bash -lc 'cmake -S . -B build/rtems-generic -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-generic.cmake -DFF_PLATFORM=rtems-generic && cmake --build build/rtems-generic -j$$(nproc)'

# QEMU emulated-target verification (milestone M10, SRS-REQ-043); SKIP proxy when
# the generic image is absent.
qemu:
	bash tools/scripts/qemu_vv.sh

clean:
	rm -rf build

# Formatting is pinned to the clang-format major version used by the CI runner
# (ubuntu-24.04 ships 18.x). clang-format >= 19 lays out single-line macro
# definitions differently, so formatting with a newer host version silently
# breaks the CI "Format check" gate. Run 'make format' inside the V&V image or
# set FF_CLANG_FORMAT_ALLOW_UNPINNED=1 to override deliberately.
FF_CLANG_FORMAT_MAJOR ?= 18

define ff_clang_format_guard
	@v="$$(clang-format --version 2>/dev/null | grep -oE '[0-9]+' | head -1)"; \
	if [ -z "$$v" ]; then echo "error: clang-format not found on PATH"; exit 1; fi; \
	if [ "$$v" != "$(FF_CLANG_FORMAT_MAJOR)" ] && [ -z "$(FF_CLANG_FORMAT_ALLOW_UNPINNED)" ]; then \
	  echo "error: clang-format $$v != pinned major $(FF_CLANG_FORMAT_MAJOR) (CI runner uses 18.x)"; \
	  echo "       use the V&V image (docker run --rm -v \$$PWD:/workspace -w /workspace frick-vv:local make format)"; \
	  echo "       or set FF_CLANG_FORMAT_ALLOW_UNPINNED=1 to override"; exit 1; fi
endef

format:
	$(ff_clang_format_guard)
	find src config tests -name '*.[ch]' -print0 | xargs -0 clang-format -i

# Same check as the CI "Format check" step (norm N10; tool T-format).
format-check:
	$(ff_clang_format_guard)
	find src config tests -name '*.[ch]' -print0 | xargs -0 -n1 clang-format --dry-run --Werror

docs:
	rm -rf build/doxygen
	mkdir -p build/doxygen
	doxygen Doxyfile
	test -s build/doxygen/html/index.html || { echo "error: 'make docs' produced no build/doxygen/html/index.html (silent doxygen failure)"; exit 1; }

coverage-configure: pus-asn1
	$(CMAKE) -S . -B build/coverage -G Ninja -DCMAKE_BUILD_TYPE=Debug -DFF_ENABLE_COVERAGE=ON -DFF_USE_PUS_ASN1=ON

coverage: coverage-configure
	$(CMAKE) --build build/coverage -j$(shell nproc)
	ctest --test-dir build/coverage --output-on-failure
	python3 tools/scripts/coverage.py build/coverage

metrics: coverage
	python3 tools/scripts/metrics.py --readme
	python3 tools/scripts/metrics.py
