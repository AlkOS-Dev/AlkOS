          [bits 64]

          section .text
          global EnterKernel
          ; void EnterKernel(u64 kernel_entry, LoaderData *loader_data)
          ;                  [rdi]            [rsi]
EnterKernel:
          mov r10, rdi ; Swap rdi and rsi
          mov rdi, rsi
          mov rsi, r10 ; rdi = loader_data, rsi = kernel_entry
          jmp rsi
