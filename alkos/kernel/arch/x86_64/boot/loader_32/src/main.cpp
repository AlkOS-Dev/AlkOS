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

#include "abi/transition_data.hpp"
#include "cpu/utils.hpp"
#include "elf/elf_64.hpp"
#include "elf/error.hpp"
#include "mem/memory_manager.hpp"
#include "multiboot2/info.hpp"
#include "multiboot2/memory_map.hpp"
#include "multiboot2/multiboot2.h"
#include "sys/panic.hpp"
#include "sys/terminal.hpp"

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
// High-Level Boot Steps
//==============================================================================

/**
 * @brief Initializes terminal and verifies the boot environment and hardware.
 */
static void InitializeAndVerifyEnvironment(u32 boot_loader_magic)
{
    TerminalInit();
    TRACE_INFO("Loader 32-Bit Stage");

    if (boot_loader_magic != Multiboot::kMultiboot2BootloaderMagic) {
        KernelPanic("Multiboot2 check failed! Invalid magic number.");
    }

    TRACE_INFO("Checking for required hardware features...");
    if (CheckCpuId()) {
        KernelPanic("CPUID check failed!");
    }
    if (CheckLongMode()) {
        KernelPanic("Long mode check failed!");
    }

    BlockHardwareInterrupts();
}

/**
 * @brief Sets up the complete memory manager for the loader.
 * This includes identity mapping, processing the Multiboot memory map,
 * and reserving memory used by the loader itself.
 * @param multiboot_info A reference to the MultibootInfo object.
 * @return An initialized MemoryManager instance.
 */
static MemoryManager* SetupMemoryManagement(MultibootInfo& multiboot_info)
{
    TRACE_DEBUG("Setting up memory management...");
    auto* memory_manager = new (kLoaderPreAllocatedMemory) MemoryManager();

    // Identity map the first 512 GiB of memory
    static constexpr u64 k1GiB = 1ULL << 30;
    for (u32 i = 0; i < 512; i++) {
        u64 addr = static_cast<u64>(i) * k1GiB;
        memory_manager->MapVirtualMemoryToPhysical<MemoryManager::PageSize::Page1G>(
            addr, addr, MemoryManager::kPresentBit | MemoryManager::kWriteBit
        );
    }

    // Add available memory regions from the Multiboot map
    auto mmap_tag_res = multiboot_info.FindTag<TagMmap>();
    if (!mmap_tag_res) {
        KernelPanic("Error finding memory map tag in multiboot info!");
    }
    MemoryMap(mmap_tag_res.value()).WalkEntries([&](MmapEntry& entry) {
        if (entry.type == MmapEntry::kMemoryAvailable) {
            memory_manager->AddFreeMemoryRegion(entry.addr, entry.addr + entry.len);
        }
    });

    // Reserve memory currently in use by this loader
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

    return memory_manager;
}

/**
 * @brief Enables critical 64-bit CPU features: Long Mode and Paging.
 */
static void EnableCpuFeatures(MemoryManager* memory_manager)
{
    TRACE_INFO("Enabling long mode and paging...");
    EnableLongMode();
    EnablePaging(reinterpret_cast<void*>(memory_manager->GetPml4Table()));
}

/**
 * @brief Finds and loads the 64-bit loader module ('loader64') into memory.
 * @param multiboot_info A reference to the MultibootInfo object.
 * @return The 64-bit entry point address of the loaded module.
 */
static u64 LoadNextStageModule(MultibootInfo& multiboot_info)
{
    TRACE_DEBUG("Loading next stage module (loader64)...");
    auto loader64_module_res = multiboot_info.FindTag<TagModule>([](TagModule* tag) {
        return strcmp(tag->cmdline, "loader64") == 0;
    });

    if (!loader64_module_res) {
        KernelPanic("Could not find the 'loader64' module tag!");
    }

    byte* module_start_addr = reinterpret_cast<byte*>(loader64_module_res.value()->mod_start);
    auto entry_point_res    = Elf64::Load(module_start_addr, 0);
    if (!entry_point_res) {
        KernelPanic("Failed to load the next stage module!");
    }
    return entry_point_res.value();
}

/**
 * @brief Prepares the final data structure and jumps to the 64-bit loader.
 *        This function does not return.
 */
[[noreturn]] static void TransitionTo64BitMode(
    u64 entry_point, MemoryManager* memory_manager, u32 multiboot_info_addr_32
)
{
    TRACE_INFO("Preparing to transition to 64-bit mode...");

    // Prepare the data to be passed to the 64-bit stage
    loader_data.multiboot_info_addr         = multiboot_info_addr_32;
    loader_data.multiboot_header_start_addr = reinterpret_cast<u64>(multiboot_header_start);
    loader_data.multiboot_header_end_addr   = reinterpret_cast<u64>(multiboot_header_end);
    loader_data.memory_manager_addr         = reinterpret_cast<u64>(memory_manager);

    // Free the memory used by this 32-bit loader before jumping
    memory_manager->AddFreeMemoryRegion(
        reinterpret_cast<u64>(loader_32_start), reinterpret_cast<u64>(loader_32_end)
    );

    // Split the 64-bit address for the 32-bit extern function
    void* entry_high = reinterpret_cast<void*>(static_cast<u32>(entry_point >> 32));
    void* entry_low  = reinterpret_cast<void*>(static_cast<u32>(entry_point & kBitMask32));

    TRACE_INFO("Jumping to 64-bit loader at entry point: 0x%llX", entry_point);
    EnterElf64(entry_high, entry_low, &loader_data);

    // This code should be unreachable
    KernelPanic("EnterElf64 should not return!");
}

//==================================================================================
// Main Entry Point
//==================================================================================

extern "C" void MainLoader32(u32 boot_loader_magic, u32 multiboot_info_addr_32)
{
    InitializeAndVerifyEnvironment(boot_loader_magic);

    MultibootInfo multiboot_info(multiboot_info_addr_32);

    MemoryManager* memory_manager = SetupMemoryManagement(multiboot_info);

    EnableCpuFeatures(memory_manager);

    u64 next_stage_entry_point = LoadNextStageModule(multiboot_info);

    TransitionTo64BitMode(next_stage_entry_point, memory_manager, multiboot_info_addr_32);
}
