    bits 64

    %include "startup/panic.nasm"
    ; Intel manual page 814 Vol 2A CPUID - CPU Identification

    ; CPUID flags
    FEATURE_FLAG equ 0x1      ; CPUID EAX code
    SSE_FLAG     equ 1<<25    ; EDX

    ; SSE Control Register flags
    OSFXSR_FLAG     equ 1<<9   ; CR4.OSFXSR
    OSXMMEXCPT_FLAG equ 1<<10  ; CR4.OSXMMEXCPT
    EM_FLAG         equ 1<<2   ; CR0.EM
    MP_FLAG         equ 1<<1   ; CR0.MP


    section .rodata
    FAIL_SSE   db "Missing SEE feature set. Unable to start...", 0

    section .text
    global EnableSSE

; According to intel manual page: 3570 (15.1.3 - Initialization of the SSE Extensions)
; TODO:
; 1. SSE CONTEXT SWITCHING
; 2. SSE EXCEPTION HANDLING
; 3. SSE BEHAVIOR
EnableSSE:
    ; query the cpu for supported features
    mov eax, FEATURE_FLAG
    cpuid

    ; check if the SSE flag is set
    test edx, SSE_FLAG
    jz enable_sse_fail

    ; One of below is expected to be supported to provide context switching for SSE registers

    ; In theory there should be  check if XSAVE or XRSTOR instructions are supported,
    ; but in our case we assumes that XSAVE is supported and those checks are performed before this function is called
    ; NOT: enable_osxsave MUST be called before this function
    ; test ecx, XSAVE_FLAG
    ; test edx, FXSAVE_FLAG

    mov rax, cr4

    ; step 1
    ; set CR4.OSFXSR[bit 9] to 1 - explicitly stating that os supports context switching of SSE registers
    ; TODO: STATED NOT IMPLEMENTED
    or rax, OSFXSR_FLAG

    ; step 2
    ; set CR4.OSXMMEXCPT[bit 10] to 1 - explicitly stating that os supports SSE exceptions by providing an SSE exception handler
    ; TODO: STATED NOT IMPLEMENTED
    or rax, OSXMMEXCPT_FLAG

    ; step 3
    ; write back to CR4 and load CR0
    mov cr4, rax
    mov rax, cr0

    ; step 4
    ; set CR0.EM[bit 2] to 0 - disable x87 FPU emulation - required for SSE
    and rax, ~EM_FLAG

    ; step 5
    ; set CR0.MP[bit 1] to 1 - referenced to section 11.2.1 - not stated but required for SSE
    or rax, MP_FLAG

    ; step 6
    ; write back to CR0
    mov cr0, rax

    ; step 7
    ; configure the MXCSR register
    ; TODO: IMPLEMENT TOGETHER WITH HANDLERS ETC

    ret

enable_sse_fail:
    panic FAIL_SSE
