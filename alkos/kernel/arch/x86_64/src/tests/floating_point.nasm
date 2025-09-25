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
