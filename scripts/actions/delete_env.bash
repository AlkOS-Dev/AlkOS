#!/bin/bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

DELETE_ENV_SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

DELETE_ENV_TOOL_INSTALL_DIR="${DELETE_ENV_SCRIPT_DIR}/../../tools"
DELETE_ENV_BUILD_DIR="${DELETE_ENV_SCRIPT_DIR}/../../build"

source "${DELETE_ENV_SCRIPT_DIR}/../utils/pretty_print.bash"

pretty_info "Deleting tool dir..."
rm -rf "${DELETE_ENV_TOOL_INSTALL_DIR}"
pretty_success "Tool dir deleted"

pretty_info "Deleting build dir..."
rm -rf "${DELETE_ENV_BUILD_DIR}"
pretty_success "Build dir deleted"

pretty_success "Environment deleted"
