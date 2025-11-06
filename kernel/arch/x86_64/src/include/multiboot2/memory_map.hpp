#ifndef KERNEL_ARCH_X86_64_SRC_INCLUDE_MULTIBOOT2_MEMORY_MAP_HPP_
#define KERNEL_ARCH_X86_64_SRC_INCLUDE_MULTIBOOT2_MEMORY_MAP_HPP_

#include <bit.hpp>
#include <types.hpp>
#include "include/models/memory_span.hpp"
#include "include/multiboot2/multiboot2.h"

namespace Multiboot
{

//------------------------------------------------------------------------------//
// Concepts
//------------------------------------------------------------------------------//

template <typename Callback>
concept MmapEntryCallback = requires(Callback cb, MmapEntry &entry) {
    { cb(entry) };
};

class MemoryMap
{
    public:
    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//

    MemoryMap(TagMmap *mmap_tag_ptr) : mmap_tag_ptr_{mmap_tag_ptr} {}

    //------------------------------------------------------------------------------//
    // Public Methods
    //------------------------------------------------------------------------------//

    template <MmapEntryCallback Callback>
    void WalkEntries(Callback callback)
    {
        for (auto *mmap_entry = reinterpret_cast<MmapEntry *>(mmap_tag_ptr_->entries);
             reinterpret_cast<uintptr_t>(mmap_entry) <
             reinterpret_cast<uintptr_t>(mmap_tag_ptr_) + mmap_tag_ptr_->size;
             mmap_entry = reinterpret_cast<MmapEntry *>(
                 reinterpret_cast<uintptr_t>(mmap_entry) + mmap_tag_ptr_->entry_size
             )) {
            auto &entry_ref = *mmap_entry;
            callback(entry_ref);
        }
    }

    private:
    //------------------------------------------------------------------------------//
    // Private Fields
    //------------------------------------------------------------------------------//
    TagMmap *mmap_tag_ptr_;
};

}  // namespace Multiboot

#endif  // KERNEL_ARCH_X86_64_SRC_INCLUDE_MULTIBOOT2_MEMORY_MAP_HPP_
