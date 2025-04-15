#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_

#include <core.hpp>
#include <todo.hpp>

namespace hardware
{
class CoresController final
{
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
    arch::Core
};
}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_
