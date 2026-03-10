FROM ubuntu:24.04

RUN apt update && apt install -y \
    build-essential \
    cmake \
    make \
    gcc \
    binutils \
    gnu-efi \
    qemu-system-x86 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work
