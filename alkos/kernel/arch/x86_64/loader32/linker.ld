OUTPUT_FORMAT("elf32-i386")
ENTRY(boot32)

SECTIONS
{
          /* https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html#OS-image-format */
          /* " The Multiboot2 header must be contained completely within the first 32768 bytes of the OS image. " */
	. = 2k;
	PROVIDE(multiboot_header_start = .);

          .multiboot ALIGN(64) : AT(.)
          {
                    KEEP(*(.multiboot))
	}

          PROVIDE(multiboot_header_end = .);


	. = ALIGN(4K);

	PROVIDE(loader_start = .);

	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text)
	}

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}

	PROVIDE(loader_end = .);
}
