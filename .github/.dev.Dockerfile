# Start from a stable Ubuntu base image
FROM ubuntu:22.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

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

COPY ./scripts/ /tmp/scripts/

# Build and install the cross-compiler toolchain from the scripts
RUN /tmp/scripts/env/install_toolchain.bash "/tools" "/build" "x86_64"

# Permanently add the toolchain binaries to the PATH environment variable
# This makes `x86_64-elf-gcc` and other tools available directly
ENV PATH="/tools/i386-elf/bin:/tools/x86_64-elf/bin:${PATH}"

# Run the initial debug configuration for the environment
RUN /tmp/scripts/config/configure.bash "x86_64" debug -p test_mode

# Set the default working directory for subsequent commands
WORKDIR /github/workspace
