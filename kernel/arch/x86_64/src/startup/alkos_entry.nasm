; SPDX-License-Identifier: MIT
; Copyright (c) 2025-2026 The AlkOS Authors
; See the AUTHORS file for the full list of contributors.

          ; Stack
          extern stack_bottom
          extern stack_top

          ; GDT64
          extern GDT64.Pointer
          extern GDT64.Data

          ; GCC compiler global destructor support
          extern _fini

          ; Kernel Entry Point
          extern KernelMain

          global alkos.entry
          section .text
          bits 64
alkos.entry:
          mov rsp, stack_top
          mov rbp, rsp

          ; Call actual kernel entry point
          call KernelMain

          ; Not actually needed (as we expect to never return from Kernel), but exists for completeness
          call _fini

          ; Infinite loop
os_hang:
          hlt
          jmp os_hang
