#include "mem/phys/mngr/buddy.hpp"
#include "mem/phys/mngr/bitmap.hpp"

using namespace Mem;
using B = BuddyPmm;

void B::Init(BitmapPmm &b_pmm)
{
    auto bitmap_view = b_pmm.GetBitmapView();

    // Initialize
}
