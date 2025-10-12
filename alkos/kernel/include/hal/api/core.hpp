#ifndef ALKOS_KERNEL_HAL_CORE_HPP_
#define ALKOS_KERNEL_HAL_CORE_HPP_

#include <extensions/defines.hpp>

namespace arch
{

class Core;

struct CoreAPI {
    /* Should perform full initialisation of single core */
    void EnableCore();

    /* Should return unique core ID */
    NODISCARD u32 GetCoreId() const;
};

NODISCARD WRAP_CALL u32 GetCurrentCoreId();

}  // namespace arch

#endif  // ALKOS_KERNEL_HAL_CORE_HPP_
