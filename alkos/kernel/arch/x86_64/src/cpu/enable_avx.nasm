    bits 64

    %include "startup/panic.nasm"
    ; Intel manual page 814 Vol 2A CPUID - CPU Identification

    ; CPUID flags
    FEATURE_FLAG equ 0x1      ; CPUID EAX code
    AVX_FLAG     equ 1<<28    ; ECX

    ; AVX Control Register flags

    section .rodata
    FAIL_AVX   db "Missing AVX feature set. Unable to start...", 0

    section .text
    global EnableAVX

; NOTE: Should be called ONLY after enable_osxsave
EnableAVX:
    ; query the cpu for supported features
    mov eax, FEATURE_FLAG
    cpuid

    test ecx, AVX_FLAG
    jz enable_avx_fail

    ; NOTE: At this moment XSAVE feature is assumed to be supported, enabled and configure before this function is called
    ; NOTE: Furthermore AVX support is already available after XSAVE (with AVX support) and SEE support is enabled

    ret

enable_avx_fail:
    panic FAIL_AVX
