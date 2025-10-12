          ; Stack
          extern stack_bottom
          extern stack_top

          ; GDT64
          extern GDT64.Pointer
          extern GDT64.Data

          ; GCC compiler global destructor support
          extern _fini

          ; Pre-Kernel Initialization that couldn't be in loaders
          extern PreKernelInit

          ; Kernel Entry Point
          extern KernelMain

          global alkos.entry
          section .text
          bits 64
alkos.entry:
          mov rsp, stack_top
          mov rbp, rsp

          ; LoaderData *loader_data = rdi
          call PreKernelInit

          ; Call actual kernel entry point
          call KernelMain

          ; Not actually needed (as we expect to never return from Kernel), but exists for completeness
          call _fini

          ; Infinite loop
os_hang:
          hlt
          jmp os_hang
