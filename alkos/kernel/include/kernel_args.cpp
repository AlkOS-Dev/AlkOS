
#include <assert.h>

#include "hal/kernel.hpp"
#include "kernel_args.hpp"
#include "mem/phys/ptr.hpp"
#include "mem/virt/ptr.hpp"

using namespace mem;

KernelArguments SanitizeKernelArgs(const hal::RawKernelArguments raw_args)
{
    R_ASSERT_NOT_ZERO(
        raw_args.multiboot_info_phys_addr, "Multiboot info physical address is null."
    );
    R_ASSERT_NOT_ZERO(raw_args.kernel_start_addr, "Kernel start physical address is null.");
    R_ASSERT_NOT_ZERO(raw_args.kernel_end_addr, "Kernel end physical address is null.");
    R_ASSERT_NOT_ZERO(
        raw_args.mem_info_bitmap_phys_addr, "Memory bitmap physical address is null."
    );
    R_ASSERT_GT(raw_args.mem_info_total_pages, 0, "Total memory pages is zero.");

    KernelArguments sanitized_k_args{
        .kernel_start      = VirtualPtr<void>(raw_args.kernel_start_addr),
        .kernel_end        = VirtualPtr<void>(raw_args.kernel_end_addr),
        .root_page_table   = PhysicalPtr<void>(raw_args.page_table_phys_addr),
        .mem_bitmap        = PhysicalPtr<void>(raw_args.mem_info_bitmap_phys_addr),
        .total_page_frames = static_cast<size_t>(raw_args.mem_info_total_pages),
        .multiboot_info    = PhysicalPtr<void>(raw_args.multiboot_info_phys_addr),
    };

    return sanitized_k_args;
}
