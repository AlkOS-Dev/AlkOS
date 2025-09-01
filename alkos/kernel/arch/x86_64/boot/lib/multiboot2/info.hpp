#ifndef ALKOS_BOOT_LIB_MULTIBOOT2_INFO_HPP_
#define ALKOS_BOOT_LIB_MULTIBOOT2_INFO_HPP_

#include <extensions/bit.hpp>
#include <extensions/concepts.hpp>
#include <extensions/defines.hpp>
#include <extensions/expected.hpp>
#include <extensions/types.hpp>
#include "extensions/style_aliases.hpp"

#include "multiboot2/error.hpp"
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
    { tag.type };
};

template <class FilterT, class TagT>
concept TagFilter = requires(FilterT filter, TagT* tag) {
    { filter(tag) } -> std::convertible_to<bool>;
};

class MultibootInfo
{
    public:
    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//

    MultibootInfo(const u64 multiboot_info_addr) : multiboot_info_addr_{multiboot_info_addr} {}

    //------------------------------------------------------------------------------//
    // Public Methods
    //------------------------------------------------------------------------------//

    template <TagCallback Callback>
    void WalkTags(Callback cb)
    {
        for (Tag* tag_ptr = reinterpret_cast<Tag*>(multiboot_info_addr_ + 8);
             tag_ptr->type != kMultibootTagTypeEnd;
             tag_ptr = reinterpret_cast<Tag*>(
                 reinterpret_cast<u8*>(tag_ptr) + AlignUp(tag_ptr->size, 8u)
             )) {
            Tag& tag = *tag_ptr;
            cb(tag);
        }
    }

    template <TagT Tag, typename Filter>
    Expected<Tag*, Error> FindTag(Filter filter)
    {
        const u32 kType = TagMetadata<Tag>::kValue;
        static_assert(kType != kInvalidTagNumber);

        for (auto* tag_ptr = reinterpret_cast<Multiboot::Tag*>(multiboot_info_addr_ + 8);
             tag_ptr->type != kMultibootTagTypeEnd;
             tag_ptr = reinterpret_cast<Multiboot::Tag*>(
                 reinterpret_cast<u8*>(tag_ptr) + AlignUp(tag_ptr->size, 8u)
             )) {
            if (tag_ptr->type == kType) {
                auto* specific_tag = reinterpret_cast<Tag*>(tag_ptr);
                if (filter(specific_tag)) {
                    return specific_tag;
                }
            }
        }
        return Unexpected(Error::TagNotFound);
    }

    template <TagT Tag>
    Expected<Tag*, Error> FindTag()
    {
        return FindTag<Tag>([](const Tag*) {
            return true;
        });
    }

    private:
    //------------------------------------------------------------------------------//
    // Private Fields
    //------------------------------------------------------------------------------//
    const u64 multiboot_info_addr_;
};

}  // namespace Multiboot

#endif  // ALKOS_BOOT_LIB_MULTIBOOT2_INFO_HPP_
