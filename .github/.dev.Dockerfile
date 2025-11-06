# Start from a stable Arch Linux base image
FROM archlinux:latest

# Update the system and install packages required for the CI environment.
RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm sudo git curl grub python-pip

# Set the working directory and copy the entire project context into the image
WORKDIR /app
COPY . .

# Install project-specific dependencies using the existing script.
# The script was modified to not fail if virtualization is not present (e.g. in CI).
RUN /app/scripts/env/install_deps_arch.bash -i -v

# Build and install the cross-compiler toolchain
RUN /app/scripts/env/install_toolchain.bash "/tools" "/build_toolchain" "x86_64" "-v"

# Permanently add the toolchain binaries to the PATH
ENV PATH="/tools/i386-elf/bin:/tools/x86_64-elf/bin:${PATH}"

# Run the initial debug configuration for the environment
RUN /app/scripts/config/configure.bash "x86_64" debug -p test_mode -v

# Set the final working directory for GitHub Actions
WORKDIR /github/workspace
