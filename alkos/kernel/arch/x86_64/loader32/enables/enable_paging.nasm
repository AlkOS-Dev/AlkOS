          bits 32

          PAGING_BIT          equ 1 << 31
          PAE_BIT             equ 1 << 5
          LONG_MODE_BIT       equ 1 << 8

          EFER_MSR            equ 0xC0000080

          section .text
          global EnablePaging
          ; void EnablePaging(void* pml4_table)
          ;                   [ebp + 8]
EnablePaging:
          push ebp
          mov ebp, esp

          ; Enable PAE
          mov eax, cr4
          or eax, PAE_BIT
          mov cr4, eax

          ; Load the PML4 table into CR3
          mov eax, [ebp + 8]
          mov cr3, eax

          ; Note: Processors starting from Ice Lake support 5-level paging

;          ; Enable paging
          mov eax, cr0
          or eax, PAGING_BIT
          mov cr0, eax

          mov eax, 0

          pop ebp
          ret
