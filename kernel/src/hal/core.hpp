#ifndef KERNEL_SRC_HAL_CORE_HPP_
#define KERNEL_SRC_HAL_CORE_HPP_

#include <hal/impl/core.hpp>

namespace hal
{
using arch::Core;
using arch::CoreConfig;
using arch::CoreController;
using arch::CoreLocal;

NODISCARD WRAP_CALL u32 GetCurrentCoreId() { return arch::GetCurrentCoreId(); }
WRAP_CALL void SetCoreLocalData(void *data) { arch::SetCoreLocalData(data); }
template <typename T, size_t kOffset>
NODISCARD FAST_CALL T GetCoreLocalField()
{
    return arch::GetCoreLocalField<T, kOffset>();
}
template <typename T, size_t Offset>
FAST_CALL void SetCoreLocalField(T value)
{
    arch::SetCoreLocalField<T, Offset>(value);
}
}  // namespace hal

#endif  // KERNEL_SRC_HAL_CORE_HPP_
