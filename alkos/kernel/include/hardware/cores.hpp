#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_

#include "hal/constants.hpp"
#include "hal/core.hpp"

#include <extensions/cstddef.hpp>
#include "mem/allocators.hpp"

namespace hardware
{

struct CoreConfig : hal::CoreConfig {
    u16 hwid;
    u16 lid;
    bool enabled;
};

class alignas(hal::kCacheLineSizeBytes) Core final : public hal::Core
{
    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    explicit Core(const CoreConfig &config);

    // ------------------------------
    // Class interaction
    // ------------------------------

    FORCE_INLINE_F void EnableCore()
    {
        ASSERT_FALSE(Core::IsEnabled());
        hal::Core::EnableCore();
        config_.enabled = true;
    }

    NODISCARD FORCE_INLINE_F u16 GetHwId() const { return config_.hwid; }

    NODISCARD FORCE_INLINE_F u16 GetLId() const { return config_.lid; }

    NODISCARD FORCE_INLINE_F bool IsEnabled() const { return config_.enabled; }

    NODISCARD FORCE_INLINE_F u32 GetInterruptNestingLevel() const
    {
        return interrupt_nesting_level_;
    }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    CoreConfig config_{};
    u32 interrupt_nesting_level_{0};
};

class CoresController final
{
    public:
    using CoreTable     = alloca::DynArray<Core>;
    using HwToCoreIdMap = alloca::DynArray<u16>;

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

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    CoreTable core_arr_{};
    HwToCoreIdMap hw_to_core_id_map_{};
};

}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_
