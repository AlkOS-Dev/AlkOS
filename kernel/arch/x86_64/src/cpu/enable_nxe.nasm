bits 64

    %include "startup/panic.nasm"

    ; Extended Feature Enable Register (EFER)
    EFER_MSR        equ 0xC0000080
    EFER_NXE_BIT    equ 1 << 11

    ; CPUID Extended Processor Info
    CPUID_EXT_FUNC  equ 0x80000001
    CPUID_NX_BIT    equ 1 << 20    ; In EDX

    section .rodata
    FAIL_NX         db "CPU does not support No-Execute (NX) bit. Halting.", 0

    section .text
    global EnableNXE

EnableNXE:
    push rbx
    push rcx
    push rdx

    ; Check if CPU supports NX (CPUID 0x80000001, Bit 20 of EDX)
    mov eax, 0x80000000
    cpuid
    cmp eax, CPUID_EXT_FUNC
    jb .no_nx_support       ; Extended functions not supported

    mov eax, CPUID_EXT_FUNC
    cpuid
    test edx, CPUID_NX_BIT
    jz .no_nx_support

    ; Enable NXE in EFER MSR
    mov ecx, EFER_MSR
    rdmsr                   ; Read EFER into EDX:EAX
    bts eax, 11             ; Set bit 11 (NXE)
    wrmsr                   ; Write EFER back

    pop rdx
    pop rcx
    pop rbx
    ret

.no_nx_support:
    panic FAIL_NX
