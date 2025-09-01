#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__x86_64__)
#error "AlkOS needs to be compiled with a x86_64-elf compiler"
#endif

#include <extensions/debug.hpp>
#include <extensions/expected.hpp>
#include "extensions/style_aliases.hpp"

#include "cpu/utils.hpp"

#include "abi/transition_data.hpp"

#include "elf/elf_64.hpp"
#include "elf/error.hpp"

#include "mem/memory_manager.hpp"

#include "sys/panic.hpp"
#include "sys/terminal.hpp"

#include "multiboot2/info.hpp"
#include "multiboot2/multiboot2.h"

#include "settings.hpp"

using namespace Multiboot;

/* external init procedures */
extern "C" void EnterKernel(u64 kernel_entry_addr, KernelInitialParams* kernel_inital_params);
extern const char loader_64_start[];
extern const char loader_64_end[];
extern byte kLoaderPreAllocatedMemory[];

KernelInitialParams kernel_inital_params;

static bool ValidateTransitionData(TransitionData* transition_data)
{
    if (transition_data == nullptr) {
        return false;
    }
    return true;
}

static Multiboot::TagModule* FindKernelModule(MultibootInfo multiboot_info)
{
    auto kernel_module_res =
        multiboot_info.FindTag<Multiboot::TagModule>([](Multiboot::TagModule* tag) -> bool {
            return strcmp(tag->cmdline, "kernel") == 0;
        });

    if (!kernel_module_res) {
        KernelPanic("Kernel module not found in multiboot tags!");
    }

    return kernel_module_res.value();
}

extern "C" void MainLoader64(TransitionData* transition_data)
{
    TerminalInit();
    TRACE_INFO("In 64 bit mode");

    if (!ValidateTransitionData(transition_data)) {
        KernelPanic("TransitionData check failed!");
    }

    TRACE_DEBUG("Finding the kernel module...");
    auto* kernel_module = FindKernelModule(transition_data->multiboot_info_addr);

    TRACE_DEBUG("Calculating kernel size...");
    auto bounds_res = Elf64::GetProgramBounds(reinterpret_cast<byte*>(kernel_module->mod_start));
    if (!bounds_res) {
        KernelPanic("Failed to get ELF bounds!");
    }
    auto [elf_lower_bound, elf_upper_bound] = bounds_res.value();
    u64 elf_effective_size                  = elf_upper_bound - elf_lower_bound;

    TRACE_DEBUG("Finding the memory map...");
    MultibootInfo multiboot_info{transition_data->multiboot_info_addr};
    auto mmap_tag_res = multiboot_info.FindTag<Multiboot::TagMmap>();
    if (!mmap_tag_res) {
        KernelPanic("Error finding memory map tag in multiboot tags!");
    }

    TRACE_DEBUG("Mapping the kernel module...");
    auto* memory_manager = reinterpret_cast<MemoryManager*>(transition_data->memory_manager_addr);
    memory_manager->MarkMemoryAreaNotFree(
        reinterpret_cast<u64>(loader_64_start), reinterpret_cast<u64>(loader_64_end)
    );
    memory_manager->MapVirtualRangeUsingExternalMemoryMap<MemoryManager::WalkDirection::Descending>(
        mmap_tag_res.value(), kKernelVirtualAddressStart, elf_effective_size, 0
    );

    TRACE_DEBUG("Loading the kernel module...");
    byte* kernel_module_start_addr = reinterpret_cast<byte*>(kernel_module->mod_start);
    auto k_entry_res               = Elf64::Load(kernel_module_start_addr, 0);
    if (!k_entry_res) {
        KernelPanic("Failed to load kernel module!");
    }

    memory_manager->AddFreeMemoryRegion(
        reinterpret_cast<u64>(loader_64_start), reinterpret_cast<u64>(loader_64_end)
    );

    TRACE_INFO("Jumping to kernel");
    kernel_inital_params.kernel_start_addr           = elf_lower_bound;
    kernel_inital_params.kernel_end_addr             = elf_upper_bound;
    kernel_inital_params.memory_manager_addr         = transition_data->memory_manager_addr;
    kernel_inital_params.multiboot_info_addr         = transition_data->multiboot_info_addr;
    kernel_inital_params.multiboot_header_start_addr = transition_data->multiboot_header_start_addr;
    kernel_inital_params.multiboot_header_end_addr   = transition_data->multiboot_header_end_addr;

    EnterKernel(k_entry_res.value(), &kernel_inital_params);
}
