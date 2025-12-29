; ------------------------------------------------------------
; Scheduling utilities and crucial functionality
; ------------------------------------------------------------

%include "include/regs.nasm"
%include "include/thread.nasm"

bits 64

extern cdecl_GetCurrentTCB
extern cdecl_SetCurrentTCB
extern cdecl_GetThreadsPageTable
extern cdecl_SetTssRsp0
extern cdecl_EnableHardwareInterrupts
extern cdecl_SetNextThreadFs
extern cdecl_SwapFsIfNeeded
extern HandleHardwareInterrupt

_context_switch_stack_space equ 21*8
_context_switch_stack_space_ext equ 20*8; 8 bytes already reserved by caller
_rip_call_offset equ _context_switch_stack_space_ext
_rip_int_frame_offset equ _all_reg_size + 8
_cs_int_frame_offset equ _rip_int_frame_offset + 8
_flags_int_frame_offset equ _cs_int_frame_offset + 8
_sp_int_frame_offset equ _flags_int_frame_offset + 8
_ss_int_frame_offset equ _sp_int_frame_offset + 8

_kernel_code_selector equ 0x08
_kernel_data_selector equ 0x10
_user_code_selector equ 0x18
_user_data_selector equ 0x20

_rip_user_space_offset equ 0
_cs_user_space_offset equ _rip_user_space_offset + 8
_flags_user_space_offset equ _cs_user_space_offset + 8
_sp_user_space_offset equ _flags_user_space_offset + 8
_ss_user_space_offset equ _sp_user_space_offset + 8
_jump_userspace_stack_space equ  5*8
_userspace_initial_flags equ 0x202

section .text
global ContextSwitch
global ConvertContext
global JumpToUserSpace

; c_decl
; void ConvertContext(Thread* thread)
;   RDI = thread
; Note: Caller is responsible for ensuring proper environment before calling (disabling IRQs)
; Note: ASSUMPTION ConvertContext is always called inside KERNEL code
ConvertContext:
    mov r12, rdi                          ; Save next TCB pointer in r12 (non-volatile) to survive C++ calls
    call cdecl_SetNextThreadFs

    mov rdi, r12
    call cdecl_SetCurrentTCB              ; Change TCB
    mov  rsp, [r12+Thread.kernel_stack]   ; Change the stack

    mov rdi, [r12+Thread.kernel_stack_bottom]
    call cdecl_SetTssRsp0

    pop_all_regs                    ; Restore registers of NEW thread's stack
    add rsp, _all_reg_size          ; Deallocate register save space.
    add rsp, 8                      ; pop error code

    iretq

; c_decl
; void JumpToUserSpace(void (*func)())
;   RDI = func
; Note: Caller is responsible for ensuring proper environment before calling (disabling IRQs)
; Note: FS already should be changed during contex switch
JumpToUserSpace:
    sub rsp, _context_switch_stack_space

    mov [rsp + _rip_user_space_offset], rdi
    mov qword [rsp + _cs_user_space_offset], _user_code_selector
    mov qword [rsp + _flags_user_space_offset], _userspace_initial_flags

    call cdecl_GetCurrentTCB
    mov r12, [rax+Thread.kernel_stack_bottom]
    mov [rax+Thread.kernel_stack], r12
    mov r13, [rax+Thread.user_stack_bottom]

    mov qword [rsp + _ss_user_space_offset], _user_data_selector
    mov [rsp + _sp_user_space_offset], r13

    xor rax, rax
    mov rax, _user_data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    iretq

; c_decl
; void ContextSwitch(Thread* thread)
;   RDI = thread
; Note: Caller is responsible for ensuring proper environment before calling (disabling IRQs)
; Note: ASSUMPTION ContextSwitch is always called inside KERNEL code
ContextSwitch:
    ; ------------------------
    ; Save current task state

    sub rsp, _context_switch_stack_space_ext             ; Allocate space for saving registers.
    push_all_regs                      ; Save registers of calling TCB on ITS stack

    ; Move RIP from call to align with Interrupt frame
    ; RIP
    mov r12, [rsp + _rip_call_offset]
    mov [rsp + _rip_int_frame_offset], r12

    ; CS
    mov qword [rsp + _cs_int_frame_offset], _kernel_code_selector

    ; FLAGS
    pushfq
    pop r12
    or r12, 0x200 ; Ensure Interrupts are enabled after the context switch
    mov qword [rsp + _flags_int_frame_offset], r12

    ; RSP
    mov r13, rsp
    add r13, _context_switch_stack_space
    mov [rsp + _sp_int_frame_offset], r13

    ; SS
    mov qword [rsp + _ss_int_frame_offset], _kernel_data_selector

    mov r12, rdi                       ; Save next TCB pointer in r12 (non-volatile) to survive C++ calls
    call cdecl_SetNextThreadFs

    mov rdi, r12
    call cdecl_GetCurrentTCB           ; RAX = pointer to TCB
    mov [rax+Thread.kernel_stack], rsp ; Save RSP for previous task's kernel stack in the thread's TCB

    ; ------------------------
    ; Setup next task state

    mov rdi, r12                         ; Restore next TCB pointer to RDI for the next call
    call cdecl_SetCurrentTCB
    mov rsp, [r12+Thread.kernel_stack]   ; Change the stack

    mov rdi, [r12+Thread.kernel_stack_bottom]
    call cdecl_SetTssRsp0

    mov rdi, r12                       ; Set RDI for GetThreadsPageTable
    call cdecl_GetThreadsPageTable     ; RAX = next cr3
    mov r11, cr3                       ; R11 = current cr3

    cmp r11, rax                       ; Skip virtual address space change if not needed - omit tlb flushes
    je .done
    mov cr3, rax                       ; Load next task's virtual address space

.done:
    pop_all_regs                    ; Restore registers of NEW thread's stack
    add rsp, _all_reg_size          ; Deallocate register save space.
    add rsp, 8                      ; pop error code

    iretq                             ; Load next thread's RIP from its stack
