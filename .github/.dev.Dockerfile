# Start from a stable Ubuntu base image
FROM ubuntu:22.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install all necessary dependencies for building the toolchain
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
    cmake \
    yq \
    xorriso \
    grub2 \
    clang-format \
    libisoburn-dev \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory and copy the entire project context into the image.
# This ensures all scripts and source files are available during the image build.
WORKDIR /app
COPY . .

# Build and install the cross-compiler toolchain from the scripts.
RUN /app/scripts/env/install_toolchain.bash "/tools" "/build_toolchain" "x86_64"

# Permanently add the toolchain binaries to the PATH environment variable.
ENV PATH="/tools/i386-elf/bin:/tools/x86_64-elf/bin:${PATH}"

# Run the initial debug configuration for the environment.
RUN /app/scripts/config/configure.bash "x86_64" debug -p test_mode

# Set the final working directory to match the GitHub Actions runner environment.
WORKDIR /github/workspace
