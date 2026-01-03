; ------------------------------------
; ISR with full GPR save/restore
; ------------------------------------

%include "include/scheduling.nasm"

extern cdecl_SetKernelGs
extern cdecl_ContextSwitchOnInterrupt
extern cdecl_HandleException
extern cdecl_HandleHardwareInterrupt
extern cdecl_HandleSoftwareInterrupt
extern cdecl_UpdateTcbOnSyscallEntry
extern cdecl_UpdateTcbOnSyscallExit

%macro load_user_gs_if_needed 0
    test qword [rsp + _cs_int_frame_offset], 3
    jz .skip_user_gs_swap
    call cdecl_SetKernelGs
    swapgs
.skip_user_gs_swap:
%endmacro

%macro context_switch_if_needed 0
    cmp rax, 0
    je .done                         ; Omit context switch if there is no need to change it

    mov r13, rax                     ; save next TCB

    mov rdi, r13
    mov rsi, rsp
    call cdecl_ContextSwitchOnInterrupt

    mov rsp, [r13+Thread.kernel_stack]   ; Change the stack
.done:

    load_user_gs_if_needed

    pop_all_regs                    ; Restore registers.
    add rsp, _all_reg_size          ; Deallocate register save space.
    add rsp, 8                  ; Pop dummy error code.
    iretq
%endmacro

%macro load_kernel_gs_if_needed 0
    test qword [rsp + _cs_int_frame_offset], 3
    jz .skip_kernel_gs_swap
    swapgs
.skip_kernel_gs_swap:
%endmacro

; Macro for CPU exceptions that DO NOT push an error code.
; A dummy error code is pushed to create a consistent stack frame.
; Calls 'cdecl_HandleException(u16 lirq, ExceptionData* data)'.
%macro exception_wrapper 1
isr_wrapper_%+%1:
    push 0                      ; Push a dummy error code for alignment.
    sub rsp, _all_reg_size          ; Allocate space for saving registers.
    push_all_regs                   ; Save registers.

    load_kernel_gs_if_needed

    cld                         ; Clear direction flag.
    mov rdi, %1                 ; Arg1: interrupt number.
    mov rsi, rsp                ; Arg2: pointer to stack frame.
    call cdecl_HandleException

    context_switch_if_needed
%endmacro

; Macro for CPU exceptions that DO push an error code.
; Calls 'cdecl_HandleException(u16 lirq, ExceptionData* data)'.
%macro exception_error_wrapper 1
isr_wrapper_%+%1:
    sub rsp, _all_reg_size          ; Allocate space for saving registers.
    push_all_regs                   ; Save registers.

    load_kernel_gs_if_needed

    cld                         ; Clear direction flag.
    mov rdi, %1                 ; Arg1: interrupt number.
    mov rsi, rsp                ; Arg2: pointer to stack frame.
    call cdecl_HandleException

    context_switch_if_needed
%endmacro

; Macro for hardware or software interrupts.
; Calls a handler with the signature 'void handler(u16 lirq)'.
%macro interrupt_wrapper 3 ; %1: Logical IRQ, %2: idt idx %3: C handler function
isr_wrapper_%+%2:
    push 0                      ; Push a dummy error code for alignment.
    sub rsp, _all_reg_size          ; Allocate space for saving registers.
    push_all_regs                   ; Save registers.

    load_kernel_gs_if_needed

    cld                         ; Clear direction flag for string operations.
    mov rdi, %1                 ; Pass the mapped IRQ number as the first argument.
    mov rsi, rsp                ; Arg2: pointer to stack frame.
    call %3                     ; Call the specific ISR handler.

    context_switch_if_needed
%endmacro

bits 64
section .text

; ------------------------------
; Syscall wrapper definitions
; ------------------------------

extern g_syscall_dispatch_table
extern g_syscall_count

