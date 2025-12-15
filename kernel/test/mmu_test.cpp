#include <string.h>
#include "test_module/test.hpp"

#include "hal/mmu.hpp"
#include "mem/page.hpp"
#include "mem/virt/addr_space.hpp"
#include "modules/memory.hpp"

using namespace Mem;
using namespace hal;

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

        // Initialize AddressSpace with this root
        auto test_as_or_err = KNew<AddressSpace>(pml4_phys_);
        R_ASSERT_TRUE(test_as_or_err);
        test_as_ = test_as_or_err.value();
    }

    void TearDown_() override
    {
        if (test_as_ != nullptr) {
            KDelete(test_as_);
            test_as_ = nullptr;
        }

        // NOTE: There is no cleanup of the PML pages here. But since each test runs
        // in a separate AlkOS instace, this is ok

        auto &pmm = MemoryModule::Get().GetBitmapPmm();
        pmm.Free(pml4_phys_);
    }

    Mem::PPtr<Mem::Page> pml4_phys_;
    Mem::VPtr<void> pml4_virt_;
    Mem::VPtr<Mem::AddressSpace> test_as_{nullptr};
    hal::Mmu mmu_{};

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

    // When
    auto result = mmu_.Map(test_as_, vaddr, paddr, kDefaultFlags);

    // Then
    EXPECT_TRUE(result.has_value());

    auto trans_res = mmu_.Translate(test_as_, vaddr);
    EXPECT_TRUE(trans_res.has_value());
    EXPECT_EQ(paddr, trans_res.value());
}

TEST_F(MmuTest, Map_GivenAlreadyMappedPage_ReturnsError)
{
    // Given
    VPtr<void> vaddr  = reinterpret_cast<VPtr<void>>(0x100000);
    PPtr<void> paddr1 = reinterpret_cast<PPtr<void>>(0x200000);
    PPtr<void> paddr2 = reinterpret_cast<PPtr<void>>(0x300000);

    EXPECT_TRUE(mmu_.Map(test_as_, vaddr, paddr1, kDefaultFlags).has_value());

    // When
    auto result = mmu_.Map(test_as_, vaddr, paddr2, kDefaultFlags);

    // Then
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(Mem::MemError::InvalidArgument, result.error());

    // Verify mapping hasn't changed
    auto trans_res = mmu_.Translate(test_as_, vaddr);
    EXPECT_TRUE(trans_res.has_value());
    EXPECT_EQ(paddr1, trans_res.value());
}

TEST_F(MmuTest, Map_GivenUnalignedAddress_BehavesConsistency)
{
    // Given
    VPtr<void> vaddr_aligned   = reinterpret_cast<VPtr<void>>(0x100000);
    VPtr<void> vaddr_unaligned = reinterpret_cast<VPtr<void>>(0x100055);
    PPtr<void> paddr           = reinterpret_cast<PPtr<void>>(0x200000);

    // When: Mapping with unaligned address
    auto result = mmu_.Map(test_as_, vaddr_unaligned, paddr, kDefaultFlags);

    // Then
    EXPECT_TRUE(result.has_value());

    // Check that the aligned address is now mapped
    auto trans_res = mmu_.Translate(test_as_, vaddr_aligned);
    EXPECT_TRUE(trans_res.has_value());
    EXPECT_EQ(paddr, trans_res.value());
}

TEST_F(MmuTest, Map_GivenZeroAddress_Succeeds)
{
    // Given
    VPtr<void> vaddr = nullptr;
    PPtr<void> paddr = reinterpret_cast<PPtr<void>>(0x5000);

    // When
    auto result = mmu_.Map(test_as_, vaddr, paddr, kDefaultFlags);

    // Then
    EXPECT_TRUE(result.has_value());
    auto trans_res = mmu_.Translate(test_as_, vaddr);
    EXPECT_TRUE(trans_res.has_value());
    EXPECT_EQ(paddr, trans_res.value());
}

// ------------------------------
// UnMap Tests
// ------------------------------

TEST_F(MmuTest, UnMap_GivenMappedPage_Succeeds)
{
    // Given
    VPtr<void> vaddr = reinterpret_cast<VPtr<void>>(0x400000);
    PPtr<void> paddr = reinterpret_cast<PPtr<void>>(0x800000);
    mmu_.Map(test_as_, vaddr, paddr, kDefaultFlags);

    // When
    auto result = mmu_.UnMap(test_as_, vaddr);

    // Then
    EXPECT_TRUE(result.has_value());

    // Verify it is no longer translated
    auto trans_res = mmu_.Translate(test_as_, vaddr);
    EXPECT_FALSE(trans_res.has_value());
}

TEST_F(MmuTest, UnMap_GivenUnmappedPage_ReturnsError)
{
    // Given
    VPtr<void> vaddr = reinterpret_cast<VPtr<void>>(0x500000);  // Never mapped

    // When
    auto result = mmu_.UnMap(test_as_, vaddr);

    // Then: Should have memory error
    EXPECT_TRUE(result.error());
}

// ------------------------------
// Translate Tests
// ------------------------------

TEST_F(MmuTest, Translate_GivenMappedPage_ReturnsPhysicalAddress)
{
    // Given
    VPtr<void> vaddr = reinterpret_cast<VPtr<void>>(0x1000);
    PPtr<void> paddr = reinterpret_cast<PPtr<void>>(0x2000);
    mmu_.Map(test_as_, vaddr, paddr, kDefaultFlags);

    // When
    auto result = mmu_.Translate(test_as_, vaddr);

    // Then
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(paddr, result.value());
}

TEST_F(MmuTest, Translate_GivenUnmappedPage_ReturnsError)
{
    // Given
    VPtr<void> vaddr = reinterpret_cast<VPtr<void>>(0xDEADBEEF);

    // When
    auto result = mmu_.Translate(test_as_, vaddr);

    // Then
    EXPECT_FALSE(result.has_value());
}

// ------------------------------
// Control Register Tests
// ------------------------------

TEST_F(MmuTest, SwitchRootPageMapTable_GivenCurrentRoot_DoesNotCrash)
{
    // Given: The current kernel address space root
    auto &kernel_as              = MemoryModule::Get().GetKernelAddressSpace();
    Mem::PPtr<void> current_root = kernel_as.PageTableRoot();

    // When: We switch to the SAME root (safe to do)
    mmu_.SwitchRootPageMapTable(current_root);

    // Then: Execution continues
    EXPECT_TRUE(true);
}

TEST_F(MmuTest, DestroyRootPageMapTable_GivenAnyPtr_NoOp)
{
    // Given
    Mem::PPtr<void> dummy_root = reinterpret_cast<Mem::PPtr<void>>(0x1000);

    // When
    mmu_.DestroyRootPageMapTable(dummy_root);

    // Then: Nothing happens (no crash)
    EXPECT_TRUE(true);
}
