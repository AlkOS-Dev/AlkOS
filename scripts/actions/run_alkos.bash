#!/bin/bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

RUN_ALKOS_ACTION_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

source "${RUN_ALKOS_ACTION_DIR}/../config/conf_handlers.bash"

source_conf_file
verify_conf_var_exists CONF_ISO_PATH
"${RUN_ALKOS_ACTION_DIR}/../install/run_alkos.bash" "${CONF_ISO_PATH}" "$@"
