# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

alkos_add_host_test_suite(
    TARGET_NAME     containers
    LIBS_UNDER_TEST alkos.libcontainers.interface
    DIRECTORY       ${CMAKE_CURRENT_SOURCE_DIR}
)
