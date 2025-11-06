#ifndef KERNEL_SRC_HAL_API_BOOT_ARGS_HPP_
#define KERNEL_SRC_HAL_API_BOOT_ARGS_HPP_

#include <defines.h>
#include <types.hpp>

namespace arch
{
struct RawBootArguments;
struct PACK RawBootArgumentsAPI {
    u64 kernel_start_addr;
    u64 kernel_end_addr;

    u64 page_table_phys_addr;

    u64 mem_info_bitmap_phys_addr;
    u64 mem_info_total_pages;
};
}  // namespace arch

#endif  // KERNEL_SRC_HAL_API_BOOT_ARGS_HPP_
