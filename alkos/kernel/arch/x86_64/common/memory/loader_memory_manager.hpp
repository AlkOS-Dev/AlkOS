#ifndef ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_LOADER_MEMORY_MANAGER_LOADER_MEMORY_MANAGER_HPP_
#define ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_LOADER_MEMORY_MANAGER_LOADER_MEMORY_MANAGER_HPP_

#include <extensions/types.hpp>
#include "defines.hpp"
#include "multiboot2/extensions.hpp"
#include "multiboot2/multiboot2.h"
#include "page_tables_layout.hpp"

namespace memory
{

/**
 * @brief Structure representing a free memory region.
 *
 * Contains the physical start address and the region's length.
 */
struct FreeMemoryRegion_t {
    u64 addr;
    u64 length;
} PACK;

/**
 * @brief Concept for a callback function to process a FreeMemoryRegion_t.
 *
 * The callback must be callable with a reference to a FreeMemoryRegion_t.
 */
template <typename Callback>
concept FreeMemoryRegionCallback =
    requires(Callback cb, FreeMemoryRegion_t& region) { cb(region); };

/**
 * @brief Concept for a provider that can provide free memory regions.
 *
 * The provider must be callable with a callback that accepts a FreeMemoryRegion_t reference.
 * The callback should not return any value.
 *
 * The provider is expected to iterate over free memory regions and invoke the callback
 */
template <typename Provider>
concept FreeRegionProvider = requires(Provider provider) {
    {
        provider([](FreeMemoryRegion_t&) {
        })
    } -> std::convertible_to<void>;
};

/**
 * @brief LoaderMemoryManager manages the mapping of virtual to physical memory.
 *
 * It handles the creation and storage of paging tables (PML) and provides
 * functions to map virtual ranges, walk free memory regions, and dump debug
 */
class LoaderMemoryManager
{
    private:
    //------------------------------------------------------------------------------
    // Constants
    //------------------------------------------------------------------------------

    static constexpr u32 kMaxPmlTablesToStore = 100;  ///< Maximum number of PML tables to store.
    static constexpr u32 kMaxProtectedMemoryRegions =
        1e2;  ///< Maximum number of protected memory regions.

    /**
     * @brief Maximum number of memory map entries the loader can handle.
     *
     * If the number of entries exceeds this value, the loader will panic.
     */
    static constexpr u32 kMaxMemoryMapEntries = 1e3;

    //------------------------------------------------------------------------------//
    // Internal Types
    //------------------------------------------------------------------------------//

    private:
    static constexpr u32 kPml4Index = 0;  ///< Must be 0

    public:

    public:
    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//

    /**
     * @brief Constructs the LoaderMemoryManager.
     *
     * Initializes the internal PML tables and clears the free memory regions.
     */
    LoaderMemoryManager();

    //------------------------------------------------------------------------------//
    // Public Methods
    //------------------------------------------------------------------------------//

    /**
     * @brief Get the PML4 table.
     *
     * @return PML4_t* The PML4 table.
     */
    PML4_t* GetPml4Table();

    /**
     * @brief Maps a single virtual address to a physical address.
     *
     * This is a template function where the page size can be specified.
     *
     * @tparam page_size Page size to be used in mapping.
     * @param virtual_address Virtual address to map.
     * @param physical_address Physical address to map to.
     * @param flags Mapping flags.
     */
    template <PageSize page_size>
    void MapVirtualMemoryToPhysical(u64 virtual_address, u64 physical_address, u64 flags);

    /**
     * @brief Enum to specify the direction of the walk.
     */
    enum class WalkDirection { Ascending, Descending };

    template <WalkDirection direction = WalkDirection::Descending>
    void MapVirtualRangeUsingInternalMemoryMap(u64 virtual_address, u64 size_bytes, u64 flags = 0);

    template <WalkDirection direction = WalkDirection::Descending>
    void MapVirtualRangeUsingExternalMemoryMap(
        multiboot::tag_mmap_t* mmap_tag, u64 virtual_address, u64 size_bytes, u64 flags = 0
    );

    [[nodiscard]] u32 GetNumPmlTablesStored() const { return num_pml_tables_stored_; }

    /**
     * @brief Iterates over each free memory region.
     *
     * Applies the given callback to each free memory region.
     *
     * @tparam Callback Type of the callback satisfying FreeMemoryRegionCallback.
     * @param callback Function to invoke for each free memory region.
     */
    template <WalkDirection direction, FreeMemoryRegionCallback Callback>
    void WalkFreeMemoryRegions(Callback callback);

    void AddFreeRegion(u64 start_addr, u64 end_addr);
    void ReserveArea(u64 start_addr, u64 end_addr);

    [[nodiscard]] u64 GetAvailableMemoryBytes() const { return available_memory_bytes_; }

    void DumpMemoryMap();
    void DumpPmlTables();

    //------------------------------------------------------------------------------//
    // Public Fields
    //------------------------------------------------------------------------------//

    private:
    //------------------------------------------------------------------------------//
    // Private Methods
    //------------------------------------------------------------------------------//

    template <FreeRegionProvider Provider, WalkDirection direction = WalkDirection::Descending>
    void MapVirtualRangeUsingFreeRegionProvider(
        Provider provider, u64 virtual_address, u64 size_bytes, u64 flags = 0
    );

    template <WalkDirection direction = WalkDirection::Descending>
    void UsePartOfFreeMemoryRegion(FreeMemoryRegion_t& region, u64 size_bytes);

    //------------------------------------------------------------------------------//
    // Private Fields
    //------------------------------------------------------------------------------//

    // NOTE: The alignment is necessary here to make this class work in both 32-bit and 64-bit

    PMLTable_t buffer_[kMaxPmlTablesToStore]{};  ///< A buffer to store PML tables as new physical
                                                 ///< memory is allocated using the memory manager
    alignas(64) u64 num_pml_tables_stored_{};    ///< The number of PML tables stored in the buffer

    /// "Lower" memory is frequently required for drivers / special purposes, therefore
    /// we sort the memory map entries in descending order and allocate the upper memory first.
    alignas(64) FreeMemoryRegion_t descending_sorted_mmap_entries[kMaxMemoryMapEntries]{};
    alignas(64) u64 used_free_memory_regions_ = 0;
    alignas(64) u64 num_free_memory_regions_  = 0;

    alignas(64) u64 available_memory_bytes_ = 0;

    //------------------------------------------------------------------------------//
    // Helper Functions
    //------------------------------------------------------------------------------//
};

}  // namespace memory
#include "loader_memory_manager.tpp"

#endif  // ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_LOADER_MEMORY_MANAGER_LOADER_MEMORY_MANAGER_HPP_
