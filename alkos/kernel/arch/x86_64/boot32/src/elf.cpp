#include <memory.h>
#include <debug.hpp>
#include <elf.hpp>

namespace elf
{

void* LoadElf64(const byte* elf_start)
{
    TRACE_INFO("Loading ELF-64 ...");
    if (!IsValidElf64(elf_start)) {
        TRACE_ERROR("Invalid ELF-64.");
        return nullptr;
    }

    auto* header_64         = reinterpret_cast<const Header64_t*>(elf_start);
    auto* program_header_64 = reinterpret_cast<const ProgramHeaderEntry64_t*>(
        elf_start + header_64->program_header_table_file_offset
    );

    // Iterate through program headers
    for (u16 i = 0; i < header_64->program_header_table_entry_count; i++) {
        const ProgramHeaderEntry64_t* program_header_entry = &program_header_64[i];

        // Only load segments that should be loaded into memory
        if (program_header_entry->type == ProgramHeaderEntry64_t::kLoadableSegmentType) {
            TRACE_INFO("Loading segment %d...", i + 1);

            u32 segment_dest      = static_cast<u32>(program_header_entry->virtual_address);
            u32 segment_dest_size = static_cast<u32>(program_header_entry->size_in_memory_bytes);
            u32 segment_source =
                static_cast<u32>(reinterpret_cast<u32>(elf_start) + program_header_entry->offset);
            u32 segment_source_size = static_cast<u32>(program_header_entry->size_in_file_bytes);

            TRACE_INFO(
                "Segment %d: dest=0x%X, dest_size=0x%X, source=0x%X, source_size=0x%X", i + 1,
                segment_dest, segment_dest_size, segment_source, segment_source_size
            );

            memcpy(
                reinterpret_cast<void*>(segment_dest), reinterpret_cast<void*>(segment_source),
                segment_source_size
            );

            // Zero out the remaining memory (For things like .bss sections, etc.)
            if (segment_dest_size > segment_source_size) {
                memset(
                    reinterpret_cast<void*>(segment_dest + segment_source_size), 0,
                    segment_dest_size - segment_source_size
                );
            }

            TRACE_SUCCESS("Loaded LOAD segment %d successfully.", i + 1);
        }
    }

    if (header_64->entry_point_virtual_address == 0) {
        TRACE_ERROR("ELF entry point is null.");
        return nullptr;
    }

    TRACE_SUCCESS(
        "ELF-64 module loaded successfully. Entry point: 0x%X",
        static_cast<u32>(header_64->entry_point_virtual_address)
    );
    return reinterpret_cast<void*>(header_64->entry_point_virtual_address);
}

void GetElf64ProgramBounds(const byte* elf_start, u64& start_addr, u64& end_addr)
{
    start_addr = reinterpret_cast<u64>(nullptr);
    end_addr   = reinterpret_cast<u64>(nullptr);

    if (!IsValidElf64(elf_start)) {
        return;
    }

    auto* header_64               = reinterpret_cast<const Header64_t*>(elf_start);
    auto* program_header_entry_64 = reinterpret_cast<const ProgramHeaderEntry64_t*>(
        elf_start + header_64->program_header_table_file_offset
    );

    for (u16 i = 0; i < header_64->program_header_table_entry_count; i++) {
        const ProgramHeaderEntry64_t* phdr = &program_header_entry_64[i];

        if (phdr->type == ProgramHeaderEntry64_t::kLoadableSegmentType) {
            if (start_addr == reinterpret_cast<u64>(nullptr) ||
                phdr->virtual_address < start_addr) {
                start_addr = phdr->virtual_address;
            }

            if (end_addr == reinterpret_cast<u64>(nullptr) ||
                phdr->virtual_address + phdr->size_in_memory_bytes > end_addr) {
                end_addr = phdr->virtual_address + phdr->size_in_memory_bytes;
            }
        }
    }
}

bool IsValidElf64(const byte* elf_start)
{
    if (elf_start == nullptr) {
        TRACE_ERROR("Null ELF start address.");
        return false;
    }

    const auto* elf_header64 = reinterpret_cast<const Header64_t*>(elf_start);

    if (!(elf_header64->identifier[0] == Header64_t::kElfMagic0 &&
          elf_header64->identifier[1] == Header64_t::kElfMagic1 &&
          elf_header64->identifier[2] == Header64_t::kElfMagic2 &&
          elf_header64->identifier[3] == Header64_t::kElfMagic3)) {
        TRACE_ERROR("ELF magic number not found.");
        return false;
    }

    if (elf_header64->machine_architecture != Header64_t::kSupportedMachine /* EM_X86_64 */) {
        TRACE_ERROR("Unsupported machine type.");
        return false;
    }

    return true;
}

}  // namespace elf
