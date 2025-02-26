#include <memory.h>
#include <arch_utils.hpp>
#include <debug.hpp>
#include <elf.hpp>
#include <terminal.hpp>

void* LoadElf64Module(byte* elf_start, const byte* elf_end)
{
    TRACE_INFO("Loading ELF-64 module...");

    if (elf_start == nullptr || elf_end == nullptr) {
        TRACE_ERROR("Null ELF boundaries.");
        return nullptr;
    }

    if (elf_start >= elf_end) {
        TRACE_ERROR("Invalid ELF boundaries: start >= end.");
        return nullptr;
    }

    auto* ehdr = reinterpret_cast<Elf64_Ehdr*>(elf_start);

    if (!(ehdr->e_ident[0] == Elf64_Ehdr::kElfMagic0 &&
          ehdr->e_ident[1] == Elf64_Ehdr::kElfMagic1 &&
          ehdr->e_ident[2] == Elf64_Ehdr::kElfMagic2 &&
          ehdr->e_ident[3] == Elf64_Ehdr::kElfMagic3)) {
        TRACE_ERROR("File is not a valid ELF.");
        return nullptr;
    }

    if (ehdr->e_machine != Elf64_Ehdr::kSupportedMachine /* EM_X86_64 */) {
        TRACE_ERROR("Unsupported machine type.");
        return nullptr;
    }

    auto elf_size = static_cast<u32>(elf_end - elf_start);
    if (ehdr->e_phoff + (ehdr->e_phnum * ehdr->e_phentsize) > elf_size) {
        TRACE_ERROR("Program header table out of file bounds.");
        return nullptr;
    }

    auto* phdrs = reinterpret_cast<Elf64_Phdr*>(elf_start + ehdr->e_phoff);

    // Iterate through program headers
    for (u16 i = 0; i < ehdr->e_phnum; i++) {
        const Elf64_Phdr* phdr = &phdrs[i];

        if (phdr->p_offset + phdr->p_filesz > elf_size) {
            TRACE_ERROR("Declared ELF segment size in file exceeds the size of the ELF itself.");
            return nullptr;
        }

        // Only load segments that should be loaded into memory
        if (phdr->p_type == Elf64_Phdr::kLoadableSegmentType) {
            TRACE_INFO("Loading segment %d...", i + 1);

            u32 segment_dest      = static_cast<u32>(phdr->p_vaddr);
            u32 segment_dest_size = static_cast<u32>(phdr->p_memsz);
            u32 segment_source =
                static_cast<u32>(reinterpret_cast<u32>(elf_start) + phdr->p_offset);
            u32 segment_source_size = static_cast<u32>(phdr->p_filesz);

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

    if (ehdr->e_entry == 0) {
        TRACE_ERROR("ELF entry point is null.");
        return nullptr;
    }

    TRACE_SUCCESS(
        "ELF-64 module loaded successfully. Entry point: 0x%X", static_cast<u32>(ehdr->e_entry)
    );
    return reinterpret_cast<void*>(ehdr->e_entry);
}
