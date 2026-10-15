# frick-fsw V&V image (norm N6/N8/N10)
#
# Controlled tool image used by host, CI and agent verification activities.
# Base/package inputs are not immutable; reproducibility is not established
# (NC-EXT-35). Qualification register: docs/vv/tool-qualification.md (T1).
#
# SPDX-License-Identifier: MPL-2.0
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=C.UTF-8

RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential gcc g++ make cmake ninja-build \
      python3 python3-pip git curl ca-certificates \
      doxygen graphviz clang clang-format clang-tidy cppcheck \
      nodejs npm \
      pkg-config libc6-dev \
    && rm -rf /var/lib/apt/lists/*

# ---- Lean 4 toolchain (pinned to match verification/lean/lean-toolchain)
RUN curl https://raw.githubusercontent.com/leanprover/elan/master/elan-init.sh -sSf \
      | sh -s -- -y --default-toolchain leanprover/lean4:v4.10.0

ENV PATH="/root/.elan/bin:${PATH}"
ENV ELAN_HOME=/root/.elan

# ---- RTEMS 6.2 toolchain home (host: /opt/rtems; mounted in containers)
ENV RTEMS_HOME=/opt/rtems

WORKDIR /workspace

# Version label for the qualification register
LABEL org.opencontainers.image.title="frick-fsw V&V environment" \
      org.opencontainers.image.version="0.1.0" \
      org.opencontainers.image.description="Controlled tool image for frick-fsw verification; base and package inputs are mutable (NC-EXT-35)"

CMD ["/bin/bash"]
