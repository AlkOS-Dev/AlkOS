; SPDX-License-Identifier: MIT
; Copyright (c) 2025-2026 The AlkOS Authors
; See the AUTHORS file for the full list of contributors.

extern QemuTerminalWriteString

%ifdef __USE_DEBUG_OUTPUT__

%macro trace 1
    push rdi
    mov rdi, %1
    call QemuTerminalWriteString
    pop rdi
%endmacro

%else

%macro trace 1
%endmacro

%endif
