          bits 32
          ; This file defines an entry point for the 32-bit loader
          ; The bootloader has loaded us into 32-bit protected mode on a x86
          ; machine. Interrupts are disabled. Paging is disabled. The processor
          ; state is as defined in the multiboot standard. The kernel has full
          ; control of the CPU. The kernel can only make use of hardware features
          ; and any code it provides as part of itself.

          ; Includes
          extern MainLoader32

          ; Stack
          extern stack_bottom
          extern stack_top

          ; The multiboot standard does not define the value of the stack pointer register.
          ; The stack on x86 must be 16-byte aligned according to the
          ; System V ABI standard and de-facto extensions. The compiler will assume the
          ; stack is properly aligned and failure to align the stack will result in
          ; undefined behavior.

          section   .text
          global    Entry
Entry:

          mov esp, stack_top
          mov ebp, esp

          push ebx ; Multiboot info
          push eax ; Magic number
          call MainLoader32 ; Delegate the work to CXX code
