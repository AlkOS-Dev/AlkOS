#include "elf/elf_dynamic.hpp"
#include "elf/elf_64.hpp"

namespace Elf64
{

std::expected<void, Error> Relocate(byte *elf_ptr, u64 load_base_addr)
{
    const auto *header = reinterpret_cast<const Header *>(elf_ptr);
    if (auto res = IsValid(elf_ptr); !res) {
        return std::unexpected(res.error());
    }

    const auto *program_header_table = reinterpret_cast<const ProgramHeaderEntry *>(
        elf_ptr + header->program_header_table_file_offset
    );

    const ProgramHeaderEntry *dynamic_header = nullptr;
    for (u16 i = 0; i < header->program_header_table_entry_count; i++) {
        if (program_header_table[i].type == ProgramHeaderEntry::kDynamicSegmentType) {
            dynamic_header = &program_header_table[i];
            break;
        }
    }

    if (!dynamic_header) {
        return std::unexpected(Error::DynamicHeaderNotFound);
    }

    auto *dynamic_section = reinterpret_cast<DynamicEntry *>(elf_ptr + dynamic_header->offset);

    u64 rela_addr     = 0;
    u64 rela_size     = 0;
    u64 rela_ent_size = 0;

    for (int i = 0; dynamic_section[i].tag != DynamicEntryTag::kNull; ++i) {
        switch (dynamic_section[i].tag) {
            case DynamicEntryTag::kRela:
                rela_addr = dynamic_section[i].un.ptr;
                break;
            case DynamicEntryTag::kRelaSz:
                rela_size = dynamic_section[i].un.value;
                break;
            case DynamicEntryTag::kRelaEnt:
                rela_ent_size = dynamic_section[i].un.value;
                break;
            default:
                // Other tags are not needed for basic PIE relocation.
                break;
        }
    }

    if (rela_addr == 0 || rela_size == 0 || rela_ent_size == 0) {
        return std::unexpected(Error::RelocationTableNotFound);
    }

    // The addresses in the dynamic section are virtual addresses. We need to find
    // which segment they belong to in order to find their file offset.
    u64 rela_file_offset = 0;
    for (u16 i = 0; i < header->program_header_table_entry_count; i++) {
        const auto &ph = program_header_table[i];
        if (ph.type == ProgramHeaderEntry::kLoadableSegmentType &&
            rela_addr >= ph.virtual_address &&
            rela_addr < ph.virtual_address + ph.size_in_file_bytes) {
            rela_file_offset = rela_addr - ph.virtual_address + ph.offset;
            break;
        }
    }

    if (rela_file_offset == 0) {
        return std::unexpected(Error::RelocationTableNotFound);
    }

    auto *rela_table    = reinterpret_cast<Rela *>(elf_ptr + rela_file_offset);
    u64 num_relocations = rela_size / rela_ent_size;

    for (u64 i = 0; i < num_relocations; ++i) {
        auto &reloc = rela_table[i];
        if (reloc.GetType() == RelocationType::kRelative) {
            u64 *patch_addr = reinterpret_cast<u64 *>(load_base_addr + reloc.offset);
            *patch_addr     = load_base_addr + reloc.addend;
        }
    }

    return {};
}

}  // namespace Elf64
