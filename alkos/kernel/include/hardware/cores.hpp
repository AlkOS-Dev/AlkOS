#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_

#include <extensions/types.hpp>
#include <todo.hpp>

#include "core.hpp"

namespace hardware
{
class CoresController final
{
    class Core final : public arch::Core
    {
    };

    public:
    // ------------------------------
    // Class creation
    // ------------------------------

    CoresController() = default;

    ~CoresController() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void BootUpAllCores();

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    TODO_WHEN_VMEM_WORKS
    alignas(Core) byte mem_[sizeof(Core) * 128]{};
    size_t num_cores_{};
};
}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_
