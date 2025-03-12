#ifndef ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_64_KERNEL_DEFINITIONS_PAGE_BUFFER_HPP_
#define ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_64_KERNEL_DEFINITIONS_PAGE_BUFFER_HPP_

#include <defines.hpp>
#include "extensions/types.hpp"

namespace loader64
{

static constexpr u64 kPhysicalPageSize = 0x1000;
struct PageBufferParams_t {
    u64 buffer_addr;             // The address of the physical memory manager buffer
    u64 total_size_num_pages;    // The total size of the physical memory manager buffer
    u64 current_size_num_pages;  // The number of free pages in the physical memory manager buffer
};
}  // namespace loader64

#endif  // ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_64_KERNEL_DEFINITIONS_PAGE_BUFFER_HPP_
