# frick-fsw - open-source RTEMS 6.2 toolchain image (RTPFS160ZT / mpfs64imafdc)
#
# Builds the RTEMS toolchain + kernel for the Microchip PolarFire SoC BSP from
# the open-source RTEMS project (RTEMS Source Builder + RTEMS sources,
# https://gitlab.rtems.org). No vendor/commercial baseline or pre-built
# toolchain is used.
#
# Build:  make rtems-toolchain   (docker build -t frick-rtems-toolchain:local tools/docker/rtems)
# Use:    make rtems-docker      (cross compile inside this image)
#
# Inputs: the base image is digest-pinned; apt packages and RSB commit are pinned
# for reproducibility (NC-EXT-40 partially resolved; apt index fetch remains mutable).
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0
FROM ubuntu:24.04@sha256:534baea6a22c03a63003dbc8dbe78fe34bc0d7e595d9a9dc9834884ff530eb55

# RSB commit from successful 2026-10-20 build (GCC version string: RSB 3814cb0e7f86cca2be403eac831f9bf571984659-modified)
ARG RTEMS_RSB_COMMIT=3814cb0e7f86cca2be403eac831f9bf571984659
ARG RTEMS_BSP=riscv/mpfs64imafdc
# Generic RISC-V BSP used only for the QEMU (virt) emulated-target path (M10).
ARG RTEMS_BSP_QEMU=riscv/rv64imafdc
ARG RTEMS_PREFIX=/opt/rtems/6

ENV DEBIAN_FRONTEND=noninteractive

# Pin apt packages to exact Ubuntu 24.04 versions (NC-EXT-40)
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential=12.10ubuntu1 \
      git=1:2.43.0-1ubuntu7.4 \
      python3=3.12.3-0ubuntu2.1 \
      python3-dev=3.12.3-0ubuntu2.1 \
      python3-pip=24.0+dfsg-1ubuntu1.3 \
      texinfo=7.1-3build2 \
      flex=2.6.4-8.2build1 \
      bison=2:3.8.2+dfsg-1ubuntu0.24.04.1 \
      libncurses-dev=6.4+20240113-1ubuntu2.2 \
      zlib1g-dev=1:1.3.dfsg-3.1ubuntu2.3 \
      pkg-config=1.8.1-2build1 \
      ca-certificates=20260816ubuntu0.24.04.2 \
      wget=1.21.4-1ubuntu4.5 \
      curl=8.5.0-2ubuntu10.16 \
      bzip2=1.0.8-5.1ubuntu0.1 \
      xz-utils=5.6.1+really5.4.5-1ubuntu0.4 \
      unzip=6.0-28ubuntu4.1 \
      autoconf=2.71-3 \
      automake=1:1.16.5-1.3ubuntu1 \
      libtool=2.4.7-7build1 \
      gperf=3.1-1build1 \
      python-is-python3=3.11.4-1 \
    && rm -rf /var/lib/apt/lists/*

RUN git clone https://gitlab.rtems.org/rtems/tools/rtems-source-builder.git /rsb \
  && cd /rsb && git checkout "${RTEMS_RSB_COMMIT}"

WORKDIR /rsb/rtems
RUN ../source-builder/sb-set-builder --jobs="$(nproc)" \
      --prefix="${RTEMS_PREFIX}" 6/rtems-riscv.bset \
  && ../source-builder/sb-set-builder --jobs="$(nproc)" \
      --prefix="${RTEMS_PREFIX}" 6/rtems-kernel.bset \
      --with-rtems-bsp="${RTEMS_BSP}" \
  && ../source-builder/sb-set-builder --jobs="$(nproc)" \
      --prefix="${RTEMS_PREFIX}" 6/rtems-kernel.bset \
      --with-rtems-bsp="${RTEMS_BSP_QEMU}"

ENV PATH="${RTEMS_PREFIX}/bin:${PATH}"
ENV RTEMS_HOME=/opt/rtems

# Build tooling (so the cross build runs inside this image: make rtems-docker /
# make qemu-image) and QEMU (so the generic-BSP image can be run on the QEMU
# `virt` machine: make qemu, M10).
RUN apt-get update && apt-get install -y --no-install-recommends \
      cmake=3.28.3-1build7 \
      ninja-build=1.11.1-2 \
      qemu-system-misc=1:8.2.2+ds-0ubuntu1.18 \
    && rm -rf /var/lib/apt/lists/*