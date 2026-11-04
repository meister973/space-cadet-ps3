# PS3 toolchain image for this project. tools/build.ps1 builds it on first use.
# It runs the same script as the Docker-free route, so both use the same toolchain.
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates curl make file libelf1 zlib1g libncurses6 \
    && rm -rf /var/lib/apt/lists/*

COPY install_toolchain.sh /tmp/install_toolchain.sh
RUN sh /tmp/install_toolchain.sh /usr/local/ps3dev && rm /tmp/install_toolchain.sh

ENV PS3DEV=/usr/local/ps3dev
ENV PSL1GHT=/usr/local/ps3dev
ENV PATH=$PATH:/usr/local/ps3dev/bin:/usr/local/ps3dev/ppu/bin:/usr/local/ps3dev/spu/bin
ENV LD_LIBRARY_PATH=/usr/local/ps3dev/cg/lib

WORKDIR /work
