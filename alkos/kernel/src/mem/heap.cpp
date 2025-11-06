#include "mem/heap.hpp"

#include "hal/constants.hpp"
#include "mem/phys/mngr/bitmap.hpp"
#include "modules/memory.hpp"

namespace Mem
{

Expected<VPtr<void>, MemError> KMalloc(KMallocRequest r)
{
    using AllocationRequest = BitmapPmm::AllocationRequest;
    auto &size              = r.size;
    auto &al                = r.alignment;

    ASSERT_NEQ(size, 0UL, "KMalloc size cannot be 0");
    if (size == 0) {
        return Unexpected(MemError::InvalidArgument);
    }

    if (al > 0) {
        // TODO: mark the nearest physical page with ptr to actual start of allocated area,
        // so that Free may work
        size += al - 1 + sizeof(VPtr<void>);
    }

    const size_t required_pages = (size + hal::kPageSizeBytes - 1) / hal::kPageSizeBytes;
    auto &b_pmm                 = MemoryModule::Get().GetBitmapPmm();

    auto res = b_pmm.Alloc(AllocationRequest{.num_pages = required_pages});
    R_ASSERT_TRUE(res, "Failed to allocate memory from bitmap physical memory manager");

    auto *const phys_ptr = *res;
    VPtr<void> m         = reinterpret_cast<VPtr<void>>(PhysToVirt(phys_ptr));
    m                    = AlignUp(m, al);
    return m;
}

template <>
void KFree([[maybe_unused]] VPtr<void> ptr)
{
    // Dont care about free till buddy allocator.
}

}  // namespace Mem
