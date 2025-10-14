          bits 32

          section .text

;------------------------------------------------------------------------------;
;                                    CpuId                                     ;
;------------------------------------------------------------------------------;

          global CheckCpuId
CheckCpuId:
          ; Check if CPUID is supported by flipping the ID bit (bit 21) in
          ; the FLAGS register. If we can flip it, CPUID is avaliable.

          ; Copy FLAGS in to EAX via stack
          pushfd
          pop eax

          mov ecx, eax

          ; Flip the ID bit
          xor eax, 1 << 21

          ; Copy EAX to flags via stack
          push eax
          popfd

          ; Copy flags back to EAX (with the flipped bit if CPUID is supported)
          pushfd
          pop eax

          ; Restore FLAGS
          push ecx
          popfd

          ; If the bit was flipped, CPUID is supported
          xor eax, ecx
          jz .no_cpuid
          mov eax, 0
          ret
.no_cpuid:
          mov eax, 1
          ret

;------------------------------------------------------------------------------;
;                                   LongMode                                   ;
;------------------------------------------------------------------------------;

          EXTENDED_FUNCTIONS_THRESHOLD equ 0x80000000
          LONG_MODE_BIT equ 1 << 29

          section .text
          global CheckLongMode
CheckLongMode:
          ; Check the highest possible function supported by CPUID

          ; Source: Intel® 64 and IA-32 Architectures Software Developer’s Manual
          ; Volume 2A: CPUID-CPU Identification
          ; INPUT EAX = 0: Returns CPUID’s Highest Value for Basic Processor Information and the Vendor Identification String
          ; OUTPUT EAX = Highest value for basic processor information
          ; OUTPUT EBX, EDX, ECX = Vendor Identification String - "GenuineIntel" or "AuthenticAMD"
          mov eax, 0
          cpuid ; Highest possible function in EAX
          cmp eax, EXTENDED_FUNCTIONS_THRESHOLD
          jl .no_long_mode

          ; Check if extended functions of CPUID are supported
          ; Source: Intel® 64 and IA-32 Architectures Software Developer’s Manual
          ; Volume 2A: CPUID-CPU Identification
          ; INPUT EAX = 80000000h: Get Highest Extended Function Supported
          ; OUTPUT EAX = Highest extended function supported
          mov eax, EXTENDED_FUNCTIONS_THRESHOLD
          cpuid
          cmp eax, EXTENDED_FUNCTIONS_THRESHOLD + 1
          jl .no_long_mode

          ; Check if long mode is supported
          ; Source: Intel® 64 and IA-32 Architectures Software Developer’s Manual
          ; Volume 2A: CPUID-CPU Identification
          ; INPUT EAX = 80000001h: Extended Processor Info and Feature Bits
          ; Changes EAX, EBX, ECX, EDX
          ; We are interested in EDX bit 29
          mov eax, EXTENDED_FUNCTIONS_THRESHOLD + 1
          cpuid
          test edx, LONG_MODE_BIT
          jz .no_long_mode

          ; Long mode is supported
          mov eax, 0
          ret

.no_long_mode:
          mov eax, 1
          ret
