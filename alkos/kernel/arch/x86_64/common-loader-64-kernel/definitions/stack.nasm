          bits 64

          global stack_bottom
          global stack_top

          section .bss
          align 16
stack_bottom:
          resb 65536
stack_top:
