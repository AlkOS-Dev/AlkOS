; ------------------------------------
; ISR with full GPR save/restore
; ------------------------------------

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

; Size needed to save the registers on the stack
_reg_size equ 8*10

; Shadow space required for C++ function calls
_shadow_space equ 8*4

; TODO: This is a hotfix. In the past only
; "volatile" registers were pushed and restored
; errors with interrupts during ACPI init happened. 
; Saving all regs  was how it was solved
; Check if pushing all is really needed
%macro push_regs 0
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
%macro pop_regs 0
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

; Macro for CPU exceptions that DO NOT push an error code.
; A dummy error code is pushed to create a consistent stack frame.
; Calls 'HandleException(u16 lirq, ExceptionData* data)'.
%macro exception_wrapper 1
isr_wrapper_%+%1:
    push 0                      ; Push a dummy error code for alignment.
    sub rsp, _reg_size          ; Allocate space for saving registers.
    push_regs                   ; Save registers.
    sub rsp, _shadow_space      ; Allocate shadow space.
    cld                         ; Clear direction flag.
    mov rdi, %1                 ; Arg1: interrupt number.
    lea rsi, [rsp + _shadow_space + _reg_size + 8] ; Arg2: pointer to stack frame.
    call HandleException
    add rsp, _shadow_space      ; Deallocate shadow space.
    pop_regs                    ; Restore registers.
    add rsp, _reg_size          ; Deallocate register save space.
    add rsp, 8                  ; Pop dummy error code.
    iretq
%endmacro

; Macro for CPU exceptions that DO push an error code.
; Calls 'HandleException(u16 lirq, ExceptionData* data)'.
%macro exception_error_wrapper 1
isr_wrapper_%+%1:
    sub rsp, _reg_size          ; Allocate space for saving registers.
    push_regs                   ; Save registers.
    sub rsp, _shadow_space      ; Allocate shadow space.
    cld                         ; Clear direction flag.
    mov rdi, %1                 ; Arg1: interrupt number.
    lea rsi, [rsp + _shadow_space + _reg_size + 8] ; Arg2: pointer to stack frame.
    call HandleException
    add rsp, _shadow_space      ; Deallocate shadow space.
    pop_regs                    ; Restore registers.
    add rsp, _reg_size          ; Deallocate register save space.
    add rsp, 8                  ; Pop real error code.
    iretq
%endmacro

; Macro for hardware or software interrupts.
; Calls a handler with the signature 'void handler(u16 lirq)'.
%macro interrupt_wrapper 3 ; %1: Logical IRQ, %2: idt idx %3: C handler function
isr_wrapper_%+%2:
    sub rsp, _reg_size          ; Allocate space for saving registers.
    push_regs                   ; Save registers.
    sub rsp, _shadow_space      ; Allocate shadow space for function calls.
    cld                         ; Clear direction flag for string operations.
    mov rdi, %1                 ; Pass the mapped IRQ number as the first argument.
    call %3                     ; Call the specific ISR handler.
    add rsp, _shadow_space      ; Deallocate shadow space.
    pop_regs                    ; Restore registers.
    add rsp, _reg_size          ; Deallocate register save space.
    iretq                       ; Return from interrupt.
%endmacro

; ------------------------------
; ISR wrappers definitions
; ------------------------------

bits 64

extern HandleException
extern HandleHardwareInterrupt
extern HandleSoftwareInterrupt

section .text

; Intel-defined interrupts (0-31) -> HandleException
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

; IRQs for PICs (32–47) -> HandleHardwareInterrupt
interrupt_wrapper 0, 32, HandleHardwareInterrupt ; IRQ0: System timer
interrupt_wrapper 1, 33, HandleHardwareInterrupt ; IRQ1: Keyboard
interrupt_wrapper 2, 34, HandleHardwareInterrupt ; IRQ2: Cascade
interrupt_wrapper 3, 35, HandleHardwareInterrupt ; IRQ3: Serial port 2
interrupt_wrapper 4, 36, HandleHardwareInterrupt ; IRQ4: Serial port 1
interrupt_wrapper 5, 37, HandleHardwareInterrupt ; IRQ5: Parallel port 2 / sound card
interrupt_wrapper 6, 38, HandleHardwareInterrupt ; IRQ6: Floppy controller
interrupt_wrapper 7, 39, HandleHardwareInterrupt ; IRQ7: Parallel port 1
interrupt_wrapper 8, 40, HandleHardwareInterrupt ; IRQ8: Real-time clock
interrupt_wrapper 9, 41, HandleHardwareInterrupt ; IRQ9: Free for peripherals
interrupt_wrapper 10, 42, HandleHardwareInterrupt ; IRQ10: Free for peripherals
interrupt_wrapper 11, 43, HandleHardwareInterrupt ; IRQ11: Free for peripherals
interrupt_wrapper 12, 44, HandleHardwareInterrupt ; IRQ12: Mouse
interrupt_wrapper 13, 45, HandleHardwareInterrupt ; IRQ13: FPU (legacy)
interrupt_wrapper 14, 46, HandleHardwareInterrupt ; IRQ14: Primary ATA channel
interrupt_wrapper 15, 47, HandleHardwareInterrupt ; IRQ15: Secondary ATA channel


; Software interrupts (48–63) -> HandleSoftwareInterrupt
interrupt_wrapper 0, 48, HandleSoftwareInterrupt
interrupt_wrapper 1, 49, HandleSoftwareInterrupt
interrupt_wrapper 2, 50, HandleSoftwareInterrupt
interrupt_wrapper 3, 51, HandleSoftwareInterrupt
interrupt_wrapper 4, 52, HandleSoftwareInterrupt
interrupt_wrapper 5, 53, HandleSoftwareInterrupt
interrupt_wrapper 6, 54, HandleSoftwareInterrupt
interrupt_wrapper 7, 55, HandleSoftwareInterrupt
interrupt_wrapper 8, 56, HandleSoftwareInterrupt
interrupt_wrapper 9, 57, HandleSoftwareInterrupt
interrupt_wrapper 10, 58, HandleSoftwareInterrupt
interrupt_wrapper 11, 59, HandleSoftwareInterrupt
interrupt_wrapper 12, 60, HandleSoftwareInterrupt
interrupt_wrapper 13, 61, HandleSoftwareInterrupt
interrupt_wrapper 14, 62, HandleSoftwareInterrupt
interrupt_wrapper 15, 63, HandleSoftwareInterrupt


; Total number of ISRs.
_num_isrs equ 64

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
