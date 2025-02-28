#ifndef ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_ELF_HPP_
#define ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_ELF_HPP_

#include <stdint.h>

#include "types.hpp"

namespace elf
{
// ELF-64 Structures

// Taken from glibc https://github.com/lattera/glibc/blob/master/elf/elf.h#L60
// And https://krinkinmu.github.io/2020/11/15/loading-elf-image.html
// Modified to fit the needs of the project, mostly made it more verbose and added comments.

/**
 * @brief ELF-64 header structure
 *
 * This structure represents the ELF-64 header.
 * It is used to identify the file as an ELF file and to locate the program header table.
 * The program header table contains information about the segments in the file.
 * The entry point of the program is also stored in this header. Among other useful information.
 */
struct Header64_t {
    byte identifier[16];  ///< Magic number and other info
    u16 type;
    u16 machine_architecture;
    u32 object_file_version;
    u64 entry_point_virtual_address;
    u64 program_header_table_file_offset;
    u64 section_header_table_file_offset;
    u32 flags;  ///< Processor-specific flags
    u16 header_size_bytes;
    u16 program_header_entry_size_bytes;
    u16 program_header_table_entry_count;
    u16 section_header_table_entry_size_bytes;
    u16 section_header_table_entry_count;
    u16 section_header_string_table_index;

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
struct ProgramHeaderEntry64_t {
    u32 type;                  ///< Segment type
    u32 flags;                 ///< Segment flags
    u64 offset;                ///< Segment file offset
    u64 virtual_address;       ///< Segment virtual address
    u64 physical_address;      ///< Segment physical address
    u64 size_in_file_bytes;    ///< Segment size in file
    u64 size_in_memory_bytes;  ///< Segment size in memory
    u64 alignment_bytes;       ///< Segment alignment
    static constexpr u32 kLoadableSegmentType = 1;
} __attribute__((packed));

// Function to load ELF-64 kernel
u64 LoadElf64(const byte* elf_start);

bool IsValidElf64(const byte* elf_start);

void GetElf64ProgramBounds(const byte* elf_start, u64& start_addr, u64& end_addr);

}  // namespace elf

#endif  // ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_ELF_HPP_
