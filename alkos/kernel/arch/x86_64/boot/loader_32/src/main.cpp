#include <extensions/debug.hpp>
#include <extensions/defines.hpp>
#include <extensions/internal/formats.hpp>
#include <extensions/new.hpp>
#include "extensions/style_aliases.hpp"

#include "cpu/utils.hpp"

#include "abi/transition_data.hpp"

#include "elf/elf_64.hpp"
#include "elf/error.hpp"

#include "mem/memory_manager.hpp"

#include "sys/panic.hpp"
#include "sys/terminal.hpp"

#include "multiboot2/info.hpp"
#include "multiboot2/memory_map.hpp"
#include "multiboot2/multiboot2.h"

// External functions defined in assembly
extern "C" int CheckCpuId();
extern "C" int CheckLongMode();
extern "C" void EnablePaging(void* pml4_table_address);
extern "C" void EnableLongMode();
extern "C" void EnterElf64(
    void* higher_32_bits_of_entry_address, void* lower_32_bits_of_entry_address,
    void* loader_data_address
);

// External symbols defined in the linker script
extern const char multiboot_header_start[];
extern const char multiboot_header_end[];

extern const char loader_32_start[];
extern const char loader_32_end[];

// Pre-allocated memory for the loader memory manager
extern byte kLoaderPreAllocatedMemory[];

using namespace Multiboot;

// Data structure that holds information passed from the 32-bit loader to the 64-bit kernel
TransitionData loader_data;

static void MultibootCheck(u32 boot_loader_magic)
{
    if (boot_loader_magic != Multiboot::kMultiboot2BootloaderMagic) {
        KernelPanic("Multiboot2 check failed!");
    }
}

static void HardwareChecks()
{
    BlockHardwareInterrupts();

    TRACE_INFO("Checking for hardware features");

    if (CheckCpuId()) {
        KernelPanic("CPUID check failed!");
    }

    if (CheckLongMode()) {
        KernelPanic("Long mode check failed!");
    }
}

static void IdentityMap(MemoryManager* memory_manager)
{
    static constexpr u32 k1GiB = 1 << 30;
    for (u32 i = 0; i < 512; i++) {
        u64 addr_64bit = static_cast<u64>(i) * k1GiB;
        memory_manager->MapVirtualMemoryToPhysical<MemoryManager::PageSize::Page1G>(
            addr_64bit, addr_64bit, MemoryManager::kPresentBit | MemoryManager::kWriteBit
        );
    }
}

// TODO: Should just be a method of MemoryManager
static void InitializeMemoryManagerWithFreeMemoryRegions(
    MemoryManager* memory_manager, Multiboot::MemoryMap memory_map
)
{
    memory_map.WalkEntries([&](Multiboot::MmapEntry& mmap_entry) FORCE_INLINE_L {
        if (mmap_entry.type == Multiboot::MmapEntry::kMemoryAvailable) {
            memory_manager->AddFreeMemoryRegion(mmap_entry.addr, mmap_entry.addr + mmap_entry.len);
            TRACE_INFO(
                "Memory region: 0x%llX-0x%llX, length: %sB - Added to memory manager",
                mmap_entry.addr, mmap_entry.addr + mmap_entry.len, FormatMetricUint(mmap_entry.len)
            );
        }
    });
    TRACE_DEBUG(
        "Total available memory: %s", FormatMetricUint(memory_manager->GetAvailableMemoryBytes())
    );
}

static Multiboot::TagModule* GetLoader64Module(MultibootInfo& multiboot_info)
{
    auto loader64_module_res =
        multiboot_info.FindTag<Multiboot::TagModule>([](Multiboot::TagModule* tag) -> bool {
            return strcmp(tag->cmdline, "loader64") == 0;
        });
    if (!loader64_module_res) {
        KernelPanic("Error finding loader64 module in multiboot tags!");
    }

    return loader64_module_res.value();
}

static u64 LoadLoader64Module(Multiboot::TagModule* loader64_module)
{
    byte* loader_module_start_addr = reinterpret_cast<byte*>(loader64_module->mod_start);

    auto k_entry_res = Elf64::Load(loader_module_start_addr, 0);
    if (!k_entry_res) {
        KernelPanic("Failed to load kernel module!");
    }

    return k_entry_res.value();
}

extern "C" void MainLoader32(u32 boot_loader_magic, u32 multiboot_info_addr)
{
    TerminalInit();
    TRACE_INFO("In 32 bit mode");

    MultibootCheck(boot_loader_magic);
    HardwareChecks();

    BlockHardwareInterrupts();

    TRACE_DEBUG("Identity mapping memory");
    auto* memory_manager = new (kLoaderPreAllocatedMemory) MemoryManager();
    IdentityMap(memory_manager);

    TRACE_DEBUG("Enabling hardware features...");
    EnableLongMode();
    EnablePaging(reinterpret_cast<void*>(memory_manager->GetPml4Table()));

    TRACE_DEBUG("Finding the memory map...");
    MultibootInfo multiboot_info = MultibootInfo(static_cast<u64>(multiboot_info_addr));
    auto mmap_tag_res            = multiboot_info.FindTag<TagMmap>();
    if (!mmap_tag_res) {
        KernelPanic("Error finding memory map tag in multiboot tags!");
    }
    MemoryMap memory_map{mmap_tag_res.value()};

    TRACE_DEBUG("Initializing memory manager...");
    InitializeMemoryManagerWithFreeMemoryRegions(memory_manager, mmap_tag_res.value());
    memory_manager->MarkMemoryAreaNotFree(
        static_cast<u64>(reinterpret_cast<u32>(loader_32_start)),
        static_cast<u64>(reinterpret_cast<u32>(loader_32_end))
    );
    memory_manager->MarkMemoryAreaNotFree(
        static_cast<u64>(reinterpret_cast<u32>(multiboot_header_start)),
        static_cast<u64>(reinterpret_cast<u32>(multiboot_header_end))
    );
    memory_manager->MarkMemoryAreaNotFree(
        static_cast<u64>(reinterpret_cast<u32>(kLoaderPreAllocatedMemory)),
        static_cast<u64>(reinterpret_cast<u32>(kLoaderPreAllocatedMemory) + sizeof(MemoryManager))
    );

    TRACE_DEBUG("Loading loader64 module...");
    auto* loader64_module  = GetLoader64Module(multiboot_info);
    u64 kernel_entry_point = LoadLoader64Module(loader64_module);

    TRACE_INFO("Jumping to 64-bit loader...");
    memory_manager->AddFreeMemoryRegion(
        static_cast<u64>(reinterpret_cast<u32>(loader_32_start)),
        static_cast<u64>(reinterpret_cast<u32>(loader_32_end))
    );

    loader_data.multiboot_info_addr = static_cast<u64>(reinterpret_cast<u32>(multiboot_info_addr));
    loader_data.multiboot_header_start_addr =
        static_cast<u64>(reinterpret_cast<u32>(multiboot_header_start));
    loader_data.multiboot_header_end_addr =
        static_cast<u64>(reinterpret_cast<u32>(multiboot_header_end));
    loader_data.memory_manager_addr = reinterpret_cast<u64>(memory_manager);

    EnterElf64(
        reinterpret_cast<void*>(static_cast<u32>(kernel_entry_point >> 32)),
        reinterpret_cast<void*>(static_cast<u32>(kernel_entry_point & kBitMask32)), &loader_data
    );
}
