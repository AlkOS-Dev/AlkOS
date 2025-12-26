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

section .text
global SwitchToTask

; c_decl
; void SwitchToTask(Thread* thread)
;   RDI = thread
; Note: Caller is responsible for ensuring proper environment before calling (disabling IRQs)
; Layout:
;   RAX = current TCB | next cr3
;   RDI = next TCB | next kernel stack
;   R11 = current cr3
SwitchToTask:
    ; ------------------------
    ; Save current task state

    sub rsp, _all_reg_size          ; Allocate space for saving registers.
    push_all_regs                   ; Save registers of calling TCB on ITS stack

    call cdecl_GetCurrentTCB         ; RAX = pointer to TCB
    mov [rax+Thread.user_stack], rsp ; Save RSP for previous task's kernel stack in the thread's TCB

    ; ------------------------
    ; Setup next task state

    call cdecl_SetCurrentTCB           ; Next task TCB already in RDI
    mov rsp, [rdi+Thread.user_stack]   ; Change the stack

    call cdecl_GetThreadsPageTable     ; RAX = next cr3
    mov r11, cr3                       ; R11 = current cr3

    mov rdi, [rdi+Thread.kernel_stack] ; Load next task's kernel stack
    call cdecl_SetTssRsp0

    cmp r11, rax ; Skip virtual address space change if not needed - omit tlb flushes
    je .done
    mov cr3, rax ; Load next task's virtual address space

.done:

    pop_all_regs                    ; Restore registers of NEW thread's stack
    add rsp, _all_reg_size          ; Deallocate register save space.

    ret ; Load next thread's EIP from its kernel stack
