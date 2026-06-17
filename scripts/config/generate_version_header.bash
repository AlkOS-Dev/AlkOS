#!/bin/bash

set -euo pipefail

GENERATE_VERSION_HEADER_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

source "${GENERATE_VERSION_HEADER_DIR}/version_lib.bash"

version_generate_header "$@"
