; SPDX-License-Identifier: MIT
; Copyright (c) 2025-2026 The AlkOS Authors
; See the AUTHORS file for the full list of contributors.

section .init

    ; gcc will nicely put the contents of crtend.o's .init section here.
    pop rbp
    ret

section .fini

    ; gcc will nicely put the contents of crtend.o's .fini section here.
    pop rbp
    ret
