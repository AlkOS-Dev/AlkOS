#!/bin/bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

INSTALL_TOOLCHAIN_ACTION_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

cd "${INSTALL_TOOLCHAIN_ACTION_DIR}/../tests" || exit 1
python -m test_framework.main --path "./run_test_alkos.bash" "$@"
