#include <extensions/bit.hpp>
#include <extensions/tuple.hpp>
#include <extensions/types.hpp>
#include "multiboot2/multiboot2.h"

namespace multiboot
{
std::tuple<u64, u64> GetMultibootStructureBounds(void *multiboot_info_addr)
{
    u64 lower_bound = reinterpret_cast<u64>(multiboot_info_addr);
    u64 upper_bound = 0;

    auto *tag = reinterpret_cast<tag_t *>(static_cast<byte *>(multiboot_info_addr) + 8);
    while (tag->type != kMultibootTagTypeEnd) {
        tag = reinterpret_cast<tag_t *>(reinterpret_cast<u64>(tag) + AlignUp(tag->size, 8));
    }
    tag_t *end_tag    = tag;
    auto end_tag_addr = reinterpret_cast<uintptr_t>(end_tag);
    upper_bound       = end_tag_addr + end_tag->size;
    return {lower_bound, upper_bound};
}

}  // namespace multiboot
