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
global SwitchToKernelTask
global SwitchToUserTask
global ConvertToKernelTask

; c_decl
; void ConvertToKernelTask(Thread* thread)
;   RDI = thread
; Note: Caller is responsible for ensuring proper environment before calling (disabling IRQs)
ConvertToKernelTask:
    mov r12, rdi                          ; Save next TCB pointer in r12 (non-volatile) to survive C++ calls
    call cdecl_SetCurrentTCB              ; Change TCB
    mov  rsp, [r12+Thread.kernel_stack]   ; Change the stack

    pop_all_regs                    ; Restore registers of NEW thread's stack
    add rsp, _all_reg_size          ; Deallocate register save space.

    ret                             ; Load next thread's RIP from its stack

; c_decl
; void SwitchToKernelTask(Thread* thread)
;   RDI = thread
; Note: Caller is responsible for ensuring proper environment before calling (disabling IRQs)
SwitchToKernelTask:
    ; ------------------------
    ; Save current task state

    sub rsp, _all_reg_size             ; Allocate space for saving registers.
    push_all_regs                      ; Save registers of calling TCB on ITS stack

    mov r12, rdi                       ; Save next TCB pointer in r12 (non-volatile) to survive C++ calls

    call cdecl_GetCurrentTCB           ; RAX = pointer to TCB
    mov [rax+Thread.kernel_stack], rsp ; Save RSP for previous task's kernel stack in the thread's TCB

    ; ------------------------
    ; Setup next task state

    mov rdi, r12                         ; Restore next TCB pointer to RDI for the next call
    call cdecl_SetCurrentTCB
    mov rsp, [r12+Thread.kernel_stack]   ; Change the stack

    mov rdi, r12                       ; Set RDI for GetThreadsPageTable
    call cdecl_GetThreadsPageTable     ; RAX = next cr3
    mov r11, cr3                       ; R11 = current cr3

    cmp r11, rax                       ; Skip virtual address space change if not needed - omit tlb flushes
    je .done
    mov cr3, rax                       ; Load next task's virtual address space

.done:

    pop_all_regs                    ; Restore registers of NEW thread's stack
    add rsp, _all_reg_size          ; Deallocate register save space.

    ret                             ; Load next thread's RIP from its stack

; c_decl
; void SwitchToUserTask(Thread* thread)
;   RDI = thread
; Note: Caller is responsible for ensuring proper environment before calling (disabling IRQs)
SwitchToUserTask:
    ret

;    ; ------------------------
;    ; Save current task state
;
;    sub rsp, _all_reg_size          ; Allocate space for saving registers.
;    push_all_regs                   ; Save registers of calling TCB on ITS stack
;
;    mov r12, rdi                     ; Save next TCB pointer in r12 (non-volatile) to survive C++ calls
;
;    call cdecl_GetCurrentTCB           ; RAX = pointer to TCB
;    mov [rax+Thread.kernel_stack], rsp ; Save RSP for previous task's kernel stack in the thread's TCB
;
;    ; ------------------------
;    ; Setup next task state
;
;    mov rdi, r12                       ; Restore next TCB pointer to RDI for the next call
;    call cdecl_SetCurrentTCB
;    mov rsp, [r12+Thread.user_stack]   ; Change the stack
;
;    mov rdi, r12                     ; Set RDI for GetThreadsPageTable
;    call cdecl_GetThreadsPageTable     ; RAX = next cr3
;    mov r11, cr3                       ; R11 = current cr3
;
;    mov rdi, [r12+Thread.kernel_stack] ; Load next task's kernel stack
;    call cdecl_SetTssRsp0
;
;    cmp r11, rax ; Skip virtual address space change if not needed - omit tlb flushes
;    je .done
;    mov cr3, rax ; Load next task's virtual address space
;
;.done:
;
;    pop_all_regs                    ; Restore registers of NEW thread's stack
;    add rsp, _all_reg_size          ; Deallocate register save space.
;
;    ret ; Load next thread's RIP from its stack
