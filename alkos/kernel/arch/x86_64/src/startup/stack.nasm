          bits 64

          global stack_bottom
          global stack_top

          section .stack
          align 16
stack_bottom:
          resb 32768
stack_top:
