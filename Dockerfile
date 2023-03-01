FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive

WORKDIR /app

RUN apt update && apt install -y wget make cmake

# GCC ARM 10.3.1
# x86 url https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2
# aarch64 url https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-aarch64-linux.tar.bz2
RUN mkdir -p /opt/gcc && \
    cd /opt/gcc && \
    wget https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2 && \
    tar xjf gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2 && \
    rm -rf /opt/gcc/*.tar.bz2

ENV PATH="/opt/gcc/gcc-arm-none-eabi-10.3-2021.10/bin:${PATH}"
