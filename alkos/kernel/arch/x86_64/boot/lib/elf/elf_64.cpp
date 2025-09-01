#include <string.h>
#include <extensions/algorithm.hpp>
#include <extensions/bit.hpp>
#include <extensions/debug.hpp>
#include <extensions/internal/formats.hpp>
#include "extensions/style_aliases.hpp"

#include "elf/elf_64.hpp"
#include "elf/error.hpp"

namespace Elf64
{

Expected<u64, Error> Load(const byte* elf_start, u64 destination_begin_virtual_address)
{
    if (!IsValid(elf_start)) {
        return Unexpected(Error::InvalidElf);
    }

    const auto* header_64            = reinterpret_cast<const Header*>(elf_start);
    const auto* program_header_table = reinterpret_cast<const ProgramHeaderEntry*>(
        elf_start + header_64->program_header_table_file_offset
    );

    u64 elf_base = kFullMask<u64>;
    for (u16 i = 0; i < header_64->program_header_table_entry_count; i++) {
        const ProgramHeaderEntry* program_header_entry = &program_header_table[i];
        if (program_header_entry->type == ProgramHeaderEntry::kLoadableSegmentType) {
            if (program_header_entry->virtual_address < elf_base) {
                elf_base = program_header_entry->virtual_address;
            }
        }
    }

    // If the destination address is not specified, load the ELF at the base address
    if (destination_begin_virtual_address == 0) {
        destination_begin_virtual_address = elf_base;
    }

    for (u16 i = 0; i < header_64->program_header_table_entry_count; i++) {
        const ProgramHeaderEntry* program_header_entry = &program_header_table[i];

        if (program_header_entry->type == ProgramHeaderEntry::kLoadableSegmentType) {
            const u64 segment_dest = destination_begin_virtual_address +
                                     (program_header_entry->virtual_address - elf_base);
            const u64 segment_dest_size = program_header_entry->size_in_memory_bytes;
            const u64 segment_source =
                reinterpret_cast<u64>(elf_start) + program_header_entry->offset;
            const u64 segment_source_size = program_header_entry->size_in_file_bytes;

            // TODO
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
        }
    }

    if (header_64->entry_point_virtual_address == 0) {
        return Unexpected(Error::NullEntryPoint);
    }

    const u64 adjusted_entry_point =
        destination_begin_virtual_address + (header_64->entry_point_virtual_address - elf_base);

    return adjusted_entry_point;
}

Expected<Tuple<u64, u64>, Error> GetProgramBounds(const byte* elf_start)
{
    u64 start_addr = static_cast<u64>(kFullMask<u64>);
    u64 end_addr   = reinterpret_cast<u64>(nullptr);

    if (!IsValid(elf_start)) {
        return Unexpected(Error::InvalidElf);
    }

    const auto* header_64            = reinterpret_cast<const Header*>(elf_start);
    const auto* program_header_table = reinterpret_cast<const ProgramHeaderEntry*>(
        elf_start + header_64->program_header_table_file_offset
    );

    for (u16 i = 0; i < header_64->program_header_table_entry_count; i++) {
        const ProgramHeaderEntry* program_header_entry = &program_header_table[i];

        if (program_header_entry->type == ProgramHeaderEntry::kLoadableSegmentType) {
            start_addr = std::min(start_addr, program_header_entry->virtual_address);
            end_addr   = std::max(
                end_addr,
                program_header_entry->virtual_address + program_header_entry->size_in_memory_bytes
            );
        }
    }

    return std::make_tuple(start_addr, end_addr);
}

Expected<void, Error> IsValid(const byte* elf_start)
{
    if (elf_start == nullptr) {
        return Unexpected(Error::InvalidElf);
    }

    const auto* elf_Header = reinterpret_cast<const Header*>(elf_start);

    if (memcmp(elf_Header->identifier, Header::kMagic, sizeof(Header::kMagic)) != 0) {
        return Unexpected(Error::InvalidElf);
    }

    if (elf_Header->machine_architecture != Header::kSupportedMachine /* EM_X86_64 */) {
        return Unexpected(Error::UnsupportedMachine);
    }

    return {};
}

}  // namespace Elf64
