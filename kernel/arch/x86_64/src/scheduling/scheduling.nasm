; ------------------------------------------------------------
; Scheduling utilities and crucial functionality
; ------------------------------------------------------------

%include "include/regs.nasm"
%include "thread.nasm"

bits 64

extern cdecl_GetCurrentTCB
extern cdecl_SetCurrentTCB

section .text
global SwitchToTask

; c_decl
; void SwitchToTask(Thread* thread)
;   RDI = thread
; Note: Caller is responsible for ensuring proper environment before calling (disabling IRQs)
; Layout:
;   RAX = current TCB
;   RDI = next TCB
SwitchToTask:
    ; ------------------------
    ; Save current task state

    sub rsp, _sysv_reg_size          ; Allocate space for saving registers.
    push_sysv_regs                   ; Save registers of calling TCB on ITS stack

    call cdecl_GetCurrentTCB         ; RAX = pointer to TCB
    mov [rax+Thread.user_stack], esp ; Save ESP for previous task's kernel stack in the thread's TCB

    ; ------------------------
    ; Setup next task state

    call cdecl_SetCurrentTCB         ; Next task TCB already in RDI
    mov esp, [rdi+Thread.user_stack] ; Change the stack
    

.done:

    pop_sysv_regs                    ; Restore registers of NEW thread's stack
    add rsp, _sysv_reg_size          ; Deallocate register save space.

    ret ; Load next thread's EIP from its kernel stack
