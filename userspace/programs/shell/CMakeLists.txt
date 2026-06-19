# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

message(STATUS "    -> shell")
alkos_find_sources(SHELL_SOURCES)
alkos_register_userspace_app(shell "${SHELL_SOURCES}")

add_library(shell.libgraphics STATIC ${LIBGRAPHICS_SOURCES})
target_link_libraries(shell.libgraphics PUBLIC
    alkos.libgraphics.interface
    alkos.libc.interface
)

target_compile_options(shell.libgraphics PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:
        -fno-builtin
        -fno-rtti
        -fno-exceptions
        -fno-stack-protector
        -mcmodel=small
        -mno-red-zone
    >
)

target_link_libraries(shell PUBLIC
    alkos.libcontainers.interface
    shell.libgraphics
)
