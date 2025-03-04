#ifndef ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_MULTIBOOT2_EXTENSIONS_HPP_
#define ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_MULTIBOOT2_EXTENSIONS_HPP_

#include <multiboot2/multiboot2.h>
#include <types.hpp>

namespace multiboot
{

/**
 * @brief Get the tag name from the tag type.
 *
 * @param type The tag type.
 * @return const char* The tag name.
 */
const char* GetTagName(u32 type);

/**
 * @brief Find a tag in the multiboot info.
 *
 * @param multiboot_info_addr The address of the multiboot info.
 * @param type The type of the tag.
 *
 * @return multiboot_tag* The tag if found, nullptr otherwise.
 */
tag_t* FindTagInMultibootInfo(void* multiboot_info_addr, u32 type);

template <typename Callback>
concept MemoryMapCallback = requires(Callback cb, memory_map_t* entry) {
    { cb(entry) };
};

/**
 * @brief Walk the memory map.
 *
 * @param mmap_tag The memory map tag pointer.
 * @param callback The callback function that accepts a memory map entry.
 */
template <MemoryMapCallback Callback>
void WalkMemoryMap(tag_mmap_t* mmap_tag, Callback callback);

}  // namespace multiboot

#include "extensions.tpp"

#endif  // ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_MULTIBOOT2_EXTENSIONS_HPP_
