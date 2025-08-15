#include "physical_memory_manager.hpp"
#include <extensions/debug.hpp>

#include <extensions/internal/intervals.hpp>
#include "multiboot2/memory_map.hpp"
#include "multiboot2/multiboot_info.hpp"

namespace memory
{
PhysicalMemoryManager::PhysicalMemoryManager(
    PhysicalMemoryManager::PageBufferInfo_t page_buffer_info
)
{
    SetPageBuffer(page_buffer_info);
}
void PhysicalMemoryManager::SetPageBuffer(PhysicalMemoryManager::PageBufferInfo_t page_buffer_info)
{
    page_buffer_info_   = page_buffer_info;
    page_buffer_        = reinterpret_cast<u64*>(page_buffer_info_.start_addr);
    num_pages_on_stack_ = 0;
}

void PhysicalMemoryManager::PopulatePageBuffer(Multiboot::TagMmap* mmap)
{
    R_ASSERT_NOT_NULL(page_buffer_);
    R_ASSERT_GT(page_buffer_info_.start_addr, 0ull);

    Multiboot::MemoryMap memory_map(mmap);
    memory_map.WalkEntries([&](Multiboot::MmapEntry& entry) {
        if (entry.type != Multiboot::MmapEntry::kMemoryAvailable) {
            return;
        }

        for (u64 page_addr = AlignUp(entry.addr, kPageSize); page_addr < entry.addr + entry.len;
             page_addr += kPageSize) {
            R_ASSERT_GE(page_buffer_info_.size_bytes / sizeof(u64), num_pages_on_stack_ + 1);
            page_buffer_[num_pages_on_stack_++] = page_addr;
        }
    });
}

uintptr_t PhysicalMemoryManager::Allocate()
{
    R_ASSERT_GT(num_pages_on_stack_, 0);
    return page_buffer_[--num_pages_on_stack_];
}
void PhysicalMemoryManager::Free(uintptr_t page_address_physical)
{
    ASSERT_LT(num_pages_on_stack_, ~0ULL);
    page_buffer_[num_pages_on_stack_++] = page_address_physical;
}
void PhysicalMemoryManager::DumpPagebuffer()
{
    TRACE_INFO("PhysicalMemoryManager::DumpPageBuffer()");
    TRACE_INFO("STACK TOP");
    for (u64 i = num_pages_on_stack_; i > 0; i--) {
        TRACE_INFO("%lld: %llX", i, page_buffer_[i - 1]);
    }
    TRACE_INFO("STACK BOTTOM");
}
}  // namespace memory
