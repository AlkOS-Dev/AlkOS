#ifndef KERNEL_SRC_HAL_API_CORE_HPP_
#define KERNEL_SRC_HAL_API_CORE_HPP_

#include <defines.hpp>

namespace arch
{

class Core;

struct CoreAPI {
    /* Should perform full initialisation of single core */
    void EnableCore();
};

NODISCARD WRAP_CALL u32 GetCurrentCoreId();

}  // namespace arch

#endif  // KERNEL_SRC_HAL_API_CORE_HPP_
