#ifndef ALKOS_BOOT_LIB_MULTIBOOT2_MEMORY_MAP_HPP_
#define ALKOS_BOOT_LIB_MULTIBOOT2_MEMORY_MAP_HPP_

#include <extensions/bit.hpp>
#include <extensions/expected.hpp>
#include <extensions/types.hpp>

#include "multiboot2/multiboot2.h"

namespace Multiboot
{

//------------------------------------------------------------------------------//
// Concepts
//------------------------------------------------------------------------------//

template <typename Callback>
concept MmapEntryCallback = requires(Callback cb, MmapEntry& entry) {
    { cb(entry) };
};

class MemoryMap
{
    public:
    //------------------------------------------------------------------------------
    // Internal Classes
    //------------------------------------------------------------------------------

    class Iterator
    {
        public:
        Iterator(MmapEntry* mmap_entry, uintptr_t entries_size)
            : current_entry_{mmap_entry}, entries_size_{entries_size}
        {
        }

        MmapEntry& operator*() { return *current_entry_; }

        Iterator& operator++()
        {
            current_entry_ = reinterpret_cast<MmapEntry*>(
                reinterpret_cast<uintptr_t>(current_entry_) + entries_size_
            );
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator cp    = *this;
            current_entry_ = reinterpret_cast<MmapEntry*>(
                reinterpret_cast<uintptr_t>(current_entry_) + entries_size_
            );
            return cp;
        }

        friend bool operator!=(const Iterator& a, const Iterator& b)
        {
            return a.current_entry_ != b.current_entry_;
        }
        friend bool operator==(const Iterator& a, const Iterator& b) { return !(a != b); }

        private:
        MmapEntry* current_entry_;
        uintptr_t entries_size_;
    };

    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//

    MemoryMap(TagMmap* mmap_tag_ptr) : mmap_tag_ptr_{mmap_tag_ptr} {}

    //------------------------------------------------------------------------------//
    // Public Methods
    //------------------------------------------------------------------------------//

    // TODO: Iterator lacks methods for const objects
    // and crashes if you iterate over real objects and not references

    Iterator begin()
    {
        return Iterator{
            reinterpret_cast<MmapEntry*>(mmap_tag_ptr_->entries),
            static_cast<uintptr_t>(mmap_tag_ptr_->entry_size)
        };
    }

    Iterator end()
    {
        auto* end_ptr = reinterpret_cast<MmapEntry*>(
            reinterpret_cast<uintptr_t>(mmap_tag_ptr_) + mmap_tag_ptr_->size
        );
        return Iterator{end_ptr, static_cast<uintptr_t>(mmap_tag_ptr_->entry_size)};
    }

    private:
    //------------------------------------------------------------------------------//
    // Private Fields
    //------------------------------------------------------------------------------//
    TagMmap* mmap_tag_ptr_;
};

}  // namespace Multiboot

#endif  // ALKOS_BOOT_LIB_MULTIBOOT2_MEMORY_MAP_HPP_
