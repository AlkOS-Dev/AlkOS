#ifndef ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_
#define ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_

#include <extensions/new.hpp>
#include <extensions/template_lib.hpp>
#include <extensions/types.hpp>
#include <todo.hpp>

#include "core.hpp"

namespace hardware
{
class CoresController final
{
    public:
    class Core final : public arch::Core
    {
        /* Allow usage of arch constructor */
        using arch::Core::Core;
    };

    TODO_WHEN_VMEM_WORKS
    static constexpr size_t kTemporaryMaxCores = 128;
    using CoreTable = template_lib::StaticVector<Core, kTemporaryMaxCores>;

    // ------------------------------
    // Class creation
    // ------------------------------

    CoresController() = default;

    ~CoresController() = default;

    // ------------------------------
    // Class interaction
    // ------------------------------

    void BootUpAllCores();

    NODISCARD FORCE_INLINE_F CoreTable& GetCoreTable() { return core_table_; }

    NODISCARD FORCE_INLINE_F const CoreTable& GetCoreTable() const { return core_table_; }

    // ------------------------------
    // Class fields
    // ------------------------------

    private:
    CoreTable core_table_{};
};
}  // namespace hardware

#endif  // ALKOS_KERNEL_INCLUDE_HARDWARE_CORES_HPP_
