#include "mem/heap.hpp"

#include "hal/constants.hpp"
#include "mem/phys/mngr/bitmap.hpp"
#include "modules/memory.hpp"

using namespace mem;

Expected<VirtualPtr<void>, MemError> KMalloc(size_t size)
{
    using AllocationRequest = BitmapPmm::AllocationRequest;

    if (size == 0) {
        return Unexpected(MemError::InvalidArgument);
    }

    const size_t required_pages = (size + hal::kPageSizeBytes - 1) / hal::kPageSizeBytes;
    const auto &b_pmm           = MemoryModule::Get().GetBitmapPmm();
    const auto res              = b_pmm.Alloc(AllocationRequest{.num_pages = required_pages});
    ASSERT_TRUE(res, "KMalloc out of mem");

    const auto phys_ptr      = *res;
    const VirtualPtr<void> m = reinterpret_cast<VirtualPtr<void>>(phys_ptr.ToVirt());
    return m;
}

Expected<void, MemError> KFree(VirtualPtr<void>)
{
    // Dont care about free till buddy allocator.
}
