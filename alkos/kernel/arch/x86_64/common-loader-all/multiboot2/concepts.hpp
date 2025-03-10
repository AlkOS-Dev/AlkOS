#ifndef ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MULTIBOOT2_CONCEPTS_HPP_
#define ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MULTIBOOT2_CONCEPTS_HPP_

#include <extensions/type_traits.hpp>

namespace multiboot
{

/// Callback that returns true if the tag is the one we are looking for and false otherwise
template <class FilterT, class TagT>
concept TagFilter = requires(FilterT filter, TagT* tag) {
    { filter(tag) } -> std::convertible_to<bool>;
};

template <typename Callback>
concept MemoryMapCallback = requires(Callback cb, memory_map_t* entry) {
    { cb(entry) };
};

}  // namespace multiboot

#endif  // ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MULTIBOOT2_CONCEPTS_HPP_
