; ------------------------------------------------------------
; Register macros
; ------------------------------------------------------------

; Register offsets on the stack when saving the state
_rax equ 0
_rcx equ 8
_rdx equ 16
_rsi equ 24
_rdi equ 32
_r8  equ 40
_r9  equ 48
_r10 equ 56
_r11 equ 64
_rbp equ 72
_rbx equ 80
_r12 equ 88
_r13 equ 96
_r14 equ 104
_r15 equ 112

; Size needed to save the sysv registers on the stack
_sysv_reg_size equ 8*10

; Shadow space required for C++ function calls
_shadow_space equ 8*4

; Macro to save all volatile registers (SysV ABI) onto the stack.
%macro push_sysv_regs 0
    mov qword [rsp + _rax], rax
    mov qword [rsp + _rcx], rcx
    mov qword [rsp + _rdx], rdx
    mov qword [rsp + _rsi], rsi
    mov qword [rsp + _rdi], rdi
    mov qword [rsp + _r8], r8
    mov qword [rsp + _r9], r9
    mov qword [rsp + _r10], r10
    mov qword [rsp + _r11], r11
    mov qword [rsp + _rbp], rbp
%endmacro

; Macro to restore all volatile registers (SysV ABI) from the stack.
%macro pop_sysv_regs 0
    mov rax, qword [rsp + _rax]
    mov rcx, qword [rsp + _rcx]
    mov rdx, qword [rsp + _rdx]
    mov rsi, qword [rsp + _rsi]
    mov rdi, qword [rsp + _rdi]
    mov r8, qword [rsp + _r8]
    mov r9, qword [rsp + _r9]
    mov r10, qword [rsp + _r10]
    mov r11, qword [rsp + _r11]
    mov rbp, qword [rsp + _rbp]
%endmacro

; Size needed to save the registers on the stack
_all_reg_size equ 8*15

; Macro to save ALL general purpose registers onto the stack.
; Useful for Interrupt Service Routines (ISRs) or kernel panic dumps.
%macro push_all_regs 0
    mov qword [rsp + _rax], rax
    mov qword [rsp + _rcx], rcx
    mov qword [rsp + _rdx], rdx
    mov qword [rsp + _rsi], rsi
    mov qword [rsp + _rdi], rdi
    mov qword [rsp + _r8], r8
    mov qword [rsp + _r9], r9
    mov qword [rsp + _r10], r10
    mov qword [rsp + _r11], r11
    mov qword [rsp + _rbp], rbp
    mov qword [rsp + _rbx], rbx
    mov qword [rsp + _r12], r12
    mov qword [rsp + _r13], r13
    mov qword [rsp + _r14], r14
    mov qword [rsp + _r15], r15
%endmacro

; Macro to restore ALL general purpose registers from the stack.
; Must be called in the reverse order of push_all_regs.
%macro pop_all_regs 0
    mov rax, qword [rsp + _rax]
    mov rcx, qword [rsp + _rcx]
    mov rdx, qword [rsp + _rdx]
    mov rsi, qword [rsp + _rsi]
    mov rdi, qword [rsp + _rdi]
    mov r8, qword [rsp + _r8]
    mov r9, qword [rsp + _r9]
    mov r10, qword [rsp + _r10]
    mov r11, qword [rsp + _r11]
    mov rbp, qword [rsp + _rbp]
    mov rbx, qword [rsp + _rbx]
    mov r12, qword [rsp + _r12]
    mov r13, qword [rsp + _r13]
    mov r14, qword [rsp + _r14]
    mov r15, qword [rsp + _r15]
%endmacro
