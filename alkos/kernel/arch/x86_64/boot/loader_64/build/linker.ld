OUTPUT_FORMAT("elf64-x86-64")
ENTRY(Entry)

PHDRS
{
    /* The first LOAD segment also contains the ELF file header and program headers. */
    text    PT_LOAD FLAGS(5) FILEHDR PHDRS;
    data    PT_LOAD FLAGS(6);
    dynamic PT_DYNAMIC; /* Add this line */
}

SECTIONS
{
    /* PIE executable, base address will be defined at runtime */
    . = 0x1000;

    PROVIDE(loader_64_start = .);

    /*
     * The TEXT segment (RX).
     * ALIGN(4K) ensures this segment starts on a page boundary.
     * This entire section is mapped to the 'text' PHDR.
    */
    .text : ALIGN(4K)
    {
        *(.text .text.*)
        *(.rodata .rodata.*)
    } :text

    .dynamic : 
    {
        *(.dynamic)
    } :dynamic

    /*
     * The DATA segment (RW).
     * ALIGN(4K) ensures this segment also starts on a new page boundary.
     * This entire section is mapped to the 'data' PHDR.
    */
    .data : ALIGN(4K)
    {
        *(.data .data.*)
    } :data

    /*
     * .bss contains uninitialized data. It follows .data as part of the same RW segment.
     * It does not need its own page alignment, as the start of the segment (.data) is already aligned.
    */
    .bss :
    {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    PROVIDE(loader_64_end = .);

    /* Discard unnecessary sections to keep the executable small */
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
    }
}
