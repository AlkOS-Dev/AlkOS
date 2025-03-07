          ; Stack
          extern stack_bottom
          extern stack_top

          ; GDT64
          extern GDT64.Pointer
          extern GDT64.Data

          ; GCC compiler global constructors initialization
          extern _init
          extern _fini

          ; Kernel Entry Point
          extern KernelMain

          global alkos.entry
          section .text
          bits 64
alkos.entry:
          mov rsp, stack_top
          mov rbp, rsp

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
