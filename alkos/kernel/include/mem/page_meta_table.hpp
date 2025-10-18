#ifndef ALKOS_KERNEL_INCLUDE_MEM_PAGE_META_TABLE_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_PAGE_META_TABLE_HPP_

#include "mem/page_meta.hpp"
#include "mem/phys/ptr.hpp"
#include "mem/virt/ptr.hpp"

namespace Mem
{
class PageMetaTable
{
    public:
    PageMetaTable() = default;
    PageMetaTable(VirtualPtr<PageMeta<Dummy>> page_frames_metas, size_t num_page_frames)
        : page_frames_metas_{page_frames_metas}, num_page_frames_{num_page_frames}
    {
    }
    ~PageMetaTable() = default;

    static size_t CalcRequiredSize(size_t num_page_frames);

    private:
    VirtualPtr<PageMeta<Dummy>> page_frames_metas_;
    size_t num_page_frames_;
};
}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_PAGE_META_TABLE_HPP_
