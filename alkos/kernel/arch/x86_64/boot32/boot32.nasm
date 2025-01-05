          bits 32

          MULTI_BOOT_INFO_T_LOCATION equ stack_top - 4
          FRAMEBUFFER_INFO_T_LOCATION equ stack_top - 8

          ; Includes
          extern PreKernelInit

          ; GDT64
          extern GDT64.Pointer
          extern GDT64.Code

          ; Stack
          extern stack_bottom
          extern stack_top

; The multiboot standard does not define the value of the stack pointer register.
; The stack on x86 must be 16-byte aligned according to the
; System V ABI standard and de-facto extensions. The compiler will assume the
; stack is properly aligned and failure to align the stack will result in
; undefined behavior.

          section   .text
          global    boot32
boot32:
          ; The bootloader has loaded us into 32-bit protected mode on a x86
          ; machine. Interrupts are disabled. Paging is disabled. The processor
          ; state is as defined in the multiboot standard. The kernel has full
          ; control of the CPU. The kernel can only make use of hardware features
          ; and any code it provides as part of itself.

          mov esp, stack_top
          mov ebp, esp

          push ebx ; Multiboot info
          push eax ; Magic number
          call PreKernelInit ; This jumps
