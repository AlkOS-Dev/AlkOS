#ifndef ALKOS_KERNEL_ABI_CORE_HPP_
#define ALKOS_KERNEL_ABI_CORE_HPP_

#include <extensions/type_traits.hpp>

namespace arch
{
/* Should be defined by architecture, all CPU cores handling and state should be stored here */
class Core;

/* Abi which Core class should follow */
struct CoreABI {
    /* Should perform full initialisation of single core */
    void EnableCore();

    /* Should return unique core ID */
    u32 GetCoreId() const;
};

NODISCARD u32 GetCurrentCoreId();

}  // namespace arch

/* Load architecture definition of component */
#include <abi/core.hpp>
static_assert(
    std::is_base_of_v<arch::CoreABI, arch::Core>, "Core implementation must derive from the ABI"
);

#endif  // ALKOS_KERNEL_ABI_CORE_HPP_
