#include "hardware/cores.hpp"

#include <assert.h>
#include <extensions/debug.hpp>

static constexpr size_t kMaxAllowedHwId = hal::kMaxCores * 4;

void hardware::CoresController::AllocateTables(const size_t num_cores, const size_t max_hw_id)
{
    ASSERT_TRUE(core_arr_.empty());
    ASSERT_TRUE(hw_to_core_id_map_.empty());
    ASSERT_NOT_ZERO(num_cores);
    ASSERT_LE(max_hw_id, kMaxAllowedHwId);

    const auto result1 = core_arr_.Reallocate(num_cores);
    R_ASSERT_TRUE(static_cast<bool>(result1));

    const auto result2 = hw_to_core_id_map_.Reallocate(max_hw_id + 1);
    R_ASSERT_TRUE(static_cast<bool>(result2));
}

hardware::Core &hardware::CoresController::AllocateCore(const CoreConfig &config)
{
    ASSERT_FALSE(core_arr_.empty());
    ASSERT_FALSE(hw_to_core_id_map_.empty());

    core_arr_.AllocEntry(config.lid, config);
    hw_to_core_id_map_[config.hwid] = config.lid;
    return core_arr_[config.lid];
}

hardware::Core::Core(const CoreConfig &config) : config_(config)
{
    TRACE_INFO("Core with Hardware ID: %hu, Logical ID: %hu created", GetHwId(), GetLId());
}

void hardware::CoresController::BootUpAllCores()
{
    TRACE_INFO("Booting up all cores...");

    for (auto &core : core_arr_) {
        core.EnableCore();
    }

    TRACE_INFO("Finished booting up all cores...");
}
