; SPDX-License-Identifier: MIT
; Copyright (c) 2025-2026 The AlkOS Authors
; See the AUTHORS file for the full list of contributors.

          [bits 64]

          section .text
          global EnterKernel
          ; void EnterKernel(u64 kernel_entry, KernelInitialParams *loader_data)
          ;                  [rdi]            [rsi]
EnterKernel:
          xchg rdi, rsi ; Swap rdi and rsi
          ; rdi = loader_data, rsi = kernel_entry
          jmp rsi
