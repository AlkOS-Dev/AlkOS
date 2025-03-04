          bits 32

          section .bss
          align 8
          ; For memcpy64: store dest (qword), src (qword), n (qword)
memcpy64_args: resq 3
          ; For memset64: store dest (qword), c (qword), n (qword)
memset64_args: resq 3

          section .text
          global memcpy64
          global memset64

          ; ---------------------------
          ; 32-bit stub for memcpy64
          ; ---------------------------
          ; This code runs in 32-bit mode.
          ; It gathers the two halves of each u64 parameter and stores them.

          ; void memcpy64(u32 dest_low, u32 dest_high, u32 src_low, u32 src_high, u32 n_low, u32 n_high)
          ;              [ebp+8]        [ebp+12]       [ebp+16]     [ebp+20]      [ebp+24]   [ebp+28]
memcpy64:
          push ebp
          mov ebp, esp

          mov ebx, [ebp+8]    ; dest_low
          mov eax, [ebp+12]   ; dest_high
          mov edx, [ebp+16]   ; src_low
          mov ecx, [ebp+20]   ; src_high
          mov edi, [ebp+24]   ; n_low
          mov esi, [ebp+28]   ; n_high

          ; Store dest as a 64-bit value in little-endian
          mov dword [memcpy64_args], ebx
          mov dword [memcpy64_args+4], eax

          ; Store src (next 8 bytes)
          mov dword [memcpy64_args+8], edx
          mov dword [memcpy64_args+12], ecx

          ; Store n (last 8 bytes)
          mov dword [memcpy64_args+16], edi
          mov dword [memcpy64_args+20], esi

          ; Jump to the 64-bit routine.
          jmp memcpy64_64

          ; ---------------------------
          ; 32-bit stub for memset64
          ; ---------------------------
          ; void memset64(u32 dest_low, u32 dest_high, u32 c, u32 n_low, u32 n_high)
          ;               [ebp+8]        [ebp+12]     [ebp+16] [ebp+20]    [ebp+24]
          memset64:
              push ebp
              mov ebp, esp
              mov ebx, [ebp+8]
              mov eax, [ebp+12]
              mov ecx, [ebp+16]
              mov edi, [ebp+20]
              mov edx, [ebp+24]

              ; Store dest into memset64_args
              mov dword [memset64_args], ebx
              mov dword [memset64_args+4], eax

              ; Store c as a 64-bit value (zero extended)
              mov dword [memset64_args+8], ecx
              mov dword [memset64_args+12], 0

              ; Store n
              mov dword [memset64_args+16], edi    ; n_low
              mov dword [memset64_args+20], edx    ; n_high

              jmp memset64_64

          ; ----------------------------------------------------------
          ; Now, switch to 64-bit mode for the actual implementations.
          ; ----------------------------------------------------------

          [BITS 64]
          memcpy64_64:
              [BITS 64]
              ; Load parameters from the temporary storage.
              mov rdi, [memcpy64_args]      ; rdi = dest
              mov rsi, [memcpy64_args+8]    ; rsi = src
              mov rdx, [memcpy64_args+16]   ; rdx = n
              cld
              ; Use rep movsb to copy n bytes.
              rep movsb
              ; Jump back to 32-bit stub exit.
              jmp memcpy64_exit_32

          memcpy64_exit_32:
              ; Switch back to 32-bit mode.
              [BITS 32]
              pop ebp
              ret

          memset64_64:
              [BITS 64]
              ; Load parameters from temporary storage.
              mov rdi, [memset64_args]      ; rdi = dest
              ; Although c was only 32-bit, it was stored in a qword.
              mov rsi, [memset64_args+8]    ; rsi = c
              mov rdx, [memset64_args+16]   ; rdx = n
              cld
              rep stosb
              jmp memset64_exit_32

          memset64_exit_32:
              [BITS 32]
              pop ebp
              ret
