; SPDX-License-Identifier: MIT
; Copyright (c) 2025-2026 The AlkOS Authors
; See the AUTHORS file for the full list of contributors.

          ; This is a 32-bit stack for the 64-bit loader

          global stack_bottom
          global stack_top

          section .bss
          align 16
stack_bottom:
          resb 32768
stack_top:
