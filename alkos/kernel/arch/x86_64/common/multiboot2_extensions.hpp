#ifndef ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_MULTIBOOT2_EXTENSIONS_HPP_
#define ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_MULTIBOOT2_EXTENSIONS_HPP_

#include <multiboot2.h>
#include <types.hpp>

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
multiboot_tag* FindTagInMultibootInfo(void* multiboot_info_addr, u32 type);

template <typename Callback>
concept MemoryMapCallback = requires(Callback cb, multiboot_memory_map_t* entry) {
    { cb(entry) };
};

/**
 * @brief Walk the memory map.
 *
 * @param mmap_tag The memory map tag pointer.
 * @param callback The callback function that accepts a memory map entry.
 */
template <MemoryMapCallback Callback>
void WalkMemoryMap(multiboot_tag_mmap* mmap_tag, Callback callback);

#include <multiboot2_extensions.tpp>

#endif  // ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_MULTIBOOT2_EXTENSIONS_HPP_
