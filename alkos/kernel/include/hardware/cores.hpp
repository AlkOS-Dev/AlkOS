#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_

#include "hal/constants.hpp"
#include "hal/core.hpp"

#include <extensions/cstddef.hpp>

namespace hardware
{

struct alignas(hal::kCacheLineSizeBytes) CoreLocal {
    u8 nested_interrupts{};
    u16 lid{};
};

FAST_CALL CoreLocal &GetCoreLocalData()
{
    return *static_cast<CoreLocal *>(hal::GetCoreLocalData());
}

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

}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_
