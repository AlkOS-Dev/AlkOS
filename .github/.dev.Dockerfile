# Start from a stable Ubuntu base image
FROM ubuntu:22.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

RUN apt update

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    bison \
    flex \
    libgmp3-dev \
    libmpc-dev \
    libmpfr-dev \
    texinfo \
    python3 \
    python3-pip \
    nasm \
    mtools \
    qemu-system-x86 \
    git \
    wget \
    curl \
    && rm -rf /var/lib/apt/lists/*

COPY scripts/env/ubuntu_packages.txt /tmp/ubuntu_packages.txt
RUN apt-get update && \
    xargs -a /tmp/ubuntu_packages.txt apt-get install -y --no-install-recommends && \
    rm -rf /var/lib/apt/lists/*

# Set the working directory and copy the entire project context into the image
WORKDIR /app
COPY . .

# Build and install the cross-compiler toolchain
RUN /app/scripts/env/install_toolchain.bash "/tools" "/build_toolchain" "x86_64"

# Permanently add the toolchain binaries to the PATH
ENV PATH="/tools/i386-elf/bin:/tools/x86_64-elf/bin:${PATH}"

# Run the initial debug configuration for the environment
RUN /app/scripts/config/configure.bash "x86_64" debug -p test_mode

# Set the final working directory
WORKDIR /github/workspace
