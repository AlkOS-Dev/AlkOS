#ifndef ALKOS_KERNEL_ARCH_X86_64_KERNEL_MEMORY_MANAGEMENT_PHYSICAL_MEMORY_MANAGER_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_KERNEL_MEMORY_MANAGEMENT_PHYSICAL_MEMORY_MANAGER_HPP_

#include <multiboot2/multiboot2.h>
#include <stddef.h>
#include <extensions/template_lib.hpp>
#include <extensions/types.hpp>

namespace memory
{

class PhysicalMemoryManager : template_lib::StaticSingletonHelper
{
    private:
    //------------------------------------------------------------------------------//
    // Internal Types
    //------------------------------------------------------------------------------//

    public:
    static constexpr u64 kPageSize = 1 << 12;

    struct PageBufferInfo_t {
        u64 start_addr;
        u64 size_bytes;
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
    void PopulatePageBuffer(Multiboot::TagMmap* mmap);

    uintptr_t Allocate();
    void Free(uintptr_t page_address_physical);

    [[nodiscard]] u64 GetNumFreePages() const { return num_pages_on_stack_; }

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
    uintptr_t* page_buffer_ = nullptr;
    u64 num_pages_on_stack_ = 0;

    //------------------------------------------------------------------------------//
    // Helpers
    //------------------------------------------------------------------------------//
};

}  // namespace memory

using PhysicalMemoryManager = template_lib::StaticSingleton<memory::PhysicalMemoryManager>;

#endif  // ALKOS_KERNEL_ARCH_X86_64_KERNEL_MEMORY_MANAGEMENT_PHYSICAL_MEMORY_MANAGER_HPP_
