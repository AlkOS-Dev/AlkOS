#include <memory.h>
#include <elf64.hpp>
#include <extensions/algorithm.hpp>
#include <extensions/bit.hpp>
#include <extensions/debug.hpp>
#include <extensions/internal/formats.hpp>
#include "todo.hpp"

namespace elf
{

u64 LoadElf64(const byte* elf_start, u64 destination_begin_virtual_address)
{
    TODO_WHEN_DEBUGGING_FRAMEWORK
    TRACE_INFO("Loading ELF-64 ...");
    if (!IsValidElf64(elf_start)) {
        TRACE_ERROR("Invalid ELF-64.");
        return 0;
    }

    const auto* header_64            = reinterpret_cast<const Header64_t*>(elf_start);
    const auto* program_header_table = reinterpret_cast<const ProgramHeaderEntry64_t*>(
        elf_start + header_64->program_header_table_file_offset
    );

    u64 elf_base = kFullMask<u64>;
    for (u16 i = 0; i < header_64->program_header_table_entry_count; i++) {
        const ProgramHeaderEntry64_t* program_header_entry = &program_header_table[i];
        if (program_header_entry->type == ProgramHeaderEntry64_t::kLoadableSegmentType) {
            if (program_header_entry->virtual_address < elf_base) {
                elf_base = program_header_entry->virtual_address;
            }
        }
    }

    // If the destination address is not specified, load the ELF at the base address
    if (destination_begin_virtual_address == 0) {
        destination_begin_virtual_address = elf_base;
    }

    // Iterate through program headers
    for (u16 i = 0; i < header_64->program_header_table_entry_count; i++) {
        const ProgramHeaderEntry64_t* program_header_entry = &program_header_table[i];

        // Only load segments that should be loaded into memory
        if (program_header_entry->type == ProgramHeaderEntry64_t::kLoadableSegmentType) {
            TODO_WHEN_DEBUGGING_FRAMEWORK
            TRACE_INFO("Loading segment %d...", i + 1);

            const u64 segment_dest = destination_begin_virtual_address +
                                     (program_header_entry->virtual_address - elf_base);
            const u64 segment_dest_size = program_header_entry->size_in_memory_bytes;
            const u64 segment_source =
                reinterpret_cast<u64>(elf_start) + program_header_entry->offset;
            const u64 segment_source_size = program_header_entry->size_in_file_bytes;

            TODO_WHEN_DEBUGGING_FRAMEWORK
            TRACE_INFO(
                "Segment %d: dest=0x%llX, dest_size=0x%sB, source=0x%llX, source_size=0x%sB", i + 1,
                segment_dest, FormatMetricUint(segment_dest_size), segment_source,
                FormatMetricUint(segment_source_size)
            );

            memcpy(
                reinterpret_cast<void*>(segment_dest),
                reinterpret_cast<const void*>(segment_source), segment_source_size
            );

            // Zero out the remaining memory (For things like .bss sections, etc.)
            if (segment_dest_size > segment_source_size) {
                memset(
                    reinterpret_cast<void*>(segment_dest + segment_source_size), 0,
                    segment_dest_size - segment_source_size
                );
            }

            TODO_WHEN_DEBUGGING_FRAMEWORK
            TRACE_SUCCESS("Segment %d: loaded.", i + 1);
        }
    }

    if (header_64->entry_point_virtual_address == 0) {
        TRACE_ERROR("ELF entry point is null.");
        return 0;
    }

    const u64 adjusted_entry_point =
        destination_begin_virtual_address + (header_64->entry_point_virtual_address - elf_base);

    TRACE_SUCCESS("ELF-64 loaded. Entry point: 0x%llX", adjusted_entry_point);
    return adjusted_entry_point;
}

std::tuple<u64, u64> GetElf64ProgramBounds(const byte* elf_start)
{
    u64 start_addr = static_cast<u64>(kFullMask<u64>);
    u64 end_addr   = reinterpret_cast<u64>(nullptr);

    if (!IsValidElf64(elf_start)) {
        return std::make_tuple(start_addr, end_addr);
    }

    const auto* header_64            = reinterpret_cast<const Header64_t*>(elf_start);
    const auto* program_header_table = reinterpret_cast<const ProgramHeaderEntry64_t*>(
        elf_start + header_64->program_header_table_file_offset
    );

    for (u16 i = 0; i < header_64->program_header_table_entry_count; i++) {
        const ProgramHeaderEntry64_t* program_header_entry = &program_header_table[i];

        if (program_header_entry->type == ProgramHeaderEntry64_t::kLoadableSegmentType) {
            start_addr = std::min(start_addr, program_header_entry->virtual_address);
            end_addr   = std::max(
                end_addr,
                program_header_entry->virtual_address + program_header_entry->size_in_memory_bytes
            );
        }
    }

    return std::make_tuple(start_addr, end_addr);
}

bool IsValidElf64(const byte* elf_start)
{
    if (elf_start == nullptr) {
        TRACE_ERROR("Null ELF start address.");
        return false;
    }

    const auto* elf_header64 = reinterpret_cast<const Header64_t*>(elf_start);

    if (memcmp(elf_header64->identifier, Header64_t::kMagic, sizeof(Header64_t::kMagic)) != 0) {
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
