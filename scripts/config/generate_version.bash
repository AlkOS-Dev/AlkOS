#!/bin/bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

# Usage: generate_version.bash [arch] [build_type] [official]

set -euo pipefail

GENERATE_VERSION_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

source "${GENERATE_VERSION_DIR}/version_lib.bash"

version_generate "$@"
