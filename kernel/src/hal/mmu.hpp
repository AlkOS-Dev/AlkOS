#ifndef KERNEL_SRC_HAL_MMU_HPP_
#define KERNEL_SRC_HAL_MMU_HPP_

#include <hal/impl/mmu.hpp>

#include <bits_ext.hpp>
#include "hal/constants.hpp"

namespace hal
{

using PageFlags = arch::PageFlags;

class Mmu : public arch::Mmu
{
    using arch::Mmu::Mmu;

    public:
    /// Maps a range of virtual memory to physical memory
    /// The range is treated as half-open: [start, start + size)
    Expected<void, Mem::MemError> MapRange(
        Mem::VPtr<Mem::AddressSpace> as, Mem::VPtr<void> vaddr_start, Mem::PPtr<void> paddr_start,
        size_t size, PageFlags flags
    )
    {
        using namespace Mem;

        auto vaddr = PtrToUptr(AlignDown(vaddr_start, kPageSizeBytes));
        auto paddr = PtrToUptr(AlignDown(paddr_start, kPageSizeBytes));
        auto end   = AlignUp(PtrToUptr(vaddr_start) + size, kPageSizeBytes);

        for (auto v = vaddr, p = paddr; v < end; v += kPageSizeBytes, p += kPageSizeBytes) {
            auto map_res = Map(as, UptrToPtr<void>(v), UptrToPtr<void>(p), flags);
            UNEXPETED_RET_IF_ERR(map_res);
        }
        return {};
    }

    /// Unmaps a range of virtual memory
    /// The range is treated as half-open: [start, start + size)
    Expected<void, Mem::MemError> UnMapRange(
        Mem::VPtr<Mem::AddressSpace> as, Mem::VPtr<void> start, size_t size
    )
    {
        using namespace Mem;

        auto s = PtrToUptr(AlignDown(start, kPageSizeBytes));
        auto e = AlignUp(PtrToUptr(start) + size, kPageSizeBytes);

        for (auto addr = s; addr < e; addr += kPageSizeBytes) {
            auto unmap_res = UnMap(as, UptrToPtr<void>(addr));
            // Continue even if unmap fails (page might not have been mapped yet)
        }
        return {};
    }
};

}  // namespace hal

#endif  // KERNEL_SRC_HAL_MMU_HPP_
