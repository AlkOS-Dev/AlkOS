#include <extensions/new.hpp>

#include "arch_utils.hpp"
#include "defines.hpp"
#include "elf/elf64.hpp"
#include "extensions/debug.hpp"
#include "loader_data.hpp"
#include "loader_memory_manager/loader_memory_manager.hpp"
#include "terminal.hpp"

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

extern const char loader_start[];
extern const char loader_end[];

// Pre-allocated memory for the loader memory manager
extern byte kLoaderPreAllocatedMemory[];

// Data structure that holds information passed from the 32-bit loader to the 64-bit kernel
LoaderData_32_64_Pass loader_data;

static void MultibootCheck(u32 boot_loader_magic)
{
    TRACE_INFO("Checking for Multiboot2...");
    if (boot_loader_magic != multiboot::kMultiboot2BootloaderMagic) {
        KernelPanic("Multiboot2 check failed!");
    }
    TRACE_SUCCESS("Multiboot2 check passed!");
}

static void HardwareChecks()
{
    BlockHardwareInterrupts();

    TRACE_INFO("Checking for hardware features");

    TRACE_INFO("Checking for CPUID...");
    if (CheckCpuId()) {
        KernelPanic("CPUID check failed!");
    }
    TRACE_SUCCESS("CPUID check passed!");

    TRACE_INFO("Checking for long mode...");
    if (CheckLongMode()) {
        KernelPanic("Long mode check failed!");
    }
    TRACE_SUCCESS("Long mode check passed!");
}

static void IdentityMap(LoaderMemoryManager* loader_memory_manager)
{
    TRACE_INFO("Identity mapping first 512 GiB of memory...");

    static constexpr u32 k1GiB = 1 << 30;
    for (u32 i = 0; i < 512; i++) {
        u64 addr_64bit = static_cast<u64>(i) * k1GiB;
        loader_memory_manager->MapVirtualMemoryToPhysical<LoaderMemoryManager::PageSize::Page1G>(
            addr_64bit, addr_64bit,
            LoaderMemoryManager::kPresentBit | LoaderMemoryManager::kWriteBit
        );
    }
    TODO_WHEN_DEBUGGING_FRAMEWORK
    //    loader_memory_manager->DumpPmlTables();

    TRACE_SUCCESS("Identity mapping complete!");
}

static void AddingMemoryToManagerFromMap(
    LoaderMemoryManager* loader_memory_manager, multiboot::tag_mmap_t* mmap_tag
)
{
    TRACE_INFO("Adding available memory regions to memory manager...");
    WalkMemoryMap(mmap_tag, [&](multiboot::memory_map_t* mmap_entry) FORCE_INLINE_L {
        if (mmap_entry->type == multiboot::memory_map_t::kMemoryAvailable) {
            loader_memory_manager->AddMemoryMapEntry(mmap_entry);
            TRACE_INFO(
                "Memory region: 0x%llX-0x%llX, length: %llu bytes - Added to memory manager",
                mmap_entry->addr, mmap_entry->addr + mmap_entry->len, mmap_entry->len
            );
        }
    });
    TODO_WHEN_DEBUGGING_FRAMEWORK
    //    loader_memory_manager->DumpMemoryMap();

    TRACE_INFO(
        "Total available memory: %llu MB", loader_memory_manager->GetAvailableMemoryBytes() >> 20
    );
}

static multiboot::tag_module_t* GetLoader64Module(void* multiboot_info_addr)
{
    TRACE_INFO("Searching for loader64 module...");
    auto* loader64_module = multiboot::FindTagInMultibootInfo<
        multiboot::tag_module_t, [](multiboot::tag_module_t* tag) -> bool {
            return strcmp(tag->cmdline, "loader64") == 0;
        }>(multiboot_info_addr);
    if (loader64_module == nullptr) {
        KernelPanic("loader64 module not found in multiboot tags!");
    }
    TRACE_SUCCESS("Found loader64 module in multiboot tags!");

    return loader64_module;
}

static multiboot::tag_mmap_t* GetMemoryMapTag(void* multiboot_info_addr)
{
    TRACE_INFO("Searching for memory map tag...");
    auto* mmap_tag = multiboot::FindTagInMultibootInfo<multiboot::tag_mmap_t>(multiboot_info_addr);
    if (mmap_tag == nullptr) {
        KernelPanic("Memory map tag not found in multiboot tags!");
    }
    TRACE_SUCCESS("Memory map tag found!");

    return mmap_tag;
}

