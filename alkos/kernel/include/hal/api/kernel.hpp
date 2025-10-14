#ifndef ALKOS_KERNEL_INCLUDE_HAL_API_KERNEL_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_API_KERNEL_HPP_

namespace arch
{
struct KernelArguments;
struct PACK KernelArgumentsAPI {
    u64 kernel_start_addr;
    u64 kernel_end_addr;

    u64 page_table_phys_addr;

    u64 mem_info_bitmap_addr;
    u64 mem_info_total_pages;
};
}  // namespace arch

#endif  // ALKOS_KERNEL_INCLUDE_HAL_API_KERNEL_HPP_
