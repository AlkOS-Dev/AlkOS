          ; This file defines the GDT for the 64-bit loader
          ; It is used to switch to 64-bit mode and set up the data segments

          ; Access bits
          PRESENT   equ 1 << 7 ; Present
          NOT_SYS   equ 0 << 4 ; Not a system segment
          EXEC      equ 1 << 3 ; Executable
          DC        equ 1 << 2 ; Direction/Conforming
          RW        equ 1 << 1 ; Readable/Writable
          AC        equ 1 << 0 ; Accessed
          ; Flags bits
          GRAN_4K   equ 1 << 7 ; 4 KiB granularity
          SZ_32     equ 1 << 6 ; 32-bit segment
          LONG_MODE equ 1 << 5 ; Long mode

          section   .data
          align 16
GDT64:
          .Null:
          dq 0
          .Code: equ $ - GDT64
          dw 0x0000
          dw 0x0000
          db 0x00
          db 0x9A        ; Access byte
          db 0x20        ; Flags
          db 0x00
          .Data: equ $ - GDT64
          dw 0x0000
          dw 0x0000
          db 0x00
          db 0x92        ; Access byte
          db 0x00        ; Flags
          db 0x00
.End:
          .Pointer:
          dw .End - GDT64 - 1 ;
          dq GDT64

GDT64_CODE_SELECTOR: dw GDT64.Code
GDT64_DATA_SELECTOR: dw GDT64.Data

          global GDT64
          global GDT64.Pointer
          global GDT64_CODE_SELECTOR 
          global GDT64_DATA_SELECTOR
