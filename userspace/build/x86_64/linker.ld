/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2025-2026 The AlkOS Authors */
/* See the AUTHORS file for the full list of contributors. */

/**
 * Userspace Program Linker Script
 *
 * This linker script defines the memory layout for AlkOS userspace programs.
 * The entry point (_start from crt0.o) is guaranteed to be at address 0x400000.
 */
OUTPUT_FORMAT("elf64-x86-64")
ENTRY(_start)

/* Standard User Space Entry Point */
kUserSpaceBase = 0x400000;

SECTIONS
{
    . = kUserSpaceBase;

    .text : ALIGN(4K) {
        KEEP(*(.text._start))
        *(.text)
        *(.text.*)
    }

    .init BLOCK(4K) : ALIGN(4K)
    {
        *(.init)
    }

    .fini BLOCK(4K) : ALIGN(4K)
    {
        *(.fini)
    }

    .rodata : ALIGN(4K) {
        *(.rodata)
        *(.rodata.*)
    }

    .preinit_array : {
        . = ALIGN(8);
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array*))
        PROVIDE_HIDDEN (__preinit_array_end = .);
        . = ALIGN(8);
    }

    .init_array : {
        . = ALIGN(8);
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array*))
        PROVIDE_HIDDEN (__init_array_end = .);
        . = ALIGN(8);
    }

    .fini_array : {
        . = ALIGN(8);
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array*))
        PROVIDE_HIDDEN (__fini_array_end = .);
        . = ALIGN(8);
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
