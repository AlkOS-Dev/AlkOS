; !!! Temporary file 
; !!! Will be deleted after userspace is linked against libc

bits 64
global _start
extern main

section .bss
    global g_print_fn
    g_print_fn: resq 1

section .text
_start:
    ; System V ABI: The first argument (RDI) passed by the kernel
    ; contains the function pointer to the print helper.
    mov [rel g_print_fn], rdi

    ; Call the main C++ function
    call main

    ; Return to kernel (for now, just ret, kernel wrapper will handle this)
    ret
