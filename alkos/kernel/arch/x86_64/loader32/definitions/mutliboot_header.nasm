          bits 32
          ; This file defines the multiboot header for the 32-bit loader
          ; It is directly included in the linker script and is placed at the beginning of the kernel image
          ; It is used to signify a multiboot2 compliant kernel image and request information from the bootloader

          ; Constants for Multiboot header
          MULTIBOOT_HEADER_MAGIC equ 0xE85250D6
          ; TAGS
          MULTIBOOT_TAG_ALIGN                  equ 8
          MULTIBOOT_TAG_TYPE_END               equ 0
          MULTIBOOT_TAG_TYPE_CMDLINE           equ 1
          MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME  equ 2
          MULTIBOOT_TAG_TYPE_MODULE            equ 3
          MULTIBOOT_TAG_TYPE_BASIC_MEMINFO     equ 4
          MULTIBOOT_TAG_TYPE_BOOTDEV           equ 5
          MULTIBOOT_TAG_TYPE_MMAP              equ 6
          MULTIBOOT_TAG_TYPE_VBE               equ 7
          MULTIBOOT_TAG_TYPE_FRAMEBUFFER       equ 8
          MULTIBOOT_TAG_TYPE_ELF_SECTIONS      equ 9
          MULTIBOOT_TAG_TYPE_APM               equ 10
          MULTIBOOT_TAG_TYPE_EFI32             equ 11
          MULTIBOOT_TAG_TYPE_EFI64             equ 12
          MULTIBOOT_TAG_TYPE_SMBIOS            equ 13
          MULTIBOOT_TAG_TYPE_ACPI_OLD          equ 14
          MULTIBOOT_TAG_TYPE_ACPI_NEW          equ 15
          MULTIBOOT_TAG_TYPE_NETWORK           equ 16
          MULTIBOOT_TAG_TYPE_EFI_MMAP          equ 17
          MULTIBOOT_TAG_TYPE_EFI_BS            equ 18
          MULTIBOOT_TAG_TYPE_EFI32_IH          equ 19
          MULTIBOOT_TAG_TYPE_EFI64_IH          equ 20
          MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR    equ 21

          MULTIBOOT_HEADER_TAG_END  equ 0
          MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST  equ 1
          MULTIBOOT_HEADER_TAG_ADDRESS  equ 2
          MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS  equ 3
          MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS  equ 4
          MULTIBOOT_HEADER_TAG_FRAMEBUFFER  equ 5
          MULTIBOOT_HEADER_TAG_MODULE_ALIGN  equ 6
          MULTIBOOT_HEADER_TAG_EFI_BS        equ 7
          MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI32  equ 8
          MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI64  equ 9
          MULTIBOOT_HEADER_TAG_RELOCATABLE  equ 10

          MULTIBOOT_ARCHITECTURE_I386  equ 0
          MULTIBOOT_ARCHITECTURE_MIPS32  equ 4
          MULTIBOOT_HEADER_TAG_OPTIONAL equ 1

          MULTIBOOT_LOAD_PREFERENCE_NONE equ 0
          MULTIBOOT_LOAD_PREFERENCE_LOW equ 1
          MULTIBOOT_LOAD_PREFERENCE_HIGH equ 2

          MULTIBOOT_CONSOLE_FLAGS_CONSOLE_REQUIRED equ 1
          MULTIBOOT_CONSOLE_FLAGS_EGA_TEXT_SUPPORTED equ 2

; Multiboot header
          section   .multiboot
          align 8
multiboot_start:
          dd MULTIBOOT_HEADER_MAGIC
          dd MULTIBOOT_ARCHITECTURE_I386
          dd (multiboot_end - multiboot_start)
          dd -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + (multiboot_end - multiboot_start))

          align 8
.bootinfo_request_tag_start:
          dw MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST
          dw 0 ; Flags - 0 means not optional, if bootloader can't provide information, it should fail
          dd (.bootinfo_request_tag_end - .bootinfo_request_tag_start)
          dd MULTIBOOT_TAG_TYPE_MMAP
          dd MULTIBOOT_TAG_TYPE_FRAMEBUFFER
.bootinfo_request_tag_end:

;          ; Framebuffer tag
;          ; https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html#The-framebuffer-tag-of-Multiboot2-header
          align 8
.framebuffer_tag_start:
          dw MULTIBOOT_HEADER_TAG_FRAMEBUFFER
          dw MULTIBOOT_HEADER_TAG_OPTIONAL
          dd (.framebuffer_tag_end - .framebuffer_tag_start)
          dd 0 ; width  - no preference
          dd 0 ; height - no preference
          dd 0 ; depth  - no preference
.framebuffer_tag_end:

          ; End tag (required)
          align 8
	dw MULTIBOOT_HEADER_TAG_END
	dw 0
	dd 8
multiboot_end:
