#ifndef ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_64_KERNEL_DEFINITIONS_LOADER64_DATA_HPP_
#define ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_64_KERNEL_DEFINITIONS_LOADER64_DATA_HPP_

#include "definitions/page_buffer.hpp"

/// This is the structure that is passed from the 64 bit loader to the kernel
/// It is unused for now but will be used in physical memory management update
namespace loader64
{
struct PACK LoaderData {
    PageBufferParams_t page_buffer_params;
};
}  // namespace loader64

#endif  // ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_64_KERNEL_DEFINITIONS_LOADER64_DATA_HPP_
