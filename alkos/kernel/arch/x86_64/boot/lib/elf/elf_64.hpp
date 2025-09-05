#ifndef ALKOS_BOOT_LIB_ELF_ELF64_HPP_
#define ALKOS_BOOT_LIB_ELF_ELF64_HPP_

#include <stdint.h>

#include <extensions/expected.hpp>
#include <extensions/tuple.hpp>
#include <extensions/types.hpp>

#include "elf/error.hpp"

namespace Elf64
{

// ELF-64 Structures

// Taken from glibc https://github.com/lattera/glibc/blob/master/elf/elf.h#L60
// And https://krinkinmu.github.io/2020/11/15/loading-elf-image.html
// Modified to fit the needs of the project, mostly made it more verbose

/**
 * @brief ELF-64 Header structure.
 *
 * Used to identify the file as an ELF file,
 * locate the program header table, and obtain other executable-related information.
 */
struct Header {
    byte identifier[16];
    u16 type;
    u16 machine_architecture;
    u32 object_file_version;
    u64 entry_point_virtual_address;
    u64 program_header_table_file_offset;
    u64 section_header_table_file_offset;
    u32 flags;
    u16 header_size_bytes;
    u16 program_header_entry_size_bytes;
    u16 program_header_table_entry_count;
    u16 section_header_table_entry_size_bytes;
    u16 section_header_table_entry_count;
    u16 section_header_string_table_index;

    static constexpr byte kMagic[4] = {0x7F, 'E', 'L', 'F'};  ///< ELF identification magic number
    static constexpr u16 kSupportedMachine = 0x3E;  ///< Supported machine type (EM_X86_64).
} PACK;

/**
 * @brief ELF-64 Program Header structure.
 *
 * Describes a segment to be loaded into memory.
 */
struct ProgramHeaderEntry {
    u32 type;
    u32 flags;
    u64 offset;
    u64 virtual_address;
    u64 physical_address;
    u64 size_in_file_bytes;
    u64 size_in_memory_bytes;
    u64 alignment_bytes;

    static constexpr u32 kLoadableSegmentType = 1;
} PACK;

// TODO
// nullptr would mean "load at base", but we use 0 for u64 since it's
// 32/64 agnostic. Some way must exist to implement this cleanly
std::expected<u64, Error> Load(const byte* elf_ptr, u64 destination_addr = 0);

std::expected<void, Error> IsValid(const byte* elf_ptr);

std::expected<std::tuple<u64, u64>, Error> GetProgramBounds(const byte* elf_ptr);

// Simplified version cuz 32-bit code can't handle
// the MAGIC of std::expected<std::tuple<u64, u64>, Error>
bool GetProgramBounds(const byte* elf_ptr, u64& out_start, u64& out_end);

}  // namespace Elf64

#endif  // ALKOS_BOOT_LIB_ELF_ELF64_HPP_
