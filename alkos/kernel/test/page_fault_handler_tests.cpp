#include <test_module/test.hpp>

#include <hal/constants.hpp>
#include <mem/types.hpp>
#include <mem/virt/addr_space.hpp>
#include <mem/virt/area.hpp>
#include <modules/memory.hpp>

class PageFaultTest : public TestGroupBase
{
};

using namespace Mem;

TEST_F(PageFaultTest, AnonymousMemoryAccess)
{
    // SETUP: Get the kernel address space and add a new anonymous memory region.
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
    R_ASSERT_TRUE(add_res, "Failed to add anonymous memory area");

    // ACCESS: Write to the unmapped memory, triggering a page fault.
    const u64 test_value1 = 0xDEADBEEFCAFEBABE;
    *vaddr                = test_value1;

    // THEN: The page fault is handled, and memory is accessed correctly.
    R_ASSERT_EQ(test_value1, *vaddr);

    // Verify another part of the same page.
    auto *another_addr_in_page = vaddr + 10;
    const u64 test_value2      = 0x123456789ABCDEF0;
    *another_addr_in_page      = test_value2;
    R_ASSERT_EQ(test_value2, *another_addr_in_page);

    // Cleanup
    auto rm_res = vmm.RmArea(&kernel_as, vaddr);
    R_ASSERT_TRUE(rm_res, "Failed to remove anonymous memory area");
}

TEST_F(PageFaultTest, DirectMappedMemoryAccess)
{
    // SETUP:
    // Allocate a physical page and write a known value to it.
    auto &pmm      = MemoryModule::Get().GetBitmapPmm();
    auto paddr_res = pmm.Alloc();
    R_ASSERT_TRUE(paddr_res, "Failed to allocate a physical page for direct mapping");
    Mem::PPtr<Page> paddr     = reinterpret_cast<Mem::PPtr<Page>>(*paddr_res);
    Mem::VPtr<u64> paddr_virt = reinterpret_cast<VPtr<u64>>(Mem::PhysToVirt(paddr));

    const u64 initial_value = 0xAAAAAAAAAAAAAAAA;
    *paddr_virt             = initial_value;
    R_ASSERT_EQ(initial_value, *paddr_virt);

    // Add a new direct-mapped memory region pointing to our physical page.
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
    R_ASSERT_TRUE(add_res, "Failed to add direct-mapped memory area");

    // ACCESS: Read from the new virtual address, triggering a page fault.
    u64 read_value = *vaddr;

    // THEN:
    // The value read from the virtual address matches the value at the physical address.
    R_ASSERT_EQ(initial_value, read_value);

    // Writing to the virtual address updates the physical memory.
    const u64 new_value = 0xBBBBBBBBBBBBBBBB;
    *vaddr              = new_value;
    R_ASSERT_EQ(new_value, *paddr_virt);

    // The value read back from the virtual address is the new value.
    R_ASSERT_EQ(new_value, *vaddr);

    // Cleanup
    auto rm_res = vmm.RmArea(&kernel_as, vaddr);
    R_ASSERT_TRUE(rm_res, "Failed to remove direct-mapped memory area");
    pmm.Free(paddr);
}
