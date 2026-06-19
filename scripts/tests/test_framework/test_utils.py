# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

# ANSI color codes
RED = '\033[31m'
GREEN = '\033[32m'
RESET = '\033[0m'


def print_red(text: str) -> None:
    print(f"{RED}{text}{RESET}")


def print_green(text: str) -> None:
    print(f"{GREEN}{text}{RESET}")
