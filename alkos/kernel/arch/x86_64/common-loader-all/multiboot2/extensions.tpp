#ifndef ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MULTIBOOT2_EXTENSIONS_TPP_
#define ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MULTIBOOT2_EXTENSIONS_TPP_

#include <extensions/debug.hpp>
#include "todo.hpp"

namespace multiboot
{

template <class Tag, TagFilter<Tag> auto Filter>
Tag *FindTagInMultibootInfo(void *multiboot_info_addr)
{
    const u32 kType      = TagNumber<Tag>::value;
    const char *kTagName = TagNumber<Tag>::kTagName;
    static_assert(kType != TagNumber<Tag>::kInvalidTagNumber, "Invalid tag type!");

    TODO_WHEN_DEBUGGING_FRAMEWORK
    //    TRACE_INFO("Searching for tag type: %s", GetTagName(kType));
    for (auto *tag = reinterpret_cast<tag_t *>(static_cast<byte *>(multiboot_info_addr) + 8);
         tag->type != kMultibootTagTypeEnd;
         tag =
             reinterpret_cast<tag_t *>(reinterpret_cast<uintptr_t>(tag) + AlignUp(tag->size, 8))) {
        if (tag->type == kType && Filter(reinterpret_cast<Tag *>(tag))) {
            TODO_WHEN_DEBUGGING_FRAMEWORK
            //            TRACE_SUCCESS("Found tag type: %s", kTagName);
            return reinterpret_cast<Tag *>(tag);
        }
    }
    TRACE_ERROR("Tag type: %s not found!", kTagName);
    return nullptr;
}

template <MemoryMapCallback Callback>
void WalkMemoryMap(tag_mmap_t *mmap_tag, Callback callback)
{
    TODO_WHEN_DEBUGGING_FRAMEWORK
    //    TRACE_INFO("Walking memory map...");
    for (auto *mmap_entry = reinterpret_cast<memory_map_t *>(mmap_tag->entries);
         reinterpret_cast<uintptr_t>(mmap_entry) <
         reinterpret_cast<uintptr_t>(mmap_tag) + mmap_tag->size;
         mmap_entry = reinterpret_cast<memory_map_t *>(
             reinterpret_cast<uintptr_t>(mmap_entry) + mmap_tag->entry_size
         )) {
        callback(mmap_entry);
    }
    //    TRACE_SUCCESS("Memory map walk complete!");
}

}  // namespace multiboot

#endif  // ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MULTIBOOT2_EXTENSIONS_TPP_
