#include <memory.h>
#include <debug.hpp>
#include <elf.hpp>

namespace elf
{

namespace
{
extern "C" void memcpy64(u32 dest_lo, u32 dest_hi, u32 src_lo, u32 src_hi, u32 n_lo, u32 n_hi);
extern "C" void memset64(u32 dest_lo, u32 dest_hi, u32 c, u32 n_lo, u32 n_hi);
}  // namespace

u64 LoadElf64(const byte* elf_start, u64 destination_begin_virtual_address)
{
    TRACE_INFO("Loading ELF-64 ...");
    if (!IsValidElf64(elf_start)) {
        TRACE_ERROR("Invalid ELF-64.");
        return 0;
    }

    auto* header_64            = reinterpret_cast<const Header64_t*>(elf_start);
    auto* program_header_table = reinterpret_cast<const ProgramHeaderEntry64_t*>(
        elf_start + header_64->program_header_table_file_offset
    );

    u64 elf_base = ~0ULL;
    for (u16 i = 0; i < header_64->program_header_table_entry_count; i++) {
        const ProgramHeaderEntry64_t* program_header_entry = &program_header_table[i];
        if (program_header_entry->type == ProgramHeaderEntry64_t::kLoadableSegmentType) {
            if (program_header_entry->virtual_address < elf_base) {
                elf_base = program_header_entry->virtual_address;
            }
        }
    }

    // Iterate through program headers
    for (u16 i = 0; i < header_64->program_header_table_entry_count; i++) {
        const ProgramHeaderEntry64_t* program_header_entry = &program_header_table[i];

        // Only load segments that should be loaded into memory
        if (program_header_entry->type == ProgramHeaderEntry64_t::kLoadableSegmentType) {
            TRACE_INFO("Loading segment %d...", i + 1);

            u64 segment_dest = destination_begin_virtual_address +
                               (program_header_entry->virtual_address - elf_base);
            u64 segment_dest_size = program_header_entry->size_in_memory_bytes;
            u64 segment_source    = reinterpret_cast<u64>(elf_start) + program_header_entry->offset;
            u64 segment_source_size = program_header_entry->size_in_file_bytes;

            TRACE_INFO(
                "Segment %d: dest=0x%llX, dest_size=0x%llu KB, source=0x%llX, source_size=0x%llu "
                "KB",
                i + 1, segment_dest, segment_dest_size << 10, segment_source,
                segment_source_size << 10
            );

            memcpy64(
                static_cast<u32>(segment_dest & 0xFFFFFFFF), static_cast<u32>(segment_dest >> 32),
                static_cast<u32>(segment_source & 0xFFFFFFFF),
                static_cast<u32>(segment_source >> 32),
                static_cast<u32>(segment_source_size & 0xFFFFFFFF),
                static_cast<u32>(segment_source_size >> 32)
            );

            // Zero out the remaining memory (For things like .bss sections, etc.)
            if (segment_dest_size > segment_source_size) {
                memset64(
                    static_cast<u32>(segment_dest & 0xFFFFFFFF),
                    static_cast<u32>(segment_dest >> 32), 0,
                    static_cast<u32>(segment_dest_size & 0xFFFFFFFF),
                    static_cast<u32>(segment_dest_size >> 32)
                );
            }

            TRACE_SUCCESS("Segment %d loaded.", i + 1);
        }
    }

    if (header_64->entry_point_virtual_address == 0) {
        TRACE_ERROR("ELF entry point is null.");
        return 0;
    }

    u64 adjusted_entry_point =
        destination_begin_virtual_address + (header_64->entry_point_virtual_address - elf_base);

    TRACE_SUCCESS("ELF-64 loaded. Entry point: 0x%llX", adjusted_entry_point);
    return adjusted_entry_point;
}

void GetElf64ProgramBounds(const byte* elf_start, u64& start_addr, u64& end_addr)
{
    start_addr = reinterpret_cast<u64>(nullptr);
    end_addr   = reinterpret_cast<u64>(nullptr);

    if (!IsValidElf64(elf_start)) {
        return;
    }

    auto* header_64            = reinterpret_cast<const Header64_t*>(elf_start);
    auto* program_header_table = reinterpret_cast<const ProgramHeaderEntry64_t*>(
        elf_start + header_64->program_header_table_file_offset
    );

    for (u16 i = 0; i < header_64->program_header_table_entry_count; i++) {
        const ProgramHeaderEntry64_t* program_header_entry = &program_header_table[i];

        if (program_header_entry->type == ProgramHeaderEntry64_t::kLoadableSegmentType) {
            if (start_addr == reinterpret_cast<u64>(nullptr) ||
                program_header_entry->virtual_address < start_addr) {
                start_addr = program_header_entry->virtual_address;
            }

            if (end_addr == reinterpret_cast<u64>(nullptr) ||
                program_header_entry->virtual_address + program_header_entry->size_in_memory_bytes >
                    end_addr) {
                end_addr = program_header_entry->virtual_address +
                           program_header_entry->size_in_memory_bytes;
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
