#ifndef ALKOS_KERNEL_ARCH_X86_64_BOOT_LIB_ELF_ELF_DYNAMIC_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_BOOT_LIB_ELF_ELF_DYNAMIC_HPP_

#include <extensions/expected.hpp>
#include <extensions/types.hpp>

#include "elf/error.hpp"

namespace Elf64
{

/**
 * @brief ELF-64 Dynamic Section Entry structure.
 *
 * The .dynamic section contains a series of these structures,
 * holding information for dynamic linking.
 */
struct DynamicEntry {
    i64 tag;  ///< Type of the entry.
    union {
        u64 value;  ///< Integer value.
        u64 ptr;    ///< Address value.
    } un;
};

// Dynamic Section Entry Tags
// Full list at https://docs.oracle.com/cd/E23824_01/html/819-0690/chapter6-42444.html
namespace DynamicEntryTag
{
constexpr i64 kNull     = 0;   ///< Marks end of dynamic section.
constexpr i64 kNeeded   = 1;   ///< Name of a needed library.
constexpr i64 kPltRelSz = 2;   ///< Size in bytes of PLT relocations.
constexpr i64 kPltGot   = 3;   ///< Address of PLT and/or GOT.
constexpr i64 kHash     = 4;   ///< Address of symbol hash table.
constexpr i64 kStrTab   = 5;   ///< Address of string table.
constexpr i64 kSymTab   = 6;   ///< Address of symbol table.
constexpr i64 kRela     = 7;   ///< Address of Rela relocation table.
constexpr i64 kRelaSz   = 8;   ///< Size in bytes of the Rela table.
constexpr i64 kRelaEnt  = 9;   ///< Size in bytes of one Rela entry.
constexpr i64 kStrSz    = 10;  ///< Size in bytes of string table.
constexpr i64 kSymEnt   = 11;  ///< Size in bytes of a symbol table entry.
constexpr i64 kInit     = 12;  ///< Address of init function.
constexpr i64 kFini     = 13;  ///< Address of termination function.
}  // namespace DynamicEntryTag

/**
 * @brief ELF-64 Relocation Entry with Addend structure.
 *
 * The .rela.dyn section contains a series of these structures,
 * describing how to modify the executable image.
 */
struct Rela {
    u64 offset;  ///< Address of relocation.
    u64 info;    ///< Relocation type and symbol index.
    i64 addend;

    u32 GetType() const { return static_cast<u32>(info); }
    u32 GetSymbol() const { return static_cast<u32>(info >> 32); }
};

// Relocation Types for x86-64
// Full list at https://docs.oracle.com/cd/E23824_01/html/819-0690/chapter6-42444.html
namespace RelocationType
{
constexpr u32 kRelative = 8;  ///< The most common type for PIE.
}  // namespace RelocationType

/**
 * @brief Relocates a PIE (Position-Independent Executable) ELF64 file.
 *
 * @param elf_ptr Pointer to the ELF file data.
 * @param load_base_addr The address where the ELF file has been loaded.
 * @return std::expected<void, Error> An empty expected on success, or an error.
 */
std::expected<void, Error> Relocate(byte *elf_ptr, u64 load_base_addr);

}  // namespace Elf64

#endif  // ALKOS_KERNEL_ARCH_X86_64_BOOT_LIB_ELF_ELF_DYNAMIC_HPP_
