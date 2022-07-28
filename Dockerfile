FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive

WORKDIR /app

RUN apt update && apt install -y wget make cmake

# GCC ARM
RUN mkdir -p /opt/gcc && \
    cd /opt/gcc && \
    wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2 && \
    tar xjf gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2 && \
    rm -rf /opt/gcc/*.tar.bz2

ENV PATH="/opt/gcc/gcc-arm-none-eabi-10-2020-q4-major/bin:${PATH}"
