#ifndef ALKOS_KERNEL_ARCH_X86_64_COMMON_MODELS_LOADER_DATA_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_COMMON_MODELS_LOADER_DATA_HPP_

#include <extensions/types.hpp>
#include <defines.hpp>
#include "memory_span.hpp"

namespace loader {
namespace bit32 {

struct LoaderData {
  u64 multiboot_info_addr;  ///< Address of the Multiboot2 information structure
  MemorySpan multiboot_header_span;  ///< Span of the Multiboot2 header
  MemorySpan memory_manager_span;  ///< Span of the LoaderMemoryManager instance
};

namespace bit64 {
struct LoaderData {
  u64 multiboot_info_addr;
  MemorySpan multiboot_header_span;
  MemorySpan memory_manager_span;  
  MemorySpan kernel_span;
}
}

  // namespace bit32
} // namespace loader

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_MODELS_LOADER_DATA_HPP_
