          [bits 64]

          section .text
          global EnterKernel
          ; void EnterKernel(u64 kernel_entry)
          ;                  [rdi]
EnterKernel:
          jmp rdi
