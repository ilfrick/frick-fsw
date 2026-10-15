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
# Inputs: the base image is digest-pinned, but apt package resolution and the
# RTEMS Source Builder release ref remain mutable; bit-for-bit reproducibility
# is not established (NC-EXT-40).
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0
FROM ubuntu:24.04@sha256:534baea6a22c03a63003dbc8dbe78fe34bc0d7e595d9a9dc9834884ff530eb55

ARG RTEMS_RSB_REF=6.2
ARG RTEMS_BSP=riscv/mpfs64imafdc
# Generic RISC-V BSP used only for the QEMU (virt) emulated-target path (M10).
ARG RTEMS_BSP_QEMU=riscv/rv64imafdc
ARG RTEMS_PREFIX=/opt/rtems/6

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential git python3 python3-dev python3-pip texinfo flex bison \
      libncurses-dev zlib1g-dev pkg-config ca-certificates wget curl \
      bzip2 xz-utils unzip autoconf automake libtool gperf python-is-python3 \
    && rm -rf /var/lib/apt/lists/*

RUN git clone --depth 1 --branch "${RTEMS_RSB_REF}" \
      https://gitlab.rtems.org/rtems/tools/rtems-source-builder.git /rsb

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
      cmake ninja-build qemu-system-misc \
    && rm -rf /var/lib/apt/lists/*
