#include <string.h>
#include "data_structures/hash_maps.hpp"
#include "mem/heap.hpp"
#include "test_module/test.hpp"

#include "hal/mmu.hpp"
#include "hal/tlb.hpp"
#include "mem/page.hpp"
#include "mem/virt/addr_space.hpp"
#include "modules/memory.hpp"

using namespace Mem;
using namespace hal;

struct TestMmuContext {
    Mem::BitmapPmm &pmm;
    data_structures::FastMinimalStaticHashmap<uintptr_t, int, 60> ref_counts;

    TestMmuContext(Mem::BitmapPmm &pmm) : pmm(pmm) {}

    expected<Mem::PPtr<void>, Mem::MemError> AllocateTable(uint8_t level)
    {
        (void)level;
        auto res = pmm.Alloc();
        if (!res)
            return unexpected(res.error());
        auto ptr = *res;
        memset(Mem::PhysToVirt(ptr), 0, hal::kPageSizeBytes);
        return reinterpret_cast<Mem::PPtr<void>>(ptr);
    }

    void FreeTable(Mem::PPtr<void> table, uint8_t level)
    {
        (void)level;
        pmm.Free(reinterpret_cast<Mem::PPtr<Mem::Page>>(table));
        ref_counts.Remove(Mem::PtrToUptr(table));
    }

    void IncreaseUsage(Mem::PPtr<void> table)
    {
        auto key = Mem::PtrToUptr(table);
        if (auto *val = ref_counts.Find(key)) {
            (*val)++;
        } else {
            ref_counts.Insert(key, 1);
        }
    }

    bool DecreaseUsage(Mem::PPtr<void> table)
    {
        auto key = Mem::PtrToUptr(table);
        if (auto *val = ref_counts.Find(key)) {
            if (*val > 0)
                (*val)--;
            return *val == 0;
        }
        return true;
    }
};

class MmuTest : public TestGroupBase
{
    protected:
    void Setup_() override
    {
        // Allocate a physical page for the root page table (PML4)
        auto &pmm     = MemoryModule::Get().GetBitmapPmm();
        auto page_res = pmm.Alloc();
        R_ASSERT_TRUE(page_res.has_value());

        pml4_phys_ = *page_res;
        pml4_virt_ = Mem::PhysToVirt(reinterpret_cast<Mem::PPtr<void>>(pml4_phys_));

        // Zero out the PML4 to ensure it's clean and valid
        memset(pml4_virt_, 0, hal::kPageSizeBytes);

        // Initialize Mmu
        mmu_.Init(tlb_);

        // Initialize Context
        auto ctx_res = KNew<TestMmuContext>(pmm);
        R_ASSERT_TRUE(ctx_res.has_value());
        ctx_ = *ctx_res;
    }

    void TearDown_() override
    {
        if (ctx_) {
            KDelete(ctx_);
            ctx_ = nullptr;
        }

        auto &pmm = MemoryModule::Get().GetBitmapPmm();
        pmm.Free(pml4_phys_);
    }

    Mem::PPtr<Mem::Page> pml4_phys_;
    Mem::VPtr<void> pml4_virt_;
    hal::Tlb tlb_{};
    hal::Mmu mmu_{};
    TestMmuContext *ctx_{nullptr};

    // Default flags for testing
    PageFlags kDefaultFlags{
        .Present        = true,
        .Writable       = true,
        .UserAccessible = false,
        .WriteThrough   = false,
        .CacheDisable   = false,
        .Global         = true,
        .NoExecute      = false,
    };
};

// ------------------------------
// Map Tests
// ------------------------------

TEST_F(MmuTest, Map_GivenValidUnmappedPage_SucceedsAndTranslateReturnsPhys)
{
    // Given
    VPtr<void> vaddr = reinterpret_cast<VPtr<void>>(0x100000);
    PPtr<void> paddr = reinterpret_cast<PPtr<void>>(0x200000);
    auto root        = reinterpret_cast<Mem::PPtr<void>>(pml4_phys_);

    // When
    auto result = mmu_.Map(*ctx_, root, vaddr, paddr, kDefaultFlags);

    // Then
    EXPECT_TRUE(result.has_value());

    auto trans_res = mmu_.Translate(*ctx_, root, vaddr);
    EXPECT_TRUE(trans_res.has_value());
    EXPECT_EQ(paddr, trans_res.value());
}

TEST_F(MmuTest, Map_GivenAlreadyMappedPage_ReturnsError)
{
    // Given
    VPtr<void> vaddr  = reinterpret_cast<VPtr<void>>(0x100000);
    PPtr<void> paddr1 = reinterpret_cast<PPtr<void>>(0x200000);
    PPtr<void> paddr2 = reinterpret_cast<PPtr<void>>(0x300000);
    auto root         = reinterpret_cast<Mem::PPtr<void>>(pml4_phys_);

    EXPECT_TRUE(mmu_.Map(*ctx_, root, vaddr, paddr1, kDefaultFlags).has_value());

    // When
    auto result = mmu_.Map(*ctx_, root, vaddr, paddr2, kDefaultFlags);

    // Then
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(Mem::MemError::InvalidArgument, result.error());

    // Verify mapping hasn't changed
    auto trans_res = mmu_.Translate(*ctx_, root, vaddr);
    EXPECT_TRUE(trans_res.has_value());
    EXPECT_EQ(paddr1, trans_res.value());
}

