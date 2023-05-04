FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive

WORKDIR /app

RUN apt update && apt install -y wget make

# Install recent CMake
RUN if [ $(uname -m) = "x86_64" ]; then export ARCH=x86_64; else export ARCH=aarch64; fi && \
    mkdir -p /opt/cmake && \
    cd /opt/cmake && \
    wget https://github.com/Kitware/CMake/releases/download/v3.21.3/cmake-3.21.3-linux-$ARCH.sh  && \
    sh cmake-3.21.3-linux-$ARCH.sh --prefix=/opt/cmake --skip-license && \
    ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake && \
    rm /opt/cmake/cmake-3.21.3-linux-$ARCH.sh

# GCC ARM
RUN if [ $(uname -m) = "x86_64" ]; then export ARCH=x86_64; else export ARCH=aarch64; fi && \
    mkdir -p /opt/gcc && \
    cd /opt/gcc && \
    wget https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-$ARCH-linux.tar.bz2 && \
    tar xjf gcc-arm-none-eabi-10.3-2021.10-$ARCH-linux.tar.bz2 && \
    rm -rf /opt/gcc/*.tar.bz2

ENV PATH="/opt/gcc/gcc-arm-none-eabi-10.3-2021.10/bin:${PATH}"
