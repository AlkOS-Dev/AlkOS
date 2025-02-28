          bits 32

          extern GDT64.Pointer
          extern GDT64.Data
          extern GDT64.Code

          section .text
          global EnterKernel
          ; void EnterKernel(void *higher_32_bits_of_kernel_entry, void *lower_32_bits_of_kernel_entry, void *loader_data)
          ;                  [ebp + 8]                             [ebp + 12]                           [ebp + 16]
EnterKernel:
          push ebp
          mov ebp, esp

          ; Retrieve the high 32 bits
          mov eax, [ebp+8]       ; high part
          mov [k_ptr+4], eax     ; store into the upper 32 bits of k_ptr

          ; Retrieve the low 32 bits
          mov esi, [ebp+12]      ; low part
          mov [k_ptr], esi       ; store into the lower 32 bits of k_ptr

          lgdt [GDT64.Pointer]

          mov ax, GDT64.Data
          mov ss, ax
          mov ds, ax
          mov es, ax
          jmp GDT64.Code:jmp_kernel

          section .data
          align 16
k_ptr:    dq 0

          bits 64
jmp_kernel:                  ; https://wiki.osdev.org/Creating_a_64-bit_kernel_using_a_separate_loader
          mov edi, [ebp + 16] ; LoaderData address
          mov rax, [k_ptr]    ; Kernel entry address
          jmp rax
