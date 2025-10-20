#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_

#include "hal/core.hpp"

#include <extensions/data_structures/array_structures.hpp>
#include <extensions/new.hpp>
#include <extensions/template_lib.hpp>
#include <extensions/types.hpp>
#include <todo.hpp>

namespace hardware
{

class CoresController final
{
    public:
    class Core final : public hal::Core
    {
        /* Allow usage of arch constructor */
        using hal::Core::Core;
    };

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
    num
};

}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_
