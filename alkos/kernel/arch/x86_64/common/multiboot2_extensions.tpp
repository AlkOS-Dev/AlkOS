#ifndef ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_MULTIBOOT2_EXTENSIONS_TPP_
#define ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_MULTIBOOT2_EXTENSIONS_TPP_

#include <debug.hpp>

namespace multiboot
{

template <MemoryMapCallback Callback>
void WalkMemoryMap(tag_mmap_t *mmap_tag, Callback callback)
{
    TRACE_INFO("Walking memory map...");
    for (auto *mmap_entry = reinterpret_cast<memory_map_t *>(mmap_tag->entries);
         reinterpret_cast<u8 *>(mmap_entry) < reinterpret_cast<u8 *>(mmap_tag) + mmap_tag->size;
         mmap_entry = reinterpret_cast<memory_map_t *>(
             reinterpret_cast<u8 *>(mmap_entry) + mmap_tag->entry_size
         )) {
        callback(mmap_entry);
    }
    TRACE_SUCCESS("Memory map walk complete!");
}

}  // namespace multiboot

#endif  // ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_MULTIBOOT2_EXTENSIONS_TPP_
