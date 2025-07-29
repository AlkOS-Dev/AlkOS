#ifndef ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MULTIBOOT2_MULTIBOOT_INFO_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MULTIBOOT2_MULTIBOOT_INFO_HPP_

#include <extensions/bit.hpp>
#include <extensions/concepts.hpp>
#include <extensions/types.hpp>

#include "models/memory_span.hpp"
#include "multiboot2/multiboot2.h"
#include "multiboot2/tag_metadata.hpp"
#include "todo.hpp"

namespace Multiboot
{

//------------------------------------------------------------------------------//
// Concepts
//------------------------------------------------------------------------------//

template <typename Callback>
concept TagCallback = requires(Callback cb, Tag tag) {
    { cb(tag) };
};

template <typename Tag>
concept TagT = requires(Tag tag) {
    { tag->type };
};

template <class FilterT, class TagT>
concept TagFilter = requires(FilterT filter, TagT *tag) {
    { filter(tag) } -> std::convertible_to<bool>;
};

class MultibootInfo
{
    public:
    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//

    MultibootInfo(u64 multiboot_info_addr) : multiboot_info_addr_{multiboot_info_addr} {}

    //------------------------------------------------------------------------------//
    // Public Methods
    //------------------------------------------------------------------------------//

    MemorySpan GetMemorySpan()
    {
        MemorySpan ms;
        ms.start = multiboot_info_addr_;
    }

    template <TagCallback Callback>
    void WalkTags(Callback cb)
    {
        for (Tag *tag_ptr = reinterpret_cast<Tag *>(multiboot_info_addr_ + 8);
             tag_ptr->type != kMultibootTagTypeEnd; tag_ptr = tag_ptr + AlignUp(tag_ptr->size, 8)) {
            Tag &tag = *tag_ptr;
            cb(tag);
        }
    }

    template <
        TagT Tag,
        TagFilter<Tag> auto Filter = [](Tag *) constexpr -> bool {
            return true;
        }>
    Tag *FindTag()
    {
        const u32 kType = TagMetadata<Tag>::kValue;
        TODO_WHEN_DEBUGGING_FRAMEWORK
        [[maybe_unused]] const char *kName = TagMetadata<Tag>::kTagName;
        static_assert(kType != kInvalidTagNumber);

        for (Tag *tag_ptr = reinterpret_cast<Tag *>(multiboot_info_addr_ + 8);
             tag_ptr->type != kMultibootTagTypeEnd; tag_ptr = tag_ptr + AlignUp(tag_ptr->size, 8)) {
            if (tag_ptr->type == kType && Filter(tag_ptr)) {
                return tag_ptr;
            }
        }
        return nullptr;
    }

    private:
    //------------------------------------------------------------------------------//
    // Private Fields
    //------------------------------------------------------------------------------//
    const u64 multiboot_info_addr_;
};

}  // namespace Multiboot

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MULTIBOOT2_MULTIBOOT_INFO_HPP_
