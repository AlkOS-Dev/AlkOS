          [bits 32]
          ; This is a function that switches to 64-bit mode and jumps to some program entry point while
          ; preserving the loader_data pointer.

          extern GDT64.Pointer
          extern GDT64.Data
          extern GDT64.Code

          section .data
          align 16
k_ptr:    dq 0          ; Will store the full 64-bit kernel entry address
loader_data_ptr: dq 0          ; To preserve the loader_data pointer for 64-bit mode

          section .bss
          align 16
kernel_stack:       resb 4096     ; 4KB temporary stack

          section .text
          global EnterElf64
          ; void EnterElf64(void *kernel_entry_high, void *kernel_entry_low, void *loader_data)
          ;                  [ebp+8]                  [ebp+12]                [ebp+16]
EnterElf64:
          push ebp
          mov ebp, esp

          ; Save the kernel entry address parts into k_ptr
          mov eax, [ebp+8]       ; high 32 bits
          mov [k_ptr+4], eax
          mov eax, [ebp+12]      ; low 32 bits
          mov [k_ptr], eax

          ; Save the loader_data pointer into a global variable
          mov eax, [ebp+16]
          mov [loader_data_ptr], eax

          ; Load the 64-bit GDT
          lgdt [GDT64.Pointer]

          ; Set data segments to the 64-bit data selector
          mov ax, GDT64.Data
          mov ss, ax
          mov ds, ax
          mov es, ax

          ; Set up a new stack pointer for 64-bit mode (temporary stack)
          lea eax, [kernel_stack + 4096]  ; new stack top
          ; Save it in EBX (which will be preserved across the far jump)
          mov ebx, eax

          ; Far jump to switch into 64-bit mode
          jmp GDT64.Code:jmp_elf

          [bits 64]
jmp_elf:
          ; Set the new 64-bit stack pointer using the value passed in EBX
          mov rax, rbx
          mov rsp, rax

          ; Load loader_data pointer into rdi
          mov rdi, [loader_data_ptr]

          ; Load the 64-bit kernel entry address from k_ptr
          mov rax, [k_ptr]
          ; Jump to the kernel entry point
          jmp rax