; Expected system call convention:
; - RAX: syscall number
; - RDI: arg0
; - RSI: arg1
; - RDX: arg2
; - R10: arg3
; - R8:  arg4
; - R9:  arg5
; - Return value: RAX
isr_wrapper_128:  ; Syscall interrupt (128)
    sub rsp, _sysv_reg_size          ; Allocate space for saving registers.
    push_sysv_regs       ; Save registers
    cld                              ; Clear direction flag for string operations.

    ; Syscall to Sys V ABI conversion
    mov rcx, r10

    ; Check syscall number bounds
    cmp rax, [rel g_syscall_count]
    jae .invalid_syscall

    swapgs
    call cdecl_UpdateTcbOnSyscallEntry

    ; Get pointer to syscall_dispatch_table and dispatch
    mov qword rax, [rsp + _rax]
    call qword [rel g_syscall_dispatch_table + rax*8]
    mov qword [rsp + _rax], rax
    jmp .return

.invalid_syscall:
    mov qword [rsp + _rax], -1                      ; Set error return value

.return:

    call cdecl_UpdateTcbOnSyscallExit

    call cdecl_SetKernelGs
    swapgs

    pop_sysv_regs        ; Restore registers
    add rsp, _sysv_reg_size          ; Deallocate register save space.
    iretq                            ; Return from interrupt.

; ------------------------------
; ISR wrappers definitions
; ------------------------------

; Intel-defined interrupts (0-31) -> cdecl_HandleException
exception_wrapper 0  ; Division Error: Divide by zero error
exception_wrapper 1  ; Debug: Reserved for debugging exceptions
exception_wrapper 2  ; Non-Maskable Interrupt: Non-maskable interrupt detected
exception_wrapper 3  ; Breakpoint: Breakpoint detected
exception_wrapper 4  ; Overflow: Overflow detected
exception_wrapper 5  ; Bound Range Exceeded: Bound range exceeded
exception_wrapper 6  ; Invalid Opcode: Invalid instruction
exception_wrapper 7  ; Device Not Available: FPU device unavailable
exception_error_wrapper 8  ; Double Fault: Critical CPU error
exception_wrapper 9  ; Coprocessor Segment Overrun: Legacy interrupt (not used)
exception_error_wrapper 10 ; Invalid TSS: Invalid Task State Segment
exception_error_wrapper 11 ; Segment Not Present: Segment not present in memory
exception_error_wrapper 12 ; Stack Segment Fault: Stack-related fault
exception_error_wrapper 13 ; General Protection Fault: Memory access violation
exception_error_wrapper 14 ; Page Fault: Page not found in memory
exception_wrapper 15 ; Reserved: Reserved by Intel
exception_wrapper 16 ; x87 Floating-Point Exception: FPU error
exception_error_wrapper 17 ; Alignment Check: Alignment error
exception_wrapper 18 ; Machine Check: Hardware failure detected
exception_wrapper 19 ; SIMD Floating-Point Exception: SIMD operation error
exception_wrapper 20 ; Virtualization Exception: Virtualization error
exception_error_wrapper 21 ; Control Protection Exception
exception_wrapper 22 ; Reserved: Reserved by Intel
exception_wrapper 23 ; Reserved: Reserved by Intel
exception_wrapper 24 ; Reserved: Reserved by Intel
exception_wrapper 25 ; Reserved: Reserved by Intel
exception_wrapper 26 ; Reserved: Reserved by Intel
exception_wrapper 27 ; Reserved: Reserved by Intel
exception_wrapper 28 ; Hypervisor Injection Exception
exception_error_wrapper 29 ; VMM Communication Exception
exception_error_wrapper 30 ; Security Exception: Security-related error
exception_wrapper 31 ; Reserved: Reserved by Intel

