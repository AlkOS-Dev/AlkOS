/**
 * Userspace Program Linker Script
 *
 * This linker script defines the memory layout for AlkOS userspace programs.
 * The entry point (_start from crt0.o) is guaranteed to be at address 0x400000.
 */

ENTRY(_start)

SECTIONS
{
    /* Standard User Space Entry Point */
    . = 0x400000;

    .text : ALIGN(4K) {
        KEEP(*(.text._start))
        *(.text)
        *(.text.*)
    }

    .rodata : ALIGN(4K) {
        *(.rodata)
        *(.rodata.*)
    }

    .data : ALIGN(4K) {
        *(.data)
        *(.data.*)
    }

    .bss : ALIGN(4K) {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }
}
