# frick-fsw - ASN1SCC compiler image (ESA ASN.1/ACN compiler)
#
# Builds the ESA ASN1SCC compiler (GPL-3.0 with runtime exception; the generated
# code and the runtime are NOT GPL-restricted) from a pinned commit, so the
# N7/ESA asn1-pusc-lib ASN.1/ACN model can be compiled to C for the PUS-C
# default backend. See docs/status/loop-audit.md and docs/tooling.md.
#
# Build:  docker build -t frick-asn1scc:local tools/docker/asn1scc
# Use:    tools/scripts/gen_pus_asn1.sh
#
# Copyright (C) 2026 Ilfrick
# https://github.com/ilfrick/frick-fsw | https://www.housefz.com/git/ilfrick/frick-fsw
# SPDX-License-Identifier: MPL-2.0
FROM ubuntu:22.04

ARG ASN1SCC_REPO=https://github.com/esa/asn1scc.git
ARG ASN1SCC_REF=3124590a1d57d7d7db97b8850a5e2687ef46d0c2

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      wget ca-certificates git make openjdk-17-jre-headless \
      libicu70 libssl3 libstdc++6 zlib1g libgssapi-krb5-2 \
    && rm -rf /var/lib/apt/lists/*

# .NET 10 SDK (Ubuntu 22.04 apt does not ship it: install via dotnet-install.sh)
RUN wget -q https://dot.net/v1/dotnet-install.sh -O /tmp/dotnet-install.sh \
    && bash /tmp/dotnet-install.sh --channel 10.0 --install-dir /usr/share/dotnet \
    && ln -s /usr/share/dotnet/dotnet /usr/bin/dotnet \
    && rm /tmp/dotnet-install.sh
ENV DOTNET_ROOT=/usr/share/dotnet
ENV DOTNET_CLI_TELEMETRY_OPTOUT=1

RUN git clone "${ASN1SCC_REPO}" /asn1scc-src \
    && git -C /asn1scc-src checkout "${ASN1SCC_REF}"
WORKDIR /asn1scc-src
RUN dotnet build -c Release Antlr/ \
    && dotnet build -c Release "asn1scc.sln" \
    && dotnet publish -c Release --self-contained true -r linux-x64 asn1scc.sln -o /opt/asn1scc

ENV PATH="/opt/asn1scc:${PATH}"
ENTRYPOINT []
CMD ["asn1scc", "--help"]
