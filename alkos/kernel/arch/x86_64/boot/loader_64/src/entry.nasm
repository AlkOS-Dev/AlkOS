          ; Stack
          extern stack_bottom
          extern stack_top

          ; GDT64
          extern GDT64.Pointer
          extern GDT64.Data

          extern MainLoader64

          global Entry
          section .text
          bits 64
Entry:
          jmp OsHang ; Temporary hang

          mov esp, stack_top
          mov ebp, esp

          mov r10, 0
          mov r10d, edi ; Edi is a 32 bit LoaderData pointer filled by the loader

          lgdt [GDT64.Pointer]
          mov ax, GDT64.Data
          mov ss, ax
          mov ds, ax
          mov es, ax

          sub rsp, 32 ; shadow space

          mov rdi, 0
          mov edi, r10d
          call MainLoader64 ; Delegate the work to CXX code

          ; Infinite loop
          cli
OsHang:
          hlt
          jmp OsHang
