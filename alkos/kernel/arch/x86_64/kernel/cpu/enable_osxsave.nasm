    bits 64

    %include "panic.nasm"

    ; Intel manual page 814 Vol 2A CPUID - CPU Identification
    ; cpuid flags
    FEATURE_CPUID_FLAG equ 0x1      ; CPUID EAX code
    XSAVE_CPUID_FLAG   equ 0xD      ; CPUID EAX code
    XSAVE_FLAG         equ 1<<26    ; ECX
    XSAVE_AVX_FLAG     equ 1<<2     ; EAX

    ; OSXSAVE Control Register flags
    OSXSAVE_FLAG equ 1<<18  ; CR4.OSXSAVE[bit 18]

    ; XCR0 flags
    X87_FLAG     equ 1<<0   ; x87 state
    SSE_FLAG     equ 1<<1   ; SSE state
    AVX_FLAG     equ 1<<2   ; AVX state

    section .rodata
    FAIL_OSXSAVE     db "Missing OSXSAVE feature set. Unable to start...", 0
    FAIL_OSXSAVE_AVX db "Missing AVX support in OSXSAVE feature set. Unable to start...", 0

    section .text
    global EnableOSXSave

; Intel manual page 319 (Chapter 13 - Managing State Using the XSAVE Feature Set)
EnableOSXSave:
    mov rax, FEATURE_CPUID_FLAG
    cpuid

    ; check if the CPU supports XSAVE
    test rcx, XSAVE_FLAG
    jz enable_osxsave_fail

    ; check if the CPU supports requested states
    mov rax, XSAVE_CPUID_FLAG
    xor rcx, rcx ; subfunction 0
    cpuid

    ; Checks for supported states
    ; -----------------------------------------
    ; NOTE: add new checks for XSAVE supported states here
    ; -----------------------------------------
    test rax, AVX_FLAG
    jz enable_osxsave_avx_fail

    ; As stated at page CPUID manual page EBX and ECX at this moment contains sizes of XSAVE area
    ; TODO: USE THE VALUES IN THE FUTURE

    ; TODO: check more sophisticated XSAVE features with CPUID subfunction 1

    ; set CR4.OSXSAVE[bit 18] to 1 - explicitly stating that os supports XSAVE feature set
    mov rax, cr4
    or  rax, OSXSAVE_FLAG
    mov cr4, rax

    ; Prepare registers for XSAVE setup
    xor rax, rax
    xor rcx, rcx
    xor rdx, rdx

    ; Note x87 must be enabled otherwise XSAVE will fail with #GP exception
    ; Enable XSAVE states
    ; -----------------------------------------
    ; NOTE: Add new states switches here
    ; -----------------------------------------
    or rax, X87_FLAG
    or rax, SSE_FLAG
    or rax, AVX_FLAG

    ; TODO: Enable other states if needed

     XSETBV ; Set XCR0 to enable requested states

    ; TODO: after setting XCR0, we could query the cpu for XSAVE area sizes again

    ret

enable_osxsave_avx_fail:
    panic FAIL_OSXSAVE_AVX


enable_osxsave_fail:
    panic FAIL_OSXSAVE
