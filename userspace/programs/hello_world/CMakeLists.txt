# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

message(STATUS "    -> hello")
alkos_find_sources(HELLO_WORLD_SOURCES)
alkos_register_userspace_app(hello "${HELLO_WORLD_SOURCES}")
