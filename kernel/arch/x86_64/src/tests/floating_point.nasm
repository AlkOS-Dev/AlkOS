; SPDX-License-Identifier: MIT
; Copyright (c) 2025-2026 The AlkOS Authors
; See the AUTHORS file for the full list of contributors.

bits 64

section .text
    global AvxFloatingPointTest

AvxFloatingPointTest:
    ; void AvxFloatingPointTest(double a, double b);
    vbroadcastsd ymm0, xmm0
    vbroadcastsd ymm1, xmm1
    vaddpd ymm0, ymm0, ymm1
    vmulpd ymm0, ymm0, ymm1
    vfmadd132pd ymm0, ymm1, ymm1

    ret
