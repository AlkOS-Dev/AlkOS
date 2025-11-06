#ifndef ALKOS_KERNEL_INCLUDE_HAL_CORE_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_CORE_HPP_

#include <hal/impl/core.hpp>

namespace hal
{
using arch::Core;
using arch::CoreConfig;

NODISCARD WRAP_CALL u32 GetCurrentCoreId() { return arch::GetCurrentCoreId(); }
WRAP_CALL void SetCoreLocalData(void *data) { arch::SetCoreLocalData(data); }
NODISCARD WRAP_CALL void *GetCoreLocalData() { return arch::GetCoreLocalData(); }
}  // namespace hal

#endif  // ALKOS_KERNEL_INCLUDE_HAL_CORE_HPP_
