
#include <assert.h>

#include "boot_args.hpp"
#include "hal/boot_args.hpp"
#include "mem/types.hpp"

using namespace Mem;

BootArguments SanitizeBootArgs(const hal::RawBootArguments &raw_args)
{
    R_ASSERT_NOT_ZERO(
        raw_args.multiboot_info_phys_addr, "Multiboot info physical address is null."
    );
    R_ASSERT_NOT_ZERO(raw_args.kernel_start_addr, "Kernel start physical address is null.");
    R_ASSERT_NOT_ZERO(raw_args.kernel_end_addr, "Kernel end physical address is null.");
    R_ASSERT_NOT_ZERO(
        raw_args.mem_info_bitmap_phys_addr, "Memory bitmap physical address is null."
    );
    R_ASSERT_GT(raw_args.mem_info_total_pages, 0UL, "Total memory pages is zero.");

    BootArguments sanitized_k_args{
        .kernel_start      = UptrToPtr<void>(raw_args.kernel_start_addr),
        .kernel_end        = UptrToPtr<void>(raw_args.kernel_end_addr),
        .root_page_table   = UptrToPtr<void>(raw_args.page_table_phys_addr),
        .mem_bitmap        = UptrToPtr<void>(raw_args.mem_info_bitmap_phys_addr),
        .total_page_frames = static_cast<size_t>(raw_args.mem_info_total_pages),
        .multiboot_info    = UptrToPtr<void>(raw_args.multiboot_info_phys_addr),
    };

    return sanitized_k_args;
}
