#include "mem/heap.hpp"

#include "hal/constants.hpp"
#include "mem/phys/mngr/bitmap.hpp"
#include "modules/memory.hpp"

namespace Mem
{

Expected<VPtr<void>, MemError> KMalloc(size_t size)
{
    using AllocationRequest = BitmapPmm::AllocationRequest;

    ASSERT_NEQ(size, 0, "KMalloc size cannot be 0");
    if (size == 0) {
        return Unexpected(MemError::InvalidArgument);
    }

    const size_t required_pages = (size + hal::kPageSizeBytes - 1) / hal::kPageSizeBytes;
    auto &b_pmm                 = MemoryModule::Get().GetBitmapPmm();
    auto res                    = b_pmm.Alloc(AllocationRequest{.num_pages = required_pages});
    R_ASSERT_TRUE(res, "Failed to allocate memory from bitmap physical memory manager");

    const auto phys_ptr = *res;
    const VPtr<void> m  = reinterpret_cast<VPtr<void>>(PhysToVirt(phys_ptr));
    return m;
}

template <>
void KFree([[maybe_unused]] VPtr<void> ptr)
{
    // Dont care about free till buddy allocator.
}

}  // namespace Mem
