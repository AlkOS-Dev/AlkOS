          ; Stack
          extern stack_bottom
          extern stack_top

          ; GDT64
          extern GDT64.Pointer
          extern GDT64.Data

          ; 64-bit part of Pre-Kernel Initialization
          extern PreKernelInit

          ; GCC compiler global constructors initialization
          extern _init
          extern _fini

          ; Kernel Entry Point
          extern KernelMain

          global boot64
          section .text
          bits 64
boot64:
          ; TODO Remap the kernel to the higher half / setup paging again
          ; TODO Use Multiboot2 information to get the memory map and other information
          ; Then clear both multiboot header and loader from memory
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

          ; Invoke CXX global constructors
          call _init

          ; Call actual kernel entry point
          call KernelMain

          ; Not actually needed (as we expect to never return from Kernel), but exists for completeness
          call _fini

          ; Infinite loop
          cli
os_hang:
          hlt
          jmp os_hang
