OUTPUT_FORMAT("elf64-x86-64")
ENTRY(Entry)

PHDRS
{
    text    PT_LOAD FLAGS(5) FILEHDR PHDRS;
    data    PT_LOAD FLAGS(6);
    dynamic PT_DYNAMIC; 
}

SECTIONS
{
    /* PIE executable, base address will be defined at runtime */
    . = 0x1000;

    PROVIDE(loader_64_start = .);

    .text : ALIGN(4K)
    {
        *(.text .text.*)
        *(.rodata .rodata.*)
    } :text

    .dynamic : 
    {
        *(.dynamic)
    } :dynamic

    .data : ALIGN(4K)
    {
        *(.data .data.*)
    } :data

    .bss : ALIGN(4K)
    {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    PROVIDE(loader_64_end = .);

    /* Discard unnecessary sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
    }
}
