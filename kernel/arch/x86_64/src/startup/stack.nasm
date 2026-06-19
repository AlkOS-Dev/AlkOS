; SPDX-License-Identifier: MIT
; Copyright (c) 2025-2026 The AlkOS Authors
; See the AUTHORS file for the full list of contributors.

          bits 64

          global stack_bottom
          global stack_top

          section .stack
          align 16
stack_bottom:
          resb 32768
stack_top:
