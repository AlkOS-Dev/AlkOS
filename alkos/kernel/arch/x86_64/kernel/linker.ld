OUTPUT_FORMAT("elf64-x86-64")
ENTRY(alkos.entry)

kKernelBase = 0xFFFFFFFF80000000;

SECTIONS
{
	. = kKernelBase;

	.init BLOCK(4K) : ALIGN(4K)
          {
                    *(.init)
          }

          .fini BLOCK(4K) : ALIGN(4K)
          {
                    *(.fini)
          }

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

	/* Provide the start and end symbols for the kernel */
	PROVIDE(kKernelVirtualAddressStart = kKernelBase);
	PROVIDE(kKernelVirtualAddressEnd = .);
}
