#ifndef ALKOS_ALKOS_KERNEL_ARCH_X86_64_KERNEL_MEMORY_MANAGEMENT_VIRTUAL_MEMORY_MANAGER_HPP_
#define ALKOS_ALKOS_KERNEL_ARCH_X86_64_KERNEL_MEMORY_MANAGEMENT_VIRTUAL_MEMORY_MANAGER_HPP_

#include <extensions/concepts.hpp>
#include <extensions/template_lib.hpp>
#include <memory/page_table_helpers.hpp>
#include <memory/page_tables_layout.hpp>

namespace memory
{

template <typename Entry>
concept EntryTypeConcept = requires(Entry entry) {
    { entry.present } -> std::convertible_to<bool>;
    { entry.writable } -> std::convertible_to<bool>;
    { entry.user_access } -> std::convertible_to<bool>;
    { entry.write_through } -> std::convertible_to<bool>;
    { entry.cache_disabled } -> std::convertible_to<bool>;
    { entry.accessed } -> std::convertible_to<bool>;
    { entry.dirty } -> std::convertible_to<bool>;
    { entry.page_size } -> std::convertible_to<bool>;
    { entry.global } -> std::convertible_to<bool>;
    { entry.frame } -> std::convertible_to<u64>;
};

template <typename TableAllocator>
concept TableAllocatorConcept = requires(TableAllocator allocator) {
    { allocator.Allocate() } -> std::convertible_to<uintptr_t>;
};

class VirtualMemoryManager : TemplateLib::StaticSingletonHelper
{
    public:
    //------------------------------------------------------------------------------//
    // Internal Types
    //------------------------------------------------------------------------------//
    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//
    //------------------------------------------------------------------------------//
    // Public Methods
    //------------------------------------------------------------------------------//

    void Allocate(u64 virtual_address, u64 flags);
    void Free(u64 virtual_address);

    template <u8 level_end>
    PageEntryT<level_end>& WalkPageTables(u64 virtual_address);

    PML4_t& GetPml4Table();

    //------------------------------------------------------------------------------//
    // Public Fields
    //------------------------------------------------------------------------------//

    private:
    //------------------------------------------------------------------------------//
    // Private Methods
    //------------------------------------------------------------------------------//

    template <u8 level>
    void EnsureEntryPresent(PageEntryT<level>& entry, TableAllocatorConcept auto& allocator);

    template <u8 level>
    PageTableT<level - 1>& GetChildPageDirectory(PageEntryT<level>& entry);

    template <u8 level>
    [[nodiscard]] u16 GetPmlIndex(u64 virtual_address) const;

    template <u8 level>
    [[nodiscard]] constexpr u8 GetIndexShift() const;

    template <PageSize page_size>
    [[nodiscard]] constexpr u8 GetPageAddressShift() const;

    template <u8 level>
    constexpr void AssertDescendable();

    template <u8 level>
    constexpr void AssertValidLevel();

    //------------------------------------------------------------------------------//
    // Private Fields
    //------------------------------------------------------------------------------//

    PML4_t pml4_{};

    //------------------------------------------------------------------------------//
    // Helpers
    //------------------------------------------------------------------------------//
};

}  // namespace memory

#include "virtual_memory_manager.tpp"

using VirtualMemoryManager = TemplateLib::StaticSingleton<memory::VirtualMemoryManager>;

#endif  // ALKOS_ALKOS_KERNEL_ARCH_X86_64_KERNEL_MEMORY_MANAGEMENT_VIRTUAL_MEMORY_MANAGER_HPP_
