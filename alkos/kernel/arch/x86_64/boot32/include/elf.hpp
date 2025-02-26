#ifndef ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_ELF_HPP_
#define ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_ELF_HPP_

#include <stdint.h>

// ELF-64 Structures

// Taken from glibc https://github.com/lattera/glibc/blob/master/elf/elf.h#L60
// And https://krinkinmu.github.io/2020/11/15/loading-elf-image.html

/**
 * @brief ELF-64 header structure
 *
 * This structure represents the ELF-64 header.
 * It is used to identify the file as an ELF file and to locate the program header table.
 * The program header table contains information about the segments in the file.
 * The entry point of the program is also stored in this header. Among other useful information.
 */
struct Elf64_Ehdr {
    unsigned char e_ident[16];  ///< Magic number and other info
    u16 e_type;                 ///< Object file type
    u16 e_machine;              ///< Architecture
    u32 e_version;              ///< Object file version
    u64 e_entry;                ///< Entry point virtual address
    u64 e_phoff;                ///< Program header table file offset
    u64 e_shoff;                ///< Section header table file offset
    u32 e_flags;                ///< Processor-specific flags
    u16 e_ehsize;               ///< ELF header size in bytes
    u16 e_phentsize;            ///< Program header table entry size
    u16 e_phnum;                ///< Program header table entry count
    u16 e_shentsize;            ///< Section header table entry size
    u16 e_shnum;                ///< Section header table entry count
    u16 e_shstrndx;             ///< Section header string table index

    static constexpr byte kElfMagic0 = 0x7F;  ///< ELF identification magic number byte 0
    static constexpr byte kElfMagic1 = 'E';   ///< ELF identification magic number byte 1
    static constexpr byte kElfMagic2 = 'L';   ///< ELF identification magic number byte 2
    static constexpr byte kElfMagic3 = 'F';   ///< ELF identification magic number byte 3

    static constexpr u16 kSupportedMachine = 0x3E;  ///< EM_X86_64
} __attribute__((packed));

/**
 * @brief ELF-64 program header structure
 *
 * This structure represents an entry in the program header table.
 * It contains information about a segment in the file.
 * Such as the offset in the file, the virtual address in memory, the physical address, the size in
 * the file, the size in memory, and the alignment etc.
 *
 */
struct Elf64_Phdr {
    u32 p_type;    ///< Segment type
    u32 p_flags;   ///< Segment flags
    u64 p_offset;  ///< Segment file offset
    u64 p_vaddr;   ///< Segment virtual address
    u64 p_paddr;   ///< Segment physical address
    u64 p_filesz;  ///< Segment size in file
    u64 p_memsz;   ///< Segment size in memory
    u64 p_align;   ///< Segment alignment
    static constexpr u32 kLoadableSegmentType = 1;
} __attribute__((packed));

// Function to load ELF-64 kernel
void* LoadElf64Module(byte* elf_start, const byte* elf_end);

#endif  // ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_ELF_HPP_