static u64 LoadLoader64Module(multiboot::tag_module_t* loader64_module)
{
    byte* loader_module_start_addr = reinterpret_cast<byte*>(loader64_module->mod_start);

    TRACE_INFO("Loading module...");
    u64 kernel_entry_point = elf::LoadElf64(loader_module_start_addr, 0);
    if (kernel_entry_point == 0) {
        KernelPanic("Failed to load kernel module!");
    }
    TRACE_SUCCESS("Module loaded!");

    return kernel_entry_point;
}

extern "C" void MainLoader32(u32 boot_loader_magic, void* multiboot_info_addr)
{
    //    OsHang();
    TerminalInit();
    TRACE_INFO("In 32 bit mode");

    MultibootCheck(boot_loader_magic);

    ////////////////////////////// Hardware Checks ///////////////////////////////
    TRACE_INFO("Starting hardware checks...");
    HardwareChecks();

    ///////////////////////////// Enabling Hardware //////////////////////////////
    TRACE_INFO("Enabling hardware features...");
    BlockHardwareInterrupts();
    auto* loader_memory_manager = new (kLoaderPreAllocatedMemory) LoaderMemoryManager();
    TRACE_SUCCESS("Loader memory manager created!");
    IdentityMap(loader_memory_manager);

    TRACE_INFO("Enabling long mode...");
    EnableLongMode();
    TRACE_SUCCESS("Long mode enabled!");

    TRACE_INFO("Enabling paging...");
    EnablePaging(reinterpret_cast<void*>(loader_memory_manager->GetPml4Table()));
    TRACE_SUCCESS("Paging enabled!");

    TRACE_INFO("Finished hardware features setup for 32-bit mode.");

    /////////////////////// Preparation for jump to 64 bit ///////////////////////
    TRACE_INFO("Starting 64-bit kernel...");

    auto* mmap_tag = GetMemoryMapTag(multiboot_info_addr);
    AddingMemoryToManagerFromMap(loader_memory_manager, mmap_tag);

    //////////////////////////// Loading Loader64 Module //////////////////////////

    auto* loader64_module  = GetLoader64Module(multiboot_info_addr);
    u64 kernel_entry_point = LoadLoader64Module(loader64_module);

    ///////////////////// Initializing LoaderData Structure //////////////////////
    loader_data.multiboot_info_addr         = reinterpret_cast<u32>(multiboot_info_addr);
    loader_data.multiboot_header_start_addr = reinterpret_cast<u32>(multiboot_header_start);
    loader_data.multiboot_header_end_addr   = reinterpret_cast<u32>(multiboot_header_end);
    loader_data.loader_start_addr           = reinterpret_cast<u32>(loader_start);
    loader_data.loader_end_addr             = reinterpret_cast<u32>(loader_end);
    loader_data.loader_memory_manager_addr  = reinterpret_cast<u64>(loader_memory_manager);

    //////////////////////////// Printing LoaderData Info /////////////////////////
    TODO_WHEN_DEBUGGING_FRAMEWORK
    // Convert addresses to hexadecimal strings
    //    TRACE_INFO("LoaderData Address: 0x%X", reinterpret_cast<u32>(&loader_data));
    //    TRACE_INFO("LoaderData multiboot_info_addr: 0x%X", loader_data.multiboot_info_addr);
    //    TRACE_INFO(
    //        "LoaderData multiboot_header_start_addr: 0x%X",
    //        loader_data.multiboot_header_start_addr
    //    );
    //    TRACE_INFO("LoaderData multiboot_header_end_addr: 0x%X",
    //    loader_data.multiboot_header_end_addr); TRACE_INFO("LoaderData loader_start_addr: 0x%X",
    //    loader_data.loader_start_addr); TRACE_INFO("LoaderData loader_end_addr: 0x%X",
    //    loader_data.loader_end_addr);

    //////////////////////////// Jumping to 64-bit /////////////////////////
    TRACE_INFO("Jumping to 64-bit loader...");

    TODO_WHEN_DEBUGGING_FRAMEWORK
    //    TRACE_INFO(
    //        "Kernel entry point: 0x%X-%X", static_cast<u32>(kernel_entry_point >> 32),
    //        static_cast<u32>(kernel_entry_point & k32BitMask)
    //    );

    EnterElf64(
        (void*)static_cast<u32>(kernel_entry_point >> 32),
        (void*)static_cast<u32>(kernel_entry_point & kBitMask32), &loader_data
    );
}
