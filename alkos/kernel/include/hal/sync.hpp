#ifndef ALKOS_KERNEL_INCLUDE_HAL_SYNC_HPP_
#define ALKOS_KERNEL_INCLUDE_HAL_SYNC_HPP_

#include <hal/impl/sync.hpp>

namespace hal
{
WRAP_CALL void FullMemFence() { arch::FullMemFence(); }

WRAP_CALL void LoadMemFence() { arch::LoadMemFence(); }

WRAP_CALL void SaveMemFence() { arch::SaveMemFence(); }
}  // namespace hal

#endif  // ALKOS_KERNEL_INCLUDE_HAL_SYNC_HPP_
