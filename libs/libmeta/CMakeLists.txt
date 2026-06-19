# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

add_library(alkos.libmeta.interface INTERFACE)

target_include_directories(alkos.libmeta.interface
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(alkos.libmeta.interface
    INTERFACE
        alkos.libc.interface
)
