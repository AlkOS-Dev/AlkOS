#include <memory.h>
#include <arch_utils.hpp>
#include <elf.hpp>
#include <extensions/debug.hpp>
#include <tags.hpp>
#include <terminal.hpp>

void* LoadElf64Module(uint8_t* elf_start, uint8_t* elf_end)
{
    TRACE_INFO("Loading ELF-64 module...");

    // Validate ELF boundaries
    if (elf_start == nullptr || elf_end == nullptr) {
        TRACE_ERROR("Null ELF boundaries.");
        return nullptr;
    }

    if (elf_start >= elf_end) {
        TRACE_ERROR("Invalid ELF boundaries: start >= end.");
        return nullptr;
    }

    Elf64_Ehdr* ehdr = reinterpret_cast<Elf64_Ehdr*>(elf_start);

    // Verify ELF magic
    if (!(ehdr->e_ident[0] == ELF_MAGIC0 && ehdr->e_ident[1] == ELF_MAGIC1 &&
          ehdr->e_ident[2] == ELF_MAGIC2 && ehdr->e_ident[3] == ELF_MAGIC3)) {
        TRACE_ERROR("File is not a valid ELF.");
        return nullptr;
    }

    // Check machine type
    if (ehdr->e_machine != 0x3E /* EM_X86_64 */) {
        TRACE_ERROR("Unsupported machine type.");
        return nullptr;
    }

    size_t elf_size = static_cast<size_t>(elf_end - elf_start);
    if (ehdr->e_phoff + (ehdr->e_phnum * ehdr->e_phentsize) > elf_size) {
        TRACE_ERROR("Program header table out of file bounds.");
        return nullptr;
    }

    Elf64_Phdr* phdrs = reinterpret_cast<Elf64_Phdr*>(elf_start + ehdr->e_phoff);

    // Iterate through program headers
    for (uint16_t i = 0; i < ehdr->e_phnum; i++) {
        const Elf64_Phdr* phdr = &phdrs[i];

        if (phdr->p_offset + phdr->p_filesz > elf_size) {
            TRACE_ERROR("Declared ELF segment size in file exceeds the size of the ELF itself.");
            return nullptr;
        }

        // Only load segments that should be loaded into memory
        if (phdr->p_type == PT_LOAD) {
            TRACE_INFO("Loading segment %d...", i + 1);

            uint64_t segment_dest        = phdr->p_vaddr;
            uint64_t segment_dest_size   = phdr->p_memsz;
            uint64_t segment_source      = reinterpret_cast<uint64_t>(elf_start) + phdr->p_offset;
            uint64_t segment_source_size = phdr->p_filesz;

            // Copy segment data from ELF to destination
            memcpy(
                reinterpret_cast<void*>(segment_dest), reinterpret_cast<void*>(segment_source),
                static_cast<size_t>(segment_source_size)
            );

            // Zero out the remaining memory if p_memsz > p_filesz
            if (segment_dest_size > segment_source_size) {
                memset(
                    reinterpret_cast<void*>(segment_dest + segment_source_size), 0,
                    static_cast<size_t>(segment_dest_size - segment_source_size)
                );
            }

            TRACE_SUCCESS("Loaded LOAD segment %d successfully.", i + 1);
        }
    }

    // Verify entry point
    if (ehdr->e_entry == 0) {
        TRACE_ERROR("ELF entry point is null.");
        return nullptr;
    }

    TRACE_SUCCESS(
        "ELF-64 module loaded successfully. Entry point: 0x%X", static_cast<uint32_t>(ehdr->e_entry)
    );
    return reinterpret_cast<void*>(ehdr->e_entry);
}
