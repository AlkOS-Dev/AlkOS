#include "mem/page_meta_table.hpp"
#include "hal/constants.hpp"

using namespace Mem;
using enum PageMetaType;

size_t PageMetaTable::CalcRequiredSize(size_t num_page_frames)
{
    return num_page_frames * sizeof(PageMeta<Dummy>);
};
