# frick-fsw - Renode V&V image (pinned)
#
# Wraps the official Antmicro Renode image (pinned by digest) and adds the
# frick-fsw PolarFire SoC V&V scripts. See docs/vv/renode-environment.md.
#
# Build:  docker build -t frick-renode:local tools/docker/renode
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0
FROM antmicro/renode@sha256:7905c3295be5b35a934ddb353e43e47e64c073b97da248d378b13c664b9fb061

COPY renode_env.resc /opt/frick/renode_env.resc
COPY frick-renode.resc /opt/frick/frick-renode.resc
