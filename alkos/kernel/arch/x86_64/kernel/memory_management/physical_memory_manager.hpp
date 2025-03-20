#ifndef ALKOS_ALKOS_KERNEL_ARCH_X86_64_KERNEL_MEMORY_MANAGEMENT_PHYSICAL_HPP_
#define ALKOS_ALKOS_KERNEL_ARCH_X86_64_KERNEL_MEMORY_MANAGEMENT_PHYSICAL_HPP_

#include <multiboot2/multiboot2.h>
#include <stddef.h>
#include <multiboot2/extensions.hpp>

TODO_WHEN_DEBUGGING_FRAMEWORK

namespace memory
{

class PhysicalMemoryManager : TemplateLib::StaticSingletonHelper
{
    private:
    //------------------------------------------------------------------------------//
    // Internal Types
    //------------------------------------------------------------------------------//

    public:
    static constexpr u64 kPageSize = 1 << 12;

    struct PageBufferInfo_t {
        u64 size_bytes;
        u64 start_addr;
    };

    public:
    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//

    protected:
    explicit PhysicalMemoryManager() = default;
    explicit PhysicalMemoryManager(PageBufferInfo_t page_buffer_info);

    public:
    //------------------------------------------------------------------------------//
    // Public Methods
    //------------------------------------------------------------------------------//

    void SetPageBuffer(PageBufferInfo_t page_buffer_info);
    void PopulatePageBuffer(multiboot::tag_mmap_t* mmap);

    uintptr_t Allocate();
    void Free(uintptr_t page_address_physical);

    void DumpPagebuffer();

    //------------------------------------------------------------------------------//
    // Public Fields
    //------------------------------------------------------------------------------//

    private:
    //------------------------------------------------------------------------------//
    // Private Methods
    //------------------------------------------------------------------------------//

    //------------------------------------------------------------------------------//
    // Private Fields
    //------------------------------------------------------------------------------//

    PageBufferInfo_t page_buffer_info_;
    u64* page_buffer_       = nullptr;
    u64 num_pages_on_stack_ = 0;

    //------------------------------------------------------------------------------//
    // Helpers
    //------------------------------------------------------------------------------//
};

}  // namespace memory

using PhysicalMemoryManager = TemplateLib::StaticSingleton<memory::PhysicalMemoryManager>;

#endif  // ALKOS_ALKOS_KERNEL_ARCH_X86_64_KERNEL_MEMORY_MANAGEMENT_PHYSICAL_HPP_
