# frick-fsw - convenience wrapper around CMake
#
# Full gates (norm N15) are aggregated by `make check`; `make ci-local` runs
# the same cycle inside the pinned Docker V&V image (norm N6).
#
# SPDX-License-Identifier: AGPL-3.0-only

BUILD_DIR ?= build/native
CMAKE ?= cmake

.PHONY: all configure build test run check ci-local verify-rtems verify-lean \
        trace format docs doxygen rtems docker-image clean smoke gsw-selfcheck

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

# RTEMS 6.2 cross build (requires the RTEMS toolchain in /opt/rtems).
rtems:
	$(CMAKE) -S . -B build/rtems -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/rtems-riscv-mpfs.cmake \
	  -DFF_PLATFORM=rtems
	$(CMAKE) --build build/rtems -j$(shell nproc)

verify-rtems:
	bash tools/scripts/ci_rtems_cross_build.sh

clean:
	rm -rf build

format:
	clang-format -i src config tests

docs:
	doxygen Doxyfile