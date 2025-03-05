          bits 32

          global stack_bottom
          global stack_top

          section .bss
          align 16
stack_bottom:
          resb 32768
stack_top:
