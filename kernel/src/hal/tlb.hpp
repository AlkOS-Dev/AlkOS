#ifndef KERNEL_SRC_HAL_TLB_HPP_
#define KERNEL_SRC_HAL_TLB_HPP_

#include <hal/impl/tlb.hpp>

#include <bits_ext.hpp>
#include "hal/constants.hpp"

namespace hal
{

class Tlb : public arch::Tlb
{
    using arch::Tlb::Tlb;

    public:
    /// Invalidates TLB entries for the range [start, start + size)
    /// The range is treated as half-open: start is inclusive, end is exclusive.
    void InvalidateRange(Mem::VPtr<void> start, size_t size)
    {
        using namespace Mem;

        auto s = PtrToUptr(AlignDown(start, kPageSizeBytes));
        auto e = AlignUp(PtrToUptr(start) + size, kPageSizeBytes);
        for (auto i = s; i < e; i += kPageSizeBytes) {
            InvalidatePage(UptrToPtr<void>(i));
        }
    }
    void InvalidateRange(Mem::VPtr<void> start, Mem::VPtr<void> end)
    {
        using namespace Mem;
        ASSERT_GE(PtrToUptr(end), PtrToUptr(start));

        size_t size = static_cast<size_t>(PtrToUptr(end) - PtrToUptr(start));
        InvalidateRange(start, size);
    }
};

}  // namespace hal

#endif  // KERNEL_SRC_HAL_TLB_HPP_
