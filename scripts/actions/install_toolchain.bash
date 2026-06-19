#!/bin/bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

INSTALL_TOOLCHAIN_ACTION_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

source "${INSTALL_TOOLCHAIN_ACTION_DIR}/../config/conf_handlers.bash"

source_conf_file
verify_conf_var_exists CONF_TOOL_DIR
verify_conf_var_exists CONF_BUILD_DIR
verify_conf_var_exists CONF_ARCH
"${INSTALL_TOOLCHAIN_ACTION_DIR}/../env/install_toolchain.bash" "${CONF_TOOL_DIR}" "${CONF_BUILD_DIR}" "${CONF_ARCH}" "$@"
