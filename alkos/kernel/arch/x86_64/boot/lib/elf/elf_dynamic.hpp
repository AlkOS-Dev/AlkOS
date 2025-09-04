#ifndef ALKOS_BOOT_LIB_ELF_ELF_DYNAMIC_HPP_
#define ALKOS_BOOT_LIB_ELF_ELF_DYNAMIC_HPP_

#include <extensions/types.hpp>

namespace Elf64
{

// TODO: This was a POC implementation. Clean up.

// ELF Dynamic Section Entry
struct Dyn {
    i64 d_tag;
    union {
        u64 d_val;
        u64 d_ptr;
    } d_un;
};

// Dynamic Section Entry Tags
constexpr i64 DT_NULL    = 0;  // Marks end of dynamic section
constexpr i64 DT_RELA    = 7;  // Address of Rela relocation table
constexpr i64 DT_RELASZ  = 8;  // Size in bytes of the Rela table
constexpr i64 DT_RELAENT = 9;  // Size in bytes of one Rela entry

// ELF Relocation Entry with Addend
struct Rela {
    u64 r_offset;  // Address of relocation
    u64 r_info;    // Relocation type and symbol index
    i64 r_addend;  // Addend
};

// Relocation Types
constexpr u64 R_X86_64_RELATIVE = 8;  // The most common type for PIE

}  // namespace Elf64

#endif  // ALKOS_BOOT_LIB_ELF_ELF_DYNAMIC_HPP_
