#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__i386__)
#error "loader_32 needs to be compiled with a i386-elf compiler"
#endif

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

using namespace Multiboot;

//==============================================================================
// External Functions and Variables
//==============================================================================

extern "C" int CheckCpuId();
extern "C" int CheckLongMode();
extern "C" void EnablePaging(void* pml4_table_address);
extern "C" void EnableLongMode();
extern "C" void EnterElf64(
    void* higher_32_bits_of_entry_address, void* lower_32_bits_of_entry_address,
    void* loader_data_address
);

extern const char multiboot_header_start[];
extern const char multiboot_header_end[];
extern const char loader_32_start[];
extern const char loader_32_end[];
extern byte kLoaderPreAllocatedMemory[];

//==============================================================================
// Global Data
//==============================================================================

static TransitionData loader_data;

//==============================================================================
// Hardware and System Verification
//==============================================================================

static void VerifyMultiboot(u32 boot_loader_magic)
{
    if (boot_loader_magic != Multiboot::kMultiboot2BootloaderMagic) {
        KernelPanic("Multiboot2 check failed! Invalid magic number.");
    }
}

static void PerformHardwareChecks()
{
    TRACE_INFO("Checking for required hardware features...");
    if (CheckCpuId()) {
        KernelPanic("CPUID check failed!");
    }
    if (CheckLongMode()) {
        KernelPanic("Long mode check failed!");
    }
}

//==================================================================================
// Memory Management Setup
// ==================================================================================

static void IdentityMapInitialMemory(MemoryManager* memory_manager)
{
    TRACE_DEBUG("Identity mapping the first 512 GiB of memory...");
    static constexpr u64 k1GiB = 1ULL << 30;
    for (u32 i = 0; i < 512; i++) {
        u64 addr = static_cast<u64>(i) * k1GiB;
        memory_manager->MapVirtualMemoryToPhysical<MemoryManager::PageSize::Page1G>(
            addr, addr, MemoryManager::kPresentBit | MemoryManager::kWriteBit
        );
    }
}

static void InitializeMemoryManagerFromMmap(MemoryManager* memory_manager, TagMmap* mmap_tag)
{
    TRACE_DEBUG("Initializing memory manager with available regions...");
    MemoryMap(mmap_tag).WalkEntries([&](MmapEntry& entry) {
        if (entry.type == MmapEntry::kMemoryAvailable) {
            memory_manager->AddFreeMemoryRegion(entry.addr, entry.addr + entry.len);
        }
    });
    TRACE_DEBUG(
        "Total available memory: %s", FormatMetricUint(memory_manager->GetAvailableMemoryBytes())
    );
}

static void ReserveLoaderMemory(MemoryManager* memory_manager)
{
    TRACE_DEBUG("Reserving memory used by the 32-bit loader...");
    memory_manager->MarkMemoryAreaNotFree(
        reinterpret_cast<u64>(loader_32_start), reinterpret_cast<u64>(loader_32_end)
    );
    memory_manager->MarkMemoryAreaNotFree(
        reinterpret_cast<u64>(multiboot_header_start), reinterpret_cast<u64>(multiboot_header_end)
    );
    memory_manager->MarkMemoryAreaNotFree(
        reinterpret_cast<u64>(kLoaderPreAllocatedMemory),
        reinterpret_cast<u64>(kLoaderPreAllocatedMemory) + sizeof(MemoryManager)
    );
}

//==================================================================================
// Module Loading
//==================================================================================

static TagModule* FindLoader64Module(MultibootInfo& multiboot_info)
{
    auto loader64_module_res = multiboot_info.FindTag<TagModule>([](TagModule* tag) {
        return strcmp(tag->cmdline, "loader64") == 0;
    });

    if (!loader64_module_res) {
        KernelPanic("Could not find the 'loader64' module tag!");
    }
    return loader64_module_res.value();
}

static u64 LoadElfModule(const TagModule* module)
{
    TRACE_DEBUG("Loading 'loader64' ELF module...");
    byte* module_start_addr = reinterpret_cast<byte*>(module->mod_start);
    auto entry_point_res    = Elf64::Load(module_start_addr, 0);

    if (!entry_point_res) {
        KernelPanic("Failed to load the kernel/loader ELF module!");
    }
    return entry_point_res.value();
}

//==================================================================================================
// Transition to 64-bit Mode
//==================================================================================================

static void PrepareTransitionData(u64 multiboot_info_addr, MemoryManager* memory_manager)
{
    loader_data.multiboot_info_addr         = multiboot_info_addr;
    loader_data.multiboot_header_start_addr = reinterpret_cast<u64>(multiboot_header_start);
    loader_data.multiboot_header_end_addr   = reinterpret_cast<u64>(multiboot_header_end);
    loader_data.memory_manager_addr         = reinterpret_cast<u64>(memory_manager);
}

static void JumpTo64BitLoader(u64 kernel_entry_point, MemoryManager* memory_manager)
{
    TRACE_INFO("Jumping to 64-bit loader at entry point: 0x%llX", kernel_entry_point);

    // Free the memory used by this 32-bit loader, as it's no longer needed.
    memory_manager->AddFreeMemoryRegion(
        reinterpret_cast<u64>(loader_32_start), reinterpret_cast<u64>(loader_32_end)
    );

    // Split the 64-bit address for the 32-bit extern function
    void* entry_high = reinterpret_cast<void*>(static_cast<u32>(kernel_entry_point >> 32));
    void* entry_low  = reinterpret_cast<void*>(static_cast<u32>(kernel_entry_point & kBitMask32));

    EnterElf64(entry_high, entry_low, &loader_data);
}

//==================================================================================
// Main Entry Point
//==================================================================================

extern "C" void MainLoader32(u32 boot_loader_magic, u32 multiboot_info_addr_32)
{
    // Initial Setup and Verification
    TerminalInit();
    TRACE_INFO("Loader 32-Bit Stage");
    VerifyMultiboot(boot_loader_magic);
    PerformHardwareChecks();
    BlockHardwareInterrupts();

    // Memory Management
    auto* memory_manager = new (kLoaderPreAllocatedMemory) MemoryManager();
    IdentityMapInitialMemory(memory_manager);

    MultibootInfo multiboot_info(multiboot_info_addr_32);
    auto mmap_tag_res = multiboot_info.FindTag<TagMmap>();
    if (!mmap_tag_res) {
        KernelPanic("Error finding memory map tag in multiboot info!");
    }
    InitializeMemoryManagerFromMmap(memory_manager, mmap_tag_res.value());
    ReserveLoaderMemory(memory_manager);

    // Enable 64-bit CPU features
    TRACE_INFO("Enabling long mode and paging...");
    EnableLongMode();
    EnablePaging(reinterpret_cast<void*>(memory_manager->GetPml4Table()));

    // Load the next stage
    TagModule* loader64_module = FindLoader64Module(multiboot_info);
    u64 kernel_entry_point     = LoadElfModule(loader64_module);

    // Prepare for and execute transition to 64-bit mode
    PrepareTransitionData(multiboot_info_addr_32, memory_manager);
    JumpTo64BitLoader(kernel_entry_point, memory_manager);

    // This part is never reached
    KernelPanic("Failed to transition to 64-bit mode.");
}
