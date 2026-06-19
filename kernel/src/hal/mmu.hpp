// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 The AlkOS Authors
// See the AUTHORS file for the full list of contributors.

#ifndef KERNEL_SRC_HAL_MMU_HPP_
#define KERNEL_SRC_HAL_MMU_HPP_

#include <hal/impl/mmu.hpp>
#include <internal/macros.hpp>

#include "hal/constants.hpp"
#include "mem/mmu/contexts.hpp"

namespace hal
{

using arch::MmuContext;

using std::expected;
using std::unexpected;

using PageFlags = arch::PageFlags;

class Mmu : public arch::Mmu
{
    public:
    /// The range is treated as half-open: [start, start + size)
    template <MmuContext Context>
    expected<void, Mem::MemError> MapRange(
        Context &ctx, Mem::PPtr<void> root, Mem::VPtr<void> vaddr_start,
        Mem::PPtr<void> paddr_start, size_t size, PageFlags flags
    )
    {
        using namespace Mem;

        auto vaddr = PtrToUptr(AlignDown(vaddr_start, kPageSizeBytes));
        auto paddr = PtrToUptr(AlignDown(paddr_start, kPageSizeBytes));
        auto end   = AlignUp(PtrToUptr(vaddr_start) + size, kPageSizeBytes);

        for (auto v = vaddr, p = paddr; v < end; v += kPageSizeBytes, p += kPageSizeBytes) {
            auto map_res = Map(ctx, root, UptrToPtr<void>(v), UptrToPtr<void>(p), flags);
            RET_UNEXPECTED_IF_ERR(map_res);
        }
        return {};
    }

    /// The range is treated as half-open: [start, start + size)
    template <MmuContext Context>
    void UnmapRange(Context &ctx, Mem::PPtr<void> root, Mem::VPtr<void> start, size_t size)
    {
        using namespace Mem;

        auto s = PtrToUptr(AlignDown(start, kPageSizeBytes));
        auto e = AlignUp(PtrToUptr(start) + size, kPageSizeBytes);

        for (auto addr = s; addr < e; addr += kPageSizeBytes) {
            arch::Mmu::Unmap(ctx, root, UptrToPtr<void>(addr));
        }
    }
};

}  // namespace hal

#endif  // KERNEL_SRC_HAL_MMU_HPP_
