          ; Stack
          extern stack_bottom
          extern stack_top

          ; GDT64
          extern GDT64.Pointer
          extern GDT64_DATA_SELECTOR ; Use the new symbol

          extern MainLoader64

          global Entry
          section .text
          bits 64
Entry:
          ; Set up the stack using a RIP-relative address
          lea rsp, [rel stack_top]
          mov rbp, rsp

          mov r10, 0
          mov r10d, edi ; Edi is a 32 bit LoaderData pointer filled by the loader

          ; Load the GDT pointer using a RIP-relative address
          lea rax, [rel GDT64.Pointer]
          lgdt [rax]

          ; Load the data segment selector using a RIP-relative memory load
          mov ax, [rel GDT64_DATA_SELECTOR]
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
