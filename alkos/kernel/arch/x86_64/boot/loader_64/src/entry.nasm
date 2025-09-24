          ; Stack
          extern stack_bottom
          extern stack_top

          ; GDT64
          extern GDT64.Pointer
          extern GDT64_DATA_SELECTOR 

          extern MainLoader64

          global Entry
          section .text
          bits 64
Entry:
          ; Note: RIP Relative Addressing for PIC

          ; Set up the stack 
          lea rsp, [rel stack_top]
          mov rbp, rsp

          mov r10, 0
          mov r10d, edi ; edi - Transition Data

          ; Load the GDT pointer 
          lea rax, [rel GDT64.Pointer]
          lgdt [rax]

          ; Load the data segment selector
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
