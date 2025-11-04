#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_CORE_CONTROLLER_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_CORE_CONTROLLER_HPP_

#include "hardware/core_local.hpp"
#include "hardware/cores.hpp"
#include "mem/allocators.hpp"

namespace hardware
{

class CoresController final
{
    public:
    using CoreTable      = alloca::DynArray<Core>;
    using CoreLocalTable = alloca::DynArray<CoreLocal>;
    using HwToCoreIdMap  = alloca::DynArray<u16>;

    // ------------------------------
    // Class creation
    // ------------------------------

    CoresController() = default;

    ~CoresController() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void BootUpAllCores();

    void AllocateTables(size_t num_cores, size_t max_hw_id);

    Core &AllocateCore(const CoreConfig &config);

    NODISCARD FORCE_INLINE_F bool AreCoresKnown() const { return !core_arr_.empty(); }

    NODISCARD FORCE_INLINE_F u16 MapHwToLogical(const u16 hwid) const
    {
        return hw_to_core_id_map_[hwid];
    }

    NODISCARD FORCE_INLINE_F Core &GetCoreByLid(const u16 lid) { return core_arr_[lid]; }

    NODISCARD FORCE_INLINE_F Core &GetCoreByHw(const u16 hwid)
    {
        return core_arr_[MapHwToLogical(hwid)];
    }

    NODISCARD FORCE_INLINE_F Core &GetCurrentCore()
    {
        const u32 hwid = hal::GetCurrentCoreId();
        return GetCoreByHw(static_cast<u16>(hwid));
    }

    FORCE_INLINE_F void SetupCoreLocalData()
    {
        ASSERT_FALSE(core_arr_.empty());
        hal::SetCoreLocalData(&core_local_table_[MapHwToLogical(hal::GetCurrentCoreId())]);
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    CoreTable core_arr_{};
    HwToCoreIdMap hw_to_core_id_map_{};
    CoreLocalTable core_local_table_{};
};

}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_CORE_CONTROLLER_HPP_
