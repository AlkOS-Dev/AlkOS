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
 * @brief ELF-64 Header structure.
 *
 * This structure represents the ELF-64 file header. It is used to identify the file as an ELF file,
 * locate the program header table, and obtain other executable-related information.
 */
struct Header64_t {
    byte identifier[16];                        ///< Magic number and other information.
    u16 type;                                   ///< Object file type.
    u16 machine_architecture;                   ///< Target machine architecture.
    u32 object_file_version;                    ///< Object file version.
    u64 entry_point_virtual_address;            ///< Entry point virtual address.
    u64 program_header_table_file_offset;       ///< File offset of the program header table.
    u64 section_header_table_file_offset;       ///< File offset of the section header table.
    u32 flags;                                  ///< Processor-specific flags.
    u16 header_size_bytes;                      ///< Size of this header in bytes.
    u16 program_header_entry_size_bytes;        ///< Size of one entry in the program header table.
    u16 program_header_table_entry_count;       ///< Number of entries in the program header table.
    u16 section_header_table_entry_size_bytes;  ///< Size of one entry in the section header table.
    u16 section_header_table_entry_count;       ///< Number of entries in the section header table.
    u16 section_header_string_table_index;      ///< Section header string table index.

    static constexpr byte kElfMagic0 = 0x7F;  ///< ELF identification magic number byte 0
    static constexpr byte kElfMagic1 = 'E';   ///< ELF identification magic number byte 1
    static constexpr byte kElfMagic2 = 'L';   ///< ELF identification magic number byte 2
    static constexpr byte kElfMagic3 = 'F';   ///< ELF identification magic number byte 3

    static constexpr u16 kSupportedMachine = 0x3E;  ///< Supported machine type (EM_X86_64).
} PACK;

/**
 * @brief ELF-64 Program Header structure.
 *
 * This structure represents an entry in the ELF-64 program header table.
 * It describes a segment to be loaded into memory.
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

    /// Constant representing a loadable segment type
    static constexpr u32 kLoadableSegmentType = 1;
} PACK;

/**
 * @brief Load an ELF-64 binary.
 *
 * Loads the ELF-64 binary from the provided start address into memory. It verifies the binary's
 * validity and relocates segments to the destination virtual address.
 *
 * @param[in] elf_start_virtual_address The starting address of the ELF-64 binary in memory.
 * @param[in] destination_begin_virtual_address The base virtual address where the binary will be
 * loaded. If zero, the ELF base address is used.
 * @return The adjusted entry point after loading the ELF; returns 0 on failure.
 */
u64 LoadElf64(const byte* elf_start_virtual_address, u64 destination_begin_virtual_address);

/**
 * @brief Validate an ELF-64 binary.
 *
 * Checks if the provided ELF binary starts with the correct magic number and is of the supported
 * machine type.
 *
 * @param[in] elf_start The starting address of the ELF-64 binary.
 * @return true if the ELF file is valid, false otherwise.
 */
bool IsValidElf64(const byte* elf_start);

/**
 * @brief Retrieve the memory bounds of an ELF-64 binary.
 *
 * Determines the lowest and highest virtual addresses of the loadable segments in the ELF binary.
 *
 * @param[in] elf_start The starting address of the ELF-64 binary.
 * @param[out] start_addr Set to the lowest virtual address found.
 * @param[out] end_addr Set to the highest virtual address plus its size.
 */
void GetElf64ProgramBounds(const byte* elf_start, u64& start_addr, u64& end_addr);

}  // namespace elf

#endif  // ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_ELF_HPP_
