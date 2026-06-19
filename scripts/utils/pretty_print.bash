#!/bin/bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

pretty_echo() {
    local type=$1
    shift
    local message="$@"

    local red='\033[0;31m'
    local yellow='\033[1;33m'
    local green='\033[0;32m'
    local blue='\033[0;34m'
    local cyan='\033[0;36m'
    local nc='\033[0m'
    local bold='\033[1m'

    local label_width=12

    case "${type,,}" in
        "error")
            printf "${red}${bold}[ERROR]${nc}%-$((label_width-7))s%s\n" " " "${message}"
            ;;
        "warning")
            printf "${yellow}${bold}[WARNING]${nc}%-$((label_width-9))s%s\n" " " "${message}"
            ;;
        "info")
            printf "${blue}${bold}[INFO]${nc}%-$((label_width-6))s%s\n" " " "${message}"
            ;;
        "success")
            printf "${green}${bold}[SUCCESS]${nc}%-$((label_width-9))s%s\n" " " "${message}"
            ;;
        "debug")
            printf "${cyan}${bold}[DEBUG]${nc}%-$((label_width-7))s%s\n" " " "${message}"
            ;;
        *)
            printf "${red}${bold}[ERROR]${nc}%-$((label_width-7))s%s\n" " " "Unknown message type in pretty print: ${type}!"
            exit 1
            ;;
    esac
}

pretty_info() {
    pretty_echo "info" "$@"
}

pretty_error() {
    pretty_echo "error" "$@"
}

pretty_warn() {
    pretty_echo "warning" "$@"
}

pretty_success() {
    pretty_echo "success" "$@"
}

pretty_debug() {
    pretty_echo "debug" "$@"
}
