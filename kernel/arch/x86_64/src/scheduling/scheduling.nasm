; ------------------------------------------------------------
; Scheduling utilities and crucial functionality
; ------------------------------------------------------------

%include "include/regs.nasm"
%include "include/thread.nasm"

bits 64

extern cdecl_GetCurrentTCB
extern cdecl_SetCurrentTCB
extern cdecl_GetThreadsPageTable

section .text
global SwitchToTask

; c_decl
; void SwitchToTask(Thread* thread)q
;   RDI = thread
; Note: Caller is responsible for ensuring proper environment before calling (disabling IRQs)
; Layout:
;   RAX = current TCB
;   RDI = next TCB
;   R10 = next cr3
;   R11 = next kernel stack | current cr3
SwitchToTask:
    ; ------------------------
    ; Save current task state

    sub rsp, _sysv_reg_size          ; Allocate space for saving registers.
    push_sysv_regs                   ; Save registers of calling TCB on ITS stack

    call cdecl_GetCurrentTCB         ; RAX = pointer to TCB
    mov [rax+Thread.user_stack], esp ; Save ESP for previous task's kernel stack in the thread's TCB

    ; ------------------------
    ; Setup next task state

    call cdecl_SetCurrentTCB           ; Next task TCB already in RDI
    mov esp, [rdi+Thread.user_stack]   ; Change the stack
    mov r11, [rdi+Thread.kernel_stack] ; Load next task's kernel stack
;    mov [TSS.ESP0]

;    call cdecl_GetThreadsPageTable   ; RAX = next cr3


.done:

    pop_sysv_regs                    ; Restore registers of NEW thread's stack
    add rsp, _sysv_reg_size          ; Deallocate register save space.

    ret ; Load next thread's EIP from its kernel stack
