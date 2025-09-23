#ifndef ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_VMM_IPML_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_VMM_IPML_HPP_

#include <extensions/template_lib.hpp>
#include <mem/phys_ptr.hpp>
#include <mem/pmm_abi.hpp>
#include <mem/virt_ptr.hpp>
#include <mem/vmm_abi.hpp>

#include "hal/mem/vmm/impl_config.hpp"
#include "mem/page_map.hpp"

namespace arch
{

template <class PmmT>
    requires std::is_base_of_v<PhysicalMemoryManagerABI, PmmT>
class VirtualMemoryManagerImpl : public VirtualMemoryManagerABI,
                                 public template_lib::DelayedInitMixin<
                                     VirtualMemoryManagerImpl<PmmT>, VirtualMemoryManagerImplConfig>
{
    public:
    static constexpr u64 kNoFlags = 0;
    struct MapOnePageTag {
    };

    using ConfigT = VirtualMemoryManagerImplConfig;
    using BaseDelayedInitMixin =
        template_lib::DelayedInitMixin<VirtualMemoryManagerImpl<PmmT>, ConfigT>;

    VirtualMemoryManagerImpl(PmmT& pmm, const ConfigT& config)
        : BaseDelayedInitMixin(config), pmm_(pmm)
    {
    }

    //==============================================================================
    // ABI : VirtualMemoryManagerABI
    //==============================================================================

    void Alloc(const VirtualPtr<byte> vaddr, const u64 size, const u64 flags = kNoFlags);

    template <PageSizeTag kPageSizeTag = PageSizeTag::k4Kb>
    void Map(
        const VirtualPtr<byte> vaddr, const PhysicalPtr<byte> paddr, const u64 size,
        const u64 flags = kNoFlags
    );

    template <PageSizeTag kPageSizeTag = PageSizeTag::k4Kb>
    void Map(
        MapOnePageTag, const VirtualPtr<byte> vaddr, const PhysicalPtr<byte> paddr,
        const u64 flags = kNoFlags
    );

    //==============================================================================
    // ABI : DelayedInitMixin
    //==============================================================================

    void InitImpl();

    //==============================================================================
    // Observers
    //==============================================================================

    PhysicalPtr<PageMapTable<4>> GetPml4Table() { return pm_table_4_; }

    //==============================================================================
    // Private Methods
    //==============================================================================

    private:
    template <size_t kLevel>
    FORCE_INLINE_F void EnsurePMEntryPresent(PageMapEntry<kLevel>& pme);

    template <size_t kLevel>
    FORCE_INLINE_F PhysicalPtr<PageMapTable<kLevel - 1>> AllocNextLevelTable();

    template <size_t kLevel>
    FORCE_INLINE_F u64 PmeIdx(u64 addr);

    //==============================================================================
    // Private Fields
    //==============================================================================

    PmmT& pmm_;
    PhysicalPtr<PageMapTable<4>> pm_table_4_;
};

}  // namespace arch

#include "hal/mem/vmm/impl.tpp"

#endif  // ALKOS_KERNEL_ARCH_X86_64_SRC_HAL_MEM_VMM_IPML_HPP_
