; SPDX-License-Identifier: MIT
; Copyright (c) 2025-2026 The AlkOS Authors
; See the AUTHORS file for the full list of contributors.

extern KernelPanic

%macro panic 1
    mov rdi, %1
    call KernelPanic
%endmacro
