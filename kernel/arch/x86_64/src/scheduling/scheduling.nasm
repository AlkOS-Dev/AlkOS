; ------------------------------------------------------------
; Scheduling utilities and crucial functionality
; ------------------------------------------------------------

%include "include/scheduling.nasm"

bits 64

extern cdecl_ConvertContextEntry
extern cdecl_JumpToUserSpaceEntry
extern cdecl_ContextSwitchEntry

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
    mov r12, rdi
    call cdecl_ConvertContextEntry
.done:
    mov  rsp, [r12+Thread.kernel_stack]   ; Change the stack

    pop_all_regs                    ; Restore registers of NEW thread's stack
    add rsp, _all_reg_size          ; Deallocate register save space.
    add rsp, 8                      ; pop error code

    iretq

; c_decl
; void JumpToUserSpace(void (*func)(), void* arg)
;   RDI = func
;   RSI = arg
; Note: Caller is responsible for ensuring proper environment before calling (disabling IRQs)
; Note: FS already should be changed during contex switch
JumpToUserSpace:
    sub rsp, _jump_userspace_stack_space
    push rsi
    ; aligned properly

    mov rsi, rsp
    add rsi, 8
    call cdecl_JumpToUserSpaceEntry

    xor rax, rax
    mov rax, _user_data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    pop rsi
    mov rdi, rsi ; prepare void* arg for func if needed
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

    mov r13, rdi ; save rdi before c++ call
    mov rsi, rsp
    mov rdx, [rsp + _rip_call_offset]

    call cdecl_ContextSwitchEntry

.done:
    mov rsp, [r13+Thread.kernel_stack]   ; Change the stack

    pop_all_regs                    ; Restore registers of NEW thread's stack
    add rsp, _all_reg_size          ; Deallocate register save space.
    add rsp, 8                      ; pop error code

    iretq                             ; Load next thread's RIP from its stack
