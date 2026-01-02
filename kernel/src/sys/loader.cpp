#include "sys/loader.hpp"

#include <string.h>
#include <bits_ext.hpp>
#include <template/scope_guard.hpp>

#include "hal/constants.hpp"
#include "mem/heap.hpp"
#include "mem/virt/area.hpp"
#include "modules/memory.hpp"
#include "modules/vfs.hpp"
#include "sys/elf/elf.hpp"
#include "trace_framework.hpp"

namespace System
{

using namespace Mem;
using std::expected;
using std::unexpected;

namespace
{
bool ValidateElfHeader(const Elf::Header &header)
{
    if (memcmp(header.identifier, Elf::Header::kMagic, 4) != 0) {
        TRACE_WARN_GENERAL("Invalid ELF magic");
        return false;
    }

    if (header.machine != hal::kElfMachineType) {
        TRACE_WARN_GENERAL(
            "Invalid ELF machine type: expected 0x%X, got 0x%X", hal::kElfMachineType,
            header.machine
        );
        return false;
    }

    return true;
}
}  // namespace

expected<Mem::VPtr<void>, LoadError> ElfLoader::Load(const vfs::Path &path, AddressSpace &as)
{
    TRACE_INFO_GENERAL("ElfLoader::Load()");
    auto &vfs = VfsModule::Get();

    // 1. Check if file exists
    TRACE_FREQ_INFO_GENERAL("Validating the file exists");
    auto exists_res = vfs.FileExists(path);
    if (!exists_res.has_value() || !exists_res.value()) {
        return unexpected(LoadError::FileNotFound);
    }

    // 2. Read ELF Header
    Elf::Header header;
    auto read_res = vfs.ReadFile(path, &header, sizeof(header), 0);
    if (!read_res.has_value() || read_res.value() != sizeof(header)) {
        return unexpected(LoadError::IoError);
    }

    // 3. Validate ELF
    TRACE_FREQ_INFO_GENERAL("Validating the ELF header");
    if (!ValidateElfHeader(header)) {
        return unexpected(LoadError::InvalidElf);
    }

    // 4. Read Program Headers
    size_t ph_size     = static_cast<size_t>(header.phnum * header.phentsize);
    auto ph_buffer_res = Mem::KMalloc(ph_size);
    if (!ph_buffer_res) {
        return unexpected(LoadError::MemoryError);
    }

    // Auto-free buffer on exit
    VPtr<void> ph_raw = *ph_buffer_res;
    template_lib::ScopeGuard guard([ph_raw]() {
        Mem::KFree(ph_raw);
    });

    TRACE_FREQ_INFO_GENERAL("Validating the ELF header");
    read_res = vfs.ReadFile(path, ph_raw, ph_size, header.phoff);
    if (!read_res.has_value() || read_res.value() != ph_size) {
        return unexpected(LoadError::IoError);
    }

    auto *ph_table = reinterpret_cast<Elf::ProgramHeader *>(ph_raw);

    // Track loaded segments so we can roll back changes to the address space
    // if loading fails halfway through.
    u16 loaded_segments_count = 0;
    template_lib::ScopeGuard vma_cleanup_guard([&]() {
        u16 segments_cleaned = 0;
        for (u16 i = 0; i < header.phnum && segments_cleaned < loaded_segments_count; ++i) {
            auto &ph = ph_table[i];
            if (ph.type != Elf::ProgramHeader::kTypeLoad || ph.memsz == 0) {
                continue;
            }

            u64 virt_start = AlignDown(ph.vaddr, hal::kPageSizeBytes);
            if (auto res =
                    MemoryModule::Get().GetVmm().RmArea(&as, Mem::UptrToPtr<void>(virt_start));
                !res) {
                TRACE_WARN_GENERAL(
                    "Failed to clean up VMA for segment %d during error recovery", i
                );
            }
            segments_cleaned++;
        }
    });

    // 5. Load Segments
    TRACE_FREQ_INFO_GENERAL("Validating the ELF header");
    for (u16 i = 0; i < header.phnum; ++i) {
        auto &ph = ph_table[i];
        if (ph.type != Elf::ProgramHeader::kTypeLoad) {
            continue;
        }
        if (ph.memsz == 0) {
            continue;
        }

        u64 virt_start = AlignDown(ph.vaddr, hal::kPageSizeBytes);
        u64 virt_end   = AlignUp(ph.vaddr + ph.memsz, hal::kPageSizeBytes);
        u64 size       = virt_end - virt_start;

        TRACE_INFO_GENERAL("Loading segment %d: [0x%llX - 0x%llX]", i, virt_start, virt_end);

        // Permissions
        VirtualMemAreaFlags original_flags = {};
        original_flags.readable            = (ph.flags & Elf::ProgramHeader::kFlagRead) != 0U;
        original_flags.writable            = (ph.flags & Elf::ProgramHeader::kFlagWrite) != 0U;
        original_flags.executable          = (ph.flags & Elf::ProgramHeader::kFlagExec) != 0U;

        VirtualMemAreaFlags loading_flags = original_flags;
        loading_flags.writable            = true;

        // Add VMA to Address Space
        auto vma_res = Mem::KNew<Mem::AnonymousVMemArea>(
            Mem::UptrToPtr<void>(virt_start), size, loading_flags
        );

        if (!vma_res) {
            TRACE_WARN_GENERAL("Failed to allocate VMA for segment %d", i);
            return unexpected(LoadError::MemoryError);
        }

        // AddArea takes ownership of the pointer
        if (auto res = MemoryModule::Get().GetVmm().AddArea(&as, *vma_res); !res) {
            TRACE_WARN_GENERAL("Failed to add VMA for segment %d", i);
            return unexpected(LoadError::MemoryError);
        }
        loaded_segments_count++;

        VMemArea *vma = *vma_res;

        // Copy file data into the mapped region
        if (ph.filesz > 0) {
            void *dest = reinterpret_cast<void *>(ph.vaddr);
            read_res   = vfs.ReadFile(path, dest, ph.filesz, ph.offset);
            if (!read_res.has_value() || read_res.value() != ph.filesz) {
                return unexpected(LoadError::IoError);
            }
        }

        // Zero out BSS
        if (ph.memsz > ph.filesz) {
            void *bss_dest = reinterpret_cast<void *>(ph.vaddr + ph.filesz);
            memset(bss_dest, 0, ph.memsz - ph.filesz);
        }

        // Restore flags
        if (loading_flags.writable != original_flags.writable) {
            if (auto res = MemoryModule::Get().GetVmm().UpdateAreaFlags(
                    &as, vma->GetStart(), original_flags
                );
                !res) {
                TRACE_WARN_GENERAL("Failed to restore flags for segment %d", i);
                return unexpected(LoadError::MemoryError);
            }
        }
    }

    vma_cleanup_guard.dismiss();
    u64 entry_point = header.entry;
    return Mem::UptrToPtr<void>(entry_point);
}

}  // namespace System