TEST_F(MmuTest, Map_GivenUnalignedAddress_BehavesConsistency)
{
    // Given
    VPtr<void> vaddr_aligned   = reinterpret_cast<VPtr<void>>(0x100000);
    VPtr<void> vaddr_unaligned = reinterpret_cast<VPtr<void>>(0x100055);
    PPtr<void> paddr           = reinterpret_cast<PPtr<void>>(0x200000);
    auto root                  = reinterpret_cast<Mem::PPtr<void>>(pml4_phys_);

    // When: Mapping with unaligned address
    auto result = mmu_.Map(*ctx_, root, vaddr_unaligned, paddr, kDefaultFlags);

    // Then
    EXPECT_TRUE(result.has_value());

    // Check that the aligned address is now mapped
    auto trans_res = mmu_.Translate(*ctx_, root, vaddr_aligned);
    EXPECT_TRUE(trans_res.has_value());
    EXPECT_EQ(paddr, trans_res.value());
}

TEST_F(MmuTest, Map_GivenZeroAddress_Succeeds)
{
    // Given
    VPtr<void> vaddr = nullptr;
    PPtr<void> paddr = reinterpret_cast<PPtr<void>>(0x5000);
    auto root        = reinterpret_cast<Mem::PPtr<void>>(pml4_phys_);

    // When
    auto result = mmu_.Map(*ctx_, root, vaddr, paddr, kDefaultFlags);

    // Then
    EXPECT_TRUE(result.has_value());
    auto trans_res = mmu_.Translate(*ctx_, root, vaddr);
    EXPECT_TRUE(trans_res.has_value());
    EXPECT_EQ(paddr, trans_res.value());
}

// ------------------------------
// Unmap Tests
// ------------------------------

TEST_F(MmuTest, Unmap_GivenMappedPage_Succeeds)
{
    // Given
    VPtr<void> vaddr = reinterpret_cast<VPtr<void>>(0x400000);
    PPtr<void> paddr = reinterpret_cast<PPtr<void>>(0x800000);
    auto root        = reinterpret_cast<Mem::PPtr<void>>(pml4_phys_);
    mmu_.Map(*ctx_, root, vaddr, paddr, kDefaultFlags);

    // When
    mmu_.Unmap(*ctx_, root, vaddr);

    // Then
    // Verify it is no longer translated
    auto trans_res = mmu_.Translate(*ctx_, root, vaddr);
    EXPECT_FALSE(trans_res.has_value());
}

TEST_F(MmuTest, Unmap_GivenUnmappedPage_NoOp)
{
    // Given
    VPtr<void> vaddr = reinterpret_cast<VPtr<void>>(0x500000);  // Never mapped
    auto root        = reinterpret_cast<Mem::PPtr<void>>(pml4_phys_);

    // When
    mmu_.Unmap(*ctx_, root, vaddr);

    // Then: Should not crash and still be unmapped
    auto trans_res = mmu_.Translate(*ctx_, root, vaddr);
    EXPECT_FALSE(trans_res.has_value());
}

// ------------------------------
// Translate Tests
// ------------------------------

TEST_F(MmuTest, Translate_GivenMappedPage_ReturnsPhysicalAddress)
{
    // Given
    VPtr<void> vaddr = reinterpret_cast<VPtr<void>>(0x1000);
    PPtr<void> paddr = reinterpret_cast<PPtr<void>>(0x2000);
    auto root        = reinterpret_cast<Mem::PPtr<void>>(pml4_phys_);
    mmu_.Map(*ctx_, root, vaddr, paddr, kDefaultFlags);

    // When
    auto result = mmu_.Translate(*ctx_, root, vaddr);

    // Then
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(paddr, result.value());
}

TEST_F(MmuTest, Translate_GivenUnmappedPage_ReturnsError)
{
    // Given
    VPtr<void> vaddr = reinterpret_cast<VPtr<void>>(0xDEADBEEF);
    auto root        = reinterpret_cast<Mem::PPtr<void>>(pml4_phys_);

    // When
    auto result = mmu_.Translate(*ctx_, root, vaddr);

    // Then
    EXPECT_FALSE(result.has_value());
}

// ------------------------------
// Control Register Tests
// ------------------------------

TEST_F(MmuTest, SwitchRoot_GivenCurrentRoot_DoesNotCrash)
{
    // Given: The current kernel address space root
    auto &kernel_as              = MemoryModule::Get().GetKernelAddressSpace();
    Mem::PPtr<void> current_root = kernel_as.PageTableRoot();

    // When: We switch to the SAME root (safe to do)
    mmu_.SwitchRoot(current_root);

    // Then: Execution continues
    EXPECT_TRUE(true);
}
