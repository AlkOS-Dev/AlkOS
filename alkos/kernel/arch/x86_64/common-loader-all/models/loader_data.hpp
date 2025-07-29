#ifndef ALKOS_KERNEL_ARCH_X86_64_COMMON_MODELS_LOADER_DATA_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_COMMON_MODELS_LOADER_DATA_HPP_

#include <defines.hpp>
#include <extensions/types.hpp>
#include "models/magic_signature.hpp"
#include "models/memory_span.hpp"

namespace Loader
{
namespace Bit32
{

struct LoaderData {
    // Alignas is NECESSARY here to maintain compatibility with both 32-bit and 64-bit modes.
    // It is in fact necessary for all classes passed between 32-bit and 64-bit modes
    alignas(64) const MagicSignature<"ALKOS32"> magic;  ///< Magic number to identify the loader
                                                        ///< data structure
    alignas(64) u64 multiboot_info_addr;  ///< Address of the Multiboot2 information structure
    alignas(64) MemorySpan multiboot_header_span;  ///< Span of the Multiboot2 header
    alignas(64) MemorySpan memory_manager_span;    ///< Span of the LoaderMemoryManager instance
};
}  // namespace Bit32

namespace Bit64
{
struct LoaderData {
    const MagicSignature<"ALKOS64"> magic;  ///< Magic number to identify the loader data structure
    u64 multiboot_info_addr;
    MemorySpan multiboot_header_span;
    MemorySpan memory_manager_span;
    MemorySpan kernel_span;
};
}  // namespace Bit64

}  // namespace Loader

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_MODELS_LOADER_DATA_HPP_
