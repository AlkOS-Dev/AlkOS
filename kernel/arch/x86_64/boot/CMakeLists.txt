# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

message(STATUS "    -> Configuring x86_64 bootloader")

add_subdirectory(lib)
add_subdirectory(loader_32)
add_subdirectory(loader_64)