; IRQs for PICs (32–47) -> cdecl_HandleHardwareInterrupt
interrupt_wrapper 0, 32, cdecl_HandleHardwareInterrupt ; IRQ0: System timer
interrupt_wrapper 1, 33, cdecl_HandleHardwareInterrupt ; IRQ1: Keyboard
interrupt_wrapper 2, 34, cdecl_HandleHardwareInterrupt ; IRQ2: Cascade
interrupt_wrapper 3, 35, cdecl_HandleHardwareInterrupt ; IRQ3: Serial port 2
interrupt_wrapper 4, 36, cdecl_HandleHardwareInterrupt ; IRQ4: Serial port 1
interrupt_wrapper 5, 37, cdecl_HandleHardwareInterrupt ; IRQ5: Parallel port 2 / sound card
interrupt_wrapper 6, 38, cdecl_HandleHardwareInterrupt ; IRQ6: Floppy controller
interrupt_wrapper 7, 39, cdecl_HandleHardwareInterrupt ; IRQ7: Parallel port 1
interrupt_wrapper 8, 40, cdecl_HandleHardwareInterrupt ; IRQ8: Real-time clock
interrupt_wrapper 9, 41, cdecl_HandleHardwareInterrupt ; IRQ9: Free for peripherals
interrupt_wrapper 10, 42, cdecl_HandleHardwareInterrupt ; IRQ10: Free for peripherals
interrupt_wrapper 11, 43, cdecl_HandleHardwareInterrupt ; IRQ11: Free for peripherals
interrupt_wrapper 12, 44, cdecl_HandleHardwareInterrupt ; IRQ12: Mouse
interrupt_wrapper 13, 45, cdecl_HandleHardwareInterrupt ; IRQ13: FPU (legacy)
interrupt_wrapper 14, 46, cdecl_HandleHardwareInterrupt ; IRQ14: Primary ATA channel
interrupt_wrapper 15, 47, cdecl_HandleHardwareInterrupt ; IRQ15: Secondary ATA channel

; IRQs for APICs and other devices (48–127) -> cdecl_HandleHardwareInterrupt
%assign idt_num 48
%assign irq_num 16
%rep 127-48+1
    interrupt_wrapper irq_num, idt_num, cdecl_HandleHardwareInterrupt
%assign idt_num idt_num + 1
%assign irq_num irq_num + 1
%endrep

; Software interrupts (128–143) -> cdecl_HandleSoftwareInterrupt
; interrupt_wrapper 0, 128, cdecl_HandleSoftwareInterrupt ; Syscall handled separately above
interrupt_wrapper 0, 129, cdecl_HandleSoftwareInterrupt
interrupt_wrapper 1, 130, cdecl_HandleSoftwareInterrupt
interrupt_wrapper 2, 131, cdecl_HandleSoftwareInterrupt
interrupt_wrapper 3, 132, cdecl_HandleSoftwareInterrupt
interrupt_wrapper 4, 133, cdecl_HandleSoftwareInterrupt
interrupt_wrapper 5, 134, cdecl_HandleSoftwareInterrupt
interrupt_wrapper 6, 135, cdecl_HandleSoftwareInterrupt
interrupt_wrapper 7, 136, cdecl_HandleSoftwareInterrupt
interrupt_wrapper 8, 137, cdecl_HandleSoftwareInterrupt
interrupt_wrapper 9, 138, cdecl_HandleSoftwareInterrupt
interrupt_wrapper 10, 139, cdecl_HandleSoftwareInterrupt
interrupt_wrapper 11, 140, cdecl_HandleSoftwareInterrupt
interrupt_wrapper 12, 141, cdecl_HandleSoftwareInterrupt
interrupt_wrapper 13, 142, cdecl_HandleSoftwareInterrupt
interrupt_wrapper 14, 143, cdecl_HandleSoftwareInterrupt


; Total number of ISRs.
_num_isrs equ 144

; ----------------------------------
; ISR wrapper table definition
; ----------------------------------

section .data

; Global table of ISR wrappers.
global IsrWrapperTable
IsrWrapperTable:
%assign i 0
%rep    _num_isrs
    dq isr_wrapper_%+i
%assign i i+1
%endrep
