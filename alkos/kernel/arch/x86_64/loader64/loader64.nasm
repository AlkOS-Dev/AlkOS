          ; Stack
          extern stack_bottom
          extern stack_top

          ; GDT64
          extern GDT64.Pointer
          extern GDT64.Data

          extern PreKernelInit

          global boot64
          section .text
          bits 64
boot64:
;          ; TODO Remap the kernel to the higher half / setup paging again
;          ; TODO Use Multiboot2 information to get the memory map and other information
;          ; Then clear both multiboot header and loader from memory
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
          call PreKernelInit ; 64-bit part of Pre-Kernel Initialization

          ; Infinite loop
          cli
os_hang:
          hlt
          jmp os_hang
