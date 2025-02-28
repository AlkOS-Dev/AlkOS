; mem_ops.asm
; 32-bit stub for memcpy64 and memset64.
; They expect their parameters in the standard cdecl stack layout.
; For memcpy64, the parameters (in order) are:
;   u64 dest, u64 src, u64 n
; which are pushed (low part first) as:
;   [ebp+8]  = dest_low
;   [ebp+12] = dest_high
;   [ebp+16] = src_low
;   [ebp+20] = src_high
;   [ebp+24] = n_low
;   [ebp+28] = n_high
;
; For memset64, the parameters are:
;   u64 dest, u32 c, u64 n
; which are pushed as:
;   [ebp+8]  = dest_low
;   [ebp+12] = dest_high
;   [ebp+16] = c       (32-bit)
;   [ebp+20] = n_low
;   [ebp+24] = n_high

; ----------------------------------------------------------
; Data section for temporary storage
; (Assuming single-threaded boot, so fixed scratch areas are acceptable.)
section .bss
align 8
; For memcpy64: store dest (qword), src (qword), n (qword)
memcpy64_args: resq 3
; For memset64: store dest (qword), c (qword), n (qword)
memset64_args: resq 3

section .text
global memcpy64
global memset64

; ---------------------------
; 32-bit stub for memcpy64
; ---------------------------
; This code runs in 32-bit mode.
; It gathers the two halves of each u64 parameter and stores them.
memcpy64:
    push ebp
    mov ebp, esp
    ; Get parameters (remember: lower 32 bits are at lower address)
    mov ebx, [ebp+8]    ; dest_low
    mov eax, [ebp+12]   ; dest_high
    mov edx, [ebp+16]   ; src_low
    mov ecx, [ebp+20]   ; src_high
    mov edi, [ebp+24]   ; n_low
    mov esi, [ebp+28]   ; n_high

    ; Store dest as a 64-bit value in little-endian: lower then higher dword.
    mov dword [memcpy64_args], ebx      ; dest_low at offset 0
    mov dword [memcpy64_args+4], eax    ; dest_high at offset 4

    ; Store src (next 8 bytes)
    mov dword [memcpy64_args+8], edx     ; src_low at offset 8
    mov dword [memcpy64_args+12], ecx    ; src_high at offset 12

    ; Store n (last 8 bytes)
    mov dword [memcpy64_args+16], edi    ; n_low at offset 16
    mov dword [memcpy64_args+20], esi    ; n_high at offset 20

    ; Jump to the 64-bit routine.
    ; This far jump must switch the CPU into long mode (64-bit).
    jmp memcpy64_64

; ---------------------------
; 32-bit stub for memset64
; ---------------------------
memset64:
    push ebp
    mov ebp, esp
    ; Parameters:
    ; [ebp+8]  = dest_low
    ; [ebp+12] = dest_high
    ; [ebp+16] = c (32-bit value)
    ; [ebp+20] = n_low
    ; [ebp+24] = n_high
    mov ebx, [ebp+8]    ; dest_low
    mov eax, [ebp+12]   ; dest_high
    mov ecx, [ebp+16]   ; c
    mov edi, [ebp+20]   ; n_low
    mov edx, [ebp+24]   ; n_high

    ; Store dest into memset64_args
    mov dword [memset64_args], ebx      ; dest_low
    mov dword [memset64_args+4], eax    ; dest_high

    ; Store c as a 64-bit value (zero extend c into a qword)
    mov dword [memset64_args+8], ecx    ; c (low part)
    mov dword [memset64_args+12], 0      ; c (high part zero)

    ; Store n
    mov dword [memset64_args+16], edi    ; n_low
    mov dword [memset64_args+20], edx    ; n_high

    jmp memset64_64

; ----------------------------------------------------------
; Now, switch to 64-bit mode for the actual implementations.
; In our bootloader, a far jump (using the proper GDT entries) is assumed
; to be available for switching between 32-bit and 64-bit code.
; The following routines use the System V AMD64 calling convention.
; ----------------------------------------------------------

[BITS 64]
memcpy64_64:
    [BITS 64]
    ; Load parameters from the temporary storage.
    ; In little-endian, the qword at memcpy64_args is formed by (dest_high << 32 | dest_low)
    mov rdi, [memcpy64_args]      ; rdi = dest
    mov rsi, [memcpy64_args+8]    ; rsi = src
    mov rdx, [memcpy64_args+16]   ; rdx = n
    cld
    ; Use rep movsb to copy n bytes.
    rep movsb
    ; Jump back to 32-bit stub exit.
    jmp memcpy64_exit_32

memcpy64_exit_32:
    ; Switch back to 32-bit mode.
    [BITS 32]
    pop ebp
    ret

memset64_64:
    [BITS 64]
    ; Load parameters from temporary storage.
    mov rdi, [memset64_args]      ; rdi = dest
    ; Although c was only 32-bit, it was stored in a qword.
    mov rsi, [memset64_args+8]    ; rsi = c
    mov rdx, [memset64_args+16]   ; rdx = n
    cld
    rep stosb
    jmp memset64_exit_32

memset64_exit_32:
    [BITS 32]
    pop ebp
    ret
