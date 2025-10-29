#include <test_module/test.hpp>

#include <hal/constants.hpp>
#include <mem/types.hpp>
#include <mem/virt/addr_space.hpp>
#include <mem/virt/area.hpp>
#include <modules/memory.hpp>

class PageFaultHandlerTest : public TestGroupBase
{
};

using namespace Mem;

// ------------------------------
// Anonymous Memory Tests
// ------------------------------

TEST_F(PageFaultHandlerTest, AnonymousMemory_WhenWritingToUnmappedPage_ShouldHandleFault)
{
    // Given: A kernel address space with a new anonymous memory region
    auto &vmm       = MemoryModule::Get().GetVmm();
    auto &kernel_as = MemoryModule::Get().GetKernelAddressSpace();

    auto *vaddr = reinterpret_cast<Mem::VPtr<u64>>(0xABCD0000);
    Mem::VMemArea vma{
        .start                = vaddr,
        .size                 = hal::kPageSizeBytes,
        .flags                = {.readable = true, .writable = true, .executable = false},
        .type                 = VirtualMemAreaT::Anonymous,
        .direct_mapping_start = nullptr
    };

    auto add_res = vmm.AddArea(&kernel_as, vma);
    ASSERT_TRUE(add_res, "Failed to add anonymous memory area");

    // When: Writing to the unmapped memory, triggering a page fault
    const u64 test_value1 = 0xDEADBEEFCAFEBABE;
    *vaddr                = test_value1;

    // Then: The page fault is handled, and memory is accessed correctly
    EXPECT_EQ(test_value1, *vaddr);

    // Cleanup
    auto rm_res = vmm.RmArea(&kernel_as, vaddr);
    ASSERT_TRUE(rm_res, "Failed to remove anonymous memory area");
}

TEST_F(PageFaultHandlerTest, AnonymousMemory_WhenAccessingMultipleLocationsInSamePage_ShouldSucceed)
{
    // Given: A kernel address space with a new anonymous memory region
    auto &vmm       = MemoryModule::Get().GetVmm();
    auto &kernel_as = MemoryModule::Get().GetKernelAddressSpace();

    auto *vaddr = reinterpret_cast<Mem::VPtr<u64>>(0xABCD0000);
    Mem::VMemArea vma{
        .start                = vaddr,
        .size                 = hal::kPageSizeBytes,
        .flags                = {.readable = true, .writable = true, .executable = false},
        .type                 = VirtualMemAreaT::Anonymous,
        .direct_mapping_start = nullptr
    };

    auto add_res = vmm.AddArea(&kernel_as, vma);
    ASSERT_TRUE(add_res, "Failed to add anonymous memory area");

    // When: Writing to different locations in the same page
    const u64 test_value1 = 0xDEADBEEFCAFEBABE;
    *vaddr                = test_value1;

    auto *another_addr_in_page = vaddr + 10;
    const u64 test_value2      = 0x123456789ABCDEF0;
    *another_addr_in_page      = test_value2;

    // Then: Both values are stored and retrieved correctly
    EXPECT_EQ(test_value1, *vaddr);
    EXPECT_EQ(test_value2, *another_addr_in_page);

    // Cleanup
    auto rm_res = vmm.RmArea(&kernel_as, vaddr);
    ASSERT_TRUE(rm_res, "Failed to remove anonymous memory area");
}

// ------------------------------
// Direct Mapped Memory Tests
// ------------------------------

TEST_F(PageFaultHandlerTest, DirectMappedMemory_WhenReadingFromUnmappedPage_ShouldMapToPhysical)
{
    // Given: A physical page with a known value
    auto &pmm      = MemoryModule::Get().GetBitmapPmm();
    auto paddr_res = pmm.Alloc();
    ASSERT_TRUE(paddr_res, "Failed to allocate a physical page for direct mapping");

    Mem::PPtr<Page> paddr     = reinterpret_cast<Mem::PPtr<Page>>(*paddr_res);
    Mem::VPtr<u64> paddr_virt = reinterpret_cast<VPtr<u64>>(Mem::PhysToVirt(paddr));

    const u64 initial_value = 0xAAAAAAAAAAAAAAAA;
    *paddr_virt             = initial_value;
    EXPECT_EQ(initial_value, *paddr_virt);

    // Given: A new direct-mapped memory region pointing to the physical page
    auto &vmm       = MemoryModule::Get().GetVmm();
    auto &kernel_as = MemoryModule::Get().GetKernelAddressSpace();
    auto *vaddr     = reinterpret_cast<Mem::VPtr<u64>>(0xBCDE0000);

    Mem::VMemArea vma{
        .start                = vaddr,
        .size                 = hal::kPageSizeBytes,
        .flags                = {.readable = true, .writable = true, .executable = false},
        .type                 = Mem::VirtualMemAreaT::DirectMapping,
        .direct_mapping_start = paddr,
    };

    auto add_res = vmm.AddArea(&kernel_as, vma);
    ASSERT_TRUE(add_res, "Failed to add direct-mapped memory area");

    // When: Reading from the new virtual address, triggering a page fault
    u64 read_value = *vaddr;

    // Then: The value read matches the physical memory value
    EXPECT_EQ(initial_value, read_value);

    // Cleanup
    auto rm_res = vmm.RmArea(&kernel_as, vaddr);
    ASSERT_TRUE(rm_res, "Failed to remove direct-mapped memory area");
    pmm.Free(paddr);
}

TEST_F(PageFaultHandlerTest, DirectMappedMemory_WhenWritingToVirtualAddress_ShouldUpdatePhysical)
{
    // Given: A physical page with a known value
    auto &pmm      = MemoryModule::Get().GetBitmapPmm();
    auto paddr_res = pmm.Alloc();
    ASSERT_TRUE(paddr_res, "Failed to allocate a physical page for direct mapping");

    Mem::PPtr<Page> paddr     = reinterpret_cast<Mem::PPtr<Page>>(*paddr_res);
    Mem::VPtr<u64> paddr_virt = reinterpret_cast<VPtr<u64>>(Mem::PhysToVirt(paddr));

    const u64 initial_value = 0xAAAAAAAAAAAAAAAA;
    *paddr_virt             = initial_value;
    EXPECT_EQ(initial_value, *paddr_virt);

    // Given: A direct-mapped memory region pointing to the physical page
    auto &vmm       = MemoryModule::Get().GetVmm();
    auto &kernel_as = MemoryModule::Get().GetKernelAddressSpace();
    auto *vaddr     = reinterpret_cast<Mem::VPtr<u64>>(0xBCDE0000);

    Mem::VMemArea vma{
        .start                = vaddr,
        .size                 = hal::kPageSizeBytes,
        .flags                = {.readable = true, .writable = true, .executable = false},
        .type                 = Mem::VirtualMemAreaT::DirectMapping,
        .direct_mapping_start = paddr,
    };

    auto add_res = vmm.AddArea(&kernel_as, vma);
    ASSERT_TRUE(add_res, "Failed to add direct-mapped memory area");

    // When: Writing to the virtual address
    const u64 new_value = 0xBBBBBBBBBBBBBBBB;
    *vaddr              = new_value;

    // Then: The physical memory is updated and values match
    EXPECT_EQ(new_value, *paddr_virt);
    EXPECT_EQ(new_value, *vaddr);

    // Cleanup
    auto rm_res = vmm.RmArea(&kernel_as, vaddr);
    ASSERT_TRUE(rm_res, "Failed to remove direct-mapped memory area");
    pmm.Free(paddr);
}
